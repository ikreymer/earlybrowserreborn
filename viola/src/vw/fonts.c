/*
 * fonts.c
 *
 * Mechanism for loading the title font.  Avoids the fontList resource bug
 * in Motif 1.2.3.
 *
 */
/*
 * Copyright 1993 O'Reilly & Associates. All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */

#include <Xm/Xm.h>


XFontStruct *loadFont(display, fontNames)
    Display *display;
    char **fontNames;
{
    int i=0;
    XFontStruct *font;

    while(fontNames[i])
        if ( !(font = XLoadQueryFont(display, fontNames[i])))
	    i++;
	else
	    break;
    return(font);
}


char *titleFonts[] = {
    "*times*bold*-r-*--24-*",
    "*times*bold*-r-*--25-*",
    "*times*bold*-r-*--18-*",
    "*times*bold*-r-*--17-*",
    "*helvetica*medium*-r-*--24-*",
    "*helvetica*medium*-r-*--25-*",
    "*helvetica*medium*-r-*--18-*",
    "*helvetica*medium*-r-*--17-*",
    "*bold*-r-*--24-*iso8859*",
    "*bold*-r-*--25-*iso8859*",
    "*bold*-r-*--18-*iso8859*",
    "*bold*-r-*--17-*iso8859*",
    "9x15",
    "8x13",
    "*bold*-r-*iso8859*",
    "*bold*iso8859*",
    "*iso8859*",
};


char *urlFonts[] = {
    "*times*bold*-r-*--18-*",
    "*times*bold*-r-*--17-*",
    "*times*bold*-r-*--14-*",
    "*times*bold*-r-*--13-*",
    "*helvetica*medium*-r-*--18-*",
    "*helvetica*medium*-r-*--17-*",
    "*helvetica*medium*-r-*--14-*",
    "*helvetica*medium*-r-*--13-*",
    "*bold*-r-*--18-*iso8859*",
    "*bold*-r-*--17-*iso8859*",
    "*bold*-r-*--14-*iso8859*",
    "*bold*-r-*--13-*iso8859*",
    "9x15",
    "8x13",
    "*bold*-r-*iso8859*",
    "*bold*iso8859*",
    "*iso8859*",
};


char *subTitleFonts[] = {
    "*helvetica*medium*-r-*--14-*",
    "*helvetica*medium*-r-*--12-*",
    "*helvetica*medium*-r-*--11-*",
    "9x15",
    "8x13",
    "*medium*-r-*--14-*iso8859*",
    "*medium*-r-*--12-*iso8859*",
    "*medium*-r-*--11-*iso8859*",
    "*medium*-r-*iso8859*",
    "*iso8859*",
};


XmFontList loadFonts(display)
    Display *display;
{
    XmFontList fl;
    XFontStruct *fp;

    fp = loadFont(display, subTitleFonts);
    fl = XmFontListCreate(fp, "subTitleFont");
    fp = loadFont(display, titleFonts);
    fl = XmFontListAdd(fl, fp, "titleFont");
    fp = loadFont(display, urlFonts);
    fl = XmFontListAdd(fl, fp, "urlFont");
    return(fl);
}
