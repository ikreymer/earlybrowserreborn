/*==================================================================*/
/*                                                                  */
/* SGMLCaptionTextObject                                            */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "SGMLCaptionTextP.h"

/* 
  Private functions 
*/

/* 
  Private data struct 
*/

/*
  Widget class methods
*/

static void    ComputeSize();

#define Offset(field) XtOffsetOf(SGMLCaptionTextRec,sgml_caption_text.field)

static XtResource resources[] = {

    {SGMLNcaption, SGMLCCaption, XtRWidget, sizeof(Widget),
     Offset(caption), XtRImmediate, (XtPointer) NULL}

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLCaptionTextClassRec  sGMLCaptionTextClassRec = {
    {
    (WidgetClass) &sGMLCompositeTextClassRec, 
                                         /* superclass            */
    "SGMLCaptionText",                   /* class_name            */
    sizeof(SGMLCaptionTextRec),          /* widget_size           */
    NULL,                                /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    NULL,                                /* initialize            */
    NULL,                                /* initialize_hook       */
    NULL,                                /* obj1                  */
    NULL,                                /* obj2                  */
    0,                                   /* obj3                  */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    0,                                   /* obj4                  */
    0,                                   /* obj5                  */
    0,                                   /* obj6                  */
    0,                                   /* obj7                  */
    NULL,                                /* destroy               */
    NULL,                                /* obj8                  */
    NULL,                                /* obj9                  */
    NULL,                                /* set_values            */
    NULL,                                /* set_values_hook       */
    NULL,                                /* obj10                 */
    NULL,                                /* get_values_hook       */
    NULL,                                /* obj11                 */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    NULL,                                /* obj12                 */
    NULL,                                /* obj13                 */
    NULL,                                /* obj14                 */
    NULL,                                /* extension             */
    },
    {
    ComputeSize,          	         /* compute_size          */
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    SGMLInheritExpose,                   /* expose                */
    SGMLInheritActivate,                 /* activate              */
    SGMLInheritHilite,                   /* hilite                */
    SGMLInheritContains,                 /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    SGMLInheritCallMapCallback,          /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    NULL,                                /* sgml_set_values       */
    NULL,                                /* inherit_changes       */
    SGMLInheritComputeChangeMask,        /* compute_change_mask   */
    SGMLInheritSearch,                   /* search                */
    SGMLInheritClearSelect,              /* clear_select          */
    SGMLInheritHiliteSelection,          /* hilite_selection      */
    SGMLInheritXYToPos,                  /* xy_to_pos             */
    SGMLInheritPosToXY,                  /* pos_to_xy             */
    SGMLInheritDumpText,                 /* dump_text             */
    NULL,                                /* extension             */
    },
    {
    SGMLInheritGeometryManager,          /* geometry_manager      */
    SGMLInheritChangeManaged,            /* change_managed        */
    SGMLInheritInsertChild,              /* insert_child          */
    SGMLInheritDeleteChild,              /* delete_child          */
    SGMLInheritCallAdjustSize,           /* call_adjust_size      */
    NULL,                                /* ignore                */
    },
    {
    NULL,                                /* ignore                */
    },
 };

WidgetClass sGMLCaptionTextObjectClass = (WidgetClass) &sGMLCaptionTextClassRec;

static void DoNothing() {}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLCaptionTextObject w;
SGMLGeometry *geom;
CallAdjustSizeProc Adjust;
Opaque Closure;
{
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Boolean *managed = w->sgml_composite_text.managed;
   SGMLPosition xmin, xmax;

   /*
    * Always Break before
    */  

   _SGMLBreak(geom, w->sgml_text.space_before);

   /*
    * Deal with caption first
    */
  
   w->sgml_text.begin = geom->coord;
   xmin = xmax = geom->coord.x; 

   if (w->sgml_caption_text.caption)
     {
       SGMLTextObject t = (SGMLTextObject) w->sgml_caption_text.caption; 
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) t);
 
       (*childClass->sgml_text_class.compute_size)(t,geom,DoNothing,NULL);

       if (t->sgml_text.margin < xmin) xmin = t->sgml_text.margin;
       if (t->sgml_text.margin + t->sgml_text.width > xmax)
       xmax = t->sgml_text.margin + t->sgml_text.width;  
    }

   /*
    * Query children?
    */  

   for (; n-- > 0; children++, managed++)
     {
       if (*managed && *children != w->sgml_caption_text.caption)
         {     
           SGMLTextObject t = (SGMLTextObject) *children; 

           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           SGMLCompositeTextObjectClass myClass = (SGMLCompositeTextObjectClass) XtClass((Widget) w);

           (*childClass->sgml_text_class.compute_size)
              (*children,geom,myClass->sgml_composite_text_class.call_adjust_size,*children);
          
           if (t->sgml_text.margin < xmin) xmin = t->sgml_text.margin;
           if (t->sgml_text.margin + t->sgml_text.width > xmax)
             xmax = t->sgml_text.margin + t->sgml_text.width; 
         }  
     }  

   w->sgml_text.end = geom->coord;
   w->sgml_text.height = geom->coord.y + geom->coord.descent + geom->coord.ascent - w->sgml_text.begin.y;
   w->sgml_text.width  = xmax - xmin;
   w->sgml_text.margin = xmin; 
 
   /*
    * Always Break after
    */  

   _SGMLBreak(geom,w->sgml_text.space_after);
   w->sgml_text.size_valid = TRUE; 
}

