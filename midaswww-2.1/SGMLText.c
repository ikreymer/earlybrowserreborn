/*==================================================================*/
/*                                                                  */
/* SGMLTextObject                                                   */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLTextP.h"
#include "SGMLHyperP.h" 
#include "SGMLCompositeTextP.h"

/* 
  External functions 
*/
extern Widget ExtHiliteSelection();

/* 
  Private functions 
*/

#define max(a,b) (a>b?a:b)
void _SGMLBreak(); 

/*
  Widget class methods
*/

static void    ClassPartInitialize();
static void    Initialize();
static void    ClassInitialize();
static void    Destroy();
static Boolean SetValues();
static void    ComputeSize();
static Boolean AdjustSize();
static void    AdjustPosition();
static void    Redisplay();
static void    Activate();
static void    Hilite();
static Widget  Contains();
static void    CallCreateCallback();
static void    CallMapCallback();
static void    MakeVisible();
static Boolean SGMLSetValues();
static Boolean InheritChanges();
static int     ComputeChangeMask();  
static Widget  Search();
static Boolean ClearSelect();
static Widget  HiliteSelection();
static void    XYToPos();
static void    PosToXY();
static Boolean DumpText();

#define Offset(field) XtOffsetOf(SGMLTextRec,sgml_text.field)

