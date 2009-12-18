/* $Id: Ui.h,v 1.3 1992/04/28 00:34:53 kny Exp kny $ */


#define UI_OK       0
#define UI_ERROR    1
#define UI_NOTFOUND 2

#define UI_NONE  0
#define UI_SHIFT 1
#define UI_CTRL  4


/*
 * Config defines
 */
#define C_GLOBALSETTINGS  "GlobalSettings"

#define C_TOPMARGIN      "topmargin"
#define C_BOTTOMMARGIN   "bottommargin"
#define C_LEFTMARGIN     "leftmargin"
#define C_RIGHTMARGIN    "rightmargin"
#define C_WIDTH          "width"
#define C_HEIGHT         "height"
#define C_ONEPAGEMODE     "onepagemode"
#define C_FIXEDWIDTHMODE  "fixedwidthmode"
#define C_FIXEDWIDTH      "fixedwidth"
#define C_DOUBLECLICKTIME "doubleclicktime"

#define C_SEARCH          "search"
#define C_CONTROLPANEL    "controlpanel"
#define C_LIST            "list"
#define C_RECALL          "recall"
#define C_PAGESETTINGS    "pagesettings"

#define C_DEFAULTS        "Defaults"
#define C_DEFAULTSTABLE   "defaultstable"


typedef enum {
    uiVTint,
    uiVTdouble,
    uiVTstring
} uiVarType_t;


extern int UiInitialize(int argc, char *argv[],
			 void *(*configpf) (void *table, char *item),
			 void *(*configsetpf) (void *table, char *item,
					        void *value));
extern void UiMainLoop(void);

extern int UiDisplaySelectionBox(void (*callback) (char *filename));

extern int UiDisplayPage(char *address, HText_t * prevhtext, HText_t * htext,
			  HTextObject_t * htextobject, char *title);
extern int UiDeletePage(char *address, HText_t * htext);
extern int UiSetCursor(char *address, HText_t * htext,
		        HTextObject_t * htextobject);

extern int UiDisplaySearchDialog(int type);

extern int UiDisplayPageSettingsDialog(int type);

extern int UiDisplayListDialog(char **listitems, char **addresses, int nitems,
			        void (*callback) (char *topaddress,
						   char *address,
						   char *parentaddress));

extern int UiDisplayRecallDialog(char **listitems, int nitems,
				  void (*callback) (char *topaddress,
						     char *address,
						   char *parentaddress));

extern int UiAttachCallback(char *actionname,
			     void (*callback) (char *address,
					        HText_t * htext,
					     HTextObject_t * htextobject,
					        void *parameter),
			     void *parameter);
extern int UiBindKey(char *keyname, int modifier,
		      void (*callback) (char *address,
					 HText_t * htext,
					 HTextObject_t * htextobject,
					 void *parameter),
		      void *parameter);
extern int UiBindVariable(char *varname, void *variable, uiVarType_t type);
extern int UiUpdateVariable(char *varname);
extern void UiGetNextAction(void (*helponactioncb) (char *actionstring));
extern int UiAddTimeOut(int timeout, void (*callback) (void *data),
			 void *data);
extern void UiDeleteTimeOut(int timoutid);
extern int UiAddInputFD(int fd, void (*callback) (void *data), void *data);
extern void UiDeleteInputFD(int inputid);
extern void UiAddStringToCutBuffer(char *data);
void UiDisplayPopup(void (*callback) (char *address, char *topaddress,
				       char *parentaddress),
		     char *topaddress, char **items, int nitems);

extern void UiShowInfo(void);

extern int UiDisplayControlPanel(void);

extern int UiDisplayFileSelection(void (*callback) (char *topaddress,
						     HText_t * htext,
					     HTextObject_t * htextobject,
						     void *parameter));
