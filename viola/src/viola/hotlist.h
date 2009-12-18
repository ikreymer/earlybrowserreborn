typedef struct HotListItem {
	char *url;
	char *comment;
	char *date; /* ugh */
	struct HotListItem *next;
} HotListItem;

extern HotListItem *theHotList;
extern int theHotListCount;
