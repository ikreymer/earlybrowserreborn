/*		Structured stream to Rich hypertext converter
**		============================================
**
**	This generates of a hypertext object.  It converts from the
**	structured stream interface fro FOSI events into the style-
**	oriented iunterface of the HText.h interface.  This module is
**	only used in clients and shouldnot be linked into servers.
**
**	Override this module is making a new GUI browser.
**
*/
#include "FOSI.h"

#define CAREFUL		/* Check nesting here notreally necessary */

#include <ctype.h>
#include <stdio.h>

#include "HTAtom.h"
#include "HTChunk.h"
#include "HText.h"
#include "HTStyle.h"
#include "FOSIDTD.h"

extern HTStyleSheet * styleSheet;	/* Application-wide */

/*	Module-wide style cache
*/
PRIVATE int 		got_styles = 0;
PRIVATE HTStyle *styles[FOSI_ELEMENTS];
PRIVATE HTStyle *default_style;

#define MAJORBUFF 20000
char majorBuff[MAJORBUFF];
int majorBuffi;

/*		FOSI Object
**		-----------
*/
#define MAX_NESTING 20		/* Should be checked by parser */

typedef struct _stack_element {
        HTStyle *	style;
	int		tag_number;
} stack_element;

struct _HTStructured {
    CONST HTStructuredClass * 	isa;
    HTParentAnchor * 		node_anchor;
    HText * 			text;

    HTStream*			target;			/* Output stream */
    HTStreamClass		targetClass;		/* Output routines */

    HTChunk 			title;		/* Grow by 128 */
    
    char *			comment_start;	/* for literate programming */
    char *			comment_end;

    HTTag *			current_tag;
    BOOL			style_change;
    HTStyle *			new_style;
    HTStyle *			old_style;
    BOOL			in_word;  /* Have just had a non-white char */
    stack_element 	stack[MAX_NESTING];
    stack_element 	*sp;		/* Style stack pointer */
};

struct _HTStream {
    CONST HTStreamClass *	isa;
    /* .... */
};

/*		Forward declarations of routines
*/

PRIVATE void actually_set_style PARAMS((HTStructured * this));
PRIVATE void change_style PARAMS((HTStructured * this, HTStyle * style));

/*	Style buffering avoids dummy paragraph begin/ends.
*/
#define UPDATE_STYLE if (this->style_change) { actually_set_style(this); }


#ifdef OLD_CODE
/* The following accented characters are from peter Flynn, curia project */

/* these ifdefs don't solve the problem of a simple terminal emulator
** with a different character set to the client machine. But nothing does,
** except looking at the TERM setting */


        { "ocus" , "&" },       /* for CURIA */
#ifdef IBMPC
        { "aacute" , "\240" },	/* For PC display */
        { "eacute" , "\202" },
        { "iacute" , "\241" },
        { "oacute" , "\242" },
        { "uacute" , "\243" },
        { "Aacute" , "\101" },
        { "Eacute" , "\220" },
        { "Iacute" , "\111" },
        { "Oacute" , "\117" },
        { "Uacute" , "\125" },
#else
        { "aacute" , "\341" },	/* Works for openwindows -- Peter Flynn */
        { "eacute" , "\351" },
        { "iacute" , "\355" },
        { "oacute" , "\363" },
        { "uacute" , "\372" },
        { "Aacute" , "\301" },
        { "Eacute" , "\310" },
        { "Iacute" , "\315" },
        { "Oacute" , "\323" },
        { "Uacute" , "\332" }, 
#endif
	{ 0,	0 }  /* Terminate list */
};
#endif


