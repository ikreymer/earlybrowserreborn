/*
 * dialog.c
 *
 * Error, information, and utility dialogs.
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

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

#include "vw.h"
#include "dialog.h"


void dialogOK(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    XtDestroyWidget(XtParent((Widget) clientData));
}


void messageDialog(dvip, msg, type)
    DocViewInfo *dvip;
    char *msg;
    int type;
{
    Widget dlog, button;
    Arg args[8];
    int n=0;
    XmString msg_xms;
    
    if (!msg)
	return;

    msg_xms = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);

    XtSetArg(args[n], XmNmessageString, msg_xms);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4); n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
    
    switch (type) {
    case INFO:
	dlog = XmCreateInformationDialog(dvip->shell, "Information", args, n);
	break;
    case WARNING:
	dlog = XmCreateWarningDialog(dvip->shell, "Warning", args, n);
/*	XtVaSetValues(dlog, XmNdialogStyle,
		      XmDIALOG_PRIMARY_APPLICATION_MODAL, NULL);
*/
	break;
    case ERROR:
	dlog = XmCreateErrorDialog(dvip->shell, "Error", args, n);
	XtVaSetValues(dlog, XmNdialogStyle,
		      XmDIALOG_PRIMARY_APPLICATION_MODAL, NULL);
	break;
    default:
	return;
    }

    XmStringFree(msg_xms);

    button = XmMessageBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(button);

    button = XmMessageBoxGetChild(dlog, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild(button);

    XtManageChild(dlog);
}


void modalOK(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    int *donep = (int *) clientData;
    *donep = 1;
}


void modalErrorDialog(dvip, msg)
    DocViewInfo *dvip;
    char *msg;
{
    Widget dlog, button;
    Arg args[8];
    int n=0, done=0;
    XEvent event;
    XmString msg_xms;
    ClientData *cd;
    
    if (!msg)
	return;

    msg_xms = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);

    XtSetArg(args[n], XmNmessageString, msg_xms);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);  n++;
    dlog = XmCreateErrorDialog(dvip->shell, "Error", args, n);
    
    XmStringFree(msg_xms);

    button = XmMessageBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(button);

    button = XmMessageBoxGetChild(dlog, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild(button);

    button = XmMessageBoxGetChild(dlog, XmDIALOG_OK_BUTTON);
    XtAddCallback(button, XmNactivateCallback, modalOK, (XtPointer) &done);
    
    XtManageChild(dlog);

    while(!done) {
	XtAppNextEvent(appCon, &event);
	XtDispatchEvent(&event);
    }

    XtDestroyWidget(dlog);
    XtDestroyWidget(dlog);
}


void promptCancel(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;

    ps->result = NULL;
    *(ps->done) = 1;
}


void promptClearText(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    Widget *textField = (Widget *) clientData;

    XmTextFieldSetString(*textField, "");
}


void promptOK(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;
    XmSelectionBoxCallbackStruct *sbcb = (XmSelectionBoxCallbackStruct *) callData;

    if (!XmStringGetLtoR(sbcb->value, XmSTRING_DEFAULT_CHARSET, &(ps->result)))
	ps->result = NULL;
    *(ps->done) = 1;
}


char *promptDialog(dvip, msg, defaultString)
    DocViewInfo *dvip;
    char *msg, *defaultString;
{
    Widget dlog, button, textField;
    Arg args[12];
    int n=0, done=0;
    XEvent event;
    XmString msg_xms, apply_xms, default_xms;
    struct promptStruct ps;
    XtCallbackRec okCallback[2], applyCallback[2], cancelCallback[2];
    
    if (!msg)
	return(NULL);

    okCallback[0].callback = promptOK;
    okCallback[0].closure = (XtPointer) &ps;
    cancelCallback[0].callback = promptCancel;
    cancelCallback[0].closure = (XtPointer) &ps;
    applyCallback[0].callback = promptClearText;
    applyCallback[0].closure = (XtPointer) &textField;
    okCallback[1].callback = cancelCallback[1].callback =
	applyCallback[1].callback = NULL;
    okCallback[1].closure = cancelCallback[1].closure = 
	applyCallback[1].closure = NULL;

    msg_xms = XmStringCreateLtoR(msg, "largeFont");
    apply_xms = XmStringCreateSimple("Clear Text");

    if (defaultString) {
	default_xms = XmStringCreateSimple(defaultString);
	XtSetArg(args[n], XmNtextString, default_xms);  n++;
    }

    XtSetArg(args[n], XmNselectionLabelString, msg_xms);  n++;
    XtSetArg(args[n], XmNapplyLabelString, apply_xms);  n++;
    XtSetArg(args[n], XmNapplyCallback, applyCallback);  n++;
    XtSetArg(args[n], XmNokCallback, okCallback);  n++;
    XtSetArg(args[n], XmNcancelCallback, cancelCallback);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;
    dlog = XmCreatePromptDialog(dvip->shell, "Printing", args, n);

    textField = XmSelectionBoxGetChild(dlog, XmDIALOG_TEXT);
    XtVaSetValues(textField,
		  XmNcolumns, 50,
		  NULL);
    
    XmStringFree(msg_xms);
    XmStringFree(apply_xms);
    if (defaultString)
	XmStringFree(default_xms);

    button = XmSelectionBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(button);
    button = XmSelectionBoxGetChild(dlog, XmDIALOG_APPLY_BUTTON);
    XtManageChild(button);

    ps.done = &done;
    XtManageChild(dlog);

    while(!done) {
	XtAppNextEvent(appCon, &event);
	XtDispatchEvent(&event);
    }

    XtDestroyWidget(dlog);

    return(ps.result);
}


