/*==================================================================*/
/*                                                                  */
/* SGMLRuleTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines rule text segment for the SGMLText widget              */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLRULETEXT_H
#define  SGMLRULETEXT_H
#include "SGMLRuleText.h"

extern WidgetClass sGMLRuleTextObjectClass;
typedef struct _SGMLRuleTextRec  *SGMLRuleTextObject;

#define SGMLNruleHeight     "ruleHeight"
#define SGMLCRuleHeight     "RuleHeight"

#ifdef _NO_PROTO

extern Widget  CreateSGMLRuleText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateRuleText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsRuleText(w)     XtIsSubclass(w,sGMLRuleTextObjectClass)

#endif SGMLRULETEXT_H

