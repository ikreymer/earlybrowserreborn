/*==================================================================*/
/*                                                                  */
/* SGMLTextObject                                                   */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLTEXTP_H
#define SGMLTEXTP_H

#include "SGMLText.h"

/* SGMLText class */

typedef void    (*ComputeSizeProc)();
typedef Boolean (*AdjustSizeProc)();
typedef void    (*AdjustPositionProc)();
typedef void    (*ExposeProc)();
typedef void    (*ActivateProc)();
typedef void    (*HiliteProc)();
typedef Widget  (*ContainsProc)();
typedef void    (*CallCreateCallbackProc)();
typedef void    (*CallMapCallbackProc)();
typedef void    (*MakeVisibleProc)();
typedef Boolean (*SGMLSetValuesProc)(); 
typedef Boolean (*InheritChangesProc)();
typedef int     (*ComputeChangeMaskProc)();
typedef Widget  (*SearchProc)();
typedef Boolean (*ClearSelectProc)();
typedef Widget  (*HiliteSelectionProc)();
typedef void    (*XYToPosProc)();
typedef void    (*PosToXYProc)();
typedef Boolean (*DumpTextProc)();

#define SGMLInheritComputeSize        ((ComputeSizeProc)        _XtInherit) 
#define SGMLInheritAdjustSize         ((AdjustSizeProc)         _XtInherit) 
#define SGMLInheritAdjustPosition     ((AdjustPositionProc)     _XtInherit) 
#define SGMLInheritExpose             ((ExposeProc)             _XtInherit) 
#define SGMLInheritActivate           ((ActivateProc)           _XtInherit) 
#define SGMLInheritHilite             ((HiliteProc)             _XtInherit) 
#define SGMLInheritContains           ((ContainsProc)           _XtInherit) 
#define SGMLInheritCallCreateCallback ((CallCreateCallbackProc) _XtInherit) 
#define SGMLInheritCallMapCallback    ((CallMapCallbackProc)    _XtInherit) 
#define SGMLInheritMakeVisible        ((MakeVisibleProc)        _XtInherit) 
#define SGMLInheritComputeChangeMask  ((ComputeChangeMaskProc)  _XtInherit) 
#define SGMLInheritSearch             ((SearchProc)             _XtInherit)
#define SGMLInheritClearSelect        ((ClearSelectProc)        _XtInherit)
#define SGMLInheritHiliteSelection    ((HiliteSelectionProc)    _XtInherit)
#define SGMLInheritXYToPos            ((XYToPosProc)            _XtInherit)
#define SGMLInheritPosToXY            ((PosToXYProc)            _XtInherit)
#define SGMLInheritDumpText           ((DumpTextProc)           _XtInherit)

#define SGMLMLEFT_MARGIN        (1 << 0)
#define SGMLMRIGHT_MARGIN       (1 << 1)
#define SGMLMPARAGRAPH_INDENT   (1 << 2)
#define SGMLMSPACE_BEFORE       (1 << 3) 
#define SGMLMSPACE_AFTER        (1 << 4)
#define SGMLMBREAK_BEFORE       (1 << 5)
#define SGMLMBREAK_AFTER        (1 << 6)
#define SGMLMALIGNMENT          (1 << 7)
#define SGMLMSENSITIVE          (1 << 8)
#define SGMLMEXACT_TERMINATOR   (1 << 9)
#define SGMLMHIDE_ILLEGAL_FLAGS (1 << 10)
#define SGMLMSUPPORTS_ENTITIES  (1 << 11)

#define SGMLMOUTLINE            (1 << 12)
#define SGMLMUNDERLINE          (1 << 13)  
#define SGMLMUNDERLINE_HEIGHT   (1 << 14)
#define SGMLMUNDERLINE_STYLE    (1 << 15)
#define SGMLMCOLOR              (1 << 16)

#define SGMLMFAMILY             (1 << 17)
#define SGMLMWEIGHT             (1 << 18) 
#define SGMLMSLANT              (1 << 19)  
#define SGMLMWIDTH              (1 << 20) 
#define SGMLMSPACING            (1 << 21) 
#define SGMLMREGISTRY           (1 << 22)
#define SGMLMSIZE               (1 << 23)
#define SGMLMFONT16             (1 << 24)
 
