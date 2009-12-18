/*
 * Most of this code is Jon's fault.
 *
 * Basically, it's trying to be a real fast lexical analyzer.
 *
 */


#define TEST


#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "ytabh"
#include "violakeywords.h"

char getc_buf[BUFSIZ + 1];
static char *getc_ptr = getc_buf + 1;

enum { FROMSTRING, FROMFILE };

#define lexHandleError(s) fputs(s, stdout)

extern int verbose;

/*
 * Here should be all the really viola-specific stuff about the lexer.
 * General lex stuff should go later.
 *
 */

#define TRUE 1
#define FALSE 0
#include "ast.h"
#include "math.h"

#include "utils.h"
#include "hash.h"
#include "mystrings.h"
#include "ident.h"

char scanComment();

#define strdup saveString
#define io_getc() (*getc_ptr++)
#define io_ungetc(c) (--getc_ptr)

#define input io_getc
#define unput io_ungetc

int identStrID = STR_LAST_OF_THE_PREDEFINED; 
int varStrID = 0;

/* 
 * Stores identifier in hashtable
 */
int storeIdent(identStr)
	char *identStr;
{
	HashEntry *entry;

	if (!(entry = symStr2ID->get(symStr2ID, (long)identStr))) {
		entry = symStr2ID->put(symStr2ID,(long)identStr,++identStrID);
		symID2Str->put(symID2Str, identStrID, (long)identStr);
	}

	return entry->val;
}

int getIdent(identStr)
        char *identStr;
{
        HashEntry *entry;

        if (entry = symStr2ID->get(symStr2ID, (long)identStr))
	  return entry->val;

        return NULL;
}

int tokenize(identStr)
	char *identStr;
{
	HashEntry *entry;
	char *cp;

	if (!(entry = symStr2ID->get(symStr2ID, (long)identStr))) {
		cp = saveString(identStr);
		entry = symStr2ID->put(symStr2ID, (long)cp, ++identStrID);
		symID2Str->put(symID2Str, identStrID, (long)cp);
	}
	return entry->val;
}

/*
 * Scan for integer, maximum of n digits.
 * Return integer value.
 */
int scanIntNDigits(n)
        int n;
{
        int i = 0;
        char c;
        char buf[BUFSIZ];

	while (isdigit(c = input()) && (n-- > 0)) buf[i++] = c;
        unput(c);
	buf[i] = '\0';
	i = (int)atoi(buf);

	return i;
}

/*
 * Convert a "back-slash code" character representation to a character
 * i.e. "a" or "\n" or "\253"
 */
char convertEscChar()
{
	char c;
/*	char escbuf[4];
	int i;*/

	if (input() == '\\') {  /* deal with escape character */
		switch (c = input()) {
		case 'n': 
			return '\n'; 	/* newline */
		case 't':
			return '\t'; 	/* horizontal tab */
		case 'v': 
			return '\v'; 	/* vertal tab */
		case 'b': 
			return '\b'; 	/* backspace */
		case 'r': 
			return '\r';	/* carriage return */
		case 'f': 
			return '\f'; 	/* form feed */
		case '\\': 
			return '\\'; 	/* backslash */
		case '\'': 
			return '\''; 	/* single quote */
		case '\"': 
			return '\"';	/* double quote */
		default:
			unput(c);	/* number after '\' is the ascii # */
/*
			i = 0;
			while (isdigit(c) && (i < 3)) {
			    escbuf[i++] = c;
			    c = input();
			}
			if (i < 3) unput(c);
			escbuf[i] = '\0';
		 	return atoi(escbuf);
*/
			return (char)scanIntNDigits(3); 
		}
	}
	return c;
}

/*
 * Scan a character string, enclosed in two delimeter chars, into yytext
 * and return token and semantic value.
 * Converts back-slash representations to char. ie. "\n" => '\n'
 * NOTE: ascii codes must be three digits, unless it is at the end of 
 * the string, or the next character is anything other than a digit.
 * example: "\65B" => "AB"   "\0652" => "A2"   "\65\66" => "AB"
 */
