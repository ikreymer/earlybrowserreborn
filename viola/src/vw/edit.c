/*
 * edit.c
 *
 * HTML source editor dialog.
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
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vw.h"
#include "dialog.h"
#include "callbacks.h"
#include "selection.h"
#include "fileIO.h"
#include "cursor.h"
#include "edit.h"



void showSourceCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi;
    char *arg[4];
    char buf[256];

    dvi = (DocViewInfo*)((ClientData*)clientData)->shellInfo;

    /*
     *  Send viola a "showSource" message, wait for "showSourceString"
     *  or "showSourceFile" message response.
     */
    ViolaRegisterMessageHandler("showSourceString",
				showSourceStringMH,
				(void *) dvi);
    ViolaRegisterMessageHandler("showSourceFile",
				showSourceFileMH,
				(void *) dvi);
    sendMessage1(dvi->violaDocViewObj, "showSource");


    /* CODE BELOW is for testing purposes only... */
    /*
    arg[0] = "showSourceFile";
    sprintf(buf, "%d", dvi->cloneID);
    arg[1] = buf;
    arg[2] = "/usr/work/viola/vw/vw.c";
    arg[3] = "editable";

    showSourceFileMH(arg, 4, (void *) dvi);
    */
}


/*
 * arg[0] = "showSourceString"
 * arg[1] = "cloneID (cookie)
 * arg[2] = string of source data
 */
void showSourceStringMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *parentDocViewInfo = (DocViewInfo *) clientData;
    char *sourceData;
    int editable=0;

    if (argc < 3)
	return;

    if (parentDocViewInfo->cloneID != atol(arg[1]))
	return;

    if (!strcmp(arg[3], "editable"))
	editable = 1;

    ViolaDeleteMessageHandler("showSourceString",
			      showSourceStringMH,
			      clientData);
    ViolaDeleteMessageHandler("showSourceFile",
			      showSourceFileMH,
			      clientData);
    
    sourceData = makeString(arg[2]);
    if (sourceData)
	showSourceEditor(parentDocViewInfo, sourceData, editable);
}


/*
 * arg[0] = "showSourceFile"
 * arg[1] = "cloneID (cookie)
 * arg[2] = file name
 * arg[3] = editable = "editable" or "readOnly"
 */
void showSourceFileMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *parentDocViewInfo = (DocViewInfo *) clientData;
    char *fileName, *sourceData;
    FILE *fp;
    int bytes=0, bytesInFile=0, bytesRead=0, editable=0;

    if (argc < 4)
	return;

    if (parentDocViewInfo->cloneID != atol(arg[1]))
	return;

    if (!strcmp(arg[3], "editable"))
	editable = 1;
    
    ViolaDeleteMessageHandler("showSourceString",
			      showSourceStringMH,
			      clientData);
    ViolaDeleteMessageHandler("showSourceFile",
			      showSourceFileMH,
			      clientData);
    
    sourceData = readFile(arg[2], parentDocViewInfo);
    if (sourceData) {
	showSourceEditor(parentDocViewInfo, sourceData, editable);
	free(sourceData);
    }
    unlink(arg[2]);
}


/*
 * readFile()
 *
 * Returns a pointer to the file contents.  The pointer must be free'd
 * when the caller is done with it.  If there is an error, readFile()
 * returns NULL.
 */
char *readFile(fileName, dvi)
    char *fileName;
    DocViewInfo *dvi;
{
    int bytes, bytesRead=0, bytesInFile;
    char *fileData;
    FILE *fp;
    
    fp = fopen(fileName, "r");
    if (!fp) {
	/* Inform user of error. */
	return;
    }

    /*
     * This is a hack to determine the length of the file, but at
     * least it is ANSI.  I don't know how portable fstat() is.
     */
    fseek(fp, 0, SEEK_END);
    bytesInFile = ftell(fp);
    if (bytesInFile <= 0) {
	/* Inform user of problem. */
	fclose(fp);
	return;
    }
    rewind(fp);

    fileData = malloc(bytesInFile+1);
    if (!fileData) {
	/* Inform user of error. */
	fclose(fp);
	return;
    }
    
    while(!feof(fp) && !ferror(fp) && bytesRead < bytesInFile) {
	bytes = fread((char *) (fileData+bytesRead),
		      1, (bytesInFile-bytesRead), fp);
	bytesRead += bytes;
    }
    fclose(fp);

    if (ferror(fp)) {
	free(fileData);
	/* Inform user of error. */
	return(NULL);
    } else {
	fileData[bytesRead] = '\0';
	return(fileData);
    }
}



/*
 * Pops up a text editor with the given string data in it.
 */
