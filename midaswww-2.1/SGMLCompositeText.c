/*==================================================================*/
/*                                                                  */
/* SGMLCompositeTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLCompositeTextP.h"
#include "SGMLFormattedText.h"

/* 
  Private functions 
*/

static void    DestroyChildren();
static void    MakeGeometryRequest();
static Widget  ExternalSearch();

/* 
  Private data struct 
*/

typedef struct _searchdata {
  int currentpos;
  int lastpos;
  SGMLCompositeTextObject cw;
  Widget (*extsearch)();
  XtPointer pclosure;
} searchdata;

/*
  Widget class methods
*/

static void    ClassInitialize();
static void    ClassPartInitialize();
static void    Initialize();
static void    Destroy();
static void    InsertChild();
static void    DeleteChild();
static void    ComputeSize();
static void    Redisplay();
static void    Hilite();
static Boolean SGMLSetValues();
static Widget  Contains();
static void    CallMapCallback();
static Widget  Search();
static Boolean ClearSelect(); 
static XtGeometryResult GeometryManager();
static void    CallAdjustSize();
static Boolean AdjustSize(); 
static void    ChangeManaged();  
static Boolean InheritChanges();
static Boolean DumpText();
static Widget  HiliteSelection();

#define Offset(field) XtOffsetOf(SGMLCompositeTextRec,sgml_composite_text.field)

static XtResource resources[] = {

    {XtNchildren, XtCReadOnly, XtRWidgetList, sizeof(WidgetList),
     Offset(children), XtRImmediate, NULL},
    {SGMLNchildrenManaged, XtCReadOnly, XtRPointer, sizeof(XtPointer),
     Offset(managed), XtRImmediate, NULL},    
    {XtNnumChildren, XtCReadOnly, XtRCardinal, sizeof(Cardinal),
     Offset(num_children), XtRImmediate, 0},
    {XtNinsertPosition, XtCInsertPosition, XtRFunction, sizeof(XtOrderProc),
     Offset(insert_position), XtRImmediate, NULL},

    {SGMLNtagList, SGMLCTagList, SGMLRTagList, sizeof(SGMLTagList),
     Offset(tag_list), XtRImmediate, SGMLINHERIT_TAGLIST},
    {SGMLNdefaultClass, SGMLCClass, SGMLRClass, sizeof(WidgetClass),
     Offset(default_class), XtRImmediate, SGMLINHERIT_CLASS},

    {SGMLNmanagementPolicy, SGMLCManagementPolicy, SGMLRManagementPolicy, sizeof(int),
     Offset(management_policy), XtRImmediate, (XtPointer)SGMLMANAGE_ALL}

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLCompositeTextClassRec  sGMLCompositeTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLCompositeText",                 /* class_name            */
    sizeof(SGMLCompositeTextRec),        /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
    ClassPartInitialize,                 /* class_part_initialize */
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
    Destroy,                             /* destroy               */
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
    ComputeSize,          		 /* compute_size          */
    AdjustSize,                          /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
    SGMLInheritActivate,                 /* activate              */
    Hilite,                              /* hilite                */
    Contains,                            /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    CallMapCallback,                     /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    SGMLSetValues,                       /* sgml_set_values       */
    InheritChanges,                      /* inherit_changes       */
    SGMLInheritComputeChangeMask,        /* compute_change_mask   */
    Search,                              /* search                */
    ClearSelect,                         /* clear_select          */
    HiliteSelection,                     /* hilite_selection      */
    SGMLInheritXYToPos,                  /* xy_to_pos             */
    SGMLInheritPosToXY,                  /* pos_to_xy             */
    DumpText,                            /* dump_text             */
    NULL,                                /* extension             */
    },
    {
    GeometryManager,                     /* geometry_manager      */
    ChangeManaged,                       /* change_managed        */
    InsertChild,                         /* insert_child          */
    DeleteChild,                         /* delete_child          */
    CallAdjustSize,                      /* call_adjust_size      */
    NULL,                                /* ignore                */
    }
};

