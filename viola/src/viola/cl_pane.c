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
 * class	: pane
 * superClass	: field
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
#include "cl_pane.h"
#include "misc.h"
#include "glib.h"

char *paneConfigStr[] = {
	"freeForm",
	"northToSouth",
	"southToNorth",
	"westToEast",
	"eastToWest",
	"northToSouth_edge",
	"southToNorth_edge",
	"westToEast_edge",
	"eastToWest_edge",
	"center",
	NULL
};

SlotInfo cl_pane_NCSlots[] = {
	NULL
};
SlotInfo cl_pane_NPSlots[] = {
{
	STR_paneConfig,
 	PTRS | SLOT_RW,
	(long)"freeForm"
},{
	STR__paneConfig,
 	LONG,
	PANE_CONFIG_FREE
},{
	NULL
}
};
SlotInfo cl_pane_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"pane"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"render\":\n\
		case \"expose\":\n\
			render();\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
			get(\"x\"),get(\"y\"),get(\"width\"),get(\"height\"));\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"mouseMove\":\n\
		case \"enter\":\n\
		case \"leave\":\n\
		case \"buttonPress\":\n\
		case \"buttonRelease\":\n\
		case \"keyPress\":\n\
		case \"keyRelease\":\n\
		case \"shownPositionH\":\n\
		case \"shownPositionV\":\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = \", get(\"class\"),\n\
				\": self = \", get(\"name\"), \" args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_pane_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_pane
},{
	NULL
}
};

SlotInfo *slots_pane[] = {
	(SlotInfo*)cl_pane_NCSlots,
	(SlotInfo*)cl_pane_NPSlots,
	(SlotInfo*)cl_pane_CSlots,
	(SlotInfo*)cl_pane_PSlots
};

MethodInfo meths_pane[] = {
	/* local methods */
{
	STR_clone,
	meth_pane_clone
},{
	STR_clone2,
	meth_pane_clone2
},{
	STR_config,
	meth_pane_config
},{
	STR_expose,
	meth_pane_expose,
},{
	STR_geta,
	meth_pane_get,
},{
	STR_initialize,
	meth_pane_initialize
},{
	STR_render,
	meth_pane_render
},{
	STR_seta,
	meth_pane_set
},{
	NULL
}
};

ClassInfo class_pane = {
	helper_pane_get,
	helper_pane_set,
	slots_pane,		/* class slot information	*/
	meths_pane,		/* class methods		*/
	STR_pane,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

/*
 * clone(clone name suffix)
 * 
 * Make a clone self
 *
 * Result: clone object, and optinally name it
 * Return: 1 if successful, 0 if error occured
 */
int meth_pane_clone(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *cloneObj;

	if (!meth_pane_clone2(self, result, argc, argv)) return 0;
	cloneObj = result->info.o;
	if (cloneObj) {
		sendInitToChildren(cloneObj);
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = cloneObj;
		return 1;
	}
	result->type = PKT_OBJ;
	result->canFree = 0;
	result->info.o = NULL;
	return 0;
}

int meth_pane_clone2(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_field_clone2(self, result, argc, argv)) return 0;
	return 1;
}

/*
 */
