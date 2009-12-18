/*
 * ClMisc.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Mon Apr 20 19:27:04 1992 tvr
 * Last modified: Mon Apr 20 19:48:47 1992 tvr
 *
 */

#include "Cl.h"


/*
 * Set mode to poll or to select connection
 */

void
WWWErwiseSetPoll ()
{
  CL_DEBUG (("SetPoll\n"));

  WWWErwiseConnection->select_fd = 0;

  WWWErwiseConnection->function++;
}

void
WWWErwiseSetSelect ()
{
  CL_DEBUG (("SetSelect\n"));

  WWWErwiseConnection->select_fd = WWWErwiseConnection->fd;

  WWWErwiseConnection->function++;
}
