%{
#ifdef i386
#define SCRIPT_FROM_FILE
#endif

#ifdef USE_FLEX
#define SCRIPT_FROM_FILE
#undef yywrap
#include "flexdef.h"
#endif

#ifdef SCRIPT_FROM_FILE
int flex_called = 0;
#endif

#define TRUE 1
#define FALSE 0
#include <stdio.h>
#include "y.tab.h"
#include "ast.h"
#include "math.h"

#include "utils.h"
#include <ctype.h>
#include "hash.h"
#include "mystrings.h"
#include "ident.h"

char scanComment();

char myc;
int lineno;

typedef union  {
	struct AST	*p;
	int		i;
	float		f;
	char		c;
	char		*s;
} YYSTYPE;
extern YYSTYPE yylval;

%}

DIGIT		[0-9]
LETTER		[A-Za-z_]
IDTAIL		[A-Za-z0-9_.]
IDENT		{LETTER}{IDTAIL}*
SPACE		[ \t\n\r\f\b]
INTEGER         {SIGN}{DIGIT}+
SIGN            [+-]?
FLOAT           {SIGN}{DIGIT}*\.{DIGIT}*

%%
\n		{ lineno++; }
"'"		return scanChar();
"\""		return scanString();
"+"		return '+';
"-"		return '-';
"%"		return '%';
"*"		return '*';
"="		return '=';
"."		return '.';
":"		return ':';
","		return ',';
"("		return '(';
")"		return ')';
"["		return '[';
"]"		return ']';
"{"		return '{';
"}"		return '}';
";"		return ';';
">"		return '>';
"<"		return '<';
"!"		return '!';
"\\"		return '\\';
"=="		return EQ;
"!="		return NE;
"<="		return LE;
">="		return GE;
"++"		return INC;
"--"		return DEC;
"+="		return PLUS_ASSERT;
"-="		return MINUS_ASSERT;
"%="		return MOD_ASSERT;
"*="		return MULT_ASSERT;
"/="		return DIV_ASSERT;
"&&"		return AND;
"||"		return OR;
"/"		{ if ((myc = scanComment()) != NULL) return myc; }
"true"		{ yylval.i = 1; return INTCONST; }
"false"		{ yylval.i = 0; return INTCONST; }
"if"		return IF;
"else"		return ELSE;
"for"		return FOR;
"do"		return DO;
"while"		return WHILE;
"switch"	return SWITCH;
"eswitch"	return ESWITCH;
"case"		return CASE;
"default"	return DEFAULT;
"break"		return BREAK;
"return"	return RETURN;
"get"		return GET;
"set"		return SET;
"persistent"	return PERSISTENT;
{IDENT}		return scanIdentifier(yytext);
{INTEGER}	{ yylval.i = scanInt(); return INTCONST; }
{FLOAT}         { yylval.f = (float)atof(yytext); return FLOATCONST; }
{SPACE}		;
.		printf("Unknown character: %.1o", yytext[0]);
%%

/*
int yywrap()

    {
    if ( --num_input_files > 0 )
        {
        set_input_file( *++input_files );
        return ( 0 );
        }

    else
        return ( 1 );
    }

setupbuf(size)
	int size;
{
	yy_current_buffer = yy_create_buffer(yyin, size);
}
*/

/*
 * Convert a "back-slash code" character representation to a character
 * i.e. "a" or "\n" or "\253"
 */
char convertEscChar()
{
	char c;

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
void scanStringBlock(delimeter)
	char delimeter;
{
	char c;
	short i=0;

L:	if ((c = input()) == '\\') {
		unput(c);
		yytext[i++] = convertEscChar();
		goto L;
	} else if (c == delimeter || c == '\0') {
		yytext[i] = '\0';
		return;
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
	scanStringBlock('\''); /* extract and convert the escape-char */
	yylval.c = yytext[0]; /* representation */

	return CHARCONST;
}

/*
 * Scan a character string into yytext, store it in string table,
 * and return token and semantic value.
 */
int scanString()
{
	scanStringBlock('\"');	/* process escape characters */
	yylval.s = SaveString(yytext); /* should use headp... */

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

/*
 * Scan for integer
 * Return integer value.
 */
int scanInt()
{
	int i = 0;
/*
	char c;

	while (isdigit(c = input())) buff[i++] = c;
	unput(c);
	buff[i] = '\0';
	i = atoi(buff);
*/
	i = (int)atoi(yytext);

	return i;
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
	while (isdigit(c = input()) && (n-- > 0)) buff[i++] = c;
	unput(c);
	buff[i] = '\0';
	i = (int)atoi(buff);

	return i;
}

/*
 * Scan for either '/' token or drow away comment line. 
 * No nested comments allowed.
 */
char scanComment()
{
	char c = yylval.c = input();

	unput(c);
	if (c != '*') return '/';
L:	if ((c = input()) == '*') goto L;
	if (c == '/') return NULL;
	while (input() != '*');
	goto L;
}

my_init_lex(fp)
	FILE *fp;
{
#ifdef USE_FLEX
	yyrestart(fp);
#endif
}