char *scanStringBlock(delimeter)
	char delimeter;
{
	char c;
	short i=0;
	static char yytext[BUFSIZ];

L:	if ((c = input()) == '\\') {
		unput(c);
		yytext[i++] = convertEscChar();
		goto L;
	} else if (c == delimeter || c == '\0') {
		yytext[i] = '\0';
		return yytext;
	} else {
		yytext[i++] = c;
	}
	goto L;
}

/*
 * Scan a character constant into yytext
 * and return token and semantic value.
 */
int scanChar()
{
        char *s;

	s = scanStringBlock('\''); /* extract and convert the escape-char */
	yylval.c = *s; /* representation */

	return CHARCONST;
}

/*
 * Scan a character string into yytext, store it in string table,
 * and return token and semantic value.
 */
int scanString()
{
	char *s;

	s = scanStringBlock('\"');	/* process escape characters */
	yylval.s = SaveString(s);       /* should use headp... */

	return STRINGCONST;
}

/*
 * Scan an identifier.
 */
int scanIdentifier(sp)
	char *sp;
{
	HashEntry *entry;

	/* store in symbolic table */
	yylval.i = storeIdent(SaveString(sp));
	return IDENT;
}

int dumpProximityErrorLine(buff, i)
        char *buff;
        int *i;
{
        char c, j;

        for (j = 0; c = yyscript[(*i)++]; j++) {
                if (c == '\n') {
                        buff[j] = '\0';
                        return 1;
                }
                buff[j] = c;
        }
        buff[j] = '\0';
        return 0;
}

reportError()
{
	int i = 0, ln = 0, hasmore = 1;
	char c;

	if (yyscriptcontext == SCRIPT_ORIGIN_OBJ_SCRIPT) {
		fprintf(stderr, 
			"error at line %d... (obj=%s, in script)\n", 
			lineno, yyobjcontext);
	} else if (yyscriptcontext == SCRIPT_ORIGIN_CLASS_SCRIPT) {
		fprintf(stderr,
			"error at line %d... (obj=%s, in classScript)\n", 
			lineno, yyobjcontext);
	} else {
		fprintf(stderr,
			"error at line %d... (obj=%s, in eval)\n", 
			lineno, yyobjcontext);
 		fprintf(stderr, 
			"script = {%s}\n", 
			yyscript);
	}

	for (;;) {
		c = yyscript[i];
		if (c == '\0') return;
		if (c == '\n') {
			ln++;
			if (ln >= lineno - 2) break;
		}
		i++;
	}

	hasmore = dumpProximityErrorLine(buff, &i);
	printf(" %d:\t%s\n", ln++, buff);
	if (!hasmore) return;
	hasmore = dumpProximityErrorLine(buff, &i);
	printf(" %d:\t%s\n", ln++, buff);
	if (!hasmore) return;
	hasmore = dumpProximityErrorLine(buff, &i);
	printf("*%d:\t%s\n", ln++, buff);
	if (!hasmore) return;
	hasmore = dumpProximityErrorLine(buff, &i);
	printf(" %d:\t%s\n", ln++, buff);
	if (!hasmore) return;
	hasmore = dumpProximityErrorLine(buff, &i);
	printf(" %d:\t%s\n", ln++, buff);
	if (!hasmore) return;

}



/* 
 * 
 * From here on should be mostly stuff that is generically lexical-
 * analyzerish.
 *
 */

char scratchbuf[BUFSIZ];
FILE *datastream;

#ifdef TEST

YYSTYPE yylval;
/*#define scanString grabstring*/

#define make_strn alloc_stringn
    
#endif /* TEST */


char *alloc_string(s)
    char *s;
{
	char *t = (char *)malloc(strlen(s) + 1);
	assert (t != NULL);
	strcpy(t, s);
	return t;
}

char *alloc_stringn(s, l)
    char *s;
    int l;
{
	char *t = (char *)malloc(l + 1);
	assert(t != NULL);
	bcopy(s, t, l);
	t[l] = '\0';
	return t;
}

