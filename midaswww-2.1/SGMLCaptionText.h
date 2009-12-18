/*==================================================================*/
/*                                                                  */
/* SGMLCaptionTextObject                                            */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a caption  text segment for the SGMLText widget          */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLCAPTIONTEXT_H
#define  SGMLCAPTIONTEXT_H
#include "SGMLText.h"

#define SGMLNcaption          "caption"
#define SGMLCCaption          "Caption"

extern WidgetClass sGMLCaptionTextObjectClass;
typedef struct _SGMLCaptionTextRec  *SGMLCaptionTextObject;

#define SGMLIsCaptionText(w)     XtIsSubclass(w,sGMLCaptionTextObjectClass)

#endif SGMLCAPTIONTEXT_H