/* used to hold position, span, spanOther info */
#define CHILD_POSN(i) 	geomBuff[i][0]
#define CHILD_SPAN(i) 	geomBuff[i][1]
#define CHILD_SPANO(i) 	geomBuff[i][2]
#define CHILD_GAP(i) 	geomBuff[i][3]
#define CHILD_GAPO(i) 	geomBuff[i][4]
#define CHILD_MAX	100			/*XXX*/
helper_pane_config(self)
	VObj *self;
{
	VObjList *olist, *children = GET__children(self);
	int loopbreak = 999; /* precaution to infinite loop -- breaking limit */
	int i, doit = 1, positive = 1;
	int posIdx, spanIdx, minIdx, maxIdx;
	int spanOtherIdx, spanOtherIdxMax; 
	int gapIdx, gapOtherIdx;
	static int geomBuff[CHILD_MAX][5];

/* printf("GM self=%s **************\n", GET_name(self));*/

	if (children) {
		switch (GET__paneConfig(self)) {
		case PANE_CONFIG_N2S_EDGE:
		case PANE_CONFIG_N2S:
			posIdx = 1;
			spanIdx = IDX_height(self);
			spanOtherIdx = IDX_width(self);
			spanOtherIdxMax = IDX_maxWidth(self);
			minIdx = IDX_minHeight(self);
			maxIdx = IDX_maxHeight(self);
			gapIdx = IDX_gapV(self);
			gapOtherIdx = IDX_gapH(self);
			break;
		case PANE_CONFIG_S2N_EDGE:
		case PANE_CONFIG_S2N:
			posIdx = 1;
			spanIdx = IDX_height(self);
			spanOtherIdx = IDX_width(self);
			spanOtherIdxMax = IDX_maxWidth(self);
			minIdx = IDX_minHeight(self);
			maxIdx = IDX_maxHeight(self);
			gapIdx = IDX_gapV(self);
			gapOtherIdx = IDX_gapH(self);
			positive = 0;
		break;
		case PANE_CONFIG_W2E_EDGE:
		case PANE_CONFIG_W2E:
			posIdx = 0;
			spanIdx = IDX_width(self);
			spanOtherIdx = IDX_height(self);
			spanOtherIdxMax = IDX_maxHeight(self);
			minIdx = IDX_minWidth(self);
			maxIdx = IDX_maxWidth(self);
			gapIdx = IDX_gapH(self);
			gapOtherIdx = IDX_gapV(self);
		break;
		case PANE_CONFIG_E2W_EDGE:
		case PANE_CONFIG_E2W:
			posIdx = 0;
			spanIdx = IDX_width(self);
			spanOtherIdx = IDX_height(self);
			spanOtherIdxMax = IDX_maxHeight(self);
			minIdx = IDX_minWidth(self);
			maxIdx = IDX_maxWidth(self);
			gapIdx = IDX_gapH(self);
			gapOtherIdx = IDX_gapV(self);
			positive = 0;
		break;
		case PANE_CONFIG_FREE:
		default:
			doit = 0;
		}
	} else {
		doit = 0;
	}

	if (doit) {
		int nchildren = 0, span, position = 0, bd;
		int nmarked = 0, markedsum = 0;
		int pbd = borderStyleThickness[GET_border(self)] * 2;
		int gap, gap2, gapOther;

		/* 0 indicates undetermined value */
		for (olist = children; olist; olist = olist->next) {
			if (nchildren < CHILD_MAX) {
				CHILD_SPAN(nchildren) = 0;
			} else {
				fprintf(stderr, 
			"Internal error: pane config buffer overflowed.\n");
				break;
			}
			nchildren++;
		}
		while (loopbreak--) {
			if (nchildren - nmarked == 0) {
				/*printf("pane config error\n");*/
				break;
			}
			span = (self[spanIdx] - markedsum)
					/ (nchildren - nmarked);
			i = 0;
	
/*printf("GM lookbreak=%d\n", loopbreak);*/

			for (olist = children; olist; olist = olist->next) {
				if (!olist->o) {
					printf("error: pane, NULL child.");
					break;
				}
/*printf("GM cobj=%s\t\n", GET_name(olist->o));*/
				bd =borderStyleThickness[GET_border(olist->o)];

				CHILD_GAP(i) = olist->o[gapIdx];
				CHILD_GAPO(i) = olist->o[gapOtherIdx];
				gap2 = CHILD_GAP(i) * 2;

				if (!CHILD_SPAN(i)) {
					if (span <= olist->o[minIdx] 
							+ bd + gap2) {
						CHILD_SPAN(i) = 
							olist->o[minIdx];
						markedsum += CHILD_SPAN(i)
								+ bd + gap2;
						++nmarked;
						goto escape;
					} else if (span >= olist->o[maxIdx]
							+ bd + gap2){
						CHILD_SPAN(i) = 
							olist->o[maxIdx];
						markedsum += CHILD_SPAN(i)
								+ bd + gap2;
						++nmarked;
						goto escape;
					}
				}
				i++;
			}
			break;
		escape:;
		}
		if (positive) {
			for (i = 0; i < nchildren; i++) {
				if (!CHILD_SPAN(i)) CHILD_SPAN(i) = 
					span - CHILD_GAP(i) * 2;
				CHILD_SPANO(i) = self[spanOtherIdx]
							- CHILD_GAPO(i) * 2;
				CHILD_POSN(i) = position - 1 + CHILD_GAP(i);
				position += CHILD_SPAN(i) + 1
						+ CHILD_GAP(i) * 2;
			}
		} else {
			position = self[spanIdx];
			for (i = nchildren - 1; i >= 0; i--) {
				if (!CHILD_SPAN(i)) CHILD_SPAN(i) = 
					span - CHILD_GAP(i) * 2;
				CHILD_SPANO(i) = self[spanOtherIdx]
							- CHILD_GAPO(i) * 2;
				position -= CHILD_SPAN(i) - 1 
						- CHILD_GAP(i) * 2;
				CHILD_POSN(i) = position + 1 + CHILD_GAP(i);
			}
		}
	}

	if (posIdx == 1) {
		for (i = 0, olist = children; olist; olist = olist->next, i++){
			if (!olist->o) {
				printf("error: pane, NULL child.");
				break;
			}

			if (CHILD_SPAN(i) < 0) CHILD_SPAN(i) = 0;

			if (CHILD_SPANO(i) > olist->o[spanOtherIdxMax])
				CHILD_SPANO(i) = olist->o[spanOtherIdxMax];
			if (CHILD_SPANO(i) < 0) CHILD_SPANO(i) = 0;

/*printf("GM Set cobj=%s\t span=%d  ospan=%d ospanMax=%d\n", 
    GET_name(olist->o), CHILD_SPAN(i), CHILD_SPANO(i), olist->o[spanOtherIdxMax]);
*/
/*			if (GET_y(olist->o) != CHILD_POSN(i) ||
			    GET_width(olist->o) != CHILD_SPANO(i) ||
			    GET_height(olist->o) != CHILD_SPAN(i)) {
*/
				if (verbose) 
					fprintf(stderr, 
						"pane y: %s : %d %d %d %d\n", 
						GET_name(olist->o),
						GET_x(olist->o), CHILD_POSN(i),
						CHILD_SPANO(i), CHILD_SPAN(i));

				SET_x(olist->o, CHILD_GAPO(i));
				SET_y(olist->o, CHILD_POSN(i));
				SET_width(olist->o, CHILD_SPANO(i));
				SET_height(olist->o, CHILD_SPAN(i));

				if (GET_window(olist->o))
					if (CHILD_SPANO(i) >= 1 &&
					    CHILD_SPAN(i) >= 1) {
					  GLUpdateGeometry(0,
						GET_window(olist->o),
						CHILD_GAPO(i), CHILD_POSN(i),
						CHILD_SPANO(i), CHILD_SPAN(i));
					}
			/*}*/
		}
	} else {
		for (i = 0, olist = children; olist; olist = olist->next, i++){
			if (!olist->o) {
				printf("error: pane, NULL child.");
				break;
			}

			if (CHILD_SPAN(i) < 0) CHILD_SPAN(i) = 0;

			if (CHILD_SPANO(i) > olist->o[spanOtherIdxMax])
				CHILD_SPANO(i) = olist->o[spanOtherIdxMax];
			if (CHILD_SPANO(i) < 0) CHILD_SPANO(i) = 0;

/*
printf("GM Set cobj=%s\t span=%d  ospan=%d ospanMax=%d\n", 
    GET_name(olist->o), CHILD_SPAN(i), CHILD_SPANO(i), olist->o[spanOtherIdxMax]);

*/
/*			if (GET_x(olist->o) != CHILD_POSN(i) ||
			    GET_width(olist->o) != CHILD_SPAN(i) ||
			    GET_height(olist->o) != CHILD_SPANO(i)) {
*/
				if (verbose) 
					fprintf(stderr, 
						"pane x: %s : %d %d %d %d\n", 
						GET_name(olist->o),
						CHILD_POSN(i), GET_y(olist->o),
						CHILD_SPAN(i), CHILD_SPANO(i));

				SET_x(olist->o, CHILD_POSN(i));
				SET_y(olist->o, CHILD_GAPO(i));
				SET_width(olist->o, CHILD_SPAN(i));
				SET_height(olist->o, CHILD_SPANO(i));

				if (GET_window(olist->o))
					if (CHILD_SPANO(i) >= 1 &&
					    CHILD_SPAN(i) >= 1) {
					  GLUpdateGeometry(0,
						GET_window(olist->o),
						CHILD_POSN(i), CHILD_GAPO(i),
						CHILD_SPAN(i), CHILD_SPANO(i));
					}
			/*}*/
		      }
	}
}

