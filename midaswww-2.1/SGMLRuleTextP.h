/*==================================================================*/
/*                                                                  */
/* SGMLRuleTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLRULETEXTP_H
#define SGMLRULETEXTP_H

#include "SGMLRuleText.h"
#include "SGMLTextP.h"

typedef struct _SGMLRuleTextClassPart{

    XtPointer ignore;   /* no new class elements */

} SGMLRuleTextClassPart;

typedef struct _SGMLRuleTextClassRec{

    ObjectClassPart         object_class;
    SGMLTextClassPart       sgml_text_class;
    SGMLRuleTextClassPart   sgml_rule_text_class;

} SGMLRuleTextClassRec, *SGMLRuleTextObjectClass;

extern SGMLRuleTextClassRec sGMLRuleTextClassRec;

typedef struct _SGMLRuleTextPart {

   Dimension height; 
   GC gc1;
   GC gc2; 

} SGMLRuleTextPart;

typedef struct _SGMLRuleTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLRuleTextPart     sgml_rule_text;
} SGMLRuleTextRec;

#endif SGMLRULETEXTP_H