void showSourceEditor(parentDVI, data, editable)
    DocViewInfo *parentDVI;
    char *data;
    int editable;
{
    Widget shell,
           titleFrame,
           title,
           helpLabel,
           form,
           frame,
           buttonBox, reloadB, saveB, saveAsB, closeB,
           textEditor;
    Pixel bg;
    DocViewInfo *localDocViewInfo;
    ClientData *cd;
    char *name, shellName[1024];
    XmString xms;
    Arg args[16];
    int n=0, sourceLength;
    extern Pixmap icon;

    localDocViewInfo = (DocViewInfo *) calloc(1, sizeof(DocViewInfo));
    if (!localDocViewInfo) {
	fprintf(stderr, "Out of memory.\n");
	return;
    }

    name = parentDVI->docName;
    sprintf(shellName, "Document Source Editor");

    shell = XtVaAppCreateShell(shellName, "Viola",
			       applicationShellWidgetClass,
			       XtDisplay(parentDVI->shell),
			       XmNborderWidth, 2,
			       XmNiconPixmap, icon,
			       NULL);

    form =  XtVaCreateManagedWidget("form", xmFormWidgetClass, shell,
				    XmNhorizontalSpacing, FORM_SPACING,
				    XmNverticalSpacing, FORM_SPACING,
				    NULL);
    localDocViewInfo->topMostWidget = form;

    titleFrame =  XtVaCreateManagedWidget("titleFrame", xmFrameWidgetClass, form,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNmarginHeight, 2,
					  XmNmarginWidth, 2,
					  NULL);

     if (!strlen(parentDVI->docName))
	/* The document doesn't have an official <TITLE>. */
	xms = makeXMSTitle(parentDVI->URL, parentDVI->docName);
    else
	xms = makeXMSTitle(parentDVI->docName, parentDVI->URL);
    title = XtVaCreateManagedWidget("editorTitle", xmLabelWidgetClass, titleFrame,
				    XmNlabelString, xms,
				    XmNborderWidth, 4,
				    XmNfontList, titleFontList,
				    NULL);
    XtVaGetValues(title, XmNbackground, &bg, NULL);
    XtVaSetValues(title, XmNborderColor, bg, NULL);
    XmStringFree(xms);


    /* URL selection mechanism. */
    XtAddEventHandler(title, ButtonReleaseMask, FALSE,
		      editorTitleButtonEH, (XtPointer) localDocViewInfo);
    /*
    XtAddEventHandler(title, 0L, TRUE,
		      urlSelectionRequest, (XtPointer) localDocViewInfo);
    XtAddEventHandler(title, 0L, TRUE,
		      urlSelectionClear, (XtPointer) title);
    */

    /*
     * Buttons
     */
    buttonBox = XtVaCreateManagedWidget("buttonBox",
					xmFormWidgetClass,
					form,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNfractionBase, 4,
					NULL);

    cd = (ClientData *) malloc(sizeof(ClientData));
    cd->data = (void *) parentDVI;
    cd->shellInfo = (void *) localDocViewInfo;
    
    reloadB = XtVaCreateManagedWidget("Upload",
				      xmPushButtonWidgetClass,
				      buttonBox,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 1,
				      XmNsensitive, FALSE,
				      /*XmNsensitive, (editable? TRUE: FALSE),*/
				      NULL);
    XtAddCallback(reloadB, XmNactivateCallback,
		  reloadSource, (XtPointer) cd);

    saveB = XtVaCreateManagedWidget("Save",
				    xmPushButtonWidgetClass,
				    buttonBox,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget, reloadB,
				    XmNrightAttachment, XmATTACH_POSITION,
				    XmNrightPosition, 2,
				    XmNsensitive, FALSE,
				    NULL);
    XtAddCallback(saveB, XmNactivateCallback,
		  saveSource, (XtPointer) cd);
    
    saveAsB = XtVaCreateManagedWidget("Save As...",
				      xmPushButtonWidgetClass,
				      buttonBox,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_WIDGET,
				      XmNleftWidget, saveB,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 3,
				      XmNsensitive, (editable? TRUE: FALSE),
				      NULL);
    XtAddCallback(saveAsB, XmNactivateCallback,
		  saveAsSource, (XtPointer) localDocViewInfo);

    closeB = XtVaCreateManagedWidget("Close",
				     xmPushButtonWidgetClass,
				     buttonBox,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, saveAsB,
				     NULL);
    XtAddCallback(closeB, XmNactivateCallback,
		  closeSourceEditor, (XtPointer) cd);

    
    helpLabel = XtVaCreateManagedWidget("helpLabel", xmLabelWidgetClass, form,
					XmNbottomAttachment, XmATTACH_WIDGET,
					XmNbottomWidget, buttonBox,
					XmNrightAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrecomputeSize, FALSE,
					XmNborderWidth, 1,
					NULL);
    setHelp(title, helpLabel, "Click right mouse button to copy the document URL to the clipboard.");
    setHelp(reloadB, helpLabel, "Reloads this source into the WWW Document Viewer.  Changes you have made will be reflected.");
    setHelp(closeB, helpLabel, "Closes this document source editor.");
    setHelp(saveB, helpLabel, "Saves this source text to the original source of the document.");
    setHelp(saveAsB, helpLabel, "Saves this source text to a specified file on your local system.");

    
    frame = XtVaCreateManagedWidget("frame", xmFormWidgetClass, form,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, titleFrame,
				    XmNbottomAttachment, XmATTACH_WIDGET,
				    XmNbottomWidget, helpLabel,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNshadowThickness, 4,
				    XmNshadowType, XmSHADOW_ETCHED_IN,
				    XmNhorizontalSpacing, 4,
				    XmNverticalSpacing, 4,
				    NULL);

    /*
     * Should keep track of whether or not the data was altered.
     * Pop up a verification dialog if close before save.
     */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNtopWidget, titleFrame);  n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNbottomWidget, helpLabel);  n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNscrollVertical, TRUE);  n++;
    XtSetArg(args[n], XmNresizeWidth, TRUE);  n++;
    XtSetArg(args[n], XmNwidth, 550);  n++;
    XtSetArg(args[n], XmNheight, 500);  n++;
    XtSetArg(args[n], XmNeditable, (editable? TRUE: FALSE));  n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
    XtSetArg(args[n], XmNvalue, data);  n++;
    textEditor = XmCreateScrolledText(frame, "textEditor", args, n);
    XtManageChild(textEditor);
    setHelp(textEditor, helpLabel, "This is the source text from which the current document was generated.");

    localDocViewInfo->editable = editable;
    localDocViewInfo->editorDataChanged = 0;
    localDocViewInfo->textEditor = textEditor;

    localDocViewInfo->editorSaveButton = saveB;
    if (editable)
	XtAddCallback(textEditor, XmNvalueChangedCallback,
		      editorValueChangedCB, (XtPointer) localDocViewInfo);
    
    localDocViewInfo->shellName = makeString(shellName);
    localDocViewInfo->docName = makeString(parentDVI->docName);
    localDocViewInfo->URL = makeString(parentDVI->URL);
    localDocViewInfo->shell = shell;
    localDocViewInfo->cloneID = (long) shell;
    localDocViewInfo->title = title;
    localDocViewInfo->scrollBar = (Widget) 0;
    localDocViewInfo->canvas = textEditor;
    localDocViewInfo->quitDlog = (Widget) 0;
    localDocViewInfo->violaDocViewName = NULL;
    localDocViewInfo->violaDocToolsName = NULL;
    localDocViewInfo->violaDocViewObj = NULL;
    localDocViewInfo->violaDocToolsObj = NULL;
    localDocViewInfo->violaDocViewWindow = (Window) 0;
    localDocViewInfo->violaDocToolsWindow = (Window) 0;
    localDocViewInfo->nHistoryItems = 0;
    localDocViewInfo->historyListWidget = (Widget) 0;

    ViolaRegisterMessageHandler("busyCursor", busyCursorMH, (void *) localDocViewInfo);
    ViolaRegisterMessageHandler("idleCursor", idleCursorMH, (void *) localDocViewInfo);

    XtPopup(shell, XtGrabNone);
}