char *realloc_strcat(s, t, l, m)
    char *s;
    char *t;
    int l;
    int m;
{
	s = (char *)realloc(s, l + m + 1);
	bcopy(t, s + l, m);
	s[m + l] = '\0';
	return s;
}

int lineno;
int lexsource;
char *langstring;

#ifdef ANSI
void clear_lexbuf(void);
void activate_symbol_intercept(void);

static int grabident(char c);
static int grabstring();
static int grabop(char c);
static int grabint(char c);
static int iskeyword(char *name);
static int refresh_buf(void);
#endif /* ANSI */


static char *translate_token(token)
    int token;
{
        static char buf[80];
	if (token == 0) {
	    sprintf(buf, "END-OF-INPUT");
	} else
	if (token < 256) {
	    sprintf(buf, "'%c'", token);
	} else if (token == STRING) {
	    sprintf(buf, "a string");
	} else
	if (token == INTCONST) {
	    sprintf(buf, "the integer %d", yylval.i);
	} else if (token == FLOATCONST) {
	    sprintf(buf, "the float %f", yylval.f);
	} else sprintf(buf, "token %d", token);
	return buf;
}

#ifdef NOT
int yyerror(s)
    char *s;
{
/*	extern int yychar;*/
	char buf[BUFSIZ];
	unsigned int chars;

	sprintf(buf, "Parse error near line %d, character %d.", lineno,
		chars);
	lexHandleError(buf);

/*	sprintf(buf, "Offending token: %d.", yychar);*/
	lexHandleError(buf);

	return 1;
}
#endif /* NOT */

#define TRIEMAX 128
typedef void **trielevel;   /* TRIEMANIA */

trielevel trietop;

/*
 * We use element #0 of a level of the trie to store the value of the token
 * that we can accept here, or NULL if no such.  This, of course, assumes
 * that character #0 is never going to exist in one of our keywords.
 *
 */

static unsigned int numtries = 0;
static trielevel newlevel() {
    unsigned int i = 0;
    trielevel l;

    l = (trielevel)malloc(sizeof(void *) * TRIEMAX);
    while (i < TRIEMAX) l[i++] = NULL;
    numtries++;
    return l;
}

int init_scanutils() {
    unsigned int i;
    void **tp, **tq;
    char *s;
    char c;

    /*
     * Fill trie.
     *
     */

    trietop = newlevel();

    i = 0;
    while (i < PRIMSYMS) {
        s = primstuff[i].name;
	tp = trietop;
	while (*s) {
	    c = *s;
	    if (tp[c] != NULL) tq = (void **)(tp[c]);
	    else {
	        tq = newlevel();
		tp[c] = tq;
	    }
	    if (islower(c)) tp[toupper(c)] = tp[c];
	    tp = tq;
	    s++;
	}
	tp[0] = (void *)(primstuff[i].token);
        i++;
    }
    if (verbose) printf("Built trie with %d levels (%d bytes.)\n", numtries,numtries*128*4);

    return 1;
}

void clear_lexbuf() {
        lineno = 1;
	refresh_buf();
}

void skip_hashbang() {
        if ((*(getc_ptr) == '#') && (*(getc_ptr + 1) == '!')) {
	    getc_ptr += 2;
	    while (*getc_ptr && (*getc_ptr != '\n')) getc_ptr++;
	}
}

