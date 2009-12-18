/*
 * Copyright 1990 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */

/*
 * hash.c
 */
#include "utils.h"
#include "mystrings.h"
#include "hash.h"
/*
static int scatter[] = {
112269159,      1414846676,     156382461,      1455714,
872078403,      1874748096,     916896761,      769267518,
820312479,      92728044,       344858293,      1121741130,
1495896251,     208521688,      860811569,      1660932118,
525761943,      736209796,      493907821,      639686562,
654007347,      1905261552,     78746985,       1398328302,
856000655,      1452336924,     1573199653,     2013903098,
500965035,      1800624392,     2077067425,     1672112838
};
*/
int hash_int(ht, n)
	HashTable *ht;
	int n;
{
	return n % ht->size;
}

int hash_str(ht, str)
	HashTable *ht;
	char *str;
{
	int key = 0;

	while (*str) key += *str++;
	key = key % ht->size;

	return key;
}

/*
int hash_str(ht, str)
	HashTable *ht;
	char *str;
{
	int key = 0;

	while (*str) key = (key << 1) + scatter[*str++];
	key &= ht->size - 1;

	return key;
}
*/
HashTable *initHashTable(size, func_hash, func_cmp, 
			func_freeLabel, func_freeVal, 
			func_get, func_put, func_put_replace, 
			func_remove)
	long size;
	long (*func_hash)();
	long (*func_cmp)();
	long (*func_freeLabel)();
	long (*func_freeVal)();
	HashEntry * (*func_get)();
	HashEntry * (*func_put)();
	HashEntry * (*func_put_replace)();
	int (*func_remove)();
{
	struct HashTable *ht;
	HashEntry *entryp;
	int i;

	ht = (HashTable*)malloc(sizeof(struct HashTable));
	if (ht) {
		ht->entries = (HashEntry*)malloc(sizeof(struct HashEntry)
						 * size);
		ht->size = size;
		ht->func_hash = func_hash;
		ht->func_cmp = func_cmp;
		ht->func_freeLabel = func_freeLabel;
		ht->func_freeVal = func_freeVal;
		ht->get = func_get;
		ht->put = func_put;
		ht->put_replace = func_put_replace;
		ht->remove = func_remove;

		for (entryp = ht->entries, i = size - 1; i >= 0; i--) {
			entryp->next  = NULL;
			entryp->label = NULL;
			entryp->val   = NULL;
			entryp++;
		}
	}
	return ht;
}

#define PUT_HASH_ENTRY_BLOCK(ht, key, val)\
{\
	HashEntry *entry, *base_entry, *new_entry;\
	base_entry = &(ht->entries[key]);\
	if (base_entry->label) {\
		for (entry = base_entry; entry->next; entry = entry->next);\
		new_entry = (HashEntry*)malloc(sizeof(struct HashEntry));\
		if (new_entry) {\
			entry->next = new_entry;\
			new_entry->label = (long)label;\
			new_entry->val = val;\
			new_entry->next = NULL;\
		} else {\
			fprintf(stderr, "malloc failed.\n");\
		}\
		entry->next = new_entry;\
		return new_entry;\
	} else {\
		base_entry->label = (long)label;\
		base_entry->val = val;\
		base_entry->next = NULL;\
		return base_entry;\
	}\
}

HashEntry *putHashEntry(ht, label, val)
	HashTable *ht;
	long label;
	long val;
{
	int key = ht->func_hash(ht, label);
/*
	if (ht->func_hash == hash_str) {
		printf("label='%s' \tkey=%ld \tval=%d\n", label, key, val);
	} else {
		printf("label=%d \tkey=%ld \tval='%s'\n", label, key, val);
	}
*/
	PUT_HASH_ENTRY_BLOCK(ht, key, val);
}

HashEntry *putHashEntry_int(ht, label, val)
	HashTable *ht;
	long label;
	long val;
{
	int key = label % ht->size;

	PUT_HASH_ENTRY_BLOCK(ht, key, val);
}

HashEntry *putHashEntry_str(ht, label, val)
	HashTable *ht;
	char *label;
	long val;
{
	int key = 0;
	char *str = label;

	while (*str) key += *str++;
	key = key % ht->size;

	PUT_HASH_ENTRY_BLOCK(ht, key, val);
}

