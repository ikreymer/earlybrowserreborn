/*==================================================================*/
/*                                                                  */
/* SGMLListTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a list text segment for the SGMLText widget              */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLLISTTEXT_H
#define  SGMLLISTTEXT_H
#include "SGMLCompositeText.h"

#define SGMLNnumBullets   "numBullets"
#define SGMLNallocBullets "allocBullets"
#define SGMLNbulletType   "bulletType"
#define SGMLCBulletType   "BulletType"
#define SGMLRBulletType   "BulletType"
#define SGMLNbulletSize   "bulletSize"
#define SGMLCBulletSize   "BulletSize"
#define SGMLNbulletColor  "bulletColor"
#define SGMLCBulletColor  "BulletColor"
#define SGMLNbulletIndent "bulletIndent"
#define SGMLCBulletIndent "BulletIndent"
#define SGMLNbulletMarker "bulletMarker"
#define SGMLCBulletMarker "BulletMarker"

#define SGMLBULLET_ORDERED     -3
#define SGMLBULLET_UNORDERED   -2
#define SGMLBULLET_DEFAULT     -1
#define SGMLBULLET_BULLET       0
#define SGMLBULLET_DIAMOND      1
#define SGMLBULLET_SQUARE       2

#define SGMLBULLET_NUMERIC      3
#define SGMLBULLET_UPPER        4
#define SGMLBULLET_LOWER        5

#define SGMLBULLET_ARROW       30
#define SGMLBULLET_LINKEDARROW 31


extern WidgetClass sGMLListTextObjectClass;
typedef struct _SGMLListTextRec  *SGMLListTextObject;

#ifdef _NO_PROTO

extern Widget  CreateSGMLListText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateListText(Widget parent,
        char *name,
        ArgList al,
        int ac);

    extern void SGMLListTextInsertChild(Widget parent, Widget child);
    
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsListText(w)     XtIsSubclass(w,sGMLListTextObjectClass)

#endif SGMLLISTTEXT_H

