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
 * class	: XPMBG
 * superClass	: XPM
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
#include "cl_XPMBG.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_XPMBG_NCSlots[] = {
	NULL
};
SlotInfo cl_XPMBG_NPSlots[] = {
	NULL
};
SlotInfo cl_XPMBG_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"XPMBG"
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
		case \"buttonRelease\":\n\
		case \"shownPositionH\":\n\
		case \"shownPositionV\":\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"expose\":\n\
			expose(arg[1], arg[2], arg[3], arg[4]);\n\
			clearWindow();\n\
		break;\n\
		case \"configSelf\":\n\
			/* icky! but necessary to give script a chance to \n\
			 * intercept and do something...*/\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"render\":\n\
			render();\n\
			clearWindow();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
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
		default:\n\
			print(\"unknown message, clsss = XPMBG: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_XPMBG_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_XPMBG
},{
	NULL
}
};

SlotInfo *slots_XPMBG[] = {
	(SlotInfo*)cl_XPMBG_NCSlots,
	(SlotInfo*)cl_XPMBG_NPSlots,
	(SlotInfo*)cl_XPMBG_CSlots,
	(SlotInfo*)cl_XPMBG_PSlots
};

MethodInfo meths_XPMBG[] = {
	/* local methods */
{
	STR_render,
	meth_XPMBG_render
},{
	NULL
}
};

ClassInfo class_XPMBG = {
	helper_XPM_get,
	helper_XPM_set,
	slots_XPMBG,		/* class slot information	*/
	meths_XPMBG,		/* class methods		*/
	STR_XPMBG,		/* class identifier number	*/
	&class_XPM,		/* super class info		*/
};


int meth_XPMBG_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;

	if (!w) meth_field_render(self, result, argc, argv);
	
	if (!(w = GET_window(self))) return 0;
	if (pixmap = (Pixmap)GET__label(self)) {
		GLDisplayXPMBG(w, 0, 0, GET_width(self), GET_height(self),
				pixmap); 
	} else {
		int width, height, hotx, hoty;
		char *cp;

		if (!(cp = GET_label(self))) return 0;
		pixmap = GLMakeXPMFromASCII(w, cp, &width, &height, 
						&hotx, &hoty);
		if (!pixmap) return 0;

		SET__label(self, pixmap);
		SET_width(self, width);
		SET_height(self, height);
		GLDisplayXPMBG(w, 0, 0, 
			GET_width(self), GET_height(self), pixmap);
	}
	return 1;
}





