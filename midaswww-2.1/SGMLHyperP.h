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

#ifndef SGMLHYPERP_H
#define SGMLHYPERP_H
#define MOTIF
#include "SGMLHyper.h"

#ifdef MOTIF
#include <Xm/XmP.h>
#if XmVersion >= 1002
#include <Xm/ManagerP.h>
#endif
#endif

/* SGMLHyper class : no new fields */

typedef struct _SGMLHyperClassPart{
    int ignore;
} SGMLHyperClassPart;

typedef struct _SGMLHyperClassRec{
    CoreClassPart       core_class;
    CompositeClassPart  composite_class; 
#ifdef MOTIF
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
#endif
    SGMLHyperClassPart  sgml_hyper_class;
} SGMLHyperClassRec, *SGMLHyperWidgetClass;

extern SGMLHyperClassRec sGMLHyperClassRec;


typedef struct _SGMLHyperPart {

    Cursor    hand;                  /* Selecting cursor shape */
    char      open_tag;              /* start of highlighted text mark */
    char      close_tag;             /* end of highlighted text mark */
    char      end_tag;               /* character used to hide text in highlight */
    char      parameter_tag;         /* parameter tag */
    char      entity_tag;            /* entity tag */
    char      entity_end_tag;        /* entity ending tag */

    Boolean   case_sensitive_tags;   /* Case sensitive tags */
    Widget    managed;
    Widget    search_widget;
    Widget    sw_flag;
    int       cache_size;            /* maximum number of cached text segments */

    Dimension margin;                /* margins size */
    Dimension natural_width;	     /* size it tries to fit text into */
    
/*
    text_segment         *grep_seg;  /* segment where found text is *
*/
    char                 *grep_txt;  /* pointer to found text */
    int                   grep_len;  /* length of found text */
    int                   grep_off;  /* offset of found text */

    Position          grep_x;        /* rectangle of found text*/
    Position          grep_y;
    Dimension         grep_width;
    Dimension         grep_height;
    
    WidgetList        contains;      /* List of contained widgets */
    int               contain_slots;
    int               contain_used;

    WidgetList        cache_list;      /* List of cached widgets */
    int               cache_slots;
    int               cache_used;
 
    SGMLTextObject    last_selected; /* last selected segment     */
    SGMLTextObject    last_cursor;   /* last under cursor segment */

    XtCallbackList    activate;      /* callback list             */

    Time              last_time;     /* time of last button click */
    int               select_index;  /* select index              */
    char              *copy_text;    /* copy text                 */
    Time              selection_time;
    Boolean           race;          /* race condition            */
    Boolean           select_start;  /* flag for select start     */
    SGMLCopyInfo      copy_info;

} SGMLHyperPart;

typedef struct _SGMLHyperRec {
    CorePart          core;
    CompositePart     composite; 
#ifdef MOTIF
    ConstraintPart    constraint;
    XmManagerPart     manager;
#endif
    SGMLHyperPart     sgml_hyper;
} SGMLHyperRec;

#endif SGMLHYPERP_H
