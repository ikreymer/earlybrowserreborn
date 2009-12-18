static char *rcsid = "$Id: UiPage.c,v 1.4 1992/03/26 18:13:50 kny Exp kny $";

#include "UiIncludes.h"


static uiHierarchy_t *uifindoraddhierarchy(char *topaddress);
static uiHierarchy_t *uifindhierarchy(char *topaddress);
static int uideletehierarchy(uiHierarchy_t * hierarchy);
static uiPage_t *uifindpage(HText_t * htext, uiHierarchy_t * hierarchy);
static uiPage_t *
 uiaddpage(HText_t * htext, HTextObject_t * htextobject,
	    uiHierarchy_t * hierarchy, uiPage_t * prevpage);
static int uideletepage(uiPage_t * page, uiHierarchy_t * hierarchy);
static void uideletepagecallbacks(uiPage_t * page);
static uiPage_t *uifindshadowpage(uiHierarchy_t * hierarchy, uiPage_t * page);
static int uicreatepagewidgets(uiPage_t * page, uiPage_t * prevpage,
			        char *title);
static void uicreatepageform(uiPage_t * page, char *title, Widget topwdg);
static Widget uicreatepagemenu(uiPage_t * page, Widget formwdg);
static Widget uicreatepagemenuitem(uiPage_t * page, Widget menuwdg,
				    int itempos, int level);
static int uifindnextitem(int itempos, int level);
static Widget uicreatepagecontrol(uiPage_t * page, Widget formwdg, char *title);
static void uicreatepagelabel(Widget parentwdg, char *label);
static void uicreatepagebutton(uiPage_t * page, Widget parentwdg, char *name,
			        int leftpos, int rightpos);
static Widget uicreatepagefind(uiPage_t * page, Widget formwdg,
			        Widget controlwdg);
static Widget uicreatepagescroll(uiPage_t * page, Widget formwdg,
				  Widget menuwdg, Widget controlwdg);
static void uiconnectpage(uiPage_t * page);
static void uipageupdatescrollbars(uiPage_t * page);
static uiActionData_t *uicreatepageactiondata(uiPage_t * page,
					       char *actionname);
static void uisetcurrentpage(uiPage_t * page);
static int uideletepageinternal(char *topaddress, HText_t * htext);

static void uipageactivatecb(Widget wdg, uiActionData_t * actiondata,
			      XmAnyCallbackStruct * calldata);
static void uipagekludgecb(Widget wdg, uiActionData_t * actiondata,
			    XEvent * event);
static void uipageexposecb(Widget wdg, uiPage_t * page,
			    XmDrawingAreaCallbackStruct * calldata);
static void uipageresizecb(Widget wdg, uiPage_t * page,
			    XmDrawingAreaCallbackStruct * calldata);
