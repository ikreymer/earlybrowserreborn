static char *rcsid = "$Id: Misc.c,v 1.1 1992/05/18 21:43:03 tvr Exp $";

#include "Includes.h"


void displaypage(char *topaddress, HText_t * parenthtext, HText_t * htext,
		  char *address);
int addresscmp(char *addr1, char *addr2);


Connection_t *Connections = (Connection_t *) NULL;
Page_t *Pages = (Page_t *) NULL;


Page_t *FindPage(hierarchy, address)
Page_t *hierarchy;
char *address;
{
    while (address && hierarchy)
	if (!addresscmp(hierarchy->Address, address))
	    return hierarchy;
	else
	    hierarchy = hierarchy->Next;

    return (Page_t *) NULL;
}


Page_t *
 GlobalFindPage(address)
char *address;
{
    Page_t *hierarchy = Pages, *tmppage;

    while (address && hierarchy)
	if (tmppage = FindPage(hierarchy->Children, address))
	    return tmppage;
	else
	    hierarchy = hierarchy->Next;

    return (Page_t *) NULL;
}


Page_t *
 AddPage(page, address, htext, toppage)
Page_t **page;
char *address;
HText_t *htext;
Page_t *toppage;
{
    if (*page) {
	while ((*page)->Next)
	    page = &(*page)->Next;
	(*page)->Next = (Page_t *) Malloc(sizeof(**page));
	page = &(*page)->Next;
    } else
	*page = (Page_t *) Malloc(sizeof(**page));

    (*page)->Address = strdup(address);
    (*page)->HText = htext;
    (*page)->ParentPage = toppage;
    (*page)->Parents = (Page_t *) NULL;
    (*page)->Children = (Page_t *) NULL;
    (*page)->Next = (Page_t *) NULL;

    return *page;
}


void DeletePage(page, address)
Page_t **page;
char *address;
{
    Page_t *oldpage = *page;

    if (!strcmp((*page)->Address, address)) {
	*page = (*page)->Next;
	Free(oldpage);
    } else
	while (*page)
	    if (!strcmp((*page)->Address, address)) {
		*page = (*page)->Next;
		Free(oldpage);
	    } else {
		page = &(*page)->Next;
		oldpage = *page;
	    }
}


void DisplayWarning(text)
char *text;
{
    fprintf(stderr, "Warning: %s\n", text);
}


void DisplayFatal(text)
char *text;
{
    fprintf(stderr, "Fatal: %s\n", text);

    abort();
}


int CanBeCursor(htextobject)
HTextObject_t *htextobject;
{
    int i;

    if (htextobject->paragraph)
	return FALSE;

    for (i = 0; i < htextobject->length; i++)
	if (htextobject->data[i] != ' ')
	    return TRUE;

    return FALSE;
}


void *
 Malloc(size)
int size;
{
    void *tmpptr;

    if (!(tmpptr = (void *) malloc(size))) {
	DisplayFatal("No swap, buy a computer");
	exit(1);
    }				/* Not reached */
    return tmpptr;
}


void *
 ReAlloc(ptr, size)
void *ptr;
int size;
{
    void *tmpptr;

    if (!ptr)
	return Malloc(size);
    else if (!(tmpptr = (void *) realloc((char *) ptr, size))) {
	DisplayFatal("No swap, buy a computer");
	exit(1);
    }				/* Not reached */
    return tmpptr;
}


void Free(ptr)
void *ptr;
{
    if (ptr)
	free(ptr);
}


