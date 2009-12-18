/*
 * loader.c
 */
#include "utils.h"
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#include <unistd.h> /*for R_OK*/
#include <sys/file.h>
#include "mystrings.h"
#include "sys.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "classlist.h"
#include "slotaccess.h"
#include "loader.h"
#include "file.h"
#include "misc.h"

#include "objs.h"/*XXXX for hard-coded ViolaWWW objects */
int loadFromBuiltInCache();
int load_objects_slots_fromBuiltInCache();


/* this puts a limit on the number of objects in one file */
#define NEWOBJ_SIZE 1000

#define readFileTillChar(fp, tc) \
	while (c = getc(fp)) if (c == EOF || c == tc) break;
char c;

#define VIOLA_PATH_ARRAY_SIZE	32
char *viola_path[VIOLA_PATH_ARRAY_SIZE];

#define DFLT_VIOLA_PATH "/usr/local/lib/WWW/viola/apps" /*PATCHED*/


#ifdef unusednotyet
#define INPUT(fp, c) {\
	if (INPUTbuffi > INPUTBUFFSIZE) {\
		fread(INPUTbuff, INPUTBUFFSIZE, 1, fp);\
		INPUTbuffi = 0;\
	}\
	c = INPUTbuff + INPUTbuffi;\
}

#define INIT_INPUT(fp) {\
	fread(INPUTbuff, INPUTBUFFSIZE, 1, fp);\
	INPUTbuffi = 0;\
}

#define UN_INPUT(fp, c) {\
	if (--INPUTbuffi < 0) {\
		fread(INPUTbuff, LIMIT, 1, fp);\
		INPUTbuffi = 0;\
	}\
}

#define INPUTBUFFSIZE 128
int INPUTbuffi;
char INPUTbuff[INPUTBUFFSIZE];
#endif

int init_loader(path)
	char *path;
{
	if (path) {
		if (setViolaPath(path) > 0) {
			if (verbose) 
				fprintf(stderr, "violaPath set to: ``%s''\n", 
					path);
			return 1;
		}
	}
	return 0;
}

/*
 * Returns:
 *	number of paths set.
 *	0, if not set.
 *	-i, if exceed VIOLA_PATH_ARRAY_SIZE (which is = i)
 */
int setViolaPath(newPathStr)
	char *newPathStr;
{
	int i, stri;
	char *cp, pathStr[BUFF_SIZE], buff[BUFF_SIZE];

	for (i = 0, stri = 0; i < VIOLA_PATH_ARRAY_SIZE; i++) {
		SkipBlanks(newPathStr, &stri);
		stri = NextWord(newPathStr, stri, pathStr);
		if (AllBlank(pathStr)) {
			viola_path[i] = NULL;
			return i;
		} else {
			if (viola_path[i]) free(viola_path[i]);
			if (cp = vl_expandPath(pathStr, buff))
				viola_path[i] = SaveString(cp);
		}
	}
	viola_path[VIOLA_PATH_ARRAY_SIZE] = NULL;
	return -i;
}

/*
 * return:	number of objects loaded
 * 		-1 if failed to open object file
 *		use VIOLA_PATH if path is not given (NULL)
 */
int load_object(filename, pathname)
	char *filename;
	char *pathname;
{
	FILE *fp;
	int slotv[100][2], slotc, *slotp;
	int i, status = 1, newObjCount;
	VObj *obj, *newObj[NEWOBJ_SIZE];
	char path[256]; /*XXX*/

	newObjCount = 0;

	if (filename[0] == '\0') {
/*	  fprintf(stderr, "can't find \"%s\"\n", filename);*/
	  return -1;
	}
	if (pathname == NULL) {
		for (i = 0; i < VIOLA_PATH_ARRAY_SIZE; i++) {
			if (viola_path[i]) {
				strcpy(path, viola_path[i]);
				strcat(path, "/");
				strcat(path, filename);
				if (access(path, R_OK) == 0) break;
			} else {
/*				fprintf(stderr, 
					"can't find \"%s\"\n", filename);
*/
				return loadFromBuiltInCache(filename);
			}
		}
	} else {
		strcpy(path, pathname);
		strcat(path, filename);
	}
	if ((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "can't find file %s, path=%s\n", 
				filename, path);
		return -1;
	}

	while (status) {
		slotc = 0;
		status = load_objects_slots(fp, slotv, &slotc);
		if (slotc > 0) {
			if (obj = instantiateObj(slotv, &slotc)) {
				objID2Obj->put_replace(objID2Obj, 
				     storeIdent(saveString(GET_name(obj))),
				     (int)obj);
				objObj2ExistP->put_replace(objObj2ExistP, 
							   obj, 1);
				newObj[newObjCount++] = obj;
				if (newObjCount > NEWOBJ_SIZE) {
					fprintf(stderr, 
				    "Internal error: newObj[] overflowed.\n");
				}
			} else {
				fprintf(stderr,
					"failed to build object...\n");
			}
		}
	}

	fclose(fp);
/*
	fprintf(stderr, "--------------------------------\n");
	for (i = 0; i < slotc; i++) {
		if (slotv[i][0])
			fprintf(stderr,
				"labelid=%d, content=\"%s\"\n",
				(int)slotv[i][0], 
				(char*)slotv[i][1]);
	}
	fprintf(stderr, "--------------------------------\n");
*/
	/* for all objects just loaded, init them */
	/* THIS IS NOT CORRECT! initialization method should be called
	 * without entering script *first*!
	 */
	for (i = 0; i < newObjCount; i++) {

	/*XXX later, put class mg size in class public space */
/*	SET__memoryGroup(newObj[i], newMemoryGroup(1024));
*/
		sendMessage1(newObj[i], "init");
	}

	return newObjCount;
}

