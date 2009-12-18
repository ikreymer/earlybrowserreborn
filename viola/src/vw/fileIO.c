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

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/SelectioB.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>

#include "vw.h"
#include "dialog.h"
#include "fileIO.h"


int vwSaveAs(message, dvi)
    char *message;
    DocViewInfo *dvi;
{
    FILE *fp;
    int done=0, length;
    char *data, *fileName, *answer, buf[1024];
    
    while (!done) {
	getcwd(buf, MAXPATHLEN);
	length = strlen(buf);
	if (length < MAXPATHLEN) {
	    buf[length] = '/';
	    buf[length+1] = '\0';
	}
	fileName = promptDialog(dvi, message, buf);
	
	if (!fileName)
	    return(CANCELED);   /* User selected "Cancel" */

	fp = fopen(fileName, "r");
	if (fp) {
	    fclose(fp);
	    sprintf(buf,
		    "The file \"%s\" already exists!\n\nDo you wish to overwrite it?",
		    fileName);
	    answer = questionDialog(dvi,
				    buf,
				    "Pick a New File Name",
				    "Overwrite File",
				    "Pick a New File Name",
				    "Cancel");
	    if (!strcmp(answer, "Pick a New File Name")) {
		free(fileName);
		continue;
	    } else if (!strcmp(answer, "Cancel")) {
		free(fileName);
		return(CANCELED);
	    }
	}

	data = XmTextGetString(dvi->textEditor);
	done = writeFile(data, fileName, dvi);
	XtFree(data);
	free(fileName);
    }
    
    return(SAVED);
}


