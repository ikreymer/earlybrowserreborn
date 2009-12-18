/*
 * Abstract Syntax Tree abstraction.
 */

#define DEFAULT_AST_STACK_SIZE 3000

#define SCRIPT_ORIGIN_OBJ_SCRIPT	0
#define SCRIPT_ORIGIN_CLASS_SCRIPT	1
#define SCRIPT_ORIGIN_TEMPORARY		2

extern FILE	*yyin, *yyout;
/*extern unsigned char	yytext[];*/

#ifndef USE_FLEX
extern char myinput();
#ifdef input
#undef input
#endif
#define input() myinput()
#ifdef unput
#undef unput
#endif
#define unput(myc) {if ((yyscript[--yyscriptidx] = myc) == '\n') yylineno--;}
#endif

#ifdef USE_FLEX
#undef YY_INPUT
#define YY_INPUT(b, r, ms) (r = my_yyinput(b, ms))
#endif

extern int	lineno;

extern int	yytchar;
extern int	yyleng;
extern char 	*yyscript;
extern int	yyscriptidx;
extern char	*yyobjcontext;
extern int	yyscriptcontext;

enum ASTTypes {
	AST_EXPR_PLUS,
	AST_EXPR_MINUS,
	AST_EXPR_MOD,
	AST_EXPR_MULT,
	AST_EXPR_DIV,
	AST_EXPR_EQ,
	AST_EXPR_NE,
	AST_EXPR_LT,
	AST_EXPR_LE,
	AST_EXPR_GT,
	AST_EXPR_GE,
	AST_EXPR_AND,
	AST_EXPR_OR,
	AST_EXPR_CMP,

	AST_LIST,
	AST_LISTC,
	AST_STACK,
	AST_MOVTO_REF,
	AST_MOVTO_LIST,
	AST_CALL,
	AST_GET,
	AST_SET,
	AST_CHAR,
	AST_IF,
	AST_INTEGER,
	AST_FLOAT,
	AST_STMTS,
	AST_STRING,
	AST_WHILE,
	AST_RETURN,
	AST_SWITCH,
	AST_CASE,
	AST_CASE_SEG,
	AST_CASE_CLAUSE,
	AST_DEFAULT_CLAUSE,
	AST_BREAK,
	AST_BODY,
	AST_EXPR,
	AST_DO,
	AST_FOR,
	AST_INC_PRE,
	AST_INC_POST,
	AST_DEC_PRE,
	AST_DEC_POST,
	AST_UMINUS,
	AST_PLUS_MOVTO_REF,
	AST_MINUS_MOVTO_REF,
	AST_MOD_MOVTO_REF,
	AST_MULT_MOVTO_REF,
	AST_DIV_MOVTO_REF,
	AST_PLUS_MOVTO_LIST,
	AST_MINUS_MOVTO_LIST,
	AST_MOD_MOVTO_LIST,
	AST_MULT_MOVTO_LIST,
	AST_DIV_MOVTO_LIST,
	AST_PERSISTENT,
	AST_REF,
	AST_LIST_REF,
/*
hard coded methos -- optimization hack 
 */
	AST_METH_PARENT,
	AST_METH_SELF,
	AST_METH_SEND,
};

/* Attributes holder for AST (see below) */
typedef struct ATT {
	struct ATT	*next;
	union {
		int	i;
		float	f;
		char	c;
		char	*s;
	} info;
} ATT;

/* Abstract Syntac Tree */
typedef struct AST {
	struct AST	*next;			/* sibling nodes 	*/
	struct AST	*children;		/* children nodes	*/
	struct AST	*parent;		/* parent node 		*/
	struct ATT	attribute;		/* attribute	 	*/
	int		lineno;			/* source line number	*/
	int		type;			/* node type 		*/
} AST;

extern int ASTStackIdx;
extern int ASTTypeStrSize;
extern char *ASTTypeStr[];

#define clearASTStack() ASTStackIdx = 0

extern int init_ast();
extern int countASTSiblings();
extern AST *makeAST();

