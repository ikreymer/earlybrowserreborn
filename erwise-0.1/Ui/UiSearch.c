static char *rcsid = "$Id: UiSearch.c,v 1.1 1992/03/26 18:13:50 kny Exp kss $";

#include "UiIncludes.h"


static Widget uicreatesearchformdialog();
static Widget uicreatesearchtext(Widget formwdg);
static Widget uicreatesearchlevel(Widget formwdg, Widget textwdg);
static Widget uicreatesearchlevelform(Widget formwdg, Widget levelwdg);
static Widget uicreatesearchleveldown(Widget formwdg);
static Widget uicreatesearchlevelup(Widget formwdg);
static Widget uicreatesearchcase(Widget formwdg, Widget levelwdg);
static Widget uicreatesearchbackward(Widget formwdg, Widget levelwdg);
static Widget uicreatesearchforward(Widget formwdg, Widget textwdg);
static Widget uicreatesearchlabel(Widget formwdg, Widget closewdg);
static Widget uicreatesearchclose(Widget formwdg, Widget forwardwdg);

static void uisearchlevelcb(char *address, HText_t * htex,
			   HTextObject_t * htextobject, void *parameter);
static void uisearchclosecb(char *address, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);


int UiDisplaySearchDialog(type)
int type;
{
    uiSearchGfx_t *searchgfx = &uiTopLevel.SearchGfx;

    if (searchgfx->FormWdg) {
	XtMapWidget(XtParent(searchgfx->FormWdg));
	uiWidgetPlacement(XtParent(searchgfx->FormWdg),
			  uiTopLevel.GlobalSettings.SearchPlacement);
	uiSearchUpdateDialog();
	return UI_OK;
    }
    searchgfx->FormWdg = uicreatesearchformdialog();
    searchgfx->TextWdg = uicreatesearchtext(searchgfx->FormWdg);
    searchgfx->LevelWdg = uicreatesearchlevel(searchgfx->FormWdg,
					      searchgfx->TextWdg);
    searchgfx->LevelFormWdg = uicreatesearchlevelform(searchgfx->FormWdg,
						    searchgfx->LevelWdg);
    searchgfx->CaseWdg = uicreatesearchcase(searchgfx->FormWdg,
					    searchgfx->LevelWdg);
    searchgfx->BackwardWdg = uicreatesearchbackward(searchgfx->FormWdg,
						    searchgfx->TextWdg);
    searchgfx->ForwardWdg = uicreatesearchforward(searchgfx->FormWdg,
						  searchgfx->BackwardWdg);
    searchgfx->CloseWdg = uicreatesearchclose(searchgfx->FormWdg,
					      searchgfx->ForwardWdg);
    searchgfx->LabelWdg = uicreatesearchlabel(searchgfx->FormWdg,
					      searchgfx->CloseWdg);

    XtManageChild(searchgfx->FormWdg);
    XtRealizeWidget(XtParent(searchgfx->FormWdg));

    uiWidgetPlacement(XtParent(searchgfx->FormWdg),
		      uiTopLevel.GlobalSettings.SearchPlacement);

    return UI_OK;
}


void uiSearchUpdateDialog()
{
    XmString labelstr;

    if (uiTopLevel.SearchGfx.FormWdg) {
	if (uiPageInfo.CurrentPage) {
	    labelstr =
		XmStringCreateSimple(HTAnchor_address((HTAnchor *)
						 uiPageInfo.CurrentPage->
						    HText->node_anchor));
	    XtVaSetValues(uiTopLevel.SearchGfx.LabelWdg,
			  XmNlabelString, labelstr, NULL);
	    XmStringFree(labelstr);
	} else
	    XtUnmapWidget(XtParent(uiTopLevel.SearchGfx.FormWdg));
    }
}


static Widget
 uicreatesearchformdialog()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("Search",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_SEARCH_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "Search", args, nargs);

    return formwdg;
}


