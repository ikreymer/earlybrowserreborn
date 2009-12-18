/*==================================================================*/
/*                                                                  */
/* SGMLHyperWidget                                                  */
/*                                                                  */
/* B.Raoult (mab@ecmwf.co.uk)                              Oct.91   */
/* T.Johnson - added SGML facilities                      June.92   */
/*             (Tony Johnson)                                       */
/*                                                                  */
/* Hyper text like widget.                                          */
/*                                                                  */
/*==================================================================*/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/ScrolledW.h>
#include <X11/Xatom.h>

#ifdef VMS
#include <Xmu/Atoms.h>
#ifdef UCX
#define bcopy(a, b, c) (void) memmove ((void *) (b), (void *) (a), (size_t) (c))
#endif
#else
#include <X11/Xmu/Atoms.h>
#endif

#include "SGMLText.h"
#include "SGMLPlainText.h"
#include "SGMLFormattedText.h"
#include "SGMLFormText.h"
#include "SGMLCaptionText.h"
#include "SGMLTableText.h"
#include "SGMLCompositeText.h"
#include "SGMLContainerTextP.h"
#include "SGMLAnchorText.h"
#include "SGMLMarkerText.h"
#include "SGMLRuleText.h"
#include "SGMLInputText.h"
#include "SGMLListText.h"
#include "SGMLHyperP.h"

#ifndef ABS
#define ABS(a)           ((a)>=0?(a):-(a))
#endif
#ifndef MIN
#define MIN(a,b)         ((a)>(b)?(b):(a))
#endif
#define MAX_LINE_SIZE    1024

/* 	Entity Names
**	------------
**
**	This table must be matched exactly the table below 
*/
static char* entities[] = {
  "AElig",	/* capital AE diphthong (ligature) */ 
  "Aacute",	/* capital A, acute accent */ 
  "Acirc",	/* capital A, circumflex accent */ 
  "Agrave",	/* capital A, grave accent */ 
  "Aring",	/* capital A, ring */ 
  "Atilde",	/* capital A, tilde */ 
  "Auml",	/* capital A, dieresis or umlaut mark */ 
  "Ccedil",	/* capital C, cedilla */ 
  "ETH",	/* capital Eth, Icelandic */ 
  "Eacute",	/* capital E, acute accent */ 
  "Ecirc",	/* capital E, circumflex accent */ 
  "Egrave",	/* capital E, grave accent */ 
  "Euml",	/* capital E, dieresis or umlaut mark */ 
  "Iacute",	/* capital I, acute accent */ 
  "Icirc",	/* capital I, circumflex accent */ 
  "Igrave",	/* capital I, grave accent */ 
  "Iuml",	/* capital I, dieresis or umlaut mark */ 
  "Ntilde",	/* capital N, tilde */ 
  "Oacute",	/* capital O, acute accent */ 
  "Ocirc",	/* capital O, circumflex accent */ 
  "Ograve",	/* capital O, grave accent */ 
  "Oslash",	/* capital O, slash */ 
  "Otilde",	/* capital O, tilde */ 
  "Ouml",	/* capital O, dieresis or umlaut mark */ 
  "THORN",	/* capital THORN, Icelandic */ 
  "Uacute",	/* capital U, acute accent */ 
  "Ucirc",	/* capital U, circumflex accent */ 
  "Ugrave",	/* capital U, grave accent */ 
  "Uuml",	/* capital U, dieresis or umlaut mark */ 
  "Yacute",	/* capital Y, acute accent */ 
  "aacute",	/* small a, acute accent */ 
  "acirc",	/* small a, circumflex accent */ 
  "aelig",	/* small ae diphthong (ligature) */ 
  "agrave",	/* small a, grave accent */ 
  "amp",	/* ampersand */ 
  "aring",	/* small a, ring */ 
  "atilde",	/* small a, tilde */ 
  "auml",	/* small a, dieresis or umlaut mark */ 
  "ccedil",	/* small c, cedilla */ 
  "eacute",	/* small e, acute accent */ 
  "ecirc",	/* small e, circumflex accent */ 
  "egrave",	/* small e, grave accent */ 
  "eth",	/* small eth, Icelandic */ 
  "euml",	/* small e, dieresis or umlaut mark */ 
  "gt",		/* greater than */ 
  "iacute",	/* small i, acute accent */ 
  "icirc",	/* small i, circumflex accent */ 
  "igrave",	/* small i, grave accent */ 
  "iuml",	/* small i, dieresis or umlaut mark */ 
  "lt",		/* less than */ 
  "ntilde",	/* small n, tilde */ 
  "oacute",	/* small o, acute accent */ 
  "ocirc",	/* small o, circumflex accent */ 
  "ograve",	/* small o, grave accent */ 
  "oslash",	/* small o, slash */ 
  "otilde",	/* small o, tilde */ 
  "ouml",	/* small o, dieresis or umlaut mark */ 
  "szlig",	/* small sharp s, German (sz ligature) */ 
  "thorn",	/* small thorn, Icelandic */ 
  "uacute",	/* small u, acute accent */ 
  "ucirc",	/* small u, circumflex accent */ 
  "ugrave",	/* small u, grave accent */ 
  "uuml",	/* small u, dieresis or umlaut mark */ 
  "yacute",	/* small y, acute accent */ 
  "yuml",	/* small y, dieresis or umlaut mark */ 
};
/* 	Entity values -- for ISO Latin 1 local representation
**
**	This MUST match exactly the table above!
*/
static int ISO_Latin1[] = {
  	0306,	/* capital AE diphthong (ligature) */ 
  	0301,	/* capital A, acute accent */ 
  	0302,	/* capital A, circumflex accent */ 
  	0300,	/* capital A, grave accent */ 
  	0305,	/* capital A, ring */ 
  	0303,	/* capital A, tilde */ 
  	0304,	/* capital A, dieresis or umlaut mark */ 
  	0307,	/* capital C, cedilla */ 
  	0320,	/* capital Eth, Icelandic */ 
  	0311,	/* capital E, acute accent */ 
  	0312,	/* capital E, circumflex accent */ 
  	0310,	/* capital E, grave accent */ 
  	0313,	/* capital E, dieresis or umlaut mark */ 
  	0315,	/* capital I, acute accent */ 
  	0316,	/* capital I, circumflex accent */ 
  	0314,	/* capital I, grave accent */ 
  	0317,	/* capital I, dieresis or umlaut mark */ 
  	0321,	/* capital N, tilde */ 
  	0323,	/* capital O, acute accent */ 
  	0324,	/* capital O, circumflex accent */ 
  	0322,	/* capital O, grave accent */ 
  	0330,	/* capital O, slash */ 
  	0325,	/* capital O, tilde */ 
  	0326,	/* capital O, dieresis or umlaut mark */ 
  	0336,	/* capital THORN, Icelandic */ 
  	0332,	/* capital U, acute accent */ 
  	0333,	/* capital U, circumflex accent */ 
  	0331,	/* capital U, grave accent */ 
  	0334,	/* capital U, dieresis or umlaut mark */ 
  	0335,	/* capital Y, acute accent */ 
  	0341,	/* small a, acute accent */ 
  	0342,	/* small a, circumflex accent */ 
  	0346,	/* small ae diphthong (ligature) */ 
  	0340,	/* small a, grave accent */ 
  	0046,	/* ampersand */ 
  	0345,	/* small a, ring */ 
  	0343,	/* small a, tilde */ 
  	0344,	/* small a, dieresis or umlaut mark */ 
  	0347,	/* small c, cedilla */ 
  	0351,	/* small e, acute accent */ 
  	0352,	/* small e, circumflex accent */ 
  	0350,	/* small e, grave accent */ 
  	0360,	/* small eth, Icelandic */ 
  	0353,	/* small e, dieresis or umlaut mark */ 
  	0076,	/* greater than */ 
  	0355,	/* small i, acute accent */ 
  	0356,	/* small i, circumflex accent */ 
  	0354,	/* small i, grave accent */ 
  	0357,	/* small i, dieresis or umlaut mark */ 
  	0074,	/* less than */ 
  	0361,	/* small n, tilde */ 
  	0363,	/* small o, acute accent */ 
  	0364,	/* small o, circumflex accent */ 
  	0362,	/* small o, grave accent */ 
  	0370,	/* small o, slash */ 
  	0365,	/* small o, tilde */ 
  	0366,	/* small o, dieresis or umlaut mark */ 
  	0337,	/* small sharp s, German (sz ligature) */ 
  	0376,	/* small thorn, Icelandic */ 
  	0372,	/* small u, acute accent */ 
  	0373,	/* small u, circumflex accent */ 
  	0371,	/* small u, grave accent */ 
  	0374,	/* small u, dieresis or umlaut mark */ 
  	0375,	/* small y, acute accent */ 
  	0377,	/* small y, dieresis or umlaut mark */ 
};

#define SGML_ENTITIES 65
/* 
  Private functions 
*/

static void    create_gcs();
static void    selectt();
static void    cursor();
static void    activate();
static Widget  add_to_text();
static void    set_tag();
static void    calc_new_size ();
static void    show_selection();
static void    lowcase();
static void    SelectWord();
static void    SelectLine();
static void    SelectRegion();
static void    SelectAll();
static void    SelectStart();
static void    SelectExtend();
static void    SelectEnd();
static Boolean Convert();
static void    LoseOwnership();
static void    SelectionDone();
static Boolean GetSelection();
static void    _OwnSelection();

static int              NumberOfClasses = 0;
static WidgetClass     *ClassList = NULL; 
static XrmName          NullQuark;

/*
  Widget class methods
*/

static void    DeleteChild();
static void    InsertChild();
static void    ClassInitialize();
static void    Initialize();
static void    Redisplay();
static void    Resize();
static void    Destroy();
static Boolean SetValues();
static XtGeometryResult GeometryManager();

static XrmName dl_quark;
static XrmName dlc_quark;

static char defaultTranslations[] = 
"!<Btn1Down>: selectstart()          \n\
 !<Btn1Motion>: selectextend()       \n\
 !<Btn1Up>: selectend(PRIMARY, CUT_BUFFER0)              \n\
 !Ctrl <Btn3Up>: selectall(PRIMARY, CUT_BUFFER0)         \n\
 <Motion>:cursor()                   ";

static XtActionsRec actionsList[] = {
    { "selectstart",   (XtActionProc) SelectStart},
    { "selectextend",  (XtActionProc) SelectExtend},
    { "selectend",  (XtActionProc) SelectEnd},
    { "selectall",  (XtActionProc) SelectAll},
    { "cursor",   (XtActionProc) cursor},
};

#define Offset(field) XtOffsetOf(SGMLHyperRec, sgml_hyper.field)

static XtResource resources[] = {

    {SGMLNactivateCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (activate),XtRCallback,NULL},

    {SGMLNcaseSensitiveTags,SGMLCCaseSensitiveTags,XtRBoolean,sizeof(Boolean),
    Offset (case_sensitive_tags),XtRImmediate,(XtPointer)FALSE},

    {SGMLNcacheSize,SGMLCCacheSize,XtRInt,sizeof(int),
    Offset (cache_size),XtRImmediate,(XtPointer)0},

    {SGMLNopenTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(open_tag),XtRImmediate,(XtPointer)'<'},

    {SGMLNcloseTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(close_tag),XtRImmediate,(XtPointer)'>'},

    {SGMLNendTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(end_tag),XtRImmediate,(XtPointer)'/'},

    {SGMLNparameterTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(parameter_tag),XtRImmediate,(XtPointer)' '},

    {SGMLNentityTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(entity_tag),XtRImmediate,(XtPointer)'&'},

    {SGMLNentityEndTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(entity_end_tag),XtRImmediate,(XtPointer)';'},

    {SGMLNmargin,SGMLCMargin,XtRDimension,sizeof(Dimension),
    Offset (margin),XtRImmediate,(XtPointer)10},

    {SGMLNnaturalWidth,SGMLCWidth,XtRDimension,sizeof(Dimension),
    Offset (natural_width),XtRImmediate,(XtPointer)500},
};

