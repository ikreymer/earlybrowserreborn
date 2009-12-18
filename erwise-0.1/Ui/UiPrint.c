static char *rcsid = "$Id$";

#include "UiIncludes.h"


static Widget uicreateprintformdialog();
static Widget uicreateprintcmd(Widget formwdg);
static Widget
 uicreateprintmargin(Widget formwdg, Widget topwdg,
		      char *name, char *labeltext, int pos);
static Widget uicreateprintseparator(Widget formwdg, Widget topwdg);
static Widget uicreateprintwidth(Widget formwdg, Widget topwdg);
static Widget uicreateprintfile(Widget formwdg, Widget topwdg);
static void uicreateprintbuttons(Widget formwdg, Widget topwdg);
static void uiprintmargincb(char *address, HText_t * htext,
			     HTextObject_t * htextobject,
			     void *parameter);
static void uiprintwidthcb(char *address, HText_t * htext,
			    HTextObject_t * htextobject,
			    void *parameter);
static void uiprintbuttoncb(char *address, HText_t * htext,
			     HTextObject_t * htextobject,
			     void *parameter);


static uiActionData_t uiactiondata[8] =
{
    {"LArrowDown", (uiPage_t *) NULL},
    {"LArrowUp", (uiPage_t *) NULL},
    {"RArrowDown", (uiPage_t *) NULL},
    {"RArrowUp", (uiPage_t *) NULL},
    {"TArrowDown", (uiPage_t *) NULL},
    {"TArrowUp", (uiPage_t *) NULL},
    {"BArrowDown", (uiPage_t *) NULL},
    {"BArrowUp", (uiPage_t *) NULL}
};


static Widget uiprinttextwidget[] =
{
    (Widget) NULL,
    (Widget) NULL,
    (Widget) NULL,
    (Widget) NULL
};


int UiDisplayPrintDialog(type)
int type;
{
    uiPrintGfx_t *printgfx = &uiTopLevel.PrintGfx;
    Widget separatorwdg, tmpwdg;


    if (printgfx->FormWdg) {
	XtMapWidget(XtParent(printgfx->FormWdg));

	return UI_OK;
    }
    printgfx->FormWdg = uicreateprintformdialog();

    printgfx->PrintCmdWdg = uicreateprintcmd(printgfx->FormWdg);
    printgfx->PrintFileWdg = uicreateprintfile(printgfx->FormWdg,
					       printgfx->PrintCmdWdg);

    printgfx->LeftMarginWdg = uicreateprintmargin(printgfx->FormWdg,
						  printgfx->PrintFileWdg,
						  "PrintLeft", "Left",
						  UI_LEFT);
    printgfx->RightMarginWdg =
	uicreateprintmargin(printgfx->FormWdg, printgfx->LeftMarginWdg,
			    "PrintRight", "Right", UI_LEFT);
    printgfx->TopMarginWdg = uicreateprintmargin(printgfx->FormWdg,
						 printgfx->PrintFileWdg,
					    "PrintTop", "Top", UI_RIGHT);
    printgfx->BottomMarginWdg =
	uicreateprintmargin(printgfx->FormWdg, printgfx->TopMarginWdg,
			    "PrintBottom", "Bottom", UI_RIGHT);

    tmpwdg = uicreateprintwidth(printgfx->FormWdg, printgfx->BottomMarginWdg);

    separatorwdg = uicreateprintseparator(printgfx->FormWdg, tmpwdg);
    uicreateprintbuttons(printgfx->FormWdg, separatorwdg);
    XtManageChild(printgfx->FormWdg);
    XtRealizeWidget(XtParent(printgfx->FormWdg));

    return UI_OK;
}


void uiPrintUpdateDialog()
{
    if (uiTopLevel.PrintGfx.FormWdg) {
	if (uiPageInfo.CurrentPage) {
	} else
	    XtUnmapWidget(XtParent(uiTopLevel.PrintGfx.FormWdg));
    }
}


