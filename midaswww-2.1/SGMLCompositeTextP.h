/*==================================================================*/
/*                                                                  */
/* SGMLCompositeTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLCOMPOSITETEXTP_H
#define SGMLCOMPOSITETEXTP_H

#include "SGMLCompositeText.h"
#include "SGMLTextP.h"

typedef void (*CallAdjustSizeProc)();
#define SGMLInheritCallAdjustSize         ((CallAdjustSizeProc)         _XtInherit) 
#define SGMLInheritGeometryManager    XtInheritGeometryManager
#define SGMLInheritChangeManaged      XtInheritChangeManaged
#define SGMLInheritInsertChild        XtInheritInsertChild
#define SGMLInheritDeleteChild        XtInheritDeleteChild
 
typedef struct _SGMLCompositeTextClassPart{

    XtGeometryHandler  geometry_manager;  /* geometry manager for children      */
    XtWidgetProc       change_managed;	  /* change managed state of child      */
    XtWidgetProc       insert_child;	  /* physically add child to parent     */
    XtWidgetProc       delete_child;	  /* physically remove child	        */
    CallAdjustSizeProc call_adjust_size;  /* call children's adjust size method */
    XtPointer	       extension;	  /* pointer to extension record        */

} SGMLCompositeTextClassPart;

typedef struct _SGMLCompositeTextClassRec{

    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLCompositeTextClassPart sgml_composite_text_class;

} SGMLCompositeTextClassRec, *SGMLCompositeTextObjectClass;

extern SGMLCompositeTextClassRec sGMLCompositeTextClassRec;

typedef struct _SGMLCompositeTextPart {

    WidgetList  children;	     /* array of ALL widget children	     */
    Boolean     *managed;            /* which children are managed           */
    Cardinal    num_children;	     /* total number of widget children	     */
    Cardinal    num_slots;           /* number of slots in children array    */
    XtOrderProc insert_position;     /* compute position of new child	     */

    SGMLTagList *tag_list;           /* list of allowed tags                 */
    Boolean     own_tag_list;        /* If the tag list needs to be freed    */
    WidgetClass default_class;       /* default class of children            */ 
    int         management_policy;   /* policy for managing children         */

} SGMLCompositeTextPart;

typedef struct _SGMLCompositeTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLCompositeTextPart  sgml_composite_text;
} SGMLCompositeTextRec;

#endif SGMLCOMPOSITETEXTP_H
