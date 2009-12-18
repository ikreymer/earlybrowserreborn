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
 * class	: dial
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
#include "cl_dial.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_dial_NCSlots[] = {
	NULL
};
SlotInfo cl_dial_NPSlots[] = {
	NULL
};
SlotInfo cl_dial_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"dial"
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
SlotInfo cl_dial_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_dial
},{
	NULL
}
};

SlotInfo *slots_dial[] = {
	(SlotInfo*)cl_dial_NCSlots,
	(SlotInfo*)cl_dial_NPSlots,
	(SlotInfo*)cl_dial_CSlots,
	(SlotInfo*)cl_dial_PSlots
};

MethodInfo meths_dial[] = {
	/* local methods */
{
	STR_config,
	meth_dial_config
},{
	STR_expose,
	meth_dial_expose
},{
	STR_initialize,
	meth_dial_initialize
},{
	STR_render,
	meth_dial_render
},{
	NULL
}
};

ClassInfo class_dial = {
	helper_field_get,
	helper_field_set,
	slots_dial,		/* class slot information	*/
	meths_dial,		/* class methods		*/
	STR_dial,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

int meth_dial_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_config(self, result, argc, argv);
}

int meth_dial_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_render(self, result, argc, argv);
}

int meth_dial_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	meth_field_initialize(self, result, argc, argv);
	return 1;
}

int meth_dial_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	if (!w) meth_field_render(self, result, argc, argv);
	if (!(w = GET_window(self))) return 0;
	GLDrawDial(w, GET_width(self) / 2, GET_height(self) / 2, 
		   (int)((float)GET_width(self) / 1.8), 
		   GET_shownPositionV(self), 0); /*XXX lame*/
	return 1;
}

