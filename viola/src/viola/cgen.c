/*
 * cgen.c
 *
 * Pseudo-code generation procedures
 */
/*
 * Copyright 1990 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */

#include "utils.h"
#include "ast.h"
#include "cgen.h"
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "ident.h"
#include "scanutils.h"

char *PCodeStr[] = {
	"PLUS",
	"MINUS",
	"MOD",
	"MULT",
	"DIV",
	"EQ",
	"NE",
	"LT",
	"LE",
	"GT",
	"GE",
	"AND",
	"OR",

	"PLUS_INT",
	"MINUS_INT",
	"MOD_INT",
	"MULT_INT",
	"DIV_INT",
	"EQ_INT",
	"NE_INT",
	"LT_INT",
	"LE_INT",
	"GT_INT",
	"GE_INT",
	"AND_INT",
	"OR_INT",

	"PLUS_CHAR",
	"MINUS_CHAR",
	"MOD_CHAR",
	"MULT_CHAR",
	"DIV_CHAR",
	"EQ_CHAR",
	"NE_CHAR",
	"LT_CHAR",
	"LE_CHAR",
	"GT_CHAR",
	"GE_CHAR",
	"AND_CHAR",
	"OR_CHAR",

	"PLUS_FLOAT",
	"MINUS_FLOAT",
	"MOD_FLOAT",
	"MULT_FLOAT",
	"DIV_FLOAT",
	"EQ_FLOAT",
	"NE_FLOAT",
	"LT_FLOAT",
	"LE_FLOAT",
	"GT_FLOAT",
	"GE_FLOAT",
	"AND_FLOAT",
	"OR_FLOAT",

	"PLUS_STR",
	"MINUS_STR",
	"MOD_STR",
	"MULT_STR",
	"DIV_STR",
	"EQ_STR",
	"NE_STR",
	"LT_STR",
	"LE_STR",
	"GT_STR",
	"GE_STR",
	"AND_STR",
	"OR_STR",

	"PLUS_LIST",
	"MINUS_LIST",
	"MOD_LIST",
	"MULT_LIST",
	"DIV_LIST",
	"EQ_LIST",
	"NE_LIST",
	"LT_LIST",
	"LE_LIST",
	"GT_LIST",
	"GE_LIST",
	"AND_LIST",
	"OR_LIST",

	"PLUS_LISTC",
	"MINUS_LISTC",
	"MOD_LISTC",
	"MULT_LISTC",
	"DIV_LISTC",
	"EQ_LISTC",
	"NE_LISTC",
	"LT_LISTC",
	"LE_LISTC",
	"GT_LISTC",
	"GE_LISTC",
	"AND_LISTC",
	"OR_LISTC",

	"EQ_STACK",
	"EQ_STACK_NBR_NZERO2", /* frequent operation */

	"PLUS_POP",
	"MINUS_POP",
	"MOD_POP",
	"MULT_POP",
	"DIV_POP",
	"EQ_POP",
	"NE_POP",
	"LT_POP",
	"LE_POP",
	"GT_POP",
	"GE_POP",
	"AND_POP",
	"OR_POP",

	"REF",
	"REF2",
	"LIST_REF2",
	"AND_REF",
	"DIV_REF",
	"GE_REF",
	"GT_REF",
	"LE_REF",
	"LT_REF",
	"EQ_REF",
	"MINUS_REF",
	"MOD_REF",
	"MULT_REF",
	"NE_REF",
	"OR_REF",
	"PLUS_REF",
	"PUSH_REF",
	"PUSH_REFP",
	"PUSH_REFPS",
	"MOVTO_REF2",
	"PLUS_MOVTO_REF",
	"MINUS_MOVTO_REF",
	"MOD_MOVTO_REF",
	"MULT_MOVTO_REF",
	"DIV_MOVTO_REF",
	"MOVTO_LIST",
	"PLUS_MOVTO_LIST",
	"MINUS_MOVTO_LIST",
	"MOD_MOVTO_LIST",
	"MULT_MOVTO_LIST",
	"DIV_MOVTO_LIST",
	"BR2",
	"BR2_EQ",
	"BR2_GE",
	"BR2_GT",
	"BR2_LE",
	"BR2_LT",
	"BR2_NE",
	"BR2_NZERO",
	"BR2_ZERO",
	"NBR2",
	"NBR2_EQ",
	"NBR2_GE",
	"NBR2_GT",
	"NBR2_LE",
	"NBR2_LT",
	"NBR2_NE",
	"NBR2_NZERO",
	"NBR2_ZERO",
	"CALL2",
	"CALL2_C",
	"CMP",
	"INTEGER",
	"FLOAT",
	"LOAD",
	"POP",
	"PUSH",
	"STMTS",
	"STRING",
        "CHAR",
	"UMINUS",
	"RETURN",
	"GET2",
	"PUSH_SET2",
	"LIST2",
	"LISTC",
	"STACK",
	"INC_PRE",
	"INC_POST",
	"DEC_PRE",
	"DEC_POST",
	"TOKENIZE_PUSH",

	"METH_SELF",
	"METH_PARENT",
	"METH_SEND",
};

typedef struct BinaryOpInfo {
	int	code_int;
	int	code_char;
	int	code_float;
	int	code_str;
	int	code_ref;
	int	code_listc;
	int	code_list;
	int	code_stack;
} BinaryOpInfo;

