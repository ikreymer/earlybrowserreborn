/*			General SGML Parser code		SGML.c
**			========================
*/
#include <ctype.h>
#include <stdio.h>
#include "HTUtils.h"
#include "HTChunk.h"
#include "SGML.h"

#include "tcp.h"	/* For TOUPPER  ! */

static void 		(*contents_treatment) PARAMS((char c));
static HTTag 		*current_tag;
static attr 		*current_attribute;
static HTChunk		*string =0;
static HTElement	*element_stack;

/*	Handle Attribute
**	----------------
*/
PUBLIC CONST char * SGML_default = "";

#ifdef __STDC__
PRIVATE void handle_attribute_name(const char * s)
#else
PRIVATE void handle_attribute_name(s)
    char *s;
#endif
{
    for(    current_attribute = current_tag->attributes;
	    current_attribute->name;
	    current_attribute++) {
	if (0==strcasecomp(current_attribute->name, s))
	    break;
    }
    if (!current_attribute->name) {
	if (TRACE)
	    fprintf(stderr, "SGML: Unknown attribute %s for tag %s\n",
		s, current_tag->name);
        current_attribute = 0;	/* Invalid */
	return;
    }
    current_attribute->present = YES;
    if (current_attribute->value) {
        free(current_attribute->value);
	current_attribute->value = 0;
    }
}

/*	Handle attribute value
**	----------------------
*/
#ifdef __STDC__
PRIVATE void handle_attribute_value(const char * s)
#else
PRIVATE void handle_attribute_value(s)
    char *s;
