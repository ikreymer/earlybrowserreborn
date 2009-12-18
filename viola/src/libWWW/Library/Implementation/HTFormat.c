
/*		Manage different file formats			HTFormat.c
**		=============================
**
** Bugs:
**	Not reentrant.
**
**	Assumes the incoming stream is ASCII, rather than a local file
**	format, and so ALWAYS converts from ASCII on non-ASCII machines.
**	Therefore, non-ASCII machines can't read local files.
**
*/


/* Implements:
*/
#include "HTFormat.h"

PUBLIC float HTMaxSecs = 1e10;		/* No effective limit */
PUBLIC float HTMaxLength = 1e10;	/* No effective limit */

#ifdef unix
#ifdef NeXT
#define PRESENT_POSTSCRIPT "open %s; /bin/rm -f %s\n"
#else
#define PRESENT_POSTSCRIPT "(ghostview %s ; /bin/rm -f %s)&\n"	
	/* Full pathname would be better! */
#endif
#endif


#include "HTUtils.h"
#include "tcp.h"

#include "HTML.h"
#include "HTMLDTD.h"
#undef HTEXT_H /*PYW*/
#include "HText.h"
#include "HTAlert.h"
#include "HTList.h"
#include "HTInit.h"
/*	Streams and structured streams which we use:
*/
#include "HTFWriter.h"
#include "HTPlain.h"
#include "SGML.h"
#include "HTML.h"
#include "HTMLGen.h"

PUBLIC	BOOL HTOutputSource = NO;	/* Flag: shortcut parser to stdout */
extern  BOOL interactive;

#ifdef ORIGINAL
struct _HTStream {
      CONST HTStreamClass*	isa;
      /* ... */
};
#endif

/* this version used by the NetToText stream */
struct _HTStream {
	CONST HTStreamClass *		isa;
	BOOL			had_cr;
	HTStream * 		sink;
};


/*	Presentation methods
**	--------------------
*/

PUBLIC  HTList * HTPresentations = 0;
PUBLIC  HTPresentation* default_presentation = 0;


/*	Define a presentation system command for a content-type
**	-------------------------------------------------------
*/
PUBLIC void HTSetPresentation ARGS5(
	CONST char *, representation,
	CONST char *, command,
	float,	quality,
	float,	secs, 
	float,	secs_per_byte
){

    HTPresentation * pres = (HTPresentation *)malloc(sizeof(HTPresentation));
    if (pres == NULL) outofmem(__FILE__, "HTSetPresentation");
    
    pres->rep = HTAtom_for(representation);
    pres->rep_out = WWW_PRESENT;		/* Fixed for now ... :-) */
    pres->converter = HTSaveAndExecute;		/* Fixed for now ...     */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->rep = HTAtom_for(representation);
    pres->command = 0;
    StrAllocCopy(pres->command, command);
    
    if (!HTPresentations) HTPresentations = HTList_new();
    
    if (strcmp(representation, "*")==0) {
        if (default_presentation) free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObject(HTPresentations, pres);
    }
}


/*	Define a built-in function for a content-type
**	---------------------------------------------
*/
PUBLIC void HTSetConversion ARGS6(
	CONST char *, representation_in,
	CONST char *, representation_out,
	HTConverter*,	converter,
	float,	quality,
	float,	secs, 
	float,	secs_per_byte
){

    HTPresentation * pres = (HTPresentation *)malloc(sizeof(HTPresentation));
    if (pres == NULL) outofmem(__FILE__, "HTSetPresentation");
    
    pres->rep = HTAtom_for(representation_in);
    pres->rep_out = HTAtom_for(representation_out);
    pres->converter = converter;
    pres->command = NULL;		/* Fixed */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->command = 0;
    
    if (!HTPresentations) HTPresentations = HTList_new();
    
    if (strcmp(representation_in, "*")==0) {
        if (default_presentation) free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObject(HTPresentations, pres);
    }
}



/*	File buffering
**	--------------
**
**	The input file is read using the macro which can read from
**	a socket or a file.
**	The input buffer size, if large will give greater efficiency and
**	release the server faster, and if small will save space on PCs etc.
*/
#define INPUT_BUFFER_SIZE 1024	/* Tradeoff */
PRIVATE char * input_buffer;
PRIVATE char * input_pointer;
PRIVATE char * input_limit;
PRIVATE int input_file_number;

