/*
 * cexec.c
 *
 * Pseudo-code execution procedures
 */
/*
 * Copyright 1992 Pei-Yuan Wei. All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
#include "utils.h"
#include "sys.h"
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "y.tab.h"
#include "ast.h"
#include "slotaccess.h"
#include "cgen.h"
#include "vlist.h"
#include "attr.h"
#include "cexec.h"
#include "mystrings.h"
#include "ident.h"
#include "scanutils.h"

#ifdef hpux
#include <time.h>
#else hpux
#include <sys/time.h>
#endif hpux

int total_secs = 0;
int total_usecs = 0;

char tokenBuff[2];

int i;
int j;
Packet reg1;
Packet reg2;
Packet *packetp;
Attr *attrp;
Attr *attrp2;
HashEntry *entry;

char *passthru_argument= "";

int flag_printExec = 0;
int flag_vwatch = 0;

int script_watch_source_size = 0;
int script_watch_pcode_size = 0;

#define EXEC_STACK_SIZE_INCREMENT 10000

int	execStackSize = 0;
int	execStackSizeCheck = 0;
Packet	*execStack;
long	stackExecIdx = -1;
long	stackBaseIdx  = -1;

union PCode pcode_buff[5000]; /* limit on maximum opcode size */

#define ARG_STACK_SIZE 256
Attr argAttrStack[ARG_STACK_SIZE];
int argCountStack[ARG_STACK_SIZE];
int argAttrStackIdx = 0;
int argCountStackIdx = 0;
Attr *argListSaveStack[ARG_STACK_SIZE];
int argListSaveStackIdx = 0;

#define OBJ_CALL_STACK 200 /* this limits on the max depth of messaging */
CallObjStack callObjStack[OBJ_CALL_STACK];
int callObjStackIdx = 0;

int init_cexec() {
	nullPacket(&reg1);
	nullPacket(&reg2);
	return incrementExecStack();
}

/* reallocating the stack is unsafe because of the stack's usage 
 * (may be pointers into the stack?)
 * oh well, just start out with a large enough stack, I guess, for now.
 */
int incrementExecStack()
{
	Packet *newStack;

	newStack = (Packet*)malloc(sizeof(struct Packet*) * 
				(execStackSize + EXEC_STACK_SIZE_INCREMENT));
	if (!newStack) {
		return 0;
	}
	if (execStackSize > 0) {
		fprintf(stderr, 
		"WARNING: pcode execution stack overflow. Errors may occur\n");
		bcopy(newStack, execStack, 
			execStackSize * sizeof(struct Packet*));
	/*	free(execStack);*/
	} else {
		bzero(newStack, (execStackSize + EXEC_STACK_SIZE_INCREMENT) 
					* sizeof(struct Packet*));
	}
	execStackSize = execStackSize + EXEC_STACK_SIZE_INCREMENT;
	execStackSizeCheck = execStackSize - 100; /* 100 as arbitrary margin */
	execStack = newStack;

	return (int)execStack;
}

int getlistcount(attrp)
	Attr *attrp;
{
	for (i = 0, attrp2 = (Attr*)((Packet*)(attrp->val))->info.a; 
	     attrp2; attrp2 = attrp2->next) i++;
	return i;
}

dumpStack()
{
	for (i = 0; i < stackExecIdx; i++) {
		printf(": %d\t ", i);
		dumpPacket(&execStack[i]);
		printf("\n");
	}
}

Packet *getlistitem(attrp)
	Attr *attrp;
{
  if (!attrp) return NULL;

	i = reg1.info.i;

	attrp2 = (Attr*)((Packet*)(attrp->val))->info.a; 
/*
printf("getlistitem: i=%d; attrp:\n", i);
dumpVarList(attrp);
printf("\nattrp2:\n");
dumpVarList(attrp2);
printf("\n");
*/
	for (attrp2 = (Attr*)((Packet*)(attrp->val))->info.a; 
	     attrp2; attrp2 = attrp2->next) {
		if (attrp2->id == i) {
			if (attrp2) return (Packet*)attrp2->val;
			return NULL;
		}
	}
	return NULL;
}

/* must be called before makeArgList. Makes sure that the arg variable exists
 */
Attr *makeArgAttr(self)
	VObj *self;
{
	Attr *argAttr = GET__argAttr(self);
	Attr *attrp, *attrlist = NULL;
	Packet *argPk;

	if (!argAttr) {
		Attr *varlist = GET__varList(self);

		if (varlist) {
			for (attrp = varlist; attrp; attrp = attrp->next) {
				if (attrp->id == STR_arg) {
					argAttr = attrp;
					break;
				}
			}
		}
		if (!argAttr) {
			argAttr = (Attr*)malloc(sizeof(struct Attr));
			prependAttr(&varlist, argAttr);
			SET__varList(self, varlist);
			SET__argAttr(self, argAttr);
		}
		if (!argAttr) {
			fprintf(stderr,
				"internal error: failed to make argAttr.\n");
			return NULL;
		}
		argPk = makePacket();
		argPk->info.a = argAttr;
		argPk->type = PKT_ATR;
		argPk->canFree = 0;

		argAttr->id = STR_arg;
		argAttr->val = (long)argPk;
		argAttr->next = NULL;
	}
/*
print("++++++++++NEW VARLIST::::::\n");
dumpVarList(GET__varList(self));

print("++++++++++NEW argAttr::::::\n");
dumpVarList(GET__argAttr(self));
*/
	return argAttr;
}

int makeArgList(self, argc)
	VObj *self;
	int argc;
{
	int i;
	Attr *argAttr, *attrp, *attrlist = NULL;
	Packet *argPk;
	
	argAttr = GET__argAttr(self);
	if (!argAttr) {
		if (!makeArgAttr(self)) {
		  printf("makeArgList(): error, makeArgAttr() failed\n");
		  return 0;
		}
		argAttr = GET__argAttr(self);		
	}
	argPk = (Packet*)argAttr->val;

	if (argCountStackIdx >= ARG_STACK_SIZE) {
		fprintf(stderr, 
		"internal error: argCountStack overflow. Error may occur.\n");
	}
	argCountStack[argCountStackIdx++] = argc;

	/* this is necessary to allow for recursive messaging */
	argListSaveStack[argListSaveStackIdx++] = argPk->info.a;

	argPk->info.a = &argAttrStack[argAttrStackIdx];

	for (i = 0; i < argc; i++) {
		attrp = &argAttrStack[argAttrStackIdx++];
		attrp->next = &argAttrStack[argAttrStackIdx];
		attrp->id = i;
		attrp->val = (long)(&(execStack[stackExecIdx - argc + 1 + i]));
/*
		printf("makeArgList()-----> arg[%d]=", i);
		packetp = (Packet*)attrp->val;
		dumpPacket(packetp);
		printf("<<<\n");
		dumpPacket(&(execStack[stackExecIdx - argc + 1 + i]));
		printf("<<<\n");
*/
	}
	attrp->next = NULL;
/*
printf("argAttr=0x%x\n", GET__argAttr(self));
printf("argAttr=%d      should be: id=490, val!=0.\n", GET__argAttr(self));
dumpVarList(GET__argAttr(self));
printf("\n argAttr->val as packet:");
dumpPacket((Packet*)((Attr*)GET__argAttr(self))->val);
printf("\n");

print("END ################ makeArgList() ################\n");
*/
	return 1;
}

void freeArgListForDestroyed()
{
	int argAttrStackIdx_end = argAttrStackIdx;
	Packet *pk, *argPk;
	
	argAttrStackIdx -= argCountStack[--argCountStackIdx];
	while (argAttrStackIdx_end >= argAttrStackIdx) {
		attrp = &argAttrStack[argAttrStackIdx];
		pk = (Packet*)(attrp->val);
		if (pk->canFree & PK_CANFREE_STR) {
			free(pk->info.s);
			pk->info.s = NULL;
			pk->canFree = 0;
		}
		--argAttrStackIdx_end;
	}
	--argListSaveStackIdx;

}

