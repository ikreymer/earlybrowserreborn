/*==================================================================*/
/*                                                                  */
/* SGMLMarkerTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a marker text segment for the SGMLHyper widget           */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <Xm/Xm.h>
#include "SGMLMarkerTextP.h"

/* 
  Private functions 
*/

static void SGMLDrawShadow();
#define BADIMAGE ((XImage *) -1)

/*
  Widget class methods
*/

static void    ComputeSize();
static void    Initialize();
static void    Destroy();
static Boolean SetValues();
static void    Redisplay();
static void    Hilite();  
static void    Activate();
static Boolean AdjustSize();
static void    CallMapCallback();

#define Offset(field) XtOffsetOf(SGMLMarkerTextRec,sgml_marker_text.field)

static XtResource resources[] = {

    {SGMLNcolumnAlign,SGMLCColumnAlign,XtRDimension,sizeof(Dimension),
    Offset (column_align),XtRImmediate,(XtPointer)0},

    {SGMLNposition,SGMLCPosition,XtRPosition,sizeof(Position),
    Offset (position),XtRImmediate,(XtPointer)-9999},

    {SGMLNpixmap,SGMLCPixmap,"Icon",sizeof(Pixmap),
    Offset (pixmap),XtRImmediate,(XtPointer)NULL},

    {SGMLNbombPixmap,SGMLCPixmap,"Icon",sizeof(Pixmap),
    Offset (bomb_pixmap),XtRImmediate,(XtPointer)NULL},
 
    {SGMLNimage,SGMLCImage,"Image",sizeof(GIFImage *),
    Offset (image),XtRImmediate,(XtPointer)NULL}, 

    {SGMLNpixmapDepth,XtCReadOnly,XtRInt,sizeof(int),
    Offset (pixmap_depth), XtRImmediate, (XtPointer) 0},

    {SGMLNdeleteImageWhenDone,SGMLCDeleteImageWhenDone,XtRBoolean,sizeof(Boolean),
    Offset (delete_image),XtRImmediate,(XtPointer)FALSE},
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLMarkerTextClassRec  sGMLMarkerTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLMarkerText",                    /* class_name            */
    sizeof(SGMLMarkerTextRec),           /* widget_size           */
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
    AdjustSize,                          /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
    Activate,                            /* activate              */
    Hilite,                              /* hilite                */
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
    NULL,                                /* ignore                */
    }
};


WidgetClass sGMLMarkerTextObjectClass = (WidgetClass) &sGMLMarkerTextClassRec;

/*--------------------------------------------------------------*/
/* SetupPixmap:                                                 */
/*--------------------------------------------------------------*/

static void SetupPixmap(w)
SGMLMarkerTextObject w;
{
  Pixmap pixmap = w->sgml_marker_text.pixmap;
  Window root;
  int x,y;
  Display *dpy = XtDisplayOfObject((Widget) w);

  XGetGeometry(dpy,pixmap,&root,&x,&y,&w->sgml_marker_text.pixmap_width,
                                      &w->sgml_marker_text.pixmap_height,
                                      &w->sgml_marker_text.pixmap_border_width,
                                      &w->sgml_marker_text.pixmap_depth);
                                      
  /*                                     
   * The pixmap must either have the same depth as the widgets window, or depth 1
   */

  if (w->sgml_marker_text.pixmap_depth != 1)
    {
      Widget parent = (Widget) w;

      for (; !XtIsWidget(parent) ; parent = XtParent(parent));
      if (parent->core.depth != w->sgml_marker_text.pixmap_depth)   
        {
          printf("Pixmap specified for SGMLHyperText has invalid depth\n");
          w->sgml_marker_text.pixmap = (Pixmap)NULL;
        }   
    }    
  w->sgml_marker_text.image = NULL;
}
/*--------------------------------------------------------------*/
/* SetupImage:                                                  */
/*--------------------------------------------------------------*/