void questionYes(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;

    ps->result = ps->yesLabel;
    *(ps->done) = 1;
}


void questionNo(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;

    ps->result = ps->noLabel;
    *(ps->done) = 1;
}


void questionCancel(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;

    ps->result = ps->cancelLabel;
    *(ps->done) = 1;
}


char *questionDialog(dvip, question, defaultChoice, yesLabel, noLabel, cancelLabel)
    DocViewInfo *dvip;
    char *question, *defaultChoice, *yesLabel, *noLabel, *cancelLabel;
{
    Widget dlog, button;
    Arg args[16];
    int n=0, done=0;
    XEvent event;
    XmString question_xms, yesLabel_xms, noLabel_xms, cancelLabel_xms;
    struct promptStruct ps;
    XtCallbackRec yesCallback[2], noCallback[2], cancelCallback[2];
    
    if (!question || !defaultChoice || !yesLabel)
	return(NULL);

    yesLabel_xms = XmStringCreateSimple(yesLabel);
    yesCallback[0].callback = questionYes;
    yesCallback[0].closure = (XtPointer) &ps;
    yesCallback[1].callback = NULL;
    yesCallback[1].closure = NULL;
    
    XtSetArg(args[n], XmNokLabelString, yesLabel_xms);  n++;
    XtSetArg(args[n], XmNokCallback, yesCallback);  n++;

    ps.yesLabel = yesLabel;
    /* This should probably be a string copy, but since I know that
     *  the dialog is modal, control doesn't get back to Viola until
     *  after the user responds to the dialog anyway.
     */
    
    if (noLabel) {
	noLabel_xms = XmStringCreateSimple(noLabel);
	noCallback[0].callback = questionNo;
	noCallback[0].closure = (XtPointer) &ps;
	noCallback[1].callback = NULL;
	noCallback[1].closure = NULL;
	
	XtSetArg(args[n], XmNcancelCallback, noCallback);  n++;
	XtSetArg(args[n], XmNcancelLabelString, noLabel_xms);  n++;

	ps.noLabel = noLabel;
	/* This should probably be a string copy, but since I know that
	 *  the dialog is modal, control doesn't get back to Viola until
	 *  after the user responds to the dialog anyway.
	 */
    }
    if (cancelLabel) {
	cancelLabel_xms = XmStringCreateSimple(cancelLabel);

	cancelCallback[0].callback = questionCancel;
	cancelCallback[0].closure = (XtPointer) &ps;
	cancelCallback[1].callback = NULL;
	cancelCallback[1].closure = NULL;
	
	XtSetArg(args[n], XmNhelpCallback, cancelCallback);  n++;
	XtSetArg(args[n], XmNhelpLabelString, cancelLabel_xms);  n++;

	ps.cancelLabel = cancelLabel;
	/* This should probably be a string copy, but since I know that
	 *  the dialog is modal, control doesn't get back to Viola until
	 *  after the user responds to the dialog anyway.
	 */
    }

    if (defaultChoice) {
	if (cancelLabel && !strcmp(defaultChoice, cancelLabel)) {
	    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_HELP_BUTTON);  n++;
	} else if (noLabel && !strcmp(defaultChoice, noLabel)) {
	    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON);  n++;
	} else if (yesLabel && !strcmp(defaultChoice, yesLabel)) {
	    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_OK_BUTTON);  n++;
	}
    } else {
	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE);  n++;
    }	
	    
    question_xms = XmStringCreateLtoR(question, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[n], XmNmessageString, question_xms);  n++;
    
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;
    XtSetArg(args[n], XmNautoUnmanage, FALSE);  n++;
    dlog = XmCreateQuestionDialog(dvip->shell, "Question", args, n);
    
    XmStringFree(question_xms);
    XmStringFree(yesLabel_xms);
    if (noLabel) {
	XmStringFree(noLabel_xms);
    } else {
	button = XmMessageBoxGetChild(dlog, XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild(button);
    }
    if (cancelLabel) {
	XmStringFree(cancelLabel_xms);
    } else {
	button = XmMessageBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild(button);
    }

    ps.done = &done;

    XtManageChild(dlog);

    while(!done) {
	XtAppNextEvent(appCon, &event);
	XtDispatchEvent(&event);
    }

    XtUnmanageChild(dlog);
    XtDestroyWidget(dlog);

    return(ps.result);
}


void infoDialogMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    
    if (argc < 3)
	return;

    /* Check cloneID */
    if (dvip->cloneID != atol(arg[1]))
	return;

    infoDialog(dvip, arg[2]);
}


void warningDialogMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;

    if (argc < 3)
	return;

    /* Check cloneID */
    if (dvip->cloneID != atol(arg[1]))
	return;

    warningDialog(dvip, arg[2]);
}


void errorDialogMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;

    if (argc < 3)
	return;

    /* Check cloneID */
    if (dvip->cloneID != atol(arg[1]))
	return;

    errorDialog(dvip, arg[2]);
}


void modalErrorDialogMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    
    if (argc < 3)
	return;

    /* Check cloneID */
    if (dvip->cloneID != atol(arg[1]))
	return;

    modalErrorDialog(dvip, arg[2]);
}


/*
 * arg[0] = "promptDialog"
 * arg[1] = cloneID
 * arg[2] = promptID
 * arg[3] = prompt message
 * arg[4] = defaultString
 */
void promptDialogMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    char *result=NULL, *defaultString=NULL;
    DocViewInfo *dvip = (DocViewInfo *) clientData;

    if (argc < 4)
	return;

    /* Check cloneID */
    if (dvip->cloneID != atol(arg[1]))
	return;

    if (argc > 4)
	defaultString = arg[4];

    result = promptDialog(dvip, arg[3], defaultString);

    /*
     * Transmit answer to Viola using propmtID.
     */
}


/*
 * questionDialog(): posts a dialog with three buttons.
 *
 * arg[0] = "promptDialog"
 * arg[1] = cloneID
 * arg[2] = promptID
 * arg[3] = question
 * arg[4] = defaultChoice
 * arg[5] = yesLabel
 * arg[6] = noLabel
 * arg[7] = cancelLabel
 */
void questionDialogMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    char *result=NULL;
    char *defaultChoice=NULL, *yesLabel=NULL, *noLabel=NULL, *cancelLabel=NULL;
    DocViewInfo *dvip = (DocViewInfo *) clientData;

    if (argc < 4)
	return;

    /* Check cloneID */
    if (dvip->cloneID != atol(arg[1]))
	return;

    if (argc > 4)
	defaultChoice = arg[4];
    if (argc > 5)
	yesLabel = arg[5];
    if (argc > 6)
	noLabel = arg[6];
    if (argc > 7)
	cancelLabel = arg[7];

    result = questionDialog(dvip, arg[3], defaultChoice,
			    yesLabel, noLabel, cancelLabel);

    /*
     * Transmit answer to Viola using propmtID.
     */
    /*
    sendMessage1N1strByName(dvip->violaDocViewName,
                            "yesNoResult",
                            // promptID as an int //,
	                    ps.result);
    */
}


void printYes(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;

    *(ps->done) = 1;
    ps->result = "yes";
}


void printCancel(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    struct promptStruct *ps = (struct promptStruct *) clientData;

    *(ps->done) = 1;
    ps->result = NULL;
}


void printDialog(dvip, docName)
    DocViewInfo *dvip;
    char *docName;
{
    Widget dlog, label, commandLabel, commandText, optionLabel, optionText,
           printButton, cancelButton, frame, form;
    Arg args[8];
    int n=0, done=0;
    char *printer, *printCommand, *printOptions;
    char labelString[256], optionDefault[256], printString[512];
    XEvent event;
    XmString labelStringXMS;
    struct promptStruct ps;
    

    XtSetArg(args[n], XmNhorizontalSpacing, 4);  n++;
    XtSetArg(args[n], XmNverticalSpacing, 4);  n++;
    XtSetArg(args[n], XmNfractionBase, 100);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;
    dlog = XmCreateFormDialog(dvip->shell, "Printing", args, n);


    frame = XtVaCreateManagedWidget("frame",
				    xmFrameWidgetClass,
				    dlog,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNtopOffset, 4,
				    XmNbottomOffset, 4,
				    XmNleftOffset, 4,
				    XmNrightOffset, 4,
				    XmNmarginWidth, 4,
				    XmNmarginHeight, 4,
				    XmNshadowThickness, 0,
				    NULL);

    form = XtVaCreateManagedWidget("form", xmFormWidgetClass, frame, NULL);

    
    if (docName && strlen(docName) < 256)
	sprintf(labelString, "Printing %s", docName);
    else
	sprintf(labelString, "Printing");
    
    labelStringXMS = XmStringCreateLtoR(labelString, "largeFont");
    label = XtVaCreateManagedWidget("Print",
				    xmLabelWidgetClass,
				    form,
				    XmNalignment, XmALIGNMENT_BEGINNING,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNlabelString, labelStringXMS,
				    NULL);
    XmStringFree(labelStringXMS);
    
    commandLabel = XtVaCreateManagedWidget("Print Command:",
					   xmLabelWidgetClass,
					   form,
					   XmNalignment, XmALIGNMENT_BEGINNING,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, label,
					   XmNtopOffset, 6,
					   XmNleftAttachment, XmATTACH_FORM,
					   NULL);
    
    commandText = XtVaCreateManagedWidget("commandText",
					  xmTextFieldWidgetClass,
					  form,
					  XmNalignment, XmALIGNMENT_BEGINNING,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, label,
					  XmNtopOffset, 6,
					  XmNleftAttachment, XmATTACH_WIDGET,
					  XmNleftWidget, commandLabel,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNvalue, PRINT_COMMAND,
					  NULL);
    
    optionLabel = XtVaCreateManagedWidget("Command-Line Options:",
					  xmLabelWidgetClass,
					  form,
					  XmNalignment, XmALIGNMENT_BEGINNING,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, commandText,
					  XmNleftAttachment, XmATTACH_FORM,
					  NULL);
    
    printer = getenv("PRINTER");
    if (printer)
	sprintf(optionDefault, "-P%s", printer);
    else
	sprintf(optionDefault, "-Plp");
    
    optionText = XtVaCreateManagedWidget("commandText",
					 xmTextFieldWidgetClass,
					 form,
					 XmNalignment, XmALIGNMENT_BEGINNING,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, commandText,
					 XmNleftAttachment, XmATTACH_WIDGET,
					 XmNleftWidget, optionLabel,
					 XmNrightAttachment, XmATTACH_FORM,
					 XmNvalue, optionDefault,
					 NULL);
    
    printButton = XtVaCreateManagedWidget("Print",
					  xmPushButtonWidgetClass,
					  form,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, optionText,
					  XmNtopOffset, 6,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_POSITION,
					  XmNrightPosition, 50,
					  NULL);
    XtAddCallback(printButton, XmNactivateCallback,
		  printYes, (XtPointer) &ps);
    
    cancelButton = XtVaCreateManagedWidget("Cancel",
					   xmPushButtonWidgetClass,
					   form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, optionText,
					   XmNtopOffset, 6,
					   XmNleftAttachment, XmATTACH_POSITION,
					   XmNleftPosition, 51,
					   XmNrightAttachment, XmATTACH_FORM,
					   NULL);
    XtAddCallback(cancelButton, XmNactivateCallback,
		  printCancel, (XtPointer) &ps);
    
    ps.done = &done;
    XtManageChild(dlog);
    
    while(!done) {
	XtAppNextEvent(appCon, &event);
	XtDispatchEvent(&event);
    }

    if (ps.result) {
	printCommand = XmTextFieldGetString(commandText);
	printOptions = XmTextFieldGetString(optionText);

	if (printCommand) {
	    sprintf(printString, "%s %s", printCommand, printOptions);
	    sendMessage1N1str(dvip->violaDocViewObj,
				    "printDoc",
				    printString);
	    fprintf(stderr, printString);
	}

	if (printCommand)
	    free(printCommand);
	if (printOptions)
	    free(printOptions);
    }

    XtDestroyWidget(dlog);
}


