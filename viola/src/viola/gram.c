extern char *malloc(), *realloc();

# line 12 "gram.y"
#define YYDEBUG 0
/*#define mydebug(x,y) {printf("%s...\n",x); PrintAST(y);}*/

#include <stdio.h>
#include "ast.h"
#include "cgen.h"

char 	*yyscript;
int	yyscriptidx;
char	*yyobjcontext;
int	yyscriptcontext;


# line 27 "gram.y"
/* Reserved words */
# define INT 257
# define CHAR 258
# define STRING 259
# define IF 260
# define ELSE 261
# define WHILE 262
# define DO 263
# define FOR 264
# define SWITCH 265
# define CASE 266
# define DEFAULT 267
# define BREAK 268
# define ESWITCH 269
# define RETURN 270
# define PERSISTENT 271
# define GET 272
# define SET 273

# line 41 "gram.y"
/* Constants */
# define IDENT 274
# define STRINGCONST 275
# define CHARCONST 276
# define INTCONST 277
# define FLOATCONST 278

# line 51 "gram.y"
/* Precedence definition */	
# define AND 279
# define OR 280
# define EQ 281
# define NE 282
# define PLUS_ASSERT 283
# define MINUS_ASSERT 284
# define MULT_ASSERT 285
# define DIV_ASSERT 286
# define MOD_ASSERT 287
# define LT 288
# define LE 289
# define GT 290
# define GE 291
# define UMINUS 292
# define INC 293
# define DEC 294

# line 65 "gram.y"
/* Semantic stack definition */

# line 68 "gram.y"
typedef union  {
	struct AST	*p;		/* an AST node	*/
	int		i;
	float		f;
	char		c;
	char		*s;
} YYSTYPE;

# line 77 "gram.y"
/* Nonterminals declearation */

# line 100 "gram.y"
/* Reduction code declarations */

extern YYSTYPE yylval;
AST *theAST = NULL;
AST *persistentAST = NULL;
AST *ast;

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 687 "gram.y"


yyerror(s) 
char *s;
{
	reportError();
}

yywrap() {
	return 1;
}




int yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 99
# define YYLAST 1024
int yyact[]={

    18,    68,    69,    39,   161,    22,   174,   175,   168,     3,
   182,   169,    59,   159,    88,   195,    46,    55,    54,    14,
    49,   171,    56,    63,   194,    18,   142,    33,    62,     5,
    22,    44,    47,    45,   188,    58,    96,    57,    59,   190,
   178,   177,   176,    55,    14,   162,    87,    66,    56,    66,
    18,    59,   149,   148,    95,    22,    55,    54,   147,    49,
    80,    56,    79,    78,    77,    76,    75,    81,    73,    14,
    20,    59,    38,    37,    35,    18,    55,    54,    36,    49,
    22,    56,    24,    13,    18,   179,    16,    34,    15,    22,
    65,     7,    70,    45,    14,    23,    17,    88,    18,   165,
   170,   173,   172,    22,   113,    11,    12,    67,    13,    67,
   110,    18,   163,   138,     8,    74,    22,     9,    10,     2,
     1,    18,     0,     0,    47,    45,    22,     0,     0,     0,
     0,     0,     0,    13,    18,   152,     0,     0,     0,    22,
   155,     0,     0,     0,    18,     0,     0,     0,     0,    22,
     0,     0,     0,     0,     0,     0,     0,    18,    13,     0,
     0,     0,    22,   140,     0,     0,     0,    18,   129,   151,
     0,     0,    22,     0,   158,     0,   160,     0,     0,     0,
    18,   184,   183,    59,     0,    22,     0,   193,    55,    54,
     0,    49,     0,    56,   185,   186,     0,     0,   181,   114,
     0,     0,     0,     0,     0,     0,    58,    18,    57,     0,
   131,     0,    22,     0,    45,     0,     0,     0,     0,     0,
    28,    59,    30,    26,    27,    29,    55,    54,   191,    49,
    25,    56,    31,    32,    39,    43,    42,    41,    40,     0,
     0,     0,   192,     0,    58,    28,    57,    30,    26,    27,
    29,     0,     0,    19,    21,    25,     0,    31,    32,    39,
    43,    42,    41,    40,    52,    89,    53,     0,     0,     0,
    28,     0,    30,    26,    27,    29,     0,     0,    19,    21,
    25,     4,    31,    32,    39,    43,    42,    41,    40,    82,
    83,    85,    86,    84,     0,    28,     0,    30,    26,    27,
    29,     0,     0,    19,    21,    25,     0,    31,    32,    39,
    43,    42,    41,    40,     0,     0,    31,    32,    39,    43,
    42,    41,    40,    52,     0,    53,     0,     0,    19,    21,
    31,    32,    39,    43,    42,    41,    40,    19,    21,     0,
     0,     0,     0,    31,    32,    39,    43,    42,    41,    40,
     0,    19,    21,    31,    32,    39,    43,    42,    41,    40,
     0,     0,     0,     0,    19,    21,    31,    32,    39,    43,
    42,    41,    40,     0,    19,    21,    31,    32,    39,    43,
    42,    41,    40,     0,     0,     0,     0,    19,    21,    31,
    32,    39,    43,    42,    41,    40,     0,    19,    21,    31,
    32,    39,    43,    42,    41,    40,     0,     0,     0,     0,
    19,    21,    31,    32,    39,    43,    42,    41,    40,     0,
    19,    21,     0,     0,     0,    60,    61,    50,    51,     0,
     0,     0,     0,    19,    21,    52,     0,    53,     0,    31,
    32,    39,    43,    42,    41,    40,    59,     0,     0,     0,
     0,    55,    54,     0,    49,     0,    56,     0,     0,     0,
    19,    21,     0,    60,    61,    50,    51,     0,   167,    58,
     0,    57,    59,    52,     0,    53,     0,    55,    54,     0,
    49,     0,    56,     0,     0,     0,     0,    90,    91,    93,
    94,    92,     0,    59,   154,    58,     0,    57,    55,    54,
     0,    49,     0,    56,     0,     0,     0,    59,     0,     0,
     0,   146,    55,    54,     0,    49,    58,    56,    57,     0,
     0,    59,     0,     0,     0,   145,    55,    54,     0,    49,
    58,    56,    57,     0,     0,    59,     0,     0,     0,   144,
    55,    54,     0,    49,    58,    56,    57,     0,     0,   150,
     0,   115,     0,     0,     0,     0,    59,     0,    58,     0,
    57,    55,    54,   143,    49,     0,    56,     0,     0,     0,
    59,     0,     0,     0,   139,    55,    54,     0,    49,    58,
    56,    57,     0,     0,    59,     0,     0,     0,   111,    55,
    54,     0,    49,    58,    56,    57,     0,     0,    59,     0,
     0,     0,     0,    55,    54,     0,    49,    58,    56,    57,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
    48,    58,     0,    57,    55,    54,     0,    49,     0,    56,
    59,   120,   121,     0,     0,    55,    54,     0,    49,   128,
    56,     0,    58,     0,    57,     0,     0,     0,     0,     0,
     0,     0,     0,    58,     0,    57,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    60,    61,
    50,    51,   153,     0,     0,   157,     0,     0,    52,     0,
    53,     0,     0,     0,     0,     0,   164,   166,     0,     0,
     0,     0,     0,     0,    60,    61,    50,    51,     0,   180,
     0,     0,     0,     0,    52,     0,    53,     0,     0,     0,
     0,     0,     0,     0,     0,    60,    61,    50,    51,     0,
     0,     0,     0,     0,     0,    52,     0,    53,     0,    60,
    61,    50,    51,     0,     0,     0,     0,     0,     0,    52,
     0,    53,     0,    60,    61,    50,    51,     0,     0,     0,
     0,     0,     0,    52,     0,    53,     0,    60,    61,    50,
    51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
     0,     0,     0,     0,     0,     0,     0,     0,    60,    61,
    50,    51,     0,     0,     0,     0,     0,     0,    52,     0,
    53,     0,    60,    61,    50,    51,     0,     0,     0,     0,
     0,     0,    52,     0,    53,     0,    60,    61,    50,    51,
     0,     0,     0,     0,     0,     0,    52,     0,    53,     0,
    60,    61,    50,    51,     0,     0,     6,     0,     0,     0,
    52,     0,    53,     0,     0,     0,     0,     0,     0,     0,
     0,    60,    61,    50,    51,    64,     0,     0,     0,    71,
     0,    52,    72,    53,    50,    51,     0,     0,     0,     0,
     0,     0,    52,     0,    53,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    97,    98,    99,   100,
   101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   112,     0,   116,   117,   118,   119,   116,   116,   122,   123,
   124,   125,   126,   127,   116,   130,   132,   133,   134,   135,
   136,   137,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   141,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   116,     0,   156,
   116,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   116,   116,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   116,     0,     0,     0,     0,     0,
     0,   187,     0,   189 };