static int gcMask = SGMLMUNDERLINE_HEIGHT | SGMLMCOLOR | SGMLMFAMILY |
                    SGMLMWEIGHT | SGMLMSLANT |  SGMLMWIDTH |  SGMLMSPACING |
                    SGMLMREGISTRY |  SGMLMSIZE | SGMLMUNDERLINE_STYLE |
                    SGMLMFONT16;

typedef struct _SGMLTextClassPart{

   ComputeSizeProc        compute_size;
   AdjustSizeProc         adjust_size;
   AdjustPositionProc     adjust_position;
   ExposeProc             expose;
   ActivateProc           activate;
   HiliteProc             hilite;
   ContainsProc           contains;
   CallCreateCallbackProc call_create_callback;
   CallMapCallbackProc    call_map_callback;
   MakeVisibleProc        make_visible;
   SGMLSetValuesProc      sgml_set_values;
   InheritChangesProc     inherit_changes; 
   ComputeChangeMaskProc  compute_change_mask; 
   SearchProc             search;
   ClearSelectProc        clear_select;
   HiliteSelectionProc    hilite_selection;
   XYToPosProc            xy_to_pos;
   PosToXYProc            pos_to_xy;
   DumpTextProc           dump_text;
   XtPointer              extension;

} SGMLTextClassPart;

typedef struct _SGMLTextClassRec{

    ObjectClassPart     object_class;
    SGMLTextClassPart   sgml_text_class;

} SGMLTextClassRec, *SGMLTextObjectClass;

extern SGMLTextClassRec sGMLTextClassRec;

typedef struct _SGMLTextPart {

    String              text;    /* pointer to text */
    String              param;   /* pointer to parameters */
    int                 length;         /* length of text */
    int                 param_length ;  /* length of parameters */

    SGMLCoord           begin;
    SGMLCoord           end;
    SGMLDimension       width,height;   /* Size of drawn text */

    /*
     * This horrible fix is to work around a bug in some versions
     * of the intrinsics where XtPhase2Destroy attempts to remove a widget
     * from its (non-widget) parent's popup list, (which doesn't exist)  
     */
#ifdef VAX
    char		dummy;
    int                 num_popups;
    Position            margin;
#else  /* HP seems to have problems unless we use this silly contruct */
#ifdef DEC /* dec - alpha osf */
    int                 num_popups;
    Position            margin;
#else
    Position            margin;
    int                 num_popups;
#endif
#endif
    int                 inheritance;
    int                 right_margin;
    int                 left_margin;
    int                 right_indent;
    int                 left_indent;
    int                 paragraph_indent;

    Dimension           space_before;
    Dimension           space_after;
    int                 break_before;
    int                 break_after;  
    int                 alignment;
    Boolean             sensitive;
    Boolean             managed;
    SGMLRendition       normal_rendition;
    Dimension           ascent;
    Dimension           descent;
    Dimension           spacing;
    Boolean             size_valid;
    XtPointer           userdata;
    
    Boolean             exact_terminator;
    Boolean		hide_illegal_tags;
    Boolean		supports_entities;
    int                 matched;

    char                *begin_select;
    char                *end_select;

    GC                  gc;
    GC                  invert_gc;
    GC                  reverse_gc;
    Boolean             own_gcs;
    XFontStruct         *font;

    XrmQuark            font16_name;
    GC                  font16_gc;
    GC                  font16_invert_gc;
    XFontStruct         *font16;

    XtCallbackList      activate;       /* callback list */
    XtCallbackList      create;         /* callback list */
    XtCallbackList      map;            /* callback list */
    XtCallbackList      unmap;          /* callback list */

} SGMLTextPart;

typedef struct _SGMLTextRec {
    ObjectPart        object;
    SGMLTextPart      sgml_text;
} SGMLTextRec;

#endif SGMLTEXTP_H
