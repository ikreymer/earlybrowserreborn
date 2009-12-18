/*
 * Copyright 1990 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * class	: stack
 * superClass	: pane
 */
#include "utils.h"
#include <ctype.h>
#include "error.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "cl_stack.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_stack_NCSlots[] = {
	NULL
};
SlotInfo cl_stack_NPSlots[] = {
{
	STR_BCardList,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_FCardList,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__BCardList,
	OBJL,
	NULL
},{
	STR__FCardList,
	OBJL,
	NULL
},{
	STR__currentCard,
	OBJP,
	NULL
},{
	NULL
}
};
SlotInfo cl_stack_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"stack"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
			config(get(\"x\"), get(\"y\"), get(\"width\"), get(\"height\"));\n\
		break;\n\
		case \"expose\":\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_stack_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_stack
},{
	NULL
}
};

SlotInfo *slots_stack[] = {
	(SlotInfo*)cl_stack_NCSlots,
	(SlotInfo*)cl_stack_NPSlots,
	(SlotInfo*)cl_stack_CSlots,
	(SlotInfo*)cl_stack_PSlots
};

MethodInfo meths_stack[] = {
	/* local methods */
{
	STR_anyCard,
	meth_stack_anyCard
},{
	STR_card,
	meth_stack_card
},{
	STR_create,
	meth_stack_create
},{
	STR_config,
	meth_stack_config
},{
	STR_expose,
	meth_stack_expose
},{
	STR_firstCard,
	meth_stack_firstCard
},{
	STR_geta,
	meth_stack_get
},{
	STR_getCurrentCard,
	meth_stack_getCurrentCard
},{
	STR_initialize,
	meth_stack_initialize
},{
	STR_lastCard,
	meth_stack_lastCard
},{
	STR_nextCard,
	meth_stack_nextCard
},{
	STR_countBCards,
	meth_stack_countBCards
},{
	STR_countFCards,
	meth_stack_countFCards
},{
	STR_previousCard,
	meth_stack_previousCard
},{
	STR_render,
	meth_stack_render
},{
	STR_seta,
	meth_stack_set
},{
	STR_setCurrentCard,
	meth_stack_setCurrentCard
},{
	NULL
}
};

ClassInfo class_stack = {
	helper_stack_get,
	helper_stack_set,
	slots_stack,		/* class slot information	*/
	meths_stack,		/* class methods		*/
	STR_stack,		/* class identifier number	*/
	&class_pane,		/* super class info		*/
};

int meth_stack_anyCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

int meth_stack_card(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

int meth_stack_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_pane_config(self, result, argc, argv)) return 0;
	return 1;
}

/*
 * countBCards()
 */
int meth_stack_countBCards(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist;
	VObj *stackObj = findStackObj(self);
	int n = 0;

	olist = GET__BCardList(stackObj);

	while (olist) {
		++n;
		if (verbose) printf("\tBoreground Card: [%x]\"%s\"\n",
					olist->o, GET_name(olist->o));
		olist = olist->next;
	}

	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = n;
	return 1;
}

/*
 * countFCards()
 */
int meth_stack_countFCards(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist;
	VObj *stackObj = findStackObj(self);
	int n = 0;

	olist = GET__FCardList(stackObj);

	while (olist) {
		++n;
		if (verbose) printf("\tForeground Card: [%x]\"%s\"\n",
					olist->o, GET_name(olist->o));
		olist = olist->next;
	}

	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = n;
	return 1;
}

int meth_stack_create(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

int meth_stack_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_pane_render(self, result, argc, argv);
}

int meth_stack_firstCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist;
	VObj *nobj, *stackObj = findStackObj(self);

	result->type = PKT_OBJ;
	result->canFree = 0;
	olist = GET__FCardList(stackObj);
	if (olist) {
		if (olist->o) {
			result->info.o = olist->o;
			return 1;
		}
	}
	result->info.o = NULL;
	return 0;
}

int helper_stack_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_BCardList:
		result->info.s = SaveString(GET_BCardList(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_FCardList:
		result->info.s = SaveString(GET_FCardList(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR__currentCard:
		result->info.o = GET__currentCard(self);
		result->type = PKT_OBJ;
		result->canFree = 0;
		return 1;
	}
	return helper_pane_get(self, result, argc, argv, labelID);
}
int meth_stack_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_stack_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_stack_getCurrentCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (GET__currentCard(self)) {
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = GET__currentCard(self);
		return 1;
	}
	clearPacket(result);
 	return 0;
}

int meth_stack_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;

	if (!meth_pane_initialize(self, result, argc, argv)) return 0;

	cp = GET_BCardList(self);
	if (cp) if (*cp) SET__BCardList(self, strOListToOList(cp));

	cp = GET_FCardList(self);
	if (cp) if (*cp) SET__FCardList(self, strOListToOList(cp));

	return 1;
}

