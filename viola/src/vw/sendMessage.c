/*
 * sendMessage.c
 *
 * Code for sending messages with arbitrary data types to a named Viola object.
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

#include <varargs.h>

sendMessage(va_alist)
    va_dcl
{
    va_list ap;
    char *objName, *msg;

    va_start(ap);

    objName = va_arg(ap, char *);
    msg = va_arg(ap, char *);

    while (msgArgType = va_arg(ap, int)) {
	switch (msgArgType) {
	case INT:
	    /* Interpret next arg as an int. */
	    break;
	case STR:
	    /* Interpret next arg as a char *. */
	    break;
	}
    }

    va_end(ap);

    /* Send off the message. */
}
    
    
    
