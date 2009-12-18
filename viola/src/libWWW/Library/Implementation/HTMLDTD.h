/*                                               The HTML DTD -- software interface in libwww
                              HTML DTD - SOFTWARE INTERFACE
                                             
   SGML purists should excuse the use of the term "DTD" in this file to represent
   DTD-related information which is not exactly a DTD itself.
   
   The C modular structure doesn't work very well here, as the dtd is partly in the .h and
   partly in the .c which are not very independent.  Tant pis.
   
 */
#ifndef HTMLDTD_H
#define HTMLDTD_H

#include "HTUtils.h"
#include "SGML.h"

/*	Element Numbers
**	---------------
**
**   Must Match all tables by element! These include tables in HTMLDTD.c 
**   and code in HTML.c.
*/

typedef enum _HTMLElement {
	HTML_A,
	HTML_ACTION, /*vr exp*/
	HTML_ADDRESS,
	HTML_ATTR,   /*vobj exp*/
	HTML_AUTHOR,
	HTML_AXIS, /*vr exp*/
	HTML_B,
	HTML_BASE,
	HTML_BDCOLOR, /*vr exp*/
	HTML_BGCOLOR, /*vr exp*/
	HTML_BLOCKQUOTE,
	HTML_BODY,
	HTML_BOLD,
	HTML_BOX,
	HTML_BR,
	HTML_BUTTON, /*vr exp*/
	HTML_BYLINE,
	HTML_CAUTION,
	HTML_CHANGED,
	HTML_CIRCLE, /*vr exp*/
	HTML_CITE,
	HTML_CMD,
	HTML_CODE,
	HTML_COMMENT,
	HTML_DD,
	HTML_DFN,
	HTML_DIR,
	HTML_DL,
	HTML_DLC,
	HTML_DT,
	HTML_EM,
	HTML_EMPH,
	HTML_ENTRY,
	HTML_EXAMPLE,
	HTML_FGCOLOR, /*vr exp*/
	HTML_FIGA,
	HTML_FIGCAP,
	HTML_FIGDATA,
	HTML_FIG,
	HTML_FIGURE,
	HTML_FOOTNOTE,
	HTML_FORM,
	HTML_GRAPHICS, /*vr exp*/
	HTML_GROUP,
	HTML_H1,
	HTML_H2,
	HTML_H3,
	HTML_H4,
	HTML_H5,
	HTML_H6,
	HTML_H7,
	HTML_HEAD,
	HTML_HINT, /*vr exp*/
	HTML_HPANE,
	HTML_HR,
	HTML_HTML,	
	HTML_I,
	HTML_IMAGE,
	HTML_IMG,
	HTML_INPUT,
	HTML_INSERT,
	HTML_ISINDEX,
	HTML_ITALIC,
	HTML_KBD,
	HTML_KEY,
	HTML_L,
	HTML_LABEL,/*html, vrml*/
	HTML_LI,
	HTML_LINE, /*vr exp*/
	HTML_LINK,
	HTML_LISTING,
	HTML_LIT,
	HTML_MATH,
	HTML_MENU,
	HTML_MH,
	HTML_NEXTID,	
	HTML_OL,
	HTML_OLC,
	HTML_OPTION,
	HTML_OVAL, /*vr exp*/
	HTML_OVER,
	HTML_P,
	HTML_PLAIN,
	HTML_PLAINTEXT,
	HTML_POINT, /*vr exp*/
	HTML_POLYGON, /*vr exp*/
	HTML_POS, /*vr exp*/
	HTML_PRE,
	HTML_QUOTE,
	HTML_RECT, /*vr exp*/
	HTML_ROT, /*vr exp*/
	HTML_SCALE, /*vr exp*/
	HTML_SAMP,
	HTML_SCREEN,
	HTML_SECTION,
	HTML_SECURITY,
	HTML_SELECT,
	HTML_SIZE, /*vr exp*/
	HTML_SQUARE, /*vr exp*/
	HTML_STRONG,
	HTML_SUB,
	HTML_SUP,
	HTML_TABLE,
	HTML_TB,
	HTML_TBL,
	HTML_TCAP,
	HTML_TD,
	HTML_TEXT, /*vr exp*/
	HTML_TEXTAREA,
	HTML_TH,
	HTML_TITLE,
	HTML_TR,
	HTML_TT,
	HTML_U,
	HTML_UL,
	HTML_ULC,
	HTML_UNDER,
	HTML_VAR,
	HTML_VINSERT,
	HTML_VOBJ,    /*vobj exp*/
	HTML_VOBJF,
	HTML_VR,
	HTML_VSCRIPT,
	HTML_VVIEW,
	HTML_XMP
} HTMLElement;