void freeArgList(self)
	VObj *self;
{
	int argAttrStackIdx_end = argAttrStackIdx;
	Attr *argAttr, *attrp;
	Packet *pk, *argPk;
	
	argAttrStackIdx -= argCountStack[--argCountStackIdx];
	while (argAttrStackIdx_end >= argAttrStackIdx) {
		attrp = &argAttrStack[argAttrStackIdx];
		pk = (Packet*)(attrp->val);
		if (pk->canFree & PK_CANFREE_STR) {
			free(pk->info.s);
			pk->info.s = NULL;
			pk->canFree = 0;
		}
		--argAttrStackIdx_end;
	}
	--argListSaveStackIdx;

	ASSERT(validObjectP(self), "freeArgList() invalid object\n");

	argAttr = GET__argAttr(self);
	argPk = (Packet*)argAttr->val;
	argPk->info.a = argListSaveStack[argListSaveStackIdx];
}

void freeVarList(varlist)
	Attr *varlist;
{
	Attr *attrlist;
	Attr *attrp2, *nextp, *nextp2;
	Packet *pk;
	int i = 0;

	for (attrp = varlist; attrp; attrp = nextp) {
/*
		printf("%d) id=%d=\"%s\" (not freed)", i, 
		      	attrp->id,
			(char*)symID2Str->get(symID2Str, attrp->id));
*/
		packetp = (Packet*)attrp->val;

		if (packetp) {
		  if (packetp->type == PKT_ATR) {
/*			dumpPacket(packetp);
			printf(":\n");
*/
			for (attrp2 = packetp->info.a; attrp2; attrp2 = nextp2) {
/*				printf("\tid=%d,", attrp2->id);
				dumpPacket((Packet*)(attrp2->val));
*/
				pk = (Packet*)(attrp2->val);
				if (!(pk->canFree & PK_CANFREE_NOT))
					clearPacket((Packet*)(attrp2->val));
				nextp2 = attrp2->next;

				/*XXX, BUG, Wastage, Leakage...:
				 * Until there's a way to reliably 
				 * mark an attr(and associated data) 
				 * as sticky, free'ing them isn't safe.
				 */
/*				printf(";  (cleared)\n");*/
			}
		  } else {
/*			dumpPacket(packetp);*/
		  }
		}
		nextp = attrp->next;
		if (!(packetp->canFree & PK_CANFREE_NOT)) {
			clearPacket(packetp);
			free(packetp);
		}
		i++;
/*		printf(";\n");*/
	}
/*	printf("<\n");*/
}

void dumpVarList(varlist)
	Attr *varlist;
{
	Attr *attrlist;
	Attr *attrp2;
	int i = 0;

	for (attrp = varlist; attrp; attrp = attrp->next) {
		printf("%d) id=%d=\"%s\", val=%d ", i, 
		       attrp->id,
		       (char*)symID2Str->get(symID2Str, attrp->id),
		       attrp->val);
		packetp = (Packet*)attrp->val;
		if (packetp) {
			if (packetp->type == PKT_ATR) {
				dumpPacket(packetp);
				printf(":\n");
				for (attrp2 = packetp->info.a; attrp2; 
					attrp2 = attrp2->next) {
					printf("\tid=%d,", attrp2->id);
					dumpPacket((Packet*)(attrp2->val));
					printf(";\n");
				}
			} else {
				dumpPacket(packetp);
			}
			printf(";\n");
		}
		i++;
	}
	printf("<\n");
}

/*
 * XXX reduce redundant code...
 */
