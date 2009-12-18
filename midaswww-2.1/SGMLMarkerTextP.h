/*==================================================================*/
/*                                                                  */
/* SGMLMarkerTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLMARKERTEXTP_H
#define SGMLMARKERTEXTP_H

#include "SGMLMarkerText.h"
#include "SGMLTextP.h"
#include "xgifload.h"

typedef struct _SGMLMarkerTextClassPart{

    XtPointer ignore;   /* no new class elements */

} SGMLMarkerTextClassPart;

typedef struct _SGMLMarkerTextClassRec{

    ObjectClassPart         object_class;
    SGMLTextClassPart       sgml_text_class;
    SGMLMarkerTextClassPart sgml_marker_text_class;

} SGMLMarkerTextClassRec, *SGMLMarkerTextObjectClass;

extern SGMLMarkerTextClassRec sGMLMarkerTextClassRec;

typedef struct _SGMLMarkerTextPart {

    Dimension column_align; 
    Position  position; 
    Pixmap    pixmap;
    GIFImage  *image;
    XImage    *ximage;
    unsigned int pixmap_width;
    unsigned int pixmap_height;
    unsigned int pixmap_border_width;
    unsigned int pixmap_depth;
    Boolean   delete_image;
    Pixmap    bomb_pixmap;
    GC        bomb_gc; 
    GC        gc1; 
    GC        gc2; 
} SGMLMarkerTextPart;

typedef struct _SGMLMarkerTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLMarkerTextPart     sgml_marker_text;
} SGMLMarkerTextRec;

#endif SGMLMARKERTEXTP_H
