#ifndef _VIOLA_OBJ_H_
#define _VIOLA_OBJ_H_

typedef int VObj;

typedef struct ObjList {
	struct ObjList *next;	/* must be the first item  (see vlist.h) */
	VObj *o;		/* must be the second item (see vlist.h) */
} VObjList;

extern HashTable *objID2Obj;		/* Obj strID  -> Obj vector 	*/
extern HashTable *objObj2ExistP;	/* Obj vector -> existence flag */

extern VObj *VResourceObj;	/* refers to "res" object		*/
extern VObj *VFontResourceObj;  /* refers to "res.font" object 		*/
extern VObj *VCurrentObj;	/* refers to the current object context */
extern VObj *VTargetObj;	/* refers to the marked object		*/
extern VObj *VPrevObj;		/* refers to the previous object	*/

extern int securityMode;

/*
 * scans and return the VObjList node objentified by obj
 */
#define scanVObjListNode(head_olist, obj)\
	(VObjList*)scanVListNode((VList*)head_olist, obj, cmp_int)

/*
 * remove and return the VObjList node identified by obj
 */
#define removeVObjListNode(head_olist, obj)\
	(VObjList*)removeVListNode((VList*)head_olist, obj, cmp_int)

/*
 * prepend and return the VObjList node
 */
#define prependVObjListNode(head_olist, obj)\
	(VObjList*)prependVListNode((VList*)head_olist, obj)

#define appendVObjListNode(olist, obj) appendObjToList(olist, obj)

/*int validObjectP();*/
#define validObjectP(self) objObj2ExistP->get(objObj2ExistP, self)

VObj *findObject();
VObjList *appendObjToList();
void freeAllObjects();

int exitingViola;

#endif _VIOLA_OBJ_H_
