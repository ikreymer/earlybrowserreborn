/*==================================================================*/
/*                                                                  */
/* SGMLRuleTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a rule text segment for the SGMLHyper widget           */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <Xm/Xm.h>
#include "SGMLRuleTextP.h"

/* 
  Private functions 
*/


/*
  Widget class methods
*/

static void    ComputeSize();
static void    Initialize();
static void    Destroy();
static Boolean SetValues();
static void    Redisplay();

#define Offset(field) XtOffsetOf(SGMLRuleTextRec,sgml_rule_text.field)

static XtResource resources[] = {


    {SGMLNruleHeight,SGMLCRuleHeight,XtRDimension,sizeof(Dimension),
    Offset(height),XtRImmediate,(XtPointer)2},
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLRuleTextClassRec  sGMLRuleTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLRuleText",                    /* class_name            */
    sizeof(SGMLRuleTextRec),           /* widget_size           */
    NULL,                                /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    Initialize,                          /* initialize            */
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
    Destroy,	                         /* destroy               */
    NULL,                                /* obj8                  */
    NULL,                                /* obj9                  */
    SetValues,                           /* set_values            */
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
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
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
    NULL,                                /* ignore                */
    }
};


WidgetClass sGMLRuleTextObjectClass = (WidgetClass) &sGMLRuleTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new) 
SGMLRuleTextObject request, new;
{
  Widget parent;
  XGCValues values;
  XtGCMask  valueMask;  
  Pixel top, bottom;
  int n = 0;
  Arg arglist[10]; 

  valueMask = GCForeground;

  for (parent = (Widget) new; !XtIsSubclass(parent,xmManagerWidgetClass); parent = XtParent(parent));   
   
  XtSetArg(arglist[n],XmNbottomShadowColor,&bottom); n++;
  XtSetArg(arglist[n],XmNtopShadowColor,&top); n++;
  XtGetValues(parent,arglist,n);   

  values.foreground = top;
  new->sgml_rule_text.gc1 = XtGetGC((Widget) new, valueMask, &values); 
  
  values.foreground = bottom;
  new->sgml_rule_text.gc2 = XtGetGC((Widget) new, valueMask, &values); 

}
/*--------------------------------------------------------------*/
/* Destroy:                                                     */
/*--------------------------------------------------------------*/

static void Destroy(w) 
SGMLRuleTextObject w;
{
  XtReleaseGC((Widget) w, w->sgml_rule_text.gc1);
  XtReleaseGC((Widget) w, w->sgml_rule_text.gc2);
}
/*------------------------------------------------------------------*/
/* SetValues :                                                      */
/*------------------------------------------------------------------*/

static Boolean SetValues (current, request, new)
SGMLRuleTextObject current, request, new;
{

#define HAS_CHANGED(a) (new->sgml_rule_text.a != current->sgml_rule_text.a)

#undef HAS_CHANGED  
  
  return FALSE;
}
/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLRuleTextObject t;
XEvent *event;
Region region;
{
   Position y = t->sgml_text.begin.y;
   Position margin = t->sgml_text.margin;
   Dimension width  = t->sgml_text.width;
   Dimension height = t->sgml_text.height;

   if(XRectInRegion(region,margin,y,width,height) != RectangleOut)
     {
       Display *dpy = XtDisplayOfObject((Widget) t);
       Window  wind = XtWindowOfObject((Widget) t);
       GC gc1 =  t->sgml_rule_text.gc1;
       GC gc2 =  t->sgml_rule_text.gc2;

       XFillRectangle(dpy,wind,gc2,margin,y,width,height/2);
       XFillRectangle(dpy,wind,gc1,margin,y+height/2,width,height/2);
     }
}
/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/
static void ComputeSize(w,geom,Adjust,Closure)
SGMLRuleTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
  Dimension height = 2 * w->sgml_rule_text.height;
  Dimension width = geom->natural_width - w->sgml_text.left_margin - w->sgml_text.left_indent
                                        - w->sgml_text.right_margin - w->sgml_text.right_indent; 

  _SGMLBreak(geom,w->sgml_text.space_before);

  geom->coord.x = w->sgml_text.left_margin + w->sgml_text.left_indent;

  w->sgml_text.begin  = geom->coord;
  w->sgml_text.margin = w->sgml_text.left_margin + w->sgml_text.left_indent;

  geom->coord.x += width;
  geom->coord.y += height;
  geom->broken = FALSE; 

  w->sgml_text.height = height;   
  w->sgml_text.width  = width; 

  if (geom->actual_width < geom->natural_width) 
    geom->actual_width = geom->natural_width;
     
  w->sgml_text.end = geom->coord;

  _SGMLBreak(geom,w->sgml_text.space_after);
  w->sgml_text.size_valid = TRUE; 
}
 /*-----------------------------------------------------------------------*/
/* Create a new SGMLRuleTextObject                                      */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateRuleText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLRuleTextObjectClass,parent,al,ac);
}