static XtResource resources[] = {

    {SGMLNtext, SGMLCText, XtRString, sizeof(String),
    Offset(text), XtRString, ""},

    {SGMLNparam, SGMLCParam, XtRString, sizeof(String),
    Offset(param), XtRString, ""},

    {SGMLNlength, SGMLCLength, XtRInt, sizeof(int),
    Offset(length), XtRImmediate, 0},

    {SGMLNparamLength, SGMLCParamLength, XtRInt, sizeof(int),
    Offset(param_length), XtRImmediate, 0},
 
    {SGMLNheight, XtCReadOnly, XtRDimension, sizeof(Dimension),
    Offset(height), XtRImmediate, 0},

    {SGMLNwidth, XtCReadOnly, XtRDimension, sizeof(Dimension),
    Offset(width), XtRImmediate, 0},
 
    {SGMLNinheritance, XtCReadOnly, XtRInt, sizeof(int),
    Offset(inheritance), XtRImmediate, 0},
 
    {SGMLNcolor, SGMLCColor, XtRPixel, sizeof (Pixel),
    Offset(normal_rendition.color),XtRString, SGMLINHERIT_COLOR},

    {SGMLNoutline,SGMLCOutline,XtRBoolean,sizeof(Boolean),
    Offset (normal_rendition.outline),XtRImmediate,(XtPointer)SGMLINHERIT_OUTLINE},

    {SGMLNunderline,SGMLCUnderline,XtRInt,sizeof(int),
    Offset (normal_rendition.underline),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINE},

    {SGMLNunderlineHeight, SGMLCUnderlineHeight, XtRDimension, sizeof(Dimension),
    Offset (normal_rendition.underline_height),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINEHEIGHT},
   
    {SGMLNunderlineStyle, SGMLCUnderlineStyle, SGMLRLineStyle, sizeof(int),
    Offset (normal_rendition.underline_style),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINESTYLE},
 
    {SGMLNfontFamily, SGMLCFontFamily, SGMLRQuark, sizeof(XrmQuark),
    Offset (normal_rendition.family), XtRImmediate, SGMLINHERIT_QUARK},
 
    {SGMLNfontWeight, SGMLCFontWeight, SGMLRQuark, sizeof(XrmQuark),
    Offset (normal_rendition.weight), XtRImmediate, SGMLINHERIT_QUARK},
 
    {SGMLNfontSlant, SGMLCFontSlant, SGMLRQuark, sizeof(XrmQuark),
    Offset (normal_rendition.slant), XtRImmediate, SGMLINHERIT_QUARK},
 
    {SGMLNfontWidth, SGMLCFontWidth, SGMLRQuark, sizeof(XrmQuark),
    Offset (normal_rendition.width), XtRImmediate, SGMLINHERIT_QUARK},
 
    {SGMLNfontSpacing, SGMLCFontSpacing, SGMLRQuark, sizeof(XrmQuark),
    Offset (normal_rendition.spacing), XtRImmediate, SGMLINHERIT_QUARK},
 
    {SGMLNfontRegistry, SGMLCFontRegistry, SGMLRQuark, sizeof(XrmQuark),
    Offset (normal_rendition.registry), XtRImmediate, SGMLINHERIT_QUARK},

    {SGMLNfontSize, SGMLCFontSize, XtRInt, sizeof(int),
    Offset (normal_rendition.size), XtRImmediate, SGMLINHERIT_SIZE},

    {SGMLNbreakBefore,SGMLCBreak,SGMLRBreak,sizeof(int),
    Offset (break_before),XtRImmediate,(XtPointer)SGMLBREAK_SOFT},

    {SGMLNbreakAfter,SGMLCBreak,SGMLRBreak,sizeof(int),
    Offset (break_after),XtRImmediate,(XtPointer)SGMLBREAK_SOFT},

    {SGMLNsensitive,SGMLCSensitive,XtRBoolean,sizeof(Boolean),
    Offset (sensitive),XtRImmediate,(XtPointer)SGMLINHERIT_SENSITIVITY},

    {SGMLNmatched,SGMLCMatched,XtRInt,sizeof(int),
    Offset (matched),XtRImmediate,(XtPointer)TRUE},
  
    {SGMLNmanaged,SGMLCManaged,XtRBoolean,sizeof(Boolean),
    Offset (managed),XtRImmediate,(XtPointer)TRUE},
 
    {SGMLNexactTerminator,SGMLCExactTerminator,XtRBoolean,sizeof(Boolean),
    Offset (exact_terminator),XtRImmediate,(XtPointer)SGMLINHERIT_SENSITIVITY},

    {SGMLNhideIllegalTags,SGMLCHideIllegalTags,XtRBoolean,sizeof(Boolean),
    Offset (hide_illegal_tags),XtRImmediate,(XtPointer)SGMLINHERIT_SENSITIVITY},

    {SGMLNsupportsEntities,SGMLCSupportsEntities,XtRBoolean,sizeof(Boolean),
    Offset (supports_entities),XtRImmediate,(XtPointer)SGMLINHERIT_SENSITIVITY},

    {SGMLNrightMargin,SGMLCMargin,XtRInt,sizeof(int),
    Offset (right_margin),XtRImmediate,(XtPointer)SGMLINHERIT_MARGIN},

    {SGMLNleftMargin,SGMLCMargin,XtRInt,sizeof(int),
    Offset (left_margin),XtRImmediate,(XtPointer)SGMLINHERIT_MARGIN},

    {SGMLNrightIndent,SGMLCMargin,XtRInt,sizeof(int),
    Offset (right_indent),XtRImmediate,(XtPointer)0},

    {SGMLNleftIndent,SGMLCMargin,XtRInt,sizeof(int),
    Offset (left_indent),XtRImmediate,(XtPointer)0},

    {SGMLNparagraphIndent,SGMLCMargin,XtRInt,sizeof(int),
    Offset (paragraph_indent),XtRImmediate,(XtPointer)SGMLINHERIT_PARAGRAPHINDENT},

    {SGMLNalignment,SGMLCAlignment,XtRInt,sizeof(int),
    Offset (alignment),XtRImmediate,(XtPointer)SGMLINHERIT_ALIGNMENT},

    {SGMLNspacing,SGMLCSpacing,XtRDimension,sizeof(Dimension),
    Offset (spacing),XtRImmediate,(XtPointer)0},

    {SGMLNspaceBefore,SGMLCSpacing,XtRDimension,sizeof(Dimension),
    Offset (space_before),XtRImmediate,0},

    {SGMLNspaceAfter,SGMLCSpacing,XtRDimension,sizeof(Dimension),
    Offset (space_after),XtRImmediate,0},

    {SGMLNdescent,SGMLCDescent,XtRDimension,sizeof(Dimension),
    Offset (descent),XtRImmediate,(XtPointer)0},

    {SGMLNascent,SGMLCAscent,XtRDimension,sizeof(Dimension),
    Offset (ascent),XtRImmediate,(XtPointer)0},

    {SGMLNuserdata,SGMLCUserdata,XtRPointer,sizeof(XtPointer),
    Offset (userdata),XtRImmediate,(XtPointer)0},

    {SGMLNactivateCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (activate),XtRCallback,NULL},

    {SGMLNmapCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (map),XtRCallback,NULL},

    {SGMLNunmapCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (unmap),XtRCallback,NULL},

    {SGMLNcreateCallback, SGMLCCallback, XtRCallback, sizeof(caddr_t),
    Offset (create),XtRCallback,NULL}, 

    {SGMLNfont16, SGMLCFont16, SGMLRQuark, sizeof(XrmQuark),
    Offset (font16_name), XtRImmediate, SGMLINHERIT_QUARK}
 
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLTextClassRec  sGMLTextClassRec = {
    {
    (WidgetClass) &objectClassRec,       /* superclass            */
    "SGMLText",                          /* class_name            */
    sizeof(SGMLTextRec),                 /* widget_size           */
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
    ComputeSize,          		 /* compute_size          */
    AdjustSize,          		 /* adjust_size           */
    AdjustPosition,          		 /* adjust_position       */
    Redisplay,                           /* expose                */
    Activate,                            /* activate              */
    Hilite,                              /* hilite                */
    Contains,                            /* contains              */
    CallCreateCallback,                  /* call_create_callback  */ 
    CallMapCallback,                     /* call_map_callback     */ 
    MakeVisible,                         /* make_visible          */
    SGMLSetValues,                       /* sgml_set_values       */
    InheritChanges,                      /* inherit_changes       */
    ComputeChangeMask,                   /* compute_change_mask   */
    Search,                              /* search                */
    ClearSelect,                         /* clear_select          */
    HiliteSelection,                     /* hilite_selection      */
    XYToPos,                             /* xy_to_pos             */
    PosToXY,                             /* pos_to_xy             */
    DumpText,                            /* dump_text             */
    NULL,                                /* extension             */
    }
};

WidgetClass sGMLTextObjectClass = (WidgetClass) &sGMLTextClassRec;

#define	done(type, value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XtPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}
static Boolean CvtStringToBreak (dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    char *s;
    static int flag;
       
    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToBreak","XtToolkitError",
                  "String to Break conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = (char *) fromVal->addr;
    flag = SGMLBREAK_NEVER;

    if      (!strcmp(s,"SGMLBREAK_NEVER" )) flag = SGMLBREAK_NEVER; 
    else if (!strcmp(s,"SGMLBREAK_ALWAYS")) flag = SGMLBREAK_ALWAYS; 
    else if (!strcmp(s,"SGMLBREAK_SOFT"))   flag = SGMLBREAK_SOFT; 
    else XtStringConversionWarning((char *) fromVal->addr, SGMLRBreak);

    done(int,flag);
}
static Boolean CvtStringToLineStyle (dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    char *s;
    static int flag;
       
    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToLineStyle","XtToolkitError",
                  "String to Break conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = (char *) fromVal->addr;
    flag = SGMLLINE_STYLE_SOLID;

    if      (!strcmp(s,"SGMLLINE_STYLE_SOLID" )) flag = SGMLLINE_STYLE_SOLID; 
    else if (!strcmp(s,"SGMLLINE_STYLE_DOTTED")) flag = SGMLLINE_STYLE_DOTTED; 
    else if (!strcmp(s,"SGMLLINE_STYLE_DASHED")) flag = SGMLLINE_STYLE_DASHED; 
    else XtStringConversionWarning((char *) fromVal->addr, SGMLRLineStyle);

    done(int,flag);
}
static void lowcase(p)
register char *p;
{
    while(*p)
    {
        *p = tolower(*p); 
        p++;
    }
}
static Boolean CvtStringToQuark(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    char *s; 
    static XrmQuark q;

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToQuark","XtToolkitError",
                  "String to Quark conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = XtNewString((char *) fromVal->addr);
    lowcase(s); 
    q = XrmStringToQuark(s);          
    XtFree(s);
    done(XrmQuark,q);
}
static Boolean CvtRenditionToFontStruct(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    XFontStruct	    *f;
    Display*	display;
    char fontbuf[255];
    SGMLRendition *rendition = (SGMLRendition *) fromVal->addr;

    if (*num_args != 1) {
     XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	     "wrongParameters","cvtRenditionToFontStruct","XtToolkitError",
             "String to font conversion needs display argument",
              (String *) NULL, (Cardinal *)NULL);
     return False;
    }

    display = *(Display**)args[0].addr;

    sprintf(fontbuf,"-*-%s-%s-%s-%s-*-%d-*-*-*-%s-*-%s-1",
                     XrmQuarkToString(rendition->family),
                     XrmQuarkToString(rendition->weight),
                     XrmQuarkToString(rendition->slant),
                     XrmQuarkToString(rendition->width),
                     rendition->size,
                     XrmQuarkToString(rendition->spacing),
                     XrmQuarkToString(rendition->registry)); 

    f = XLoadQueryFont(display, fontbuf);
    if (f != NULL) { done( XFontStruct*, f); }
    else
      {
         XrmQuark star = XrmPermStringToQuark("*");
         static XrmQuark old_registry, old_spacing;
         static char **fontlist = NULL;
         static nf;
         int i, bestPenalty = 99999, best = 0;           
#ifdef DEBUG
         XtDisplayStringConversionWarning(dpy, fontbuf, XtRFontStruct);
#endif
         /* Try to find a substitute font */

         /*
          * MacX seems to take forever if you ask it for fonts matching a particular
          * pattern, but responds reaonably if you ask for all fonts. Most other servers
          * tested seem to be faster if only asked for a subset of the fonts available.
          */

         if (!strncmp(XServerVendor(display),"MacX",4))
           {
             if (!fontlist) fontlist = XListFonts(display,"*",500,&nf);
           }
         else if (!fontlist || rendition->spacing!=old_spacing || rendition->registry!= old_registry)
           {
             if (fontlist) XFreeFontNames(fontlist);
             sprintf(fontbuf,"-*-*-*-*-*-*-*-*-*-*-%s-*-%s-1",
                              XrmQuarkToString(rendition->spacing),
                              XrmQuarkToString(rendition->registry)); 

             old_spacing  = rendition->spacing;
             old_registry = rendition->registry;    
 
             fontlist = XListFonts(display,fontbuf,500,&nf);
           }
         for (i = 0; i < nf; i++ )
           {
             int penalty = 0;
             char *p, *token = strcpy(fontbuf,fontlist[i]);  

             for (p = token; *p ; p++) if (*p == '-') *p = '\0'; 

             token += strlen(token) + 1;
             token += strlen(token) + 1;
             if (rendition->family != star && strcasecmp(token,XrmQuarkToString(rendition->family))) penalty += 10;  
             token += strlen(token) + 1;
             if (rendition->weight != star && strcasecmp(token,XrmQuarkToString(rendition->weight))) penalty += 5;  
             token += strlen(token) + 1;
             if (rendition->slant  != star && strcasecmp(token,XrmQuarkToString(rendition->slant ))) penalty += 20;  
             token += strlen(token) + 1;
             if (rendition->width  != star && strcasecmp(token,XrmQuarkToString(rendition->width ))) penalty += 20;  
             token += strlen(token) + 1;
             token += strlen(token) + 1;
             penalty += (atoi(token)-rendition->size) * (atoi(token)-rendition->size);   
             token += strlen(token) + 1;
             token += strlen(token) + 1;
             token += strlen(token) + 1;
             if (rendition->spacing  != star && strcasecmp(token,XrmQuarkToString(rendition->spacing ))) penalty += 200;  
             token += strlen(token) + 1;
             token += strlen(token) + 1;
             if (rendition->registry != star && strcasecmp(token,XrmQuarkToString(rendition->registry))) penalty += 2000;  
             
 
             if (penalty < bestPenalty) { bestPenalty = penalty; best = i; } 
           }

         if (best)
           {
             int one = 1;
#ifdef DEBUG   
             XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	       "subFont","cvtRenditionToFontStruct","XtToolkitError",
               "Substituted font '%s'",
                (String *) &fontlist[best], (Cardinal *) &one);
#endif 
             f = XLoadQueryFont(display,fontlist[best]);
           }

         if (f != NULL) { done( XFontStruct*, f); }
       }

    f = XLoadQueryFont(display,"fixed");
    if (f != NULL) { done( XFontStruct*, f); }

    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	     "noFont","cvtRenditionToFontStruct","XtToolkitError",
             "Unable to load emergency font 'fixed'",
              (String *) NULL, (Cardinal *)NULL);
    
    return False;
}

