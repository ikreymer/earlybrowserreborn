/*
 * Copyright 1991 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * class.c
 */
#include "utils.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "classlist.h"
#include "slotaccess.h"
#include "attr.h"
#include "glib.h"

#define INTARRAYBUFF_SIZE 32
int intArrayBuff[INTARRAYBUFF_SIZE];

int loadClassScriptsP = 0;

long initSlot();

MHInfo allmhp;

/*
 * initialize the slot offset values 
 */
int initSlotOffsetInfo(cip, section)
	ClassInfo *cip;
	int section;
{
	SlotInfo *sip;
	int i, offset;

	if (cip->superClass) {
		offset = initSlotOffsetInfo(cip->superClass, section);
	} else {
		offset = 0;
	}
	for (sip = cip->slots[section]; sip->id; sip++) {
		sip->offset = offset++;
		sip->section = section;
	}
	return offset;
}

void dumpSlotInfo(cip, section)
	ClassInfo *cip; 
	int section;
{
	SlotInfo *sip;
	int i, offset;

	if (cip->superClass) dumpSlotInfo(cip->superClass, section);
	for (i = 0, sip = cip->slots[section]; sip[i].id; i++) {
		printf("idx = %d\n", i);
		printf("\tid = %d \"%s\"\n", 
		       sip[i].id,
		       (char*)symID2Str->get(symID2Str, sip[i].id)->val);
		printf("\ttype = %d\n", sip[i].flags & SLOT_MASK_TYPE);
		printf("\tval = %d\n", sip[i].val);
		printf("\toffset = %d\n", sip[i].offset);
		printf("\tsection = %d\n", sip[i].section);
		printf("\ttmp = %d\n", sip[i].tmp);
	}
}

void dumpSlotLookup(sip, size)
	SlotInfo **sip;
	int size;
{
	int i;

	for (i = 0; i < size; i++) {
		printf("idx = %d\n", i);
		printf("\tid = %d \"%s\"\n", 
		       sip[i]->id,
		       (char*)symID2Str->get(symID2Str, sip[i]->id)->val);
		printf("\ttype = %d\n", sip[i]->flags & SLOT_MASK_TYPE);
		printf("\tval = %d\n", sip[i]->val);
		printf("\toffset = %d\n", sip[i]->offset);
		printf("\tsection = %d\n", sip[i]->section);
		printf("\ttmp = %d\n", sip[i]->tmp);
	}
}

/* 
 * 
 */
int initSlotLookUpTable(cip, section, slookup)
	ClassInfo *cip;
	int section;
	SlotInfo **slookup;
{
	SlotInfo *sip;
	int offset, i;

	if (cip->superClass)
		offset = initSlotLookUpTable(cip->superClass, section,slookup);
	else
		offset = 0;

	/* copy in the new slots for the class */
	for (sip = cip->slots[section]; sip->id; sip++)
		slookup[offset++] = sip;

	return offset;
}

/* 
 * ``section'' can only be 0 or 1 (new common or private slots)
 */
void overRideSlotLookUpTable(cip, section, slookup, slookup_size)
	ClassInfo *cip;
	int section;
	SlotInfo **slookup;
	int slookup_size;
{
	SlotInfo *sip;
	int slookupi;
	int offset, i;

	if (cip->superClass) 
		overRideSlotLookUpTable(cip->superClass, section, slookup,
					slookup_size);

	/* copy in the new slots for the class */
	for (i = 0, sip = cip->slots[section + 2]; sip[i].id; i++)
		for (slookupi = 0; slookupi < slookup_size; slookupi++)
			if (sip[i].id == slookup[slookupi]->id)
				slookup[slookupi] = &(sip[i]);
}

/* 
 */
