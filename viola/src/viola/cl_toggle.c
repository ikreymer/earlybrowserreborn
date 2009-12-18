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
 * class	: toggle
 * superClass	: XBM
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
#include "cl_toggle.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_toggle_NCSlots[] = {
	NULL
};
SlotInfo cl_toggle_NPSlots[] = {
{
	STR_toggleState,
	LONG | SLOT_RW,
	0
},{
	STR_toggleStyle,
	PTRS | SLOT_RW,
	(long)"check"
},{
	NULL
}
};
SlotInfo cl_toggle_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"toggle"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"mouseMove\":\n\
		case \"enter\":\n\
		case \"leave\":\n\
		case \"keyPress\":\n\
		case \"keyRelease\":\n\
		case \"buttonPress\":\n\
		break;\n\
		case \"buttonRelease\":\n\
			toggle();\n\
			return;\n\
		break;\n\
		case \"toggleTo\":\n\
			if (get(\"toggleState\") != arg[1])\n\
				set(\"toggleState\", arg[1]);\n\
			return;\n\
		break;\n\
		case \"toggle\":\n\
			toggle();\n\
			return;\n\
		break;\n\
		case \"expose\":\n\
			render();\n\
			return;\n\
		break;\n\
		case \"render\":\n\
			render();\n\
			return;\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
			return;\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			return;\n\
		break;\n\
		case \"configSelf\":\n\
			config(get(\"x\"), get(\"y\"), get(\"width\"), get(\"height\"));\n\
			return;\n\
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
			return;\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
			return;\n\
		break;\n\
		case \"info\":\n\
			info();\n\
			return;\n\
		break;\n\
		case \"freeSelf\":\n\
			return freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = toggle: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_toggle_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_toggle
},{
	NULL
}
};

SlotInfo *slots_toggle[] = {
	(SlotInfo*)cl_toggle_NCSlots,
	(SlotInfo*)cl_toggle_NPSlots,
	(SlotInfo*)cl_toggle_CSlots,
	(SlotInfo*)cl_toggle_PSlots
};

MethodInfo meths_toggle[] = {
	/* local methods */
{
	STR_geta,
	meth_toggle_get,
},{
	STR_render,
	meth_toggle_render
},{
	STR_seta,
	meth_toggle_set
},{
	STR_toggle,
	meth_toggle_toggle
},{
	NULL
}
};

ClassInfo class_toggle = {
	helper_toggle_get,
	helper_toggle_set,
	slots_toggle,		/* class slot information	*/
	meths_toggle,		/* class methods		*/
	STR_toggle,		/* class identifier number	*/
	&class_XBM,		/* super class info		*/
};

int helper_toggle_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_toggleState:
		result->info.i = GET_toggleState(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_toggleStyle:
		result->info.s = SaveString(GET_toggleStyle(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_XBM_get(self, result, argc, argv, labelID);
}
int meth_toggle_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_toggle_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_toggle_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_toggleState:
		SET_toggleState(self, argv[1].info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		if (GET_visible(self)) 
			meth_toggle_render(self, result, argc, argv);
		return 1;

	case STR_toggleStyle:
		result->info.s = SaveString(argv[1].info.s);
		SET_toggleStyle(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		if (GET_visible(self))
			meth_toggle_render(self, result, argc, argv);
		return 1;
	}
	return helper_XBM_set(self, result, argc, argv, labelID);
}
int meth_toggle_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_toggle_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_toggle_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;

	if (!meth_field_render(self, result, argc, argv)) return 0;

	if (!(w = GET_window(self))) return 0;

	/* button, disc, check (default) */
	if (GET_toggleState(self)) {
		switch (*GET_toggleStyle(self)) {
		case 'd': 		pixmap = togglePixmap_disc1;	break;
		case 'b':		pixmap = togglePixmap_button1;	break;
		case 'c': default:	pixmap = togglePixmap_check1;	break;
		}
	} else {
		switch (*GET_toggleStyle(self)) {
		case 'd': 		pixmap = togglePixmap_disc0;	break;
		case 'b':		pixmap = togglePixmap_button0;	break;
		case 'c': default:	pixmap = togglePixmap_check0;	break;
		}
	}
	GLDisplayXBM(w, 0, 0, GET_width(self), GET_height(self), pixmap); 
	GLDrawBorder(w, 0, 0, 
		     GET_width(self)-1, GET_height(self)-1,
		     GET_border(self), 1);
	return 1;
}

int meth_toggle_toggle(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (GET_toggleState(self))
		SET_toggleState(self, 0);
	else 
		SET_toggleState(self, 1);

	meth_toggle_render(self, result, argc, argv);

	return 1;
}