#define SLOT_LABEL_SIZE 64
/*
 * Return: zero if end of file is encountered
 *
 * Size of slot labels are limited to SLOT_LABEL_SIZE characters.
 * Size of slot content has no limitation.
 *
 * someday, the reading ought to be buffer read...
 */
int load_objects_slots(fp, slotv, slotc)
	FILE *fp;
	int (*slotv)[100][2];
	int *slotc;
{
	char label[SLOT_LABEL_SIZE], *dynaBuff;
	int labeli, sloti, dynaBuffi;
	long seekBackPosition = 0;
	int done = 0;
	int llc = 0, lc = 0, c, nc;
	HashEntry *entry;

	for (;;) {

		/* read the slot label */
		labeli = 0;
		llc = 0; lc = 0;
		for (;;) {
			c = fgetc(fp);

			if (c == EOF) {
				return 0;
			} else if (c == '\\') {
				if (lc == '\\') {
					/* \\comments */
					readFileTillChar(fp, '\n');
					labeli = 0;
					llc = 0; lc = 0;
				} else if (lc == '\n') {
					return 1;
				}
			} else if (c == '{') {
				break;
			} else if (!isspace(c)) {
				label[labeli++] = c;
			}
			if (labeli > SLOT_LABEL_SIZE) {
				label[labeli] = '\0';
				fprintf(stderr,
				      "unreasonably large slot label:\"%s\"\n",
					label);
				return NULL;
			}
			llc = lc; lc = c;
		}
		label[labeli++] = '\0';

/*		printf("..label = \"%s\"\n", label);*/

		/* read the slot content */
		sloti = 0;
		llc = 0; lc = 0;
		for (;;) {
			c = fgetc(fp);
			if (c == EOF) {
				if (lc == '\n') {
					done = 1;
					goto donecontent;
				} else if (lc == '}') {
					--sloti;
					done = 1;
					goto donecontent;
				} else {
					fprintf(stderr, "premature EOF...\n");
					return 0;
				}
			} else if (c == '\\') {
				if (llc == '}' && lc == '\n') {
					nc = getc(fp);
					ungetc(nc, fp);
					if (nc != '*') {
					/* it's not "}"+left comment case */
						goto donecontent;
					}
				}
				if (llc == '\r' && lc == '\n') {/*XXX*/
					--sloti;
					nc = getc(fp);
					ungetc(nc, fp);
					if (nc != '*') {
					/* it's not "}"+left comment case */
						goto donecontent;
					}
				}
			}
			if (!seekBackPosition) {
				if (sloti >= BUFF_SIZE - 1) {
					dynaBuffi = sloti;
					seekBackPosition = ftell(fp);
				} else {
					buff[sloti++] = c;
				}
			} else {
				sloti++;
			}
			llc = lc; lc = c;
		}
donecontent:
		if (entry = symStr2ID->get(symStr2ID, (int)label)) {	
			(*slotv)[*slotc][0] = (int)(entry->val);
		} else {
			fprintf(stderr,	"unknown slot label:\"%s\"\n", label);
			return 0;
		}
/*
		fprintf(stderr,	"slotc = %d\n", *slotc);
		fprintf(stderr, "******label str id = %d\n",
			(*slotv)[*slotc][0]);
*/
		if (seekBackPosition) {
/*
			printf("dynaBuffi = %d\n", dynaBuffi);
			printf("seekBackPosition = %d, sloti = %d\n", 
				seekBackPosition, sloti);
*/
			dynaBuff = (char*)malloc(sizeof(char) * (sloti + 2));
			strncpy(dynaBuff, buff, dynaBuffi);
			if (sloti - dynaBuffi > 0) {
				if (fseek(fp, seekBackPosition - 1, SEEK_SET)
					== -1) return 0;
				if (fread(&dynaBuff[dynaBuffi], sizeof(char),
					  sloti - dynaBuffi + 2, fp) == 0)
					return 0;
			}
			dynaBuff[sloti - 1] = '\0';
/*			printf("dynabuff=\"%s\"\n", dynaBuff);*/
			seekBackPosition = 0;
			(*slotv)[*slotc][1] = (int)dynaBuff;
		} else {
			char *cp = (char*)malloc(sizeof(char) * (sloti - 1));
			buff[sloti - 2] = '\0';
			(*slotv)[*slotc][1] = (int)strcpy(cp, buff);
/*			printf("buff=\"%s\"\n", buff);*/
		}
/*
		fprintf(stderr,
			"** slot label id = %d, slot content=\"%s\"\n",
			(int)(*slotv)[*slotc][0], (char*)(*slotv)[*slotc][1]);
*/
		(*slotc)++;
		if (done) return 1;
	}
}

