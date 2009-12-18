/*		FOSI Definition Interface			FOSIDTD.h
**		=========================
**
**	SGML purists should excuse the use of the term "DTD" in this
**	file to represent DTD-related information which is not exactly
**	a DTD itself.
**
**	The C modular structure doesn't work very well here, as the
**	dtd is partly in the .h and partly in the .c which are not very
**	independent.  Tant pis.
*/

#ifndef FOSIDTD_H
#define FOSIDTD_H

#include "SGML.h"

/*	Element Numbers
**	---------------
**
**	Must Match all tables by element!
*/
typedef enum _FOSIElement {
	FOSI_CHARLIST,
	FOSI_DOCDESC,
	FOSI_E_I_C,
	FOSI_FONT,
 	FOSI_INDENT,
	FOSI_STYLDESC,
} FOSIElement;

#define FOSI_ELEMENTS 46

/*	Attribute numbers
**	-----------------
**
**	Name is <element>_<attribute>.  These must match the tables in .c!
*/
#define FOSI_E_I_C_GI 		0
#define FOSI_E_I_C_ATTRIBUTES	1

#define FOSI_FONT_BGCOL		0
#define FOSI_FONT_FGCOL		1
#define FOSI_FONT_POSTURE	2
#define FOSI_FONT_SIZE		3
#define FOSI_FONT_STYLE		4
#define FOSI_FONT_WEIGHT	5
#define FOSI_FONT_WIDTH		6
#define FOSI_FONT_ATTRIBUTES	7

#define FOSI_INDENT_FIRSTLN	0
#define FOSI_INDENT_LEFTIND	1
#define FOSI_INDENT_RIGHTIND	2
#define FOSI_INDENT_ATTRIBUTES	3

extern CONST SGML_dtd FOSI_dtd;

#endif /* FOSIDTD_H */