BinaryOpInfo binaryOpInfo[] = {
{
	CODE_PLUS_INT,
	CODE_PLUS_CHAR,
	CODE_PLUS_FLOAT,	
	CODE_PLUS_STR,	
	CODE_PLUS_REF,	
	CODE_PLUS_LISTC,  
	CODE_PLUS_LIST,
	CODE_PLUS_POP
},{	
	CODE_MINUS_INT, 
	CODE_MINUS_CHAR, 
	CODE_MINUS_FLOAT, 
	CODE_MINUS_STR, 
	CODE_MINUS_REF, 
	CODE_MINUS_LISTC, 
	CODE_MINUS_LIST,
	CODE_MINUS_POP
},{	
	CODE_MOD_INT,   
	CODE_MOD_CHAR,	
	CODE_MOD_FLOAT,	
	CODE_MOD_STR,	
	CODE_MOD_REF,	
	CODE_MOD_LISTC,   
	CODE_MOD_LIST,
	CODE_MOD_POP
},{	
	CODE_MULT_INT,
	CODE_MULT_CHAR,	
	CODE_MULT_FLOAT,	
	CODE_MULT_STR,	
	CODE_MULT_REF,	
	CODE_MULT_LISTC,  
	CODE_MULT_LIST,
	CODE_MULT_POP
},{	
	CODE_DIV_INT,   
	CODE_DIV_CHAR,	
	CODE_DIV_FLOAT,	
	CODE_DIV_STR,	
	CODE_DIV_REF,	
	CODE_DIV_LISTC,   
	CODE_DIV_LIST,
	CODE_DIV_POP
},{	
	CODE_EQ_INT,    
	CODE_EQ_CHAR,	
	CODE_EQ_FLOAT,	
	CODE_EQ_STR,	
	CODE_EQ_REF,	
	CODE_EQ_LISTC,    
	CODE_EQ_LIST,
	CODE_EQ_POP
},{	
	CODE_NE_INT,    
	CODE_NE_CHAR,	
	CODE_NE_FLOAT,	
	CODE_NE_STR,	
	CODE_NE_REF,	
	CODE_NE_LISTC,	   
	CODE_NE_LIST,
	CODE_NE_POP
},{	
	CODE_LT_INT,    
	CODE_LT_CHAR,	
	CODE_LT_FLOAT,	
	CODE_LT_STR,	
	CODE_LT_REF,	
	CODE_LT_LISTC,    
	CODE_LT_LIST,
	CODE_LT_POP
},{	
	CODE_LE_INT,	
	CODE_LE_CHAR,	
	CODE_LE_FLOAT,	
	CODE_LE_STR,	
	CODE_LE_REF,	
	CODE_LE_LISTC,
	CODE_LE_LIST,
	CODE_LE_POP
},{	
	CODE_GT_INT,	
	CODE_GT_CHAR,	
	CODE_GT_FLOAT,	
	CODE_GT_STR,	
	CODE_GT_REF,	
	CODE_GT_LISTC,    
	CODE_GT_LIST,
	CODE_GT_POP
},{	
	CODE_GE_INT,	
	CODE_GE_CHAR,
	CODE_GE_FLOAT,
	CODE_GE_STR,
	CODE_GE_REF,	
	CODE_GE_LISTC,    
	CODE_GE_LIST,
	CODE_GE_POP
},{	
	CODE_AND_INT,	
	CODE_AND_CHAR,
	CODE_AND_FLOAT,
	CODE_AND_STR,
	CODE_AND_REF,	
	CODE_AND_LISTC,   
	CODE_AND_LIST,
	CODE_AND_POP
},{	
	CODE_OR_INT,	
	CODE_OR_CHAR,
	CODE_OR_FLOAT,
	CODE_OR_STR,
	CODE_OR_REF,	
	CODE_OR_LISTC,    
	CODE_OR_LIST,
	CODE_OR_POP
}
};

int flag_printPCode = 0;
int flag_printAST = 0;

#define IEMIT(val) pcode[(*pc)++].i = (int)(val)
#define FEMIT(val) pcode[(*pc)++].f = (float)(val)
#define SEMIT(val) pcode[(*pc)++].s = (char*)(val)
#define CEMIT(val) pcode[(*pc)++].c = (char)(val)
#define XEMIT(val) pcode[(*pc)++].x = (int)(val)
/*
#define IEMIT(val) pcode[(*pc)++].i = (int)val;\
	printf("pc=%d\tint\t%d\n", *pc-1, (int)val)
#define FEMIT(val) pcode[(*pc)++].f = (float)val;\
	printf("pc=%d\tfloat\t%f\n", *pc-1, (float)val)
#define SEMIT(val) pcode[(*pc)++].s = (char*)val;\
	printf("pc=%d\tstr\t\"%s\"\n", *pc-1, (char*)val)
#define CEMIT(val) pcode[(*pc)++].c = (char)val;\
	printf("pc=%d\tchar\t%c\n", *pc-1, (char)val)
#define XEMIT(val) pcode[(*pc)++].x = (int)val;\
	printf("pc=%d\tcode\t%s\n", *pc-1, PCodeStr[val])
*/

RefInfo varArray[VAR_ARRAY_SIZE];
int varArrayIdx;

RefInfo listRefArray[VAR_ARRAY_SIZE];
int listRefArrayIdx;

int tempi;
int tempw;
char tempc;

#ifdef INLINED
/* this routine is called from gram.y, must set varArrayIdx=0 before
 * entering parser 
 */
