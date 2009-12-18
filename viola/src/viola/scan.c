# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
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

# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{ lineno++; }
break;
case 2:
	return scanChar();
break;
case 3:
	return scanString();
break;
case 4:
	return '+';
break;
case 5:
	return '-';
break;
case 6:
	return '%';
break;
case 7:
	return '*';
break;
case 8:
	return '=';
break;
case 9:
	return '.';
break;
case 10:
	return ':';
break;
case 11:
	return ',';
break;
case 12:
	return '(';
break;
case 13:
	return ')';
break;
case 14:
	return '[';
break;
case 15:
	return ']';
break;
case 16:
	return '{';
break;
case 17:
	return '}';
break;
case 18:
	return ';';
break;
case 19:
	return '>';
break;
case 20:
	return '<';
break;
case 21:
	return '!';
break;
case 22:
	return '\\';
break;
case 23:
	return EQ;
break;
case 24:
	return NE;
break;
case 25:
	return LE;
break;
case 26:
	return GE;
break;
case 27:
	return INC;
break;
case 28:
	return DEC;
break;
case 29:
	return PLUS_ASSERT;
break;
case 30:
	return MINUS_ASSERT;
break;
case 31:
	return MOD_ASSERT;
break;
case 32:
	return MULT_ASSERT;
break;
case 33:
	return DIV_ASSERT;
break;
case 34:
	return AND;
break;
case 35:
	return OR;
break;
case 36:
	{ if ((myc = scanComment()) != NULL) return myc; }
break;
case 37:
	{ yylval.i = 1; return INTCONST; }
break;
case 38:
	{ yylval.i = 0; return INTCONST; }
break;
case 39:
	return IF;
break;
case 40:
	return ELSE;
break;
case 41:
	return FOR;
break;
case 42:
	return DO;
break;
case 43:
	return WHILE;
break;
case 44:
return SWITCH;
break;
case 45:
return ESWITCH;
break;
case 46:
	return CASE;
break;
case 47:
return DEFAULT;
break;
case 48:
	return BREAK;
break;
case 49:
return RETURN;
break;
case 50:
	return GET;
break;
case 51:
	return SET;
break;
case 52:
return PERSISTENT;
break;
case 53:
	return scanIdentifier(yytext);
break;
case 54:
{ yylval.i = scanInt(); return INTCONST; }
break;
case 55:
        { yylval.f = (float)atof(yytext); return FLOATCONST; }
break;
case 56:
	;