HashEntry *putHashEntry_cancelable_int(ht, label, val)
	HashTable *ht;
	long label;
	long val;
{
	int key;
	HashEntry *entry, *base_entry, *new_entry;

	key = label % ht->size;
	base_entry = &(ht->entries[key]);

	if (base_entry->label) {
		for (entry = base_entry; entry->label; entry = entry->next) {
			if (entry->label == label) {
				/* nevermind. entry of same id already exist */
				return base_entry;
			}
			if (!entry->next) break;
		}
		new_entry = (HashEntry*)malloc(sizeof(struct HashEntry));
		if (new_entry) {
			entry->next = new_entry;
			new_entry->label = label;
			new_entry->val = val;
			new_entry->next = NULL;
		} else {
			fprintf(stderr, "malloc() failed.\n");
		}
		entry->next = new_entry;
		return new_entry;
	} else {
		base_entry->label = label;
		base_entry->val   = val;
		base_entry->next  = NULL;
		return base_entry;
	}
}

#define PUT_HASH_ENTRY_REPLACE(ht, key, val)\
{\
	HashEntry *entry, *base_entry, *new_entry;\
	base_entry = &(ht->entries[key]);\
\
	if (base_entry->label) {\
		for (entry = base_entry; entry->label; entry = entry->next) {\
			if (entry->label == label) {\
				/* override */\
				entry->val = val;\
				return base_entry;\
			}\
			if (!entry->next) break;\
		}\
		new_entry = (HashEntry*)malloc(sizeof(struct HashEntry));\
		if (new_entry) {\
			new_entry->label = label;\
			new_entry->val = val;\
			new_entry->next = NULL;\
		} else {\
			fprintf(stderr, "malloc() failed.\n");\
		}\
		entry->next = new_entry;\
		return new_entry;\
	} else {\
		base_entry->label = label;\
		base_entry->val = val;\
		base_entry->next = NULL;\
		return base_entry;\
	}\
}

HashEntry *putHashEntry_replace(ht, label, val)
	HashTable *ht;
	long label;
	long val;
{
	int key = ht->func_hash(ht, label);
	PUT_HASH_ENTRY_REPLACE(ht, key, val);
}

HashEntry *putHashEntry_replace_int(ht, label, val)
	HashTable *ht;
	long label;
	long val;
{
	int key = label % ht->size;
	PUT_HASH_ENTRY_REPLACE(ht, key, val);
}

HashEntry *putHashEntry_replace_str(ht, label, val)
	HashTable *ht;
	char *label;
	long val;
{
	int key = 0;
	char *str = label;

	while (*str) key += *str++;
	key = key % ht->size;

	PUT_HASH_ENTRY_REPLACE(ht, key, val);
}

#ifdef NOT_USED
HashEntry *getHashEntry(ht, label)
	HashTable *ht;
	long label;
{
	int key;
	HashEntry *entry, *base_entry;

	if (!ht) {
		printf("Internal error: ht==NULL\n");
		return 0;
	}

/*	key = ht->func_hash(ht, label);*/

	if (ht->func_hash == hash_str) {
		char *str = label;

		key = 0;
		while (*str) key += *str++;
		key = key % ht->size;
/*
		key = 0;
		while (*str) key = (key << 1) + scatter[*str++];
		key &= ht->size - 1;
*/
	} else if (ht->func_hash == hash_int) {
		key = label % ht->size;
	}

	base_entry = &(ht->entries[key]);

	if (ht->func_cmp == cmp_str) {
		for (entry = base_entry; entry; entry = entry->next) {
			if (entry->label) {
				if (((char*)entry->label)[0] 
				    == ((char*)label)[0]) {
					if (!strcmp((char*)entry->label, 
						   (char*)label)) return entry;
				}
			}
		}
	} else if (ht->func_cmp == cmp_int) {
		for (entry = base_entry; entry; entry = entry->next) {
/*!!!! next line MUST be present, else things seems to not work right. very strance... must be using entry-existence-only value somewhere */
			if (entry->label)

				if (entry->label == label) return entry;
		}
	} else {
		for (entry = base_entry; entry; entry = entry->next) {
			if (ht->func_cmp(entry->label, label)) return entry;
		}
	}
/*
	if (ht->func_hash == hash_str) {
	     printf("key=%d \tlabel=\"%s\" \tval=%d\n",
		key, (char*)label, entry->val);
	} else {
	    printf("key=%d \tlabel=%d \tval=0x%x\n",
		key, entry->label, entry->val);
	}	
*/
	return NULL;
}
#endif

HashEntry *getHashEntry_str(ht, label)
	HashTable *ht;
	char *label;
{
	HashEntry *entry;
	int key = 0;
	char *str = label;

	if (label == 0) return NULL;

	while (*str) key += *str++;
	key = key % ht->size;

	for (entry = &(ht->entries[key]); entry; entry = entry->next)
		if (entry->label)
			if (((char*)entry->label)[0] == ((char*)label)[0])
				if (!strcmp((char*)entry->label, (char*)label))
					return entry;
	return NULL;
}