static void uipageinputcb(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
static void uipagescrollbarcb(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
static void uipagedowncb(char *topaddress, HText_t * htext,
			  HTextObject_t * htextobject, void *parameter);
static void uipageupcb(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject, void *parameter);
static void uipagetopcb(char *topaddress, HText_t * htext,
			 HTextObject_t * htextobject, void *parameter);
static void uipagebottomcb(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);

uiPageInfo_t uiPageInfo =
{
    (uiPage_t *) NULL,
    (Widget) NULL,
    (void *) NULL
};


int UiDisplayPage(topaddress, prevhtext, htext, htextobject, title)
char *topaddress;
HText_t *prevhtext;
HText_t *htext;
HTextObject_t *htextobject;
char *title;
{
    uiHierarchy_t *hierarchy;
    uiPage_t *prevpage, *page;

    if (!(hierarchy = uifindoraddhierarchy(topaddress)))
	return UI_ERROR;

    if (!(page = uifindpage(htext, hierarchy)) || !page->Visible) {
	prevpage = uifindpage(prevhtext, hierarchy);

	if (!page)
	    if (!(page = uiaddpage(htext, htextobject, hierarchy, prevpage))) {
		if (!hierarchy->Pages)
		    (void) uideletehierarchy(hierarchy);	/* already an error */

		return UI_ERROR;
	    }
	if (uicreatepagewidgets(page, prevpage, title) == UI_OK) {
	    uisetcurrentpage(page);

	    return UI_OK;
	} else
	    return UI_ERROR;
    } else {
	XRaiseWindow(XtDisplay(page->Gfx.TopWdg), XtWindow(page->Gfx.TopWdg));
	uisetcurrentpage(page);
    }

    return UI_OK;
}


int UiDeletePage(topaddress, htext)
char *topaddress;
HText_t *htext;
{
    uiHierarchy_t *hierarchy;
    uiPage_t *page, *shadowpage;
    uiAction_t *tmpaction;

    if (!(hierarchy = uifindhierarchy(topaddress)))
	return UI_ERROR;

    if (!(page = uifindpage(htext, hierarchy)))
	return UI_ERROR;

    XlDeleteText(htext);

    shadowpage = uifindshadowpage(hierarchy, page);

    if (page->Visible)
	XtDestroyWidget(XtParent(page->Gfx.FormWdg));
    uideletepage(page, hierarchy);

    if (shadowpage) {		/* Recursive destroy, oh yeah */
	tmpaction = uiFindAction("Close");
	(tmpaction->Callback) (topaddress, shadowpage->HText,
			  shadowpage->HTextObject, tmpaction->Parameter);
    }
    return UI_OK;
}


int UiSetCursor(topaddress, htext, htextobject)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
{
    uiHierarchy_t *hierarchy;
    uiPage_t *page;
    int tmpx, tmpy;

    if (!(hierarchy = uifindhierarchy(topaddress)))
	return UI_ERROR;

    if (!(page = uifindpage(htext, hierarchy)))
	return UI_ERROR;

    page->HTextObject = htextobject;

    if (!page->Visible)
	return UI_OK;

    XlSetCursor(htext, htextobject);

    XlGetCoordinates(&tmpx, &tmpy, htextobject);
    if (tmpx < page->Layout.X || tmpx > page->Layout.X + page->Layout.Width)
	page->Layout.X = tmpx - page->Layout.Width / 2;
    if (tmpy < page->Layout.Y || tmpy > page->Layout.Y + page->Layout.Height)
	page->Layout.Y = tmpy - page->Layout.Height / 2;

    uipageupdatescrollbars(page);
    XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);

    return UI_OK;
}


void uiPageUpdateWindow(page)
uiPage_t *page;
{
    HTextObject_t *htextobject;
    int tmpwidth;

    if (page->Settings.UseFixed)
	tmpwidth = page->Settings.FixedWidth - page->Settings.LeftMargin -
	    page->Settings.RightMargin;
    else
	tmpwidth = page->Layout.Width - page->Settings.LeftMargin -
	    page->Settings.RightMargin;

    htextobject = XlLocateHTextObject(0, 0, page->HText);
    XlFormatText(page->Settings.LeftMargin, tmpwidth,
		 page->Settings.TopMargin, &page->Layout.VirtualWidth,
		 &page->Layout.VirtualHeight, page->HText);
    XlGetCoordinates(&page->Layout.X, &page->Layout.Y, htextobject);

    if (page->Layout.VirtualWidth > tmpwidth + page->Settings.LeftMargin)
	page->Layout.VirtualWidth += page->Settings.RightMargin;
    else
	page->Layout.VirtualWidth = tmpwidth + page->Settings.LeftMargin +
	    page->Settings.RightMargin;
    page->Layout.VirtualHeight += page->Settings.BottomMargin;

    if (!page->Settings.UseFixed)
	page->Layout.X = 0;
    uipageupdatescrollbars(page);

    XlSetPageCoordinates(page->Layout.X, page->Layout.Y, page->HText);
}


void uiPageAttachCallbacks()
{
    UiAttachCallback("Top", uipagetopcb, (void *) NULL);
    UiAttachCallback("Bottom", uipagebottomcb, (void *) NULL);
}


void uiPageDefineKeys()
{
    UiBindKey("space", UI_NONE, uipagedowncb, (void *) NULL);
    UiBindKey("Delete", UI_NONE, uipageupcb, (void *) NULL);

    UiBindKey("less", UI_SHIFT, uipagetopcb, (void *) NULL);
    UiBindKey("greater", UI_SHIFT, uipagebottomcb, (void *) NULL);
}


static uiHierarchy_t *
 uifindoraddhierarchy(topaddress)
char *topaddress;
{
    uiHierarchy_t *tmphierarchy;

    if (!(tmphierarchy = uifindhierarchy(topaddress))) {
	tmphierarchy = uiTopLevel.Hierarchies;
	if (!tmphierarchy)
	    tmphierarchy = uiTopLevel.Hierarchies =
		(uiHierarchy_t *) uiMalloc(sizeof(*tmphierarchy));
	else {
	    while (tmphierarchy->Next)
		tmphierarchy = tmphierarchy->Next;
	    tmphierarchy = tmphierarchy->Next =
		(uiHierarchy_t *) uiMalloc(sizeof(*tmphierarchy));
	}
	tmphierarchy->Address = topaddress;
	tmphierarchy->Pages = (uiPage_t *) NULL;
	tmphierarchy->Next = (uiHierarchy_t *) NULL;
    }
    return tmphierarchy;
}


static uiHierarchy_t *
 uifindhierarchy(topaddress)
char *topaddress;
{
    uiHierarchy_t *tmphierarchy = uiTopLevel.Hierarchies;

    while (tmphierarchy && strcmp(tmphierarchy->Address, topaddress))
	tmphierarchy = tmphierarchy->Next;

    return tmphierarchy;
}


static int uideletehierarchy(hierarchy)
uiHierarchy_t *hierarchy;
{
    uiHierarchy_t *tmphierarchy = uiTopLevel.Hierarchies;

    if (hierarchy == tmphierarchy)
	if (tmphierarchy->Next) {
	    uiTopLevel.Hierarchies = hierarchy->Next;
	    uisetcurrentpage(hierarchy->Next->Pages);
	} else {
	    uiTopLevel.Hierarchies = (uiHierarchy_t *) NULL;
	    uisetcurrentpage((uiPage_t *) NULL);
	}
    else {
	while (tmphierarchy->Next && tmphierarchy->Next != hierarchy)
	    tmphierarchy = tmphierarchy->Next;
	if (tmphierarchy->Next) {
	    tmphierarchy->Next = tmphierarchy->Next->Next;
	    uisetcurrentpage(tmphierarchy->Pages);
	} else {
	    /* This shouldn't happen unless we have a stray pointer */
	    uiDisplayWarning("Messed up hierarchy-list");

	    return UI_ERROR;
	}
    }

    uiFree(hierarchy);

    return UI_OK;
}


static uiPage_t *
 uifindpage(htext, hierarchy)
HText_t *htext;
uiHierarchy_t *hierarchy;
{
    uiPage_t *tmppage = hierarchy->Pages;

    while (tmppage && tmppage->HText != htext)
	tmppage = tmppage->Next;

    return tmppage;
}


static uiPage_t *
 uiaddpage(htext, htextobject, hierarchy, prevpage)
HText_t *htext;
HTextObject_t *htextobject;
uiHierarchy_t *hierarchy;
uiPage_t *prevpage;
{
    uiPage_t *tmppage = hierarchy->Pages;

    if (!tmppage)
	tmppage = hierarchy->Pages =
	    (uiPage_t *) uiMalloc(sizeof(*tmppage));
    else {
	while (tmppage->Next)
	    tmppage = tmppage->Next;
	tmppage = tmppage->Next =
	    (uiPage_t *) uiMalloc(sizeof(*tmppage));
    }
    tmppage->HText = htext;
    tmppage->HTextObject = htextobject;
    tmppage->Hierarchy = hierarchy;

    if (prevpage) {
	tmppage->Settings.TopMargin = prevpage->Settings.TopMargin;
	tmppage->Settings.BottomMargin = prevpage->Settings.BottomMargin;
	tmppage->Settings.LeftMargin = prevpage->Settings.LeftMargin;
	tmppage->Settings.RightMargin = prevpage->Settings.RightMargin;
	tmppage->Settings.OnePageMode = prevpage->Settings.OnePageMode;
	tmppage->Settings.UseFixed = prevpage->Settings.UseFixed;
	tmppage->Settings.FixedWidth = prevpage->Settings.FixedWidth;
	tmppage->Layout.Width = prevpage->Layout.Width;
	tmppage->Layout.Height = prevpage->Layout.Height;
    } else {
	uiGlobalSettings_t *gs = &uiTopLevel.GlobalSettings;

	tmppage->Settings.TopMargin = gs->TopMargin;
	tmppage->Settings.BottomMargin = gs->BottomMargin;
	tmppage->Settings.LeftMargin = gs->LeftMargin;
	tmppage->Settings.RightMargin = gs->RightMargin;
	tmppage->Settings.OnePageMode = gs->OnePageMode;
	tmppage->Settings.UseFixed = gs->UseFixed;
	tmppage->Settings.FixedWidth = gs->FixedWidth;
	tmppage->Layout.Width = gs->Width;
	tmppage->Layout.Height = gs->Height;
    }

    /* X and Y are href-dependent, Width and Height default, or previous */
    tmppage->Layout.X = 0;
    tmppage->Layout.Y = 0;

    tmppage->Callbacks = (uiPageCBList_t *) NULL;
    tmppage->Next = (uiPage_t *) NULL;

    return tmppage;
}


static int uideletepage(page, hierarchy)
uiPage_t *page;
uiHierarchy_t *hierarchy;
{
    uiPage_t *tmppage = hierarchy->Pages;

    if (tmppage == page)
	if (tmppage->Next) {
	    hierarchy->Pages = tmppage->Next;
	    uisetcurrentpage(tmppage->Next);
	} else
	    uideletehierarchy(hierarchy);
    else {
	while (tmppage->Next && tmppage->Next != page)
	    tmppage = tmppage->Next;
	if (tmppage->Next) {
	    tmppage->Next = tmppage->Next->Next;
	    if (tmppage->Next)
		uisetcurrentpage(tmppage->Next);
	    else
		uisetcurrentpage(tmppage);
	} else {
	    /* This shouldn't happen unless we have a stray pointer */
	    uiDisplayWarning("Messed up page-list");

	    return UI_ERROR;
	}
    }

    uiListUpdateDialog(page);
    uideletepagecallbacks(page);
    uiFree(page);

    return UI_OK;
}


static void uideletepagecallbacks(page)
uiPage_t *page;
{
    uiPageCBList_t *tmpcb = page->Callbacks;

    while (tmpcb) {
	tmpcb = tmpcb->Next;
	uiFree(page->Callbacks);
	page->Callbacks = tmpcb;
    }
}


static uiPage_t *
 uifindshadowpage(hierarchy, page)
uiHierarchy_t *hierarchy;
uiPage_t *page;
{
    uiPage_t *tmppage = hierarchy->Pages;

    while (tmppage)
	if (tmppage != page && tmppage->Gfx.TopWdg == page->Gfx.TopWdg)
	    return tmppage;
	else
	    tmppage = tmppage->Next;

    return (uiPage_t *) NULL;
}


static int uicreatepagewidgets(page, prevpage, title)
uiPage_t *page;
uiPage_t *prevpage;
char *title;
{
    uiTopLevelGfx_t *topgfx = &uiTopLevel.TopGfx;
    uiPageGfx_t *pagegfx = &page->Gfx;
    Widget topwdg, bottomwdg;

    /* Ahemm, we could of course try to salvage as much as possible
       of the widget tree, but why bother? No, just destroy everything
       except for the toplevel shell, no need to remove callbacks and
       event handlers
    */
    if (prevpage && prevpage->Settings.OnePageMode) {
	topwdg = prevpage->Gfx.TopWdg;
	uideletepageinternal(prevpage->Hierarchy->Address, prevpage->HText);
	uicreatepageform(page, title, topwdg);
    } else
	uicreatepageform(page, title, (Widget) NULL);

    page->Visible = TRUE;
    pagegfx->MenuWdg = uicreatepagemenu(page, pagegfx->FormWdg);
    XtManageChild(pagegfx->MenuWdg);
    pagegfx->ControlWdg = uicreatepagecontrol(page, pagegfx->FormWdg, title);
    XtManageChild(pagegfx->ControlWdg);
    page->Gfx.FindTextWdg = (Widget) NULL;
    if (HTAnchor_isIndex(page->HText->node_anchor)) {
	pagegfx->FindWdg = uicreatepagefind(page, pagegfx->FormWdg,
					    pagegfx->ControlWdg);
	XtManageChild(pagegfx->FindWdg);
	bottomwdg = pagegfx->FindWdg;
    } else
	bottomwdg = pagegfx->ControlWdg;
    pagegfx->DrawAreaWdg = uicreatepagescroll(page, pagegfx->FormWdg,
					    pagegfx->MenuWdg, bottomwdg);

    XtManageChild(pagegfx->FormWdg);
    XtRealizeWidget(pagegfx->TopWdg);

    /* Do setup that cannot be done before managing the form */
    XtVaSetValues(pagegfx->HScrollBarWdg,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, uiGetArg(pagegfx->FormWdg, XmNwidth) -
		  uiGetArg(pagegfx->DrawAreaWdg, XmNwidth), NULL);
    uiconnectpage(page);

    /* Is this really necessary?
    XmUpdateDisplay(XtParent(pagegfx->FormWdg));
    Nope, don't think so */

    return UI_OK;
}


static void uicreatepageform(page, title, topwdg)
uiPage_t *page;
char *title;
Widget topwdg;
{
    ArgList args;
    Cardinal nargs;
    char *tmptitle;

    if (topwdg)
	page->Gfx.TopWdg = topwdg;
    else
	page->Gfx.TopWdg = XtCreateApplicationShell("Page",
						topLevelShellWidgetClass,
						    NULL, 0);
    if (title) {
	tmptitle = uiMalloc(strlen(UI_ERWISE_TITLE) + strlen(title) + 4);
	sprintf(tmptitle, "%s - %s", UI_ERWISE_TITLE, title);
	XtVaSetValues(page->Gfx.TopWdg,
		      XmNtitle, tmptitle, NULL);
	uiFree(tmptitle);
    } else
	XtVaSetValues(page->Gfx.TopWdg,
		      XmNtitle, UI_ERWISE_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       NULL);
    page->Gfx.FormWdg = XmCreateForm(page->Gfx.TopWdg, "Form", args, nargs);
}


static char *uimenu[] =
{
    "Page", NULL,
    " Search", "Search",
    " Copy", "Copy",
    " List", "List",
    " Load to file", "Load to file",
    " Print", "Print",
    " Settings", "Settings",
    " ", NULL,
    " Close", "Close",
    "Movement", "Movement",
    " Top", "Top",
    " Bottom", "Bottom",
    " Prev tag", "Prev tag",
    " Next tag", "Next tag",
    "Hierarchy", NULL,
    " Home", "Home",
    " Recall", "Recall",
    " Back", "Back",
    " Prev page", "Prev page",
    " Next page", "Next page",
    " Close", NULL,
    "  hierarchy", "Close hierarchy",
    "Misc", NULL,
    " Connections", "Connections",
    " Controlpanel", "Controlpanel",
    " Defaults", "Defaults",
    "Help", NULL,
    " On function", "On function",
    " Manual", "Help",
    NULL
};

static Widget
 uicreatepagemenu(page, formwdg)
uiPage_t *page;
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget menuwdg, tmpmenuwdg;
    int itempos = 0;

    args = uiVaSetArgs(&nargs,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNtopAttachment, XmATTACH_FORM, NULL);
    menuwdg = XmCreateMenuBar(formwdg, "menubar", args, nargs);

    do {
	tmpmenuwdg = uicreatepagemenuitem(page, menuwdg, itempos, 0);
	if (ui_HELPMENU(uimenu[itempos]))
	    XtVaSetValues(menuwdg,
			  XmNmenuHelpWidget, tmpmenuwdg, NULL);
	itempos = uifindnextitem(itempos, 0);
    }
    while (ui_VALID(itempos));

    return menuwdg;
}


