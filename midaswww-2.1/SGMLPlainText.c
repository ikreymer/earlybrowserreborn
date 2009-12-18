/*==================================================================*/
/*                                                                  */
/* SGMLPlainTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a plain text segment for the SGMLHyper widget            */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLPlainTextP.h"

Widget ExtHiliteSelection();
/* 
  Private functions 
*/

static int LineSize();
static void DrawByParts();

/*
  Widget class methods
*/


static void    Initialize();
static void    Destroy();
static void    ComputeSize();
static void    Redisplay();
static Boolean AdjustSize();
static Widget  HiliteSelection(); 
static void    XYToPos();
static void    PosToXY();
static Boolean DumpText();

#define MINIMUM(a,b) ((a)<(b)?(a):(b))
#define MAXIMUM(a,b) ((a)>(b)?(a):(b))

#define Offset(field) XtOffsetOf(SGMLPlainTextRec,sgml_plain_text.field)

static XtResource resources[] = {

    {SGMLNtabs,SGMLCTabs,XtRInt,sizeof(int),
    Offset (tabs),XtRImmediate,(XtPointer)0},

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLPlainTextClassRec  sGMLPlainTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLPlainText",                     /* class_name            */
    sizeof(SGMLPlainTextRec),            /* widget_size           */
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
    HiliteSelection,                     /* hilite_selection      */
    XYToPos,                             /* xy_to_pos             */
    PosToXY,                             /* pos_to_xy             */
    DumpText,                            /* dump_text             */
    NULL,                                /* extension             */
    },
    {
    NULL,                                /* ignore                */
    }
};


