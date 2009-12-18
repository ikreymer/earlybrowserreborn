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
 * class	: HTML
 * superClass	: txtDisp
 */
#include "utils.h"
#include "error.h"
#include "mystrings.h"
#include "sys.h"
#include "hash.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "cl_HTML.h"
#include "misc.h"
#include "glib.h"
#include "tfed.h"
#include "attr.h"
#include "sgml.h"
#include "html.h"
#include "html2.h"

SlotInfo cl_HTML_NCSlots[] = {
	NULL
};
SlotInfo cl_HTML_NPSlots[] = {
{
	STR_HTMLAddress,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_HTMLAnchor,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_HTMLIsIndex,
	LONG | SLOT_RW,
	0
},{
	STR_HTMLSource,
	PTRS,
	(long)""
},{
	STR_HTMLStyle,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_HTMLTitle,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_HTMLStruct,
	PTRV,
	NULL
},{
	NULL
}
};
SlotInfo cl_HTML_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"HTML"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"keyPress\":\n\
			insert(key());\n\
		break;\n\
		case \"mouseMove\":\n\
		case \"keyRelease\":\n\
		case \"buttonRelease\":\n\
		break;\n\
		case \"shownPositionV\":\n\
			set(\"shownPositionV\", arg[1]);\n\
		break;\n\
		case \"buttonPress\":\n\
			processMouseInput();\n\
		break;\n\
		case \"expose\":\n\
			expose(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"shownPositionH\":\n\
			set(\"shownPositionH\", arg[1]);\n\
		break;\n\
		case \"enter\":\n\
			set(\"cursor\", 1);\n\
		break;\n\
		case \"leave\":\n\
			set(\"cursor\", 0);\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"scroll\":\n\
			if (arg[1] == \"down\") {\n\
				set(\"shownPositionV\",\n\
					get(\"shownPositionV\") -\n\
					get(\"shownSizeV\"));\n\
			} else if (arg[1] == \"up\") {\n\
				set(\"shownPositionV\",\n\
					get(\"shownPositionV\") +\n\
					get(\"shownSizeV\"));\n\
			}\n\
		break;\n\
		case \"config\":\n\
			purgeCache();\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			set(\"HTMLAddress\", get(\"HTMLAddress\"));\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"visible\":\n\
		case \"clearSelection\":\n\
			clearSelection();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
			set(\"HTMLAddress\", get(\"HTMLAddress\"));\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_HTML_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_HTML
},{
	NULL
}
};

SlotInfo *slots_HTML[] = {
	(SlotInfo*)cl_HTML_NCSlots,
	(SlotInfo*)cl_HTML_NPSlots,
	(SlotInfo*)cl_HTML_CSlots,
	(SlotInfo*)cl_HTML_PSlots
};

MethodInfo meths_HTML[] = {
	/* local methods */
{
	STR_pathSimplify,
	meth_HTML_pathSimplify,
},{
	STR_back,
	meth_HTML_back
},{
	STR_config,
	meth_HTML_config
},{
	STR_geta,
	meth_HTML_get
},{
	STR_initialize,
	meth_HTML_initialize
},{
	STR_purgeCache,
	meth_HTML_purgeCache
},{
	STR_render,
	meth_HTML_render
},{
	STR_search,
	meth_HTML_search
},{
	STR_seta,
	meth_HTML_set
},{
	NULL
}
};

ClassInfo class_HTML = {
	helper_HTML_get,
	helper_HTML_set,
	slots_HTML,		/* class slot information	*/
	meths_HTML,		/* class methods		*/
	STR_HTML,		/* class identifier number	*/
	&class_txtDisp,		/* super class info		*/
};

int meth_HTML_back(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = html_backtrack();
	return 1;
}

int meth_HTML_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	/* don't like this explicit naming of meth_pane_config... but...
	 * remember to change it if meth_txtDisp_config be added 
	 */
	if (!meth_pane_config(self, result, argc, argv)) return 0;
	if (!html_updateTFStruct(self, GET_HTMLAddress(self))) {
		return 0;
	}
	return 1;
}

