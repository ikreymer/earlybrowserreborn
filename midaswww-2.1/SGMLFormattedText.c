/*==================================================================*/
/*                                                                  */
/* SGMLFormattedTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a formatted text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLFormattedTextP.h"

Widget ExtHiliteSelection();
/* 
  Private functions 
*/

#define max(a,b) (a>b?a:b)

static void WordSize();
static void LineSize();
static void DrawByParts();

/*
  Widget class methods
*/

static void    Initialize();
static void    Destroy();
static void    ComputeSize();
static Boolean AdjustSize();
static void    Redisplay();
static Boolean SGMLSetValues();
static Boolean InheritChanges();
static Widget  HiliteSelection(); 
static void    XYToPos();
static void    PosToXY();
static Boolean DumpText();

#define MINIMUM(a,b) ((a)<(b)?(a):(b))
#define MAXIMUM(a,b) ((a)>(b)?(a):(b))
 
#define Offset(field) XtOffsetOf(SGMLFormattedTextRec,sgml_formatted_text.field)

static XtResource resources[] = {

    {SGMLNrightIndent,SGMLCMargin,XtRDimension,sizeof(Dimension),
    Offset (right_indent),XtRImmediate,(XtPointer)0},

    {SGMLNleftIndent,SGMLCMargin,XtRDimension,sizeof(Dimension),
    Offset (left_indent),XtRImmediate,(XtPointer)0},

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLFormattedTextClassRec  sGMLFormattedTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLFormattedText",                 /* class_name            */
    sizeof(SGMLFormattedTextRec),        /* widget_size           */
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
    AdjustSize,          		 /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
    SGMLInheritActivate,                 /* activate              */
    SGMLInheritHilite,                   /* hilite                */
    SGMLInheritContains,                 /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    SGMLInheritCallMapCallback,          /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    SGMLSetValues,                       /* sgml_set_values       */
    InheritChanges,                      /* inherit_changes       */
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


WidgetClass sGMLFormattedTextObjectClass = (WidgetClass) &sGMLFormattedTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLFormattedTextObject request, new;
{
   new->sgml_formatted_text.word_info = NULL;
   new->sgml_formatted_text.line_info = NULL;
   new->sgml_formatted_text.nwords = 0; 
   new->sgml_formatted_text.nlines = 0; 
   new->sgml_formatted_text.left_indent  = new->sgml_text.left_indent;
   new->sgml_formatted_text.right_indent = new->sgml_text.right_indent;

}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLFormattedTextObject w;
{
   XtFree((char *) w->sgml_formatted_text.word_info);
   XtFree((char *) w->sgml_formatted_text.line_info);
}

/*--------------------------------------------------------------*/
/* Inherit Changes                                              */
/*--------------------------------------------------------------*/

static Boolean InheritChanges(w,inputMask)
SGMLFormattedTextObject w;
int inputMask;
{  
    if (inputMask & gcMask) 
      {   
        XtFree((char *) w->sgml_formatted_text.word_info);
        XtFree((char *) w->sgml_formatted_text.line_info);
        
	w->sgml_formatted_text.word_info = NULL;
	w->sgml_formatted_text.line_info = NULL;
	w->sgml_formatted_text.nwords = 0; 
	w->sgml_formatted_text.nlines = 0; 
     }  
  return FALSE; 
}

/*------------------------------------------------------------------*/
/* SetValues : redraw only for font or color changes                */
/*------------------------------------------------------------------*/

static Boolean SGMLSetValues (current, request, new)
SGMLFormattedTextObject current, request, new;
{
  SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) new);
  int mask = (*class->sgml_text_class.compute_change_mask)(new,current);   

  if (mask & gcMask)
    {
        XtFree((char *) new->sgml_formatted_text.word_info);
        XtFree((char *) new->sgml_formatted_text.line_info);
        
	new->sgml_formatted_text.word_info = NULL;
	new->sgml_formatted_text.line_info = NULL;
	new->sgml_formatted_text.nwords = 0; 
	new->sgml_formatted_text.nlines = 0;    
    }
  return FALSE; 
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region,Offset)
SGMLFormattedTextObject t;
XEvent *event;
Region region;
SGMLPosition Offset;
{
   SGMLPosition Y = t->sgml_text.begin.y;
   Position y = Y - Offset;
   Position margin = t->sgml_text.margin;    
   Dimension width  = MINIMUM(t->sgml_text.width,32767-margin);
   Dimension height = MINIMUM(t->sgml_text.height,32767-y);

   if (y != Y) return;
 
   if(XRectInRegion(region,margin,y,width,height) != RectangleOut)
   {
      Display *dpy = XtDisplayOfObject((Widget) t);
      Window  wind = XtWindowOfObject((Widget) t);
      GC gc =  t->sgml_text.gc;
      XSegment *segs = NULL;

      Position x = t->sgml_text.begin.x;
      Dimension ascent = t->sgml_text.begin.ascent;
      Dimension depth = t->sgml_text.begin.descent + ascent;
      WordInfo *info = t->sgml_formatted_text.word_info;
      LineInfo *line = t->sgml_formatted_text.line_info; 
      SGMLRendition *rendition; 
      int l;
      GC rv_gc = t->sgml_text.reverse_gc;
      char *b = t->sgml_text.begin_select;
      char *e = t->sgml_text.end_select;
      int a_x = 0, a_y = 0;
      Boolean line_select_flag = FALSE;
      Boolean line_select = FALSE;
      Boolean f16 = FALSE;

      if (t->sgml_text.font16_name != XrmStringToQuark("") &&
          t->sgml_text.font16_name != NULL)
        f16 = TRUE;

      rendition = &t->sgml_text.normal_rendition;

      for( l = 0 ; l < t->sgml_formatted_text.nlines; )
      {
        if (XRectInRegion(region,margin,y,width,depth) != RectangleOut && line[l].nchars > 0)
        { 
          int m;
          char *p = XtMalloc(line[l].nchars);
          char *q = p;
          char *bp, *ep, *ip, *eol;
          int bf, ef;
          int st = line[l].start;
          int sp = line[l].stop - 1;
          int sl, sls;
          bp = ep = p;
          bf = ef = FALSE;
          eol = info[sp].word + info[sp].length;

          if ( b != NULL && e != NULL)
            {
              if ((info[st].word >= b )|| (info[st].word <= b && eol >= b)) 
                { line_select = TRUE; line_select_flag = TRUE; }
              if (info[st].word <= e && eol >= e)
                 line_select_flag = FALSE;
              if (info[st].word > e) 
                line_select = FALSE;
            }
          for ( m = line[l].start ; m < line[l].stop ; m++)
            {
              strncpy(p,info[m].word,info[m].length);
              if (line_select) {
                char *eow = info[m].word;
                eow += info[m].length;
                if (info[m].word <= b && eow >= b && !bf) {
                  for (bp = p, ip= info[m].word; ip < b; bp++, ip++);
                  bf = TRUE;
                  }
                if (info[m].word <= e && eow >= e && !ef) {
                  for (ep = p, ip= info[m].word; ip < e; ep++, ip++);
                  ef = TRUE;
                  }
                }
              p += info[m].length;
              *p++ = ' ';
            }  
          *--p = '\0'; 
          if (line_select)
            {
              if (!bf) bp = q;
              if (!ef) ep = p;
            }

          if (line_select && b!= NULL)
            {
                  
              DrawByParts(q,bp,ep,p,t,x,y+ascent,&sl,&sls);
              a_x = sl + sls; 
              a_y = y;

              /* fill addition rectangle when > 2 line_select */
              if (line_select_flag)
                {
                  unsigned int a_width = t->sgml_text.width + margin - a_x;
                  if (a_width > 0)
                    XFillRectangle(dpy,wind,gc,a_x,a_y,a_width,(unsigned int)depth); 
                }
            }
          else if (!f16)
            XDrawString(dpy, wind, gc, x, y+ascent, q, line[l].nchars-1); 
          else
            ExtDrawString((Widget)t,FALSE,dpy,wind,gc,x,y+ascent,q,line[l].nchars-1);

          if (rendition->underline)
            {
              int s;
              Position yy = y+ascent; 
 
              if (!segs) segs = (XSegment *) XtMalloc(sizeof(XSegment) * rendition->underline);

              for (s=0; s < rendition->underline ; s++ )
                {
                   yy += max(rendition->underline_height,1); 
                   segs[s].x1 = x;  
                   segs[s].x2 = line[l].size;
                   segs[s].y1 = yy;  
                   segs[s].y2 = yy;  
                   yy += max(rendition->underline_height,1); 
                }
              XDrawSegments(dpy,wind,gc,segs,rendition->underline);
              if (line_select && b!= NULL)
                {
                  yy = y+ascent; 
                  for (s=0; s < rendition->underline ; s++ )
                    {
                       yy += max(rendition->underline_height,1); 
                       segs[s].x1 = sl;  
                       segs[s].x2 = sl + sls;
                       segs[s].y1 = yy;  
                       segs[s].y2 = yy;  
                       yy += max(rendition->underline_height,1); 
                    }
                  XDrawSegments(dpy,wind,rv_gc,segs,rendition->underline);

                  /* fill additional rectangle for selected underline text */
                  if (yy > y+ascent+t->sgml_text.font->descent)
                    XFillRectangle(dpy,wind,gc,sl,yy-1,sls,1);
                }
            }
          if (rendition->outline)
            {
              XDrawRectangle(dpy,wind,gc,x,y,line[l].size-x,depth-1);
            }
          XtFree(q); 
        }   
        x = margin;
        y += depth;
        Y += depth;
        if (y != Y) break;   

        if (l++ == t->sgml_formatted_text.nlines) break;
        else if ( l+1 == t->sgml_formatted_text.nlines)
          {
            ascent = t->sgml_text.end.ascent;
            depth  = t->sgml_text.end.descent + ascent;
          }
        else
          {
            ascent = t->sgml_text.ascent;
            depth  = t->sgml_text.descent + ascent;
          }
      }

     XtFree((char *) segs);
   }
}
/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,descent)
SGMLFormattedTextObject w;
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
SGMLFormattedTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   Dimension left_clearance  = w->sgml_text.left_margin  + w->sgml_formatted_text.left_indent;
   Dimension right_clearance = w->sgml_text.right_margin + w->sgml_formatted_text.right_indent;
   Boolean punctuation;
   char *punc_char = ".,:;)"; 
   SGMLPosition xmin, xmax;

