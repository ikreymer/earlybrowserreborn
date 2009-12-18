#ifndef _VIOLA_HASH_H_
#define _VIOLA_HASH_H_

/* hash.h
 *
 */

typedef struct HashEntry {
	struct HashEntry 	*next;
	long			label;
	long			val;
} HashEntry;

typedef struct HashTable {
	HashEntry	*entries;
	int 		size;
	long		(*func_hash)();
	long		(*func_cmp)();
	long		(*func_freeLabel)();
	long		(*func_freeVal)();
	HashEntry *	(*get)();
	HashEntry *	(*put)();
	HashEntry *	(*put_replace)();
	int		(*remove)();
} HashTable;

int hash_int();
int hash_str();
HashTable *initHashTable();

HashEntry *putHashEntry();
HashEntry *putHashEntry_replace();
HashEntry *getHashEntry();
int removeHashEntry();

HashEntry *putHashEntry_cancelable_int();

HashEntry *putHashEntry_int();
HashEntry *putHashEntry_replace_int();
HashEntry *getHashEntry_int();
int removeHashEntry_int();

HashEntry *putHashEntry_str();
HashEntry *putHashEntry_replace_str();
HashEntry *getHashEntry_str();
int removeHashEntry_str();

void dumpHashTable();

#endif _VIOLA_HASH_H_
