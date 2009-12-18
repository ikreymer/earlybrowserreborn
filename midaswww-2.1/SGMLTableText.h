/*==================================================================*/
/*                                                                  */
/* SGMLTableTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a table  text segment for the SGMLText widget            */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLTABLETEXT_H
#define  SGMLTABLETEXT_H
#include "SGMLText.h"

#define SGMLNrows          "rows"
#define SGMLNcolumns       "columns" 
#define SGMLNborder        "border"
#define SGMLCBorder        "Border"

extern WidgetClass sGMLTableTextObjectClass;
typedef struct _SGMLTableTextRec  *SGMLTableTextObject;

#define SGMLIsTableText(w)     XtIsSubclass(w,sGMLTableTextObjectClass)

#endif SGMLTABLETEXT_H

