/*
 * obj.c
 */
/*
 * Copyright 1991 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
#include "utils.h"
#include "mystrings.h"
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "ident.h"

VObj *VResourceObj;
VObj *VFontResourceObj;
VObj *VCurrentObj;
VObj *VTargetObj;
VObj *VPrevObj;

HashTable *objID2Obj;
HashTable *objObj2ExistP;

int securityMode = 0;

/* temporary way to know not to free heap, but just X resources */
int exitingViola = 0;

int init_obj()
{
	objID2Obj = initHashTable(1023, hash_int, cmp_int, NULL, NULL,
				  getHashEntry_int, 
				  putHashEntry_int, 
				  putHashEntry_replace_int,
				  removeHashEntry_int);
	if (!objID2Obj) return 0;

	objObj2ExistP = initHashTable(1023, hash_int, cmp_int, NULL, NULL,
				      getHashEntry_int, 
				      putHashEntry_int, 
				      putHashEntry_replace_int,
				      removeHashEntry_int);
	if (!objObj2ExistP) return 0;

	return 1;
}

void dumpVObjList(olist)
	VObjList *olist;
{
	fprintf(stderr, "{");
	while (olist) {
/*		fprintf(stderr, "[%x]\"%s\"", olist->o, ObjName(olist->o));
		fprintf(stderr, "\"%s\"", ObjName(olist->o));
*/
		olist = olist->next;
		if (olist) fprintf(stderr, ", ");
	}
	fprintf(stderr, "}");
}

/*MACRONIZED
int validObjectP(self)
	VObj *self;
{
	return (objObj2ExistP->get(objObj2ExistP, self) ? 1 : 0);
}
*/

VObj *findObject(objNameStrID)
	int objNameStrID;
{
	HashEntry *entry = objID2Obj->get(objID2Obj, objNameStrID);
	if (entry) return (VObj*)(entry->val);
	return NULL;
}

/*
 ** Append obj to tail of olist.
 *
 * NOTE: olist always point to atleast one VObjList structure, which may or
 *      may not point to any object.
 */
VObjList *appendObjToList(olist, obj)
	VObjList *olist;
	VObj *obj;
{
	VObjList *newl;

	/* allocate node for base_set list */
	if (newl = (VObjList*)malloc(sizeof(VObjList))) {
		newl->next = NULL;
		newl->o = obj;
		if (olist) {
			VObjList *head = olist;
			while (olist->next) olist = olist->next;
			olist->next = newl;
			return head;
		}
		return newl;
	}
	return olist;
}
/*
VObjList *removeObjFromList(olist, obj)
	VObjList *olist;
	VObj *obj;
{
	if (olist) {
		VObjList *head = olist;
		do {
			if (olist->o == obj) {
				if (olist->o == obj) {
			}
			olist = olist->next;
		} while (olist);
		olist->next = newl;
		return head;
	} else {
		return olist;
	}
}
*/

void freeAllObjects()
{
	int i;
	HashEntry *hp;
	Packet *result = borrowPacket();
	VObj *obj;
/*
print(">>>>\n");
dumpHashTable(objID2Obj);
print(">>>>\n");
*/
/*
print("freeAllObjects(): BEGIN ############################\n");
*/
	for (i = 0; i < objID2Obj->size; i++) {
		for (hp = &(objID2Obj->entries[i]); hp; hp = hp->next) {
			if (hp->label) {
				obj = (VObj*)(hp->val);
				if (obj) {
/*print("freeAllObjects(): obj=0x%x, %s\n", obj, (char*)obj[1]);*/
					callMeth(obj, result, 0, NULL, 
						STR_freeSelf);
				}
			}
		}
	}
/*print("freeAllObjects(): DONE############################\n");*/

	returnPacket();
}

#ifdef fuandafagdsg


OLIST *linkStrList(self, listName, strList)
		 VObj self;
		 char *listName;
		 char *strList;
{
	VObj obj;
	OLIST *olist = NULL;
	char *objNamep;
	XXXXXXXXXXXXXXXXXXXX = initPacket();
	int i, n;

	if (!strList) return(NULL);
	n = numOfChar(strList, ',') + 1;
	for (i = 1; i <= n; i++) {
		if (objNamep = listItem(strList, i, i)) {
			deBracket(objNamep);
			trimEdgeSpaces(objNamep);
			if (obj = retrieveObj(objNamep, (char*)NULL)) {
	olist = method_generic_appendObject(self, result, obj, listName);
			} else {
	MERROR(self,
				 "linkStrListObj(self=`%s', listName=`%s', strList=`%s'): obj=NULL.\n",
				 ObjName(self), listName, strList);
			}
			free(objNamep);
		}
	}
	freePacket(result);
	return olist;
}

VObj copyObjLink(self, clone, slotName, objFile, CID)
		 VObj self, clone;
		 char *slotName;
		 char *objFile;
		 long CID;
{
	VObj obj = Get_ptr(self, slotName, VObj);
	char *objName;
	char cloneName[200];

	if (obj) {
		objName = ObjName(obj);
	} else {
		MERROR(self, "copyObjLink: can't find object type '%s'.\n", slotName);
		return NULL;
	}
	sprintf(cloneName, "%s_%ld", objName, CID);
	if (obj = retrieveObj(cloneName, objFile)) {
		Set_ptr(clone, objName, obj);
		return obj;
	} else {
		IERROR_SELF(self, "can't find cloned object '%s' in list '%s'.\n",
		cloneName, objName);
	}
	return NULL;
}

OLIST *copyObjListLink(self, clone, listName, objFile, CID)
		 VObj self, clone;
		 char *listName;
		 char *objFile;
		 long CID;
{
	VObj obj;
	OLIST *olist, *colist;
	char *objName;
	char cloneName[200];

	olist = Get_ptr(self, listName, OLIST*);
	colist = Get_ptr(clone, listName, OLIST*);
	while (olist) {
		objName = ObjName(olist->o);
		sprintf(cloneName, "%s_%ld", objName, CID);
		if (obj = retrieveObj(cloneName, objFile)) {
			colist = append_obj_to_list(colist, obj);
		} else {
			IERROR_SELF(self, "can't find cloned object '%s' in list '%s'.\n",
			cloneName, objName);
		}
		olist = olist->next;
	}
	Set_ptr(clone, listName, colist);
	return colist;
}

#endif