WidgetClass sGMLCompositeTextObjectClass = (WidgetClass) &sGMLCompositeTextClassRec;

#define done(address, type) \
        { (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

static struct _namepair {
    XrmQuark quark;
    char *name;
    int policy;
} names[] = {
    { NULLQUARK, SGMLEManageAll,  SGMLMANAGE_ALL },
    { NULLQUARK, SGMLEManageOne,  SGMLMANAGE_ONE },
    { NULLQUARK, SGMLEManageNone, SGMLMANAGE_NONE},
    { NULLQUARK, NULL, 0 }
};

void CvtStringToManagementPolicy (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static Boolean haveQuarks = FALSE;
    char lowerName[40];
    XrmQuark q;
    char *s, *t;
    struct _namepair *np;

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToManagementPolicy","XtToolkitError",
                  "String to ManagementPolicy conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    if (!haveQuarks) {
	for (np = names; np->name; np++) {
	    np->quark = XrmStringToQuark (np->name);
	}
	haveQuarks = TRUE;
    }

    s = (char *) fromVal->addr;
    if (strlen(s) < sizeof lowerName) {
	for (t=lowerName;  (*t++ = tolower(*s++)) != '\0'; );
	q = XrmStringToQuark (lowerName);

	for (np = names; np->name; np++) {
	    if (np->quark == q) {
		done (&np->policy, int);
		return;
	    }
	}
    }
    XtStringConversionWarning((char *) fromVal->addr, SGMLRManagementPolicy);
}

/*--------------------------------------------------------------*/
/* ClassInitialize:                                             */
/*--------------------------------------------------------------*/

static void ClassInitialize ()
{
    XtAddConverter (XtRString, SGMLRManagementPolicy, CvtStringToManagementPolicy,
		        (XtConvertArgList) NULL, (Cardinal) 0);
}

/*--------------------------------------------------------------*/
/* ClassPartInitialize:                                         */
/*--------------------------------------------------------------*/

static void ClassPartInitialize(widgetClass)
	WidgetClass widgetClass;
{
    register SGMLCompositeTextClassPart *wcPtr;
    register SGMLCompositeTextClassPart *superPtr;

    wcPtr = (SGMLCompositeTextClassPart *)
	&(((SGMLCompositeTextObjectClass)widgetClass)->sgml_composite_text_class);

    if (widgetClass != sGMLCompositeTextObjectClass)
	/* don't compute possible bogus pointer */
	superPtr = (SGMLCompositeTextClassPart *)&(((SGMLCompositeTextObjectClass)widgetClass
			->core_class.superclass)->sgml_composite_text_class);
#ifdef lint
    else
	superPtr = NULL;
#endif

    /* We don't need to check for null super since we'll get to composite
       eventually, and it had better define them!  */

    if (wcPtr->geometry_manager == XtInheritGeometryManager) {
	wcPtr->geometry_manager =
		superPtr->geometry_manager;
    }

    if (wcPtr->change_managed == XtInheritChangeManaged) {
	wcPtr->change_managed =
		superPtr->change_managed;
    }

    if (wcPtr->insert_child == XtInheritInsertChild) {
	wcPtr->insert_child = superPtr->insert_child;
    }

    if (wcPtr->delete_child == XtInheritDeleteChild) {
	wcPtr->delete_child = superPtr->delete_child;
    }

    if (wcPtr->change_managed == XtInheritChangeManaged) {
	wcPtr->change_managed = superPtr->change_managed;
    }

    if (wcPtr->call_adjust_size == SGMLInheritCallAdjustSize) {
	wcPtr->call_adjust_size = superPtr->call_adjust_size;
    }

}

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLCompositeTextObject request, new;
{
    register SGMLCompositeTextObject cw;
    SGMLCompositeTextObject parent = (SGMLCompositeTextObject) XtParent((Widget) new);

    cw = (SGMLCompositeTextObject) new;
    cw->sgml_composite_text.num_children = 0;
    cw->sgml_composite_text.children = NULL;
    cw->sgml_composite_text.managed = NULL;
    cw->sgml_composite_text.num_slots = 0;
    cw->sgml_composite_text.own_tag_list = FALSE;

    if (SGMLIsCompositeText((Widget) parent))
    {
       if (new->sgml_composite_text.default_class == SGMLINHERIT_CLASS)
         new->sgml_composite_text.default_class = parent->sgml_composite_text.default_class;

       if (new->sgml_composite_text.tag_list == SGMLINHERIT_TAGLIST)
         new->sgml_composite_text.tag_list = parent->sgml_composite_text.tag_list;

       else if (new->sgml_composite_text.tag_list[0].polarity)
         {
           /* Deal with partial inheritance of taglists */
           
           int i,j,k,num_slots = 0; 
           SGMLTagList *oldlist = parent->sgml_composite_text.tag_list; 
           SGMLTagList *list    = new->sgml_composite_text.tag_list;
           SGMLTagList *newlist;

           for (i = 0; oldlist[i].name; i++) num_slots++;
           for (i = 0; list[i].name; i++) if (list[i].polarity > 0) num_slots++;
           num_slots++; /* leave space for terminating null */

           newlist = (SGMLTagList *) XtMalloc(sizeof(SGMLTagList) * num_slots); 
           
           for (k = 0; oldlist[k].name; k++) newlist[k] = oldlist[k];
           for (i = 0; list[i].name; i++)
             {
               for (j = 0; j < k ; j++) 
                 if (list[i].name == newlist[j].name) break;
                 
               if      (list[i].polarity > 0 && j < k ) continue; 
               else if (list[i].polarity < 0 && j == k) continue;
               else if (list[i].polarity > 0) 
                 {
                   newlist[k] = list[i];
                   newlist[k].polarity = 0;
                   k++;
                 }  
               else
                 {
                   k--;
                   newlist[j] = newlist[k];
                 }
             }
           newlist[k].name = 0;
           newlist[k].class = NULL;
           newlist[k].polarity = 0;   
             
           new->sgml_composite_text.tag_list = newlist;        
           new->sgml_composite_text.own_tag_list = TRUE;
         } 
    }
    else
    {
       if (new->sgml_composite_text.default_class == SGMLINHERIT_CLASS)
         new->sgml_composite_text.default_class = sGMLFormattedTextObjectClass;
     }

}

/*--------------------------------------------------------------*/
/* DestroyChildnen:                                             */
/*--------------------------------------------------------------*/

static void DestroyChildren(w)
SGMLCompositeTextObject w;
{
    int n = w->sgml_composite_text.num_children;
    WidgetList children = w->sgml_composite_text.children;

    for (; n-- > 0; ) XtDestroyWidget(*children++);
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLCompositeTextObject w;
{
/*
 * We have to explicitly delete all of out children since the 
 * intrincics dont know to do it for us.  
 */
    DestroyChildren(w);
       
    XtFree((char *) w->sgml_composite_text.children);
    XtFree((char *) w->sgml_composite_text.managed);
    if (w->sgml_composite_text.own_tag_list) XtFree((char *)w->sgml_composite_text.tag_list);
}

/*--------------------------------------------------------------*/
/* InsertChild: adds a new child                                */
/*--------------------------------------------------------------*/

static void InsertChild(w)
    Widget	w;
{
    register Cardinal	             position;
    register Cardinal                i;
    register SGMLCompositeTextObject cw;
    register WidgetList              children;
    register Boolean		     *managed;

    cw = (SGMLCompositeTextObject) w->core.parent;
    children = cw->sgml_composite_text.children;
    managed = cw->sgml_composite_text.managed;

    if (cw->sgml_composite_text.insert_position != NULL)
	position = (*(cw->sgml_composite_text.insert_position))(w);
    else
	position = cw->sgml_composite_text.num_children;

    if (cw->sgml_composite_text.num_children == cw->sgml_composite_text.num_slots) {
	/* Allocate more space */
	cw->sgml_composite_text.num_slots +=  (cw->sgml_composite_text.num_slots / 2) + 2;
	cw->sgml_composite_text.children = children = 
	    (WidgetList)  XtRealloc((XtPointer) children,
	    (unsigned) (cw->sgml_composite_text.num_slots) * sizeof(Widget));
	cw->sgml_composite_text.managed = managed = 
	    (Boolean *) XtRealloc((XtPointer) managed,
	    (unsigned) (cw->sgml_composite_text.num_slots) * sizeof(Boolean));
    }

    /* Ripple children up one space from "position" */

    for (i = cw->sgml_composite_text.num_children; i > position; i--) {
	children[i] = children[i-1];
	managed[i]  = managed[i-1];
    }
    children[position] = w;

    if      (!((SGMLTextObject) w)->sgml_text.managed)                     managed[position] = FALSE; 
    else if (cw->sgml_composite_text.management_policy == SGMLMANAGE_NONE) managed[position] = FALSE;
    else if (cw->sgml_composite_text.management_policy == SGMLMANAGE_ALL ) managed[position] = TRUE;
    else
      {
         for (i = 0 ; i < cw->sgml_composite_text.num_children; i++) 
           {
             SGMLTextObject t = (SGMLTextObject) children[i];  
             managed[i] = FALSE;

             if (t->sgml_text.managed && t->sgml_text.size_valid) 
               { 
                 SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) t);
                 (*class->sgml_text_class.call_map_callback)(t,NULL,FALSE);
               }
             t->sgml_text.managed = FALSE;
           }
         managed[position] = TRUE;
      }
    
    ((SGMLTextObject) children[position])->sgml_text.managed = managed[position];
    cw->sgml_composite_text.num_children++;
 
    if (managed[position] && cw->sgml_text.size_valid)
      { 
        XtWidgetGeometry dummy;
        SGMLCompositeTextObjectClass class = (SGMLCompositeTextObjectClass) XtClass((Widget) cw);
        XtGeometryResult result;
        result = (*class->sgml_composite_text_class.geometry_manager)((Widget)cw,&dummy,&dummy); 
      }
}