int yypact[]={

    10, -1000,   -28,    35,  -271, -1000,   561,   -31, -1000, -1000,
 -1000, -1000, -1000,    35, -1000, -1000, -1000, -1000,   167,  -271,
  -292,  -271,   167, -1000, -1000,   167,    28,    26,    25,    24,
    23,    22,    20,     6,   204, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000,    35, -1000, -1000,    -8, -1000,   167,
   167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
   167,   167, -1000,   -15,   547, -1000,   -77, -1000, -1000, -1000,
 -1000, -1000,   582,   167,    35,   140,   167,   167,   167,   167,
   167,   167,   167,   167,   167,   167,   167,   127,   117,   167,
   167,   167,   167,   167,   167,    35,  -271,     1,   -25,   -25,
    14,    14,     1, -1000, -1000,    34,    34, -1000,   593,   593,
 -1000, -1000,   533, -1000,   104,   -33,   519,   498,   484,   470,
    17,    12,    34,    34,    34,    34,    34,    34,    11, -1000,
   456, -1000,    34,    34,    34,    34,    34,    34, -1000,    35,
    94,   435,    81,   167,    35,  -110,    35, -1000, -1000, -1000,
 -1000,  -258, -1000,     4,    71,    58,   409, -1000,  -253,  -260,
 -1000,     2, -1000, -1000,     0, -1000,    -1,    44,    35,  -115,
  -260,    35,  -260,  -260,   167,   -24,   167, -1000, -1000, -1000,
    -2, -1000, -1000, -1000,   -40, -1000, -1000,   184, -1000,   146,
 -1000,   -35, -1000,   -44, -1000, -1000 };
int yypgo[]={

     0,   120,   119,    29,     9,   118,   117,   115,   114,   106,
   105,   102,   101,    21,   100,    11,    96,    95,    91,    88,
    86,   846,   551,    82,    87,    78,    74,    73,    72,    27,
    16,    70 };
int yyr1[]={

     0,     1,     1,     2,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     4,     4,     4,    16,    16,    16,    16,
    16,    16,    16,    16,    16,    16,    16,    16,    17,    17,
    19,    20,     5,     5,     9,     8,     7,     7,     7,     7,
     7,     7,     7,     7,     6,    11,    12,    13,    13,    13,
    13,    14,    15,    15,    10,    18,    18,    23,    23,    23,
    23,    23,    24,    24,    21,    21,    21,    21,    21,    21,
    21,    21,    21,    21,    21,    21,    21,    21,    21,    21,
    21,    21,    21,    21,    21,    21,    21,    21,    22,    22,
    31,    31,    29,    30,    30,    25,    26,    27,    28 };
int yyr2[]={

     0,     7,     3,     5,     4,     4,     2,     2,     2,     2,
     2,     7,     3,     5,     3,     1,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     7,     7,     7,     9,     7,
     9,     9,    11,    15,    11,    21,     9,    11,    11,    13,
    11,    13,    13,    15,     7,     7,     5,     5,     5,     2,
     2,     9,     5,     2,    15,     5,     3,     2,     2,     2,
     2,     2,     9,     7,     2,     2,     2,     7,     7,     5,
     5,     5,     5,     7,     7,     7,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     5,     2,     2,     2,     7,
     3,     2,     3,     7,     2,     3,     3,     3,     3 };