static Widget
 uicreateprintformdialog()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("Print",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_PRINT_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "Print", args, nargs);

    return formwdg;
}


static Widget
 uicreateprintmargin(formwdg, topwdg, name, labeltext, pos)
Widget formwdg;
Widget topwdg;
char *name;
char *labeltext;
int pos;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    static int callnr = 0;
    Widget marginformwdg, labelwdg;
    Widget margindownwdg, marginupwdg, textwdg;
    static char textvar[4][18];	/* strlen("PrintBottomMargin") */
    char *text;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5 + 50 * (pos == UI_RIGHT),
		       XmNrightAttachment, XmATTACH_POSITION,
		    XmNrightPosition, 45 + 50 * (pos == UI_RIGHT), NULL);
    marginformwdg = XmCreateForm(formwdg, "MarginForm", args, nargs);
    XtManageChild(marginformwdg);

    labelstr = XmStringCreateSimple(labeltext);
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNalignment, XmALIGNMENT_BEGINNING,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 40,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(marginformwdg, "TextLabel", args, nargs);
    XtManageChild(labelwdg);
    XmStringFree(labelstr);

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_DOWN,
		       XmNwidth, 15,
		       XmNheight, 5,
		       XmNtopAttachment, XmATTACH_POSITION,
		       XmNtopPosition, 50,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    margindownwdg = XmCreateArrowButtonGadget(marginformwdg,
					 uiactiondata[callnr].ActionName,
					      args, nargs);
    uiactiondata[callnr].Page = uiPageInfo.CurrentPage;
    XtAddCallback(margindownwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & uiactiondata[callnr]);
    UiAttachCallback(uiactiondata[callnr].ActionName, uiprintmargincb,
		     uiactiondata[callnr].ActionName);
    callnr++;
    XtManageChild(margindownwdg);

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_UP,
		       XmNwidth, 15,
		       XmNheight, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_POSITION,
		       XmNbottomPosition, 50, NULL);
    marginupwdg = XmCreateArrowButtonGadget(marginformwdg, "ArrowDown",
					    args, nargs);
    uiactiondata[callnr].Page = uiPageInfo.CurrentPage;
    XtAddCallback(marginupwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & uiactiondata[callnr]);
    UiAttachCallback(uiactiondata[callnr].ActionName, uiprintmargincb,
		     uiactiondata[callnr].ActionName);
    callnr++;
    XtManageChild(marginupwdg);

    text = textvar[callnr / 2 - 1];
    sprintf(text, "%sMargin", name);
    args = uiVaSetArgs(&nargs,
		       XmNcolumns, 4,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_WIDGET,
		       XmNrightWidget, margindownwdg,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNvalue, "0", NULL);
    textwdg = XmCreateText(marginformwdg, text, args, nargs);
    uiprinttextwidget[callnr / 2 - 1] = textwdg;
    XtAddCallback(textwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogVariableCB, (caddr_t) text);
    XtAddCallback(textwdg, XmNlosingFocusCallback,
		  (XtCallbackProc) uiDialogVariableCB, (caddr_t) text);
    XtAddCallback(textwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB, (caddr_t) text);
    (void) uiAddWidgetInfo(text, textwdg, uiWTtext);	/* ignore */
    XtManageChild(textwdg);

    return marginformwdg;
}


static Widget
 uicreateprintseparator(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(formwdg, "PRINTSeparator",
					   args, nargs);
    XtManageChild(separatorwdg);

    return separatorwdg;
}


static Widget
 uicreateprintcmd(formwdg)