#undef Offset

/* stack pointer */

static int sp = 0;

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

static CompositeClassExtensionRec compositeExtension = {
    NULL,                                 /* next_extension   */
    NULLQUARK,                            /* record_type      */
    XtCompositeExtensionVersion,          /* version          */
    sizeof(CompositeClassExtensionRec),   /* record_size      */
    TRUE				  /* accepts_objects  */  
}; 

SGMLHyperClassRec  sGMLHyperClassRec = {
    {
#ifdef MOTIF
    (WidgetClass) &xmManagerClassRec,    /* superclass            */
#else
    (WidgetClass) &compositeClassRec,    /* superclass            */
#endif
    "SGMLHyper",                         /* class_name            */
    sizeof(SGMLHyperRec),                /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    Initialize,                          /* initialize            */
    NULL,                                /* initialize_hook       */
    XtInheritRealize,                    /* realize               */
    actionsList,                         /* actions               */
    XtNumber(actionsList),               /* num_actions           */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    TRUE,                                /* compress_motion       */
    TRUE,                                /* compress_exposure     */
    TRUE,                                /* compress_enterleave   */
    TRUE,                                /* visible_interest      */
    Destroy,                             /* destroy               */
    Resize,                              /* resize                */
    Redisplay,                           /* expose                */
    SetValues,                           /* set_values            */
    NULL,                                /* set_values_hook       */
    XtInheritSetValuesAlmost,            /* set_values_almost     */
    NULL,                                /* get_values_hook       */
    NULL,                                /* accept_focus          */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    defaultTranslations,                 /* tm_table              */
    NULL,                                /* query_geometry        */
    NULL,                                /* display_accelerator   */
    NULL,                                /* extension             */
    },
    {
    GeometryManager,                     /* geometry manager      */
    NULL,				 /* change_managed        */
    InsertChild,                         /* insert_child          */
    DeleteChild,                         /* delete_child          */
    (XtPointer) &compositeExtension      /* extension             */
    },
#ifdef MOTIF
   {		/* constraint_class fields */
      NULL,					/* resource list        */   
      0,					/* num resources        */   
      0,					/* constraint size      */   
      NULL,					/* init proc            */   
      NULL,             			/* destroy proc         */   
      NULL,					/* set values proc      */   
      NULL,                                     /* extension            */
   },    

   {
    XtInheritTranslations,                /* translations                  */
    NULL,                                 /* syn_resources                 */
    0,                                    /* num_syn_resources             */
    NULL,                                 /* syn_constraint_resources      */
    0,                                    /* num_syn_constraint_resources  */
    NULL,                                 /* parent_process                */
    NULL,                                 /* extension                     */
    },
#endif
    {
    0,                                    /* ignore                */
    }
};

static int EntityToIsoLatin1(entity)
char *entity;
{
  int high, low, i, diff;

  for (low = 0, high = SGML_ENTITIES ; high > low ; 
       diff < 0 ? (low = i+1) : (high = i))     /* Binary search */
    { 
	i = (low + (high-low)/2);
	diff = strcmp(entities[i], entity);	/* Case sensitive! */
	if (diff==0) {				/* success: found it */
	    return ISO_Latin1[i];
	}
    }
  return '\0'; 
}
static Boolean IsSubClass(sub,class)
WidgetClass sub, class;
{
  for (; sub != NULL; sub = sub->core_class.superclass)
    if (sub == class) return TRUE;
   
  return FALSE;
}

WidgetClass sGMLHyperWidgetClass = (WidgetClass) &sGMLHyperClassRec;


static void CvtStringToClass (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    char *s;
    int p;
       
    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToClass","XtToolkitError",
                  "String to Class conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = (char *) fromVal->addr;
    
    for (p=0; p<NumberOfClasses; p++)
      if (!strcmp(s,ClassList[p]->core_class.class_name)) 
        {
          toVal->size = sizeof(WidgetClass);
          toVal->addr = (XtPointer) &ClassList[p];
          return;
        }     
    XtStringConversionWarning((char *) fromVal->addr, SGMLRClass);
}
static void CvtStringToTagList (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    int polarity;
    char *s, *t, *p;
    WidgetClass current_class = NULL;
       
    Cardinal num_slots = 10;
    Cardinal num_used = 0; 
    static SGMLTagList *list;
    
    list = (SGMLTagList *) XtMalloc(sizeof(SGMLTagList) * num_slots);

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToTagList","XtToolkitError",
                  "String to Tag List conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = XtNewString((char *) fromVal->addr);

    t = strtok(s," \t");
    if (t && !strcmp(t,"*"))
      {
        polarity = 1;        
        t = strtok(NULL," \t");
      }
    else polarity = 0;   

    for (; t != NULL; t = strtok(NULL," \t"))
      {
        if ((p = strchr(t,':')) == NULL)
          {
            XrmName tag;
            int sign = 1, match = 1;
            
            if      (*t == '+') { sign =  1; t++; }
            else if (*t == '-') { sign = -1; t++; }
            
            if      (*t == '%') { match = 0; t++; }

            tag = XrmStringToQuark(t);
            if (num_used == num_slots) 
              list = (SGMLTagList *) XtRealloc((XtPointer) list, sizeof(SGMLTagList) * (num_slots *= 2)); 
              
            list[num_used].name = tag;
            list[num_used].class = current_class;  
            list[num_used].polarity = polarity * sign;
            list[num_used].matched = match;
            num_used++;
          }
        else   
          {
             int i;

             *p = '\0';
             current_class = NULL; 
 
             for (i=0; i<NumberOfClasses; i++)
               if (!strcmp(t,ClassList[i]->core_class.class_name)) 
                 current_class = ClassList[i];
                 
             if (current_class == NULL)
               XtStringConversionWarning(t, SGMLRClass); 
          } 
      }
    if (num_used == num_slots) 
       list = (SGMLTagList *) XtRealloc((XtPointer) list, sizeof(SGMLTagList) * (++num_slots)); 

    list[num_used].name = 0;
    list[num_used].class = NULL;
    list[num_used].polarity = 0;  
    num_used++; 
    toVal->addr = (XtPointer) &list;
    toVal->size = sizeof(XtPointer);
    
    XtFree(s);
}

static Boolean ConvertStringToString(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    int size = sizeof(from->addr);
    if (to->addr == 0)
      {
         to->addr = (XtPointer) &from->addr;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         memcpy(to->addr,&from->addr,size);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}

/*--------------------------------------------------------------*/
/* ClassInitialize: Register the standard text classes          */
/*--------------------------------------------------------------*/
static void ClassInitialize()
{
    XtAddConverter (XtRString, SGMLRTagList, CvtStringToTagList,
		       (XtConvertArgList) NULL, (Cardinal) 0);

    XtAddConverter (XtRString, SGMLRClass, CvtStringToClass,
		        (XtConvertArgList) NULL, (Cardinal) 0);

/*
 *  Intrinsics bug ... trys to convert STRING to STRING 
 */

   XtSetTypeConverter(XtRString,XtRString,ConvertStringToString,
                      NULL,0,XtCacheNone,NULL);
 

   SGMLHyperDeclareClass(sGMLTextObjectClass);
   SGMLHyperDeclareClass(sGMLPlainTextObjectClass);
   SGMLHyperDeclareClass(sGMLFormattedTextObjectClass);
   SGMLHyperDeclareClass(sGMLFormTextObjectClass);
   SGMLHyperDeclareClass(sGMLAnchorTextObjectClass);
   SGMLHyperDeclareClass(sGMLMarkerTextObjectClass);
   SGMLHyperDeclareClass(sGMLRuleTextObjectClass);
   SGMLHyperDeclareClass(sGMLCompositeTextObjectClass);
   SGMLHyperDeclareClass(sGMLInputTextObjectClass);
   SGMLHyperDeclareClass(sGMLListTextObjectClass);
   SGMLHyperDeclareClass(sGMLCaptionTextObjectClass);
   SGMLHyperDeclareClass(sGMLTableTextObjectClass);

   NullQuark = XrmStringToQuark(NULL);
   dlc_quark = XrmStringToQuark("dl_compact");
   dl_quark  = XrmStringToQuark("dl"); 
}

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new) 
SGMLHyperWidget request, new;
{
    Arg arglist[10];
    int n = 0;

    /* Check the size of the widget */

    if (request->core.width  == 0) new->core.width = 100;
    if (request->core.height == 0) new->core.height = 100;

    new->sgml_hyper.hand = XCreateFontCursor(XtDisplay(new),XC_hand2);

    new->sgml_hyper.grep_txt = NULL;
    new->sgml_hyper.grep_len = 0;
    new->sgml_hyper.grep_off = 0;

    new->sgml_hyper.managed = NULL;
    new->sgml_hyper.search_widget = NULL;
    new->sgml_hyper.sw_flag = NULL;
    new->sgml_hyper.contains = NULL;
    new->sgml_hyper.contain_slots = 0;
    new->sgml_hyper.contain_used = 0;

    new->sgml_hyper.last_time = CurrentTime;
    new->sgml_hyper.select_index = 0;
    new->sgml_hyper.copy_text = NULL;

    new->sgml_hyper.copy_info.last_x = 0;
    new->sgml_hyper.copy_info.last_y = 0;
    new->sgml_hyper.copy_info.first_char = NULL;
    new->sgml_hyper.copy_info.last_widget = NULL;
    new->sgml_hyper.copy_info.flag = 0;
   
    new->sgml_hyper.cache_list = NULL;
    new->sgml_hyper.cache_slots = 0;
    new->sgml_hyper.cache_used = 0;
}


/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLHyperWidget w;
{
    XtRemoveAllCallbacks ((Widget) w,SGMLNactivateCallback);
    XtFree((char *) w->sgml_hyper.cache_list);
    XFreeCursor(XtDisplay(w),w->sgml_hyper.hand);
}

/*--------------------------------------------------------------*/
/* DeleteChild : deal with the case where this is managed       */
/*--------------------------------------------------------------*/

static void DeleteChild(w)
Widget w;
{
   int i;
   SGMLHyperWidget parent = (SGMLHyperWidget) XtParent(w);

   if (parent->sgml_hyper.managed == w) parent->sgml_hyper.managed = NULL;
   
   if (XtIsWidget(w))
     {
        WidgetList contains = parent->sgml_hyper.contains;
        int i;
        int last = 2*parent->sgml_hyper.contain_used; 
        
        for (i=0; i < last; i += 2)
          if (contains[i] == w)
            {
              contains[i]   = contains[last-2];
              contains[i+1] = contains[last-1];  
              parent->sgml_hyper.contain_used--;
              break;
            }
     }
   
   /* If the child was on the cache list we had better remove it */ 

   for (i = 0 ; i < parent->sgml_hyper.cache_used ; i++)
     if (w == parent->sgml_hyper.cache_list[i])
       { 
         parent->sgml_hyper.cache_list[i] =
           parent->sgml_hyper.cache_list[--parent->sgml_hyper.cache_used];
       }

   /* Invoke the superclass's delete_child method  (see note under InsertChild below) */   

   (*compositeClassRec.composite_class.delete_child)(w);
}

/*--------------------------------------------------------------*/
/* InsertChild : Only accept SGMLTextObject as child            */
/*               (or any widget)                                */
/*--------------------------------------------------------------*/

