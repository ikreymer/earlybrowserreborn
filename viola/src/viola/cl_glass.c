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
 * class	: glass
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
#include "cl_glass.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_glass_NCSlots[] = {
	NULL
};
SlotInfo cl_glass_NPSlots[] = {
{
	NULL
}
};
SlotInfo cl_glass_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"glass"
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
		case \"expose\":\n\
		case \"render\":\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			return;\n\
		break;\n\
		case \"configSelf\":\n\
			config(get(\"x\"), get(\"y\"), get(\"width\"), get(\"height\"));\n\
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
			print(\"unknown message, clsss = glass: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_glass_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_glass
},{
	STR_border,
	LONG | SLOT_RW,
	BORDER_NONE
},{
	NULL
}
};

SlotInfo *slots_glass[] = {
	(SlotInfo*)cl_glass_NCSlots,
	(SlotInfo*)cl_glass_NPSlots,
	(SlotInfo*)cl_glass_CSlots,
	(SlotInfo*)cl_glass_PSlots
};

MethodInfo meths_glass[] = {
	/* local methods */
{
	NULL
}
};

ClassInfo class_glass = {
	helper_field_get,
	helper_field_set,
	slots_glass,		/* class slot information	*/
	meths_glass,		/* class methods		*/
	STR_glass,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};