int meth_stack_lastCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist;
	VObj *nobj, *stackObj = findStackObj(self);

	olist = GET__FCardList(stackObj);
	nobj = olist->o;
	for (; olist; olist = olist->next) {
		if (!(olist->next)) {
			nobj = olist->o;
			break;
		}
	}
	result->type = PKT_OBJ;
	result->canFree = 0;
	if (nobj) {
		result->info.o = nobj;
		return 1;
	} else {
		result->info.o = NULL;
		return 0;
	}
}

/*
 * nextCard
 */
int meth_stack_nextCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist;
	VObj *currentCard, *nobj = NULL;
	VObj *stackObj = findStackObj(self);

	olist = GET__FCardList(stackObj);
	if (currentCard = GET__currentCard(stackObj)) {
		/* find the next object */
		VObjList *l = olist;

		nobj = l->o; /* in case there is only one card in list */
		while (l) {
			if (l->o == currentCard) {
				if (l->next) nobj = l->next->o;
				else nobj = l->o;
				break;
			}
			l = l->next;
		}
	} else {
		if (olist) nobj = olist->o;
	}

	result->type = PKT_OBJ;
	result->canFree = 0;
	if (nobj) {
		result->info.o = nobj;
		return 1;
	} else {
		result->info.o = NULL;
		return 0;
	}
}

/*
 * previousCard
 */
int meth_stack_previousCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist;
	VObj *currentCard, *lobj = NULL;
	VObj *stackObj = findStackObj(self);

	olist = GET__FCardList(stackObj);
	if (currentCard = GET__currentCard(stackObj)) {
		/* find the next object */
		VObjList *l = olist;

		lobj = l->o;
		while (l) {
			if (l->o == currentCard) break;
			lobj = l->o;
			l = l->next;
		}
	} else {
		if (olist) lobj = olist->o;
	}

	result->type = PKT_OBJ;
	result->canFree = 0;
	if (lobj) {
		result->info.o = lobj;
		return 1;
	} else {
		result->info.o = NULL;
		return 0;
	}
}

int meth_stack_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;

	if (!w) meth_pane_render(self, result, argc, argv);
	if (!(w = GET_window(self))) return 0;
	return 1;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_stack_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_currentCard:
		result->info.o = PkInfo2Obj(&argv[1]);
		SET__currentCard(self, result->info.o);
		result->type = PKT_OBJ;
		result->canFree = 0;
		return 1;
	}
	return helper_pane_set(self, result, argc, argv, labelID);
}
int meth_stack_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_stack_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_stack_setCurrentCard(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *obj = GET__currentCard(self);
	Packet pks[2];
	int (*func)();

	pks[0].type = PKT_STR;
	pks[0].info.s = "visible";
	pks[0].canFree = 0;
	pks[1].type = PKT_INT;
	pks[1].canFree = 0;

	/* lights off
	 */
	if (obj) {
		pks[1].info.i = 0;
		func = GET__classInfo(obj)->slotSetMeth;
		if (func) {
			((long (*)())(func))(obj, result, 2, pks,
						STR_visible);
		}
	}
	/* lights on
	 */
	if (obj = PkInfo2Obj(&argv[0])) {

		pks[1].info.i = 1;
		SET__parent(obj, self);
		func = GET__classInfo(obj)->slotSetMeth;
		if (func) {
			((long (*)())(func))(obj, result, 2, pks,
						STR_visible);
		}
		result->type = PKT_OBJ;
		result->info.o = SET__currentCard(self, obj);
	 	return 1;
	}
	clearPacket(result);
 	return 0;
}

#ifdef stackgarbage
/********************************************************************/

long method_stack_render(self)
		 OBJ self;
{
	if (ObjActive(self)) {
		OBJ BCard, FCard;
		Packetp attrValResult = initPacket();
		Packetp result = initPacket();

		if (!(FCard = ObjCurrentCard(self))) {
			if (verbose) 
	MERROR(self, "render: currentCard of stack is not set.\n");
			return NULL;
		}
		if (!(BCard = ObjBCard(FCard))) {
			if (verbose) 
	MERROR(self, "render: BCard of currentCard is not set.\n");
			return NULL;
		}

		setPacketContent(attrValResult, "1");
		callMethod(self, "set", result, "visible", attrValResult, NULL);

		setPacketContent(attrValResult, "1");
		callMethod(BCard, "set", result, "visible", attrValResult, NULL);

		setPacketContent(attrValResult, "1");
		callMethod(FCard, "set", result, "visible", attrValResult, NULL);

		freePacket(result);
		freePacket(attrValResult);
	}
	return NULL;
}