/* 	Entity values -- for ISO Latin 1 local representation
**
**	This MUST match exactly the table referred to in the DTD!
*/
static char * ISO_Latin1[] = {
  	"\306",	/* capital AE diphthong (ligature) */ 
  	"\301",	/* capital A, acute accent */ 
  	"\302",	/* capital A, circumflex accent */ 
  	"\300",	/* capital A, grave accent */ 
  	"\305",	/* capital A, ring */ 
  	"\303",	/* capital A, tilde */ 
  	"\304",	/* capital A, dieresis or umlaut mark */ 
  	"\307",	/* capital C, cedilla */ 
  	"\320",	/* capital Eth, Icelandic */ 
  	"\311",	/* capital E, acute accent */ 
  	"\312",	/* capital E, circumflex accent */ 
  	"\310",	/* capital E, grave accent */ 
  	"\313",	/* capital E, dieresis or umlaut mark */ 
  	"\315",	/* capital I, acute accent */ 
  	"\316",	/* capital I, circumflex accent */ 
  	"\314",	/* capital I, grave accent */ 
  	"\317",	/* capital I, dieresis or umlaut mark */ 
  	"\321",	/* capital N, tilde */ 
  	"\323",	/* capital O, acute accent */ 
  	"\324",	/* capital O, circumflex accent */ 
  	"\322",	/* capital O, grave accent */ 
  	"\330",	/* capital O, slash */ 
  	"\325",	/* capital O, tilde */ 
  	"\326",	/* capital O, dieresis or umlaut mark */ 
  	"\336",	/* capital THORN, Icelandic */ 
  	"\332",	/* capital U, acute accent */ 
  	"\333",	/* capital U, circumflex accent */ 
  	"\331",	/* capital U, grave accent */ 
  	"\334",	/* capital U, dieresis or umlaut mark */ 
  	"\335",	/* capital Y, acute accent */ 
  	"\341",	/* small a, acute accent */ 
  	"\342",	/* small a, circumflex accent */ 
  	"\346",	/* small ae diphthong (ligature) */ 
  	"\340",	/* small a, grave accent */ 
  	"\046",	/* ampersand */ 
  	"\345",	/* small a, ring */ 
  	"\343",	/* small a, tilde */ 
  	"\344",	/* small a, dieresis or umlaut mark */ 
  	"\347",	/* small c, cedilla */ 
  	"\351",	/* small e, acute accent */ 
  	"\352",	/* small e, circumflex accent */ 
  	"\350",	/* small e, grave accent */ 
  	"\360",	/* small eth, Icelandic */ 
  	"\353",	/* small e, dieresis or umlaut mark */ 
  	"\076",	/* greater than */ 
  	"\355",	/* small i, acute accent */ 
  	"\356",	/* small i, circumflex accent */ 
  	"\354",	/* small i, grave accent */ 
  	"\357",	/* small i, dieresis or umlaut mark */ 
  	"\074",	/* less than */ 
  	"\361",	/* small n, tilde */ 
  	"\363",	/* small o, acute accent */ 
  	"\364",	/* small o, circumflex accent */ 
  	"\362",	/* small o, grave accent */ 
  	"\370",	/* small o, slash */ 
  	"\365",	/* small o, tilde */ 
  	"\366",	/* small o, dieresis or umlaut mark */ 
  	"\337",	/* small sharp s, German (sz ligature) */ 
  	"\376",	/* small thorn, Icelandic */ 
  	"\372",	/* small u, acute accent */ 
  	"\373",	/* small u, circumflex accent */ 
  	"\371",	/* small u, grave accent */ 
  	"\374",	/* small u, dieresis or umlaut mark */ 
  	"\375",	/* small y, acute accent */ 
  	"\377",	/* small y, dieresis or umlaut mark */ 
};


