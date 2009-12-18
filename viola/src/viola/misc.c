#include "utils.h"
#include <ctype.h>
#include <math.h>
#include "error.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "packet.h"
#include "slotaccess.h"
#include "misc.h"

int cmd_history = 0;
int verbose = 0;

char strBuff[32];

/*
 * parse the string and set the numbers onto the array.
 *
 * RETURN: number of transfered.
 */
int transferNumList2Array(numStr, array, n)
	char *numStr;
	int *array;
	int n;
{
	int count = 0, stri = 0, ai = 0;

	/* make sure to start out with number */
	while (*numStr) if (isdigit(*numStr)) break;

	for (;;) {
		if (isdigit(*numStr)) {
			strBuff[stri++] = *numStr;
		} else {
			if (stri > 0) {
				strBuff[stri] = '\0';
				stri = 0;
				array[ai++] = atoi(strBuff);
				if (++count >= n) return count;
			}
		}
		if (*numStr == '\0') break;
		++numStr;
	}
	return count;
}

int argNumsToInt(argv, n, intBuff)
	Packet argv[];
	int n;
	int intBuff[];
{
	int i;
	for (i = 0; i < n; i++) {
		switch (argv[i].type) {
		case PKT_FLT:
			intBuff[i] = (int)(argv[i].info.f);
		break;
		case PKT_STR:
			intBuff[i] = atoi(argv[i].info.s);
		break;
		case PKT_CHR:
			intBuff[i] = (int)(argv[i].info.c);
		break;
		case PKT_INT:
		default:
			intBuff[i] = argv[i].info.i;
		break;
		}
	}
	return i;
}

VObjList *strOListToOList(str)
	char *str;
{
	char name[100];
	int i = 0;
	VObj *obj;
	VObjList *objl = NULL;

	for (;;) {
		while (ISSPACE(str[i])) i++;
		i = NextWord(str, i, name);
		if (AllBlank(name)) break;
		obj = findObject(getIdent(name));
		if (obj) objl = appendObjToList(objl, obj);
	}
	return objl;
}

/* XXX Sins: uses global buff, 2xstrcat()... 
 */
char *OListToStr(olist)
	VObjList *olist;
{
	buff[0] = '\0';

	for (; olist; olist = olist->next) {
		strcat(buff, GET_name(olist->o));
		strcat(buff, " ");
	}
	return buff;
}

/* XXX Sins: uses global buff, 2xstrcat()... 
 */
char *OListToStrPlusSuffix(olist, suffix)
	VObjList *olist;
	char *suffix;
{
	buff[0] = '\0';

	for (; olist; olist = olist->next) {
		if (olist->o) {
			strcat(buff, GET_name(olist->o));
			strcat(buff, suffix);
			strcat(buff, " ");
		}
	}
	return buff;
}

/*
void setDepentShownInfo(self, attrStrID, position, size)
	VObj *self;
	int attrStrID;
	int position;
	int size;
{
	VObjList *objl;

	for (objl = GET__shownDepend(self); objl; objl = objl->next)
		if (objl->o)
			sendMessage1N2int(objl->o, attrStrID, position, size);
}

void setNotifyShownInfo(self, attrStrID, position, size)
	VObj *self;
	int attrStrID;
	int position;
	int size;
{
	VObjList *objl;

	for (objl = GET__shownNotify(self); objl; objl = objl->next)
		if (objl->o)
			sendMessage1N2int(objl->o, attrStrID, position, size);
}

*/

int makeArgv(argv, argline)
	char *argv[];
	char *argline;
{
	int argc = 0, i = 0, j = 0;

	do {
		j = i;
		i = NextWord(argline, i, buff);
		if (*buff) argv[argc++] = SaveString(buff);
	} while (i != j);
	return argc;
}

/*
 * NOTE: the returned string is stored in shared buffer space.
 */
