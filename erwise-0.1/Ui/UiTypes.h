typedef struct {
    int TopMargin, BottomMargin;
    int LeftMargin, RightMargin;
    int UseFixed;
    int FixedWidth;
    int OnePageMode;
} uiPageSettings_t;


typedef struct {
    int X, Y;
    int Width, Height;
    int VirtualWidth, VirtualHeight;
} uiPageLayout_t;


typedef struct {
    Widget TopWdg;
    Widget FormWdg;
    Widget MenuWdg;
    Widget DrawAreaWdg;
    Widget HScrollBarWdg;
    Widget VScrollBarWdg;
    Widget FindFrameWdg;
    Widget FindWdg;
    Widget FindTextWdg;
    Widget ControlWdg;
} uiPageGfx_t;


struct _uipage;

typedef struct {
    char *ActionName;
    struct _uipage *Page;
} uiActionData_t;


typedef struct _pagecblist {
    uiActionData_t ActionData;
    struct _pagecblist *Next;
} uiPageCBList_t;


struct _uihierarchy;

typedef struct _uipage {
    HText_t *HText;
    HTextObject_t *HTextObject;
    int Visible;
    struct _uihierarchy *Hierarchy;
    uiPageSettings_t Settings;
    uiPageLayout_t Layout;
    uiPageGfx_t Gfx;
    uiPageCBList_t *Callbacks;
    struct _uipage *Next;
} uiPage_t;


typedef struct _uihierarchy {
    char *Address;
    uiPage_t *Pages;
    struct _uihierarchy *Next;
} uiHierarchy_t;


typedef struct _uiaction {
    char *Name;
    void (*Callback) (char *address, HText_t * htext,
		       HTextObject_t * htextobject, void *parameter);
    void *Parameter;
    struct _uiaction *Next;
} uiAction_t;


typedef struct _uikey {
    char *Name;
    int Modifier;
    void (*Callback) (char *address, HText_t * htext,
		       HTextObject_t * htextobject, void *parameter);
    void *Parameter;
    struct _uikey *Next;
} uiKey_t;


typedef enum {
    uiWTtext,
    uiWToptionmenu,
    uiWTradiobox,
    uiWTcheckbutton,
    uiWTscale
} uiWdgType_t;


typedef struct _uivariable {
    char *Name;
    void *Value;
    uiVarType_t VarType;
    Widget Wdg;
    uiWdgType_t WdgType;
    struct _uivariable *Next;
} uiVariable_t;


typedef struct {
    int TopMargin, BottomMargin;
    int LeftMargin, RightMargin;
    int UseFixed;
    int FixedWidth;
    int OnePageMode;
    int Width;
    int Height;
    int SearchPlacement;
    int ControlPanelPlacement;
    int ListPlacement;
    int RecallPlacement;
    int PageSettingsPlacement;
    Time DoubleClickTime;
} uiGlobalSettings_t;


typedef struct {
    Widget TopWdg;
    Widget FormWdg;
    Widget InfoWdg;
    Widget OpenWdg;
    Widget QuitWdg;
    Widget HelpWdg;
} uiTopLevelGfx_t;


typedef struct {
    Widget FormWdg;
    Widget FSBoxWdg;
    Widget SeparatorWdg;
    Widget ListWdg;
    Widget AddWdg;
    Widget DeleteWdg;
} uiSelectionBoxGfx_t;


typedef struct {
    Widget FormWdg;
    Widget TextWdg;
    Widget LevelWdg;
    Widget LevelFormWdg;
    Widget LevelDownWdg;
    Widget LevelUpWdg;
    Widget CaseWdg;
    Widget BackwardWdg;
    Widget ForwardWdg;
    Widget LabelWdg;
    Widget CloseWdg;
} uiSearchGfx_t;


typedef struct {
    Widget FormWdg;
    Widget LabelWdg;
    Widget LeftMarginWdg;
    Widget RightMarginWdg;
    Widget TopMarginWdg;
    Widget BottomMarginWdg;
    Widget LeftMarginTextWdg;
    Widget RightMarginTextWdg;
    Widget TopMarginTextWdg;
    Widget BottomMarginTextWdg;
    Widget UseFixedWdg;
    Widget UseFixedTextWdg;
    Widget SinglePageWdg;
} uiPageSettingsGfx_t;


typedef struct {
    Widget FormWdg;
    Widget PrintCmdWdg;
    Widget PrintFileWdg;
    Widget LeftMarginWdg;
    Widget RightMarginWdg;
    Widget TopMarginWdg;
    Widget BottomMarginWdg;
    Widget LeftMarginTextWdg;
    Widget RightMarginTextWdg;
    Widget TopMarginTextWdg;
    Widget BottomMarginTextWdg;
    Widget WidthWdg;
} uiPrintGfx_t;


typedef struct {
    Widget FormWdg;
    Widget LabelWdg;
    Widget ListWdg;
    Widget SeparatorWdg;
    Widget OpenWdg;
    Widget CloseWdg;
} uiListGfx_t;


typedef struct {
    Widget FormWdg;
    Widget LabelWdg;
    Widget ListWdg;
    Widget SeparatorWdg;
    Widget OpenWdg;
    Widget CloseWdg;
} uiRecallGfx_t;


typedef struct {
    Widget FormWdg;
    Widget OkWdg;
} uiInfoGfx_t;


typedef struct {
    Widget SearchWdg;
    Widget FormWdg;
    Widget HomeWdg;
    Widget RecallWdg;
    Widget CloseWdg;
    Widget LabelWdg;
    Widget DialogCloseWdg;
    Widget ListWdg;
} uiControlPanelGfx_t;


typedef struct {
    Widget FormWdg;
    Widget LabelWdg;
    Widget SearchWindow;
    Widget ControlPanel;
    Widget ListWindow;
    Widget RecallWindow;
    Widget PageSettings;
    Widget CloseWdg;
} uiDefaultsGfx_t;


typedef struct {
    Widget FormWdg;
    Widget LabelWdg;
    Widget KillWdg;
    Widget CloseWdg;
    Widget SeparatorWdg;
    Widget ListWdg;
} uiConnectionsGfx_t;


typedef struct {
    Widget FormWdg;
    Widget FSBoxWdg;
} uiFileSelectionGfx_t;


typedef struct {
    uiHierarchy_t *Hierarchies;
    uiAction_t *Actions;
    uiKey_t *Keys;
    uiVariable_t *Variables;
    uiGlobalSettings_t GlobalSettings;
    uiTopLevelGfx_t TopGfx;
    uiSelectionBoxGfx_t SBGfx;
    uiSearchGfx_t SearchGfx;
    uiPageSettingsGfx_t PageSettingsGfx;
    uiListGfx_t ListGfx;
    uiRecallGfx_t RecallGfx;
    uiInfoGfx_t InfoGfx;
    uiControlPanelGfx_t ControlPanelGfx;
    uiDefaultsGfx_t DefaultsGfx;
    uiPrintGfx_t PrintGfx;
    uiConnectionsGfx_t ConnectionsGfx;
    uiFileSelectionGfx_t FSGfx;
} uiTopLevel_t;


typedef struct {
    uiPage_t *CurrentPage;
    Widget Wdg;
    void *CallData;
} uiPageInfo_t;
