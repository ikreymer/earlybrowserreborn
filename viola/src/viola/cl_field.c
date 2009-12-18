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
 * class	: field
 * superClass	: generic
 */
#include "utils.h"
#include "sys.h"
#include <ctype.h>
#include "error.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "vlist.h"
#include "attr.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "cl_field.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

SlotInfo cl_field_NCSlots[] = {
	NULL
};
SlotInfo cl_field_NPSlots[] = {
{
	STR_content,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__content,		/* internalized format */
	LONG,
	0
},{
	STR_content2,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__content2,		/* internalized format */
	LONG,
	0
},{
	STR_label,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__label,		/* internalized format */
	LONG,
	0
},{
	STR_border,
	LONG | SLOT_RW,
	BORDER_FIELD
},{
	STR_x,
	LONG | SLOT_RW,
	0
},{
	STR_y,
	LONG | SLOT_RW,
	0
},{
	STR_width,
	LONG | SLOT_RW,
	1
},{
	STR_height,
	LONG | SLOT_RW,
	1
},{
	STR_minWidth,
	LONG | SLOT_RW,
	1
},{
	STR_minHeight,
	LONG | SLOT_RW,
	1
},{
	STR_maxWidth,
	LONG | SLOT_RW,
	9999
},{
	STR_maxHeight,
	LONG | SLOT_RW,
	9999
},{
	STR_gapH,
	LONG | SLOT_RW,
	0
},{
	STR_gapV,
	LONG | SLOT_RW,
	0
},{
	STR_window,		/* probably should hide this (internal) */
	LONG,
 	0
},{
	STR_shownPositionH,
	LONG | SLOT_R,
 	0
},{
	STR_shownPositionV,
	LONG | SLOT_R,
 	0
},{
	STR_shownSizeH,
	LONG | SLOT_R,
 	0
},{
	STR_shownSizeV,
	LONG | SLOT_R,
 	0
},{
	STR_shownDepend,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__shownDepend,
	OBJL,
	NULL
},{
	STR_shownNotify,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__shownNotify,
	OBJL,
	NULL
},{
	STR_lock,
	LONG,
 	0
},{
	STR_visible,
	LONG,
	0
},{
	STR__colorInfo,
	RGBV,
 	NULL
},{
	STR_BDColor,		/* border color */
	PTRS | SLOT_RW,
 	NULL/*(long)"black"*/
},{
	STR_BGColor,		/* background color */
	PTRS | SLOT_RW,
 	NULL/*(long)"black"*/
},{
	STR_CRColor,		/* cursor color */
	PTRS | SLOT_RW,
 	NULL/*(long)"white"*/
},{
	STR_FGColor,		/* foreground color */
	PTRS | SLOT_RW,
 	NULL/*(long)"white"*/
},{
	STR__eventMask,
	LONG,
 	(long)	KeyPressMask | KeyReleaseMask | 
		ButtonPressMask | ButtonReleaseMask |
		EnterWindowMask | LeaveWindowMask | PointerMotionHintMask |
		Button1MotionMask | Button2MotionMask | Button3MotionMask | 
		StructureNotifyMask | ExposureMask
},{
	STR_eventMask,
	PTRS | SLOT_RW,
 	(long)""
},{
	NULL
}
};
SlotInfo cl_field_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"field"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"expose\":\n\
			expose(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
			/* icky! but necessary to give script a chance to \n\
			 * intercept and do something...*/\n\
	        	send(self(), \"config\", \n\
		  get(\"x\"),get(\"y\"),get(\"width\"),get(\"height\"));\n\
		break;\n\
		case \"render\":\n\
			render();\n\
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
		case \"shownInfoV\":\n\
		case \"shownInfoH\":\n\
			return;\n\
		break;\n\
		case \"focus\":\n\
			mousePos = mouse();\n\
			winPos = windowPosition();\n\
			mx = mousePos[0];\n\
			my = mousePos[1];\n\
			dx = ((winPos[0] + width() / 2) - mx) / 10.0;\n\
			dy = ((winPos[1] + height() / 2) - my) / 10.0;\n\
			for (i = 0; i < 10; i++) {\n\
				mx += dx;\n\
				my += dy;\n\
				setMouse(mx, my);\n\
			}\n\
		break;\n\
		case \"key_up\":\n\
			send(parent(), \"key_up\");\n\
			return;\n\
		break;\n\
		case \"key_down\":\n\
			send(parent(), \"key_down\");\n\
			return;\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"freeSelf\":\n\
			return freeSelf();\n\
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
SlotInfo cl_field_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_field
},{
	NULL
}
};

SlotInfo *slots_field[] = {
	(SlotInfo*)cl_field_NCSlots,
	(SlotInfo*)cl_field_NPSlots,
	(SlotInfo*)cl_field_CSlots,
	(SlotInfo*)cl_field_PSlots
};

