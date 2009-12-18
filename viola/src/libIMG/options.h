/* options.h:
 *
 * optionNumber() definitions
 *
 * jim frost 10.03.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

/* enum with the options in it.  If you add one to this you also have to
 * add its information to Options[] in options.c before it becomes available.
 */

typedef enum option_id {

  /* global options
   */

  OPT_NOTOPT= 0, OPT_BADOPT, OPT_SHORTOPT, OA_BORDER, OA_DBUG, OA_DEFAULT, OA_DELAY,
  OA_DISPLAY, OA_FIT, OA_FORK, OA_FULLSCREEN, OA_GEOMETRY, OA_HELP, OA_IDENTIFY, OA_INSTALL,
  OA_LIST, OA_ONROOT, OA_PATH, OA_PIXMAP, OA_PRIVATE, OA_QUIET, OA_SUPPORTED, OA_VERBOSE,
  OA_VER_NUM, OA_VIEW, OA_VISUAL, OA_WINDOWID,

  /* local options
   */

  OA_AT, OA_BACKGROUND, OA_BRIGHT, OA_CENTER, OA_CLIP, OA_COLORS, OA_DITHER, OA_FOREGROUND,
  OA_GAMMA, OA_GOTO, OA_GRAY, OA_HALFTONE, OA_IDELAY, OA_INVERT, OA_MERGE, OA_NAME, OA_NEWOPTIONS,
  OA_NORMALIZE, OA_ROTATE, OA_SMOOTH, OA_XZOOM, OA_YZOOM, OA_ZOOM
} OptionId;

typedef struct option_array {
  char     *name;        /* name of the option minus preceeding '-' */
  OptionId  option_id;   /* OptionId of this option */
  char     *args;        /* arguments this option uses or NULL if none */
  char     *description; /* description of this option */
} OptionArray;

OptionId optionNumber(); /* options.c */