/* If we haven't already calculated the size of the words, do so now. */

   if (w->sgml_formatted_text.word_info == NULL) WordSize(w); 
 
   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before == SGMLBREAK_ALWAYS ||
/*       (w->sgml_text.break_before == SGMLBREAK_SOFT && geom->coord.x > left_clearance) || */
       (w->sgml_text.alignment != geom->alignment && geom->alignment))
   {
      _SGMLBreak(geom,w->sgml_text.space_before);
   } 

   if (w->sgml_formatted_text.nwords)
     {
/*
 *  We will leave a space before the first word if:
 *       a) Not beginning a new line
 *       b) Not a punctuation character
 *       c) the geometry structure requests it.
 */
 
       punctuation = !w->sgml_formatted_text.word_info || strchr(punc_char,*w->sgml_formatted_text.word_info[0].word);
  
       if (!punctuation) 
         if (geom->coord.x + w->sgml_text.spacing + w->sgml_formatted_text.word_info[0].size + right_clearance > geom->natural_width)
           {
              geom->coord.x = left_clearance;
              geom->coord.y += geom->coord.descent + geom->coord.ascent;
              geom->coord.descent = w->sgml_text.descent;
              geom->coord.ascent = w->sgml_text.ascent;
           }
         else if (geom->leave_space) geom->coord.x += w->sgml_text.spacing;   

       if (w->sgml_text.descent  > geom->coord.descent  ||
           w->sgml_text.ascent > geom->coord.ascent )
         {  
           if (w->sgml_text.descent  > geom->coord.descent)  geom->coord.descent  =  w->sgml_text.descent;
           if (w->sgml_text.ascent > geom->coord.ascent) geom->coord.ascent =  w->sgml_text.ascent;
       
           Adjust(Closure, geom->coord.y, geom->coord.ascent, geom->coord.descent);
         }  

       if (left_clearance > geom->coord.x && geom->broken) geom->coord.x = left_clearance;
     }

   w->sgml_text.begin = geom->coord;

   LineSize(w,geom,&xmin,&xmax);

   if (xmax > geom->actual_width) geom->actual_width = xmax;
   
   w->sgml_text.width  = xmax - xmin;
   w->sgml_text.margin = xmin;  
   w->sgml_text.height = geom->coord.y + geom->coord.descent + geom->coord.ascent  - w->sgml_text.begin.y;

   w->sgml_text.end = geom->coord;

   /*
    * Break after?
    */  

   if (w->sgml_text.break_after == SGMLBREAK_ALWAYS)
   {
     _SGMLBreak(geom,w->sgml_text.space_after);
   } 
   w->sgml_text.size_valid = TRUE; 
}