void StartLoading(address, topaddress, parentaddress)
char *address;
char *topaddress;
char *parentaddress;
{
    Page_t *oldpage, *tmppage, *toppage, *parentpage;
    HText_t *newhtext;
    ClConnection_t *clconnection;
    Connection_t *connection;

    toppage = FindPage(Pages, topaddress);
    parentpage = (toppage && parentaddress) ?
	FindPage(toppage->Children, parentaddress) : (Page_t *) NULL;

    if (toppage && (oldpage = GlobalFindPage(address))) {
	if (tmppage = FindPage(toppage->Children, address)) {
	    if (tmppage->HText) {
		/* add the parent */
		if (parentaddress && strcmp(parentaddress, address) &&
		    !FindPage(tmppage->Parents, parentaddress))
		    AddPage(&tmppage->Parents, parentaddress, tmppage->HText,
			    parentpage);
		/* display the page */
		displaypage(toppage ? toppage->Address : (char *) NULL,
		       parentpage ? parentpage->HText : (HText_t *) NULL,
			    tmppage->HText, address);

		return;
	    }
	} else {
	    if (oldpage->HText) {
#if 0
		/* copy the htext */
		newhtext = HtDuplicate(oldpage->HText);
		/* add the page */
		tmppage = AddPage(&toppage->Children, address, newhtext,
				  toppage);
		/* add the parent */
		if (parentaddress && strcmp(parentaddress, address) &&
		    !FindPage(tmppage->Parents, parentaddress))
		    AddPage(&tmppage->Parents, parentaddress, tmppage->HText,
			    parentpage);
		/* display the page */
		displaypage(toppage ? toppage->Address : (char *) NULL,
		       parentpage ? parentpage->HText : (HText_t *) NULL,
			    newhtext, address);

		return;
#endif
		HTAnchor_setDocument((void *) oldpage->HText->node_anchor,
				     (void *) NULL);

	    }
	}
    }
    if (FindConnection(address))
	return;

    if (!(clconnection = ClOpenConnection(address))) {
	DisplayWarning("Error in ClOpenConnection");
	return;
    }
    connection = AddConnection(address, toppage, parentpage, clconnection);

    PollConnection(connection);
}


void PollConnection(connection)
Connection_t *connection;
{
    HText_t *htext;
    int status;
    int oldfd = connection->FD;
    char *topaddress, *parentaddress;
    Page_t *tmppage, *toppage, *parentpage;

    htext = ClReadData(connection->ClConnection, &status, &connection->FD);

    switch (status) {
    case CL_CONTINUES:
	if (connection->FD && connection->Status == POLLING) {
	    connection->InputId =
		UiAddInputFD(connection->FD,
			     (void (*) (void *)) &PollConnection,
			     (void *) connection);
	    connection->Status = SELECTING;
	} else if (!connection->FD) {
	    if (oldfd)
		UiDeleteInputFD(connection->InputId);
	    connection->TimeOutId =
		UiAddTimeOut(200, (void (*) (void *)) &PollConnection,
			     (void *) connection);
	    connection->Status = POLLING;
	}
	break;
    case CL_COMPLETED:
	if (connection->Status == SELECTING)
	    UiDeleteInputFD(connection->InputId);
	if (htext && !connection->TopPage->HText)
	    connection->TopPage->HText = htext;

	if (htext) {
	    parentaddress = connection->ParentPage ?
		connection->ParentPage->Address : (char *) NULL;
	    toppage = connection->TopPage;

	    parentpage = connection->ParentPage;
	    if (!(tmppage = FindPage(toppage->Children, connection->Address))) {
		tmppage = AddPage(&toppage->Children, connection->Address,
				  htext, toppage);
		/* add the parent (what the fuck have I been thinkin' here?) */
		if (connection->ParentPage && parentaddress &&
		    !FindPage(tmppage->Parents, parentaddress))
		    AddPage(&tmppage->Parents, parentaddress,
			    tmppage->HText, parentpage);
	    } else
		tmppage->HText = htext;

	    displaypage(connection->TopPage ?
			connection->TopPage->Address : (char *) NULL,
			connection->ParentPage ?
			connection->ParentPage->HText : (HText_t *) NULL,
			htext, connection->Address);
	}
	DeleteConnection(connection->Address);
	break;
    case CL_FAILED:
	if (connection->Status == SELECTING)
	    UiDeleteInputFD(connection->InputId);
	DeleteConnection(connection->Address);
	break;
    default:
	DisplayWarning("Uh? :O");
    }
}


