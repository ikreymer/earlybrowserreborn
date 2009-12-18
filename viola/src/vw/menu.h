/*
 * menu.h
 *
 * Definitions for MotifWWW main menus.
 * Copied from O'Reilly's "Motif Programming Manual", Vol 6 of X series.
 *
 */
/*
 * Copyright 1993 O'Reilly & Associates. All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */

#ifndef _MENU_H_
#define _MENU_H_


#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>

Widget buildMenus();
void genericCallback();
void quitCallback();


typedef struct menuItemStruct {
    char *label;
    WidgetClass *class;
    char mnemonic;
    char *accelerator;
    char *accelText;
    void (*callback)();
    XtPointer clientData;
    char *helpText;
    Boolean sensitive;
    struct menuItemStruct *subMenu;
} MenuItem;


typedef struct menuStruct {
    char *title;
    char mnemonic;
    MenuItem *menuItems;
} Menu;


#define BOOKMARK_MENU_NAME "Marked Documents"


#endif _MENU_H_
