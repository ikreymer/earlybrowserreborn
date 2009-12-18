/*
 * html2.c
 *
 * THIS CODE IS IN EXTREME STATE OF ENTROPY. MUST REVIEW.
 */
#include "utils.h"
#include <ctype.h>

#include "HTFont.h"

char *HTAppName = "ViolaWWW";
char *HTAppVersion = "3.1";

#define PRINT_TAG_DELIMS
int printTagDelims = 0;

int http_method;
char *http_dataToPost;

int http_progress_reporter_level = 0;
int http_progress_expected_total_bytes = 0;
int http_progress_total_bytes = 0;
int http_progress_subtotal_bytes = 0;

#undef HTSTRING_H
#include "../libWWW/Library/Implementation/HTString.h"
#include "../libWWW/Library/Implementation/HTTCP.h"
#include "../libWWW/Library/Implementation/HTAnchor.h"
#include "../libWWW/Library/Implementation/HTParse.h"
#include "../libWWW/Library/Implementation/HTAccess.h"
#include "../libWWW/Library/Implementation/HTHistory.h"
#include "../libWWW/Library/Implementation/HText.h"
#include "../libWWW/Library/Implementation/SGML.h"
#include "../libWWW/Library/Implementation/HTML.h"
#include "../libWWW/Library/Implementation/HTMLDTD.h"

#undef HTWRITE_H
#include "../libWWW/Library/Implementation/HTWriter.h"
#include "../libWWW/Library/Implementation/HTFile.h"

#include "mystrings.h"
#include "sys.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "vlist.h"
#include "attr.h"
#include "glib.h"
#include "event.h"
#include "slib.h"
#include "misc.h"
#include "tfed.h"
#include "sgml.h"
#include "html.h"
#include "html2.h"
#include "DefaultStyles.h"

extern VObj *insert_obj;	/* from sgml.c */
extern int insert_width;
extern Packet scrapPk;
extern SGMLDocMappingInfo SGMLForms[];

VObj *mesgObj = NULL;

SGMLBuildInfo SBI;

enum sgmlsAttributeTypes {
	SGMLS_ATTR_IMPLIED,
	SGMLS_ATTR_CDATA,
	SGMLS_ATTR_TOKEN,
	SGMLS_ATTR_ID,
	SGMLS_ATTR_IDREF,
	SGMLS_ATTR_ENTITY,
	SGMLS_ATTR_NOTATION
};

int sgml_verbose = 0;

static char *URLEncodeSeq[] = {	
	NULL,	"%01",	"%02",	"%03",	"%04",	"%05",	"%06",	"%07",
	"%08",	"%09",	"%0A",	"%0B",	"%0C",	"%0D",	"%0E",	"%0F",
	"%10",	"%11",	"%12",	"%13",	"%14",	"%15",	"%16",	"%17",
	"%18",	"%19",	"%1A",	"%1B",	"%1C",	"%1D",	"%1E",	"%1F",
	"+",	"%21",	"%22",	"%23",	"%24",	"%25",	"%26",	"%27",
	"%28",	"%29",	"*",	"%2B",	"%2C",	"-",	".",	"%2F",
	"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",
	"8",	"9",	"%3A",	"%3B",	"%3C",	"%3D",	"3E",	"%3F",
	"@",	"A",	"B",	"C",	"D",	"E",	"F",	"G",
	"H",	"I",	"J",	"K",	"L",	"M",	"N",	"O",
	"P",	"Q",	"R",	"S",	"T",	"U",	"V",	"W",
	"X",	"Y",	"Z",	"%5B",	"%5C",	"%5D",	"%5E",	"_",
	"%60",	"a",	"b",	"c",	"d",	"e",	"f",	"g",
	"h",	"i",	"j",	"k",	"l",	"m",	"n",	"o",
	"p",	"q",	"r",	"s",	"t",	"u",	"v",	"w",
	"x",	"y",	"z",	"%7B",	"%7C",	"%7D",	"%7E",	"%7F",
	"%80",	"%81",	"%82",	"%83",	"%84",	"%85",	"%86",	"%87",	
	"%88",	"%89",	"%8A",	"%8B",	"%8C",	"%8D",	"%8E",	"%8F",	
	"%90",	"%91",	"%92",	"%93",	"%94",	"%95",	"%96",	"%97",	
	"%98",	"%99",	"%9A",	"%9B",	"%9C",	"%9D",	"%9E",	"%9F",	
	"%A0",	"%A1",	"%A2",	"%A3",	"%A4",	"%A5",	"%A6",	"%A7",	
	"%A8",	"%A9",	"%AA",	"%AB",	"%AC",	"%AD",	"%AE",	"%AF",	
	"%B0",	"%B1",	"%B2",	"%B3",	"%B4",	"%B5",	"%B6",	"%B7",	
	"%B8",	"%B9",	"%BA",	"%BB",	"%BC",	"%BD",	"%BE",	"%BF",	
	"%C0",	"%C1",	"%C2",	"%C3",	"%C4",	"%C5",	"%C6",	"%C7",	
	"%C8",	"%C9",	"%CA",	"%CB",	"%CC",	"%CD",	"%CE",	"%CF",	
	"%D0",	"%D1",	"%D2",	"%D3",	"%D4",	"%D5",	"%D6",	"%D7",	
	"%D8",	"%D9",	"%DA",	"%DB",	"%DC",	"%DD",	"%DE",	"%DF",	
	"%E0",	"%E1",	"%E2",	"%E3",	"%E4",	"%E5",	"%E6",	"%E7",	
	"%E8",	"%E9",	"%EA",	"%EB",	"%EC",	"%ED",	"%EE",	"%EF",	
	"%F0",	"%F1",	"%F2",	"%F3",	"%F4",	"%F5",	"%F6",	"%F7",	
	"%F8",	"%F9",	"%FA",	"%FB",	"%FC",	"%FD",	"%FE",	"%FF",
};
char URLEncodeBuff[1000];

/***** hot list defs/decls *****/

#define HBUFF_SIZE 100
char hbuff[HBUFF_SIZE];

int hbuffi = 0, hbuffLimit = 0;
char line[500];
int linei;

HotListItem *theHotList = NULL;
FILE *HotList_fp;
char *HotList_path = NULL;
int theHotListCount = 0;

int HotListLoadedP = 0;

/***** end of hot list defs/decls *****/

int init_html2()
{
  return 1;
}


SGMLTagMappingInfo *findTMI(tagMappingInfo, tag)
	SGMLTagMappingInfo *tagMappingInfo;
	char *tag;
{
	int i;

	for (i = 0; tagMappingInfo[i].tag; i++)
		if (!STRCMP(tag, tagMappingInfo[i].tag))
			return &tagMappingInfo[i];
	return NULL;
}