WidgetClass sGMLPlainTextObjectClass = (WidgetClass) &sGMLPlainTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLTextObject request, new;
{
   /*
    * check that the font REALLY is a fixed font
    */

   short min = new->sgml_text.font->min_bounds.width;
   short max = new->sgml_text.font->max_bounds.width;

   if (min != max)
     { 
       Arg arglist[10];
       int n = 0;  
       XrmQuark c = XrmPermStringToQuark("m");

#ifdef DEBUG
       printf("Inconsistent fonts specified for a SGMLPlainText object\n");
       printf("Using 'fixed' font instead\n"); 
#endif       

       XtSetArg(arglist[n],SGMLNfontSpacing,c); n++;
       XtSetValues((Widget)new,arglist,n);
     }

}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLTextObject w;
{
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region,Offset)
SGMLPlainTextObject t;
XEvent *event;
Region region;
{
   SGMLPosition Y = t->sgml_text.begin.y;
   Position y = Y;
   Position margin = t->sgml_text.margin;
   Dimension width  = MINIMUM(t->sgml_text.width,32767-margin);
   Dimension height = MINIMUM(t->sgml_text.height,32767-y);

   if (y != Y) return;
  
   if(XRectInRegion(region,margin,y,width,height) != RectangleOut)
   {
      Display *dpy = XtDisplayOfObject((Widget) t);
      Window  wind = XtWindowOfObject((Widget) t);
      GC gc =  t->sgml_text.gc;
      int tabs = t->sgml_plain_text.tabs;
      XSegment *segs = NULL;

      Position size;
      Position x = t->sgml_text.begin.x;
      Dimension ascent = t->sgml_text.begin.ascent;
      Dimension depth = t->sgml_text.begin.descent + ascent;
      char *p = t->sgml_text.text;
      char *line = p;
      SGMLRendition *rendition = &t->sgml_text.normal_rendition; 
      char *b = t->sgml_text.begin_select;
      char *e = t->sgml_text.end_select;
      int line_select = FALSE;
      char *bp, *ep, *eol;
      int a_x = 0, a_y = 0;
      Boolean line_select_flag = FALSE;
      Boolean f16 = FALSE;

      if (t->sgml_text.font16_name != XrmStringToQuark("") &&
          t->sgml_text.font16_name != NULL)
        f16 = TRUE;

      for (;; p++)
        {
          if ( *p == '\n' && b != NULL && e != NULL )
            {
              if (p >= b ) 
                { line_select = TRUE; line_select_flag = TRUE; }
              if (p >= e && line_select_flag)
                line_select_flag = FALSE; 
              if (line > e) 
                 line_select = FALSE;
            }

          /* Fill additional rectangle when there is a newline   */
          /* located in between the begin_select and end_select. */
          if (*p == '\n' && p == line && line_select && line_select_flag &&
               XRectInRegion(region,x,y,width,depth) != RectangleOut )
             XFillRectangle(dpy,wind,gc,x,y,t->sgml_text.width,depth);  

          if (*p == '\n' || *p == '\0') 
            { 
               if (p != line && XRectInRegion(region,x,y,width,height) != RectangleOut)
                 {
                   if (*p == '\0')
                     { 
                       ascent = t->sgml_text.end.ascent;
                       depth  = t->sgml_text.end.descent + ascent;
                     }                   
                   if (tabs)
                     {
                       int nt = 0, pos=0;
                       char *q, *copy;

                       for (q = line; q < p; q++) if (*q == '\t') nt++;
        
                       copy = XtMalloc(p-line + nt * tabs);
                       bp = ep = eol = copy;
                     
                       for (q = line; q < p; q++,pos++)
                         {
                           if (line_select) 
                             if (q == b) bp += pos;

                           if (*q != '\t') 
                             copy[pos] = *q;
                           else 
                             for ( copy[pos] = ' '; pos%tabs ; ) 
                                copy[++pos] = ' ';

                           if (line_select)
                             if (q == e) ep += pos;
                         }
                       if (line_select && ep == copy) ep += (pos - 1);
                       eol += (pos - 1);
   
                       if (f16)
                         size = ExtTextWidth((Widget)t,&copy,pos,FALSE);
                       else
                         size = pos * t->sgml_text.spacing;                         

                       if (line_select && b != NULL) {

                         DrawByParts(copy,bp,ep,eol,t,x,y+ascent);
                         if (f16)
                           a_x = x + ExtTextWidth((Widget)t,&copy,ep + 1 - copy,FALSE);
                         else
                           a_x = x + (ep + 1 - copy) * t->sgml_text.spacing;
                         a_y = y;

                        /* fill addition rectangle when > 2 line_select */
                        /* useful for the hilite (ie: clear it)         */
                        if (line_select_flag) 
                          {
                            unsigned int a_width = t->sgml_text.width + margin - a_x;
                            if (a_width > 0)
                              XFillRectangle(dpy,wind,gc,a_x,a_y,a_width,(unsigned int)depth);
                          }

                         }
                       else
                         {
                           if (t->sgml_text.font16_name == XrmStringToQuark(""))
                             XDrawString(dpy, wind,gc, x, y+ascent,copy,pos); 
                           else
                             ExtDrawString((Widget)t,FALSE,dpy,wind,gc,x,y+ascent,copy,pos);
                         }
                       XtFree(copy); 
                     }
                   else 
                     {
                       if (f16)
                         size = ExtTextWidth((Widget)t,&line,p-line,FALSE);
                       else
                         size = (p-line) * t->sgml_text.spacing;  
                       if (line_select && b != NULL) {
                         if (line > b) bp = line;
                         else bp = b;
                         if (e > p) ep = p;
                         else ep = e;

                         DrawByParts(line,bp,ep,p,t,x,y+ascent);
                         if (f16)
                           a_x = x + ExtTextWidth((Widget)t,&line,ep+1-line,FALSE);
                         else
                           a_x = x + (ep +1 - line)*t->sgml_text.spacing;
                         a_y = y;

                         /* fill addition rectangle when > 2 line_select */
                         /* useful for the hilite (ie: clear it)         */
                         if (line_select_flag) 
                           {
                             unsigned int a_width = t->sgml_text.width + margin - a_x;
                             if (a_width > 0)
                               XFillRectangle(dpy,wind,gc,a_x,a_y,a_width,(unsigned int)depth);
                           }

                         }
                       else
                         {
                           if (t->sgml_text.font16_name == XrmStringToQuark(""))
                             XDrawString(dpy, wind,gc, x, y+ascent, line, p-line); 
                           else
                             ExtDrawString((Widget)t,FALSE,dpy,wind,gc,x,y+ascent,line,p-line);
                         }
                     }

                   if (rendition->underline && size)
                     {
                       int s;
                       Position yy = y+ascent; 
    
                       if (!segs) segs = (XSegment *) XtMalloc(sizeof(XSegment) * rendition->underline);

                       for (s=0; s < rendition->underline ; s++ )
                         {
                           yy += MAXIMUM(rendition->underline_height,1); 
                           segs[s].x1 = x;  
                           segs[s].x2 = size;
                           segs[s].y1 = yy;  
                           segs[s].y2 = yy;  
                           yy += MAXIMUM(rendition->underline_height,1); 
                         }
                       XDrawSegments(dpy,wind,gc,segs,rendition->underline);
                     }  
                 }
               if (*p == '\0') break;

               x = margin;  
               y += depth;
               Y += depth;
               if (y != Y) break;

               line = p+1;

               ascent = t->sgml_text.ascent;
               depth  = t->sgml_text.descent + ascent;
            }
        }
   }
}
/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,descent)
SGMLPlainTextObject w;
SGMLPosition y;
SGMLDimension ascent, descent;
{
   if (w->sgml_text.end.y != y) return FALSE;
     
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
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   Dimension left_clearance  = w->sgml_text.left_margin;
   int nlines;
   SGMLPosition xmin, xmax;

   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before == SGMLBREAK_ALWAYS)
   {
     _SGMLBreak(geom,w->sgml_text.space_before);
   } 

   if (w->sgml_text.descent  > geom->coord.descent  ||
       w->sgml_text.ascent > geom->coord.ascent )
     {  
       if (w->sgml_text.descent  > geom->coord.descent)  geom->coord.descent  =  w->sgml_text.descent;
       if (w->sgml_text.ascent > geom->coord.ascent) geom->coord.ascent =  w->sgml_text.ascent;
       
       Adjust(Closure, geom->coord.y, geom->coord.ascent, geom->coord.descent);
     }  
   if (left_clearance > geom->coord.x    ) geom->coord.x = left_clearance;

   w->sgml_text.begin  = geom->coord;

   nlines = LineSize(w,&geom->coord.x,&geom->actual_width,&xmin,&xmax);

   w->sgml_text.height = geom->coord.ascent + geom->coord.descent;
   
   if (nlines>1) 
     {
       geom->coord.ascent =  w->sgml_text.ascent;  
       geom->coord.descent =  w->sgml_text.descent;  
       w->sgml_text.height += (w->sgml_text.descent + w->sgml_text.ascent) * (nlines - 1); 
       geom->coord.y += w->sgml_text.height - w->sgml_text.descent - w->sgml_text.ascent;
     }

   w->sgml_text.margin = xmin;
   w->sgml_text.width = xmax - xmin;
   
   geom->space = 0;
   geom->broken = FALSE;
   geom->leave_space = FALSE; 

   w->sgml_text.end = geom->coord;

   /*
    * Break after?
    */  

   if ( w->sgml_text.break_after  == SGMLBREAK_ALWAYS)
   {
     _SGMLBreak(geom,w->sgml_text.space_after);
   } 
   w->sgml_text.size_valid = TRUE; 
}

