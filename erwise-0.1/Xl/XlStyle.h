/*
 * XlStyle.h --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Thu Mar  5 00:32:52 1992 tvr
 * Last modified: Tue May 12 21:49:52 1992 tvr
 *
 */

/*
 * Flags needed for text formatting are on XlFormatText.h
 */

/*
 * This structure contains *all* information needed to format text
 */

typedef struct XlStyle {
    /*
     * Font structure
     */
    XFontStruct *fontinfo;

    /*
     * Name of the font used to display this style. When we setup resources,
     * identify whether we should or should not free original string.
     */
    char *fontname;
    int must_free_fontname;

    /*
     * Name of the font on configure -data
     */

    char *config_name;

    /*
     * Tags. This way no enumeration is needed
     */
    char *styletags;

    /*
     * Width of a character 'm'.
     */
    int char_width;

    /*
     * Stylespecific object positioning
     */
    int mode;

    /*
     * How much to indent first line of a paragraph
     */
    int indent1st;

    /*
     * left and righ limits
     */
    int left;
    int right;

    /*
     * Spaces on linespaces, stylechanges and paragraphchanges
     */
    int space_line;
    int space_paragraph;
    int space_style_before;
    int space_style;

} XlStyle_t;



extern XlStyle_t XlStyles[];
extern int xl_lineheightandgap();
extern int xl_wordgap();
extern int xl_object_style();



#define MIDDLE_L_HACK		25
#define MIDDLE_R_HACK		30
