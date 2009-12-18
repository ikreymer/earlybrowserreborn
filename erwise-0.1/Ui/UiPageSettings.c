static char *rcsid = "$Id: UiPageSettings.c,v 1.1 1992/03/26 18:13:50 kny Exp kny $";

#include "UiIncludes.h"


static void uibindpsvariables(void);
static void uiupdatepsvariables(void);
static Widget uicreatepsformdialog();
static Widget uicreatepslabel(Widget parent);
static Widget
 uicreatepsmargin(Widget parent, Widget topwdg, char *name,
		   int position);
static Widget uicreatepsseparator(Widget formwdg, Widget topwdg);
static Widget uicreatepsusefixed(Widget parent, Widget topwdg);
static Widget uicreatepssinglepage(Widget parent, Widget topwdg);
static void uicreatepsbuttons(Widget formwdg, Widget topwdg);

static void uipagesettingsmargincb(char *address, HText_t * htext,
				    HTextObject_t * htextobject,
				    void *parameter);
static void uipagesettingsusefixedcb(char *address, HText_t * htext,
				      HTextObject_t * htextobject,
				      void *parameter);
static void uipagesettingsbuttoncb(char *address, HText_t * htext,
				    HTextObject_t * htextobject,
				    void *parameter);


int UiDisplayPageSettingsDialog(type)
int type;
{
    uiPageSettingsGfx_t *psgfx = &uiTopLevel.PageSettingsGfx;
    Widget separatorwdg;

    uibindpsvariables();

    if (psgfx->FormWdg) {
	XtMapWidget(XtParent(psgfx->FormWdg));

	uiWidgetPlacement(XtParent(psgfx->FormWdg),
			uiTopLevel.GlobalSettings.PageSettingsPlacement);

	uiupdatepsvariables();

	return UI_OK;
    }
    psgfx->FormWdg = uicreatepsformdialog();
    psgfx->LabelWdg = uicreatepslabel(psgfx->FormWdg);

    psgfx->LeftMarginWdg = uicreatepsmargin(psgfx->FormWdg, psgfx->LabelWdg,
					    "Left", UI_LEFT);
    psgfx->RightMarginWdg =
	uicreatepsmargin(psgfx->FormWdg, psgfx->LeftMarginWdg,
			 "Right", UI_LEFT);
    psgfx->TopMarginWdg = uicreatepsmargin(psgfx->FormWdg, psgfx->LabelWdg,
					   "Top", UI_RIGHT);
    psgfx->BottomMarginWdg =
	uicreatepsmargin(psgfx->FormWdg, psgfx->TopMarginWdg,
			 "Bottom", UI_RIGHT);

    separatorwdg = uicreatepsseparator(psgfx->FormWdg, psgfx->RightMarginWdg);

    psgfx->UseFixedWdg = uicreatepsusefixed(psgfx->FormWdg,
					    separatorwdg);
    psgfx->SinglePageWdg = uicreatepssinglepage(psgfx->FormWdg,
						psgfx->UseFixedWdg);

    separatorwdg = uicreatepsseparator(psgfx->FormWdg, psgfx->SinglePageWdg);
    uicreatepsbuttons(psgfx->FormWdg, separatorwdg);
    XtManageChild(psgfx->FormWdg);
    XtRealizeWidget(XtParent(psgfx->FormWdg));

    uiWidgetPlacement(XtParent(psgfx->FormWdg),
		      uiTopLevel.GlobalSettings.PageSettingsPlacement);

    uiupdatepsvariables();

    return UI_OK;
}


void uiPageSettingsUpdateDialog()
{
    if (uiTopLevel.PageSettingsGfx.FormWdg) {
	if (uiPageInfo.CurrentPage) {
	    uibindpsvariables();
	    uiupdatepsvariables();
	} else
	    XtUnmapWidget(XtParent(uiTopLevel.PageSettingsGfx.FormWdg));
    }
}


static void uibindpsvariables()
{
    UiBindVariable("TopMargin",
		   (void *) &uiPageInfo.CurrentPage->Settings.TopMargin,
		   uiVTint);
    UiBindVariable("BottomMargin",
		 (void *) &uiPageInfo.CurrentPage->Settings.BottomMargin,
		   uiVTint);
    UiBindVariable("LeftMargin",
		   (void *) &uiPageInfo.CurrentPage->Settings.LeftMargin,
		   uiVTint);
    UiBindVariable("RightMargin",
		   (void *) &uiPageInfo.CurrentPage->Settings.RightMargin,
		   uiVTint);

    UiBindVariable("UseFixed",
		   (void *) &uiPageInfo.CurrentPage->Settings.UseFixed,
		   uiVTint);
    UiBindVariable("FixedWidth",
		   (void *) &uiPageInfo.CurrentPage->Settings.FixedWidth,
		   uiVTint);
    UiBindVariable("OnePageMode",
		   (void *) &uiPageInfo.CurrentPage->Settings.OnePageMode,
		   uiVTint);
}