#define HTML_ELEMENTS 125

/*
Attribute numbers
 */

/*

   Identifier is HTML_<element>_<attribute>. These must match the tables in HTMLDTD.c !
   
 */
/*	Attribute numbers
**	-----------------
**
**	Name is <element>_<attribute>.  These must match the tables in .c!
*/
/* HMML artifacts should be considered experimental.
 */
#define HTML_A_HREF             0
#define HTML_A_NAME             1
#define HTML_A_REL              2
#define HTML_A_REV              3
#define HTML_A_TITLE            4
#define HTML_A_TYPE             5
#define HTML_A_URN              6
#define HTML_A_ATTRIBUTES       7

#define HTML_ADDRESS_ALIGN	0
#define HTML_ADDRESS_BORDER	1
#define HTML_ADDRESS_FOLD	2
#define HTML_ADDRESS_LABEL	3
#define HTML_ADDRESS_ATTRIBUTES	4

#define HTML_ATTR_ID		0
#define HTML_ATTR_ATTRIBUTES    1

#define HTML_AUTHOR_HREF	0
#define HTML_AUTHOR_ATTRIBUTES	1

#define HTML_BASE_HREF		0
#define HTML_BASE_ATTRIBUTES	1

#define HTML_CHANGED_ID		0
#define HTML_CHANGED_IDREF	1
#define HTML_CHANGED_ATTRIBUTES	2

#define HTML_BUTTON_HREF	0
#define HTML_BUTTON_ID		1
#define HTML_BUTTON_NAME	2
#define HTML_BUTTON_ATTRIBUTES	3

#define HTML_COLOR_NAME		0
#define HTML_COLOR_RGB		1
#define HTML_COLOR_ATTRIBUTES	2

#define HTML_CIRCLE_ID		0
#define HTML_CIRCLE_NAME	1
#define HTML_CIRCLE_ATTRIBUTES	2

#define HTML_EMPH_B		0
#define HTML_EMPH_I		1
#define HTML_EMPH_INDEX		2
#define HTML_EMPH_SUB		3
#define HTML_EMPH_SUP		4
#define HTML_EMPH_TT		5
#define HTML_EMPH_TYPE		6
#define HTML_EMPH_ATTRIBUTES	7

#define HTML_ENTRY_HREF		0
#define HTML_ENTRY_NAME		1
#define HTML_ENTRY_TYPE		2
#define HTML_ENTRY_ATTRIBUTES	3

#define HTML_EXAMPLE_HREF       0
#define HTML_EXAMPLE_BORDER     1
#define HTML_EXAMPLE_ATTRIBUTES	2

#define HTML_FIGA_AREA		0
#define HTML_FIGA_HREF		1
#define HTML_FIGA_ATTRIBUTES	2

#define HTML_FIGURE_FOLD	0
#define HTML_FIGURE_HEIGHT	1
#define HTML_FIGURE_ISMAP	2
#define HTML_FIGURE_LABEL	3
#define HTML_FIGURE_MAXWIDTH	4
#define HTML_FIGURE_MAYDELAY	5
#define HTML_FIGURE_MINWIDTH	6
#define HTML_FIGURE_SRC		7
#define HTML_FIGURE_TYPE	8
#define HTML_FIGURE_WIDTH	9
#define HTML_FIGURE_ATTRIBUTES	10

