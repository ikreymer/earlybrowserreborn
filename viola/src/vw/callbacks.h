/*
 * callbacks.h
 *
 * Prototype for button and menu callbacks.
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

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_


void genericCallback();
void okCallback();
void quitCallback();
void quitOk();
void openDocument();
void searchCallback();
void vwSaveAsCB();
void saveAsMH();
void printFile();
void reloadDoc();
void reloadDocCB();
void oneMessageCB();

void clonePage(); /* clone page for navigation */
void clonePageMH();
void clonePageCB();
void pageCloneMapped();
void showPageClone();
void closePageCB();
void closePageShell();
void cloneApp(); /* clone app */
void appCloneMapped();
void showAppClone();
void closeThisShell();

void flipGlobe();
void showHelpMessage();
void showHelpMessageInMainWindow();
void helpHandler();
void blankHelpHandler();
void blankHelpMessageInMainWindow();

void navigateBackUp();
void navigateNext();
void navigatePrev();
void navigateHome();
void navigateHomeEH();

void changeFonts();

void oneWordMessageCB();

void titleButtonEH();
void editorTitleButtonEH();

void newDocument();
void searchModeMH();
void sliderConfig();

void doViolaEvent();
void doViolaIdle();
void resizeViola();

void scrollBarValueChanged();
void scrollBarDrag();

void printTest();


#endif _CALLBACKS_H_