/*
 */
VObj *html2_parseHTMLDocument(self, address, simpleAddress, anchorSearch, 
			      width, method, dataToPost)
	VObj *self;
	char *address;
	char **simpleAddress;
	char **anchorSearch;
	int width;
	int  method;
	char *dataToPost;
{
	SGMLBuildInfoState *bstate;
	char c, anchorInfo[64];
	extern char *current_addr;
	int i, stat;

	trimEdgeSpaces(address);

	/* get full address, without anchor */
	*simpleAddress = saveString(HTParse(address, current_addr,
					    PARSE_ACCESS | PARSE_HOST | 
					    PARSE_PATH | PARSE_PUNCTUATION));
	HTSimplify(*simpleAddress);

	if (current_addr) free(current_addr);
	current_addr = saveString(*simpleAddress);

	/* extract anchor info, if any */
	*anchorSearch = "";
	for (i = 0; c = address[i]; i++) {
		if (c == '#') {
			strcpy(anchorInfo, address + i + 1);
			*anchorSearch = saveString(anchorInfo);
			break;
		}
	}

	/* prepare SGMLBuildInfo structure
	 */
	SBI.stacki = 0;
	SBI.endP = 0;
	SBI.caller = self;

	bstate = &SBI.stack[SBI.stacki];
	bstate->tmi = NULL;
	bstate->dataBuffIdx_localStart = 0;
	bstate->tag = NULL;
	bstate->parent = NULL;
	bstate->obj = self;
	bstate->width = bstate->sub_width = width;
	bstate->y = bstate->sub_y = 0;
	bstate->height = bstate->sub_height = 0;
	bstate->insert = NULL;

	HTOutputStream = NULL;
	HTOutputFormat = NULL;
	HTOutputSource = NO;

	CB_HTML_new();

	http_method = method;
	http_dataToPost = dataToPost;
	HTPushInputBuffer();
	stat = HTLoadAbsolute(*simpleAddress);
	http_method = HTTP_METHOD_GET;
	http_dataToPost = NULL;
	HTPopInputBuffer();

	if (stat) return SBI.stack[0].obj;
	/* delete objects (SBI.stack[0].obj) */
	return NULL;
}

void CB_HTML_new()
{
	SGMLBuildInfoState *bstate;
	int i, j, tagCount, idx;
	char *dtd = "HTML", *tag;
	SGMLDocMappingInfo *docMappingInfo = NULL;
	SGMLTagMappingInfo *tmi, *tagMappingInfo = NULL;
	SGMLDocMappingInfo *dmi = NULL;
/*	SGML_dtd sgmldtd;*/
	int *tag2StyleIndexMap;

/*printf("CB_HTML_new\n");*/

	if (!STRCMP(dtd, "HTML")) {
/*		sgmldtd = HTML_dtd;*/
	} else {
		fprintf(stderr, 
			 "Unknown DTD %s, not hard coded!\n", dtd);
		return;
	}
	for (i = 0; SGMLForms[i].DTDName; i++) {
		if (!STRCMP(SGMLForms[i].DTDName, dtd)) {
			dmi = &SGMLForms[i];
			break;
		}
	}
	if (!dmi) {
		char *stylesheetFN[128];
		extern char *sgmlPath;

		sprintf(stylesheetFN, "%s/styles/%s.style", sgmlPath, dtd);

		if (sgml_verbose) 
			fprintf(stderr, 
				"loading stylesheet \"%s\" for dtd \" %s\".\n",
				stylesheetFN, dtd);

		dmi = setSGMLStylesheet(dtd, stylesheetFN);
		if (!dmi) {
			fprintf(stderr, 
			 "Failed to read stylesheet \"%s\" for dtd \" %s\".\n",
				stylesheetFN, dtd);
			return;
		}

		/* setup the tag->styleObj map 
		 */
		tagCount = HTML_dtd.number_of_tags;
		tag2StyleIndexMap = (int*)malloc(sizeof(int) * tagCount);
		dmi->tag2StyleIndexMap = tag2StyleIndexMap;

/*		fprintf(stdout,	"DTD=%s tagCount =%d\n", dtd, tagCount);*/

		tagMappingInfo = dmi->tagMap;

		for (i = 0; i < tagCount; i++) {
			tag = HTML_dtd.tags[i].name;
			idx = -1;
			for (j = 0; tagMappingInfo[j].tag; j++) {

				if (!STRCMP(tag, tagMappingInfo[j].tag)) {
					idx = j;
					break;
				}
			}
			if (idx == -1) {
				fprintf(stdout,
	"Warning: element %s of DTD %s is not defined in stylesheet %s.\n",
					tag, dtd, stylesheetFN);
				tag2StyleIndexMap[i] = -1;
			} else {
				tag2StyleIndexMap[i] = idx;
			}
/*			fprintf(stdout,
				"%d tag={%s} idx=%d\n", 
				i, HTML_dtd.tags[i].name, idx);
*/
		}
	}
	tagMappingInfo = dmi->tagMap;
/*
	for (i = 0; tagMappingInfo[i].tag; i++) {
		fprintf(stdout,
			"%d {%s}\n", 
			i, tagMappingInfo[i].tag);
	}
*/
	dataBuffIdxStackIdx = 0;
	dataBuffIdxStack[dataBuffIdxStackIdx] = 0;
	dataBuffIdx = 0;

	SBI.stacki = 0;
	SBI.endP = 0;
	SBI.dmi = dmi;

	SBI.dmi->tagMap = dmi->tagMap;

	bstate = &SBI.stack[SBI.stacki];

	bstate->parent = SBI.caller;
	bstate->dataBuffIdx_localStart = dataBuffIdx;
	bstate->tag = "_HTML";
	bstate->tmi = findTMI(SBI.dmi->tagMap, bstate->tag);

	if (!bstate->tmi) {
		fprintf(stderr,
		  "CB_HTML_new: internal error: no tmi struct for tag=%s\n", 
			tag);
		return;
	}

	bstate->sub_width = bstate->width = GET_width(SBI.caller);

	HTMLBuildObj(bstate, bstate->width, bstate->tmi);

	bstate->sub_y = 0;
	bstate->height = 0;
	bstate->insert = NULL;
}

void CB_HTML_data(str, size)
	char *str;
	int size;
{
	SGMLBuildInfoState *bstate = &SBI.stack[SBI.stacki];

	if (bstate->obj) {
/*		fprintf(stderr, "@@@@ DATA(%d) -%s\n", size, str);
*/
		strncpy(&dataBuff[dataBuffIdx], str, size);

		dataBuff[dataBuffIdx + size] = '\0';

/*
printf("DATA size===%d\n", size);
printf("...obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(bstate->obj), dataBuffIdxStack[dataBuffIdxStackIdx], 
dataBuffIdx, &dataBuff[dataBuffIdx]);
}*/
		dataBuffIdx += size;
	} 
/*	else {
		fprintf(stderr, 
			"@@@@ NO object to receive: DATA(%d) -%s\n", 
			size, str);
	}
*/
}

