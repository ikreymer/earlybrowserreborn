/*
 * html.c
 * 
 * Parts of this file originated from the lineMode browser 
 * (HTBrowse.c and GridText.c) by Nicola Pellow and Tim Berners-Lee.
 *
 * Authors: Nicola Pellow, Tim Berners-Lee, Pei-Yuan Wei.
 *
 */
#include <ctype.h>
#include "utils.h"

#include "HTFont.h"

#undef HTSTRING_H
#include "../libWWW/Library/Implementation/HTString.h"
#include "../libWWW/Library/Implementation/HTFormat.h"
#include "../libWWW/Library/Implementation/HTTCP.h"
#include "../libWWW/Library/Implementation/HTAnchor.h"
#include "../libWWW/Library/Implementation/HTParse.h"
#include "../libWWW/Library/Implementation/HTAccess.h"
#include "../libWWW/Library/Implementation/HTHistory.h"
#include "../libWWW/Library/Implementation/HText.h"
#include "../libWWW/Library/Implementation/HTML.h"

#undef HTWRITE_H
#include "../libWWW/Library/Implementation/HTWriter.h"
#include "../libWWW/Library/Implementation/HTFile.h"

#include "mystrings.h"
#include "sys.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "vlist.h"
#include "attr.h"
#include "glib.h"
#include "slib.h"
#include "misc.h"
#include "tfed.h"
#include "sgml.h"
#include "html.h"
#include "html2.h"
#include "DefaultStyles.h"

#ifdef TRACE
#undef TRACE
#define TRACE WWW_TraceFlag
#endif

int loaded_limit = 30;			/* default cache limit */
char *default_default = 0; /* @@ Parse home relative to this */
char *current_addr = 0;    /* most recent doc addr. Bogus Alert... */

/* Forward decls
 */
TFStruct *html_updateTFtruct();
int html_convertStrToNodeLines();
int moveToSelectedAnchor();
HText *html_findHTextByAddress();
HText *html_updateHTextTFStruct();
HText *html_loadDocument();

/* Used to by HText_new() to get Viola information
 */
TFStruct *new_tfstruct;

/*	From default style sheet:
*/
extern HTStyleSheet * styleSheet;	/* Default or overridden */

PRIVATE HTStyle default_style =
	{ 0,  "(Unstyled)", "",
	HT_NORMAL, 1.0, HT_BLACK,	0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0, 
	NO, NO, 0, 0,			0 };	

typedef struct _line {
	struct _line	*next;
	struct _line	*prev;
	short unsigned	offset;		/* Implicit initial spaces */
	short unsigned	size;		/* Number of characters */
	BOOL	split_after;		/* Can we split after? */
	BOOL	bullet;			/* Do we bullet? */
	char	data[1];		/* Space for terminator at least! */
	int	pixel_x;		/* current pixel size */
} HTLine;

typedef struct _TextAnchor {
	struct _TextAnchor *	next;
	int			number;		/* For user interface */
	int			start;		/* Characters */
	int			extent;		/* Characters */
	int			beginLine;	/* Line# in text field */
	int			endLine;	/* Line# in text field */
	HTChildAnchor *		anchor;
} TextAnchor;


struct _HText {
	HTParentAnchor *	node_anchor;
	char *			expandedAddress;
	char *			simpleAddress;
	char *			title;
	HTLine * 		last_line;
	int			lines;		/* Number of them */
	int			chars;		/* Number of them */
	TextAnchor *		first_anchor;	/* Singly linked list */
	TextAnchor *		last_anchor;
	int			last_anchor_number;	/* user number */
/* For Internal use: */	
	HTStyle *		style;			/* Current style */
	HTStyle *		next_style;		/* next style */
	int			permissible_split;	/* in last line */
	BOOL			in_line_1;		/* of paragraph */
/* For Viola use: */
	TFStruct		*tfstruct;
	TFLineNode 		*currentp;
	TFLineNode 		*insertp;
	int 			size;
	int 			px;
	int 			fontID;
	int 			next_fontID;
	int 			tagID;
	TFChar 			*tbuff;
	int 			tbuffi;
	TagInfo			tbuffTagInfo[TAGINFO_SIZE];
	int			tbuffTagInfoCount;
	int 			maxFontHeight;
	int 			maxFontDescent;
	int			flags;
	char			*htmlSrc;
	int 			htmlSrcSize;
	int 			isIndex;
};

PUBLIC HText * HTMainText = 0;		/* Equivalent of main window */
PUBLIC HTParentAnchor * HTMainAnchor = 0;	/* Anchor for HTMainText */

#ifdef NONONONONONONO
PUBLIC  int  WWW_TraceFlag = 0;     /* Off unless -v option given */
PUBLIC FILE * logfile = 0;          /* File to output one-liners to */
PUBLIC char * HTClientHost = 0;	/* Name or number of telnetting host */
#endif

char *srcBuff;
char *srcName = "";
int srcBuffi = 0;
/*int srcBuffSize = 100000; *//*XXX obsolete*/
int srcBuffSize = 1; /*XXX*/

PRIVATE HTList * loaded_texts;	/* A list of all those in memory */

/* name of temporary file used to save data fetched by HTJustRead()
 * HTJustRead is passed to HTLoadAbsolute()...
 */
char *tempFileName = NULL;
int tempFileStat = -1;
HTFormat tempFileFormat;

#define ANCHOR_LINES_SPAN 64
int anchorPatchIdx = 0;
int **anchorPatch[ANCHOR_LINES_SPAN];
int anchorInP = 0;

/* Define Statements */
/* ================= */

#ifndef EOF
#define EOF (-1)                    /* End of file character defined as -1 */
#endif

#define WHITE_SPACE(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r'))
                                  /* Definition for any kind of white space */

/*	Public Variables
**	================
*/
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))


/* Arrays for storing the HyperText References */ 
PRIVATE char *	     logfile_root = 0;	    /* Log file name */
PRIVATE char *	     logfile_name = 0;	    /* Root of log file name */


/* Misc Externs */
/* ================================ */
extern char *saveString(); /* in mystrings.c */

/* Forward Declaration of Functions */
/* ================================ */

void History_List NOPARAMS; 

#define TBUFFSIZE 1024 /*XXX maximum line length. Used by tfed_buildLines() */
TFChar tbuff[TBUFFSIZE];

#define HBUFFSIZE 100000 /* buffer for hidden/embedding text (scripts) */
char hbuff[HBUFFSIZE];
int hbuffi;
int inHiddenState = 0;

int init_html()
{
	extern char *getcwd();
	char wd[MAXPATHLEN];
	char *result;

	WWW_TraceFlag = verbose ? 1 : 0;

	/* Eg file://cernvax.cern.ch */
	StrAllocCopy(default_default, "file://");
	StrAllocCat(default_default, HTHostName());

	result = getcwd(wd, MAXPATHLEN);
	if (result) {
		StrAllocCat(default_default, wd);
	} else {
		fprintf(stderr, 
			"HTBrowse: Can't read working directory.\n");
		StrAllocCat(default_default, "/default.html");
	}

	current_addr = saveString(default_default);

	srcBuff = (char*)malloc(sizeof(char) * srcBuffSize);
	if (!srcBuff) return 0;
	srcBuffi = 0;

	init_html2();

	return 1;
}

