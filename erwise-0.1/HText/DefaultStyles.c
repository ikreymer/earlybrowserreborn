/*	A real style sheet for the Character Grid browser
**
**	The dimensions are all in characters!
*/

#include "HTStyle.h"

/* Template:
**	link to next, name, tag, tagtype,
**	font, size, colour, 		superscript, anchor id,
**	indents: 1st, left, right, 	lineheight, descent,	tabs,
**	word wrap, free format, space: before, after, flags.
*/

PRIVATE HTStyle HTStyleNormal =
{0, "Normal", "P",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 3, 3, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 1, 0};

PRIVATE HTStyle HTStyleList =
{&HTStyleNormal, "List", "UL",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 7, 10, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 1, 0};

PRIVATE HTStyle HTStyleListCompact =
{
  &HTStyleList, "ListCompact", "ULC",
  HT_FONT, 1.0, HT_BLACK, 0, 0,
  1, 4, 0, HT_LEFT, 1, 0, 0,
  YES, YES, 0, 0, 0
};
PRIVATE HTTabStop tabs_16[] =
{
  {0, 16},
  {0, 32},
  {0, 48},
  {0, 64},
  {0, 80},
  {0, 96},
  {0, 112}
};

PRIVATE HTStyle HTStyleColumns16 =
{
  &HTStyleListCompact, "Columns16", "ULC",
  HT_FONT, 1.0, HT_BLACK, 0, 0,
  1, 4, 0, HT_LEFT, 1, 0, tabs_16,
  YES, YES, 0, 0, 0
};

PRIVATE HTStyle HTStyleGlossary =
{
  &HTStyleColumns16, "Glossary", "DL",
  HT_FONT, 1.0, HT_BLACK, 0, 0,
  2, 25, 0, HT_LEFT, 1, 0, 0,
  YES, YES, 1, 1, 0
};
PRIVATE HTStyle HTStyleGlossaryCompact =
{
  &HTStyleGlossary, "GlossaryCompact", "DLC",
  HT_FONT, 1.0, HT_BLACK, 0, 0,
  0, 24, 0, HT_LEFT, 1, 0, 0,
  YES, YES, 0, 0, 0
};
PRIVATE HTStyle HTStyleExample =
{&HTStyleGlossaryCompact, "Example", "XMP",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 0, 0, 0, HT_LEFT, 1, 0, 0,
 NO, NO, 1, 1, 0};

PRIVATE HTStyle HTStyleListing =
{&HTStyleExample, "Listing", "LISTING",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 0, 0, 0, HT_LEFT, 1, 0, 0,
 NO, NO, 1, 1, 0};

PRIVATE HTStyle HTStyleAddress =
{&HTStyleListing, "Address", "ADDRESS",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 0, 0, 0, HT_RIGHT, 1, 0, 0,
 NO, NO, 1, 1, 0};

PRIVATE HTStyle HTStyleHeading1 =
{&HTStyleAddress, "Heading1", "H1",
 HT_FONT + HT_CAPITALS + HT_BOLD, 1.0, HT_BLACK, 0, 0,
 0, 0, 0, HT_CENTER, 1, 0, 0,
 YES, YES, 1, 1, 0};

PRIVATE HTStyle HTStyleHeading2 =
{&HTStyleHeading1, "Heading2", "H2",
 HT_FONT + HT_BOLD, 1.0, HT_BLACK, 0, 0,
 0, 0, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 1, 0};

PRIVATE HTStyle HTStyleHeading3 =
{&HTStyleHeading2, "Heading3", "H3",
 HT_FONT + HT_CAPITALS, 1.0, HT_BLACK, 0, 0,
 2, 2, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 0, 0};

PRIVATE HTStyle HTStyleHeading4 =
{&HTStyleHeading3, "Heading4", "H4",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 4, 4, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 0, 0};

PRIVATE HTStyle HTStyleHeading5 =
{&HTStyleHeading4, "Heading5", "H5",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 6, 6, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 0, 0};

PRIVATE HTStyle HTStyleHeading6 =
{&HTStyleHeading5, "Heading6", "H6",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 8, 8, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 0, 0};

PRIVATE HTStyle HTStyleHeading7 =
{&HTStyleHeading6, "Heading7", "H7",
 HT_FONT, 1.0, HT_BLACK, 0, 0,
 10, 10, 0, HT_LEFT, 1, 0, 0,
 YES, YES, 1, 0, 0};

/* Style sheet points to the last in the list:
*/
PRIVATE HTStyleSheet sheet =
{"default.style", &HTStyleHeading7};	/* sheet */

PUBLIC HTStyleSheet *styleSheet = &sheet;
