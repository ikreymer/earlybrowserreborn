/*==================================================================*/
/*                                                                  */
/* SGMLListTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a list text segment for the SGMLHyper widget             */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "SGMLListTextP.h"
#include "SGMLMarkerText.h"

/* 
  Private functions 
*/


/*
  Widget class methods
*/

static void         	  ClassInitialize();
static void    		  Initialize();
static void    		  Destroy();
static void               ComputeSize();
static void               Redisplay();
static void		  CallAdjustSize();

#define Offset(field) XtOffsetOf(SGMLListTextRec,sgml_list_text.field)

static XtResource resources[] = {

    {SGMLNnumBullets, XtCReadOnly, XtRInt, sizeof(int),
     Offset(num_bullets), XtRImmediate, (XtPointer) 0},
    {SGMLNallocBullets, XtCReadOnly, XtRInt, sizeof(int),
     Offset(alloc_bullets), XtRImmediate, (XtPointer) 0},

    {SGMLNbulletType, SGMLCBulletType, SGMLRBulletType, sizeof(int),
     Offset(bullet_type), XtRImmediate, (XtPointer) SGMLBULLET_DEFAULT},
    {SGMLNbulletSize, SGMLCBulletSize, XtRDimension, sizeof(Dimension),
     Offset(bullet_size), XtRImmediate, (XtPointer) 11},
    {SGMLNbulletIndent, SGMLCBulletIndent, XtRDimension, sizeof(Dimension),
     Offset(bullet_indent), XtRImmediate, (XtPointer) 6},
    {SGMLNbulletMarker, SGMLCBulletMarker, SGMLRQuark, sizeof(XrmQuark),
     Offset(bullet_marker), XtRImmediate, (XtPointer) NULL},
    {SGMLNbulletColor, SGMLCBulletColor, XtRPixel, sizeof(Pixel),
     Offset(bullet_color), XtRImmediate, (XtPointer) 0}
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLListTextClassRec  sGMLListTextClassRec = {
    {
    (WidgetClass) &sGMLCompositeTextClassRec,
                                         /* superclass            */
    "SGMLListText",                      /* class_name            */
    sizeof(SGMLListTextRec),             /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
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
    XtInheritGeometryManager,            /* geometry_manager      */
    XtInheritChangeManaged,              /* change_managed        */
    XtInheritInsertChild,                /* insert_child          */
    XtInheritDeleteChild,                /* delete_child          */
    CallAdjustSize,                      /* call_adjust_size      */
    NULL,                                /* ignore                */
    },
    {
    NULL                                 /* ignore                */
    }
};

WidgetClass sGMLListTextObjectClass = (WidgetClass) &sGMLListTextClassRec;

#define done(address, type) \
        { (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

static struct _namepair {
    XrmQuark quark;
    char *name;
    int bullet;
} names[] = {
    { NULLQUARK, "ordered"    ,SGMLBULLET_ORDERED},
    { NULLQUARK, "unordered"  ,SGMLBULLET_UNORDERED},
    { NULLQUARK, "default"    ,SGMLBULLET_DEFAULT},
    { NULLQUARK, "bullet"     ,SGMLBULLET_BULLET},
    { NULLQUARK, "diamond"    ,SGMLBULLET_DIAMOND},
    { NULLQUARK, "square"     ,SGMLBULLET_SQUARE},
    { NULLQUARK, "numeric"    ,SGMLBULLET_NUMERIC},
    { NULLQUARK, "lower"      ,SGMLBULLET_LOWER},
    { NULLQUARK, "upper"      ,SGMLBULLET_UPPER},
    { NULLQUARK, "arrow"      ,SGMLBULLET_ARROW},
    { NULLQUARK, "linkedarrow",SGMLBULLET_LINKEDARROW},
    { NULLQUARK, NULL         ,SGMLBULLET_DEFAULT}
};

void CvtStringToBullet (args, num_args, fromVal, toVal)
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
        XtWarningMsg("wrongParameters","cvtStringToBullet","XtToolkitError",
                  "String to Bullet conversion needs no extra arguments",
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
		done (&np->bullet,int);
		return;
	    }
	}
    }
    XtStringConversionWarning((char *) fromVal->addr, SGMLRBulletType);
}
/*--------------------------------------------------------------*/
/* ClassInitialize:                                             */
/*--------------------------------------------------------------*/