void editorValueChangedCB(textEditor, clientData, callData)
    Widget textEditor;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    dvi->editorDataChanged = 1;
    XtRemoveCallback(textEditor, XmNvalueChangedCallback,
		     editorValueChangedCB, (XtPointer) dvi);
    XtVaSetValues(dvi->editorSaveButton, XmNsensitive, TRUE, NULL);
}


void closeSourceEditor(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    DocViewInfo *parentDVI = (DocViewInfo *) cd->data;
    char *answer;

    if (dvi->editorDataChanged) {
	answer = questionDialog(dvi,
			     "There are unsaved changes.",
			     "Cancel",
			     "Save Changes...",
			     "Don't Save Changes",
			     "Cancel");
	if (!strcmp(answer, "Save Changes...")) {
	    answer = questionDialog(dvi,
				 "Would you like to save the changes to the original\nsource of the document or to a new file?",
				 "Save As a File...",
				 "Save to Original Source",
				 "Save As a File...",
				 "Cancel");
	    if (!strcmp(answer, "Save to Original Source")) {
		modalErrorDialog(dvi, "Sorry, it is not yet possible to save changes\nto the original source of the document.\n(So go away!)");
		return;
	    } else if (!strcmp(answer, "Save As a File...")) {
		if (vwSaveAs("Saving document source to a file...\n\nEnter the full path name:",
			     dvi) == CANCELED) {
		    return;
		} else if (dvi->editorDataChanged) {
		    dvi->editorDataChanged = 0;
		    XtAddCallback(dvi->textEditor, XmNvalueChangedCallback,
				  editorValueChangedCB, (XtPointer) dvi);
		}
	    } else {
		return;
	    }
	} else if (!strcmp(answer, "Cancel")) {
	    return;
	}
    }

    ViolaDeleteMessageHandler("busyCursor", busyCursorMH, (void *) dvi);
    ViolaDeleteMessageHandler("idleCursor", idleCursorMH, (void *) dvi);
    XtDestroyWidget(dvi->shell);
    freeDocViewInfo(dvi);
} 


