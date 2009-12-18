static char *rcsid = "$Id: Page.c,v 1.1 1992/05/18 21:43:03 tvr Exp $";

#include "Includes.h"


void getanddisplaypage(char *topaddress, HText_t * htext,
		        HTextObject_t * htextobject);
int matchingstring(char *word);


char FindText[256] = "\0";
char SearchText[256] = "\0";
int SearchDepth = 1;
int SearchCase = FALSE;


static int SearchDialogType;
static int PageSettingsDialogType;

static char *CopyTopAddress;
static HText_t *CopyHText;
static HTextObject_t *CopyHTextObject;


void PageSearchCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplaySearchDialog(SearchDialogType);
}


void PageCopyCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    CopyHTextObject = htextobject;
    CopyTopAddress = strdup(topaddress);
    CopyHText = htext;
}


void PageListCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    int i = 0, j, stringlength;
    char **items = (char **) NULL;
    char **addresses = (char **) NULL;
    HTextObject_t *tmphtextobject = htext->first;
    HTAnchor *tmpanchor, *destanchor;

    while (tmphtextobject)
	if (tmpanchor = (HTAnchor *) tmphtextobject->anchor) {
	    addresses = (char **) ReAlloc((void *) addresses,
					  ++i * sizeof(char *));
	    destanchor =
		HTAnchor_followMainLink((HTAnchor *) tmphtextobject->anchor);
	    addresses[i - 1] = HTAnchor_address(destanchor);
	    items = (char **) ReAlloc((void *) items, i * sizeof(char *));
	    j = 1;
	    items[i - 1] = (char *) NULL;
	    while (tmphtextobject &&
		   (HTAnchor *) tmphtextobject->anchor == tmpanchor) {
		stringlength = strlen(tmphtextobject->data);
		j += stringlength;
		items[i - 1] = (char *) ReAlloc((void *) items[i - 1],
						j * sizeof(char));
		strcpy(&items[i - 1][j - stringlength - 1],
		       tmphtextobject->data);
		items[i - 1][j - 1] = '\0';
		tmphtextobject = tmphtextobject->next;
	    }
	} else
	    tmphtextobject = tmphtextobject->next;

    UiDisplayListDialog(items, addresses, i, StartLoading);
}


void PageLoadToFileCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTAnchor *destanchor;

    if (!htextobject || !htextobject->anchor) {
	UiDisplayWarningDialog("No active tag", (void (*) (int)) NULL);
	return;
    }
    destanchor = HTAnchor_followMainLink((HTAnchor *) htextobject->anchor);
    if (ClCanLoadToFile(HTAnchor_address(destanchor)))
	UiDisplayFileSelection(PageGetPageCB);
    else
	UiDisplayWarningDialog("Load to file not supported for this tag",
			       (void (*) (int)) NULL);
}


int TruthValue(value)
char *value;
{
    if (value && (!strncasecmp(value, STR_TRUE, strlen(STR_TRUE)) ||
		  !strncasecmp(value, STR_ON, strlen(STR_ON)) ||
		  !strncasecmp(value, STR_YES, strlen(STR_YES))))
	return TRUE;
    return FALSE;
}


void PagePrintCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    static void *table = (void *) NULL;
    int update = FALSE;
    char *configstr;

    if (!table) {
	update = TRUE;
	table = ConfigGetValue((void *) NULL, C_PRINT);
	PrintTopMargin = atoi((char *) ConfigGetValue(table, C_TOPMARGIN));
	PrintBottomMargin =
	    atoi((char *) ConfigGetValue(table, C_BOTTOMMARGIN));
	PrintLeftMargin = atoi((char *) ConfigGetValue(table, C_LEFTMARGIN));
	PrintWidth = atoi((char *) ConfigGetValue(table, C_WIDTH));
	if (configstr = (char *) ConfigGetValue(table, C_COMMAND))
	    strcpy(PrintCommand, configstr);
	else
	    strcpy(PrintCommand, "");
	PrintToFile = TruthValue((char *) ConfigGetValue(table, C_PRINTTOFILE));
	if (configstr = (char *) ConfigGetValue(table, C_FILENAME))
	    strcpy(PrintFileName, configstr);
	else
	    strcpy(PrintFileName, "");
    }
    UiDisplayPrintDialog(htext);

    if (update) {
	UiUpdateVariable("PrintTopMargin");
	UiUpdateVariable("PrintBottomMargin");
	UiUpdateVariable("PrintLeftMargin");
	UiUpdateVariable("PrintWidth");
	UiUpdateVariable("PrintCommand");
	UiUpdateVariable("PrintToFile");
	UiUpdateVariable("PrintFileName");
    }
}


void PageSettingsCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplayPageSettingsDialog(PageSettingsDialogType);
}


void PageCloseCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    char *address = HTAnchor_address((HTAnchor *) htext->node_anchor);
    Page_t *toppage, *page;

    toppage = FindPage(Pages, topaddress);
    page = FindPage(toppage->Children, address);

    UiDeletePage(toppage->Address, htext);
    page->HText = (HText_t *) NULL;
    HText_free(htext);

    page = toppage->Children;
    while (page)
	if (page->HText)
	    return;
	else
	    page = page->Next;

    HierarchyClose(topaddress, htext, htextobject, parameter);
}


void PagePrevWordCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTextObject_t *tmphtextobject;

    if (!htextobject)
	return;

    tmphtextobject = htextobject->prev;

    while (tmphtextobject && !CanBeCursor(tmphtextobject))
	tmphtextobject = tmphtextobject->prev;

    if (tmphtextobject)
	UiSetCursor(topaddress, htext, tmphtextobject);
}


void PageNextWordCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTextObject_t *tmphtextobject;

    if (!htextobject)
	return;

    tmphtextobject = htextobject->next;

    while (tmphtextobject && !CanBeCursor(tmphtextobject))
	tmphtextobject = tmphtextobject->next;

    if (tmphtextobject)
	UiSetCursor(topaddress, htext, tmphtextobject);
}


void PagePrevTagCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTextAnchor_t *tmphtanchor;
    HTAnchor *tmpanchor, *destanchor;
    HText_t *newhtext;
    HTextObject_t *newhtextobject;

    if (!htextobject)
	return;

    tmpanchor = (HTAnchor *) htextobject->anchor;

    /* Ignore htextobject with same anchor */
    while (tmpanchor && htextobject &&
	   ((HTAnchor *) htextobject->anchor == tmpanchor))
	htextobject = htextobject->prev;

    /* Search for previous anchor */
    while (htextobject && !htextobject->anchor)
	htextobject = htextobject->prev;

    /* No anchor found */
    if (!htextobject)
	return;

    /* We want the first word in the tag to be highlighted */
    while (htextobject->prev &&
	   (htextobject->anchor == htextobject->prev->anchor))
	htextobject = htextobject->prev;

    UiSetCursor(topaddress, htext, htextobject);

    if ((int) parameter == NO_AUTOGET)
	return;

    getanddisplaypage(topaddress, htext, htextobject);
}


void PageNextTagCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTAnchor *tmpanchor;
    HTextObject_t *newhtextobject;

    if (!htextobject)
	return;

    tmpanchor = (HTAnchor *) htextobject->anchor;

    /* Ignore htextobject with same anchor */
    while (tmpanchor && htextobject &&
	   ((HTAnchor *) htextobject->anchor == tmpanchor))
	htextobject = htextobject->next;

    /* Search for previous anchor */
    while (htextobject && !htextobject->anchor)
	htextobject = htextobject->next;

    /* No anchor found */
    if (!htextobject)
	return;

    UiSetCursor(topaddress, htext, htextobject);

    if ((int) parameter == NO_AUTOGET)
	return;

    getanddisplaypage(topaddress, htext, htextobject);
}


void PageHomeCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    StartLoading(topaddress, topaddress, (char *) NULL);
}


void PageRecallCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Page_t *toppage, *page;
    int i = 0;
    char **items = (char **) NULL;

    toppage = FindPage(Pages, topaddress);
    page = toppage->Children;

    while (page) {
	items = (char **) ReAlloc((void *) items, ++i * sizeof(char *));
	items[i - 1] = strdup(page->Address);
	page = page->Next;
    }

    UiDisplayRecallDialog(items, i, StartLoading);
}


void PageBackCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    char *address = HTAnchor_address((HTAnchor *) htext->node_anchor);
    Page_t *toppage, *parentpage, *page;

    toppage = FindPage(Pages, topaddress);
    page = FindPage(toppage->Children, address);

    if (parentpage = page->Parents) {
	if (parentpage->Next)
	    PageGeneratePopup(parentpage, topaddress);
	else {
	    while (parentpage->Next)
		parentpage = parentpage->Next;
	    StartLoading(parentpage->Address, topaddress, parentpage->Address);
	}
    }
}


