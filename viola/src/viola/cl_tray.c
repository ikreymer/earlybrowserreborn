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
 * class	: tray
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
#include "cl_tray.h"
#include "misc.h"
#include "glib.h"

SlotInfo cl_tray_NCSlots[] = {
	NULL
};
SlotInfo cl_tray_NPSlots[] = {
	NULL
};
SlotInfo cl_tray_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"tray"
},{
	NULL
}
};
SlotInfo cl_tray_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_tray
},{
	NULL
}
};

SlotInfo *slots_tray[] = {
	(SlotInfo*)cl_tray_NCSlots,
	(SlotInfo*)cl_tray_NPSlots,
	(SlotInfo*)cl_tray_CSlots,
	(SlotInfo*)cl_tray_PSlots
};

MethodInfo meths_tray[] = {
	/* local methods */
{
	STR_config,
	meth_tray_config
},{
	STR_initialize,
	meth_tray_initialize
},{
	STR_render,
	meth_tray_render
},{
	NULL
}
};

ClassInfo class_tray = {
	helper_field_get,
	helper_field_set,
	slots_tray,		/* class slot information	*/
	meths_tray,		/* class methods		*/
	STR_tray,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

helper_tray_config(self, oldWidth, oldHeight)
	VObj *self;
	int oldWidth, oldHeight;
{
	VObjList *olist, *children = GET__children(self);
	float xratio, yratio;
	Window w;
	int x, y, width, height;

	xratio = GET_width(self) / oldWidth;
	yratio = GET_height(self) / oldHeight;

	for (olist = children; olist; olist = olist->next) {
		x = xratio * (float)GET_x(olist->o);
		y = yratio * (float)GET_y(olist->o);
		width = xratio * (float)GET_width(olist->o);
		height = yratio * (float)GET_height(olist->o);
		if (GET_window(self) && GET_visible(self))
		  GLUpdateGeometry(0, GET_window(olist->o), 
				   x, y, width, height);
	}
}

int meth_tray_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int oldWidth, oldHeight;

	oldWidth = GET_width(self);
	oldHeight = GET_height(self);

	if (!meth_field_config(self, result, argc, argv)) return 0;

	if (GET__children(self))
		helper_tray_config(self, oldWidth, oldHeight);

	return 1;
}

int meth_tray_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_field_initialize(self, result, argc, argv);
}

int meth_tray_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	if (!w)
		if (!(w = GLOpenWindow(self, GET_x(self), GET_y(self),
				  GET_width(self), GET_height(self), 0)))
			return 0;
	GLDrawBorder(w, 0, 0, GET_width(self), GET_height(self), 
		     GET_border(self), 1);
	return 1;
}

