/*
 * ast.c
 */
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
/*#include <stdio.h>*/
#include "utils.h"
#ifdef SVR4
#include <strings.h>
#endif
#include "y.tab.h"
#include "ast.h"

AST *ASTStack;

int ASTStackSize = DEFAULT_AST_STACK_SIZE; 
	/* overiden with option, if large script is anticipated */
int ASTStackIdx = 0;

int ASTTypeStrSize;
char *ASTTypeStr[] = {
	"EXPR_PLUS",	/* the EXPR's must be grouped in the front. */
	"EXPR_MINUS",   /* because codeGen() depends on it */
	"EXPR_MOD",
	"EXPR_MULT",
	"EXPR_DIV",
	"EXPR_EQ",
	"EXPR_NE",
	"EXPR_LT",
	"EXPR_LE",
	"EXPR_GT",
	"EXPR_GE",
	"EXPR_AND",
	"EXPR_OR",
	"EXPR_CMP",	/* end of EXPR's grouping */
	"LIST",
	"LISTC",
	"STACK",
	"MOVTO_REF",
	"MOVTO_LIST",
	"CALL",
	"GET",
	"SET",
	"CHAR",
	"IF",
	"INTEGER",
	"FLOAT",
	"STMTS",
	"STRING",
	"WHILE",
	"RETURN",
	"SWITCH",
	"CASE",
	"CASE_SEG",
	"CASE_CLAUSE",
	"DEFAULT_CLAUSE",
	"BREAK",
	"BODY",
	"EXPR",
	"DO",
	"FOR",
	"INC_PRE",
	"INC_POST",
	"DEC_PRE",
	"DEC_POST",
	"UMINUS",
	"PLUS_MOVTO_REF",
	"MINUS_MOVTO_REF",
	"MOD_MOVTO_REF",
	"MULT_MOVTO_REF",
	"DIV_MOVTO_REF",
	"PLUS_MOVTO_LIST",
	"MINUS_MOVTO_LIST",
	"MOD_MOVTO_LIST",
	"MULT_MOVTO_LIST",
	"DIV_MOVTO_LIST",
	"PERSISTENT",
	"REF",
	"LIST_REF",

	"METH_SELF",
	"METH_PARENT",
	"METH_SEND",
	NULL
};

int my_yyinput(buf, max_size)
     char *buf;
     int max_size;
{
	int n, l = strlen(yyscript + yyscriptidx);
	n = max_size < l ? max_size : l;
        if (n > 0) {
                memcpy(buf, yyscript + yyscriptidx, n);
                yyscriptidx += n;
        }
        return n;
}

char myinput()
{
	char c = yyscript[yyscriptidx++]; 
	if (c == '\n') lineno++; 
	return c;
}

int init_ast()
{
	for (ASTTypeStrSize = 0; ASTTypeStr[ASTTypeStrSize]; ASTTypeStrSize++);

	do {
		if (ASTStackSize < 50) {
			fprintf(stderr, "can't get enough memory...\n");
			return 0;
		}

		ASTStack = (AST*)malloc(ASTStackSize * sizeof (struct AST));
		if (!ASTStack) {
			ASTStackSize = ASTStackSize / 2;
			fprintf(stderr,
				"trying to malloc for AST stack (size=%d)\n",
				ASTStackSize);
		}
	} while (!ASTStackSize);

	return 1;
}

AST *makeAST(type)
     int type;
{
	AST *ast;

	if (++ASTStackIdx >= ASTStackSize) {
		/* try to make a larger AST stack... */
		fprintf(stderr, 
			"internal error: AST stack overflow: ASTStackIdx=%d, ASTStackSize=%d.\n", 
			ASTStackIdx, ASTStackSize);
		fprintf(stderr,
			"use ``-astack'' option to allocate larger stack.\n");
		/* XXX unimplemented */
		exit(1);
	}
	/*bzero(&ASTStack[ASTStackIdx], sizeof(struct AST));*/
	ast = &ASTStack[ASTStackIdx];
	ast->next = NULL;
	ast->children = NULL;
	ast->parent = NULL;
	ast->parent = NULL;
	ast->attribute.next = NULL;
	ast->attribute.info.i = NULL;
	ast->type = type;
	ast->lineno = lineno;

	return ast;
}

int countASTSiblings(ast)
     AST *ast;
{
	int i;
	for (i = 0; ast; ast = ast->next) i++;
	return i;
}

void printAST(ast, tab)
     AST *ast;
     int tab;
{
	int i;
	AST *children;

	for (; ast; ast = ast->next) {

		fprintf(stderr, "%d\t", ast->lineno);
		for (i = 0; i < tab; i++) putc(' ', stderr);
		fprintf(stderr, "%s\t", ASTTypeStr[ast->type]);

		switch (ast->type) {
		case AST_CALL:
		case AST_REF:
			fprintf(stderr,
				"info=%d", ast->attribute.info.i);
		break;

		case AST_STRING:
			fprintf(stderr,	
				"info=\"%s\"", (char*)(ast->attribute.info.s));
		break;

		case AST_CHAR:
			fprintf(stderr, 
				"info='%c' (%d)",
				(char)(ast->attribute.info.c),
				(int)(ast->attribute.info.c));
		break;

		case AST_FLOAT:
			fprintf(stderr,
				"info=%f", (float)(ast->attribute.info.f));
		break;

		case AST_INTEGER:
		default:
			fprintf(stderr,
				"info=%d", (int)(ast->attribute.info.i));
		break;
		}
		putc('\n', stderr);

		if (ast->children) printAST(ast->children, tab + 4);
	}
}