int html_backtrack()
{
	if (!HTHistory_canBacktrack()) {
		return 0;
	}
	HTLoadAnchor(HTHistory_backtrack());
	return 1;
}

int html_search(self, keyword)
	VObj *self;
	char *keyword;
{
	HText *htext;
	TFStruct *oldtf, *tf;
	int status;
	char *address = HTAnchor_address((HTAnchor*)HTMainAnchor);
	char *newAddress;
	char *p;	          /* pointer to first non-blank */
	char *q, *s;

	oldtf = GET__TFStruct(self);

	/* instead of changing HTSearch(), its code is transplanted here 
	 * for edition
	 */
	p = HTStrip(keyword);
	for (q=p; *q; q++) if (WHITE(*q)) *q = '+';
	newAddress = saveString(address);
	s=strchr(newAddress, '?');		/* Find old search string */
	if (s) *s = 0;			        /* Chop old search off */
	StrAllocCat(newAddress, "?");
	StrAllocCat(newAddress, p);

	if (htext = html_findHTextByAddress(self, newAddress)) {
		tf = htext->tfstruct;
	} else {
		tf = (TFStruct*)Vmalloc(GET__memoryGroup(self), 
					sizeof(struct TFStruct));
		if (!tf) {
			perror("malloc");
			return 0;
		}
		new_tfstruct = tf; /* for HText_new() to pick up */

		http_method = HTTP_METHOD_GET;
		http_dataToPost = NULL;
		HTPushInputBuffer();
		status = HTLoadRelative(newAddress, 
				HTAnchor_findAddress(newAddress)/*???*/);
		HTPopInputBuffer();
		http_method = HTTP_METHOD_GET;
		http_dataToPost = NULL;

		if (verbose) fprintf(stderr, "HTSearch result=%d\n", status);
		if (!status) return 0;

		HTHistory_record((HTAnchor*)HTMainAnchor);
		htext = HTMainText;
		newAddress = HTAnchor_address((HTAnchor*)HTMainAnchor);
		htext->simpleAddress = newAddress;
		htext->title = saveString(HTAnchor_title(htext->node_anchor));
		htext->expandedAddress = 
			saveString(htext->node_anchor->address);

		htext->htmlSrc = saveString(srcBuff);
		htext->htmlSrcSize = srcBuffi;
		htext->isIndex = HTAnchor_isIndex(htext->node_anchor) ? 1 : 0;

		if (oldtf) bcopy(oldtf, tf, sizeof(struct TFStruct));

		tf->lineNodeCount = tf->lineVisibleCount = htext->lines;
		tf->firstp = htext->insertp;
		tf->offsetp = tf->firstp;
		tf->currentp = tf->firstp;
		tf->current_row = 0;
		tf->current_col_sticky = tf->current_col = 0;
		tf->screen_col_offset = 0;
		tf->screen_row_offset = 0;
		tf->shownPosition = 0;
		tf->shownSize = 0;

		/* could be better. !make an (inheritable) flag in class def */
		tf->editableP = 0;
		tf->mg = GET__memoryGroup(self);

		if (!html_updateHTextTFStruct(oldtf, tf)) {
			fprintf(stderr, 
			"Error: html_updateHTextTFStruct() failed\n");
		}
		if (oldtf) {
			if (oldtf->firstp) freeNodeLines(oldtf);
			Vfree(tf->mg, oldtf);
		}
	}
	updateEStrUser(NULL); /* purge cache */
	SET__TFStruct(self, tf);
	SET_HTMLAddress(self, newAddress ? newAddress : "?");
	SET_HTMLAnchor(self, "");
	SET_HTMLIsIndex(self, htext->isIndex);
	SET_HTMLTitle(self, htext->title ? htext->title : "");
	SET_HTMLSource(self, htext->htmlSrc);

	renderTF(tf);

	free(address);
	return 1;
}

char *html_pathSimplify(path)
	char *path;
{
	HTSimplify(path);
	return path;
}

PUBLIC HText *	HText_new2 ARGS2(HTParentAnchor *,anchor, HTStream *, output_stream)
{
  printf("############### HText_new2() was called -> HText_new()\n");
  return HText_new(anchor);
}

/*			Creation Method
**			---------------
*/
PUBLIC HText *	HText_new ARGS1(HTParentAnchor *,anchor)
{
	HText * htObj = (HText*) malloc(sizeof(*htObj));
	VObj *self;
	extern SGMLBuildInfo SBI;

	if (!htObj) {
		fprintf(stderr, "malloc failed\n");
		return 0;
	}
    
	htObj->last_line = NULL;
	htObj->lines = htObj->chars = 0;
	htObj->title = NULL;
	htObj->expandedAddress = NULL;
	htObj->simpleAddress = NULL;
	htObj->first_anchor = htObj->last_anchor = 0;
	htObj->style = &default_style;
	htObj->next_style = &default_style; /* badness... */
	htObj->node_anchor = anchor;
	htObj->last_anchor_number = 0;	/* Numbering of them for references */
	htObj->permissible_split = 0;
	htObj->in_line_1 = 0;		/* of paragraph */	

	self = SBI.stack[SBI.stacki].obj;
	if (!self) {
	  return 0;
	}
	htObj->tfstruct = GET__TFStruct(self);

	htObj->size = 0;
	htObj->px = 0;
	htObj->flags = NULL;
	htObj->currentp = NULL;
	htObj->insertp = NULL;
	htObj->fontID = NULL;
	htObj->tbuff = tbuff;	/* XXX non-reentrant */
	htObj->tbuffi = 0;
	htObj->tagID = 0;
	htObj->maxFontHeight = 0;
	htObj->maxFontDescent = 0;

	htObj->htmlSrc = NULL;
	htObj->htmlSrcSize = 0;

	htObj->tbuffTagInfoCount = TAGINFO_SIZE;
	bzero(htObj->tbuffTagInfo, sizeof(struct TagInfo) * TAGINFO_SIZE);

	HTAnchor_setDocument(anchor, (HyperDoc *)htObj);

	htObj->node_anchor = anchor;
	htObj->isIndex = 0;

	HTMainText = htObj;
	HTMainAnchor = anchor;
	
	return htObj;
}


/*	Free Entire Text
**	----------------
*/
PUBLIC void	HText_free ARGS1(HText *,self)
{
	HTAnchor_setDocument(self->node_anchor, (HyperDoc *)0);

#ifdef oneOfThoseThingsIShouldDoButNotDoingHTFREE
	while(YES) {		/* Free off line array */
		HTLine * l = self->last_line;
	l->next->prev = l->prev;
	l->prev->next = l->next;	/* Unlink l */
	self->last_line = l->prev;
	free(l);
	if (l == self->last_line) break;	/* empty */
	};
	
	while(self->first_anchor) {		/* Free off anchor array */
		TextAnchor * l = self->first_anchor;
	self->first_anchor = l->next;
	free(l);
	}
#endif
	free(self);
}

/*	Object Building methods
**	-----------------------
**
**	These are used by a parser to build the text in an object
*/
PUBLIC void HText_beginAppend ARGS1(HText *,text)
{
	text->permissible_split = 0;
	text->in_line_1 = YES;
	text->tbuffi = 0;
}