#define BUFF_INFO_SIZE 8					/*PYW*/
PRIVATE char *   input_buffer_stack[BUFF_INFO_SIZE];
PRIVATE int input_file_number_stack[BUFF_INFO_SIZE];
PRIVATE char *  input_pointer_stack[BUFF_INFO_SIZE];
PRIVATE char *    input_limit_stack[BUFF_INFO_SIZE];
PRIVATE int      input_buffer_stacki = 0;

/*	Set up the buffering
**
**	These routines are public because they are in fact needed by
**	many parsers, and on PCs and Macs we should not duplicate
**	the static buffer area.
*/
PUBLIC void HTInitInput ARGS1 (int,file_number)
{
    input_buffer = input_buffer_stack[input_buffer_stacki];/*PYW*/
    input_file_number = file_number;
    input_pointer = input_limit = input_buffer;
}

/*
 * push buffer reading information into stack    	PYW 930602
 *
 * this is a reentrant hack
 *
 * MUST CALL THIS before HTInitInput(), and MUST CALL HTPopBuffer() afterwards.
 *
 * XXX study calling sequence and make this push/pop transparent
 *	add buffer stack? buffer is turned off for now.!
 */
PUBLIC BOOL HTPushInputBuffer NOARGS
{
    static BOOL doInitialize = 1;

    if (doInitialize) {
	int i;
        for (i = 0; i < BUFF_INFO_SIZE; i++) input_buffer_stack[i] = NULL;
	doInitialize = 0;
    }
    if (++input_buffer_stacki > BUFF_INFO_SIZE) {
        fprintf(stderr, "HTFormat.c: File buffering stack overflow.\n");
        return NO;
    }
    if (!input_buffer_stack[input_buffer_stacki]) {
        input_buffer_stack[input_buffer_stacki] = 
            (char*)malloc(sizeof(char) * INPUT_BUFFER_SIZE);
    }

    input_buffer = input_buffer_stack[input_buffer_stacki];

    if (input_buffer_stacki > 0) {
        input_file_number_stack[input_buffer_stacki - 1] = input_file_number;
        input_pointer_stack[input_buffer_stacki - 1] = input_pointer;
        input_limit_stack[input_buffer_stacki - 1] = input_limit;
    }
    return YES;
}

/*
 * pop buffer reading information from stack		PYW 930602
 */
PUBLIC BOOL HTPopInputBuffer NOARGS
{
    --input_buffer_stacki;

    input_buffer = input_buffer_stack[input_buffer_stacki];
    input_file_number = input_file_number_stack[input_buffer_stacki];
    input_pointer = input_pointer_stack[input_buffer_stacki];
    input_limit = input_limit_stack[input_buffer_stacki];

    return YES;
}

PUBLIC char HTGetChararcter NOARGS
{
    char ch;
    do {
	if (input_pointer >= input_limit) {
	    int status = NETREAD(
		    input_file_number, input_buffer, INPUT_BUFFER_SIZE);
	    if (status <= 0) {
		if (status == 0) return (char)EOF;
		if (TRACE) fprintf(stderr,
		    "HTFormat: File read error %d\n", status);
		return (char)EOF; /* -1 is returned by UCX at end of HTTP link */
	    }
	    input_pointer = input_buffer;
	    input_limit = input_buffer + status;
	}
	ch = *input_pointer++;
    } while (ch == (char) 13); /* Ignore ASCII carriage return */
    
    return FROMASCII(ch);
}

/*	Stream the data to an ouput file as binary
*/
PUBLIC int HTOutputBinary ARGS2( int, 		input,
				  FILE *, 	output)
{
    do {
	    int status = NETREAD(
		    input, input_buffer, INPUT_BUFFER_SIZE);
	    if (status <= 0) {
		if (status == 0) return 0;
		if (TRACE) fprintf(stderr,
		    "HTFormat: File read error %d\n", status);
		return 2;			/* Error */
	    }
	    fwrite(input_buffer, sizeof(char), status, output);
    } while (YES);
}


