/* tfed internal decls */

extern int MSTAT_tfed;
extern int MSTAT_tfed_convertNodeLinesToStr;

void tfed_registerTmpFileToFree();
void tfed_FreeTmpFileToFree();

/*MemoryGroup *tfed_mg;*/

/*#define Vmalloc(group, size) malloc(size)*/
/*#define Vfree(group, ptr) free(ptr)*/

extern MethodInfo defaultKeyBinding[];

extern int (*kbflookup[])();
extern char sbuff[];
extern int buffi;

/*  
 * VT100 sequence for reversing and unreverse video 
 */
extern char enterReverse_vt100[];
extern char leaveReverse_vt100[];

/* used to specify 
 */
#define STAG_OPEN '('
#define STAG_CLOSE ')'

extern TFChar TFC_ARRAY_NEWLINE[];

#define TFC_TAB_SPACES 8

extern TFChar TFC_ARRAY_SPACES[];

#define RANGEMODE_NONE 0
#define RANGEMODE_CLIP 1
#define RANGEMODE_DELETE 2

#define BORDERTW 4
#define BORDERTH 4
#define PXX 2
#define PYY 2

/* CHAR=no update, LINE=update line, SCREEN=update screen */
#define CHAR 0
#define LINE 1
#define SCREEN 2
extern int refreshMode; 
extern int updateShown; 

#define TFCBUFF_SIZE 100000
extern TFChar tfcEditStr[TFCBUFF_SIZE];
extern TFChar tfcYankStr[TFCBUFF_SIZE];
extern TFChar tfcBuffStr[TFCBUFF_SIZE];
extern TFLineNode *theEditLN;	/* editing buffer		*/
extern TFLineNode *theYankLN;	/* cut/paste/kill/yank buffer 	*/
extern TFLineNode *theBuffLN;	/* very temporary buffer	*/

#define ANCHOR_LINES_SPAN 64
extern int tfed_anchorPatchIdx;
extern char **tfed_anchorPatch[];
extern int tfed_anchorInP;

extern TFPic *dunselPic;

extern XTextItem xcharitem;	/* used by drawChar() */

#define cursorWithinField(tf) \
	((tf->csr_py >= 0) && (tf->csr_py <= tf->yLR))

#define TFCCopy(tfcTo, tfcFrom) \
	{(tfcTo)->c = (tfcFrom)->c; \
	 (tfcTo)->tagID = (tfcFrom)->tagID; \
	 (tfcTo)->fontID = (tfcFrom)->fontID; \
	 (tfcTo)->flags = (tfcFrom)->flags;}

#define textPixelWidth(fontID, FontFont) \
        XTextWidth(FontFont(fontID), str, (int)strlen(str))

#define TBUFFSIZE 1024 /*XXX maximum line length. Used by tfed_buildLines() */

#define TFWINDOW GET_window(tf->self)

typedef struct TFCBuildInfo {
	VObj *self;
	TFLineNode *currentp;
	TFLineNode *beginp;
	char *str;
	int fontID;
	int lineNodeCountp;
	int lineVisibleCountp;
	TFChar *tbuff;
	int tbuffi;
	TagInfo buffTagInfo[TAGINFO_SIZE];	
	int buffTagID[TAGINFO_SIZE];
	int tagID;
	int flags;
	int maxFontHeight;
	int maxFontDescent;
	int vspan;
	int breaki;
	int px;
	int makeLinep;
	int format;
	int verbatim;
	TFPic *pics;
	int spaceWidth;
} TFCBuildInfo;

/*
 * forward declaration of all procedures refenced only within this file
 */
int TFCstrlen();
int TFCstrcat();
int str2EBuff();
int TFC2StrStrcpy();
int TFCstrcpy();
int TFCstrncpy();

int translateCol2Px();
int translatePx2Col();

void placeCursorWithinStr();
void placeCursor();
int joinLine();
int TFCInsertStr();
int moveOffset();
int jumpLine();
int tfed_scroll_delta();
int moveLine();
int moveLineNode();
int tfed_buildLines();
int splitLine();
int deleteLineNode();
TFLineNode *insertLineNode();
TFLineNode *insertBelowLineNode();
char *convertNodeLineToStr();
void freeNodeLines();
void dumpNodeLines();
int renderTF();
int TimedDrawCursor();
int TimedEraseCursor();
void invertCursor();
void drawCursor();
void eraseCursor();
void drawChar();
int traverseToRightEdge();
int scrollLineForward();
int scrollLineBackward();
int scrollDownLowerPart();
int scrollDownLowerPartNEW();
int scrollUpLowerPart();
int scrollUpLowerPartNEW();
int lineSegWidth();
void drawLine();
int drawLineOffset();
int drawTextFieldCursor();
TFStruct *updateEStrUser();
int TFCShiftStr();
void TFCInsertChar();
int setBreaks();
int setCurrentFontID();
int LogicOrTFCFlag();
int LogicAndTFCFlag();
char *rangeOperation();