/*--------------------------------------------------------------*/
/* DeleteChild: delete a child                                  */
/*--------------------------------------------------------------*/

static void DeleteChild(w)
    Widget	w;
{
    Boolean  WasManaged; 
    register Cardinal	             position;
    register Cardinal	             i;
    register SGMLCompositeTextObject cw;

    cw = (SGMLCompositeTextObject) w->core.parent;

    for (position = 0; position < cw->sgml_composite_text.num_children; position++) {
        if (cw->sgml_composite_text.children[position] == w) {
	    break;
	}
    }
    WasManaged = cw->sgml_composite_text.managed[position];

    if (position != cw->sgml_composite_text.num_children)
    {

      /* Ripple children down one space from "position" */
      cw->sgml_composite_text.num_children--;
      for (i = position; i < cw->sgml_composite_text.num_children; i++) {
          cw->sgml_composite_text.children[i] = cw->sgml_composite_text.children[i+1];
      }
    }

    if (WasManaged && cw->sgml_text.managed) MakeGeometryRequest(cw);
}

/*--------------------------------------------------------------*/
/* Contains:                                                    */
/*--------------------------------------------------------------*/

static Widget Contains(w,x,y,mode)
SGMLCompositeTextObject w;
Position x,y;
Boolean mode;
{
  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;
  Boolean *managed = w->sgml_composite_text.managed;

  /*
   * Two possibilities, if I am sensitive myself then maybe I contain
   * the point in question, otherwise maybe one of my children does.
   */

  for (; n-- > 0; children++, managed++)
    {
       if (*managed)
         {     
           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           Widget result = (*childClass->sgml_text_class.contains)(*children,x,y,mode);
           if (result) return result;  
         }           
    }         
  return NULL;
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region,Offset)
SGMLCompositeTextObject t;
XEvent *event;
Region region;
SGMLPosition Offset;
{
   int n = t->sgml_composite_text.num_children;
   WidgetList children = t->sgml_composite_text.children;
   Boolean *managed = t->sgml_composite_text.managed;

   for (; n-- > 0; children++, managed++)
     {
       if (*managed)
         {     
           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           (*childClass->sgml_text_class.expose)(*children,event,region,Offset);
         }  
     }  
}


