/*			General SGML Parser code		SGML.c
**			========================
**
**	This module implements an HTStream object. To parse an
**	SGML file, create this object which is a parser. The object
**	is (currently) created by being passed a DTD structure,
**	and a target HTStructured oject at which to throw the parsed stuff.
**	
**	 6 Feb 93  Binary seraches used. Intreface modified.
*/
#include "SGML.h"

/*#define TRACE 1*/
/*#define IMPLY_VERBOSE 1*/

#include <ctype.h>
#include <stdio.h>
#include "HTUtils.h"
#include "HTChunk.h"
#include "tcp.h"		/* For FROMASCII */

#define INVALID (-1)

/*	The State (context) of the parser
**
**	This is passed with each call to make the parser reentrant
**
*/

#define MAX_ATTRIBUTES 20	/* Max number of attributes per element */

	
/*		Element Stack
**		-------------
**	This allows us to return down the stack reselcting styles.
**	As we return, attribute values will be garbage in general.
*/
typedef struct _HTElement HTElement;
struct _HTElement {
	HTElement *	next;	/* Previously nested element or 0 */
	HTTag *		tag;	/* The tag at this level  */
};


/*	Internal Context Data Structure
**	-------------------------------
*/
struct _HTStream {

    CONST HTStreamClass *	isa;		/* inherited from HTStream */
    
    CONST SGML_dtd 		*dtd;
    HTStructuredClass	*actions;	/* target class  */
    HTStructured	*target;	/* target object */

    HTTag 		*current_tag;
    int 		current_attribute_number;
    HTChunk		*string;
    HTElement		*element_stack;
    enum sgml_state { S_text, S_litteral, S_tag, S_tag_gap, 
		S_attr, S_attr_gap, S_equals, S_value,
		S_ero, S_cro,
		  S_squoted, S_dquoted, S_end, S_entity, S_junk_tag} state;
#ifdef CALLERDATA		  
    void *		callerData;
#endif
    BOOL present[MAX_ATTRIBUTES];	/* Flags: attribute is present? */
    char * value[MAX_ATTRIBUTES];	/* malloc'd strings or NULL if none */
} ;


#define PUTC(ch) ((*context->actions->put_character)(context->target, ch))



/*	Handle Attribute
**	----------------
*/
/* PUBLIC CONST char * SGML_default = "";   ?? */

#ifdef __STDC__
PRIVATE void handle_attribute_name(HTStream * context, const char * s)
#else
PRIVATE void handle_attribute_name(context, s)
    HTStream * context;
    char *s;
#endif
{

    HTTag * tag = context->current_tag;
    attr* attributes = tag->attributes;

    int high, low, i, diff;		/* Binary search for attribute name */
    for(low=0, high=tag->number_of_attributes;
    		high > low ;
		diff < 0 ? (low = i+1) : (high = i) )  {
	i = (low + (high-low)/2);
	diff = strcasecomp(attributes[i].name, s);
	if (diff==0) {			/* success: found it */
    	    context->current_attribute_number = i;
	    context->present[i] = YES;
	    if (context->value[i]) {
		free(context->value[i]);
		context->value[i] = NULL;
	    }
	    return;
	} /* if */
	
    } /* for */
    
    if (TRACE)
	fprintf(stderr, "SGML: Unknown attribute %s for tag %s\n",
	    s, context->current_tag->name);
    context->current_attribute_number = INVALID;	/* Invalid */
}


/*	Handle attribute value
**	----------------------
*/
#ifdef __STDC__
PRIVATE void handle_attribute_value(HTStream * context, const char * s)
#else
PRIVATE void handle_attribute_value(context, s)
    HTStream * context;
    char *s;
#endif
{
    if (context->current_attribute_number != INVALID) {
	StrAllocCopy(context->value[context->current_attribute_number], s);
    } else {
        if (TRACE) fprintf(stderr, "SGML: Attribute value %s ignored\n", s);
    }
    context->current_attribute_number = INVALID; /* can't have two assignments! */
}


/*	Handle entity
**	-------------
**
** On entry,
**	s	contains the entity name zero terminated
** Bugs:
**	If the entity name is unknown, the terminator is treated as
**	a printable non-special character in all cases, even if it is '<'
*/
#ifdef __STDC__
PRIVATE void handle_entity(HTStream * context, char term)
#else
PRIVATE void handle_entity(context, term)
    HTStream * context;
    char term;