MethodInfo meths_field[] = {
	/* local methods */
{
	STR_canvalize,
	meth_field_canvalize
},{
	STR_clean,
	meth_field_clean,
},{
	STR_clearArea,
	meth_field_clearArea,
},{
	STR_clearWindow,
	meth_field_clearWindow
},{
	STR_clone,
	meth_field_clone
},{
	STR_clone2,
	meth_field_clone2
},{
	STR_config,
	meth_field_config
},{
	STR_copyArea,
	meth_field_copyArea,
},{
	STR_cycleColors,
	meth_field_cycleColors
},{
	STR_deepObjectListSend,
	meth_field_deepObjectListSend
},{
	STR_drawFillOval,
	meth_field_drawFillOval,
},{
	STR_drawFillRect,
	meth_field_drawFillRect,
},{
	STR_drawLine,
	meth_field_drawLine,
},{
	STR_drawOval,
	meth_field_drawOval,
},{
	STR_drawRect,
	meth_field_drawRect,
},{
	STR_drawText,
	meth_field_drawText,
},{
	STR_eraseFillOval,
	meth_field_eraseFillOval,
},{
	STR_eraseFillRect,
	meth_field_eraseFillRect,
},{
	STR_eraseLine,
	meth_field_eraseLine,
},{
	STR_eraseOval,
	meth_field_eraseOval,
},{
	STR_eraseRect,
	meth_field_eraseRect,
},{
	STR_eraseText,
	meth_field_eraseText,
},{
	STR_expose,
	meth_field_expose,
},{
	STR_eventMask,
	meth_field_eventMask,
},{
	STR_fillArc,
	meth_field_fillArc,
},{
	STR_flush,
	meth_field_flush,
},{
	STR_freeSelf,
	meth_field_freeSelf,
},{
	STR_geta,
	meth_field_get
},{
	STR_iconName,
	meth_field_iconName
},{
	STR_initialize,
	meth_field_initialize
},{
	STR_invertFillOval,
	meth_field_invertFillOval,
},{
	STR_invertFillRect,
	meth_field_invertFillRect,
},{
	STR_invertLine,
	meth_field_invertLine,
},{
	STR_invertOval,
	meth_field_invertOval,
},{
	STR_invertRect,
	meth_field_invertRect,
},{
	STR_invertText,
	meth_field_invertText
},{
	STR_lower,
	meth_field_lower
},{
	STR_mouse,
	meth_field_mouse
},{
	STR_mouseLocal,
	meth_field_mouseLocal
},{
	STR_mouseButton,
	meth_field_mouseButton
},{
	STR_mouseX,
	meth_field_mouseX
},{
	STR_mouseY,
	meth_field_mouseY
},{
	STR_objectListAppend,
	meth_field_objectListAppend
},{
	STR_objectListCount,
	meth_field_objectListCount
},{
	STR_objectListDelete,
	meth_field_objectListDelete
},{
	STR_objectListPrepend,
	meth_field_objectListPrepend
},{
	STR_objectListSend,
	meth_field_objectListSend
},{
	STR_raise,
	meth_field_raise
},{
	STR_randomizeArea,
	meth_field_randomizeArea,
},{
	STR_render,
	meth_field_render
},{
	STR_smudge,
	meth_field_smudge
},{
	STR_seta,
	meth_field_set
},{
	STR_windowName,
	meth_field_windowName
},{
	STR_windowPosition,
	meth_field_windowPosition
},{
	NULL
}
};

ClassInfo class_field = {
	helper_field_get,
	helper_field_set,
	slots_field,		/* class slot information	*/
	meths_field,		/* class methods		*/
	STR_field,		/* class identifier number	*/
	&class_generic,		/* super class info		*/
};

/*
 * canvalize()
 */
int meth_field_canvalize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * clean()
 */
int meth_field_clean(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (GET_window(self) && GET_visible(self)) {
		if (GET__classInfo(self) != &class_glass)
			GLClearWindow(GET_window(self));
		sendMessage1(self, "render");
		return 1;
	}
	return 0;
}

/*
 * clearWindow()
 */
int meth_field_clearWindow(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	if (GET_window(self) && GET_visible(self)) 
		if (GET__classInfo(self) != &class_glass)
			GLClearWindow(GET_window(self));
	return 1;
}

/*
 * clearArea(x0, y0, x1, y1)
 */
int meth_field_clearArea(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;

	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);
	if (GET_window(self)) {
	  	if (GET__classInfo(self) != &class_glass) {
			GLPaintFillRect(GET_window(self), 
					gc_bg, x0, y0, x1, y1);
		}
		return 1;
	}
	return 0;
}

/*
 * clone(clone name suffix)
 * 
 * Make a clone self
 *
 * Result: clone object, and optinally name it
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_clone(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *cloneObj;

	if (!meth_field_clone2(self, result, argc, argv)) return 0;
	cloneObj = result->info.o;
	if (cloneObj) {
		sendInitToChildren(cloneObj);
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = cloneObj;
		return 1;
	}
	result->type = PKT_OBJ;
	result->info.o = NULL;
	result->canFree = 0;
	return 0;
}

int colorsizes_clone = 0;

/*
 * the clone() procedure has made copies of data, here we finish up
 * the job.
 */
int meth_field_clone2(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *suffix;
	VObj *cloneObj;
	VObjList *olist;

	if (!meth_generic_clone2(self, result, argc, argv)) return 0;
	cloneObj = result->info.o;

	/* window is not shareable */
	SET_window(cloneObj, NULL);

	suffix = VsaveString(GET__memoryGroup(cloneObj), PkInfo2Str(&argv[0]));

	/* make the links to associated (and assumed) clone objects */
	olist = GET__shownDepend(cloneObj);
	if (olist && suffix) {
		SET_shownDepend(cloneObj, 
			saveString(OListToStrPlusSuffix(olist, suffix)));
	} else {
		SET_shownDepend(cloneObj, saveString(""));
	}

	olist = GET__shownNotify(cloneObj);
	if (olist && suffix) {
		SET_shownNotify(cloneObj, 
			saveString(OListToStrPlusSuffix(olist, suffix)));
	} else {
		SET_shownNotify(cloneObj, saveString(""));
	}

	if (GET_label(cloneObj))
		SET_label(cloneObj, saveString(GET_label(cloneObj)));
/*
	if (GET_BDColor(self))
		SET_BDColor(self, saveString(GET_BDColor(self)));
	if (GET_BGColor(self))
		SET_BGColor(self, saveString(GET_BGColor(self)));
	if (GET_FGColor(self)) 
		SET_FGColor(self, saveString(GET_FGColor(self)));
	if (GET_CRColor(self))
		SET_CRColor(self, saveString(GET_CRColor(self)));
*/
colorsizes_clone ++;
/*
colorsizes_clone += strlen(GET_BDColor(self));
colorsizes_clone += strlen(GET_BDColor(self));
colorsizes_clone += strlen(GET_FGColor(self));
colorsizes_clone += strlen(GET_CRColor(self));
*/
	if (GET_eventMask(self))
		SET_eventMask(self, saveString(GET_eventMask(self)));

	SET__label(cloneObj, NULL);

	Vfree(GET__memoryGroup(cloneObj), suffix);

	return 1;
}

/*
 * config([x, y, width, height])
 */