int init_class()
{
	ClassInfo *base_clip, *clip;
	ClassInfo *cip, *tcip;
	SlotInfo *sip;
	VObj *obj;
	int offset, i, j;
	int load_classScripts();
	char classScriptPathBuff[128];

/*	extern StrInfo init_obj_script;*/
/*	init_obj_script = (StrInfo*)malloc(sizeof(struct StrInfo));*/
/*	init_obj_script.s = saveString("usual();");
	init_obj_script.refc = 1;
*/
	for (i = 0; cip = classList[i]; i++) {
		/* count the number of slots defined in each new class */
		cip->common_newcount = 0;
		for (j = 0, sip = (SlotInfo*)cip->slots[0];
		     sip[j++].id; 
		     cip->common_newcount++);
		cip->newcount = 0;
		for (j = 0, sip = (SlotInfo*)cip->slots[1];
		     sip[j++].id; 
		     cip->newcount++);
	}

	for (i = 0; cip = classList[i]; i++) {
		cip->totalcount = 0;
		cip->common_totalcount = 0;
		for (tcip = cip; tcip; tcip = tcip->superClass) {
			cip->totalcount += tcip->newcount;
			cip->common_totalcount += tcip->common_newcount;
		}
	}

	/* allocate and initialize slot lookup tables */
	for (i = 0; cip = classList[i]; i++) {
		cip->common_slookup = 
			(SlotInfo**)malloc(sizeof(struct SlotInfo*)
					 * cip->common_totalcount);
		cip->slookup =
			(SlotInfo**)malloc(sizeof(struct SlotInfo*)
					 * cip->totalcount);
		if (cip->common_totalcount > 0)
			initSlotLookUpTable(cip, 0, cip->common_slookup);
		if (cip->totalcount > 0)
			initSlotLookUpTable(cip, 1, cip->slookup);
	}
/*
	for (i = 0; cip = classList[i]; i++) {
		printf("----------\n");
		dumpSlotInfo(cip, 0);
	}
	for (i = 0; cip = classList[i]; i++) {
		printf("==========\n");
		dumpSlotInfo(cip, 1);
	}
*/
	/* overrides inherited slot info with local class ones
	 */
	for (i = 0; cip = classList[i]; i++) {
		overRideSlotLookUpTable(cip, 0,
					cip->common_slookup, 
					cip->common_totalcount);
		overRideSlotLookUpTable(cip, 1,
					cip->slookup, 
					cip->totalcount);
		initSlotOffsetInfo(cip, 0);
		initSlotOffsetInfo(cip, 1);
	}

	/* set membership hints for methods 
	 */
	allmhp.bitsSize = 1024;
	allmhp.bits = (long*)malloc(sizeof(long) * allmhp.bitsSize);
	bzero(allmhp.bits, sizeof(long) * allmhp.bitsSize);
	allmhp.hits = 0;
	allmhp.misses = 0;
	allmhp.collisions = 0;
	allmhp.refc = 0;

	for (i = 0; cip = classList[i]; i++) {
		int bitsSize;
		HashEntry *entry;
		HashTable *methHashTable;
		MethodInfo *mip;
		int id;

		for (j = 0; cip->methods[j].id; j++); 
		bitsSize = j + 1;
		cip->mhp.bitsSize = bitsSize;
		cip->mhp.bits = (long*)malloc(sizeof(long) 
						* cip->mhp.bitsSize);
		bzero(cip->mhp.bits, sizeof(long) * cip->mhp.bitsSize);
		cip->mhp.hits = 0;
		cip->mhp.misses = 0;
		cip->mhp.collisions = 0;
		cip->mhp.refc = 0;

		cip->mht = methHashTable = 
			initHashTable(127, hash_int, cmp_int, NULL, NULL,
				      getHashEntry_int, 
				      putHashEntry_int, 
				      putHashEntry_replace_int,
				      removeHashEntry_int);

		for (mip = cip->methods; id = mip->id; mip++) {
			setMember(&(cip->mhp), id);
			setMember(&(allmhp), id);
		}
		for (tcip = cip; tcip; tcip = tcip->superClass) {
			for (mip = tcip->methods; id = mip->id; mip++) {
				putHashEntry_cancelable_int(methHashTable, 
					mip->id, (int)mip->method);
			}
		}
/*
		entry = symID2Str->get(symID2Str, (long)cip->id);
		printf("#### %6d class= %20s  %6ld\n", 
		       bitsSize, (char*)entry->val, cip->mhp.collisions);
		dumpHashTable(methHashTable);
*/
	}

/*
	for (i = 0; cip = classList[i]; i++) {
		printf("--------------------------\n");
		dumpSlotLookup(cip->common_slookup, cip->common_totalcount);
	}
	for (i = 0; cip = classList[i]; i++) {
		printf("==========================\n");
		dumpSlotLookup(cip->slookup, cip->totalcount);
	}
*/
	/*
	 * build common objects
	 */
	for (i = 0; cip = classList[i]; i++) {
		obj = (VObj*)malloc(sizeof(long) * cip->common_totalcount);
		for (j = 0; j < cip->common_totalcount; j++)
			initSlot(obj, &obj[j],
				 cip->common_slookup[j],
				 cip->common_slookup[j]->val);
		cip->common = obj;
	}

	/*
	 * override hard coded classScript with loaded ones
	 */
	if (loadClassScriptsP) 
		if (vl_expandPath(".", classScriptPathBuff)) {
			load_classScripts(classScriptPathBuff);
		}
	return 1;
}