static void uiupdatepsvariables()
{
    UiUpdateVariable("TopMargin");
    UiUpdateVariable("BottomMargin");
    UiUpdateVariable("LeftMargin");
    UiUpdateVariable("RightMargin");

    UiUpdateVariable("UseFixed");
    UiUpdateVariable("FixedWidth");
    UiUpdateVariable("OnePageMode");
}


static Widget
 uicreatepsformdialog()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("PageSettings",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_SETTINGS_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "PageSettings", args, nargs);

    return formwdg;
}


static Widget
 uicreatepslabel(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    Widget labelwdg;

    labelstr = XmStringCreateSimple("Define Margins:");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(formwdg, "TextLabel", args, nargs);
    XtManageChild(labelwdg);
    XmStringFree(labelstr);

    return labelwdg;
}


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

static Widget uipstextwidget[] =
{
    (Widget) NULL,
    (Widget) NULL,
    (Widget) NULL,
    (Widget) NULL
};

static Widget
 uicreatepsmargin(formwdg, topwdg, name, pos)
Widget formwdg;
Widget topwdg;
char *name;
int pos;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    static int callnr = 0;
    Widget marginformwdg, labelwdg;
    Widget margindownwdg, marginupwdg, textwdg;
    static char textvar[4][13];	/* strlen("BottomMargin") */
    char *text;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5 + 50 * (pos == UI_RIGHT),
		       XmNrightAttachment, XmATTACH_POSITION,
		    XmNrightPosition, 45 + 50 * (pos == UI_RIGHT), NULL);
    marginformwdg = XmCreateForm(formwdg, "MarginForm", args, nargs);
    XtManageChild(marginformwdg);

    labelstr = XmStringCreateSimple(name);
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
    UiAttachCallback(uiactiondata[callnr].ActionName, uipagesettingsmargincb,
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
    UiAttachCallback(uiactiondata[callnr].ActionName, uipagesettingsmargincb,
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
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    textwdg = XmCreateText(marginformwdg, text, args, nargs);
    uipstextwidget[callnr / 2 - 1] = textwdg;
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
 uicreatepsseparator(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(formwdg, "PSSeparator",
					   args, nargs);
    XtManageChild(separatorwdg);

    return separatorwdg;
}


static Widget
 uicreatepsusefixed(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget tmpformwdg, usefixedwdg;
    Widget textwdg, usefixeddownwdg, usefixedupwdg;
    static uiActionData_t actiondata[2];

    labelstr = XmStringCreateSimple("Use Fixed Width");
    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 20, NULL);
    tmpformwdg = XmCreateForm(formwdg, "UseFixedForm", args, nargs);
    XtManageChild(tmpformwdg);

    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    usefixedwdg = XmCreateToggleButtonGadget(tmpformwdg, "UseFixed",
					     args, nargs);
    XtAddCallback(usefixedwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "UseFixed");
    /* Ignore */
    (void) uiAddWidgetInfo("UseFixed", usefixedwdg, uiWTcheckbutton);
    XtManageChild(usefixedwdg);

    args = uiVaSetArgs(&nargs,
		       XmNcolumns, 4,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, usefixedwdg,
		       XmNleftOffset, UI_PAGESETTINGS_WDG_OFFSET, NULL);
    textwdg = XmCreateText(tmpformwdg, "FixedWidth", args, nargs);
    uiTopLevel.PageSettingsGfx.UseFixedTextWdg = textwdg;
    XtAddCallback(textwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "FixedWidth");
    XtAddCallback(textwdg, XmNlosingFocusCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "FixedWidth");
    XtAddCallback(textwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "FixedWidth");
    /* Ignore */
    (void) uiAddWidgetInfo("FixedWidth", textwdg, uiWTtext);
    XtManageChild(textwdg);

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_DOWN,
		       XmNwidth, 15,
		       XmNheight, 5,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, textwdg,
		       XmNtopAttachment, XmATTACH_POSITION,
		       XmNtopPosition, 50, NULL);
    usefixeddownwdg = XmCreateArrowButtonGadget(tmpformwdg, "UseFixedDown",
						args, nargs);
    actiondata[0].ActionName = "UseFixedDown";
    actiondata[0].Page = uiPageInfo.CurrentPage;
    XtAddCallback(usefixeddownwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[0]);
    UiAttachCallback("UseFixedDown", uipagesettingsusefixedcb, "UseFixedDown");
    XtManageChild(usefixeddownwdg);

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_UP,
		       XmNwidth, 15,
		       XmNheight, 5,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, textwdg,
		       XmNbottomAttachment, XmATTACH_POSITION,
		       XmNbottomPosition, 50, NULL);
    usefixedupwdg = XmCreateArrowButtonGadget(tmpformwdg, "UseFixedUp",
					      args, nargs);
    actiondata[1].ActionName = "UseFixedUp";
    actiondata[1].Page = uiPageInfo.CurrentPage;
    XtAddCallback(usefixedupwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[1]);
    UiAttachCallback("UseFixedUp", uipagesettingsusefixedcb, "UseFixedUp");
    XtManageChild(usefixedupwdg);

    return tmpformwdg;
}