int writeFile(textData, fileName, dvi)
    char *textData;
    char *fileName;
    DocViewInfo *dvi;
{
    FILE *fp;
    char buf[1024];
    int success=0;

    if (!textData || !fileName)
	return(0);
    
    fp = fopen(fileName, "w");
    
    if (!fp) {
	sprintf(buf, "Unable to open the file \"%s\" for writing!\n", fileName);
	modalErrorDialog(dvi, buf);
    } else {
	int bytesLeft, numBytes;
	bytesLeft = numBytes = strlen(textData);
	
	while(bytesLeft>0 && !ferror(fp))
	    bytesLeft -= fwrite((textData + numBytes-bytesLeft),
				sizeof(char),
				bytesLeft,
				fp);
	
	if (ferror(fp)) {
	    sprintf(buf, "Unable to save all the data to \"%s\"!\n", fileName);
	    modalErrorDialog(dvi, buf);
	} else {
	    success = 1;
	}
	
	fclose(fp);
    }

    return(success);
}


void saveSource(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    DocViewInfo *parentDVI = (DocViewInfo *) cd->data;

    if (!dvi->editable)
	return;
    
    if (!strncmp(dvi->URL, "file:", 5)) {
	/* This document's URL is a file, try to save to that. */
	
	char fileName[256], *data, *answer, buf[512];
	int i=0, j=0, length, writeStatus;
	struct stat statbuf;

	length = strlen(dvi->URL);
	if (length > 6) {
	    if (dvi->URL[6] == '/')
		i = 6;
	    else
		i = 5;

	    /* See if the first part is a machine name or part of the path. */
	    fileName[j++] = dvi->URL[i++];
	    while (i<length && dvi->URL[i]!='/')
		fileName[j++] = dvi->URL[i++];
	    fileName[j+1] = '\0';
	    if (stat(fileName, &statbuf) < 0)
		j = 0;
	    
	    /* Extract the file name. */
	    while (i<=length)
		fileName[j++] = dvi->URL[i++];

	    sprintf(buf, "Are you sure you want to overwrite\nthe file \"%s\" with your changes?", fileName);
	    answer = questionDialog(dvi, buf, "Cancel", "Yes", "Cancel", NULL);

	    if (!strcmp(answer, "Yes")) {
		data = XmTextGetString(dvi->textEditor);
		writeStatus = writeFile(data, fileName, dvi);
		XtFree(data);

		if (writeStatus) {
		    dvi->editorDataChanged = 0;
		    XtAddCallback(dvi->textEditor, XmNvalueChangedCallback,
				  editorValueChangedCB, (XtPointer) dvi);
		    XtVaSetValues(dvi->editorSaveButton,
				  XmNsensitive, FALSE,
				  NULL);
		}
	    }
	}
    } else {
	modalErrorDialog(dvi, "Sorry, it is not yet possible to upload document\nsource changes to a remote server.  (So go away!)");
	return;

	/* Send message to MVW to save the doc. */
	/*
	 *  Wait for a "saveSourceSuccess" message comes back,
	 *  free the data, update editorDataChanged, and
	 *  install editorValueChanged.
	 */
    }
}


void saveAsSource(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    (void) vwSaveAs("Saving document source to a file...\n\nEnter the full path name:",
		    dvi);

    if (dvi->editorDataChanged) {
	dvi->editorDataChanged = 0;
	XtAddCallback(dvi->textEditor, XmNvalueChangedCallback,
		      editorValueChangedCB, (XtPointer) dvi);
    }
}


void reloadSource(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    DocViewInfo *parentDVI = (DocViewInfo *) cd->data;

    /*
     * Wrap data from textEditor in a string or a file.  Pass the string
     * or file name to Viola.
     */
    
    infoDialog(dvi, "Sorry, reloading edited source documents is not\nsupported at this time."); 

    /*
    if (parentDVI->violaDocViewName) {
	data = XmTextGetString(dvi->textEditor);
	sendMessage1N1str(parentDVI->violaDocViewObj, "reloadSource", data);
	free(data);
    }
    */
}