char *PkInfos2Str(argc, argv)
	int argc;
	Packet argv[];
{
	int i;
	char sbuff[64000]; /*XXX*/

	buff[0] = '\0';

	for (i = 0; i < argc; i++) {
		switch (argv[i].type) {
		case PKT_STR:
			sprintf(sbuff, "%s", argv[i].info.s);
		break;
		case PKT_CHR:
			sprintf(sbuff, "%c", argv[i].info.c);
		break;
		case PKT_INT:
			sprintf(sbuff, "%d", argv[i].info.i);
		break;
		case PKT_FLT:
			sprintf(sbuff, "%f", argv[i].info.f);
		break;
		case PKT_OBJ:
			if (argv[i].info.o)
				sprintf(sbuff, "%s", GET_name(argv[i].info.o));
			else 
				sprintf(sbuff, "(NULL)");
		break;
		case PKT_ARY:
			if (argv[i].info.y) {
				int n;
				Array *array = argv[i].info.y;
				for (n = 0; n < array->size; n++)
					sprintf(sbuff, "%d ", array->info[n]);
			}
		break;
/*		case PKT_STRI:
			if (argv[i].info.si) {
				if (argv[i].info.si.s)
					sprintf("%s", argv[i].info.si.s);
				else
					*sbuff = '\0';
			} else {
				*sbuff = '\0';
			}
		break;
*/
		default:
			sprintf(sbuff, "?");
		break;
		}
/*printf("argc=%d i=%d sbuff=``%s''\n", argc, i, sbuff);*/
		strcat(buff, sbuff);
	}
	return buff;
}

/*
 * NOTE: the returned string is stored in shared buffer space.
 */
char *PkInfo2Str(pk)
	Packet *pk;
{
	switch (pk->type) {
	case PKT_STR:
		return pk->info.s;
	case PKT_CHR:
		sprintf(buff, "%c", pk->info.c);
	break;
	case PKT_INT:
		sprintf(buff, "%d", pk->info.i);
	break;
	case PKT_FLT:
		sprintf(buff, "%f", pk->info.f);
	break;
	case PKT_OBJ:
		if (pk->info.o)
			sprintf(buff, "%s", GET_name(pk->info.o));
		else
			sprintf(buff, "");
	break;
	case PKT_ARY:
		if (pk->info.y) {
			int n;
			Array *array = pk->info.y;
			for (n = 0; n < array->size; n++)
				sprintf(buff, "%d ", array->info[n]);
		}
	break;
/*	case PKT_STRI:
		printf(stderr,"PKT_STRI: NOT IMPLEMENTED XXX\n");
	break;
*/
	default:
		buff[0] = '\0';
	break;
	}
	return buff;
}

float PkInfo2Flt(pk)
	Packet *pk;
{
	switch (pk->type) {
	case PKT_FLT:
		return pk->info.f;
	case PKT_INT:
		return (float)(pk->info.i);
	case PKT_STR:
		return (float)atof(pk->info.s);
	case PKT_CHR:
		return (float)(pk->info.c);
	default:
		return (float)(pk->info.f);
	}
}

char PkInfo2Char(pk)
	Packet *pk;
{
	switch (pk->type) {
	case PKT_CHR:
		return pk->info.c;
	case PKT_STR:
		return pk->info.s[0];
	case PKT_INT:
		return (char)(pk->info.i);	/* ?? */
	case PKT_FLT:
		return (char)(pk->info.f);
	default:
		return (char)pk->info.c;
	}
}

int PkInfo2Int(pk)
	Packet *pk;
{
	switch (pk->type) {
	case PKT_INT:
		return pk->info.i;
	case PKT_STR:
		if (pk->info.s) {
			return atoi(pk->info.s);
		} else {
			fprintf(stderr, 
				"warning: PkInfo2Int() pk->info.s == NULL\n");
			return 0;
		}
	case PKT_FLT:
		return (int)(pk->info.f);
	case PKT_CHR:
		return (int)(pk->info.c);	/* ?? */
	}
	return (int)(pk->info.i);
}

/* load if necessary 
 */
VObj *getObject(objName)
	char *objName;
{
	int symID;
	VObj *obj = NULL;

	symID = getIdent(trimEdgeSpaces(objName));
	if (symID) obj = findObject(symID);
	if (obj) {
		return obj;
	} else { 
		char *fname, *objname;
		int length;
		HashEntry *entry;

		length = sizeof(char) * (strlen(objName) + 3);
		fname = (char*)malloc(length);
		objname = (char*)malloc(length);
		strcpy(fname, objName);
		strcpy(objname, objName);
		length = strlen(fname);
		if (length >= 2) {
			if (fname[length-2] != '.' ||
			    fname[length-1] != 'v')
				strcat(fname, ".v");
		}
		load_object(fname, NULL);
		entry = objID2Obj->get(objID2Obj, storeIdent(objname));
		free(fname);
		if (entry) return (VObj*)entry->val;
	}
	return NULL;
}

VObj *PkInfo2Obj(pk)
	Packet *pk;
{
	VObj *obj = NULL;

	if (pk->type == PKT_OBJ) {
		return pk->info.o;
	} else if (pk->type == PKT_STR) {
		if (pk->info.s) return getObject(pk->info.s);
	}
	return NULL;
}


