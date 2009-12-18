/*
 * Copyright 1990-1992 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * class	: menu
 * superClass	: txt
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
#include "cl_menu.h"
#include "misc.h"
#include "glib.h"
#include "event.h"
#include "attr.h"
#include "cexec.h"

#include "../libXPA/src/xpa.h"

#define MENU_ITEM_TYPE_NONE 0
#define MENU_ITEM_TYPE_ENTRY 1
#define MENU_ITEM_TYPE_SEPARATOR 2

SlotInfo cl_menu_NCSlots[] = {
	NULL
};
SlotInfo cl_menu_NPSlots[] = {
{
	STR_menuConfig,		/* ASCII description of the menu */
 	PTRS | SLOT_RW,
	(long)".{}"
},{
	STR__menu,		/* internal */
 	PTRV,
	NULL
},{
	STR__menuEntries,	/* internal */
 	PTRV,
	NULL
},{
	NULL
}
};
SlotInfo cl_menu_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"menu"
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
		case \"buttonRelease\":\n\
		break;\n\
		case \"buttonPress\":\n\
			drawRect(0,0,width()-1,height()-1);/*smudge();*/\n\
			processMouseInput();\n\
			render();/*clean();*/\n\
		break;\n\
		case \"expose\":\n\
			render();\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
			render();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"configSelf\":\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"freeSelf\":\n\
			return freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = menu: args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_menu_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_menu
},{
	STR_border,
	LONG | SLOT_RW,
	BORDER_MENU
},{
	NULL
}
};

SlotInfo *slots_menu[] = {
	(SlotInfo*)cl_menu_NCSlots,
	(SlotInfo*)cl_menu_NPSlots,
	(SlotInfo*)cl_menu_CSlots,
	(SlotInfo*)cl_menu_PSlots
};

MethodInfo meths_menu[] = {
	/* local methods */
{
	STR_config,
	meth_menu_config
},{
	STR_freeSelf,
	meth_menu_freeSelf
},{
	STR_geta,
	meth_menu_get,
},{
	STR_initialize,
	meth_menu_initialize
},{
	STR_processMouseInput,
	meth_menu_processMouseInput
},{
	STR_render,
	meth_menu_render
},{
	STR_seta,
	meth_menu_set
},{
	NULL
}
};

ClassInfo class_menu = {
	helper_menu_get,
	helper_menu_set,
	slots_menu,		/* class slot information	*/
	meths_menu,		/* class methods		*/
	STR_menu,		/* class identifier number	*/
	&class_txt,	/* super class info		*/
};

#define MIN_THUMB_PIXEL_SIZE 5

#define HORIZONTAL_DIR(dirc) (dirc == 'l' || dirc == 'r')
#define THUMB_IN_RANGE(s) (s >= 0 && s <= 100)

int parse_menu_entryScan();
int parse_menu_entryLine();
xpa_entry *parse_menu_allocateEntryTree();
xpa_entrys parse_menu_configuration();
static unsigned long res_color();
int funny_evt();
xpa_menu make_menu();
char *get_menu_message();
int set_menuConfig();


int meth_menu_config(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_txt_config(self, result, argc, argv);
}

/*
 * freeSelf()
 *
 * Free object attributes and.
 * 
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_menu_freeSelf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	free(GET_menuConfig(self));

	/*XXX NEED TO deep destroy menu structure */
	if (GET__menu(self)) free(GET__menu(self));
	if (GET__menuEntries(self)) free(GET__menuEntries(self));

	meth_txt_freeSelf(self, result, argc, argv);
	return 1;
}

int helper_menu_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_menuConfig:
		result->info.s = GET_menuConfig(self);
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	return helper_txt_get(self, result, argc, argv, labelID);
}
int meth_menu_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_menu_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_menu_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!meth_txt_initialize(self, result, argc, argv)) return 0;
	return set_menuConfig(self);
}