/*-----------------------------------------------------------------------*/
/* Private routines                                                      */
/*-----------------------------------------------------------------------*/
/* WordSize: Build an array giving the length of each word               */
/*-----------------------------------------------------------------------*/

static void WordSize(w)
SGMLFormattedTextObject w;
{
   int nwords;
   char *p = w->sgml_text.text;
   char *delim  = " \t\n\r"; 
   WordInfo *info = w->sgml_formatted_text.word_info;
   XFontStruct *font;
   Boolean f16 = FALSE;
   
   if (w->sgml_text.font16_name != XrmStringToQuark("") &&
       w->sgml_text.font16_name != NULL)
     f16 = TRUE;

   font=w->sgml_text.font;

   /* count how many words */
   
   for (nwords=0; *p != '\0';nwords++) 
   {
     for (; *p != '\0' && strchr(delim,*p);  p++); /* scan white space */
     if (*p == '\0') break;
      
     for (; !strchr(delim,*p); p++); 
   }
   
   /* Allocate storage for word statistics */

   XtFree((char *) info); 
   info = nwords?(WordInfo *) XtMalloc(sizeof(WordInfo) * nwords):NULL; 

   /* Now store length of each word */
   
   p = w->sgml_text.text;   
   for (nwords=0; *p != '\0' ; nwords++) 
   {
     for (; *p != '\0' && strchr(delim,*p);  p++); /* scan white space */
     if (*p == '\0') break;
     
     info[nwords].word = p;
     for (; !strchr(delim,*p); p++);
     info[nwords].length = p - info[nwords].word;

     if (!f16)
       info[nwords].size = XTextWidth(font,info[nwords].word,info[nwords].length);
     else
       info[nwords].size = ExtTextWidth((Widget)w,&info[nwords].word,info[nwords].length,FALSE);
   }
   w->sgml_formatted_text.word_info = info;
   w->sgml_formatted_text.nwords = nwords;

}
/*-----------------------------------------------------------------------*/
/* LineSize: Build an array giving the length of each line               */
/*-----------------------------------------------------------------------*/

