/*		Our Static DTD for FOSI
**		-----------------------
** An extreme subset of the FOSI DTD.
** -Pei
*/
/* Implements:
*/
#include "FOSIDTD.h"

/* 	Entity Names*/
static CONST char* entities[] = {0};
#define FOSI_ENTITIES 0

/*		Attribute Lists
**		---------------
**
**	Lists must be in alphatbetical order by attribute name
**	The tag elements contain the number of attributes
*/
static attr no_attr[] = 
	{{ 0 }};

static attr e_i_c_attr[] = {
	{ "GI"},
	{ 0 }	/* Terminate list */
};	

static attr font_attr[] = {
	{ "BGCOL"},
	{ "FGCOL"},
	{ "POSTURE"},
	{ "SIZE"},
	{ "STYLE"},
	{ "WEIGHT"},
	{ "WIDTH"},
	{ 0 }	/* Terminate list */
};

static attr indent_attr[] = {
	{ "FIRSTLN" },
	{ "LEFTIND" },
	{ "RIGHTIND" },
	{ 0 }	/* Terminate list */
};

/*	Elements
*/
/*    Name, 	Attributes, 		content
*/
static HTTag tags[] = {
    { "CHARLIST", no_attr,      0,              	SGML_MIXED, 0, 0},
    { "DOCDESC" , no_attr,      0,              	SGML_MIXED, 0, 0},
    { "E-I-C"   , e_i_c_attr,   FOSI_E_I_C_ATTRIBUTES,  SGML_MIXED, 0, 0},
    { "FONT"    , font_attr,    FOSI_FONT_ATTRIBUTES,   SGML_EMPTY, 0, 0},
    { "INDENT"  , indent_attr,  FOSI_INDENT_ATTRIBUTES, SGML_EMPTY, 0, 0},
    { "STYLDESC", no_attr,      0,              	SGML_MIXED, 0, 0},
};
#define FOSI_TAGS 6


PUBLIC CONST SGML_dtd FOSI_dtd = {
	tags,
	FOSI_TAGS,
	NULL,
	sizeof(entities)/sizeof(char**)
};