int meth_field_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	if (argc == 0) {
		return 1;
	} else if (argc == 2) {
		SET_x(self, PkInfo2Int(&argv[0]));
		SET_y(self, PkInfo2Int(&argv[1]));
		return 1;
	} else if (argc == 4) {
		SET_x(self, PkInfo2Int(&argv[0]));
		SET_y(self, PkInfo2Int(&argv[1]));
		SET_width(self, PkInfo2Int(&argv[2]));
		SET_height(self, PkInfo2Int(&argv[3]));
/*
		int old, change = 0;

		if (old != SET_width(self, PkInfo2Int(&argv[2]))) change = 1;
		old = GET_height(self);
		if (old != SET_height(self, PkInfo2Int(&argv[3]))) change = 1;
		old = GET_width(self);
		if (change) callMeth(self, result, 0, argv, STR_render);
*/
		return 1;
	}
	return 0;
}

/*
 * copyArea(fromx, fromy, width, height, tox, toy)
 */
int meth_field_copyArea(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int fromx, fromy, width, height, tox, toy;

	fromx = PkInfo2Int(&argv[0]);
	fromy = PkInfo2Int(&argv[1]);
	width = PkInfo2Int(&argv[2]);
	height = PkInfo2Int(&argv[3]);
	tox = PkInfo2Int(&argv[4]);
	toy = PkInfo2Int(&argv[5]);

	clearPacket(result);
	if (GET_window(self)) { /*XXX*/
		XCopyArea(display, GET_window(self), GET_window(self), gc_copy,
			fromx, fromy, width, height, tox, toy);
		return 1;
	}
	return 0;
}

/*
 * cycleColors(0|1);
 *
 * Result: unaffected
 * Return: 1
 */
int meth_field_cycleColors(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;

	clearPacket(result);

	if (GET__classInfo(self) != &class_glass) {
		if (PkInfo2Int(&argv[0])) {
			cp = saveString(GLGetFGColor(self));
			GLSetFGColor(self, GLGetBGColor(self));
			GLSetBGColor(self, GLGetBDColor(self));
			GLSetBDColor(self, cp);
		} else {
			cp = saveString(GLGetBDColor(self));
			GLSetBDColor(self, GLGetBGColor(self));
			GLSetBGColor(self, GLGetFGColor(self));
			GLSetFGColor(self, cp);
		}
/*		SET_BDColor(self, GLGetBDColor(self));
		SET_BGColor(self, GLGetBGColor(self));
		SET_FGColor(self, GLGetFGColor(self));
*/
		return 1;
	} else {
		return 0;
	}
}
/*
 * deepObjectListSend(objectListName, arg1, arg2, ..., argn)
 *
 * Recursively send the arguments to each objects in the object list.
 *
 * Result: cleared
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_deepObjectListSend(self, result, argc, argv)
      VObj *self;
      Packet *result;
      int argc;
      Packet argv[];
{
      VObjList *olist = NULL;
      char *listName = PkInfo2Str(argv);

      if (!STRCMP(listName, "shownDepend")) {
              olist = GET__shownDepend(self);
      } else if (!STRCMP(listName, "shownNotify")) {
              olist = GET__shownNotify(self);
      } else {
              return meth_generic_deepObjectListSend(self, 
                                                     result, argc, argv);
      }
      while (olist) {
              if (olist->o) {
                      sendMessagePackets(olist->o, &argv[1], argc - 1);
                      callMeth(olist->o, result, argc, argv, 
                               STR_deepObjectListSend);
              } else {
                      /* error */
              }
              olist = olist->next;
      }
      return 1;
}

/*
 * drawFillOval(x0, y0, x1, y1)
 */
