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
 * class	: PS
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
#include "cl_PS.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_PS_NCSlots[] = {
	NULL
};
SlotInfo cl_PS_NPSlots[] = {
	NULL
};
SlotInfo cl_PS_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"PS"
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
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = PS: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_PS_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_PS
},{
	NULL
}
};

SlotInfo *slots_PS[] = {
	(SlotInfo*)cl_PS_NCSlots,
	(SlotInfo*)cl_PS_NPSlots,
	(SlotInfo*)cl_PS_CSlots,
	(SlotInfo*)cl_PS_PSlots
};

MethodInfo meths_PS[] = {
	/* local methods */
{
	STR_config,
	meth_PS_config
},{
	STR_geta,
	meth_PS_get,
},{
	STR_initialize,
	meth_PS_initialize
},{
	STR_render,
	meth_PS_render
},{
	STR_seta,
	meth_PS_set
},{
	NULL
}
};

ClassInfo class_PS = {
	helper_PS_get,
	helper_PS_set,
	slots_PS,		/* class slot information	*/
	meths_PS,		/* class methods		*/
	STR_PS,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

int meth_PS_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_config(self, result, argc, argv);
}

int helper_PS_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_direction:
		result->info.s = SaveString(GET_direction(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_field_get(self, result, argc, argv, labelID);
}
int meth_PS_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_PS_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_PS_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	meth_field_initialize(self, result, argc, argv);
	return 1;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_PS_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_label: {
		Window w = GET_window(self);
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_label(self, result->info.s);
		if (w) {
			sprintf(buff, "%d %d %d %d %d %d %f %f %d %d %d %d",
				0, 			/* bpixmap */
				0,	 		/* orientation */
				0, GET_height(self),	/* lower left */
				0, GET_width(self),	/* upper right */
				(float)screenDPI, (float)screenDPI,	/* resolution */
				0, 0,		/* left, bottom */
				0, 0);		/* top, right */
printf("buff=>%s<\n", buff);
			XChangeProperty(display, GET_window(self),
				XmuInternAtom(display, 	
					XmuMakeAtom("GHOSTVIEW")),
				XA_STRING, 8, PropModeReplace,
				(unsigned char *)buff, strlen(buff));
printf("buff=>%s<\n", buff);
			sprintf(buff, "%d", GET_window(self));
printf("buff=>%s<\n", buff);
/*			setenv("GHOSTVIEW", buff, True);*/
/*			setenv("DISPLAY", itoa(display...), True);*/
			sprintf(buff, "gs %s", GET_label(self));
			system(buff);

			SET__label(self, NULL);
		} else {
			SET__label(self, NULL);
		}
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;
	}
	}
	return helper_field_set(self, result, argc, argv, labelID);
}
int meth_PS_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_PS_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_PS_render(self, result, argc, argv)
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
	} else {
		int width, height, hotx, hoty;
		char *cp;

		if (!(cp = GET_label(self))) return 0;
		pixmap = GLMakeXPMFromASCII(w, cp, &width, &height, 
						&hotx, &hoty);
		SET__label(self, pixmap);
/*		GLDisplayPS(w, 0, 0, GET_width(self), GET_height(self),
			     pixmap);
*/
	}
	return 1;
}