HashEntry *getHashEntry_int(ht, label)
	HashTable *ht;
	long label;
{
	HashEntry *entry;
	int key;

	if (label == 0) return NULL;

	key = label % ht->size;
	for (entry = entry = &(ht->entries[key]); entry; entry = entry->next)
		if (entry->label == label) return entry;

	return NULL;
}

#ifdef NOT_USED
int removeHashEntry(ht, label)
	HashTable *ht;
	long label;
{
	int key;
	HashEntry *entry, *base_entry, *prevEntry;

	key = ht->func_hash(ht, label);
	base_entry = &(ht->entries[key]);

	prevEntry = NULL;
	for (entry = base_entry; entry; entry = entry->next) {
		if (entry->label) {
			if (ht->func_cmp(entry->label , label)) {
/*
				if (ht->func_hash == hash_str) {
			printf("REMOVE key=%d \tlabel=\"%s\" \tval=%d\n",
					key, (char*)entry->label, entry->val);
				} else {
			printf("REMOVE key=%d \tlabel=%d \tval=0x%x\n",
					key, entry->label, entry->val);
				}
*/
				if (ht->func_freeLabel) 
					ht->func_freeLabel(entry->label);
				if (ht->func_freeVal)
					ht->func_freeVal(entry->val);

				if (prevEntry) {
					prevEntry->next = entry->next;
					free(entry);
				} else {
					if (entry->next) {
					    entry->label = entry->next->label;
					    entry->val = entry->next->val;
					    entry->next = entry->next->next;
					} else {
					    entry->label = NULL;
					    entry->val = NULL;
					}
				}
				return 1;
			}
		}
		prevEntry = entry;
	}
	return NULL;
}
#endif

int removeHashEntry_int(ht, label)
	HashTable *ht;
	long label;
{
	int key;
	HashEntry *entry, *prevEntry;

	if (label == 0) return 0;

	key = label % ht->size;

	prevEntry = NULL;
	for (entry = &(ht->entries[key]); entry; entry = entry->next) {
		if (entry->label == label) {
			if (ht->func_freeLabel) 
				ht->func_freeLabel(entry->label);
			if (ht->func_freeVal)
				ht->func_freeVal(entry->val);
			if (prevEntry) {
				prevEntry->next = entry->next;
				free(entry);
			} else {
				if (entry->next) {
				    entry->label = entry->next->label;
				    entry->val = entry->next->val;
				    entry->next = entry->next->next;
				} else {
				    entry->label = NULL;
				    entry->val = NULL;
				}
			}
			return 1;
		}
		prevEntry = entry;
	}
	return 0;
}

int removeHashEntry_str(ht, label)
	HashTable *ht;
	char *label;
{
	HashEntry *entry, *prevEntry;
	int key;
	char *str;

	if (label == 0) return 0;

	str = label;
	key = 0;
	while (*str) key += *str++;
	key = key % ht->size;

	prevEntry = NULL;
	for (entry = &(ht->entries[key]); entry; entry = entry->next) {
	    if (entry->label) {
		if (((char*)(entry->label))[0] == ((char*)label)[0]) {
			if (!strcmp((char*)(entry->label), label)) {
				if (ht->func_freeLabel)
					ht->func_freeLabel(entry->label);
				if (ht->func_freeVal)
					ht->func_freeVal(entry->val);
				if (prevEntry) {
					prevEntry->next = entry->next;
					free(entry);
				} else {
					if (entry->next) {
					    entry->label = entry->next->label;
					    entry->val = entry->next->val;
					    entry->next = entry->next->next;
					} else {
					    entry->label = NULL;
					    entry->val = NULL;
					}
				}
				return 1;
			}
		}
	    }
	    prevEntry = entry;
	}
	return 0;
}

void dumpHashTable(ht)
	HashTable *ht;
{
	int i;
	HashEntry *hp;

	printf("size = %d\n", ht->size);

	for (i = 0; i < ht->size; i++) {
		for (hp = &(ht->entries[i]); hp; hp = hp->next)
			if (hp->label)
				if (ht->func_hash == hash_str) {
					printf("%d\tlabel=\"%s\"\tval=%d\n", 
						i, 
						(char*)hp->label,
						(int)hp->val);
				} else {
					printf("%d\tlabel=%d\tval=0x%x\n", 
						i,
						(int)hp->label,
						hp->val);
				}
	}
}