int load_classScripts(classScriptPath)
	char *classScriptPath;
{
	HashEntry *entry;
	ClassInfo *cip;
	char *classScript;
	int i;

	for (cip = classList; cip; cip++) {
		if (entry = symID2Str->get(symID2Str, (long)cip->id)) {
			sprintf(buff, "%s/cs_%s.vs", 
				classScriptPath, (char*)entry->val);
			if (loadFile(buff, &classScript) != -1) {
/*				printf(">>>old %s {%s}\n", buff, (char*)cip->common[0]);
				printf(">>>new %s {%s}\n", buff, classScript);
*/
				cip->common[SLOT_IDX_classScript] = 
					(long)classScript;
			}
		}
	}
	return 1;
}

long initSlot(self, slotp, sip, val)
	VObj *self;
	long *slotp;
	SlotInfo *sip;
	long val;
{
	switch (sip->flags & SLOT_MASK_TYPE) {

	case LONG:
	case PTRV:
	case ATTR:
	case OBJP:
	case OBJL:
	case PCOD:
	case CLSI:
	case TFLD:
		return *slotp = val;

	case RGBV:
		{
		  ColorInfo *ci;
		  ci = (ColorInfo*)malloc(sizeof(struct ColorInfoStruct));
		  ci->bd = NULL;
		  ci->bg = NULL;
		  ci->fg = NULL;
		  ci->cr = NULL;
		  return *slotp = ci;
		}

	case STRI:
		{
		  StrInfo *si = (StrInfo*)malloc(sizeof(struct StrInfo));
		  si->s = saveString((char*)val);
		  si->refc = 1;
		  return *slotp = (long)si;
		}

	case ARRY: 
		if (val) {
			Array *array = (Array*)malloc(sizeof(struct Array));
			array->size = transferNumList2Array((char*)val,
					intArrayBuff, INTARRAYBUFF_SIZE);
			array->info = (int*)malloc(sizeof(int) * array->size);
			bcopy(intArrayBuff, array->info, 
					sizeof(int) * array->size);
			return *slotp = (long)array;
		}
		return NULL;

	case PTRS:
		return *slotp = (long)saveString((char*)val);

	case FUNC:
		return *slotp = ((long (*)())val)();

	case PROC:
		return ((long (*)())val)(self, slotp);

	default:
		fprintf(stderr, 
			"initSlot(): unknown slot type = %d. Setting to NULL.\n",
			sip->flags & SLOT_MASK_TYPE);
		return NULL;
	}
}

char *returnSlotCast(type)
	int type;
{
	switch (type) {
	case PTRV: return "";	/* don't specify type. let it be implicit */
	case ATTR: return "(Attr*)";
	case OBJP: return "(VObj*)";
	case OBJL: return "(VObjList*)";
	case PTRS:
	case PTRA: return "(char*)";
	case ARRY: return "(int*)";
	case TFLD: return "(TFStruct*)";
	case CLSI: return "(ClassInfo*)";
	case PCOD: return "(union PCode*)";
	case LONG:
	case PROC:
	case FUNC:
	default:   return "(long)";
	}
}