void CB_HTML_special_entity(entity_number, data, dataLength)
	int entity_number;
	char *data;
	int dataLength;
{
	SGMLBuildInfoState *bstate;
	SGMLBuildInfoState *parent_bstate;

	parent_bstate = &SBI.stack[SBI.stacki-1];
	bstate = &SBI.stack[SBI.stacki];
/*
	printf("SPECIAL ENTITY: self=%s entity#=%d\n",
		GET_name(bstate->obj), entity_number);
*/

	if (bstate->obj && parent_bstate &&
	    (parent_bstate->tmi->flushAlwaysP ||
	     parent_bstate->tmi->flushOnSubTagP)) {
		int src_starti, src_endi, size;
		char *cp;
		int span;

		if (dataLength > 0) {
			cp = (char*)malloc(sizeof(char) * (dataLength + 1));
			strncpy(cp, data, dataLength);
			cp[dataLength] = '\0';
		} else {
			cp = (char*)malloc(sizeof(char));
			*cp = '\0';
		}

		if (GET_label(bstate->obj)) free(GET_label(bstate->obj));
		SET_label(bstate->obj, cp);
/*
		span = getVSpan(bstate->obj, STR_F);
*/
		sendMessage1N1int(bstate->obj, "entity", entity_number);
	}
}

