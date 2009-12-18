static char *rcsid = "$Id: TopLevel.c,v 1.1 1992/05/18 21:43:03 tvr Exp $";

#include "Includes.h"


void TopQuitCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplayWarningDialog("Quit Erwise?", NukeErwiseCB);
}


void NukeErwiseCB(button)
int button;
{
    FILE *fp;

    if (fp = OpenErwiserc("w")) {
	ConfigSave(fp);
	fclose(fp);
    } else
	fprintf(stderr, "Warning: could not save settings to .erwiserc\n");

    exit(0);
}


void TopInfoCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplayInfo();
}


void TopOpenCB(topaddress, htext, htextobject, parameter)
char *topaddress;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    UiDisplaySelectionBox(GetPageCB);
}


void GetPageCB(address)
char *address;
{
    if (FindPage(Pages, address)) {
	DisplayWarning("Already loaded");
	return;
    }
    AddPage(&Pages, address, (HText_t *) NULL, (Page_t *) NULL);

    StartLoading(address, address, (char *) NULL);
}
