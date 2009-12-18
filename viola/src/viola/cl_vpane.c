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
 * class	: vpane
 * superClass	: pane
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
#include "cl_vpane.h"
#include "misc.h"
#include "glib.h"

SlotInfo cl_vpane_NCSlots[] = {
	NULL
};
SlotInfo cl_vpane_NPSlots[] = {
	NULL
};
SlotInfo cl_vpane_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"vpane"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"render\":\n\
		case \"expose\":\n\
			render();\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
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
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
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
SlotInfo cl_vpane_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_vpane
},{
	STR_paneConfig,
 	PTRS,
	(long)"northToSouth"
},{
	STR__paneConfig,
 	LONG,
	PANE_CONFIG_N2S		/* default with N2S */
},{
	NULL
}
};

SlotInfo *slots_vpane[] = {
	(SlotInfo*)cl_vpane_NCSlots,
	(SlotInfo*)cl_vpane_NPSlots,
	(SlotInfo*)cl_vpane_CSlots,
	(SlotInfo*)cl_vpane_PSlots
};

MethodInfo meths_vpane[] = {
	/* local methods */
	NULL
};

ClassInfo class_vpane = {
	helper_pane_get,
	helper_pane_set,
	slots_vpane,		/* class slot information	*/
	meths_vpane,		/* class methods		*/
	STR_vpane,		/* class identifier number	*/
	&class_pane,		/* super class info		*/
};