/*------------------------------------------------------------------*/
/* Adjust the size of a child                                       */
/*------------------------------------------------------------------*/

static void CallAdjustSize(child,y,ascent,descent)
Widget child;
SGMLPosition y;
SGMLDimension ascent, descent;
{
   SGMLCompositeTextObject w = (SGMLCompositeTextObject) XtParent(child);
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Boolean *managed = w->sgml_composite_text.managed;
   WidgetList p = children;
   Boolean *q = managed; 
   Boolean cont;
  
   for (; n-- > 0; p++, q++)
     if (*p == child)
       {
         for (;  p-- != children ; ) 
           {
             if (*--q)
               {
                 SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*p);
                 cont =  (*childClass->sgml_text_class.adjust_size)(*p,y,ascent,descent);
                 if (!cont) return; 
               }  
           }
         break;
       }      
   
   /*
    * If we get here then we have adjusted all the children and we are
    * still on the same line, so must adjust ourself, and then invoke our
    * parent's call adjust size procedure.
    */   

   if (w->sgml_text.begin.y == y) 
     {
       Widget parent = XtParent((Widget) w);

       w->sgml_text.begin.ascent = ascent;
       w->sgml_text.begin.descent = descent;

       if (SGMLIsCompositeText(parent))
         {
           SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
           (*parentClass->sgml_composite_text_class.call_adjust_size)(w,y,ascent,descent);
         }
     }
}
/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,descent)
SGMLCompositeTextObject w;
SGMLPosition y;
SGMLDimension ascent, descent;
{
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Boolean *managed = w->sgml_composite_text.managed;
   WidgetList p = children + n;
   Boolean *q = managed + n; 
   Boolean cont;
  
   for (; p-- != children ; ) 
     {
       if (*--q)
         {
           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*p);
           cont =  (*childClass->sgml_text_class.adjust_size)(*p,y,ascent,descent);
           if (!cont) return FALSE; 
         }  
     }
   return TRUE;
}  
/*--------------------------------------------------------------*/
/* Call the map Callback:                                       */
/*--------------------------------------------------------------*/