#endif
{

    CONST char ** entities = context->dtd->entity_names;
    CONST char *s = context->string->data;
    
    int high, low, i, diff;
    for(low=0, high = context->dtd->number_of_entities;
    		high > low ;
		diff < 0 ? (low = i+1) : (high = i))   {  /* Binary serach */
	i = (low + (high-low)/2);
	diff = strcmp(entities[i], s);	/* Csse sensitive! */
	if (diff==0) {			/* success: found it */
	    (*context->actions->put_entity)(context->target, i);
	    return;
	}
    }
    /* If entity string not found, display as text */
    if (TRACE)
	fprintf(stderr, "SGML: Unknown entity %s\n", s); 
    PUTC('&');
    {
	CONST char *p;
	for (p=s; *p; p++) {
	    PUTC(*p);
	}
    }
    PUTC(term);
}


/*	End element
**	-----------
*/
#ifdef __STDC__
PRIVATE void end_element(HTStream * context, HTTag * old_tag)
#else
PRIVATE void end_element(context, old_tag)
    HTTag * old_tag;
    HTStream * context;
#endif
{
    if (TRACE) fprintf(stderr, "SGML: End   </%s>\n", old_tag->name);
    if (old_tag->contents == SGML_EMPTY) {
        if (TRACE) fprintf(stderr,"SGML: Illegal end tag </%s> found.\n",
		old_tag->name);
	return;
    }
#ifdef IMPLY_VERBOSE
	{
	HTElement * ptr;
		printf("STACK: ");
		for (ptr = context->element_stack; ptr; ptr = ptr->next) {
			printf("%s ", ptr->tag->name);
		}
		printf("\n");
	}
#endif

    while (context->element_stack) 	{/* Loop is error path only */
	HTElement * N = context->element_stack;
	HTTag * t = N->tag;
	
	if (old_tag != t) {		/* Mismatch: syntax error */
	    if (context->element_stack->next) {	/* This is not the last level */
		if (TRACE) fprintf(stderr,
	    	"SGML: Found </%s> when expecting </%s>. </%s> assumed.\n",
		    old_tag->name, t->name, t->name);
	    } else {			/* last level */
		if (TRACE) fprintf(stderr,
	            "SGML: Found </%s> when expecting </%s>. </%s> Ignored.\n",
		    old_tag->name, t->name, old_tag->name);
	        return;			/* Ignore */
	    }
	}
	
	context->element_stack = N->next;		/* Remove from stack */
	free(N);
	(*context->actions->end_element)(context->target,
		 t - context->dtd->tags);
	if (old_tag == t) return;  /* Correct sequence */
	
	/* Syntax error path only */
	
    }
    if (TRACE) fprintf(stderr,
	"SGML: Extra end tag </%s> found and ignored.\n", old_tag->name);
}

/*	Close Element (End Element in EMPTY_CONTENT case)     PYW
**	-----------
*/
#ifdef __STDC__
PRIVATE void close_element(HTStream * context, HTTag * old_tag)
#else
PRIVATE void close_element(context, old_tag)
    HTStream * context;
    HTTag * old_tag;
#endif
{
    /*PYW: for emulating SGMLS behaviour */
	(*context->actions->end_element)(context->target,
		 old_tag - context->dtd->tags);
}


/*	Start a element
*/
#ifdef __STDC__
PRIVATE void start_element(HTStream * context)
#else
PRIVATE void start_element(context)
    HTStream * context;
#endif
{
    HTTag * new_tag = context->current_tag;
    
    if (TRACE) fprintf(stderr, "SGML: Start <%s>\n", new_tag->name);
    (*context->actions->start_element)(
    	context->target,
	new_tag - context->dtd->tags,
	context->present,
	(CONST char**) context->value,   /* coerce type for think c */
	new_tag /*PYW*/);
    if (new_tag->contents != SGML_EMPTY) {		/* i.e. tag not empty */
	HTElement * N = (HTElement *)malloc(sizeof(HTElement));
        if (N == NULL) outofmem(__FILE__, "start_element");
	N->next = context->element_stack;
	N->tag = new_tag;
	context->element_stack = N;
    }
}


