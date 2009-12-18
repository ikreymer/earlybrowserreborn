/*
 * mail.c
 *
 * Send mail.  Used mostly for comments to Viola developers.
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
#include <Xm/TextF.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "vw.h"
#include "fileIO.h"
#include "edit.h"
#include "dialog.h"
#include "callbacks.h"
#include "selection.h"
#include "mail.h"


typedef struct mailCBStruct {
    DocViewInfo *dvi;
    Widget commandText;
    Widget addressText;
    Widget subjectText;
    Widget letterText;
} MailCBStruct;


/*
 * Pops up a text editor with the given string data in it.
 */
void showMailEditor(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    Widget shell,
           titleFrame,
           title,
           helpLabel,
           form,
           frame,
           commandFrame,
           commandLabel,
           commandText,
           addressLabel,
           addressText,
           subjectLabel,
           subjectText,
           buttonBox, insertFileB, sendMailB, saveAsB, closeB,
           textEditor;
    Pixel bg;
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *localDocViewInfo, *parentDVI = (DocViewInfo *) cd->shellInfo;
    MailCBStruct *ms;
    XmString xms;
    Arg args[24];
    int n=0, sourceLength;
    extern Pixmap icon;

    
    localDocViewInfo = (DocViewInfo *) calloc(1, sizeof(DocViewInfo));
    if (!localDocViewInfo) {
	fprintf(stderr, "Out of memory.\n");
	return;
    }
    ms = (MailCBStruct *) malloc(sizeof(MailCBStruct));
    if (!ms) {
	fprintf(stderr, "Out of memory.\n");
	return;
    }
    ms->dvi = localDocViewInfo;

    shell = XtVaAppCreateShell("Mail Editor", "Viola",
			       applicationShellWidgetClass,
			       XtDisplay(parentDVI->shell),
			       XmNborderWidth, 2,
			       XmNiconPixmap, icon,
			       NULL);

    form =  XtVaCreateManagedWidget("form", xmFormWidgetClass, shell,
				    XmNhorizontalSpacing, FORM_SPACING,
				    XmNverticalSpacing, 6,
				    NULL);

    titleFrame =  XtVaCreateManagedWidget("titleFrame", xmFrameWidgetClass, form,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNmarginHeight, 2,
					  XmNmarginWidth, 2,
					  NULL);

    xms = makeXMSTitle("Send Comments to the Developers",
		       "(at O'Reilly & Associates)");
    title = XtVaCreateManagedWidget("mailTitle", xmLabelWidgetClass, titleFrame,
				    XmNlabelString, xms,
				    XmNborderWidth, 4,
				    XmNfontList, titleFontList,
				    NULL);
    XtVaGetValues(title, XmNbackground, &bg, NULL);
    XtVaSetValues(title, XmNborderColor, bg, NULL);
    XmStringFree(xms);


    commandFrame =  XtVaCreateManagedWidget("commandFrame",
					    xmFormWidgetClass,
					    form,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, titleFrame,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNrightAttachment, XmATTACH_FORM,
					    XmNhorizontalSpacing, 4,
					    XmNverticalSpacing, 4,
					    XmNshadowThickness, 2,
					    XmNshadowType, XmSHADOW_ETCHED_IN,
					    NULL);

    commandText = XtVaCreateManagedWidget("mailCommandText",
					  xmTextFieldWidgetClass,
					  commandFrame,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNtopOffset, 6,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, 25,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNvalue, MAIL_COMMAND,
					  NULL);
    commandLabel = XtVaCreateManagedWidget("EMail Program:",
					   xmLabelWidgetClass,
					   commandFrame,
					   XmNtopAttachment, XmATTACH_FORM,
					   XmNtopOffset, 6,
					   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					   XmNbottomWidget, commandText,
					   XmNbottomOffset, 0,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_WIDGET,
					   XmNrightWidget, commandText,
					   XmNalignment, XmALIGNMENT_BEGINNING,
					   NULL);
    addressText = XtVaCreateManagedWidget("mailAddressText",
					  xmTextFieldWidgetClass,
					  commandFrame,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, commandText,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, 25,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNvalue, DEVELOPER_ADDRESS,
					  NULL);
    addressLabel = XtVaCreateManagedWidget("Address:",
					   xmLabelWidgetClass,
					   commandFrame,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, commandText,
					   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					   XmNbottomWidget, addressText,
					   XmNbottomOffset, 0,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_WIDGET,
					   XmNrightWidget, addressText,
					   XmNalignment, XmALIGNMENT_BEGINNING,
					   NULL);
    subjectText = XtVaCreateManagedWidget("mailSubjectText",
					  xmTextFieldWidgetClass,
					  commandFrame,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, addressText,
					  XmNbottomAttachment, XmATTACH_WIDGET,
					  XmNbottomOffset, 6,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, 25,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNvalue, "-s \"subject\"",
					  NULL);
    subjectLabel = XtVaCreateManagedWidget("Subject:",
					   xmLabelWidgetClass,
					   commandFrame,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, addressText,
					   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					   XmNbottomWidget, subjectText,
					   XmNbottomOffset, 0,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_WIDGET,
					   XmNrightWidget, subjectText,
					   XmNalignment, XmALIGNMENT_BEGINNING,
					   NULL);

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

    sendMailB = XtVaCreateManagedWidget("Send Mail",
				      xmPushButtonWidgetClass,
				      buttonBox,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 1,
				      NULL);
    XtAddCallback(sendMailB, XmNactivateCallback,
		  sendMailCB, (XtPointer) ms);

    insertFileB = XtVaCreateManagedWidget("Insert File...",
					xmPushButtonWidgetClass,
					buttonBox,
					XmNtopAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_WIDGET,
					XmNleftWidget, sendMailB,
					XmNrightAttachment, XmATTACH_POSITION,
					XmNrightPosition, 2,
					NULL);
    XtAddCallback(insertFileB, XmNactivateCallback,
		  mailLoadFileCB, (XtPointer) ms);

    saveAsB = XtVaCreateManagedWidget("Save As...",
				      xmPushButtonWidgetClass,
				      buttonBox,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_WIDGET,
				      XmNleftWidget, insertFileB,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 3,
				      NULL);
    XtAddCallback(saveAsB, XmNactivateCallback,
		  mailSaveAsCB, (XtPointer) ms);
    
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
		  closeMailEditorCB, (XtPointer) ms);

    
    helpLabel = XtVaCreateManagedWidget("helpLabel", xmLabelWidgetClass, form,
					XmNbottomAttachment, XmATTACH_WIDGET,
					XmNbottomWidget, buttonBox,
					XmNrightAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNalignment, XmALIGNMENT_BEGINNING,
					XmNrecomputeSize, FALSE,
					XmNborderWidth, 1,
					NULL);
    setHelp(title, helpLabel, "Your comments will help us make Viola better for YOU.   Thank you for your time!");
    setHelp(commandLabel, helpLabel, "This is the program that Viola will use to send your letter.");
    setHelp(commandText, helpLabel, "This is the program that Viola will use to send your letter");
    setHelp(addressLabel, helpLabel, "InterNet address that your letter will be sent to.");
    setHelp(addressText, helpLabel, "InterNet address that your letter will be sent to.");
    setHelp(subjectLabel, helpLabel, "Command line option that sets the subject.");
    setHelp(subjectText, helpLabel, "Command line option that sets the subject.");
    setHelp(insertFileB, helpLabel, "Read a text file from the local disk and insert it into the letter.");
    setHelp(saveAsB, helpLabel, "Save the contents of the mail editor to a local file.");
    setHelp(sendMailB, helpLabel, "Send letter using the mail command, email address, and the subject specified above.");
    setHelp(closeB, helpLabel, "Exit the mail editor without sending a letter.");

    
    /*
     * Should keep track of whether or not the data was altered.
     * Pop up a verification dialog if close before save.
     */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNtopWidget, commandFrame);  n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNbottomWidget, helpLabel);  n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNscrollVertical, TRUE);  n++;
    XtSetArg(args[n], XmNresizeWidth, TRUE);  n++;
    XtSetArg(args[n], XmNwidth, 550);  n++;
    XtSetArg(args[n], XmNheight, 300);  n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
    XtSetArg(args[n], XmNwordWrap, TRUE);  n++;
    textEditor = XmCreateScrolledText(form, "textEditor", args, n);
    XtManageChild(textEditor);
    setHelp(textEditor, helpLabel, "Enter the body of your letter here.");

    ms->commandText = commandText;
    ms->addressText = addressText;
    ms->subjectText = subjectText;
    ms->letterText = textEditor;

    localDocViewInfo->editable = TRUE;
    localDocViewInfo->editorDataChanged = 0;
    localDocViewInfo->textEditor = textEditor;

    localDocViewInfo->editorSaveButton = saveAsB;
    XtAddCallback(textEditor, XmNvalueChangedCallback,
		  mailValueChangedCB, (XtPointer) localDocViewInfo);
    
    localDocViewInfo->shellName = makeString("Mail Editor");
    localDocViewInfo->docName = NULL;
    localDocViewInfo->URL = NULL;
    localDocViewInfo->shell = shell;
    localDocViewInfo->cloneID = (long) shell;
    localDocViewInfo->title = title;
    localDocViewInfo->scrollBar = (Widget) 0;
    localDocViewInfo->canvas = textEditor;
    localDocViewInfo->quitDlog = (Widget) 0;
    localDocViewInfo->violaDocViewObj = NULL;
    localDocViewInfo->violaDocToolsObj = NULL;
    localDocViewInfo->violaDocViewName = NULL;
    localDocViewInfo->violaDocToolsName = NULL;
    localDocViewInfo->violaDocViewWindow = (Window) 0;
    localDocViewInfo->violaDocToolsWindow = (Window) 0;
    localDocViewInfo->nHistoryItems = 0;
    localDocViewInfo->historyListWidget = (Widget) 0;

    XtPopup(shell, XtGrabNone);
}


