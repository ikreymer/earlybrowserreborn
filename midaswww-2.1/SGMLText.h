/*==================================================================*/
/*                                                                  */
/* SGMLTextObject                                                   */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLText widget                   */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLTEXT_H
#define  SGMLTEXT_H

#if XtSpecificationRelease == 4
#define XrmPermStringToQuark(x) XrmStringToQuark(x)
#endif 

extern WidgetClass sGMLTextObjectClass;
typedef struct _SGMLTextRec  *SGMLTextObject;

#define SGMLALIGNMENT_NONE   0
#define SGMLALIGNMENT_LEFT   1
#define SGMLALIGNMENT_RIGHT  2
#define SGMLALIGNMENT_CENTER 3
 
#define SGMLLINE_STYLE_SOLID  0
#define SGMLLINE_STYLE_DOTTED 1
#define SGMLLINE_STYLE_DASHED 2
 
#define SGMLBREAK_NEVER 0
#define SGMLBREAK_ALWAYS 2
#define SGMLBREAK_SOFT 1

#define SGMLINHERIT_ALIGNMENT -999
#define SGMLINHERIT_MARGIN -1
#define SGMLINHERIT_PARAGRAPHINDENT -999
#define SGMLINHERIT_SPACE 32767
#define SGMLINHERIT_BREAK -999
#define SGMLINHERIT_SENSITIVITY 2
#define SGMLINHERIT_UNDERLINE -999
#define SGMLINHERIT_OUTLINE 2
#define SGMLINHERIT_UNDERLINEHEIGHT 999
#define SGMLINHERIT_UNDERLINESTYLE 999
#define SGMLINHERIT_COLOR NULL 
#define SGMLINHERIT_SIZE 0
#define SGMLINHERIT_QUARK NULL

#define SGMLNcolor                    "color"
#define SGMLCColor                    "Color"
#define SGMLNunderline                "underline"
#define SGMLCUnderline                "Underline"
#define SGMLNoutline                  "outline"
#define SGMLCOutline                  "Outline"
#define SGMLNunderlineHeight          "underlineHeight"
#define SGMLCUnderlineHeight          "UnderlineHeight"
#define SGMLNunderlineStyle           "underlineStyle"
#define SGMLCUnderlineStyle           "UnderlineStyle"
#define SGMLNbreakBefore              "breakBefore"
#define SGMLNbreakAfter               "breakAfter"
#define SGMLCBreak                    "Break"
#define SGMLRBreak                    "Break"
#define SGMLRLineStyle                "LineStyle"
#define SGMLNsensitive                "sensitive"
#define SGMLCSensitive                "Sensitive"
#define SGMLNmanaged                  "managed"
#define SGMLCManaged                  "Managed"
#define SGMLNmatched                  "matched"
#define SGMLCMatched                  "Matched"
#define SGMLNrightIndent              "rightIndent"
#define SGMLNleftIndent               "leftIndent"
#define SGMLNrightMargin              "rightMargin"
#define SGMLNleftMargin               "leftMargin"
#define SGMLNparagraphIndent          "paragraphIndent"
#define SGMLNspaceBefore              "spaceBefore"
#define SGMLNspaceAfter               "spaceAfter"
#define SGMLCMargin                   "Margin"
#define SGMLNalignment                "alignment"
#define SGMLCAlignment                "Alignment"
#define SGMLNspacing                  "spacing"
#define SGMLCSpacing                  "Spacing" 
#define SGMLNdescent                  "descent"
#define SGMLCDescent                  "Descent" 
#define SGMLNascent                   "ascent"
#define SGMLCAscent                   "Ascent" 
#define SGMLNuserdata                 "userdata"
#define SGMLCUserdata                 "Userdata" 

#define SGMLNfontSize                 "fontSize"
#define SGMLCFontSize		      "FontSize"	
#define SGMLNfontRegistry             "fontRegistry"
#define SGMLCFontRegistry	      "FontRegistry"	 
#define SGMLNfontSpacing              "fontSpacing"
#define SGMLCFontSpacing	      "FontSpacing"	
#define SGMLNfontWidth                "fontWidth"
#define SGMLCFontWidth		      "FontWidth"	
#define SGMLNfontSlant                "fontSlant"
#define SGMLCFontSlant		      "FontSlant"	
#define SGMLNfontWeight               "fontWeight"
#define SGMLCFontWeight		      "FontWeight"	
#define SGMLNfontFamily               "fontFamily"
#define SGMLCFontFamily		      "FontFamily"	

