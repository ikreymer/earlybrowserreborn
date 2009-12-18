/*==================================================================*/
/*                                                                  */
/* SGMLFormTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLFormTextP.h"

/* 
  Private functions 
*/


/* 
  Private data struct 
*/


/*
  Widget class methods
*/

static void    Initialize();
static void    Destroy();

#define Offset(field) XtOffsetOf(SGMLFormTextRec,sgml_form_text.field)

static XtResource resources[] = {

    {SGMLNmethod,SGMLCMethod,SGMLRQuark,sizeof(XrmQuark),
     Offset (method),XtRString,(XtPointer) "get"},

    {SGMLNresult,XtCReadOnly,XtRString,sizeof(String),
     Offset (result),XtRImmediate,(XtPointer)0},

    {SGMLNaction,SGMLCAction,XtRString,sizeof(String),
     Offset (action),XtRImmediate,(XtPointer)0},

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLFormTextClassRec  sGMLFormTextClassRec = {
    {
    (WidgetClass) &sGMLCompositeTextClassRec,     
                                         /* superclass            */
    "SGMLFormText",                      /* class_name            */
    sizeof(SGMLFormTextRec),             /* widget_size           */
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
    SGMLInheritComputeSize,          	 /* compute_size          */
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
    SGMLInheritGeometryManager,          /* geometry_manager      */
    SGMLInheritChangeManaged,            /* change_managed        */
    SGMLInheritInsertChild,              /* insert_child          */
    SGMLInheritDeleteChild,              /* delete_child          */
    SGMLInheritCallAdjustSize,           /* call_adjust_size      */
    NULL,                                /* ignore                */
    },
    {
    NULL,                                /* extension             */
    }
};

WidgetClass sGMLFormTextObjectClass = (WidgetClass) &sGMLFormTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLFormTextObject request, new;
{
  char    *action = new->sgml_form_text.action;

  if (action) new->sgml_form_text.action = XtNewString(action);

  new->sgml_form_text.result = NULL;
}
 
/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/
 
static void Destroy (w)
SGMLFormTextObject w;
{

    XtFree(w->sgml_form_text.action);
    XtFree(w->sgml_form_text.result);
} 
 
/*--------------------------------------------------------------*/
/* Set Result                                                   */
/*--------------------------------------------------------------*/
 
void SGMLFormSetResult(w,result)
SGMLFormTextObject w;
char *result;
{
  SGMLFormTextObjectClass class = (SGMLFormTextObjectClass) XtClass((Widget) w); 

  XtFree(w->sgml_form_text.result);
  w->sgml_form_text.result = result;

  (*class->sgml_text_class.activate)(w,NULL);
}