static char *protocols[] = {
    "http",
    "file",
    "gopher",
    "wais",
    "ftp",
};
#define nprotocols 5

struct urlDlogStruct {
    Widget dlog;
    Widget urlText;
    Widget protocolText;
    Widget machineText;
    Widget fileText;
    char *data;
    DocViewInfo *dvi;
};


/*
 * BUGGY:
 *
 * This routine assumes that all URL's are either just a file name or are
 * of the format "protocol://machine/file".
 *
 * However, http: may have a ":portnumber" immediately following the machine
 * name.  file: may or may not have a machine name.
 *
 */
void urlDlogParseURL(uds)
    struct urlDlogStruct *uds;
{
    char *protocol, *machine, *file;
    char *newURL = XmTextFieldGetString(uds->urlText);
    char hostname[256];
    char changedURL[256];

    if (!newURL || !strlen(newURL))
	return;

    protocol = newURL;
    if (machine = strchr(newURL, (int) ':')) {
	int i=0;
	
	*(machine++) = '\0';
	if (*machine == '/')
	    machine++;
	if (*machine == '/')
	    machine++;
	while(*machine != '/')
	    hostname[i++] = *(machine++);
	hostname[i] = '\0';
	file = machine;
	machine = hostname;
    } else {
	gethostname(hostname, 256);
	protocol = "file";
	machine = hostname;
	file = newURL;

	sprintf(changedURL, "file://%s%s", machine, newURL);
	XmTextFieldSetString(uds->urlText, changedURL);
    }

    XmTextFieldSetString(uds->protocolText, protocol);
    XmTextFieldSetString(uds->machineText, machine);
    XmTextFieldSetString(uds->fileText, file);

    XtFree(newURL);
}


void urlDlogConstructURL(uds)
    struct urlDlogStruct *uds;
{
    int length;
    char *ptr, *psave, *msave, *fsave;
    char *protocol = XmTextFieldGetString(uds->protocolText);
    char *machine = XmTextFieldGetString(uds->machineText);
    char *file = XmTextFieldGetString(uds->fileText);
    char newURL[1024];

    psave = protocol;
    msave = machine;
    fsave = file;

    if (!protocol) protocol = "";
    if (!machine) machine = "";
	