/* 	Entity values -- for NeXT local representation
**
**	This MUST match exactly the table referred to in the DTD!
**
*/
static char * NeXTCharacters[] = {
  	"\341",	/* capital AE diphthong (ligature) 	*/ 
  	"\202",	/* capital A, acute accent		*/ 
  	"\203",	/* capital A, circumflex accent 	*/ 
  	"\201",	/* capital A, grave accent 		*/ 
  	"\206",	/* capital A, ring 			*/ 
  	"\204",	/* capital A, tilde 			*/ 
  	"\205",	/* capital A, dieresis or umlaut mark	*/ 
  	"\207",	/* capital C, cedilla 			*/ 
  	"\220",	/* capital Eth, Icelandic 		*/ 
  	"\211",	/* capital E, acute accent 				*/ 
  	"\212",	/* capital E, circumflex accent 			*/ 
  	"\210",	/* capital E, grave accent 				*/ 
  	"\213",	/* capital E, dieresis or umlaut mark 			*/ 
  	"\215",	/* capital I, acute accent 				*/ 
  	"\216",	/* capital I, circumflex accent 	these are	*/ 
  	"\214",	/* capital I, grave accent		ISO -100 hex	*/ 
  	"\217",	/* capital I, dieresis or umlaut mark			*/ 
  	"\221",	/* capital N, tilde 					*/ 
  	"\223",	/* capital O, acute accent 				*/ 
  	"\224",	/* capital O, circumflex accent 			*/ 
  	"\222",	/* capital O, grave accent 				*/ 
  	"\351",	/* capital O, slash 		'cept this */ 
  	"\225",	/* capital O, tilde 					*/ 
  	"\226",	/* capital O, dieresis or umlaut mark			*/ 
  	"\234",	/* capital THORN, Icelandic */ 
  	"\230",	/* capital U, acute accent */ 
  	"\231",	/* capital U, circumflex accent */ 
  	"\227",	/* capital U, grave accent */ 
  	"\232",	/* capital U, dieresis or umlaut mark */ 
  	"\233",	/* capital Y, acute accent */ 
  	"\326",	/* small a, acute accent */ 
  	"\327",	/* small a, circumflex accent */ 
  	"\361",	/* small ae diphthong (ligature) */ 
  	"\325",	/* small a, grave accent */ 
  	"\046",	/* ampersand */ 
  	"\332",	/* small a, ring */ 
  	"\330",	/* small a, tilde */ 
  	"\331",	/* small a, dieresis or umlaut mark */ 
  	"\333",	/* small c, cedilla */ 
  	"\335",	/* small e, acute accent */ 
  	"\336",	/* small e, circumflex accent */ 
  	"\334",	/* small e, grave accent */ 
  	"\346",	/* small eth, Icelandic 	*/ 
  	"\337",	/* small e, dieresis or umlaut mark */ 
  	"\076",	/* greater than */ 
  	"\342",	/* small i, acute accent */ 
  	"\344",	/* small i, circumflex accent */ 
  	"\340",	/* small i, grave accent */ 
  	"\345",	/* small i, dieresis or umlaut mark */ 
  	"\074",	/* less than */ 
  	"\347",	/* small n, tilde */ 
  	"\355",	/* small o, acute accent */ 
  	"\356",	/* small o, circumflex accent */ 
  	"\354",	/* small o, grave accent */ 
  	"\371",	/* small o, slash */ 
  	"\357",	/* small o, tilde */ 
  	"\360",	/* small o, dieresis or umlaut mark */ 
  	"\373",	/* small sharp s, German (sz ligature) */ 
  	"\374",	/* small thorn, Icelandic */ 
  	"\363",	/* small u, acute accent */ 
  	"\364",	/* small u, circumflex accent */ 
  	"\362",	/* small u, grave accent */ 
  	"\366",	/* small u, dieresis or umlaut mark */ 
  	"\367",	/* small y, acute accent */ 
  	"\375",	/* small y, dieresis or umlaut mark */ 
};

/* 	Entity values -- for IBM/PC Code Page 850 (International)
**
**	This MUST match exactly the table referred to in the DTD!
**
*/
/* @@@@@@@@@@@@@@@@@ TBD */



/*		Set character set
**		----------------
*/

PRIVATE char** p_entity_values = ISO_Latin1;	/* Pointer to translation */

PUBLIC void FOSIUseCharacterSet ARGS1(FOSICharacterSet, i)
{
    p_entity_values = (i == FOSI_NEXT_CHARS) ? NeXTCharacters
    					     : ISO_Latin1;
}


/*		Flattening the style structure
**		------------------------------
**
On the NeXT, and on any read-only browser, it is simpler for the text to have
a sequence of styles, rather than a nested tree of styles. In this
case we have to flatten the structure as it arrives from SGML tags into
a sequence of styles.
*/

