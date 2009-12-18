/*
 * box.c
 *
 * Generic container for structures.
 *
 */
/*
 * Copyright 1993 O'Reilly & Associates. All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */

#include <stdio.h>

#define _NOBOOLEAN_
#include "box.h"


Box *cloneBox(box)
    Box *box;
{
    Box *newone=NULL, *bp;

    if (!box)
	return(NULL);

    newone = bp = (Box *) malloc(sizeof(Box));
    bp->data = box->data;
    bp->dataIsCopy = 1;
    bp->next = NULL;
    
    while (box->next) {
	bp->next = (Box *) malloc(sizeof(Box));
	bp->next->data = box->next->data;
	bp->next->dataIsCopy = 1;
	bp->next->next = NULL;
	box = box->next;
	bp = bp->next;
    }

    return newone;
}
    


void putInBox(box, item)
    Box **box;
    void *item;
{
    if (!item) {
	return;
    } else {
	Box *bp = (Box *) malloc(sizeof(Box));
	bp->data = item;
	bp->dataIsCopy = 0;

	if (!*box) {
	    bp->next = NULL;
	    *box = bp;
	} else {
	    bp->next = *box;
	    *box = bp;
	}
    }
}


void *getFromBox(box, key, compare, findFirstOne)
    Box **box;
    void *key;
    CompareFunction compare;
    Boolean findFirstOne;
{
    static int whichOne=1;
    int i=0;
    Box *bp = *box;

    if (!*box || !key)
	return(NULL);

    if (findFirstOne)
	whichOne = 1;

    do {
	i++;
	while(bp && !compare(key, bp->data))
	    bp = bp->next;
	if (i != whichOne && bp) 
	    bp = bp->next;
    } while (i<whichOne && bp);

    if (bp) {
	whichOne++;
	return(bp->data);
    } else {
	whichOne = 1;
	return(NULL);
    }
}


void deleteFromBox(box, key, compare, freeData, deleteAllItems)
    Box **box;
    void *key;
    CompareFunction compare;
    FreeFunction freeData;
    Boolean deleteAllItems;
{
    Box *bp = *box;
    Box *prev = NULL;

    do {
	while (bp && !compare(key, bp->data)) {
	    prev = bp;
	    bp = bp->next;
	}
	
	if (bp) {
	    if (bp == *box) {
		*box = bp->next;
		if (!bp->dataIsCopy && freeData)
		    freeData(bp->data);
		free(bp);
		bp = *box;
	    } else {
		prev->next = bp->next;
		if (!bp->dataIsCopy && freeData)
		    freeData(bp->data);
		free(bp);
		bp = prev->next;
	    }
	}
    } while (bp && deleteAllItems);
}


void deleteBox(box, freeData)
    Box *box;
    FreeFunction freeData;
{
    if (!box)
	return;
    deleteFromBox(&box, NULL, alwaysTrue, freeData, TRUE);
}

    
Boolean alwaysTrue(vkey, vdata)
    void *vkey, *vdata;
{
    return(TRUE);
}
