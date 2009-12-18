/*==================================================================*/
/*                                                                  */
/* SGMLAnchorTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a anchortext segment for the SGMLHyper widget            */
/*                                                                  */
/* Revision history:                                                */
/*                                                                  */
/*   Nov 29 1992 Temporary hack to support quoted urls.   TonyJ     */
/*                                                                  */
/*==================================================================*/

#include <ctype.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLAnchorTextP.h"
#include <string.h>

/* 
  Private functions 
*/

/*
  Widget class methods
*/

static void    Initialize();
static void    Destroy();

#define Offset(field) XtOffsetOf(SGMLAnchorTextRec,sgml_anchortext.field)

static XtResource resources[] = {


    {SGMLNhref, SGMLCHref, XtRString, sizeof(String),
    Offset(href), XtRString, ""},

    {SGMLNname, SGMLCName, XtRString, sizeof(String),
    Offset(name), XtRString, ""},

    {SGMLNtitle, SGMLCTitle, XtRString, sizeof(String),
    Offset(title), XtRString, ""},
 
    {SGMLNhrefLength, XtCReadOnly, XtRInt, sizeof(int),
    Offset(href_length), XtRImmediate, 0},

    {SGMLNnameLength, XtCReadOnly, XtRInt, sizeof(int),
    Offset(name_length), XtRImmediate, 0},
 
    {SGMLNtitleLength, XtCReadOnly, XtRInt, sizeof(int),
    Offset(title_length), XtRImmediate, 0}
 
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLAnchorTextClassRec  sGMLAnchorTextClassRec = {
    {
    (WidgetClass) &sGMLCompositeTextClassRec,    /* superclass    */
    "SGMLAnchorText",                    /* class_name            */
    sizeof(SGMLAnchorTextRec),           /* widget_size           */
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
    SGMLInheritComputeSize,         	 /* compute_size          */
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    SGMLInheritExpose,                   /* expose                */
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
    SGMLInheritCallAdjustSize,           /* call_adjust_size      */
    NULL,                                /* ignore                */
    },
    {
    NULL                                 /* ignore                */
    }
};

WidgetClass sGMLAnchorTextObjectClass = (WidgetClass) &sGMLAnchorTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLAnchorTextObject request, new;
{
   if (new->sgml_anchortext.href)
     { 
       new->sgml_anchortext.href = XtNewString(new->sgml_anchortext.href);
       new->sgml_anchortext.href_length = strlen(new->sgml_anchortext.href);
     }  
   if (new->sgml_anchortext.name) 
     { 
       new->sgml_anchortext.name = XtNewString(new->sgml_anchortext.name);
       new->sgml_anchortext.name_length = strlen(new->sgml_anchortext.name);
     }     
   if (new->sgml_anchortext.title) 
     { 
       new->sgml_anchortext.title = XtNewString(new->sgml_anchortext.title);
       new->sgml_anchortext.title_length = strlen(new->sgml_anchortext.title);
     }  
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLAnchorTextObject w;
{
   XtFree(w->sgml_anchortext.href);
   XtFree(w->sgml_anchortext.name);
   XtFree(w->sgml_anchortext.title);
}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLAnchorTextObject                                     */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateAnchorText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLAnchorTextObjectClass,parent,al,ac);
}