void displaypage(topaddress, parenthtext, htext, address)
char *topaddress;
HText_t *parenthtext;
HText_t *htext;
char *address;
{
    HTextAnchor_t *htanchor = (HTextAnchor_t *) NULL;
    HTextObject_t *htextobject;
    char *tag = HTParse(address, "", PARSE_ANCHOR);

    if (tag && tag[0]) {
	htanchor = htext->anchorlist;
	while (htanchor)
	    if (htanchor->anchor && htanchor->anchor->tag &&
		!strcmp(htanchor->anchor->tag, tag))
		break;
	    else
		htanchor = htanchor->next;
	Free(tag);
    }
    htextobject = htanchor ? htanchor->object : htext->first;

    while (htextobject && !CanBeCursor(htextobject))
	htextobject = htextobject->next;

    UiDisplayPage(topaddress, parenthtext, htext, htextobject,
		  (char *) HTAnchor_title(htext->node_anchor));
}


Connection_t *
 AddConnection(address, toppage, parentpage, clconnection)
char *address;
Page_t *toppage;
Page_t *parentpage;
ClConnection_t *clconnection;
{
    Connection_t *tmpconnection = Connections;

    if (tmpconnection) {
	while (tmpconnection->Next)
	    tmpconnection = tmpconnection->Next;
	tmpconnection = tmpconnection->Next = Malloc(sizeof(*tmpconnection));
    } else
	tmpconnection = Connections = Malloc(sizeof(*tmpconnection));

    tmpconnection->Address = strdup(address);
    tmpconnection->TopPage = toppage;
    tmpconnection->ParentPage = parentpage;
    tmpconnection->ClConnection = clconnection;
    tmpconnection->FD = 0;
    tmpconnection->Status = POLLING;
    tmpconnection->Next = (Connection_t *) NULL;

    if (UiConnectionsDialogDisplayed())
	ConnectionsCB((char *) NULL, (HText_t *) NULL,
		      (HTextObject_t *) NULL, (void *) NULL);

    return tmpconnection;
}


Connection_t *
 FindConnection(address)
char *address;
{
    Connection_t *tmpconnection = Connections;

    while (tmpconnection)
	if (!addresscmp(tmpconnection->Address, address))
	    return tmpconnection;
	else
	    tmpconnection = tmpconnection->Next;

    return (Connection_t *) NULL;
}


void DeleteConnection(address)
char *address;
{
    Connection_t *tmpconnection = Connections;

    if (tmpconnection)
	if (!strcmp(tmpconnection->Address, address))
	    Connections = Connections->Next;
	else {
	    while (tmpconnection->Next &&
		   strcmp(tmpconnection->Next->Address, address))
		tmpconnection = tmpconnection->Next;
	    if (tmpconnection->Next) {
		Connection_t *tmptmpconnection;

		tmptmpconnection = tmpconnection->Next;
		tmpconnection->Next = tmpconnection->Next->Next;
		tmpconnection = tmptmpconnection;
	    } else
		tmpconnection = (Connection_t *) NULL;
	}
    if (tmpconnection) {
	if (UiConnectionsDialogDisplayed())
	    ConnectionsCB((char *) NULL, (HText_t *) NULL,
			  (HTextObject_t *) NULL, (void *) NULL);
	Free(tmpconnection->Address);
	Free(tmpconnection);
    } else
	DisplayWarning("DeleteConnection failed");
}


int addresscmp(addr1, addr2)
char *addr1, *addr2;
{
    int i = 0;

    while (addr1[i] && addr2[i] && addr1[i] != '#' && addr2[i] != '#')
	if (addr1[i] != addr2[i])
	    return 1;
	else
	    i++;

    if ((addr1[i] && addr1[i] != '#') || (addr2[i] && addr2[i] != '#'))
	return 1;

    return 0;
}