Packet *codeExec(self, pcode, pcode_end, varVectorp)
	register VObj *self;
	union PCode *pcode;
	union PCode *pcode_end;
	Attr **varVectorp[];
{
	register int currBaseIdx = stackBaseIdx;
	union PCode *pcode_start = pcode;
	extern char *PCodeStr[];
	int data, code;

	while (pcode < pcode_end) {

	    if (flag_printExec) {
		fprintf(stderr, 
			"pc=%d\tcurrBaseIdx=%d\tstackExecIdx=%d\n",
			pcode - pcode_start,
			currBaseIdx, 
			stackExecIdx);
	    }

	    code = (*pcode++).x;
	    if (code & 0xe0000000) {
		data = code & 0xffff;
		switch (code & 0x0fff0000) {
		case CODE_INTEGER << 16:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = data;
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;
		case CODE_CHAR << 16:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.c = data;
			reg1.type = PKT_CHR;
			reg1.canFree = 0;
		continue;

		case CODE_BR2 << 16:
			pcode += data;
		continue;

		case CODE_BR2_NZERO << 16:
			if (reg1.info.i) pcode += data;
		continue;

		case CODE_BR2_ZERO << 16:
			if (!reg1.info.i) pcode += data;
		continue;

		case CODE_NBR2 << 16:
			pcode -= data;
		continue;

		case CODE_NBR2_NZERO << 16:
			if (reg1.info.i) pcode -= data;
		continue;

		case CODE_EQ_STACK_NBR_NZERO2 << 16:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info = execStack[stackExecIdx].info;
			reg2.type = execStack[stackExecIdx].type;
			reg2.canFree = 0;
			if (reg2.type == 4) {
				/* this is not efficient... should map
				 * PKT_OBJ type into biop.c */
				reg2.type = PKT_STR;
				reg2.info.s = GET_name(reg2.info.o);
				reg2.canFree = 0;
			} else if (reg2.type == 5) {
				reg2.type = reg2.info.p->type;
				reg2.info = reg2.info.p->info;
				reg2.canFree = 0;
			}
			biOp(CODE_EQ, &reg1, &reg2);
			reg2.info.s = NULL;
			if (reg1.info.i) pcode -= data;
		continue;

		case CODE_NBR2_ZERO << 16:
			if (!reg1.info.i) pcode -= data;
		continue;

		case CODE_REF2 << 16:
			packetp = (Packet*)((*varVectorp)[data]->val);
			copyPacket(&reg1, packetp);
		continue;

		case CODE_MOVTO_REF2 << 16:
			packetp = (Packet*)((*varVectorp)[data]->val);
			clearPacket(packetp);
			copyPacket(packetp, &reg1);
		continue;

		case CODE_LIST2 << 16:
/*
fprintf(stderr, "CODE_LIST obj=%s: list item, id=%d\n",
GET_name(self), (*pcode).i);
printf("argAttr=0x%x\n", GET__argAttr(self));
printf("argAttr=%d      should be: id=490, val!=0.\n", GET__argAttr(self));
dumpVarList(GET__argAttr(self));
printf("-->val\n");
dumpPacket((Packet*)GET__argAttr(self)->val);
printf("\n");
*/
			packetp = getlistitem((*varVectorp)[data]);
			if (packetp) {
				copyPacket(&reg1, packetp);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_LIST_REF2 << 16:
			packetp = (Packet*)((*varVectorp)[data]->val);
			if (packetp) copyPacket(&reg1, packetp);
		continue;

		case CODE_LISTC2 << 16:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = getlistcount((*varVectorp)[data]);
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;

		case CODE_GET2 << 16: {
			int (*func)() = GET__classInfo(self)->slotGetMeth;
			if (func) {
				clearPacket(&reg1);
				((long (*)())(func))
					(self, &reg1, 0, NULL, data);
			}
		}
		continue;

		case CODE_PUSH_SET2 << 16: {
			int (*func)() = GET__classInfo(self)->slotSetMeth;

			if (stackExecIdx > execStackSizeCheck) {
				if (!incrementExecStack()) return 0;
				fprintf(stderr,
				     "warning: execution stack overflowed.\n");
			} else {
				packetp = &execStack[++stackExecIdx];
				clearPacket(packetp);
				copyPacket(packetp, &reg1);
			}
			if (func) {
				int save_stackExecIdx = stackExecIdx - 1;
	
				clearPacket(&reg1);

				/* note that argv[0] is not used and undefined!
				 * helper_*_set() should not read argv[0] 
				 */
				((long (*)())(func))
					(self, &reg1, 
					2, &execStack[stackExecIdx - 2 + 1], 
					data);

				/* don't clear execStack[save_stackExecIdx]
				 * because it's the equivalent of arg[0],
				 * an undefined packet, just there to fill
				 * space for helper_*_set()
				 */
				clearPacket(&execStack[save_stackExecIdx]);

				stackBaseIdx = currBaseIdx;
				stackExecIdx = save_stackExecIdx;
			}
		}
		continue;

		case CODE_CALL2 << 16: {
			int save_stackExecIdx;
			int funcid, argc;
			extern MHInfo allmhp;
			CallObjStack *cs;

			funcid = (*pcode++).i;
			argc = data;
/*
			fprintf(stderr, "%d\t%d (fid)\n", funcid);
			fprintf(stderr, "%d\t%d (argc)\n", argc);
			fprintf(stderr, 
				">>1 base=%d, idx=%d, func=\"%s\" argc=%d\n",
				currBaseIdx, stackExecIdx, 
				(char*)symID2Str->get(symID2Str, funcid)->val,
				argc);
*/
			stackBaseIdx = stackExecIdx;
			save_stackExecIdx = stackExecIdx - argc;

			cs = &callObjStack[callObjStackIdx++];
			cs->destroyed = 0;
			cs->obj = self;

			/* if funcid isn't a known symbol, it can't be a
			 * method. So, don't bother searching for it
			 */
			if (allmhp.bits[funcid % allmhp.bitsSize] 
				& (1 << funcid % sizeof(long))) {
				ClassInfo *cip = GET__classInfo(self);

				/* make sure object exists */
	ASSERT(validObjectP(self), "cexect() CODE_CALL: invalid object\n");
				  entry = &(cip->mht->
					     entries[funcid % cip->mht->size]);

				  for (; entry; entry = entry->next) {
				    if (entry->label == funcid) {
				      if (entry->val) {
					if (reg1.canFree & PK_CANFREE_STR) {
/*
extern int fcount;
print("### (cexec) free'ing, count=%d\n", fcount++);
*/
						free(reg1.info.s);/*suspect*/
						reg1.info.s = NULL;
						reg1.canFree = 0;
					}
					/* mutate CODE_CALL2 to CODE_CALL2_C
					 */
/*print("### (cexec) mutating CALL '%s' to C func\n", 
	(char*)symID2Str->get(symID2Str, funcid)->val);
*/
					(*(pcode-2)).x = (*(pcode-2)).x &
					    0xf000ffff | (CODE_CALL2_C << 16);
					(*(pcode-1)).i = (long)(entry->val);

					((long (*)())
					(entry->val))(self, &reg1, argc, 
					  &execStack[stackExecIdx - argc + 1]);

					goto doneCall;
				      } 
				    }
				  }
			}

			/* It's an object .. (bad mix) */
			{
				VObj *obj;
				int stat;

				if (entry = objID2Obj->get(objID2Obj,funcid)) {
					obj = (VObj*)entry->val;
/*
printf("found object:%s...\n", GET_name((VObj*)entry->val));
printf("before >>>>> self=%s\n", GET_name(obj));
dumpVarList(GET__varList(obj));
printf("<<<<<\n");
*/
					makeArgList(obj, argc);
					execObjScript(obj);

	if (callObjStack[callObjStackIdx].destroyed) freeArgListForDestroyed();
	else freeArgList(obj);
					goto doneCall;
				}
			}
			{
				char fname[64], *oname;
				int length;
				VObj *obj;

				oname = (char*)symID2Str->get(symID2Str,
							    funcid)->val;
				strcpy(fname, oname);
				length = strlen(fname);
				if (length >= 2) {
					if (fname[length-2] != '.' ||
					    fname[length-1] != 'v')
						strcat(fname, ".v");
				}
/*fprintf(stderr, "accessing %s.\n", fname);*/
				load_object(fname, NULL);
				if (entry = objID2Obj->get(objID2Obj, 
					storeIdent(saveString(oname)))) {
					obj = (VObj*)entry->val;
					makeArgList(obj, argc);
					execObjScript(obj);

	if (callObjStack[callObjStackIdx].destroyed) freeArgListForDestroyed();
	else freeArgList(obj);

				} else {
					fprintf(stderr,
						"obj=%s: unknown call: %s()\n",
						GET_name(self),
					     (char*)symID2Str->get(symID2Str,
							    funcid)->val);
				}
			}
		doneCall:
			for (i = 0; i < argc; i++) {
/*
				fprintf(stdout,
					"FREEING: (%d,%d) :", 
					i, stackExecIdx - i);
				dumpPacket(&execStack[stackExecIdx - i]);
				fprintf(stdout, "\n");
*/
				packetp = &execStack[stackExecIdx - i];
		  		if (packetp->canFree & PK_CANFREE_STR) {
					if (packetp->info.s) {
						free(packetp->info.s);
						packetp->info.s = NULL;
					}
					packetp->canFree = 0;
				}
				packetp->info.i = 0;
				packetp->type = 0;
			}
			stackBaseIdx = currBaseIdx;
			stackExecIdx = save_stackExecIdx;

			--callObjStackIdx;
			if (cs->destroyed) return &reg1;

		} continue;

		case CODE_CALL2_C << 16: {
			int save_stackExecIdx;
			int (*func)(), argc;
			CallObjStack *cs;

			func = (*pcode++).i;
			argc = data;

			stackBaseIdx = stackExecIdx;
			save_stackExecIdx = stackExecIdx - argc;

			cs = &callObjStack[callObjStackIdx++];
			cs->destroyed = 0;
			cs->obj = self;

	ASSERT(validObjectP(self), "cexect() CODE_CALL2_C: invalid object\n");

			if (reg1.canFree & PK_CANFREE_STR) {
			    free(reg1.info.s);
			    reg1.info.s = NULL;
			    reg1.canFree = 0;
			}
			func(self, &reg1, argc, 
			       &execStack[stackExecIdx - argc + 1]);

			for (i = 0; i < argc; i++) {
			  packetp = &execStack[stackExecIdx - i];
			  if (packetp->canFree & PK_CANFREE_STR) {
			    if (packetp->info.s) {
			      free(packetp->info.s);
			      packetp->info.s = NULL;
			    }
			  } else {
			    packetp->info.i = 0;
			  }
			  packetp->type = 0;
			  packetp->canFree = 0;
			}
			stackBaseIdx = currBaseIdx;
			stackExecIdx = save_stackExecIdx;

			--callObjStackIdx;
			if (cs->destroyed) return &reg1;
		} continue;

/*		case CODE_BR2_EQ:
		case CODE_BR2_GE:
		case CODE_BR2_GT:
		case CODE_BR2_LE:
		case CODE_BR2_LT:
		case CODE_BR2_NE:
		case CODE_NBR2_EQ:
		case CODE_NBR2_GE:
		case CODE_NBR2_GT:
		case CODE_NBR2_LE:
		case CODE_NBR2_LT:
		case CODE_NBR2_NE:
*/
		}
	    } else {
		switch (code) {
		case CODE_INTEGER:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = (*pcode++).i;
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;

        	case CODE_CHAR:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.c = (*pcode++).c;
			reg1.type = PKT_CHR;
			reg1.canFree = 0;
		continue;

		case CODE_FLOAT:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.f = (*pcode++).f;
			reg1.type = PKT_FLT;
			reg1.canFree = 0;
		continue;

		case CODE_STRING:
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.s = (*pcode++).s;
			reg1.type = PKT_STR;
			reg1.canFree = 0;
		continue;

		/* note that in this implementation:
		 * 'x += y' evals to 'y', not 'x + y'
		 */
		case CODE_PLUS_MOVTO_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			biOp(CODE_PLUS, packetp, &reg1);
		continue;

		case CODE_MINUS_MOVTO_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			biOp(CODE_MINUS, packetp, &reg1);
		continue;

		case CODE_MOD_MOVTO_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			biOp(CODE_MOD, packetp, &reg1);
		continue;

		case CODE_MULT_MOVTO_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			biOp(CODE_MULT, packetp, &reg1);
		continue;

		case CODE_DIV_MOVTO_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			biOp(CODE_DIV, packetp, &reg1);
		continue;

		case CODE_MOVTO_LIST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			i = reg1.info.i;
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			copyPacket(packetp, &reg1);
		continue;

		case CODE_PLUS_MOVTO_LIST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			i = reg1.info.i;
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_PLUS, packetp, &reg1);
		continue;

		case CODE_MINUS_MOVTO_LIST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			i = reg1.info.i;
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_MINUS, packetp, &reg1);
		continue;

		case CODE_MOD_MOVTO_LIST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			i = reg1.info.i;
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_MOD, packetp, &reg1);
		continue;

		case CODE_MULT_MOVTO_LIST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			i = reg1.info.i;
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_MULT, packetp, &reg1);
		continue;

		case CODE_DIV_MOVTO_LIST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			i = reg1.info.i;
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_DIV, packetp, &reg1);
		continue;

