
/*	A real style sheet (inherited from the Character Grid browser)
**
**	The dimensions are all in characters!
*/

#include "../libWWW/Library/Implementation/HTStyle.h"	/* modified for viola */
#include "DefaultStyles.h"

#ifdef THIS_IS_OBSOLETE

/* Template:
**	link to next, name, tag, tagtype,
**	font, size, colour, 		superscript, anchor id,
**	indents: 1st, left, right, 	lineheight, descent,	tabs,
**	word wrap, free format, space: before, after, flags.
*/

PUBLIC HTTabStop tabs_8[] = {
	{0, 1}, {0, 9}, {0, 17}, {0, 25}, {0, 33}, {0, 41},
	{0, 49}, {0, 57}, {0, 65}, {0, 73}, {0, 81}, {0, 89}, 
	{0, 97}, {0, 105}, {0, 113}, {0, 121}, {0, 129}
};

PUBLIC HTTabStop tabs_16[] = {
	{0, 1}, {0, 17}, {0, 33},
	{0, 49}, {0, 65}, {0, 81}, 
	{0, 97}, {0, 113}, {0, 129}
};

PUBLIC HTTabStop tabs_dir[] = {
	{0, 16}, {0, 32}, {0, 48}, {0, 64}, {0, 80}, 
	{0, 96}, {0, 112}, {0, 128}, {0, 144}, {0, 160},
	{0, 176}, {0, 192}, {0, 208}, {0, 224}, {0, 240}, 
	{0, 256}, {0, 272}, {0, 288}, {0, 304}, {0, 320}, 
	{0, 336}, {0, 352}, {0, 368}, {0, 384}, {0, 400}, 
	{0, 416}, {0, 424}, {0, 448}, {0, 464}, {0, 480}, 
	{0, 496}, {0, 512}, {0, 528}, {0, 544}, {0, 560}, 
	{0, 576}, {0, 592}, {0, 608}, {0, 624}, {0, 640}, 
	{0, 656}, {0, 672}, {0, 688}, {0, 704}, {0, 720}, 
	{0, 736}, {0, 752}, {0, 768}, 
	{0, -1}/* end signal */
};

PUBLIC HTStyle HTStyleHidden = { 
	0,  "Hidden", "S",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleNormal = {
	&HTStyleHidden,  "Normal", "P",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleVObj = {
	&HTStyleNormal,  "VObj", "VOBJ",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStylePMenu = {
	&HTStyleVObj,  "PMenu", "PMENU",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleXPM = {
	&HTStylePMenu,  "XPM", "XPM",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleXBM = {
	&HTStyleXPM,  "XBM", "XBM",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleIcon = {
	&HTStyleXBM,  "Icon", "ICON",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStylePS = {
	&HTStyleIcon,  "Hidden", "PS",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleOrderedList = { 
	&HTStylePS,  "OrderedList", "OL",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 8, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleList = { 
	&HTStyleOrderedList,  "List", "UL",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 8, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleListCompact = {
	&HTStyleList,  "ListCompact", "ULC",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 150, 8, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0
};

PUBLIC HTStyle HTStyleColumns16 = {
	&HTStyleListCompact,  "Columns16", "ULC",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	tabs_16,/*not really*/
	YES, NO, 1, 0,			0
};	

PUBLIC HTStyle HTStyleGlossary = {
	&HTStyleColumns16,  "Glossary", "DL",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 64, 24, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0
};	

PUBLIC HTStyle HTStyleGlossaryCompact = {
	&HTStyleGlossary,  "GlossaryCompact", "DLC",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 64, 8, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0
};	

PUBLIC HTStyle HTStyleListDirectory = { 
	&HTStyleGlossaryCompact,  "ListDirectory", "LI",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	tabs_dir,
	YES, NO, 0, 1,			0
};	

PUBLIC HTStyle HTStyleDirectory = {
	&HTStyleListDirectory, "Dir", 	"DIR",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	tabs_dir,
	YES, NO, 1, 1,			0
};	

PUBLIC HTStyle HTStyleMenu = {
	&HTStyleDirectory,  "Menu",	"MENU",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	24, 24, 24, HT_LEFT,		1, 0,	0, 
	YES, NO, 1, 1,			0
};	

PUBLIC HTStyle HTStyleHighLight = { 
	&HTStyleMenu, "HighLight",	"HP",
	HT_INVERSE, 1.0, HT_BLACK,	0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleExampleA = { 
	&HTStyleHighLight, "ExampleA",	"XMPA",
	HT_FIXED, 1.0, HT_BLACK,	0, 0,
	0, 0, 1, HT_LEFT,		1, 0,	tabs_8,
	NO, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStylePRE = { 
	&HTStyleExampleA, "ExampleA",	"PRE",
	HT_FIXED, 1.0, HT_BLACK,	0, 0,
	0, 0, 1, HT_LEFT,		1, 0,	tabs_8,
	NO, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleExample = { 
	&HTStylePRE, "Example",	"XMP",
	HT_FIXED, 1.0, HT_BLACK,	0, 0,
	0, 0, 1, HT_LEFT,		1, 0,	tabs_8,
	NO, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleListing = { 
	&HTStyleExample,  "Listing", "LISTING",
	HT_FIXED, 1.0, HT_BLACK,	0, 0,
	0, 0, 1, HT_LEFT,		1, 0,	tabs_8,
	NO, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleAddress = {
	&HTStyleListing,  "Address", "ADDRESS",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	0, 0, 24, HT_RIGHT,		1, 0,	0,
	NO, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleHeading1 = { 
  	&HTStyleAddress,  "Heading1", "H1",
	HT_BOLD_LARGEST, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_CENTER,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleHeading2 = {
	&HTStyleHeading1,  "Heading2", "H2",
	HT_BOLD_LARGE, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 1,			0 };	

PUBLIC HTStyle HTStyleHeading3 = { 
	&HTStyleHeading2,  "Heading3", "H3",
	HT_BOLD, 1.0, HT_BLACK,		0, 0,
	24, 24, 0, HT_LEFT,		1, 0,	0, 
	YES, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleHeading4 = { 
	&HTStyleHeading3,  "Heading4", "H4",
	HT_BOLD, 1.0, HT_BLACK,		0, 0,
	24, 24, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleHeading5 = { 
	&HTStyleHeading4,  "Heading5", "H5",
	HT_BOLD, 1.0, HT_BLACK,		0, 0,
	64, 64, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleHeading6 = { 
	&HTStyleHeading5,  "Heading6", "H6",
	HT_BOLD, 1.0, HT_BLACK,		0, 0,
	128, 128, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

PUBLIC HTStyle HTStyleHeading7 = { 
	&HTStyleHeading6,  "Heading7", "H7",
	HT_BOLD, 1.0, HT_BLACK,		0, 0,
	160, 170, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

#endif

PUBLIC HTStyle HTStyleHeading7 = { /* UNUSED */
	NULL,  "Heading7", "H7",
	0, 1.0, 0,		0, 0,
	160, 170, 0, HT_LEFT,		1, 0,	0,
	YES, NO, 1, 0,			0 };	

/* Style sheet points to the last in the list:
*/

PUBLIC HTStyleSheet sheet = { "default.style", &HTStyleHeading7 }; /* sheet */

PUBLIC HTStyleSheet * styleSheet = &sheet;




