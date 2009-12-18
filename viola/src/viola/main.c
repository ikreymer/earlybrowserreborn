/*
 * This is main for the binary program "viola".
 */
#include <stdio.h>
#include <X11/Xlib.h>
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"

void main(argc, argv)
	int argc;
	char *argv[];
{
    initViola(argc, argv, 
	      (char*)NULL, (Display*)NULL, (Screen*)NULL, (Window)0);
}

/* the libWWW calls these routines to report HTTP loading progress 
 */
void showHelpMessageInMainWindow(message)
    char *message;
{
	extern VObj *mesgObj;
	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) sendMessage1N1str(mesgObj, "show", message);
}

void blankHelpHandlerInMainWindow()
{
	extern VObj *mesgObj;
	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) sendMessage1(mesgObj, "clear");
}