static Widget
 uicreatepssinglepage(formwdg, usefixedwdg)
Widget formwdg;
Widget usefixedwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget singlepagewdg;

    labelstr = XmStringCreateSimple("Single Page Mode");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, usefixedwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 20, NULL);
    singlepagewdg = XmCreateToggleButtonGadget(formwdg, "OnePageMode",
					       args, nargs);
    XtAddCallback(singlepagewdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "OnePageMode");
    /* Ignore */
    (void) uiAddWidgetInfo("OnePageMode", singlepagewdg, uiWTcheckbutton);
    XtManageChild(singlepagewdg);

    return singlepagewdg;
}


static void uicreatepsbuttons(formwdg, topwdg)
Widget formwdg;
Widget topwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget okwdg, applywdg, closewdg;
    static uiActionData_t actiondata[3];

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 25,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, 10, NULL);
    okwdg = XmCreatePushButtonGadget(formwdg, "Ok", args, nargs);
    actiondata[0].ActionName = "PSOk";
    actiondata[0].Page = uiPageInfo.CurrentPage;
    XtAddCallback(okwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[0]);
    UiAttachCallback("PSOk", uipagesettingsbuttoncb, "PSOk");
    XtManageChild(okwdg);

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 40,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 60,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, 10, NULL);
    applywdg = XmCreatePushButtonGadget(formwdg, "Apply", args, nargs);
    actiondata[1].ActionName = "PSApply";
    actiondata[1].Page = uiPageInfo.CurrentPage;
    XtAddCallback(applywdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[1]);
    UiAttachCallback("PSApply", uipagesettingsbuttoncb, "PSApply");
    XtManageChild(applywdg);

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_PAGESETTINGS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 75,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, 10, NULL);
    closewdg = XmCreatePushButtonGadget(formwdg, "Close", args, nargs);
    actiondata[2].ActionName = "PSClose";
    actiondata[2].Page = uiPageInfo.CurrentPage;
    XtAddCallback(closewdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogActivateCB,
		  (caddr_t) & actiondata[2]);
    UiAttachCallback("PSClose", uipagesettingsbuttoncb, "PSClose");
    XtManageChild(closewdg);
}


static void uipagesettingsmargincb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    int i;
    Widget textwdg;
    char *text;
    int margin;
    char tmpbuffer[4];

    for (i = 0; i < 8; i++)
	if (!strcmp(uiactiondata[i].ActionName, (char *) parameter)) {
	    textwdg = uipstextwidget[i / 2];
	    text = XmTextGetString(textwdg);
	    margin = atoi(text);
	    if (i % 2) {
		margin += 10;
		if (margin > 9999)
		    margin = 9999;
		sprintf(tmpbuffer, "%d", margin);
		XmTextSetString(textwdg, tmpbuffer);
	    } else {
		margin = (margin - 10) * (margin > 9);
		sprintf(tmpbuffer, "%d", margin);
		XmTextSetString(textwdg, tmpbuffer);
	    }
	    XtFree(text);

	    return;
	}
}


static void uipagesettingsusefixedcb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Widget textwdg = uiTopLevel.PageSettingsGfx.UseFixedTextWdg;
    char *fixedtext;
    int width;
    char tmpbuffer[4];

    fixedtext = XmTextGetString(textwdg);
    width = atoi(fixedtext);

    if (!strcmp("UseFixedDown", (char *) parameter)) {
	if (width) {
	    width = (width - 10) * (width > 9);
	    sprintf(tmpbuffer, "%d", width);
	    XmTextSetString(textwdg, tmpbuffer);
	}
    } else {
	width += 10;
	if (width > 9999)
	    width = 9999;
	sprintf(tmpbuffer, "%d", width);
	XmTextSetString(textwdg, tmpbuffer);
    }

    XtFree(fixedtext);
}


static void uipagesettingsbuttoncb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiPage_t *page = uiPageInfo.CurrentPage;

    if (!strcmp("PSOk", (char *) parameter)) {
	XlClearWindow(page->Layout.Width, page->Layout.Height, page->HText);
	uiPageUpdateWindow(page);
	XtUnmapWidget(XtParent(uiTopLevel.PageSettingsGfx.FormWdg));
    } else if (!strcmp("PSApply", (char *) parameter)) {
	XlClearWindow(page->Layout.Width, page->Layout.Height,
		      page->HText);
	uiPageUpdateWindow(page);
    } else if (!strcmp("PSClose", (char *) parameter))
	XtUnmapWidget(XtParent(uiTopLevel.PageSettingsGfx.FormWdg));
    else			/* Shouldn't reach this point */
	uiDisplayWarning("psbuttoncb called with illegal parameter");
}
