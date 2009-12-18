/*
 * history.h
 *
 * Routines and data structures for handling document traversal history.
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

#ifndef _HISTORY_H_
#define _HISTORY_H_


/* CONSTANTS */
#define HISTORY_CHUNK 256


/* PROTOTYPES */
void historyPrev();
void historyNext();
void historyBackUp();

void historyPrevMH();
void historyNextMH();
void historyBackUpMH();

void historyAdd();
void historyAddMH();
void setHistoryList();
void setHistoryListMH();

void freeHistoryList();
void growHistoryList();

Widget createHistoryDialog();
void showHistory();
void showHistoryCB();
void hideHistory();
void historySelect();
void historySelectCB();


#endif _HISTORY_H_