/* POP */
		case CODE_POP:
			if (stackExecIdx > execStackSizeCheck) {
				if (!incrementExecStack()) return 0;
				fprintf(stderr,
				     "warning: execution stack overflowed.\n");
			} else {
				packetp = &execStack[stackExecIdx];
				copyPacket(&reg1, packetp);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
			}
		continue;

/* TOKENIZE then PUSH */
		case CODE_TOKENIZE_PUSH:
			if (stackExecIdx > execStackSizeCheck) {
				if (!incrementExecStack()) return 0;
				fprintf(stderr,
				     "warning: execution stack overflowed.\n");
			} else {
				packetp = &execStack[++stackExecIdx];
				clearPacket(packetp);
			}
			if (reg1.type == PKT_STR) {
				i = tokenize(reg1.info.s);
				packetp->info.i = i;
				packetp->type = PKT_INT;
			} else if (reg1.type == PKT_INT) {
				packetp->info.i = reg1.info.i;
				packetp->type = PKT_INT;
			} else if (reg1.type == PKT_CHR) {
				tokenBuff[0] = reg1.info.c;
				tokenBuff[1] = '\0';
				packetp->info.i = tokenize(tokenBuff);
				packetp->type = PKT_INT;
			} else {
		print("ERROR: switch() value must be string or char.\n");
			}
		continue;
/* PUSH */
		case CODE_PUSH:
			if (stackExecIdx > execStackSizeCheck) {
				if (!incrementExecStack()) return 0;
				fprintf(stderr,
				     "warning: execution stack overflowed.\n");
			} else {
				packetp = &execStack[++stackExecIdx];
				clearPacket(packetp);
				copyPacket(packetp, &reg1);
/*
fprintf(stdout, "PUSHING: (%d) :", stackExecIdx);
dumpPacket(&execStack[stackExecIdx]);
fprintf(stdout, "\n");
*/
			}
/*
printf("PUSH::: stackExecIdx=%d currBaseIdx=%d reg=%d\n",
stackExecIdx, currBaseIdx, reg1.info.i);
dumpStack();
*/
		continue;

/* PUSH_REFPS */
		case CODE_PUSH_REFPS: {
			Attr *varlist;
			int varID;
			int varCount;
			Packet *vvp;
			int vvi;
			int varCount_TMP;

			varCount = (*pcode++).i;
			if (*varVectorp) {
				/* exists, so skip to next instruction */
				pcode += varCount;
				continue;
			}
			vvi = varCount;
			varCount_TMP = varCount;

			varlist = GET__varList(self);
			*varVectorp = (Attr**)malloc(sizeof(struct Packet) *
							varCount);
			while (varCount--) {
				varID = (*pcode++).i;
/*
				printf("***** varID=%d,\"%s\"\n", varID,
				(char*)(symID2Str->get(symID2Str,varID)->val));
*/
				/* lookup to see if varID corresponds 
				 * to some slot in this object.
				 * If so, make link to that instead 
				 * of a variable...?
				 */
				attrp = NULL;
				for (attrp2 = varlist; attrp2; 
				     attrp2 = attrp2->next) {
					if (attrp2->id == varID) {
						attrp = attrp2;
						break;
					}
				}
				if (attrp) {
					packetp = (Packet*)attrp->val;
				} else {
					packetp = makePacket();
					nullPacket(packetp);
					attrp = makeAttr(varID,(long)packetp);
					prependAttr(&varlist, attrp);
					SET__varList(self, varlist);
				}
				(*varVectorp)[--vvi] = attrp;
			}
		}
		continue;

                case CODE_UMINUS:
                        reg1.info.i *= -1;
                        reg1.type = PKT_INT;
                        reg1.canFree = 0;
                break;

/* INC */
		case CODE_INC_PRE:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = ++(packetp->info.i);
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;

		case CODE_INC_POST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = (packetp->info.i)++;
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;
/* DEC */
		case CODE_DEC_PRE:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = --(packetp->info.i);
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;

		case CODE_DEC_POST:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			if (reg1.canFree & PK_CANFREE_STR) free(reg1.info.s);
			reg1.info.i = (packetp->info.i)--;
			reg1.type = PKT_INT;
			reg1.canFree = 0;
		continue;
/* DIV */
		case CODE_DIV_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_DIV, &reg1, &reg2);
		continue;

		case CODE_DIV_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_DIV, &reg1, &reg2);
		continue;

		case CODE_DIV_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_DIV, &reg1, &reg2);
		continue;

		case CODE_DIV_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_DIV, &reg1, &reg2);
		continue;

		case CODE_DIV_LISTC:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_DIV, &reg1, &reg2);
		continue;

		case CODE_DIV_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_DIV, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_DIV_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_DIV, &reg1, &reg2);
		continue;

/* PLUS */
		case CODE_PLUS_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_PLUS, &reg1, &reg2);
		continue;

		case CODE_PLUS_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_PLUS, &reg1, &reg2);
		continue;

		case CODE_PLUS_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_PLUS, &reg1, &reg2);
		continue;

		case CODE_PLUS_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_PLUS, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_PLUS_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_PLUS, &reg1, &reg2);
		continue;

		case CODE_PLUS_LISTC:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_PLUS, &reg1, &reg2);
		continue;

		case CODE_PLUS_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_PLUS, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_PLUS_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_PLUS, &reg1, &reg2);
		continue;

/* MINUS */
		case CODE_MINUS_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_MINUS, &reg1, &reg2);
		continue;

		case CODE_MINUS_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_MINUS, &reg1, &reg2);
		continue;

		case CODE_MINUS_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_MINUS, &reg1, &reg2);
		continue;

		case CODE_MINUS_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_MINUS, &reg1, &reg2);
		continue;

		case CODE_MINUS_LISTC:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_MINUS, &reg1, &reg2);
		continue;

		case CODE_MINUS_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_MINUS, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_MINUS_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_MINUS, &reg1, &reg2);
		continue;

/* MULT */
		case CODE_MULT_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_MULT, &reg1, &reg2);
		continue;

		case CODE_MULT_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_MULT, &reg1, &reg2);
		continue;

		case CODE_MULT_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_MULT, &reg1, &reg2);
		continue;

		case CODE_MULT_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_MULT, &reg1, &reg2);
		continue;

		case CODE_MULT_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_MULT, &reg1, &reg2);
		continue;

		case CODE_MULT_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_MULT, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_MULT_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_MULT, &reg1, &reg2);
		continue;

