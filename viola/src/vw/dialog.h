/*
 * dialog.h
 *
 * Error and warning dialogs.
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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include "vw.h"

enum dlogType {INFO, WARNING, ERROR};

struct promptStruct {
    int *done;
    char *result;
    char *yesLabel, *noLabel, *cancelLabel;
    DocViewInfo *dvip;
};

struct lentryDlogStruct {
    Widget dlog;
    Widget lineText;
    char *data;
    DocViewInfo *dvi;
    void (*okCB)();
};

#define infoDialog(dvip, msg)  messageDialog(dvip, msg, INFO)
#define warningDialog(dvip, msg)  messageDialog(dvip, msg, WARNING)
#define errorDialog(dvip, msg)  messageDialog(dvip, msg, ERROR)


void messageDialog();
void modalErrorDialog();
char *promptDialog();
char *questionDialog();
char *fileSelectionDialog();

void infoDialogMH();
void warningDialogMH();
void modalErrorDialogMH();
void errorDialogMH();
void promptDialogMH();
void questionDialogMH();

void printDialog();
void openURLDialog();
void simpleOpenURLDialog();

void openURLInSelectionBuffer();

#endif _DIALOG_H_