static void InsertChild(w)
Widget w;
{
   String params[2];
   Cardinal num_params;
   SGMLHyperWidget parent = (SGMLHyperWidget) XtParent(w);

   if (SGMLIsText(w))
     {
       /* 
        * We have added a new SGMLText child, so we need to think about
        * removing a cached child. For now we simply remove the oldest cached
        * widget
        */

       int j , i = parent->sgml_hyper.cache_used;
       Widget *cache = parent->sgml_hyper.cache_list; 
  
       for (; i > 0 && i >=  parent->sgml_hyper.cache_size ; ) 
         {
           /* Warning, delete child from cache list before calling destroy, to prevent DeleteChild
            * method from itself messing with the cache list
            */
           Widget old = cache[0];

           parent->sgml_hyper.cache_used = --i;
           for (j = 0 ; j < i ; j++)  cache[j] = cache[j+1];  

           XtDestroyWidget(old);
	 }

       /* Add new widget to end of cache list, making sure there is room */
       
       if (i == parent->sgml_hyper.cache_slots) 
         {
           parent->sgml_hyper.cache_slots += 10;
           parent->sgml_hyper.cache_list = cache = (WidgetList) XtRealloc((XtPointer) cache,
                                 (unsigned) parent->sgml_hyper.cache_slots * sizeof(Widget)); 
         } 
       cache[i++] = w;
       parent->sgml_hyper.cache_used = i;      
     }
   else if (XtIsWidget(w))
     {
        WidgetList contains = parent->sgml_hyper.contains;

	Widget container = NULL; /* needed in case w has no userdata resource */
        int i = parent->composite.num_children;
	Widget curr;
        Arg arglist[10];
	int n=0;
 
        XtSetArg(arglist[n],XmNuserData,&container); n++;
        XtGetValues(w,arglist,n); n = 0; 

        if (!container)
          {   
            for (;curr = parent->composite.children[--i];) 
              if (SGMLIsCompositeText(curr)) break;/* bug */

    	    XtSetArg(arglist[n],SGMLNchild,w); n++; 
            container = XtCreateWidget("container",sGMLContainerTextObjectClass,curr,arglist,n);
            SGMLCompositeTextInsertChild(container);
          }
        if (parent->sgml_hyper.contain_used == parent->sgml_hyper.contain_slots) 
          {
            parent->sgml_hyper.contain_slots += 10;
            parent->sgml_hyper.contains = contains = (WidgetList) XtRealloc((XtPointer) contains,
                                 (unsigned) parent->sgml_hyper.contain_slots * sizeof(Widget) * 2);
          }
        contains += 2*( parent->sgml_hyper.contain_used++ );
        contains[0] = w;
        contains[1] = container;
     } 
   else
     {  
       params[0] = XtClass(w)->core_class.class_name;
       params[1] = XtClass(parent)->core_class.class_name;
       num_params = 2;
       XtAppErrorMsg(XtWidgetToApplicationContext(w),
                     "childError","class","WidgetError",
                     "Children of class %s cannot be added to %s widgets",
                     params, &num_params);
     }

/* Invoke the superclass's insert_child method 
   NB Actually we skip out superclasses insert_child since the
      Motif widgets treat non-widget children in some incompatible 
      way... instead we go straight to the composite widget's insert_child
      method, but will this cause problems with XmManager functionality?  */   

   (*compositeClassRec.composite_class.insert_child)(w);
 
}
/*--------------------------------------------------------------*/
/* Resize : not implemented                                     */
/*--------------------------------------------------------------*/

static void Resize (w)
SGMLHyperWidget w;
{
}
/*--------------------------------------------------------------*/
/* GeometryManager:                                             */
/* Only real widgets can make geometry requests, so pass the    */
/* request on to the corresponding container widget             */ 
/*--------------------------------------------------------------*/


static XtGeometryResult GeometryManager(w,desired,allowed)
Widget w;
XtWidgetGeometry *desired, *allowed;
{
   SGMLHyperWidget parent = (SGMLHyperWidget) XtParent(w);
   WidgetList contains = parent->sgml_hyper.contains;
   int i;
   int last = 2*parent->sgml_hyper.contain_used; 
        
   for (i=0; i < last; i += 2)
     if (contains[i] == w)
       {
          Widget container = contains[i+1];
          SGMLContainerTextObjectClass class = (SGMLContainerTextObjectClass) XtClass(container);
          return (*class->sgml_container_text_class.geometry_manager)(container,desired,allowed);
       }
   return XtGeometryNo; 
}

/*--------------------------------------------------------------*/
/* Redisplay : redraw the text                                  */
/*--------------------------------------------------------------*/

static void Redisplay (w, event, region)
SGMLHyperWidget  w;
XEvent       *event;
Region        region;
{
    if(w->core.visible && w->sgml_hyper.managed)
    {
      SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
      (*childClass->sgml_text_class.expose)(w->sgml_hyper.managed,event,region,0);
    }  
}

/*------------------------------------------------------------------*/
/* SetValues : redraw only for font or color changes                */
/*------------------------------------------------------------------*/

static Boolean SetValues (current, request, new)
SGMLHyperWidget current, request, new;
{
    Boolean    redraw = FALSE;

#define HAS_CHANGED(a)    (new->a != current->a)

    if(
        HAS_CHANGED(sgml_hyper.natural_width)
      )
    {

        /* invalidate size of all text widget children */
        
        int n = new->composite.num_children;
        SGMLTextObject *children = (SGMLTextObject *) new->composite.children;
        
        for (; n-- > 0; children++) 
          if (SGMLIsText( (Widget) *children)) (*children)->sgml_text.size_valid = FALSE;

        /* rebuild text */

        calc_new_size(new);
        redraw = TRUE;
    }

    return (redraw);

#undef HAS_CHANGED

}

/*------------------------------------------------------------------*/
/* Adjust the size of a child                                       */
/*------------------------------------------------------------------*/

static void adjust_size(child,y,ascent,descent)
Widget child;
Position y;
Dimension ascent, descent;
{
}
/*------------------------------------------------------------------*/
/* Calculate the size of the widget                                 */
/*------------------------------------------------------------------*/

static void calc_new_size(w)
SGMLHyperWidget  w;
{
    XtGeometryResult    result;
    SGMLDimension       width=0, height=0;
    SGMLGeometry        geom;
    Dimension requestWidth, requestHeight;

    geom.coord.x = 0;
    geom.coord.y = w->sgml_hyper.margin;
    geom.coord.descent = 0;
    geom.coord.ascent = 0;
    geom.broken = TRUE;
    geom.leave_space = FALSE;
    geom.space = 99999;
    geom.actual_width = 2 * w->sgml_hyper.margin;
    geom.natural_width = w->sgml_hyper.natural_width;

    /* Loop over all of the text segments, getting each to 
       tell us how much space it needs                     */

    if (w->sgml_hyper.managed) 
      {
        SGMLTextObject t = (SGMLTextObject) w->sgml_hyper.managed;
        
        if (t->sgml_text.size_valid)
          {
            width =  t->sgml_text.width;
            height = geom.coord.y + t->sgml_text.height;
          }
        else
          {
            SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) t);
            (*childClass->sgml_text_class.compute_size)(t,&geom,adjust_size,w->sgml_hyper.managed);
       
            t->sgml_text.size_valid = TRUE; 
            width = geom.actual_width;
            height = geom.coord.y + geom.coord.descent + geom.coord.ascent;
          }
      } 
    width  += w->sgml_hyper.margin;
    height += w->sgml_hyper.margin;

    /* 
    Tell our parent we want a new size 
    */

    requestWidth = width > 32767 ? 32767 : width;
    requestHeight = height > 32767 ? 32767 : height;       

    /* Temporarily until something better comes along */

    if (requestHeight != height) MidasQueueCommand(w,"Popup .^^^WWWMain###WWWTruncate");

    if(w->core.width != width || w->core.height != height)
    {
        Dimension replyWidth = 0, replyHeight = 0;

        result = XtMakeResizeRequest((Widget) w,requestWidth,requestHeight, 
            &replyWidth, &replyHeight) ;

        if (result == XtGeometryAlmost)
            XtMakeResizeRequest ((Widget) w, replyWidth, replyHeight,NULL, NULL);
    }

    /* 
    Redraw the window
    */
   
    if (XtIsRealized((Widget) w)) XClearArea(XtDisplay((Widget)w),XtWindow((Widget)w),0,0,0,0,True);

}

/*-----------------------------------------------------------------------*/
/* Find the "visible" part of a widget as the intersection of all the    */
/* windows of it's parents' windows                                      */
/*-----------------------------------------------------------------------*/

static void find_visible_part(w,x,y,width,height)
Widget    w;
Position  *x;
Position  *y;
Dimension *width;
Dimension *height;
{
    Position root_x,root_y;
    Widget   p = w;

    *width  = w->core.width;
    *height = w->core.height;
    XtTranslateCoords(w,0,0,&root_x,&root_y);

    *x = 0;
    *y = 0;

    while(p = XtParent(p))
    {
        Position  rx,ry;
        Dimension w,h;

        /* 
           make all computations in the root's
           coordinate system
        */

        XtTranslateCoords(p,0,0,&rx,&ry);

        w = p->core.width;
        h = p->core.height;

        /* 
            use the smallest rectangle
        */

        if(w < *width)  *width  = w;
        if(h < *height) *height = h;

        if(rx>root_x) root_x = rx;
        if(ry>root_y) root_y = ry;

        /* stop when reach a shell,
          don't go to top level shell */
        if(XtIsShell(p)) break;
    }

    /* Back to the widget's coordinate system */

    XtTranslateCoords(w,0,0,x,y);
    *x = root_x - *x;
    *y = root_y - *y;


}


/*----------------------------------------------------------------------*/
/* Find the Widget (Gadget) at point (x,y)                              */
/*----------------------------------------------------------------------*/
SGMLTextObject find_segment(w,x,y,mode)
SGMLHyperWidget w;
int x,y;
Boolean mode;
{
   if (w->sgml_hyper.managed)
      {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
         Widget result = (*childClass->sgml_text_class.contains)(w->sgml_hyper.managed,x,y,mode);
         return (SGMLTextObject) result;  
      }         

    return NULL;
}
/*-----------------------------------------------------------------------*/
/* Check for mouse down                                                  */
/*-----------------------------------------------------------------------*/

/* This function is never used and is replaced by SelectStart            */

static void selectt (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
char          *args[];
int            n_args;
{
    /* 
       Find if the user clicked in a (sensitive) text object 
    */

    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,FALSE);
    w->sgml_hyper.last_selected = s;

    if (s != NULL) 
      {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) s);
         (*childClass->sgml_text_class.hilite)(s,SGMLSELECT_WIDGET);  
      }
}

/*-----------------------------------------------------------------------*/
/* Check for mouse up                                                    */
/*-----------------------------------------------------------------------*/

/* This function is never used and is replaced by SelectEnd              */

static void activate (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
char          *args[];
int            n_args;
{
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,FALSE);

    /* 
       Find if the user clicked in a highlighted text 
    */

    if(s != NULL && s == w->sgml_hyper.last_selected)
    {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) s);

         (*childClass->sgml_text_class.hilite)(s,SGMLSELECT_WIDGET);  
         (*childClass->sgml_text_class.activate)(s,event);  

    }
    w->sgml_hyper.last_selected = NULL;
}

/*-----------------------------------------------------------------------*/
/* Check for mouse moves                                                 */
/*-----------------------------------------------------------------------*/

static void cursor (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
char          *args[];
int            n_args;
{
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,FALSE);
    SGMLTextObject l = w->sgml_hyper.last_selected;
 
    if(s != w->sgml_hyper.last_cursor)
    {
        if ( l != NULL && (s == l || l == w->sgml_hyper.last_cursor))
          {
            SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) l);
            (*childClass->sgml_text_class.hilite)(l,SGMLSELECT_WIDGET);
          }
           
        if(s) XDefineCursor(XtDisplay(w),XtWindow(w),w->sgml_hyper.hand);
        else  XUndefineCursor(XtDisplay(w),XtWindow(w));
        
        w->sgml_hyper.last_cursor = s;
    }
}