int method_stack_numOfCards(self, result)
		 OBJ self;
		 Packetp result;
{
	int cards = 0;
	OLIST *BCList;

	if (BCList = ObjBCardList(self)) {
		for (; BCList; BCList = BCList->next) { 
			OLIST *FCardList;

			if (verbose)
	printf("Background Card: [%x]\"%s\"\n", BCList->o, ObjName(BCList->o));
			++cards;

			if (FCardList = ObjCardList(self)) {
	if (verbose) printf("Foreground Cards: ");
	while (FCardList) {
		++cards;
		if (verbose) printf("\tForeground Card: [%x]\"%s\"\n",
						FCardList->o, ObjName(FCardList->o));
		FCardList = FCardList->next;
	}
			}
		}
	} else {
		if (verbose) fprintf(stderr, "no BCards in stack.\n");
	}
	clearPacket(result);
	setPacketContent(result, (char*)valToStr((long)cards, buff));
	return(cards);
}

OBJ method_stack_card(self, result, cardName)
		 OBJ self;
		 Packetp result;
		 char *cardName;
{
	OBJ stackObj = ObjStack(self);

	method_generic_getNamedListObj(stackObj, result, cardName, "cardList");
	return NULL;
}


/*
 * firstCard [of stack <stackName>]
 *						NOT YET
 * REUTRN: first card from cardList
 */
OBJ method_stack_currentCard(self, result)
		 OBJ self;
		 Packetp result;
{
	OBJ stackObj = ObjStack(self);

	if (stackObj) {
		method_generic_self(ObjCurrentCard(stackObj), result);
	} else {
		MERROR(self, "currentCard: stack pointer isn't set!\n");
	}
	return PkObject(result);
}

/*
 * firstCard [of stack <stackName>]
 *						NOT YET
 * REUTRN: first card from cardList
 */

OBJ method_stack_firstCard(self, result)
		 OBJ self;
		 Packetp result;
{
	OBJ stackObj = ObjStack(self);

	if (stackObj) {
		if (!method_generic_getListObj(stackObj, result, "cardList", 1)) {
			if (verbose)
	fprintf(stderr, "firstCard: no card in cardList of stack.\n");
		}
	} else {
		if (verbose)
			fprintf(stderr,
				"method_stack_firstCard(self='%s'): no stack object found.",
				ObjName(self));
	}
	return PkObject(result);
}

/*
 * lastCard [of stack <stackName>]
 *						NOT YET
 * REUTRN: last card from cardList
 */

OBJ method_stack_lastCard(self, result)
		 OBJ self;
		 Packetp result;
{
	OBJ stackObj = ObjStack(self);

	if (!method_generic_lastObjInList(stackObj, result, "cardList"))
		if (verbose) fprintf(stderr, "lastCard: no card in cardList of stack.\n");
	return PkObject(result);
}

ATTR *method_stack_set(self, result, attrName, attrValResult, flag)
		 OBJ self;
		 Packetp result;
		 char *attrName;
		 Packetp attrValResult;
		 int flag;
{
	if (PkAnswer(attrValResult) == NULL) {
		clearPacket(result);
		sprintf(buff, "set: attr=NULL. aborted.\n");
		messageToUser(self, MESSAGE_ERROR, buff);
	}

	if (!STRCMP(attrName, "visible")) {
		if (ObjActive(self)) {
			OBJ BCard, FCard;

			method_generic_set(self, result, attrName, attrValResult, flag);

			FCard = ObjCurrentCard(self);
			if (FCard) {
	if (BCard = ObjBCard(FCard)) {
		callMethod(BCard, "set", result, attrName, attrValResult, NULL);
	}
	callMethod(FCard, "set", result, attrName, attrValResult, NULL);
			}
		}

	} else if (!STRCMP(attrName, "param")) {
/*
		int active = ObjActive(self);
		int visible = ObjVisible(self);
		OLIST *olist;

		if (!(active && visible && window))
			if (flag == SET_UPDATE) flag = SET_NO_UPDATE;
*/			
		method_generic_set(self, result, attrName, attrValResult, flag);
		/*
		 * update param of cards as well... to avoid hidden newly created objs...
		 * this hyperCard stack/BCard/card-in-one-window thing sucks...
		 */
/*
		for (olist = ObjCardList(self); olist; olist = olist->next)
			callMethod(olist->o, "set", result, attrName, attrValResult,
		 flag);

		for (olist = ObjBCardList(self); olist; olist = olist->next)
			callMethod(olist->o, "set", result, attrName, attrValResult,
		 flag);
*/
	} else if (!STRCMP(attrName, "currentCard")) {
		Set_ptr(self, attrName, PkObject(attrValResult));

	} else {
		method_card_set(self, result, attrName, attrValResult, flag);
	}
	return NULL;
}

#endif


/***********************************************************************/

VObj *findStackObj(obj)
	VObj *obj;
{
	while (obj) {
		if (GET__classInfo(obj) == &class_stack) return obj;
		obj = GET__parent(obj);
	}
	return NULL;
}
