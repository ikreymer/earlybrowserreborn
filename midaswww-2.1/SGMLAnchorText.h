/*==================================================================*/
/*                                                                  */
/* SGMLAnchorTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLAnchorText widget             */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLANCHORTEXT_H
#define  SGMLANCHORTEXT_H
#include "SGMLCompositeText.h"

extern WidgetClass sGMLAnchorTextObjectClass;
typedef struct _SGMLAnchorTextRec  *SGMLAnchorTextObject;

#define SGMLNhref        "href"
#define SGMLCHref        "Href"
#define SGMLNname        "name"
#define SGMLCName        "Name"
#define SGMLNtitle       "title"
#define SGMLCTitle       "Title"
#define SGMLNhrefLength  "hrefLength"
#define SGMLNnameLength  "nameLength"
#define SGMLNtitleLength "titleLength"
#define SGMLNvisited     "visited"
#define SGMLCVisited     "Visited"

#ifdef _NO_PROTO

extern Widget  CreateSGMLAnchorText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateAnchorText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsAnchorText(w)     XtIsSubclass(w,sGMLAnchorTextObjectClass)

#endif SGMLANCHORTEXT_H