/*-----------------------------------------------------------------------*/
/* select a word by double clicks                                        */
/*-----------------------------------------------------------------------*/
static void SelectWord (w, event)
SGMLHyperWidget   w;
XEvent        *event;
{
    Widget selection;
    int x = event->xbutton.x;
    int y = event->xbutton.y;
    char *pos = NULL;
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,TRUE);
 
    /* No other widgets except text widgets ONLY are allowed */
    if (!s || !s->sgml_text.text || *(s->sgml_text.text) == '\0') return;

    /* race condition with motion? */
    if (w->sgml_hyper.race) return; 
    else w->sgml_hyper.race = TRUE;

    if(s != NULL)
    {
      SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) s);

      w->sgml_hyper.copy_info.current_x = x;
      w->sgml_hyper.copy_info.current_y = y;
      w->sgml_hyper.copy_info.current_widget = (Widget)s;

      selection = (*class->sgml_text_class.hilite_selection)(s,SGMLSELECT_WORD,&w->sgml_hyper.copy_info);  
    }

}

/*-----------------------------------------------------------------------*/
/* select a line by triple clicks                                        */
/*-----------------------------------------------------------------------*/
static void SelectLine (w, event)
SGMLHyperWidget   w;
XEvent        *event;
{
    Widget selection;
    int x = event->xbutton.x;
    int y = event->xbutton.y;
    char *pos = NULL;
    SGMLTextObject lasts = NULL;
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,TRUE);
 
    /* No other widgets except text widgets ONLY are allowed */
    if (s == NULL || s->sgml_text.text == NULL || *(s->sgml_text.text) == '\0') return;

    /* race condition with motion? */
    if (w->sgml_hyper.race) return;
    else w->sgml_hyper.race = TRUE;

    if(s != NULL)
    {
      SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) s);

      for (x=10;x < w->core.width; x += 10)
        {
          s = find_segment(w,x,y,TRUE);
          
          if (s == lasts || s == NULL) continue;

          /* No other widgets except text widgets ONLY are allowed */
          if (s->sgml_text.text == NULL || *(s->sgml_text.text) == '\0') 
            continue;

          w->sgml_hyper.copy_info.current_x = x;
          w->sgml_hyper.copy_info.current_y = y;
          w->sgml_hyper.copy_info.current_widget = (Widget) s;
          selection = (*class->sgml_text_class.hilite_selection)(s,SGMLSELECT_LINE,&w->sgml_hyper.copy_info);  
          lasts = s;

        }

      if (selection)
        {
          w->sgml_hyper.copy_info.sw_flag = TRUE;
          w->sgml_hyper.copy_info.last_widget = selection;
          w->sgml_hyper.copy_info.last_mode = FALSE;
        }
     }
        
}


/*-----------------------------------------------------------------------*/
/* select whole document by Ctrl-click                                   */
/*-----------------------------------------------------------------------*/
static void SelectAll (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
String        *args;
Cardinal      *n_args;
{
  SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
  Widget selection;

  w->sgml_hyper.selection_time = event->xbutton.time;

  if (w->sgml_hyper.search_widget || w->sgml_hyper.sw_flag)
    {
      (*childClass->sgml_text_class.clear_select)(w->sgml_hyper.managed,w->sgml_hyper.sw_flag,TRUE);
      w->sgml_hyper.search_widget = NULL;
      w->sgml_hyper.sw_flag = NULL;
    }

  w->sgml_hyper.copy_info.sw_flag = FALSE;
  w->sgml_hyper.copy_info.last_widget = NULL;

  selection =  (*childClass->sgml_text_class.hilite_selection)(w->sgml_hyper.managed,SGMLSELECT_ALL,&w->sgml_hyper.copy_info);
  
  if (w->sgml_hyper.copy_text)
    {
      XtFree(w->sgml_hyper.copy_text);
      w->sgml_hyper.copy_text = NULL;
    }

  if (w->sgml_hyper.copy_info.sw_flag)
    {
      w->sgml_hyper.search_widget = w->sgml_hyper.copy_info.last_widget;
      w->sgml_hyper.sw_flag = w->sgml_hyper.search_widget;
    }

  _OwnSelection(w, event, args, n_args);

}

/*-----------------------------------------------------------------------*/
/* select a region of text by button + motion                            */
/*-----------------------------------------------------------------------*/
static void SelectRegion(w,event)
SGMLHyperWidget w;
XEvent *event;
{
   SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,TRUE);

   if(s != NULL)
     {
       Widget selection;
       int x = event->xbutton.x;
       int y = event->xbutton.y;
       char *pos;
       SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) s);

       /* No other widgets except text widgets ONLY are allowed */
       if (s->sgml_text.text == NULL || *(s->sgml_text.text) == '\0') return;

       (*class->sgml_text_class.xy_to_pos)(s,&x,&y,&pos);
       w->sgml_hyper.copy_info.current_widget = (Widget)s;
       w->sgml_hyper.copy_info.last_widget = (Widget) s;
       w->sgml_hyper.copy_info.first_char = pos;
       w->sgml_hyper.copy_info.first_x = x;
       w->sgml_hyper.copy_info.first_y = y;
       w->sgml_hyper.copy_info.current_x = x;
       w->sgml_hyper.copy_info.current_y = y;
       w->sgml_hyper.copy_info.last_x = x;
       w->sgml_hyper.copy_info.last_y = y;
       w->sgml_hyper.copy_info.flag = FALSE;
       w->sgml_hyper.copy_info.sw_flag = FALSE;
       w->sgml_hyper.copy_info.first_widget = (Widget)s;
       w->sgml_hyper.copy_info.current_mode = FALSE;
       w->sgml_hyper.copy_info.last_mode = FALSE;
       w->sgml_hyper.copy_info.cw = w->sgml_hyper.managed;
     }
   else 
     w->sgml_hyper.select_start = FALSE;
}

/*-----------------------------------------------------------------------*/
/* Selection start                                                       */
/*-----------------------------------------------------------------------*/
static void SelectStart (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
String        *args;
Cardinal      *n_args;
{
  Time mct = XtGetMultiClickTime(XtDisplay(w));
  Time sel_time = event->xbutton.time;
  SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
  SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,FALSE);


  /* clear the selection whenever button click */
  if (w->sgml_hyper.search_widget || w->sgml_hyper.sw_flag)
    {
      (*childClass->sgml_text_class.clear_select)(w->sgml_hyper.managed,w->sgml_hyper.sw_flag,TRUE);
      w->sgml_hyper.search_widget = NULL;
      w->sgml_hyper.sw_flag = NULL;
    }

  if (w->sgml_hyper.copy_text) 
    { 
      XtFree(w->sgml_hyper.copy_text);
      w->sgml_hyper.copy_text = NULL;
    } 

  w->sgml_hyper.last_selected = s;
  if (s != NULL) 
    {
       SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) s);
       (*class->sgml_text_class.hilite)(s,SGMLSELECT_WIDGET);  
       w->sgml_hyper.select_start = FALSE;
       return;
    }

  w->sgml_hyper.select_start = TRUE;
  if (sel_time > w->sgml_hyper.last_time && 
      sel_time - w->sgml_hyper.last_time < mct)
    {
      if (w->sgml_hyper.select_index < SGMLSELECT_LINE)
        w->sgml_hyper.select_index++;
      else w->sgml_hyper.select_index = SGMLSELECT_REGION;
    }
  else w->sgml_hyper.select_index = SGMLSELECT_REGION;

  w->sgml_hyper.last_time = sel_time;
   
  if (w->sgml_hyper.select_index)
    {
      w->sgml_hyper.race = FALSE;
      if (w->sgml_hyper.select_index == SGMLSELECT_WORD)
        SelectWord(w,event); 
      else if (w->sgml_hyper.select_index == SGMLSELECT_LINE)
        SelectLine(w,event);
      else 
        SelectRegion(w,event);
    }
    
}

/*-----------------------------------------------------------------------*/
/* Selection extend                                                      */
/*-----------------------------------------------------------------------*/
static void SelectExtend (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
String        *args;
Cardinal      *n_args;
{
    Widget selection;
    int x = event->xbutton.x;
    int y = event->xbutton.y;
    char *pos = NULL;
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,TRUE);
    SGMLTextObject ss = find_segment(w,event->xbutton.x,event->xbutton.y,FALSE);
    SGMLTextObject l = w->sgml_hyper.last_selected;

    if (ss != w->sgml_hyper.last_cursor)
    {
      if ( l != NULL && (s == l || l == w->sgml_hyper.last_cursor)) 
        {
          SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) l);
          (*childClass->sgml_text_class.hilite)(l,SGMLSELECT_WIDGET);
        }
      if (ss) XDefineCursor(XtDisplay(w),XtWindow(w),w->sgml_hyper.hand);
      else XUndefineCursor(XtDisplay(w),XtWindow(w));
      w->sgml_hyper.last_cursor = ss;
    }

    if (!w->sgml_hyper.select_start) return;

    /* race condition with button down? */
    if (w->sgml_hyper.race) return;

    if (w->sgml_hyper.select_index != SGMLSELECT_REGION &&
        w->sgml_hyper.copy_info.last_widget == NULL ) return;

    if (s != NULL) 
    {
      SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget) s);
      SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
      SGMLTextObject last_widget = (SGMLTextObject)w->sgml_hyper.copy_info.last_widget;
  
      /* No other widgets except text widgets ONLY are allowed */
      if (s->sgml_text.text == NULL || *(s->sgml_text.text) == '\0') return;

      (*class->sgml_text_class.xy_to_pos)(s,&x,&y,&pos);
      w->sgml_hyper.copy_info.current_x = x;
      w->sgml_hyper.copy_info.current_y = y;
      w->sgml_hyper.copy_info.current_widget = (Widget)s;
      if ((w->sgml_hyper.copy_info.current_x == w->sgml_hyper.copy_info.last_x &&
          w->sgml_hyper.copy_info.current_y == w->sgml_hyper.copy_info.current_y) ||
          w->sgml_hyper.copy_info.last_pos == pos )
        return;

      w->sgml_hyper.copy_info.last_pos = pos;
      if (s == last_widget)
        selection =  (*class->sgml_text_class.hilite_selection)(s,SGMLSELECT_REGION,&w->sgml_hyper.copy_info);
      else 
        selection =  (*childClass->sgml_text_class.hilite_selection)(w->sgml_hyper.managed,SGMLSELECT_REGION,&w->sgml_hyper.copy_info);

      w->sgml_hyper.copy_info.last_mode = w->sgml_hyper.copy_info.current_mode;
      w->sgml_hyper.copy_info.last_widget = w->sgml_hyper.copy_info.current_widget;
      w->sgml_hyper.copy_info.last_x = w->sgml_hyper.copy_info.current_x;
      w->sgml_hyper.copy_info.last_y = w->sgml_hyper.copy_info.current_y;

    }
}