static Widget
 uicreatepagemenuitem(page, menuwdg, itempos, level)
uiPage_t *page;
Widget menuwdg;
int itempos;
int level;
{
    int childitempos = itempos;
    int childlevel = level + 1;
    char *label;
    ArgList args;
    Cardinal nargs;
    Widget pulldownwdg;
    Widget cascadewdg;
    Widget tmpwdg;
    uiActionData_t *actiondata;

    label = &uimenu[itempos][level];
    if (ui_VALID(uifindnextitem(itempos, childlevel))) {
	pulldownwdg = XmCreatePulldownMenu(menuwdg, "Pulldown", NULL, 0);
	args = uiVaSetArgs(&nargs, XmNsubMenuId, pulldownwdg, NULL);
	cascadewdg = XmCreateCascadeButtonGadget(menuwdg, label, args, nargs);
	XtManageChild(cascadewdg);

	while (ui_VALID(childitempos =
			uifindnextitem(childitempos, childlevel)))
	    uicreatepagemenuitem(page, pulldownwdg, childitempos, childlevel);

	tmpwdg = cascadewdg;
    } else {
	if (label[0]) {
	    if (ui_TOGGLE(label)) {
		tmpwdg = XmCreateToggleButtonGadget(menuwdg, label, NULL, 0);
	    } else {
		tmpwdg = XmCreatePushButtonGadget(menuwdg, label, NULL, 0);
		actiondata = uicreatepageactiondata(page, uimenu[itempos + 1]);
		XtAddCallback(tmpwdg, XmNactivateCallback,
			      (XtCallbackProc) uipageactivatecb,
			      (caddr_t) actiondata);
	    }
	} else
	    tmpwdg = XmCreateSeparatorGadget(menuwdg, "Separator", NULL, 0);
	XtManageChild(tmpwdg);
    }

    return tmpwdg;
}