int yychk[]={

 -1000,    -1,    -2,    -4,   271,    -3,   -21,   -18,    -8,    -6,
    -5,   -10,    -9,   123,    59,   -19,   -20,   -16,    40,   293,
   -31,   294,    45,   -17,   -23,   270,   263,   264,   260,   265,
   262,   272,   273,   -29,   -24,   -26,   -25,   -27,   -28,   274,
   278,   277,   276,   275,    59,    -3,   -30,   -29,    59,    45,
   281,   282,   289,   291,    43,    42,    47,    62,    60,    37,
   279,   280,    59,    -4,   -21,   -31,   -29,   -24,   293,   294,
   -31,   -21,   -21,    40,    -7,    40,    40,    40,    40,    40,
    40,    61,   283,   284,   287,   285,   286,    40,    91,    61,
   283,   284,   287,   285,   286,    -4,    44,   -21,   -21,   -21,
   -21,   -21,   -21,   -21,   -21,   -21,   -21,   -21,   -21,   -21,
   125,    41,   -21,    -3,    59,   -22,   -21,   -21,   -21,   -21,
   -22,   -22,   -21,   -21,   -21,   -21,   -21,   -21,   -22,    41,
   -21,    93,   -21,   -21,   -21,   -21,   -21,   -21,   -30,    41,
    59,   -21,    59,    44,    41,    41,    41,    41,    41,    41,
    93,    -3,    41,   -22,    59,    59,   -21,   -22,    -3,   123,
    -3,   262,    41,    41,   -22,    41,   -22,    59,   261,   -15,
   -14,   -13,   -11,   -12,   266,   267,    40,    41,    41,    41,
   -22,    -3,   125,   -15,    -4,   -13,   -13,   -21,    58,   -21,
    41,   268,    58,    41,    59,    59 };
