/*==================================================================*/
/*                                                                  */
/* SGMLContainerTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines container text segment for the SGMLText widget           */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLCONTAINERTEXT_H
#define  SGMLCONTAINERTEXT_H

extern WidgetClass sGMLContainerTextObjectClass;
typedef struct _SGMLContainerTextRec  *SGMLContainerTextObject;

#define SGMLNchild         "child"
#define SGMLCChild         "Child"
#define SGMLNverticalOffset "verticalOffset"
#define SGMLCVerticalOffset "VerticalOffset"
#define SGMLRWidget        "Widget"

#ifdef _NO_PROTO

extern Widget  CreateSGMLContainerText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateContainerText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsContainerText(w)     XtIsSubclass(w,sGMLContainerTextObjectClass)

#endif SGMLCONTAINERTEXT_H