static Widget
 uicreatesearchtext(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    Widget labelwdg, textwdg;

    labelstr = XmStringCreateSimple("Search for:");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_SEARCH_WDG_OFFSET, NULL);
    labelwdg = XmCreateLabelGadget(formwdg, "TextLabel", args, nargs);
    XtManageChild(labelwdg);
    XmStringFree(labelstr);

    args = uiVaSetArgs(&nargs,
		       XmNcolumns, 40,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, labelwdg,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET / 2,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_SEARCH_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, UI_SEARCH_WDG_OFFSET, NULL);
    textwdg = XmCreateText(formwdg, "Text", args, nargs);
    XtAddCallback(textwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogVariableCB, (caddr_t) "SearchText");
    XtAddCallback(textwdg, XmNlosingFocusCallback,
	    (XtCallbackProc) uiDialogVariableCB, (caddr_t) "SearchText");
    XtAddCallback(textwdg, XmNvalueChangedCallback,
	    (XtCallbackProc) uiDialogVariableCB, (caddr_t) "SearchText");
    (void) uiAddWidgetInfo("SearchText", textwdg, uiWTtext);	/* Ignore */
    XtManageChild(textwdg);

    return textwdg;
}


static Widget
 uicreatesearchlevel(formwdg, textwdg)
Widget formwdg;
Widget textwdg;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    Widget labelwdg, levelwdg;

    labelstr = XmStringCreateSimple("Depth of search:");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, textwdg,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_SEARCH_WDG_OFFSET, NULL);
    labelwdg = XmCreateLabelGadget(formwdg, "LevelLabel", args, nargs);
    XtManageChild(labelwdg);
    XmStringFree(labelstr);

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, labelwdg,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET / 2,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_SEARCH_WDG_OFFSET,
		       XmNcolumns, 3,
		       XmNmaxLength, 3,
		       XmNvalue, "1", NULL);
    levelwdg = XmCreateText(formwdg, "Level", args, nargs);
    XtAddCallback(levelwdg, XmNactivateCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "SearchLevel");
    XtAddCallback(levelwdg, XmNlosingFocusCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "SearchLevel");
    XtAddCallback(levelwdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB,
		  (caddr_t) "SearchLevel");
    (void) uiAddWidgetInfo("SearchLevel", levelwdg, uiWTtext);	/* Ignore */
    XtManageChild(levelwdg);

    return levelwdg;
}


static Widget
 uicreatesearchlevelform(formwdg, levelwdg)
Widget formwdg;
Widget levelwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget levelformwdg;
    uiSearchGfx_t *searchgfx = &uiTopLevel.SearchGfx;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNtopWidget, levelwdg,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, levelwdg,
		       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNbottomWidget, levelwdg, NULL);
    levelformwdg = XmCreateForm(formwdg, "LevelForm", args, nargs);
    XtManageChild(levelformwdg);

    searchgfx->LevelDownWdg = uicreatesearchleveldown(levelformwdg);
    searchgfx->LevelUpWdg = uicreatesearchlevelup(levelformwdg);

    return levelformwdg;
}


static Widget
 uicreatesearchleveldown(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget leveldownwdg;
    char *actionname = "LevelDown";
    static uiActionData_t actiondata;

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_DOWN,
		       XmNwidth, 15,
		       XmNtopAttachment, XmATTACH_POSITION,
		       XmNtopPosition, 50,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    leveldownwdg = XmCreateArrowButtonGadget(formwdg, actionname,
					     args, nargs);
    actiondata.ActionName = actionname;
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(leveldownwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    UiAttachCallback(actionname, uisearchlevelcb, (void *) actionname);
    XtManageChild(leveldownwdg);

    return leveldownwdg;
}


static Widget
 uicreatesearchlevelup(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget levelupwdg;
    char *actionname = "LevelUp";
    static uiActionData_t actiondata;

    args = uiVaSetArgs(&nargs,
		       XmNarrowDirection, XmARROW_UP,
		       XmNwidth, 15,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_POSITION,
		       XmNbottomPosition, 50, NULL);
    levelupwdg = XmCreateArrowButtonGadget(formwdg, "LevelUp",
					   args, nargs);
    actiondata.ActionName = actionname;
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(levelupwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    UiAttachCallback(actionname, uisearchlevelcb, (void *) actionname);
    XtManageChild(levelupwdg);

    return levelupwdg;
}


static Widget
 uicreatesearchcase(formwdg, levelwdg)
Widget formwdg;
Widget levelwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget casewdg;

    labelstr = XmStringCreateSimple("Case sensitive");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNtopWidget, levelwdg,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 30,
		       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNbottomWidget, levelwdg, NULL);
    casewdg = XmCreateToggleButtonGadget(formwdg, "Case", args, nargs);
    XtAddCallback(casewdg, XmNvalueChangedCallback,
	    (XtCallbackProc) uiDialogVariableCB, (caddr_t) "SearchCase");
    /* Ignore */
    (void) uiAddWidgetInfo("SearchCase", casewdg, uiWTcheckbutton);
    XtManageChild(casewdg);

    return casewdg;
}


