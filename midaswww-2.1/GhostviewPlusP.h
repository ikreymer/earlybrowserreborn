/*
 * GhostviewPlusP.h -- Private header file for GhostviewPlus widget.
 * Copyright (C) 1993  Tony Johnson
 *
 * Rather trivially expands the capabilities of the Ghostview Widget by
 * encapsulating some of the other functionality of the ghostview
 * application.
 */
 
#ifndef _GhostviewPlusP_h
#define _GhostviewPlusP_h

#include "GhostviewP.h"
#include "GhostviewPlus.h"
#include "ps.h"

typedef struct {
    XtPointer empty;
} GhostviewPlusClassPart;

typedef struct _GhostviewPlusClassRec {
    CoreClassPart		core_class;
    GhostviewClassPart		ghostview_class;
    GhostviewPlusClassPart	ghostview_plus_class;
} GhostviewPlusClassRec;

extern GhostviewPlusClassRec ghostviewPlusClassRec;

typedef struct {
    /* resources */
    
    int pages; /* nuber of pages in document (or -1 if unknown) */
    int current_page;    /* Current page number diplayed */ 
    char *filename;
    int magstep;

    /* private state */
    
    struct document *doc;
    float xdpi, ydpi;

} GhostviewPlusPart;

typedef struct _GhostviewPlusRec {
    CorePart		core;
    GhostviewPart	ghostview;
    GhostviewPlusPart	ghostview_plus;

} GhostviewPlusRec;

#endif /* _GhostviewPlusP_h */