#endif
{
    if (current_attribute) {
	StrAllocCopy(current_attribute->value, s);
    } else {
        if (TRACE) fprintf(stderr, "SGML: Attribute value %s ignored\n", s);
    }
    current_attribute = 0;	/* can't have two assignments! */
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
PRIVATE void handle_entity(const char * s, entity * entities, char term)
#else
PRIVATE void handle_entity(s,entities, term)
    char * s;
    entity * entities;
    char term;
#endif
{
    entity * e;
    for(e = entities; e->name; e++) {
	if (0==strcmp(e->name, s)) {
	    char * p;
	    for (p=e->representation; *p; p++) {
		(*contents_treatment)(*p);
	    }
	    return;	/* Good */
	}
    }
    /* If entity string not found, display as text */
    if (TRACE)
	fprintf(stderr, "SGML: Unknown entity %s\n", s); 
    (*contents_treatment)('&');
    {
	CONST char *p;
	for (p=s; *p; p++) {
	    (*contents_treatment)(*p);
	}
    }
    (*contents_treatment)(term);
}

/*	End element
*/
#ifdef __STDC__
PRIVATE void end_element(HTTag * old_tag)
#else
PRIVATE void end_element(old_tag)
    HTTag * old_tag;
#endif
{
    if (TRACE) fprintf(stderr, "SGML: End   </%s>\n", old_tag->name);
    if (!old_tag->end) {
        if (TRACE) fprintf(stderr,"SGML: Illegal end tag </%s> found.\n",
		old_tag->name);
	return;
    }
    while (element_stack) 	{/* Loop is error path only */
	HTElement * N = element_stack;
	HTTag * t = element_stack->tag;
	
	if (old_tag != t) {		/* Mismatch: syntax error */
	    if (element_stack->next) {	/* This is not the last level */
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
	
	element_stack = N->next;		/* Remove from stack */
	free(N);
	(t->end)(t, element_stack);		/* Assume tag end */
	if (element_stack)			/* not end of document */
	    contents_treatment = element_stack->tag->treat;
	if (old_tag == t) return;  /* Correct sequence */
	
	/* Syntax error path only */
	
    }
    fprintf(stderr,
	"SGML: Extra end tag </%s> found and ignored.\n", old_tag->name);
}


/*	Start a element
*/
#ifdef __STDC__
PRIVATE void start_element(HTTag * new_tag)
#else
PRIVATE void start_element(new_tag)
    HTTag * new_tag;
#endif
{
    if (TRACE) fprintf(stderr, "SGML: Start <%s>\n", new_tag->name);
    (*new_tag->begin)(new_tag, element_stack);
    if (new_tag->end) {		/* i.e. tag not empty */
	HTElement * N = (HTElement *)malloc(sizeof(HTElement));
        if (N == NULL) outofmem(__FILE__, "start_element");
	N->next = element_stack;
	N->tag = new_tag;
	element_stack = N;
	contents_treatment = new_tag->treat;
    }
}



/*	SGML Engine
**	-----------
**
** On entry,
**	dtd->tags		represents the DTD, along with
**	dtd->entities
**
**	default_tag	represents the initial and final actions,
**			and the character processing, for data outside
**			any tags. May not be empty.
*/

PRIVATE enum sgml_state { S_text, S_litteral, S_tag, S_tag_gap, 
		S_attr, S_attr_gap, S_equals, S_value,
		  S_quoted, S_end, S_entity, S_junk_tag} state;

PUBLIC void SGML_begin  ARGS1(SGML_dtd *,dtd)
{
    if (!string) string = HTChunkCreate(128);	/* Grow by this much */
    
    state = S_text;
    start_element(dtd->default_tag);	/* Start document */
}

PUBLIC void SGML_end  ARGS1(SGML_dtd *,dtd)
{
    end_element(dtd->default_tag);	/* End document */
}

PUBLIC void SGML_character ARGS2(SGML_dtd *,dtd, char,c)

{
    switch(state) {
    case S_text:
	if (c=='&' && !(element_stack &&
	    		element_stack->tag  &&
	    		element_stack->tag->litteral)) {
	    string->size = 0;
	    state = S_entity;
	    
	} else if (c=='<') {
	    string->size = 0;
	    state = (element_stack &&
	    		element_stack->tag  &&
	    		element_stack->tag->litteral) ?
	    			S_litteral : S_tag;
	} else (*contents_treatment)(c);
	break;

/*	In litteral mode, waits only for specific end tag!
*/
    case S_litteral :
	HTChunkPutc(string, c);
	if ( TOUPPER(c) != ((string->size ==1) ? '/'
		: element_stack->tag->name[string->size-2])) {
	    int i;
	    
	    /*	If complete match, end litteral */
	    if ((c=='>') && (!element_stack->tag->name[string->size-2])) {
		end_element(element_stack->tag);
		string->size = 0;
		current_attribute = (attr *) 0;
		state = S_text;
		break;
	    }		/* If Mismatch: recover string. */
	    (*contents_treatment)('<');
	    for (i=0; i<string->size; i++)	/* recover */
	       (*contents_treatment)(string->data[i]);
	    state = S_text;	
	}
	
        break;
	
/*	Handle Entities
*/
    case S_entity:
	if (isalnum(c))
	    HTChunkPutc(string, c);
	else {
	    HTChunkTerminate(string);
	    handle_entity(string->data, dtd->entities, c);
	    state = S_text;
	}
	break;
	
/*		Tag
*/	    
    case S_tag:				/* new tag */
	if (isalnum(c))
	    HTChunkPutc(string, c);
	else {				/* End of tag name */
	    attr * a;
	    if (c=='/') {
		if (TRACE) if (string->size!=0)
		    fprintf(stderr,"SGML:  `<%s/' found!\n", string->data);
		state = S_end;
		break;
	    }
	    HTChunkTerminate(string) ;
	    for(current_tag = dtd->tags; current_tag->name; current_tag++) {
		if (0==strcasecomp(current_tag->name, string->data)) {
		    break;
		}
	    }
	    if (!current_tag->name) {
		if(TRACE) fprintf(stderr, "Unknown tag %s\n",
			string->data);
		state = (c=='>') ? S_text : S_junk_tag;
		break;
	    }
	    
	    for (a = current_tag->attributes; a->name; a++ ) {
		a->present = NO;
	    }
	    string->size = 0;
	    current_attribute = (attr *) 0;
	    
	    if (c=='>') {
		if (current_tag->name) start_element(current_tag);
		state = S_text;
	    } else {
	        state = S_tag_gap;
	    }
	}
	break;

		
    case S_tag_gap:		/* Expecting attribute or > */
	if (WHITE(c)) break;	/* Gap between attributes */
	if (c=='>') {		/* End of tag */
	    if (current_tag->name) start_element(current_tag);
	    state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	state = S_attr;		/* Get attribute */
	break;
	
   				/* accumulating value */
    case S_attr:
	if (WHITE(c) || (c=='>') || (c=='=')) {		/* End of word */
	    HTChunkTerminate(string) ;
	    handle_attribute_name(string->data);
	    string->size = 0;
	    if (c=='>') {		/* End of tag */
		if (current_tag->name) start_element(current_tag);
		state = S_text;
		break;
	    }
	    state = (c=='=' ?  S_equals: S_attr_gap);
	} else {
	    HTChunkPutc(string, c);
	}
	break;
		
    case S_attr_gap:		/* Expecting attribute or = or > */
	if (WHITE(c)) break;	/* Gap after attribute */
	if (c=='>') {		/* End of tag */
	    if (current_tag->name) start_element(current_tag);
	    state = S_text;
	    break;
	} else if (c=='=') {
	    state = S_equals;
	    break;
	}
	HTChunkPutc(string, c);
	state = S_attr;		/* Get next attribute */
	break;
	
    case S_equals:			/* After attr = */ 
	if (WHITE(c)) break;	/* Before attribute value */
	if (c=='>') {		/* End of tag */
	    fprintf(stderr, "SGML: found = but no value\n");
	    if (current_tag->name) start_element(current_tag);
	    state = S_text;
	    break;
	    
	} else if (c=='"') {
	    state = S_quoted;
	    break;
	}
	HTChunkPutc(string, c);
	state = S_value;
	break;
	
    case S_value:
	if (WHITE(c) || (c=='>')) {		/* End of word */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(string->data);
	    string->size = 0;
	    if (c=='>') {		/* End of tag */
		if (current_tag->name) start_element(current_tag);
		state = S_text;
		break;
	    }
	    else state = S_tag_gap;
	} else {
	    HTChunkPutc(string, c);
	}
	break;
		
    case S_quoted:			/* Quoted attribute value */
	if (c=='"') {		/* End of attribute value */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(string->data);
	    string->size = 0;
	    state = S_tag_gap;
	} else {
	    HTChunkPutc(string, c);
	}
	break;
	
    case S_end:					/* </ */
	if (isalnum(c))
	    HTChunkPutc(string, c);
	else {				/* End of end tag name */
	    HTChunkTerminate(string) ;
	    if (c!='>') {
		if (TRACE) fprintf(stderr,"SGML:  `</%s%c' found!\n",
		    string->data, c);
		state = S_junk_tag;
		break;
	    }
	    for(current_tag = dtd->tags; current_tag->name; current_tag++) {
		if (0==strcasecomp(current_tag->name, string->data)) {
		    end_element(current_tag);
		    break;
		}
	    }
	    if (!current_tag->name) {
		if(TRACE) fprintf(stderr,
		    "Unknown end tag </%s>\n", string->data); 
	    }
	    string->size = 0;
	    current_attribute = (attr *) 0;
	    state = S_text;
	}
	break;

		
    case S_junk_tag:
	if (c=='>') {
	    state = S_text;
	}
	
    } /* switch on state */

}