/*		Find Tag in DTD tag list
**		------------------------
**
** On entry,
**	dtd	points to dtd structire including valid tag list
**	string	points to name of tag in question
**
** On exit,
**	returns:
**		NULL		tag not found
**		else		address of tag structure in dtd
*/
PUBLIC HTTag * SGMLFindTag ARGS2(CONST SGML_dtd*, dtd, CONST char *, string)
{
    int high, low, i, diff;
    for(low=0, high=dtd->number_of_tags;
    		high > low ;
		diff < 0 ? (low = i+1) : (high = i))   {  /* Binary serach */
	i = (low + (high-low)/2);
	diff = strcasecomp(dtd->tags[i].name, string);	/* Case insensitive */
	if (diff==0) {			/* success: found it */
	    return &dtd->tags[i];
	}
    }
    return NULL;
}

/*________________________________________________________________________
**			Public Methods
*/


/*	Could check that we are back to bottom of stack! @@  */

PUBLIC void SGML_free  ARGS1(HTStream *, context)
{
    (*context->actions->free)(context->target);
    HTChunkFree(context->string);
    free(context);
}

PUBLIC void SGML_end  ARGS1(HTStream *, context)
{
    (*context->actions->end)(context->target);
}

PUBLIC void SGML_abort  ARGS2(HTStream *, context, HTError, e)
{
    (*context->actions->abort)(context->target, e);
    HTChunkFree(context->string);
    free(context);
}


/*	Read and write user callback handle
**	-----------------------------------
**
**   The callbacks from the SGML parser have an SGML context parameter.
**   These calls allow the caller to associate his own context with a
**   particular SGML context.
*/

#ifdef CALLERDATA		  
PUBLIC void* SGML_callerData ARGS1(HTStream *, context)
{
    return context->callerData;
}

PUBLIC void SGML_setCallerData ARGS2(HTStream *, context, void*, data)
{
    context->callerData = data;
}
#endif

PRIVATE void closeImpliedTags ARGS2(HTStream *, context, HTTag *, tag)
{
	int *subep;
	HTElement *bottom; /* bottom item of stack */
	char *tagName;
	int implyETagP = 1;
	int elemCount;
	int i, high, low, diff;
	int tagIdx;

	tagIdx = tag - context->dtd->tags;
	tagName = context->dtd->tags[tagIdx].name;

	subep = (int*)context->element_stack->tag->valid_sub_elements;
	if (!subep) return;

#ifdef sequentialSearch /* don't work anymore-- no terminator in list */
	for (; *subep != -1; subep++) {
/*		printf("//// valid sub elem: %s (%d==%d)\n",
			context->dtd->tags[*subep].name,
			*subep, tag->name);
*/
		if (*subep == tagIdx) {
			implyETagP = 0;
			break;
		}
	}
#endif
	for (low = 0, 
	       high = context->element_stack->tag->valid_sub_elementsCount;
	     high > low;
	     diff < 0 ? (low = i + 1) : (high = i)) {
		i = (low + (high - low)/2);
		diff = *(subep + i) - tagIdx;
		if (diff == 0) {
			implyETagP = 0;
			break;
		}
	}

	if (implyETagP) {
#ifdef IMPLY_VERBOSE
	printf("!!!!!! %s isn't valid sub elem of %s --> backup the stack.\n",
		tagName, context->element_stack->tag->name);
#endif
		while (context->element_stack) {
			implyETagP = 1;
			bottom = context->element_stack;
			if (bottom == NULL) {
#ifdef IMPLY_VERBOSE
				fprintf(stderr,
					"::: empty stack. BREAK\n");
#endif
				break;
			}
#ifdef IMPLY_VERBOSE
			fprintf(stderr,
				"::: bottom-tag(n)=%s  tag=%s\n",
				bottom->tag->name, tagName);
#endif
			subep = (int*)bottom->tag->valid_sub_elements;
			if (subep) {

				for (low = 0, 
			       high = bottom->tag->valid_sub_elementsCount;
				     high > low;
				     diff < 0 ? (low = i + 1) : (high = i)) {
					i = (low + (high - low)/2);
					diff = *(subep + i) - tagIdx;
					if (diff == 0) {
						implyETagP = 0;
						break;
					}
				}

#ifdef NONONO
				for (; *subep != -1; subep++) {
			    printf(">>> valid sub elem: %s (%d==%d)\n",
					context->dtd->tags[*subep].name,
					*subep, tagName);
					if (*subep == tagIdx) {
						implyETagP = 0;
						break;
					}
				}
#endif
				if (implyETagP) {
#ifdef IMPLY_VERBOSE
printf(">>>> %s isn't valid sub elem of bottom tag %s --> imply closure.\n",
	tagName, bottom->tag->name);
#endif
					end_element(context, bottom->tag);
				} else break;
			} else break;
		}
	}
}