#define HTML_FORM_ACTION	0
#define HTML_FORM_ENVVAR	1
#define HTML_FORM_FOLD		2
#define HTML_FORM_LABEL		3
#define HTML_FORM_MAXWIDTH	4
#define HTML_FORM_METHOD	5
#define HTML_FORM_MINWIDTH	6
#define HTML_FORM_NAME		7
#define HTML_FORM_ATTRIBUTES	8

#define HTML_GRAPHICS_HEIGHT	0
#define HTML_GRAPHICS_ID	1
#define HTML_GRAPHICS_NAME	2
#define HTML_GRAPHICS_WIDTH	3
#define HTML_GRAPHICS_ATTRIBUTES 4

#define HTML_HEADER_ALIGN       0
#define HTML_HEADER_ID          1
#define HTML_HEADER_NAME        2
#define HTML_HEADER_ATTRIBUTES  3

#define HTML_HPANE_CENTER 	0
#define HTML_HPANE_ID		1
#define HTML_HPANE_LEFT 	2
#define HTML_HPANE_MAXWIDTH	3
#define HTML_HPANE_MINWIDTH	4
#define HTML_HPANE_RIGHT 	5
#define HTML_HPANE_STYLE 	6
#define HTML_HPANE_ATTRIBUTES	7

#define HTML_IMG_ALT            0
#define HTML_IMG_BGAP		1
#define HTML_IMG_HEIGHT		2
#define HTML_IMG_HREF		3
#define HTML_IMG_ISMAP		4       /* Obsolete but supported */
#define HTML_IMG_LGAP		5
#define HTML_IMG_MAXWIDTH	6
#define HTML_IMG_MINWIDTH	7
#define HTML_IMG_RGAP		8
#define HTML_IMG_SRC		9
#define HTML_IMG_TGAP		10
#define HTML_IMG_WIDTH		11
#define HTML_IMG_ATTRIBUTES	12

#define HTML_INPUT_CHARS	0
#define HTML_INPUT_CHECKED	1
#define HTML_INPUT_COL		2
#define HTML_INPUT_COLS		3
#define HTML_INPUT_DISABLED	4
#define HTML_INPUT_ENVVAR	5
#define HTML_INPUT_MAXCHARS	6
#define HTML_INPUT_MAXWIDTH	7
#define HTML_INPUT_MINCHARS	8
#define HTML_INPUT_MINWIDTH	9
#define HTML_INPUT_NAME		10
#define HTML_INPUT_ROW		11
#define HTML_INPUT_ROWS		12
#define HTML_INPUT_SIZE		13
#define HTML_INPUT_SRC		14
#define HTML_INPUT_TYPE		15
#define HTML_INPUT_VALUE	16
#define HTML_INPUT_ATTRIBUTES	17

#define HTML_INSERT_AFTER	0
#define HTML_INSERT_ASIS	1
#define HTML_INSERT_BORDER	2
#define HTML_INSERT_FOLD	3
#define HTML_INSERT_HREF	4
#define HTML_INSERT_LABEL	5
#define HTML_INSERT_MAXWIDTH	6
#define HTML_INSERT_MINWIDTH	7
#define HTML_INSERT_ATTRIBUTES	8

#define HTML_ISINDEX_ACTION	0
#define HTML_ISINDEX_ATTRIBUTES 1

#define HTML_L_ALIGN		0
#define HTML_L_ID		1
#define HTML_L_INDEX		2
#define HTML_L_LANG		3
#define HTML_L_ATTRIBUTES	4

#define HTML_LINE_ID		0
#define HTML_LINE_NAME		1
#define HTML_LINE_ATTRIBUTES 	2

#define HTML_LINK_ARG		0
#define HTML_LINK_HREF		1
#define HTML_LINK_ID		2
#define HTML_LINK_REL		3
#define HTML_LINK_REV		4
#define HTML_LINK_ATTRIBUTES	5

