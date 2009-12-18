#include "midaslist.h"
#include "midasoperand.h"
#include "midasshell.h"

MidasOperand MidasGetIngot();
MidasOperand MidasConvertFromInteger();
MidasOperand MidasConvertFromString();
MidasOperand MidasConvertFromBoolean();
XtAppContext MidasInitialize();
MidasShell *MidasGetShell(); 
Widget MidasTraceWidgetTree();
Widget MidasScanWidgetTree();
Widget MidasFetch(); 
void MidasSetupWidget();
char *MidasQueryUser();
XmAnyCallbackStruct *MidasGetActiveCallback(); 

#define XtSetFloatArg(arg, n, d) \
    if (sizeof(float) > sizeof(XtArgVal)) { \
	XtSetArg(arg, n, &(d)); \
    } else { \
	XtArgVal *ld = (XtArgVal *)&(d); \
	XtSetArg(arg, n, *ld); \
    }
