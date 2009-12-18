/*
 * edit.h
 *
 * HTML source editor dialog.
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

#ifndef _EDIT_H_
#define _EDIT_H_


void showSourceCB();
void showSourceStringMH();
void showSourceFileMH();
void showSourceEditor();
void editorValueChangedCB();
void closeSourceEditor();
void saveSource();
void saveAsSource();
void reloadSource();
void uploadSource();

char *readFile();
int writeFile();

#include <stdio.h>
#ifndef SEEK_END
#define SEEK_END 2L
#endif


#endif _EDIT_H_
