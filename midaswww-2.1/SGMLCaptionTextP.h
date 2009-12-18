/*==================================================================*/
/*                                                                  */
/* SGMLCaptionTextObject                                            */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLCAPTIONTEXTP_H
#define SGMLCAPTIONTEXTP_H

#include "SGMLCaptionText.h"
#include "SGMLCompositeTextP.h"

typedef struct _SGMLCaptionTextClassPart{

    XtPointer	       extension;	  /* pointer to extension record        */

} SGMLCaptionTextClassPart;

typedef struct _SGMLCaptionTextClassRec{

    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLCompositeTextClassPart sgml_composite_text_class;
    SGMLCaptionTextClassPart   sgml_caption_text_class;

} SGMLCaptionTextClassRec, *SGMLCaptionTextObjectClass;

extern SGMLCaptionTextClassRec sGMLCaptionTextClassRec;

typedef struct _SGMLCaptionTextPart {

   Widget caption;

} SGMLCaptionTextPart;

typedef struct _SGMLCaptionTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLCompositeTextPart  sgml_composite_text;
    SGMLCaptionTextPart    sgml_caption_text;
} SGMLCaptionTextRec;

#endif SGMLCAPTIONTEXTP_H
