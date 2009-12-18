/* @(#)59	1.3  com/XTOP/X11/Xmu/CloseHook.h, gos, gos320 5/22/91 10:48:18 */
/*
 *   COMPONENT_NAME: XSAMPLE
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 16,27,40,42
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * $XConsortium: CloseHook.h,v 1.1 89/07/14 17:51:53 jim Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_CLOSEHOOK_H_
#define _XMU_CLOSEHOOK_H_

typedef caddr_t CloseHook;
CloseHook XmuAddCloseDisplayHook (/* Display *, int (*)(), caddr_t */);
Bool XmuRemoveCloseDisplayHook (/* Display *,CloseHook,int (*)(),caddr_t */);
Bool XmuLookupCloseDisplayHook (/* Display *,CloseHook,int (*)(),caddr_t */);

#endif /* _XMU_CLOSEHOOK_H_ */