void regRef(ast)
	AST *ast;
{
  int id = ast->attribute.info.i;
/*
  if (id) 
printf("REG REF: varArray[%d] ast=%x sym='%s'\n",
       varArrayIdx, ast, (char*)symID2Str->get(symID2Str, id)->val);
  else 
printf("REG REF: varArray[%d] ast=%x sym=???\n",
       varArrayIdx, ast);
*/
	if (varArrayIdx > VAR_ARRAY_SIZE) {
		fprintf(stderr, "varArray stack overflow!\n");
		return;
	}
	varArray[varArrayIdx].ast = ast;
	varArray[varArrayIdx++].flag = 1;
}
#endif

void regListRef(ast)
	AST *ast;
{
	int i, id;
	RefInfo *lfap, *lfap_end;

	id = ast->attribute.info.i;
/*
  if (id) 
printf("REG LIST REF: listRefArray[%d] ast=%x sym='%s'\n",
       listRefArrayIdx, ast, (char*)symID2Str->get(symID2Str, id)->val);
  else 
printf("REG LISTE REF: listRefArray[%d] ast=%x sym=???\n",
       listRefArrayIdx, ast);
*/
	if (varArrayIdx > VAR_ARRAY_SIZE) {
		fprintf(stderr, "varArray stack overflow!\n");
		return;
	}
	varArray[varArrayIdx].ast = ast;
	varArray[varArrayIdx++].flag = 1;

	if (listRefArrayIdx > VAR_ARRAY_SIZE) {
		fprintf(stderr, "listRefArray stack overflow!\n");
		return;
	}
	lfap = listRefArray;
	lfap_end = &listRefArray[listRefArrayIdx];
	id = ast->attribute.info.i;
	for (; lfap < lfap_end; lfap++) 
		if (lfap->ast->attribute.info.i == id) return;

	listRefArray[listRefArrayIdx].ast = ast;
	listRefArray[listRefArrayIdx++].flag = 1;
}

/* ...
 * also, changes REF to LIST_REF
 *
 * XXX This algorithm sucks. rewrite.
 */
int assignReferences(ast, pcode, pc)
	AST *ast;
	union PCode *pcode;
	int *pc;
{
	int i, j, top, bottom, *ip, id, refID = 0;
	int pcodePC_varCount, varCount = 0;
	RefInfo *vap, *vap_end, *refArrayp;
	AST *vastp;

	if (varArrayIdx == 0) return 1; /* no variables... */
/*
	printf(">>>> assignReferences varArrayIdx=%d\n",varArrayIdx);
	printf(">>>> assignReferences listRefArrayIdx=%d\n",listRefArrayIdx);
*/
	/*
	 * scan for indication of list type, and change REFs of the same
	 * id to LIST_REF
	 */
/* not good, should just separate out AST_REF and AST_MOVTO */
	for (i = 0; i < listRefArrayIdx; i++) {
		id = listRefArray[i].ast->attribute.info.i;
/*
printf("!!! looking to convert id=%d sym='%s' to REF\n",
       id, (char*)symID2Str->get(symID2Str, id)->val);
*/
		refArrayp = varArray;
		for (j = 0; j < varArrayIdx; j++, refArrayp++) {
			if (refArrayp->flag) {
				if (refArrayp->ast->type == AST_REF &&
				    refArrayp->ast->attribute.info.i == id) {

					refArrayp->ast->type = AST_LIST_REF;
					refArrayp->flag = 0;
/*
		printf("Converting id=%d sym='%s' to REF\n",
		       id, (char*)symID2Str->get(symID2Str, id)->val);
*/
				}
			}
		}
	}

	/* reset flags */
	for (vap = varArray, vap_end = &varArray[varArrayIdx]; 
	    vap < vap_end; vap++) 
		vap->flag = 1;
/*
	for (i=0, vap = varArray, vap_end = &varArray[varArrayIdx]; 
	    vap < vap_end; vap++, i++) {
id = ast->attribute.info.i;
if (id)
printf("&&& varArray[%d] id=%d sym='%s'\n",
       i, id, (char*)symID2Str->get(symID2Str, id)->val);
else
printf("&&& varArray[%d] id=%d sym=????\n",
       i, id);
}
*/
	/*
	 * assign reference offset value 
	 */
	XEMIT(CODE_PUSH_REFPS);
	pcodePC_varCount = (*pc)++;

	refArrayp = varArray;
	top = 0;
	bottom = varArrayIdx;
	for (i = top; i < bottom; i++) {
		refArrayp = &varArray[i];
		if (refArrayp->flag) {
			++varCount;
			id = refArrayp->ast->attribute.info.i;
			IEMIT(id);
			vap = refArrayp;
			for (j = i; j < bottom; j++, vap++) {
				if (vap->flag) {
					if (id == vap->ast->attribute.info.i) {
					    vap->ast->attribute.info.i = refID;
					    vap->flag = 0;
/*
if (id) {
		printf("assigning id=%d (sym='%s') to refID=%d\n", id,
		       (char*)symID2Str->get(symID2Str, id)->val, refID);
} else {
		printf("assigning id=%d (sym=????) to refID=%d\n", id,
		       refID);
}
*/
					}
				}
			}
			refID++;
		}
	}

	pcode[pcodePC_varCount].i = varCount;

	/* reverse offset values */
	refID--;
	
	for (vap = varArray, vap_end = &varArray[varArrayIdx]; 
	     vap < vap_end; vap++) {
		ip = &(vap->ast->attribute.info.i);
		*ip = refID - *ip;
	}

	return 1;
}

