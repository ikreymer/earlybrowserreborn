/*==================================================================*/
/*                                                                  */
/* SGMLContainerTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLCONTAINERTEXTP_H
#define SGMLCONTAINERTEXTP_H

#include "SGMLContainerText.h"
#include "SGMLTextP.h"

#define SGMLInheritGeometryManager    XtInheritGeometryManager

typedef struct _SGMLContainerTextClassPart{

    XtGeometryHandler geometry_manager;	  /* geometry manager for children   */
    XtPointer	      extension;	  /* pointer to extension record     */

} SGMLContainerTextClassPart;

typedef struct _SGMLContainerTextClassRec{
  
    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLContainerTextClassPart sgml_container_text_class;

} SGMLContainerTextClassRec, *SGMLContainerTextObjectClass;

extern SGMLContainerTextClassRec sGMLContainerTextClassRec;

typedef struct _SGMLContainerTextPart {

    Widget                 child;
    Position		   vertical_offset;	

} SGMLContainerTextPart;

typedef struct _SGMLContainerTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLContainerTextPart  sgml_container_text;
} SGMLContainerTextRec;

#endif SGMLCONTAINERTEXTP_H