/*-----------------------------------------------------------------------*/
/* Private routines                                                      */
/*-----------------------------------------------------------------------*/
/* DrawByParts: Draw each part of a line in different gc                 */
/*              Use to Redisplay when a search string is found           */
/*-----------------------------------------------------------------------*/
static void DrawByParts(copy,b,e,l,t,x,y)
char *copy;
char *b, *e, *l;
SGMLPlainTextObject t;
Position x;
Position y;
{
  Display *dpy = XtDisplayOfObject((Widget) t);
  Window  wind = XtWindowOfObject((Widget) t);
  GC gc =  t->sgml_text.gc;
  GC rv_gc = t->sgml_text.reverse_gc;
  int spacing = t->sgml_text.spacing;
  Position ix = x;
  int np, size = 0;
  Boolean f16 = FALSE;
  
  if (t->sgml_text.font16_name != XrmStringToQuark("") &&
      t->sgml_text.font16_name != NULL )
    f16 = TRUE;

  np = b - copy;
  if (np) 
  {
    if (f16)
      {
        ExtDrawString((Widget)t, FALSE, dpy, wind, gc, ix, y, copy, np);
        size = ExtTextWidth((Widget)t,&copy,np,FALSE);
      }
    else
      {
        XDrawString(dpy, wind, gc, ix, y, copy, np);
        size = np * spacing;
      }
    ix += size;
  }
 
  np = (e - b) + 1;
  if (np) 
  {
    if (f16)
      {
        ExtDrawString((Widget)t, TRUE, dpy, wind, rv_gc, ix, y, b, np);
        size = ExtTextWidth((Widget)t,&b,np,FALSE);
      }
    else
      {
        XDrawImageString(dpy, wind, rv_gc, ix, y, b, np);
        size = np * spacing;
      }
    ix += size;
  }

  np = l - e;
  if (np) 
  {
    if (f16)
      ExtDrawString((Widget)t, FALSE, dpy, wind, gc, ix, y, e+1, np);
    else
      XDrawString(dpy, wind, gc, ix, y, e+1, np);
  } 
}
/*-----------------------------------------------------------------------*/
/* Private routines                                                      */
/*-----------------------------------------------------------------------*/
/* LineSize: In the plain text case, just count the newlines             */
/*-----------------------------------------------------------------------*/

