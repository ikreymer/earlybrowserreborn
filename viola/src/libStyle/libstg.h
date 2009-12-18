/*
 * Stylesheet Library (libstg.h)
 *
 * Pei Y. Wei (pei@ora.com), O'Reilly & Associates
 * 
 * 
 * Refer to http://.... for more documentation and usage.
 * 
 * 
 * 
 */

/**********************************/

typedef struct STGStrList {
	struct STGStrList *next;
	char *val;
} STGStrList;

typedef struct STGAssert {
	struct STGAssert *next;
	char *name;		/* replace with hashtable token */
	char *val;
} STGAssert;

typedef struct STGMajor {
	struct STGMajor *next;
	struct STGMajor *super;
	struct STGStrList *IDList;
	struct STGAssert *firstAssert;
	struct STGMajor *firstMajorChild;
	struct STGMinor *firstMinorChild;
} STGMajor;

typedef struct STGMinor {
	struct STGMinor *next;
	struct STGStrList *IDList;
	struct STGAssert *firstAssert;
	struct STGMajor *super;
} STGMinor;

typedef struct STGResult {
	struct STGMajor *smajor;
	struct STGMinor *sminor;
} STGResult;

typedef struct STGGroup {
	struct STGGroup *next;
	struct STGMajor *first;
	char *DTD;
} STGGroup;

typedef struct STGLib {
	struct STGGroup *first;
} STGLib;

void STG_dumpLib();
void STG_dumpGroup();
void STG_dumpMajor();
void STG_dumpMinor();
STGMajor *parseMajor();
STGMinor *parseMinor();
STGGroup *addGroup();
STGMajor *addMajor();
STGMinor *addMinor();
STGAssert *assertAttr();
int nextToken();

STGLib *STG_init();
STGGroup *STG_makeGroup();
int STG_findStyle();
int STG_findStyle1();
STGAssert *STGFindAssert();

int freeGroup();
int freeLib();