/*
 * Return: zero if end of file is encountered
 *
 * Size of slot labels are limited to SLOT_LABEL_SIZE characters.
 * Size of slot content has no limitation.
 */
int load_objects_slots_fromBuiltInCache(slotsInfo, slotsInfoIdx, slotv, slotc)
	SlotStruct *slotsInfo;
	int *slotsInfoIdx;
	int (*slotv)[100][2];
	int *slotc;
{
	HashEntry *entry;
	char *label, *cp;

	for (;;) {
		/* read the slot label
		 */
/*
printf("CONSIDERING: {%s} {%s}\n", 
       slotsInfo[(*slotsInfoIdx)].slotName, 
       slotsInfo[(*slotsInfoIdx)].slotValue);
*/
		label = slotsInfo[(*slotsInfoIdx)].slotName;
		if (!label) {
			if ((int)slotsInfo[(*slotsInfoIdx)++].slotValue) {
				/* 1== end of object reached */
				return 1;
			} else {
				/* 0== end of object-file reached */
				return 0;
			}
		}

		if (entry = symStr2ID->get(symStr2ID, (int)label)) {	
			(*slotv)[*slotc][0] = (int)(entry->val);
		} else {
			fprintf(stderr,	"unknown slot label:\"%s\"\n", label);
			return 0;
		}

		/* read the slot content 
		 */
		(*slotv)[*slotc][1] = 
		    (int)saveString(slotsInfo[(*slotsInfoIdx)++].slotValue);

		(*slotc)++;
	}
}

/* load from objects specified in objs.c, if it's there.
 */
int loadFromBuiltInCache(filename)
	char *filename;
{
	int slotv[100][2], slotc, *slotp;
	int i, status = 1, newObjCount;
	VObj *obj, *newObj[NEWOBJ_SIZE];
	char path[256]; /*XXX*/
	SlotStruct *slotsInfo = NULL;
	int slotsInfoIdx = 0;

/*	fprintf(stderr, "TRYING to load objects from \"%s\"\n", filename);
*/
	newObjCount = 0;

	for (i = 0; ; i++) {
		if (!objsNames[i].name) return -1;
		if (!STRCMP(objsNames[i].name, filename)) {
			slotsInfo = objsNames[i].slots;
			break;
		}
	}
	if (!slotsInfo) return -1;

	while (status) {
		slotc = 0;
		status = load_objects_slots_fromBuiltInCache(
					slotsInfo, &slotsInfoIdx,
					slotv, &slotc);
		if (slotc > 0) {
			if (obj = instantiateObj(slotv, &slotc)) {
				objID2Obj->put_replace(objID2Obj, 
				     storeIdent(saveString(GET_name(obj))),
				     (int)obj);
				objObj2ExistP->put_replace(objObj2ExistP,
							   obj, 1);
				newObj[newObjCount++] = obj;
				if (newObjCount > NEWOBJ_SIZE) {
					fprintf(stderr, 
				    "Internal error: newObj[] overflowed.\n");
				}
			} else {
				fprintf(stderr,
					"failed to build object...\n");
			}
/*			fprintf(stderr, "--------------------------------\n");
			for (i = 0; i < slotc; i++) {
				if (slotv[i][0])
					fprintf(stderr,
						"labelid=%d, content=\"%s\"\n",
						(int)slotv[i][0], 
						(char*)slotv[i][1]);
			}
			fprintf(stderr, "--------------------------------\n");
*/
		}
	}

	/* for all objects just loaded, init them */
	/* THIS IS NOT CORRECT! initialization method should be called
	 * without entering script *first*!
	 */
	for (i = 0; i < newObjCount; i++) {
		sendMessage1(newObj[i], "init");
	}
	return newObjCount;
}