static int uifindnextitem(itempos, level)
int itempos;
int level;
{
    int i = itempos + 2;

    while (uimenu[i] &&
	   (uimenu[i][level] == ' ' || uimenu[i][level] == '*'))
	i += 2;
    if (!uimenu[i] ||
    (level && uimenu[i][level - 1] != ' ' && uimenu[i][level - 1] != '*'))
	return UI_INVALID;

    return i;
}


static Widget
 uicreatepagecontrol(page, formwdg, title)
uiPage_t *page;
Widget formwdg;
char *title;
{
    ArgList args;
    Cardinal nargs;
    Widget controlwdg;

    args = uiVaSetArgs(&nargs,
		       XmNheight, 70,
		       XmNshadowThickness, 2,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    controlwdg = XmCreateForm(formwdg, "Control", args, nargs);

    uicreatepagelabel(controlwdg,
		HTAnchor_address((HTAnchor *) page->HText->node_anchor));

    uicreatepagebutton(page, controlwdg, "Prev tag", 1, 16);
    uicreatepagebutton(page, controlwdg, "Next tag", 17, 32);
    uicreatepagebutton(page, controlwdg, "Prev page", 35, 50);
    uicreatepagebutton(page, controlwdg, "Back", 51, 66);
    uicreatepagebutton(page, controlwdg, "Next page", 67, 82);
    uicreatepagebutton(page, controlwdg, "Close", 85, 99);

    return controlwdg;
}


static void uicreatepagelabel(parentwdg, label)
Widget parentwdg;
char *label;
{
    Widget labelwdg;
    XmString labelstr;

    labelstr = XmStringCreateSimple(label);
    labelwdg = XmCreateLabelGadget(parentwdg, "Label", (ArgList) NULL, 0);
    XtVaSetValues(labelwdg,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNtopOffset, 5,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, 5,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, 5,
		  XmNlabelString, labelstr, NULL);

    XtManageChild(labelwdg);
}


static void uicreatepagebutton(page, parentwdg, name, leftpos, rightpos)
uiPage_t *page;
Widget parentwdg;
char *name;
int leftpos;
int rightpos;
{
    Widget tmpwdg;
    uiActionData_t *actiondata;

    tmpwdg = XmCreatePushButtonGadget(parentwdg, name, (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, 5,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, leftpos,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, rightpos, NULL);
    actiondata = uicreatepageactiondata(page, name);
    XtAddCallback(tmpwdg, XmNactivateCallback, uipageactivatecb,
		  (caddr_t) actiondata);

    XtManageChild(tmpwdg);
}


static Widget
 uicreatepagefind(page, formwdg, controlwdg)
uiPage_t *page;
Widget formwdg;
Widget controlwdg;
{
    Widget framewdg, findformwdg, buttonwdg, textwdg;
    uiActionData_t *actiondata;
    XmString labelstr;

    labelstr = XmStringCreateSimple("     Find:     ");

    framewdg = XmCreateFrame(formwdg, "FindFrame", (ArgList) NULL, 0);
    XtVaSetValues(framewdg,
		  XmNshadowType, XmSHADOW_OUT,
		  XmNmarginWidth, 2,
		  XmNmarginHeight, 2,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, controlwdg,
		  XmNbottomOffset, 1, NULL);

    findformwdg = XmCreateForm(framewdg, "FindForm", (ArgList) NULL, 0);
    XtManageChild(findformwdg);

    buttonwdg = XmCreatePushButtonGadget(findformwdg, "FindButton",
					 (ArgList) NULL, 0);
    XtVaSetValues(buttonwdg,
		  XmNlabelString, labelstr,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM, NULL);
    actiondata = uicreatepageactiondata(page, "IndexFind");
    XtAddCallback(buttonwdg, XmNactivateCallback, uipageactivatecb,
		  (caddr_t) actiondata);
    XmStringFree(labelstr);
    XtManageChild(buttonwdg);

    textwdg = XmCreateText(findformwdg, "FindText", (ArgList) NULL, 0);
    XtVaSetValues(textwdg,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, buttonwdg,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM, NULL);
    XtAddCallback(textwdg, XmNactivateCallback, uipageactivatecb,
		  (caddr_t) actiondata);
    XtAddCallback(textwdg, XmNactivateCallback,
	      (XtCallbackProc) uiDialogVariableCB, (caddr_t) "FindText");
    XtAddCallback(textwdg, XmNlosingFocusCallback,
	      (XtCallbackProc) uiDialogVariableCB, (caddr_t) "FindText");
    XtAddCallback(textwdg, XmNvalueChangedCallback,
	      (XtCallbackProc) uiDialogVariableCB, (caddr_t) "FindText");
    (void) uiAddWidgetInfo("FindText", textwdg, uiWTtext);	/* Ignore */
    page->Gfx.FindTextWdg = textwdg;
    XtManageChild(textwdg);

    return framewdg;
}


static Widget
 uicreatepagescroll(page, formwdg, menuwdg, bottomwdg)
uiPage_t *page;
Widget formwdg;
Widget menuwdg;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget drawwdg, hsbwdg, vsbwdg;
    uiActionData_t *actiondata;

    args = uiVaSetArgs(&nargs,
		       XmNorientation, XmHORIZONTAL,
		       XmNwidth, page->Layout.Width,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, 2, NULL);
    hsbwdg = XmCreateScrollBar(formwdg, "HScrollBar", args, nargs);
    page->Gfx.HScrollBarWdg = hsbwdg;
    actiondata = uicreatepageactiondata(page, "HScrollBar");
    XtAddCallback(hsbwdg, XmNvalueChangedCallback,
	      (XtCallbackProc) uipageactivatecb, (caddr_t *) actiondata);
    UiAttachCallback("HScrollBar", uipagescrollbarcb, (void *) "HScrollBar");

    args = uiVaSetArgs(&nargs,
		       XmNorientation, XmVERTICAL,
		       XmNheight, page->Layout.Height,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, menuwdg,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, hsbwdg, NULL);
    vsbwdg = XmCreateScrollBar(formwdg, "VScrollBar", args, nargs);
    page->Gfx.VScrollBarWdg = vsbwdg;
    actiondata = uicreatepageactiondata(page, "VScrollBar");
    XtAddCallback(vsbwdg, XmNvalueChangedCallback,
	      (XtCallbackProc) uipageactivatecb, (caddr_t *) actiondata);
    UiAttachCallback("VScrollBar", uipagescrollbarcb, (void *) "VScrollBar");

    args = uiVaSetArgs(&nargs,
		       XmNwidth, page->Layout.Width,
		       XmNheight, page->Layout.Height,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, menuwdg,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_WIDGET,
		       XmNrightWidget, vsbwdg,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, hsbwdg, NULL);
    drawwdg = XmCreateDrawingArea(formwdg, "Draw", args, nargs);
    XtAddCallback(drawwdg, XmNexposeCallback, (XtCallbackProc) uipageexposecb,
		  (caddr_t) page);
    XtAddCallback(drawwdg, XmNresizeCallback, (XtCallbackProc) uipageresizecb,
		  (caddr_t) page);
    actiondata = uicreatepageactiondata(page, "PageInput");
    XtAddCallback(drawwdg, XmNinputCallback, (XtCallbackProc) uipageactivatecb,
		  (caddr_t) actiondata);
    XtAddEventHandler(drawwdg, KeyPressMask, FALSE, uipagekludgecb,
		      (caddr_t) actiondata);
    UiAttachCallback("PageInput", uipageinputcb, (void *) NULL);

    XtManageChild(drawwdg);
    XtManageChild(hsbwdg);
    XtManageChild(vsbwdg);

    return drawwdg;
}


static void uiconnectpage(page)
uiPage_t *page;
{
    Widget drawwdg = page->Gfx.DrawAreaWdg;
    Widget hsbwdg = page->Gfx.HScrollBarWdg;
    Widget vsbwdg = page->Gfx.VScrollBarWdg;
    int tmpwidth;

    XlSetupText(XtDisplay(drawwdg), XtWindow(drawwdg),
		(Pixel) uiGetArg(drawwdg, XmNforeground),
		(Pixel) uiGetArg(drawwdg, XmNbackground), page->HText);

    if (page->Settings.UseFixed)
	tmpwidth = page->Settings.FixedWidth - page->Settings.LeftMargin -
	    page->Settings.RightMargin;
    else
	tmpwidth = page->Layout.Width - page->Settings.LeftMargin -
	    page->Settings.RightMargin;

    XlFormatText(page->Settings.LeftMargin, tmpwidth,
		 page->Settings.TopMargin, &page->Layout.VirtualWidth,
		 &page->Layout.VirtualHeight, page->HText);

    if (page->Layout.VirtualWidth > tmpwidth + page->Settings.LeftMargin)
	page->Layout.VirtualWidth += page->Settings.RightMargin;
    else
	page->Layout.VirtualWidth = tmpwidth + page->Settings.LeftMargin +
	    page->Settings.RightMargin;
    page->Layout.VirtualHeight += page->Settings.BottomMargin;

    if (page->HTextObject)
	XlGetCoordinates(&page->Layout.X, &page->Layout.Y, page->HTextObject);
    page->Layout.X = 0;
    uipageupdatescrollbars(page);
    if (page->HTextObject)
	XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);

    XlSetCursor(page->HText, page->HTextObject);
}