int helper_HTML_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	char *str;

	switch (labelID) {
	case STR_HTMLAddress: {
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		result->info.s = SaveString(GET_HTMLAddress(self));
		return 1;
	}
	case STR_HTMLIsIndex: {
		result->type = PKT_INT;
		result->canFree = 0;
		result->info.i = GET_HTMLIsIndex(self);
		return 1;
	}
	case STR_HTMLSource: {
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		result->info.s = SaveString(GET_HTMLSource(self));
		return 1;
	}
	case STR_HTMLStyle: {
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		result->info.s = SaveString(GET_HTMLStyle(self));
		return 1;
	}
	case STR_HTMLTitle: {
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		result->info.s = SaveString(GET_HTMLTitle(self));
		return 1;
	}
	}
	return helper_txtDisp_get(self, result, argc, argv, labelID);
}
int meth_HTML_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_HTML_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_HTML_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	TFStruct *tf;
	VObjList *objl;

	if (!meth_txt_initialize(self, result, argc, argv)) return 0;
	if (tf = html_setUpTFStruct(self, NULL)) {
		html_updateTFStruct(self, NULL);
		scanVerticalMetrics(tf);
		if (!helper_txtDisp_updateShownInfo(tf)) return 0;
		for (objl = GET__shownDepend(self); objl; 
			objl = objl->next) {
		  	if (objl->o)
				sendMessage1N2int(objl->o, 
					  "shownInfoV",
					  GET_shownPositionV(self),
					  GET_shownSizeV(self));
		}
		return 1;
	}
	return 0;
}

int meth_HTML_pathSimplify(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_STR;
	result->canFree = PK_CANFREE_STR;
	result->info.s = html_pathSimplify(SaveString(PkInfo2Str(&argv[1])));
	return 1;
}

int meth_HTML_purgeCache(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *obj;

	clearPacket(result);
	if (argc == 1) {
		obj = PkInfo2Obj(&argv[0]);
		if (obj) html_clearCache(obj);
	} else {
		html_clearCache(NULL);
	}
	return 1;
}

int meth_HTML_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (meth_txtDisp_render(self, result, argc, argv)) {
		TFStruct *tf = GET__TFStruct(self);

		/* if the super render methods have newly opened up a window,
		 * then update the tf structure.
		 */
		if (tf) 
			if (!tf->w && GET_window(self)) {
				tf->w = GET_window(self);
			}
		return 1;
	}
	return 0;
}

int help_HTML_shownPositionV(self, newPosition)
	VObj *self;
	int newPosition;
{
	return help_txtDisp_shownPositionV(self, newPosition);
}

int meth_HTML_search(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = html_search(self, PkInfo2Str(&argv[0]));
	return 1;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_HTML_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	char *str;

	switch (labelID) {
	case STR_HTMLAddress: {
		VObjList *objl;
		TFStruct *tf;
		char *address;
		extern SGMLBuildInfo SBI;

		address = saveString(PkInfo2Str(&argv[1]));
		SET_HTMLAddress(self, address);
		if (!(tf = html_updateTFStruct(self, address))) {
			result->info.i = 0;
			result->canFree = 0;
			result->type = PKT_INT;
			return 0;
		}
/*
		result->info.s = GET_HTMLAddress(self);
		result->canFree = 0;
		result->type = PKT_STR;
*/
		scanVerticalMetrics(tf);
		if (!helper_txtDisp_updateShownInfo(tf)) {
		  result->info.i = 0;
		  result->canFree = 0;
		  result->type = PKT_INT;
		  return 0;
		}
		for (objl = GET__shownDepend(self); objl; objl = objl->next) {
		  	if (objl->o)
				sendMessage1N2int(objl->o, 
					  "shownInfoV",
					  GET_shownPositionV(self),
					  GET_shownSizeV(self));
		}
		result->info.o = SBI.stack[0].obj;
		result->canFree = 0;
		result->type = PKT_OBJ;

		return 1;
	}
	case STR_HTMLIsIndex: {
		result->type = PKT_INT;
		result->canFree = 0;
		result->info.i = PkInfo2Int(&argv[1]);
		SET_HTMLIsIndex(self, result->info.i);
		return 1;
	}
	case STR_HTMLSource: {
		result->type = PKT_STR;
		result->canFree = 0;
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_HTMLSource(self, result->info.s);
		return 1;
	}
	case STR_HTMLStyle: {
		result->type = PKT_STR;
		result->canFree = 0;
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_HTMLStyle(self, result->info.s);
		return 1;
	}
	case STR_HTMLTitle: {
		result->type = PKT_STR;
		result->canFree = 0;
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_HTMLTitle(self, result->info.s);
		return 1;
	}
	}
	return helper_txtDisp_set(self, result, argc, argv, labelID);
}
int meth_HTML_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_HTML_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_HTML_WWWNameOfFile(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_STR;
	result->canFree = 0;
	result->info.s = html_WWWNameOfFile(PkInfo2Str(&argv[0]));
	return 1;
}

