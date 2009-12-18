/*
 * Ghostview.h -- Public header file for GhostviewPlus widget.
 */
 
#ifndef _GhostviewPlus_h
#define _GhostviewPlus_h

#include "Ghostview.h"

/****************************************************************
 *
 * Ghostview widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 pages		     Pages		int		dynamic
 currentPage	     CurrentPage	int		1	*/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#define XtNpages "pages"
#define XtCPages "Pages"
#define XtNcurrentPage "currentPage"
#define XtCCurrentPage "CurrentPage"
#define XtNdoc	"doc"
#define XtCDoc	"Doc"
#define XtNmagstep	"magstep"
#define XtCMagstep	"Magstep"
 
/* declare specific GhostviewPlusWidget class and instance datatypes */

typedef struct _GhostviewPlusClassRec*	GhostviewPlusWidgetClass;
typedef struct _GhostviewPlusRec*	GhostviewPlusWidget;

/* declare the class constant */

extern WidgetClass ghostviewPlusWidgetClass;

#endif /* _GhostviewPlus_h */
