/*
 * GhostviewPlus.c -- GhostviewPlus widget.
 */
  
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include "GhostviewPlusP.h"
#include <ctype.h>

#define XtSetFloatArg(arg, n, d) \
    if (sizeof(float) > sizeof(XtArgVal)) { \
	XtSetArg(arg, n, &(d)); \
    } else { \
	XtArgVal *ld = (XtArgVal *)&(d); \
	XtSetArg(arg, n, *ld); \
    }

static XtResource resources[] = {
#define offset(field) XtOffsetOf(GhostviewPlusRec, ghostview_plus.field)

    { XtNpages, XtCPages, XtRInt, sizeof(int),
	  offset(pages), XtRImmediate, (XtPointer)-1 },
    { XtNcurrentPage, XtCCurrentPage, XtRInt, sizeof(int),
	  offset(current_page), XtRImmediate, (XtPointer)0},
    { XtNdoc, XtCDoc, XtRPointer, sizeof(XtPointer),
	  offset(doc), XtRImmediate, (XtPointer)NULL},
    { XtNfile, XtCFilename, XtRString, sizeof(String),
	  offset(filename), XtRString, (XtPointer)NULL },
    { XtNmagstep, XtCMagstep, XtRInt, sizeof(int),
	  offset(magstep), XtRImmediate, (XtPointer)0 },

#undef offset
#define offset(field) XtOffsetOf(GhostviewPlusRec, ghostview.field)

    { XtNcursor, XtCCursor, XtRCursor, sizeof(XtPointer),
	  offset(cursor), XtRImmediate, NULL},
#undef offset
};
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Realize();

static void show_page();

GhostviewPlusClassRec ghostviewPlusClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &ghostviewClassRec,
    /* class_name		*/	"GhostviewPlus",
    /* widget_size		*/	sizeof(GhostviewPlusRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
    /* expose			*/	NULL,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	NULL,
    /* display_accelerator	*/	NULL,
    /* extension		*/	NULL
  },
  { /* ghostview fields */
    /* ghostview		*/	NULL,
    /* next			*/	NULL,
    /* page			*/	NULL,
    /* done			*/	NULL
  },
  { /* ghostviewPlus fields */
    /* empty			*/	NULL,
  }
};

WidgetClass ghostviewPlusWidgetClass = (WidgetClass)&ghostviewPlusClassRec;

static void Setup(gvw)
GhostviewPlusWidget gvw;
{
    struct document *doc;
    gvw->ghostview_plus.doc = NULL;
    gvw->ghostview_plus.pages = -1;
    
    /* Unlike Ghostview we always copy the filename argument */
    
    if (gvw->ghostview_plus.filename) 
      {
        FILE *psfile = fopen(gvw->ghostview_plus.filename,"r");

        gvw->ghostview_plus.filename = XtNewString(gvw->ghostview_plus.filename);
        
        if (psfile) 
          {
            doc = gvw->ghostview_plus.doc = psscan(psfile);
            fclose(psfile);
          } 
        if (doc)
          {
            gvw->ghostview_plus.pages =  doc->numpages;  
            if      (gvw->ghostview_plus.pages < 1) gvw->ghostview_plus.pages = -1;
            
            if (doc->epsf &&
    	       (doc->boundingbox[URX] > doc->boundingbox[LLX]) &&
	       (doc->boundingbox[URY] > doc->boundingbox[LLY])) 
	      {
 	        int new_llx = doc->boundingbox[LLX];
	        int new_lly = doc->boundingbox[LLY];
	        int new_urx = doc->boundingbox[URX];
	        int new_ury = doc->boundingbox[URY];

                gvw->ghostview.llx = new_llx;
                gvw->ghostview.lly = new_lly;
                gvw->ghostview.urx = new_urx;
                gvw->ghostview.ury = new_ury;
       
                GhostviewLayout((Widget) gvw,True,True);        
              }  
          }
        if (gvw->ghostview_plus.pages < 0) gvw->ghostview.filename = gvw->ghostview_plus.filename;
        else gvw->ghostview.disable_start = True;

      }
}      
/* Initialize private state. */

static void
Initialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;		/* unused */
    Cardinal *num_args;	/* unused */
{
    GhostviewPlusWidget ngvw = (GhostviewPlusWidget) new;
    GhostviewPlusWidget rgvw = (GhostviewPlusWidget) request;

    ngvw->ghostview_plus.xdpi = ngvw->ghostview.xdpi; 
    ngvw->ghostview_plus.ydpi = ngvw->ghostview.ydpi; 

    Setup(ngvw);
}

/* Destroy routine: kill the interpreter and release the GC */