#define HTML_LISTING_BORDER     0
#define HTML_LISTING_FOLD	1
#define HTML_LISTING_HREF       2
#define HTML_LISTING_LABEL	3
#define HTML_LISTING_ATTRIBUTES	4

#define HTML_MENU_COMPACT	0
#define HTML_MENU_FOLD		1
#define HTML_MENU_LABEL		2
#define HTML_MENU_MAXWIDTH	3
#define HTML_MENU_MINWIDTH	4
#define HTML_MENU_ATTRIBUTES	5

#define HTML_MH_HIDDEN		0
#define HTML_MH_ATTRIBUTES	1

#define HTML_NEXTID_N		0
#define HTML_NEXTID_ATTRIBUTES	1

#define HTML_OL_COMPACT		0
#define HTML_OL_FOLD		1
#define HTML_OL_LABEL		2
#define HTML_OL_MAXWIDTH	3
#define HTML_OL_MINWIDTH	4
#define HTML_OL_START		5
#define HTML_OL_ATTRIBUTES	6

#define HTML_OVAL_ID		0
#define HTML_OVAL_NAME		1
#define HTML_OVAL_ATTRIBUTES 	2

#define HTML_OPTION_CHECKED	0
#define HTML_OPTION_DISABLED	1
#define HTML_OPTION_NAME	2
#define HTML_OPTION_SELECTED	3
#define HTML_OPTION_VALUE	4
#define HTML_OPTION_ATTRIBUTES	5

#define HTML_OUTPUT_ARG		0
#define HTML_OUTPUT_HREF	1
#define HTML_OUTPUT_ID		2
#define HTML_OUTPUT_REL		3
#define HTML_OUTPUT_REV		4
#define HTML_OUTPUT_ATTRIBUTES	5

#define HTML_P_ALIGN		0
#define HTML_P_FOLD		1
#define HTML_P_ID		2
#define HTML_P_INDEX		3
#define HTML_P_LABEL		4
#define HTML_P_LANG		5
#define HTML_P_MAXWIDTH		6
#define HTML_P_MINWIDTH		7
#define HTML_P_STYLE		8
#define HTML_P_ATTRIBUTES	9

#define HTML_POINT_ID		0
#define HTML_POINT_X		1
#define HTML_POINT_Y		2
#define HTML_POINT_ATTRIBUTES 	3

#define HTML_POLYGON_ID		0
#define HTML_POLYGON_NAME	1
#define HTML_POLYGON_ATTRIBUTES	2

#define HTML_POS_SC		0
#define HTML_POS_X		1
#define HTML_POS_Y		2
#define HTML_POS_Z		3
#define HTML_POS_ATTRIBUTES	4

#define HTML_PRE_WIDTH          0
#define HTML_PRE_ATTRIBUTES     1

#define HTML_RECT_ID		0
#define HTML_RECT_NAME		1
#define HTML_RECT_ATTRIBUTES 	2

#define HTML_ROT_SC		0
#define HTML_ROT_X		1
#define HTML_ROT_Y		2
#define HTML_ROT_Z		3
#define HTML_ROT_ATTRIBUTES	4

#define HTML_SCALE_SC		0
#define HTML_SCALE_X		1
#define HTML_SCALE_Y		2
#define HTML_SCALE_Z		3
#define HTML_SCALE_ATTRIBUTES	4

#define HTML_SELECT_CHECKED	0
#define HTML_SELECT_DISABLED	1
#define HTML_SELECT_ENVVAR	2
#define HTML_SELECT_MAXWIDTH	3
#define HTML_SELECT_MINWIDTH	4
#define HTML_SELECT_NAME	5
#define HTML_SELECT_SIZE	6
#define HTML_SELECT_TYPE	7
#define HTML_SELECT_VALUE	8
#define HTML_SELECT_ATTRIBUTES	9

