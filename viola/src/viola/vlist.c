/*
 * vlist.c
 */
#include "utils.h"
#include "mystrings.h"
#include "vlist.h"

/*
 * scans and return the listi node identified by ``id'', 
 * using cmpf as the comparison function.
 */
VList *scanVListNode(head_list, id, cmpf)
	VList **head_list;
	long id;
	int (*cmpf)();
{
	if (*head_list) {
		VList *list;
		for (list = *head_list; list; list = list->next)
			if (cmpf(list->id, id)) return list;
	}
	return 0;
}

/*
 * remove and return the list node identified by ``id'', 
 * using cmpf as the comparison function.
 */
VList *removeVListNode(head_list, id, cmpf)
	VList **head_list;
	long id;
	int (*cmpf)();
{
	if (*head_list) {

		VList *list, *prev_list = NULL;

		for (list = *head_list; list; list = list->next) {
			if (cmpf(list->id, id)) {
				if (prev_list) {
					prev_list->next = list->next;
				} else {
					*head_list = (*head_list)->next;
				}
				return *head_list;
			}
			prev_list = list;
		}
	}
	return 0;
}

VList *prependVListNode(head_list, list)
	VList **head_list;
	VList *list;
{
	if (*head_list) list->next = *head_list;
	*head_list = list;
	return list;
}