/* MOD */
		case CODE_MOD_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_MOD, &reg1, &reg2);
		continue;

		case CODE_MOD_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_MOD, &reg1, &reg2);
		continue;

		case CODE_MOD_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_MOD, &reg1, &reg2);
		continue;

		case CODE_MOD_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_MOD, &reg1, &reg2);
		continue;

		case CODE_MOD_LISTC:
			reg2.info.i =
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_MOD, &reg1, &reg2);
		continue;

		case CODE_MOD_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_MOD, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_MOD_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_MOD, &reg1, &reg2);
		continue;

/* AND && */
		case CODE_AND_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_AND, &reg1, &reg2);
		continue;

		case CODE_AND_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_AND, &reg1, &reg2);
		continue;

		case CODE_AND_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_AND, &reg1, &reg2);
		continue;

		case CODE_AND_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_AND, &reg1, &reg2);
		continue;

		case CODE_AND_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_AND, &reg1, &reg2);
		continue;

		case CODE_AND_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_AND, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_AND_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_AND, &reg1, &reg2);
		continue;

/* OR || */
		case CODE_OR_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.canFree = 0;
			reg2.type = PKT_INT;
			biOp(CODE_OR, &reg1, &reg2);
		continue;

		case CODE_OR_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_OR, &reg1, &reg2);
		continue;

		case CODE_OR_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_OR, &reg1, &reg2);
		continue;

		case CODE_OR_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_OR, &reg1, &reg2);
		continue;

		case CODE_OR_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_OR, &reg1, &reg2);
		continue;

		case CODE_OR_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_OR, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_OR_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_OR, &reg1, &reg2);
		continue;

/* EQ == */
		case CODE_EQ_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_EQ, &reg1, &reg2);
		continue;

		case CODE_EQ_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_EQ, &reg1, &reg2);
		continue;

		case CODE_EQ_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_EQ, &reg1, &reg2);
		continue;

		case CODE_EQ_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_EQ, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_EQ_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_EQ, &reg1, &reg2);
		continue;

		case CODE_EQ_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_EQ, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_EQ_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_EQ, &reg1, &reg2);
		continue;

		case CODE_EQ_STACK:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info = execStack[stackExecIdx].info;
			reg2.type = execStack[stackExecIdx].type;
			reg2.canFree = 0;
			if (reg2.type == 4) {
				/* this is not efficient... should map
				 * PKT_OBJ type into biop.c */
				reg2.type = PKT_STR;
				reg2.info.s = GET_name(reg2.info.o);
				reg2.canFree = 0;
			} else if (reg2.type == 5) {
				reg2.type = reg2.info.p->type;
				reg2.info = reg2.info.p->info;
				reg2.canFree = 0;
/*				dumpPacket(&reg2);
				printf("\n");
*/
			}
			biOp(CODE_EQ, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

/* NE != */
		case CODE_NE_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_NE, &reg1, &reg2);
		continue;

		case CODE_NE_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_NE, &reg1, &reg2);
		continue;

		case CODE_NE_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_NE, &reg1, &reg2);
		continue;

		case CODE_NE_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_NE, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_NE_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_NE, &reg1, &reg2);
		continue;

		case CODE_NE_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_NE, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_NE_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_NE, &reg1, &reg2);
		continue;

		case CODE_NE_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_NE, &reg1, &reg2);
		continue;

/* LT < */
		case CODE_LT_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_LT, &reg1, &reg2);
		continue;

		case CODE_LT_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_LT, &reg1, &reg2);
		continue;

		case CODE_LT_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_LT, &reg1, &reg2);
		continue;

		case CODE_LT_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_LT, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_LT_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_LT, &reg1, &reg2);
		continue;

		case CODE_LT_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_LT, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_LT_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_LT, &reg1, &reg2);
		continue;

		case CODE_LT_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_LT, &reg1, &reg2);
		continue;

/* LE <= */
		case CODE_LE_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_LE, &reg1, &reg2);
		continue;

		case CODE_LE_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_LE, &reg1, &reg2);
		continue;

		case CODE_LE_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_LE, &reg1, &reg2);
		continue;

		case CODE_LE_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_LE, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_LE_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_LE, &reg1, &reg2);
		continue;

		case CODE_LE_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_LE, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_LE_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_LE, &reg1, &reg2);
		continue;

		case CODE_LE_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_LE, &reg1, &reg2);
		continue;

/* GT > */
		case CODE_GT_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_GT, &reg1, &reg2);
		continue;

		case CODE_GT_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_GT, &reg1, &reg2);
		continue;

		case CODE_GT_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_GT, &reg1, &reg2);
		continue;

		case CODE_GT_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_GT, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_GT_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_GT, &reg1, &reg2);
		continue;

		case CODE_GT_LISTC:
			reg2.info.i = 
				getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_GT, &reg1, &reg2);
		continue;

		case CODE_GT_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_GT, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_GT_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_GT, &reg1, &reg2);
		continue;

/* GE >= */
		case CODE_GE_INT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.i = (*pcode++).i;
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_GE, &reg1, &reg2);
		continue;

		case CODE_GE_CHAR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.c = (*pcode++).c;
			reg2.type = PKT_CHR;
			reg2.canFree = 0;
			biOp(CODE_GE, &reg1, &reg2);
		continue;

		case CODE_GE_FLOAT:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.f = (*pcode++).f;
			reg2.type = PKT_FLT;
			reg2.canFree = 0;
			biOp(CODE_GE, &reg1, &reg2);
		continue;

		case CODE_GE_STR:
			if (reg2.canFree & PK_CANFREE_STR) free(reg2.info.s);
			reg2.info.s = (*pcode++).s;
			reg2.type = PKT_STR;
			reg2.canFree = 0;
			biOp(CODE_GE, &reg1, &reg2);
			reg2.info.s = NULL;
		continue;

		case CODE_GE_REF:
			packetp = (Packet*)((*varVectorp)[(*pcode++).i]->val);
			copyPacket(&reg2, packetp);
			biOp(CODE_GE, &reg1, &reg2);
		continue;

		case CODE_GE_LISTC:
			reg2.info.i = 
			  getlistcount((*varVectorp)[(*pcode++).i]);
			reg2.type = PKT_INT;
			reg2.canFree = 0;
			biOp(CODE_GE, &reg1, &reg2);
		continue;

		case CODE_GE_LIST:
			packetp = getlistitem((*varVectorp)[(*pcode++).i]);
			if (packetp) {
				copyPacket(&reg1, &execStack[stackExecIdx]);
				clearPacket(&execStack[stackExecIdx]);
				--stackExecIdx;
				copyPacket(&reg2, packetp);
				biOp(CODE_GE, &reg1, &reg2);
			} else {
				fprintf(stderr,
					"obj=%s: undefined list item, id=%d\n",
					GET_name(self), reg1.info.i);
			}
		continue;

		case CODE_GE_POP:
			copyPacket(&reg2, &reg1);
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			biOp(CODE_GE, &reg1, &reg2);
		continue;

		case CODE_RETURN:
			return &reg1;

		case CODE_CMP:
		case CODE_GE:
		case CODE_GT:
		case CODE_LE:
		case CODE_LT:
		case CODE_NE:
		case CODE_LOAD:
		case CODE_STMTS:
			fprintf(stderr, 
				"pc=%d, unimplemented pcode=%d\n", 
				pcode - pcode_start - 1, (*(pcode - 1)).x);
		continue;
		default:
			fprintf(stderr, 
				"pc=%d, unknown pcode=%d\n", 
				pcode - pcode_start - 1, (*(pcode - 1)).x);
		continue;
		}
	    }
	}
	return &reg1;
}