static void
Destroy(w)
    Widget w;
{
    GhostviewPlusWidget gvw = (GhostviewPlusWidget) w;

    XtFree(gvw->ghostview_plus.filename);
    if (gvw->ghostview_plus.doc   ) psfree(gvw->ghostview_plus.doc);
}    

/* SetValues routine.  Set new private state, based on changed values
 * in the widget.  Always returns False, because redisplay is never needed.
 */

static Boolean
SetValues(current, request, new)
    Widget current, request, new;
{
    GhostviewPlusWidget cgvw = (GhostviewPlusWidget) current;
    GhostviewPlusWidget rgvw = (GhostviewPlusWidget) request;
    GhostviewPlusWidget ngvw = (GhostviewPlusWidget) new;
    String cfilename;
    String rfilename;
    int rpage = rgvw->ghostview_plus.current_page;
    int cpage = cgvw->ghostview_plus.current_page;
 
    cfilename = cgvw->ghostview.filename;
    if (cfilename == NULL) cfilename = "(null)";
    rfilename = rgvw->ghostview.filename;
    if (rfilename == NULL) rfilename = "(null)";
    
    if (strcmp(cfilename,rfilename))
      {
         XtFree(cgvw->ghostview_plus.filename);
         if (cgvw->ghostview_plus.doc   ) psfree(cgvw->ghostview_plus.doc);
         Setup(ngvw);
         cpage = 1;
      }
    else if (rpage != cpage)
      {   
        if (rpage > 0)
          {  
            if (ngvw->ghostview_plus.pages < 0)
              { 
                rpage = 1;      
              }
            else  
              {
                if (rpage > ngvw->ghostview_plus.pages) rpage = ngvw->ghostview_plus.pages;
                else if (rpage < 1) rpage = 1;

                if (XtIsRealized((Widget)ngvw)) show_page(ngvw,rpage,TRUE);
              }
          }
        ngvw->ghostview_plus.current_page = rpage;
      } 
#define HAS_CHANGED(field) (ngvw->ghostview_plus.field != cgvw->ghostview_plus.field)

    if (HAS_CHANGED(magstep))
      {
        Arg arglist[2];
        float newxdpi = ngvw->ghostview_plus.xdpi;
        float newydpi = ngvw->ghostview_plus.ydpi;
        int   magstep = ngvw->ghostview_plus.magstep;

        if (magstep < 0)
          for (;magstep++;) { newxdpi /= 1.2 ; newydpi /= 1.2; }
        else 
          for (;magstep--;) { newxdpi *= 1.2 ; newydpi *= 1.2; }

        XtSetFloatArg(arglist[0],XtNxdpi,newxdpi);
        XtSetFloatArg(arglist[1],XtNydpi,newydpi);
        XtSetValues((Widget)ngvw,arglist,2);
      }

#undef HAS_CHANGED
#define HAS_CHANGED(field) (ngvw->ghostview.field != cgvw->ghostview.field)

    else if (HAS_CHANGED(xdpi) || HAS_CHANGED(ydpi) || HAS_CHANGED(orientation))
      {
         if (ngvw->ghostview_plus.pages >= 0) show_page(ngvw,rpage,FALSE);
      }

#undef HAS_CHANGED

   return FALSE;
}      
static void Realize(w, valueMask, attributes)
    Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    GhostviewPlusWidget gvw = (GhostviewPlusWidget) w;
    WidgetClass superClass = XtSuperclass(w);
    (*superClass->core_class.realize)(w, valueMask, attributes);
    
    if (gvw->ghostview_plus.current_page) show_page(gvw,gvw->ghostview_plus.current_page,TRUE);
}
static void show_page(gvw,page,wait)
GhostviewPlusWidget gvw;
int page;
Boolean wait;
{
   int i;
   FILE *fp = fopen(gvw->ghostview_plus.filename,"r");
   struct document *doc = gvw->ghostview_plus.doc;

   if (!wait || GhostviewIsInterpreterReady((Widget)gvw)) {
      GhostviewNextPage((Widget)gvw);
   } else {
      GhostviewEnableInterpreter((Widget)gvw); 
      wait = FALSE;
   }
   if (!wait)
   {   
      GhostviewSendPS((Widget)gvw, fp, doc->beginprolog,
		  doc->lenprolog, False);
      GhostviewSendPS((Widget)gvw, fp, doc->beginsetup,
		  doc->lensetup, False);
   }
   if (doc->pageorder == DESCEND)  i = doc->numpages - page;
   else                            i = page - 1;
   
   if (!GhostviewSendPS((Widget)gvw,fp,doc->pages[i].begin,doc->pages[i].len, True))
      printf("GhostviewSendPS failed\n");
}      