static Widget
 uicreatesearchbackward(formwdg, textwdg)
Widget formwdg;
Widget textwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget backwardwdg;
    char *actionname = "SearchBackward";
    static uiActionData_t actiondata;

    labelstr = XmStringCreateSimple("Search backward");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, textwdg,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNrightWidget, textwdg, NULL);
    backwardwdg = XmCreatePushButtonGadget(formwdg, "SearchBackward",
					   args, nargs);
    actiondata.ActionName = actionname;
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(backwardwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    XtManageChild(backwardwdg);
    XmStringFree(labelstr);

    return backwardwdg;
}


static Widget
 uicreatesearchforward(formwdg, backwardwdg)
Widget formwdg;
Widget backwardwdg;
{
    XmString labelstr;
    ArgList args;
    Cardinal nargs;
    Widget forwardwdg;
    char *actionname = "SearchForward";
    static uiActionData_t actiondata;

    labelstr = XmStringCreateSimple("Search forward");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, backwardwdg,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET / 2,
		       XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNleftWidget, backwardwdg,
		       XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNrightWidget, backwardwdg, NULL);
    forwardwdg = XmCreatePushButtonGadget(formwdg, "SearchForward",
					  args, nargs);
    actiondata.ActionName = actionname;
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(forwardwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    XtManageChild(forwardwdg);
    XmStringFree(labelstr);

    return forwardwdg;
}


static Widget
 uicreatesearchclose(parentwdg, topwdg)
Widget parentwdg;
Widget topwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;
    Widget tmpwdg;
    static uiActionData_t actiondata;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_SEARCH_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(parentwdg, "SearchSeparator",
					   args, nargs);
    XtManageChild(separatorwdg);

    tmpwdg = XmCreatePushButtonGadget(parentwdg, "Close", (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNwidth, 70,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, UI_SEARCH_WDG_OFFSET,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, separatorwdg,
		  XmNtopOffset, UI_SEARCH_WDG_OFFSET,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, UI_SEARCH_WDG_OFFSET, NULL);
    actiondata.ActionName = "SearchClose";
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(tmpwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    UiAttachCallback("SearchClose", uisearchclosecb, "SearchClose");

    XtManageChild(tmpwdg);

    return tmpwdg;
}


static Widget
 uicreatesearchlabel(formwdg, rightwdg)
Widget formwdg;
Widget rightwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget labelwdg;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNtopWidget, rightwdg,
		       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNbottomWidget, rightwdg,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_SEARCH_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_WIDGET,
		       XmNrightWidget, rightwdg,
		       XmNrightOffset, UI_SEARCH_WDG_OFFSET, NULL);
    labelwdg = XmCreateLabelGadget(formwdg, "Label", args, nargs);
    XtManageChild(labelwdg);

    return labelwdg;
}


static void uisearchlevelcb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Widget levelwdg = uiTopLevel.SearchGfx.LevelWdg;
    char *leveltext;
    int level;
    char tmpbuffer[4];

    leveltext = XmTextGetString(levelwdg);
    level = atoi(leveltext);

    if (!strcmp("LevelDown", (char *) parameter)) {
	if (level > 1) {
	    level--;
	    sprintf(tmpbuffer, "%d", level);
	    XmTextSetString(levelwdg, tmpbuffer);
	}
    } else if (level < 1) {
	level++;
	sprintf(tmpbuffer, "%d", level);
	XmTextSetString(levelwdg, tmpbuffer);
    }
    XtFree(leveltext);
}


static void uisearchclosecb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    XtUnmapWidget(XtParent(uiTopLevel.SearchGfx.FormWdg));
}