/*		Create a filter stack
**		---------------------
**
**	If a wildcard match is made, a temporary HTPresentation
**	structure is made to hold the destination format while the
**	new stack is generated. This is just to pass the out format to
**	MIME so far.  Storing the format of a stream in the stream might
**	be a lot neater.
**
**	The www/source format is special, in that if you can take
**	that you can take anything. However, we
*/
PUBLIC HTStream * HTStreamStack ARGS4(
	HTFormat,		rep_in,
	HTFormat,		rep_out,
	HTStream*,		sink,
	HTParentAnchor*,	anchor)
{
    HTAtom * wildcard = HTAtom_for("*");
    HTFormat source = WWW_SOURCE;
    if (TRACE) fprintf(stderr,
    	"HTFormat: Constructing stream stack for %s to %s\n",
	HTAtom_name(rep_in),	
	HTAtom_name(rep_out));
		
    if (rep_out == WWW_SOURCE ||
    	rep_out == rep_in) return sink;

    if (!HTPresentations) HTFormatInit();	/* set up the list */
    
    {
	int n = HTList_count(HTPresentations);
	int i;
	HTPresentation * pres, *match, *wildcard_match=0,
			*source_match=0, *source_wildcard_match=0;
	for(i=0; i<n; i++) {
	    pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep == rep_in) {
	        if (pres->rep_out == rep_out)
	            return (*pres->converter)(pres, anchor, sink);
		if (pres->rep_out == wildcard) {
		    wildcard_match = pres;
		}
	    }
	    if (pres->rep == source) {
	        if (pres->rep_out == rep_out)
	            source_match = pres;
		if (pres->rep_out == wildcard) {
		    source_wildcard_match = pres;
		}
	    }
	}
	
	match = wildcard_match ? wildcard_match :
		source_match ?	source_match : 
		source_wildcard_match;
	
	if (match) {
		HTPresentation temp;
		temp = *match;			/* Specific instance */
		temp.rep = rep_in;		/* yuk */
		temp.rep_out = rep_out;		/* yuk */
		return (*match->converter)(&temp, anchor, sink);
        }
    }

    
#ifdef XMOSAIC_HACK_REMOVED_NOW  /* Use above source method instead */
    return sink;
#else
    return NULL;
#endif
}
	

/*		Find the cost of a filter stack
**		-------------------------------
**
**	Must return the cost of the same stack which StreamStack would set up.
**
** On entry,
**	length	The size of the data to be converted
*/
PUBLIC float HTStackValue ARGS4(
	HTFormat,		rep_in,
	HTFormat,		rep_out,
	float,			initial_value,
	long int,		length)
{
    HTAtom * wildcard = HTAtom_for("*");

    if (TRACE) fprintf(stderr,
    	"HTFormat: Evaluating stream stack for %s worth %.3f to %s\n",
	HTAtom_name(rep_in),	initial_value,
	HTAtom_name(rep_out));
		
    if (rep_out == WWW_SOURCE ||
    	rep_out == rep_in) return 0.0;

    if (!HTPresentations) HTFormatInit();	/* set up the list */
    
    {
	int n = HTList_count(HTPresentations);
	int i;
	HTPresentation * pres;
	for(i=0; i<n; i++) {
	    pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep == rep_in && (
	    		pres->rep_out == rep_out ||
			pres->rep_out == wildcard)) {
	        float value = initial_value * pres->quality;
		if (HTMaxSecs != 0.0)
		value = value - (length*pres->secs_per_byte + pres->secs)
			                 /HTMaxSecs;
		return value;
	    }
	}
    }
    
    return -1e30;		/* Really bad */

}
	