static void CallMapCallback(w,event,map)
SGMLCompositeTextObject w;
XEvent         *event;
Boolean        map;
{
   /*
    * Invoke our childrens map callbacks 
    */

    SGMLTextCallbackStruct cb;
    int n = w->sgml_composite_text.num_children;
    WidgetList children = w->sgml_composite_text.children;
    Boolean *managed = w->sgml_composite_text.managed;
 
    for (; n-- > 0; children++, managed++)
      {
       if (*managed)
         {     
           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           (*childClass->sgml_text_class.call_map_callback)(*children,event,map);
         }  
      }  

   /*
    * Then invoke our superclasses map callback 
    */

    cb.reason = SGMLTEXT_MAP;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;
  
    if (map) XtCallCallbacks((Widget) w,SGMLNmapCallback  ,(XtPointer) &cb);
    else     XtCallCallbacks((Widget) w,SGMLNunmapCallback,(XtPointer) &cb);

}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLCompositeTextObject w;
SGMLGeometry *geom;
CallAdjustSizeProc Adjust;
Opaque Closure;
{
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Boolean *managed = w->sgml_composite_text.managed;
   SGMLPosition xmin, xmax;

   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before == SGMLBREAK_ALWAYS)
   {
      _SGMLBreak(geom, w->sgml_text.space_before);
   } 

   /*
    * Query children?
    */  
  
   w->sgml_text.begin = geom->coord;
   xmin = xmax = geom->coord.x; 

   for (; n-- > 0; children++, managed++)
     {
       if (*managed)
         {     
           SGMLTextObject t = (SGMLTextObject) *children; 

           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           (*childClass->sgml_text_class.compute_size)(*children,geom,CallAdjustSize,*children);
          
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
    * Break after?
    */  

   if ( w->sgml_text.break_after == SGMLBREAK_ALWAYS)
   {
      _SGMLBreak(geom,w->sgml_text.space_after);
   } 
   w->sgml_text.size_valid = TRUE; 
}

/*-----------------------------------------------------------------------*/
/* change_managed method                                                 */
/*-----------------------------------------------------------------------*/

static void ChangeManaged(child)
Widget child;
{
  Widget parent = XtParent(child);
  SGMLCompositeTextObject w = (SGMLCompositeTextObject) parent;

  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;
  Boolean *managed = w->sgml_composite_text.managed;
 
  if (((SGMLTextObject) child)->sgml_text.managed)
    {
      for (; n-- > 0; children++, managed++)
        {
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
          if (*children == child)
            {     
              if (*managed) return;
              *managed = TRUE;

              (*childClass->sgml_text_class.call_map_callback)(*children,NULL,TRUE);
      
              /*
               * If our size is valid, but our newly managed child's isn't, we need to compute
               * to set size_valid now, so that SetValues will actually call ComputeSize for 
               * the widget.
               */
              if (w->sgml_text.size_valid) ((SGMLTextObject) child)->sgml_text.size_valid = TRUE;
            }
          else if (w->sgml_composite_text.management_policy == SGMLMANAGE_ONE)             
            {     
              if (*managed) (*childClass->sgml_text_class.call_map_callback)(*children,NULL,FALSE);
              *managed = FALSE; 
              ((SGMLTextObject) *children)->sgml_text.managed = FALSE;
            }     
        }           
    } 
  else
    {
      for (; n-- > 0; children++, managed++)
        {
          if (*children == child)
            {     
              SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
              if (*managed) (*childClass->sgml_text_class.call_map_callback)(*children,NULL,FALSE);
              *managed = FALSE;
            }
        }            
    }
}

/*--------------------------------------------------------------*/
/* Hilite:                                                      */
/*--------------------------------------------------------------*/

static void Hilite(t,state)
SGMLCompositeTextObject t;
int state;
{
  int n = t->sgml_composite_text.num_children;
  WidgetList children = t->sgml_composite_text.children;
  Boolean *managed = t->sgml_composite_text.managed;

  for (; n-- > 0; children++, managed++)
    {
      if (*managed)
        {     
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
          (*childClass->sgml_text_class.hilite)(*children,state);
        }  
    }  
}

/*-----------------------------------------------------------------------*/
/* Search:                                                               */
/*-----------------------------------------------------------------------*/

static Widget Search(exp,sens,w,mode,extsearch,pclosure,sw)
char *exp;
Boolean sens;
SGMLCompositeTextObject w;
Boolean mode;
Widget (*extsearch)();
XtPointer pclosure;
Widget *sw;
{
  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;
  Boolean *managed = w->sgml_composite_text.managed;
  searchdata closure;
  Widget result;

  closure.currentpos = 0;
  closure.lastpos = n;
  closure.cw = w;
  closure.extsearch = extsearch;
  closure.pclosure = pclosure;

  for(; n-- > 0 ; children++, managed++)
    { 
      closure.currentpos++;

      if (*managed)
        {
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
          if (result = (*childClass->sgml_text_class.search)(exp,sens,*children,mode,ExternalSearch,&closure,sw))
            return result;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/
/* External Search                                                       */
/*-----------------------------------------------------------------------*/

static Widget ExternalSearch(exp,sens,closure)
char *exp;
Boolean sens;
searchdata *closure;
{
  SGMLCompositeTextObject w = closure->cw;
  SGMLTextObjectClass childClass;
  int i, j;
  Widget result;
  Widget tempnil = NULL;

  i = closure->currentpos;
  if (closure->currentpos < closure->lastpos)
    {
      WidgetList children = w->sgml_composite_text.children;
      Boolean *managed = w->sgml_composite_text.managed;

      for(j = i; j-- > 0; children++, managed++);
      closure->currentpos++;
      while(!(*managed))
         {
            closure->currentpos++;
            if (closure->currentpos == closure->lastpos)
              {
                closure->currentpos = i;
                return NULL;
              }
            children++; 
            managed++;
         }
      childClass = (SGMLTextObjectClass) XtClass(*children);
      if (result = (*childClass->sgml_text_class.search)(exp,sens,*children,FALSE,ExternalSearch,closure,&tempnil))
         {
           closure->currentpos = i; 
           return result;
          }
       else
          {
            closure->currentpos = i; 
            return NULL;
          }
    }
  else 
    {
      searchdata *pclosure = (searchdata *)closure->pclosure;
      closure->currentpos = i;
      if (pclosure->currentpos < pclosure->lastpos)
        {
          result = closure->extsearch(exp,sens,pclosure);
          return (result ? result : NULL);
        }
      else return NULL;
    }
} 

/*-----------------------------------------------------------------------*/
/* Clear Select                                                          */
/*-----------------------------------------------------------------------*/
static Boolean ClearSelect(w,sw,mode)
SGMLCompositeTextObject w;
Widget sw;
Boolean mode;
{
  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;
  Boolean *managed = w->sgml_composite_text.managed;

  for(; n-- > 0 ; children++, managed++)
    {
      if (*managed)
        {
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
          if ((*childClass->sgml_text_class.clear_select)(*children,sw,mode))
            return TRUE;
        }
    }
  return FALSE;
}


/*-----------------------------------------------------------------------*/
/* Make Geometry Request                                                 */
/*-----------------------------------------------------------------------*/

static void MakeGeometryRequest(w)
SGMLCompositeTextObject w;
{
/*
 *   Very kludgy implementation for now
 */

    Widget parent = (Widget) w;
    for ( ; SGMLIsCompositeText(parent); parent = XtParent(parent))
      {
        SGMLTextObject t = (SGMLTextObject) parent;
        if (!t->sgml_text.size_valid) return;
        t->sgml_text.size_valid = FALSE; 
      }  
    SGMLHyperRedisplay(parent);
}
/*--------------------------------------------------------------*/
/* GeometryManager:                                             */
/*--------------------------------------------------------------*/

static XtGeometryResult GeometryManager(w,desired,allowed)
Widget w;
XtWidgetGeometry *desired, *allowed;
{
   MakeGeometryRequest(w);
   return XtGeometryYes;
}

/*--------------------------------------------------------------*/
/* Inherit Changes                                              */
/*--------------------------------------------------------------*/

static Boolean CallInheritChanges(class, w, mask)
SGMLTextObjectClass class;
Widget      w;
int mask;    
{
    Boolean redisplay = FALSE;

    if ((WidgetClass) class != sGMLTextObjectClass)
        redisplay = CallInheritChanges(class->object_class.superclass, w, mask);
    if (class->sgml_text_class.inherit_changes != NULL)
        redisplay |= (*class->sgml_text_class.inherit_changes) (w, mask);

    return (redisplay);
}
 
static Boolean InheritChanges(w,inputMask)
SGMLCompositeTextObject w;
int inputMask;
{  
    Boolean redraw = FALSE;
    int newMask = w->sgml_text.inheritance & inputMask; 

    if (newMask) 
      {    
        int n = w->sgml_composite_text.num_children;
        WidgetList children = w->sgml_composite_text.children;
        Boolean *managed = w->sgml_composite_text.managed;

        for (; n-- > 0; children++, managed++)
          {
            SGMLTextObject t = (SGMLTextObject) *children; 
            SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children); 
            int mask = t->sgml_text.inheritance & newMask;

            if (mask) 
              {
                Boolean redrawChild = CallInheritChanges(childClass,t,mask);
                redraw |= (*managed && redrawChild);
              }
          }  
      } 
    return redraw;
}
/*------------------------------------------------------------------*/
/* SetValues : redraw only for font or color changes                */
/*------------------------------------------------------------------*/

static Boolean SGMLSetValues (current, request, new, args, num_args)
SGMLCompositeTextObject current, request, new;
ArgList args;
Cardinal *num_args;
{
    Boolean    redraw = FALSE;
    int        inherit = 0;
    SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) new);

    int n = new->sgml_composite_text.num_children;
    WidgetList children = new->sgml_composite_text.children;
    Boolean *managed = new->sgml_composite_text.managed;
 
    /*
     * Maybe our kids inherited some changed value from us?
     */
 
    inherit = (*class->sgml_text_class.compute_change_mask)(new,current);   

    /*
     * If so give them a chance to update the inherited values
     */
 
    for (; n-- > 0; children++, managed++)
      {
        SGMLTextObject t = (SGMLTextObject) *children; 
        SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children); 
        int mask = t->sgml_text.inheritance & inherit;

        if (mask)
          {
            Boolean redrawChild = CallInheritChanges(childClass,t,mask);
            redraw |= (*managed && redrawChild);
          }
      }  
    return redraw;
}





