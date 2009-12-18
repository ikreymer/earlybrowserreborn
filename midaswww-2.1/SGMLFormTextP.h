/*==================================================================*/
/*                                                                  */
/* SGMLFormTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLFORMTEXTP_H
#define SGMLFORMTEXTP_H

#include "SGMLFormText.h"
#include "SGMLCompositeTextP.h"

typedef struct _SGMLFormTextClassPart{

    XtPointer	       extension;	  /* pointer to extension record        */

} SGMLFormTextClassPart;

typedef struct _SGMLFormTextClassRec{

    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLCompositeTextClassPart sgml_composite_text_class;
    SGMLFormTextClassPart      sgml_form_text_class;

} SGMLFormTextClassRec, *SGMLFormTextObjectClass;

extern SGMLFormTextClassRec sGMLFormTextClassRec;

typedef struct _SGMLFormTextPart {

    char        *action;
    XrmQuark	 method;
    char        *result;

} SGMLFormTextPart;

typedef struct _SGMLFormTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLCompositeTextPart  sgml_composite_text;
    SGMLFormTextPart       sgml_form_text;
} SGMLFormTextRec;

#endif SGMLFORMTEXTP_H