Packet *execObjScript(obj)
	VObj *obj;
{
        union PCode *pcode = GET__script(obj);

	if (!pcode) {
		extern AST *theAST;
		int size;
		int pc_limit = 0;
		FILE *fp;
		char *tmpfile;
		extern int varArrayIdx;
		extern int listRefArrayIdx;
		
/*		struct timeval time1, time2;
		gettimeofday(&time1, (struct timezone*)NULL);
		if (time1.tv_usec > 1000000) {
		  time1.tv_usec -= 1000000;
		  time1.tv_sec += 1;
		}
		if (time1.tv_usec <0) time1.tv_sec = 0;
*/
		lineno = 1;
		yyscriptidx = 0;
		yyscript = GET_script(obj)->s;
		set_parsing_string(yyscript);
		yyobjcontext = GET_name(obj);
		yyscriptcontext = SCRIPT_ORIGIN_OBJ_SCRIPT;

		varArrayIdx = 0;
		listRefArrayIdx = 0;

#ifdef SCRIPT_FROM_FILENOT
        tmpfile = saveString("/usr/tmp/violaXXXXXX");
        mktemp(tmpfile);
        fp = fopen(tmpfile, "w");
        fputs(yyscript, fp);
        fclose(fp);
        fp = fopen(tmpfile, "r");
        yyin = fp;
#endif SCRIPT_FROM_FILENOT

#ifdef SCRIPT_FROM_FILENOT
        if (flex_called) {
          my_init_lex(yyin);
        }
        flex_called = 1;
#endif SCRIPT_FROM_FILENOT

/*
		fprintf(stderr, 
			"execObjScript: compiling for object %s.\n",
			GET_name(obj));
*/
		/* pass 1: build AST */
		clearASTStack();
        	if (yyparse()) return NULL;

#ifdef SCRIPT_FROM_FILENOT
        fclose(fp);
        unlink(tmpfile);
#endif SCRIPT_FROM_FILENOT

		/* collect identifiers and assign reference offset */
		if (!assignReferences(theAST, 
					&pcode_buff[PCODE_IDX_INSTR],
					&pc_limit)) return 0;

		if (flag_printAST) printAST(theAST, 0);

		for (; theAST; theAST = theAST->next) 
			codeGen(theAST, 
				&pcode_buff[PCODE_IDX_INSTR], 
				&pc_limit);

		pcode_buff[PCODE_IDX_REFC].i = 1;
		pcode_buff[PCODE_IDX_SIZE].i = pc_limit;

		size = sizeof(union PCode) * (pc_limit + PCODE_IDX_INSTR);
		pcode = (union PCode*)Vmalloc(GET__memoryGroup(obj), size);
		bcopy(pcode_buff, pcode, size);

		script_watch_source_size += strlen(yyscript);
		script_watch_pcode_size += size;


		SET__script(obj, pcode);
/*
		gettimeofday(&time2, (struct timezone*)NULL);
		if (time2.tv_usec > 1000000) {
		  time2.tv_usec -= 1000000;
		  time2.tv_sec += 1;
		}
		if (time2.tv_usec <0) time2.tv_sec = 0;

		total_secs += time2.tv_sec - time1.tv_sec;
		total_usecs += time2.tv_usec - time1.tv_usec;
		printf("**** durationObjScript=%d secs, %d usecs, size=%d (%d,%d)\n", 
		       time2.tv_sec - time1.tv_sec,
		       time2.tv_usec - time1.tv_usec, size,
		       total_secs, total_usecs);
*/
	}
	if (flag_printPCode) {
		int pc = 0;
		printPCode(&pcode[PCODE_IDX_INSTR], 
			   &pc,
			   pcode[PCODE_IDX_SIZE].i);
	}
	codeExec(obj,
		 &pcode[PCODE_IDX_INSTR], 
		 &pcode[PCODE_IDX_INSTR] + pcode[PCODE_IDX_SIZE].i,
		 PTR__scriptVV(obj));

	return &reg1;
}

Packet *execObjClassScript(obj, result)
	VObj *obj;
	Packet *result;
{
        union PCode *pcode = GET__classScript(obj);

	if (!pcode) {
		extern AST *theAST;
		int size;
		int pc_limit = 0;
		FILE *fp;
		char *tmpfile;
		extern int varArrayIdx;
		extern int listRefArrayIdx;
/*
		struct timeval time1, time2;
		gettimeofday(&time1, (struct timezone*)NULL);
		if (time1.tv_usec > 1000000) {
		  time1.tv_usec -= 1000000;
		  time1.tv_sec += 1;
		}
		if (time1.tv_usec <0) time1.tv_sec = 0;
*/
		lineno = 1;
		yyscriptidx = 0;
		yyscript = GET_classScript(obj);
		set_parsing_string(yyscript);
		yyobjcontext = GET_name(obj);
		yyscriptcontext = SCRIPT_ORIGIN_CLASS_SCRIPT;

		varArrayIdx = 0;
		listRefArrayIdx = 0;

#ifdef SCRIPT_FROM_FILENOT
        tmpfile = saveString("/usr/tmp/violaXXXXXX");
        mktemp(tmpfile);
        fp = fopen(tmpfile, "w");
        fputs(yyscript, fp);
        fclose(fp);
        fp = fopen(tmpfile, "r");
        yyin = fp;
#endif SCRIPT_FROM_FILENOT

#ifdef SCRIPT_FROM_FILENOT
        if (flex_called) {
          my_init_lex(yyin);
        }
        flex_called = 1;
#endif SCRIPT_FROM_FILENOT

		/* pass 1: build AST */
		clearASTStack();
        	if (yyparse()) return NULL;

#ifdef SCRIPT_FROM_FILENOT
        fclose(fp);
        unlink(tmpfile);
#endif SCRIPT_FROM_FILENOT

		if (flag_printAST) printAST(theAST, 0);

		/* collect identifiers and assign reference offset */
		if (!assignReferences(theAST, 
					&pcode_buff[PCODE_IDX_INSTR],
					&pc_limit)) return 0;

		if (flag_printAST) printAST(theAST, 0);

		for (; theAST; theAST = theAST->next) 
			codeGen(theAST, 
				&pcode_buff[PCODE_IDX_INSTR], 
				&pc_limit);

		pcode_buff[PCODE_IDX_REFC].i = 1;
		pcode_buff[PCODE_IDX_SIZE].i = pc_limit;

		size = sizeof(union PCode) * (pc_limit + PCODE_IDX_INSTR);
		pcode = (union PCode*)malloc(size);
		bcopy(pcode_buff, pcode, size);

		SET__classScript(obj, pcode);
/*
		gettimeofday(&time2, (struct timezone*)NULL);
		if (time2.tv_usec > 1000000) {
		  time2.tv_usec -= 1000000;
		  time2.tv_sec += 1;
		}
		if (time2.tv_usec <0) time2.tv_sec = 0;

		total_secs += time2.tv_sec - time1.tv_sec;
		total_usecs += time2.tv_usec - time1.tv_usec;
		printf("**** durationObjScript=%d secs, %d usecs, size=%d (%d,%d)\n", 
		       time2.tv_sec - time1.tv_sec,
		       time2.tv_usec - time1.tv_usec, size,
		       total_secs, total_usecs);
*/
	}
	if (flag_printPCode) {
		int pc = 0;
		printPCode(&pcode[PCODE_IDX_INSTR], 
			   &pc,
			   pcode[PCODE_IDX_SIZE].i);
	}
	codeExec(obj,
		 &pcode[PCODE_IDX_INSTR], 
		 &pcode[PCODE_IDX_INSTR] + pcode[PCODE_IDX_SIZE].i,
		 PTR__classScriptVV(obj));

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		clearPacket(&reg1);
	}

	return result;
}

