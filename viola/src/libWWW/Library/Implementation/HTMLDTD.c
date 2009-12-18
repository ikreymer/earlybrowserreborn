/*		Our Static DTD for HTML
**		-----------------------
**
**	 6 Nov 93	MD	Increased size of img_attr array to make space
**  				for terminator.
**	20 Dec 93	PYW	Added HTML+ stuff; valid-subelements DTD info.
*/

/* Implements:
*/

#include "HTMLDTD.h"

/* 	Entity Names
**	------------
**
**	This table must be matched exactly with ALL the translation tables
*/
static CONST char* entities[] = {
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
  "emsp",       /* emsp, em space - not collapsed */
  "ensp",       /* ensp, en space - not collapsed */
  "eth",	/* small eth, Icelandic */ 
  "euml",	/* small e, dieresis or umlaut mark */ 
  "ge",		/* >= */ 
  "gt",		/* greater than */ 
  "iacute",	/* small i, acute accent */ 
  "icirc",	/* small i, circumflex accent */ 
  "igrave",	/* small i, grave accent */ 
  "infin",	/* infin -- special cases */
  "integral",	/* integral -- special cases */
  "iuml",	/* small i, dieresis or umlaut mark */ 
  "ldquo",	/* left double quote */
  "le",		/* <= */ 
  "lt",		/* less than */ 
  "nbsp",	/* nbsp, non breaking space */
  "ntilde",	/* small n, tilde */ 
  "oacute",	/* small o, acute accent */ 
  "ocirc",	/* small o, circumflex accent */ 
  "ograve",	/* small o, grave accent */ 
  "oslash",	/* small o, slash */ 
  "otilde",	/* small o, tilde */ 
  "ouml",	/* small o, dieresis or umlaut mark */ 
  "quote",	/* quote */
  "rdquo",	/* right double quote */
  "sigma",	/* sigma -- special case */
  "szlig",	/* small sharp s, German (sz ligature) */ 
  "thorn",	/* small thorn, Icelandic */ 
  "uacute",	/* small u, acute accent */ 
  "ucirc",	/* small u, circumflex accent */ 
  "ugrave",	/* small u, grave accent */ 
  "uuml",	/* small u, dieresis or umlaut mark */ 
  "yacute",	/* small y, acute accent */ 
  "yuml",	/* small y, dieresis or umlaut mark */ 
};

#define HTML_ENTITIES 76


/*		Attribute Lists
**		---------------
**
**	Lists must be in alphatbetical order by attribute name
**	The tag elements contain the number of attributes
*/

static attr no_attr[] = 
	{{ 0 }};

static attr a_attr[] = {				/* Should be ID */
	{ "HREF" },
	{ "NAME" },
	{ "REL" },
	{ "REV" },
	{ "TITLE" },
	{ "TYPE" },
	{ "URN" },
	{0}
};

static attr attr_attr[] = {
	{ "ID" },
	{0}
};

static attr address_attr[] = {
	{ "ALIGN" },
	{ "BORDER" },
	{ "FOLD" },
	{ "LABEL" },
	{0}
};

static attr author_attr[] = {
	{ "HREF" },
	{0}
};

static attr base_attr[] = {
	{ "HREF" },
	{0}
};

