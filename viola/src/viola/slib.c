/*
 * Sound Library
 *
 * well, what can I say... we're just not there yet.
 */
#include "utils.h"
#include "mystrings.h"
#include "error.h"
#include "file.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "slotaccess.h"
#include "glib.h"

int SLBellVolume(percent) 
	int percent;
{
	XBell(display, percent);
	return percent;
}

int SLBell()
{
	write(1, "\007", 1);  /* beep */
}

