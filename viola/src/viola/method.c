/*
 * method.c
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
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "attr.h"
#include "cexec.h"
#include "slotaccess.h"

int fcount = 0;
/* 
 * optimization for later... use a hash table to hash unique method str id's.
 * the value of the hash node is an dispatch array, according to 
 * class-offset-id, of pointers to each class's method... thus, the only
 * search required is in the hashing to get the dispatch array.
 */
long callMeth(self, result, argc, argv, funcid)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int funcid;
{
	ClassInfo *cip = GET__classInfo(self);
/*	MethodInfo *mip;*/
	HashEntry *entry = &(cip->mht->entries[funcid % cip->mht->size]);

	for (; entry; entry = entry->next) {
		if (entry->label == funcid) {
			if (entry->val) {
			 	if (result->canFree & PK_CANFREE_STR) {
/*print("### (callMeth) free'ing, count=%d\n", fcount++);
*/
					free(result->info.s);/*XXX*/
					result->canFree = 0;
				}
				((long (*)())(entry->val))
					(self, result, argc, argv);
				return 1;
			} 
		}
	}	
	return 0;
}

int findMeth(self, funcid)
	VObj *self;
	int funcid;
{
	ClassInfo *cip = GET__classInfo(self);
	HashEntry *entry = &(cip->mht->entries[funcid % cip->mht->size]);

	for (; entry; entry = entry->next) {
		if (entry->label == funcid) {
			if (entry->val) {
				return entry->val;
			} 
		}
	}	
	return 0;
}


#ifdef oldstuffunuseddonwantgoodbyerusty
long callMeth(self, result, argc, argv, fid)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int fid;
{
	ClassInfo *cip;
	MethodInfo *mip;
	HashEntry *entry;

	for (cip = GET__classInfo(self); cip; cip = cip->superClass) {

		{
		int byte = cip->mhp.bits[fid % cip->mhp.bitsSize];
		int nthBits = fid % sizeof(long);
		int result;

/*		cip->mhp.refc++;*/

		result = byte & (1 << nthBits);

/*		if (result) cip->mhp.hits++;
		else cip->mhp.misses++;
*/
		}

		if (cip->mhp.bits[fid % cip->mhp.bitsSize] 
			& (1 << fid % sizeof(long))) {
/*
		if (isMember(&(cip->mhp), fid)) {
			printf("callMeth: hit  \t\tfid=%d\n", fid);
*/
			entry = cip->mht->get(cip->mht, (long)fid);
			if (entry) {
				if (entry->val) {
					((long (*)())(entry->val))
						(self, result, argc, argv);
					return 1;
				}
/*				printf("!!!!!!!!!!!!!!!!!!B  callMeth: how could this be? fid=%d\n", fid);*/
			} 
/*			printf("!!!!!!!!A callMeth: how could this be? fid=%d\n", fid);*/

/*			for (mip = cip->methods; mip->id; mip++) {
				if (mip->id == fid) {
					((long (*)())(mip->method))
						(self, result, argc, argv);
					return 1;
				}
			}
*/
		} else {
/*			printf("callMeth: miss \t\tfid=%d\n", fid);*/
			continue;
		} 
	}
	return 0;
}
#endif
