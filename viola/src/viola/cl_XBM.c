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
 * class	: XBM
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
#include "cl_XBM.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_XBM_NCSlots[] = {
	NULL
};
SlotInfo cl_XBM_NPSlots[] = {
	NULL
};
SlotInfo cl_XBM_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"XBM"
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
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"configSelf\":\n\
			config(x(), y(), width(), height());\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"key_up\":\n\
			send(parent(), \"key_up\");\n\
		break;\n\
		case \"key_down\":\n\
			send(parent(), \"key_down\");\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = XBM: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_XBM_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_XBM
},{
	NULL
}
};

SlotInfo *slots_XBM[] = {
	(SlotInfo*)cl_XBM_NCSlots,
	(SlotInfo*)cl_XBM_NPSlots,
	(SlotInfo*)cl_XBM_CSlots,
	(SlotInfo*)cl_XBM_PSlots
};

MethodInfo meths_XBM[] = {
	/* local methods */
{
	STR_config,
	meth_XBM_config
},{
	STR_expose,
	meth_XBM_expose
},{
	STR_geta,
	meth_XBM_get,
},{
	STR_freeSelf,
	meth_XBM_freeSelf,
},{
	STR_initialize,
	meth_XBM_initialize
},{
	STR_render,
	meth_XBM_render
},{
	STR_seta,
	meth_XBM_set
},{
	NULL
}
};

ClassInfo class_XBM = {
	helper_XBM_get,
	helper_XBM_set,
	slots_XBM,		/* class slot information	*/
	meths_XBM,		/* class methods		*/
	STR_XBM,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

int meth_XBM_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_config(self, result, argc, argv);
}

int meth_XBM_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;

	meth_field_expose(self, result, argc, argv);
	
	if (!(w = GET_window(self))) return 0;
	if (!(pixmap = (Pixmap)GET__label(self))) {
		int width = 0, height = 0, hotx, hoty;
		char *cp;

		if (!(cp = GET_label(self))) return 0;
		pixmap = GLMakeXBMFromASCII(w, cp, &width, &height, 
					    &hotx, &hoty);
		if (GET__label(self)) 
			XFreePixmap(display, (Pixmap)GET__label(self));
		SET__label(self, pixmap);
		SET_width(self, width);
		SET_height(self, height);
	}
	if (pixmap && GET_visible(self)) {
		GLDisplayXBM(w, 0, 0, GET_width(self), GET_height(self),
			     pixmap); 
		result->info.i = 1;
		return 1;
	}
	result->info.i = 0;
	return 0;
}

int meth_XBM_freeSelf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!exitingViola) {
	  if (GET__label(self)) {
	    XFreePixmap(display, (Pixmap)GET__label(self));
	    SET__label(self, NULL);
	  }
	}
	meth_field_freeSelf(self, result, argc, argv);
	return 1;
}

int helper_XBM_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	return helper_field_get(self, result, argc, argv, labelID);
}
int meth_XBM_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_XBM_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_XBM_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_initialize(self, result, argc, argv);
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_XBM_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_label: {
		Pixmap pixmap;
		int width = 0, height = 0, hotx, hoty;
		char *cp;

		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		if (GET_label(self)) free(GET_label(self));
		SET_label(self, result->info.s);

		pixmap = GLMakeXBMFromASCII(rootWindow, result->info.s,
					    &width, &height, &hotx, &hoty);
		if (GET__label(self)) 
			XFreePixmap(display, (Pixmap)GET__label(self));
		SET__label(self, pixmap);

		SET_width(self, width);
		SET_height(self, height);
		return 1;
		}
	}      
	return helper_field_set(self, result, argc, argv, labelID);
}
int meth_XBM_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_XBM_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_XBM_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w;
	Pixmap pixmap;

	if (!meth_field_render(self, result, argc, argv)) return 0;
	
	if (!(w = GET_window(self))) return 0;
	if (!(pixmap = (Pixmap)GET__label(self))) {
		int width = 0, height = 0, hotx, hoty;
		char *cp;

		if (!(cp = GET_label(self))) return 0;
		pixmap = GLMakeXBMFromASCII(w, cp, &width, &height, 
					    &hotx, &hoty);
		if (GET__label(self)) 
			XFreePixmap(display, (Pixmap)GET__label(self));
		SET__label(self, pixmap);
		SET_width(self, width);
		SET_height(self, height);
	}
	if (pixmap && GET_visible(self)) {
		GLDisplayXBM(w, 0, 0, GET_width(self), GET_height(self),
			     pixmap); 
		result->info.i = 1;
		return 1;
	}
	result->info.i = 0;
	return 0;
}








