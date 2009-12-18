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
 * class	: XBMBG
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
#include "cl_XBMBG.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_XBMBG_NCSlots[] = {
	NULL
};
SlotInfo cl_XBMBG_NPSlots[] = {
	NULL
};
SlotInfo cl_XBMBG_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"XBMBG"
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
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = XBMBG: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_XBMBG_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_XBMBG
},{
	NULL
}
};

SlotInfo *slots_XBMBG[] = {
	(SlotInfo*)cl_XBMBG_NCSlots,
	(SlotInfo*)cl_XBMBG_NPSlots,
	(SlotInfo*)cl_XBMBG_CSlots,
	(SlotInfo*)cl_XBMBG_PSlots
};

MethodInfo meths_XBMBG[] = {
	/* local methods */
{
	STR_expose,
	meth_XBMBG_expose
},{
	STR_render,
	meth_XBMBG_render
},{
	NULL
}
};

ClassInfo class_XBMBG = {
	helper_XBM_get,
	helper_XBM_set,
	slots_XBMBG,		/* class slot information	*/
	meths_XBMBG,		/* class methods		*/
	STR_XBMBG,		/* class identifier number	*/
	&class_XBM,		/* super class info		*/
};

int meth_XBMBG_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;
	int width, height, hotx, hoty;

	meth_field_expose(self, result, argc, argv);
	
	if (!(w = GET_window(self))) return 0;

	/* until pixmap width/height can be gotten othewise, 
	 * callGLMakeXBMFromASCII()...
	 */
	{
		char *cp;

		if (!(cp = GET_label(self))) return 0;
		pixmap = GLMakeXBMFromASCII(w, cp, &width, &height, 
					    &hotx, &hoty);
		SET__label(self, pixmap);
		SET_width(self, width);
		SET_height(self, height);

		if (pixmap && GET_visible(self)) {
			GLDisplayXBMBG(w, 0, 0, 
				GET_width(self), GET_height(self),
				pixmap, width, height); 
			GLClearWindow(w);
			result->info.i = 1;
			return 1;
		}
	}
	result->info.i = 0;
	return 0;
}

int meth_XBMBG_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;
	int width, height;
	char *cp;

	if (!meth_field_render(self, result, argc, argv)) return 0;
	
	if (!(w = GET_window(self))) return 0;

	if (!(cp = GET_label(self))) {
		result->type = PKT_INT;
		result->info.i = 0;
		return 0;
	}
	pixmap = GLMakeXBMFromASCII(w, cp, &width, &height, 
					(int*)NULL, (int*)NULL);
	SET__label(self, pixmap);
	SET_width(self, width);
	SET_height(self, height);

	if (pixmap && GET_visible(self)) {
		GLDisplayXBMBG(w, 0, 0, GET_width(self), GET_height(self),
			pixmap, width, height); 
		result->info.i = 1;
		return 1;
	}
	result->info.i = 0;
	return 0;
}








