/*
 * fileIO.c
 *
 * Routines for doing file I/O.
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

#ifndef _FILE_IO_H_
#define _FILE_IO_H_


#include <Xm/Xm.h>
#include <stdlib.h>
#include <stdio.h>

#include "vw.h"


#define CANCELED 0
#define SAVED 1


int vwSaveAs();


#endif _FILE_IO_H_
