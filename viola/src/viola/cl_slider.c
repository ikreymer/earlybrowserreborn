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
 * class	: slider
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
#include "cl_slider.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_slider_NCSlots[] = {
	NULL
};
SlotInfo cl_slider_NPSlots[] = {
{
	STR_direction,
	PTRS | SLOT_RW,
	(long)"t"		/* default is top2bottom */
},{
	NULL
}
};
SlotInfo cl_slider_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"slider"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"_shownPositionV\":\n\
			set(\"shownPositionV\", arg[1]);\n\
			objectListSend(\"shownNotify\",\n\
					\"shownPositionV\",\n\
					arg[1]);\n\
		break;\n\
		case \"_shownPositionH\":\n\
			set(\"shownPositionH\", arg[1]);\n\
			objectListSend(\"shownNotify\",\n\
					\"shownPositionH\",\n\
					arg[1]);\n\
		break;\n\
		case \"shownInfoV\":\n\
			seta(\"shownPositionSizeV\", arg[1], arg[2]);\n\
		break;\n\
		case \"shownInfoH\":\n\
			seta(\"shownPositionSizeH\", arg[1], arg[2]);\n\
		break;\n\
		case \"_shownInfoV\":\n\
			seta(\"shownPositionSizeV\", arg[1], arg[2]);\n\
			objectListSend(\"shownNotify\",\n\
					\"shownInfoV\",\n\
					arg[1], arg[2]);\n\
		break;\n\
		case \"_shownInfoH\":\n\
			set(\"shownPositionSizeH\", arg[1], arg[2]);\n\
			objectListSend(\"shownNotify\",\n\
					\"shownInfoH\",\n\
					arg[1], arg[2]);\n\
		break;\n\
		case \"shownPositionV\":\n\
			set(\"shownPositionV\", arg[1]);\n\
		break;\n\
		case \"shownPositionH\":\n\
			set(\"shownPositionH\", arg[1]);\n\
		break;\n\
		case \"shownSizeV\":\n\
			set(\"shownSizeV\", arg[1]);\n\
		break;\n\
		case \"shownSizeH\":\n\
			set(\"shownSizeH\", arg[1]);\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"mouseMove\":\n\
		case \"buttonRelease\":\n\
		case \"keyPress\":\n\
		case \"keyRelease\":\n\
		case \"enter\":\n\
		case \"leave\":\n\
		break;\n\
		case \"expose\":\n\
			render();\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
			config(x(), y(), width(), height());\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"buttonPress\":\n\
			processMouseInput();\n\
/*\n\
			buttonStates = mouseButton();\n\
			if (nthChar(buttonStates, 0) == '1') {\n\
				send(get(\"name\"), \"scroll\", \"down\");\n\
			} else if (nthChar(buttonStates, 2) == '1') {\n\
				send(get(\"name\"), \"scroll\", \"up\");\n\
			} else {\n\
				processMouseInput();\n\
			}\n\
*/\n\
		break;\n\
		case \"scroll\":\n\
			dir = get(\"direction\");\n\
			if (arg[1] == \"down\") {\n\
/*XXX*/			  if (dir == \"left2right\") {\n\
				newPosition = get(\"shownPositionH\") +\n\
						get(\"shownSizeH\");\n\
				if (newPosition > 100) newPosition = 100;\n\
				set(\"shownPositionH\", newPosition);\n\
			  } else {\n\
				newPosition = get(\"shownPositionV\") +\n\
						get(\"shownSizeV\");\n\
				if (newPosition > 100) newPosition = 100;\n\
				set(\"shownPositionV\", newPosition);\n\
			  }\n\
			} else if (arg[1] == \"up\") {\n\
/*XXX*/			  if (dir == \"left2right\") {\n\
				newPosition = get(\"shownPositionH\") -\n\
						get(\"shownSizeH\");\n\
				if (newPosition < 0) newPosition = 0;\n\
				set(\"shownPositionH\", newPosition);\n\
			  } else {\n\
				newPosition = get(\"shownPositionV\") -\n\
						get(\"shownSizeV\");\n\
				if (newPosition < 0) newPosition = 0;\n\
				set(\"shownPositionV\", newPosition);\n\
			  }\n\
			}\n\
			objectListSend(\"shownNotify\", \"shownPositionV\",\n\
					newPosition);\n\
		break;\n\
		case \"key_up\":\n\
			send(self(), \"scroll\", \"up\");\n\
			return;\n\
		break;\n\
		case \"key_down\":\n\
			send(self(), \"scroll\", \"down\");\n\
			return;\n\
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
			print(\"unknown message, clsss = slider: args: \");\n\
			for (i =0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\\n\");\n\
			break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_slider_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_slider
},{
	NULL
}
};