static void FreeFontStruct(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;	/* unused */
    XrmValuePtr	args;
    Cardinal	*num_args;
{
    Display *display;
    if (*num_args != 1) {
     XtAppWarningMsg(app,
	     "wrongParameters","freeFontStruct","XtToolkitError",
             "Free FontStruct requires display argument",
              (String *) NULL, (Cardinal *)NULL);
     return;
    }

    display = *(Display**)args[0].addr;
    XFreeFont( display, *(XFontStruct**)toVal->addr );
}

static void FetchDisplayArg(widget, size, value)
    Widget widget;
    Cardinal *size;
    XrmValue* value;
{
    static Display *dpy;
    
    if (widget == NULL)
	XtErrorMsg("missingWidget", "fetchDisplayArg","XtToolkitError",
		   "FetchDisplayArg called without a widget to reference",
		   (String*)NULL, (Cardinal*)NULL);

        /* can't return any useful Display and caller will de-ref NULL,
	   so aborting is the only useful option */

    dpy = DisplayOfScreen(XtScreenOfObject(widget));

    value->size = sizeof(Display*);
    value->addr = (XtPointer) &dpy;
}
/*--------------------------------------------------------------*/
/* ClassInitialize: Register the converters                     */
/*--------------------------------------------------------------*/

static void ClassInitialize()
{
   int bug1, bug2;
   static XtConvertArgRec displayConvertArg[] = {
       {XtProcedureArg, (XtPointer) FetchDisplayArg, 0} };

   XtSetTypeConverter("SGMLRendition", "SGMLFontStruct", CvtRenditionToFontStruct,
 	              displayConvertArg, XtNumber(displayConvertArg),
                      XtCacheByDisplay, FreeFontStruct);
   XtSetTypeConverter(XtRString, SGMLRBreak, CvtStringToBreak,
 	              (XtConvertArgList) NULL, (Cardinal) 0, XtCacheNone, NULL);
   XtSetTypeConverter(XtRString, SGMLRLineStyle, CvtStringToLineStyle,
 	              (XtConvertArgList) NULL, (Cardinal) 0, XtCacheNone, NULL);
   XtSetTypeConverter(XtRString, SGMLRQuark, CvtStringToQuark,
 	              (XtConvertArgList) NULL, (Cardinal) 0, XtCacheNone, NULL);
   /* 
    * Check that the horrible hack to get around Xt bug is going to work 
    */  
  
   bug1 = XtOffsetOf(WidgetRec,core.num_popups);
   bug2 = XtOffsetOf(SGMLTextRec,sgml_text.num_popups); 

   if (bug1 != bug2) 
     {
       printf("The terrible bug fix strikes again!!!! %d %d\n",bug1,bug2); 
       exit(1); 
     }
}

/*--------------------------------------------------------------*/
/* SetFontInfo: (private function)                              */
/*--------------------------------------------------------------*/
static void SetFontInfo(new)
SGMLTextObject new;
{
    /* if possible we will inherit graphic contexts from our parent */ 

    SGMLTextObject parent = (SGMLTextObject) XtParent((Widget) new);
    SGMLRendition  *rendition, *parent_rendition;
    int dir, ascent, descent;
    XCharStruct charinfo;

    if (SGMLIsText((Widget) parent)) parent_rendition = &parent->sgml_text.normal_rendition;

    rendition = &new->sgml_text.normal_rendition;

    if (SGMLIsText((Widget) parent)                           &&
        parent_rendition->family      == rendition->family    &&
        parent_rendition->weight      == rendition->weight    &&
        parent_rendition->slant       == rendition->slant     &&
        parent_rendition->width       == rendition->width     &&
        parent_rendition->size        == rendition->size      &&
        parent_rendition->spacing     == rendition->spacing   &&
        parent_rendition->registry    == rendition->registry) 
      { 
        new->sgml_text.font = parent->sgml_text.font;
      }
    else
      {
        XrmValue from;
        XrmValue to;
        SGMLRendition x;

        /*
         * To get the caching to work we have to make sure the unused space
         * in the structure is zeroed, and only copy font specific components.
         * We could save space in the cache by parsing a structure with only
         * the font components themselves. 
         */
 
        memset(&x,0,sizeof(SGMLRendition));
        x.family   = rendition->family;
        x.weight   = rendition->weight;
        x.slant    = rendition->slant;
        x.width    = rendition->width;
        x.size     = rendition->size;
        x.spacing  = rendition->spacing;
        x.registry = rendition->registry; 

        to.addr = (XtPointer) &new->sgml_text.font;
        to.size = sizeof(new->sgml_text.font); 
        from.addr = (XtPointer) &x;     
        from.size = sizeof(SGMLRendition);

        XtConvertAndStore((Widget)new,"SGMLRendition",&from,"SGMLFontStruct",&to);        
      }

    if (SGMLIsText((Widget) parent) &&
        parent->sgml_text.font16_name == new->sgml_text.font16_name )
      new->sgml_text.font16 = parent->sgml_text.font16;
    else
      {
       if (new->sgml_text.font16_name != XrmStringToQuark("") &&
           new->sgml_text.font16_name != NULL)
         {
            XrmValue from;
            XrmValue to;
            char *fname = XrmQuarkToString(new->sgml_text.font16_name);
            to.addr = (XtPointer) &new->sgml_text.font16;
            to.size = sizeof(new->sgml_text.font16);
            from.addr = (XtPointer) fname;
            from.size = sizeof(char *);
            XtConvertAndStore((Widget)new,XtRString,&from,XtRFontStruct,&to);

          }
        else new->sgml_text.font16 = NULL;
       }

    if (SGMLIsText((Widget) parent)                                   &&
        parent_rendition->color       == rendition->color             &&
        parent->sgml_text.font        == new->sgml_text.font          &&
        parent_rendition->underline_height  
                                      == rendition->underline_height  &&
        parent_rendition->underline_style  
                                      == rendition->underline_style   &&
        parent->sgml_text.font16_name == new->sgml_text.font16_name )
    {    

        new->sgml_text.gc        = parent->sgml_text.gc;
        new->sgml_text.invert_gc = parent->sgml_text.invert_gc;
        new->sgml_text.reverse_gc = parent->sgml_text.reverse_gc;
        new->sgml_text.own_gcs   = FALSE;
        new->sgml_text.font16_gc = parent->sgml_text.font16_gc;
        new->sgml_text.font16_invert_gc = parent->sgml_text.font16_invert_gc;
      }
    else
      {
        XGCValues values;
        XtGCMask  valueMask;
        Pixel background, color; 
        Widget w;
        Visual *Vis; 
               
        for (w=XtParent((Widget) new); !XtIsWidget(w); w= XtParent(w));
        background = w->core.background_pixel; 
        
        valueMask = GCBackground | GCForeground | GCFont | GCLineWidth | GCLineStyle | GCDashList;
  
        if (DefaultDepthOfScreen(XtScreen(w))==1) color = 1 - background;
        else color = rendition->color;
      
        values.background = background;
        values.foreground = color;
        values.font       = new->sgml_text.font->fid;
        values.line_width = rendition->underline_height;
        if      (rendition->underline_style == SGMLLINE_STYLE_SOLID)
          {
            values.line_style = LineSolid;
            values.dashes     = 4;
          }
        else if (rendition->underline_style == SGMLLINE_STYLE_DOTTED) 
          {
            values.line_style = LineOnOffDash;
            values.dashes     = 2;
          }
        else
          {
            values.line_style = LineOnOffDash;
            values.dashes     = 8;
          }

        new->sgml_text.gc = XtGetGC ((Widget) new, valueMask, &values);

        if (new->sgml_text.font16)
          {
            values.font = new->sgml_text.font16->fid;
            new->sgml_text.font16_gc = XtGetGC ((Widget) new, valueMask, &values);
            values.foreground = background;
            values.background = color;

            new->sgml_text.font16_invert_gc = XtGetGC ((Widget) new, valueMask, &values);
          }

        values.font       = new->sgml_text.font->fid;
        values.foreground = background;
        values.background = color;

        new->sgml_text.reverse_gc = XtGetGC ((Widget) new, valueMask, &values);
   
        valueMask = GCForeground | GCFunction | GCLineWidth | GCLineStyle;
        values.foreground = background ^ color;
        values.function   = GXxor;
        new->sgml_text.invert_gc = XtGetGC ((Widget) new, valueMask, &values);

        new->sgml_text.own_gcs   = TRUE;
    
      }
    XTextExtents(new->sgml_text.font," ",1,&dir,&ascent,&descent,&charinfo);

    if (new->sgml_text.font16)
      {
        int temp;
        new->sgml_text.ascent = max(new->sgml_text.font16->ascent, ascent);
        temp = max(descent,2*max(rendition->underline_height,1)* rendition->underline);
        new->sgml_text.descent = max(temp,new->sgml_text.font16->descent);
      }
    else
      {
        new->sgml_text.ascent    = ascent;
        new->sgml_text.descent   = max(descent,2*max(rendition->underline_height,1)*rendition->underline);
      }
    new->sgml_text.spacing = charinfo.width;
}   

/*--------------------------------------------------------------*/
/* FreeFontInfo: (private function)                             */
/*--------------------------------------------------------------*/
static void FreeFontInfo(new)
SGMLTextObject new;
{
    if (new->sgml_text.own_gcs)
      {
        XtReleaseGC((Widget) new, new->sgml_text.gc);
        XtReleaseGC((Widget) new, new->sgml_text.invert_gc);
        XtReleaseGC((Widget) new, new->sgml_text.reverse_gc);
        if (new->sgml_text.font16)
          {
            XtReleaseGC((Widget) new, new->sgml_text.font16_gc);
            XtReleaseGC((Widget) new, new->sgml_text.font16_invert_gc);
          }

        new->sgml_text.own_gcs = FALSE;
      }
}
/*--------------------------------------------------------------*/
/* ClassPartInitialize:                                         */
/*--------------------------------------------------------------*/

static void ClassPartInitialize (class)
SGMLTextObjectClass class;
{
   SGMLTextObjectClass superclass = (SGMLTextObjectClass) class->object_class.superclass; 
      
#define inherit(field,defval) if (class->sgml_text_class.field == defval) \
                                  class->sgml_text_class.field = superclass->sgml_text_class.field

   inherit(compute_size        ,SGMLInheritComputeSize       );
   inherit(adjust_size         ,SGMLInheritAdjustSize        );
   inherit(adjust_position     ,SGMLInheritAdjustPosition    );  
   inherit(compute_size        ,SGMLInheritComputeSize       );
   inherit(expose              ,SGMLInheritExpose            );
   inherit(activate            ,SGMLInheritActivate          );
   inherit(hilite              ,SGMLInheritHilite            );
   inherit(contains            ,SGMLInheritContains          );
   inherit(call_create_callback,SGMLInheritCallCreateCallback);  
   inherit(call_map_callback   ,SGMLInheritCallMapCallback   );
   inherit(make_visible        ,SGMLInheritMakeVisible       );  
   inherit(compute_change_mask ,SGMLInheritComputeChangeMask );  
   inherit(search              ,SGMLInheritSearch            );
   inherit(clear_select        ,SGMLInheritClearSelect       );
   inherit(hilite_selection    ,SGMLInheritHiliteSelection   );
   inherit(xy_to_pos           ,SGMLInheritXYToPos           );
   inherit(pos_to_xy           ,SGMLInheritPosToXY           );
   inherit(dump_text           ,SGMLInheritDumpText          );
#undef inherit
}
/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLTextObject request, new;
{
    SGMLTextObject parent = (SGMLTextObject) XtParent((Widget) new);
    SGMLHyperWidget parentH = (SGMLHyperWidget) parent;
    XrmQuark any = XrmPermStringToQuark("*");
    XrmQuark none = XrmPermStringToQuark("");

    new->sgml_text.num_popups = 0; /* Why ask why? */ 

    new->sgml_text.text   = new->sgml_text.text?XtNewString(new->sgml_text.text):NULL;
    new->sgml_text.length = new->sgml_text.text?strlen(new->sgml_text.text):0;

    new->sgml_text.param        = new->sgml_text.param?XtNewString(new->sgml_text.param):NULL;
    new->sgml_text.param_length = new->sgml_text.param?strlen(new->sgml_text.param):0;

    new->sgml_text.size_valid = FALSE;
    new->sgml_text.font = NULL;

    new->sgml_text.begin_select = NULL;
    new->sgml_text.end_select = NULL;

    new->sgml_text.font16 = NULL;

    /*
     * Many quantities can be inherited from our parent 
     */

#define inherit(mask,field,defval,def) \
    if (new->sgml_text.field == defval) \
      { new->sgml_text.field = (SGMLIsText((Widget) parent)) ? parent->sgml_text.field : def; \
        new->sgml_text.inheritance |= mask; }

    if (SGMLIsText((Widget) parent))         
    {
       new->sgml_text.left_indent  +=  parent->sgml_text.left_indent;
       new->sgml_text.right_indent +=  parent->sgml_text.right_indent;
    }

    inherit(SGMLMLEFT_MARGIN       ,left_margin         ,SGMLINHERIT_MARGIN           ,parentH->sgml_hyper.margin);
    inherit(SGMLMRIGHT_MARGIN      ,right_margin        ,SGMLINHERIT_MARGIN           ,parentH->sgml_hyper.margin);
    inherit(SGMLMPARAGRAPH_INDENT  ,paragraph_indent    ,SGMLINHERIT_PARAGRAPHINDENT  ,0);
    inherit(SGMLMSPACE_BEFORE      ,space_before        ,SGMLINHERIT_SPACE            ,0); 
    inherit(SGMLMSPACE_AFTER       ,space_after         ,SGMLINHERIT_SPACE            ,0);
    inherit(SGMLMBREAK_BEFORE      ,break_before        ,SGMLINHERIT_BREAK            ,SGMLBREAK_SOFT);
    inherit(SGMLMBREAK_AFTER       ,break_after         ,SGMLINHERIT_BREAK            ,SGMLBREAK_SOFT);
    inherit(SGMLMALIGNMENT         ,alignment           ,SGMLINHERIT_ALIGNMENT        ,SGMLALIGNMENT_LEFT);
    inherit(SGMLMSENSITIVE         ,sensitive           ,SGMLINHERIT_SENSITIVITY      ,FALSE;);
    inherit(SGMLMEXACT_TERMINATOR  ,exact_terminator   ,SGMLINHERIT_SENSITIVITY      ,TRUE);
    inherit(SGMLMHIDE_ILLEGAL_FLAGS,hide_illegal_tags  ,SGMLINHERIT_SENSITIVITY      ,FALSE);
    inherit(SGMLMSUPPORTS_ENTITIES ,supports_entities  ,SGMLINHERIT_SENSITIVITY      ,FALSE);

    inherit(SGMLMOUTLINE           ,normal_rendition.outline            ,SGMLINHERIT_OUTLINE        ,FALSE;);
    inherit(SGMLMUNDERLINE         ,normal_rendition.underline          ,SGMLINHERIT_UNDERLINE      ,0);  
    inherit(SGMLMUNDERLINE_HEIGHT  ,normal_rendition.underline_height   ,SGMLINHERIT_UNDERLINEHEIGHT,0);
    inherit(SGMLMUNDERLINE_STYLE   ,normal_rendition.underline_style    ,SGMLINHERIT_UNDERLINESTYLE ,SGMLLINE_STYLE_SOLID);
    inherit(SGMLMCOLOR             ,normal_rendition.color              ,SGMLINHERIT_COLOR          ,FALSE);

    inherit(SGMLMFAMILY            ,normal_rendition.family             ,SGMLINHERIT_QUARK          ,any);
    inherit(SGMLMWEIGHT            ,normal_rendition.weight             ,SGMLINHERIT_QUARK          ,any); 
    inherit(SGMLMSLANT             ,normal_rendition.slant              ,SGMLINHERIT_QUARK          ,any); 
    inherit(SGMLMWIDTH             ,normal_rendition.width              ,SGMLINHERIT_QUARK          ,any); 
    inherit(SGMLMSPACING           ,normal_rendition.spacing            ,SGMLINHERIT_QUARK          ,any); 
    inherit(SGMLMREGISTRY          ,normal_rendition.registry           ,SGMLINHERIT_QUARK          ,any); 
    inherit(SGMLMSIZE              ,normal_rendition.size               ,SGMLINHERIT_SIZE           ,14);
    inherit(SGMLMFONT16            ,font16_name                         ,SGMLINHERIT_QUARK          ,none);
 
    SetFontInfo(new);

#undef inherit
}
/*--------------------------------------------------------------*/
/* Inherit Changes                                              */
/*--------------------------------------------------------------*/

static Boolean InheritChanges(w,inputMask)
SGMLTextObject w;
int inputMask;
{  
    Boolean redraw = FALSE;
    SGMLTextObject parent = (SGMLTextObject) XtParent((Widget) w); 

#define inherit(mask,field) if (mask & inputMask) w->sgml_text.field = parent->sgml_text.field; 

    inherit(SGMLMLEFT_MARGIN       ,left_margin        );
    inherit(SGMLMRIGHT_MARGIN      ,right_margin       );
    inherit(SGMLMPARAGRAPH_INDENT  ,paragraph_indent   );
    inherit(SGMLMSPACE_BEFORE      ,space_before       ); 
    inherit(SGMLMSPACE_AFTER       ,space_after        );
    inherit(SGMLMBREAK_BEFORE      ,break_before       );
    inherit(SGMLMBREAK_AFTER       ,break_after        );
    inherit(SGMLMALIGNMENT         ,alignment          );
    inherit(SGMLMSENSITIVE         ,sensitive          );
    inherit(SGMLMEXACT_TERMINATOR  ,exact_terminator   );
    inherit(SGMLMHIDE_ILLEGAL_FLAGS,hide_illegal_tags  );
    inherit(SGMLMSUPPORTS_ENTITIES ,supports_entities  );

    inherit(SGMLMOUTLINE           ,normal_rendition.outline          );
    inherit(SGMLMUNDERLINE         ,normal_rendition.underline        );  
    inherit(SGMLMUNDERLINE_HEIGHT  ,normal_rendition.underline_height );
    inherit(SGMLMUNDERLINE_STYLE   ,normal_rendition.underline_style  );
    inherit(SGMLMCOLOR             ,normal_rendition.color            );

    inherit(SGMLMFAMILY            ,normal_rendition.family           );
    inherit(SGMLMWEIGHT            ,normal_rendition.weight           ); 
    inherit(SGMLMSLANT             ,normal_rendition.slant            ); 
    inherit(SGMLMWIDTH             ,normal_rendition.width            ); 
    inherit(SGMLMSPACING           ,normal_rendition.spacing          ); 
    inherit(SGMLMREGISTRY          ,normal_rendition.registry         ); 
    inherit(SGMLMSIZE              ,normal_rendition.size             );
    inherit(SGMLMFONT16            ,font16_name                       );
 
#undef inherit
 
    if (inputMask & gcMask) 
      {   
        FreeFontInfo(w);
        SetFontInfo(w);
        redraw = TRUE;
      }
    return redraw;
}

/*--------------------------------------------------------------*/
/* Compute Change Mask                                          */
/*--------------------------------------------------------------*/
 
static int ComputeChangeMask(new,old)
SGMLTextObject new,old;                                        
{ 
    int outputMask = 0;

#define inherit(mask,field) if (old->sgml_text.field != new->sgml_text.field) outputMask |= mask; 

    inherit(SGMLMLEFT_MARGIN       ,left_margin        );
    inherit(SGMLMRIGHT_MARGIN      ,right_margin       );
    inherit(SGMLMPARAGRAPH_INDENT  ,paragraph_indent   );
    inherit(SGMLMSPACE_BEFORE      ,space_before       ); 
    inherit(SGMLMSPACE_AFTER       ,space_after        );
    inherit(SGMLMBREAK_BEFORE      ,break_before       );
    inherit(SGMLMBREAK_AFTER       ,break_after        );
    inherit(SGMLMALIGNMENT         ,alignment          );
    inherit(SGMLMSENSITIVE         ,sensitive          );
    inherit(SGMLMEXACT_TERMINATOR  ,exact_terminator   );
    inherit(SGMLMHIDE_ILLEGAL_FLAGS,hide_illegal_tags  );
    inherit(SGMLMSUPPORTS_ENTITIES ,supports_entities  );

    inherit(SGMLMOUTLINE           ,normal_rendition.outline          );
    inherit(SGMLMUNDERLINE         ,normal_rendition.underline        );  
    inherit(SGMLMUNDERLINE_HEIGHT  ,normal_rendition.underline_height );
    inherit(SGMLMUNDERLINE_STYLE   ,normal_rendition.underline_style  );
    inherit(SGMLMCOLOR             ,normal_rendition.color            );

    inherit(SGMLMFAMILY            ,normal_rendition.family           );
    inherit(SGMLMWEIGHT            ,normal_rendition.weight           ); 
    inherit(SGMLMSLANT             ,normal_rendition.slant            ); 
    inherit(SGMLMWIDTH             ,normal_rendition.width            ); 
    inherit(SGMLMSPACING           ,normal_rendition.spacing          ); 
    inherit(SGMLMREGISTRY          ,normal_rendition.registry         ); 
    inherit(SGMLMSIZE              ,normal_rendition.size             );
    inherit(SGMLMFONT16            ,font16_name                       );
 
#undef inherit
  
    return outputMask;
}

/*--------------------------------------------------------------*/
/* Call the create Callback:                                    */
/*--------------------------------------------------------------*/
 
static void CallCreateCallback(w,event)
SGMLTextObject w;
XEvent         *event;
{
    SGMLTextCallbackStruct cb;

    cb.reason = SGMLTEXT_CREATE;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;
  
    XtCallCallbacks((Widget) w,SGMLNcreateCallback,(XtPointer) &cb);
}
/*--------------------------------------------------------------*/
/* Call the map Callback:                                       */
/*--------------------------------------------------------------*/

static void CallMapCallback(w,event,map)
SGMLTextObject w;
XEvent         *event;
Boolean        map;
{
    SGMLTextCallbackStruct cb;

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
/* Destroy the widget: release all memory alocated              */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLTextObject w;
{
    Widget parent = XtParent((Widget) w);
    /*
     * If our parent is a SGMLCompositeTextObject and is not itself being
     * destroyed then we better let them in on the act 
     */
 
    if (SGMLIsCompositeText(parent) & !parent->core.being_destroyed)
      {
        SGMLCompositeTextDeleteChild((Widget) w); 
      }

    FreeFontInfo(w);

    XtFree(w->sgml_text.text);
    XtFree(w->sgml_text.param);
}

/*------------------------------------------------------------------*/
/* SetValues : redraw only for font or color changes                */
/*------------------------------------------------------------------*/

static Boolean SGMLSetValues (current, request, new, args, num_args)
SGMLTextObject current, request, new;
ArgList args;
Cardinal *num_args;
{
    Boolean    redraw = FALSE;
    int mask = ComputeChangeMask(new,current);
    new->sgml_text.inheritance &= ~mask;
 
#define HAS_CHANGED(a)    (new->sgml_text.a != current->sgml_text.a)

    if (HAS_CHANGED(text))
      {
        new->sgml_text.text   = new->sgml_text.text?XtNewString(new->sgml_text.text):NULL;
        new->sgml_text.length = new->sgml_text.text?strlen(new->sgml_text.text):0;
        XtFree(current->sgml_text.text);
        
        redraw = TRUE;
      }

    if (HAS_CHANGED(param))
      {
        new->sgml_text.param  = new->sgml_text.param?XtNewString(new->sgml_text.param):NULL;
        new->sgml_text.length = new->sgml_text.param?strlen(new->sgml_text.param):0;
        XtFree(current->sgml_text.param);
      }

    if (HAS_CHANGED(managed))
      {
          Widget parent = XtParent((Widget) new);
          SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
          (*parentClass->sgml_composite_text_class.change_managed)((Widget)new);
          redraw = TRUE;
      }

    if (HAS_CHANGED(normal_rendition.underline)) redraw = TRUE;

    if (mask & gcMask)
      {
        FreeFontInfo(new);
        SetFontInfo(new);
        redraw = TRUE;
      }

    return redraw;

#undef HAS_CHANGED

}

static Boolean CallSetValues(class, current, request, new, args, num_args)
SGMLTextObjectClass class;
Widget      current, request, new;
ArgList     args;
Cardinal    *num_args;
{
    Boolean redisplay = FALSE;

    if ((WidgetClass) class != sGMLTextObjectClass)
        redisplay = CallSetValues(class->object_class.superclass, current, request, new, args, num_args);
    if (class->sgml_text_class.sgml_set_values != NULL)
        redisplay |= (*class->sgml_text_class.sgml_set_values) (current, request, new, args, num_args);

    return (redisplay);
}
 
static Boolean SetValues(current, request, new, args, num_args)
SGMLTextObject current, request, new;
ArgList args;
Cardinal *num_args;
{
    Boolean redraw = CallSetValues(XtClass((Widget) new), current, request, new, args, num_args);  
    if (redraw & new->sgml_text.size_valid)
      {
        XtWidgetGeometry dummy;
        if (SGMLIsCompositeText((Widget) new))
          {
            SGMLCompositeTextObjectClass class = (SGMLCompositeTextObjectClass) XtClass((Widget) new);
            XtGeometryResult result;
            result = (*class->sgml_composite_text_class.geometry_manager)((Widget)new,&dummy,&dummy); 
          } 
        else 
          {
            Widget parent = XtParent((Widget) new);
            if (SGMLIsCompositeText(parent))
              {
                SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
                XtGeometryResult result;
                result = (*parentClass->sgml_composite_text_class.geometry_manager)(parent,&dummy,&dummy);
              }
          }
      }
    return redraw;

#undef HAS_CHANGED

}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLTextObject t;
XEvent *event;
Region region;
{
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
  w->sgml_text.height = 0;   
  w->sgml_text.width  = 0;
  w->sgml_text.end    = geom->coord;
  w->sgml_text.begin  = geom->coord;
  w->sgml_text.margin = geom->coord.x;  
  w->sgml_text.size_valid = TRUE; 
}

/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,descent)
SGMLTextObject w;
Position y;
Dimension ascent, descent;
{
  return TRUE;
}

/*--------------------------------------------------------------*/
/* Adjust Position:                                             */
/*--------------------------------------------------------------*/

static void AdjustPosition(w)
SGMLTextObject w;
{
}

/*--------------------------------------------------------------*/
/* Contains:                                                    */
/*--------------------------------------------------------------*/

static Widget Contains(w,x,y,mode)
SGMLTextObject w;
Position x,y;
Boolean mode;
{
 
  if ((w->sgml_text.sensitive || mode ) && 
       w->sgml_text.begin.y < y  &&  w->sgml_text.begin.y + w->sgml_text.height > y && 
       w->sgml_text.margin  < x  &&  w->sgml_text.margin  + w->sgml_text.width   > x )
    {
       if (w->sgml_text.begin.y == w->sgml_text.end.y)
         {
           return (Widget) w;
         } 
       else if (y > w->sgml_text.end.y)
         {
           return (x < w->sgml_text.end.x)?(Widget) w:NULL; 
         }
       else if (y > w->sgml_text.begin.y + w->sgml_text.begin.descent + w->sgml_text.begin.ascent)
         {
            return (Widget) w;
         }
       else
         {
           return (x > w->sgml_text.begin.x)?(Widget) w:NULL; 
         }      
    }    

  return NULL;
}

/*--------------------------------------------------------------*/
/* Hilite:                                                      */
/*--------------------------------------------------------------*/

static void Hilite(t,state)
SGMLTextObject t;
int state;
{
  Display *dpy = XtDisplayOfObject((Widget) t);
  Window  wind = XtWindowOfObject((Widget) t);
  GC        gc = t->sgml_text.invert_gc;
  Position margin = t->sgml_text.margin;
  Dimension depth = t->sgml_text.begin.ascent + t->sgml_text.begin.descent;
  Position bx, by, ex, ey;
  Dimension width, height;
  SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget)t);
  char *text = t->sgml_text.text;
  char *b = t->sgml_text.begin_select;
  char *e = t->sgml_text.end_select;
  
  switch (state)
  {
    case SGMLSELECT_WIDGET:
      bx = t->sgml_text.begin.x; 
      by = t->sgml_text.begin.y; 
      ex = t->sgml_text.end.x;
      ey = t->sgml_text.end.y;
      width  = t->sgml_text.width;
      height = t->sgml_text.height;
      break;
    default:
      if (b == NULL || e == NULL || text == NULL || *text == '\0') return;
      (*class->sgml_text_class.pos_to_xy)(t,&bx,&by,b,FALSE);
      (*class->sgml_text_class.pos_to_xy)(t,&ex,&ey,e,TRUE);
      if (t->sgml_text.begin.ascent > t->sgml_text.ascent)
        {
          if (by == t->sgml_text.begin.y)
            by += (t->sgml_text.begin.ascent - t->sgml_text.ascent);
          if (ey == t->sgml_text.begin.y)
            ey += (t->sgml_text.begin.ascent - t->sgml_text.ascent);
          depth = t->sgml_text.ascent + t->sgml_text.descent;
        }
      if (by == ey)
        {
          width = ex - bx;
          height = depth;
        }
      else
        {
          width = t->sgml_text.width;
          height = ey - by + depth;
        }
      if (width <= 0 || height <=0) return;
      break;
  }

  if (by == ey)
    XFillRectangle(dpy,wind,gc,bx,by,width,height);
  else
    {
      XPoint p[8];

      p[0].x = bx;
      p[0].y = by;
      p[1].x = margin + width;
      p[1].y = p[0].y;
      p[2].x = p[1].x;
      p[2].y = ey;
      p[3].x = ex;
      p[3].y = p[2].y;
      p[4].x = p[3].x;
      p[4].y = ey + depth;
      p[5].x = margin;
      p[5].y = p[4].y;
      p[6].x = p[5].x;
      p[6].y = by + depth;
      p[7].x = p[0].x;
      p[7].y = p[6].y;
      
      XFillPolygon(dpy,wind,gc,p,XtNumber(p),Nonconvex,CoordModeOrigin);
    }
}

