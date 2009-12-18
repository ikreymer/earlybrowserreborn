static char *rcsid = "$Id: Help.c,v 1.1 1992/05/18 21:43:03 tvr Exp tvr $";

#include "Includes.h"

static struct anchorstr_st {
    char *action, *address;
} anchortable[] =

{
    "PageInput", "browserwin.html#browserwindow",
    "Search", "browserwin.html#search",
    "Copy", "browserwin.html#copy",
    "List", "browserwin.html#list",
    "Print", "browserwin.html#print",
    "Settings", "browserwin.html#settings",
    "Close", "browserwin.html#close",
    "Top", "browserwin.html#top",
    "Bottom", "browserwin.html#bottom",
    "Prev tag", "browserwin.html#prevtag",
    "Next tag", "browserwin.html#nexttag",
    "Home", "browserwin.html#home",
    "Recall", "browserwin.html#recall",
    "Back", "browserwin.html#back",
    "Prev page", "browserwin.html#prevpage",
    "Next page", "browserwin.html#nextpage",
    "Close hierarchy", "browserwin.html#close_hierarchy",
    "Controlpanel", "browserwin.html#controlpanel",
    "Defaults", "browserwin.html#defaults",
    "On action", "browserwin.html#on_action",
    "Help", "browserwin.html#manual",
    NULL, NULL
};

static void helponactioncb(char *actionstring);


void HelpOnFunctionCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiGetNextAction(helponactioncb);
}


void HelpManualCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    if (!FindPage(Pages, HELP_TOPLEVEL))
	AddPage(&Pages, HELP_TOPLEVEL, (HText_t *) NULL, (Page_t *) NULL);

    StartLoading(HELP_TOPLEVEL, HELP_TOPLEVEL, HELP_TOPLEVEL);
}


static void helponactioncb(actionstring)
char *actionstring;
{
    int i;
    char *addressstring;
    Page_t *toppage;

    if (!FindPage(Pages, HELP_TOPLEVEL)) {
	toppage = AddPage(&Pages, HELP_TOPLEVEL, (HText_t *) NULL,
			  (Page_t *) NULL);
	AddPage(&toppage->Children, HELP_TOPLEVEL, (HText_t *) NULL,
		toppage);
    }
    for (i = 0; anchortable[i].action; i++) {
	if (!strcmp(anchortable[i].action, actionstring)) {
	    addressstring = (char *) malloc(strlen(HELP_DIR) +
					 strlen(anchortable[i].address) +
					    1);
	    sprintf(addressstring, "%s%s",
		    HELP_DIR, anchortable[i].address);
	    StartLoading(addressstring, HELP_TOPLEVEL, HELP_TOPLEVEL);

	    return;
	}
    }
}