void CB_HTML_stag(element_number, present, value, tagInfo)
	int element_number;
	BOOL *present;
	char **value;
	HTTag *tagInfo;
{
	SGMLBuildInfoState *parent_parent_bstate;
	SGMLBuildInfoState *parent_bstate;
	SGMLBuildInfoState *bstate;
	char *cp;
	char attrName[64];
	int attrTypeID;
	SGMLTagMappingInfo *ctmi;
	VObj *_insert_obj = insert_obj;
	int i, x, y, width;
	char *tag;
	SGMLTagMappingInfo *tmi;
	int hadFlushed = 0;
	char *str;
	int size, span, src_starti, src_endi;

	tag = HTML_dtd.tags[element_number].name;

#ifdef PRINT_TAG_DELIMS
	if (printTagDelims) {
		fprintf(stderr, "########################## HTML\t(%s\n", tag);
	}
#endif

	parent_bstate = &SBI.stack[SBI.stacki];
	SBI.stacki++;
	bstate = &SBI.stack[SBI.stacki];

/*	fprintf(stderr, "@@@@ stacki=%d\n", SBI.stacki);*/

	/* map HTML parser's tag ID (element_number) to stylesheet element */
	bstate->tmi = (SBI.dmi ? 
		&SBI.dmi->tagMap[SBI.dmi->tag2StyleIndexMap[element_number]]
		: NULL);

	if (!bstate->tmi) {
		fprintf(stderr,
			"CB_HTML_stag: no tmi struct found for tag=%s\n", tag);
		return;
	}

	/* make parent object its own-- the *first time* that the
	 * inline object becomes a container.
	 */
	if (parent_bstate->tmi->inlineP &&
	    !bstate->tmi->inlineP && /* don't copy if sub-obj is inlined */
	    parent_bstate->obj && !GET__children(parent_bstate->obj)) {
		extern int global_cloneID;
		Packet argv[2];
		VObjList *olist;
		VObj *parentParent;

		if (!parent_bstate->tmi->cloneMeth)
			parent_bstate->tmi->cloneMeth = 
			  findMeth(parent_bstate->tmi->template, STR_clone);
		if (!parent_bstate->tmi->template) {
			fprintf(stderr,
	"SGMLBuild_B: can't find clone method for template object '%s'\n",
				parent_bstate->tmi->type);
		}
		argv[0].info.i = global_cloneID++;
		argv[0].type = PKT_INT;
		argv[0].canFree = 0;
		if (parent_bstate->tmi->cloneMeth)
			parent_bstate->tmi->
				cloneMeth(parent_bstate->tmi->template, 
					  &scrapPk, 1, &argv[0]);

		parentParent = GET__parent(parent_bstate->obj);
		if (parentParent) {
			olist = GET__children(parentParent);
			if (olist) {
				for (;olist; olist = olist->next) {
/*printf(">>>%s\n", GET_name(olist->o));
*/
					if (olist->o == parent_bstate->obj) {
						olist->o = scrapPk.info.o;
/*printf(">>>>>>>>%s\n", GET_name(olist->o));
*/
						break;
					}
				}
			} else {
				/* unexpected error*/
			}
		}
		argv[0].info.s = "copyObj";
		argv[0].type = PKT_STR;
		argv[0].canFree = 0;
		argv[1].info.o = parent_bstate->obj;
		argv[1].type = PKT_OBJ;
		argv[1].canFree = 0;
		sendMessagePackets(scrapPk.info.o, argv, 2);
		parent_bstate->obj = scrapPk.info.o;

		/* because the n-1 level object has been turned into a
		 * container, n-2 level object needs to be told to flush
		 */
		{
		  if (dataBuffIdxStackIdx >= 2) {
		    src_starti = dataBuffIdxStack[dataBuffIdxStackIdx - 2];
		    src_endi = dataBuffIdxStack[dataBuffIdxStackIdx-1];

		    dataBuffIdxStack[dataBuffIdxStackIdx-2] = src_endi;
		    size = src_endi - src_starti;
/*
printf("COPY (for n-2) src_starti=%d\n", src_starti);
printf("COPY (for n-2) src_endi=%d\n", src_endi);
*/
		  } else {
		    size = 0;
		  }
		  if (size < 0) {
		    cp = (char*)malloc(sizeof(char));
		    *cp = '\0';
		  } else {
		    cp = (char*)malloc(sizeof(char) * (size + 1));
		    strncpy(cp, &dataBuff[src_starti], size);
		    cp[size] = '\0';
		  }
/*printf("COPY (for n-2) str={%s}\n", cp);*/
	
		  parent_parent_bstate = &SBI.stack[SBI.stacki-2];
		  if (parent_parent_bstate) {
			if (GET_label(parent_parent_bstate->obj))
			    free(GET_label(parent_parent_bstate->obj));
			SET_label(parent_parent_bstate->obj, cp);
		  
			/* in case obj needs this information */
			SGMLBuildDoc_span = parent_parent_bstate->sub_y;
			span = getVSpan(parent_parent_bstate->obj, STR_F);
		  }
		}
#ifdef fdlkjsdjsdfl
		/* flush n-1 level object
		 */
		{
		  if (dataBuffIdxStackIdx >= 1) {
		    src_starti = dataBuffIdxStack[dataBuffIdxStackIdx-1];
		    src_endi = dataBuffIdx;

		    dataBuffIdxStack[dataBuffIdxStackIdx] = src_endi;
		    size = src_endi - src_starti;

printf("COPY (for n-1) src_starti=%d\n", src_starti);
printf("COPY (for n-1) src_endi=%d\n", src_endi);

		  } else {
		    size = 0;
		  }
		  if (size < 0) {
		    cp = (char*)malloc(sizeof(char));
		    *cp = '\0';
		  } else {
		    cp = (char*)malloc(sizeof(char) * (size + 1));
		    strncpy(cp, &dataBuff[src_starti], size);
		    cp[size] = '\0';
		  }
printf("COPY (for n-1) str={%s}\n", cp);
		  if (parent_bstate) {
			if (GET_label(parent_bstate->obj))
			    free(GET_label(parent_bstate->obj));
			SET_label(parent_bstate->obj, cp);
		  
			/* in case obj needs this information */
			SGMLBuildDoc_span = parent_bstate->sub_y;
			span = getVSpan(parent_bstate->obj, STR_F);
			hadFlushed = 1;
		  }
		}
#endif
	}
	bstate->dataBuffIdx_localStart = dataBuffIdx;
	dataBuffIdxStack[++dataBuffIdxStackIdx] = dataBuffIdx;

	bstate->parent = parent_bstate->obj;

	bstate->tag = tag;
	bstate->obj = HTMLBuildObj(bstate, parent_bstate->sub_width, 
					bstate->tmi);

	if (bstate->obj && parent_bstate->obj &&
	    bstate->obj != parent_bstate->obj) {
		SET__parent(bstate->obj, parent_bstate->obj);
		SET_parent(bstate->obj, GET_name(parent_bstate->obj));
	}
	if (bstate->tmi->notifyOnCreationP) {
		sendTokenMessage(bstate->obj, STR_C);

#ifdef DOES_NOT_WORK
		if (insert_obj) {
			VObjList *olist, *subObj;
			int _width, span;

printf("HTML2.c INSERT_OBJ is called for STAG!!!\n");

			_insert_obj = insert_obj;
			insert_obj = NULL;

			if (insert_width == -1) {
				_width = bstate->width;
			} else {
				_width = insert_width;
				insert_width = -1;
			}

/*
			SET__parent(bstate->obj, _insert_obj);
			SET_parent(bstate->obj, GET_name(_insert_obj));
*/
			if (bstate->obj) {
				olist = GET__children(bstate->obj);
/*
				parent_bstate->sub_y += bstate->span;
				parent_bstate->height += bstate->span;
*/
				SET__children(bstate->obj,
				      appendObjToList(olist, _insert_obj);
			}
			/* Due to the call-back sequencencing, it's
			 * not clear how to achieve totally transparent
			 * object insertion. So, the bug is that
			 * the inserted object can't expect normal
			 * ESIS data stream...
			 */
			
			span = getVSpan(_insert_obj, STR_C);
			SGMLBuildDoc_span = span;

			if (span == 0) {
				/* destroy object -- it's useless XXX*/
/*				subObj = NULL;*/

			} else if (span != -1) {
/*				*h = span;
				SET_height(subObj, *h);
*/
			}

			bstate->insert = _insert_obj;
		}
#endif
	}

	/*
	 * please excuse the use of __content2 attr 
	 */
	if (bstate->obj) SET__content2(bstate->obj, bstate->tmi);

	bstate->sub_y = 0;
	bstate->height = 0;

	if (bstate->obj && bstate->tmi) 
		SET_x(bstate->obj, bstate->tmi->left);

	if (bstate->tmi) {
		y = parent_bstate->sub_y + bstate->tmi->top;
		width = parent_bstate->sub_width - 
			bstate->tmi->left - bstate->tmi->right;
	} else {
		y = parent_bstate->sub_y;
		width = parent_bstate->sub_width;
	}
	if (bstate->obj) {
		SET_y(bstate->obj, y);
		SET_width(bstate->obj, width);
	}

	if (tagInfo && tagInfo->number_of_attributes && present && value) {
		char **tagAttrNames = tagInfo->attributes;
		for (i = 0; i < tagInfo->number_of_attributes; i++) {
			if (present[i]){
				sendMessage1N2str(bstate->obj, "AA", 
					  tagAttrNames[i], value[i]);
			}
		}
	}

/*
printf("obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(bstate->obj), dataBuffIdxStack[dataBuffIdxStackIdx], 
dataBuffIdx, &dataBuff[dataBuffIdxStack[dataBuffIdxStackIdx]]);
*/

/*	if (parent_bstate->obj && bstate->tmi &&
	    parent_bstate->tmi->flushAlwaysP || bstate->tmi->flushOnSubTagP ||
	    parent_bstate->tmi->inlineP) {
*/

	/* Flush any text in buffer 
	 */
	if (!hadFlushed)
	if (parent_bstate->obj && bstate->tmi && 
	    (parent_bstate->tmi->flushAlwaysP || 
	    (bstate->tmi->flushOnSubTagP && 
	     !bstate->tmi->inlineP))) {

		span = 0;
		src_starti = dataBuffIdxStack[dataBuffIdxStackIdx - 1];
		src_endi = dataBuffIdx;

		dataBuffIdxStack[dataBuffIdxStackIdx - 1] = src_starti;

/*
 * XXX THIS MALLOCing is NOT necessary for HTML, but needed for HMML. 
 * UNIFY THIS DUDE. BUT, MALLOC (AND FREE) PROPPERLY. MUST FIX THIS.?
 */
		size = src_endi - src_starti;
		if (size < 0) {
		  size = 0;
		}
		if (size) {
			cp = (char*)malloc(sizeof(char) * (size + 1));
			strncpy(cp, &dataBuff[src_starti], size);
			cp[size] = '\0';
		} else {
			cp = (char*)malloc(sizeof(char));
			*cp = '\0';
		}

		if (GET_label(parent_bstate->obj))
			free(GET_label(parent_bstate->obj));
		SET_label(parent_bstate->obj, cp);

/*
printf("FLUSH as parent: obj=%s lidx=%d idx=%d DATA {%s}\n", 
  GET_name(parent_bstate->obj), dataBuffIdxStack[dataBuffIdxStackIdx - 1],
  dataBuffIdx, GET_label(parent_bstate->obj));

printf("FLUSH1 as parent: obj=%s height=%d sub_y=%d\n", 
  GET_name(parent_bstate->obj),
  parent_bstate->height, parent_bstate->sub_y);
*/

		if (1/*!hadFlushed*/) {
		  /* in case obj needs this information */
		  SGMLBuildDoc_span = parent_bstate->sub_y;

		  /* Tell parent object to flush out any data (and 
		   * create  objects to process that data, if necessary).
		   * Return any vspan to insert.
		   */
		  span = getVSpan(parent_bstate->obj, STR_F);
		}
/*
printf("FLUSH2 as parent: obj=%s height=%d sub_y=%d span=%d\n", 
  GET_name(parent_bstate->obj),
  parent_bstate->height, parent_bstate->sub_y, span);
*/
		/* Flush action might have caused object insertion, 
		 * and thus affected sub_y
		 */
		SET_y(bstate->obj, 
			parent_bstate->sub_y + span + bstate->tmi->top);

		if (span == 0) {
			/* destroy object -- it's useless */
/*
printf("DESTROYING obj=%s data={%s}\n", 
GET_name(parent_bstate->obj), GET_label(parent_bstate->obj));
*/
			parent_bstate->obj = NULL;
			dataBuffIdx = parent_bstate->dataBuffIdx_localStart;
			/*???*/

		} else if (span != -1) {

/*			span += bstate->tmi->top + bstate->tmi->bottom;*/

			parent_bstate->sub_y += span;
			parent_bstate->height += span;

			SET_height(parent_bstate->obj, parent_bstate->height);
/*
printf("FLUSH3 as parent: obj=%s height=%d sub_y=%d span=%d\n", 
GET_name(parent_bstate->obj),
parent_bstate->height, parent_bstate->sub_y, span);
*/
		}
		/* if obj is a hpane, then postpone its and
		 * its children's geometry till the end of
		 * hpane is reached...
		 */
/*		if (!tmi->hpaneP) 
			... += bstate->tmi->top + bstate->tmi->bottom;
*/

/*
		if (parent_bstate->obj) {
		    fprintf(stderr, 
			"FLUSH exiting %s. obj='%s' span=%d height=%d\n", 
			"stag", GET_name(parent_bstate->obj), 
			span, parent_bstate->height);
		}
*/
	}
	if (parent_bstate->obj && bstate->tmi) {
		if (insert_obj) {
			VObjList *olist, *subObj;
			int _width, span;

/*printf("HTML2.c INSERT_OBJ is called for STAG!!!\n");*/

			_insert_obj = insert_obj;
			insert_obj = NULL;

			if (insert_width == -1) {
				_width = bstate->width;
			} else {
				_width = insert_width;
				insert_width = -1;
			}

			/* can't use parent's sub_y, since the insertion
			 * object is the new reference frame 
			 */
			SET_y(bstate->obj, bstate->tmi->top);

			SET__parent(bstate->obj, _insert_obj);
			SET_parent(bstate->obj, GET_name(_insert_obj));

			olist = GET__children(_insert_obj);
			SET__children(_insert_obj,
			      appendObjToList(olist, bstate->obj));

			if (parent_bstate->obj) {
				olist = GET__children(parent_bstate->obj);
				SET__children(parent_bstate->obj,
				      appendObjToList(olist, _insert_obj));
			}
			/* Due to the call-back sequencencing, it's
			 * not clear how to achieve totally transparent
			 * object insertion. So, the bug is that
			 * the inserted object can't expect normal
			 * ESIS data stream...
			 */
			
			span = getVSpan(_insert_obj, STR_C);
			SGMLBuildDoc_span = span;

			if (span == 0) {
				/* destroy object -- it's useless XXX*/
/*				subObj = NULL;*/

			} else if (span != -1) {
/*				*h = span;
				SET_height(subObj, *h);
*/
			}
			if (SBI.stacki > 0) 
				parent_bstate->insert = _insert_obj;

/*			if (!bstate->tmi->inlineP) {
			  VObjList *olist;

			  olist = GET__children(parent_bstate->obj);
			  SET__children(parent_bstate->obj, 
					appendObjToList(olist, _insert_obj));
			}
*/
		} else {
			VObjList *olist;
			olist = GET__children(parent_bstate->obj);
			SET__children(parent_bstate->obj, 
				appendObjToList(olist, bstate->obj));
		}
	}
}

void CB_HTML_etag(element_number)
	int element_number;
{
	SGMLBuildInfoState *parent_bstate;
	SGMLBuildInfoState *bstate;
	Packet evalResult;
	char *cp;
	int size, src_starti, src_endi;

	nullPacket(&evalResult);

#ifdef PRINT_TAG_DELIMS
	if (printTagDelims) {
		fprintf(stderr, "########################## HTML\t)%s\n",
			HTML_dtd.tags[element_number].name);
	}
#endif

	bstate = &SBI.stack[SBI.stacki--];
	if (SBI.stacki >= 0) parent_bstate = &SBI.stack[SBI.stacki];
	else parent_bstate = NULL;

	if (!bstate->obj) {
		++SBI.stacki;
		fprintf(stderr,
		  "!!!!!!!!!!!! internal error: bstate->obj == NULL.\n");
		return;
	}

	if (bstate->tmi) {
		if (bstate->tmi->inlineP
		     /*&& !GET__children(bstate->obj)*/) {/*HIO*/
			src_starti = dataBuffIdxStack[dataBuffIdxStackIdx];
			src_endi = dataBuffIdx;
			size = src_endi - src_starti;
			if (size < 0) {
				fprintf(stderr, 
					"html parsing error occured.\n");
				size = 0;
			}
			if (size) {
				cp = (char*)malloc(sizeof(char) * (size + 1));
				strncpy(cp, &dataBuff[src_starti], size);
				cp[size] = '\0';
			} else {
				cp = (char*)malloc(sizeof(char));
				*cp = '\0';
			}

			if (GET_label(bstate->obj)) 
				free(GET_label(bstate->obj));

			SET_label(bstate->obj, cp);

			/* rock back, and insert text with hyper directives
			 */
			sendMessage1chr_result(bstate->obj, 'i', &evalResult);

			dataBuffIdx = dataBuffIdxStack[dataBuffIdxStackIdx];

			/* rolll back parent's index as well */
			dataBuffIdxStack[dataBuffIdxStackIdx] = src_starti;

			cp = PkInfo2Str(&evalResult);
			if (cp) 
				for (; *cp; cp++) 
					dataBuff[dataBuffIdx++] = *cp;
			clearPacket(&evalResult);
/*
printf("INLINE1 obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(bstate->obj), dataBuffIdxStack[dataBuffIdxStackIdx], dataBuffIdx,
&dataBuff[dataBuffIdxStack[dataBuffIdxStackIdx]]);
*/
			if (parent_bstate && parent_bstate->obj/*lame check*/){
				VObjList* olist;
				olist = GET__children(parent_bstate->obj);
				removeVObjListNode(&olist, bstate->obj);
				SET__children(parent_bstate->obj, olist);
			}
		} else {
			int span;

/*XXX			src_starti = dataBuffIdxStack[dataBuffIdxStackIdx];
*/
			src_starti = dataBuffIdxStack[dataBuffIdxStackIdx];
			src_endi = dataBuffIdx;
/*
			while (src_starti < src_endi &&
			      isspace(dataBuff[src_starti]))
				src_starti++;
*/
			dataBuffIdxStack[dataBuffIdxStackIdx] = src_starti;

			size = src_endi - src_starti;
			if (size < 0) {
				fprintf(stderr, 
					"html parsing error occured.\n");
				size = 0;
			}
			if (size) {
				cp = (char*)malloc(sizeof(char) * (size + 1));
				if (!cp) {
					perror("malloc failed");
					return;
				}
				strncpy(cp, &dataBuff[src_starti], size);
				cp[size] = '\0';
				if (GET_label(bstate->obj)) 
					free(GET_label(bstate->obj));
				SET_label(bstate->obj, cp);
/*
printf("etag: obj=%s lidx=%d idx=%d DATA size=%d cp={%s} dataBuffIdxStackIdx=%d\n", 
GET_name(bstate->obj), bstate->dataBuffIdx_localStart,
dataBuffIdx, size, cp, dataBuffIdxStackIdx);
*/
			}
/*
printf("etag: obj=%s lidx=%d idx=%d DATA {%s} dataBuffIdxStackIdx=%d\n", 
GET_name(bstate->obj), dataBuffIdxStack[dataBuffIdxStackIdx - 1], 
dataBuffIdx, GET_label(bstate->obj), dataBuffIdxStackIdx);
*/
			/* in case obj needs this information
			 */
			if (parent_bstate)
				SGMLBuildDoc_span = parent_bstate->sub_y;

			if (parent_bstate != bstate->obj)
			  span = getVSpan(bstate->obj, STR_D);
			else 
			  span = 0;
			if (span == 0) {
				/* destroy object -- it's useless */
				/* safer not to use parent_bstate for parent
				 * info, because of insertion, script action..?
				 */
				VObj *parent = GET__parent(bstate->obj);
				if (parent) {
				    VObjList* olist = GET__children(parent);
				    removeVObjListNode(&olist, bstate->obj);
				    sendMessage1(bstate->obj, "freeSelf");
				    SET__children(parent, olist);
				}
				bstate->obj = NULL;

			} else if (span != -1) {

				bstate->height = span;
				SET_height(bstate->obj, span);
	
				span += bstate->tmi->top + bstate->tmi->bottom;

				if (parent_bstate) {
					parent_bstate->sub_y += span;
					parent_bstate->height += span;
					if (parent_bstate->obj)
						SET_height(parent_bstate->obj, 
						        parent_bstate->height);
				}

/*
   fprintf(stderr, 
	"XXXX EXITING etag: parent: obj='%s' bs->h=%d bs->sub_y=%d h=%d\n", 
	parent_bstate->obj ? GET_name(parent_bstate->obj) : "<NULL>",
	parent_bstate->height,
	parent_bstate->sub_y, bstate->height);
*/
			}
/*
	if (bstate->obj) 
	    fprintf(stderr, 
	"EXITING etag: obj='%s' bs->h=%d parent_bs->h=%d parent_bs->sub_y=%d\n", 
	GET_name(bstate->obj), bstate->height, parent_bstate->height,
	parent_bstate->sub_y);
*/
		}
	}
/*
	if (bstate->obj) {
		fprintf(stderr, 
			"exiting %s (end). obj='%s'\n", "tag", 
				GET_name(bstate->obj));
		fprintf(stderr, 
			"Obj=%s: parent=%s x=%d y=%d width=%d  height=%d\n",
			GET_name(bstate->obj), 
			GET__parent(bstate->obj) ? GET_name(GET__parent(bstate->obj)) : "(NULL)",
			GET_x(bstate->obj), GET_y(bstate->obj),
			GET_width(bstate->obj), GET_height(bstate->obj)); 
	}
*/
	if (bstate->tmi)
		if (bstate->tmi->inlineP && 
		    bstate->obj /* && !GET__children(bstate->obj) */) {
			--dataBuffIdxStackIdx;
		} else {
			dataBuffIdx = dataBuffIdxStack[--dataBuffIdxStackIdx];
		}

	if (SBI.stacki > 0 && parent_bstate && parent_bstate->insert) {
/*		fprintf(stderr, "####### Self: insert=%s, parent_h=%d h=%d\n",
			GET_name(parent_bstate->insert), 
			parent_bstate->height,
			bstate->height);
*/
		SGMLBuildDoc_span = bstate->height +
				    bstate->tmi->top + bstate->tmi->bottom;
		SGMLBuildDoc_span = getVSpan(parent_bstate->insert, STR_D);
		parent_bstate->insert = NULL;
	}
/*
	if (bstate->insert) {
		fprintf(stderr,
		  "!!!!!!!!!!!! POPing parent that was inserted--------.\n");
		CB_HTML_etag(-1);
		fprintf(stderr,
		  "!!!!!!!!!!!! finished POPing------------------------.\n");
	}
*/
}

void CB_HTML_end()
{
	SGMLBuildInfoState *bstate = &SBI.stack[SBI.stacki];
	int i, elemNum = 0;/*lame*/
	char *tag = bstate->tag;
	HTTag *htagp;

	SBI.endP = 1;

	/* do binary search, atleast! */
	for (i = 0; i < HTML_ELEMENTS; i++) {
		htagp = &HTML_dtd.tags[i];
		if (htagp && htagp->name && !STRCMP(tag, htagp->name)) {
			CB_HTML_etag(i);
			return;
		}
	}
	CB_HTML_etag(elemNum/*not correct*/);
/*	fprintf(stderr, "@@@@ HTML END\n");
*/
}

VObj *HTMLBuildObj(bstate, parentWidth, tmi)
	SGMLBuildInfoState *bstate;
	int parentWidth;
	SGMLTagMappingInfo *tmi;
{
	Packet argv[1];
	char *dtd;
	extern int global_cloneID;
	int mask;
	int containsAnchor = 1;
/*
	bstate->tmi = &(bstate->tagMappingInfo[
			bstate->localizingTagIDMap[bstate->tagID]]);
	bstate->tmi = findTMI(SBI.dmi->tagMap, bstate->tag);
*/
/*
	fprintf(stderr,
		"HTMLBuildObj: tag '%s' .....................\n", 
		bstate->tmi->tag);
*/
	if (!bstate->tmi->template) {
/*		fprintf(stderr,
			"LOADING object '%s'\n", bstate->tmi->type);
*/
		argv[0].info.s = bstate->tmi->type;
		argv[0].type = PKT_STR;
		argv[0].canFree = 0;

		bstate->tmi->template = PkInfo2Obj(&argv[0]);

		if (!bstate->tmi->template) {
			fprintf(stderr,
				"...can't get template object '%s'\n",
				bstate->tmi->type);
		}
		if (bstate->tmi->template) {
			bstate->tmi->cloneMeth = 
				findMeth(bstate->tmi->template, STR_clone);
			if (!bstate->tmi->template) {
				fprintf(stderr,
	"SGMLBuild_B: can't find clone method for template object '%s'\n",
					bstate->tmi->type);
			}
		}
	}		
	if (!bstate->tmi->template) {
		fprintf(stderr,
			"HTMLBuildObj: Failed to get template object\n");
		return NULL;
	}
	/* clone template object
	 */
	argv[0].info.i = global_cloneID++;
	argv[0].type = PKT_INT;
	argv[0].canFree = 0;

	if (bstate->tmi->inlineP) {
		bstate->obj = bstate->tmi->template;
	} else {
		if (bstate->tmi->cloneMeth)
			bstate->tmi->cloneMeth(bstate->tmi->template, 
						&scrapPk, 1, &argv[0]);
		bstate->obj = scrapPk.info.o;
	}

	if (!bstate->obj) {
		fprintf(stderr,
			"HTMLBuildObj: failed to clone template object '%s'\n",
			GET_name(bstate->tmi->template));
		fprintf(stderr,
			"SGMLBuild_B: STR_clone=%d\n", STR_clone);
		callMeth(bstate->tmi->template, &scrapPk, 1, 
			 &argv[0], STR_clone);
		bstate->obj = scrapPk.info.o;
		return NULL;
	}
/*
	fprintf(stderr,
		"HTMLBuildObj: BUILT: '%s'\n", GET_name(bstate->obj));
*/
	/* modify template object attributes
	 */
	if (bstate->parent) {
		if (bstate->obj == bstate->parent) {
			/* to avoid circular references possibly 
			 * caused by bad markup*/
			fprintf(stderr,
			  "Detected illegal recursive use of tag '%s'.\n",
				bstate->tmi->tag);
		} else {
			SET__parent(bstate->obj, bstate->parent);
			SET_parent(bstate->obj, GET_name(bstate->parent));
		}
	}

	SET_x(bstate->obj, bstate->tmi->left);

	bstate->y += bstate->tmi->top;
	SET_y(bstate->obj, bstate->y);

	bstate->width = parentWidth - bstate->tmi->left - bstate->tmi->right;
	if (!bstate->tmi->inlineP) {
	  SET_width(bstate->obj, bstate->width);
	}

	bstate->sub_width = bstate->width;

	/* not useful now */
/*
	dtd = SBI.dmi->DTDName;
	SET__varList(bstate->obj,
		setVariable_id_STR(GET__varList(bstate->obj), 
				STR_dtd, dtd, 0));
*/
	/* ick ick ick etc */
	if (containsAnchor)
		mask = GET__eventMask(bstate->obj) | PointerMotionMask;
	else
		mask = GET__eventMask(bstate->obj) &
			~(EnterWindowMask | LeaveWindowMask);

	SET__eventMask(bstate->obj, mask);

	return bstate->obj;
}

char *encodeURL(str)
	char *str;
{
	char *s, *buffp = URLEncodeBuff, *encode;

	while (encode = URLEncodeSeq[*str++])
		do { *buffp++ = *encode++; } while (*encode);
	*buffp++ = '\0';

	s = (char*)malloc(sizeof(char) * (buffp - URLEncodeBuff));
	strcpy(s, URLEncodeBuff);

	return s;
}

char *decodeURL(str)
	char *str;
{
	char *s = str, *buffp = URLEncodeBuff, dd[3];

	while (*s) {
		if (*s == '%') {
			s++;
			dd[0] = *s++;
			dd[1] = *s++;
			dd[2] = '\0';
			*buffp++ = (char)strtol(dd, NULL, 16);
		} else {
			*buffp++ = *s++;
		}
	}
	*buffp++ = '\0';

	s = (char*)malloc(sizeof(char) * (buffp - URLEncodeBuff));
	strcpy(s, URLEncodeBuff);

	return s;
}

int http_progress_notify(n)
	int n;
{
	char notice[256];
	extern int perishableActiveHelp;

	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) {
		perishableActiveHelp++;

/*		printf("PROGRES: level=%d   n=%d  subtotal=%d\n",
		       http_progress_reporter_level,
			n, http_progress_subtotal_bytes);
*/
		http_progress_subtotal_bytes += n;
		if (http_progress_expected_total_bytes &&
		    http_progress_reporter_level <= 1) {
			http_progress_total_bytes += n;
/*
			sprintf(notice, "Retrieved %d percent (%d out of %d).",
				(int)((float)http_progress_total_bytes / 
				(float)http_progress_expected_total_bytes *
				100.0),
				http_progress_total_bytes,
				http_progress_expected_total_bytes);
*/
/*
			printf("\t\t\tRetrieved %d percent (%d out of %d).\n",
				(int)((float)http_progress_total_bytes / 
				(float)http_progress_expected_total_bytes *
				100.0),
				http_progress_total_bytes,
				http_progress_expected_total_bytes);
*/
			sendMessage1N2int(mesgObj, "show_progress",
				(int)((float)http_progress_total_bytes / 
				(float)http_progress_expected_total_bytes *
				100.0),
				http_progress_total_bytes);
		} else {
			sprintf(notice, "Secondary retrieval: %d bytes.",
				http_progress_subtotal_bytes);
/*			printf("Secondary retrieval: %d bytes.\n",
				http_progress_subtotal_bytes);
*/
			sendMessage1N1str(mesgObj, "show", notice);
		}
		perishableActiveHelp--;
	}
}