static void SetupImage(w)
SGMLMarkerTextObject w;
{
  Widget parent = (Widget) w;
  GIFImage *gifImage = w->sgml_marker_text.image;
  Window root;
  int x,y;
  Display *dpy = XtDisplayOfObject((Widget) w);

  w->sgml_marker_text.pixmap_width  = gifImage->Width;
  w->sgml_marker_text.pixmap_height = gifImage->Height;
  w->sgml_marker_text.pixmap_border_width = 0;
  w->sgml_marker_text.ximage = (XImage *)NULL;
  w->sgml_marker_text.pixmap = (Pixmap)NULL;
}
/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new) 
SGMLMarkerTextObject request, new;
{
  Widget parent;
  XGCValues values;
  XtGCMask  valueMask;  
  Pixel top, bottom;
  Pixmap topPixmap, bottomPixmap;
  int n = 0;
  Arg arglist[10]; 

  for (parent = (Widget) new; !XtIsSubclass(parent,xmManagerWidgetClass); parent = XtParent(parent));   
   
  XtSetArg(arglist[n],XmNbottomShadowColor,&bottom); n++;
  XtSetArg(arglist[n],XmNtopShadowColor,&top); n++;
  XtSetArg(arglist[n],XmNbottomShadowPixmap,&bottomPixmap); n++;
  XtSetArg(arglist[n],XmNtopShadowPixmap,&topPixmap); n++;
  XtGetValues(parent,arglist,n);   

  valueMask = GCForeground;
  values.foreground = top;
  if (topPixmap != XmUNSPECIFIED_PIXMAP)
    {
      valueMask += GCTile + GCFillStyle;
      values.tile = topPixmap;
      values.fill_style = FillTiled;
    }
  new->sgml_marker_text.gc1 = XtGetGC((Widget) new, valueMask, &values); 
  
  valueMask = GCForeground;
  values.foreground = bottom; 
  if  (bottomPixmap != XmUNSPECIFIED_PIXMAP)
    {
      valueMask += GCTile + GCFillStyle;
      values.tile = bottomPixmap;
      values.fill_style = FillTiled;
    }
  new->sgml_marker_text.gc2 = XtGetGC((Widget) new, valueMask, &values);
  new->sgml_marker_text.bomb_gc = NULL; 

  if      (new->sgml_marker_text.pixmap) SetupPixmap(new);
  else if (new->sgml_marker_text.image)  SetupImage(new);
}
/*--------------------------------------------------------------*/
/* Destroy:                                                     */
/*--------------------------------------------------------------*/

static void Destroy(w) 
SGMLMarkerTextObject w;
{
  if (w->sgml_marker_text.bomb_gc) XtReleaseGC((Widget) w,w->sgml_marker_text.bomb_gc);
  if (w->sgml_marker_text.delete_image)
    {
      if      (w->sgml_marker_text.image ) GIFFreeFile(w->sgml_marker_text.image);
      else if (w->sgml_marker_text.pixmap) XFreePixmap(XtDisplayOfObject((Widget) w),w->sgml_marker_text.pixmap);
    } 
}
/*------------------------------------------------------------------*/
/* SetValues :                                                      */
/*------------------------------------------------------------------*/