/*-----------------------------------------------------------------------*/
/* Create a new SGMLCompositeTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateCompositeText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLCompositeTextObjectClass,parent,al,ac);
}

/*-----------------------------------------------------------------------*/
/* Public routine to add a child                                         */
/*-----------------------------------------------------------------------*/

void SGMLCompositeTextInsertChild(child)
Widget child;
{ 
    Widget parent = XtParent(child);
    SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
    (*parentClass->sgml_composite_text_class.insert_child)(child);
}
 
/*-----------------------------------------------------------------------*/
/* Public routine to delete a child                                      */
/*-----------------------------------------------------------------------*/

void SGMLCompositeTextDeleteChild(child)
Widget child;
{ 
    Widget parent = XtParent(child);
    SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
    (*parentClass->sgml_composite_text_class.delete_child)(child);
}
/*-----------------------------------------------------------------------*/
/* Public routine to create a widget                                     */
/*-----------------------------------------------------------------------*/
 
Widget SGMLCreateWidget(name,class,parent,arglist,n)
char *name;
Widget parent;
WidgetClass class;
Arg arglist[];
int n;
{
  Widget w = XtCreateWidget(name,class,parent,arglist,n);  
  SGMLTextObjectClass cl = (SGMLTextObjectClass) class;
  
  (*cl->sgml_text_class.call_create_callback)(w,NULL);
  SGMLCompositeTextInsertChild(w);

  return w;
}  

