/*==================================================================*/
/*                                                                  */
/* SGMLContainerTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines input text segment for the SGMLText widget               */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLINPUTTEXT_H
#define  SGMLINPUTTEXT_H

extern WidgetClass sGMLInputTextObjectClass;
typedef struct _SGMLInputTextRec  *SGMLInputTextObject;

#define SGMLNtype          "type"
#define SGMLCType          "Type"
#define SGMLNname          "name"
#define SGMLCName          "Name"
#define SGMLNvalue         "value"
#define SGMLCValue         "Value"
#define SGMLNsize          "size"
#define SGMLCSize          "Size"
#define SGMLNrows          "rows"
#define SGMLCRows          "Rows"
#define SGMLNcols          "cols"
#define SGMLCCols          "Cols"
#define SGMLNchecked       "checked"
#define SGMLCChecked       "Checked"
#define SGMLNselected      "selected"
#define SGMLCSelected      "Selected"
#define SGMLNmultiple      "multiple"
#define SGMLCMultiple      "Multiple"
#define SGMLNmaxlength     "maxlength"
#define SGMLCMaxlength     "Maxlength"
#define SGMLNautosubmit    "autosubmit"
#define SGMLCAutosubmit    "Autosubmit"
#define SGMLNimage         "image"
#define SGMLCImage         "Image"
#define SGMLNbombPixmap    "bombPixmap"
#define SGMLCBombPixmap    "BombPixmap" 
 
#ifdef _NO_PROTO

extern Widget  CreateSGMLInputText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateInputText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsInputText(w)     XtIsSubclass(w,sGMLInputTextObjectClass)

#endif SGMLINPUTTEXT_H