int yydef[]={

    15,    -2,     0,     2,     0,    14,     0,     0,     6,     7,
     8,     9,    10,    15,    12,    64,    65,    66,     0,     0,
    61,     0,     0,    86,    87,    56,     0,     0,     0,     0,
     0,     0,     0,    90,    91,    57,    58,    59,    60,    92,
    96,    95,    97,    98,    15,    13,     3,    94,     4,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     5,     0,     0,    69,    90,    91,    70,    72,
    71,    85,    55,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     1,     0,    67,    73,    74,
    75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
    11,    68,     0,    44,     0,     0,    88,     0,     0,     0,
     0,     0,    16,    18,    20,    22,    24,    26,     0,    29,
     0,    63,    17,    19,    21,    23,    25,    27,    93,     0,
     0,     0,     0,     0,     0,     0,     0,    30,    31,    28,
    62,     0,    36,     0,     0,     0,     0,    89,    32,     0,
    34,     0,    37,    38,     0,    40,     0,     0,     0,     0,
    53,    15,    49,    50,     0,     0,     0,    39,    41,    42,
     0,    33,    54,    52,     0,    47,    48,     0,    46,     0,
    43,     0,    45,     0,    51,    35 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"INT",	257,
	"CHAR",	258,
	"STRING",	259,
	"IF",	260,
	"ELSE",	261,
	"WHILE",	262,
	"DO",	263,
	"FOR",	264,
	"SWITCH",	265,
	"CASE",	266,
	"DEFAULT",	267,
	"BREAK",	268,
	"ESWITCH",	269,
	"RETURN",	270,
	"PERSISTENT",	271,
	"GET",	272,
	"SET",	273,
	"IDENT",	274,
	"STRINGCONST",	275,
	"CHARCONST",	276,
	"INTCONST",	277,
	"FLOATCONST",	278,
	",",	44,
	"AND",	279,
	"OR",	280,
	"EQ",	281,
	"NE",	282,
	">",	62,
	"<",	60,
	"=",	61,
	"PLUS_ASSERT",	283,
	"MINUS_ASSERT",	284,
	"MULT_ASSERT",	285,
	"DIV_ASSERT",	286,
	"MOD_ASSERT",	287,
	"LT",	288,
	"LE",	289,
	"GT",	290,
	"GE",	291,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"UMINUS",	292,
	"INC",	293,
	"DEC",	294,
	"!",	33,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"prog : persistent ';' stmts",
	"prog : stmts",
	"persistent : PERSISTENT idents",
	"stmt : expr ';'",
	"stmt : return_stmt ';'",
	"stmt : do_flow",
	"stmt : for_flow",
	"stmt : if_flow",
	"stmt : switch_flow",
	"stmt : while_flow",
	"stmt : '{' stmts '}'",
	"stmt : ';'",
	"stmts : stmts stmt",
	"stmts : stmt",
	"stmts : /* empty */",
	"assert_stmt : ident '=' expr",
	"assert_stmt : list_term '=' expr",
	"assert_stmt : ident PLUS_ASSERT expr",
	"assert_stmt : list_term PLUS_ASSERT expr",
	"assert_stmt : ident MINUS_ASSERT expr",
	"assert_stmt : list_term MINUS_ASSERT expr",
	"assert_stmt : ident MOD_ASSERT expr",
	"assert_stmt : list_term MOD_ASSERT expr",
	"assert_stmt : ident MULT_ASSERT expr",
	"assert_stmt : list_term MULT_ASSERT expr",
	"assert_stmt : ident DIV_ASSERT expr",
	"assert_stmt : list_term DIV_ASSERT expr",
	"call_stmt : ident '(' exprs ')'",
	"call_stmt : ident '(' ')'",
	"get_stmt : GET '(' exprs ')'",
	"set_stmt : SET '(' exprs ')'",
	"if_flow : IF '(' expr ')' stmt",
	"if_flow : IF '(' expr ')' stmt ELSE stmt",
	"while_flow : WHILE '(' expr ')' stmt",
	"do_flow : DO '(' expr ')' stmt WHILE '(' expr ')' ';'",
	"exprs_stmts : '(' ';' ';' ')'",
	"exprs_stmts : '(' ';' ';' exprs ')'",
	"exprs_stmts : '(' ';' expr ';' ')'",
	"exprs_stmts : '(' ';' expr ';' exprs ')'",
	"exprs_stmts : '(' exprs ';' ';' ')'",
	"exprs_stmts : '(' exprs ';' ';' exprs ')'",
	"exprs_stmts : '(' exprs ';' expr ';' ')'",
	"exprs_stmts : '(' exprs ';' expr ';' exprs ')'",
	"for_flow : FOR exprs_stmts stmt",
	"case_clause : CASE expr ':'",
	"default_clause : DEFAULT ':'",
	"case_clauses : case_clause case_clauses",
	"case_clauses : default_clause case_clauses",
	"case_clauses : case_clause",
	"case_clauses : default_clause",
	"case_block : case_clauses stmts BREAK ';'",
	"case_blocks : case_block case_blocks",
	"case_blocks : case_block",
	"switch_flow : SWITCH '(' expr ')' '{' case_blocks '}'",
	"return_stmt : RETURN expr",
	"return_stmt : RETURN",
	"term : float",
	"term : integer",
	"term : char",
	"term : string",
	"term : lval",
	"list_term : ident '[' expr ']'",
	"list_term : ident '[' ']'",
	"expr : get_stmt",
	"expr : set_stmt",
	"expr : assert_stmt",
	"expr : expr '-' expr",
	"expr : '(' expr ')'",
	"expr : INC lval",
	"expr : lval INC",
	"expr : DEC lval",
	"expr : lval DEC",
	"expr : expr EQ expr",
	"expr : expr NE expr",
	"expr : expr LE expr",
	"expr : expr GE expr",
	"expr : expr '+' expr",
	"expr : expr '*' expr",
	"expr : expr '/' expr",
	"expr : expr '>' expr",
	"expr : expr '<' expr",
	"expr : expr '%' expr",
	"expr : expr AND expr",
	"expr : expr OR expr",
	"expr : '-' expr",
	"expr : call_stmt",
	"expr : term",
	"exprs : expr",
	"exprs : expr ',' exprs",
	"lval : ident",
	"lval : list_term",
	"ident : IDENT",
	"idents : ident ',' idents",
	"idents : ident",
	"integer : INTCONST",
	"float : FLOATCONST",
	"char : CHARCONST",
	"string : STRINGCONST",
};
#endif /* YYDEBUG */
#line 1 "/usr/lib/yaccpar"
/*	@(#)yaccpar 1.10 89/04/04 SMI; from S5R3 1.10	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	{ free(yys); free(yyv); return(0); }
#define YYABORT		{ free(yys); free(yyv); return(1); }
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** static variables used by the parser
*/
static YYSTYPE *yyv;			/* value stack */
static int *yys;			/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */

int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */
	unsigned yymaxdepth = YYMAXDEPTH;

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yyv = (YYSTYPE*)malloc(yymaxdepth*sizeof(YYSTYPE));
	yys = (int*)malloc(yymaxdepth*sizeof(int));
	if (!yyv || !yys)
	{
		yyerror( "out of memory" );
		return(1);
	}
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			(void)printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			yymaxdepth += YYMAXDEPTH;
			yyv = (YYSTYPE*)realloc((char*)yyv,
				yymaxdepth * sizeof(YYSTYPE));
			yys = (int*)realloc((char*)yys,
				yymaxdepth * sizeof(int));
			if (!yyv || !yys)
			{
				yyerror( "yacc stack overflow" );
				return(1);
			}
			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			(void)printf( "Received token " );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				(void)printf( "Received token " );
				if ( yychar == 0 )
					(void)printf( "end-of-file\n" );
				else if ( yychar < 0 )
					(void)printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					(void)printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						(void)printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					(void)printf( "Error recovery discards " );
					if ( yychar == 0 )
						(void)printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						(void)printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						(void)printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			(void)printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 115 "gram.y"
{
			persistentAST = yypvt[-2].p;
			yyval.p = makeAST(AST_BODY);
			yyval.p->children = yypvt[-2].p;
			theAST = yyval.p;
		} break;
case 2:
# line 122 "gram.y"
{
			yyval.p = makeAST(AST_BODY);
			yyval.p->children = yypvt[-0].p;
			theAST = yyval.p;
		} break;
case 3:
# line 133 "gram.y"
{
			yyval.p = makeAST(AST_PERSISTENT);
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-0].p);
		} break;
case 11:
# line 152 "gram.y"
{
			yyval.p = makeAST(AST_BODY);
			yyval.p->children = yypvt[-1].p;
		} break;
case 12:
# line 157 "gram.y"
{
			yyval.p = NULL;
		} break;
case 13:
# line 163 "gram.y"
{
/*printf("stmts-> stmts stmt \n<<<<<<<");*/
			for (ast = yypvt[-1].p; ast->next; ast = ast->next);
			ast->next = yypvt[-0].p;
			yyval.p = yypvt[-1].p;
		} break;
case 14:
# line 170 "gram.y"
{
/*printf("stmts-> stmt \n<<<<<<<");*/
			yyval.p = yypvt[-0].p;
		} break;
case 15:
# line 175 "gram.y"
{
/*printf("stmts-> 0 \n<<<<<<<");*/
			yyval.p = NULL;
		} break;
case 16:
# line 182 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MOVTO_REF;
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-2].p);
		} break;
case 17:
# line 189 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MOVTO_LIST;
			yyval.p->children->next = yypvt[-0].p;
		} break;
case 18:
# line 195 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_PLUS_MOVTO_REF;
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-2].p);
		} break;
case 19:
# line 202 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_PLUS_MOVTO_LIST;
			yyval.p->children->next = yypvt[-0].p;
		} break;
case 20:
# line 208 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MINUS_MOVTO_REF;
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-2].p);
		} break;
case 21:
# line 215 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MINUS_MOVTO_LIST;
			yyval.p->children->next = yypvt[-0].p;
		} break;
case 22:
# line 221 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MOD_MOVTO_REF;
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-2].p);
		} break;
case 23:
# line 228 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MOD_MOVTO_LIST;
			yyval.p->children->next = yypvt[-0].p;
		} break;
case 24:
# line 234 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MULT_MOVTO_REF;
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-2].p);
		} break;
case 25:
# line 241 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_MULT_MOVTO_LIST;
			yyval.p->children->next = yypvt[-0].p;
		} break;
case 26:
# line 247 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_DIV_MOVTO_REF;
			yyval.p->children = yypvt[-0].p;
			regRef(yypvt[-2].p);
		} break;
case 27:
# line 254 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_DIV_MOVTO_LIST;
			yyval.p->children->next = yypvt[-0].p;
		} break;
case 28:
# line 262 "gram.y"
{
			yyval.p = yypvt[-3].p;
			yyval.p->type = AST_CALL;
			yyval.p->children = yypvt[-1].p;
		} break;
case 29:
# line 268 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_CALL;
		} break;
case 30:
# line 275 "gram.y"
{
			yyval.p = makeAST(AST_GET);
			yyval.p->children = yypvt[-1].p;
		} break;
case 31:
# line 282 "gram.y"
{
			yyval.p = makeAST(AST_SET);
			yyval.p->children = yypvt[-1].p;
		} break;