    if (file && strlen(file)) {
	/*
	 * Remove possible "://" at end of protocol string.
	 */
	length = strlen(protocol);
	if (length) {
	    ptr = &protocol[length-1];
	    if (*ptr == ':') {
		*ptr = '\0';
	    } else if (*ptr == '/') {
		ptr--;
		if (*ptr == '/')
		    ptr--;
		if (*ptr == ':')
		    *ptr = '\0';
	    }
	}
	
	/*
	 * Remove possible "://" from beginning and remove possible "/"
	 * from end of machine string.
	 */
	length = strlen(machine);
	if (length) {
	    if (machine[length-1] == '/')
		machine[length-1] = '\0';
	    if (*machine == ':')
		machine++;
	    if (*machine == '/')
		machine++;
	    if (*machine == '/')
		machine++;
	}
	
	/*
	 * Make sure file starts with a '/'.
	 */
	if (*file == '/') {
	    sprintf(newURL, "%s://%s%s", protocol, machine, file);
	} else {
	    sprintf(newURL, "%s://%s/%s", protocol, machine, file);
	}
	
	XmTextFieldSetString(uds->urlText, newURL);
    } else {
	XmTextFieldSetString(uds->urlText, "/");
    }
    
    if (psave) XtFree(psave);
    if (msave) XtFree(msave);
    if (fsave) XtFree(fsave);
}


void setURLProtocol(protocolWidget, clientData, callData)
    Widget protocolWidget;
    XtPointer clientData, callData;
{
    struct urlDlogStruct *uds = (struct urlDlogStruct *) clientData;
    char *widgetName = XtName(protocolWidget);
    char *protocol;
    
    if (!strcmp("protocolText", widgetName)) {
	protocol = XmTextFieldGetString(protocolWidget);
	
	/*
	 * Make sure there is a "://" at the end of the string.
	 */
    } else {
	protocol = malloc(strlen(widgetName)+4);
	sprintf(protocol, "%s://", widgetName);
	XmTextFieldSetString(uds->protocolText, widgetName);
    }

    /*
     * Set the protocol part in urlText.
     */

    free(protocol);
}
    

void setURLMachine(machineText, clientData, callData)
    Widget machineText;
    XtPointer clientData, callData;
{
}
    

void setURLFile(fileText, clientData, callData)
    Widget fileText;
    XtPointer clientData, callData;
{
}
    

void openURL(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct urlDlogStruct *uds = (struct urlDlogStruct *) clientData;
    char *url = XmTextFieldGetString(uds->urlText);

    sendMessage1N1str(uds->dvi->violaDocViewObj, "show", url);

    XtDestroyWidget(uds->dlog);
    free(uds);
    free(url);
}
    

void clearURL(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct urlDlogStruct *uds = (struct urlDlogStruct *) clientData;
    XmTextFieldSetString(uds->urlText, "");
}
    

void cancelURL(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct urlDlogStruct *uds = (struct urlDlogStruct *) clientData;
    char *url = XmTextFieldGetString(uds->urlText);

    XtDestroyWidget(uds->dlog);
    free(uds);
    free(url);
}
    

