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
 * class	: XBMButton
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
#include "cl_XBMButton.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_XBMButton_NCSlots[] = {
	NULL
};
SlotInfo cl_XBMButton_NPSlots[] = {
	NULL
};
SlotInfo cl_XBMButton_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"XBMButton"
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
		break;\n\
		case \"buttonPress\":\n\
			copyArea(0,0,width(),height(),1,1);\n\
			/*smudge();*/\n\
		break;\n\
		case \"buttonRelease\":\n\
			render();/*clean();*/\n\
		break;\n\
		case \"expose\":\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"key_up\":\n\
			send(parent(), \"key_up\");\n\
		break;\n\
		case \"key_down\":\n\
			send(parent(), \"key_down\");\n\
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
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"interpret\":\n\
			interpret(arg[1]);\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = XBMButton: args: \");\n\
			for (i =0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\\n\");\n\
			break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_XBMButton_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_XBMButton
},{
	NULL
}
};

SlotInfo *slots_XBMButton[] = {
	(SlotInfo*)cl_XBMButton_NCSlots,
	(SlotInfo*)cl_XBMButton_NPSlots,
	(SlotInfo*)cl_XBMButton_CSlots,
	(SlotInfo*)cl_XBMButton_PSlots
};

MethodInfo meths_XBMButton[] = {
	/* local methods */
{
	STR_render,
	meth_XBMButton_render
},{
	NULL
}
};

ClassInfo class_XBMButton = {
	helper_XBM_get,
	helper_XBM_set,
	slots_XBMButton,	/* class slot information	*/
	meths_XBMButton,	/* class methods		*/
	STR_XBMButton,		/* class identifier number	*/
	&class_XBM,		/* super class info		*/
};

int meth_XBMButton_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
  	Window w = GET_window(self);
	int stat;

	stat = meth_XBM_render(self, result, argc, argv);
	if (w && GET_visible(self)) {
	  GLDrawBorder(w, 0, 0, 
		       GET_width(self)-1, GET_height(self)-1,
		       GET_border(self), 1);
	}
	return stat;
}