Widget formwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget tmpformwdg;
    Widget labelwdg;
    Widget textwdg;
    static uiActionData_t actiondata[2];

    labelstr = XmStringCreateSimple("Print command");
    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_PRINT_WDG_OFFSET, NULL);
    tmpformwdg = XmCreateForm(formwdg, "PrintcmdForm", args, nargs);
    XtManageChild(tmpformwdg);

    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(tmpformwdg, "Label", args, nargs);
    XtManageChild(labelwdg);

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_WIDGET,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, labelwdg,
		       XmNleftOffset, UI_PRINT_WDG_OFFSET,
		       XmNvalue, "lpr", NULL);
    textwdg = XmCreateText(tmpformwdg, "PrintCmd", args, nargs);
    uiTopLevel.PrintGfx.WidthWdg = textwdg;
    XtAddCallback(textwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintCommand");
    XtAddCallback(textwdg, XmNlosingFocusCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintCommand");
    XtAddCallback(textwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintCommand");
    /* Ignore */
    (void) uiAddWidgetInfo("PrintCommand", textwdg, uiWTtext);
    XtManageChild(textwdg);

    return tmpformwdg;
}


static Widget
 uicreateprintfile(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget tmpformwdg, widthwdg;
    Widget textwdg;
    static uiActionData_t actiondata[2];

    labelstr = XmStringCreateSimple("Print to a file:");
    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_PRINT_WDG_OFFSET, NULL);
    tmpformwdg = XmCreateForm(formwdg, "FileForm", args, nargs);
    XtManageChild(tmpformwdg);

    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    widthwdg = XmCreateToggleButtonGadget(tmpformwdg, "PrintToFile",
					  args, nargs);
    XtAddCallback(widthwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintToFile");
    /* Ignore */
    (void) uiAddWidgetInfo("PrintToFile", widthwdg, uiWTcheckbutton);
    XtManageChild(widthwdg);

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, widthwdg,
		       XmNvalue, "foobar.www", NULL);
    textwdg = XmCreateText(tmpformwdg, "PrintFileName", args, nargs);
    uiTopLevel.PrintGfx.WidthWdg = textwdg;
    XtAddCallback(textwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintFileName");
    XtAddCallback(textwdg, XmNlosingFocusCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintfileName");
    XtAddCallback(textwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintFileName");
    /* Ignore */
    (void) uiAddWidgetInfo("PrintFileName", textwdg, uiWTtext);
    XtManageChild(textwdg);

    return tmpformwdg;
}


static Widget
 uicreateprintwidth(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget tmpformwdg, labelwdg;
    Widget textwdg, widthdownwdg, widthupwdg;
    static uiActionData_t actiondata[2];

    labelstr = XmStringCreateSimple("Width");
    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 45, NULL);
    tmpformwdg = XmCreateForm(formwdg, "WidthForm", args, nargs);
    XtManageChild(tmpformwdg);

    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNalignment, XmALIGNMENT_BEGINNING,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 40,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(tmpformwdg, "Label", args, nargs);
    XtManageChild(labelwdg);

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_DOWN,
		       XmNwidth, 15,
		       XmNheight, 5,
		       XmNtopAttachment, XmATTACH_POSITION,
		       XmNtopPosition, 50,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    widthdownwdg = XmCreateArrowButtonGadget(tmpformwdg, "WidthDown",
					     args, nargs);
    actiondata[0].ActionName = "WidthDown";
    actiondata[0].Page = uiPageInfo.CurrentPage;
    XtAddCallback(widthdownwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[0]);
    UiAttachCallback("WidthDown", uiprintwidthcb, "WidthDown");
    XtManageChild(widthdownwdg);

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_UP,
		       XmNwidth, 15,
		       XmNheight, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_POSITION,
		       XmNbottomPosition, 50, NULL);
    widthupwdg = XmCreateArrowButtonGadget(tmpformwdg, "WidthUp",
					   args, nargs);
    actiondata[1].ActionName = "WidthUp";
    actiondata[1].Page = uiPageInfo.CurrentPage;
    XtAddCallback(widthupwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[1]);
    UiAttachCallback("WidthUp", uiprintwidthcb, "WidthUp");
    XtManageChild(widthupwdg);

    args = uiVaSetArgs(&nargs,
		       XmNcolumns, 4,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_WIDGET,
		       XmNrightWidget, widthdownwdg,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNvalue, "80", NULL);
    textwdg = XmCreateText(tmpformwdg, "PrintWidth", args, nargs);
    uiTopLevel.PrintGfx.WidthWdg = textwdg;
    XtAddCallback(textwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintWidth");
    XtAddCallback(textwdg, XmNlosingFocusCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintWidth");
    XtAddCallback(textwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "PrintWidth");
    /* Ignore */
    (void) uiAddWidgetInfo("PrintWidth", textwdg, uiWTtext);
    XtManageChild(textwdg);

    return tmpformwdg;
}


static void uicreateprintbuttons(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget okwdg, closewdg;
    static uiActionData_t actiondata[3];

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 25,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, UI_PRINT_WDG_OFFSET, NULL);
    okwdg = XmCreatePushButtonGadget(formwdg, "Print", args, nargs);
    actiondata[0].ActionName = "PRINTOk";
    actiondata[0].Page = uiPageInfo.CurrentPage;
    XtAddCallback(okwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[0]);
    UiAttachCallback("PRINTOk", uiprintbuttoncb, "PRINTOk");
    XtManageChild(okwdg);

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PRINT_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 75,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, UI_PRINT_WDG_OFFSET, NULL);
    closewdg = XmCreatePushButtonGadget(formwdg, "Close", args, nargs);
    actiondata[2].ActionName = "PRINTClose";
    actiondata[2].Page = uiPageInfo.CurrentPage;
    XtAddCallback(closewdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[2]);
    UiAttachCallback("PRINTClose", uiprintbuttoncb, "PRINTClose");
    XtManageChild(closewdg);
}



static void uiprintmargincb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    int i;
    Widget textwdg;
    char *text;
    int margin;
    char tmpbuffer[5];

    for (i = 0; i < 8; i++)
	if (!strcmp(uiactiondata[i].ActionName, (char *) parameter)) {
	    textwdg = uiprinttextwidget[i / 2];
	    text = XmTextGetString(textwdg);
	    margin = atoi(text);
	    if (i % 2) {
		margin += 1;
		if (margin > 9999)
		    margin = 9999;
		sprintf(tmpbuffer, "%d", margin);
		XmTextSetString(textwdg, tmpbuffer);
	    } else {
		margin = (margin - 1) * (margin > 0);
		sprintf(tmpbuffer, "%d", margin);
		XmTextSetString(textwdg, tmpbuffer);
	    }
	    XtFree(text);

	    return;
	}
}


static void uiprintwidthcb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Widget textwdg = uiTopLevel.PrintGfx.WidthWdg;
    char *fixedtext;
    int width;
    char tmpbuffer[5];

    fixedtext = XmTextGetString(textwdg);
    width = atoi(fixedtext);

    if (!strcmp("WidthDown", (char *) parameter)) {
	if (width) {
	    width = (width - 5) * (width > 4);
	    sprintf(tmpbuffer, "%d", width);
	    XmTextSetString(textwdg, tmpbuffer);
	}
    } else {
	width += 5;
	if (width > 9999)
	    width = 9999;
	sprintf(tmpbuffer, "%d", width);
	XmTextSetString(textwdg, tmpbuffer);
    }

    XtFree(fixedtext);
}


static void uiprintbuttoncb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiPage_t *page = uiPageInfo.CurrentPage;

    if (!strcmp("PRINTOk", (char *) parameter)) {
	Print(htext);
	XtUnmapWidget(XtParent(uiTopLevel.PrintGfx.FormWdg));
    } else if (!strcmp("PRINTClose", (char *) parameter))
	XtUnmapWidget(XtParent(uiTopLevel.PrintGfx.FormWdg));
    else			/* Shouldn't reach this point */
	uiDisplayWarning("printbuttoncb called with illegal parameter");
}
