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
 * class	: txtLabel
 * superClass	: txt
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
#include "cl_txtLabel.h"
#include "misc.h"
#include "glib.h"

SlotInfo cl_txtLabel_NCSlots[] = {
	NULL
};
SlotInfo cl_txtLabel_NPSlots[] = {
	NULL
};
SlotInfo cl_txtLabel_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"txtLabel"
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
		break;\n\
		case \"expose\":\n\
			render();\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			render();\n\
		break;\n\
		case \"configSelf\":\n\
			config();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
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
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = txtLabel: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_txtLabel_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_txtLabel
},{
	STR_paneConfig,
 	PTRS | SLOT_RW,
	(long)"center"
},{
	NULL
}
};

SlotInfo *slots_txtLabel[] = {
	(SlotInfo*)cl_txtLabel_NCSlots,
	(SlotInfo*)cl_txtLabel_NPSlots,
	(SlotInfo*)cl_txtLabel_CSlots,
	(SlotInfo*)cl_txtLabel_PSlots
};

MethodInfo meths_txtLabel[] = {
	/* local methods */
{
	STR_config,
	meth_txtLabel_config,
},{
	STR_initialize,
	meth_txtLabel_initialize
},{
	STR_render,
	meth_txtLabel_render
},{
	NULL
}
};

ClassInfo class_txtLabel = {
	helper_txt_get,
	helper_txt_set,
	slots_txtLabel,		/* class slot information	*/
	meths_txtLabel,		/* class methods		*/
	STR_txtLabel,		/* class identifier number	*/
	&class_txt,		/* super class info		*/
};

int meth_txtLabel_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_txt_config(self, result, argc, argv);
}

int meth_txtLabel_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_txt_initialize(self, result, argc, argv);
}

int meth_txtLabel_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w;

	if (!meth_txt_render(self, result, argc, argv)) return 0;

	w = GET_window(self);
	if (w && GET_visible(self)) {

		char *config = GET_paneConfig(self);
		char *str = GET_label(self);
		int fontID = GET__font(self);

		if (!str) return 0;

		if (config[0] == 'c') {			/* center */
			GLDrawText(w, fontID, 
			  (GET_width(self) - GLTextWidth(fontID, str)) / 2,
			  (GET_height(self) - GLTextHeight(fontID, str)) / 2,
			  str);
		} else if (config[0] == 'w') {		/* westToEast */
			GLDrawText(w, fontID, 
			  0,
			  (GET_height(self) - GLTextHeight(fontID, str)) / 2,
			  str);
		} else if (config[0] == 'e') {		/* eastToWest */
			GLDrawText(w, fontID, 
			  GET_width(self) - GLTextWidth(fontID, str) - 1,
			  (GET_height(self) - GLTextHeight(fontID, str)) / 2,
			  str);
		} else if (config[0] == 'n') {		/* northToSouth */
			GLDrawText(w, fontID, 
			  (GET_width(self) - GLTextWidth(fontID, str)) / 2,
			  0,
			  str);
		} else if (config[0] == 's') {		/* southToNorth */
			GLDrawText(w, fontID, 
			  (GET_width(self) - GLTextWidth(fontID, str)) / 2,
			  GET_height(self) - GLTextHeight(fontID, str),
			  str);
		} else {				/* default northWest */
			GLDrawText(w, fontID, 
			  0,
 			  0,
			  str);
		}
		return 1;
	}
	return 0;
	
}

