/*
 * dummy.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Wed Feb 26 16:30:17 1992 tvr
 * Last modified: Wed Feb 26 23:12:57 1992 tvr
 *
 */

#include <stdio.h>
#include "HTStyle.h"
#include "HText.h"

/*
 * something needed here to compile, but not to HText ..
 */

FILE *logfile = NULL;

char *HTClientHost = NULL;

struct HText *HTMainText = NULL;

struct HTParentAnchor *HTMainAnchor = NULL;
