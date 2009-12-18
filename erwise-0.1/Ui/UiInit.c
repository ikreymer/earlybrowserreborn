static char *rcsid = "$Id: UiInit.c,v 1.3 1992/03/26 18:13:50 kny Exp kny $";

#include "UiIncludes.h"


static void uisetupdefaults(int argc, char *argv[],
			     void *(*configpf) (void *table, char *item));
static Widget uicreatebutton(Widget parentwdg, char *name, int leftpos,
			      int rightpos);

static void uitopactivatecb(Widget wdg, XtPointer actionname,
			     XtPointer calldata);


uiTopLevel_t uiTopLevel;
Cursor uiBusyCursor, uiArrowCursor;
void *(*uiConfigPF) (void *table, char *item);
void *(*uiConfigSetPF) (void *table, char *item, void *value);


int UiInitialize(argc, argv, configpf, configsetpf)
int argc;
char *argv[];
void *(*configpf) (void *table, char *item);
void *(*configsetpf) (void *table, char *item, void *value);
{
    uiTopLevelGfx_t *topgfx = &uiTopLevel.TopGfx;

    uiConfigPF = configpf;
    uiConfigSetPF = configsetpf;

    topgfx->TopWdg = XtInitialize(argv[0], "Erwise", NULL, 0, &argc, argv);
    topgfx->FormWdg = XmCreateForm(topgfx->TopWdg, "Form",
				   (ArgList) NULL, 0);
    XtVaSetValues(topgfx->FormWdg,
		  XmNwidth, 280,
		  XmNheight, 70, NULL);
    topgfx->InfoWdg = uicreatebutton(topgfx->FormWdg, "Info", 0, 25);
    topgfx->OpenWdg = uicreatebutton(topgfx->FormWdg, "Open", 25, 50);
    topgfx->QuitWdg = uicreatebutton(topgfx->FormWdg, "Quit", 50, 75);
    topgfx->HelpWdg = uicreatebutton(topgfx->FormWdg, "Help", 75, 100);

    XtManageChild(topgfx->FormWdg);
    XtRealizeWidget(topgfx->TopWdg);

    XlSetupResources(configpf);

    uisetupdefaults(argc, argv, configpf);

    uiBusyCursor = XCreateFontCursor(XtDisplay(topgfx->TopWdg), XC_watch);
    uiArrowCursor = XCreateFontCursor(XtDisplay(topgfx->TopWdg),
				      XC_right_ptr);

    return UI_OK;
}


void UiMainLoop()
{
    XtMainLoop();
}


int PlaceValue(value)
char *value;
{
    if (value && !strncasecmp(value, "mouse", strlen("mouse")))
	return TRUE;
    return FALSE;
}


static void uisetupdefaults(argc, argv, configpf)
int argc;
char *argv[];
void *(*configpf) (void *table, char *item);
{
    uiGlobalSettings_t *gs = &uiTopLevel.GlobalSettings;
    void *table;

    if (table = configpf((void *) NULL, C_GLOBALSETTINGS)) {
	gs->TopMargin = atoi((char *) configpf(table, C_TOPMARGIN));
	gs->BottomMargin = atoi((char *) configpf(table, C_BOTTOMMARGIN));
	gs->LeftMargin = atoi((char *) configpf(table, C_LEFTMARGIN));
	gs->RightMargin = atoi((char *) configpf(table, C_RIGHTMARGIN));
	gs->UseFixed = TruthValue((char *) configpf(table, C_FIXEDWIDTHMODE));
	gs->FixedWidth = atoi((char *) configpf(table, C_FIXEDWIDTH));
	gs->OnePageMode = TruthValue((char *) configpf(table, C_ONEPAGEMODE));
	gs->Width = atoi((char *) configpf(table, C_WIDTH));
	gs->Height = atoi((char *) configpf(table, C_HEIGHT));
	gs->DoubleClickTime =
	    (Time) atoi((char *) configpf(table, C_DOUBLECLICKTIME));
	gs->SearchPlacement = PlaceValue((char *) configpf(table, C_SEARCH));
	gs->ControlPanelPlacement =
	    PlaceValue((char *) configpf(table, C_CONTROLPANEL));
	gs->ListPlacement =
	    PlaceValue((char *) configpf(table, C_LIST));
	gs->RecallPlacement =
	    PlaceValue((char *) configpf(table, C_RECALL));
	gs->PageSettingsPlacement =
	    PlaceValue((char *) configpf(table, C_PAGESETTINGS));
    } else
	uiDisplayWarning("failed to get configurations for global settings");

    if (table = configpf((void *) NULL, C_DEFAULTS))
	uiSelectionArray = (char **) configpf(table, C_DEFAULTSTABLE);
    else
	uiDisplayWarning("failed to get configurations for selection");

    uiPageAttachCallbacks();
    uiPageDefineKeys();
}


static Widget
 uicreatebutton(parentwdg, name, leftpos, rightpos)
Widget parentwdg;
char *name;
int leftpos;
int rightpos;
{
    Widget tmpwdg;

    tmpwdg = XmCreatePushButtonGadget(parentwdg, name,
				      (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, leftpos,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, rightpos, NULL);
    XtAddCallback(tmpwdg, XmNactivateCallback, uitopactivatecb,
		  (XtPointer) name);
    XtManageChild(tmpwdg);

    return tmpwdg;
}


static void uitopactivatecb(wdg, actionname, calldata)
Widget wdg;
XtPointer actionname;
XtPointer calldata;
{
    uiAction_t *tmpaction;

    if (tmpaction = uiFindAction((char *) actionname))
	if (uiHelpOnActionCB) {
	    (*uiHelpOnActionCB) (actionname);
	    uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
	} else
	    (*tmpaction->Callback) ((char *) NULL, (HText_t *) NULL,
				    (HTextObject_t *) NULL,
				    tmpaction->Parameter);
}