PUBLIC void HText_beginHiddenAppend ARGS1(HText *,text)
{
	hbuffi = 0;
	inHiddenState = 1;
}

/*	Add a new line of text
**	----------------------
**
** On entry,
**
**	split	is zero for newline function, else number of characters
**		before split.
** On exit,
**		A new line has been made, justified according to the
**		current style. Text after the split (if split nonzero)
**		is taken over onto the next line.
*/
PRIVATE void new_line ARGS1(HText *,text)
{
	PRIVATE void split_line();
	split_line(text, text->tbuffi);
	return;
}

#define VERBOSE_SPLIT_LINE___
PRIVATE void split_line ARGS2(HText *,text, int,split)
{
	HTStyle *style = text->style;
	HTStyle *nextstyle = text->next_style;
	TFLineNode *newp;
	TFChar *tfcp, itfc;
	int size, i;
	int fontID;
	TFStruct *tf = text->tfstruct;
	int patch = 0;

	if (split < 0) return;	/* just in case */

	if (text->tbuffi > 0) {
		fontID = TFCFontID(text->tbuff);
	} else {
		fontID = text->fontID;
	}
#ifdef VERBOSE_SPLIT_LINE
	printf("SPLIT_LINE split=%d, tbuffi=%d, tagID=%d\n", 
		split, text->tbuffi, text->tagID);
	printf("::>>");
	dumpTFCArray(DUMP_CHAR|DUMP_FONT, text->tbuff, text->tbuffTagInfo,
			text->tbuffTagInfoCount);
	printf("<<\n");
#endif
	/* make lineNode
	 */
	newp = (TFLineNode *)Vmalloc(text->tfstruct->mg,
				     sizeof(struct TFLineNode));
	if (!newp) {
		fprintf(stderr, "malloc failed\n");
		return;
	}

	newp->maxPixelExtentX = 0;
	newp->maxPixelExtentY = 0;

	text->tfstruct->building_maxFontHeight = 
		newp->maxFontHeight = text->maxFontHeight;
	text->tfstruct->building_maxFontDescent = 
		newp->maxFontDescent = text->maxFontDescent;

	text->maxFontHeight = FontMaxHeight(fontID);
	text->maxFontDescent = FontDescent(fontID);

	newp->breakc = 0;
	newp->breakcIsValid = 0;

	if (text->tbuffi > 0) {
		text->tfstruct->building_vspan += newp->maxFontHeight;
	} else {
		text->tfstruct->building_vspan += FontMaxHeight(fontID);
	}
	newp->length = split;

	if (anchorInP) {
		patch = 1;
		text->tagID++;
	}
	newp->tagInfoCount = text->tagID + 1;

	/* make linep
	 */
	size = sizeof(struct TFChar) * (split + patch + 1);

	newp->linep = (TFChar*)malloc(size);
	if (!newp->linep) {
		fprintf(stderr, "malloc failed\n");
		return;
	}
	bcopy(text->tbuff, newp->linep, size);
	TFCClear(newp->linep + newp->length);
	TFCFlags(newp->linep + newp->length) = MASK_NL;

	/* make tagInfo
	 */
	newp->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo)
					 * newp->tagInfoCount);
	bzero(newp->tagInfo, sizeof(struct TagInfo) * newp->tagInfoCount);

	bcopy(text->tbuffTagInfo, newp->tagInfo, 
		sizeof(struct TagInfo) * newp->tagInfoCount);

	bzero(text->tbuffTagInfo, 
		sizeof(struct TagInfo) * TAGINFO_SIZE);

	if (patch) {
		anchorPatch[anchorPatchIdx++] = 
			&(newp->tagInfo[text->tagID].info);
		TFCTagID(newp->linep + newp->length - 1) = text->tagID;
	}

#ifdef VERBOSE_SPLIT_LINE
	printf("made line:>>");
	dumpTFCArray(DUMP_CHAR|DUMP_FONT, newp->linep, newp->tagInfo,
			newp->tagInfoCount);
	printf("<<\n");
#endif

	/*
	 * Link the new line to tf struct
	 */
	if (text->currentp) {
		if (text->currentp->next) text->currentp->next->prev = newp;
		newp->prev = text->currentp;
		newp->next = text->currentp->next;
		text->currentp->next = newp;
		text->currentp = newp;
	} else {
		text->currentp = newp;
		text->insertp = newp;
		newp->prev = NULL;
		newp->next = NULL;
	}
	text->lines++;
	text->tagID = 0;
	text->px = 0;

	/* this should not be done! */
#ifdef OBSOLETE
	if (text->tbuffi - split > 0) {
		TFChar *tfcp2;
		int skip = 0;

		tfcp = text->tbuff + split;
		if (TFCChar(tfcp) == ' ') {
			tfcp++;
			split++;
		}
		tfcp2 = text->tbuff;

		/* skip extraneous spaces */
		for (i = text->tbuffi - split; i > 0; i--) {
			if (!ISSPACE(TFCChar(tfcp)) || TFCFlags(tfcp)) break;
			tfcp++;
			skip++;
		}
		for (; i > 0; i--) {
			TFCChar(tfcp2) = TFCChar(tfcp);
			TFCFontID(tfcp2) = fontID;
			TFCTagID(tfcp2) = TFCTagID(tfcp);
			TFCFlags(tfcp2) = TFCFlags(tfcp);
			text->px += TFCWidth(tfcp2);
			tfcp++;
			tfcp2++;
		}
		text->tbuffi = text->size = text->tbuffi - split - skip;
	} else {
		text->tbuffi = text->size = text->tbuffi - split;
	}
#endif
	text->tbuffi = text->size = text->tbuffi - split;

	text->in_line_1 = NO;		/* unless caller sets it otherwise */

	style = text->style = text->next_style;

	tf->lineNodeCount = tf->lineVisibleCount = text->lines;
	tf->firstp = text->insertp;
	tf->offsetp = tf->currentp = tf->firstp;
	tf->current_row = 0;
	tf->current_col_sticky = tf->current_col = 0;
	tf->screen_col_offset = tf->screen_row_offset = 0;
}


/*	Allow vertical blank space
**	--------------------------
*/
PRIVATE void blank_lines ARGS2(HText *,text, int,newlines)
{
	TFLineNode *lnp = text->currentp;

	if (text->tbuffi == 0) {
		if (lnp) {
			if (lnp->length == 0) {
				do {
					if (newlines == 0) break;
					newlines--;
					lnp = lnp->prev;
				} while (lnp && (lnp->length == 0));
			} else {
				if (newlines == 0) newlines++;
			}
		}
	} else {
		newlines++;
	}

	while (newlines--) new_line(text);
	text->in_line_1 = YES;
}


/* New paragraph in current style
 * See also: setStyle.
 */
PUBLIC void HText_appendParagraph ARGS1(HText *,text)
{
	int after = text->style->spaceAfter;
	int before = text->style->spaceBefore;
	int i = after>before ? after : before;

/*	if (!before && !after) blank_lines(text, 1);
	else if (i) blank_lines(text, i);
*/
	blank_lines(text, i);
}

/* Set Style
 *
 */
