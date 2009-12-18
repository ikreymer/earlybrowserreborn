/*
 * catalog.h
 *
 * Personal URL Link Manager
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

#ifndef _CATALOG_H_
#define _CATALOG_H_

#include "vw.h"
#include "menu.h"

#define MAX_SELECTIONS 100
#define ITEM_ALLOC_CHUNK 32

/* For testing purposes ... to be removed later. */
#define DEFAULT_CATALOG_FILE "/usr/tmp/defaultSpiderCatalog"

enum itemTypes {ITEM, FOLDER, LINK};
enum itemStates {NONE=0, SELECTED};


typedef struct itemStruct {
    char type;
    char state;

    Pixmap icon;
    GC gc;
    char *iconFile;
    short x, y, w, h;

    char *name;
    XmString nameXMS;
    short nx, ny;

    char *commentURL;
} Item;


typedef struct folderStruct {
    char type;
    char state;

    Pixmap icon;
    GC gc;
    char *iconFile;
    short x, y, w, h;

    char *name;
    XmString nameXMS;
    short nx, ny;

    char *commentURL;
    Item **items;
    short nItems;
    short allocedItems;
    short cw, ch;  /* size of folder canvas area (when open) */
   
    XmString nItemsXMS;
    short numx, numy;
} Folder;


typedef struct linkStruct {
    char type;
    char state;

    Pixmap icon;
    GC gc;
    char *iconFile;
    short x, y, w, h;

    char *name;
    XmString nameXMS;
    short nx, ny;

    char *commentURL;
    char *url;
    char *comment;
    char *folderName;
    Folder *folder;
} Link;


typedef struct catalogStruct {
    Folder *topFolder;
    Folder *currentFolder;
    char *catalogFileName;

    char visible;

    GC gc;
    Widget shell;
    Widget canvas;
    Widget helpLabel;
    DocViewInfo *dvi;
} Catalog;


typedef union ItemStruct {
    Item item;
    Link link;
    Folder folder;
} CatalogItem;


/* PROTOTYPES */
void showCatalogCB();
void closeCatalogCB();

#endif _CATALOG_H_