static attr button_attr[] = {
 	{ "HREF" },
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr changed_attr[] = {
	{ "ID" },
 	{ "IDREF" },
	{0}
};

static attr color_attr[] = {
	{ "NAME" },
 	{ "RGB" },
	{0}
};

static attr circle_attr[] = {
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr emph_attr[] = {	
	{ "B" },
	{ "I" },
	{ "INDEX" },
	{ "SUB" },
	{ "SUP" },
	{ "TT" },
	{ "TYPE" },
	{0}
};

static attr entry_attr[] = {	
	{ "HREF" },
	{ "NAME" },
	{ "TYPE" },
	{0}
};

static attr example_attr[] = {	
	{ "HREF" },
	{ "BORDER" },
	{0}
};

static attr figa_attr[] = {	
	{ "AREA" },
	{ "HREF" },
	{0}
};

static attr figure_attr[] = {	
	{ "FOLD" },
	{ "HEIGHT" },
	{ "ISMAP" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MAYDELAY" },
	{ "MINWIDTH" },
	{ "SRC" },
	{ "TYPE" },
	{ "WIDTH" },
	{0}
};

static attr form_attr[] = {	
	{ "ACTION" },
	{ "ENVVAR" },
	{ "FOLD" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "METHOD" },
	{ "MINWIDTH" },
	{ "NAME" },
	{0}
};

static attr graphics_attr[] = {
	{ "HEIGHT" },
	{ "ID" },
 	{ "NAME" },
	{ "WIDTH" },
	{0}
};

static attr header_attr[] = {	
	{ "ALIGN" },
	{ "ID" },
	{ "NAME" },
	{0}
};

static attr hpane_attr[] = {	
	{ "CENTER" },
	{ "ID" },
	{ "LEFT" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "RIGHT" },
	{ "STYLE" },
	{0}
};

static attr img_attr[] = {	
	{ "ALT" },
	{ "BGAP" },
	{ "HREF" },
	{ "ISMAP" },
	{ "LGAP" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "RGAP" },
	{ "SRC" },
	{ "TGAP" },
	{0}
};

static attr input_attr[] = {	
	{ "CHARS" },
	{ "CHECKED" },
	{ "COL" },
	{ "COLS" },
	{ "DISABLED" },
	{ "ENVVAR" },
	{ "MAXCHARS" },
	{ "MAXWIDTH" },
	{ "MINCHARS" },
	{ "MINWIDTH" },
	{ "NAME" },
	{ "ROW" },
	{ "ROWS" },
	{ "SIZE" },
	{ "SRC" },
	{ "TYPE" },
	{ "VALUE" },
	{0}
};

static attr insert_attr[] = {	
	{ "AFTER" },
	{ "ASIS" },
	{ "BORDER" },
	{ "HREF" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{0}
};

static attr isindex_attr[] = {	
	{ "ACTION" },
	{0}
};

static attr l_attr[] = {	
	{ "ALIGN" },
	{ "ID" },
	{ "INDEX" },
	{ "LANG" },
	{0}
};

static attr line_attr[] = {
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr link_attr[] = {	
	{ "ARG" },
	{ "HREF" },
	{ "ID" },
	{ "REL" },
	{ "REV" },
	{0}
};

static attr listing_attr[] = {	
	{ "BORDER" },
	{ "FOLD" },
	{ "HREF" },
	{0}
};

static attr menu_attr[] = {
	{ "COMPACT" },
	{ "FOLD" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{0}
};

static attr mh_attr[] = {	
	{ "HIDDEN" },
	{0}
};

static attr nextid_attr[] = {
	{ "N" },
	{ 0 }	/* Terminate list */
};

static attr ol_attr[] = {
	{ "COMPACT" },
	{ "FOLD" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "START" },
	{0}
};

static attr oval_attr[] = {
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr point_attr[] = {
	{ "ID" },
 	{ "X" },
 	{ "Y" },
	{0}
};

static attr polygon_attr[] = {
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr pos_attr[] = {
	{ "SC" },
	{ "X" },
	{ "Y" },
	{ "Z" },
	{0}
};

static attr option_attr[] = {	
	{ "CHECKED" },
	{ "DISABLED" },
	{ "NAME" },
	{ "SELECTED" },
	{ "VALUE" },
	{0}
};

static attr rect_attr[] = {
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr rot_attr[] = {
	{ "SC" },
	{ "X" },
 	{ "Y" },
 	{ "Z" },
	{0}
};

static attr scale_attr[] = {
	{ "SC" },
	{ "X" },
 	{ "Y" },
 	{ "Z" },
	{0}
};

static attr size_attr[] = {
	{ "SC" },
	{ "X" },
 	{ "Y" },
 	{ "Z" },
	{0}
};

static attr square_attr[] = {
	{ "ID" },
 	{ "NAME" },
	{0}
};

static attr select_attr[] = {	
	{ "CHECKED" },
	{ "DISABLED" },
	{ "ENVVAR" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "NAME" },
	{ "SIZE" },
	{ "TYPE" },
	{ "VALUE" },
	{0}
};

static attr p_attr[] = {	
	{ "ALIGN" },
	{ "FOLD" },
	{ "ID" },
	{ "INDEX" },
	{ "LABEL" },
	{ "LANG" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "STYLE" },
	{0}
};

static attr pre_attr[] = {	
	{ "WIDTH" },
	{0}
};

static attr section_attr[] = {	
	{ "BORDER" },
	{ "FOLD" },
	{ "HREF" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "NAME" },
	{0}
};

static attr security_attr[] = {	
	{ "LEVEL" },
	{0}
};

static attr table_attr[] = {	
	{ "BORDER" },
	{ "COMPACT" },
	{ "FOLD" },
	{ "ID" },
	{ "INDEX" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{0}
};

static attr tcap_attr[] = {	
	{ "ALIGN" },
	{ "TOP" },
	{0}
};

static attr td_attr[] = {	
	{ "ALIGN" },
	{ "COLSPAN" },
	{ "ROWSPAN" },
	{0}
};

static attr textarea_attr[] = {	
	{ "COL" },
	{ "COLS" },
	{ "NAME" },
	{ "ROW" },
	{ "ROWS" },
	{ "SIZE" },
	{0}
};

static attr th_attr[] = {	
	{ "ALIGN" },
	{ "COLSPAN" },
	{ "ROWSPAN" },
	{0}
};

static attr ul_attr[] = {
	{ "COMPACT" },
	{ "FOLD" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{0}
};

static attr vobjf_attr[] = {	
	{ "ARG" },
	{ "HREF" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "NAME" },
	{0}
};

static attr xmp_attr[] = {	
	{ "BORDER" },
	{ "FOLD" },
	{ "HREF" },
	{ "LABEL" },
	{ "MAXWIDTH" },
	{ "MINWIDTH" },
	{ "NAME" },
	{0}
};

/* messy */
static attr shared_list_attr[] = {
	{ "COMPACT" },
	{ 0 }	/* Terminate list */
};

#define dd_dt_validSubElementsCount 82
static HTMLElement dd_dt_validSubElements[] = {
        HTML_A,
        HTML_BASE,
        HTML_ADDRESS,
        HTML_AUTHOR,
        HTML_B,
        HTML_BLOCKQUOTE,
        HTML_BODY,
        HTML_BOLD,
        HTML_BR,
        HTML_CAUTION,
        HTML_CHANGED,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_COMMENT,
        HTML_DFN,
        HTML_DIR,
        HTML_DL,
        HTML_DLC,
        HTML_EM,
        HTML_EMPH,
        HTML_ENTRY,
        HTML_EXAMPLE,
        HTML_FIGURE,
	HTML_FOOTNOTE,
        HTML_FORM,
        HTML_GRAPHICS,
        HTML_H1,
        HTML_H2,
        HTML_H3,
        HTML_H4,
        HTML_H5,
        HTML_H6,
        HTML_H7,
        HTML_HEAD,
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
        HTML_LABEL,
        HTML_LINK,
        HTML_LISTING,
        HTML_LIT,
        HTML_MENU,
        HTML_NEXTID,
        HTML_OL,
        HTML_P,
        HTML_PLAIN,
        HTML_PLAINTEXT,
        HTML_PRE,
        HTML_QUOTE,
        HTML_SAMP,
        HTML_SCREEN,
        HTML_SECTION,
        HTML_SELECT,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TABLE,
        HTML_TBL,
        HTML_TEXTAREA,
        HTML_TITLE,
        HTML_TT,
        HTML_U,
        HTML_UL,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
        HTML_VOBJF,
        HTML_VR,
        HTML_VSCRIPT,
        HTML_VVIEW,
        HTML_XMP,
};

#define l_validSubElementsCount 3
static HTMLElement l_validSubElements[] = {
	HTML_A,
	HTML_IMAGE,
	HTML_IMG,
};

#define style_validSubElementsCount 18
static HTMLElement style_validSubElements[] = {
	HTML_A,
        HTML_B,
        HTML_BOLD,
        HTML_CMD,
        HTML_EM,
        HTML_EMPH,
        HTML_I,
        HTML_IMAGE,
        HTML_IMG,
        HTML_ITALIC,
        HTML_KBD,
        HTML_KEY,
        HTML_LIT,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TT,
        HTML_U,
};

/* Oh hell. Important thing is to omit LI. Plus other misc sub-tags */
#define li_validSubElementsCount 82
static HTMLElement li_validSubElements[] = {
        HTML_A,
        HTML_ADDRESS,
        HTML_AUTHOR,
        HTML_B,
        HTML_BASE,
        HTML_BLOCKQUOTE,
        HTML_BODY,
        HTML_BOLD,
        HTML_BR,
        HTML_CAUTION,
        HTML_CHANGED,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_COMMENT,
        HTML_DFN,
        HTML_DIR,
        HTML_DL,
        HTML_DLC,
        HTML_EM,
        HTML_EMPH,
        HTML_ENTRY,
        HTML_EXAMPLE,
        HTML_FIGURE,
	HTML_FOOTNOTE,
        HTML_FORM,
        HTML_GRAPHICS,
        HTML_H1,
        HTML_H2,
        HTML_H3,
        HTML_H4,
        HTML_H5,
        HTML_H6,
        HTML_H7,
        HTML_HEAD,
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
        HTML_LABEL,
        HTML_LINK,
        HTML_LISTING,
        HTML_LIT,
        HTML_MENU,
        HTML_NEXTID,
        HTML_OL,
        HTML_P,
        HTML_PLAIN,
        HTML_PLAINTEXT,
        HTML_PRE,
        HTML_QUOTE,
        HTML_SAMP,
        HTML_SCREEN,
        HTML_SECTION,
        HTML_SELECT,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TABLE,
        HTML_TBL,
        HTML_TEXTAREA,
        HTML_TITLE,
        HTML_TT,
        HTML_U,
        HTML_UL,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
        HTML_VOBJF,
        HTML_VR,
        HTML_VSCRIPT,
        HTML_VVIEW,
        HTML_XMP,
};

#define ol_ul_validSubElementsCount 31
static HTMLElement ol_ul_validSubElements[] = {
        HTML_A,
        HTML_B,
        HTML_BOLD,
        HTML_BR,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_DFN,
        HTML_EM,
        HTML_EMPH,
	HTML_FOOTNOTE,
        HTML_I,
        HTML_IMAGE,
        HTML_IMG,
        HTML_ITALIC,
        HTML_KBD,
        HTML_KEY,
        HTML_L,
        HTML_LABEL,
        HTML_LI,
        HTML_LINK,
        HTML_P,
        HTML_SAMP,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TT,
        HTML_U,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
};

#define p_validSubElementsCount 30
static HTMLElement p_validSubElements[] = {
        HTML_A,
        HTML_B,
        HTML_BOLD,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_DFN,
        HTML_EM,
        HTML_EMPH,
	HTML_FOOTNOTE,
        HTML_I,
        HTML_IMAGE,
        HTML_IMG,
        HTML_INPUT,
        HTML_ITALIC,
        HTML_KBD,
        HTML_KEY,
        HTML_L,
        HTML_LABEL,
        HTML_LINK,
        HTML_SAMP,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TT,
        HTML_U,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
};


#define vobj_validSubElementsCount 1
static HTMLElement vobj_validSubElements[] = {
        HTML_ATTR,
};

#define option_validSubElementsCount 29
static HTMLElement option_validSubElements[] = {
        HTML_A,
        HTML_B,
        HTML_BOLD,
        HTML_BR,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_DFN,
        HTML_EM,
        HTML_EMPH,
	HTML_FOOTNOTE,
        HTML_I,
        HTML_IMAGE,
        HTML_IMG,
        HTML_ITALIC,
        HTML_KBD,
        HTML_KEY,
        HTML_L,
        HTML_LABEL,
        HTML_LINK,
        HTML_SAMP,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TT,
        HTML_U,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
};

#define th_validSubElementsCount 34
static HTMLElement th_validSubElements[] = {
        HTML_A,
        HTML_B,
        HTML_BOLD,
        HTML_BR,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_DFN,
        HTML_DIR,
        HTML_DL,
        HTML_EM,
        HTML_EMPH,
	HTML_FOOTNOTE,
        HTML_I,
        HTML_IMAGE,
        HTML_IMG,
        HTML_ITALIC,
        HTML_KBD,
        HTML_KEY,
        HTML_L,
        HTML_LABEL,
        HTML_LINK,
        HTML_MENU,
        HTML_OL, 
        HTML_SAMP,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TT,
        HTML_U,
        HTML_UL,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
};

#define loose_validSubElementsCount 84
static HTMLElement loose_validSubElements[] = {
        HTML_A,
        HTML_ADDRESS,
        HTML_AUTHOR,
        HTML_B,
        HTML_BASE,
        HTML_BLOCKQUOTE,
        HTML_BODY,
        HTML_BOLD,
        HTML_BR,
        HTML_CAUTION,
        HTML_CHANGED,
        HTML_CITE,
        HTML_CMD,
        HTML_CODE,
        HTML_COMMENT,
        HTML_DFN,
        HTML_DIR,
        HTML_DL,
        HTML_DLC,
        HTML_EM,
        HTML_EMPH,
        HTML_ENTRY,
        HTML_EXAMPLE,
        HTML_FIGURE,
	HTML_FOOTNOTE,
        HTML_FORM,
        HTML_GRAPHICS,
        HTML_H1,
        HTML_H2,
        HTML_H3,
        HTML_H4,
        HTML_H5,
        HTML_H6,
        HTML_H7,
        HTML_HEAD,
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
        HTML_LABEL,
        HTML_LINK,
        HTML_LISTING,
        HTML_LIT,
        HTML_MENU,
        HTML_NEXTID,
        HTML_OL,
        HTML_P,
        HTML_PLAIN,
        HTML_PLAINTEXT,
        HTML_PRE,
        HTML_QUOTE,
        HTML_SAMP,
        HTML_SCREEN,
        HTML_SECTION,
        HTML_SELECT,
        HTML_SECURITY,
        HTML_STRONG,
        HTML_SUB,
        HTML_SUP,
        HTML_TABLE,
        HTML_TBL,
        HTML_TEXTAREA,
        HTML_TITLE,
        HTML_TT,
        HTML_U,
        HTML_UL,
        HTML_UNDER,
        HTML_VAR,
        HTML_VINSERT,
        HTML_VOBJ,
        HTML_VOBJF,
        HTML_VR,
        HTML_VSCRIPT,
        HTML_VVIEW,
        HTML_XMP
};

/*	Elements
*/
/*    Name, 	Attributes, 		content
*/
static HTTag tags[] = {
    { "A"       , a_attr,       HTML_A_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "ACTION"  , no_attr,      0,			SGML_LITTERAL, 0, 0},
    { "ADDRESS" , address_attr, HTML_ADDRESS_ATTRIBUTES,SGML_MIXED, 0, 0},
    { "ATTR"    , attr_attr,    HTML_ATTR_ATTRIBUTES,	SGML_LITTERAL, 0, 0},
    { "AUTHOR"  , author_attr,  HTML_AUTHOR_ATTRIBUTES, SGML_EMPTY, 0, 0},
    { "AXIS"    , no_attr,      0,             		SGML_EMPTY, 0, 0},
    { "B"       , no_attr,      0,			SGML_MIXED, 
						style_validSubElements,
						style_validSubElementsCount},
    { "BASE"    , base_attr,    HTML_BASE_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "BDCOLOR" , color_attr,   HTML_COLOR_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "BGCOLOR" , color_attr,   HTML_COLOR_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "BLOCKQUOTE", no_attr,    0,			SGML_MIXED, 0, 0},
    { "BODY"    , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "BOLD"    , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "BOX"     , no_attr,	0,			SGML_MIXED, 0, 0},
    { "BR"      , p_attr,       HTML_P_ATTRIBUTES,	SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "BUTTON"  , button_attr,  HTML_BUTTON_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "BYLINE"  , no_attr,      0,			SGML_MIXED, 0, 0},
    { "CAUTION" , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "CHANGED" , changed_attr, HTML_CHANGED_ATTRIBUTES,SGML_EMPTY, 0, 0},
    { "CIRCLE"  , circle_attr,  HTML_CIRCLE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "CITE"    , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "CMD"     , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "CODE"    , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "COMMENT" , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "DD"      , no_attr,      0,             		SGML_MIXED,
						dd_dt_validSubElements,
						dd_dt_validSubElementsCount},
    { "DFN"     , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "DIR"     , shared_list_attr,1,          		SGML_MIXED, 0, 0},/*?*/
    { "DL"      , shared_list_attr,1,          		SGML_MIXED, 0, 0},
    { "DLC"     , shared_list_attr,1,          		SGML_MIXED, 0, 0},
    { "DT"      , no_attr,      0,             		SGML_MIXED,
						dd_dt_validSubElements,
						dd_dt_validSubElementsCount},
    { "EM"      , emph_attr,    HTML_EMPH_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "EMPH"    , emph_attr,    HTML_EMPH_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "ENTRY"   , entry_attr,   HTML_ENTRY_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "EXAMPLE" , example_attr, HTML_EXAMPLE_ATTRIBUTES, SGML_LITTERAL, 0, 0},
    { "FGCOLOR" , color_attr,   HTML_COLOR_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "FIGA"    , figa_attr,    HTML_FIGA_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "FIGCAP"  , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "FIGDATA" , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "FIG" 	, figure_attr,  HTML_FIGURE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "FIGURE"  , figure_attr,  HTML_FIGURE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "FOOTNOTE", no_attr,      0,			SGML_MIXED, 0, 0},
    { "FORM"    , form_attr,    HTML_FORM_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "GRAPHICS", graphics_attr,HTML_GRAPHICS_ATTRIBUTES,SGML_MIXED, 0, 0},
    { "GROUP"   , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "H1"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED, 
						p_validSubElements,
						p_validSubElementsCount},
    { "H2"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "H3"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "H4"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "H5"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "H6"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "H7"      , header_attr,  HTML_HEADER_ATTRIBUTES, SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "HEAD"    , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "HINT"    , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "HPANE"   , hpane_attr,   HTML_HPANE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "HR"      , no_attr,      0,             		SGML_EMPTY, 0, 0},
    { "HTML"    , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "I"       , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "IMAGE"   , img_attr,     HTML_IMG_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "IMG"     , img_attr,     HTML_IMG_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "INPUT"   , input_attr,	HTML_INPUT_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "INSERT"  , insert_attr,	HTML_INSERT_ATTRIBUTES, SGML_EMPTY, 0, 0},
    { "ISINDEX" , isindex_attr, HTML_ISINDEX_ATTRIBUTES,SGML_EMPTY, 0, 0},
    { "ITALIC"  , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "KBD"     , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "KEY"     , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "L"       , l_attr,	HTML_L_ATTRIBUTES,	SGML_MIXED, 
						l_validSubElements,
						l_validSubElementsCount},
    { "LABEL"   , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "LI"      , shared_list_attr,1,          		SGML_MIXED,
						li_validSubElements,
						li_validSubElementsCount},
    { "LINE"	, line_attr,	HTML_LINE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "LINK"    , link_attr,    HTML_LINK_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "LISTING" , listing_attr, HTML_LISTING_ATTRIBUTES,SGML_MIXED, 0, 0},
    { "LIT"     , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "MATH"    , no_attr,	0,			SGML_MIXED, 0, 0},
    { "MENU"    , menu_attr,    HTML_MENU_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "MH"      , mh_attr,      HTML_MH_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "NEXTID"  , nextid_attr,  1,             		SGML_EMPTY, 0, 0},
    { "OL"      , ol_attr,      HTML_OL_ATTRIBUTES,	SGML_MIXED, 
						ol_ul_validSubElements,
						ol_ul_validSubElementsCount},
    { "OLC"      , ol_attr,      HTML_OL_ATTRIBUTES,	SGML_MIXED, 
						ol_ul_validSubElements,
						ol_ul_validSubElementsCount},
    { "OPTION"  , option_attr,  HTML_OPTION_ATTRIBUTES,	SGML_MIXED,
						option_validSubElements,
    						option_validSubElementsCount},
    { "OVAL"	, oval_attr,	HTML_OVAL_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "OVER"	, no_attr,	0,			SGML_EMPTY, 0, 0},
    { "P"       , p_attr,       HTML_P_ATTRIBUTES,	SGML_MIXED,
						p_validSubElements,
						p_validSubElementsCount},
    { "PLAIN"   , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "PLAINTEXT", no_attr,     0,             		SGML_LITTERAL, 0, 0},
    { "POINT"	, point_attr,	HTML_POINT_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "POLYGON"	, polygon_attr,	HTML_POLYGON_ATTRIBUTES,SGML_MIXED, 0, 0},
    { "POS"	, pos_attr,	HTML_POS_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "PRE"     , pre_attr,     HTML_PRE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "QUOTE"   , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "RECT"	, rect_attr,	HTML_RECT_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "ROT"	, rot_attr,	HTML_ROT_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "SAMP"    , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "SCALE"	, scale_attr,	HTML_SCALE_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "SCREEN"  , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "SECTION" , section_attr, HTML_SECTION_ATTRIBUTES,SGML_MIXED, 0, 0},
    { "SECURITY", security_attr,HTML_SECURITY_ATTRIBUTES,SGML_EMPTY, 0, 0},
    { "SELECT"  , select_attr,  HTML_SELECT_ATTRIBUTES, SGML_MIXED, 0, 0},
    { "SIZE"	, size_attr,	HTML_SIZE_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "SQUARE"	, square_attr,	HTML_SQUARE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "STRONG"  , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "SUB"     , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "SUP"     , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "TABLE"   , table_attr,   HTML_TABLE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "TB"      , no_attr,      0,             		SGML_EMPTY, 0, 0},
    { "TBL"     , table_attr,   HTML_TABLE_ATTRIBUTES,	SGML_MIXED, 0, 0},
    { "TCAP"    , tcap_attr,    HTML_TCAP_ATTRIBUTES,	SGML_CDATA, 0, 0},
    { "TD"      , td_attr,      HTML_TD_ATTRIBUTES,	SGML_MIXED,
						loose_validSubElements,
						loose_validSubElementsCount},
    { "TEXT"	, no_attr,	0,			SGML_LITTERAL, 0, 0},
/*    { "TEXTAREA", textarea_attr, HTML_TEXTAREA_ATTRIBUTES, SGML_LITTERAL,0,0},*/
    { "TEXTAREA", textarea_attr, HTML_TEXTAREA_ATTRIBUTES, SGML_MIXED,0,0},
    { "TH"      , th_attr,      HTML_TH_ATTRIBUTES,	SGML_MIXED,
						th_validSubElements,
						th_validSubElementsCount},
    { "TITLE"   , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "TR"      , no_attr,      0,             		SGML_EMPTY, 0, 0},
    { "TT"      , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "U"       , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "UL"      , ul_attr,      HTML_UL_ATTRIBUTES,	SGML_MIXED,
						ol_ul_validSubElements,
						ol_ul_validSubElementsCount},
    { "ULC"      , ul_attr,      HTML_UL_ATTRIBUTES,	SGML_MIXED,
						ol_ul_validSubElements,
						ol_ul_validSubElementsCount},
    { "UNDER"   , no_attr,      0,             		SGML_MIXED,
						style_validSubElements,
						style_validSubElementsCount},
    { "VAR"     , no_attr,      0,             		SGML_MIXED, 0, 0},
    { "VINSERT" , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "VOBJ"    , no_attr,      0,			SGML_MIXED,
						vobj_validSubElements,
						vobj_validSubElementsCount},
    { "VOBJF"   , vobjf_attr,   HTML_VOBJF_ATTRIBUTES,	SGML_EMPTY, 0, 0},
    { "VR"      , no_attr,      0,             		SGML_EMPTY, 0, 0},
    { "VSCRIPT" , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "VVIEW"   , no_attr,      0,             		SGML_LITTERAL, 0, 0},
    { "XMP"     , xmp_attr,     HTML_XMP_ATTRIBUTES,    SGML_LITTERAL, 0, 0},
    {0}
};
#define HTML_TAGS 126


PUBLIC CONST SGML_dtd HTML_dtd = {
	tags,
	HTML_ELEMENTS,
	entities,
	sizeof(entities)/sizeof(char**)
};

/*	Utility Routine: useful for people building HTML objects */

/*	Start anchor element
**	--------------------
**
**	It is kinda convenient to have a particulr routine for
**	starting an anchor element, as everything else for HTML is
**	simple anyway.
*/
struct _HTStructured {
    HTStructuredClass * isa;
	/* ... */
};

PUBLIC void HTStartAnchor ARGS3(HTStructured *, obj,
		CONST char *,  name,
		CONST char *,  href)
{
    BOOL		present[HTML_A_ATTRIBUTES];
    CONST char*		value[HTML_A_ATTRIBUTES];
    
    {
    	int i;
    	for(i=0; i<HTML_A_ATTRIBUTES; i++)
	    present[i] = NO;
    }
    if (name) {
    	present[HTML_A_NAME] = YES;
	value[HTML_A_NAME] = name;
    }
    if (href) {
        present[HTML_A_HREF] = YES;
        value[HTML_A_HREF] = href;
    }
    
    (*obj->isa->start_element)(obj, HTML_A , present, value,
    					&HTML_dtd.tags[HTML_A]/*PYW*/);

}

PUBLIC void HTNextID ARGS2(HTStructured *, obj,
		int,	next_one)
{
    BOOL		present[HTML_NEXTID_ATTRIBUTES];
    CONST char*		value[HTML_NEXTID_ATTRIBUTES];
    char string[10];
    
    sprintf(string, "z%i", next_one);
    {
    	int i;
    	for(i=0; i<HTML_NEXTID_ATTRIBUTES; i++)
	    present[i] = NO;
    }
    present[HTML_NEXTID_N] = YES;
    value[HTML_NEXTID_N] = string;
    
    (*obj->isa->start_element)(obj, HTML_NEXTID , present, value,
    					&HTML_dtd.tags[HTML_NEXTID]/*PYW*/);
}