PUBLIC void HText_setStyle ARGS2(HText *,text, HTStyle *,style)
{
#ifdef NOTNOTNOTNOTNOTNOTNONTONTONTO
	int after, before;
	if (!style) {
	  	return;
	}

	after = text->style->spaceAfter;
	before = style->spaceBefore;

	if (TRACE) {
		fprintf(stderr, 
			"HTML: Changing to style %s (%d). fontID=%d\n", 
			style->name, style->font, text->fontID);
	}

	/*	if (text->style != style) {*/
	if (1) {

		if (style->font == HT_NORMAL) {
			text->flags = NULL;
			text->fontID = fontID_normal;
		} else if (style->font & HT_BOLD) {
			text->fontID = fontID_bold;
		} else if (style->font & HT_NORMAL_LARGE) {
			text->fontID = fontID_normal_large;
		} else if (style->font & HT_BOLD_LARGE) {
			text->fontID = fontID_bold_large;
		} else if (style->font & HT_NORMAL_LARGEST) {
			text->fontID = fontID_normal_largest;
		} else if (style->font & HT_BOLD_LARGEST) {
			text->fontID = fontID_bold_largest;
		} else if (style->font & HT_FIXED) {
			text->fontID = fontID_fixed;
		}
		if (style->font & HT_INVERSE) {
			text->flags |= MASK_REVERSE;
		}
		if (style->font & HT_UNDERLINE) {
			text->flags |= MASK_UNDER;
		}
		if (text->maxFontHeight < FontMaxHeight(text->fontID)) {
			text->maxFontHeight = FontMaxHeight(text->fontID);
		}
		if (text->maxFontDescent < FontDescent(text->fontID)) {
			text->maxFontDescent = FontDescent(text->fontID);
		}
		text->style = text->next_style;
		text->next_style = style;
		text->next_fontID = text->fontID;

		if (TRACE) {
			fprintf(stderr, 
				"HTML: New fontID=%d    l=%f r=%f\n", 
				text->fontID, style->leftIndent, style->rightIndent);
		}
	}
	blank_lines (text, after>before ? after : before);
#endif
}

#define VERBOSE_APPENDCHARACTER___
/*	Append a character to the text object
**	-------------------------------------
*/
PUBLIC void NOT_HText_appendCharacter ARGS2(HText *,text, char,ch)
{
	printf("- %c\n", ch);
}

PUBLIC void HText_appendCharacter ARGS2(HText *,text, char,ch)
{
	HTStyle *style;
	HTFont font;
	TFChar *tfcp;
	int indent;
	int charWidth;
	char c;

#ifdef VERBOSE_APPENDCHARACTER
  printf("APPENDCHARACTER: %c      px=%d  fontID=%d\n", 
		ch, text->px, text->fontID);
#endif

	if (!text) return;

	style = text->style;
	font = style->font;
	indent = text->in_line_1 ? style->indent1st : style->leftIndent;

	if (ch == '\r') {
		return; /* badness? */
	}
	if (ch == '\n') {
		new_line(text);
		return;
	}
	if (ch == '\t') {

		HTTabStop * tab;
		int target;	/* Where to tab to, in pixel value */
		int here = indent + text->px;
		int limit = text->tfstruct->width - style->rightIndent;

		if (style->tabs) {
			int i, spaceWidth = FontWidths(text->fontID)['n'];
			target = 0;

			if (spaceWidth <= 0) {
				fprintf(stderr, 
				    "HTML: S.O.S. font width of 'n' == 0!\n");
				spaceWidth = FontWidths(text->fontID)['h'];
			}

			for (i = 0; ; i++) {
				target = style->tabs[i].position * spaceWidth
					+ indent;
#ifdef VERBOSE_APPENDCHARACTER
printf("[%d] here=%d, target=%d\n", i, here, target);
#endif

				if (target > here) break;
				if (target == -1) {
					new_line(text);
					return;
				}
			}
		} else if (style->leftIndent) {   /* Use 2nd indent */
			if (here >= style->leftIndent) {
				new_line(text); /* wrap */
				return;
			} else {
			        target = style->leftIndent;
			}
		} else {
			int tabWidth = FontWidths(text->fontID)['n'] * 9;
			target = here + tabWidth - (here % tabWidth);
		}

		if (target > limit) {
			new_line(text);
			return;
		} else {
			TFChar spacetfc;
			int i, count = 0;
			int spaceWidth;

			spaceWidth = FontWidths(text->fontID)[' '];
			if (spaceWidth <= 0) {
				fprintf(stderr, 
					"HTML: Eh? How is it that I think the font width of ' ' is 0? Please inform Pei about this insanity.\n");
				spaceWidth = FontWidths(text->fontID)['n'];
			}

			TFCChar(&spacetfc) = ' ';
			TFCFontID(&spacetfc) = text->fontID;
			TFCTagID(&spacetfc) = 0;
			TFCFlags(&spacetfc) = text->flags;

			/* Can split here */
			text->permissible_split = text->size;

			tfcp = text->tbuff + text->tbuffi;

			for (i = target - here - spaceWidth; i > 0; 
				i -= spaceWidth) {
				/* Put character into line */
				bcopy(&spacetfc, tfcp, 
					sizeof(struct TFChar));
				tfcp++;
				count++;
			}
			text->px += spaceWidth * count;
			text->size += count;
			text->tbuffi += count;
			if (text->tbuffi > TBUFFSIZE) {
				/* badnedd... */
			}
			return;
		}
	}
	
	if (ch == ' ') {
		text->permissible_split = text->size;	/* Can split here */
	}

 	tfcp = text->tbuff + text->tbuffi;

	TFCChar(tfcp) = ch;
	TFCFontID(tfcp) = text->fontID;
	TFCTagID(tfcp) = 0;
	TFCFlags(tfcp) = text->flags;
	charWidth = FontWidths(text->fontID)[ch];
	TFCWidth(tfcp) = charWidth;

	text->px += charWidth;
	text->size++;

	if (++(text->tbuffi) > TBUFFSIZE) {
		/* error. lossing info... */
		return;
	}
	TFCFlags(tfcp + 1) = text->flags;
}


/*	Append a (hidden) character to the text object
**	-------------------------------------
*/
PUBLIC void HText_appendHiddenCharacter ARGS2(HText *,text, char,ch)
{
	printf("hidden- %c\n", ch);
/*
	hbuff[hbuffi++] = ch;
	if (hbuffi > HBUFFSIZE) {
		printf("html: tbuffi exceeded HBUFFSIZE\n");
	}
	inHiddenState = 0;
*/
	return;
}

/*	Anchor handling
**	---------------
*/
PUBLIC void HText_beginAnchor ARGS2(HText *,text, HTChildAnchor *,anc)
{
printf("HText_beginAnchor \n");
#ifdef NOTUSED
	TextAnchor * a = (TextAnchor *) malloc(sizeof(*a));

	if (!a) {
		fprintf(stderr, "malloc failed\n");
		return;
	}

	a->start = text->chars + text->size;/* ?? */
	a->extent = 0;
	a->beginLine = text->lines;

	if (text->last_anchor) {
		text->last_anchor->next = a;
	} else {
		text->first_anchor = a;
	}
	a->next = 0;
	a->anchor = anc;
	text->last_anchor = a;
	
	if (HTAnchor_followMainLink((HTAnchor*)anc)) {
		a->number = ++(text->last_anchor_number);
	} else {
		a->number = 0;
	}
	if (anc->mainLink.dest) {
		text->flags |= MASK_BUTTON;
		if (lookAndFeel != 1) 
			HText_appendCharacter(text, HT_NON_BREAK_SPACE);
	}
	anchorPatchIdx = 0;
	anchorInP = 1;
#endif
}