int meth_menu_processMouseInput(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int *menu_result, menu_resultn;
	int root_x, root_y;
	char *data;
	xpa_menu menup = (xpa_menu)GET__menu(self);
	xpa_entrys menu_entries = (xpa_entrys)GET__menuEntries(self);

	result->type = PKT_INT;
	result->canFree = 0;

	if (!(menup && menu_entries)) {
		printf(" bark. object not initialized properly or at all.\n");
		return result->info.i = 0; /* indicates error */
	}
	GLRootPosition(GET_window(self), &root_x, &root_y);
	menu_resultn = xpa_moded(menup, 
			 root_x, root_y + GET_height(self) + 10,
			 0, (int*)0, 0, funny_evt, &menu_result);
	if (menu_resultn < 0) {
		return 0;
	} else {
		data = get_menu_message(menu_entries,
					menu_result, menu_resultn);
		if (data) {
			char *cp = SaveString(data);
			execScript(self, result, cp);
			free(cp);
		}
		return 1;
	}
}

int meth_menu_render(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Window w = GET_window(self);

	if (!meth_field_render(self, result, argc, argv)) return 0;

	if (w = GET_window(self)) {
		char *str = GET_label(self);
		int fontID = GET__font(self);
		int x, y;

		if (!str) str = "Unlabeled";
		x = (GET_width(self) - GLTextWidth(fontID, str) - 
			menuPixmap_width) / 2;
		y = (GET_height(self) - GLTextHeight(fontID, str)) / 2;
/*		GLDrawText(w, fontID, x + menuPixmap_width, y, str);
*/
		GLDrawText(w, fontID, 6 + menuPixmap_width, y, str);
		GLDisplayXBM(w, 3, 3, menuPixmap_width, menuPixmap_height, 
			     menuPixmap); 
		return 1;
	}
	return 0;
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 */
int helper_menu_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_menuConfig:
		result->type = PKT_INT;
		result->canFree = 0;

		/*XXX NEED TO deep destroy menu structure */

		if (GET_menuConfig(self)) free(GET_menuConfig(self));
		SET_menuConfig(self, saveString(PkInfo2Str(&argv[1])));
		result->info.i = set_menuConfig(self);
		return result->info.i;
	}
	return helper_txt_set(self, result, argc, argv, labelID);
}
int meth_menu_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_menu_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

/****************************************************************************/

int set_menuConfig(self)
	VObj *self;
{
	char *menuSpec = GET_menuConfig(self);
	xpa_menu menu = (xpa_menu)GET__menu(self);
	xpa_entrys menu_entries = (xpa_entrys)GET__menuEntries(self);

/*
	SET_menuConfig(self, menuSpec);
*/
	if (menu) {
/*		xpa_destroy(menu);*/
		menu_entries = NULL;
	}
	menu = make_menu(menuSpec, &menu_entries, self);

	if (menu && menu_entries) {
		SET__menu(self, menu);
		SET__menuEntries(self, menu_entries);
		return 1;
	}
	SET__menu(self, NULL);
	SET__menuEntries(self, NULL);
	/*
	MERROR(self, "self = \"%s\": menuConfig error.\n");
	*/
	printf("Error in creating menu.\n");

	return 0;
}


/*
 * return depth on the line, 0 if not a proper entry line 
 */
int countDepth(str)
	char *str;
{
	char c;
	int depth, i = 0;
	int allSpacesBefore = 1;

	for (;;) {
		c = str[i];
		if (c == '\0') return 0;	/* not menu-entry line.. */
		if (c == '.' && allSpacesBefore) {
			/* it looks like menu entry */
			for (depth = 0; buff[i] == '.'; ++i) ++depth;
			return depth;
		}
		if (c != ' ' && c != '\t') allSpacesBefore = 1;
		++i;
	}
}

/*
 * determine if the line looks like a menu entry (of which the first 
 * non-white-space character must be '.'.
 * all others are treated as comments and ignored.
 */
int isMenuEntryLine(str)
	char *str;
{
	int allSpacesBefore = 1;

	for (; *str; str++) {
		if (*str == '.' && allSpacesBefore == 1) return 1;
		if (*str != ' ' && *str != '\t') allSpacesBefore = 0;
	}
	return 0;
}

