/*
 * XlFormatText.h --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Fri Mar  6 20:44:40 1992 tvr
 * Last modified: Mon Mar  9 04:57:21 1992 tvr
 *
 */

/*
 * Returncode of line checking
 */
#define FORMAT_FULL_LINE		1
#define FORMAT_NEXT_OBJECT_TOO_LONG	2
#define FORMAT_REGION_ENDS		4
#define FORMAT_NEW_STYLE		8
#define FORMAT_ONLY_OBJECT_TOO_LONG	16

/*
 * Indent defines
 */
#define I_LEFT				1
#define I_FIRST				2
#define	I_MIDDLE_L			4
#define	I_MIDDLE_R			8
#define	I_RIGHT				16

/*
 * Font style flags stored on XlStyle_t
 */

/*
 * Line styles
 */
#define STYLE_LEFTIFY				0
#define STYLE_RIGHTIFY				1
#define STYLE_CENTER				2
#define STYLE_JUSTIFY				3

#define STYLE_MASK				3

/*
 * Do not format text
 */
#define STYLE_RAW				4

/*
 * Generate region change on tabulator
 */
#define STYLE_CHANGE_REGION_ON_TAB		8


/*
 * How to set current margins
 */

#define MARGIN_START_STYLE			0
#define MARGIN_HANDLE_P_OBJECT			1