static void LineSize(w, geom, xmin, xmax)
SGMLFormattedTextObject w;
SGMLGeometry *geom;
SGMLPosition *xmin, *xmax; 
{
  SGMLPosition x = geom->coord.x;
  SGMLPosition y = geom->coord.y; 
  SGMLPosition maxX = x;
  SGMLPosition minX = x; 
  int n = 0;  
  int l = 0;
  int nc = 0; 
  int lmax = 10;
  Dimension size = 0;  
  Dimension spacing = 0;
  WordInfo *info = w->sgml_formatted_text.word_info;
  Dimension left_clearance  = w->sgml_text.left_margin  + w->sgml_formatted_text.left_indent;
  Dimension right_clearance = w->sgml_text.right_margin + w->sgml_formatted_text.right_indent;
  LineInfo *line = (LineInfo *) XtMalloc(lmax*sizeof(LineInfo)); 

  XtFree((char *) w->sgml_formatted_text.line_info);
     
  line[l].start = n;
 
  for (; n < w->sgml_formatted_text.nwords; n++)
  {
    if (spacing>0 && x + spacing + info[n].size + right_clearance > geom->natural_width)
    {
      line[l].stop = n;
      line[l].nchars = nc;  
      line[l].size = x;
      if (x > maxX) maxX = x;

      if (++l == lmax) line = (LineInfo *) XtRealloc((char *) line, (lmax *= 2)*sizeof(LineInfo));

      line[l].start = n;
      x = left_clearance + info[n].size;
      y += geom->coord.descent + geom->coord.ascent;
      geom->coord.descent = w->sgml_text.descent;
      geom->coord.ascent = w->sgml_text.ascent;
      minX = left_clearance;
      nc = info[n].length + 1; 
    }  
    else 
    {
      x += spacing + info[n].size;
      nc += info[n].length + 1;
    }
    geom->alignment = w->sgml_text.alignment; 
    geom->broken = FALSE;
    geom->leave_space = TRUE;
    spacing =  w->sgml_text.spacing;
  }
  line[l].stop = n;
  line[l].nchars = nc;  
  line[l].size = x;
  if (x > maxX) maxX = x;
  
  w->sgml_formatted_text.line_info = line;
  w->sgml_formatted_text.nlines = l + 1;

  geom->coord.x = x; 
  geom->coord.y = y;  
  *xmin = minX; 
  *xmax = maxX;  
 
}
/*-----------------------------------------------------------------------*/
/* Private routines                                                      */
/*-----------------------------------------------------------------------*/
/* DrawByParts: Draw each part of a line in different gc                 */
/*              Use to Redisplay when a search string is found           */
/*-----------------------------------------------------------------------*/
static void DrawByParts(copy,b,e,l,t,x,y,sl,sls)
char *copy;
char *b, *e, *l;
SGMLFormattedTextObject t;
Position x, y;
int *sl, *sls;
{
  Display *dpy = XtDisplayOfObject((Widget) t);
  Window  wind = XtWindowOfObject((Widget) t);
  GC gc =  t->sgml_text.gc;
  GC rv_gc = t->sgml_text.reverse_gc;
  XFontStruct *font;
  Position ix = x;
  int np, size = 0;
  Boolean f16 = FALSE;

  if (t->sgml_text.font16_name != XrmStringToQuark("") &&
      t->sgml_text.font16_name != NULL)
    f16 = TRUE;

  font=t->sgml_text.font;
  np = b - copy;
  if (np > 0) 
  {
    if (!f16)
      {
        XDrawString(dpy, wind, gc, ix, y, copy, np);
        size = XTextWidth(font,copy,np);
      }
    else
      {
        ExtDrawString((Widget)t, FALSE, dpy, wind, gc, ix, y, copy, np);
        size = ExtTextWidth((Widget)t,&copy,np,FALSE);
      }
    ix += size;
  }

  np = (e - b) + 1;
  if (np > 0) 
  {
    if (!f16)
      {
        XDrawImageString(dpy, wind, rv_gc, ix, y, b, np);
        size = XTextWidth(font,b,np);
      }
    else
      {
        ExtDrawString((Widget)t, TRUE, dpy,wind, rv_gc, ix, y, b, np);
        size = ExtTextWidth((Widget)t,&b,np,FALSE);
      }

    *sl = ix;
    *sls = size;
    ix += size;
  }

  np = l - e;
  if (np > 0)
    if (!f16)
      XDrawString(dpy, wind, gc, ix, y, e+1, np);
    else
      ExtDrawString((Widget)t, FALSE, dpy, wind, gc, ix, y, e+1, np);
}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLFormattedTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateFormattedText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLFormattedTextObjectClass,parent,al,ac);
}