/*--------------------------------------------------------------*/
/* MakeVisible:                                                 */
/*--------------------------------------------------------------*/

static void MakeVisible(w)
SGMLTextObject w;
{
}

/*--------------------------------------------------------------*/
/* Activate:                                                    */
/*--------------------------------------------------------------*/

static void Activate(w,event)
SGMLTextObject w;
XEvent *event;
{
    SGMLTextCallbackStruct cb;

    /* Fill callback struct */

    cb.text          = w->sgml_text.text;
    cb.length        = w->sgml_text.length;
    cb.param         = w->sgml_text.param;
    cb.param_length  = w->sgml_text.param_length;
    cb.reason        = SGMLTEXT_ACTIVATE;
    cb.event         = event;

    XtCallCallbacks ((Widget) w, SGMLNactivateCallback,(XtPointer) &cb);
}
 
/*--------------------------------------------------------------*/
/* _SGMLBreak:                                                  */
/*--------------------------------------------------------------*/

void _SGMLBreak(geom,space)
SGMLGeometry *geom;
Dimension space; 
{
  if (!geom->broken)
  {
    geom->coord.y += geom->coord.descent + geom->coord.ascent;
    geom->coord.y += space;
    geom->space = space;
    geom->broken = TRUE;
  }
  else
  {
    if (space > geom->space) 
    {  
      geom->coord.y += space - geom->space;
      geom->space = space;
    }
  }
  geom->coord.x = 0;
  geom->coord.descent = 0;
  geom->coord.ascent = 0;
  geom->alignment = SGMLALIGNMENT_NONE; 
  geom->leave_space = FALSE; 
}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLTextObject                                           */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLTextObjectClass,parent,al,ac);
}