static void uipageupdatescrollbars(page)
uiPage_t *page;
{
    Widget hsbwdg = page->Gfx.HScrollBarWdg;
    Widget vsbwdg = page->Gfx.VScrollBarWdg;
    int maxwidth, maxheight;
    int hsbwidth, vsbheight;

    maxwidth = page->Layout.VirtualWidth;
    if (page->Layout.X + page->Layout.Width > maxwidth)
	page->Layout.X -= ((page->Layout.X + page->Layout.Width) - maxwidth);
    hsbwidth = (page->Layout.Width > maxwidth) ? maxwidth : page->Layout.Width;
    if (page->Layout.X < 0)
	page->Layout.X = 0;

    maxheight = page->Layout.VirtualHeight;
    if (page->Layout.Y + page->Layout.Height > maxheight)
	page->Layout.Y -= ((page->Layout.Y + page->Layout.Height) - maxheight);
    vsbheight =
	(page->Layout.Height > maxheight) ? maxheight : page->Layout.Height;
    if (page->Layout.Y < 0)
	page->Layout.Y = 0;


    XtVaSetValues(hsbwdg,
		  XmNmaximum, maxwidth,
		  XmNvalue, page->Layout.X,
		  XmNsliderSize, hsbwidth,
		  XmNincrement, hsbwidth / 10,
		  XmNpageIncrement, hsbwidth, NULL);
    XtVaSetValues(vsbwdg,
		  XmNmaximum, maxheight,
		  XmNvalue, page->Layout.Y,
		  XmNsliderSize, vsbheight,
		  XmNincrement, vsbheight / 10,
		  XmNpageIncrement, vsbheight, NULL);
}