static int LineSize(w, inoutx, inoutwidth, xmin, xmax)
SGMLPlainTextObject w;
SGMLPosition *inoutx;
SGMLDimension *inoutwidth; 
SGMLPosition *xmin, *xmax; 
{
  SGMLPosition x = *inoutx;
  SGMLDimension width, maxWidth = *inoutwidth;
  Dimension left_clearance  = w->sgml_text.left_margin;
  int nlines = 1;
  int tabs = w->sgml_plain_text.tabs;
  char *p = w->sgml_text.text;
  int size = 0;
  SGMLPosition maxX = x;
  SGMLPosition minX = x;   
  SGMLPosition b_y = w->sgml_text.begin.y;
  char *pp;
  Boolean f16 = FALSE;

  if (w->sgml_text.font16_name != XrmStringToQuark("") &&
      w->sgml_text.font16_name != NULL)
    f16 = TRUE;

  for (pp = p; *p != '\0'; p++)
    {
       if (*p == '\n') 
         {
           if (f16)
             width = ExtTextWidth((Widget)w,&pp,size,FALSE);
           else
             width = size * w->sgml_text.spacing;
           if (x+width > maxWidth) maxWidth = x+width;
           if (x+width > maxX) maxX = x+width;
           nlines++;  
           size = 0;
           x = left_clearance; 
           minX = x;
           pp = p+1;
         }
       else if (*p == '\t' && tabs)  size += tabs - size%tabs; 
       else size++;
    }
 if (f16 && *pp != '0')
   width = ExtTextWidth((Widget)w,&pp,size,FALSE);
 else
   width = size * w->sgml_text.spacing;
 x += width; 
 if (x > maxWidth) maxWidth = x;
 if (x > maxX) maxX = x;

 *inoutx = x; 
 *xmin = minX; 
 *xmax = maxX;  
 *inoutwidth = maxWidth;   
 return nlines;
}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLPlainTextObject                                      */
/*-----------------------------------------------------------------------*/

Widget SGMLCreatePlainText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLPlainTextObjectClass,parent,al,ac);
}