static int refresh_buf() {
    switch(lexsource) {
	case FROMFILE:
            getc_buf[0] = *(getc_ptr - 1);

         /*
	  * This is still okay to do even if getc_ptr is at the beginning
	  * of a new buffer (i.e. it == getc_buf + 1) since we will then
          * just copy the first byte of getc_buf[] into itself.  Since in
          * this case it is erroneous to ever io_unget() a character anyway,
          * not having read any yet, we don't care if this data is
          * meaningless.
	  *
	  * Addendum: If we're reading from a string and it's empty
	  *           then (getc_buf - 1) will be a pointer into unknown
	  *           data and using that data would invoke havoc.  However,
	  *           we will never use this data since an empty line
	  *           can only be encountered between tokens and we will
	  *           never ungetc between tokens.  Utilities like purify
	  *           and saber (codecenter, bleah) will choke on this,
	  *           reporting it as an error, when really it does no
	  *           damage.  The way to fix this would be to copy all
	  *           of the memory-resident string that we're parsing
	  *           into another buffer that has an extra character
	  *           of space at the beginning, but this sort of garbage
	  *           is just what we moved away from lex in order to
	  *           avoid.
          *
	  *  Addendum to addendum: NEVER MIND!  getc_buf is not used
	  *                        when we're reading from memory resident
	  *                        string any more, only from file, so
	  *                        the above "Addendum" paragraph is
	  *                        pointless.
	  *
          * BTW, the whole point of character 0 of getc_buf is so that we
          * can io_unget() with impunity across buffer boundaries.
          *
          */

            getc_ptr = getc_buf + 1;
	    if (fgets(getc_ptr, BUFSIZ - 1, datastream) == NULL) {
		getc_buf[0] = getc_buf[1] = '\0';
		return EOF;
	    }
	    break;
	case FROMSTRING:
	    if (lineno > 1) return EOF;
	    lineno++;
	    if (*langstring == '\0') return EOF;
	    getc_ptr = langstring;
	    /*
	     * This is okay since we will never have a buffer boundary
	     * when reading from a language-contained string, since we
	     * already have the whole thing sequentially in memory.  So,
	     * we will therefore never have to io_unget() across a
	     * boundary.
	     *
	     */
	    break;
	default:
	    assert (0);
    }
    return 0;
}

int yylex() {
	char c;
yypoint:
	switch(c = io_getc()) {
	case '\n':
	        lineno++;
	case ' ':
	case '\t':
	case '\r':
	case '\f':
	case '\b':
		goto yypoint;
	case '\0':
	        if (refresh_buf() != EOF) goto yypoint;
	case EOF:
		return EOF;
	case '.':
	case ':':
	case ',':
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
	case ';':
	case '\\':
		return c;
	case '=':
	case '!':
	case '<':
	case '>':
	case '+':
	case '-':
	case '%':
	case '*':
	case '/':
	case '&':
	case '|':
		return grabop(c);
	case '"':
/*		return grabstring();*/
		return scanString();
	case '\'':
		return scanChar();
        case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return grabint(c);
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
		return grabtag(c);
	default:
		return c;
	}
}

#ifdef ANSI
extern void *is_inlib(char *s);
#endif /* ANSI */

static int grabtag(c)
    char c;
{
	char buf[BUFSIZ];
	int i = 0, j;
	void *q;

	while (c && (i < BUFSIZ - 1)) {
	    if (isalnum(c) || (c == '_') || (c == '.')) {
	        buf[i++] = c;
		c = io_getc();
	    } else break;
	}

	io_ungetc(c);
	buf[i] = '\0';

	switch (j = iskeyword(buf)) {
	    default:
	        yylval.i = j;
		return j;
	    case SPECIAL_TRUE:
		yylval.i = 1;
		return INTCONST;
	    case SPECIAL_FALSE:
		yylval.i = 0;
		return INTCONST;
	    case 0: 
	        break;
	}

    nospec:
	return scanIdentifier(buf);
}

