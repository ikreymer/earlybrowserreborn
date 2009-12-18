#include "utils.h"
#include "mystrings.h"
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "../libStyle/libstg.h"

STGLib *stgLib;
STGGroup *stgGroup;

HashTable *HT_str2token;
HashTable *HT_token2str;

char *returnSame(a)
	char *a;
{
	return a;
}

int tagName2ID(s)
	char *s;
{
	HashEntry *entry;
	static int idCount = 0;

	entry = HT_str2token->get(HT_str2token, (int)s);
	if (entry) return entry->val;

	entry = HT_str2token->put(HT_str2token, (int)saveString(s), (int)++idCount);
	if (!entry) return 0;

	entry = putHashEntry(HT_token2str, (int)idCount, (int)saveString(s));
	if (!entry) return 0;
	return idCount;
}

char *tagID2Name(id)
	int id;
{
	HashEntry *entry = HT_token2str->get(HT_token2str, (int)id);
	if (entry) return (char*)entry->val;
	return NULL;
}


int stgcall_init()
{
	HT_str2token = initHashTable(100, hash_str, cmp_str, NULL, NULL,
				     getHashEntry_str, 
				     putHashEntry_str, 
				     putHashEntry_replace_str,
				     removeHashEntry_str);
	if (!HT_str2token) return 0;
	HT_token2str = initHashTable(100, hash_int, cmp_int, NULL, NULL,
				     getHashEntry_int, 
				     putHashEntry_int, 
				     putHashEntry_replace_int,
				     removeHashEntry_int);
	if (!HT_token2str) return 0;

	stgLib = STG_init(cmp_int, tagName2ID, tagID2Name,
			  cmp_int, tagName2ID, tagID2Name);
	if (!stgLib) return 0;

	return 1;
}

/*XXX now url is filename 
 */
int loadSTG(url)
	char *url;
{
	char *spec;
	char inFile[500];
	int stat;

	/* load a stylesheet
	 */
	strcpy(inFile, url);
	stat = loadFile(inFile, &spec);

	if (stat == -1) {
	  printf("failed to open %s\n", inFile);
	  return 0;
	}
	stgGroup = STG_makeGroup(stgLib, spec);

	free(spec);
	return (stgGroup ? 1 : 0);
}

/* 
 * access to libstg.
 */
int getSTGInfo_tagPtr(result, tagName1, tagName2)
	Packet *result;
	char *tagName1;
	char *tagName2; /* super element */
{
	int stat;
	char *context[4];
	STGResult results[2];

	clearPacket(result);

	if (!stgGroup) return 0;

	/* XXX BUG: no context spec
	 */
	context[0] = tagName2ID(tagName1);
	context[1] = NULL;
	if (tagName2) {
		context[2] = tagName2ID(tagName2);
		context[3] = NULL;
		stat = STG_findStyle(stgGroup, context, 2, results, 2);
	} else {
		stat = STG_findStyle(stgGroup, context, 1, results, 1);
	}
	if (stat) {
		result->info.i = results[0].smajor;	/*XXX*/
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;
	}
	return 0;
}

int getSTGInfo_attr(tagPtr, attrName, result)
	STGMajor *tagPtr;
	char *attrName;
	Packet *result;
{
	STGAssert *assert;

	clearPacket(result);
	if (!tagPtr) return 0;
	if (!attrName) return 0;
	assert = STGFindAssert(tagPtr, tagName2ID(attrName));
	if (assert) {
		result->info.s = assert->val;
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;
	}
	return 0;
}

int getSTGInfo_clean(tagPtr)
	STGMajor *tagPtr;
{
	freeGroup(stgLib, &stgGroup);

	return 1;
}

/* 
 * access to libstg.
 */
int getSTGInfo(tagName, attrName, result)
	char *tagName;
	char *attrName;
	Packet *result;
{
	int stat;
	char *context[20];
	STGResult results[2];
	STGAssert *assert;

	clearPacket(result);

	/* BUG no context spec
	 */
	context[0] = tagName2ID(tagName);
	context[1] = NULL;

	if (!stgGroup) return 0;

	stat = STG_findStyle(stgGroup, context, 1, results, 1);

	if (stat) {
		assert = STGFindAssert(results[0].smajor, tagName, attrName);
		if (assert) {
			result->info.s = assert->val;
			result->type = PKT_STR;
			result->canFree = 0;
			return 1;
		}
	}
	return 0;
}
