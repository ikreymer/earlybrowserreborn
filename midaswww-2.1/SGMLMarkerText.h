/*==================================================================*/
/*                                                                  */
/* SGMLMarkerTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines marker text segment for the SGMLText widget              */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLMARKERTEXT_H
#define  SGMLMARKERTEXT_H
#include "SGMLMarkerText.h"

extern WidgetClass sGMLMarkerTextObjectClass;
typedef struct _SGMLMarkerTextRec  *SGMLMarkerTextObject;

#define SGMLNcolumnAlign     "columnAlign"
#define SGMLCColumnAlign     "ColumnAlign"
#define SGMLNposition        "position"
#define SGMLCPosition        "Position"
#define SGMLNpixmapDepth     "pixmapDepth"
#define SGMLNpixmap          "pixmap"
#define SGMLCPixmap          "Pixmap"
#define SGMLNimage           "image"
#define SGMLCImage           "Image"
#define SGMLNdeleteImageWhenDone "deleteImageWhenDone"
#define SGMLCDeleteImageWhenDone "DeleteImageWhenDone"
#define SGMLNbombPixmap      "bombPixmap"

/*
  Callback structure (cf SGMLText.h, but with x,y added)
*/

typedef struct {

    int     reason;   /* always = HYPER_REASON                            */
    XEvent *event;    /* event                                            */
    char   *text;     /* pointer on highlighted text selected (read only) */
    int     length;   /* length of selected text                          */
    char   *param;    /* pointer to hidden text */
    int     param_length; 
    Position x,y;

}  SGMLMarkerTextCallbackStruct;

#ifdef _NO_PROTO

extern Widget  CreateSGMLMarkerText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateMarkerText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsMarkerText(w)     XtIsSubclass(w,sGMLMarkerTextObjectClass)

#endif SGMLMARKERTEXT_H

