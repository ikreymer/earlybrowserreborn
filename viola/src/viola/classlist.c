/*
 * Copyright 1990-1992 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
#include "utils.h"
#include <ctype.h>
#include "mystrings.h"
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "classlist.h"
/*
 * List of classes, in order of initialization.
 */
ClassInfo *classList[] = {
	&class_cosmic,		/* (internal) root of class hierarchy	*/
	&class_field,		/* general window field			*/
	&class_glass,		/* glass panel (input only window)	*/
	&class_PS,		/* postscript viewer			*/
	&class_project,		/* top level window shell		*/
	&class_generic,		/* (internal) for sharing misc methods	*/
	&class_pane,		/* configurable pane			*/
	&class_vpane,		/* vertical pane geometry manager	*/
	&class_hpane,		/* horizontal pane geometry manager	*/
	&class_rubber,		/* rubber geometry manager		*/
	&class_tray,		/* */
	&class_txt,		/* (internal) for sharing text methods	*/
	&class_txtButton,	/* single line text button		*/
	&class_txtLabel,	/* single line text label		*/
	&class_txtDisp,		/* text field for viewing (cursor off)	*/
	&class_txtEdit,		/* text field for ediging (cursor on) 	*/
	&class_txtEditLine,     /* single line txtEdit                  */
	&class_HTML,		/* HyperText Markup Language		*/
	&class_slider,		/* sliding valuator			*/
	&class_XBM,		/* MIT's bitmap format 			*/
	&class_XBMBG,		/* XBM background  			*/
	&class_XBMButton,	/* XBM with button border		*/
	&class_XPM,		/* BullGroup's XPM color bitmap format	*/
	&class_XPMBG,		/* XPM Background			*/
	&class_XPMButton,	/* XPM with button border		*/
	&class_GIF,		/* GIF viewer				*/
	&class_dial,		/* rotating dial valuator		*/
	&class_menu,		/* pull-down menu			*/
	&class_toggle,		/* toggler				*/
	&class_radio,		/* radio toggler			*/
	&class_BCard,		/* background card 			*/
	&class_FCard,		/* foreground card 			*/
	&class_stack,		/* stack of f/b cards 			*/
	&class_client,		/* (internal) for sharing IPC code	*/
	&class_TTY,		/* IPC through stdI/O (pseudo-terminal)	*/
	&class_socket,		/* IPC through sockets			*/

/* experimental classes.. replace once dynamic classing is available */
	&class_AC_SGML_txt,
/*	&class_AC_SGML_header_big,*/
	NULL
};