static Boolean SetValues (current, request, new)
SGMLMarkerTextObject current, request, new;
{

#define HAS_CHANGED(a) (new->sgml_marker_text.a != current->sgml_marker_text.a)

  if      (HAS_CHANGED(pixmap)) SetupPixmap(new);
  else if (HAS_CHANGED(image )) SetupImage(new);
  
  if (current->sgml_marker_text.delete_image)
    {
      if      (HAS_CHANGED(image ) && current->sgml_marker_text.image ) GIFFreeFile(current->sgml_marker_text.image);
      else if (HAS_CHANGED(pixmap) && current->sgml_marker_text.pixmap) XFreePixmap(XtDisplayOfObject((Widget) new),current->sgml_marker_text.pixmap);
    } 
#undef HAS_CHANGED  
  
  return FALSE;
}
/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region,Offset)
SGMLMarkerTextObject t;
XEvent *event;
Region region;
SGMLPosition Offset;
{
   Pixmap pixmap = t->sgml_marker_text.pixmap;
   GIFImage *gifImage = t->sgml_marker_text.image;

   if (pixmap || gifImage) 
     {
       SGMLRendition  *rendition = &t->sgml_text.normal_rendition;
       Dimension outlineSize  = rendition->outline ? rendition->underline_height : 0;

       XRectangle rect;
       Region new = XCreateRegion();
       Dimension gap = t->sgml_text.begin.ascent - t->sgml_marker_text.pixmap_height - outlineSize; 
       Dimension gap2 = t->sgml_text.begin.descent - outlineSize; 
       
       rect.x = t->sgml_text.margin;
       rect.y = t->sgml_text.begin.y + gap;
       rect.width  = t->sgml_text.width;
       rect.height = t->sgml_text.height - gap - gap2;

       XUnionRectWithRegion(&rect,new,new);
       XIntersectRegion(new,region,new);
       if (!XEmptyRegion(new))
         { 
           Display *dpy = XtDisplayOfObject((Widget) t);
           Window  wind = XtWindowOfObject((Widget) t);
           GC gc =  t->sgml_text.gc;
           
           if (rendition->outline)
             {
               Region inside = XCreateRegion();
               GC gc1 =  t->sgml_marker_text.gc1;
               GC gc2 =  t->sgml_marker_text.gc2;
               XRectangle inner;    

               SGMLDrawShadow(dpy,wind,gc1,gc2,outlineSize,rect.x,rect.y,rect.width,rect.height);
 
               inner.x = rect.x + outlineSize; 
               inner.y = rect.y + outlineSize;
               inner.width = rect.width - 2*outlineSize;
               inner.height = rect.height - 2*outlineSize;  

               XUnionRectWithRegion(&inner,inside,inside);
               XIntersectRegion(inside,new,new);
               XDestroyRegion(inside);
             } 

           if (!XEmptyRegion(new)) 
             {
               XClipBox(new,&rect); 
     
               if (pixmap)
                 {  
                   handle_pixmap: /* Can jump here from image code below */
                   if (t->sgml_marker_text.pixmap_depth > 1)
                     { 
                       XCopyArea(dpy, pixmap, wind, gc,  rect.x - t->sgml_text.margin - outlineSize,
                                                         rect.y - t->sgml_text.begin.y - gap -outlineSize,
                                                         rect.width,rect.height,rect.x,rect.y); 
                     }
                   else
                     { 
                       XCopyPlane(dpy, pixmap, wind, gc, rect.x - t->sgml_text.margin - outlineSize,
                                                         rect.y - t->sgml_text.begin.y - gap -outlineSize,
                                                         rect.width,rect.height,rect.x,rect.y,1);
                     }
                 }
               else
                 {
                   XImage *image = t->sgml_marker_text.ximage;
                   if (!image)
                     {
                       char message[256];
                       Widget parent = (Widget) t; 

                       for (; !XtIsWidget(parent) ; parent = XtParent(parent));
                       t->sgml_marker_text.ximage = image = GIFToXImage(parent,gifImage,message);
                       if (!image) 
                         { 
                           printf("GIF conversion error: %s\n",message);  
                           image = BADIMAGE;
                         }
                       else
                         {
                           t->sgml_marker_text.pixmap = pixmap = GIFToPixmap(parent,gifImage,message);
                           t->sgml_marker_text.pixmap_depth = parent->core.depth; 
                           if (pixmap) goto handle_pixmap; 
                         } 
                     }
                   if (image == BADIMAGE)
                     {
                       GC gc = t->sgml_marker_text.bomb_gc;
                       if (!gc)
                         {
                           XGCValues values;
                           XtGCMask  valueMask = 0;
                           if (t->sgml_marker_text.bomb_pixmap)
                             {  
                               valueMask = GCTile + GCFillStyle;
                               values.tile = t->sgml_marker_text.bomb_pixmap;
                               values.fill_style = FillTiled;
                             }
                           gc = t->sgml_marker_text.bomb_gc = XtGetGC((Widget) t, valueMask, &values); 
                         } 
                       XFillRectangle(dpy, wind, gc, rect.x,rect.y,rect.width,rect.height);
                     }  
                   else
                     XPutImage(dpy, wind, gc, image, rect.x - t->sgml_text.margin - outlineSize,
                                                     rect.y - t->sgml_text.begin.y - gap - outlineSize,
                                                     rect.x,rect.y,rect.width,rect.height); 
                 }
             }  
         }
       XDestroyRegion(new);
     }
}
/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/
static void ComputeSize(w,geom,Adjust,Closure)
SGMLMarkerTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
  Dimension left_clearance  = w->sgml_text.left_margin  + w->sgml_text.left_indent;
  Dimension right_clearance = w->sgml_text.right_margin + w->sgml_text.right_indent;
  Pixmap pixmap = w->sgml_marker_text.pixmap;
  GIFImage *gifImage = w->sgml_marker_text.image;
   /*
    * Break?
    */  

  if (w->sgml_text.break_before == SGMLBREAK_ALWAYS)
  {
    _SGMLBreak(geom,w->sgml_text.space_before);
  } 
  else if (w->sgml_marker_text.column_align)
  {
    Dimension x = geom->coord.x;
    
    if (x < left_clearance) x = 0;
    else 
      {
        x -= left_clearance;
        if (geom->leave_space) x += 2 * w->sgml_text.spacing;
      }
 
    if (x%w->sgml_marker_text.column_align)
      x += w->sgml_marker_text.column_align - x%w->sgml_marker_text.column_align;
  
    x += left_clearance;
    geom->coord.x = x; 
    geom->leave_space = FALSE;

    if (x + w->sgml_marker_text.column_align + right_clearance > 
        geom->natural_width)
    {
      _SGMLBreak(geom,0);
    }             
  }
  if (w->sgml_marker_text.position != -9999)
  {
    if (geom->leave_space) geom->coord.x += 2 * w->sgml_text.spacing;
    if (geom->coord.x > w->sgml_text.left_margin+w->sgml_text.left_indent+w->sgml_marker_text.position)
      {
        _SGMLBreak(geom,0);
      }
    geom->coord.x = w->sgml_text.left_margin+w->sgml_text.left_indent+w->sgml_marker_text.position;
    geom->leave_space = FALSE;
    geom->broken = FALSE;
  }

  if (pixmap || gifImage)
    { 
      unsigned int width  = w->sgml_marker_text.pixmap_width;
      unsigned int ascent = w->sgml_marker_text.pixmap_height;
      unsigned int descent = 0;
      SGMLRendition  *rendition;

      rendition = &w->sgml_text.normal_rendition;

      if (rendition->outline)
        {
          width   += 2*rendition->underline_height;
          ascent  += rendition->underline_height;
          descent += rendition->underline_height; 
        } 

      if (geom->coord.x + w->sgml_text.spacing + width + right_clearance > geom->natural_width)
        {
          geom->coord.x = left_clearance;
          geom->coord.y += geom->coord.descent + geom->coord.ascent;
          geom->coord.descent = w->sgml_text.descent;
          geom->coord.ascent = w->sgml_text.ascent;
       }
      else if (geom->leave_space) geom->coord.x += w->sgml_text.spacing;   

      if (ascent > geom->coord.ascent || descent > geom->coord.descent)
        {
          geom->coord.ascent  = ascent;
          geom->coord.descent = descent;
 
          Adjust(Closure, geom->coord.y, ascent, descent);
        }

      if (left_clearance > geom->coord.x && geom->broken) geom->coord.x = left_clearance;

      w->sgml_text.begin  = geom->coord;
      w->sgml_text.margin = geom->coord.x;

      geom->coord.x += width;
      geom->leave_space = TRUE;
      geom->broken = FALSE;

      if (geom->actual_width < left_clearance + width) 
         geom->actual_width = left_clearance + width;

      w->sgml_text.height = geom->coord.ascent + geom->coord.descent;   
      w->sgml_text.width  = width;
    }
  else
    {
      w->sgml_text.height = 0;   
      w->sgml_text.width  = 0;
    }
     
  w->sgml_text.margin = geom->coord.x - w->sgml_text.width; 
  w->sgml_text.end = geom->coord;

  if (w->sgml_text.break_after == SGMLBREAK_ALWAYS)
  {
    _SGMLBreak(geom,w->sgml_text.space_after);
  } 
  w->sgml_text.size_valid = TRUE; 
}

