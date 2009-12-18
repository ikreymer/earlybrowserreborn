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
 * class	: BCard
 * superClass	: field
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
#include "cl_BCard.h"
#include "misc.h"
#include "glib.h"

SlotInfo cl_BCard_NCSlots[] = {
	NULL
};
SlotInfo cl_BCard_NPSlots[] = {
	NULL
};
SlotInfo cl_BCard_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"BCard"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
			/* icky! but necessary to give script a chance to \n\
			 * intercept and do something...*/\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"expose\":\n\
			expose(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"render\":\n\
			render();\n\
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
		case \"shownInfoV\":\n\
		case \"shownInfoH\":\n\
		break;\n\
		case \"focus\":\n\
			mousePos = mouse();\n\
			winPos = windowPosition();\n\
			mx = mousePos[0];\n\
			my = mousePos[1];\n\
			dx = ((winPos[0] + width() / 2) - mx) / 10.0;\n\
			dy = ((winPos[1] + height() / 2) - my) / 10.0;\n\
			for (i = 0; i < 10; i = i + 1) {\n\
				mx = mx + dx;\n\
				my = my + dy;\n\
				setMouse(mx, my);\n\
			}\n\
		break;\n\
		case \"key_up\":\n\
			send(parent(), \"key_up\");\n\
			return;\n\
		break;\n\
		case \"key_down\":\n\
			send(parent(), \"key_down\");\n\
			return;\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"freeSelf\":\n\
			return freeSelf();\n\
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
SlotInfo cl_BCard_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_BCard
},{
	NULL
}
};

SlotInfo *slots_BCard[] = {
	(SlotInfo*)cl_BCard_NCSlots,
	(SlotInfo*)cl_BCard_NPSlots,
	(SlotInfo*)cl_BCard_CSlots,
	(SlotInfo*)cl_BCard_PSlots
};

MethodInfo meths_BCard[] = {
	/* local methods */
{
	STR_config,
	meth_BCard_config
},{
	STR_expose,
	meth_BCard_expose
},{
	STR_geta,
	meth_BCard_get,
},{
	STR_initialize,
	meth_BCard_initialize
},{
	STR_render,
	meth_BCard_render
},{
	STR_seta,
	meth_BCard_set
},{
	NULL
}
};

ClassInfo class_BCard = {
	helper_BCard_get,
	helper_BCard_set,
	slots_BCard,		/* class slot information	*/
	meths_BCard,		/* class methods		*/
	STR_BCard,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

int meth_BCard_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_field_config(self, result, argc, argv)) return 0;
	return 1;
}

int meth_BCard_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_render(self, result, argc, argv);
}

int helper_BCard_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	return helper_field_get(self, result, argc, argv, labelID);
}

int meth_BCard_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_BCard_get(self, result, argc, argv, 
					getIdent(PkInfo2Str(argv)));
}

int meth_BCard_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	meth_field_initialize(self, result, argc, argv);
	return 1;
}

int meth_BCard_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	if (!w) meth_field_render(self, result, argc, argv);
	if (!(w = GET_window(self))) return 0;
	return 1;
}

int helper_BCard_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	return helper_field_set(self, result, argc, argv, labelID);
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int meth_BCard_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_BCard_set(self, result, argc, argv, 
					getIdent(PkInfo2Str(argv)));
}








