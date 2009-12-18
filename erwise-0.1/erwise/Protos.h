/* $Id: Protos.h,v 1.1 1992/05/18 21:43:03 tvr Exp $ */


/*
 * From other objects/libraries
 */

extern HTParentAnchor *HTMainAnchor;	/* Can you spell kludge? */
extern struct HText *HtLocalText;


/*
 * Main.c
 */

extern FILE *OpenErwiserc(char *type);


/*
 * Setup.c
 */

extern void AttachCallbacks(void);
extern void BindKeys(void);
extern void BindVariables(void);


/*
 * TopLevel.c
 */

extern void TopQuitCB(char *topaddress, HText_t * htext,
		       HTextObject_t * htextobject, void *parameter);
extern void NukeErwiseCB(int button);
extern void TopInfoCB(char *topaddress, HText_t * htext,
		       HTextObject_t * htextobject, void *parameter);
extern void TopOpenCB(char *topaddress, HText_t * htext,
		       HTextObject_t * htextobject, void *parameter);
extern void TopHelpCB(char *topaddress, HText_t * htext,
		       HTextObject_t * htextobject, void *parameter);
extern void GetPageCB(char *nodename);

extern char FindText[256];
extern char SearchText[256];
int SearchDepth;
int SearchCase;


/*
 * Page.c
 */

extern void PageSearchCB(char *topaddress, HText_t * htext,
			  HTextObject_t * htextobject, void *parameter);
extern void PageCopyCB(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject, void *parameter);
extern void PageListCB(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject, void *parameter);
extern void PageLoadToFileCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void PagePrintCB(char *topaddress, HText_t * htext,
			 HTextObject_t * htextobject, void *parameter);
extern void PageSettingsCB(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void PageCloseCB(char *topaddress, HText_t * htext,
			 HTextObject_t * htextobject, void *parameter);
extern void PagePrevWordCB(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void PageNextWordCB(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void PagePrevTagCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void PageNextTagCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void PageHomeCB(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject, void *parameter);
extern void PageRecallCB(char *topaddress, HText_t * htext,
			  HTextObject_t * htextobject, void *parameter);
extern void PageBackCB(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject, void *parameter);
extern void PagePrevPageCB(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void PageNextPageCB(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void PageGetPageCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void PageClickCB(char *topaddress, HText_t * htext,
			 HTextObject_t * htextobject, void *parameter);
extern void IndexFindCB(char *topaddress, HText_t * htext,
			 HTextObject_t * htextobject, void *parameter);
extern void HierarchyCloseCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void HierarchyNukeCB(int button);
extern void HierarchyClose(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void SearchBackwardCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void SearchForwardCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void ConnectionsCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void KillCB(void *connection);
extern void ControlPanelCB(char *topaddress, HText_t * htext,
			    HTextObject_t * htextobject, void *parameter);
extern void DefaultsCB(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject, void *parameter);


/*
 * Help.c
 */

extern void HelpOnFunctionCB(char *topaddress, HText_t * htext,
			   HTextObject_t * htextobject, void *parameter);
extern void HelpManualCB(char *topaddress, HText_t * htext,
			  HTextObject_t * htextobject, void *parameter);


/*
 * Print.c
 */

extern int PrintTopMargin;
extern int PrintBottomMargin;
extern int PrintLeftMargin;
extern int PrintWidth;
extern char PrintCommand[1024];
extern int PrintToFile;
extern char PrintFileName[1024];


/*
 * Misc.c
 */

extern Page_t *FindPage(Page_t * hierarchy, char *address);
extern Page_t *GlobalFindPage(char *address);
extern Page_t *AddPage(Page_t ** page, char *address, HText_t * htext,
		        Page_t * toppage);
extern void DeletePage(Page_t ** page, char *address);
extern void DisplayWarning(char *text);
extern void DisplayFatal(char *text);
extern int CanBeCursor(HTextObject_t * htextobject);
extern void *Malloc(int size);
extern void *ReAlloc(void *ptr, int size);
extern void Free(void *ptr);
extern void StartLoading(char *address, char *topaddress, char *parentaddress);
void PollConnection(Connection_t * connection);
Connection_t *AddConnection(char *address, Page_t * toppage, Page_t * parentpage,
			     ClConnection_t * clconnection);
Connection_t *FindConnection(char *address);
void DeleteConnection(char *address);

extern Page_t *Pages;
extern Connection_t *Connections;


/*
 * Config.c
 */

extern void ConfigInit(void);
extern int ConfigRestore(FILE * fp);
extern int ConfigSave(FILE * fp);
extern void *ConfigGetValue(void *table, char *id);
extern void *ConfigSetValue(void *table, char *id, void *value);