PUBLIC void HText_endAnchor ARGS1(HText *,text)
{
printf("HText_endAnchor \n");
#ifdef UNUSED
	TextAnchor * a = text->last_anchor;
	char *cp, *tattr;
	char marker[100];
	HTLink *htlink;

	htlink = (HTLink*)((HTChildAnchor*)(a->anchor));
	tattr = HTAnchor_address(htlink->dest);

	/*
	 * embed string
	 */
	if (tattr) {
		TFChar *tfcp;

		cp = malloc(sizeof(char) * (strlen(tattr) + 1));
		if (!cp) {
			fprintf(stderr, "malloc failed\n");
			anchorInP = 0;
			return;
		}
		strcpy(cp, tattr);

		HText_appendCharacter(text, HT_NON_BREAK_SPACE);
		tfcp = text->tbuff + text->tbuffi - 1;

		if (text->tagID < TAGINFO_SIZE) {
			text->tagID++;
			/*printf("\\\\\\tagID = %d [%s]\n", text->tagID, cp);*/
			TFCTagID(tfcp) = text->tagID;
			text->tbuffTagInfo[text->tagID].info = cp;
		} else {
			printf("Error: exceeded tag limit(%d).\n",
				TAGINFO_SIZE);
		}
		text->flags &= ~MASK_BUTTON;

		if (anchorPatchIdx >= ANCHOR_LINES_SPAN) {
			fprintf(stderr, "Warning: certain anchor span more lines than the software can deal with (lower part of the lines have no links).\n");
		}
		while (anchorPatchIdx-- > 0) 
			*(anchorPatch[anchorPatchIdx]) = (int)cp;

		if (lookAndFeel != 1)
			HText_appendCharacter(text, HT_NON_BREAK_SPACE);
	}
	a->endLine = text->lines;
/*
	printf("ANCHOR #=%d tag=``%s'' delim:%d %d\n", 
		a->number, a->anchor->tag, a->beginLine, a->endLine);
*/
	anchorInP = 0;
#endif
}

PUBLIC void HText_appendText ARGS2(HText *,text, char *,str)
{
	char *p;
	for (p = str; *p; p++) HText_appendCharacter(text, *p);
}

PUBLIC void HText_endAppend ARGS1(HText *,text)
{
/*	new_line(text);*/
  printf("---\n");
}

#define WANT_		1
#define WANT_txtDisp	2
#define WANT_txtLabel	3
#define WANT_txtButton	4
#define WANT_HTML	5
#define WANT_XBM	6
#define WANT_XPM	7
#define WANT_menu	8

PUBLIC void HText_endHiddenAppend ARGS1(HText *,text)
{
}

/* 	Dump diagnostics to stderr
*/
PUBLIC void HText_dump ARGS1(HText *,text)
{
	fprintf(stderr, "HText: Dump called\n");
}
	

/*	Return the anchor associated with this node
*/
PUBLIC HTParentAnchor * HText_nodeAnchor ARGS1(HText *,text)
{
return NULL;
/*	return text->node_anchor;
*/
}

/*	GridText specials
**	=================
**	Return the anchor with index N
**
**	The index corresponds to the number we print in the anchor.
*/
PUBLIC HTChildAnchor * HText_childNumber ARGS2(HText *,text, int,number)
{
#ifdef UNUSED
printf("HTChildAnchor \n");
	TextAnchor * a;
	for (a = text->first_anchor; a; a = a->next) {
		if (a->number == number) return a->anchor;
	}
	return (HTChildAnchor *)0;	/* Fail */
#endif
}

/*		Browsing functions
**		==================
*/
PUBLIC BOOL HText_select ARGS1(HText *,text)
{
/*
	HTMainText = text;
	return YES;
*/
	return NO;
}

PUBLIC BOOL HText_selectAnchor ARGS2(HText *,text, HTChildAnchor *,anchor)
{
	return NO;
#ifdef UNUSED
	TextAnchor * a;

	for(a=text->first_anchor; a; a=a->next) {
        	if (a->anchor == anchor) break;
	}
	if (!a) {
	        if (TRACE) fprintf(stderr, 
				"HText: No such anchor in this text!\n");
	        return NO;
	}
	{
		int l = 1;
		if (1) fprintf(stderr,
			"HText: Selecting anchor [%d] at character %d, line %d\n",
			a->number, a->start, l);

	}
	return YES;
#endif
} 

/*		Editing functions		- NOT IMPLEMENTED
**		=================
**
**	These are called from the application. There are many more functions
**	not included here from the orginal text object.
*/

/*	Style handling:
**	Apply this style to the selection
*/
PUBLIC void HText_applyStyle ARGS2(HText *, me, HTStyle *,style)
{
/*	printf("HText_applyStyle.\n");*/
}

/*	Update all text with changed style.
*/
PUBLIC void HText_updateStyle ARGS2(HText *, me, HTStyle *,style)
{
/*	printf("HText_updateStyle.\n");*/
}

/*	Return style of  selection
*/
PUBLIC HTStyle * HText_selectionStyle ARGS2(
	HText *,me,
	HTStyleSheet *,sheet)
{
/*	printf("HText_selectionStyle.\n");*/
	return 0;
}

/*	Paste in styled text
*/
PUBLIC void HText_replaceSel ARGS3(
	HText *,me,
	char *,aString, 
	HTStyle *,aStyle)
{
	printf("HText_replaceSel.\n");
}

/*	Apply this style to the selection and all similarly formatted text
**	(style recovery only)
*/
PUBLIC void HTextApplyToSimilar ARGS2(HText *,me, HTStyle *,style)
{
/*	printf("HTextApplyToSimilar.\n");*/
}
 
/*	Select the first unstyled run.
**	(style recovery only)
*/
PUBLIC void HTextSelectUnstyled ARGS2(HText *,me, HTStyleSheet *,sheet)
{
/*	printf("HTextSelectUnstyled.\n");*/
}

/*	Anchor handling:
*/
PUBLIC void HText_unlinkSelection ARGS1(HText *,me)
{
/*	printf("HText_unlinkSelection.\n");*/
}

PUBLIC HTAnchor * HText_referenceSelected ARGS1(HText *,me)
{
/*	printf("HText_referenceSelected.\n");*/
	return 0;   
}

PUBLIC HTParentAnchor *	HText_referenceAll ARGS1(HText *,me)
{
/*	printf("HText_referenceAll.\n");*/
	return HTMainAnchor;
}

PUBLIC HTAnchor * HText_linkSelTo ARGS2(HText *,me, HTAnchor *,anchor)
{
/*	printf("HText_linkSelTo.\n");*/
	return 0;
}

/*
 *
 * initializes and sets up a text field structure 
 */