SlotInfo *slots_slider[] = {
	(SlotInfo*)cl_slider_NCSlots,
	(SlotInfo*)cl_slider_NPSlots,
	(SlotInfo*)cl_slider_CSlots,
	(SlotInfo*)cl_slider_PSlots
};

MethodInfo meths_slider[] = {
	/* local methods */
{
	STR_config,
	meth_slider_config
},{
	STR_geta,
	meth_slider_get,
},{
	STR_initialize,
	meth_slider_initialize
},{
	STR_processMouseInput,
	meth_slider_processMouseInput
},{
	STR_render,
	meth_slider_render
},{
	STR_seta,
	meth_slider_set
},{
	NULL
}
};

ClassInfo class_slider = {
	helper_slider_get,
	helper_slider_set,
	slots_slider,		/* class slot information	*/
	meths_slider,		/* class methods		*/
	STR_slider,		/* class identifier number	*/
	&class_field,		/* super class info		*/
};

#define MIN_THUMB_PIXEL_SIZE 5

#define HORIZONTAL_DIR(dirc) (dirc == 'l' || dirc == 'r')
#define THUMB_IN_RANGE(s) (s >= 0 && s <= 100)

int meth_slider_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_field_config(self, result, argc, argv)) return 0;
	return 1;
}

int helper_slider_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_direction:
		result->info.s = SaveString(GET_direction(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_field_get(self, result, argc, argv, labelID);
}
int meth_slider_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_slider_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_slider_initialize(self, result, argc, argv)
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
int helper_slider_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_direction:
		result->info.s = SaveString(argv[1].info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		SET_direction(self, result->info.s);
		meth_slider_render(self, result, argc, argv);
		return 1;

/*	case STR_shownPositionV:
		meth_field_set(self, result, argc, argv);
		meth_slider_render(self, result, argc, argv);
		return 1;
*/
	}
	return helper_field_set(self, result, argc, argv, labelID);
}
int meth_slider_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_slider_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_slider_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	if (!w) {
	        if (!meth_field_render(self, result, argc, argv)) return 0;
		w = GET_window(self);
	} else {
	        if (!meth_field_render(self, result, argc, argv)) return 0;
	}
	if (w && GET_visible(self)) {
		int thumbPosition, thumbSize;
		int thumbPositionPixels, thumbSizePixels;
		int thumbStartPixel, thumbEndPixel;
		int paramExtent, paramD;
		int barThickness;
		char dirc = *GET_direction(self);
		int x1, y1, x2, y2;

		if (HORIZONTAL_DIR(dirc)) {
			thumbSize = GET_shownSizeH(self);
			thumbPosition = GET_shownPositionH(self);
			paramExtent = GET_width(self) - 2;
			paramD = GET_width(self) - 1;
			barThickness = GET_height(self) - 2;
		} else {
			thumbSize = GET_shownSizeV(self);
			thumbPosition = GET_shownPositionV(self);
			paramExtent = GET_height(self) - 2;
			paramD = GET_height(self) - 1;
			barThickness = GET_width(self) - 2;
		}
		thumbSizePixels = 
			((float)thumbSize / 100.0) * (float)(paramExtent);

		if (thumbSizePixels < MIN_THUMB_PIXEL_SIZE)
			thumbSizePixels = MIN_THUMB_PIXEL_SIZE;
	
		thumbPositionPixels = ((float)thumbPosition / 100.0)
			* (float)(paramExtent - thumbSizePixels);

		thumbStartPixel = 1 + thumbPositionPixels;
		thumbEndPixel = thumbStartPixel + thumbSizePixels;
		if (thumbEndPixel > paramD)
			thumbEndPixel = paramD - 2;
		if (thumbStartPixel > thumbEndPixel)
			thumbStartPixel = thumbEndPixel;
			
		GLPrepareObjColor(self);

		switch (dirc) {
		case 't': /* top to bottom */
			GLDrawScrollBarV(w, 
					 GET_height(self), 
					 thumbStartPixel,
					 thumbEndPixel,
					 barThickness);
			break;
		case 'b': /* bottom to top */
			GLDrawScrollBarV(w,
					 GET_height(self),
					 GET_height(self) - thumbEndPixel,
					 GET_height(self) - thumbStartPixel,
 					 barThickness);
			break;
		case 'l': /* left to right */
			GLDrawScrollBarH(w, 
					 GET_width(self),
					 thumbStartPixel,
					 thumbEndPixel,
					 barThickness);
			break;
		case 'r': /* right to left */
			GLDrawScrollBarH(w,
					 GET_width(self),
					 GET_width(self) - thumbEndPixel,
					 GET_width(self) - thumbStartPixel,
					 barThickness);
			break;
		}
		return 1;
	}
	return 0;
}