uiActionData_t *
 uicreatepageactiondata(page, actionname)
uiPage_t *page;
char *actionname;
{
    uiPageCBList_t *tmppagecb = page->Callbacks;

    if (!tmppagecb)
	tmppagecb =
	    page->Callbacks = (uiPageCBList_t *) uiMalloc(sizeof(*tmppagecb));
    else {
	while (tmppagecb->Next)
	    tmppagecb = tmppagecb->Next;
	tmppagecb = tmppagecb->Next =
	    (uiPageCBList_t *) uiMalloc(sizeof(*tmppagecb));
    }
    tmppagecb->ActionData.ActionName = actionname;
    tmppagecb->ActionData.Page = page;
    tmppagecb->Next = (uiPageCBList_t *) NULL;

    return &tmppagecb->ActionData;
}


static void uisetcurrentpage(page)
uiPage_t *page;
{
    uiPage_t *oldpage;

    oldpage = uiPageInfo.CurrentPage;
    uiPageInfo.CurrentPage = page;

    if (oldpage != page) {
	uiPageSettingsUpdateDialog();
	uiSearchUpdateDialog();
	uiControlPanelUpdateDialog();
	uiPrintUpdateDialog();
	uiRecallUpdateDialog();
	uiConnectionsUpdateDialog();
    }
}