#define BIGBUFSIZ 2*BUFSIZ
static int grabstring() {
    char buf[BIGBUFSIZ];
    static int have_allocated = 0;
    char c;
    int i = 0;
    char *product;
    int total_len = 0;

    while (1) {
	while (i < BIGBUFSIZ - 1) {
	  strpt1:
	    switch(c = io_getc()) {
		case '\0':
		    if (refresh_buf() == EOF) goto endstr; /* UNT STRING */
		    goto strpt1;
		case '"':
		    goto endstr;
		case '\n':
		    lineno++;
		  gross:
		    c = io_getc();
		    switch(c) {
		        case '\0':
		            if (refresh_buf() == EOF) goto endstr;
			    goto gross;
		        case ' ':
		        case '\t':
		            goto gross;
			default:
			    io_ungetc(c);
			    c = ' ';
			    break;
		    }
		    break;
		case '\\':
		  strpt2:
		     switch(c = io_getc()) {
		       case 'n':
			 c = '\n';
			 break;
		       case 't':
			 c = '\t';
			 break;
		       case '\0':
			 if (refresh_buf() == EOF) {
			     buf[i++] = '\\';
			     goto endstr;
			 }
			 goto strpt2;
		     }
		}
		buf[i++] = c;
	}

    endstr:
	if (i == 0) {
	        if (total_len) {
		    yylval.s = make_strn(product, total_len);
		    free(product);
		} else yylval.s = strdup("");
		return STRING;

	}

	if ((total_len == 0) && (i < BIGBUFSIZ - 1)) {
		yylval.s = make_strn(buf, i);
		return STRING;
	}

	if (total_len) product = realloc_strcat(product, buf, total_len, i);
	    else product = alloc_stringn(buf, i);

	total_len += i;
	i = 0;
    }
}

static int snarf_comment2() {
    int c;

    loophead:
	c = io_getc();
    loop2:
	switch (c) {
	    case 0:
	        if (refresh_buf() == EOF) goto loopdone;
		goto loophead;
	    case '\n':
		lineno++;
		goto loophead;
	    case '*':
		c = io_getc();
		if (c == '/') return yylex();
		goto loop2;
	}
    goto loophead;

  loopdone:
    io_ungetc(c);
    return yylex();
}

static int grabop(c)
    char c;
{
	int i;
	trielevel tp;

	tp = trietop;
	while (c) {
	    if (tp[c]) {
/*	        printf("Take %c\n", c);*/
    	        tp = (void **)tp[c];
	    } else if (tp[0]) {         /* Illegal char, but accept */
	        io_ungetc(c);
/*	        printf("Accept %c\n", c);*/
	        i = (unsigned long)tp[0];
		if (i == SPECIAL_COMMENT) return snarf_comment2();
		return (yylval.i = i);
	    } else {                    /* Illegal char, no accept */
	        sprintf(scratchbuf, "Illegal operator character '%c'.\n", c);
/*		fprintf(stderr, "^^^^^^^^%s", scratchbuf);*/
	        lexHandleError(scratchbuf);
	        return (yylval.i = c);
	    }
	    c = io_getc();
	}
	return (yylval.i = (unsigned int)tp[0]);
}

#define MAXNLEN 38
static int grabint(c)
    char c;
{
    char intbuf[MAXNLEN + 2];
    unsigned int floating_pointp = 0;
    int ilen = 0;

    while (ilen < MAXNLEN) {
	if (c == '.') goto readfloat;
	intbuf[ilen++] = c;
	c = io_getc();
	if (!isdigit(c) && (c != '.')) goto donereading;
    }

  readfloat:
    floating_pointp = 1;
    while (ilen < MAXNLEN) {
	intbuf[ilen++] = c;
	c = io_getc();
	if (!isdigit(c)) goto donereading;
    }

  donereading:
    intbuf[ilen] = '\0';
    if (ilen != MAXNLEN) io_ungetc(c);

    if (floating_pointp) {
        yylval.f = atof(intbuf);
	return FLOATCONST;
    }
    yylval.i = atoi(intbuf);
    return INTCONST;
}

static int iskeyword(name)
    char *name;
{
	int index = 0;
	trielevel tb = trietop;

	while (*name) {
	    if (tb[*name]) {
	        tb = (trielevel)tb[*name];
	    } else return 0;
	    name++;
	}
	return (unsigned long)tb[0];
}

void set_parsing_string(s)
    char *s;
{
    lexsource = FROMSTRING;
    langstring = s;
    clear_lexbuf();
}

void parse_file(filename) 
    char *filename;
{
  /*
   * Do some file opening stuff
   */
    lexsource = FROMFILE; 
    datastream = fopen(filename, "r");
    clear_lexbuf();
    skip_hashbang();
    yyparse();
}