Packet *execScript(obj, result, script)
	VObj *obj;
	Packet *result;
	char *script;
{
	union PCode *pcode;
	extern AST *theAST;
	int size;
	int pc_limit = 0;
	FILE *fp;
	char *tmpfile;
	extern int varArrayIdx;
	extern int listRefArrayIdx;

	lineno = 1;
	yyscriptidx = 0;
	yyobjcontext = GET_name(obj);
	yyscriptcontext = SCRIPT_ORIGIN_TEMPORARY;
	yyscript = script;
	set_parsing_string(yyscript);

	varArrayIdx = 0;
	listRefArrayIdx = 0;

#ifdef SCRIPT_FROM_FILENOT
        tmpfile = saveString("/usr/tmp/violaXXXXXX");
        mktemp(tmpfile);
        fp = fopen(tmpfile, "w");
        fputs(yyscript, fp);
        fclose(fp);
        fp = fopen(tmpfile, "r");
        yyin = fp;
#endif SCRIPT_FROM_FILENOT

#ifdef SCRIPT_FROM_FILENOT
        if (flex_called) {
          my_init_lex(yyin);
        }
        flex_called = 1;
#endif SCRIPT_FROM_FILENOT

	/* pass 1: build AST */
	clearASTStack();
       	if (yyparse()) return 0;

#ifdef SCRIPT_FROM_FILENOT
        fclose(fp);
        unlink(tmpfile);
#endif SCRIPT_FROM_FILENOT

	if (flag_printAST) printAST(theAST, 0);

	/* collect identifiers and assign reference offset */
	if (!assignReferences(theAST, 
				&pcode_buff[PCODE_IDX_INSTR],
				&pc_limit)) return 0;

	for (; theAST; theAST = theAST->next) 
		codeGen(theAST, &pcode_buff[PCODE_IDX_INSTR], &pc_limit);

	pcode_buff[PCODE_IDX_REFC].i = 1;
	pcode_buff[PCODE_IDX_SIZE].i = pc_limit;

	size = sizeof(union PCode) * (pc_limit + PCODE_IDX_INSTR);
	pcode = (union PCode*)malloc(size);
	bcopy(pcode_buff, pcode, size);

	if (flag_printPCode) {
		int pc = 0;
		printPCode(&pcode[PCODE_IDX_INSTR], 
			   &pc,
			   pcode[PCODE_IDX_SIZE].i);
	}
	codeExec(obj,
		 &pcode[PCODE_IDX_INSTR], 
		 &pcode[PCODE_IDX_INSTR] + pcode[PCODE_IDX_SIZE].i,
		 PTR__tempScriptVV(obj));

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		clearPacket(&reg1);
	}

	if (GET__tempScriptVV(obj))
	    free(GET__tempScriptVV(obj));/*XXX this is problematic b/c obj
				might have been freed during codeExec */
	SET__tempScriptVV(obj, NULL);
	free(pcode);

	return result;
}

Packet *spp, *pp;

#define CLEAR_REG1() \
	if (reg1.canFree & PK_CANFREE_STR) {\
		free(reg1.info.s);\
		reg1.canFree = 0;\
	}\
	reg1.info.i = 0;\
	reg1.type = 0;

#define FREE_ARGLIST() \
	if (callObjStack[callObjStackIdx].destroyed)\
		freeArgListForDestroyed();\
	else\
		freeArgList(self);

#define PUSH_STACK_STR(ss) {\
	spp = &execStack[++stackExecIdx];\
	spp->info.s = ss;\
	spp->type = PKT_STR;\
	spp->canFree = 0;\
	}

#define PUSH_STACK_INT(ii) {\
	spp = &execStack[++stackExecIdx];\
	spp->info.i = ii;\
	spp->type = PKT_INT;\
	spp->canFree = 0;\
	}

#define PUSH_STACK_CHR(cc) {\
	spp = &execStack[++stackExecIdx];\
	spp->info.c = cc;\
	spp->type = PKT_CHR;\
	spp->canFree = 0;\
	}

#define PUSH_STACK_TOK(tt) {\
	spp = &execStack[++stackExecIdx];\
	spp->info.i = tt;\
	spp->type = PKT_INT;\
	spp->canFree = 0;\
	}

#define PUSH_PACKETS(packets, packetc) \
	spp = &execStack[stackExecIdx];\
	pp = packets;\
	for (i = packetc; i > 0; i--) {\
		++spp;\
		++stackExecIdx;\
		spp->info = pp->info;\
		spp->type = pp->type;\
		spp->canFree = 0;\
		++pp;\
	}

#define PUSH_INT_ARRAY(intArray, intCount) \
	spp = &execStack[stackExecIdx];\
	for (i = 0; i < intCount; i++) {\
		++spp;\
		++stackExecIdx;\
		spp->info.i = intArray[i];\
		spp->type = PKT_INT;\
		spp->canFree = 0;\
	}

#define CLEAR_STACK_PACKETS(packets, packetc) \
	spp = &execStack[stackExecIdx];\
	pp = packets;\
	for (i = packetc; i > 0; i--) {\
		++spp;\
		++stackExecIdx;\
		clearPacket(spp);\
		++pp;\
	}


int sendMessagePackets(self, packets, packetc)
	VObj *self;
	Packet *packets;
	int packetc;
{
	int i;
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();
	
	PUSH_PACKETS(packets, packetc);

	makeArgList(self, packetc);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;

	CLEAR_STACK_PACKETS(packets, packetc);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	freePossibleDangler(&reg1);

	return 1;
}

int sendMessagePackets_result(self, packets, packetc, result)
	VObj *self;
	Packet *packets;
	int packetc;
	Packet *result;
{
	int i;
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();
	
	PUSH_PACKETS(packets, packetc);

	makeArgList(self, packetc);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;

	CLEAR_STACK_PACKETS(packets, packetc);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		clearPacket(&reg1);
	}
	return 1;
}

int sendMessageAndInts(self, messg, intArray, intCount)
	VObj *self;
	char *messg;
	int *intArray;
	int intCount;
{
	int i;
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);

	PUSH_INT_ARRAY(intArray, intCount);

	makeArgList(self, intCount + 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	for (i = 0; i < intCount; i++)
		clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	freePossibleDangler(&reg1);

	return 1;
}

int sendTokenMessageAndInts(self, tok, intArray, intCount)
	VObj *self;
	int tok;
	int *intArray;
	int intCount;
{
	int i;
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_TOK(tok);

	PUSH_INT_ARRAY(intArray, intCount);

	makeArgList(self, intCount + 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	for (i = 0; i < intCount; i++)
		clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	freePossibleDangler(&reg1);

	return 1;
}

int sendTokenMessage_result(self, tok, result)
	VObj *self;
	int tok;
	Packet *result;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_TOK(tok);

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		freePossibleDangler(&reg1);
	}
	return 1;
}

int sendTokenMessage(self, tok)
	VObj *self;
	int tok;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_TOK(tok);

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;
	return 1;
}

int sendMessage1(self, messg)
	VObj *self;
	char *messg;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;

	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	freePossibleDangler(&reg1);
	return 1;
}

int sendMessage1N1str(self, messg, s1)
	VObj *self;
	char *messg;
	char *s1;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);
	PUSH_STACK_STR(s1);

	makeArgList(self, 2);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;
	return 1;
}

int sendMessage1N2str(self, messg, s1, s2)
	VObj *self;
	char *messg;
	char *s1, *s2;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);
	PUSH_STACK_STR(s1);
	PUSH_STACK_STR(s2);

	makeArgList(self, 3);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;
	return 1;
}

int sendMessage1N1int(self, messg, a)
	VObj *self;
	char *messg;
	int a;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);
	PUSH_STACK_INT(a);

	makeArgList(self, 2);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;
	return 1;
}

int sendMessage1N1int_result(self, messg, val, result)
	VObj *self;
	char *messg;
        int  val;
	Packet *result;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);
	PUSH_STACK_INT(val);

	makeArgList(self, 2);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		freePossibleDangler(&reg1);
	}
	return 1;
}

int sendTokenMessageN1int(self, tok, a)
	VObj *self;
	int tok;
	int a;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_TOK(tok);
	PUSH_STACK_INT(a);

	makeArgList(self, 2);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;
	return 1;
}