/*	Push data from a socket down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC void HTCopy ARGS2(
	int,			file_number,
	HTStream*,		sink)
{
    HTStreamClass targetClass;    
    
/*	Push the data down the stream
**
*/
    targetClass = *(sink->isa);	/* Copy pointers to procedures */
    /*	Push binary from socket down sink
    **
    **		This operation could be put into a main event loop
    */
    for(;;) {
	int status = NETREAD(
		file_number, input_buffer, INPUT_BUFFER_SIZE);
	if (status <= 0) {
	    if (status == 0) break;
	    if (TRACE) fprintf(stderr,
		"HTFormat: Read error, read returns %d\n", status);
	    break;
	}
	
#ifdef NOT_ASCII
	{
	    char * p;
	    for(p = input_buffer; p < input_buffer+status; p++) {
		*p = FROMASCII(*p);
	    }
	}
#endif
	(*targetClass.progress)(sink, status);
	(*targetClass.put_block)(sink, input_buffer, status);
    } /* next bufferload */
}



/*	Push data from a file pointer down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**
*/
PUBLIC void HTFileCopy ARGS2(
	FILE *,			fp,
	HTStream*,		sink)
{
    HTStreamClass targetClass;    
    extern int http_progress_expected_total_bytes;  /*PYW*/
    extern int http_progress_subtotal_bytes;
    extern int http_progress_total_bytes;
    extern int http_progress_reporter_level;
    
/*	Push the data down the stream
**
*/
    targetClass = *(sink->isa);	/* Copy pointers to procedures */
    
    HTPushInputBuffer();/*PYW*/
    HTInitInput(fp);/*PYW*//*XXXX*/

/*    ++http_progress_reporter_level;
*/
    /*	Push binary from socket down sink
    */
    for(;;) {
	int status = fread(
	       input_buffer, 1, INPUT_BUFFER_SIZE, fp);
	if (status == 0) { /* EOF or error */
	    if (ferror(fp) == 0) break;
	    if (TRACE) fprintf(stderr,
		"HTFormat: Read error, read returns %d\n", ferror(fp));
	    break;
	}
	(*targetClass.progress)(sink, status);
	(*targetClass.put_block)(sink, input_buffer, status);
    } /* next bufferload */

/*
    --http_progress_reporter_level;
    if (http_progress_reporter_level <= 0) { 
      http_progress_total_bytes = 0;
      http_progress_expected_total_bytes = 0;
    }
    http_progress_subtotal_bytes = 0;
*/
	
    HTPopInputBuffer();/*PYW*/
}




/*	Push data from a socket down a stream STRIPPING CR
**	--------------------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the socket.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC void HTCopyNoCR ARGS2(
	int,			file_number,
	HTStream*,		sink)
{
    HTStreamClass targetClass;    
    
/*	Push the data, ignoring CRLF, down the stream
**
*/
    targetClass = *(sink->isa);	/* Copy pointers to procedures */

    HTPushInputBuffer();/*PYW*/
    HTInitInput(file_number);

/*	Push text from telnet socket down sink
**
**	@@@@@ To push strings could be faster? (especially is we
**	cheat and don't ignore CR! :-}
*/  
    for(;;) {
	char character;
	character = HTGetChararcter();
	if (character == (char)EOF) break;
	(*targetClass.put_character)(sink, character);           
    }
    HTPopInputBuffer();/*PYW*/
}



/*	Parse a socket given format and file number
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseSocket ARGS5(
	HTFormat,		rep_in,
	HTFormat,		format_out,
	HTParentAnchor *,	anchor,
	int,			file_number,
	HTStream*,		sink)
{
    HTStream * stream;
    HTStreamClass targetClass;    

    stream = HTStreamStack(rep_in,
			format_out,
	 		sink , anchor);
    
    if (!stream) {
        char buffer[1024];	/* @@@@@@@@ */
	sprintf(buffer, "Sorry, can't convert from %s to %s.",
		HTAtom_name(rep_in), HTAtom_name(format_out));
	if (TRACE) fprintf(stderr, "HTFormat: %s\n", buffer);
        return HTLoadError(sink, 501, buffer);
    }
    
/*	Push the data, ignoring CRLF if necessary, down the stream
**
**
**   @@  Bug:  This decision ought to be made based on "encoding"
**   rather than on format.  @@@  When we handle encoding.
**   The current method smells anyway.
*/
    targetClass = *(stream->isa);	/* Copy pointers to procedures */
    if (rep_in == WWW_BINARY || HTOutputSource
        || strstr(HTAtom_name(rep_in), "image/")
	|| strstr(HTAtom_name(rep_in), "video/")) { /* @@@@@@ */
        HTCopy(file_number, stream);
    } else {   /* ascii text with CRLFs :-( */
        HTCopyNoCR(file_number, stream);
    }
    (*targetClass.end)(stream);
    (*targetClass.free)(stream);
    
    return HT_LOADED;
}



