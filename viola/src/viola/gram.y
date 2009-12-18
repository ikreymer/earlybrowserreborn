/*
 * Copyright 1991 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
%{
#define YYDEBUG 0
/*#define mydebug(x,y) {printf("%s...\n",x); PrintAST(y);}*/

#include <stdio.h>
#include "ast.h"
#include "cgen.h"

char 	*yyscript;
int	yyscriptidx;
char	*yyobjcontext;
int	yyscriptcontext;

%}

%{
/* Reserved words */
%}

%token	INT CHAR STRING
%token 	IF ELSE
%token	WHILE DO
%token	FOR
%token	SWITCH CASE DEFAULT BREAK
%token	ESWITCH
%token	RETURN 
%token	PERSISTENT
%token	GET SET

%{
/* Constants */
%}

%token <i>	IDENT
%token <s>	STRINGCONST
%token <c>	CHARCONST
%token <i>	INTCONST 
%token <f>	FLOATCONST

%{
/* Precedence definition */	
%}
%left		','
%left		AND OR
%left		EQ NE
%left		'>' '<' 
%left		IDENT
%right		'=' PLUS_ASSERT MINUS_ASSERT MULT_ASSERT DIV_ASSERT MOD_ASSERT
%left		LT LE GT GE 
%left		'+' '-' 
%left		'*' '/' '%'
%right 		UMINUS INC DEC '!'

%{
/* Semantic stack definition */
%}

%union {
	struct AST	*p;		/* an AST node	*/
	int		i;
	float		f;
	char		c;
	char		*s;
}

%{
/* Nonterminals declearation */
%}

%type <p> 	prog
%type <p> 	persistent
%type <p> 	stmt stmts
%type <p> 	if_flow 
%type <p> 	for_flow exprs_stmts
%type <p> 	do_flow
%type <p> 	while_flow
%type <p>	switch_flow case_clause default_clause case_clauses 
%type <p> 	case_block case_blocks
%type <p> 	assert_stmt
%type <p> 	call_stmt
%type <p> 	return_stmt
%type <p> 	get_stmt set_stmt
%type <p> 	expr exprs term list_term
%type <p> 	integer float char string ident idents 
%type <p> 	lval

/*%expect 1*/

%{
/* Reduction code declarations */

extern YYSTYPE yylval;
AST *theAST = NULL;
AST *persistentAST = NULL;
AST *ast;

%}

%%

/*****************************************************************************
 * Start production rules
 */
prog 	:	persistent ';' stmts
		{
			persistentAST = $1;
			$$ = makeAST(AST_BODY);
			$$->children = $1;
			theAST = $$;
		}
	|	stmts
		{
			$$ = makeAST(AST_BODY);
			$$->children = $1;
			theAST = $$;
		}
	;

/*****************************************************************************
 * Persistent variables
 */
persistent :	PERSISTENT idents
		{
			$$ = makeAST(AST_PERSISTENT);
			$$->children = $2;
			regRef($2);
		}
	;

/*****************************************************************************
 * Statements 
 */

stmt	:	expr ';'
	|	return_stmt ';'
	|	do_flow
	|	for_flow
	|	if_flow
	|	switch_flow
	|	while_flow
	|	'{' stmts '}'
		{
			$$ = makeAST(AST_BODY);
			$$->children = $2;
		}
	|	';'
		{
			$$ = NULL;
		}
	;

stmts	:	stmts stmt 
		{
/*printf("stmts-> stmts stmt \n<<<<<<<");*/
			for (ast = $1; ast->next; ast = ast->next);
			ast->next = $2;
			$$ = $1;
		}
	|	stmt
		{
/*printf("stmts-> stmt \n<<<<<<<");*/
			$$ = $1;
		}
	|
		{
/*printf("stmts-> 0 \n<<<<<<<");*/
			$$ = NULL;
		}
	;

assert_stmt :	ident '=' expr
		{
			$$ = $1;
			$$->type = AST_MOVTO_REF;
			$$->children = $3;
			regRef($1);
		}
	|	list_term '=' expr
		{
			$$ = $1;
			$$->type = AST_MOVTO_LIST;
			$$->children->next = $3;
		}
	|	ident PLUS_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_PLUS_MOVTO_REF;
			$$->children = $3;
			regRef($1);
		}
	|	list_term PLUS_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_PLUS_MOVTO_LIST;
			$$->children->next = $3;
		}
	|	ident MINUS_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_MINUS_MOVTO_REF;
			$$->children = $3;
			regRef($1);
		}
	|	list_term MINUS_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_MINUS_MOVTO_LIST;
			$$->children->next = $3;
		}
	|	ident MOD_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_MOD_MOVTO_REF;
			$$->children = $3;
			regRef($1);
		}
	|	list_term MOD_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_MOD_MOVTO_LIST;
			$$->children->next = $3;
		}
	|	ident MULT_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_MULT_MOVTO_REF;
			$$->children = $3;
			regRef($1);
		}
	|	list_term MULT_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_MULT_MOVTO_LIST;
			$$->children->next = $3;
		}
	|	ident DIV_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_DIV_MOVTO_REF;
			$$->children = $3;
			regRef($1);
		}
	|	list_term DIV_ASSERT expr
		{
			$$ = $1;
			$$->type = AST_DIV_MOVTO_LIST;
			$$->children->next = $3;
		}
	;