void mailValueChangedCB(textEditor, clientData, callData)
    Widget textEditor;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    dvi->editorDataChanged = 1;
    XtRemoveCallback(textEditor, XmNvalueChangedCallback,
		     mailValueChangedCB, clientData);
}


void closeMailEditorCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    char *answer;
    MailCBStruct *ms = (MailCBStruct *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) ms->dvi;

    if (dvi->editorDataChanged) {
	answer = questionDialog(dvi,
				"There are unsaved changes.",
				"Cancel",
				"Save Changes...",
				"Don't Save Changes",
				"Cancel");
	if (!strcmp(answer, "Save Changes...")) {
	    if (vwSaveAs("Saving letter to a file...\n\nEnter the full path name:",
			 dvi) == CANCELED) {
		return;
	    } else if (dvi->editorDataChanged) {
		dvi->editorDataChanged = 0;
		XtAddCallback(dvi->textEditor, XmNvalueChangedCallback,
			      mailValueChangedCB, (XtPointer) dvi);
	    }
	} else if (!strcmp(answer, "Cancel")) {
	    return;
	}
    }

    free(ms);
    XtDestroyWidget(dvi->shell);
    freeDocViewInfo(dvi);
} 


void mailSaveAsCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    MailCBStruct *ms = (MailCBStruct *) clientData;
    DocViewInfo *dvi = ms->dvi;

    (void) vwSaveAs("Saving letter to a file...\n\nEnter the full path name:",
		    dvi);

    if (dvi->editorDataChanged) {
	dvi->editorDataChanged = 0;
	XtAddCallback(dvi->textEditor, XmNvalueChangedCallback,
		      mailValueChangedCB, (XtPointer) dvi);
    }
}


void sendMailCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    int length;
    char *mailCommand, *address, *subject, *letter, *answer;
    char fullCommand[512], dlogMessage[800];
    FILE *fp;
    MailCBStruct *ms = (MailCBStruct *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) ms->dvi;

    mailCommand = XmTextFieldGetString(ms->commandText);
    address = XmTextFieldGetString(ms->addressText);
    subject = XmTextFieldGetString(ms->subjectText);
    letter = XmTextGetString(ms->letterText);
    length = strlen(mailCommand) + strlen(address) + strlen(subject) + 20;
    sprintf(fullCommand, "%s %s %s", mailCommand, subject, address);
    sprintf(dlogMessage,
	    "Sending your letter using the following command:\n\n%s\n\nIs this OK?",
	    fullCommand);

    answer = questionDialog(dvi,
			    dlogMessage,
			    "OK, Send Letter",
			    "OK, Send Letter",
			    "Cancel",
			    NULL);
    if (!strcmp(answer, "OK, Send Letter")) {
	/*
	 * WARNING WARNING WARNING
	 *
	 * This throws out the old SIGPIPE handler.
	 * If Viola relies on one, this will hose it.
	 */
	signal(SIGPIPE, SIG_IGN);
	fp = popen(fullCommand, "w");
	fputs(letter, fp);
	fputs(".\n", fp);
	pclose(fp);
	signal(SIGPIPE, SIG_DFL);

	dvi->editorDataChanged = 0;
    }

    XtFree(mailCommand);
    XtFree(address);
    XtFree(subject);
    XtFree(letter);
}


void mailLoadFileCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    char *fileName, *newText;
    MailCBStruct *ms = (MailCBStruct *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) ms->dvi;

    fileName = fileSelectionDialog(dvi,
				   "Select the file you wish to insert into your letter:",
				   "The contents of the file you select will be\ninserted into your letter at the text cursor.");
    
    newText = readFile(fileName, dvi);
    XmTextInsert(ms->letterText,
		 XmTextGetInsertionPosition(ms->letterText),
		 newText);
    XtFree(newText);
    XtFree(fileName);
}