int biopHelper(ast, type, pcode, pc)
	AST *ast;
	int type;
	union PCode *pcode;
	int *pc;
{
	switch (type) {
	case AST_INTEGER:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_int);
		IEMIT(ast->children->next->attribute.info.i);
	break;
	case AST_CHAR:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_char);
		CEMIT(ast->children->next->attribute.info.c);
	break;
	case AST_FLOAT:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_float);
		FEMIT(ast->children->next->attribute.info.f);
	break;
	case AST_STRING:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_str);
		SEMIT(ast->children->next->attribute.info.s);
	break;
	case AST_REF:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_ref);
		IEMIT(ast->children->next->attribute.info.i);
	break;
	case AST_LIST:
		codeGen(ast->children, pcode, pc);
		IEMIT(CODE_PUSH);
		codeGen(ast->children->next->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_list);
		IEMIT(ast->children->next->attribute.info.i);
	break;
	case AST_LISTC:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_listc);
		IEMIT(ast->children->next->attribute.info.i);
	break;
/*
	case AST_STACK:
		codeGen(ast->children, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_stack);
		IEMIT(ast->children->next->attribute.info.i);
	break;
*/
	case AST_CALL:
	case AST_EXPR_PLUS:
	case AST_EXPR_MINUS:
	case AST_EXPR_MULT:
	case AST_EXPR_DIV: /* ... */
	case AST_EXPR_MOD:
	case AST_EXPR_AND:
	case AST_EXPR_OR:
	default:
	/*
	 *	Ex: test1() * (test2() + test4())
	 *
	 *				idx = 0
	 *	1	call test1()	result of test1 -> reg1
	 *	1	push		++idx = 1, reg1 -> stack[1]
	 *	1 2	call test2()	result of test2 -> reg1
	 *	  2	push		++idx = 2, reg1 -> stack[2]
	 *	  2	call test4()	result of test4 -> reg1
	 *	  2	pop-plus	stack[2] + reg1 -> reg1; --idx = 1
	 *	1	pop-mult	stack[1] * reg1 -> reg1; --idx = 0
	 */
		codeGen(ast->children, pcode, pc);
		IEMIT(CODE_PUSH);
		codeGen(ast->children->next, pcode, pc);
		XEMIT(binaryOpInfo[ast->type].code_stack);
	break;
/*		printf("pc = %d, lineno=%d: ", *pc, ast->lineno);

		if (type <= ASTTypeStrSize) {
			printf("Error in binaryOpInfo. type=%d=%s.\n",
				type, ASTTypeStr[type]);
		} else {
			printf("Error in binaryOpInfo. unknown type=%d.\n",
				type);
		}
		return 0;
*/
	}
	return 1;
}

char tokenBuff[2] = {'\0','\0'};

int switchHelper(ast, pcode, pc)
	AST *ast;
	union PCode *pcode;
	int *pc;
{
	int br2bodyPatch[128], br2bodyPatchi = 0;
	int br2endPatch[128], br2endPatchi = 0;
	int br2testsPatch;
	int i, default_bodyPatchIdx = -1;
	AST *astp, *castp, *astp2;
	int broke = 0;
	Packet *pk;
	char *cp;

	/* remember to patch going to the tests */
	br2testsPatch = *pc;
	XEMIT(0xe0000000 | CODE_BR2 << 16);

	/* emit body pcode
	 */
	for (astp = ast->children->next; astp; astp = astp->next) {

		br2bodyPatch[br2bodyPatchi++] = *pc;

		/* code gen body */
		if (astp->children->next->children) {
			codeGen(astp->children->next, pcode, pc);
		}

		/* break */
		if (astp->children->next->next) {
			br2endPatch[br2endPatchi++] = *pc;
			XEMIT(0xe0000000 | CODE_BR2 << 16);
			broke = 1;
		} else {
			broke = 0;
		}
	}

	if (broke == 0) {
		/* unconditional branch to end of switch construct */
		br2endPatch[br2endPatchi++] = *pc;
		XEMIT(0xe0000000 | CODE_BR2 << 16);
	}

	/* patch the branch to the tests */
	pcode[br2testsPatch].i |= *pc - br2testsPatch - 1;

	/* emit code to evaluate and store switcher value */
/*
	astp = ast->children;
	if (astp->type != AST_STRING &&
	    astp->type != AST_INTEGER &&
	    astp->type != AST_CHAR) {
		print("ERROR: switch() value must be integer or string or char.\n");
	}
*/
	codeGen(ast->children, pcode, pc);
	XEMIT(CODE_TOKENIZE_PUSH);

	/* emit tests pcode
	 */
	i = 0;
	for (astp = ast->children->next; astp; astp = astp->next) {
	    for (castp = astp->children->children; castp; 
		 castp = castp->next) {
		if (castp->type == AST_CASE_CLAUSE) {
		    if (castp->children) {
			/* old untokenizing method:
			codeGen(castp->children, pcode, pc);
			 */
			astp2 = castp->children;
			if (astp2->type == AST_STRING) {
			   tempi = tokenize(astp2->attribute.info.s);
			} else if (astp2->type == AST_INTEGER){
			    tempi = astp2->attribute.info.i;
			} else if (astp2->type == AST_CHAR) {
			    tokenBuff[0] = astp2->attribute.info.c;
			    tempi = tokenize(tokenBuff);
			} else {
print("ERROR: switch()'s case value must be integer or string or char.\n");
			    tempi = 0;
			}
			/* emit tokenized value (int) */
			tempw = 0xe0000000 | CODE_INTEGER << 16 | tempi;
			IEMIT(tempw);
		    }

		/*combined following 2 PCODEs:
		    XEMIT(CODE_EQ_STACK);
		    tempi = -(br2bodyPatch[i] - *pc - 1);
		    IEMIT(0xe0000000 | CODE_NBR_NZERO2 << 16 | tempi);
		*/
		    tempi = -(br2bodyPatch[i] - *pc - 1);
		    IEMIT(0xe0000000 | CODE_EQ_STACK_NBR_NZERO2 << 16 | tempi);

		} else if (castp->type == AST_DEFAULT_CLAUSE) {
		    if (default_bodyPatchIdx != -1) {
			/* error, multiple "default:" clause */
			printf("error, multiple ``default:'' clause...\n");
			return 0;
		    }
		    /* do default_clause lastly */
		    default_bodyPatchIdx = i;
		}
	    }
	    ++i;
	}
	if (default_bodyPatchIdx != -1) {
		/*negative branch*/
		tempi = -(br2bodyPatch[default_bodyPatchIdx] - *pc - 1);
		IEMIT(0xe0000000 | CODE_NBR2 << 16 | tempi);
	}

	for (i = 0; i < br2endPatchi; i++)
		pcode[br2endPatch[i]].i |= *pc - br2endPatch[i] - 1;

	return 1;
}