call_stmt :	ident '(' exprs ')'
		{
			$$ = $1;
			$$->type = AST_CALL;
			$$->children = $3;
		}
	|	ident '(' ')'
		{
			$$ = $1;
			$$->type = AST_CALL;
		}
	;

get_stmt :	GET '(' exprs ')'
		{
			$$ = makeAST(AST_GET);
			$$->children = $3;
		}
	;

set_stmt :	SET '(' exprs ')'
		{
			$$ = makeAST(AST_SET);
			$$->children = $3;
		}
	;

if_flow :	IF '(' expr ')' stmt
		{
			ast = $5;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = $5;
			}
			$$ = makeAST(AST_IF);
			$$->children = $3;
			$3->next = ast;
		}
	|	IF '(' expr ')' stmt ELSE stmt
		{
			ast = $5;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = $5;
			}
			$$ = makeAST(AST_IF);
			$$->children = $3;
			$3->next = ast;
			ast->next = makeAST(AST_BODY);
			ast->next->children = $7;
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
		}
	;

while_flow :	WHILE '(' expr ')' stmt
		{
			ast = $5;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = $5;
			}
			$$ = makeAST(AST_WHILE);
			$$->children = $3;
			$3->next = ast;
		}
	;

do_flow :	DO '(' expr ')' stmt WHILE '(' expr ')' ';'
		{
			ast = $5;
			if (ast->type != AST_BODY) {
				ast = makeAST(AST_BODY);
				ast->children = $5;
			}
			$$ = makeAST(AST_DO);
			$$->children = $3;
			$3->next = ast;
			ast->next = $8;
		}
	;

exprs_stmts :	'('       ';'      ';'       ')'
		{
			$$ = makeAST(AST_EXPR);
			$$->next = makeAST(AST_EXPR);
			$$->next->next = makeAST(AST_EXPR);
		}
	|	'('       ';'      ';' exprs ')'
		{
			$$ = makeAST(AST_EXPR);
			$$->next = makeAST(AST_EXPR);
			$$->next->next = $4;
		}
	|	'('	  ';' expr ';'       ')'
		{
			$$ = makeAST(AST_EXPR);
			$$->next = $3;
			$$->next->next = makeAST(AST_EXPR);
		}
	|	'('	  ';' expr ';' exprs ')'
		{
			$$ = makeAST(AST_EXPR);
			$$->next = $3;
			$$->next->next = $5;
		}
	|	'(' exprs ';'      ';'	     ')'
		{
			$$ = $2;
			$$->next = makeAST(AST_EXPR);
			$$->next->next = makeAST(AST_EXPR);
		}
	|	'(' exprs ';'      ';' exprs ')'
		{
			$$ = $2;
			$$->next = makeAST(AST_EXPR);
			$$->next->next = $5;
		}
	|	'(' exprs ';' expr ';'	     ')'
		{
			$$ = $2;
			$$->next = $4;
			$$->next->next = makeAST(AST_EXPR);
		}
	|	'(' exprs ';' expr ';' exprs ')'
		{
			$$ = $2;
			$$->next = $4;
			$$->next->next = $6;
		}
	;

for_flow :	FOR exprs_stmts stmt
		{
			$$ = makeAST(AST_FOR);
			$$->children = $2;
			$$->children->next->next->next = $3;
		}
	;

case_clause :	CASE expr ':' 
		{
			$$ = makeAST(AST_CASE_CLAUSE);
			$$->children = $2;
		}
	;

default_clause : DEFAULT ':' 
		{
			$$ = makeAST(AST_DEFAULT_CLAUSE);
		}
	;

case_clauses :	case_clause case_clauses
		{
			$$ = $1;
			$1->next = $2;
		}
	|	default_clause case_clauses
		{
			$$ = $1;
			$1->next = $2;
		}
	|	case_clause
	|	default_clause
	;

case_block :	case_clauses stmts BREAK ';'
		{
			$$ = makeAST(AST_CASE_SEG);
			$$->children = makeAST(AST_CASE);
			$$->children->children = $1;
			$$->children->next = makeAST(AST_BODY);
			$$->children->next->children = $2;
			$$->children->next->next = makeAST(AST_BREAK);
		}
	;

