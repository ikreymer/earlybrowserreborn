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
 * class	: FCard
 * superClass	: pane
 */
#include "utils.h"
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
#include "cl_FCard.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_FCard_NCSlots[] = {
	NULL
};
SlotInfo cl_FCard_NPSlots[] = {
{
	STR_BCard,
	OBJP | SLOT_RW,
	NULL
},{
	NULL
}
};
SlotInfo cl_FCard_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"FCard"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"expose\":\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
			config(get(\"x\"), get(\"y\"), get(\"width\"), get(\"height\"));\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = FCard: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_FCard_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_FCard
},{
	STR_paneConfig,
 	PTRS,
	(long)"freeForm"
},{
	STR__paneConfig,
 	LONG,
	PANE_CONFIG_FREE	/* default with FREEFORM */
},{
	NULL
}
};

SlotInfo *slots_FCard[] = {
	(SlotInfo*)cl_FCard_NCSlots,
	(SlotInfo*)cl_FCard_NPSlots,
	(SlotInfo*)cl_FCard_CSlots,
	(SlotInfo*)cl_FCard_PSlots
};

MethodInfo meths_FCard[] = {
	/* local methods */
{
	STR_config,
	meth_FCard_config
},{
	STR_expose,
	meth_FCard_expose
},{
	STR_geta,
	meth_FCard_get,
},{
	STR_initialize,
	meth_FCard_initialize
},{
	STR_render,
	meth_FCard_render
},{
	NULL
}
};

ClassInfo class_FCard = {
	helper_FCard_get,
	helper_pane_set,
	slots_FCard,		/* class slot information	*/
	meths_FCard,		/* class methods		*/
	STR_FCard,		/* class identifier number	*/
	&class_pane,		/* super class info		*/
};

int meth_FCard_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_pane_config(self, result, argc, argv)) return 0;
	return 1;
}

int meth_FCard_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_pane_render(self, result, argc, argv);
}

int helper_FCard_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_BCard:
		result->info.o = GET_BCard(self);
		result->type = PKT_OBJ;
		return 1;
	}
	return helper_pane_get(self, result, argc, argv, labelID);
}
int meth_FCard_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_FCard_get(self, result, argc, argv,
				getIdent(PkInfo2Str(argv)));
}

int meth_FCard_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	meth_pane_initialize(self, result, argc, argv);
	return 1;
}

int meth_FCard_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	if (!w) meth_pane_render(self, result, argc, argv);
	if (!(w = GET_window(self))) return 0;

	return 1;
}