PageGeneratePopup(parentpage, topaddress)
Page_t *parentpage;
char *topaddress;
{
    char **items = (char **) NULL;
    int nitems = 0;

    while (parentpage) {
	items = (char **) ReAlloc((void *) items, ++nitems * sizeof(char *));
	items[nitems - 1] = parentpage->Address;
	parentpage = parentpage->Next;
    }

    UiDisplayPopup(StartLoading, topaddress, items, nitems);
}


HTextObject_t *
 FindHTextObject(htext, address)
HText_t *htext;
char *address;
{
    HTextObject_t *newhtextobject;
    HTAnchor *destanchor;

    newhtextobject = htext->first;
    while (newhtextobject) {
	if (newhtextobject->anchor) {
	    destanchor =
		HTAnchor_followMainLink((HTAnchor *) newhtextobject->anchor);
	    if (destanchor && !strcmp(HTAnchor_address(destanchor), address))
		return newhtextobject;
	}
	newhtextobject = newhtextobject->next;
    }

    return (HTextObject_t *) NULL;
}


void PagePrevPageCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    char *address = HTAnchor_address((HTAnchor *) htext->node_anchor);
    Page_t *toppage, *parentpage, *page;
    HTextObject_t *newhtextobject;

    toppage = FindPage(Pages, topaddress);
    page = FindPage(toppage->Children, address);

    if (parentpage = page->Parents) {
	if (parentpage->Next)
	    PageGeneratePopup(parentpage, topaddress);
	while (parentpage->Next)
	    parentpage = parentpage->Next;
	parentpage = parentpage->ParentPage;
	if (parentpage->HText) {
	    newhtextobject = FindHTextObject(parentpage->HText, page->Address);
	    if (newhtextobject)
		PagePrevTagCB(topaddress, parentpage->HText,
			      newhtextobject, (void *) AUTOGET);
	} else
	    StartLoading(parentpage->Address, topaddress, (char *) NULL);
    }
}


void PageNextPageCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    char *address = HTAnchor_address((HTAnchor *) htext->node_anchor);
    Page_t *toppage, *parentpage, *page;
    HTextObject_t *newhtextobject;

    toppage = FindPage(Pages, topaddress);
    page = FindPage(toppage->Children, address);

    if (parentpage = page->Parents) {
	if (parentpage->Next)
	    PageGeneratePopup(parentpage, topaddress);
	while (parentpage->Next)
	    parentpage = parentpage->Next;
	parentpage = parentpage->ParentPage;
	if (parentpage->HText) {
	    newhtextobject = FindHTextObject(parentpage->HText, page->Address);
	    if (newhtextobject)
		PageNextTagCB(topaddress, parentpage->HText,
			      newhtextobject, (void *) AUTOGET);
	} else
	    StartLoading(parentpage->Address, topaddress, (char *) NULL);
    }
}


void PageGetPageCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    if (!htextobject || !htextobject->anchor)
	return;

    if (parameter)
	ClSetFileNameForLoadingToFile((char *) parameter);

    getanddisplaypage(topaddress, htext, htextobject);
}


void PageClickCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTextObject_t *tmphtextobject = htext->first;
    int state = 0;

    if (CopyHTextObject && !strcmp(topaddress, CopyTopAddress)
	&& htext == CopyHText)
	while (tmphtextobject) {
	    if (tmphtextobject == CopyHTextObject ||
		tmphtextobject == htextobject)
		state += 1 + (CopyHTextObject == htextobject);
	    switch (state) {
	    case 2:
		if (tmphtextobject->data)
		    UiAddStringToCutBuffer(tmphtextobject->data);
		else
		    UiAddStringToCutBuffer("\n");
		UiAddStringToCutBuffer((char *) NULL);
		tmphtextobject = (HTextObject_t *) NULL;
		Free(CopyTopAddress);
		break;
	    case 1:
		if (tmphtextobject->data)
		    UiAddStringToCutBuffer(tmphtextobject->data);
		else
		    UiAddStringToCutBuffer("\n");
		/* Fall through */
	    default:
		tmphtextobject = tmphtextobject->next;
	    }
	}

    CopyHTextObject = (HTextObject_t *) NULL;
}


void IndexFindCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    char *newaddress;

    if (!FindText[0])
	return;

    HTMainAnchor = htext->node_anchor;	/* Kludguality */
    newaddress = (char *) HTSearchAddress(FindText);	/* missa proto, meeTu? */

    StartLoading(newaddress, topaddress,
		 HTAnchor_address((HTAnchor *) htext->node_anchor));
    Free(newaddress);
}


static char *hiertopaddress;
static HText_t *hierhtext;
static HTextObject_t *hierhtextobject;
static void *hierparameter;

void HierarchyCloseCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    hiertopaddress = topaddress;
    hierhtext = htext;
    hierhtextobject = htextobject;
    hierparameter = parameter;

    UiDisplayWarningDialog("Close hierarchy?", HierarchyNukeCB);
}


void HierarchyNukeCB(button)
int button;
{
    HierarchyClose(hiertopaddress, hierhtext, hierhtextobject, hierparameter);
}


void HierarchyClose(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Page_t *toppage;

    toppage = FindPage(Pages, topaddress);

    while (toppage->Children) {
	if (toppage->Children->HText) {
	    HText_free(toppage->Children->HText);
	    UiDeletePage(toppage->Address, toppage->Children->HText);
	}
	while (toppage->Children->Parents)
	    DeletePage(&toppage->Children->Parents,
		       toppage->Children->Parents->Address);
	DeletePage(&toppage->Children, toppage->Children->Address);
    }

    DeletePage(&Pages, toppage->Address);
}


void SearchBackwardCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTextObject_t *newhtextobject;

    if (!htextobject)
	return;

    newhtextobject = htextobject->prev;
    while (newhtextobject && !matchingstring(newhtextobject->data))
	newhtextobject = newhtextobject->prev;

    if (newhtextobject)
	UiSetCursor(topaddress, htext, newhtextobject);
}


void SearchForwardCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    HTextObject_t *newhtextobject;

    if (!htextobject)
	return;

    newhtextobject = htextobject->next;
    while (newhtextobject && !matchingstring(newhtextobject->data))
	newhtextobject = newhtextobject->next;

    if (newhtextobject)
	UiSetCursor(topaddress, htext, newhtextobject);
}


void ConnectionsCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    Connection_t *tmpconnection = Connections;
    char **listitems = (char **) NULL;
    void **connections = (void **) NULL;
    int nitems = 0;

    while (tmpconnection) {
	listitems = (char **) ReAlloc((void *) listitems,
				      ++nitems * sizeof(char *));
	listitems[nitems - 1] = tmpconnection->Address;
	connections = (void **) ReAlloc((void *) connections,
					nitems * sizeof(void *));
	connections[nitems - 1] = tmpconnection;
	tmpconnection = tmpconnection->Next;
    }

    UiDisplayConnectionsDialog(listitems, connections, nitems, KillCB);
}


void KillCB(connection)
void *connection;
{
    Connection_t *tmpconnection = (Connection_t *) connection;

    switch (tmpconnection->Status) {
    case SELECTING:
	UiDeleteInputFD(tmpconnection->InputId);
	break;
    case POLLING:
	UiDeleteTimeOut(tmpconnection->TimeOutId);
    }

    ClCloseConnection(tmpconnection->ClConnection);
    DeleteConnection(tmpconnection->Address);
}


void ControlPanelCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplayControlPanel();
}


void DefaultsCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplayDefaultsDialog();
}


void getanddisplaypage(topaddress, htext, htextobject)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
{
    HTAnchor *destanchor;

    destanchor = HTAnchor_followMainLink((HTAnchor *) htextobject->anchor);
    HTMainAnchor = (HTParentAnchor *) NULL;

    StartLoading(HTAnchor_address(destanchor),
		 topaddress,
		 HTAnchor_address((HTAnchor *) htext->node_anchor));
}


int matchingstring(word)
char *word;
{
    char *tmpptr = word;
    int searchlength = strlen(SearchText);

    while (tmpptr && strlen(tmpptr) >= searchlength) {
	if (SearchCase) {
	    if (!strncmp(tmpptr, SearchText, searchlength))
		return TRUE;
	} else if (!strncasecmp(tmpptr, SearchText, searchlength))
	    return TRUE;
	tmpptr++;
    }

    return FALSE;
}
