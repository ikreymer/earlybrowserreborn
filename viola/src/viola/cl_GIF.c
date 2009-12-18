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
 * class	: GIF
 * superClass	: field
 */
#include "utils.h"
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
#include "cl_GIF.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_GIF_NCSlots[] = {
	NULL
};
SlotInfo cl_GIF_NPSlots[] = {
{
	STR__baseImage,
	PTRV,
	NULL
},{
	STR__expImage,
	PTRV,
	NULL
},{
	STR_baseImageWidth,
	LONG,
	0
},{
	STR_baseImageHeight,
	LONG,
	0
},{
	NULL
}
};
SlotInfo cl_GIF_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"GIF"
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
			return;\n\
		break;\n\
		case \"expose\":\n\
			expose(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"key_up\":\n\
			send(parent(), \"key_up\");\n\
		break;\n\
		case \"key_down\":\n\
			send(parent(), \"key_down\");\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"freeSelf\":\n\
			freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = GIF: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_GIF_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_GIF
},{
	NULL
}
};

SlotInfo *slots_GIF[] = {
	(SlotInfo*)cl_GIF_NCSlots,
	(SlotInfo*)cl_GIF_NPSlots,
	(SlotInfo*)cl_GIF_CSlots,
	(SlotInfo*)cl_GIF_PSlots
};

MethodInfo meths_GIF[] = {
	/* local methods */
{
	STR_config,
	meth_GIF_config
},{
	STR_expose,
	meth_GIF_expose
},{
	STR_initialize,
	meth_GIF_initialize
},{
	STR_render,
	meth_GIF_render
},{
	NULL
}
};

ClassInfo class_GIF = {
	helper_field_get,
	helper_GIF_set,
	slots_GIF,		/* class slot information	*/
	meths_GIF,		/* class methods		*/
	STR_GIF,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

int meth_GIF_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int new_width, new_height;
	int old_width, old_height;
	XImage *baseImage;
	XImage *expImage;

	old_width = GET_width(self);
	old_height = GET_height(self);

	if (!meth_field_config(self, result, argc, argv)) return 0;

	new_width = GET_width(self);
	new_height = GET_height(self);

	if (!GET__baseImage(self)) {

		Window w;
		int base_width, base_height;
		char *fname;
		 
		w = GET_window(self);
		fname = GET_label(self);
		expImage = (XImage*)GET__expImage(self);
		baseImage = (XImage*)GET__baseImage(self);

		/* free expImage, baseImage here */

		SET__expImage(self, NULL);
		SET__baseImage(self, NULL);
		expImage = NULL;
		baseImage = NULL;

		SET__expImage(self, GLGIFLoad(fname, fname, w, 
				0, 0, new_width, new_height,
				&base_width, &base_height,
				&baseImage));
		SET__baseImage(self, baseImage);
		SET_baseImageWidth(self, base_width);
		SET_baseImageHeight(self, base_height);
	} else {
		if (old_width != new_width || old_height != new_height) {
			if (GET__baseImage(self)) {
				expImage = GLGIFResize(GET_window(self), 
					new_width, new_height,
					GET_baseImageWidth(self), 
					GET_baseImageHeight(self),
					GET__baseImage(self),
					GET__expImage(self));
				SET__expImage(self, expImage);

/*XXX*/				meth_GIF_render(self, result, argc, argv);
			}
		}
	}
	return 1;
}

int meth_GIF_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_GIF_render(self, result, argc, argv);
}

int meth_GIF_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	meth_field_initialize(self, result, argc, argv);
	return 1;
}

int meth_GIF_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	result->type = PKT_INT;
	result->canFree = 0;
	if (!GET_visible(self)) {
		result->info.i = 0;
		return 0;
	}
	GLPrepareObjColor(self);
	if (!w) {
		if (!(w = GLOpenWindow(self, GET_x(self), GET_y(self),
			 GET_width(self), GET_height(self), 0))) {
			return 0;
		}
		if (w) {
			VObjList *olist = GET__children(self);
			for (; olist; olist = olist->next) {
				if (olist->o)
					if (GET_visible(olist->o)) {
						callMeth(olist->o, result, 
						       argc, argv, STR_render);
					}
			}
		}
	}
	if (!w) return 0;
	if (GET__expImage(self)) {
		GLGIFDraw(w, GET__expImage(self),
			  0, 0,
			  GET_width(self), GET_height(self));
		GLDrawBorder(w, 0, 0, 
			GET_width(self)-1, GET_height(self)-1,
			GET_border(self), 1);
	}
	return 1;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_GIF_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_label: {
		Window w;
		int x, y, width, height;
		int base_width, base_height;
		char *fname, *id;
		XImage *baseImage;
		XImage *expImage;
		 
		w = GET_window(self);
		x = GET_x(self);
		y = GET_y(self);

		fname = SaveString(PkInfo2Str(&argv[1]));
		if (GET_label(self)) free(GET_label(self));
		SET_label(self, fname);

	/*	id = SaveString(PkInfo2Str(&argv[2]));*/
		id = SaveString(fname);
/*XXXXX KLUDGE!!!! for until set() takes multiple arguments */
		id = fname; 

		expImage = (XImage*)GET__expImage(self);
		baseImage = (XImage*)GET__baseImage(self);

		/* free expImage, baseImage here */

		SET__expImage(self, NULL);
		SET__baseImage(self, NULL);
		expImage = NULL;
		baseImage = NULL;

		width = GET_width(self);
		height = GET_height(self);

		SET__expImage(self, GLGIFLoad(id, fname, 
					w, x, y, width, height,
					&base_width, &base_height,
					&baseImage));
		SET__baseImage(self, baseImage);
		SET_baseImageWidth(self, base_width);
		SET_baseImageHeight(self, base_height);

		/* quite icky */
		if (width == 0 || width == 1) SET_width(self, base_width);
		if (height == 0 || height == 1) SET_height(self, base_height);

		return 1;
		}
	}      
	return helper_field_set(self, result, argc, argv, labelID);
}
int meth_GIF_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_GIF_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}