/*--------------------------------------------------------------*/
/* Hilite:                                                      */
/*--------------------------------------------------------------*/

static void Hilite(t,state)
SGMLMarkerTextObject t;
int state;
{
  Display *dpy = XtDisplayOfObject((Widget) t);
  Window  wind = XtWindowOfObject((Widget) t);
  SGMLRendition  *rendition = &t->sgml_text.normal_rendition;
  Dimension outlineSize = rendition->outline ? rendition->underline_height : 0;
  Dimension gap = t->sgml_text.begin.ascent - t->sgml_marker_text.pixmap_height - outlineSize; 
  Dimension gap2 = t->sgml_text.begin.descent - outlineSize; 

  Position x = t->sgml_text.begin.x; 
  Position y = t->sgml_text.begin.y + gap; 
  Dimension width  = t->sgml_text.width;
  Dimension height = t->sgml_text.height - gap - gap2;
   
  if (state != SGMLSELECT_WIDGET) return;    /* do not anything except hilite widget */

  if (t->sgml_marker_text.pixmap && t->sgml_marker_text.pixmap_depth == 1) 
    {
      GC gc = t->sgml_text.invert_gc;
      XFillRectangle(dpy,wind,gc,x,y,width,height);
    }
  else if (rendition->outline)
    {
      GC gc1 =  t->sgml_marker_text.gc1;
      GC gc2 =  t->sgml_marker_text.gc2;

      SGMLDrawShadow(dpy,wind,state?gc2:gc1,state?gc1:gc2,outlineSize,x,y,width,height); 
    }
}
/*--------------------------------------------------------------*/
/* Activate:                                                    */
/*--------------------------------------------------------------*/