/*	Parse a file given format and file pointer
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to \n for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseFile ARGS5(
	HTFormat,		rep_in,
	HTFormat,		format_out,
	HTParentAnchor *,	anchor,
	FILE *,			fp,
	HTStream*,		sink)
{
    HTStream * stream;
    HTStreamClass targetClass;    

    stream = HTStreamStack(rep_in,
			format_out,
	 		sink , anchor);
    
    if (!stream) {
        char buffer[1024];	/* @@@@@@@@ */
	sprintf(buffer, "Sorry, can't convert from %s to %s.",
		HTAtom_name(rep_in), HTAtom_name(format_out));
	if (TRACE) fprintf(stderr, "HTFormat(in HTParseFile): %s\n", buffer);
        return HTLoadError(sink, 501, buffer);
    }
    
/*	Push the data down the stream
**
**
**   @@  Bug:  This decision ought to be made based on "encoding"
**   rather than on content-type.  @@@  When we handle encoding.
**   The current method smells anyway.
*/
    targetClass = *(stream->isa);	/* Copy pointers to procedures */
    HTFileCopy(fp, stream);

    (*targetClass.end)(stream);
    (*targetClass.free)(stream);
    
    return HT_LOADED;
}


/*	Converter stream: Network Telnet to internal character text
**	-----------------------------------------------------------
**
**	The input is assumed to be in ASCII, with lines delimited
**	by (13,10) pairs, These pairs are converted into (CR,LF)
**	pairs in the local representation.  The (CR,LF) sequence
**	when found is changed to a '\n' character, the internal
**	C representation of a new line.
*/


PRIVATE void NetToText_put_character ARGS2(HTStream *, me, char, net_char)
{
    char c = FROMASCII(net_char);
    if (me->had_cr) {
        if (c==LF) {
	    me->sink->isa->put_character(me->sink, '\n');	/* Newline */
	    me->had_cr = NO;
	    return;
        } else {
	    me->sink->isa->put_character(me->sink, CR);	/* leftover */
	}
    }
    me->had_cr = (c==CR);
    if (!me->had_cr)
	me->sink->isa->put_character(me->sink, c);		/* normal */
}

PRIVATE void NetToText_put_string ARGS2(HTStream *, me, CONST char *, s)
{
    CONST char * p;
    for(p=s; *p; p++) NetToText_put_character(me, *p);
}

PRIVATE void NetToText_progress ARGS2(HTStream *, me, int, l)
{
#ifdef VIOLA  /* KLUDGE ALERT */
  http_progress_notify(l);
#endif
}

PRIVATE void NetToText_put_block ARGS3(HTStream *, me, CONST char*, s, int, l)
{
    CONST char * p;
    for(p=s; p<(s+l); p++) NetToText_put_character(me, *p);
}

PRIVATE void NetToText_free ARGS1(HTStream *, me)
{
    me->sink->isa->free(me->sink);		/* Close rest of pipe */
    free(me);
}

PRIVATE void NetToText_end ARGS1(HTStream *, me)
{
}

PRIVATE void NetToText_abort ARGS2(HTStream *, me, HTError, e)
{
    me->sink->isa->abort(me->sink,e);		/* Abort rest of pipe */
    free(me);
}

/*	The class structure
*/
PRIVATE HTStreamClass NetToTextClass = {
    "NetToText",
    NetToText_free,
    NetToText_end,
    NetToText_abort,
    NetToText_put_character,
    NetToText_put_string,
    NetToText_progress,
    NetToText_put_block,
};

/*	The creation method
*/
PUBLIC HTStream * HTNetToText ARGS1(HTStream *, sink)
{
    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "NetToText");
    me->isa = &NetToTextClass;
    
    me->had_cr = NO;
    me->sink = sink;
    return me;
}