int meth_slider_processMouseInput(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	char orientation, dirc = *GET_direction(self), *mesg;
	int thumbPos, oldThumbPos = 0;
	int thumbPixels, thumbSize, thumbStart, thumbEnd, mousePos;
	int paramExtent, delta, z;
	int os = -1, s = 0;
	int reverse = 0;
	int rootx, rooty, wx, wy;
	float m, d;
	XEvent e;

	if (GrabSuccess != XGrabPointer(display, w, TRUE, 0,
					GrabModeAsync, GrabModeAsync,
					None, None, CurrentTime))
	    return 1;
	XUngrabPointer(display, CurrentTime);

	clearPacket(result);
	if (!w || !GET_visible(self)) return 0;

	GLQueryMouse(w, &rootx, &rooty, &wx, &wy);
	if (HORIZONTAL_DIR(dirc)) {
		orientation = 'h';
		thumbSize = GET_shownSizeH(self);
		thumbPos = GET_shownPositionH(self);
		paramExtent = GET_width(self) - 2;
		mousePos = wx;
		mesg = "_shownPositionH";
	} else {
		orientation = 'v';
		thumbSize = GET_shownSizeV(self);
		thumbPos = GET_shownPositionV(self);
		paramExtent = GET_height(self) - 2;
		mousePos = wy;
		mesg = "_shownPositionV";
	}
	if (dirc == 'b' || dirc == 'r') reverse = 1;

	thumbPixels = (float)thumbSize / 100.0 * (float)paramExtent;
	if (mousePos < 0 || mousePos > paramExtent) {
/*		printf("mousePos out of bound\n");*/
		return 0;
	}
	d = (float)(paramExtent - thumbPixels);
	thumbStart = (float)thumbPos / 100.0 * d;
	thumbEnd = thumbStart + thumbPixels;
	if (mousePos > thumbStart && mousePos < thumbEnd) {
		delta = mousePos - thumbStart;
	} else {
		delta = 0;
	}
	if (d > 0.0) 
		thumbPos = abs((long)((float)(mousePos - delta) / d * 100.0));
	else 
		thumbPos = 0; 

	if (thumbPos > 100) thumbPos = 100;
	if (reverse) thumbPos = 100 - thumbPos;

	if (thumbPos != oldThumbPos) {
	  sendMessage1N1int(self, mesg, thumbPos);
	}
	for (;;) {
		VObj *obj;
		HashEntry *hentry;

		XNextEvent(display, &e);
		switch (eventType(e)) {
		case ButtonRelease:
			mouseButtonPressedState &=
				~(1<<(((XButtonEvent*)&e)->button));
			return 1;


		case MotionNotify:
			if (HORIZONTAL_DIR(dirc)) {
				GLQueryMouse(w, &rootx, &rooty, &mousePos, &z);
			} else {
				GLQueryMouse(w, &rootx, &rooty, &z, &mousePos);
			}
			m = mousePos - delta;
			if (m < 0.0) m = 0.0;
			if (d > 0.0) thumbPos = m / d * 100.0;
			else thumbPos = 0;
			if (reverse) thumbPos = 100 - thumbPos;
			if (thumbPos > 100) thumbPos = 100;
			if (thumbPos == oldThumbPos) break;
			sendMessage1N1int(self, mesg, thumbPos);
			oldThumbPos = thumbPos;
		break;
		case ButtonPress:
			mouseButtonPressedState |= 
				1<<(((XButtonEvent*)&e)->button);
			return 1;
		default:
			hentry = window2Obj->get(window2Obj, 
						(long)eventWindow(e));
			if (hentry) {
				obj = (VObj*)hentry->val;
				if (obj) VCurrentObj = obj;
			}
			process_event(&e, ACTION_TOOL);
		break;
		}
	}
}