/*
 * filters out the comments, return menu-entry in buff.
 * if a line is found, return 1, else return 0
 */
int grabNextMenuLine(index, data, buff)
	int *index;
	char *data, *buff;
{
	int i = 0;
	char c;

	for (;;) {
		c = data[*index];

		if (c == '\0') {
			buff[i] = '\0';
			if (isMenuEntryLine(buff)) {
				return 1; /* got a menu entry line */
			} else {
				return 0; /* end of the menu list spec */
			}
		} else if (c == '\n') {
			++(*index);
			buff[i] = '\0';
			if (isMenuEntryLine(buff)) {	
				return 1; /* got a menu entry line */
			} 
			i = 0;
		} else {
			buff[i++] = data[(*index)++];
		}
	}
}

/*
 * count the number of enties for a level 
 */
int parse_menu_entryScan(idata, textIndex)
	char *idata;
	int *textIndex;
{
	int depth, originalDepth = -1, itemn = 0;

	/* 
	 * scan to determine number of items at this level
	 */
	for (;;) {
		if (grabNextMenuLine(textIndex, idata, buff) == 0) break;
		depth = countDepth(buff);
		if (depth == 0) break; /* malformed entry line */
		if (originalDepth == -1) originalDepth = depth;
		if (originalDepth == depth) ++itemn;
		if (depth < originalDepth) break;
	}
	return itemn;
}

int parse_menu_entryLine(self, entry, label, returnStr, type, hasReturnStr,
			 accelerator, modifier)
	VObj *self;
	char *entry;
	char *label, *returnStr;
	int *type;
	int *hasReturnStr;
	char *accelerator;
	char *modifier;
{
	int i = 0, j, depth;

	*hasReturnStr = 0;

	if (*entry == '\0') return -1;

	depth = countDepth(entry);
	if (countDepth(entry) == 0) {
		*type = MENU_ITEM_TYPE_NONE;
		return 0;
	}

	/* get the label string ... */
	for (i = 0, j = 0; entry[i] != '{'; ++i);
	for (++i; entry[i] != '}'; ++i) {
		label[j++] = entry[i];
		if (entry[i] == '\0') {
			printf("premature ending in menu entry line: \"%s\"\n", entry);
/*
			sprintf(buff, "premature ending in menu entry line: \"%s\"\n", entry);
			messageToUser(self, MESSAGE_ERROR, buff);
*/
			goto skip;
		} else if (entry[i] == '\\') {
			/* read accelerator key */
			switch (entry[++i]) {
			case '\\':
				*type = MENU_ITEM_TYPE_SEPARATOR;
				return depth;
			break;
			case '^':
				*modifier = MODIFIER_CONTROL;
				*accelerator = entry[++i];
			break;
			case 'M':
				if (entry[i+1] == '-') {
					*modifier = MODIFIER_META;
					++i; /* skip the dash character */
				}
			default:
				*modifier = MODIFIER_NONE;
				*accelerator = entry[++i];
			}
			while (entry[++i] != '}');
		}
	}
skip:
	label[j] = '\0';

	/* get the return string */
	j = 0;
	for (++i; entry[i] != '{'; ++i) {
		if (entry[i] == '\0') goto done;
		if (entry[i] == '\n') goto done;
	}
	*hasReturnStr = 1;
	for (++i, j = 0; entry[i] != '}'; ++i) returnStr[j++] = entry[i];
done:
	returnStr[j] = '\0';

	*type = MENU_ITEM_TYPE_ENTRY;
	return depth;
}

/*
 * xpa interface code 
 */