static int uideletepageinternal(topaddress, htext)
char *topaddress;
HText_t *htext;
{
    uiHierarchy_t *hierarchy;
    uiPage_t *page;
    Widget topwdg;

    if (!(hierarchy = uifindhierarchy(topaddress)))
	return UI_ERROR;

    if (!(page = uifindpage(htext, hierarchy)))
	return UI_ERROR;

/*    XlDeleteText(htext);*/

    topwdg = page->Gfx.TopWdg;
    page = hierarchy->Pages;
    while (page) {
	if (page->Gfx.TopWdg == topwdg) {
	    if (page->Visible)
		XtDestroyWidget(page->Gfx.FormWdg);
	    page->Visible = FALSE;
	}
	page = page->Next;
    }
/*    uideletepage(page, hierarchy);*/

    return UI_OK;
}


static void uipageactivatecb(wdg, actiondata, calldata)
Widget wdg;
uiActionData_t *actiondata;
XmAnyCallbackStruct *calldata;
{
    uiAction_t *tmpaction;

    uiPageInfo.Wdg = wdg;
    uiPageInfo.CallData = (void *) calldata;
    uisetcurrentpage(actiondata->Page);
    if (actiondata->Page->Gfx.FindTextWdg) {
	(void) uiAddWidgetInfo("FindText", actiondata->Page->Gfx.FindTextWdg,
			       uiWTtext);	/* Ignore */
	(void) uiDialogVariableCB((Widget) NULL, "FindText",
				  (XmAnyCallbackStruct *) NULL);	/* Hmm? */
    }
    if (tmpaction = uiFindAction(actiondata->ActionName)) {
	uiDefineCursor(uiBusyCursor);
	if (uiHelpOnActionCB) {
	    (*uiHelpOnActionCB) (actiondata->ActionName);
	    uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
	} else
	    (*tmpaction->Callback) (actiondata->Page->Hierarchy->Address,
				    actiondata->Page->HText,
				    actiondata->Page->HTextObject,
				    tmpaction->Parameter);
	uiUndefineCursor();
    }
}


/*
 *  Massive braindamage ahead ... default translation in
 *  drawingarea-widget for return is activate, even if the widget
 *  doesn't support activate. Gawd! Anyway, we have to resort to this:
 */

static void uipagekludgecb(wdg, actiondata, event)
Widget wdg;
uiActionData_t *actiondata;
XEvent *event;
{
    XKeyEvent *kevent = (XKeyEvent *) event;
    XmAnyCallbackStruct tmpstruct;

    if (XLookupKeysym(kevent, kevent->state) == XK_Return) {
	tmpstruct.event = event;
	uipageactivatecb(wdg, actiondata, &tmpstruct);
    }
}


static void uipageexposecb(wdg, page, calldata)
Widget wdg;
uiPage_t *page;
XmDrawingAreaCallbackStruct *calldata;
{
    XExposeEvent *event = (XExposeEvent *) calldata->event;

#ifdef DEBUG
    printf("Explode: %d %d %d %d %x\n",
	   event->x, event->y, event->width, event->height, page->HText);
#endif
    XlRedraw(event->x, event->y, event->width, event->height, page->HText);
}


