/*
 * tfed.h
 */
#include "ascii.h"

extern MethodInfo defaultKeyBinding[];

#define MAX_EDIT_LINE_LENGTH 20000
#define OUT_OF_RANGE 999999;

/* shift into reverse and out of reverse */
#define SHIFT_IN 15
#define SHIFT_OUT 14

typedef struct TFChar {
	unsigned char c;
	char fontID;
	char tagID;
	char flags;
} TFChar;

/* This should be replaced with generalized viola objects 
 */
#define TFPic_XBML 0
#define TFPic_XBM 1
#define TFPic_GIF 2
#define TFPic_XPM 3

typedef struct TFPic {
	int id;
	int type; 
	XImage *data;
	int width;
	int height;
	char canFree;
	struct TFPic *next;
} TFPic;

#define TFCChar(tfcp) 	(tfcp)->c
#define TFCFontID(tfcp)	(tfcp)->fontID
#define TFCTagID(tfcp) 	(tfcp)->tagID
#define TFCFlags(tfcp) 	(tfcp)->flags
#define TFCWidth(tfcp)	FontWidths((tfcp)->fontID)[(tfcp)->c]
#define TFCClear(tfcp) 	{(tfcp)->c = 0; \
			(tfcp)->fontID = 0; \
			(tfcp)->tagID = 0; \
			(tfcp)->flags = 0;} 
#define TFCSet(tfcp, c, tagval, fontID) \
	{(tfcp)->c = c; (tfcp)->tagID = tagval; (tfcp)->fontID = fontID;}

/* used in char info 
 */
#define TAGINFO_SIZE	16

typedef struct TagInfo {
	char *info;
/*	int x_begin;
	int x_end;
	int valid;
*/
} TagInfo;

/* this whole flags setup needs to be revampped */
#define MASK_WRAP	(1<<0)	/* line wrap break (internal)*/
#define MASK_REVERSE	(1<<1)	/* reverse video	*/
/*#define MASK_UNDER	(1<<2)*//* underline		*/
#define MASK_RELIEF	(1<<2)	/* for hypertext links	*/
#define MASK_BUTTON	(1<<3) 	/* fake button 		*/
/*#define MASK_XRULE	(1<<4)*//* horizontal rule 	*/
#define MASK_OBJ	(1<<4)	/* object be here 	*/
#define MASK_NL		(1<<5)	/* really a newline	*/
#define MASK_PIC	(1<<6)	/* picture be here	*/
#define MASK_ITALIC	(1<<7)	/* be italic		*/

typedef struct TFLineNode {
	TFChar *linep;
	struct TFLineNode *next;
	struct TFLineNode *prev;
	TagInfo *tagInfo;
	int tagInfoCount;	/* count of tagInfo array elements */
	int length;		/* number of characters */
	int maxPixelExtentX;	/* for minimizing b/c of Expose events */
	int maxPixelExtentY;
	short maxFontHeight;
	short maxFontDescent;
	short breakc;		/* break count */
	short breakcIsValid;	/* to avoid redundantly recomputing breakc */
} TFLineNode;

