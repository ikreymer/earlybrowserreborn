static char *rcsid = "$Id$";

#include "UiIncludes.h"

static void uibinddefaultsvariables(void);
static void uiupdatedefaultsvariables(void);
static Widget uicreatedefaultsformdialog();
static Widget uicreatedefaultslabel(Widget parent, Widget bottomwdg);
static Widget
 uicreatedefaultsitem(char *name, Widget parent,
		       Widget bottomwdg);
static Widget uicreatedefaultsclose(Widget parent);
static void uidefaultsclosecb(char *address, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);


int UiDisplayDefaultsDialog()
{
    uiDefaultsGfx_t *defaultsgfx = &uiTopLevel.DefaultsGfx;

    uibinddefaultsvariables();

    if (defaultsgfx->FormWdg) {
	XtMapWidget(XtParent(defaultsgfx->FormWdg));

	uiupdatedefaultsvariables();

	return UI_OK;
    }
    defaultsgfx->FormWdg = uicreatedefaultsformdialog();
    defaultsgfx->CloseWdg = uicreatedefaultsclose(defaultsgfx->FormWdg);
    defaultsgfx->PageSettings = uicreatedefaultsitem("Page Settings window",
						     defaultsgfx->FormWdg,
						  defaultsgfx->CloseWdg);
    defaultsgfx->RecallWindow =
	uicreatedefaultsitem("Recall window",
			     defaultsgfx->FormWdg,
			     defaultsgfx->PageSettings);
    defaultsgfx->ListWindow = uicreatedefaultsitem("List window",
						   defaultsgfx->FormWdg,
					      defaultsgfx->RecallWindow);
    defaultsgfx->ControlPanel = uicreatedefaultsitem("Control panel",
						     defaultsgfx->FormWdg,
						defaultsgfx->ListWindow);
    defaultsgfx->SearchWindow =
	uicreatedefaultsitem("Search window", defaultsgfx->FormWdg,
			     defaultsgfx->ControlPanel);
    defaultsgfx->LabelWdg = uicreatedefaultslabel(defaultsgfx->FormWdg,
					      defaultsgfx->SearchWindow);

    XtManageChild(defaultsgfx->FormWdg);
    XtRealizeWidget(XtParent(defaultsgfx->FormWdg));

    uiupdatedefaultsvariables();

    return UI_OK;
}


void uiDefaultsUpdateDialog()
{
    if (uiTopLevel.DefaultsGfx.FormWdg) {
	if (uiPageInfo.CurrentPage) {
	    uibinddefaultsvariables();
	    uiupdatedefaultsvariables();
	} else
	    XtUnmapWidget(XtParent(uiTopLevel.DefaultsGfx.FormWdg));
    }
}


static void uibinddefaultsvariables()
{
    UiBindVariable("Page Settings window",
	       (void *) &uiTopLevel.GlobalSettings.PageSettingsPlacement,
		   uiVTint);
    UiBindVariable("Recall window",
		   (void *) &uiTopLevel.GlobalSettings.RecallPlacement,
		   uiVTint);
    UiBindVariable("List window",
		   (void *) &uiTopLevel.GlobalSettings.ListPlacement,
		   uiVTint);
    UiBindVariable("Control panel",
	       (void *) &uiTopLevel.GlobalSettings.ControlPanelPlacement,
		   uiVTint);
    UiBindVariable("Search window",
		   (void *) &uiTopLevel.GlobalSettings.SearchPlacement,
		   uiVTint);
}


static void uiupdatedefaultsvariables()
{
    UiUpdateVariable("Page Settings window");
    UiUpdateVariable("Recall window");
    UiUpdateVariable("List window");
    UiUpdateVariable("Control panel");
    UiUpdateVariable("Search window");
}


static Widget
 uicreatedefaultsformdialog()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("Defaults",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_DEFAULTS_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "Defaults", args, nargs);

    return formwdg;
}

static Widget
 uicreatedefaultslabel(parent, bottomwdg)
Widget parent;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget labelwdg;
    XmString labelstr;

    labelstr = XmStringCreateSimple("Follow pointer");

    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, UI_DEFAULTS_WDG_OFFSET,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_DEFAULTS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(parent, "DEFAULTSLabel", args, nargs);
    XtManageChild(labelwdg);

    return labelwdg;
}


static Widget
 uicreatedefaultsitem(name, parent, bottomwdg)
char *name;
Widget parent;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget togglewdg;
    static uiActionData_t actiondata;

    args = uiVaSetArgs(&nargs,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_DEFAULTS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    formwdg = XmCreateForm(parent, name, args, nargs);
    XtManageChild(formwdg);

    togglewdg = XmCreateToggleButtonGadget(formwdg, name,
					   (ArgList) NULL, 0);
    XtVaSetValues(togglewdg,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, UI_DEFAULTS_WDG_OFFSET,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, UI_DEFAULTS_WDG_OFFSET, NULL);
    XtAddCallback(togglewdg, XmNvalueChangedCallback,
		  (XtCallbackProc) uiDialogVariableCB, (caddr_t) name);
    /* Ignore */
    (void) uiAddWidgetInfo(name, togglewdg, uiWTcheckbutton);
    XtManageChild(togglewdg);

    return formwdg;
}


static Widget
 uicreatedefaultsclose(parent)
Widget parent;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;
    Widget tmpwdg;
    static uiActionData_t actiondata;

    tmpwdg = XmCreatePushButtonGadget(parent, "Close", (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNwidth, 70,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, UI_DEFAULTS_WDG_OFFSET,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, UI_DEFAULTS_WDG_OFFSET, NULL);
    actiondata.ActionName = "DefaultsClose";
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(tmpwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    UiAttachCallback("DefaultsClose", uidefaultsclosecb,
		     "DefaultsGfx.FormWdg");

    XtManageChild(tmpwdg);

    args = uiVaSetArgs(&nargs,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, tmpwdg,
		       XmNbottomOffset, UI_DEFAULTS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(parent, "DefaultsSeparator",
					   args, nargs);
    XtManageChild(separatorwdg);

    return separatorwdg;
}


static void setplacementvalue(uiconfigsetfp, table, item, placementvalue)
void *(*uiconfigsetfp) (void *table, char *item, void *value);
void *table;
char *item;
int placementvalue;
{
    if (placementvalue)
	uiconfigsetfp(table, item, "Mouse");
    else
	uiconfigsetfp(table, item, "Fixed");
}


static void uidefaultsclosecb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    uiGlobalSettings_t *gs = &uiTopLevel.GlobalSettings;
    void *table;

    table = uiConfigPF((void *) NULL, C_GLOBALSETTINGS);
    setplacementvalue(uiConfigSetPF, table, C_SEARCH, gs->SearchPlacement);
    setplacementvalue(uiConfigSetPF, table, C_CONTROLPANEL, gs->ControlPanelPlacement);
    setplacementvalue(uiConfigSetPF, table, C_LIST, gs->ListPlacement);
    setplacementvalue(uiConfigSetPF, table, C_RECALL, gs->RecallPlacement);
    setplacementvalue(uiConfigSetPF, table, C_PAGESETTINGS, gs->PageSettingsPlacement);

    XtUnmapWidget(XtParent(uiTopLevel.DefaultsGfx.FormWdg));
}