/*-----------------------------------------------------------------------*/
/* Selection end                                                         */
/*-----------------------------------------------------------------------*/
static void SelectEnd (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
String        *args;
Cardinal      *n_args;
{
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y,FALSE);

    if(s != NULL && s == w->sgml_hyper.last_selected)
    {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) s);

         (*childClass->sgml_text_class.hilite)(s,SGMLSELECT_WIDGET);
         (*childClass->sgml_text_class.activate)(s,event);
         w->sgml_hyper.last_selected = NULL;
         return;
    }
    w->sgml_hyper.last_selected = NULL;

    if (!w->sgml_hyper.search_widget && !w->sgml_hyper.sw_flag)
      {
        Boolean sw = w->sgml_hyper.copy_info.sw_flag;

        if (sw)
          {
            if (w->sgml_hyper.copy_info.last_mode)
              w->sgml_hyper.search_widget = w->sgml_hyper.copy_info.first_widget;
            else
              w->sgml_hyper.search_widget = w->sgml_hyper.copy_info.last_widget;
            w->sgml_hyper.sw_flag = w->sgml_hyper.search_widget;

            if (w->sgml_hyper.copy_text)
              {
                XtFree(w->sgml_hyper.copy_text);
                w->sgml_hyper.copy_text = NULL;
              }
          }

        w->sgml_hyper.selection_time = event->xbutton.time;

        if (w->sgml_hyper.search_widget)
          _OwnSelection(w,event,args,n_args);
      }
}

static void _OwnSelection(w, event, args, n_args)
SGMLHyperWidget w;
XEvent        *event;
String        *args;
Cardinal      *n_args;
{
   int i, buffer;
   Atom *atoms;

   atoms = (Atom *)malloc(*n_args * sizeof(Atom));
   if (atoms == NULL)
     {
        fprintf(stderr, "cannot allocate atom list\n");
        return;
     }
   XmuInternStrings(XtDisplay((Widget)w), args, *n_args, atoms);
   w->sgml_hyper.selection_time = event->xbutton.time;
   for (i=0; i< *n_args; i++)
     {
       switch (atoms[i])
       {
         case XA_CUT_BUFFER0: buffer = 0; break;
         case XA_CUT_BUFFER1: buffer = 1; break;
         case XA_CUT_BUFFER2: buffer = 2; break;
         case XA_CUT_BUFFER3: buffer = 3; break;
         case XA_CUT_BUFFER4: buffer = 4; break;
         case XA_CUT_BUFFER5: buffer = 5; break;
         case XA_CUT_BUFFER6: buffer = 6; break;
         case XA_CUT_BUFFER7: buffer = 7; break;
         default: buffer = -1; break;
       }
       if (buffer >= 0)
         {
           if (GetSelection(w,SGMLDUMP_ASCII))
             XStoreBuffer(XtDisplay((Widget)w), w->sgml_hyper.copy_text, 
                          strlen(w->sgml_hyper.copy_text), buffer); 
         }
       else
         {
           Boolean result = XtOwnSelection((Widget)w, atoms[i],
                                           event->xbutton.time,Convert,
                                           LoseOwnership,SelectionDone);
           if (!result)
             printf ("Warning: Fail to become selection owner\n");
         }
     }
   free ((char *)atoms);

}

/*-----------------------------------------------------------------------*/
/* Add text to a text segment                                            */
/*-----------------------------------------------------------------------*/

static Widget add_to_text(name,parent,class,text,param,matched)
Widget parent;
SGMLTextObjectClass class;
char *text, *param, *name;
int matched;
{
    Widget new;

/*    
 *  Scan the param string looking for possible parameters (maximum 5 at present)
 */

    if (param && *param)
      {
        char *copy;
        char *Type[5];
        char *Res[5];
        char *Val[5];
        int  Len[5]; 
        char *whiteSpace = " \t\n\r";
        char *whiteSpaceOrEqual = " \t=\n\r";
        int i, l;        
        char *p, *q, *r;

        copy = XtNewString(param);
        r = copy;

        for (i=0; i<5; )
          {
            Boolean done, equal; 

            p = r + strspn(r,whiteSpace);           /* skip leading whitespace */
            if (*p=='\0') break;
            q = p + strcspn(p,whiteSpaceOrEqual);   /* Skip until white or equal */
            r = q + strspn(q,whiteSpace);           /* Skip any whitespace */

            done  =  (*r == '\0');
            equal =  (*r == '=' );
            *q = '\0';

            if (*p != '"') lowcase(p);
            else 
              {
                p++;
                if (*(q-1)=='"') *(q-1) = '\0'; 
              }  

            /* 
             * For security reasons we mustn't let any Callbacks
             * be set using tag parameters 
             */
     
            if (strstr(p,"Callback")) continue;

            Type[i] = XtVaTypedArg;
            Res[i]  = p;
        
            if (equal)
              {
                r++;
                r +=  strspn(r,whiteSpace);
                if (*r == '"') 
                  { 
                    r++;
                    l = strcspn(r,"\"");
                  }     
                else
                  {
                    l = strcspn(r,whiteSpace);
                  }
                p = r + l;
                done = (*p == '\0');
                if (!done) *p = '\0';
        
                Val[i] = r;
                Len[i] = l + 1;
                r = p + 1;  
                i++; 
              }
            else 
              {
                Val[i] = "TRUE";
                Len[i] = 5; 
                i++; 
              }
            if (done) break;
          }
        if (i < 5) Type[i] = NULL;
        new = XtVaCreateWidget(name,(WidgetClass) class,parent,
              SGMLNtext,      (XtArgVal) text, 
              SGMLNparam,     (XtArgVal) param,
              SGMLNmatched,   (XtArgVal) matched,  
              Type[0], Res[0], XtRString, Val[0], Len[0],
              Type[1], Res[1], XtRString, Val[1], Len[1],
              Type[2], Res[2], XtRString, Val[2], Len[2],
              Type[3], Res[3], XtRString, Val[3], Len[3],
              Type[4], Res[4], XtRString, Val[4], Len[4], NULL);
 
        XtFree(copy);
      }          
    else
      {
        Arg arglist[10];
        int n=0;

        XtSetArg(arglist[n],SGMLNtext ,   (XtArgVal) text   ); n++;
        XtSetArg(arglist[n],SGMLNparam,   (XtArgVal) param  ); n++; 
        XtSetArg(arglist[n],SGMLNmatched, (XtArgVal) matched); n++; 

        new = XtCreateWidget(name,(WidgetClass) class, parent, arglist, n);
      }
     
    if (SGMLIsCompositeText(parent)) SGMLCompositeTextInsertChild(new);
    
    /*
     *  Invoke the CreateCallback
     */  

    (*class->sgml_text_class.call_create_callback)(new,NULL);
     
    return new;
}    

/*-----------------------------------------------------------------------*/
/* check a tag: returns TRUE if tag accepted                             */    
/*                      FALSE if tag rejected                            */
/* if the tag is accepted then NewClass gives its type (or null if it    */
/* was a terminator)                                                     */
/*-----------------------------------------------------------------------*/

static Boolean check_tag(w,intag,Text,Name,NewClass,tagQuark,exactTerminator,matched)
SGMLHyperWidget w;
char *intag;
XrmName Name;
Widget Text;
WidgetClass *NewClass;
XrmName *tagQuark;
Boolean exactTerminator;
int *matched; 
{
  char *tag = XtNewString(intag);
  char *oldtag = tag; 
  char et = w->sgml_hyper.end_tag;
  Arg arglist[10];
  int n = 0;
  int i;
  SGMLTagList *tagList;
  Boolean end = (*tag == et);
  if (end) tag++;  

  *NewClass = NULL; 

  if (!w->sgml_hyper.case_sensitive_tags) lowcase(tag);
  *tagQuark = XrmStringToQuark(tag);  
  XtFree(oldtag);

  if (Name != NullQuark)
    if (end) return (!exactTerminator || *tagQuark == Name );
    else     return FALSE;

  if (end) 
    {
      int d = 0; 
 
      if (XtParent(Text) == (Widget) w) return FALSE; /* nothing to end */
      if (!exactTerminator) return TRUE; 
      for (; XtParent(Text) != (Widget) w ; Text = XtParent(Text)) 
        { 
          d++;
          if (*tagQuark == Text->core.xrm_name ||  /* dlc kludge */
             (*tagQuark == dl_quark && dlc_quark == Text->core.xrm_name)) return TRUE;
        }   
      return FALSE; 
    }

  XtSetArg(arglist[n],SGMLNtagList,&tagList); n++;
  XtGetValues(Text,arglist,n);

  if (tagList)
    for (i=0; tagList[i].class != NULL; i++)
      if (tagList[i].name == *tagQuark) 
        { 
          *NewClass = tagList[i].class; 
          *matched = tagList[i].matched;
          return TRUE; 
        }
    
  return FALSE;  
}

/*-----------------------------------------------------------------------*/
/* set a new tag: the tag must previously have been accepted by          */    
/* check tag                                                             */
/*-----------------------------------------------------------------------*/


static void set_tag(w,inoutText,inoutName,inoutClass,param,NewClass,tagQuark,matched)
SGMLHyperWidget w;
char *param; 
XrmName *inoutName;
Widget *inoutText;
WidgetClass *inoutClass;
WidgetClass NewClass;  
XrmName tagQuark;
int matched;
{ 
  Widget text = *inoutText;
  XrmName name = *inoutName;

  /* temporary kludge to support dl compact */

  if (tagQuark == dl_quark && param && strstr(param,"compact")) tagQuark = dlc_quark; 

  if (name != NullQuark)
    { 
       Arg arglist[10];
       int n = 0;

       *inoutName = NullQuark;

       XtSetArg(arglist[n],SGMLNdefaultClass,inoutClass); n++;
       XtGetValues(text,arglist,n);
       return;
    }   

  if (NewClass == NULL) 
    {
      for (; tagQuark != text->core.xrm_name && (tagQuark != dl_quark || dlc_quark != text->core.xrm_name); text = XtParent(text)); 
      text = XtParent(text);   
    }
  else
    {
    if (!matched)
      {
        add_to_text(XrmQuarkToString(tagQuark),text,NewClass,NULL,param,matched);
      }
    else if (IsSubClass(NewClass,sGMLCompositeTextObjectClass))
      {
        text = add_to_text(XrmQuarkToString(tagQuark),text,NewClass,NULL,param,matched);
      }  
    else
      {
        *inoutName = tagQuark; 
        *inoutClass = NewClass;
      }
    }    
  if (text != *inoutText)
  {
    Arg arglist[10];
    int n = 0;
       
    XtSetArg(arglist[n],SGMLNdefaultClass,inoutClass); n++;
    XtGetValues(text,arglist,n);
  }
  *inoutText = text;
}
/*-----------------------------------------------------------------------*/
/* Rebuild the text structure. Called when the font changes              */
/*-----------------------------------------------------------------------*/

void SGMLHyperManageChild(w)
Widget w;
{
    SGMLHyperWidget parent = (SGMLHyperWidget) XtParent(w); 
    SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass(w);
    Widget old = parent->sgml_hyper.managed;
    Widget sw = parent->sgml_hyper.sw_flag;
    int i;

    if (XtIsRealized((Widget) parent)) XUndefineCursor(XtDisplay((Widget) parent),XtWindow((Widget) parent)); /* In case cursor was set */

    if (old == w) return;
    
    if (old)
      {
        SGMLTextObjectClass oldclass = (SGMLTextObjectClass) XtClass(old);
        (*oldclass->sgml_text_class.call_map_callback)(old,NULL,FALSE);
        if (sw) {
          (*oldclass->sgml_text_class.clear_select)(old,sw,FALSE);
          parent->sgml_hyper.sw_flag = NULL;
          if (parent->sgml_hyper.copy_text)
            {
              XtFree(parent->sgml_hyper.copy_text);
              parent->sgml_hyper.copy_text = NULL;
            }
          }
      }
    parent->sgml_hyper.managed = w;
    parent->sgml_hyper.search_widget = NULL;

    calc_new_size(parent);

    /*
     * Each time a widget is managed we put it back at the top of the cache
     * list, thus making it less likely to be removed from the cache as other
     * children are created
     */

    for (i=0 ; i < parent->sgml_hyper.cache_used - 1; i++)
      if (w == parent->sgml_hyper.cache_list[i])
        { 
          Widget swap = parent->sgml_hyper.cache_list[parent->sgml_hyper.cache_used - 1] ;
          parent->sgml_hyper.cache_list[parent->sgml_hyper.cache_used - 1] =
            parent->sgml_hyper.cache_list[i];
          parent->sgml_hyper.cache_list[i] = swap;
        }

    (*class->sgml_text_class.call_map_callback)(w,NULL,TRUE);
}
/*-----------------------------------------------------------------------*/
/* Build the text. Gets the chars from the funtion "get_next_char"       */
/* using "data" as a parameter                                           */
/*-----------------------------------------------------------------------*/