int codeGen(ast, pcode, pc)
	AST *ast;
	union PCode *pcode;
	int *pc;
{
/*	fprintf(stderr, "pc=%d [%d]\n", *pc, ast->type);*/

	switch (ast->type) {
	case AST_MOVTO_REF:
		codeGen(ast->children, pcode, pc);
		/* old 2 words code
		XEMIT(CODE_MOVTO_REF);
		IEMIT(ast->attribute.info.i);
		*/
		IEMIT(0xe0000000 | CODE_MOVTO_REF2 << 16 | 
			ast->attribute.info.i);
	break;

	case AST_PLUS_MOVTO_REF:
		codeGen(ast->children, pcode, pc);
		XEMIT(CODE_PLUS_MOVTO_REF);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MINUS_MOVTO_REF:
		codeGen(ast->children, pcode, pc);
		XEMIT(CODE_MINUS_MOVTO_REF);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MOD_MOVTO_REF:
		codeGen(ast->children, pcode, pc);
		XEMIT(CODE_MOD_MOVTO_REF);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MULT_MOVTO_REF:
		codeGen(ast->children, pcode, pc);
		XEMIT(CODE_MULT_MOVTO_REF);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_DIV_MOVTO_REF:
		codeGen(ast->children, pcode, pc);
		XEMIT(CODE_DIV_MOVTO_REF);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MOVTO_LIST:
		codeGen(ast->children->next, pcode, pc); /* index */
		XEMIT(CODE_PUSH);
		codeGen(ast->children, pcode, pc); /* set value */
		XEMIT(CODE_MOVTO_LIST);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_PLUS_MOVTO_LIST:
		codeGen(ast->children->next, pcode, pc); /* index */
		XEMIT(CODE_PUSH);
		codeGen(ast->children, pcode, pc); /* set value */
		XEMIT(CODE_PLUS_MOVTO_LIST);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MINUS_MOVTO_LIST:
		codeGen(ast->children->next, pcode, pc); /* index */
		XEMIT(CODE_PUSH);
		codeGen(ast->children, pcode, pc); /* set value */
		XEMIT(CODE_MINUS_MOVTO_LIST);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MOD_MOVTO_LIST:
		codeGen(ast->children->next, pcode, pc); /* index */
		XEMIT(CODE_PUSH);
		codeGen(ast->children, pcode, pc); /* set value */
		XEMIT(CODE_MOD_MOVTO_LIST);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_MULT_MOVTO_LIST:
		codeGen(ast->children->next, pcode, pc); /* index */
		XEMIT(CODE_PUSH);
		codeGen(ast->children, pcode, pc); /* set value */
		XEMIT(CODE_MULT_MOVTO_LIST);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_DIV_MOVTO_LIST:
		codeGen(ast->children->next, pcode, pc); /* index */
		XEMIT(CODE_PUSH);
		codeGen(ast->children, pcode, pc); /* set value */
		XEMIT(CODE_DIV_MOVTO_LIST);
		IEMIT(ast->attribute.info.i);
	break;

	case AST_CALL: {
		int argc = 0;
		AST *astp;
	
		for (astp = ast->children; astp; astp = astp->next) {
			codeGen(astp, pcode, pc);
			XEMIT(CODE_PUSH);
			argc++;
		}
		/*
		XEMIT(CODE_CALL);
		IEMIT(ast->attribute.info.i);
		IEMIT(argc);
		*/
		XEMIT(0xe0000000 | CODE_CALL2 << 16 | argc);
		IEMIT(ast->attribute.info.i);
	} break;

	case AST_GET:
		if (ast->children) {
			char *slotName = ast->children->attribute.info.s;
			if (slotName) {
				int slotSymID = getIdent(slotName);
				if (slotSymID) {
					/*
					XEMIT(CODE_GET);
					IEMIT(slotSymID);
					*/
					XEMIT(0xe0000000 | CODE_GET2 << 16 |
						slotSymID);
				}
			} else {
				fprintf(stderr,
	   "internal error: GET(): failed to get symID for argument.\n");
			}
		} else {
			fprintf(stderr,
			   "error: GET() must have exactly one argument.\n");
		}
	break;

	case AST_SET:
		if (ast->children) {
			char *slotName = ast->children->attribute.info.s;
			if (slotName) {
				int slotSymID = getIdent(slotName);
				if (slotSymID) {
					codeGen(ast->children->next, 
						pcode, pc);
					/*
					XEMIT(CODE_PUSH);
					XEMIT(CODE_SET);
					IEMIT(slotSymID);
					*/
					XEMIT(0xe0000000 | 
						CODE_PUSH_SET2 << 16 |
						slotSymID);
				}
			} else {
				fprintf(stderr,
	   "internal error: GET(): failed to get symID for argument.\n");
			}
		} else {
			fprintf(stderr, "error in SET(...)\n");
		}
	break;

	case AST_IF: {
		int stack[128], stacki = 0; /* XXX limitation */
		int lpc = 0;
		AST *astp;

		astp = ast->children;
		while (astp) {

			if (astp->type == AST_BODY) {
				if (lpc) pcode[lpc].i |= *pc - lpc - 1;

				/* else {body} */
				codeGen(astp, pcode, pc);
				break;
			} else {
				/* generate expression code */
				codeGen(astp, pcode, pc);
				lpc = *pc;
				XEMIT(0xe0000000 | CODE_BR2_ZERO << 16);

				/* generate body code */
				astp = astp->next;
				codeGen(astp, pcode, pc);

				astp = astp->next;

				if (ast) {
					/* unconditional branch to end */
					stack[stacki++] = *pc;
					XEMIT(0xe0000000 | CODE_BR2 << 16);
				}
				pcode[lpc].i |= *pc - lpc - 1;
			}
		}
		/* fix offsets to end of if construction */
		for (stacki--; stacki >= 0; stacki--)
			pcode[stack[stacki]].i |= *pc - stack[stacki] - 1;

	} break;

	case AST_SWITCH:
		switchHelper(ast, pcode, pc);
	break;

	case AST_WHILE: {
		int lpc, jpc;

		/* generate expression code */
		lpc = *pc;
		codeGen(ast->children, pcode, pc);
		jpc = *pc;
		XEMIT(0xe0000000 | CODE_BR2_ZERO << 16);

		/* generate body code */
		codeGen(ast->children->next, pcode, pc);

		/* unconditional branch back to while evaluation */
		tempi = -(lpc - *pc - 1);
		XEMIT(0xe0000000 | CODE_NBR2 << 16 | tempi);

		pcode[jpc].i |= *pc - jpc - 1;
	} break;

	case AST_FOR: {
		int lpc, jpc;

		/* generate initialization code */
		codeGen(ast->children, pcode, pc);

		/* generate expression code */
		lpc = *pc;
		codeGen(ast->children->next, pcode, pc);
		jpc = *pc;
		XEMIT(0xe0000000 | CODE_BR2_ZERO << 16);

		/* generate body code */
		codeGen(ast->children->next->next->next, pcode, pc);

		/* generate post body code */
		codeGen(ast->children->next->next, pcode, pc);

		/* unconditional branch back to while evaluation */
		tempi = -(lpc - *pc - 1);
		XEMIT(0xe0000000 | CODE_NBR2 << 16 | tempi);

		pcode[jpc].i |= *pc - jpc - 1;
	} break;

	case AST_BODY:
	case AST_EXPR:
		for (ast = ast->children; ast; ast = ast->next)
			codeGen(ast, pcode, pc);
	break;

	case AST_LISTC:
		IEMIT(0xe0000000 | CODE_LISTC2 << 16 | ast->attribute.info.i);
	break;

	case AST_LIST:
		codeGen(ast->children, pcode, pc);
		IEMIT(0xe0000000 | CODE_LIST2 << 16 | ast->attribute.info.i);
	break;
/*
	case AST_POP:
		XEMIT(CODE_POP);
		IEMIT(ast->attribute.info.i);
	break;
*/
	case AST_LIST_REF:
		/*
		XEMIT(CODE_LIST_REF);
		IEMIT(ast->attribute.info.i);
		*/
		IEMIT(0xe0000000 | CODE_LIST_REF2 << 16 |
			ast->attribute.info.i);
	break;

	/* XXX ASSUME that symIDs are small than 2^16 */
	case AST_REF:
		/* IEMIT identifier strID */
		/* old 2 words code
		XEMIT(CODE_REF);
		IEMIT(ast->attribute.info.i);
		*/
		IEMIT(0xe0000000 | CODE_REF2 << 16 | ast->attribute.info.i);
	break;

	case AST_INC_PRE:
		XEMIT(CODE_INC_PRE);
		IEMIT(ast->children->attribute.info.i);
	break;

	case AST_INC_POST:
		XEMIT(CODE_INC_POST);
		IEMIT(ast->children->attribute.info.i);
	break;

	case AST_DEC_PRE:
		XEMIT(CODE_DEC_PRE);
		IEMIT(ast->children->attribute.info.i);
	break;

	case AST_DEC_POST:
		XEMIT(CODE_DEC_POST);
		IEMIT(ast->children->attribute.info.i);
	break;

	case AST_UMINUS:
		codeGen(ast->children, pcode, pc);
		XEMIT(CODE_UMINUS);
	break;

	case AST_EXPR_PLUS:
	case AST_EXPR_MINUS:
	case AST_EXPR_MOD:
	case AST_EXPR_MULT:
	case AST_EXPR_DIV:
	case AST_EXPR_EQ:
	case AST_EXPR_NE:
	case AST_EXPR_LT:
	case AST_EXPR_LE:
	case AST_EXPR_GT:
	case AST_EXPR_GE:
	case AST_EXPR_AND:
	case AST_EXPR_OR:
	case AST_EXPR_CMP:
		biopHelper(ast, ast->children->next->type, pcode, pc);
	break;

	case AST_INTEGER:
		tempi = ast->attribute.info.i;
		if (tempi < 0) {
			/* negative integer --
			 *  don't fiddle with neg bit, for now
			 */
			XEMIT(CODE_INTEGER);
			IEMIT(tempi);
		} else if (tempi < 0x00010000) {
			/* small integer (fits into 2bytes) */
			tempw = 0xe0000000 | CODE_INTEGER << 16 | tempi;
			IEMIT(tempw);
		} else {
			/* big integer */
			XEMIT(CODE_INTEGER);
			IEMIT(tempi);
		}
	break;

	case AST_CHAR:
		IEMIT(0xe0000000 | CODE_CHAR << 16 | ast->attribute.info.c);
/*
		XEMIT(CODE_CHAR);
		CEMIT(ast->attribute.info.c);
*/
	break;

	case AST_FLOAT:
		XEMIT(CODE_FLOAT);
		FEMIT(ast->attribute.info.f);
	break;

	case AST_STRING:
		XEMIT(CODE_STRING);
		SEMIT(ast->attribute.info.s);
	break;

	case AST_RETURN:
		if (ast->children) codeGen(ast->children, pcode, pc);
		XEMIT(CODE_RETURN);
	break;

	case AST_METH_SELF:
		XEMIT(CODE_METH_SELF);
	break;

	case AST_METH_PARENT:
		XEMIT(CODE_METH_PARENT);
	break;

	case AST_METH_SEND: {
		int argc = 0;
		AST *astp;
		for (astp = ast->children; astp; astp = astp->next) {
			codeGen(astp, pcode, pc);
			XEMIT(CODE_PUSH);
			argc++;
		}
		XEMIT(CODE_METH_SEND);
		IEMIT(argc);
	} break;

	default:
		fprintf(stderr,
			"internal error: unknown ast type = %d:\n",
			ast->type);
		fprintf(stderr, "-----------------\n");
		printAST(ast, 0);
		fprintf(stderr, "-----------------\n");
	break;
	}
	return 0;
}