TFStruct *html_setUpTFStruct(self, text)
	VObj *self;
	char *text;
{
	TFStruct *tf = GET__TFStruct(self);

	if (!tf) {
		tf = (TFStruct*)Vmalloc(GET__memoryGroup(self),
					sizeof(struct TFStruct));
		if (!tf) {
			fprintf(stderr, "malloc failed\n");
			return 0;
		}
		SET__TFStruct(self, tf);

		tf->firstp =
		tf->lastp =
		tf->offsetp = 
		tf->currentp = NULL;

		tf->current_col_sticky = tf->current_col = 0;
		tf->current_row = 0;

		tf->csr_px_sticky = tf->csr_px = 0;
		tf->csr_py = 0;

		tf->screen_col_offset =
		tf->screen_row_offset = 0;

		tf->lineNodeCount = 0;
		tf->lineVisibleCount = 0;

		tf->num_of_lines = 0;		/* in field view ? */

		tf->w = NULL;
		tf->xUL = tf->yUL =
		tf->xLR = tf->yLR = 
		tf->width = tf->height = 0;

		tf->esc_toggle = 0;
		tf->bufferUsed = 0;
		tf->self = self;
		tf->fontID = 0;
		tf->currentFontID = 0;

		tf->cursorTimeInfo = 0;
		tf->cursorIsVisible = 0;
/*		tf->cursorBlinkDelay = 250;*/
		tf->cursorBlinkDelay = -1; /* means don't blink */
		tf->highLiteFrom_cx = -1;
		tf->highLiteFrom_cy = -1;
		tf->highLiteTo_cx = -1;
		tf->highLiteTo_cy = -1;

		tf->isRenderAble = 0;

		tf->building_maxFontHeight = 0;
		tf->building_maxFontDescent = 0;
		tf->building_vspan = 0;

		tf->editableP = 0;
		tf->mg = GET__memoryGroup(self);
	}
	tf->wrap = GET_wrap(self);

	html_updateTFStruct(self, text);

	return tf;
}

TFStruct *html_updateTFStruct(self, address)
	VObj *self;
	char *address;
{
	TFStruct *tf = GET__TFStruct(self);
	int fontID = GET__font(self);
	int newWidth, newHeight;
	int geometryChanged = 0;
	char *title = NULL;
	char *simpleAddress;
	char *anchorSearch;
	HText *htext;

	if (!tf || !address) return NULL;
	if (address[0] == '\0') return NULL;

	/* 
	 * update geometry of the field
	 */
	/* to allow space between border and characters */
	newWidth = GET_width(self);
	newHeight = GET_height(self);
	if (newWidth <= 1 && newHeight <= 1) return NULL;
	tf->xUL = 1;
	tf->yUL = 1;
	tf->xLR = GET_width(self) - 2;
	tf->yLR = GET_height(self) - 2;

	/* this could be smarter */
	if (newWidth != tf->width || newHeight != tf->height) {
		tf->width = newWidth;
		tf->height = newHeight;
		tf->csr_px = tf->xUL;
		tf->csr_py = tf->yUL;
		geometryChanged = 1;
	}
	tf->w = GET_window(self);
	tf->wrap = GET_wrap(self);
	tf->fontID = tf->currentFontID = fontID;

	tf->isRenderAble = (tf->w && newWidth > 2 && newHeight > 2) ? 1 : 0;

	if (newWidth > 2 && newHeight > 2) {

		if (!(htext = html_loadDocument(self, address, &title, 
					&simpleAddress, &anchorSearch))) {
			return NULL;
		}
		tf = htext->tfstruct;

		htext->simpleAddress = simpleAddress;

 		if (htext->simpleAddress) {
			SET_HTMLAddress(self, htext->simpleAddress);
		} else {
			return NULL; /* eh? */
		}
		SET_HTMLAnchor(self, (anchorSearch ? anchorSearch : ""));
/*		SET_HTMLTitle(self, (title ? title : ""));*/
		SET_HTMLTitle(self, htext->title);
		SET_HTMLIsIndex(self, htext->isIndex);
		SET_HTMLSource(self, htext->htmlSrc);

		if (geometryChanged && tf->isRenderAble) renderTF(tf);
	}
	SET__TFStruct(self, tf);

	return tf;
}

/*
 * return new htext, if different document
 */