static void uipageresizecb(wdg, page, calldata)
Widget wdg;
uiPage_t *page;
XmDrawingAreaCallbackStruct *calldata;
{
    Widget hsbwdg = page->Gfx.HScrollBarWdg;
    Widget vsbwdg = page->Gfx.VScrollBarWdg;
    HTextObject_t *htextobject;

#ifdef DEBUG
    printf("Resize: %d %d\n",
	   (int) ((Dimension) uiGetArg(wdg, XmNwidth)),
	   (int) ((Dimension) uiGetArg(wdg, XmNheight)));
#endif

    if (!XtIsRealized(page->Gfx.DrawAreaWdg))
	return;

    XlClearWindow(page->Layout.Width, page->Layout.Height, page->HText);

    page->Layout.Width = (int) ((Dimension) uiGetArg(wdg, XmNwidth));
    page->Layout.Height = (int) ((Dimension) uiGetArg(wdg, XmNheight));

    uiPageUpdateWindow(page);
}


static void uipageinputcb(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Widget wdg = uiPageInfo.Wdg;
    uiPage_t *page = uiPageInfo.CurrentPage;
    XmDrawingAreaCallbackStruct *calldata =
    (XmDrawingAreaCallbackStruct *) uiPageInfo.CallData;
    XButtonEvent *bevent = (XButtonEvent *) calldata->event;
    XKeyEvent *kevent = (XKeyEvent *) calldata->event;
    static int dragx, dragy;
    static Time oldtime = (Time) 0;
    uiAction_t *tmpaction;
    uiKey_t *tmpkey;
    KeySym keysym;
    char *keysymstring;
    switch (bevent->type) {
    case ButtonPress:
	switch (bevent->button) {
	case 1:
	    page->HTextObject = XlLocateHTextObject(bevent->x, bevent->y,
						    page->HText);
	    XlSetCursor(htext, page->HTextObject);
	    if (page->HTextObject) {
		if (bevent->time - oldtime <
		    uiTopLevel.GlobalSettings.DoubleClickTime)
		    tmpaction = uiFindAction("Get page");
		else
		    tmpaction = uiFindAction("Click page");
		if (tmpaction) {
		    uiDefineCursor(uiBusyCursor);
		    if (uiHelpOnActionCB) {
			(*uiHelpOnActionCB) ("Get page");
			uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
		    } else
			(*tmpaction->Callback) (topaddress, htext,
						page->HTextObject,
						tmpaction->Parameter);
		    uiUndefineCursor();
		}
		oldtime = (Time) 0;
	    }
	    oldtime = bevent->time;
	    break;
	case 2:
	    dragx = bevent->x;
	    dragy = bevent->y;
	    break;
	}
	break;
    case ButtonRelease:
	switch (bevent->button) {
	case 2:
	    page->Layout.X -= (bevent->x - dragx);
	    page->Layout.Y -= (bevent->y - dragy);
	    uipageupdatescrollbars(page);
	    XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);
	    break;
	}
	break;
    case KeyPress:
	keysym = XLookupKeysym(kevent, 0);
	if (keysym != NoSymbol && (keysymstring = XKeysymToString(keysym))) {
	    if (tmpkey = uiFindKey(keysymstring, kevent->state)) {
		uiDefineCursor(uiBusyCursor);
		if (uiHelpOnActionCB) {
		    (*uiHelpOnActionCB) (keysymstring);
		    uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
		} else
		    (*tmpkey->Callback) (topaddress,
					 htext,
					 page->HTextObject,
					 tmpkey->Parameter);
		uiUndefineCursor();
	    }
	}
	break;
    }
}


static void uipagescrollbarcb(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    XmUpdateDisplay(uiPageInfo.CurrentPage->Gfx.DrawAreaWdg);

    if (!strcmp("HScrollBar", (char *) parameter))
	uiPageInfo.CurrentPage->Layout.X =
	    ((XmScrollBarCallbackStruct *) uiPageInfo.CallData)->value;
    else
	uiPageInfo.CurrentPage->Layout.Y =
	    ((XmScrollBarCallbackStruct *) uiPageInfo.CallData)->value;

#ifdef DEBUG
    printf("scrollbar: %d %d\n", uiPageInfo.CurrentPage->Layout.X,
	   uiPageInfo.CurrentPage->Layout.Y);
#endif
    XlMoveWindow(uiPageInfo.CurrentPage->Layout.X,
		 uiPageInfo.CurrentPage->Layout.Y,
		 uiPageInfo.CurrentPage->HText);
}


static void uipagedowncb(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiPage_t *page = uiPageInfo.CurrentPage;

    page->Layout.Y = page->Layout.Y + page->Layout.Height;

    uipageupdatescrollbars(page);
    XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);
}


static void uipageupcb(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiPage_t *page = uiPageInfo.CurrentPage;

    page->Layout.Y = page->Layout.Y - page->Layout.Height;

    uipageupdatescrollbars(page);
    XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);
}


static void uipagetopcb(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiPage_t *page = uiPageInfo.CurrentPage;

    page->Layout.Y = 0;

    uipageupdatescrollbars(page);
    XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);
}


static void uipagebottomcb(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiPage_t *page = uiPageInfo.CurrentPage;

    page->Layout.Y = page->Layout.VirtualHeight - page->Layout.Height;

    uipageupdatescrollbars(page);
    XlMoveWindow(page->Layout.X, page->Layout.Y, page->HText);
}