/*-----------------------------------------------------------------------*/
/* Hilite Selection text                                                 */
/*-----------------------------------------------------------------------*/

static Widget HiliteSelection(w,mode,ci)
SGMLFormattedTextObject w;
int mode;
SGMLCopyInfo *ci;
{
   char *pos = NULL;
   char *delim = " \r\n\t";
   char *text = w->sgml_text.text;
   WordInfo *info = w->sgml_formatted_text.word_info;
   LineInfo *line = w->sgml_formatted_text.line_info;
   SGMLFormattedTextObjectClass class = (SGMLFormattedTextObjectClass) XtClass((Widget)w);
   char *b, *e, *eot;
   int x = ci->current_x;
   int y = ci->current_y;


   if (mode == SGMLSELECT_LINE)
     {
       int l, st, sp;

       (*class->sgml_text_class.xy_to_pos)(w,&x,&y,&pos);
       b = e = pos;
       for (l = 0; l < w->sgml_formatted_text.nlines; l++)
         {
           int stp = line[l].stop - 1;

           if (info[stp].word + info[stp].length < pos) 
             continue;
           break;  
         }
       st = line[l].start;
       sp = line[l].stop - 1;

       w->sgml_text.begin_select = b = info[st].word;
       e = info[sp].word + info[sp].length;
       for (;e && strchr(delim,*e); e--);
       w->sgml_text.end_select = e;

       (*class->sgml_text_class.hilite)(w,SGMLSELECT_LINE);

       return (Widget) w;
     }
   else 
      return ExtHiliteSelection((Widget)w,mode,ci,FALSE);

}