void openURLDialog(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    Widget dlog, frame, form, label, protocolForm, protocolLabel, menu,
           optionMenu, protocolText, machineForm, machineLabel, machineText, 
           fileForm, fileLabel, fileText, urlForm, urlLabel, urlText, 
           openButton, cancelButton, button;
    Arg args[12];
    int i, n=0, done=0;
    char labelString[256], optionDefault[256], printString[512];
    XEvent event;
    XmString labelStringXMS, protocolXMS;
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    struct urlDlogStruct *uds;


    uds = (struct urlDlogStruct *) malloc(sizeof(struct urlDlogStruct));
    uds->dvi = dvi;

    n = 0;
    XtSetArg(args[n], XmNhorizontalSpacing, 8);  n++;
    XtSetArg(args[n], XmNverticalSpacing, 8);  n++;
    XtSetArg(args[n], XmNfractionBase, 100);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    /*    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;*/
    XtSetArg(args[n], XmNautoUnmanage, FALSE);  n++;
    form = XmCreateFormDialog(dvi->shell, "URL", args, n);
    uds->dlog = XtParent(form);

    labelStringXMS = XmStringCreateLtoR("Open Document via URL (Universal Resource Locator)",
					"largeFont");
    label = XtVaCreateManagedWidget("mainLabel",
				    xmLabelWidgetClass,
				    form,
				    XmNalignment, XmALIGNMENT_BEGINNING,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNlabelString, labelStringXMS,
				    NULL);
    XmStringFree(labelStringXMS);

    
    /* URL Protocol */
    protocolForm = XtVaCreateManagedWidget("urlForm", 
					   xmFormWidgetClass,
					   form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, label,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNhorizontalSpacing, 4,
					   XmNverticalSpacing, 4,
					   XmNshadowThickness, 2,
					   XmNshadowType, XmSHADOW_ETCHED_IN,
					   NULL);
    n = 0;
    menu = XmCreatePulldownMenu(protocolForm, "protocolMenu", args, n);
    for (i=0; i<nprotocols; i++) {
	button = XmCreatePushButtonGadget(menu, protocols[i], NULL, 0);
	XtManageChild(button);
	XtAddCallback(button, XmNactivateCallback,
		      setURLProtocol, (XtPointer) uds);
    }
    
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNsubMenuId, menu);  n++;
    protocolXMS = XmStringCreateSimple("Protocol:");
    XtSetArg(args[n], XmNlabelString, protocolXMS);  n++;
    optionMenu = XmCreateOptionMenu(protocolForm, "protocolOptions", args, n);
    XtManageChild(optionMenu);
    XmStringFree(protocolXMS);

    protocolText = XtVaCreateManagedWidget("protocolText",
					   xmTextFieldWidgetClass,
					   protocolForm,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, optionMenu,
					   XmNbottomAttachment, XmATTACH_FORM,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
					   XmNrightWidget, optionMenu,
					   XmNrightOffset, 0,
					   NULL);
    XtAddCallback(protocolText, XmNactivateCallback,
		  setURLProtocol, (XtPointer) uds);
    XtAddCallback(protocolText, XmNlosingFocusCallback,
		  setURLProtocol, (XtPointer) uds);
    uds->protocolText = protocolText;

    
    /* Machine Name */
    machineForm = XtVaCreateManagedWidget("machineForm",
					  xmFormWidgetClass,
					  form,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, label,
					  XmNleftAttachment, XmATTACH_WIDGET,
					  XmNleftWidget, protocolForm,
					  XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					  XmNbottomWidget, protocolForm,
					  XmNbottomOffset, 0,
					  XmNhorizontalSpacing, 4,
					  XmNverticalSpacing, 4,
					  XmNshadowThickness, 2,
					  XmNshadowType, XmSHADOW_ETCHED_IN,
					  NULL);
					  
    machineText = XtVaCreateManagedWidget("machineNameText",
					  xmTextFieldWidgetClass,
					  machineForm,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  NULL);
    XtAddCallback(machineText, XmNactivateCallback,
		  setURLMachine, (XtPointer) uds);
    XtAddCallback(machineText, XmNlosingFocusCallback,
		  setURLMachine, (XtPointer) uds);
    uds->machineText = machineText;

    machineLabel = XtVaCreateManagedWidget("Machine Name Component:",
					   xmLabelWidgetClass,
					   machineForm,
					   XmNalignment, XmALIGNMENT_BEGINNING,
					   XmNtopAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_WIDGET,
					   XmNbottomWidget, machineText,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   NULL);

    
    /* File Name */
    fileForm = XtVaCreateManagedWidget("fileForm",
				       xmFormWidgetClass,
				       form,
				       XmNhorizontalSpacing, 4,
				       XmNverticalSpacing, 4,
				       XmNshadowThickness, 2,
				       XmNshadowType, XmSHADOW_ETCHED_IN,
				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget, label,
				       XmNleftAttachment, XmATTACH_WIDGET,
				       XmNleftWidget, machineForm,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
				       XmNbottomWidget, machineForm,
				       XmNbottomOffset, 0,
				       NULL);
    fileText = XtVaCreateManagedWidget("fileText",
				       xmTextFieldWidgetClass,
				       fileForm,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       NULL);
    XtAddCallback(fileText, XmNactivateCallback,
		  setURLFile, (XtPointer) uds);
    XtAddCallback(fileText, XmNlosingFocusCallback,
		  setURLFile, (XtPointer) uds);

    fileLabel = XtVaCreateManagedWidget("File Name Component:",
					xmLabelWidgetClass,
					fileForm,
					XmNalignment, XmALIGNMENT_BEGINNING,
					XmNtopAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_WIDGET,
					XmNbottomWidget, fileText,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    
    /* URL Text */
    urlForm = XtVaCreateManagedWidget("urlForm",
				      xmFormWidgetClass,
				      form,
				      XmNhorizontalSpacing, 4,
				      XmNverticalSpacing, 4,
				      XmNshadowThickness, 2,
				      XmNshadowType, XmSHADOW_ETCHED_IN,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, protocolForm,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      NULL);
    urlLabel = XtVaCreateManagedWidget("Complete URL:   Protocol :// MachineName / FileName",
				       xmLabelWidgetClass,
				       urlForm,
				       XmNalignment, XmALIGNMENT_BEGINNING,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       NULL);
    urlText = XtVaCreateManagedWidget("urlText",
				      xmTextFieldWidgetClass,
				      urlForm,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, urlLabel,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      NULL);
    XtAddCallback(urlText, XmNactivateCallback,
		  openURL, (XtPointer) uds);
    uds->urlText = urlText;


    /* Open Button */
    openButton = XtVaCreateManagedWidget("Open URL", 
					 xmPushButtonWidgetClass,
					 form,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, urlForm,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_POSITION,
					 XmNrightPosition, 50,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);
    XtAddCallback(openButton, XmNactivateCallback,
		  openURL, (XtPointer) uds);

    cancelButton = XtVaCreateManagedWidget("Cancel", 
					   xmPushButtonWidgetClass,
					   form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, urlForm,
					   XmNleftAttachment, XmATTACH_WIDGET,
					   XmNleftWidget, openButton,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_FORM,
					   NULL);
    XtAddCallback(cancelButton, XmNactivateCallback,
		  cancelURL, (XtPointer) uds);

    XmTextFieldSetString(uds->urlText, dvi->URL);
    urlDlogParseURL(uds);

    XtManageChild(form);
    XtPopup(XtParent(form), XtGrabNone);
}


/*
 * SIMPLE URL DIALOG
 */