case_blocks :	case_block case_blocks
		{
			$$ = $1;
			$$->next = $2;
		}
	|	case_block
	;

switch_flow :	SWITCH '(' expr ')' '{' case_blocks '}'
		{
			$$ = makeAST(AST_SWITCH);
			$$->children = $3;
			$3->next = $6;
		}
	;

return_stmt :	RETURN expr
		{
			$$ = makeAST(AST_RETURN);
			$$->children = $2;
		}
	|	RETURN
		{
			$$ = makeAST(AST_RETURN);
		}
	;

/*****************************************************************************
 * Simple expressions
 */

term	:	float
	|	integer
	|	char
	|	string
	|	lval
	;

list_term :	ident '[' expr ']'
		{
			$$ = $1;
			$$->type = AST_LIST;
			$$->children = $3;
			regListRef($1);
		}
	|	ident '[' ']'
		{
			$$ = $1;
			$$->type = AST_LISTC;
			regListRef($1);
		}
	;

expr	:	get_stmt
	|	set_stmt
	|	assert_stmt
	|	expr '-' expr
		{ 
			$$ = makeAST(AST_EXPR_MINUS);
			$$->children = $1;
			$1->next = $3;
		}
	|	'(' expr ')'
		{
			$$ = $2;
		}
	|	INC lval
		{ 
			$$ = makeAST(AST_INC_PRE);
			$$->children = $2;
		}
	|	lval INC
		{ 
			$$ = makeAST(AST_INC_POST);
			$$->children = $1;
		}
	|	DEC lval
		{ 
			$$ = makeAST(AST_DEC_PRE);
			$$->children = $2;
		}
	|	lval DEC
		{ 
			$$ = makeAST(AST_DEC_POST);
			$$->children = $1;
		}
	|	expr EQ expr
		{ 
			$$ = makeAST(AST_EXPR_EQ);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr NE expr
		{ 
			$$ = makeAST(AST_EXPR_NE);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr LE expr	
		{ 
			$$ = makeAST(AST_EXPR_LE);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr GE expr	
		{ 
			$$ = makeAST(AST_EXPR_GE);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr '+' expr
		{ 
			$$ = makeAST(AST_EXPR_PLUS);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr '*' expr
		{ 
			$$ = makeAST(AST_EXPR_MULT);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr '/' expr
		{ 
			$$ = makeAST(AST_EXPR_DIV);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr '>' expr
		{ 
			$$ = makeAST(AST_EXPR_GT);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr '<' expr
		{ 
			$$ = makeAST(AST_EXPR_LT);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr '%' expr
		{ 
			$$ = makeAST(AST_EXPR_MOD);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr AND expr
		{ 
			$$ = makeAST(AST_EXPR_AND);
			$$->children = $1;
			$1->next = $3;
		}
	|	expr OR expr
		{ 
			$$ = makeAST(AST_EXPR_OR);
			$$->children = $1;
			$1->next = $3;
		}
	|	'-' expr	%prec UMINUS 
		{ 
			if ($2->type == AST_INTEGER) {
				$2->attribute.info.i *= -1;
				$$ = $2;
			} else if ($2->type == AST_FLOAT) {
				$2->attribute.info.f *= -1.0;
				$$ = $2;
			} else {
				$$ = makeAST(AST_UMINUS);
				$$->children = $2;
			}
		}
	|	call_stmt
	|	term
	;

exprs	: 	expr 
	|	expr ',' exprs
		{
			$$ = $1;
			$1->next = $3;
		}
	;

lval	:	ident
		{
			regRef($1);
		}
	|	list_term
	;

/*****************************************************************************
 * Constants
 */

ident 	:	IDENT	
		{
			$$ = makeAST(AST_REF);
			$$->attribute.info.i = $1;
		}
	;

idents	:	ident ',' idents
		{
			$$ = $1;
			$1->next = $3;
			regRef($1);
			regRef($3);
		}
	|	ident
	;

integer :	INTCONST
		{
			$$ = makeAST(AST_INTEGER);
			$$->attribute.info.i = $1;
		}
	;

float :		FLOATCONST
		{
			$$ = makeAST(AST_FLOAT);
			$$->attribute.info.f = $1;
		}
	;

char	:	CHARCONST
		{
			$$ = makeAST(AST_CHAR);
			$$->attribute.info.c = $1;
		}
	;

string	:	STRINGCONST
		{
			$$ = makeAST(AST_STRING);
			$$->attribute.info.s = $1;
		}
	;

%%

yyerror(s) 
char *s;
{
	reportError();
}

yywrap() {
	return 1;
}