/*-----------------------------------------------------------------------*/
/* XY to a char Position                                                 */
/*-----------------------------------------------------------------------*/

static void XYToPos(w,x,y,pos)
SGMLFormattedTextObject w;
int *x, *y;
char **pos;
{
   Position wy = w->sgml_text.begin.y; 
   Position wx = w->sgml_text.begin.x;
   Position margin = w->sgml_text.margin;
   Dimension ascent = w->sgml_text.begin.ascent;
   Dimension depth = w->sgml_text.begin.descent + ascent;
   WordInfo *info = w->sgml_formatted_text.word_info;
   LineInfo *line = w->sgml_formatted_text.line_info;
   XFontStruct *font = w->sgml_text.font;
   char *text = w->sgml_text.text;
   char *delim = " \r\t\n";
   char *p;
   int nc, l, m;
   int width = 0;
   Boolean f16 = FALSE;

   if (w->sgml_text.font16_name != XrmStringToQuark("") &&
       w->sgml_text.font16_name != NULL)
     f16 = TRUE;
   
   for (;text != NULL && *text != '\0' && strchr(delim,*text);text++);
   if (*text == '\0' || (wy + depth > *y && wx + 
                        (f16 ? ExtTextWidth((Widget)w,&text,1,FALSE) : XTextWidth(font,text,1)) > *x))
     {
       *x = wx; *y = wy; *pos = text;
       return;
     }

   for (l = 0; l < w->sgml_formatted_text.nlines; l++)
     {
       if (wy + depth > *y) break;
       wy += depth;
       wx = margin;
     }

   if (*x > line[l].size)                    /* the end of line */
     {
       wx = line[l].size;
       m = line[l].stop - 1;
       p = info[m].word + info[m].length -1; /* xy point to a delim, make it point to last instead */
     }       
   else
     {
       for (m = line[l].start; m < line[l].stop; m++)
         {
           if (wx + info[m].size + w->sgml_text.spacing > *x) break;
           wx += info[m].size + w->sgml_text.spacing;
         }

       for (p = info[m].word, nc = 0; nc < info[m].length; nc++, p++)
         {
           width = f16 ? ExtTextWidth((Widget)w,&p,1,TRUE) : XTextWidth(font,p,1); 
           if (wx + width > *x ) break;
           if (nc == info[m].length -1) p++; /* xy point to a delim */
           wx += width;
         } 
     }

 
   *y = wy;
   *x = wx;
   *pos = p;  /* pos could possible be '\0' */

}

/*-----------------------------------------------------------------------*/
/* A char Position to XY                                                 */
/*-----------------------------------------------------------------------*/

