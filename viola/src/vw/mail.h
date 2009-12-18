/*
 * mail.h
 *
 * Send mail, mostly for comments to developers.
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

#ifndef _MAIL_H_
#define _MAIL_H_

#include "vw.h"

void showMailEditor();

void sendMailCB();
void mailLoadFileCB();
void mailSaveAsCB();
void mailValueChangedCB();
void closeMailEditorCB();


#endif _MAIL_H_