/*-----------------------------------------------------------------------*/
/* Dump Text                                                             */
/*-----------------------------------------------------------------------*/
static Boolean DumpText(w,dp,pp)
SGMLCompositeTextObject w;
SGMLDumpTextInfo *dp;
char **pp;
{
  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;
  Boolean *managed = w->sgml_composite_text.managed;
  Dimension space_before = w->sgml_text.space_before;
  Dimension space_after = w->sgml_text.space_after;
  int break_before = (w->sgml_text.break_before == SGMLBREAK_ALWAYS);
  int break_after = (w->sgml_text.break_after == SGMLBREAK_ALWAYS);

  if (dp->begin_sw)
    {
      if (break_before)
        {
          int nl = (int)space_before;
          dp->line_spacing = 1;
          for (; nl > 0; nl-=10) dp->line_spacing++;
        }
    }
 
  for (; n-- > 0; children++, managed++)
    {
      if (*managed)
        {
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
          if ((*childClass->sgml_text_class.dump_text)(*children,dp,pp))
            return TRUE;
        }
    }

  if (dp->begin_sw)
    {
      if (break_after)
        {
          int nl = (int) space_after;
          dp->line_spacing = 1;
          for (; nl > 0; nl-=10) dp->line_spacing++;
        }
    }

  return FALSE;
}

/*-----------------------------------------------------------------------*/
/* Hilite Selection                                                      */
/*-----------------------------------------------------------------------*/
static Widget HiliteSelection(w,mode,ci)
SGMLCompositeTextObject w;
int mode;
SGMLCopyInfo *ci;
{
  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;
  Boolean *managed = w->sgml_composite_text.managed;
  Widget selection;

  for(; n-- > 0 ; children++, managed++)
    {
      if (*managed)
        {
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
          if (selection = (*childClass->sgml_text_class.hilite_selection)(*children,mode,ci))
            return selection;
        }
    }
  return NULL;
}

