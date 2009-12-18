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
 * class	: radio
 * superClass	: toggle
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
#include "cl_radio.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_radio_NCSlots[] = {
	NULL
};
SlotInfo cl_radio_NPSlots[] = {
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
SlotInfo cl_radio_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"radio"
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
			send(get(\"parent\"), \"justMe\", get(\"name\"));\n\
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
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			return;\n\
		break;\n\
		case \"configSelf\":\n\
			config(get(\"x\"), get(\"y\"), get(\"width\"), get(\"height\"));\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
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
			print(\"unknown message, clsss = radio: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_radio_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_radio
},{
	NULL
}
};

SlotInfo *slots_radio[] = {
	(SlotInfo*)cl_radio_NCSlots,
	(SlotInfo*)cl_radio_NPSlots,
	(SlotInfo*)cl_radio_CSlots,
	(SlotInfo*)cl_radio_PSlots
};

MethodInfo meths_radio[] = {
	/* local methods */
{
	NULL
}
};

ClassInfo class_radio = {
	helper_toggle_get,
	helper_toggle_set,
	slots_radio,		/* class slot information	*/
	meths_radio,		/* class methods		*/
	STR_radio,		/* class identifier number	*/
	&class_toggle,		/* super class info		*/
};