static void ClassInitialize()
{
    XtAddConverter (XtRString, SGMLRBulletType, CvtStringToBullet,
		        (XtConvertArgList) NULL, (Cardinal) 0);
}

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLListTextObject request, new;
{
   Widget parent;
   int newtype = new->sgml_list_text.bullet_type ;

   new->sgml_list_text.bullets = NULL;
   new->sgml_list_text.num_bullets = 0;
   new->sgml_list_text.alloc_bullets = 0; 

   if (newtype == SGMLBULLET_DEFAULT   ||
       newtype == SGMLBULLET_UNORDERED ||
       newtype == SGMLBULLET_ORDERED   )
     {
       for (parent = XtParent((Widget) new); SGMLIsText(parent); parent = XtParent(parent))
         if (SGMLIsListText(parent))
           {
             SGMLListTextObject t = (SGMLListTextObject) parent;
             int type =  t->sgml_list_text.bullet_type ;

             if (newtype == SGMLBULLET_DEFAULT)
               {
                 if      (type == SGMLBULLET_BULLET  ) newtype = SGMLBULLET_DIAMOND;
                 else if (type == SGMLBULLET_DIAMOND ) newtype = SGMLBULLET_SQUARE;
                 else if (type == SGMLBULLET_SQUARE  ) newtype = SGMLBULLET_BULLET;
                 else if (type == SGMLBULLET_NUMERIC ) newtype = SGMLBULLET_UPPER;
                 else if (type == SGMLBULLET_UPPER   ) newtype = SGMLBULLET_LOWER;
                 else if (type == SGMLBULLET_LOWER   ) newtype = SGMLBULLET_NUMERIC;
                 else                                  newtype = type;
               }
             else if (newtype == SGMLBULLET_UNORDERED)
               {
                 if      (type == SGMLBULLET_BULLET  ) newtype = SGMLBULLET_DIAMOND;
                 else if (type == SGMLBULLET_DIAMOND ) newtype = SGMLBULLET_SQUARE;
                 else if (type == SGMLBULLET_SQUARE  ) newtype = SGMLBULLET_BULLET;
                 else                                  newtype = SGMLBULLET_BULLET;
               }
             else
               {
                 if      (type == SGMLBULLET_NUMERIC ) newtype = SGMLBULLET_UPPER;
                 else if (type == SGMLBULLET_UPPER   ) newtype = SGMLBULLET_LOWER;
                 else if (type == SGMLBULLET_LOWER   ) newtype = SGMLBULLET_NUMERIC;
                 else                                  newtype = SGMLBULLET_NUMERIC;
                }
             break;
           }
      if      (newtype == SGMLBULLET_DEFAULT  ) newtype = SGMLBULLET_BULLET;
      else if (newtype == SGMLBULLET_UNORDERED) newtype = SGMLBULLET_BULLET;
      else if (newtype == SGMLBULLET_ORDERED  ) newtype = SGMLBULLET_NUMERIC;

      new->sgml_list_text.bullet_type = newtype; 
   }
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLListTextObject w;
{
    XtFree((char *) w->sgml_list_text.bullets);
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region,Offset)
SGMLListTextObject t;
XEvent *event;
Region region;
SGMLPosition Offset;
{
 Display *dpy = XtDisplayOfObject((Widget) t);
 Window  wind = XtWindowOfObject((Widget) t);
 GC gc =  t->sgml_text.gc;
 int i;
 Dimension width = t->sgml_list_text.bullet_size;
 Dimension height = t->sgml_list_text.bullet_size;
  
 SGMLTextObjectClass superclass = (SGMLTextObjectClass) XtSuperclass((Widget) t);

/*
 *   Invoke our superclasses ReDisplay routine ... then draw bullets
 */ 
  
 (*superclass->sgml_text_class.expose)(t,event,region,Offset); 
  
 if (t->sgml_list_text.bullet_type == SGMLBULLET_LINKEDARROW)
   {
     Position x  = t->sgml_list_text.bullets[0].x - 2*width;
     Position y1 = t->sgml_list_text.bullets[0].top; 
     Position y2 = t->sgml_list_text.bullets[t->sgml_list_text.num_bullets - 1].top + height/2;
     if (XRectInRegion(region,x,y1,1,y2 - y1)) XDrawLine(dpy,wind,gc,x,y1,x,y2);
   }   

 for (i = 0; i < t->sgml_list_text.num_bullets; i++ )
   {
     Position x = t->sgml_list_text.bullets[i].x - width;
     SGMLPosition Y = t->sgml_list_text.bullets[i].top;
     Position y = Y - Offset;
     if (y != Y) break;
     
     if(XRectInRegion(region,x,y,width,height) != RectangleOut)
       switch (t->sgml_list_text.bullet_type)
         {
           default:
           case SGMLBULLET_BULLET:
             XFillArc(dpy,wind,gc,x,y,width,height,0,360*64);
             break;

           case SGMLBULLET_DIAMOND:
             { 
               XPoint points[4];
               points[0].x = x;
               points[0].y = y + height/2;
               points[1].x = x + width/2;
               points[1].y = y;
               points[2].x = x + width;
               points[2].y = y + height/2;
               points[3].x = x + width/2;
               points[3].y = y + height;
               XFillPolygon(dpy,wind,gc,points,XtNumber(points),Convex,CoordModeOrigin); 
             }
             break;  
 
           case SGMLBULLET_SQUARE:
             XFillRectangle(dpy,wind,gc,x,y,width,height); 
             break;

           case SGMLBULLET_ARROW:
           case SGMLBULLET_LINKEDARROW:
             {
               XPoint points[3];

               points[0].x = x + width/2;
               points[0].y = y;
               points[1].x = x + width;
               points[1].y = y + height/2;
               points[2].x = x + width/2;
               points[2].y = y + height;

               XFillPolygon(dpy,wind,gc,points,XtNumber(points),Convex,CoordModeOrigin); 
               XDrawLine(dpy,wind,gc,x - width,y + height/2,x + width/2,y + height/2);

             }
             break;  
           
           case SGMLBULLET_NUMERIC:
           case SGMLBULLET_UPPER:
           case SGMLBULLET_LOWER:

             {
               char text[8];
               char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
               char *lower = "abcdefghijklmnopqrstuvwxyz";
               int l;
               Dimension size;
 
               if      (t->sgml_list_text.bullet_type == SGMLBULLET_NUMERIC) { sprintf(text,"%d",i+1); l = strlen(text); }
               else if (t->sgml_list_text.bullet_type == SGMLBULLET_UPPER  ) { *text = upper[i%26]; l = 1; }
               else                                                          { *text = lower[i%26]; l = 1; }

               text[l++] = '.'; 
               size = XTextWidth(t->sgml_text.font,text,l);
               XDrawString(dpy,wind,gc,x+width-size,y+height,text,l);
             }
             break;
         }
   } 
}

/*------------------------------------------------------------------*/
/* Adjust the size of a child                                       */
/*------------------------------------------------------------------*/

static void CallAdjustSize(child,y,ascent,descent)
Widget child;
Position y;
Dimension ascent, descent;
{
   SGMLListTextObject w = (SGMLListTextObject) XtParent(child);
   SGMLCompositeTextObjectClass superclass = (SGMLCompositeTextObjectClass) XtSuperclass((Widget) w);
   int i;
 
   /*----------------------------------------------------------------*/
   /* If there is a bullet on this line it needs to be adjusted too  */                                            
   /*----------------------------------------------------------------*/
 
   for (i = 0; i < w->sgml_list_text.num_bullets; i++ )
     {
       if (y == w->sgml_list_text.bullets[i].y)
         w->sgml_list_text.bullets[i].top = y + ascent - w->sgml_list_text.bullet_size;  
     } 

   /*----------------------------------------------------------------*/
   /* Then invoke superclasses CallAdjustSize method                 */                                            
   /*----------------------------------------------------------------*/
 
   (*superclass->sgml_composite_text_class.call_adjust_size)(child,y,ascent,descent); 
}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/
 
static void ComputeSize(w,geom,Adjust,Closure)
SGMLListTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   int n = w->sgml_composite_text.num_children;
   int nb = 0;     
   Boolean bullet;
   WidgetList children = w->sgml_composite_text.children;
   Boolean *managed = w->sgml_composite_text.managed;
   Dimension maxWidth;
   Position x  = w->sgml_text.left_margin + w->sgml_text.left_indent - w->sgml_list_text.bullet_indent;
   Position xmin, xmax;

   XtFree((char *) w->sgml_list_text.bullets);
   w->sgml_list_text.bullets = NULL;
   w->sgml_list_text.num_bullets = 0;
   w->sgml_list_text.alloc_bullets = 0;
   
   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before )
   {
     _SGMLBreak(geom,w->sgml_text.space_before);
   } 

   xmin = xmax = geom->coord.x; 
   /*
    * Query children?
    */  

   for (bullet = TRUE; n-- > 0; children++, managed++)
     {
       if (bullet)
         {
           if ( nb == 0 ||  
               w->sgml_list_text.bullets[nb-1].x != x ||
               w->sgml_list_text.bullets[nb-1].y != geom->coord.y) 
             {
               if (nb == w->sgml_list_text.alloc_bullets)
                 { 
                    w->sgml_list_text.alloc_bullets += 10;
                    w->sgml_list_text.bullets = (BulletList) XtRealloc((char *)w->sgml_list_text.bullets,
                                 w->sgml_list_text.alloc_bullets * sizeof(BulletListItem));
                 }
               w->sgml_list_text.bullets[nb].x = x;  
               w->sgml_list_text.bullets[nb].y = geom->coord.y;  
               w->sgml_list_text.bullets[nb].top = geom->coord.y; 
               w->sgml_list_text.num_bullets = ++nb;   

               geom->coord.descent = 0;
               geom->coord.ascent = w->sgml_list_text.bullet_size;
             }
           bullet = FALSE;
         }
       if (*managed)
         {     
           SGMLTextObject t = (SGMLTextObject) *children; 

           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           (*childClass->sgml_text_class.compute_size)(*children,geom,CallAdjustSize,*children);

           bullet = SGMLIsMarkerText(*children) && 
                     (*children)->core.xrm_name == w->sgml_list_text.bullet_marker;

           if (t->sgml_text.margin < xmin) xmin = t->sgml_text.margin;
           if (t->sgml_text.margin + t->sgml_text.width > xmax)
             xmax = t->sgml_text.margin + t->sgml_text.width; 
         }
     }
   /*
    * Break after?
    */  

   w->sgml_text.end = geom->coord;
   w->sgml_text.height = geom->coord.y + geom->coord.descent + geom->coord.ascent - w->sgml_text.begin.y;
   w->sgml_text.width  = xmax - xmin;
   w->sgml_text.margin = xmin; 
 
   if ( w->sgml_text.break_after )
   {
     _SGMLBreak(geom,w->sgml_text.space_after);
   } 
   w->sgml_text.size_valid = TRUE; 
}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLListTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateListText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLListTextObjectClass,parent,al,ac);
}
