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
 * class	: AC_SGML_txt
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
#include "cl_AC_SGML_txt.h"
#include "misc.h"
#include "glib.h"

SlotInfo cl_AC_SGML_txt_NCSlots[] = {
{
	NULL
}
};
SlotInfo cl_AC_SGML_txt_NPSlots[] = {
	NULL
};
SlotInfo cl_AC_SGML_txt_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"AC_SGML_txt"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
\n\
	case 'I': /* input */\n\
	case 'A': /* absorb */\n\
	case 'D': /* done */\n\
		return test1(arg);\n\
	break;\n\
	case 'R': /* resize */\n\
		clearWindow();\n\
		vspan = SGMLGetStyle(\"P\", \"top\");\n\
		set(\"y\", arg[1] + vspan);\n\
		set(\"x\", SGMLGetStyle(\"P\", \"left\"));\n\
		set(\"width\", arg[2] - x() - SGMLGetStyle(\"P\", \"right\"));\n\
		set(\"content\", get(\"content\"));\n\
		vspan = vspan + set(\"height\", building_vspan())\n\
			+ SGMLGetStyle(\"P\", \"bottom\");\n\
		render();\n\
		return vspan;\n\
	break;\n\
	case \"config\":\n\
		return;\n\
	break;\n\
	case \"gotoAnchor\":\n\
		return 0;\n\
	break;\n\
	case \"buttonRelease\":\n\
		usual();\n\
print(\"*************** \n\");\n\
		if (charButtonMask() > 0) {\n\
			ref = nextTag();\n\
print(\"***************  ref = {\", ref, \"}\n\");\n\
			if (ref) {\n\
				if (shiftKeyP()) {\n\
					www.mesg.tf(\"show\",\n\
						concatenate(\"Dest: \", ref));\n\
				} else {\n\
					www(\"show\", ref);\n\
				}\n\
			} else {\n\
				bell();\n\
				www.mesg.tf(\"warn\", \n\
				    \"Try clicking the ending parts.\");\n\
			}\n\
		}\n\
		return;\n\
	break;\n\
\n\
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
		break;\n\
		case \"configSelf\":\n\
			config();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"clearSelection\":\n\
			clearSelection();\n\
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
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = AC_SGML_txt: args: \");\n\
			for (i =0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\\n\");\n\
			break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_AC_SGML_txt_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_AC_SGML_txt
},{
	STR_width,
	LONG,
	100
},{
	STR_height,
	LONG,
	30
},{
	STR_font,		/* font identifier */
 	PTRS | SLOT_RW,
	(long)"normal"
},{
	STR_BDColor,		/* border color */
	PTRS | SLOT_RW,
 	(long)"LemonChiffon1"
},{
	STR_BGColor,		/* background color */
	PTRS | SLOT_RW,
 	(long)"LemonChiffon1"
},{
	STR_FGColor,		/* foreground color */
	PTRS | SLOT_RW,
 	(long)"black"
},{
	STR_format,
	LONG | SLOT_RW,
	1
},{
	NULL
}
};

SlotInfo *slots_AC_SGML_txt[] = {
	(SlotInfo*)cl_AC_SGML_txt_NCSlots,
	(SlotInfo*)cl_AC_SGML_txt_NPSlots,
	(SlotInfo*)cl_AC_SGML_txt_CSlots,
	(SlotInfo*)cl_AC_SGML_txt_PSlots
};

MethodInfo meths_AC_SGML_txt[] = {
	/* local methods */
{
	STR_initialize,
	meth_AC_SGML_txt_initialize
},{
	STR_render,
	meth_AC_SGML_txt_render
},{
	NULL
}
};

ClassInfo class_AC_SGML_txt = {
	helper_txtDisp_get,
	helper_txtDisp_set,
	slots_AC_SGML_txt,		/* class slot information	*/
	meths_AC_SGML_txt,		/* class methods		*/
	STR_AC_SGML_txt,		/* class identifier number	*/
	&class_txtDisp,		/* super class info		*/
};

int meth_AC_SGML_txt_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int stat;

	/* special sin.. er case: use parent's color info */
	SET__colorInfo(self, 1); /* fool into not initialze. */

	stat = meth_txtDisp_initialize(self, result, argc, argv);

	if (GET__parent(self)) 
		SET__colorInfo(self, GET__colorInfo(GET__parent(self)));

	return stat;
}

int meth_AC_SGML_txt_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_txtDisp_render(self, result, argc, argv);
}

