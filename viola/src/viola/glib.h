#include "glib_x.h"

/****************************************************************************
 */
#define BORDER_NONE 			0
#define BORDER_NONE_THICKNESS 		0

#define BORDER_BOX  			1
#define BORDER_BOX_THICKNESS 		0

#define BORDER_FIELD 			2
#define BORDER_FIELD_THICKNESS 		0

#define BORDER_BUTTON 			3
#define BORDER_BUTTON_THICKNESS 	0

#define BORDER_THINBUTTON 		4
#define BORDER_THINBUTTON_THICKNESS 	0

#define BORDER_FRAME 			5
#define BORDER_FRAME_THICKNESS 		0

#define BORDER_BORDER 			6
#define BORDER_BORDER_THICKNESS 	1

#define BORDER_SINK 			7
#define BORDER_SINK_THICKNESS 		0

#define BORDER_MENU 			8
#define BORDER_MENU_THICKNESS	 	0

#define BORDER_SUBWINDOW 		9
#define BORDER_SUBWINDOW_THICKNESS 	0   /* exception */

#define BORDER_STYLE_COUNT 10
extern borderStyleThickness[]; 

/****************************************************************************
 */
#define VObjWIN_ASSOC_SIZE 500

/****************************************************************************
 */
typedef struct IntPair {
	int x, y;
} IntPair;

extern IntPair mouse;

/****************************************************************************
 */
#define LEFT_MOUSE_BUTTON 	1
#define MIDDLE_MOUSE_BUTTON 	2
#define RIGHT_MOUSE_BUTTON 	3

/****************************************************************************
 */
/*
#define MAXFONTS 30
*/
#define MAXFONTS 30

typedef struct FontInfo {
	Font font;
	XFontStruct *fontStruct;
	char *ref;			/* font reference string*/
	char *spec;			/* x spec 		*/
	int refHashKey;			/* for fast compares... */
	int maxwidth;
	int maxheight;
	int descent;
	int *widths;	/* font width lookup table */
} FontInfo;

#define FontFont(id) fontInfo[id].font
#define FontFontStruct(id) fontInfo[id].fontStruct
#define FontRef(id) fontInfo[id].ref
#define FontRefHashKey(id) fontInfo[id].refHashKey
#define FontSpec(id) fontInfo[id].spec
#define FontMaxWidth(id) fontInfo[id].maxwidth
#define FontMaxHeight(id) fontInfo[id].maxheight
#define FontDescent(id) fontInfo[id].descent
#define FontWidths(id) fontInfo[id].widths

/* semi-kludgy solution */
extern int fontID_fixed;
extern int fontID_normal;
extern int fontID_bold;
extern int fontID_normal_large;
extern int fontID_bold_large;
extern int fontID_normal_largest;
extern int fontID_bold_largest;

/****************************************************************************
 */

extern HashTable *window2Obj;
extern int maxNumOfFonts;
extern FontInfo fontInfo[];
extern int lookAndFeel;

#define LAF_VIOLA	0
#define LAF_BARE 	1
#define LAF_OPENL 	2
#define LAF_MOTIF 	3

#define LAF_BOX_TXT 	10	/* dummy */

/****************************************************************************
 */
int init_glib();
Window bossWindow();
void unMapObject();
VObj *findWindowObject();

