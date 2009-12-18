static char *rcsid = "$Id: Setup.c,v 1.1 1992/05/18 21:43:03 tvr Exp $";

#include "Includes.h"


void attachtoplevelcallbacks(void);
void attachpagecallbacks(void);
void attachhierarchycallbacks(void);
void attachsearchcallbacks(void);
void bindpagevariables(void);
void bindsearchvariables(void);
void bindprintvariables(void);


void AttachCallbacks()
{
    attachtoplevelcallbacks();
    attachpagecallbacks();
    attachhierarchycallbacks();
    attachsearchcallbacks();
}



char *
 parse_key(type, item)
int *type;
char *item;
{
    char *p;

    if (!item)
	return item;

    if (*item == '^') {
	*type = UI_CTRL;

	p = strdup(item + 1);

	if (strlen(p) == 1)
	    *p = tolower(*p);

	return p;
    }
    if (strlen(item) == 1) {
	if (isupper(*item)) {
	    *type = UI_SHIFT;
	} else {
	    *type = UI_NONE;
	}
	p = (char *) strdup(item);

	*p = tolower(*p);

	return p;
    }
    *type = UI_NONE;

    return (char *) strdup(item);
}


/*
 * Keyboard bindings
 */

typedef struct erwise_key_bindings_s {
    char *itemname;
    void (*callback) ();
    void *data;
} erwise_key_bindings_t;

erwise_key_bindings_t erwise_keys[] =
{
    {C_GETPAGE, PageGetPageCB, NULL},
    {C_PREVWORD, PagePrevWordCB, NULL},
    {C_NEXTWORD, PageNextWordCB, NULL},
    {C_PREVTAG, PagePrevTagCB, (void *) NO_AUTOGET},
    {C_NEXTTAG, PageNextTagCB, (void *) NO_AUTOGET},
    {C_CLOSE, PageCloseCB, NULL},
    {C_SEARCH, PageSearchCB, NULL},
    {C_SETTINGS, PageSettingsCB, NULL},
    {C_LIST, PageListCB, NULL},
    {C_PRINTDOC, PagePrintCB, NULL},
    {NULL, NULL, NULL},
};


void BindKeys()
{
    void *table;
    char *item;
    char *key;
    int keytype;

    erwise_key_bindings_t *k;

    table = (void *) ConfigGetValue((void *) NULL, C_KEYBINDINGS);

    for (k = erwise_keys; k->itemname; k++) {
	item = (char *) ConfigGetValue(table, k->itemname);

	if (item) {
	    key = parse_key(&keytype, item);

	    if (key) {
		UiBindKey(key, keytype, k->callback, k->data);
		free(key);
	    }
	}
    }
}


void BindVariables()
{
    bindpagevariables();
    bindsearchvariables();
    bindprintvariables();
}


void attachtoplevelcallbacks()
{
    UiAttachCallback("Quit", TopQuitCB, (void *) NULL);
    UiAttachCallback("Info", TopInfoCB, (void *) NULL);
    UiAttachCallback("Open", TopOpenCB, (void *) NULL);
    UiAttachCallback("Help", HelpManualCB, (void *) NULL);
}


void attachpagecallbacks()
{
    UiAttachCallback("Search", PageSearchCB, (void *) NULL);
    UiAttachCallback("Copy", PageCopyCB, (void *) NULL);
    UiAttachCallback("List", PageListCB, (void *) NULL);
    UiAttachCallback("Load to file", PageLoadToFileCB, (void *) NULL);
    UiAttachCallback("Print", PagePrintCB, (void *) NULL);
    UiAttachCallback("Settings", PageSettingsCB, (void *) NULL);
    UiAttachCallback("Close", PageCloseCB, (void *) NULL);

    UiAttachCallback("Prev tag", PagePrevTagCB, (void *) AUTOGET);
    UiAttachCallback("Next tag", PageNextTagCB, (void *) AUTOGET);

    UiAttachCallback("Home", PageHomeCB, (void *) NULL);
    UiAttachCallback("Recall", PageRecallCB, (void *) NULL);
    UiAttachCallback("Back", PageBackCB, (void *) NULL);
    UiAttachCallback("Prev page", PagePrevPageCB, (void *) NULL);
    UiAttachCallback("Next page", PageNextPageCB, (void *) NULL);

    UiAttachCallback("On function", HelpOnFunctionCB, (void *) NULL);

    UiAttachCallback("Get page", PageGetPageCB, (void *) NULL);
    UiAttachCallback("Click page", PageClickCB, (void *) NULL);
    UiAttachCallback("IndexFind", IndexFindCB, (void *) NULL);
}


void attachhierarchycallbacks()
{
    UiAttachCallback("Connections", ConnectionsCB, (void *) NULL);
    UiAttachCallback("Controlpanel", ControlPanelCB, (void *) NULL);
    UiAttachCallback("Defaults", DefaultsCB, (void *) NULL);
    UiAttachCallback("Close hierarchy", HierarchyCloseCB, (void *) NULL);
}


void attachsearchcallbacks()
{
    UiAttachCallback("SearchBackward", SearchBackwardCB, (void *) NULL);
    UiAttachCallback("SearchForward", SearchForwardCB, (void *) NULL);
}


void bindpagevariables()
{
    UiBindVariable("FindText", (void *) FindText, uiVTstring);
}


void bindsearchvariables()
{
    UiBindVariable("SearchText", (void *) SearchText, uiVTstring);
    UiBindVariable("SearchDepth", (void *) &SearchDepth, uiVTint);
    UiBindVariable("SearchCase", (void *) &SearchCase, uiVTint);
}


void bindprintvariables()
{
    UiBindVariable("PrintTopMargin", (void *) &PrintTopMargin, uiVTint);
    UiBindVariable("PrintBottomMargin", (void *) &PrintBottomMargin, uiVTint);
    UiBindVariable("PrintLeftMargin", (void *) &PrintLeftMargin, uiVTint);
    UiBindVariable("PrintWidth", (void *) &PrintWidth, uiVTint);
    UiBindVariable("PrintCommand", (void *) &PrintCommand, uiVTstring);
    UiBindVariable("PrintToFile", (void *) &PrintToFile, uiVTint);
    UiBindVariable("PrintFileName", (void *) &PrintFileName, uiVTstring);
}
