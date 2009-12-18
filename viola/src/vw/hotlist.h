/*
 * hotlist.h
 *
 * Routines and data structures for handling document traversal hotlist.
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

#ifndef _HOTLIST_H_
#define _HOTLIST_H_


/* CONSTANTS */
#define HOTLIST_CHUNK 256


/* PROTOTYPES */
void hotlistPrev();
void hotlistNext();
void hotlistBackUp();

void hotlistPrevMH();
void hotlistNextMH();
void hotlistBackUpMH();

void hotlistAdd();
void hotlistAddMH();
void setHotlistList();
void setHotlistListMH();

void freeHotlistList();
void growHotlistList();

Widget createHotlistDialog();
void showHotlist();
void showHotlistCB();
void hideHotlist();
void hotlistSelect();
void hotlistSelectCB();

void addToHotlist();
void addToHotlistCB();

void addHotlistItem();
void deleteHotlistItem();
void gotoHotlistItem();
void editHotlistItem();

#endif _HOTLIST_H_