HText *html_loadDocument(self, address, title, simpleAddress, anchorSearch)
	VObj *self;
	char *address;
	char **title;
	char **simpleAddress;
	char **anchorSearch;
{
	TFLineNode *currentp = NULL;
	TFLineNode *oldp;
	TFChar tbuff[TBUFFSIZE];
	int tbuffi = 0, sameDoc = 0;
	int i, j;
	int status = 0;
	HText *htext;
	char anchorInfo[64];
	TFStruct *oldtf, *tf = NULL;
	char *oldHTMLSrc;
	extern SGMLBuildInfo SBI;
	SGMLBuildInfoState *bstate;
	int loadRetStatus;

	oldtf = GET__TFStruct(self);
	oldp = oldtf->firstp;

	trimEdgeSpaces(address);

	/* get full address, without anchor */
	*simpleAddress = saveString(HTParse(address, current_addr,
					    PARSE_ACCESS | PARSE_HOST | 
					    PARSE_PATH | PARSE_PUNCTUATION));
	HTSimplify(*simpleAddress);

	if (current_addr) free(current_addr);
	current_addr = saveString(*simpleAddress);

	/* extract anchor info, if any */
	*anchorSearch = "";
	for (i = 0; address[i]; i++) {
		if (address[i] == '#') {
			strcpy(anchorInfo, address + i + 1);
			*anchorSearch = saveString(anchorInfo);
			break;
		}
	}

	if (htext = html_findHTextByAddress(self, *simpleAddress)) {
		tf = htext->tfstruct;
		status = 1;

		/* New data is going to be written on the existing 
		 * htext struct, so free the data in the current struct
		 */
/*		if (tf->firstp) {
			freeNodeLines(tf);
			tf->lineNodeCount = 0;
			tf->lineVisibleCount = 0;
		}
		if (htext->htmlSrc) {
			free(htext->htmlSrc);
			htext->htmlSrc = NULL;
		}
*/
	}
	if (status == 0) {

		tf = (TFStruct*)Vmalloc(GET__memoryGroup(self),
				       sizeof(struct TFStruct));
		if (!tf) {
			perror("malloc");
			return 0;
		}
		if (oldtf) bcopy(oldtf, tf, sizeof(struct TFStruct));

		if (!html_updateHTextTFStruct(oldtf, tf)) {
/*harmless?		fprintf(stderr, 
			"Error: (2) html_updateHTextTFStruct() failed\n");
*/
		}
		if (oldtf) {
			if (oldtf->firstp) freeNodeLines(oldtf);
			Vfree(GET__memoryGroup(self), oldtf);
			SET__TFStruct(self, NULL);

			/*clear cache, b/c oldtf may be the currently cached*/
			updateEStrUser(NULL);
		}

/*
		HTAnchor_delete(HTAnchor_findAddress(*simpleAddress));
*/
/*HTAnchor_parent*/

		/* set this global (oh well) for HText_new() to pick up */
		new_tfstruct = tf;

		tf->building_maxFontHeight = 0;
		tf->building_maxFontDescent = 0;
		tf->building_vspan = 0;


		/* prepare SGMLBuildInfo structure
		 */
		SBI.stacki = 0;
		SBI.endP = 0;
		SBI.caller = self;
		
		bstate = &SBI.stack[SBI.stacki];
		bstate->tmi = NULL;
		bstate->dataBuffIdx_localStart = 0;
		bstate->tag = NULL;
		bstate->parent = NULL;
		bstate->obj = self;
		bstate->width = bstate->sub_width = GET_width(self);
		bstate->y = bstate->sub_y = 0;
		bstate->height = bstate->sub_height = 0;

		HTOutputSource = NO;
		HTOutputStream = NULL;
		HTOutputFormat = NULL;

		srcBuff[0] = '\0';
		srcBuffi = 0;

		http_method = HTTP_METHOD_GET;
		http_dataToPost = NULL;
		loadRetStatus = HTLoadAbsolute(*simpleAddress);

		if (loadRetStatus) {
			HTHistory_record((HTAnchor *)HTMainAnchor);
			htext = HTMainText;

			if (!htext) {
			  if (verbose)
				fprintf(stderr, 
					"html: Failed to access ``%s''\n", 
					*simpleAddress);
			  Vfree(GET__memoryGroup(self), tf);
			  SET__TFStruct(self, NULL);
			  return NULL;
			}

			htext->title = 
			    saveString(HTAnchor_title(htext->node_anchor));

			htext->expandedAddress = 
				saveString(htext->node_anchor->address);
			htext->simpleAddress = *simpleAddress;
			htext->isIndex = 
				HTAnchor_isIndex(htext->node_anchor) ? 1 : 0;
/*
			if (htext->htmlSrc) {
				printf("freeing HTMLSrc\n");
				free(htext->htmlSrc);
			}
*/
			htext->htmlSrc = saveString(srcBuff);
			htext->htmlSrcSize = srcBuffi;

			/* make sure can scroll to see last line text */
			new_line(htext); new_line(htext);

			status = 2;
			if (verbose) 
				fprintf(stderr, 
					"html: Accessed ``%s''\n",
					*simpleAddress);
		} else {
			if (verbose)
				fprintf(stderr, 
					"html: Failed to access ``%s''\n", 
					*simpleAddress);
			Vfree(GET__memoryGroup(self), tf);
			SET__TFStruct(self, NULL);
			return NULL;
		}
		htext->tfstruct = tf;
		*title = htext->title;
	} else {
		*title = NULL;
	}

	updateEStrUser(NULL); /* purge cache */

/*	SET__TFStruct(self, tf);*/

	if (status > 0) {
		if (status > 1) {
			/* free old data.
			 */
/*
			if (status == 2) {
			oldp = tf->firstp;
			if (oldp && (oldp != tf->firstp)) {
				TFLineNode *lp = oldp;

	printf("freeing data for obj: %s\n", GET_name(tf->self));
				while (lp) {
					oldp = lp;
					lp = lp->next;
					if (oldp->linep) {
	printf("freeing >>");
	dumpTFCArray(DUMP_CHAR, oldp->linep, oldp->tagInfo);
	printf("<<\n");
						free(oldp->linep);
						free(oldp);
					}
				}
			}
			}
*/
			tf->lineNodeCount = tf->lineVisibleCount = 
				htext->lines;
			tf->firstp = htext->insertp;
			tf->offsetp = tf->currentp = tf->firstp;
			tf->current_row = 0;
			tf->current_col_sticky = tf->current_col = 0;
			tf->screen_col_offset = tf->screen_row_offset = 0;
		}
		if ((*anchorSearch)[0]) {
			tf->lineNodeCount = tf->lineVisibleCount = 
				htext->lines;
			tf->firstp = htext->insertp;
			tf->offsetp = tf->currentp = tf->firstp;
			tf->current_row = 0;
			tf->current_col_sticky = tf->current_col = 0;
			tf->screen_col_offset = tf->screen_row_offset = 0;
			moveToSelectedAnchor(htext, *anchorSearch);
		}
		return htext;
	}
	return NULL;
}

/*
 * return temporary file name
 */
char *html_fetchDocument(self, address, simpleAddress, anchorSearch, fp)
	VObj *self;
	char *address;
	char **simpleAddress;
	char **anchorSearch;
	FILE *fp;
{
	int i;
	char *cp, anchorInfo[64];
	int stat;
	HTStream* HTOutputStream_save = HTOutputStream;
	HTFormat HTOutputFormat_save = HTOutputFormat;
	BOOL HTOutputSource_save = HTOutputSource;
	char *current_addr_save = current_addr;

	trimEdgeSpaces(address);

	/* get full address, without anchor */
	*simpleAddress = saveString(HTParse(address, current_addr,
					    PARSE_ACCESS | PARSE_HOST | 
					    PARSE_PATH | PARSE_PUNCTUATION));
	HTSimplify(*simpleAddress);

/*	if (current_addr) free(current_addr);
	current_addr = saveString(*simpleAddress);
*/
	/* extract anchor info, if any */
	*anchorSearch = "";
	for (i = 0, cp = address; *cp; i++) {
		if (*cp++ == '#') {
			strcpy(anchorInfo, address + i + 1);
			*anchorSearch = saveString(anchorInfo);
			break;
		}
	}
	HTOutputStream = HTFWriter_new(fp);
	HTOutputFormat = WWW_SOURCE;
	HTOutputSource = YES;

	http_method = HTTP_METHOD_GET;
	http_dataToPost = NULL;
	HTPushInputBuffer();
	stat = HTLoadAbsolute(*simpleAddress);
	HTPopInputBuffer();
	http_method = HTTP_METHOD_GET;
	http_dataToPost = NULL;

	HTOutputStream = HTOutputStream_save;
	HTOutputFormat = HTOutputFormat_save;
	HTOutputSource = HTOutputSource_save;

	return stat;
}

/***************
 * OBSOLETE CODE
 */
int moveToSelectedAnchor ARGS2(HText *,text, char *, anchorInfo)
{
	int i;
	TFStruct *tf = text->tfstruct;
	TextAnchor *a;

	if (!tf) return 0;
	for(a = text->first_anchor; a; a = a->next) {

/*		printf("[%d]anchor tag = ``%s''.\n", 
			a->number, a->anchor->tag);
*/
		if (a->anchor) 
			if (a->anchor->tag)
				if (!strcmp((a->anchor->tag), anchorInfo)) {
					int beginLine = a->beginLine;

					for (i = 0; i < beginLine; i++) {
						if (!tf->offsetp) break;
						tf->offsetp = 
							tf->offsetp->next;
						tf->current_row++;
						tf->screen_row_offset++;
					}
					tf->currentp = tf->offsetp;
					return beginLine;
				}
	}
	return -1;
}

HText *html_findHTextByAddress(self, address)
	VObj *self;
	char *address;
{
	HTList *list;
	HText *htext;

	trimEdgeSpaces(address);

	for (list = loaded_texts; list; list = list->next) {
		htext = (HText*)list->object;
		if (htext)
		    if (htext->tfstruct)
			if (htext->tfstruct->self == self)
				if (htext->simpleAddress) {
					if (verbose) 
						fprintf(stderr, 
							"html: FTBA %s, %s\n", 
							htext->simpleAddress, 
							address);
					if (!strcmp(htext->simpleAddress, 
							address)) {
						return list->object;
					}
				}
	} 
	return NULL;
}

HText *html_updateHTextTFStruct(oldtf, newtf)
	TFStruct *oldtf;
	TFStruct *newtf;
{
	HTList *list;
	HText *htext;

	for (list = loaded_texts; list; list = list->next) {
		htext = (HText*)list->object;
		if (htext) {
			if (htext->tfstruct == oldtf) {
				htext->tfstruct = newtf;
				return htext;
			}
		}
	}
	return NULL;
}

