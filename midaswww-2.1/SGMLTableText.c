/*==================================================================*/
/*                                                                  */
/* SGMLTableTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "SGMLTableTextP.h"
#include "SGMLMarkerText.h"
/* 
  Private functions 
*/

/* 
  Private data struct 
*/

static XrmQuark tdQuark, trQuark, thQuark;

/*
  Widget class methods
*/

static void    ComputeSize();
static void    ClassInitialize();

#define Offset(field) XtOffsetOf(SGMLTableTextRec,sgml_table_text.field)

static XtResource resources[] = {

    {SGMLNrows, XtCReadOnly, XtRInt, sizeof(int),
     Offset(rows), XtRImmediate, (XtPointer) 0}, 

    {SGMLNcolumns, XtCReadOnly, XtRInt, sizeof(int),
     Offset(cols), XtRImmediate, (XtPointer) 0}, 
 
    {SGMLNborder, SGMLCBorder, XtRBoolean, sizeof(Boolean),
     Offset(border), XtRImmediate, (XtPointer) TRUE} 
 
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLTableTextClassRec  sGMLTableTextClassRec = {
    {
    (WidgetClass) &sGMLCaptionTextClassRec, 
                                         /* superclass            */
    "SGMLTableText",                     /* class_name            */
    sizeof(SGMLTableTextRec),            /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    NULL,                                /* initialize            */
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
    NULL,                                /* destroy               */
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
    ComputeSize,          	         /* compute_size          */
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
    NULL,                                /* ignore                */
    },
    {
    NULL,                                /* ignore                */
    },
 };

WidgetClass sGMLTableTextObjectClass = (WidgetClass) &sGMLTableTextClassRec;

/*--------------------------------------------------------------*/
/* ClassInitialize: Register the converters                     */
/*--------------------------------------------------------------*/

static void ClassInitialize()
{
  tdQuark = XrmPermStringToQuark("td");
  thQuark = XrmPermStringToQuark("th");
  trQuark = XrmPermStringToQuark("tr");
}
static void DoNothing() {}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLTableTextObject w;
SGMLGeometry *geom;
CallAdjustSizeProc Adjust;
Opaque Closure;
{
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Boolean *managed = w->sgml_composite_text.managed;
   SGMLPosition xmin, xmax;
   int ncols, nrows, icol, irow, iw, iwa;
   Widget *wl;
   SGMLGeometry tableGeom; 

   /*
    * Always Break before
    */  

   _SGMLBreak(geom, w->sgml_text.space_before);

   /*
    * Deal with caption first
    */
  
   w->sgml_text.begin = geom->coord;
   xmin = xmax = geom->coord.x; 

   if (w->sgml_caption_text.caption)
     {
       SGMLTextObject t = (SGMLTextObject) w->sgml_caption_text.caption; 
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) t);
 
       (*childClass->sgml_text_class.compute_size)(t,geom,DoNothing,NULL);

       if (t->sgml_text.margin < xmin) xmin = t->sgml_text.margin;
       if (t->sgml_text.margin + t->sgml_text.width > xmax)
       xmax = t->sgml_text.margin + t->sgml_text.width;  
    }
   
   /* 
    * Calculate how many rows/columns there are
    */
   
   icol = 0; ncols = 0 ; nrows = 0; iw = 0; iwa = 0; wl = NULL;

   for (; n-- > 0; children++, managed++)
     if (*managed && *children != w->sgml_caption_text.caption)
        if (SGMLIsMarkerText(*children))
          {
            SGMLTextObject t = (SGMLTextObject) *children; 
            if (t->object.xrm_name == thQuark || t->object.xrm_name == tdQuark) 
              {
                if (iw == iwa) wl = (Widget *) XtRealloc((char *) wl,(iwa += 10) * sizeof(Widget *));
                wl[iw++] = *children; 
                icol++;  
              }
            else if (t->object.xrm_name == trQuark) 
              { 
                if (iw == iwa) wl = (Widget *) XtRealloc((char *) wl,(iwa += 10) * sizeof(Widget *));
                wl[iw++] = *children; 
                nrows++; 
                if (icol > ncols ) ncols = icol;
                icol = 0;
              }
          }
   w->sgml_table_text.rows = nrows;
   w->sgml_table_text.cols = ncols;
   w->sgml_table_text.widgets = wl;

   /* 
    *  Allocate some arrays
    */
   
   w->sgml_table_text.columnWidth = (SGMLDimension *) XtMalloc(ncols * sizeof(SGMLDimension *));
   w->sgml_table_text.rowHeight   = (SGMLDimension *) XtMalloc(nrows * sizeof(SGMLDimension *));
   w->sgml_table_text.widgets = wl;
   
   for (irow=0; irow<nrows ; irow++) w->sgml_table_text.rowHeight[irow] = 0;
   for (icol=0; icol<ncols ; icol++) w->sgml_table_text.columnWidth[icol] = 0;
   printf("nrows = %d ncols = %d\n",nrows,ncols);

   tableGeom.natural_width = geom->natural_width/ncols;   

   /*
    * Query children?
    */  
   n = w->sgml_composite_text.num_children;
   children = w->sgml_composite_text.children;

   icol = -1; irow = 0; iw = 0;
   for (; n-- > 0; children++, managed++)
     {
       if (wl[iw] == *children) 
         { 
           SGMLDimension h = tableGeom.coord.y + tableGeom.coord.ascent + tableGeom.coord.descent;
           if (icol >=0 && w->sgml_table_text.columnWidth[icol] < tableGeom.actual_width)
             w->sgml_table_text.columnWidth[icol] = tableGeom.actual_width; 
           if (icol >=0 && h > w->sgml_table_text.rowHeight[irow])
             w->sgml_table_text.rowHeight[irow] = h;
           icol++;
           if (icol == ncols) { icol = -1; irow++; }        
           tableGeom.actual_width = 0;
           tableGeom.broken = TRUE;        
           tableGeom.space = 0;
           tableGeom.leave_space = FALSE;
           tableGeom.coord.x = 0;
           tableGeom.coord.y = 0;
           tableGeom.coord.ascent = 0;
           tableGeom.coord.descent = 0; 
           iw++;
         }
       else if (*managed && *children != w->sgml_caption_text.caption)
         {     
           SGMLTextObject t = (SGMLTextObject) *children; 

           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           SGMLCompositeTextObjectClass myClass = (SGMLCompositeTextObjectClass) XtClass((Widget) w);

           (*childClass->sgml_text_class.compute_size)
              (*children,&tableGeom,myClass->sgml_composite_text_class.call_adjust_size,*children);
          
           if (t->sgml_text.margin < xmin) xmin = t->sgml_text.margin;
           if (t->sgml_text.margin + t->sgml_text.width > xmax)
             xmax = t->sgml_text.margin + t->sgml_text.width; 
         }  
     }  

   for (irow = 0; irow < nrows; irow++) printf("irow = %d height = %d\n",irow,w->sgml_table_text.rowHeight[irow]);
   for (icol = 0; icol < ncols; icol++) printf("icol = %d width = %d\n", icol,w->sgml_table_text.columnWidth[icol]);
   /*
    * Query children one final time, set final size of children
    */  

   n = w->sgml_composite_text.num_children;
   children = w->sgml_composite_text.children;

   icol = -1; irow = 0; iw = 0;
   for (; n-- > 0; children++, managed++)
     {
       if (wl[iw] == *children) 
         {            
           if (icol >= 0) geom->coord.x += w->sgml_table_text.columnWidth[icol];         
           icol++;
           if (icol == ncols) 
             { 
               geom->coord.x = 0;
               geom->coord.y += w->sgml_table_text.rowHeight[irow];    
               icol = -1; 
               irow++;
             } 
           tableGeom.actual_width = 0;
           tableGeom.broken = TRUE;        
           tableGeom.space = 0;
           tableGeom.leave_space = FALSE;
           tableGeom.coord.x = geom->coord.x;
           tableGeom.coord.y = geom->coord.y;
           tableGeom.coord.ascent = 0;
           tableGeom.coord.descent = 0; 
           iw++;
         }
       
       else if (*managed && *children != w->sgml_caption_text.caption)
         {     
           SGMLTextObject t = (SGMLTextObject) *children; 

           SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
           SGMLCompositeTextObjectClass myClass = (SGMLCompositeTextObjectClass) XtClass((Widget) w);

           (*childClass->sgml_text_class.compute_size)
              (*children,&tableGeom,myClass->sgml_composite_text_class.call_adjust_size,*children);
          
           if (t->sgml_text.margin < xmin) xmin = t->sgml_text.margin;
           if (t->sgml_text.margin + t->sgml_text.width > xmax)
             xmax = t->sgml_text.margin + t->sgml_text.width; 
         }  
     }  
 
   w->sgml_text.end = geom->coord;
   w->sgml_text.height = geom->coord.y + geom->coord.descent + geom->coord.ascent - w->sgml_text.begin.y;
   w->sgml_text.width  = xmax - xmin;
   w->sgml_text.margin = xmin; 
 
   /*
    * Always Break after
    */  

   _SGMLBreak(geom,w->sgml_text.space_after);
   w->sgml_text.size_valid = TRUE; 
}