void openSimpleURLDialog(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    Widget dlog, frame, form, label, urlText, 
           openButton, clearButton, cancelButton;
    Arg args[12];
    int i, n=0, done=0;
    char labelString[256], optionDefault[256], printString[512];
    XEvent event;
    XmString labelStringXMS;
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    struct urlDlogStruct *uds;


    uds = (struct urlDlogStruct *) malloc(sizeof(struct urlDlogStruct));
    uds->dvi = dvi;

    n = 0;
    XtSetArg(args[n], XmNhorizontalSpacing, 8);  n++;
    XtSetArg(args[n], XmNverticalSpacing, 8);  n++;
    XtSetArg(args[n], XmNfractionBase, 100);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    /*XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;*/
    XtSetArg(args[n], XmNautoUnmanage, FALSE);  n++;
    form = XmCreateFormDialog(dvi->shell, "URL", args, n);
    uds->dlog = XtParent(form);

    labelStringXMS = XmStringCreateLtoR("Open Document via URL (Universal Resource Locator)",
					"largeFont");
    label = XtVaCreateManagedWidget("mainLabel",
				    xmLabelWidgetClass,
				    form,
				    XmNalignment, XmALIGNMENT_BEGINNING,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNlabelString, labelStringXMS,
				    NULL);
    XmStringFree(labelStringXMS);

    
    urlText = XtVaCreateManagedWidget("urlText",
				      xmTextFieldWidgetClass,
				      form,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, label,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNcolumns, 50,
				      NULL);
    XtAddCallback(urlText, XmNactivateCallback, openURL, (XtPointer) uds);
    uds->urlText = urlText;


    /* Open Button */
    openButton = XtVaCreateManagedWidget("Open URL", 
					 xmPushButtonWidgetClass,
					 form,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, urlText,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_POSITION,
					 XmNrightPosition, 33,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);
    XtAddCallback(openButton, XmNactivateCallback, openURL, (XtPointer) uds);

    
    /* Clear Button */
    clearButton = XtVaCreateManagedWidget("Clear", 
					 xmPushButtonWidgetClass,
					 form,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, urlText,
					 XmNleftAttachment, XmATTACH_WIDGET,
					 XmNleftWidget, openButton,
					 XmNrightAttachment, XmATTACH_POSITION,
					 XmNrightPosition, 66,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);
    XtAddCallback(clearButton, XmNactivateCallback, clearURL, (XtPointer) uds);

    
    /* Cancel Button */
    cancelButton = XtVaCreateManagedWidget("Cancel", 
					   xmPushButtonWidgetClass,
					   form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, urlText,
					   XmNleftAttachment, XmATTACH_WIDGET,
					   XmNleftWidget, clearButton,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_FORM,
					   NULL);
    XtAddCallback(cancelButton, XmNactivateCallback, cancelURL, (XtPointer) uds);

    XmTextFieldSetString(uds->urlText, dvi->URL);

    XtManageChild(form);
    XtPopup(XtParent(form), XtGrabNone);
}


void simpleOpenURLDialog(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    char *url = promptDialog(dvi,
			     "Enter the URL you wish to open:",
			     dvi->URL);

    if (url) sendMessage1N1str(dvi->violaDocViewObj, "show", url);
}

void openURLInSelectionBuffer(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1(dvi->violaDocViewObj, "show_URL_in_selectionBuffer");
}


typedef struct fileSelectStruct {
    int *done;
    char *fileName;
    DocViewInfo *dvi;
} FileSelectStruct;


void fileSelectCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    FileSelectStruct *fss = (FileSelectStruct *) clientData;
    XmFileSelectionBoxCallbackStruct *fscb = (XmFileSelectionBoxCallbackStruct *) callData;

    XmStringGetLtoR(fscb->value, XmSTRING_DEFAULT_CHARSET, &(fss->fileName));
    *(fss->done) = 1;
}


void fileCancelCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    FileSelectStruct *fss = (FileSelectStruct *) clientData;
    *(fss->done) = 1;
}


void fileHelpCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    
    infoDialog((DocViewInfo *) cd->shellInfo,
	       (char *) cd->data);
}


char *fileSelectionDialog(dvi, message, helpMsg)
    DocViewInfo *dvi;
    char *message, *helpMsg;
{
    Widget dlog, textField, button;
    int n, done=0;
    FileSelectStruct fss;
    ClientData cd;
    Arg args[8];
    XmString xms;
    XEvent event;

    fss.done = &done;
    fss.fileName = NULL;
    fss.dvi = dvi;
    
    xms = XmStringCreateSimple(message);
    n = 0;
    XtSetArg(args[n], XmNselectionLabelString, xms); n++;
    XtSetArg(args[n], XmNshadowThickness, 4); n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
    XtSetArg(args[n], XmNwordWrap, TRUE); n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;
    dlog = XmCreateFileSelectionDialog(dvi->shell,
				       "File Selection",
				       args, n);
    XmStringFree(xms);
    XtAddCallback(dlog, XmNokCallback,
		  fileSelectCB, (XtPointer) &fss);

    textField = XmFileSelectionBoxGetChild(dlog, XmDIALOG_TEXT);
    XtVaSetValues(textField, XmNcolumns, 50, NULL);
    
    button = XmFileSelectionBoxGetChild(dlog, XmDIALOG_CANCEL_BUTTON);
    XtAddCallback(button, XmNactivateCallback,
		  fileCancelCB, (XtPointer) &fss);
    
    button = XmFileSelectionBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
    if (helpMsg) {
	cd.data = (void *) helpMsg;
	cd.shellInfo = (void *) dvi;
	XtVaSetValues(button, XmNautoUnmanage, FALSE, NULL);
	XtAddCallback(button, XmNactivateCallback,
		      fileHelpCB, (XtPointer) &cd);
    } else {
	XtUnmanageChild(button);
    }
    
    XtManageChild(dlog);

    while (!done) {
	XtAppNextEvent(appCon, &event);
	XtDispatchEvent(&event);
    }

    XtDestroyWidget(dlog);
    return(fss.fileName);
}