void user_message(message)
	char *message;
{
	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) sendMessage1N1str(mesgObj, "show", message);
}

void user_alert(message)
	char *message;
{
	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) sendMessage1N1str(mesgObj, "alert", message);
}

int user_message_confirm(message)
	char *message;
{
	Packet *result = makePacket();
	int i = 0;

	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) {
		sendMessage1N1str_result(mesgObj, 
					"dialog_confirm",
					message, result);
#ifndef VWMOTIF
		modalLoop();
		i = PkInfo2Int(modalResult);
		clearPacket(modalResult);
#else
		i = PkInfo2Int(result);
#endif
	}
	clearPacket(result);
	free(result);
	return i;
}

char *user_prompt_default(message, deflt)
	char *message;
	char *deflt;
{
	Packet *result = makePacket();
	char *s = NULL;

	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) {
		sendMessage1N2str_result(mesgObj,
					"dialog_prompt_default", 
					message, deflt, result);
#ifndef VWMOTIF
		modalLoop();
		s = saveString(PkInfo2Str(modalResult));
		clearPacket(modalResult);
#else
		s = saveString(PkInfo2Str(result));
#endif
	}
	clearPacket(result);
	free(result);
	return s;
}

char *user_prompt_password(message)
	char *message;
{
	Packet *result = makePacket();
	char *s = NULL;

	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) {
		sendMessage1N1str_result(mesgObj, 
					"dialog_prompt_password", 
					message, result);
#ifndef VWMOTIF
		modalLoop();
		s = saveString(PkInfo2Str(modalResult));
		clearPacket(modalResult);
#else
		s = saveString(PkInfo2Str(result));
#endif
	}
	clearPacket(result);
	free(result);
	return s;
}