int meth_field_drawFillOval(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLDrawFillOval(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * drawFillRect(x0, y0, x1, y1)
 */
int meth_field_drawFillRect(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLDrawFillRect(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * drawLine(x0, y0, x1, y1)
 */
int meth_field_drawLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLDrawLine(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * drawOval(x0, y0, x1, y1)
 */
int meth_field_drawOval(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLDrawOval(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * fillArc(x0, y0, x1, y1, degree1, degree2)
 */
int meth_field_fillArc(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLPaintFillArc(GET_window(self), gc_fg, 
				       x0, y0, x1, y1,
				       PkInfo2Int(&argv[4]),
				       PkInfo2Int(&argv[5]));
			return 1;
		}
	}
	return 0;
}

/*
 * drawRect(x0, y0, x1, y1)
 */
int meth_field_drawRect(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLDrawRect(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * drawText(x, y, fontID, string)
 */
int meth_field_drawText(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, fontID;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	fontID = PkInfo2Int(&argv[2]);
	
	clearPacket(result);
	if (GET_window(self) && GET_visible(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLDrawText(GET_window(self), fontID, x0, y0, 
					PkInfo2Str(&argv[3]));
			return 1;
		}
	}
	return 0;
}

/*
 * eraseFillOval(x0, y0, x1, y1)
 */
int meth_field_eraseFillOval(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLEraseFillOval(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * eraseFillRect(x0, y0, x1, y1)
 */
int meth_field_eraseFillRect(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLEraseFillRect(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * eraseLine(x0, y0, x1, y1)
 */
int meth_field_eraseLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLEraseLine(GET_window(self), x0, y0, x1, y1); 
			return 1;
		}
	}
	return 0;
}

/*
 * eraseOval(x0, y0, x1, y1)
 */
int meth_field_eraseOval(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLEraseOval(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}


/*
 * eraseRect(x0, y0, x1, y1)
 */
int meth_field_eraseRect(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLEraseRect(GET_window(self), x0, y0, x1, y1);
			return 1;
		}
	}
	return 0;
}

/*
 * eraseText(x, y, fontID, string)
 */
int meth_field_eraseText(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, fontID;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	fontID = PkInfo2Int(&argv[2]);

	clearPacket(result);
	if (GET_window(self) && GET_visible(self)) {
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			GLEraseText(GET_window(self), fontID, x0, y0,
				PkInfo2Str(&argv[3]));
			return 1;
		}
	}
	return 0;
}

/*
 * eventMask(args)
 */
int meth_field_eventMask(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int currentMask = GET__eventMask(self);

	clearPacket(result);
	if (argc == 0) {
		int i = 0;

		buff[0] = '\0';
		while (eventMaskName[i].mask) {
			if (currentMask & eventMaskName[i].mask) {
				strcat(buff, eventMaskName[i].name);
				strcat(buff, " ");
			}
			++i;
		}
		result->info.s = saveString(buff);
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	} else {
		char lbuff[64];
		char *str = PkInfo2Str(&argv[0]);
		int i = 0, j, k;

		do {
			j = i;
			SkipBlanks(str, &i);
			i = NextWord(str, i, lbuff);
			if (AllBlank(lbuff)) break;
			for (k = 0; eventMaskName[k].mask; k++) {
/*				printf("(%s)(%s)\n", 
					&lbuff[1], eventMaskName[k].name);
*/
				if (!STRCMP((&lbuff[1]), 
						(eventMaskName[k].name))) {
					if (*lbuff == '+') 
						currentMask |= 
							eventMaskName[k].mask;
					else
						currentMask &= 
						     ~(eventMaskName[k].mask);
					break;
				}
			}
		} while (i != j);
		SET__eventMask(self, currentMask);
	}
	result->info.i = currentMask;
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

int meth_field_expose(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	if (GET__classInfo(self) != &class_glass) {
		GLPrepareObjColor(self);
	}
	return 1;
}

int meth_field_flush(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	XFlush(display);
}

int meth_field_freeSelf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
/* unsafe to free, b/c cloning now merely copies pointer value, not content */
/*print("freeSelf: self=0x%x, %s\n", self, GET_name(self));*/
	if (!exitingViola) {
		if (GET_content(self))
			Vfree(GET__memoryGroup(self), GET_content(self));
		if (GET_label(self)) free(GET_label(self));
		if (GET_shownNotify(self)) free(GET_shownNotify(self));
		if (GET_shownDepend(self)) free(GET_shownDepend(self));

		if (GET_BDColor(self)) free(GET_BDColor(self));
		if (GET_BGColor(self)) free(GET_BGColor(self));
		if (GET_FGColor(self)) free(GET_FGColor(self));
		if (GET_CRColor(self)) free(GET_CRColor(self));

		if (GET_eventMask(self)) free(GET_eventMask(self));
		if (GET_window(self)) {

/*XXXXXXXXXX for some bloody reason destroying window, running under
the motif shell, will casuse x to get pissed off.

fprintf(stderr, "WDESTROY: %x obj=%s\n", GET_window(self), GET_name(self));
fflush(stderr);
if (validObjectP(self) == 0) {
fprintf(stderr, "WWW Destroying invalid obj: %x obj=%s\n", GET_window(self), GET_name(self));
fflush(stderr);
}
			GLDestroyWindow(GET_window(self));
*/

			SET_window(self, 0);/*insurance*/
			window2Obj->remove(window2Obj, GET_window(self));
		}
	}
	meth_generic_freeSelf(self, result, argc, argv);
	return 1;
}

int helper_field_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_BDColor: {
		ColorInfo *ci = GET__colorInfo(self);
		if (ci && ci->fg && ci->bd->name) {
		  result->info.s = SaveString(ci->bd->name);
		  result->type = PKT_STR;
		  result->canFree = PK_CANFREE_STR;
		}
		return 1;
	}
	case STR_BGColor: {
		ColorInfo *ci = GET__colorInfo(self);
		if (ci && ci->fg && ci->bg->name) {
		  result->info.s = SaveString(ci->bg->name);
		  result->type = PKT_STR;
		  result->canFree = PK_CANFREE_STR;
		}
		return 1;
	}
	case STR_FGColor: {
		ColorInfo *ci = GET__colorInfo(self);
		if (ci && ci->fg && ci->fg->name) {
		  result->info.s = SaveString(ci->fg->name);
		  result->type = PKT_STR;
		  result->canFree = PK_CANFREE_STR;
		}
		return 1;
	}
	case STR_CRColor: {
		ColorInfo *ci = GET__colorInfo(self);
		if (ci && ci->fg && ci->cr->name) {
		  result->info.s = SaveString(ci->cr->name);
		  result->type = PKT_STR;
		  result->canFree = PK_CANFREE_STR;
		}
		return 1;
	}
	case STR_content:
		result->info.s = SaveString(GET_content(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
/*
		result->info.s = GET_content(self);
		result->type = PKT_STR;
		result->canFree = 0;
*/
		return 1;
	case STR_content2:
		result->info.s = SaveString(GET_content2(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
/*
		result->info.s = GET_content2(self);
		result->type = PKT_STR;
		result->canFree = 0;
*/
		return 1;
	case STR_label:
		result->info.s = SaveString(GET_label(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
/*
		result->info.s = GET_label(self);
		result->type = PKT_STR;
		result->canFree = 0;
*/
		return 1;
	case STR_x:
		result->info.i = GET_x(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_y:
		result->info.i = GET_y(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_width:
		result->info.i = GET_width(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_height:
		result->info.i = GET_height(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_minWidth:
		result->info.i = GET_minWidth(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_minHeight:
		result->info.i = GET_minHeight(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_maxWidth:
		result->info.i = GET_maxWidth(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_maxHeight:
		result->info.i = GET_maxHeight(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_gapH:
		result->info.i = GET_gapH(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_gapV:
		result->info.i = GET_gapV(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_lock:
		result->info.i = GET_lock(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_visible:
		result->info.i = GET_visible(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_window:
		result->info.i = GET_window(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_shownPositionH:
		result->info.i = GET_shownPositionH(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_shownPositionV:
		result->info.i = GET_shownPositionV(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_shownSizeH:
		result->info.i = GET_shownSizeH(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_shownSizeV:
		result->info.i = GET_shownSizeV(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_shownDepend:
		result->info.s = SaveString(GET_shownDepend(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_shownNotify:
		result->info.s = SaveString(GET_shownNotify(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_generic_get(self, result, argc, argv, labelID);
}
int meth_field_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_field_get(self, result, argc, argv,
				getIdent(PkInfo2Str(argv)));
}


/*
 * 
 * 
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_iconName(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp = PkInfo2Str(argv);
	Window w = GET_window(self);

	result->type = PKT_STR;
	if (cp && w) {
		GLSetIconName(GET_window(self), cp);
		result->info.s = cp;
		result->canFree = 0;
		return 1;
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

int colorsizes = 0;

int meth_field_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;
	ClassInfo *cip;

	meth_generic_initialize(self, result, argc, argv);

	cp = GET_shownDepend(self);
	if (cp) if (*cp) SET__shownDepend(self, strOListToOList(cp));

	cp = GET_shownNotify(self);
	if (cp) if (*cp) SET__shownNotify(self, strOListToOList(cp));

	if (GET__colorInfo(self) == NULL)
		SET__colorInfo(self, GLInitColorInfo());

	/* In the new regime, the ??Color slot is used only to transfer
	 * info into _colorInfo.
	 */
	cip = GET__classInfo(self);
	if (cip == &class_slider) {
		extern int DefaultBGPixel_darkest;
		GLSetBGColorPixel(self, DefaultBGPixel_darkest);
	} else if (cip == &class_txtEdit) {
		extern int DefaultBGPixel_darker;
		GLSetBGColorPixel(self, DefaultBGPixel_darker);
	} else {
		cp = GET_BGColor(self);
		if (cp) {
			GLSetBGColor(self, cp);
			free(cp);
			SET_BGColor(self, NULL);
		}
	}
	cp = GET_FGColor(self);
	if (cp) {
		GLSetFGColor(self, cp);
		free(cp);
		SET_FGColor(self, NULL);
	}
	cp = GET_CRColor(self);
	if (cp) {
		GLSetCRColor(self, cp);
		free(cp);
		SET_CRColor(self, NULL);
	}
	cp = GET_BDColor(self);
	if (cp) {
		GLSetBDColor(self, cp);
		free(cp);
		SET_BDColor(self, NULL);
	}

	return 1;
}

/*
 * invertFillOval(x0, y0, x1, y1)
 */
int meth_field_invertFillOval(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		GLPrepareObjColor(self);
		GLInvertFillOval(GET_window(self), x0, y0, x1, y1);
		return 1;
	}
	return 0;
}

/*
 * invertFillRect(x0, y0, x1, y1)
 */
int meth_field_invertFillRect(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		GLPrepareObjColor(self);
		GLInvertFillRect(GET_window(self), x0, y0, x1, y1);
		return 1;
	}
	return 0;
}

/*
 * invertLine(x0, y0, x1, y1)
 */
int meth_field_invertLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		GLPrepareObjColor(self);
		GLInvertLine(GET_window(self), x0, y0, x1, y1);
		return 1;
	}
	return 0;
}

/*
 * invertOval(x0, y0, x1, y1)
 */
int meth_field_invertOval(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		GLPrepareObjColor(self);
		GLInvertOval(GET_window(self), x0, y0, x1, y1);
		return 1;
	}
	return 0;
}

/*
 * invertRect(x0, y0, x1, y1, mode)
 */
int meth_field_invertRect(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, x1, y1;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	x1 = PkInfo2Int(&argv[2]);
	y1 = PkInfo2Int(&argv[3]);

	clearPacket(result);
	if (GET_window(self)) {
		GLPrepareObjColor(self);
		GLInvertRect(GET_window(self), x0, y0, x1, y1);
		return 1;
	}
	return 0;
}

/*
 * invertText(x, y, fontID, string)
 */
int meth_field_invertText(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int x0, y0, fontID;
	x0 = PkInfo2Int(&argv[0]);
	y0 = PkInfo2Int(&argv[1]);
	fontID = PkInfo2Int(&argv[2]);
	
	clearPacket(result);
	if (GET_window(self)) {
		GLPrepareObjColor(self);
		GLInvertText(GET_window(self), fontID, x0, y0, 
				PkInfo2Str(&argv[3]));
		return 1;
	}
	return 0;
}

/*
 * lower()
 *
 * Move object to head of list, and visually move window to bottom of 
 * window stack.
 *
 * Result/Return: 1 if successful, 0 if error occured
 */
int meth_field_lower(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *parent = GET__parent(self);

	result->type = PKT_INT;
	result->canFree = 0;
	if (parent) {
		VObjList *olist = GET__children(parent);
		if (olist) {
/*XXXX
			olist = removeVObjListNode(olist, self);
			olist = prependVObjListNode(&olist, self);
*/			  
			SET__children(parent, olist);
			if (GET_window(self)) {
				GLLowerWindow(GET_window(self));
				result->info.i = 1;
				return 1;
			} else {
				fprintf(stderr,
					"obj=%s: lower() failed, no window.\n",
					GET_name(self));
				result->info.i = 0;
				return 0;
			}
		}
	} else {
		if (GET_window(self)) {
			GLLowerWindow(GET_window(self));
			result->info.i = 1;
			return 1;
		} else {
			fprintf(stderr,
				"obj=%s: lower() failed, no window.\n",
				GET_name(self));
			result->info.i = 0;
			return 0;
		}
	}
	result->info.i = 0;
	return 0;
}

int meth_field_mouse(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int rootx, rooty, wx, wy;
	Packet *packet0 = makePacket();
	Packet *packet1 = makePacket();
	Attr *attrp;

	GLQueryMouse(rootWindow, &rootx, &rooty, &wx, &wy);

	packet0->info.i = rootx;
	packet0->type = PKT_INT;
	packet0->canFree = 0;
	packet1->info.i = rooty;
	packet1->type = PKT_INT;
	packet1->canFree = 0;

	result->type = PKT_ATR;
	result->canFree = 0;
	result->info.a = attrp = makeAttr(0, packet0);

	attrp->next = makeAttr(1, packet1);
	return 1;
}

int meth_field_mouseLocal(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int rootx, rooty, wx, wy;
	Packet *packet0 = makePacket();
	Packet *packet1 = makePacket();
	Attr *attrp;

	GLQueryMouse(GET_window(self), &rootx, &rooty, &wx, &wy);

	packet0->info.i = wx;
	packet0->type = PKT_INT;
	packet0->canFree = 0;
	packet1->info.i = wy;
	packet1->type = PKT_INT;
	packet1->canFree = 0;

	result->type = PKT_ATR;
	result->info.a = attrp = makeAttr(0, packet0);
	result->canFree = 0;

	attrp->next = makeAttr(1, packet1);
	return 1;
}

/* returns the mouse buttons last pressed */
int meth_field_mouseButton(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *state = saveString("00000");
	if (mouseButtonPressedState & MOUSE_BUTTON_1) state[0] = '1';
	if (mouseButtonPressedState & MOUSE_BUTTON_2) state[1] = '1';
	if (mouseButtonPressedState & MOUSE_BUTTON_3) state[2] = '1';
	if (mouseButtonPressedState & MOUSE_BUTTON_4) state[3] = '1';
	if (mouseButtonPressedState & MOUSE_BUTTON_5) state[4] = '1';
	result->type = PKT_STR;
	result->info.s = state;
	result->canFree = PK_CANFREE_STR;
	return 1;
}

int meth_field_mouseX(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int rootx, rooty, wx, wy;

	GLQueryMouse(GET_window(self), &rootx, &rooty, &wx, &wy);
	result->type = PKT_INT;
	result->info.i = wx;
	result->canFree = 0;
	return 1;
}

int meth_field_mouseY(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int rootx, rooty, wx, wy;

	GLQueryMouse(GET_window(self), &rootx, &rooty, &wx, &wy);
	result->type = PKT_INT;
	result->info.i = wy;
	result->canFree = 0;
	return 1;
}

/*
 * objectListAppend(<objList>, <obj>)
 *
 * Append an object to a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_objectListAppend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *listName;
	VObjList *olist = NULL;

	result->type = PKT_OBJ;
	result->canFree = 0;
	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "shownDepend")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__shownDepend(self);
		if (obj) {
			SET__shownDepend(self, appendObjToList(olist, obj));
			result->info.o = obj;
			return 1;
		}
	} else if (!STRCMP(listName, "shownNotify")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__shownNotify(self);
		if (obj) {
			SET__shownNotify(self, appendObjToList(olist, obj));
			result->info.o = obj;
			return 1;
		}
	} else {
		return meth_generic_objectListAppend(self, result, argc, argv);
	}
	result->info.o = NULL;
	return 0;
}

/*
 * objectListCount(<objList>)
 *
 * Count the number of objects in a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_objectListCount(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;
	char *listName;
	VObjList *olist = NULL;

	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "shownDepend")) {
		olist = GET__shownDepend(self);
	} else if (!STRCMP(listName, "shownNotify")) {
		olist = GET__shownNotify(self);
	} else {
		return meth_generic_objectListCount(self, result, argc, argv);
	}
	result->type = PKT_INT;
	result->canFree = 0;
	if (olist) {
		for (i = 0; olist; olist = olist->next, i++);
		result->info.i = i;
		return 1;
	}
	result->info.i = -1;
	return 0;
}

/*
 * objectListDelete(<objList>)
 *
 * Delte an object from a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_objectListDelete(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = NULL;
	char *listName = PkInfo2Str(argv);

	clearPacket(result);
	if (!STRCMP(listName, "shownDepend")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__shownDepend(self);
		if (olist && obj) {
			olist = removeVObjListNode(olist, obj);
			SET__shownDepend(self, olist);
			return 1;
		}
	} else if (!STRCMP(listName, "shownNotify")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__shownNotify(self);
		if (olist && obj) {
			olist = removeVObjListNode(olist, obj);
			SET__shownNotify(self, olist);
			return 1;
		}
	} else {
		return meth_generic_objectListDelete(self, result, argc, argv);
	}
	return 0;
}

/*
 * objectListPrepend(<objList>)
 *
 * Append an object to a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_objectListPrepend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *listName;
	VObjList *olist = NULL;

	result->type = PKT_OBJ;
	result->canFree = 0;
	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "shownDepend")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__shownDepend(self);
		if (obj) {
			SET__shownDepend(self, 
					prependVObjListNode(olist, obj));
			result->info.o = obj;
			return 1;
		}
	} else if (!STRCMP(listName, "shownNotify")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__shownNotify(self);
		if (obj) {
			SET__shownNotify(self, 
					prependVObjListNode(olist, obj));
			result->info.o = obj;
			return 1;
		}
	} else {
		return meth_generic_objectListPrepend(self, result, argc, argv);
	}
	result->info.o = NULL;
	return 0;
}

/*
 * objectListSend(<objectListName>, arg1, arg2, ..., argn)
 * send the arguments to each objects in the object list.
 */
int meth_field_objectListSend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = NULL;
	char *listName = PkInfo2Str(argv);

	if (!STRCMP(listName, "shownDepend")) {
		olist = GET__shownDepend(self);
	} else if (!STRCMP(listName, "shownNotify")) {
		olist = GET__shownNotify(self);
	} else {
		return meth_generic_objectListSend(self, result, argc, argv);
	}
	while (olist) {
		if (olist->o) {
			sendMessagePackets(olist->o, &argv[1], argc - 1);
		} else {
			/* error */
		}
		olist = olist->next;
	}
	clearPacket(result);
	return 1;
}

/*
 * raise()
 *
 * Move object to end of list, and visually move window to top of window stack.
 *
 * Result/Return: 1 if successful, 0 if error occured
 */
int meth_field_raise(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *parent = GET__parent(self);

	result->type = PKT_INT;
	result->canFree = 0;
	if (parent) {
		VObjList *olist = GET__children(parent);
		if (olist) {
			olist = removeVObjListNode(olist, self);
			olist = appendVObjListNode(olist, self);
			SET__children(parent, olist);
			if (GET_window(self)) {
					GLRaiseWindow(GET_window(self));
				result->info.i = 1;
				return 1;
			} else {
				fprintf(stderr,
					"obj=%s: raise() failed, no window.\n",
					GET_name(self));
				result->info.i = 0;
				return 0;
			}
		}
	} else {
		if (GET_window(self)) {
			GLRaiseWindow(GET_window(self));
			result->info.i = 1;
			return 1;
		} else {
			fprintf(stderr,
				"obj=%s: raise() failed, no window.\n",
				GET_name(self));
			result->info.i = 0;
			return 0;
		}
	}
	result->info.i = 0;
	return 0;
}

/*
 * randomizeArea(fromx, fromy, width, height, iteration, factor)
 */
int meth_field_randomizeArea(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int fromx, fromy, width, height, iteration, factor;
	int x0, y0, x1, y1, xx, yy;

	fromx = PkInfo2Int(&argv[0]);
	fromy = PkInfo2Int(&argv[1]);
	width = PkInfo2Int(&argv[2]);
	height = PkInfo2Int(&argv[3]);
	iteration = PkInfo2Int(&argv[4]);
	factor = PkInfo2Int(&argv[5]);

	if (GET_window(self)) { /*XXX*/
/*		while (iteration-- > 0) {
			x0 = rand() * width;
			y0 = rand() * height;
			x1 = rand() * width;
			y1 = rand() * height;
			xx = rand() * width;
			yy = rand() * height;
			XCopyArea(display, GET_window(self), gc_copy, 
				x0, y0, xx, yy, x1, y1);
		}
*/
		return 1;
	}
	return 0;
}

int meth_field_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	int isGlassP = 0;

	result->type = PKT_INT;
	result->canFree = 0;
	if (!GET_visible(self)) {
		result->info.i = 0;
		return 0;
	}
  	if (GET__classInfo(self) == &class_glass) isGlassP = 1;

	if (!isGlassP) GLPrepareObjColor(self);

#ifdef BADIDEA
	if (!w && GET_visible(self)) {
		if (!(w = GLOpenWindow(self, GET_x(self), GET_y(self),
			 GET_width(self), GET_height(self), isGlassP))) {
			result->info.i = 0;
			return 0;
		}
		if (w) {
			VObjList *olist = GET__children(self);
/*			SET_visible(self, 1);*/

			for (; olist; olist = olist->next) {
				if (olist->o)
					if (GET_visible(olist->o)) {
/*printf("meth_field_render: children: w=%x obj=%s\n", w, GET_name(olist->o));
*/
						callMeth(olist->o, result, 
						       argc, argv, STR_render);
					}
			}
		}
	}
#endif
	if (!isGlassP && w && GET_visible(self)) {
/*
printf("meth_field_render: clearWindow() w=%x, obj=%s visible=%d\n", w, GET_name(self), GET_visible(self));
*/
		GLClearWindow(w);
/*
printf("meth_field_render: done w=%x\n", w);
*/
		GLDrawBorder(w, 0, 0, 
			GET_width(self)-1, GET_height(self)-1,
			GET_border(self), 1);
	}
	result->info.i = 1;
	return 1;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_field_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	char *cp;

	switch (labelID) {
	case STR_x:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_x(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		if (GET_window(self)) 
			GLUpdatePosition(!GET__parent(self), GET_window(self), 
				GET_x(self), GET_y(self));
		return 1;

	case STR_y:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_y(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		if (GET_window(self))
			GLUpdatePosition(!GET__parent(self), GET_window(self), 
				GET_x(self), GET_y(self));
		return 1;

	case STR_width:
		result->info.i = PkInfo2Int(&argv[1]);
		if (result->info.i <= 0) result->info.i = 1;
		SET_width(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;

		if (GET_window(self)) {
/*
			GLUpdateGeometry(!GET__parent(self), GET_window(self), 
				GET_x(self), GET_y(self),
				GET_width(self), GET_height(self));
*/
			if (GET_width(self) > 0 && GET_height(self) > 0) {
				XSelectInput(display, GET_window(self), 
						GET__eventMask(self) 
						& ~StructureNotifyMask);
				XResizeWindow(display, GET_window(self), 
					GET_width(self), GET_height(self));
				XSelectInput(display, GET_window(self), 
					GET__eventMask(self));
			}
		}
		return 1;

	case STR_height:
		result->info.i = PkInfo2Int(&argv[1]);
		if (result->info.i <= 0) result->info.i = 1;
		SET_height(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		if (GET_window(self)) {
			/*
			GLUpdateGeometry(!GET__parent(self), 0,
				GET_window(self), 
				GET_x(self), GET_y(self),
				GET_width(self), GET_height(self));
			*/
			if (GET_width(self) > 0 && GET_height(self) > 0) {
				XSelectInput(display, GET_window(self), 
						GET__eventMask(self) 
						& ~StructureNotifyMask);
				XResizeWindow(display, GET_window(self), 
					GET_width(self), GET_height(self));
				XSelectInput(display, GET_window(self), 
					GET__eventMask(self));
			}
	 	}
		return 1;

/*
	case STR_BGColor:
		if (GET_BGColor(self)) free(GET_BGColor(self));
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_BGColor(self, result->info.s);
		GLSetBGColor(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;
*/
	case STR_BDColor:
		GLSetBDColor(self, PkInfo2Str(&argv[1]));
		return 1;

	case STR_BGColor:
		GLSetBGColor(self, PkInfo2Str(&argv[1]));
		return 1;

	case STR_FGColor:
		GLSetFGColor(self, PkInfo2Str(&argv[1]));
		return 1;

	case STR_CRColor:
		GLSetCRColor(self, PkInfo2Str(&argv[1]));
		return 1;

	case STR_content: {
		if (GET_content(self)) free(GET_content(self));
		result->info.s = VSaveString(GET__memoryGroup(self), 
						PkInfo2Str(&argv[1]));
		SET_content(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;
	      }
	case STR_content2:
		if (GET_content2(self)) Vfree(GET__memoryGroup(self), 
						GET_content2(self));
		result->info.s = VSaveString(GET__memoryGroup(self), 
						PkInfo2Str(&argv[1]));
		SET_content2(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;

	case STR_label:
		if (GET_label(self)) free(GET_label(self));
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_label(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;

	case STR_lock:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_lock(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_border:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_border(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_minWidth:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_minWidth(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_minHeight:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_minHeight(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_maxWidth:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_maxWidth(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_maxHeight:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_maxHeight(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_gapH:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_gapH(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_gapV:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_gapV(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_visible: {
		VObjList *olist;
		Window w = GET_window(self);
/*
printf("VISIBLE=%d, w=%x obj=%s\n",
	PkInfo2Int(&argv[1]), GET_window(self), GET_name(self));
*/
		result->type = PKT_INT;
		result->canFree = 0;
		if (PkInfo2Int(&argv[1])) {

			/* make self open window, and same for all 
			 * children objects */
			if (!w) {
				GLPrepareObjColor(self);
				w = GLOpenWindow(self, 
						GET_x(self),
						GET_y(self),
						GET_width(self),
						GET_height(self),
						(GET__classInfo(self)
						 == &class_glass ? 1 : 0));
				if (!w) {
					result->info.i = 0;
					SET_window(self, 0);
					SET_visible(self, 0);
					return 0;
				}
				SET_window(self, w);
			}
			olist = GET__children(self);
			for (; olist; olist = olist->next)
				if (olist->o && validObjectP(olist->o)) {
				  int (*func)() = 
					GET__classInfo(olist->o)->slotSetMeth;
				  if (func) {
					((long (*)())(func))
						(olist->o, result, 2, argv,
							STR_visible);
				  }
				}
			SET_visible(self, 1);
			result->info.i = 1;
		} else {
			olist = GET__children(self);
			for (; olist; olist = olist->next)
				if (olist->o && validObjectP(olist->o)) {
				  int (*func)() = 
					GET__classInfo(olist->o)->slotSetMeth;
				  if (func) {
					((long (*)())(func))
						(olist->o, result, 2, argv,
							STR_visible);
				  }
				}
			if (w) {
				GLCloseWindow(w);
			}
			SET_window(self, 0);
			SET_visible(self, 0);
			result->info.i = 0;
		}
		return 1;
	}

	case STR_window: {
		int w = PkInfo2Int(&argv[1]);

		SET_window(self, w);
		return 1;
	}

	case STR_shownPositionH: {
		int pos = PkInfo2Int(&argv[1]);

		if (pos < 0) pos = 0;
		else if (pos > 100) pos = 100;

		if (GET_shownPositionH(self) != pos) {
			SET_shownPositionH(self, pos);
			if (GET_window(self))
				callMeth(self, result, 1, argv, STR_render);
		}
		return 1;
	}

	case STR_shownPositionSizeH: {
		int pos = PkInfo2Int(&argv[1]);
		int siz = PkInfo2Int(&argv[2]);

		if (pos < 0) pos = 0;
		else if (pos > 100) pos = 100;
		if (siz < 0) siz = 0;
		else if (siz > 100) siz = 100;

		if (GET_shownPositionH(self) != pos ||
		    GET_shownSizeH(self) != siz) {
			SET_shownPositionH(self, pos);
			SET_shownSizeH(self, siz);
			if (GET_window(self))
				callMeth(self, result, 1, argv, STR_render);
		}
		return 1;
	}

	case STR_shownPositionSizeV: {
		int pos = PkInfo2Int(&argv[1]);
		int siz = PkInfo2Int(&argv[2]);

		if (pos < 0) pos = 0;
		else if (pos > 100) pos = 100;
		if (siz < 0) siz = 0;
		else if (siz > 100) siz = 100;

		if (GET_shownPositionV(self) != pos ||
		    GET_shownSizeV(self) != siz) {
			SET_shownPositionV(self, pos);
			SET_shownSizeV(self, siz);
			if (GET_window(self))
				callMeth(self, result, 1, argv, STR_render);
		}
		return 1;
	}

	case STR_shownPositionV: {
		int pos = PkInfo2Int(&argv[1]);

		if (pos < 0) pos = 0;
		else if (pos > 100) pos = 100;

		if (GET_shownPositionV(self) != pos) {	
			SET_shownPositionV(self, pos);
			if (GET_window(self))
				callMeth(self, result, 1, argv, STR_render);
		}
		return 1;
	}

	case STR_shownSizeH: {
		int siz = PkInfo2Int(&argv[1]);

		if (siz < 0) siz = 0;
		else if (siz > 100) siz = 100;

		if (GET_shownSizeH(self) != siz) {
			SET_shownSizeH(self, siz);
			if (GET_window(self))
				callMeth(self, result, 1, argv, STR_render);
		}
		return 1;
	}

	case STR_shownSizeV: {
		int siz = PkInfo2Int(&argv[1]);

		if (siz < 0) siz = 0;
		else if (siz > 100) siz = 100;

		if (GET_shownSizeV(self) != siz) {
			SET_shownSizeV(self, siz);
			if (GET_window(self))
				callMeth(self, result, 1, argv, STR_render);
		}
		return 1;
	}
	}
	return helper_generic_set(self, result, argc, argv, labelID);
}
int meth_field_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_field_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

/*
 * smudge();
 */
int meth_field_smudge(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern Pixmap thumbXPM;
	extern int thumbXPM_width;
	extern int thumbXPM_height;

	if (GET_window(self) && GET_visible(self)) {
		int rootx, rooty, wx, wy;
		GLQueryMouse(GET_window(self), &rootx, &rooty, &wx, &wy);
		if (GET__classInfo(self) != &class_glass) {
			GLPrepareObjColor(self);
			if (!thumbXPM) return 0;
			XCopyArea(display, thumbXPM, GET_window(self), gc_or, 
					0, 0, thumbXPM_width, thumbXPM_height, 
					wx - thumbXPM_width / 2, 
					wy - thumbXPM_height / 2);
			return 1;
		}
	}
	return 0;
}

/*
 * 
 * 
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_windowName(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp = PkInfo2Str(argv);
	Window w = GET_window(self);

	result->canFree = 0;
	result->type = PKT_STR;
	if (cp && w) {
		GLSetWindowName(GET_window(self), cp);
		result->info.s = cp;
		return 1;
	}
	result->info.s = "";
	return 0;
}

/*
 * windowPosition()
 *
 * rootWindow frame of reference
 *
 * Result: 
 * Return: 1 if successful, 0 if error occured
 */
int meth_field_windowPosition(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);
	int x, y;
	Packet *packet0 = makePacket();
	Packet *packet1 = makePacket();
	Attr *attrp;

	if (w) {
		GLRootPosition(w, &x, &y);
		packet0->info.i = x;
		packet0->type = PKT_INT;
		packet1->info.i = y;
		packet1->type = PKT_INT;

		result->type = PKT_ATR;
		result->info.a = attrp = makeAttr(0, packet0);
		attrp->next = makeAttr(1, packet1);
		return 1;
	}
	result->canFree = 0;
	result->type = PKT_INT;
	result->info.i = 0;
	return 0;
}