/*-----------------------------------------------------------------------*/
/* Search: search a string                                               */
/*-----------------------------------------------------------------------*/

static Widget Search(exp,sens,w,mode,extsearch,closure,sw)
char *exp;
Boolean sens;
SGMLTextObject w;
Boolean mode;
Widget (*extsearch)();
XtPointer closure;
SGMLTextObject *sw;
{
  int i;
  char *p, *d, *f = w->sgml_text.text;
  char *delim = " \t\n\r";
  int dc, pc;
  Widget result;

  /* skip all the widgets have been searched */
  if (*sw) 
    {
      if (*sw == w)
        {
          f = w->sgml_text.end_select + 1;
          *sw = NULL;
        }
      else 
        {
          if (w->sgml_text.begin_select) {
            w->sgml_text.begin_select = NULL;
            w->sgml_text.end_select = NULL;
            }
          return NULL;
        }
    }

  /* strip off delim chars of the search string and data */
  for (;f && *f != '\0' &&  strchr(delim,*f) ;f++);
  for (;exp && *exp != '\0' &&  strchr(delim,*exp); exp++);

  if (XtClass((Widget) w) == sGMLTextObjectClass)
    {
      /* skip the widget if it is a textobjectclass (ie: <title>) */
      w->sgml_text.begin_select = NULL;
      w->sgml_text.end_select = NULL;
      if (mode) return NULL;
      else return extsearch(exp,sens,closure);
    }
  else
    {
      if (mode) /* full search */
        {
         if (f != NULL && *f != '\0')
          {
          for (; *f != '\0'; f++)
            {
              for(d=f,p=exp; ;d++, p++)
                {
                  if (*p == '\0') 
                    {
                      w->sgml_text.begin_select = f;
                      w->sgml_text.end_select = d - 1;
                      return (Widget)w;
                    }
                  if (*d == '\0')
                    {
                      if (result = extsearch(p,sens,closure))
                        {
                          while (strchr(delim,*(--d)));  
                          w->sgml_text.begin_select = f;
                          w->sgml_text.end_select = d;
                          return result;
                        }
                      else break;
                     }
                  if (!sens)
                    {
                      dc = tolower(*d);
                      pc = tolower(*p);
                    }
                  else
                    {
                      dc = *d;
                      pc = *p;
                    }
                  if (*p == ' ' && strchr (delim,*d))
                    while (strchr(delim,*(d+1)) && *(d+1)!='\0') d++;
                  else if (pc != dc) break;
                }
             }
            }
           w->sgml_text.begin_select = NULL;
           w->sgml_text.end_select = NULL;
           return NULL;
         }
      else /* one loop match */
        {
          if (f != NULL && *f != '\0')
            {
              for (p=exp, d=f;  ;p++, d++)
                {
                   if (*p == '\0') 
                     {
                       w->sgml_text.begin_select = f;
                       w->sgml_text.end_select = d - 1;
                       return (Widget)w;
                     }
                   if (*d == '\0') break; 
                   if (!sens)
                     {
                       dc = tolower(*d);
                       pc = tolower(*p);
                     }
                   else
                     {
                       dc = *d;
                       pc = *p;
                     }
                   if (*p == ' ' && strchr (delim,*d))
                      while (strchr(delim,*(d+1)) && *(d+1)!='\0') d++;
                   else if (pc != dc)
                     {
                       w->sgml_text.begin_select = NULL;
                       w->sgml_text.end_select = NULL;
                       return NULL;
                     }
                 }
               if (result = extsearch(p,sens,closure))
                 {
                   while (strchr(delim,*(--d)));
                   w->sgml_text.begin_select = f;
                   w->sgml_text.end_select = d;
                 }
               return result;
             }
           else return extsearch(exp,sens,closure);
        }
    }
}