static void PosToXY(w,x,y,pos,after_pos )
SGMLFormattedTextObject w;
Position *x, *y;
char *pos;
Boolean after_pos;
{
   Position wy = w->sgml_text.begin.y; 
   Position wx = w->sgml_text.begin.x;
   Position margin = w->sgml_text.margin;
   Dimension ascent = w->sgml_text.begin.ascent;
   Dimension depth = w->sgml_text.begin.descent + ascent;
   WordInfo *info = w->sgml_formatted_text.word_info;
   LineInfo *line = w->sgml_formatted_text.line_info;
   XFontStruct *font = w->sgml_text.font;
   char *text = w->sgml_text.text;
   char *delim = " \r\t\n";
   int l, m;
   int width = 0;
   char *eot;
   int poswidth = 0;
   Boolean f16 = FALSE;

   if (w->sgml_text.font16_name != XrmStringToQuark("") &&
       w->sgml_text.font16_name != NULL)
     f16 = TRUE;

   if (pos != NULL && strchr(delim,*pos)!=NULL) poswidth = w->sgml_text.spacing; 
   else poswidth = f16 ? ExtTextWidth((Widget)w,&pos,1,FALSE) : XTextWidth(font,pos,1);

   if (pos == w->sgml_text.text)
     {
       *x = wx + (after_pos? poswidth : 0);
       *y = wy;
       return;
     }

   for (;text != NULL && *text != '\0' && strchr(delim,*text);text++);
   if ( pos == text)
     {
       *x = wx + (after_pos ? poswidth : 0);
       *y = wy;
       return;
     }

   /* if pos is a delim at the end of text, ignore after_pos  */
   if (*text != '\0')
     {
       char *end;
       for (eot = text; *eot != '\0'; eot++);
       for (end = eot;strchr(delim,*eot);eot--);
       if (pos > eot && pos < end)
         {
           *x = w->sgml_text.end.x; 
           *y = w->sgml_text.end.y;
           return;
         }
     }
  
   if (*pos == '\0')
     {
       *x = w->sgml_text.end.x;
       *y = w->sgml_text.end.y;
       return;
     }

   for (l = 0; l < w->sgml_formatted_text.nlines; l++)
     {
       int stp = line[l].stop - 1;
       char *eol = info[stp].word + info[stp].length;

       /* not neccessary, but safer to check */
       if (!line[l].stop) 
         {
           *x = wx + (after_pos  ? poswidth : 0);
           *y = wy;
           return;
         }

       for (;*eol != '\0' && strchr(delim,*(eol+1))!=NULL; eol++);
       if (eol < pos) 
         {
           wy += depth;
           wx = margin;
         }
       else
         break;  
     }

   for (m = line[l].start; m < line[l].stop; m++)
     {
       char *iw = info[m].word;
       char *eow = info[m].word + info[m].length;
       char *maxw;
 
       /* also count all the delim in between words */
       for (maxw = eow;*maxw != '\0' && strchr(delim,*(maxw+1))!=NULL; maxw++);

       if (info[m].word <= pos && maxw >= pos)
         {
           
           if (pos >= eow && pos <= maxw) /* pos is a delim */
             {
               if (m == line[l].stop -1) /* the end of line, tricky ; ignore the after_pos */
                 width += info[m].size;
               else 
                 width += info[m].size + (after_pos  ? poswidth : 0);
             }
           else
             width += (f16 ? ExtTextWidth((Widget)w,&iw,pos-iw,FALSE) : XTextWidth(font,iw,pos-iw)) 
                      + (after_pos  ? poswidth : 0);
           break;
         }
       else
         width += info[m].size + w->sgml_text.spacing; 
     }

   *y = wy;
   *x = wx + width;
}