case 32:
# line 289 "gram.y"
{
			ast = yypvt[-0].p;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = yypvt[-0].p;
			}
			yyval.p = makeAST(AST_IF);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = ast;
		} break;
case 33:
# line 300 "gram.y"
{
			ast = yypvt[-2].p;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = yypvt[-2].p;
			}
			yyval.p = makeAST(AST_IF);
			yyval.p->children = yypvt[-4].p;
			yypvt[-4].p->next = ast;
			ast->next = makeAST(AST_BODY);
			ast->next->children = yypvt[-0].p;
/*
			ast = $5;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = $5;
			}
			$$ = makeAST(AST_IF);
			$$->children = $3;
			$3->next = ast;
			ast->next = $7;
*/
		} break;
case 34:
# line 326 "gram.y"
{
			ast = yypvt[-0].p;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = yypvt[-0].p;
			}
			yyval.p = makeAST(AST_WHILE);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = ast;
		} break;
case 35:
# line 339 "gram.y"
{
			ast = yypvt[-5].p;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = yypvt[-5].p;
			}
			yyval.p = makeAST(AST_DO);
			yyval.p->children = yypvt[-7].p;
			yypvt[-7].p->next = ast;
			ast->next = yypvt[-2].p;
		} break;
case 36:
# line 353 "gram.y"
{
			yyval.p = makeAST(AST_EXPR);
			yyval.p->next = makeAST(AST_EXPR);
			yyval.p->next->next = makeAST(AST_EXPR);
		} break;
case 37:
# line 359 "gram.y"
{
			yyval.p = makeAST(AST_EXPR);
			yyval.p->next = makeAST(AST_EXPR);
			yyval.p->next->next = yypvt[-1].p;
		} break;
case 38:
# line 365 "gram.y"
{
			yyval.p = makeAST(AST_EXPR);
			yyval.p->next = yypvt[-2].p;
			yyval.p->next->next = makeAST(AST_EXPR);
		} break;
case 39:
# line 371 "gram.y"
{
			yyval.p = makeAST(AST_EXPR);
			yyval.p->next = yypvt[-3].p;
			yyval.p->next->next = yypvt[-1].p;
		} break;
case 40:
# line 377 "gram.y"
{
			yyval.p = yypvt[-3].p;
			yyval.p->next = makeAST(AST_EXPR);
			yyval.p->next->next = makeAST(AST_EXPR);
		} break;
case 41:
# line 383 "gram.y"
{
			yyval.p = yypvt[-4].p;
			yyval.p->next = makeAST(AST_EXPR);
			yyval.p->next->next = yypvt[-1].p;
		} break;
case 42:
# line 389 "gram.y"
{
			yyval.p = yypvt[-4].p;
			yyval.p->next = yypvt[-2].p;
			yyval.p->next->next = makeAST(AST_EXPR);
		} break;
case 43:
# line 395 "gram.y"
{
			yyval.p = yypvt[-5].p;
			yyval.p->next = yypvt[-3].p;
			yyval.p->next->next = yypvt[-1].p;
		} break;
case 44:
# line 403 "gram.y"
{
			yyval.p = makeAST(AST_FOR);
			yyval.p->children = yypvt[-1].p;
			yyval.p->children->next->next->next = yypvt[-0].p;
		} break;
case 45:
# line 411 "gram.y"
{
			yyval.p = makeAST(AST_CASE_CLAUSE);
			yyval.p->children = yypvt[-1].p;
		} break;
case 46:
# line 418 "gram.y"
{
			yyval.p = makeAST(AST_DEFAULT_CLAUSE);
		} break;
case 47:
# line 424 "gram.y"
{
			yyval.p = yypvt[-1].p;
			yypvt[-1].p->next = yypvt[-0].p;
		} break;
case 48:
# line 429 "gram.y"
{
			yyval.p = yypvt[-1].p;
			yypvt[-1].p->next = yypvt[-0].p;
		} break;
case 51:
# line 438 "gram.y"
{
			yyval.p = makeAST(AST_CASE_SEG);
			yyval.p->children = makeAST(AST_CASE);
			yyval.p->children->children = yypvt[-3].p;
			yyval.p->children->next = makeAST(AST_BODY);
			yyval.p->children->next->children = yypvt[-2].p;
			yyval.p->children->next->next = makeAST(AST_BREAK);
		} break;