/*		If style really needs to be set, call this
*/
PRIVATE void actually_set_style ARGS1(HTStructured *, this)
{
    if (!this->text) {			/* First time through */
	    this->text = HText_new2(this->node_anchor, this->target);
	    HText_beginAppend(this->text);
	    HText_setStyle(this->text, this->new_style);
	    this->in_word = NO;
    } else {
	    HText_setStyle(this->text, this->new_style);
    }
    this->old_style = this->new_style;
    this->style_change = NO;
}

/*      If you THINK you need to change style, call this
*/

PRIVATE void change_style ARGS2(HTStructured *, this, HTStyle *,style)
{
    if (this->new_style!=style) {
    	this->style_change = YES;
	this->new_style = style;
    }
}

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/
PRIVATE void FOSI_put_character ARGS2(HTStructured *, this, char, c)
{
/*	fprintf(stderr, "### FOSI  CHAR %c\n", c);
*/
	majorBuff[majorBuffi++] = c;

#ifdef FOSI_PUT_CHAR_NOT

    switch (this->sp[0].tag_number) {
    case FOSI_COMMENT:
    	break;					/* Do Nothing */

    case FOSI_TITLE:	
    	HTChunkPutc(&this->title, c);
	break;

	
    case FOSI_LISTING:				/* Litteral text */
    case FOSI_XMP:
    case FOSI_PLAINTEXT:
    case FOSI_PRE:
/*	We guarrantee that the style is up-to-date in begin_litteral
*/
    	HText_appendCharacter(this->text, c);
	break;
	
    default:					/* Free format text */
	if (this->style_change) {
	    if ((c=='\n') || (c==' ')) return;	/* Ignore it */
	    UPDATE_STYLE;
	}
	if (c=='\n') {
	    if (this->in_word) {
		HText_appendCharacter(this->text, ' ');
		this->in_word = NO;
	    }
	} else {
	    HText_appendCharacter(this->text, c);
	    this->in_word = YES;
	}
    } /* end switch */
#endif
}



/*	String handling
**	---------------
**
**	This is written separately from put_character becuase the loop can
**	in some cases be postponed to a lower level for speed.
*/
PRIVATE void FOSI_put_string ARGS2(HTStructured *, this, CONST char*, s)
{
	char *cp;
fprintf(stderr, "### FOSI  STRING {/%s}\n", s);


    switch (this->sp[0].tag_number) {
    default:					/* Free format text */
        {
	    CONST char *p = s;
	    if (this->style_change) {
		for (; *p && ((*p=='\n') || (*p==' ')); p++)  ;  /* Ignore leaders */
		if (!*p) return;
		UPDATE_STYLE;
	    }
	    for(; *p; p++) {
		if (this->style_change) {
		    if ((*p=='\n') || (*p==' ')) continue;  /* Ignore it */
		    UPDATE_STYLE;
		}
		if (*p=='\n') {
		    if (this->in_word) {
			majorBuff[majorBuffi++] = ' ';

			this->in_word = NO;
		    }
		} else {
		    /*HText_appendCharacter(this->text, *p);*/
		    majorBuff[majorBuffi++] = *p;
		    this->in_word = YES;
		}
	    } /* for */
	}
    } /* end switch */
}


PRIVATE void FOSI_progress ARGS2(HTStructured *, this, int, l)
{
}

/*	Buffer write
**
*/
PRIVATE void FOSI_write ARGS3(HTStructured *, this, CONST char*, s, int, l)
{
    CONST char* p;
    CONST char* e = s+l;

    for (p=s; s<e; p++) FOSI_put_character(this, *p);
fprintf(stderr, "### FOSI  WRITE {%s}\n", p);
}


/*	Start Element
**	-------------
*/
PRIVATE void FOSI_start_element ARGS5(
	HTStructured *, this,
	int,		element_number,
	BOOL*,		present,
	char**,		value,
	HTTag*,		tagInfo)
{
/* Safe to ignore only if the FOSI data is never significant
 */
/*
	majorBuff[majorBuffi] = '\0';
	fprintf(stderr, "### DATA(%d) -%s\n", majorBuffi, majorBuff);
	if (majorBuffi > 0) {
		CB_FOSI_data(majorBuff, majorBuffi);
		majorBuffi = 0;
	}
*/
	CB_FOSI_stag(element_number, present, value, tagInfo);

    	--(this->sp);
	/* Stack new style */
	this->sp[0].style = this->new_style;
	this->sp[0].tag_number = element_number;
}