#define NORMAL 0
#define TAG    1
#define PARAM  2 
#define ENTITY 4
#define PARAMILLEGAL  3 

typedef struct {
 
    SGMLHyperWidget w;
    Widget      parentText;
    Widget      topText;
    char        *word;
    char        *saveParam;

    int         mode;
    int         tag; 
    int         param;
    int         wordsize; 
    int         i;

    WidgetClass currentClass; 
    WidgetClass NewClass;
    XrmName     currentName;
    XrmName     tagQuark;
    Boolean     exactTerminator;
    Boolean     supportsEntities;
    Boolean     hideIllegalTags;
    int         matched;

    Arg arglist[10];
    int n;

} ParseState;
    
static ParseState *CreateState(inW)
Widget inW; 
{  
    ParseState *state = XtNew(ParseState);

    state->i = 0;
    state->mode = NORMAL; 
    state->currentName = NullQuark; 
    state->saveParam = NULL;

    /* Create the base text object */
 
    state->topText = state->parentText = XtCreateWidget("default",sGMLCompositeTextObjectClass,inW,NULL,0);

    state->n = 0; 
    XtSetArg(state->arglist[state->n],SGMLNdefaultClass,&state->currentClass); state->n++;
    XtGetValues(state->parentText,state->arglist,state->n);

    state->n = 0;
    XtSetArg(state->arglist[state->n],SGMLNexactTerminator,&state->exactTerminator); state->n++;
    XtSetArg(state->arglist[state->n],SGMLNsupportsEntities,&state->supportsEntities); state->n++;
    XtSetArg(state->arglist[state->n],SGMLNhideIllegalTags,&state->hideIllegalTags); state->n++;
    XtGetValues(state->parentText,state->arglist,state->n);

    state->wordsize = MAX_LINE_SIZE;
    state->word = XtMalloc(state->wordsize);

    for (; !SGMLIsHyper(inW);) inW = XtParent(inW);
    state->w = (SGMLHyperWidget) inW; 

    return state;     
}
static Widget DestroyState(state)
ParseState *state;
{
    Widget result;

    /* flush .. */

    if(state->i)
    {
      state->word[state->i] = '\0';
      if (state->i) add_to_text(XrmQuarkToString(state->currentName),state->parentText,
                                state->currentClass,state->word,state->saveParam,1);
    }

    XtFree(state->saveParam);  
    XtFree(state->word);

    result = state->topText;
    XtFree((char *)state);
    return result;
}

