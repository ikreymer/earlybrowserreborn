/*
 * attr.h
 */
typedef struct Attr {
	struct Attr *next;	/* must be the first item  (see vlist.h) */
	long id;		/* must be the second item (see vlist.h) */
	long val;
} Attr;

Attr *makeAttr();

/*
 * scans and return the attribute node identified by ``id''.
 */
#define scanAttr(head_attr, id)\
	(Attr*)scanVListNode((VList**)head_attr, id, cmp_int)

/*
 * remove and return the attribute node identified by ``id''.
 */
#define removeAttr(head_attr, id)\
	(Attr*)removeVListNode((VList**)head_attr, id, cmp_int)

/*
 * prepend and return the attribute node.
 */
#define prependAttr(head_attr, attr)\
	(Attr*)prependVListNode((VList**)head_attr, (VList*)attr)