void user_prompt_username_and_password(message, username, password)
	char *message;
	char **username;
	char **password;
{
	Packet *result = makePacket();
	char *ret, *s, buff[200];
	int i;

	*username = NULL;
	*password = NULL;

	if (!mesgObj) mesgObj = findObject(getIdent("www.mesg.tf"));
	if (mesgObj) {
		sendMessage1N1str_result(mesgObj, 
					"dialog_prompt_username_and_password",
					message, result);
#ifndef VWMOTIF
		modalLoop();
		ret = PkInfo2Str(modalResult);
#else
		ret = PkInfo2Str(result);
#endif
		s = strchr(ret, '\n');
		i = s - ret;
		strncpy(buff, ret, i);
		buff[i] = '\0';
		trimEdgeSpaces(buff);
		*username = saveString(buff);

		*password = saveString(s);
		trimEdgeSpaces(*password);
#ifndef VWMOTIF
		clearPacket(modalResult);
#endif
	}
	clearPacket(result);
	free(result);
}

void nullifyCallerDataBuff()
{
	dataBuff[SBI.stack[SBI.stacki+1].dataBuffIdx_localStart] = '\0';
}

Bool initHotList()
{
	char *url, *desc, *date, *cp;
	  
	cp = vl_expandPath("~/.mosaic-hotlist-default", buff);
	HotList_path = saveString(cp);
	HotList_fp = fopen(HotList_path, "r");
	if (HotList_fp) {
		cp = HotList_nextLine(); /* format id */
		cp = HotList_nextLine(); /* ? */

		for (;;) {
		    url = HotList_nextLine();/* URL */
		    if (*url == '\0') break;
		    date = "";
		    for (cp = url; *cp; cp++) {
		      if (*cp == ' ') {
			date = cp+1;
			*cp = '\0';
			break;
		      }
		    }
		    url = saveString(url);
		    date = saveString(trimEdgeSpaces(date));
		    desc = saveString(HotList_nextLine());/* Desc */
		    if (*desc == '\0') break;
		    addHotListItem(url, desc, date);
		}
		fclose(HotList_fp);
	}
	HotListLoadedP = 1;

	return 1;
}