int meth_pane_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_field_config(self, result, argc, argv)) return 0;

	if (GET__paneConfig(self) != PANE_CONFIG_FREE)
		if (GET__children(self)) helper_pane_config(self);

	return 1;
}

int helper_pane_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_paneConfig:
		result->info.s = SaveString(GET_paneConfig(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_field_get(self, result, argc, argv, labelID);
}
int meth_pane_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_pane_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_pane_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_expose(self, result, argc, argv);
}

int meth_pane_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Packet pks[2];
	int (*func)();

	if (!meth_field_initialize(self, result, argc, argv)) return 0;

/*	pks[0].type = PKT_STR;
	pks[0].info.s = "paneConfig";
	pks[0].canFree = 0;
*/
	pks[1].type = PKT_STR;
	pks[1].info.s = SaveString(GET_paneConfig(self));
	pks[1].canFree = PK_CANFREE_STR;

/*	callMeth(self, result, 2, &pks[0], STR_seta);*/

	func = GET__classInfo(self)->slotSetMeth;
	if (func) ((long (*)())(func))(self, result, 2, pks, STR_paneConfig);

/*	clearPacket(&pks[0]);*/
	clearPacket(&pks[1]);
	return 1;
}

int meth_pane_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_field_render(self, result, argc, argv)) return 0;
/*	if (GET__paneConfig(self) != PANE_CONFIG_FREE)
		if (GET__children(self)) helper_pane_config(self);
*/
	return 1;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_pane_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_paneConfig: {
		int i;
		char *cp = PkInfo2Str(&argv[1]);

		result->type = PKT_STR;
		for (i = 0; paneConfigStr[i]; i++) {
			if (!STRCMP(paneConfigStr[i], cp)) {
				if (GET_paneConfig(self)) 
				  	free(GET_paneConfig(self));
/*not necessary*/		SET_paneConfig(self, saveString(cp));
				SET__paneConfig(self, i);
				result->info.s = GET_paneConfig(self);
				result->canFree = 0;
				return 1;
			}
		}
		clearPacket(result);
		return 0;
	}
	}
	return helper_field_set(self, result, argc, argv, labelID);
}
int meth_pane_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_pane_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