/*-----------------------------------------------------------------------*/
/* Hilite Selection text                                                 */
/*-----------------------------------------------------------------------*/
static Widget HiliteSelection(w,mode,ci)
SGMLPlainTextObject w;
int mode;
SGMLCopyInfo *ci;
{
   char *pos = NULL;
   char *delim = " \r\n\t";
   char *text = w->sgml_text.text;
   SGMLPlainTextObjectClass class = (SGMLPlainTextObjectClass) XtClass((Widget)w);
   char *b, *e; 
   int x = ci->current_x;
   int y = ci->current_y;
   

   if (mode == SGMLSELECT_LINE)
     {
       (*class->sgml_text_class.xy_to_pos)(w,&x,&y,&pos);
       b = e = pos;
       for (;*b != '\n';b--)
         if (b == text) break;

       for (;*e != '\n' && *e != '\0';e++);

       if (b == text) w->sgml_text.begin_select = b;
       else w->sgml_text.begin_select = ++b;
       w->sgml_text.end_select = --e;

       (*class->sgml_text_class.hilite)(w,SGMLSELECT_LINE);

       return (Widget)w;
     }
   else 
      return ExtHiliteSelection((Widget)w,mode,ci,TRUE);
 
}

/*-----------------------------------------------------------------------*/
/* XY to a char Position                                                 */
/*-----------------------------------------------------------------------*/

static void XYToPos(w,x,y,pos)
SGMLPlainTextObject w;
int *x, *y;
char **pos;
{
   char *p = w->sgml_text.text;
   Position wy = w->sgml_text.begin.y;
   Position wx = w->sgml_text.begin.x;
   Position margin = w->sgml_text.margin;
   Dimension ascent = w->sgml_text.begin.ascent;
   Dimension depth = w->sgml_text.begin.descent + ascent;
   int tabs = w->sgml_plain_text.tabs;
   int spacing = w->sgml_text.spacing;
   char *np;
   int width = 0, i;
   Boolean f16 = FALSE;

   if (w->sgml_text.font16_name != XrmStringToQuark("") &&
       w->sgml_text.font16_name != NULL)
     f16 = TRUE;

   for (np = p; *p != '\0'; p++)
     {
       if (wy + depth > *y) break;
       if (*p == '\n')
         {
           wy += depth;
           wx = margin;
           np = p+1;
         }
      }


   if (wx < margin) wx = margin; 
   for (i=0; *np != '\0' && *np != '\n'; np++,i++)
     {
       if (tabs)
         {
           if (*np == '\t') 
             for (;i%tabs; i++,width+=spacing); 
         }

       if (f16)
         {
           if (wx + width + ExtTextWidth((Widget)w,&np,1,FALSE) >= *x) break;
           width += ExtTextWidth((Widget)w,&np,1,TRUE);
         }
       else
         {
           if (wx + width + spacing >= *x) break;
           width += spacing;
         }
     }
   
   *y = wy;
   *x = wx + width; /* max x is eol */
   *pos = np;       /* pos could be possible a '\0' */

}

/*-----------------------------------------------------------------------*/
/* A char Position to XY                                                 */
/*-----------------------------------------------------------------------*/

