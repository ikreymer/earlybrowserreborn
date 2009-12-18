/*XXXX No classScript specified, thus pcode may be incorrectly generated */
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
 * class	: project
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
#include "cl_project.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_project_NCSlots[] = {
	NULL
};
SlotInfo cl_project_NPSlots[] = {
{
	STR_filePath,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_projectIcon,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__projectIcon,	/* internalized format */
	PTRV,
	NULL
},{
	NULL
}
};
SlotInfo cl_project_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"project"
},{
	NULL
}
};
SlotInfo cl_project_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_project
},{
	NULL
}
};

SlotInfo *slots_project[] = {
	(SlotInfo*)cl_project_NCSlots,
	(SlotInfo*)cl_project_NPSlots,
	(SlotInfo*)cl_project_CSlots,
	(SlotInfo*)cl_project_PSlots
};

MethodInfo meths_project[] = {
	/* local methods */
{
	STR_config,
	meth_project_config
},{
	STR_expose,
	meth_project_expose
},{
	STR_geta,
	meth_project_get,
},{
	STR_initialize,
	meth_project_initialize
},{
	STR_render,
	meth_project_render
},{
	STR_seta,
	meth_project_set
},{
	NULL
}
};

ClassInfo class_project = {
	helper_project_get,
	helper_project_set,
	slots_project,		/* class slot information	*/
	meths_project,		/* class methods		*/
	STR_project,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

int meth_project_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_field_config(self, result, argc, argv)) return 0;
	return 1;
}

int meth_project_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_render(self, result, argc, argv);
}

int helper_project_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_filePath:
		result->info.s = SaveString(GET_filePath(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_projectIcon:
		result->info.s = SaveString(GET_projectIcon(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_field_get(self, result, argc, argv, labelID);
}
int meth_project_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_project_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_project_initialize(self, result, argc, argv)
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
int helper_project_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_filePath:
		result->type = PKT_STR;
		result->canFree = 0;
		result->info.s = SaveString(argv[1].info.s);
		SET_filePath(self, result->info.s);
		return 1;

	case STR_label:
		meth_field_set(self, result, argc, argv);
		if (GET_window(self)) 
			GLSetWindowName(GET_window(self), GET_label(self));
		return 1;

	case STR_projectIcon: {
		Pixmap pixmap;
		char *cp;
		int width, height, hotx, hoty;

		result->type = PKT_STR;
		result->canFree = 0;
		if (!(result->info.s = SaveString(argv[1].info.s))) return 0;

		if (cp = GET_projectIcon(self)) free(cp);
		SET_projectIcon(self, result->info.s);

		if (!(pixmap = (Pixmap)GET__projectIcon(self))) return 0;
		pixmap = GLMakeXBMFromASCII(rootWindow, cp, &width, &height, 
						&hotx, &hoty);
		SET__projectIcon(self, pixmap);
		return 1;
		}
	}
	return helper_field_set(self, result, argc, argv, labelID);
}
int meth_project_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_project_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_project_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	Pixmap pixmap;

	if (!w) meth_field_render(self, result, argc, argv);
	if (!(w = GET_window(self))) return 0;

	GLSetWindowName(w, GET_label(self));

	return 1;
}