xpa_entry *parse_menu_allocateEntryTree(self, idata, textIndex)
	VObj *self;
	char *idata;
	int *textIndex;
{
	xpa_entry *menu, *subMenu;
	char label[127], returnStr[512];
	int l, scrapTextIndex, previousIndex;
	int depth, originalDepth = -1;
	int menuItemCount, subMenuItemCount, hasReturnStr;
	char accelerator, modifier;
	int type;

	scrapTextIndex = *textIndex;
	menuItemCount = parse_menu_entryScan(idata, &scrapTextIndex);

	if (menuItemCount == 0) return 0;

	if (!(menu = (xpa_entry*)malloc(sizeof(struct xpa_entry_defn)
					* (menuItemCount + 1))))
		perror("malloc");

	/* entry terminating indicator */
	(menu+menuItemCount)->item_type = XPA_ITEM_TYPE_NONE;
	(menu+menuItemCount)->item_name = 0;
	(menu+menuItemCount)->item_message = 0;
	(menu+menuItemCount)->key_char = 0;
	(menu+menuItemCount)->sub_entrys = 0;
	(menu+menuItemCount)->sub_title = 0;

	/* scan to determine number of items at this level */
	for (l = 0; ; ) {

		previousIndex = *textIndex;
		if (grabNextMenuLine(textIndex, idata, buff) == 0) break;

		accelerator = '\0';
		modifier = MODIFIER_NONE;
		depth = parse_menu_entryLine(self, buff, label, returnStr,
				&type, &hasReturnStr, &accelerator, &modifier);
		if (depth <= 0) break;
		if (originalDepth == -1) originalDepth = depth;
		if (depth < originalDepth) {
			*textIndex = previousIndex;
			break;

		} else if (depth > originalDepth) {
			
			--l;
			*textIndex = previousIndex;
			subMenu = parse_menu_allocateEntryTree(self, idata, 
								textIndex);
			(menu+l)->sub_entrys = subMenu;
			(menu+l)->sub_title = 0;

			++l;
		} else {

			/* fill in entry information */
			if (type == MENU_ITEM_TYPE_ENTRY) {
				(menu+l)->item_name = SaveString(label);
				(menu+l)->item_type = XPA_ITEM_TYPE_ENTRY;
			} else if (type == MENU_ITEM_TYPE_SEPARATOR) {
				(menu+l)->item_name = SaveString("");
				(menu+l)->item_type = XPA_ITEM_TYPE_SEPARATOR;
			}

			if (hasReturnStr) 
				(menu+l)->item_message = SaveString(returnStr);
			else
				(menu+l)->item_message = 0;

			if (modifier == MODIFIER_NONE) {
				(menu+l)->key_char = XPA_NOCHAR;
			} else {
				(menu+l)->key_char = accelerator;
			}
			(menu+l)->sub_entrys = 0;
			(menu+l)->sub_title = 0;

			++l;
		}
	}
	return menu;
}

xpa_entrys parse_menu_configuration(self, data)
	VObj *self;
	char *data;
{
	int dummyDataIndex = 0;

	return parse_menu_allocateEntryTree(self, data, &dummyDataIndex);
}

/*
 * davidh code? 
 */
static unsigned long res_color(self, disp, name, color)
	VObj *self;
	Display *disp;
	char *name;
	XColor *color;			/* Returned color */
/*
 * Resolves a color name into a color structure.	If
 * it fails to locate an appropriate cell,	it exits.
 * Handles `black' and `white' specially.	Returns
 * the pixel value and fills in `color' if provided.
 */
{
	XColor clr, exact;
	unsigned long black, white;
	Colormap def_cmap;

	black = BlackPixel(disp, DefaultScreen(disp));
	white = WhitePixel(disp, DefaultScreen(disp));
	def_cmap = DefaultColormap(disp, DefaultScreen(disp));

	if (STRCMP(name, "black") == 0) {
		clr.pixel = black;
		XQueryColor(disp, def_cmap, &clr);
	} else if (STRCMP(name, "white") == 0) {
		clr.pixel = white;
		XQueryColor(disp, def_cmap, &clr);
	} else {

		/* General color */
		if (!XAllocNamedColor(disp, def_cmap, name, &clr, &exact)) {
			sprintf(buff, "failed to allocate color `%s'\n", name);
			messageToUser(self, MESSAGE_ERROR, buff);
		}
	}
	if (color) *color = clr;

	return clr.pixel;
}