case 52:
# line 449 "gram.y"
{
			yyval.p = yypvt[-1].p;
			yyval.p->next = yypvt[-0].p;
		} break;
case 54:
# line 457 "gram.y"
{
			yyval.p = makeAST(AST_SWITCH);
			yyval.p->children = yypvt[-4].p;
			yypvt[-4].p->next = yypvt[-1].p;
		} break;
case 55:
# line 465 "gram.y"
{
			yyval.p = makeAST(AST_RETURN);
			yyval.p->children = yypvt[-0].p;
		} break;
case 56:
# line 470 "gram.y"
{
			yyval.p = makeAST(AST_RETURN);
		} break;
case 62:
# line 487 "gram.y"
{
			yyval.p = yypvt[-3].p;
			yyval.p->type = AST_LIST;
			yyval.p->children = yypvt[-1].p;
			regListRef(yypvt[-3].p);
		} break;
case 63:
# line 494 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yyval.p->type = AST_LISTC;
			regListRef(yypvt[-2].p);
		} break;
case 67:
# line 505 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_MINUS);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 68:
# line 511 "gram.y"
{
			yyval.p = yypvt[-1].p;
		} break;
case 69:
# line 515 "gram.y"
{ 
			yyval.p = makeAST(AST_INC_PRE);
			yyval.p->children = yypvt[-0].p;
		} break;
case 70:
# line 520 "gram.y"
{ 
			yyval.p = makeAST(AST_INC_POST);
			yyval.p->children = yypvt[-1].p;
		} break;
case 71:
# line 525 "gram.y"
{ 
			yyval.p = makeAST(AST_DEC_PRE);
			yyval.p->children = yypvt[-0].p;
		} break;
case 72:
# line 530 "gram.y"
{ 
			yyval.p = makeAST(AST_DEC_POST);
			yyval.p->children = yypvt[-1].p;
		} break;
case 73:
# line 535 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_EQ);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 74:
# line 541 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_NE);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 75:
# line 547 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_LE);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 76:
# line 553 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_GE);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 77:
# line 559 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_PLUS);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 78:
# line 565 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_MULT);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 79:
# line 571 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_DIV);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 80:
# line 577 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_GT);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 81:
# line 583 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_LT);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 82:
# line 589 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_MOD);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 83:
# line 595 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_AND);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 84:
# line 601 "gram.y"
{ 
			yyval.p = makeAST(AST_EXPR_OR);
			yyval.p->children = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 85:
# line 607 "gram.y"
{ 
			if (yypvt[-0].p->type == AST_INTEGER) {
				yypvt[-0].p->attribute.info.i *= -1;
				yyval.p = yypvt[-0].p;
			} else if (yypvt[-0].p->type == AST_FLOAT) {
				yypvt[-0].p->attribute.info.f *= -1.0;
				yyval.p = yypvt[-0].p;
			} else {
				yyval.p = makeAST(AST_UMINUS);
				yyval.p->children = yypvt[-0].p;
			}
		} break;
case 89:
# line 625 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
		} break;
case 90:
# line 632 "gram.y"
{
			regRef(yypvt[-0].p);
		} break;
case 92:
# line 643 "gram.y"
{
			yyval.p = makeAST(AST_REF);
			yyval.p->attribute.info.i = yypvt[-0].i;
		} break;
case 93:
# line 650 "gram.y"
{
			yyval.p = yypvt[-2].p;
			yypvt[-2].p->next = yypvt[-0].p;
			regRef(yypvt[-2].p);
			regRef(yypvt[-0].p);
		} break;
case 95:
# line 660 "gram.y"
{
			yyval.p = makeAST(AST_INTEGER);
			yyval.p->attribute.info.i = yypvt[-0].i;
		} break;
case 96:
# line 667 "gram.y"
{
			yyval.p = makeAST(AST_FLOAT);
			yyval.p->attribute.info.f = yypvt[-0].f;
		} break;
case 97:
# line 674 "gram.y"
{
			yyval.p = makeAST(AST_CHAR);
			yyval.p->attribute.info.c = yypvt[-0].c;
		} break;
case 98:
# line 681 "gram.y"
{
			yyval.p = makeAST(AST_STRING);
			yyval.p->attribute.info.s = yypvt[-0].s;
		} break;
	}
	goto yystack;		/* reset registers in driver code */
}