Bool addHotListItem(url, comment, date)
	char *url, *comment, *date;
{
	HotListItem *new, *l_hip, *hip;

	new = (HotListItem*)malloc(sizeof(struct HotListItem));
	if (!new) return 0;
	new->url = url;
	new->comment = comment;
	new->date = trimEdgeSpaces(date);
	new->next = NULL;
	theHotListCount++;

	if (!theHotList) {
		theHotList = new;
		return 1;
	} else {
		for (hip = theHotList; hip; hip = hip->next) {
			if (!hip->next) {
				hip->next = new;
				return 1;
			}
		}
		return 0;
	}
}

HotListItem *getNthHotListItem(n)
	int n;
{
	HotListItem *hip;
	int i = 0;

	for (hip = theHotList; hip; hip = hip->next) {
		if (i++ == n) {
			return hip;
		}
	}
	return NULL;
}

Bool deleteHotListItem(n)
	int n;
{
	HotListItem *hip, *l_hip = NULL;
	int i = 0;

	for (hip = theHotList; hip; hip = hip->next) {
		if (i++ == n) {
			if (l_hip) l_hip->next = hip->next;
			else theHotList = hip->next;
			/* free list item */
			theHotListCount--;
			return 1;
		}
		l_hip = hip;
	}
	return 0;
}

