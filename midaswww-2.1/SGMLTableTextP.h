/*==================================================================*/
/*                                                                  */
/* SGMLTableTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLTABLETEXTP_H
#define SGMLTABLETEXTP_H

#include "SGMLTableText.h"
#include "SGMLCaptionTextP.h"

typedef struct _SGMLTableTextClassPart{

    XtPointer	       extension;	  /* pointer to extension record        */

} SGMLTableTextClassPart;

typedef struct _SGMLTableTextClassRec{

    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLCompositeTextClassPart sgml_composite_text_class;
    SGMLCaptionTextClassPart   sgml_caption_text_class;
    SGMLTableTextClassPart     sgml_table_text_class;

} SGMLTableTextClassRec, *SGMLTableTextObjectClass;

extern SGMLTableTextClassRec sGMLTableTextClassRec;

typedef struct _SGMLTableTextPart {

    int rows;        /* Number of rows in table    */
    int cols;        /* Number of columns in table */
    SGMLDimension *columnWidth;
    SGMLDimension *rowHeight;
    Widget *widgets; /* Pointers to marker widgets */   
    Boolean border;  /* Draw borders around table  */

} SGMLTableTextPart;

typedef struct _SGMLTableTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLCompositeTextPart  sgml_composite_text;
    SGMLCaptionTextPart    sgml_caption_text;
    SGMLTableTextPart      sgml_table_text;
} SGMLTableTextRec;

#endif SGMLTABLETEXTP_H
