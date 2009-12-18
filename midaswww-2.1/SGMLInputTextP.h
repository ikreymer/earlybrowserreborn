/*==================================================================*/
/*                                                                  */
/* SGMLInputTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLINPUTTEXTP_H
#define SGMLINPUTTEXTP_H

#include "SGMLInputText.h"
#include "SGMLContainerTextP.h"
#include "xgifload.h"

typedef struct _SGMLInputTextClassPart{

    XtGeometryHandler geometry_manager;	  /* geometry manager for children   */
    XtPointer	      extension;	  /* pointer to extension record     */

} SGMLInputTextClassPart;

typedef struct _SGMLInputTextClassRec{
  
    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLContainerTextClassPart sgml_container_text_class;
    SGMLInputTextClassPart     sgml_input_text_class;

} SGMLInputTextClassRec, *SGMLInputTextObjectClass;

extern SGMLInputTextClassRec sGMLInputTextClassRec;

typedef struct _SGMLInputTextPart {

    XrmQuark     type;
    char        *value;
    char	*name;
    int          size;
    int          maxlength;
    int		 rows;
    int		 cols;
    Boolean      checked;
    Boolean      selected;
    Boolean      multiple;
    Boolean      auto_submit;
    Widget       child;
    Pixmap       bomb;
    GIFImage    *image;
    char        *passwd;

} SGMLInputTextPart;

typedef struct _SGMLInputTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLContainerTextPart  sgml_container_text;
    SGMLInputTextPart      sgml_input_text;
} SGMLInputTextRec;

#endif SGMLINPUTTEXTP_H