/* Simple Line Entry 
 */
void LineEntry_okCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct lentryDlogStruct *lds = (struct lentryDlogStruct *) clientData;
    char *lineText = XmTextFieldGetString(lds->lineText);

    lds->data = lineText; /*!!! Responsibility of caller to free */
    lds->okCB(lds);

    XtDestroyWidget(lds->dlog);
    free(lds);
}

void LineEntry_clearCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct lentryDlogStruct *lds = (struct lentryDlogStruct *) clientData;
    XmTextFieldSetString(lds->lineText, "");
}
    
void LineEntry_cancelCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    struct lentryDlogStruct *lds = (struct lentryDlogStruct *) clientData;
    char *lineText = XmTextFieldGetString(lds->lineText);

    XtDestroyWidget(lds->dlog);
    free(lds);
    free(lineText);
}

/*
 * SIMPLE LINE ENTRY DIALOG
 */
char *openSimpleLineEntryDialog(dvi, dialogLabel, defaultAns, okCB)
     DocViewInfo *dvi;
     char *dialogLabel, *defaultAns;
     void (*okCB)();
{
    Widget dlog, frame, form, label, lineText, 
           LineEntry_okButton, LineEntry_clearButton, LineEntry_cancelButton;
    Arg args[12];
    int i, n=0, done=0;
    XEvent event;
    XmString labelStringXMS, defaultStringXMS;

    struct lentryDlogStruct *lds;

    lds = (struct lentryDlogStruct *) malloc(sizeof(struct lentryDlogStruct));
    lds->dvi = dvi;
    lds->okCB = okCB;

    n = 0;
    XtSetArg(args[n], XmNhorizontalSpacing, 8);  n++;
    XtSetArg(args[n], XmNverticalSpacing, 8);  n++;
    XtSetArg(args[n], XmNfractionBase, 100);  n++;
    XtSetArg(args[n], XmNshadowThickness, 4);  n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT);  n++;
    /*XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);  n++;*/
    XtSetArg(args[n], XmNautoUnmanage, FALSE);  n++;
    form = XmCreateFormDialog(dvi->shell, "LineEntry", args, n);
    lds->dlog = XtParent(form);

    labelStringXMS = XmStringCreateLtoR(dialogLabel, "largeFont");
    label = XtVaCreateManagedWidget("mainLabel",
				    xmLabelWidgetClass, form,
				    XmNalignment, XmALIGNMENT_BEGINNING,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNlabelString, labelStringXMS,
				    NULL);
     XmStringFree(labelStringXMS);

     lineText = XtVaCreateManagedWidget("lineText",
				      xmTextFieldWidgetClass, form,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, label,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNvalue, defaultAns,
				      XmNcolumns, 50,
				      NULL);
/*     defaultStringXMS = XmStringCreateLtoR(defaultAns, "largeFont");
    XmStringFree(defaultStringXMS);
*/
    XtAddCallback(lineText, XmNactivateCallback, 
		  LineEntry_okCB, (XtPointer) lds);
    lds->lineText = lineText;

    /* Ok Button */
    LineEntry_okButton = XtVaCreateManagedWidget("OK", 
					 xmPushButtonWidgetClass, form,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, lineText,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_POSITION,
					 XmNrightPosition, 33,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);
    XtAddCallback(LineEntry_okButton, XmNactivateCallback, LineEntry_okCB, 
		  (XtPointer) lds);

    
    /* Clear Button */
    LineEntry_clearButton = XtVaCreateManagedWidget("Clear", 
					 xmPushButtonWidgetClass, form,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, lineText,
					 XmNleftAttachment, XmATTACH_WIDGET,
					 XmNleftWidget, LineEntry_okButton,
					 XmNrightAttachment, XmATTACH_POSITION,
					 XmNrightPosition, 66,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);
    XtAddCallback(LineEntry_clearButton, XmNactivateCallback, 
		  LineEntry_clearCB, (XtPointer) lds);

    
    /* Cancel Button */
    LineEntry_cancelButton = XtVaCreateManagedWidget("Cancel", 
					   xmPushButtonWidgetClass, form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, lineText,
					   XmNleftAttachment, XmATTACH_WIDGET,
					   XmNleftWidget, LineEntry_clearButton,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_FORM,
					   NULL);
    XtAddCallback(LineEntry_cancelButton, XmNactivateCallback, 
		  LineEntry_cancelCB, (XtPointer) lds);

    XtManageChild(form);
    XtPopup(XtParent(form), XtGrabNone);
}