/*-----------------------------------------------------------------------*/
/* Clear Select: clear the selected string from search                   */
/*-----------------------------------------------------------------------*/

static Boolean ClearSelect(w,sw,mode)
SGMLTextObject w;
SGMLTextObject sw;
Boolean mode;
{
  if (w->sgml_text.begin_select == NULL ||
      w->sgml_text.end_select == NULL ) return FALSE;

  if (mode && w->sgml_text.begin_select)
    {
      SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget)w);

     (*class->sgml_text_class.hilite)(w,SGMLSELECT_WORD);
    } 

      
  w->sgml_text.begin_select = NULL;
  w->sgml_text.end_select = NULL;
  if (w == sw)
    return TRUE;
  else return FALSE;
}

static Widget HiliteSelection(w,mode,ci)
SGMLTextObject w;
int mode;
SGMLCopyInfo *ci;
{
   return NULL;
}


static void XYToPos(w,x,y,pos)
SGMLTextObject w;
int *x, *y;
char **pos;
{
}

static void PosToXY(w,x,y,pos,after)
SGMLTextObject w;
Position *x, *y;
char *pos;
Boolean after;
{
}

static Boolean DumpText(w,dp,pp)
SGMLTextObject w;
SGMLDumpTextInfo *dp;
char **pp;
{
   Dimension space_before = w->sgml_text.space_before;
   Dimension space_after = w->sgml_text.space_after;
   int break_before = w->sgml_text.break_before;
   int break_after = w->sgml_text.break_after;
   
   if (dp->begin_sw)
      {
        if (break_before == SGMLBREAK_ALWAYS || break_after == SGMLBREAK_ALWAYS)  
          {
            int nl = (int)max(space_before,space_after);
            dp->line_spacing = 1; 
            for (; nl > 0; nl-=10) dp->line_spacing++;
          }

      } 

    return FALSE;
}