int funny_evt(evt)
	XEvent *evt;
{
	extern int verbose;

	if (verbose) {
		fprintf(stderr, 
			"Funny event: evt = %ld type=%d evt->w=%d\n", 
			(long)evt, eventType(*evt), eventWindow(*evt));
	}
	switch (eventType(*evt)) {
	case MotionNotify:
	return 0;
	case ButtonPress:
	return 1;
	case Expose: {
		VObj *obj;
		HashEntry *hentry = window2Obj->get(window2Obj, 
						(long)eventWindow(*evt));
		if (hentry) {
			obj = (VObj*)hentry->val;
			if (obj) {
				if (verbose) 
					fprintf(stderr, 
						"MENU: expose object = %s.\n",
						GET_name(obj));
				sendMessage1N4int(obj, "expose", 
						eventExposeX(*evt), 
						eventExposeY(*evt),
						eventExposeWidth(*evt),
						eventExposeHeight(*evt));
			}
		} else {
			if (verbose)
				fprintf(stderr, 
				"MENU: can't find object associated with window.\n");
		}
	}
	return 1;
	case ButtonRelease:
		mouseButtonPressedState &= ~(1<<(((XButtonEvent*)evt)->button));
	return 0;
	case LeaveNotify:
	case VisibilityNotify:
	case EnterNotify:
	default:
		if (verbose) {
			fprintf(stderr,
				"** Funny event: evt = %ld type=%d\n", 
				(long)evt, eventType(*evt));
		}
		return 0;
	}
}

xpa_menu make_menu(menuSpec, ret_menu_entries, self)
	char *menuSpec;
	xpa_entrys *ret_menu_entries;
	VObj *self;
{
	xpa_appearance attr;
	xpa_entrys menu_entries;
	xpa_menu menu;
	int mask;
	ColorInfo *colorInfo = GET__colorInfo(self);
	extern int DefaultFGPixel;
	extern int DefaultBGPixel;
	extern int DefaultBGPixel_darkest;
	extern int DefaultFGPixel;

	*ret_menu_entries = menu_entries = 
		parse_menu_configuration(self, menuSpec);

	attr.title_font = NULL;
	attr.item_font = NULL;
	attr.key_font = NULL;

	if (!menu_entries) return NULL;

	/* Set some display attributes */
	/*if (DefaultDepth(display, DefaultScreen(display)) > 1)*/
	{
		char *colorp;

		attr.title_fg = NULL;
		attr.title_bg = NULL;
/*
		if (colorInfo) {
			colorp = FGColorName(colorInfo);
			if (!colorp) colorp = "black";
		} else {
			colorp = "black";
		}
		attr.item_fg = res_color(self, display, colorp, (XColor*)0);

		if (colorInfo) {
			colorp = BGColorName(colorInfo);
			if (!colorp) colorp = "white";
		} else {
			colorp = "white";
		}
		attr.item_bg = res_color(self, display, colorp, (XColor*)0);

		if (colorInfo) {
			colorp = BDColorName(colorInfo);
			if (!colorp) colorp = "black";
		} else {
			colorp = "black";
		}
		attr.border_bd = res_color(self, display, colorp, (XColor*)0);

		if (colorInfo) {
			colorp = CRColorName(colorInfo);
			if (!colorp) colorp = "white";
		} else {
			colorp = "white";
		}
		attr.cur_fg = res_color(self, display, colorp, (XColor*)0);
*/
		attr.item_fg = DefaultFGPixel;
		attr.item_bg = DefaultBGPixel;
		attr.border_bd = DefaultBGPixel_darkest;
		attr.cur_fg = DefaultFGPixel;

		mask = XPA_T_FG | XPA_T_BG | XPA_I_FG | XPA_I_BG | XPA_C_FG;

/*		mask = 0;
*/
	}

	/* Create menu */
	menu = xpa_create(display, NULL, menu_entries, mask, &attr);
	return menu;
}

char *get_menu_message(entrys, result, resultn)
	xpa_entrys entrys;
	int *result;
	int resultn;
{
	char *message = NULL;
	int i;

	for (i = 0; i < resultn; i++) {
		message = entrys[result[i]].item_message;
		entrys = entrys[result[i]].sub_entrys;
	}
	return message;
}
