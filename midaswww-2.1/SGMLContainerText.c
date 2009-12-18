/*==================================================================*/
/*                                                                  */
/* SGMLContainerTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a container text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLContainerTextP.h"
#include "SGMLCompositeTextP.h"


/* 
  Private functions 
*/


/*
  Widget class methods
*/

static void    ClassPartInitialize();
static void    ComputeSize();
static Boolean AdjustSize();
static void    Redisplay();
static void    CallMapCallback();
static XtGeometryResult GeometryManager();

#define Offset(field) XtOffsetOf(SGMLContainerTextRec,sgml_container_text.field)

static XtResource resources[] = {

    {SGMLNchild,SGMLCChild,SGMLRWidget,sizeof(Widget),
    Offset (child),XtRImmediate,(XtPointer)0},
  
    {SGMLNverticalOffset,SGMLCVerticalOffset,XtRPosition,sizeof(Position),
    Offset (vertical_offset),XtRImmediate,(XtPointer)0},
 
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLContainerTextClassRec  sGMLContainerTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLContainerText",                 /* class_name            */
    sizeof(SGMLContainerTextRec),        /* widget_size           */
    NULL,                                /* class_initialize      */
    ClassPartInitialize,                 /* class_part_initialize */
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
    NULL,	                         /* destroy               */
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
    ComputeSize,	    		 /* compute_size          */
    AdjustSize,                          /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
    SGMLInheritActivate,                 /* activate              */
    SGMLInheritHilite,                   /* hilite                */
    SGMLInheritContains,                 /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    CallMapCallback,                     /* call_map_callback     */
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
    GeometryManager,                     /* geometry_manager      */ 
    NULL,                                /* extension             */
    }
};


WidgetClass sGMLContainerTextObjectClass = (WidgetClass) &sGMLContainerTextClassRec;

/*--------------------------------------------------------------*/
/* ClassPartInitialize:                                         */
/*--------------------------------------------------------------*/

static void ClassPartInitialize (class)
SGMLContainerTextObjectClass class;
{
   SGMLContainerTextObjectClass superclass = (SGMLContainerTextObjectClass) class->object_class.superclass; 
      
#define inherit(field,defval) if (class->sgml_container_text_class.field == defval) \
                                  class->sgml_container_text_class.field = superclass->sgml_container_text_class.field

   inherit(geometry_manager,SGMLInheritGeometryManager);
#undef inherit
}
/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLContainerTextObject t;
XEvent *event;
Region region;
{
}
/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/
static void ComputeSize(w,geom,Adjust,Closure)
SGMLContainerTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   Dimension left_clearance  = w->sgml_text.left_margin + w->sgml_text.left_indent;
   Dimension right_clearance = w->sgml_text.right_margin + w->sgml_text.right_indent;
   Widget child = w->sgml_container_text.child;

   if (w->sgml_text.break_before == SGMLBREAK_ALWAYS) _SGMLBreak(geom,w->sgml_text.space_before);

   if (left_clearance > geom->coord.x ) geom->coord.x = left_clearance;

   w->sgml_text.begin  = geom->coord;

   if (child)
     { 
      Dimension width  = child->core.width;
      Dimension height = child->core.height;
      int offset =  w->sgml_container_text.vertical_offset;
      Dimension ascent  = (height + offset > 0) ? height + offset : 0;
      Dimension descent = (offset < 0) ? -offset : 0;   

      if (geom->coord.x + w->sgml_text.spacing + width + right_clearance > geom->natural_width &&
          w->sgml_text.break_before != SGMLBREAK_NEVER)
        {
          geom->coord.x = left_clearance;
          geom->coord.y += geom->coord.descent + geom->coord.ascent;
          geom->coord.descent = w->sgml_text.descent;
          geom->coord.ascent = w->sgml_text.ascent;
       }
      else if (geom->leave_space) geom->coord.x += w->sgml_text.spacing;   

      if (ascent > geom->coord.ascent || descent > geom->coord.descent)
        {
          if (ascent  > geom->coord.ascent)  geom->coord.ascent  = ascent;
          if (descent > geom->coord.descent) geom->coord.descent = descent; 

          Adjust(Closure, geom->coord.y, geom->coord.ascent, geom->coord.descent);
        }

      if (left_clearance > geom->coord.x && geom->broken) geom->coord.x = left_clearance;

      w->sgml_text.begin  = geom->coord;
      w->sgml_text.margin = geom->coord.x;

      XtMoveWidget(child,geom->coord.x,geom->coord.y + geom->coord.ascent - ascent);

      geom->coord.x += width;
      geom->leave_space = TRUE;
      geom->broken = FALSE;

      if (geom->actual_width < geom->coord.x) 
         geom->actual_width = geom->coord.x;

      w->sgml_text.height = height;   
      w->sgml_text.width  = width;
    }
  else
    {
      w->sgml_text.height = 0;   
      w->sgml_text.width  = 0;
    }

  w->sgml_text.margin = geom->coord.x - w->sgml_text.width; 
  w->sgml_text.end = geom->coord;

  if (w->sgml_text.break_after == SGMLBREAK_ALWAYS) _SGMLBreak(geom,w->sgml_text.space_after);
  w->sgml_text.size_valid = TRUE;   
}
/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,descent)
SGMLContainerTextObject w;
Position y;
Dimension ascent, descent;
{
   Widget child = w->sgml_container_text.child;
   if (w->sgml_text.end.y != y) return FALSE;
     
   if (child && ascent > w->sgml_text.end.ascent)
     {
       XtMoveWidget(child,w->sgml_text.begin.x,w->sgml_text.begin.y + ascent - w->sgml_text.end.ascent);
     }

   w->sgml_text.height += descent - w->sgml_text.end.descent + ascent - w->sgml_text.end.ascent; 
   w->sgml_text.end.ascent = ascent;
   w->sgml_text.end.descent = descent;

   if (w->sgml_text.begin.y == y)
     {
       w->sgml_text.begin.ascent = ascent;
       w->sgml_text.begin.descent = descent;
       return TRUE;
     }
        
   return FALSE;
}
/*--------------------------------------------------------------*/
/* Call the map Callback:                                       */
/*--------------------------------------------------------------*/

static void CallMapCallback(w,event,map)
SGMLContainerTextObject w;
XEvent         *event;
Boolean        map;
{
    Widget child = w->sgml_container_text.child;
    SGMLTextCallbackStruct cb;

    cb.reason = SGMLTEXT_MAP;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;

    if (child)
      {
        if (map)  XtManageChild(child);
        else      XtUnmanageChild(child);
      }  
  
    if (map) XtCallCallbacks((Widget) w,SGMLNmapCallback  ,(XtPointer) &cb);
    else     XtCallCallbacks((Widget) w,SGMLNunmapCallback,(XtPointer) &cb);
}
/*--------------------------------------------------------------*/
/* GeometryManager:                                             */
/*--------------------------------------------------------------*/

static XtGeometryResult GeometryManager(w,desired,allowed)
SGMLContainerTextObject w;
XtWidgetGeometry *desired, *allowed;
{
  Widget child = w->sgml_container_text.child;
  Widget parent = XtParent((Widget) w);
  SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
  
#define Wants(flag) (desired->request_mode & flag)  

  if (Wants(XtCWQueryOnly)) return XtGeometryYes;
  if (Wants(CWWidth))  child->core.width  = desired->width;
  if (Wants(CWHeight)) child->core.height = desired->height;

  return (*parentClass->sgml_composite_text_class.geometry_manager)(parent,desired,allowed);

#undef Wants
}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLContainerTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateContainerText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLContainerTextObjectClass,parent,al,ac);
}