int sendMessage1N2int(self, messg, a, b)
	VObj *self;
	char *messg;
	int a, b;
{
	int buff[2];

	buff[0] = a;
	buff[1] = b;
	return sendMessageAndInts(self, messg, buff, 2);
}

int sendMessage1N4int(self, messg, a, b, c, d)
	VObj *self;
	char *messg;
	int a, b, c, d;
{
	int buff[4];

	buff[0] = a;
	buff[1] = b;
	buff[2] = c;
	buff[3] = d;
	return sendMessageAndInts(self, messg, buff, 4);
}

int sendMessage1chr(self, c1)
	VObj *self;
	char c1;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_CHR(c1);

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;
	return 1;
}

int sendMessage1_result(self, messg, result)
	VObj *self;
	char *messg;
	Packet *result;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		freePossibleDangler(&reg1);
	}
	return 1;
}

int sendMessage1chr_result(self, c1, result)
	VObj *self;
	char c1;
	Packet *result;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_CHR(c1);

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		freePossibleDangler(&reg1);
	}
	return 1;
}

int sendMessage1N1str_result(self, messg, s1, result)
	VObj *self;
	char *messg, *s1;
	Packet *result;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);
	PUSH_STACK_STR(s1);

	makeArgList(self, 2);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		freePossibleDangler(&reg1);
	}
	return 1;
}

int sendMessage1N2str_result(self, messg, s1, s2, result)
	VObj *self;
	char *messg, *s1, *s2;
	Packet *result;
{
	int save_stackExecIdx = stackExecIdx;
	int save_stackBaseIdx = stackBaseIdx;

	CLEAR_REG1();

	PUSH_STACK_STR(messg);
	PUSH_STACK_STR(s1);
	PUSH_STACK_STR(s2);

	makeArgList(self, 3);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	FREE_ARGLIST();

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	if (&reg1 != result) {
		copyPacket(result, &reg1);
		freePossibleDangler(&reg1);
	}
	return 1;
}


int getVariable(varlist, name, result)
	Attr *varlist;
	char *name;
	Packet *result;
{
	HashEntry *entry;
	int varid;
	Packet *pk;

	/* note: if the identifier is not even in the dictionary, then no 
	 * such variable exists -- all variable IDs are entered into the dict
	 */
	if (entry = symStr2ID->get(symStr2ID, (long)name)) {
		varid = entry->val; 
		
		for (; varlist; varlist = varlist->next) {
/*
			pk = (Packet*)(varlist->val);
			printf("id=%d=\"%s\"\n", varlist->id, pk->info.s);
*/
			if (varlist->id == varid) {
				pk = (Packet*)(varlist->val);
				copyPacket(result, pk);
				return 1;
			}
		}
	}
	return 0;
}

int getVariable_id(varlist, varid)
	Attr *varlist;
	int varid;
{
	/* note: if the identifier is not even in the dictionary, then no 
	 * such variable exists -- all variable IDs are entered into the dict
	 */
	for (; varlist; varlist = varlist->next) {
/*
		pk = (Packet*)(varlist->val);
		printf("id=%d=\"%s\"\n", varlist->id, pk->info.s);
*/
		if (varlist->id == varid) {
			return varlist->val;
		}
	}
	return 0;
}

int setVariable(varlist, name, valp)
	Attr *varlist;
	char *name;
	char *valp;
{
	return 0;
}

Attr *setVariable_STR(varlist, name, valp, canFree)
	Attr *varlist;
	char *name;
	char *valp;
	int canFree;
{
	Attr *head = varlist;
	HashEntry *entry;
	int varid;
	Packet *pk;

	if (entry = symStr2ID->get(symStr2ID, (long)name)) {
		varid = entry->val; 

		for (; varlist; varlist = varlist->next) {
			if ((varlist)->id == varid) {
				pk = (Packet*)(varlist->val);
				pk->info.s = valp;
				pk->type = PKT_STR;
				pk->canFree = canFree;
				return head;
			}
		}
		pk = makePacket();
		pk->info.s = valp;
		pk->type = PKT_STR;
		pk->canFree = canFree;
		prependAttr(&head, makeAttr(varid, (long)pk));
	}
	return head;
}

Attr *setVariable_id_STR(attrp, varid, valp, canFree)
	Attr *attrp;
	int varid;
	char *valp;
	int canFree;
{
	Attr *head = attrp;
	Packet *pk;

	for (; attrp; attrp = attrp->next) {
		if ((attrp)->id == varid) {
			pk = (Packet*)(attrp->val);
			pk->info.s = valp;
			pk->type = PKT_STR;
			pk->canFree = canFree;
			return head;
		}
	}
	pk = makePacket();
	pk->info.s = valp;
	pk->type = PKT_STR;
	pk->canFree = canFree;

	attrp = makeAttr(varid, (long)pk);
	if (head) attrp->next = head;
	return attrp;
}

/*messed up
Attr *setVariable(varlistp, name, resultp)
	Attr **varlistp;
	char *name;
	Packet **resultp;
{
	HashEntry *entry;
	int varid;
	Packet *pk;

	if (entry = symStr2ID->get(symStr2ID, (long)name)) {
		varid = entry->val; 

		if (attrp) 
			for (; varlist; varlist = varlist->next) {
				if (varlist->id == varid) {
					pk = (Packet*)(varlist->val);
					*resultp = pk;
					return 1;
				}
			}
		pk = makePacket();
		*resultp = pk;
		prependAttr(&varlist, makeAttr(varid, (long)pk));
		return 1;
	}
	return NULL;
}
*/

int destroyVariable(varlist, name, retp)
	Attr *varlist;
	char *name;
{
	return 0;
}

int sendMessageAndIntsByName(objName, messg, intArray, intCount)
    char *objName;
    char *messg;
    int *intArray;
    int intCount;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessageAndInts(theObj, messg, intArray, intCount);
}

int sendMessage1ByName(objName, messg)
    char *objName;
    char *messg;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessage1(theObj, messg);
}

int sendMessage1N1strByName(objName, messg, s1)
    char *objName;
    char *messg;
    char *s1;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessage1N1str(theObj, messg, s1);
}

int sendMessage1N2strByName(objName, messg, s1, s2)
    char *objName;
    char *messg;
    char *s1, *s2;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessage1N2str(theObj, messg, s1, s2);
}

int sendMessage1N1intByName(objName, messg, a)
    char *objName;
    char *messg;
    int a;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessage1N1int(theObj, messg, a);
}

int sendMessage1N2intByName(objName, messg, a, b)
    char *objName;
    char *messg;
    int a, b;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessage1N2int(theObj, messg, a, b);
}

int sendMessage1N4intByName(objName, messg, a, b, c, d)
    char *objName;
    char *messg;
    int a, b, c, d;
{
    VObj *theObj = findObject(getIdent(objName));

    if (!theObj) return 0;

    return sendMessage1N4int(theObj, messg, a, b, c, d);
}

/*-------------*/
/* UNUSED CODE
for (; cip; cip = cip->superClass) {
  if (cip->mhp.bits[funcid % cip->mhp.bitsSize] & 
	(1 << funcid % sizeof(long))) {
	HashEntry *entry, *base_entry;
	base_entry = &(cip->mht->entries[funcid % cip->mht->size]);
	for (entry = base_entry; entry; entry = entry->next) {
		if (entry->label) {
		  if (entry->label == funcid) {
			entry = cip->mht->get(cip->mht, (long)funcid);
			if (entry) {
			  if (entry->val) {
				((long (*)())(entry->val))(self,
				  &reg1, argc, 
			          &execStack[stackExecIdx - argc + 1]);
				goto doneCall;
			  }
			} 
		  } 
		}
	}
  }
}
*/

#ifdef NOT_USED
int ASSERT(assertion, mesg) 
	char *mesg;
{
	if (assertion == 0) {
		fprintf(stderr, "%s", mesg);
		fflush(stderr);
	}
	return assertion;
}
#endif