/*-----------------------------------------------------------------------*/
/* Dump Text                                                             */
/*-----------------------------------------------------------------------*/
static Boolean DumpText(w,dp,pp)
SGMLFormattedTextObject w;
SGMLDumpTextInfo *dp;
char **pp;
{
   char *text = w->sgml_text.text;
   char *b = w->sgml_text.begin_select;
   char *e = w->sgml_text.end_select;
   Dimension space_before = w->sgml_text.space_before;
   Dimension space_after = w->sgml_text.space_after;
   int break_before = w->sgml_text.break_before;
   int break_after = w->sgml_text.break_after;
   int left_indent = w->sgml_text.left_indent;
   int right_indent = w->sgml_text.right_indent;
   int para_indent = w->sgml_text.paragraph_indent;
   SGMLRendition *rendition = &w->sgml_text.normal_rendition; 
   WordInfo *info = w->sgml_formatted_text.word_info;
   LineInfo *line = w->sgml_formatted_text.line_info;
   int nwords = w->sgml_formatted_text.nwords;
   char *delim = " \t\r\n";
   char *punc_char = ".,:;)"; 
   char *p;
   int len, m, bm, em, tl;
   int maxlen = 80;
   int width;

   if (dp->calc_len) { len = (int) *pp; width = len - dp->last_len; }
   else              { p = *pp;         width = p - dp->last_pos;   }

   switch (dp->format_type)
   {
     case SGMLDUMP_ASCII: /* dump text in 80 chars width */
       if (dp->sw)
         {
           if (b == NULL || e == NULL) return FALSE;

           dp->begin_sw = TRUE;

           for (m=0; m < nwords; m++) /* locate the selected words */
             {
               char *nw;
               if (m == nwords -1) nw = info[m].word + info[m].length;
               else nw = info[m+1].word;
               if (info[m].word <= b && nw > b) bm = m;
               if (info[m].word <= e && nw > e) { em = m; break; }
             } 

           if (dp->calc_len)
             {
               if (dp->spacing && !dp->line_spacing && !strchr(punc_char,*b)) /* spaces */ 
                 { len += dp->spacing; width += dp->spacing; }

               if (dp->line_spacing) 
                 { len += dp->line_spacing; dp->last_len = len; width = 0; }  /* lines  */

               if (bm != em)
                 {
                   tl = info[bm].word + info[bm].length - b;
                   if (width + tl +1 > maxlen) 
                     { len += 1; dp->last_len = len; width = (tl < 0 ? 0 : tl) +1; }
                   else width += (tl < 0 ? 0 : tl);
                   len += 1 + ( tl < 0 ? 0 : tl);
                   for (m = bm +1; m < em; m++)
                     {
                       if (width + info[m].length +1 > maxlen) 
                         { len += 1; dp->last_len = len; width = info[m].length +1; }
                       else width += info[m].length + 1;
                       len += info[m].length + 1;
                     }
                   tl = e - info[em].word + 1;
                   if (width + tl +1 > maxlen) 
                     { len += 1; dp->last_len = len; width = MINIMUM(tl,info[em].length +1); }
                   else width += MINIMUM(tl,info[em].length +1);
                   len += MINIMUM(tl, info[em].length +1); 
                 }
               else 
                 {
                   tl = MINIMUM(e - b + 1, info[bm].length +1);
                   if (width + tl > maxlen) { len += 1; dp->last_len = len; width = tl; }
                   else width += tl;
                   len += tl; 
                 }

               if (dp->copy_type == SGMLSELECT_LINE &&
                   w == (SGMLFormattedTextObject) dp->sw) len++;   /* explicitely add a line */

               *pp = (char *)len;
             }
           else
             {
                if (dp->spacing && !dp->line_spacing && !strchr(punc_char,*b))  /* spaces */ 
                  for (;dp->spacing > 0; dp->spacing--, width++)  *p++ = ' ';

                if (dp->line_spacing)                                           /* lines */
                   for (width=0;dp->line_spacing > 0; dp->line_spacing--)  
                      { *p++ = '\n'; dp->last_pos = p; }

                if (bm != em)
                  {
                    tl = info[bm].word + info[bm].length - b;
                    if (tl > 0) 
                      { 
                        if (width + tl +1 > maxlen) { *p++ = '\n'; dp->last_pos = p; width = tl; }
                        else width += tl;
                        strncpy(p,b,tl); p += tl; 
                      }
                    *p++ = ' '; 
                    width++;
                    for (m=bm+1; m < em; m++)
                      {
                        if (width + info[m].length +1 > maxlen) 
                          { *p++ = '\n'; dp->last_pos = p; width = info[m].length +1; }
                        else width += info[m].length +1;
                        strncpy(p,info[m].word,info[m].length);
                        p += info[m].length;
                        *p++ = ' ';
                      }
                    tl = MINIMUM(e-info[em].word +1,info[em].length);
                    if (width + tl +1 > maxlen) { *p++ = '\n'; dp->last_pos = p; width = tl; }
                    else width += tl;
                    strncpy(p,info[em].word,tl);
                    p += tl;
                  }
                else
                  {
                    char *eow = info[bm].word + info[bm].length;
                    if (width + e -b +1 > maxlen) { *p++ = '\n'; dp->last_pos = p; width = e - b +1; }
                    else width += e -b +1;
                    for (;b != e && b != eow && *b != '\0'; b++, p++)
                      *p = *b;
                    if (b == eow) *p++ = ' ';
                    else          *p++ = *b;
                  }

                if (dp->copy_type == SGMLSELECT_LINE &&
                    w == (SGMLFormattedTextObject) dp->sw) *p++ = '\n'; /* explicitly add a line */

                *pp = p;

              }

           dp->spacing = 1;               /* space flag for next widget */
           dp->line_spacing = 0;          /* line flag for next widget  */

           if (w == (SGMLFormattedTextObject) dp->sw) return TRUE;
           else return FALSE;
         }
       else
         {
           if (dp->calc_len)
             len += strlen(text);
           else
             for (;*text != '\0';text++,p++) *p = *text;
         }
       break;
     case SGMLDUMP_HTML_ASCII:
     default:
       printf ("not implement");
   }

   return FALSE;

}