PUBLIC void SGML_character ARGS2(HTStream *, context, char,c)

{
    CONST SGML_dtd	*dtd	=	context->dtd;
    HTChunk	*string = 	context->string;

#ifdef IMPLY_VERBOSE
printf(">>> SGML_character [%c]:", c);
/*
if (context->current_tag)
	printf("current=%s  ",
		context->current_tag->name ?
			context->current_tag->name : "(NIL)");
*/
if (context->element_stack)
	if (context->element_stack->next)
		if (context->element_stack->next->tag)
			printf("next=%s\n", 
			(context->element_stack->next->tag->name ?
			 context->element_stack->next->tag->name : "(NIL)"));
else printf("\n");
else printf("\n");
else printf("\n");
#endif

    switch(context->state) {
    case S_text:   /* !     PYW:  seems either this or change <A>'s
		            content model to PCDATA XXX ??? */ 
	if (c=='&' && (context->element_stack &&
	    		context->element_stack->tag  &&
	    		( context->element_stack->tag->contents == SGML_MIXED
			 || context->element_stack->tag->contents == SGML_RCDATA)
			)) {
/* Original code:
	if (c=='&' && (!context->element_stack || (
	    		 context->element_stack->tag  &&
	    		 ( context->element_stack->tag->contents == SGML_MIXED
			   || context->element_stack->tag->contents ==
			      				 SGML_RCDATA)
			))) 
*/
	    string->size = 0;
	    context->state = S_ero;
	    
	} else if (c=='<') {
	    string->size = 0;
	    context->state = (context->element_stack &&
	    		context->element_stack->tag  &&
	    		context->element_stack->tag->contents == SGML_LITTERAL) ?
	    			S_litteral : S_tag;
	} else PUTC(c);
	break;

/*	In litteral mode, waits only for specific end tag!
**	Only foir compatibility with old servers.
*/
    case S_litteral :
	HTChunkPutc(string, c);
	if ( TOUPPER(c) != ((string->size ==1) ? '/'
		: context->element_stack->tag->name[string->size-2])) {
	    int i;
	    
	    /*	If complete match, end litteral */
	    if ((c=='>') && (!context->element_stack->tag->name[string->size-2])) {
		end_element(context, context->element_stack->tag);
		string->size = 0;
		context->current_attribute_number = INVALID;
		context->state = S_text;
		break;
	    }		/* If Mismatch: recover string. */
	    PUTC( '<');
	    for (i=0; i<string->size; i++)	/* recover */
	       PUTC(
	       				      string->data[i]);
	    context->state = S_text;	
	}
	
        break;

/*	Character reference or Entity
*/
   case S_ero:
   	if (c=='#') {
	    context->state = S_cro;  /*   &# is Char Ref Open */ 
	    break;
	}
	context->state = S_entity;    /* Fall through! */
	
/*	Handle Entities
*/
    case S_entity:
	if (isalnum(c))
	    HTChunkPutc(string, c);
	else {
	    HTChunkTerminate(string);
	    handle_entity(context, c);
	    context->state = S_text;
	}
	break;

/*	Character reference
*/
    case S_cro:
	if (isalnum(c))
	    HTChunkPutc(string, c);	/* accumulate a character NUMBER */
	else {
	    int value;
	    HTChunkTerminate(string);
	    if (sscanf(string->data, "%d", &value)==1)
	        PUTC(FROMASCII((char)value));
	    context->state = S_text;
	}
	break;

/*		Tag
*/	    
    case S_tag:				/* new tag */
	if (isalnum(c))
	    HTChunkPutc(string, c);
	else {				/* End of tag name */
	    HTTag * t;
	    if (c=='/') {
		if (TRACE) if (string->size!=0)
		    fprintf(stderr,"SGML:  `<%s/' found!\n", string->data);
		context->state = S_end;
		break;
	    }
	    HTChunkTerminate(string) ;

	    t = SGMLFindTag(dtd, string->data);
	    if (!t) {
		if(TRACE) fprintf(stderr, "SGML: *** Unknown element %s\n",
			string->data);
		context->state = (c=='>') ? S_text : S_junk_tag;
		break;
	    }
	    context->current_tag = t;
	    
	    if (t /* context->current_tag*/ &&		/*PYW*/
		context->element_stack &&
		context->element_stack->tag) {

/*		char *newtag = dtd->tags[t].name;*//* same t as old lib??*/
		char *newtag = t->name;
#ifdef IMPLY_VERBOSE
		fprintf(stderr,
			"Stag implications: Oldtag=%s, Newtag=%s\n",
			context->element_stack->tag->name,
			newtag);
#endif
		closeImpliedTags(context, t);
	    }


	    /*  Clear out attributes
	    */
	    
	    {
	        int i;
	        for (i=0; i< context->current_tag->number_of_attributes; i++)
	    	    context->present[i] = NO;
	    }
	    string->size = 0;
	    context->current_attribute_number = INVALID;
	    
	    if (c=='>') {
		if (context->current_tag->name) {/*PYW*/
			start_element(context);
			if (context->current_tag->contents == SGML_EMPTY) {
			  close_element(context, context->current_tag);
			}
		}
		context->state = S_text;
	    } else {
	        context->state = S_tag_gap;
	    }
	}
	break;

		
    case S_tag_gap:		/* Expecting attribute or > */
	if (WHITE(c)) break;	/* Gap between attributes */
	if (c=='>') {		/* End of tag */
	    if (context->current_tag->name) {
		start_element(context);
		if (context->current_tag->contents == SGML_EMPTY) {
		  close_element(context, context->current_tag);
		}
	    }
	    context->state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	context->state = S_attr;		/* Get attribute */
	break;
	
   				/* accumulating value */
    case S_attr:
	if (WHITE(c) || (c=='>') || (c=='=')) {		/* End of word */
	    HTChunkTerminate(string) ;
	    handle_attribute_name(context, string->data);
	    string->size = 0;
	    if (c=='>') {		/* End of tag */
	        if (context->current_tag->name) { /*PYW*/
		  start_element(context);
		  if (context->current_tag->contents == SGML_EMPTY) {
		    close_element(context, context->current_tag);
		  }
	        }
		context->state = S_text;
		break;
	    }
	    context->state = (c=='=' ?  S_equals: S_attr_gap);
	} else {
	    HTChunkPutc(string, c);
	}
	break;
		
    case S_attr_gap:		/* Expecting attribute or = or > */
	if (WHITE(c)) break;	/* Gap after attribute */
	if (c=='>') {		/* End of tag */
	    if (context->current_tag->name) { /*PYW*/
		start_element(context);
		if (context->current_tag->contents == SGML_EMPTY) {
		  close_element(context, context->current_tag);
		}
	    }
	    context->state = S_text;
	    break;
	} else if (c=='=') {
	    context->state = S_equals;
	    break;
	}
	HTChunkPutc(string, c);
	context->state = S_attr;		/* Get next attribute */
	break;
	
    case S_equals:			/* After attr = */ 
	if (WHITE(c)) break;	/* Before attribute value */
	if (c=='>') {		/* End of tag */
	    if (TRACE) fprintf(stderr, "SGML: found = but no value\n");
	    if (context->current_tag->name) start_element(context);
	    context->state = S_text;
	    break;
	    
	} else if (c=='\'') {
	    context->state = S_squoted;
	    break;

	} else if (c=='"') {
	    context->state = S_dquoted;
	    break;
	}
	HTChunkPutc(string, c);
	context->state = S_value;
	break;
	
    case S_value:
	if (WHITE(c) || (c=='>')) {		/* End of word */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(context, string->data);
	    string->size = 0;
	    if (c=='>') {		/* End of tag */
		if (context->current_tag->name) {/*PYW*/
		  start_element(context);
		  if (context->current_tag->contents == SGML_EMPTY)
		    close_element(context, context->current_tag);
		}
		context->state = S_text;
		break;
	    }
	    else context->state = S_tag_gap;
	} else {
	    HTChunkPutc(string, c);
	}
	break;
		
    case S_squoted:		/* Quoted attribute value */
	if (c=='\'') {		/* End of attribute value */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(context, string->data);
	    string->size = 0;
	    context->state = S_tag_gap;
	} else {
	    HTChunkPutc(string, c);
	}
	break;
	
    case S_dquoted:		/* Quoted attribute value */
	if (c=='"') {		/* End of attribute value */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(context, string->data);
	    string->size = 0;
	    context->state = S_tag_gap;
	} else {
	    HTChunkPutc(string, c);
	}
	break;
	
    case S_end:					/* </ */
	if (isalnum(c))
	    HTChunkPutc(string, c);
	else {				/* End of end tag name */
	    HTTag * t;
	    HTChunkTerminate(string) ;
	    if (!*string->data)	{	/* Empty end tag */
	        t = context->element_stack->tag;
	    } else {
		t = SGMLFindTag(dtd, string->data);
	    }
	    if (!t) {
		if(TRACE) fprintf(stderr,
		    "Unknown end tag </%s>\n", string->data); 
	    } else {
	        if (context->current_tag) {
		  char *prevTag, *closeTag;
		  int implyETagP = 1;
		  char *oldTagName;
		  HTElement *upper;  /* bottom-1 item on stack */
		  HTElement *bottom; /* bottom item of stack */
		  int bottomTagID;
		  int *subep;

		  closeTag = t->name;

		  prevTag = context->current_tag->name;
#ifdef IMPLY_VERBOSE
		    if (context->element_stack && 
			context->element_stack->tag) {
		      fprintf(stderr,
			      "Etag implications: Oldtag=%s, Endtag=%s\n",
			      context->element_stack->tag->name,
			      closeTag);
		    }
#endif
		  /* don't try to imply closing tag if oldTag==endTag
		   */
		  if ((context->element_stack && context->element_stack->tag)
		      && (context->element_stack->tag != t))
		    closeImpliedTags(context, t);
	        }
	        context->current_tag = t;
		end_element( context, context->current_tag);
	    }

	    string->size = 0;
	    context->current_attribute_number = INVALID;
	    if (c!='>') {
		if (TRACE && !WHITE(c))
		    fprintf(stderr,"SGML:  `</%s%c' found!\n",
		    	string->data, c);
		context->state = S_junk_tag;
	    } else {
	        context->state = S_text;
	    }
	}
	break;

		
    case S_junk_tag:
	if (c=='>') {
	    context->state = S_text;
	}
	
    } /* switch on context->state */

}  /* SGML_character */


PUBLIC void SGML_string ARGS2(HTStream *, context, CONST char*, str)
{
    CONST char *p;
    for(p=str; *p; p++)
        SGML_character(context, *p);
}


PUBLIC void SGML_progress ARGS2(HTStream *, context, int, l)
{
#ifdef VIOLA  /* KLUDGE ALERT */
  http_progress_notify(l);
#endif
}


PUBLIC void SGML_write ARGS3(HTStream *, context, CONST char*, str, int, l)
{
    CONST char *p;
    CONST char *e = str+l;
    for(p=str; p<e; p++)
        SGML_character(context, *p);
}

/*_______________________________________________________________________
*/

/*	Structured Object Class
**	-----------------------
*/
PUBLIC CONST HTStreamClass SGMLParser = 
{		
	"SGMLParser",
	SGML_free,
	SGML_end,
	SGML_abort,
	SGML_character, 
	SGML_string,
	SGML_progress,
	SGML_write
}; 

/*	Create SGML Engine
**	------------------
**
** On entry,
**	dtd		represents the DTD, along with
**	actions		is the sink for the data as a set of routines.
**
*/

PUBLIC HTStream* SGML_new  ARGS2(
	CONST SGML_dtd *,	dtd,
	HTStructured *,		target)
{
    int i;
    HTStream* context = (HTStream *) malloc(sizeof(*context));
    if (!context) outofmem(__FILE__, "SGML_begin");

    context->isa = &SGMLParser;
    context->string = HTChunkCreate(128);	/* Grow by this much */
    context->dtd = dtd;
    context->target = target;
    context->actions = (HTStructuredClass*)(((HTStream*)target)->isa);
    					/* Ugh: no OO */
    context->state = S_text;
    context->element_stack = 0;			/* empty */
#ifdef CALLERDATA		  
    context->callerData = (void*) callerData;
#endif    
    for(i=0; i<MAX_ATTRIBUTES; i++) context->value[i] = 0;

    return context;
}