/*
 * External functions
 */
Widget ExtHiliteSelection(ww,mode,ci,plaintext)
Widget ww;
int mode;
SGMLCopyInfo *ci;
Boolean plaintext;
{
   SGMLTextObject w = (SGMLTextObject)ww;
   char *pos = NULL;
   char *delim = " \r\n\t";
   char *text = w->sgml_text.text;
   SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget)w);
   char *b, *e, *eot;
   int x = ci->current_x;
   int y = ci->current_y;


   /* strip off the blank chars */
   if (!plaintext)
     for (;text != NULL && *text != '\0' && strchr(delim,*text);text++);

   if (*text == '\0') return NULL;

   if (mode == SGMLSELECT_WORD)
     {

       (*class->sgml_text_class.xy_to_pos)(w,&x,&y,&pos);
       b = e = pos;
       if (iscntrl(*pos)) return NULL;

       if (ispunct(*pos))
         {
           w->sgml_text.begin_select = pos;
           w->sgml_text.end_select = pos;
         }
       else if (strchr(delim,*pos))
         {
           for (;b != text && strchr(delim,*b); b--);

           for (;*e!= '\0' && strchr(delim,*e); e++);

           if (b != text) b++;
           w->sgml_text.begin_select = b;
           w->sgml_text.end_select = --e;
         }
       else
         { 
           for (;b != text; b--)
             if (strchr(delim,*b) || ispunct(*b) || iscntrl(*b) ) break;

           for (;;e++)
             if (strchr(delim,*e) || ispunct(*e) || iscntrl(*e) ) break;

           if (b != text) b++;
           w->sgml_text.begin_select = b;
           w->sgml_text.end_select = --e;

         }

       (*class->sgml_text_class.hilite)(w,SGMLSELECT_WORD);

       /* set the ci flags for the SelectEnd */
       ci->sw_flag = TRUE;
       ci->last_mode = FALSE;
       ci->last_widget = (Widget) w;

       return (Widget) w;
     }

   /* Possible do the SGMLSELECT_LINE here also ? */

   else if (mode == SGMLSELECT_REGION)
     {
       char *tempb = NULL, *tempe = NULL;
       SGMLTextObject lastw = (SGMLTextObject) ci->last_widget;
       SGMLTextObject currentw = (SGMLTextObject) ci->current_widget;
       int lastx = ci->last_x;
       int lasty = ci->last_y;
       Boolean unsel, revsel;
    

       if (x == lastx && y == lasty) return (Widget) lastw;

       if (y < ci->first_y || (x < ci->first_x && y <= ci->first_y))
         {
           revsel = TRUE;
           ci->current_mode = TRUE;
         }
       else 
         {
           revsel = FALSE;
           ci->current_mode = FALSE;
         }

       /* clear the selection when reverse <-> direct select */
       if (ci->current_mode != ci->last_mode)
         { 
           SGMLTextObjectClass cclass = (SGMLTextObjectClass) XtClass(ci->cw);
           Widget search_widget;

           search_widget = ci->last_mode ? ci->first_widget : ci->last_widget;
           (*cclass->sgml_text_class.clear_select)(ci->cw,search_widget,TRUE);
           ci->last_mode = ci->current_mode;
           ci->last_widget = ci->first_widget;
           lastw = (SGMLTextObject) ci->first_widget;
           if (w == lastw && w != currentw) return NULL;
         }

       if (w != lastw && w != currentw && !ci->flag) return NULL;

       pos = ci->last_pos;
       ci->sw_flag = TRUE;

       if (revsel) /* reverse select */
       {


       if (y < lasty || (x < lastx && y <= lasty))
         unsel = FALSE;
       else unsel = TRUE;

       if (w == lastw && w == currentw)
         {
           if ((lasty == ci->first_y && lastx == ci->first_x) && !w->sgml_text.begin_select)
             {
               w->sgml_text.begin_select = ci->first_char;
               w->sgml_text.end_select = ci->first_char;
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             }
           if (unsel)
             {
               tempe = w->sgml_text.end_select;
               w->sgml_text.end_select = pos-1;
             }
           else
             {
               if (w->sgml_text.begin_select == NULL) /* in case of switching mode */
                 {
                   w->sgml_text.begin_select = pos;
                   w->sgml_text.end_select = ci->first_char;
                 }
               else
                 {
                   tempe = w->sgml_text.end_select;
                   w->sgml_text.end_select = w->sgml_text.begin_select -1;
                   w->sgml_text.begin_select = pos;
                 }
             }

           (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
           if (tempe)
             w->sgml_text.end_select = tempe;
           w->sgml_text.begin_select = pos;
          
           return (Widget) w;
         }
       else if (w == lastw)
         {
           if (unsel)
           {
             if (w->sgml_text.begin_select)
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.begin_select = NULL;
             w->sgml_text.end_select = NULL;
             ci->flag = TRUE;
             return NULL;
           }
           else
           {
             if (!w->sgml_text.begin_select)
             {
               w->sgml_text.begin_select = ci->first_char;
               w->sgml_text.end_select = ci->first_char;
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             }

             ci->flag = FALSE;
             tempe = w->sgml_text.end_select;
             if (w->sgml_text.begin_select != text)
               w->sgml_text.end_select = w->sgml_text.begin_select -1;
             else
               return (Widget) w;
             w->sgml_text.begin_select = text;
             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.end_select = tempe;
             return (Widget) w;
           }
         }
       else if (w == currentw)
         {
           if (unsel)
           {
             ci->flag = FALSE;
             tempe = w->sgml_text.end_select;
             if (pos != text)
               w->sgml_text.end_select = pos - 1;
             else
               return (Widget) w;

             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.begin_select = pos;
             w->sgml_text.end_select = tempe;
             return (Widget)w;
           }
           else
           {
             w->sgml_text.begin_select = pos;
             for (eot = pos; *eot != '\0';eot++);
             for (;strchr(delim,*eot);eot--);
             w->sgml_text.end_select = eot;
             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             ci->flag = TRUE;
             return NULL;
           }
         }
       
       }
       else  /* direct select */
       {


       if (y > lasty || (x > lastx && y >= lasty))
         unsel = FALSE;
       else unsel = TRUE;

       if (w == lastw && w == currentw)
         {
           if ((lasty == ci->first_y && lastx == ci->first_x) && !w->sgml_text.begin_select)
             {
               w->sgml_text.begin_select = ci->first_char;
               w->sgml_text.end_select = ci->first_char;
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION); 
             } 
           if (unsel)
             {
               tempb = w->sgml_text.begin_select;
               if (*pos != '\0')
                 w->sgml_text.begin_select = pos +1;
               else
                 return (Widget) w;
             }
           else
             {
               if (w->sgml_text.begin_select == NULL)  /* in case of switching mode only */
                 w->sgml_text.begin_select = ci->first_char;
               else
                 {
                   tempb = w->sgml_text.begin_select;
                   if (*w->sgml_text.end_select != '\0')
                     w->sgml_text.begin_select = w->sgml_text.end_select +1;
                   else 
                     return (Widget) w;
                 }
               w->sgml_text.end_select = pos;
             }

           (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
           if (tempb)
             w->sgml_text.begin_select = tempb;

           w->sgml_text.end_select = pos;

           return (Widget) w;

         } 
       else if (w == lastw)
         {
           if (unsel)
           {
             if (w->sgml_text.begin_select)
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.begin_select = NULL;
             w->sgml_text.end_select = NULL;
             ci->flag = FALSE;
             return (Widget)currentw;
           }
           else
           {
             if (!w->sgml_text.begin_select)
             {
               w->sgml_text.begin_select = ci->first_char;
               w->sgml_text.end_select = ci->first_char;
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             } 

             ci->flag = TRUE;
             tempb = w->sgml_text.begin_select;
             if (w->sgml_text.end_select != '\0')
               w->sgml_text.begin_select = w->sgml_text.end_select +1;
             else 
               return NULL;
             for (eot = w->sgml_text.end_select;*eot != '\0';eot++); 
             for (; strchr(delim,*eot)!= NULL && eot >= w->sgml_text.begin_select; eot--);
             w->sgml_text.end_select = eot;
             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.begin_select = tempb;
             return NULL;
           }
         }
       else if (w == currentw)
         {
           if (unsel)
           {
             ci->flag = TRUE;
             tempb = w->sgml_text.begin_select;
             if (*pos != '\0')
               w->sgml_text.begin_select = pos + 1;
             else 
               return NULL;
              
             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.begin_select = tempb;
             w->sgml_text.end_select = pos;
             return NULL;
           } 
           else
           {
             w->sgml_text.begin_select = text;
             w->sgml_text.end_select = pos;
             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             ci->flag = FALSE;
             return (Widget) w;
           }
         }
       }

       if (ci->flag)
         {
           if (unsel)
           {
             if (w->sgml_text.begin_select && w->sgml_text.end_select)
               (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
             w->sgml_text.begin_select = NULL;
             w->sgml_text.end_select = NULL;
           }
           else
           {
             if (*text == '\0') return NULL;
             w->sgml_text.begin_select = text;
             for (eot = text ; *eot != '\0'; eot++);
             for (; strchr(delim,*eot)!= NULL; eot--);
             w->sgml_text.end_select = eot;
             (*class->sgml_text_class.hilite)(w,SGMLSELECT_REGION);
           }

           return NULL;
         }
     } 
   else /* select all */
     {
       if (text == NULL || *text == '\0') return NULL; 

       w->sgml_text.begin_select = b = text;
       for (e = text; *e != '\0'; e++);
       if (!plaintext)
         for (;e > b && strchr(delim,*e)!=NULL; e--);
       w->sgml_text.end_select = e;
       (*class->sgml_text_class.hilite)(w,SGMLSELECT_ALL);
       ci->sw_flag = TRUE;
       ci->last_widget = (Widget) w; 
     }

   return NULL;
}