#define HTML_SECTION_BORDER	0
#define HTML_SECTION_FOLD	1
#define HTML_SECTION_HREF	2
#define HTML_SECTION_LABEL	3
#define HTML_SECTION_MAXWIDTH	4
#define HTML_SECTION_MINWIDTH	5
#define HTML_SECTION_NAME 	6
#define HTML_SECTION_ATTRIBUTES	7

#define HTML_SECURITY_MINWIDTH	0
#define HTML_SECURITY_ATTRIBUTES 1

#define HTML_SIZE_SC		0
#define HTML_SIZE_X		1
#define HTML_SIZE_Y		2
#define HTML_SIZE_Z		3
#define HTML_SIZE_ATTRIBUTES	4

#define HTML_SQUARE_ID		0
#define HTML_SQUARE_NAME	1
#define HTML_SQUARE_ATTRIBUTES	2

#define HTML_TABLE_BORDER	0
#define HTML_TABLE_COMPACT	1
#define HTML_TABLE_FOLD		2
#define HTML_TABLE_ID		3
#define HTML_TABLE_INDEX	4
#define HTML_TABLE_LABEL	5
#define HTML_TABLE_MAXWIDTH	6
#define HTML_TABLE_MINWIDTH	7
#define HTML_TABLE_ATTRIBUTES	8

#define HTML_TCAP_ALIGN		0
#define HTML_TCAP_TOP		1
#define HTML_TCAP_ATTRIBUTES	2

#define HTML_TD_ALIGN		0
#define HTML_TD_COLSPAN		1
#define HTML_TD_ROLSPAN		2
#define HTML_TD_ATTRIBUTES	3

#define HTML_TEXTAREA_COL	0
#define HTML_TEXTAREA_COLS	1
#define HTML_TEXTAREA_NAME	2
#define HTML_TEXTAREA_ROW	3
#define HTML_TEXTAREA_ROWS	4
#define HTML_TEXTAREA_SIZE	5
#define HTML_TEXTAREA_ATTRIBUTES 6

#define HTML_TH_ALIGN		0
#define HTML_TH_COLSPAN		1
#define HTML_TH_ROLSPAN		2
#define HTML_TH_ATTRIBUTES	3

#define HTML_UL_COMPACT		0
#define HTML_UL_FOLD		1
#define HTML_UL_LABEL		2
#define HTML_UL_MAXWIDTH	3
#define HTML_UL_MINWIDTH	4
#define HTML_UL_ATTRIBUTES	5

#define HTML_VOBJ_ATTR		0
#define HTML_VOBJ_ATTRIBUTES	1

#define HTML_VOBJF_ARG		0
#define HTML_VOBJF_HREF		1
#define HTML_VOBJF_MAXWIDTH	2
#define HTML_VOBJF_MINWIDTH	3
#define HTML_VOBJF_NAME		4
#define HTML_VOBJF_ATTRIBUTES	5

#define HTML_XMP_BORDER		0
#define HTML_XMP_FOLD		1
#define HTML_XMP_HREF		2
#define HTML_XMP_LABEL		3
#define HTML_XMP_MAXWIDTH	4
#define HTML_XMP_MINWIDTH	5
#define HTML_XMP_NAME		6
#define HTML_XMP_ATTRIBUTES	7

extern CONST SGML_dtd HTML_dtd;

/*

Start anchor element

   It is kinda convenient to have a particulr routine for starting an anchor element, as
   everything else for HTML is simple anyway.
   
  ON ENTRY
  
   targetstream poinst to a structured stream object.
   
   name and href point to attribute strings or are NULL if the attribute is to be omitted.
   
 */
extern void HTStartAnchor PARAMS((
                HTStructured * targetstream,
                CONST char *    name,
                CONST char *    href));


#endif /* HTMLDTD_H */


/*

Specify next ID to be used

   This is anoter convenience routine, for specifying the next ID to be used by an editor
   in the series z1. z2,...
   
 */
extern void HTNextID PARAMS((HTStructured * targetStream, int n));

/*

   End of module definition */