typedef struct TFStruct {

	/* line node pointers
	 */
	TFLineNode *currentp;
	TFLineNode *firstp;
	TFLineNode *lastp;
	TFLineNode *offsetp;

	/* position, in chars, of the cursor,
	 * offset from left-upper corner of the text.
	 */
	int current_col;
	int current_col_sticky;
	int current_row;

	/* position, in chars, of the cursor,
	 * offset from left-upper corner of the field.
	 */
	int csr_px;
	int csr_px_sticky;
	int csr_py;

	/* offset of visible portion of the field from the entire field...
	 */
	int screen_col_offset;
	int screen_row_offset;

	/* number of line nodes in the entire field
	 */
	int lineNodeCount;

	/* number of visible lines in the entire field (sum of breakc)
	 */
	int lineVisibleCount;

	/* number of lines within the param 
	 */
	int num_of_lines;

	int esc_toggle;
	int bufferUsed;		/* used in yank stuff */

	/* stuff copied from self
	 */
	Window w;
	VObj *self;
	int xUL, yUL;		/* upper left x,y */
	int xLR, yLR;		/* lower right x,y */
	int width, height;
	int fontID;
	int currentFontID;
	int wrap;
	int cursorIsVisible;
	long cursorTimeInfo;
	long cursorBlinkDelay;  /* millisecs to next state change */
	int highLiteFrom_cx, highLiteFrom_cy;
	int highLiteTo_cx, highLiteTo_cy;
	int isRenderAble;
	int shownPosition;
	int shownSize;

	int building_maxFontHeight;	/* used for object embedding */
	int building_maxFontDescent;
	int building_vspan;
	int editableP;
	MemoryGroup *mg;

	int align;/*using paneConfig values */

	TFPic *pics;
	
	/*
	 * Dumb stuff for the text search "Feature"
	 *
	 */
	int search_len, search_x, search_y;

} TFStruct;
		
/**************************************************************************
 * for external referencing
 */
extern int 	tfed_init();
TFStruct *	tfed_setUpTFStruct();
TFStruct *	tfed_updateTFStruct();
TFStruct *	tfed_clone();
TFStruct *	updateEStrUser();
int 		tfed_render();
int 		tfed_processMouseInput();
char *		tfed_processMouseMove();
int 		tfed_processKeyEvent();
void 		tfed_insertChar();
int 		tfed_setBuffer();
int 		tfed_getBuffer();
int 		tfed_drawCursor();
int 		tfed_eraseCursor();
int 		tfed_jumpToOffsetLine();
int 		tfed_jumpToLine();
int		tfed_setCursorBlinkDelay();
int		tfed_expose();
int 		kbf_newline();
int		kbf_dump();
int		kbf_refresh();
int		kbf_beginning_of_line();
int		kbf_backward_char();
int		kbf_delete_char();
int		kbf_end_of_line();
int		kbf_forward_char();
int		kbf_delete_backward_char();
int		kbf_join_line();
int		kbf_kill_line();
int		kbf_open_line_below();
int		kbf_next_line();
int		kbf_open_line();
int		kbf_previous_line();
int		kbf_scroll_up_line();
int		kbf_delete_line();
int		kbf_scroll_up();
int		kbf_scroll_down();
int		kbf_scroll_up();
int		kbf_insert_yank();
int		kbf_scroll_down_line();
int		kbf_delete_backward_char();
int		kbf_esc_prefix();
int		kbf_newline();
int		kbf_ident();
int		kbf_useFont_context();
int		kbf_useFont_fixed();
int		kbf_useFont_normal();
int		kbf_useFont_normal_large();
int		kbf_useFont_normal_largest();
int		kbf_useFont_bold();
int		kbf_useFont_bold_large();
int		kbf_useFont_bold_largest();

int tfed_get_currentChar();
int tfed_get_currentLine();
int tfed_get_cursorColumn();
int tfed_get_cursorRow();
int tfed_get_lineRowOffset();
int tfed_get_charMask();
char *tfed_get_currentTag();
int tfed_get_currentWord();
char *tfed_get_nextTag();
char *tfed_get_previousTag();
int tfed_get_numberOfLinesDisplayed();
int tfed_get_totalLineCount();
int tfed_set_wrap();
int tfed_append();
int tfed_clearSelection();
char *tfed_getSelection();

char *convertNodeLinesToStr();
int scanVerticalMetrics();
int lineFlagSet();

#define DUMP_CHAR 1
#define DUMP_FONT 2
#define DUMP_WRAP 3
#define DUMP_NL   4
void dumpTFCArray();

int tfed_setReverseMaskInButtonRange();
int tfed_setReliefMaskInButtonRange();

TFPic *tfed_addPicFromFile();
TFPic *tfed_addPic();