#define SGMLNfont16                   "font16"
#define SGMLCFont16                   "Font16"

#define SGMLNexactTerminator          "exactTerminator"		
#define SGMLCExactTerminator          "ExactTerminator"
#define SGMLNhideIllegalTags          "hideIllegalTags"
#define SGMLCHideIllegalTags          "HideIllegalTags"
#define SGMLNsupportsEntities         "supportsEntities"
#define SGMLCSupportsEntities         "SupportsEntities"

#define SGMLNtext        "text"
#define SGMLCText        "Text"
#define SGMLNparam       "param"
#define SGMLCParam       "param"
#define SGMLNlength      "length"
#define SGMLCLength      "Length"
#define SGMLNparamLength "paramLength"
#define SGMLCParamLength "ParamLength"

#define SGMLNwidth       "width"
#define SGMLNheight      "height"
#define SGMLNinheritance "inheritance"    

#define SGMLNactivateCallback "activateCallback"
#define SGMLNcreateCallback   "createCallback"
#define SGMLNmapCallback      "mapCallback"
#define SGMLNunmapCallback    "unmapCallback"
#define SGMLCCallback         "Callback" 

#define SGMLRQuark            "Quark"		

#define SGMLSELECT_REGION	1
#define SGMLSELECT_WORD		2
#define SGMLSELECT_LINE		3
#define SGMLSELECT_ALL          4
#define SGMLSELECT_WIDGET	999

#define SGMLDUMP_ASCII		1
#define SGMLDUMP_HTML		2
#define SGMLDUMP_HTML_ASCII	3

/*
  Callback structure
*/

#define SGMLTEXT_ACTIVATE 1
#define SGMLTEXT_CREATE 2
#define SGMLTEXT_MAP 2

typedef struct {

    int     reason;   /* always = HYPER_REASON                            */
    XEvent *event;    /* event                                            */
    char   *text;     /* pointer on highlighted text selected (read only) */
    int     length;   /* length of selected text                          */
    char   *param;    /* pointer to hidden text */
    int     param_length;

}  SGMLTextCallbackStruct;

/*
   Other exported data structures
*/

typedef struct _SGMLRendition {

    Pixel               color;
    Boolean             outline;                 
    int                 underline;
    Dimension           underline_height;
    int                 underline_style;

    XrmQuark            family;
    XrmQuark		weight;
    XrmQuark		slant;
    XrmQuark		width;
    int			size;
    XrmQuark            spacing;
    XrmQuark		registry;		 	

} SGMLRendition;

/*
 * Note: We use longs for all positions and dimensions since
 * SGML widgets can be BIG. This differs from Xt which uses
 * shorts for these quantities.
 */

typedef long SGMLPosition;
typedef unsigned long SGMLDimension; 

typedef struct _SGMLCoord {

    SGMLPosition            x,y;
    Dimension           descent;
    Dimension           ascent;

} SGMLCoord;     

typedef struct {

   SGMLCoord coord;
   int       alignment;
   Boolean   leave_space;
   Boolean   broken;
   Dimension space;
   SGMLDimension natural_width;
   SGMLDimension actual_width; 

} SGMLGeometry;  

typedef struct _SGMLDumpTextInfo {
   
  Widget sw;          /* search widget */
  Boolean begin_sw;
  Boolean calc_len;    /* calculate the length */
  int format_type;     /* html, plain, html-plain */
  int copy_type;       /* word, line, region */

  char *current_pos;
  char *last_pos;
  int last_len;
  int line_spacing;
  int spacing;

} SGMLDumpTextInfo; 

typedef struct _SGMLCopy {

    int  first_x, first_y;
    int  current_x, current_y;
    int  last_x, last_y;
    Widget last_widget;
    Widget current_widget;
    Widget first_widget;
    Widget cw;
    Boolean last_mode, current_mode;
    char *first_char;
    char *last_pos;
    Boolean flag, sw_flag;

} SGMLCopyInfo;


#ifdef _NO_PROTO

extern Widget  CreateSGMLText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsText(w)     XtIsSubclass(w,sGMLTextObjectClass)

#endif SGMLTEXT_H