break;
case 57:
	printf("Unknown character: %.1o", yytext[0]);
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

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
int yyvstop[] = {
0,

57,
0,

56,
57,
0,

1,
56,
0,

21,
57,
0,

3,
57,
0,

6,
57,
0,

57,
0,

2,
57,
0,

12,
57,
0,

13,
57,
0,

7,
57,
0,

4,
57,
0,

11,
57,
0,

5,
57,
0,

9,
55,
57,
0,

36,
57,
0,

54,
57,
0,

10,
57,
0,

18,
57,
0,

20,
57,
0,

8,
57,
0,

19,
57,
0,

53,
57,
0,

14,
57,
0,

22,
57,
0,

15,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

53,
57,
0,

16,
57,
0,

57,
0,

17,
57,
0,

24,
0,

31,
0,

34,
0,

32,
0,

27,
0,

55,
0,

54,
0,

29,
0,

28,
0,

30,
0,

33,
0,

25,
0,

23,
0,

26,
0,

53,
0,

53,
0,

53,
0,

53,
0,

42,
53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

39,
53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

35,
0,

53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

41,
53,
0,

50,
53,
0,

53,
0,

53,
0,

51,
53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

46,
53,
0,

53,
0,

40,
53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

53,
0,

37,
53,
0,

53,
0,

48,
53,
0,

53,
0,

53,
0,

38,
53,
0,

53,
0,

53,
0,

53,
0,

43,
53,
0,

53,
0,

53,
0,

53,
0,

49,
53,
0,

44,
53,
0,

47,
53,
0,

45,
53,
0,

53,
0,

53,
0,

53,
0,

52,
53,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,4,	0,0,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,6,	1,7,	
0,0,	0,0,	1,8,	1,9,	
1,10,	1,11,	1,12,	1,13,	
1,14,	1,15,	1,16,	1,17,	
1,18,	1,19,	9,46,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,20,	
1,21,	1,22,	1,23,	1,24,	
6,44,	8,45,	1,25,	13,47,	
14,48,	18,54,	22,55,	14,49,	
23,56,	14,50,	14,50,	14,50,	
14,50,	14,50,	14,50,	14,50,	
14,50,	14,50,	14,50,	24,57,	
0,0,	0,0,	14,51,	0,0,	
0,0,	0,0,	0,0,	0,0,	
1,26,	1,27,	1,28,	0,0,	
0,0,	0,0,	0,0,	1,29,	
1,30,	1,31,	1,32,	1,33,	
1,34,	30,60,	1,35,	34,67,	
33,65,	31,61,	35,68,	36,69,	
37,70,	1,36,	32,63,	1,37,	
1,38,	1,39,	29,59,	31,62,	
1,40,	32,64,	33,66,	39,73,	
1,41,	1,42,	1,43,	2,6,	
2,7,	40,74,	42,75,	2,8,	
2,9,	2,10,	2,11,	2,12,	
2,13,	59,76,	2,15,	2,16,	
60,77,	2,18,	17,49,	17,49,	
17,49,	17,49,	17,49,	17,49,	
17,49,	17,49,	17,49,	17,49,	
2,20,	2,21,	2,22,	2,23,	
2,24,	16,52,	16,49,	61,78,	
16,50,	16,50,	16,50,	16,50,	
16,50,	16,50,	16,50,	16,50,	
16,50,	16,50,	38,71,	63,79,	
19,49,	16,53,	19,50,	19,50,	
19,50,	19,50,	19,50,	19,50,	
19,50,	19,50,	19,50,	19,50,	
64,80,	2,26,	2,27,	2,28,	
38,72,	65,81,	66,82,	67,83,	
2,29,	2,30,	2,31,	2,32,	
2,33,	2,34,	69,84,	2,35,	
70,85,	71,86,	72,87,	73,88,	
74,89,	76,90,	2,36,	77,91,	
2,37,	2,38,	2,39,	78,92,	
79,93,	2,40,	80,94,	81,95,	
84,96,	2,41,	2,42,	2,43,	
25,58,	85,97,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
87,98,	88,99,	89,100,	90,101,	
92,102,	94,103,	95,104,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	96,105,	97,106,	98,107,	
100,108,	25,58,	102,109,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	25,58,	25,58,	25,58,	
25,58,	103,110,	105,111,	106,112,	
107,113,	109,114,	110,115,	111,116,	
116,117,	117,118,	118,119,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-94,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+3,	0,		yyvstop+9,
yycrank+0,	0,		yyvstop+12,
yycrank+4,	0,		yyvstop+15,
yycrank+12,	0,		yyvstop+18,
yycrank+0,	0,		yyvstop+20,
yycrank+0,	0,		yyvstop+23,
yycrank+0,	0,		yyvstop+26,
yycrank+6,	0,		yyvstop+29,
yycrank+25,	0,		yyvstop+32,
yycrank+0,	0,		yyvstop+35,
yycrank+112,	0,		yyvstop+38,
yycrank+94,	0,		yyvstop+41,
yycrank+8,	0,		yyvstop+45,
yycrank+126,	0,		yyvstop+48,
yycrank+0,	0,		yyvstop+51,
yycrank+0,	0,		yyvstop+54,
yycrank+9,	0,		yyvstop+57,
yycrank+11,	0,		yyvstop+60,
yycrank+22,	0,		yyvstop+63,
yycrank+174,	0,		yyvstop+66,
yycrank+0,	0,		yyvstop+69,
yycrank+0,	0,		yyvstop+72,
yycrank+0,	0,		yyvstop+75,
yycrank+4,	yysvec+25,	yyvstop+78,
yycrank+8,	yysvec+25,	yyvstop+81,
yycrank+8,	yysvec+25,	yyvstop+84,
yycrank+6,	yysvec+25,	yyvstop+87,
yycrank+11,	yysvec+25,	yyvstop+90,
yycrank+6,	yysvec+25,	yyvstop+93,
yycrank+8,	yysvec+25,	yyvstop+96,
yycrank+10,	yysvec+25,	yyvstop+99,
yycrank+11,	yysvec+25,	yyvstop+102,
yycrank+69,	yysvec+25,	yyvstop+105,
yycrank+9,	yysvec+25,	yyvstop+108,
yycrank+25,	yysvec+25,	yyvstop+111,
yycrank+0,	0,		yyvstop+114,
yycrank+6,	0,		yyvstop+117,
yycrank+0,	0,		yyvstop+119,
yycrank+0,	0,		yyvstop+122,
yycrank+0,	0,		yyvstop+124,
yycrank+0,	0,		yyvstop+126,
yycrank+0,	0,		yyvstop+128,
yycrank+0,	0,		yyvstop+130,
yycrank+0,	yysvec+17,	yyvstop+132,
yycrank+0,	yysvec+19,	yyvstop+134,
yycrank+0,	0,		yyvstop+136,
yycrank+0,	0,		yyvstop+138,
yycrank+0,	0,		yyvstop+140,
yycrank+0,	0,		yyvstop+142,
yycrank+0,	0,		yyvstop+144,
yycrank+0,	0,		yyvstop+146,
yycrank+0,	0,		yyvstop+148,
yycrank+0,	yysvec+25,	yyvstop+150,
yycrank+36,	yysvec+25,	yyvstop+152,
yycrank+25,	yysvec+25,	yyvstop+154,
yycrank+57,	yysvec+25,	yyvstop+156,
yycrank+0,	yysvec+25,	yyvstop+158,
yycrank+56,	yysvec+25,	yyvstop+161,
yycrank+65,	yysvec+25,	yyvstop+163,
yycrank+81,	yysvec+25,	yyvstop+165,
yycrank+76,	yysvec+25,	yyvstop+167,
yycrank+75,	yysvec+25,	yyvstop+169,
yycrank+0,	yysvec+25,	yyvstop+171,
yycrank+84,	yysvec+25,	yyvstop+174,
yycrank+84,	yysvec+25,	yyvstop+176,
yycrank+85,	yysvec+25,	yyvstop+178,
yycrank+97,	yysvec+25,	yyvstop+180,
yycrank+86,	yysvec+25,	yyvstop+182,
yycrank+99,	yysvec+25,	yyvstop+184,
yycrank+0,	0,		yyvstop+186,
yycrank+108,	yysvec+25,	yyvstop+188,
yycrank+106,	yysvec+25,	yyvstop+190,
yycrank+114,	yysvec+25,	yyvstop+192,
yycrank+111,	yysvec+25,	yyvstop+194,
yycrank+109,	yysvec+25,	yyvstop+196,
yycrank+100,	yysvec+25,	yyvstop+198,
yycrank+0,	yysvec+25,	yyvstop+200,
yycrank+0,	yysvec+25,	yyvstop+203,
yycrank+101,	yysvec+25,	yyvstop+206,
yycrank+104,	yysvec+25,	yyvstop+208,
yycrank+0,	yysvec+25,	yyvstop+210,
yycrank+116,	yysvec+25,	yyvstop+213,
yycrank+132,	yysvec+25,	yyvstop+215,
yycrank+126,	yysvec+25,	yyvstop+217,
yycrank+128,	yysvec+25,	yyvstop+219,
yycrank+0,	yysvec+25,	yyvstop+221,
yycrank+119,	yysvec+25,	yyvstop+224,
yycrank+0,	yysvec+25,	yyvstop+226,
yycrank+121,	yysvec+25,	yyvstop+229,
yycrank+137,	yysvec+25,	yyvstop+231,
yycrank+160,	yysvec+25,	yyvstop+233,
yycrank+152,	yysvec+25,	yyvstop+235,
yycrank+168,	yysvec+25,	yyvstop+237,
yycrank+0,	yysvec+25,	yyvstop+239,
yycrank+167,	yysvec+25,	yyvstop+242,
yycrank+0,	yysvec+25,	yyvstop+244,
yycrank+162,	yysvec+25,	yyvstop+247,
yycrank+198,	yysvec+25,	yyvstop+249,
yycrank+0,	yysvec+25,	yyvstop+251,
yycrank+183,	yysvec+25,	yyvstop+254,
yycrank+189,	yysvec+25,	yyvstop+256,
yycrank+196,	yysvec+25,	yyvstop+258,
yycrank+0,	yysvec+25,	yyvstop+260,
yycrank+185,	yysvec+25,	yyvstop+263,
yycrank+198,	yysvec+25,	yyvstop+265,
yycrank+187,	yysvec+25,	yyvstop+267,
yycrank+0,	yysvec+25,	yyvstop+269,
yycrank+0,	yysvec+25,	yyvstop+272,
yycrank+0,	yysvec+25,	yyvstop+275,
yycrank+0,	yysvec+25,	yyvstop+278,
yycrank+203,	yysvec+25,	yyvstop+281,
yycrank+195,	yysvec+25,	yyvstop+283,
yycrank+190,	yysvec+25,	yyvstop+285,
yycrank+0,	yysvec+25,	yyvstop+287,
0,	0,	0};
struct yywork *yytop = yycrank+306;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
010 ,010 ,012 ,01  ,010 ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
010 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'+' ,01  ,'+' ,'.' ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,'A' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
