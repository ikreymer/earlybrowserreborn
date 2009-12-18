/*
 * attr.c
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
#include "mystrings.h"
#include "vlist.h"
#include "attr.h"

Attr *makeAttr(id, val)
	long id;
	long val;
{
	Attr *attr = (Attr*)malloc(sizeof(struct Attr));
	if (attr) {
		attr->id = id;
		attr->val = val;
		attr->next = NULL;
		return attr;
	}
	fprintf(stderr, "malloc() failed\n");
	return NULL;
}
