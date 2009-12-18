#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/TextF.h>

#include "SGMLHyper.h"
#include "Tree.h"
#include "Ghostview.h"
#include "GhostviewPlus.h"

void MidasClassInit()
{
  MidasDeclareClass(xmRowColumnWidgetClass);
  MidasDeclareClass(xmToggleButtonWidgetClass);
  MidasDeclareClass(xmPushButtonWidgetClass);
  MidasDeclareClass(xmToggleButtonGadgetClass);
  MidasDeclareClass(xmPushButtonGadgetClass);
  MidasDeclareClass(xmLabelWidgetClass);
  MidasDeclareClass(xmLabelGadgetClass);
  MidasDeclareClass(xmTextFieldWidgetClass);
  MidasDeclareClass(sGMLHyperWidgetClass);
  MidasDeclareClass(treeWidgetClass);
  MidasDeclareClass(ghostviewWidgetClass);
  MidasDeclareClass(ghostviewPlusWidgetClass);
}
