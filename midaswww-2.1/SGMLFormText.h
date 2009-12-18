/*==================================================================*/
/*                                                                  */
/* SGMLFormTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a form  text segment for the SGMLText widget             */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLFORMTEXT_H
#define  SGMLFORMTEXT_H
#include "SGMLCompositeText.h"

#define SGMLNaction        "action"
#define SGMLCAction        "Action"
#define SGMLNmethod        "method"
#define SGMLCMethod        "Method"
#define SGMLNresult        "result"
#define SGMLCResult        "Result"

extern WidgetClass sGMLFormTextObjectClass;
typedef struct _SGMLFormTextRec  *SGMLFormTextObject;

void   SGMLFormSetResult();

#define SGMLIsFormText(w)     XtIsSubclass(w,sGMLFormTextObjectClass)

#endif SGMLFORMTEXT_H