char *dumpHotList(mode)
	int mode;
{
	HotListItem *hip;
	int size = 1;/*for last char*/
	char *list, *cp;

	if (mode == 0) {
		for (hip = theHotList; hip; hip = hip->next)
			size += strlen(hip->url) + 1;
		list = cp = (char*)malloc(sizeof(char)*size);
		for (hip = theHotList; hip; hip = hip->next) {
			size = strlen(hip->url);
			strncpy(cp, hip->url, size);
			cp[size] = '\n';
			cp += size+1;
		}
		*cp = '\0';
		return list;
	} else {
		for (hip = theHotList; hip; hip = hip->next)
			size += strlen(hip->comment) + 1;
		list = cp = (char*)malloc(sizeof(char)*size);
		for (hip = theHotList; hip; hip = hip->next) {
			size = strlen(hip->comment);
			strncpy(cp, hip->comment, size);
			cp[size] = '\n';
			cp += size+1;
		}
		*cp = '\0';
		return list;
	}
/*			printf("url     : %s\n", hip->url);
			printf("date    : %s\n", hip->date);
			printf("comment : %s\n\n", hip->comment);
*/
	return NULL;
}

int saveHotList()
{
	HotListItem *hip;

	HotList_fp = fopen(HotList_path, "w+");

	if (!HotList_fp) {
		return 0;
	}
	fprintf(HotList_fp, "ncsa-xmosaic-hotlist-format-1\nDefault\n");/*ick*/
	for (hip = theHotList; hip; hip = hip->next) {
		fprintf(HotList_fp, "%s %s\n", hip->url, hip->date);
		fprintf(HotList_fp,"%s\n", hip->comment);
	}
	fclose(HotList_fp);
}

char *HotList_nextLine()
{
  int end = 0;
  char c;

  linei = 0;
  for (;;) {
    if (hbuffi >= hbuffLimit) {
      hbuffi = 0;
      hbuffLimit = fread(hbuff, sizeof(char), HBUFF_SIZE - 2, HotList_fp);
      hbuff[hbuffLimit] = '\0';
      if (hbuffLimit == 0) goto done;
    }
    c = hbuff[hbuffi++];
    if (c == '\n') {
      goto done;
    } else {
      line[linei++] = c;
    }
  }
 done:
  line[linei++] = '\0';
  return line;
}