/*
 * given address, return associated title
 */
char *html_get_title(self, address)
	VObj *self;
	char *address;
{
	HText *text;

	trimEdgeSpaces(address);

	if (text = html_findHTextByAddress(self, address)) {
		if (text->title) return text->title;
	}
	return NULL;
}

/*
 * given address, return html source text
 */
char *html_get_src(address)
	char *address;
{
	return NULL;	
}

/*
 * for force updates
 */
int html_deleteDoc(self, address)
	VObj *self;
	char *address;
{
	HText *text;

	if (text = html_findHTextByAddress(self, address)) {
		if (HTList_removeObject(loaded_texts, text) == YES) return 1;
	}
	return 0;
}

int html_setCacheLimit(n)
	int n;
{
	loaded_limit = n;
	return n;
}

int html_clearCache(self)
	VObj *self;
{
	HTList *list;
	HText *htext;
	VObj *obj;
	TFStruct *tf;

	if (self) {
		/*
		 * Purges text for specified object
		 */
		HTList *plist = NULL;

		for (list = loaded_texts; list; list = list->next) {
			htext = (HText*)list->object;
			if (htext) {
				obj = htext->tfstruct->self;
				if (self == obj) {
					printf("1Purging object text: %s\n", 
						GET_name(obj));
					printf("htext=%x\n", htext);
					tf = GET__TFStruct(obj);
					if (tf->firstp) {
						freeNodeLines(tf);
						tf->lineNodeCount = 0;
						tf->lineVisibleCount = 0;
					}
					if (htext->htmlSrc) {
						free(htext->htmlSrc);
						htext->htmlSrc = NULL;
					}
					free(htext);
					if (plist) plist->next = list->next;
					else loaded_texts = list->next;

					updateEStrUser(NULL); /* purge cache */
/*					SET__TFStruct(self, NULL);
*/

/*					HTAnchor_purgeCache();*/

					return 1;
				}
			}
			plist = list;
		} 
	} else {
		/*
		 * Purges all texts
		 */
		for (list = loaded_texts; list; list = list->next) {
			htext = (HText*)list->object;
			if (htext) {
				obj = htext->tfstruct->self;
				if (obj && validObjectP(obj)) {
					printf("Purging object text: %s\n", 
						GET_name(obj));
					tf = GET__TFStruct(obj);
					if (tf->firstp) {
						freeNodeLines(tf);
						tf->lineNodeCount = 0;
						tf->lineVisibleCount = 0;
					}
					if (htext->htmlSrc) {
						free(htext->htmlSrc);
						htext->htmlSrc = NULL;
					}
					/*free(htext);*/
				}
			}
		} 
		loaded_texts = NULL;
/*		HTAnchor_purgeCache();*/
	}
	return 1;
}

#ifdef OBSOLETE
int setHTMLStyle()
{
	Attr *attrp;
	char *tagName, *styleList, *cp;
	Packet *pk = borrowPacket();
	Packet *objpk = borrowPacket();
	VObj *obj;
	char font[100], align[32];
	int i, wrap, free, first, left, right, before, after;
	HTStyle *style;
	extern int numberOfFontIDs;


	objpk->info.s = "res.HTMLStyle"; /* make sure this isn't modified*/
	objpk->type = PKT_STR;
	objpk->canFree = 0;
 	obj = PkInfo2Obj(objpk);

	if (!obj) {
		if (verbose) 
			fprintf(stderr, 
				"object ``res.HTMLStyle'' not found. Using default style.\n");
		returnPackets(2);
		return 1;
	}

	if (verbose) fprintf(stderr, "Processing HTML Style sheet\n");

	attrp = GET__varList(obj);
	for (; attrp; attrp = attrp->next) {
		tagName = (char*)symID2Str->get(symID2Str, attrp->id)->val;
		pk = (Packet*)(attrp->val);
		if (pk) {
		  styleList = PkInfo2Str(pk);
		  if (styleList[0] != '\0') {
			if (verbose)
				fprintf(stderr, 
				"tagName=%s, styleList=``%s''\n", 
				tagName, styleList);
			sscanf(styleList, 
				"%s %s %d %d %d %d %d %d %d",
				font, align, &wrap, &free, &first, 
				&left, &right, &before, &after);

			for (style = styleSheet->styles; style; 
				style = style->next) {
				if (!STRCMP(tagName, style->SGMLTag)) {

					for (i = 0; i < numberOfFontIDs; i++) {
						if (!STRCMP(FontRef(i), font))
							style->font = i;
					}

					if (!STRCMP(align, "left"))
						style->alignment = HT_LEFT;
					else if (!STRCMP(align, "right"))
						style->alignment = HT_RIGHT;
					else if (!STRCMP(align, "center"))
						style->alignment = HT_CENTER;
					else 
						fprintf(stderr, 
					     "unknown alignment style: %s\n", 
							align);

					style->wordWrap = wrap;
					style->freeFormat = free;
					style->indent1st = first;
					style->leftIndent = left;
					style->rightIndent = right;
					style->spaceBefore = before;
					style->spaceAfter = after;
				}
			}
		  }
		}
	}
	returnPackets(2);
	return 1;
}
#endif

char *html_headerInfo(self)
	VObj *self;
{
	char *cp;
	return NULL;
}

char *html_WWWNameOfFile(name)
	char *name;
{
	return (char*)WWW_nameOfFile(name);
}

/*	File buffering
**	--------------
**
**	The input file is read using the macro which can read from
**	a socket or a file.
**	The input buffer size, if large will give greater efficiency and
**	release the server faster, and if small will save space on PCs etc.
*/
#define INPUT_BUFFER_SIZE 8096		/* Tradeoff */
PRIVATE char input_buffer[INPUT_BUFFER_SIZE];
PRIVATE char * input_pointer;
PRIVATE char * input_limit;
PRIVATE int input_file_number;

/*	Read a file given filen_umber
**	------------
*/
PUBLIC void HTJustRead ARGS3(
	HTFormat,format,
	HTParentAnchor *,anchor,  	/* ignored */
	int,file_number)
{
  	FILE *fp;
	char tfn[200];

	input_file_number = file_number;
	input_pointer = input_limit = input_buffer;

	tempFileFormat = format;

	sprintf(tfn, "%s%ld", tempFileNamePrefix, tempFileNameIDCounter++);
	tempFileName = saveString(tfn);

	fp = fopen(tempFileName, "w");
	if (fp) {
		char cc;

		for(;;) {
			if (input_pointer >= input_limit) {
				tempFileStat = NETREAD(input_file_number, 
					     input_buffer, INPUT_BUFFER_SIZE);
				if (tempFileStat <= 0) {
					if (tempFileStat < 0) {
						if (TRACE) fprintf(stderr,
				    "HTJustRead: File read error %d\n",
						tempFileStat);
					}
					break;
				}
				input_pointer = input_buffer;
				input_limit = input_buffer + tempFileStat;
			}
			cc = *input_pointer++;
			fputc(cc, fp);
		}
		fclose(fp);
		tempFileStat = 1;
	}
	tempFileStat = 0;
}

