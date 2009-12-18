/*==================================================================*/
/*                                                                  */
/* SGMLCompositeTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite  text segment for the SGMLText widget        */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLCOMPOSITETEXT_H
#define  SGMLCOMPOSITETEXT_H
#include "SGMLText.h"

#define SGMLNtagList          "tagList"
#define SGMLCTagList          "TagList"
#define SGMLRTagList          "TagList"
#define SGMLNdefaultClass     "defaultClass"
#define SGMLCClass            "Class"
#define SGMLRClass            "Class"
#define SGMLNmanagementPolicy "managementPolicy"
#define SGMLCManagementPolicy "ManagementPolicy"
#define SGMLRManagementPolicy "ManagementPolicy"
#define SGMLNchildrenManaged  "childrenManaged"


#define SGMLINHERIT_CLASS NULL
#define SGMLINHERIT_TAGLIST NULL
 
#define SGMLMANAGE_NONE 0
#define SGMLMANAGE_ONE  1
#define SGMLMANAGE_ALL  2

#define SGMLEManageAll  "manageall"
#define SGMLEManageOne  "manageone"
#define SGMLEManageNone "managenone"


extern WidgetClass sGMLCompositeTextObjectClass;
typedef struct _SGMLCompositeTextRec  *SGMLCompositeTextObject;

typedef struct _TagList {

      XrmName     name; 
      WidgetClass class;
      int         polarity; /* Controls inheritance 
                             *    +1 = Add to parents list 
                             *    -1 = Remove from parents list 
                             *     0 = No inheritance 
                             */
      int         matched;  /* Controls searching for matching close tag */

} SGMLTagList;

#ifdef _NO_PROTO

extern void    SGMLCompositeTextInsertChild();
extern Widget  CreateSGMLCompositeText();
extern void    SGMLCompositeTextDeleteChild();
extern Widget  SGMLCreateWidget();

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateCompositeText(Widget parent,char *name,ArgList al,int ac);
    extern void SGMLCompositeTextInsertChild(Widget child);
    extern void SGMLCompositeTextDeleteChild(Widget child);
    extern Widget SGMLCreateWidget(char *name,WidgetClass class,Widget parent,Arg al[],int ac);
    
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsCompositeText(w)     XtIsSubclass(w,sGMLCompositeTextObjectClass)

#endif SGMLCOMPOSITETEXT_H