static void munge_text(state,get_next_char,data)
ParseState *state;
int (*get_next_char)();
XtPointer data; 
{
    int c;
    int i = state->i;
    int mode = state->mode;
    char *word = state->word;
    int wordsize = state->wordsize; 

    int ot  = state->w->sgml_hyper.open_tag;
    int ct  = state->w->sgml_hyper.close_tag;
    int pt  = state->w->sgml_hyper.parameter_tag;
    int est = state->w->sgml_hyper.entity_tag;
    int eet = state->w->sgml_hyper.entity_end_tag;
 
    while((c = (get_next_char)(data)) != EOF)  
    {
        /* kludge */

        if (mode == TAG && (c == '\n' || c == '\t')) c = ' '; 

        /* Open Tag */
    
        if (mode == NORMAL && c == ot)
        {
            word[i++] = c;
            if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
            state->tag = i;    
            mode = TAG;
        }

        /* Entity */

        else if (state->supportsEntities && mode == NORMAL && c == est)
        {
            word[i++] = c;
            if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
            state->tag = i;
            mode = ENTITY; 
        }

        /* Parameter */

        else if (mode == TAG && c == pt)
        {
            word[i] = '\0';
            if (check_tag(state->w,word+state->tag,state->parentText,state->currentName,
                          &state->NewClass,&state->tagQuark,state->exactTerminator,&state->matched))
            {
              word[state->tag-1] = '\0';
              if (*word) add_to_text(XrmQuarkToString(state->currentName),state->parentText,
                                     state->currentClass,word,state->saveParam,1);
              i++;
              if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              XtFree(state->saveParam);
              state->saveParam = NULL;
              state->param = i;
              mode = PARAM;
            }
            else if (state->hideIllegalTags) 
            {
              word[state->tag-1] = '\0';
              if (*word) add_to_text(XrmQuarkToString(state->currentName),state->parentText,
                                     state->currentClass,word,state->saveParam,1);
              word[i++] = pt;
              if (i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              XtFree(state->saveParam);
              state->saveParam = NULL;
              mode = PARAMILLEGAL;
            }    
            else 
            {
              word[i++] = pt;
              if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              mode = NORMAL;
            } 
        }

        /* Close Tag */
        
        else if (mode == TAG && c == ct)
        {
            word[i] = '\0';
            if (check_tag(state->w,word+state->tag,state->parentText,state->currentName,
                          &state->NewClass,&state->tagQuark,state->exactTerminator,&state->matched))
            {
              word[state->tag-1] = '\0';
              add_to_text(XrmQuarkToString(state->currentName),state->parentText,
                                     state->currentClass,word,state->saveParam,1);
              set_tag(state->w,&state->parentText,&state->currentName,&state->currentClass,
                      state->saveParam,state->NewClass,state->tagQuark,state->matched);
              XtGetValues(state->parentText,state->arglist,state->n); 
              
              i = 0;     
              mode = NORMAL;
              XtFree(state->saveParam); 
              state->saveParam = NULL;   
            }
            else if (state->hideIllegalTags) 
            {
              word[state->tag-1] = '\0';
              if (*word) add_to_text(XrmQuarkToString(state->currentName),state->parentText,
                                     state->currentClass,word,state->saveParam,1);
 
              word[state->tag-1] = ot;
              word[i++] = ct;
              if (i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              word[i++] = '\0'; 
              add_to_text("hidden",state->parentText,sGMLFormattedTextObjectClass,word+state->tag-1,NULL,1);

              i = 0;     
              mode = NORMAL;
              XtFree(state->saveParam); 
              state->saveParam = NULL;   
            }    
            else
            {
              word[i++] = ct;
              if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              mode = NORMAL;
            } 
        }
         
        else if (mode == PARAM && c == ct)
        {
            word[i] = '\0';
            set_tag(state->w,&state->parentText,&state->currentName,&state->currentClass,
                    word+state->param,state->NewClass,state->tagQuark,state->matched);
            XtGetValues(state->parentText,state->arglist,state->n);
            
            if (state->NewClass && state->matched)
               state->saveParam = XtNewString(word+state->param);
            mode = NORMAL;
            i = 0;     
        }
        else if (mode == PARAMILLEGAL && c == ct)
        {
            word[state->tag-1] = ot;
            word[i++] = ct;
            if (i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
            word[i++] = '\0'; 
            add_to_text("hidden",state->parentText,sGMLFormattedTextObjectClass,word+state->tag-1,NULL,1);
            
            mode = NORMAL;
            i = 0;     
        }
        else if (mode == ENTITY && c == eet) 
        { 
            char entity[32];
            char *e = entity; 
            int j;

            for (j = state->tag ; j<i ; j++) *e++ = word[j]; 

            if (c == eet)
             { 
               *e = '\0';

               if      (isdigit(*entity)) c = (char) atoi(entity);
               else if (*entity == '#')   c = (char) atoi(entity+1);
               else if ((c = EntityToIsoLatin1(entity)) == '\0') c = '?'; 
 
               i = state->tag;
               word[i-1] = c;
             }
            mode = NORMAL; 
        }
        else if (mode == ENTITY && (i-state->tag > 30 || (c != '#' && !isalnum(c))))
        {
            mode = NORMAL; 
        }
        else
        {
            word[i++] = c;
            if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
        }
    }
    
    state->i = i;
    state->mode = mode; 
    state->word = word;
    state->wordsize = wordsize; 
    return;
}

static Widget set_text(inW,get_next_char,data)
Widget inW;
int (*get_next_char)();
XtPointer data;
{    
   ParseState *state = CreateState(inW);
   munge_text(state,get_next_char,data);
   return DestroyState(state);
}


/*-----------------------------------------------------------------------*/
/* Declare a Widget class for the converter                              */
/*-----------------------------------------------------------------------*/

void SGMLHyperDeclareClass(class)
WidgetClass class;
{
   static int MaxClasses = 0;  
   if (NumberOfClasses == MaxClasses) 
     ClassList = (WidgetClass *)  XtRealloc((XtPointer) ClassList, (MaxClasses += 10) * sizeof(WidgetClass));
   ClassList[NumberOfClasses++] = class;
}

/*-----------------------------------------------------------------------*/
/* Create a new HyperWidget                                              */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateHyper(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLHyperWidgetClass,parent,al,ac);
}

/*-----------------------------------------------------------------------*/
/* Redisplay HyperWidget                                                 */
/*-----------------------------------------------------------------------*/

void SGMLHyperRedisplay(w)
Widget w;
{
   SGMLHyperWidget h = (SGMLHyperWidget) w;
   if (h->sgml_hyper.managed) calc_new_size(h);
}
/*-----------------------------------------------------------------------*/
/* Load the text from a file                                             */
/*-----------------------------------------------------------------------*/

/* provides chars to "set_text" routine */

static int get_from_file(f)
FILE **f;
{
    int n =  getc(*f);
    return n;
}

/* Public routine */

Widget SGMLHyperLoadFile(widget,fname)
Widget widget;
char   *fname;
{
//    extern char *sys_errlist[];
    Widget result = NULL;

    FILE *f = fopen(fname,"r");
    if(f)
    {
        result = set_text(widget,get_from_file,&f);
        fclose(f);
    }
    else
    {
        char msg[1024];
        sprintf(msg,"%s: %s",fname,sys_errlist[errno]);
        XtWarning(msg);
    }
    return result;  
}

/*-----------------------------------------------------------------------*/
/* Load text using a getchar like routine                                */
/*-----------------------------------------------------------------------*/

Widget SGMLHyperLoadText(widget,get_character,closure)
Widget widget;
int (*get_character)();
XtPointer closure;
{
   return set_text(widget,get_character,closure);
}

/*-----------------------------------------------------------------------*/
/* Load text from memory buffer                                          */
/*-----------------------------------------------------------------------*/

/* provides chars to "set_text" routine */

static int get_from_buffer(buffer)
char **buffer;
{
    char c = **buffer;
    (*buffer)++;
    return c?c:EOF;
}

/* Public routines */

Widget SGMLHyperSetText(widget,text)
Widget  widget;
char *text;
{
    return set_text(widget,get_from_buffer,&text);
}

XtPointer SGMLHyperOpen(widget)
Widget widget;
{
  return (XtPointer) CreateState(widget);
}
void SGMLHyperWrite(state,text)
XtPointer state;
char *text;
{
  munge_text((ParseState *) state, get_from_buffer, &text); 
}
void SGMLHyperWriteData(state,get_character,closure)
XtPointer state;
int (*get_character)();
XtPointer closure;
{
  munge_text((ParseState *) state, get_character, closure); 
}
Widget SGMLHyperClose(state)
XtPointer state;
{
  return DestroyState((ParseState *) state);
}

/*-----------------------------------------------------------------------*/
/* SGMLHyperSearch: Public routine                                       */
/*-----------------------------------------------------------------------*/
Widget SGMLHyperSearch(w,exp,sens,cont)
SGMLHyperWidget w;
char *exp;
Boolean sens;
Boolean cont;
{
   Widget result = NULL;


   if (w->sgml_hyper.managed)
     {
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);

       if (!cont && w->sgml_hyper.search_widget)
         {
           (*childClass->sgml_text_class.clear_select)(w->sgml_hyper.managed,w->sgml_hyper.sw_flag,TRUE); 
           w->sgml_hyper.search_widget = NULL;
         }

       /* clear the already hilite widget */        
       if (w->sgml_hyper.search_widget)
         (*childClass->sgml_text_class.hilite)(w->sgml_hyper.managed,SGMLSELECT_WORD);

       result = (*childClass->sgml_text_class.search)(exp,sens,w->sgml_hyper.managed,TRUE,NULL,NULL,&w->sgml_hyper.search_widget);
       if (result)
         (*childClass->sgml_text_class.hilite)(w->sgml_hyper.managed,SGMLSELECT_WORD);
     }

   /* clear the last search widget */
   if (w->sgml_hyper.sw_flag && !result)
     {
       XClearArea(XtDisplay((Widget)w),XtWindow((Widget)w),0,0,0,0,True);
       w->sgml_hyper.sw_flag = NULL;
     }
     
   w->sgml_hyper.search_widget = result;
   w->sgml_hyper.sw_flag = w->sgml_hyper.search_widget;
   return result;        
}

/*-----------------------------------------------------------------------*/
/* Specifies start and end of highlignt chars                            */
/*-----------------------------------------------------------------------*/

 
void SGMLHyperSetTags(widget,open_tag,close_tag,end_tag,parameter_tag)
Widget widget;
unsigned char open_tag;
unsigned char close_tag;
unsigned char end_tag;
unsigned char parameter_tag;
{
    ((SGMLHyperWidget)widget)->sgml_hyper.open_tag      = open_tag;
    ((SGMLHyperWidget)widget)->sgml_hyper.close_tag     = close_tag;
    ((SGMLHyperWidget)widget)->sgml_hyper.end_tag       = end_tag;
    ((SGMLHyperWidget)widget)->sgml_hyper.parameter_tag = parameter_tag;
}


/*-----------------------------------------------------------------------*/
/* convert a string to lower case                                        */
/*-----------------------------------------------------------------------*/

static void lowcase(p)
register char *p;
{
    while(*p)
    {
        *p = tolower(*p); 
        p++;
    }
}

/*-----------------------------------------------------------------------*/
/* Returns the text of the widget                                        */
/* the memory is allocated. It must be freed by the application          */
/* If include_tags is FALSE, the special characters are not returned     */
/*-----------------------------------------------------------------------*/

static void dump_text(hyper,w,include_tags,calc_len,pp)
SGMLHyperWidget hyper;
SGMLTextObject w;
Boolean include_tags, calc_len;
char **pp;
{
  char *special = "<>&\"";
  char ot = hyper->sgml_hyper.open_tag;
  char ct = hyper->sgml_hyper.close_tag;
  char et = hyper->sgml_hyper.end_tag;
  char pt = hyper->sgml_hyper.parameter_tag;
  char est = hyper->sgml_hyper.entity_tag;
  char eet = hyper->sgml_hyper.entity_end_tag;
  Boolean supportsEntities;

  char *p;
  int len, matched; 
  char *name = XtName((Widget)w);

  Arg arglist[10];
  int n = 0;

  XtSetArg(arglist[n],SGMLNsupportsEntities,&supportsEntities); n++;
  XtSetArg(arglist[n],SGMLNmatched,&matched); n++;
  XtGetValues((Widget)w,arglist,n);

  if (w->object.being_destroyed) return;

  if (calc_len) len = (int) *pp;
  else          p   = *pp;   
 
  if (!strcmp(name,"default")) name = "";
  if (!strcmp(name,"hidden"))  { name = ""; supportsEntities = FALSE; }
  
  if (*name && include_tags)
  {
    char *q = name;
    if (calc_len) len++;
    else          *p++ = ot; 
    if (calc_len) len += strlen(q);
    else          for (; *q != '\0' && *q != '_'; ) *p++ = *q++; 
    if (w->sgml_text.param_length)
      {  
        char *param = w->sgml_text.param;
        if (calc_len) len++;
        else          *p++ = pt;
        if (calc_len) len += strlen(param);
        else          for (; *param != '\0';) *p++ = *param++;
      }               
    if (calc_len) len++;
    else          *p++ = ct;
  }  
  if (w->sgml_text.text)
  {
    if (supportsEntities)
      {
        char *text = w->sgml_text.text; 
        if (calc_len)
          {
            for (; *text != '\0';) 
              if (strchr(special,*text++)) len += 6;
              else len++;
          }
        else
          {
            for (; *text != '\0';) 
              if (strchr(special,*text)) { sprintf(p,"%c#%03d%c",est,(int)*text++,eet); p += 6; }
              else  *p++ = *text++;
          }
      } 
    else
      {
        char *text = w->sgml_text.text; 
        if (calc_len) len += strlen(text);
        else          for (; *text != '\0';) *p++ = *text++;
      } 
  }
  if (SGMLIsCompositeText((Widget)w))
  {
    int n; 
    WidgetList children;
    int nn = 0;
    Arg arglist[10];
    
    XtSetArg(arglist[nn],XtNnumChildren,&n); nn++;
    XtSetArg(arglist[nn],XtNchildren,&children); nn++;
    XtGetValues((Widget)w,arglist,nn); 
    
    if (calc_len) for (; n-- > 0; children++) dump_text(hyper,*children,include_tags,calc_len,&len);
    else          for (; n-- > 0; children++) dump_text(hyper,*children,include_tags,calc_len,&p);   
  }
  if (matched && include_tags && *name)
  {
    char *q = name;
    if (calc_len) len ++;
    else          *p++ = ot;
    if (calc_len) len ++;
    else          *p++ = et;
    if (calc_len) len += strlen(q);
    else          for (; *q != '\0' && *q != '_'; ) *p++ = *q++; 
    if (calc_len) len ++; 
    else          *p++ = ct;
  }  
  if (calc_len) *pp = (char *) len;
  else          *pp = p; 
}

char *SGMLHyperGetText(widget, include_tags)
Widget  widget;
Boolean include_tags;
{

    SGMLHyperWidget w = (SGMLHyperWidget)widget;
    char  *result, *p;
    int   len = 1;
     
    if (w->sgml_hyper.managed)
      {
        dump_text(w,w->sgml_hyper.managed,include_tags,TRUE,&len);
        result = p = XtMalloc(len);
        dump_text(w,w->sgml_hyper.managed,include_tags,FALSE,&p);

        *p++ = '\0';
        return result;
      }
    else return NULL; 
}

/*-----------------------------------------------------------------------*/
/* Only for Motif                                                        */
/* If the widget is in a XmScrolledWindow, scroll it so the selection is */
/* visible                                                               */
/*-----------------------------------------------------------------------*/

void SGMLHyperShowObject(h,object,sel)
SGMLHyperWidget h;
SGMLTextObject object;
Boolean sel;
{
#define SetArg(a,b)  XtSetArg(al[ac],a,b);ac++
#define GetValues(w) XtGetValues(w,al,ac);ac=0
#define SetValues(w) XtSetValues(w,al,ac);ac=0

    Widget clip = XtParent(h);
    Widget swin;

    Widget h_scroll;
    Widget v_scroll;

    int ac = 0;

    Position    x_parent,y_parent;
    Position    x_grep,y_grep;
    Dimension   h_grep,w_grep;
    Position    x_clip,y_clip;
    Dimension   h_clip,w_clip;
    Position    dv=0,dh=0;
    int min,max;
    int v_val,v_size,v_inc,v_page;
    int h_val,h_size,h_inc,h_page;
    Position x,y;

    Arg al[5];

    /* check if selection exists */

    /* check if the widget is in a scrolled window */
    /* the XmScrolledWindow creates a clip window  */
    /* The widget's parent is the clip window      */


    if(!clip) return;
    swin = XtParent(clip);

    if(!swin || !XmIsScrolledWindow(swin)) return;


    /* Get window scroll bars */

    SetArg(XmNhorizontalScrollBar, &h_scroll);
    SetArg(XmNverticalScrollBar  , &v_scroll);
    GetValues(swin);

    /* Get size of clip window and selection rect */

    w_clip = clip->core.width;
    h_clip = clip->core.height;

    /* If no object specified then position at top */
    
    if (!object)
    {
        SetArg(XmNminimum,&min);
        GetValues(v_scroll);
        XmScrollBarGetValues(v_scroll,&v_val,&v_size,&v_inc,&v_page);
        dv = min - v_val;

        SetArg(XmNminimum,&min);
        GetValues(h_scroll);
        XmScrollBarGetValues(h_scroll,&h_val,&h_size,&h_inc,&h_page);
        dh = min - h_val;
    }
    else 
    {
      w_grep = object->sgml_text.width;
      h_grep = object->sgml_text.height;

      /* Get global coordinates of clip and selection rect */

      XtTranslateCoords(clip,0,0,&x_clip,&y_clip);
      if (sel && object->sgml_text.begin_select)
        {
          Position sx, sy;
          char *sb = object->sgml_text.begin_select;
          SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass((Widget)object);
          sx = object->sgml_text.margin;
          sy = object->sgml_text.begin.y;
          /* deliberately make object small */
          w_grep = 2;
          h_grep = 2;
          (*class->sgml_text_class.pos_to_xy)(object,&sx,&sy,sb,FALSE);
          XtTranslateCoords((Widget)h,sx,sy,&x_grep,&y_grep);
        }
      else
        XtTranslateCoords((Widget)h,object->sgml_text.margin,object->sgml_text.begin.y,&x_grep,&y_grep);
  
      /* offset of selection within clip window */

      x = x_grep - x_clip;
      y = y_grep - y_clip;


      /* selection y coordinate is not visible */

      if( y < 0 || y + h_grep > h_clip)
      {
          /* the widget must be moved verticaly by dv pixels */

          dv = (y + h_grep / 2)  - h_clip / 2;

          SetArg(XmNminimum,&min);
          SetArg(XmNmaximum,&max);

          GetValues(v_scroll);

          XmScrollBarGetValues(v_scroll,&v_val,&v_size,&v_inc,&v_page);

          max -= v_size;
  
          if( dv + v_val > max ) dv = max - v_val;
          if( dv + v_val < min ) dv = min - v_val;
  
  
      }

      /* selection x coordinate is not visible */

      if( x < 0 || x + w_grep > w_clip)
      {
          /* the widget must be moved horizontaly by dh pixels */

          dh = (x + w_grep / 2)  - w_clip / 2;

          SetArg(XmNminimum,&min);
          SetArg(XmNmaximum,&max);
          GetValues(h_scroll);

          XmScrollBarGetValues(h_scroll,&h_val,&h_size,&h_inc,&h_page);

          max -= h_size;

          if( dh + h_val > max ) dh = max - h_val;
          if( dh + h_val < min ) dh = min - h_val;

      }
    
    }
    /* if the widget must be moved */

    if(dv || dh)
    {
        Position x = h->core.x-dh;
        Position y = h->core.y-dv;

        /* move it */

        SetArg(XmNx,x);
        SetArg(XmNy,y);
        SetValues((Widget) h);

        /* update scroll bars */

        if(dv) XmScrollBarSetValues(v_scroll,v_val+dv,v_size,v_inc,
            v_page,TRUE);
        if(dh) XmScrollBarSetValues(h_scroll,h_val+dh,h_size,h_inc,
            h_page,TRUE);


    }

}

/*-----------------------------------------------------------------------*/
/* Clear previous selection                                              */
/*-----------------------------------------------------------------------*/

static void clear_selection(w)
SGMLHyperWidget w;
{
/*
    if(w->sgml_hyper.grep_seg)
    {
        if(XtIsRealized(w))
*/
            /* force a redraw */
/*
            XClearArea(XtDisplay(w),XtWindow(w),
                w->sgml_hyper.grep_x,
                w->sgml_hyper.grep_y,
                w->sgml_hyper.grep_width,
                w->sgml_hyper.grep_height,
                TRUE);

    }
    w->sgml_hyper.grep_seg = NULL;
*/
}

/*-----------------------------------------------------------------------*/
/* Set the new selection                                                 */
/*-----------------------------------------------------------------------*/

static void set_selection(w)
SGMLHyperWidget w;
{
/*
    if(w->sgml_hyper.grep_seg)
    {
        text_segment *s = w->sgml_hyper.grep_seg;
        char *p = s->text;
        XCharStruct   char_info;
        int dir,ascent,desc;
*/
        /* get size of the begining of
           the segment, up to the found string */
/*
        XTextExtents(
            s->style->sgml_style.font,
            s->text,
            w->sgml_hyper.grep_off,
            &dir,&ascent,&desc,&char_info);

        w->sgml_hyper.grep_x      = s->x + char_info.width;
        w->sgml_hyper.grep_y      = s->y + desc;
        w->sgml_hyper.grep_height = s->height;
*/
        /* Get size of the selection */
/*
        XTextExtents(
            s->style->sgml_style.font,
            w->sgml_hyper.grep_txt,
            w->sgml_hyper.grep_len,
            &dir,&ascent,&desc,&char_info);


        w->sgml_hyper.grep_width  = char_info.width;
*/
        /* force update */
/*
        if(XtIsRealized(w))
            XClearArea(XtDisplay(w),XtWindow(w),
                w->sgml_hyper.grep_x,
                w->sgml_hyper.grep_y,
                w->sgml_hyper.grep_width,
                w->sgml_hyper.grep_height,
                TRUE);
    }
*/
}

/* size of regexp buffer */

#define ESIZE 1024

/*-----------------------------------------------------------------------*/
/* if you have regexp, define USE_REGEXP                                 */
/* NOTE : since regexp variables and functions are not static            */
/* you can have some problems if you use the same names or include       */
/* regexp somewhere else                                                 */
/*-----------------------------------------------------------------------*/
#ifdef USE_REGEXP

/* regexp macros ... see "man regexp" */

#define INIT        register char *sp = instring;
#define GETC()      (*sp++)
#define PEEKC()     (*sp)
#define UNGETC(c)   (--sp)
#define RETURN(c)   return;
#define ERROR(c)    fprintf(stderr,"Warning regexp error %d\n",c)


#include <regexp.h>


#else 

/*-----------------------------------------------------------------------*/
/* If we don't have regexp mimic it.                                     */
/* Just find plain text using strncmp. no regular expression matching    */
/*-----------------------------------------------------------------------*/

static char *loc1,*loc2;
static int len;

static compile(w,buf,end,dummy)
char *w,*buf;
int end;
int dummy;
{
    strcpy(buf,w);
    len = strlen(w);
}

static step(w,buf)
char *w;
char *buf;
{
    loc1 = w;
    while(*loc1)
    {
        if(strncmp(loc1,buf,len) == 0)
        {
            loc2 = loc1+len;
            return TRUE;
        }
        loc1++;
    }
    return FALSE;
}


#endif

/*-----------------------------------------------------------------------*/
/* Select a word in the hyper widget                                     */
/* word : word to find ( or regular expression if USE_REGEX is defined)  */
/* ignore_case : if TRUE ignore case in comparaison                      */
/* from_start : if TRUE search from start of text, else search from      */
/* current selection                                                     */
/* wrap: if TRUE, continue search from the begining of text if the end   */
/* is reached                                                            */
/*-----------------------------------------------------------------------*/

#ifdef _NO_PROTO

Boolean SGMLHyperGrep(widget,word,ignore_case,from_start,wrap)
Widget   widget;
char     *word;
Boolean  ignore_case;
Boolean  from_start;
Boolean  wrap;

#else

Boolean SGMLHyperGrep(Widget widget,
		      char *word,
     		      Boolean ignore_case,
		      Boolean from_start,
		      Boolean wrap)

#endif

{
#ifdef dummy
    SGMLHyperWidget  h = (SGMLHyperWidget)widget;
    char         *w = word;
    char         *p;
    int          offset,from,to;
    text_segment *s;
    char          expbuf[ESIZE];

    if(!h->sgml_hyper.first_seg) return;

    if(ignore_case)
    {
        /* if ignore case, change word to lower case */ 
        w = XtNewString(word);
        lowcase(w);
    }

    /* compile the regular expression */
    compile(w,expbuf,&expbuf[ESIZE],'\0');


    if(ignore_case) XtFree(w);

    /* if from_start or no previous selection, 
       start from first segment */

    if(from_start || h->sgml_hyper.grep_seg == NULL)
    {
        offset=0;
        wrap = FALSE;
        s = h->sgml_hyper.first_seg;
    }
    else 
    {
        /* start from last selection */

        offset = h->sgml_hyper.grep_off + h->sgml_hyper.grep_len;
        s = h->sgml_hyper.grep_seg;
    }

    for(;;)
    {
        if(s->text)
        {
            if(ignore_case)
            {
                /* if ignore case, change segment to lower case */
                p = XtNewString(s->text);
                lowcase(p);
            }

            /* search the string */

            if(step(p+offset,expbuf))
            {
                /* if found ...*/

                /* clear previous selection */
                clear_selection(h);

                h->sgml_hyper.grep_seg = s;
                h->sgml_hyper.grep_off = offset + (loc1-(p+offset));
                h->sgml_hyper.grep_txt = s->text + h->sgml_hyper.grep_off;
                h->sgml_hyper.grep_len = loc2-loc1;

                /* set new selection */

                set_selection(h);

                /* make it visible */

                show_selection(h);

                if(ignore_case) XtFree(p);

                return TRUE;
            }

            if(ignore_case) XtFree(p);
        }

        offset = 0;
        s = s->next;

        /* if end of text and wrap mode, go to start of text */
        if(!s)
            if(wrap)
            {
                wrap = FALSE;
                s = h->sgml_hyper.first_seg;
            }
            else break;

    }


#endif
    return FALSE;
}

static Boolean Convert(w, selection, target, type, value, length, format)
Widget w;
Atom *selection, *target, *type;
XtPointer *value;
unsigned long *length;
int *format;
{
  Display *d = XtDisplay(w);
  SGMLHyperWidget hw = (SGMLHyperWidget)w;

  if (*selection != XA_PRIMARY) return False; 
   
  /*
   * XA_TARGETS identifies what targets the text widget can
   * provide data for.
   */
  if (*target == XA_TARGETS(d)) 
     {
       Atom* targetP;
       Atom* std_targets;
       unsigned long std_length;
       XmuConvertStandardSelection(
                    w, hw->sgml_hyper.selection_time, selection,
                    target, type, (caddr_t*)&std_targets, &std_length, format
                   );
       *length = std_length + 5;
       *value = (XtPointer)XtMalloc(sizeof(Atom)*(*length));
       targetP = *(Atom**)value;
       *targetP++ = XA_STRING;
       *targetP++ = XA_TEXT(d);
       *targetP++ = XA_COMPOUND_TEXT(d);
       *targetP++ = XA_LIST_LENGTH(d);
       *targetP++ = XA_LENGTH(d);
       bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
       XtFree((char*)std_targets);
       *type = XA_ATOM;
       *format = 32;
       return TRUE;
     } 

   if (*target == XA_STRING || 
       *target == XA_TEXT(d)||
       *target == XA_COMPOUND_TEXT(d) ) 
     {
       Boolean result = TRUE;

       if (*target == XA_COMPOUND_TEXT(d))
         *type = *target;
       else
         *type = XA_STRING;
      
       if (hw == NULL) return FALSE;

       if (hw->sgml_hyper.copy_text == NULL && hw->sgml_hyper.search_widget)
         result = GetSelection(hw,SGMLDUMP_ASCII);  

      *format = 8;
      if (result && hw->sgml_hyper.copy_text)
        {
          *length = (unsigned long) strlen(hw->sgml_hyper.copy_text);
          *value = hw->sgml_hyper.copy_text;
        }
      else
        {
          *length = 0;
          *value = NULL;
        }

       return TRUE;
     } 

   if (*target == XA_LIST_LENGTH(d))
     {
       *value = XtMalloc(4);
        if (sizeof(long) == 4)
          {
            *(long*)*value = 1;
          }
        else
          {
            long temp = 1;
            bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
          }
        *type = XA_INTEGER;
        *length = 1;
        *format = 32;
        return TRUE;
     }

   if (*target == XA_LENGTH(d))
     {
        *value = XtMalloc(4);
        if (sizeof(long) == 4)
            *(long*)*value = strlen(hw->sgml_hyper.copy_text);
        else {
            long temp = strlen(hw->sgml_hyper.copy_text);
            bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
        }
        *type = XA_INTEGER;
        *length = 1;
        *format = 32;
        return True;
     }

   if (XmuConvertStandardSelection(w, hw->sgml_hyper.selection_time, selection,
                                   target, type,
                                   (caddr_t *)value, length, format))
     return TRUE;

   /* else */
   return FALSE;

}

static void LoseOwnership(w, selection)
Widget w;
Atom *selection;
{
   SGMLHyperWidget hw = (SGMLHyperWidget)w;
   SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(hw->sgml_hyper.managed);

   if (hw->sgml_hyper.copy_text)
     {
       XtFree(hw->sgml_hyper.copy_text);
       hw->sgml_hyper.copy_text = NULL;
     }

   if (hw->sgml_hyper.sw_flag || hw->sgml_hyper.search_widget)
     {
       (*childClass->sgml_text_class.clear_select)(hw->sgml_hyper.managed,hw->sgml_hyper.sw_flag,TRUE);
        hw->sgml_hyper.search_widget = NULL;
        hw->sgml_hyper.sw_flag = NULL;
      }
}

static void SelectionDone(w, selection, target)
Widget w;
Atom *selection, *target;
{
        /* empty proc so Intrinsics know we want to keep storage */
}

static Boolean GetSelection(hw,type)
SGMLHyperWidget hw;
int type;
{
   Display *d = XtDisplay((Widget)hw);
   SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(hw->sgml_hyper.managed);
   int len = 0;
   char *p;
   SGMLDumpTextInfo dp;

   dp.sw = hw->sgml_hyper.search_widget;
   dp.begin_sw = FALSE;
   dp.calc_len = TRUE;
   dp.format_type = type;
   dp.copy_type = hw->sgml_hyper.select_index;
   dp.current_pos = NULL;
   dp.last_pos = NULL;
   dp.line_spacing = 0;
   dp.spacing = 0;
   dp.last_len = 0;
   (*childClass->sgml_text_class.dump_text)(hw->sgml_hyper.managed,&dp,&len);

   if (!len) return FALSE;

   dp.calc_len = FALSE;
   dp.begin_sw = FALSE;
   dp.line_spacing = 0;
   dp.spacing = 0;
   hw->sgml_hyper.copy_text = p = XtMalloc((unsigned)len+1);
   dp.current_pos = dp.last_pos = hw->sgml_hyper.copy_text;
   (*childClass->sgml_text_class.dump_text)(hw->sgml_hyper.managed,&dp,&p);
   *p = '\0';

   return TRUE;
}