/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
*/
PRIVATE void FOSI_end_element ARGS2(HTStructured *, this, int , element_number)
{
/*	majorBuff[majorBuffi] = '\0';

	fprintf(stderr, "### DATA(%d) -%s\n", majorBuffi, majorBuff);

	if (majorBuffi > 0) {
		CB_FOSI_data(majorBuff, majorBuffi);
		majorBuffi = 0;
	}
*/
fprintf(stderr, "### FOSI\t)%s\n",
 FOSI_dtd.tags[element_number].name);

	CB_FOSI_etag(element_number);

    this->sp++;				/* Pop state off stack */
}

/*		Expanding entities
**		------------------
*/
/*	(In fact, they all shrink!)
*/

PRIVATE void FOSI_put_entity ARGS2(HTStructured *, this, int, entity_number)
{
	majorBuff[majorBuffi++] = *ISO_Latin1[entity_number];

	fprintf(stderr, "### FOSI  ENTITY {%c}\n",
		*ISO_Latin1[entity_number]);
}


/*	Free an FOSI object
**	-------------------
**
**	If non-interactive, everything is freed off.
**	Otherwise, the interactive object is left.	
*/
PUBLIC void FOSI_free ARGS1(HTStructured *, this)
{
	fprintf(stderr, "### FOSI  FREE (not done)\n");
/*
    if (this->target) {
        (*this->targetClass.end)(this->target);
        (*this->targetClass.free)(this->target);
	HText_free(this->text);
    }
    free(this);
*/
}


PRIVATE void FOSI_end_document ARGS1(HTStructured *, this)

/* If the document is empty, the text object will not yet exist.
   So we could in fact abandon creating the document and return
   an error code.  In fact an empty document is an important type
   of document, so we don't.
*/
{
    UPDATE_STYLE;		/* Creates empty document here! */

	majorBuff[majorBuffi] = '\0';

	fprintf(stderr, "### DATA(%d) -%s\n", majorBuffi, majorBuff);

	if (majorBuffi > 0) {
		CB_FOSI_data(majorBuff, majorBuffi);
		majorBuffi = 0;
	}
	CB_FOSI_end();
}

/*				P U B L I C
*/

/*	Structured Object Class
**	-----------------------
*/
PUBLIC CONST HTStructuredClass FOSIPresentation = /* As opposed to print etc */
{		
	"text/FOSI",
	FOSI_free,
	FOSI_end_document,
	FOSI_put_character, 	FOSI_put_string,  FOSI_write,
	FOSI_progress,
	FOSI_start_element, 	FOSI_end_element,
	FOSI_put_entity
}; 

/*		New Structured Text object
**		--------------------------
**
**	If the stream is NULL then an interactive object is produced
*/
PUBLIC HTStructured* FOSI_new ARGS2(
	HTParentAnchor *, 	anchor,
	HTStream*,		stream)
{
    HTStructured * this = malloc(sizeof(*this));

    this->isa = &FOSIPresentation;
    this->node_anchor =  anchor;
    this->title.size = 0;
    this->title.growby = 128;
    this->title.allocated = 0;
    this->title.data = 0;
    this->text = 0;
    this->style_change = YES; /* Force check leading to text creation */
    this->new_style = default_style;
    this->old_style = 0;
    this->sp = this->stack + MAX_NESTING - 1;
    this->sp->tag_number = -1;				/* INVALID */
    this->sp->style = default_style;			/* INVALID */
    
    this->comment_start = NULL;
    this->comment_end = NULL;
    this->target = stream;
    if (stream) this->targetClass = *stream->isa;	/* Copy pointers */
    
	majorBuffi = 0;

	CB_FOSI_new();

    return (HTStructured*) this;
}

/*	Presenter for FOSI
**	------------------
**
**	This will convert from FOSI to presentation or plain text.
**
**	Override this if you have a windows version
*/
#ifndef GUI
PUBLIC HTStream* FOSIPresent ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    return SGML_new(&FOSI_dtd, FOSI_new(anchor, NULL));
}
#endif