static void PosToXY(w,x,y,pos,after)
SGMLPlainTextObject w;
Position *x, *y;
char *pos;
Boolean after;
{
   char *p = w->sgml_text.text;
   Position wy = w->sgml_text.begin.y;
   Position wx = w->sgml_text.begin.x;
   Position margin = w->sgml_text.margin;
   Dimension ascent = w->sgml_text.begin.ascent;
   Dimension depth = w->sgml_text.begin.descent + ascent;
   int tabs = w->sgml_plain_text.tabs;
   char *np;
   int width, nc = 0;
   Boolean f16 = FALSE;
   int swidth;
   char *copy;

   if (w->sgml_text.font16_name != XrmStringToQuark("") &&
       w->sgml_text.font16_name != NULL)
     f16 = TRUE;

   if (*pos == '\0')
     {
       *x = w->sgml_text.end.x;
       *y = w->sgml_text.end.y;
       return;
     }

   for (np = p; *p != '\0'; p++)
     {
       if (p == pos)
         break;
       if (*p == '\n')
         {
           wy += depth;    
           wx = margin;
           np = p+1;
         }
     }

   copy = XtMalloc(1 + (p-np)*(tabs?tabs:1));
   for (width=0; np < p && *np != '\0';width++,np++)
     {
        if (tabs)
          if (*np == '\t' && np != p) for (;width%tabs; copy[width] = ' ', width++);
          else copy[width] = *np; 
     }

   if (*pos == '\t')
     nc = tabs - (width%tabs ? width%tabs : tabs) +1;
   else nc = 1;

   if (f16) 
   {
     swidth = ExtTextWidth((Widget)w,&copy,width,FALSE);
     if (*pos == '\n')
       *x = wx + (after ? w->sgml_text.width : swidth);
     else
       {
         if (*pos&0x80)
         {
          int temp = w->sgml_text.font16->max_bounds.width;
          *x = wx + swidth + (after ? ((nc-1)*w->sgml_text.spacing + temp) : 0);
         }
         else
          *x = wx + swidth + (after ? (nc*w->sgml_text.spacing) : 0);
       }
   }
   else
   {
    if (*pos == '\n')
      *x = wx + (after ? w->sgml_text.width : (width * w->sgml_text.spacing));
    else 
      *x = wx + (width * w->sgml_text.spacing) + (after ? (nc*w->sgml_text.spacing) : 0);
   }

   *y = wy;
   XtFree(copy);
}

/*-----------------------------------------------------------------------*/
/* Dump Text                                                             */
/*-----------------------------------------------------------------------*/
static Boolean DumpText(w,dp,pp)
SGMLPlainTextObject w;
SGMLDumpTextInfo *dp;
char **pp;
{
   char *text = w->sgml_text.text;
   char *b = w->sgml_text.begin_select;
   char *e = w->sgml_text.end_select;
   char *p;
   int len;
   char *t, *nl = NULL;

   if (dp->calc_len) len = (int) *pp;
   else          p = *pp;

   if (dp->sw)
     {
       if (b == NULL || e == NULL) return FALSE;
       if (dp->calc_len)
         {
           if (dp->spacing && !dp->line_spacing)     /* spaces */
             len += dp->spacing;

           if (dp->line_spacing)                     /* lines */
             { len += dp->line_spacing; dp->last_len = len; }

           len += e - b + (*e != '\0' ? 1 : 0); /* do not count the '\0' */
           for (t=b; t < e && *t !='\0'; t++)   /* locate the \n for last_len */
             if (*t == '\n') nl = t; 
           if (*t == '\n') nl = t;
           if (nl) dp->last_len += nl - b + 1;

           if (dp->copy_type == SGMLSELECT_LINE &&
               w == (SGMLPlainTextObject) dp->sw) len++;

           *pp = (char *)len;
         }
       else
         {
           if (dp->spacing && !dp->line_spacing)     /* spaces */
             for (;dp->spacing > 0; dp->spacing--)  *p++ = ' ';

           if (dp->line_spacing)                     /* lines */
             for (;dp->line_spacing > 0; dp->line_spacing--)
                { *p++ = '\n'; dp->last_pos = p; }

            for (t=b; t < e && *t != '\0'; t++, p++)
              {
                *p = *t;
                if (*t == '\n') dp->last_pos = t; /* locate the \n for last_pos */
              }

            if (dp->copy_type == SGMLSELECT_LINE &&
                w == (SGMLPlainTextObject) dp->sw) *p++ = '\n';

            if (*t != '\0') /* do not include the '\0' */
              *p++ = *t;

            *pp = p;

          }

       dp->spacing = 1;
       dp->line_spacing = 0;

       if (w == (SGMLPlainTextObject) dp->sw) return TRUE;
       else return FALSE;
     }
   else
     {
       if (dp->calc_len)
         len += strlen(text);
       else
         for (;*text != '\0';text++,p++) *p = *text;
     }
   return FALSE;
}