static void Activate(w,event)
SGMLMarkerTextObject w;
XEvent *event;
{
    SGMLMarkerTextCallbackStruct cb;
    Position x,y;
    
    if (event->type == ButtonPress || event->type == ButtonRelease)
      {
        XButtonEvent *be = (XButtonEvent *) event;
        SGMLRendition *rendition = &w->sgml_text.normal_rendition;

        x = be->x - w->sgml_text.margin;
        y = be->y - w->sgml_text.begin.y;  

        if (rendition->outline)
          {
            x -= rendition->underline_height;
            y -= rendition->underline_height;
          }
      }
    else x = y = 0;

    /* Fill callback struct */

    cb.text          = w->sgml_text.text;
    cb.length        = w->sgml_text.length;
    cb.param         = w->sgml_text.param;
    cb.param_length  = w->sgml_text.param_length;
    cb.reason        = SGMLTEXT_ACTIVATE;
    cb.event         = event;
    cb.x             = x;
    cb.y             = y;

    XtCallCallbacks ((Widget) w, SGMLNactivateCallback,(XtPointer) &cb);
}

/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,descent)
SGMLMarkerTextObject w;
SGMLPosition y;
SGMLDimension ascent, descent;
{
   if (w->sgml_text.end.y != y) return FALSE;
     
   w->sgml_text.height = descent + ascent; 
   w->sgml_text.end.ascent = ascent;
   w->sgml_text.end.descent = descent;
   w->sgml_text.begin.ascent = ascent;
   w->sgml_text.begin.descent = descent;
   return TRUE;
}   

/*--------------------------------------------------------------*/
/* Call the map Callback:                                       */
/*--------------------------------------------------------------*/

static void CallMapCallback(w,event,map)
SGMLMarkerTextObject w;
XEvent         *event;
Boolean        map;
{
    SGMLTextCallbackStruct cb;

    if (!map && w->sgml_marker_text.image)
      { 
        Widget parent = (Widget) w;
 
        for (; !XtIsWidget(parent) ; parent = XtParent(parent));   
        GIFFreeImage(parent,w->sgml_marker_text.image);
        w->sgml_marker_text.ximage = NULL; 
        w->sgml_marker_text.pixmap = NULL; 
      }
    cb.reason = SGMLTEXT_MAP;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;
  
    if (map) XtCallCallbacks((Widget) w,SGMLNmapCallback  ,(XtPointer) &cb);
    else     XtCallCallbacks((Widget) w,SGMLNunmapCallback,(XtPointer) &cb);
} 
/************************************************************************
 *
 *	Draw an n segment wide bordering shadow on the drawable
 *	d, using the provided GC's and rectangle.
 *
 ************************************************************************/

static void SGMLDrawShadow (display, d, top_GC, bottom_GC, size, x, y, width, height)
Display * display;
Drawable d;
GC top_GC;
GC bottom_GC;
register int size;
register int x;
register int y;
register int width;
register int height;
{
   static XRectangle * rects = NULL;
   static int rect_count = 0;
   register int i;
   register int size2;
   register int size3;

   if (size <= 0) return;
   if (size > width / 2) size = width / 2;
   if (size > height / 2) size = height / 2;
   if (size <= 0) return;

   if (rect_count == 0)
   {
      rects = (XRectangle *) XtMalloc (sizeof (XRectangle) * size * 4);
      rect_count = size;
   }

   if (rect_count < size)
   {
      rects = (XRectangle *) XtRealloc((char *) rects, sizeof (XRectangle) * size * 4);
      rect_count = size;
   }

   size2 = size + size;
   size3 = size2 + size;

   for (i = 0; i < size; i++)
   {
      /*  Top segments  */

      rects[i].x = x;
      rects[i].y = y + i;
      rects[i].width = width - i;
      rects[i].height = 1;


      /*  Left segments  */

      rects[i + size].x = x + i;
      rects[i + size].y = y;
      rects[i + size].width = 1;
      rects[i + size].height = height - i;


      /*  Bottom segments  */

      rects[i + size2].x = x + i + 1;
      rects[i + size2].y = y + height - i - 1;
      rects[i + size2].width = width - i - 1;
      rects[i + size2].height = 1;


      /*  Right segments  */

      rects[i + size3].x = x + width - i - 1;
      rects[i + size3].y = y + i + 1;
      rects[i + size3].width = 1;
      rects[i + size3].height = height - i - 1;
   }

   XFillRectangles (display, d, top_GC, &rects[0], size2);
   XFillRectangles (display, d, bottom_GC, &rects[size2], size2);
}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLMarkerTextObject                                      */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateMarkerText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLMarkerTextObjectClass,parent,al,ac);
}