printPCode(pcode, pc, size)
	union PCode *pcode;
	int *pc;
	int size;
{
	int code, data;

	while (*pc < size) {

	    code = pcode[(*pc)++].x;
	    if (code & 0xe0000000) {
		data = code & 0x0000ffff;
		switch (code & 0x0fff0000) {
		case CODE_INTEGER << 16:
			fprintf(stderr, "%d\tINTEGER2, %d\n",
				(*pc - 1), data);
		break;
		case CODE_CHAR << 16:
			fprintf(stderr, "%d\tCHAR2, %d\n",
				(*pc - 1), data);
		break;
		case CODE_BR2 << 16:
		case CODE_BR2_ZERO << 16:
		case CODE_BR2_NZERO << 16:
		case CODE_BR2_EQ << 16:
		case CODE_BR2_GE << 16:
		case CODE_BR2_GT << 16:
		case CODE_BR2_LE << 16:
		case CODE_BR2_LT << 16:
		case CODE_BR2_NE << 16:
		case CODE_NBR2 << 16:
		case CODE_NBR2_ZERO << 16:
		case CODE_NBR2_NZERO << 16:
		case CODE_NBR2_EQ << 16:
		case CODE_NBR2_GE << 16:
		case CODE_NBR2_GT << 16:
		case CODE_NBR2_LE << 16:
		case CODE_NBR2_LT << 16:
		case CODE_NBR2_NE << 16:
		case CODE_REF2 << 16:
		case CODE_MOVTO_REF2 << 16:
		case CODE_EQ_STACK_NBR_NZERO2 << 16:
		case CODE_LIST2 << 16:
        	case CODE_LIST_REF2 << 16:
        	case CODE_GET2 << 16:
        	case CODE_PUSH_SET2 << 16:
		case CODE_LISTC2 << 16:
			fprintf(stderr, "%d\t%s, %d\n",
				(*pc - 1),
				PCodeStr[(code & 0x0fff0000) >> 16],
				data);
		break;
        	case CODE_CALL2 << 16:
        	case CODE_CALL2_C << 16:
			fprintf(stderr, "%d\t%s, %d (argc)\n",
				(*pc - 1),
				PCodeStr[(code & 0x0fff0000) >> 16],
				data);
			fprintf(stderr, "%d\t%d (id)\n", *pc, pcode[*pc].i);
			(*pc)++;
		break;
		}
	    } else {
		fprintf(stderr, "%d\t%s\n", (*pc-1), PCodeStr[code]);

		switch (code) {
        	case CODE_PUSH_REFP:
			fprintf(stderr, "%d\t%i (%s)\n", *pc, pcode[*pc].i,
				(char*)((symID2Str->get)(symID2Str, 
							 pcode[*pc].i)->val));
			(*pc)++;
		break;

        	case CODE_PUSH_REFPS: {
			int i, n;
			n = pcode[*pc].i;
			fprintf(stderr, "%d\tcount=%d\n", *pc, n);
			(*pc)++;
			while (n--) {
				fprintf(stderr, "%d\t%i (%s)\n",
					*pc, pcode[*pc].i,
				      (char*)symID2Str->get(symID2Str, 
						pcode[*pc].i)->val);
				(*pc)++;
			}
		}
		break;

		case CODE_INTEGER:

		case CODE_MOVTO_LIST:

		case CODE_PLUS_MOVTO_LIST:
		case CODE_MINUS_MOVTO_LIST:
		case CODE_MOD_MOVTO_LIST:
		case CODE_MULT_MOVTO_LIST:
		case CODE_DIV_MOVTO_LIST:

		case CODE_PLUS_MOVTO_REF:
		case CODE_MINUS_MOVTO_REF:
		case CODE_MOD_MOVTO_REF:
		case CODE_MULT_MOVTO_REF:
		case CODE_DIV_MOVTO_REF:

		case CODE_INC_PRE:
		case CODE_INC_POST:
		case CODE_DEC_PRE:
		case CODE_DEC_POST:

		case CODE_AND:
		case CODE_OR:

		case CODE_DIV_INT:
		case CODE_PLUS_INT:
		case CODE_MOD_INT:
		case CODE_MINUS_INT:
		case CODE_MULT_INT:
		case CODE_EQ_INT:
		case CODE_NE_INT:
		case CODE_LT_INT:
		case CODE_LE_INT:
		case CODE_GE_INT:
		case CODE_GT_INT:
		case CODE_AND_INT:
		case CODE_OR_INT:

		case CODE_DIV_REF:
		case CODE_PLUS_REF:
		case CODE_MOD_REF:
		case CODE_MINUS_REF:
		case CODE_MULT_REF:
		case CODE_EQ_REF:
		case CODE_NE_REF:
		case CODE_LT_REF:
		case CODE_LE_REF:
		case CODE_GE_REF:
		case CODE_GT_REF:
		case CODE_AND_REF:
		case CODE_OR_REF:

		case CODE_DIV_LIST:
		case CODE_PLUS_LIST:
		case CODE_MOD_LIST:
		case CODE_MINUS_LIST:
		case CODE_MULT_LIST:
		case CODE_EQ_LIST:
		case CODE_NE_LIST:
		case CODE_LT_LIST:
		case CODE_LE_LIST:
		case CODE_GE_LIST:
		case CODE_GT_LIST:
		case CODE_AND_LIST:
		case CODE_OR_LIST:

		case CODE_DIV_LISTC:
		case CODE_PLUS_LISTC:
		case CODE_MOD_LISTC:
		case CODE_MINUS_LISTC:
		case CODE_MULT_LISTC:
		case CODE_EQ_LISTC:
		case CODE_NE_LISTC:
		case CODE_LT_LISTC:
		case CODE_LE_LISTC:
		case CODE_GE_LISTC:
		case CODE_GT_LISTC:
		case CODE_AND_LISTC:
		case CODE_OR_LISTC:

		case CODE_GE:
		case CODE_GT:
		case CODE_LE:
		case CODE_LT:
		case CODE_NE:
		case CODE_EQ:

			fprintf(stderr, "%d\t%d\n", *pc, pcode[*pc].i);
			(*pc)++;
		break;

		case CODE_CHAR:

		case CODE_DIV_CHAR:
		case CODE_PLUS_CHAR:
		case CODE_MOD_CHAR:
		case CODE_MINUS_CHAR:
		case CODE_MULT_CHAR:
		case CODE_EQ_CHAR:
		case CODE_NE_CHAR:
		case CODE_LT_CHAR:
		case CODE_LE_CHAR:
		case CODE_GE_CHAR:
		case CODE_GT_CHAR:
		case CODE_AND_CHAR:
		case CODE_OR_CHAR:
			fprintf(stderr, "%d\t'%c' (%d)\n", 
				*pc, pcode[*pc].c, (int)pcode[*pc].c);
			(*pc)++;
		break;

		case CODE_FLOAT:

		case CODE_DIV_FLOAT:
		case CODE_PLUS_FLOAT:
		case CODE_MOD_FLOAT:
		case CODE_MINUS_FLOAT:
		case CODE_MULT_FLOAT:
		case CODE_EQ_FLOAT:
		case CODE_NE_FLOAT:
		case CODE_LT_FLOAT:
		case CODE_LE_FLOAT:
		case CODE_GE_FLOAT:
		case CODE_GT_FLOAT:
		case CODE_AND_FLOAT:
		case CODE_OR_FLOAT:
			fprintf(stderr, "%d\t%f\n", *pc, pcode[*pc].f);
			(*pc)++;
		break;

		case CODE_STRING:

		case CODE_DIV_STR:
		case CODE_PLUS_STR:
		case CODE_MOD_STR:
		case CODE_MINUS_STR:
		case CODE_MULT_STR:
		case CODE_EQ_STR:
		case CODE_NE_STR:
		case CODE_LT_STR:
		case CODE_LE_STR:
		case CODE_GE_STR:
		case CODE_GT_STR:
		case CODE_AND_STR:
		case CODE_OR_STR:
			fprintf(stderr, "%d\t\"%s\"\n", *pc, pcode[*pc].s);
			(*pc)++;
		break;

		case CODE_EQ_STACK:

		case CODE_DIV_POP:
		case CODE_PLUS_POP:
		case CODE_MOD_POP:
		case CODE_MINUS_POP:
		case CODE_MULT_POP:
		case CODE_EQ_POP:
		case CODE_NE_POP:
		case CODE_LT_POP:
		case CODE_LE_POP:
		case CODE_GE_POP:
		case CODE_GT_POP:
		case CODE_AND_POP:
		case CODE_OR_POP:

		case CODE_UMINUS:
		case CODE_POP:
		case CODE_PUSH:
		case CODE_CMP:
		case CODE_LOAD:
		case CODE_STMTS:
		case CODE_RETURN:
		case CODE_TOKENIZE_PUSH:
		break;

		default:
			fprintf(stderr, "pc=%d, unknown pcode=%d\n", 
			*pc - 1, pcode[*pc - 1].x);
		break;
		}
	    }
	}
}