#ifdef NOTUSED
int generateSlotAccessors() 
{
	FILE *fp;
	ClassInfo *cip;
	SlotInfo *sip;
	char *slotCast;
	int i, j;

	if ((fp = fopen("slotaccess.h", "w+")) == NULL) {
		fprintf(stderr, "failed to generate slotaccess.h\n");
		return 0;
	}

	for (i = 0; cip = classList[i]; i++) {

		fprintf(fp,
			"\n/*******************\n * class %s\n */\n",
			(char*)symID2Str->get(symID2Str, cip->id)->val);

		fprintf(fp,	"/* common slots accessors */\n");
		for (j = 0; j < cip->common_totalcount; j++) {
			
			sip = cip->common_slookup[j];
			slotCast = returnSlotCast(sip->flags & SLOT_MASK_TYPE);
			
			fprintf(fp,
				"#define GET_%s(o) (%s((ClassInfo*)((o)[0])->common[%d]))\n",
				(char*)symID2Str->get(symID2Str, sip->id)->val,
				slotCast, sip->offset);
			fprintf(fp,
				"#define SET_%s(o,v) (%s((ClassInfo*)((o)[0])->common[%d]=(long)(v)))\n\n",
				(char*)symID2Str->get(symID2Str, sip->id)->val,
				slotCast, sip->offset);
		}

		fprintf(fp,	"/* private slots accessors */\n");
		for (j = 0; j < cip->totalcount; j++) {

			sip = cip->slookup[j];
			slotCast = returnSlotCast(sip->flags & SLOT_MASK_TYPE);
/*
			fprintf(fp,
				"#define SDX_%s %d\n",
				(char*)symID2Str->get(symID2Str, sip->id)->val,
				sip->offset);
*/
			fprintf(fp,
				"#define GET_%s(o) (%s((o)[%d]))\n",
				(char*)symID2Str->get(symID2Str, sip->id)->val,
				slotCast, sip->offset);
			fprintf(fp,
				"#define SET_%s(o,v) (%s((o)[%d]=(long)(v)))\n\n",
				(char*)symID2Str->get(symID2Str, sip->id)->val,
				slotCast, sip->offset);
		}
	}
	fclose(fp);
	return 1;
}
#endif

ClassInfo *getClassInfoByID(classid)
	int classid;
{
	ClassInfo *cip;
	HashEntry *entry;
	int i;

	for (i = 0; cip = classList[i]; i++)
		if (classid == cip->id) return cip;

	return 0;
}

ClassInfo *getClassInfoByName(className)
	char *className;
{
	ClassInfo *cip;
	HashEntry *entry;
	int classid, i;

	if (entry = symStr2ID->get(symStr2ID, (int)className)) {
		classid = entry->val;
		for (i = 0; cip = classList[i++]; cip++)
			if (classid == cip->id) return cip;
	}
	return 0;
}


VObj *buildObjWithLoadedSlots(cip, slotv, slotc)
	ClassInfo *cip;
	int slotv[100][2];
	int slotc;
{
	VObj *obj = (VObj*)malloc(sizeof(long) * cip->totalcount);
	SlotInfo *sip;
	long val;
	int i, j;

	if (!obj) return 0;

	for (i = 0; i < cip->totalcount; i++) {
		sip = cip->slookup[i];
		for (j = 0; j < slotc; j++) {
			if (sip->id == slotv[j][0]) {
				sip->tmp = (long)slotv[j];
				goto next;
			}
		}
		sip->tmp = 0;
		next:;
	}
	for (i = 0; i < cip->totalcount; i++) {
		sip = cip->slookup[i];
		if (sip->tmp) {
/*			printf("%d file   \t%s\n",
				i, 
			       (char*)symID2Str->get(symID2Str, sip->id)->val);
*/
			if ((sip->flags & SLOT_MASK_TYPE) == LONG) {
				val = atoi((char*)((int*)sip->tmp)[1]);
			} else {
				val = (long)((int*)sip->tmp)[1];
			}
		} else {
/*			printf("%d default\t%s\n",
				i, 
				(char*)symID2Str->get(symID2Str, sip->id)->val);
*/
			val = (long)(sip->val);
		}
		initSlot(obj, &obj[i], sip, val);
	}

	return obj;
}

long *searchSlot(slotv, slotc, key)
	int (*slotv)[100][2];
	int slotc;
	int key;
{
	int i;

	for (i = 0; i < slotc; i++)
		if ((*slotv)[i][0] == key) return (long*)(*slotv)[i];

	return 0;
}

