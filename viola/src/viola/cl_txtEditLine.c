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
 * class	: txtEditLine
 * superClass	: txtDisp
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
#include "cl_txtEditLine.h"
#include "misc.h"
#include "glib.h"

SlotInfo cl_txtEditLine_NCSlots[] = {
{
	NULL
}
};
SlotInfo cl_txtEditLine_NPSlots[] = {
	NULL
};
SlotInfo cl_txtEditLine_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"txtEditLine"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"keyPress\":\n\
			insert(key());\n\
		break;\n\
		case \"mouseMove\":\n\
		case \"keyRelease\":\n\
		case \"buttonRelease\":\n\
		break;\n\
		case \"shownPositionV\":\n\
			set(\"shownPositionV\", arg[1]);\n\
		break;\n\
		case \"buttonPress\":\n\
			processMouseInput();\n\
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
		case \"shownPositionH\":\n\
			set(\"shownPositionH\", arg[1]);\n\
		break;\n\
		case \"enter\":\n\
			set(\"cursor\", 1);\n\
		break;\n\
		case \"leave\":\n\
			set(\"cursor\", 0);\n\
		break;\n\
		case \"scroll\":\n\
			if (arg[1] == \"down\") {\n\
				set(\"shownPositionV\",\n\
					get(\"shownPositionV\") -\n\
					get(\"shownSizeV\"));\n\
			} else if (arg[1] == \"up\") {\n\
				set(\"shownPositionV\",\n\
					get(\"shownPositionV\") +\n\
					get(\"shownSizeV\"));\n\
			}\n\
		break;\n\
		case \"key_up\":\n\
			send(self(), \"scroll\", \"up\");\n\
			return;\n\
		break;\n\
		case \"key_down\":\n\
			send(self(), \"scroll\", \"down\");\n\
			return;\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			render();\n\
			return;\n\
		break;\n\
		case \"configSelf\":\n\
			config();\n\
			return;\n\
		break;\n\
		case \"info\":\n\
			info();\n\
			return;\n\
		break;\n\
		case \"clearSelection\":\n\
			clearSelection();\n\
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
		break;\n\
		case \"init\":\n\
			initialize();\n\
			return;\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
			return;\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = txtEditLine: args: \");\n\
			for (i =0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\\n\");\n\
			break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_txtEditLine_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_txtEditLine
},{
	STR_border,
	LONG | SLOT_RW,
	BORDER_SINK
},{
	NULL
}
};

SlotInfo *slots_txtEditLine[] = {
	(SlotInfo*)cl_txtEditLine_NCSlots,
	(SlotInfo*)cl_txtEditLine_NPSlots,
	(SlotInfo*)cl_txtEditLine_CSlots,
	(SlotInfo*)cl_txtEditLine_PSlots
};

MethodInfo meths_txtEditLine[] = {
	/* local methods */
{
	STR_initialize,
	meth_txtEditLine_initialize
},{
	STR_expose,
	meth_txtEditLine_expose,
},{
	STR_render,
	meth_txtEditLine_render
},{
	NULL
}
};

ClassInfo class_txtEditLine = {
	helper_txtDisp_get,
	helper_txtDisp_set,
	slots_txtEditLine,		/* class slot information	*/
	meths_txtEditLine,		/* class methods		*/
	STR_txtEditLine,		/* class identifier number	*/
	&class_txtDisp,		/* super class info		*/
};

int meth_txtEditLine_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_txtDisp_initialize(self, result, argc, argv);
}

int meth_txtEditLine_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	int stat;
/*XXX WATCH OUT. This is cheating and not calling super methods.
	meth_txt_expose(self, result, argc, argv);
*/

	GLPrepareObjColor(self);
	if (argc == 0) {
		stat = tfed_expose(self, 0, 0, 
					GET_width(self), GET_height(self));
	} else {
		stat = tfed_expose(self, argv[0].info.i, argv[1].info.i, 
				   	 argv[2].info.i, argv[3].info.i);
	}
	if (w && GET_visible(self)) {
	  GLDrawBorder(w, 0, 0, 
		       GET_width(self)-1, GET_height(self)-1,
		       GET_border(self), 1);
	}
	return stat;
}


int meth_txtEditLine_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_txtDisp_render(self, result, argc, argv);
}