VObj *instantiateObj(slotv, slotc)
	int (*slotv)[100][2];
	int *slotc;
{
	long *slotp;
	int i;
	char *objName;
	HashEntry *entry;
	VObj *obj;
	ClassInfo *cip;

	slotp = searchSlot(slotv, *slotc, STR_name);
	if (!slotp) {
		fprintf(stderr, 
			"Error: instantiateObj(): no name specified.\n");
		return NULL;
	}
	objName = (char*)slotp[1];

	slotp = searchSlot(slotv, *slotc, STR_class);
	if (!slotp) {
		fprintf(stderr,
	"instantiateObj(): no class specified for obj %s. Using generic.\n",
			objName);
		(*slotv)[*slotc][0] = STR_class;
		(*slotv)[*slotc][1] = (int)saveString("generic");
		slotp = (*slotv)[*slotc];
		++(*slotc);
	}
/*
	fprintf(stderr,
		"class id = %d, class name=\"%s\"\n",
		(int)(slotp)[0], (char*)(slotp)[1]);
*/
	entry = symStr2ID->get(symStr2ID, (int)slotp[1]);
	if (!entry) {
		fprintf(stderr,
			"unknown class \"%s\" (not in symbolic table).\n",
			(char*)slotp[1]);
		return NULL;
	}
	cip = getClassInfoByID(entry->val);
	if (!cip) {
		fprintf(stderr,
			"unknown class \"%s\" (not in class info list).\n",
			(char*)slotp[1]);
		return NULL;
	}

	/* eliminate the "class" slot by replacing it with 
	 * the last slot 
	 */
	free((char*)slotp[1]);
	--(*slotc);
	slotp[0] = (*slotv)[*slotc][0];
	slotp[1] = (*slotv)[*slotc][1];
/*	print("----> \"%s\"\n", slotp[1]);
*/
	obj = buildObjWithLoadedSlots(cip, slotv, *slotc);

	for (i = 0; i < *slotc; i++) {
		/*print("FREE'ing: \"%s\"\n", (char*)(*slotv)[i][1]);*/
		free((*slotv)[i][1]);
	}
	if (securityMode > 0) SET_security(obj, securityMode);

	return obj;
}

int saveSelfAndChildren(obj, fp)
	VObj *obj;
	FILE *fp;
{
	VObjList *olist;

	fprintf(fp, "\\class {%s}\n", GET_class(obj));

	dumpObj(obj, fprintf, fp, SLOT_W, 1);
	for (olist = GET__children(obj); olist; olist = olist->next)
		saveSelfAndChildren(olist->o, fp);

	return 1;
}

/*
 * flag: 1=dump only if not same as default
 */
int dumpObj(obj, dumpFunc, dumpDest, filter, flag)
	VObj *obj;
	long (*dumpFunc)();
	FILE *dumpDest;
	int filter;
	int flag;
{
	SlotInfo *sip;
	VObjList *olist;
	ClassInfo *cip;
	int i;

	cip = GET__classInfo(obj);

	for (i = 0; i < cip->totalcount; i++) {
		sip = cip->slookup[i];
		if ((sip->flags & filter) != filter) continue;
		if (flag) if (sip->val == obj[i]) continue;

		dumpFunc(dumpDest, "\\%s {",
			(char*)symID2Str->get(symID2Str, sip->id)->val);
	
		switch (sip->flags & SLOT_MASK_TYPE) {
		case LONG:
			dumpFunc(dumpDest, "%d}\n", obj[i]);
		break;

		case PTRV:
			dumpFunc(dumpDest, "%x}\n", obj[i]);
		break;

		case OBJP:
			if (obj[i])
				dumpFunc(dumpDest, "%s}\n", 
					GET_name((VObj*)obj[i]));
			else
				dumpFunc(dumpDest, "NULL}\n");
		break;

		case OBJL:
			for (olist = (VObjList*)obj[i]; olist; 
			     olist = olist->next) {
				if (olist->o) 
					dumpFunc(dumpDest, "\"%s\"", 
						GET_name(olist->o));
				else 
					dumpFunc(dumpDest, "NULL");
				if (olist->next) 
					dumpFunc(dumpDest, ",");
			}
			dumpFunc(dumpDest, "}\n");
		break;

		case ATTR:
			dumpFunc(dumpDest, "attrs %x = {", obj[i]);
/*
			for (olist = (VObjList*)obj[i]; olist; 
				olist = olist->next)
				dumpFunc(dumpDest, "\"%s\"", 
					GET_name(olist->o));
*/
			if (obj[i])
				dumpVarList((Attr*)obj[i]);
			else 
				dumpFunc(dumpDest, "NULL");
			dumpFunc(dumpDest, "}\n");
		break;

		case PTRS:
		case PTRA:
			dumpFunc(dumpDest, "%s}\n", (char*)obj[i]);
		break;

		case RGBV: {
			ColorInfo *ci = (ColorInfo*)obj[i];
			if (ci->bd)
				dumpFunc(dumpDest, "bd=%s}\n", ci->bd->name);
			else 
				dumpFunc(dumpDest, "bd=NULL}\n");

			if (ci->bg)
				dumpFunc(dumpDest, "bg=%s}\n", ci->bg->name);
			else 
				dumpFunc(dumpDest, "bg=NULL}\n");

			if (ci->cr)
				dumpFunc(dumpDest, "cr=%s}\n", ci->cr->name);
			else 
				dumpFunc(dumpDest, "cr=NULL}\n");

			if (ci->fg)
				dumpFunc(dumpDest, "fg=%s}\n", ci->fg->name);
			else 
				dumpFunc(dumpDest, "fg=NULL}\n");
		}
		break;

		case ARRY: {
			int n;
			Array *array = (Array*)obj[i];
			if (array) {
				for (n = 0; n < array->size; n++) {
					dumpFunc(dumpDest, "%d ", 
						array->info[n]);
				}
				dumpFunc(dumpDest, "}\n");
			} else {
				dumpFunc(dumpDest, "NULL}\n");
			}
		}
		break;

		case TFLD:
			dumpFunc(dumpDest, "tfstruct ptr=%d}\n", 
				obj[i]);
		break;
		case PROC:
		case FUNC:
		default:
			dumpFunc(dumpDest, "unknown val=%d}\n", obj[i]);
		break;
		}
	}
	dumpFunc(dumpDest, "\\\n");

	return 1;
}

VObj *clone(original)
	VObj *original;
{
	VObj *clone;
	ClassInfo *cip;
	SlotInfo *sip;
	register int i, j;
	VObj *clonep;
	VObj *originalp;

	cip = GET__classInfo(original);
	clone = (VObj*)malloc(sizeof(long) * cip->totalcount);
	if (!clone) return 0;

/*printf("cloning original '%s'\n", GET_name(original));*/

	clonep = clone;
	originalp = original;

	for (i = 0, j = cip->totalcount; i < j; i++) {

		sip = cip->slookup[i];

		switch (sip->flags & SLOT_MASK_TYPE) {
		case LONG:
		case PTRV:
		case ATTR:
		case OBJP:
		case OBJL:
		case CLSI:
		case PCOD:
		case TFLD:
			*clonep = *originalp;
			break;

		case RGBV:
			{
			  ColorInfo *ci, *oci;
			  ci = (ColorInfo*)malloc(
				     sizeof(struct ColorInfoStruct));
			  if (ci == NULL) {
			    *clonep = NULL;
			    break;
			  }
			  oci = (ColorInfo*)(*originalp);
			  if (oci) {
			    ci->bd = oci->bd;
			    ci->bg = oci->bg;
			    ci->fg = oci->fg;
			    ci->cr = oci->cr;
			  } else {
			    ci->bd = NULL; 
			    ci->bg = NULL;
			    ci->fg = NULL;
			    ci->cr = NULL;
			  }
			  *clonep = ci;
			}
			break;

		case STRI:
			{
			  StrInfo *si = *originalp;
			  si->refc++;
			  *clonep = *originalp;
			}
			break;

		case PTRS:
			*clonep = (long)saveString((char*)(*originalp));
			break;

		case PROC:
			((long (*)())(*originalp))(original, clonep);
			break;

		case FUNC:
			*clonep = ((long (*)())(*originalp))();
			break;

		case ARRY: 
			if (*originalp) {
				Array *array;

				array = (Array*)malloc(sizeof(struct Array));
				array->size = transferNumList2Array(
						(char*)(*originalp),
						intArrayBuff, 
						INTARRAYBUFF_SIZE);
				array->info = (int*)malloc(sizeof(int) 
							* array->size);
				bcopy(intArrayBuff, array->info, 
						sizeof(int) * array->size);
				*clonep = (long)array;
			}
			break;
		default:
			fprintf(stderr, 
		"initSlot(): unknown slot type = %d. Setting to NULL.\n",
				sip->flags & SLOT_MASK_TYPE);
		}

		++clonep;
		++originalp;
	}
/*printf("clone done\n");*/

	/* is securityMode > 0 (unsure), then override original object's
	 * security rating. note: this code is secure only for 2 levels scheme.
	 */
	if (securityMode > 0) SET_security(clone, securityMode);

	return clone;
} 

void methodMembershipProfile()
{
	ClassInfo *cip;
	MHInfo *mhp;
	HashEntry *entry;
	int i;

	printf("Method Calls Profile:\n");
	for (cip = classList; cip; cip++) {
		mhp = &cip->mhp;

		entry = symID2Str->get(symID2Str, (long)cip->id);

		printf("%20s refc=%5d\t hits=%5d\t misses=%5d collision=%5d\n", 
		    (char*)entry->val, mhp->refc, mhp->hits, mhp->misses, 
			mhp->collisions);
	}
}

