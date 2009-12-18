/*
 * Copyright 1993 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * sgml.c
 * 
 * Various SGML related procedures, and interface to sgmls.
 */
#include "utils.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "vlist.h"
#include "attr.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "glib.h"
#include "slib.h"
#include "ast.h"
#include "cgen.h"
#include "cexec.h"
#include "misc.h"
#include "sgml.h"
#include "stgcall.h"

#include "HTML_style.h"


/*#define malloc(s) printf("malloc(%d)\t%s:%d\n",s,__FILE__,__LINE__);malloc(s)*/
/*#define free(s) printf("free(%d)\t%s:%d\n",s,__FILE__,__LINE__);free(s)*/


char *sgmlPath = NULL;

/*
	version info in ASCII\n
	0(4b)
	byteOrderHint(4b)
	versionID(4b)
	DTDNameSize(4b)	DTDName			(added in version 2)
	TAGDICT(b)	tagCount(4b)
	[		tagID(4b)	size(4b) 	tagName
	]...
	[
			TAG(b)	tagID(4b)
			[ATTR(b) type(b) size(4b) attrName size(4b) attrValue
			 ...]
			DATA(b)	size(4b)	data
			END
	]...

*/
#define sgml_verbose verbose

#define TOKEN_TAGDICT 	1
#define TOKEN_TAG 	2
#define TOKEN_END 	3
#define TOKEN_ATTR 	4
#define TOKEN_DATA 	5

char *sgmlAttrTypesStr[] = {
	"IMPLIED",
	"CDATA",
	"TOKEN",
	"ID",
	"IDREF",
	"ENTITY",
	"NOTATION"
};

SGMLDocMappingInfo *loadStyleSheet();

/* XXX arbitrary limit (+1 for end-marker) */
#define MAX_SGML_FORMS 20
SGMLDocMappingInfo SGMLForms[MAX_SGML_FORMS + 1];

#define ATTRSTACK_SIZE 30 /* limit on #of attributes an element can have */
char *elemAttrStack_name[ATTRSTACK_SIZE];
char *elemAttrStack_val[ATTRSTACK_SIZE];
int elemAttrStackCount = 0;

#define TAGBUFFSIZE 1024
/*char *tagDict[TAGBUFFSIZE];*/
int localizingTagIDMap[TAGBUFFSIZE];

int   tagDictCount = 0;

#define LINEBUFF_SIZE 500
char lineBuff[LINEBUFF_SIZE];

char dataBuff[DATABUFF_SIZE];
int dataBuffIdx = 0;
int dataBuffIdxStack[100];	/* used to roll back to the local idx place */
int dataBuffIdxStackIdx = -1;

int SGMLBuildDoc_span = 0;

#define ATTR_STACK_SIZE 512
char *attrsStack[ATTR_STACK_SIZE];
int attrsStackIdx = 0;

char lbuff[512];

VObj *SGMLBuild();
VObj *SGMLBuild_B();

VObj *insert_obj = NULL;
int insert_width = -1;

Packet scrapPk;

int SGMLInit()
{
	int i;

	for (i = MAX_SGML_FORMS - 1; i; i--) {
		SGMLForms[i].DTDName = NULL;
		SGMLForms[i].tagMap = NULL;
	}

	stgcall_init();

	nullPacket(&scrapPk);
	return 1;
}

void SGMLBuildDoc_insertObj(obj, width)
	VObj *obj;
	int width;
{
	insert_obj = obj;
	insert_width = width;
}

int SGMLBuildDoc_setBuff(obj, offset)
	VObj *obj;
	int offset;
{
	char *cp;
	int size, src_starti, src_endi;

	/* skip whitespace ... beware that this is regardLESS of the 
	 * type of data in the buffer!
	 */
	src_starti = dataBuffIdxStack[dataBuffIdxStackIdx + offset];
	src_endi = dataBuffIdx;
/*	while (src_starti < src_endi && isspace(dataBuff[src_starti]))
		src_starti++;
*/
	dataBuff[src_endi] = '\0';
	size = src_endi - src_starti;
	if (size < 0) size = 0;

	cp = (char*)malloc(sizeof(char) * (size + 1));
	if (size) strncpy(cp, &dataBuff[src_starti], size);
	cp[size] = '\0';

	if (GET_label(obj)) free(GET_label(obj));
	SET_label(obj, cp);

	dataBuffIdxStack[dataBuffIdxStackIdx + offset] = dataBuffIdx;

	return size;
}


/***BINARY VERSION
 ****************************************************/

#define INPUT_BUFFSIZE 100000
char 	INPUT_buff[INPUT_BUFFSIZE + 1];
int 	INPUT_idxLow = 0;
int 	INPUT_idxHigh = 0;
FILE	*INPUT_fp;

#define INPUTC() ((INPUT_idxLow < INPUT_idxHigh) ? INPUT_buff[INPUT_idxLow++] : _INPUTC())

#define SKIPC() (void)((INPUT_idxLow < INPUT_idxHigh) ? INPUT_idxLow++ : _INPUTC())

char _INPUTC()
{
	INPUT_idxHigh = fread(INPUT_buff, 1, INPUT_BUFFSIZE, INPUT_fp);
	INPUT_buff[INPUT_idxHigh] = '\0';
	INPUT_idxLow = 0;
	if (INPUT_idxHigh != INPUT_BUFFSIZE)
		INPUT_buff[INPUT_idxHigh++] = '\0';
	return INPUT_buff[INPUT_idxLow++];
}

/* BUG: INPUT_BUFFSIZE must be larger than (largest-possible-data-size / 2).
 */
char *INPUTS(n, ret)
	int n;
	char *ret;
{
	if (INPUT_idxLow + n < INPUT_idxHigh) {
		strncpy(ret, INPUT_buff + INPUT_idxLow, n);
/*
		ret[n] = '\0';
		printf("%d: INPUTS1 n=%d  >>%s<<\n", INPUT_idxLow, n, ret);
*/
		INPUT_idxLow += n;
		return ret;
	} else {
		int span = INPUT_idxHigh - INPUT_idxLow;

		strncpy(ret, INPUT_buff + INPUT_idxLow, span);
		INPUT_idxHigh = fread(INPUT_buff, 1, INPUT_BUFFSIZE, INPUT_fp);
		if (INPUT_idxHigh != INPUT_BUFFSIZE) {
			INPUT_buff[INPUT_idxHigh] = '\0';
			strncpy(ret + span, INPUT_buff, n - span);/*XX*/

/*			ret[n] = '\0';
			printf("%d: INPUTS2 n=%d  >>%s<<\n", 
				INPUT_idxLow, n, ret);
*/
			INPUT_idxLow = n - span;
			return ret;
		}
		strncpy(ret + span, INPUT_buff, n - span);
/*
		ret[n] = '\0';
		printf("%d: INPUTS3 n=%d  >>%s<<\n", INPUT_idxLow, n, ret);
*/
		INPUT_idxLow = n - span;
		return ret;
	}
}

int INPUT_LINE_filtered(buff)
	char *buff;
{
	char c, *cp = buff;

	while (c = INPUTC()) {
		if (c == '\n') {
			break;
		} else if (c != '\\') {
			*cp++ = c;
		} else {
			switch (c = INPUTC()) {
			case 'n': 
				*cp++ = '\n'; 	/* newline */
				continue;
			case 't':
				*cp++ = '\t'; 	/* horizontal tab */
				continue;
			case 'b': 
				*cp++ = '\b'; 	/* backspace */
				continue;
			case 'r': 
				*cp++ = '\r';	/* carriage */
				continue;
			case 'f': 
				*cp++ = '\f'; 	/* form feed */
				continue;
			case '\\': 
				*cp++ = '\\'; 	/* backslash */
				continue;
			case '\'': 
				*cp++ = '\''; 	/* single quote */
				continue;
			case '\"': 
				*cp++ = '\"';
				continue;
			default:
				if (isdigit(c)) {
					/* ie: "\011" ->
					 * ' ','\t',' ',' '
					 */
					int i;
					i = (int)(c - '0') * 64;
					i += (int)(INPUTC() - '0') * 8;
					i += (int)(INPUTC() - '0');

					*cp++ = (char)i;
				} else {
					*cp++ = '\\';
					*cp++ = c;
				}
				continue;
			}
		}
	}
	return cp - buff;
}

int INPUT_LINE (buff)
	char *buff;
{
	char c, *cp = buff;
	while ((c = INPUTC()) != '\n') *cp++ = c;
	return cp - buff;
}

#define skipByte() SKIPC()
#define readByte() ((int)(INPUTC()) & 0xff)

#define READ_INT(size) \
	size = (unsigned int)(INPUTC() & 0xff) << 24; \
	size |= (unsigned int)(INPUTC() & 0xff) << 16; \
	size |= (unsigned int)(INPUTC() & 0xff) << 8; \
	size |= (unsigned int)(INPUTC() & 0xff); \
	if (size > DATABUFF_SIZE) \
		fprintf(stderr, \
"Alert: sgml.c: DATABUFF_SIZE isn't big enough, and memory corruption will occur.\n");


int readInt()
{
	int i;

	i = (unsigned int)(INPUTC() & 0xff) << 24;
	i |= (unsigned int)(INPUTC() & 0xff) << 16;
	i |= (unsigned int)(INPUTC() & 0xff) << 8;
	i |= (unsigned int)(INPUTC() & 0xff);
/*printf("READINT: %d\n", (unsigned int)i);*/
	return i;
}

SGMLTagMappingInfo *findTagMappingInfo(tmi, tagName)
	SGMLTagMappingInfo tmi[];
	char *tagName;
{
	int i;
	for (i = 0; tmi[i].tag; i++) {
		if (!STRCMP(tmi[i].tag, tagName)) return &tmi[i];
	}
	return NULL;
}

/*
char *findTagName(tagID)
	int tagID;
{
	int i;

	if (tagID > tagDictCount) return NULL;
	return tagDict[tagID];
}
*/

/* ESIS VERSION 
 */
VObj *SGMLBuildDoc(srcp, url, parent, name, width, anchor)
	char *srcp;
	char *url;
	VObj *parent;
	char *name;
	int width;
	char *anchor;
{
	VObj *docObj;
	char *cp, *ccp;
	SGMLDocMappingInfo *docMappingInfo = NULL;
	SGMLTagMappingInfo *tmi, *tagMappingInfo = NULL;
	SGMLDocMappingInfo *dmi = NULL;
	int h = 0;
	int y = 0;
	int i, j;
	char dtd[32];
	char data[100];
	int tagSize, tagID;
	FILE *fp;
	char c;
	int byteOrderHint;
	int version;
	Packet margv[3];
	Packet argv[1];
	char *itag;
	char tagName[64];
	int size;

	fp = fopen(url, "r");
	if (!fp) {
		fprintf(stderr, 
			"SGMLBuildDoc(): failed to open file '%s'\n", 
			url);
		return NULL;
	}

	INPUT_fp = fp;
 	INPUT_idxLow = 0;
 	INPUT_idxHigh = 0;

	INPUTC();
	size = INPUT_LINE(tagName);
	tagName[size] = '\0';

 	/*XXX assumes first tag is the same name as DTD */
	strcpy(dtd, tagName);

	for (i = 0; SGMLForms[i].DTDName; i++) {
		if (!STRCMP(SGMLForms[i].DTDName, dtd)) {
			dmi = &SGMLForms[i];
			break;
		}
	}
	if (!dmi) {
		dmi = loadStyleSheet(dtd);
		if (!dmi) {
			printf("getSGMLStyle: unknown DTD \"%s\"\n", dtd);
			fclose(fp);
			return NULL;
		}
	}
	tagMappingInfo = dmi->tagMap;

	dataBuffIdx = 0;

	docObj = SGMLBuild(dmi, tagMappingInfo, tagName, localizingTagIDMap,
				parent, width, y, &h, 0);
	if (!docObj) {
		if (sgml_verbose)
			fprintf(stdout, "SGMLBuildDoc_B: docObj = NULL\n");
		fclose(fp);
		return NULL;
	}
/*
	margv[0].type = margv[1].type = margv[2].type = PKT_STR;
	margv[0].info.s = "nameAndURL";
	margv[0].canFree = 0;
	margv[1].info.s = name;
	margv[1].canFree = 0;
	margv[2].info.s = url;
	margv[2].canFree = 0;
	sendMessagePackets(docObj, margv, 3);
*/
	fclose(fp);
	return docObj;
}

void loadTemplateObjectNMethod(tmi)
	SGMLTagMappingInfo *tmi;
{
	Packet argv[1];
/*
	fprintf(stderr,
		"loadTemplateObjectNMethod: loading object '%s'\n",
		tmi->type);
*/
	argv[0].info.s = tmi->type;
	argv[0].type = PKT_STR;
	argv[0].canFree = 0;
	tmi->template = PkInfo2Obj(&argv[0]);
	if (!tmi->template) {
		fprintf(stderr,
		 "loadTemplateObjectNMethod: can't get template object '%s'\n",
			tmi->type);
	}
	if (tmi->template) {
		tmi->cloneMeth = findMeth(tmi->template, STR_clone);
		if (!tmi->template) {
			fprintf(stderr,
				"loadTemplateObjectNMethod: can't find clone method for template object '%s'\n",
				tmi->type);
		}
	}
}

VObj *SGMLBuild(dmi, tagMappingInfo, tagName, localizingTagIDMap, parent, 
		  width, y, h, PinlineP)
	SGMLDocMappingInfo *dmi;
	SGMLTagMappingInfo *tagMappingInfo;
	char *tagName;
	int *localizingTagIDMap;
	VObj *parent;
	int width;
	int y;
	int *h;
	int PinlineP;
{
	int endP = 0;
	char token;
	Packet argv[1];
	int i, j, size;
	SGMLTagMappingInfo *tmi = NULL;
	VObj *obj, *template = NULL, *subObj;
	char c, c0, *s, *anchor, *subTagp, *datap;
	int span, height;
	char *name, *ref;
	char **srcp;
	int dataBuffIdx_localStart = dataBuffIdx;
	char *dtd;
	char *attrName;
	int attrTypeID;
	extern int global_cloneID;

	dataBuffIdxStack[++dataBuffIdxStackIdx] = dataBuffIdx;

	tmi = findTagMappingInfo(tagMappingInfo, tagName);

	if (!tmi) {
		fprintf(stderr,
			"SGMLBuild: Don't know about tag '%s'\n", tagName);
		--dataBuffIdxStackIdx;
		return NULL;
	}

/*	fprintf(stderr,
		"SGMLBuild'ing: tag '%s' .....................\n", tmi->tag);
*/
	template = tmi->template;
	if (!template) {
		loadTemplateObjectNMethod(tmi);
		template = tmi->template;
		if (!template) {
			fprintf(stderr,
			    "SGMLBuild: Failed to get template object\n");
			--dataBuffIdxStackIdx;
			return NULL;
		}
	}
	/* clone template object
	 */
	if (tmi->inlineP) {
		obj = template;
	} else {
		if (tmi->cloneMeth) {
			argv[0].info.i = global_cloneID++;
			argv[0].type = PKT_INT;
			argv[0].canFree = 0;
			tmi->cloneMeth(tmi->template, &scrapPk, 1, &argv[0]);
		}
		obj = scrapPk.info.o;
	}

	if (!obj) {
		argv[0].info.i = global_cloneID++;
		argv[0].type = PKT_INT;
		argv[0].canFree = 0;

		fprintf(stderr,
			"SGMLBuild: failed to clone template object '%s'\n",
			GET_name(template));
		fprintf(stderr,
			"SGMLBuild: STR_clone=%d\n", STR_clone);
		callMeth(template, &scrapPk, 1, &argv[0], STR_clone);
		obj = scrapPk.info.o;
		--dataBuffIdxStackIdx;
		return NULL;
	}

	/* modify template object attributes
	 */
	SET_security(obj, 1); /* non secure */

	SET__parent(obj, parent);
	SET_parent(obj, GET_name(parent));

	SET_x(obj, tmi->left);

	y += tmi->top;
	SET_y(obj, y);

	width = width - tmi->left - tmi->right;
	SET_width(obj, width);

	dtd = dmi->DTDName;
	SET__varList(obj,
		setVariable_id_STR(GET__varList(obj), STR_dtd, dtd, 0));

	/* ick ick ick etc */
	SET__eventMask(obj, GET__eventMask(obj) &
				~(EnterWindowMask | LeaveWindowMask)); 


	/* Note that IMPLIED attribute are not reported.
	 * could be optimized.
	 */
	if (obj && elemAttrStackCount) {
		char *name;
		for (i = 0; i < elemAttrStackCount; i++) {
			name = elemAttrStack_name[i];
			/* should just generalize... */
			if (!STRCMP(name, "NAME")) {
				if (elemAttrStack_val[i]) {
					SET_content2(obj, 
					             elemAttrStack_val[i]);
					elemAttrStack_val[i] = NULL;
				}
			} else if (!STRCMP(name, "HREF")) {
				if (elemAttrStack_val[i]) {
					sendMessage1N1str(obj, "setRef", 
						  elemAttrStack_val[i]);
				}
			} else {
				if (elemAttrStack_val[i]) {
					sendMessage1N2str(obj, "AA", name,
						  elemAttrStack_val[i]);
				} else {
					sendMessage1N1str(obj, "AI", name);
				}
			}
			if (elemAttrStack_name[i])
				free(elemAttrStack_name[i]);
			if (elemAttrStack_val[i])
				free(elemAttrStack_val[i]);
		}
		elemAttrStackCount = 0;
	}

	*h = 0;

	do {
		token = INPUTC();

		switch (token) {
		case '-': {

			size = INPUT_LINE_filtered(&dataBuff[dataBuffIdx]);
			dataBuff[dataBuffIdx + size] = '\0';
/*
printf("DATA size===%d\n", size);
printf("...obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], 
dataBuffIdx, &dataBuff[dataBuffIdx]);
*/
			dataBuffIdx += size;
		}
		break;
		case 'A': { /* Attribute */
			char *cp;
			char *idx;
			int length;

			length = INPUT_LINE(lineBuff);
			lineBuff[length] = '\0';

			idx = index(lineBuff, ' ');
			size = idx - lineBuff;
			if (size < 0) {
				size = 0;
			}
			attrName = (char*)malloc(sizeof(char) * (size + 1));
			strncpy(attrName, lineBuff, size);
			attrName[size] = '\0';
/*
			fprintf(stdout, 
				"Attr: name={%s}\n",
				attrName);
*/
			c = lineBuff[idx - lineBuff + 1];
			if (c == 'I') {
				/* A<ATTRNAME> IMPLIED */

				elemAttrStack_name[elemAttrStackCount] = 
					attrName;
				elemAttrStack_val[elemAttrStackCount] = NULL;
				++elemAttrStackCount;

			} else if (c == 'N') {
			} else {
				/* A<ATTRNAME> <TYPE> <VALUE> */
				idx = index(idx + 1, ' ') + 1;

				size = length - (int)(idx - lineBuff);
				if (size < 0) {
					size = 0;
				}
				cp = (char*)malloc(sizeof(char) * (size + 1));
				strncpy(cp, idx, size);
				cp[size] = '\0';

/*				fprintf(stdout, 
					"Attr NOT IMPLIED: value={%s}\n", cp);
*/
				elemAttrStack_name[elemAttrStackCount] = 
					attrName;
				elemAttrStack_val[elemAttrStackCount] = cp;
				++elemAttrStackCount;
			}
		}
		break;
		case '(': {
			int tagID;
			SGMLTagMappingInfo *ctmi;
			VObj *_insert_obj = insert_obj;
			char subTagName[64];

			size = INPUT_LINE(subTagName);
			subTagName[size] = '\0';

			ctmi = findTagMappingInfo(tagMappingInfo, subTagName);

			if (obj && ctmi)
			  if (tmi->flushAlwaysP || ctmi->flushOnSubTagP) {
				char *cp;

				size = dataBuffIdx - 
					dataBuffIdxStack[dataBuffIdxStackIdx];
				cp = (char*)malloc(sizeof(char) * (size + 1));
				if (size) { 
					strncpy(cp, 
						&dataBuff[dataBuffIdxStack[
							dataBuffIdxStackIdx]],
					size);
				}
				cp[size] = '\0';
				if (GET_label(obj)) free(GET_label(obj));
				SET_label(obj, cp);

/*
printf("obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], 
dataBuffIdx, GET_label(obj));
*/
				/* in case obj needs this information */
				SGMLBuildDoc_span = *h;

				span = getVSpan(obj, STR_F);
				if (span == 0) {
					/* destroy object -- it's useless */
					obj = NULL;
				} else if (span != -1) {
					*h = span;
					SET_height(obj, *h);
				}
				/* if obj is a hpane, then postpone its and
				 * its children's geometry till the end of
				 * hpane is reached...
				 */
				if (!tmi->hpaneP) {
				  *h += tmi->top + tmi->bottom;
				}

				if (obj)
				  if (sgml_verbose)
				    fprintf(stderr, 
				      "exiting %s. obj='%s' span=%d *h=%d\n", 
					"tag", GET_name(obj), span, *h);

				dataBuffIdx = dataBuffIdx_localStart;
			}
			if (obj) {
			  if (insert_obj) {
				VObjList *olist;
				int _width;

				_insert_obj = insert_obj;
				insert_obj = NULL;

				if (insert_width == -1) {
					_width = width;
				} else {
					_width = insert_width;
					insert_width = -1;
				}

				subObj = SGMLBuild(dmi, tagMappingInfo,
					     	subTagName, localizingTagIDMap,
					     	_insert_obj,
						_width, 0, &span, 
						tmi->inlineP);

				olist = GET__children(obj);
				SET__children(obj, 
				      appendObjToList(olist, _insert_obj));

				if (subObj)
				  if (!tmi->inlineP) {
					olist = GET__children(_insert_obj);
					*h += span;
					SET__children(_insert_obj, 
					      appendObjToList(olist, subObj));
				   }

				/* in case this is needed, at _insert_obj's
				 * D time */
				SGMLBuildDoc_span = *h;

				span = getVSpan(_insert_obj, STR_D);
				if (span == 0) {
					/* destroy object -- it's useless XXX*/
					subObj = NULL;
				} else if (span != -1) {
					*h = span;
					SET_height(subObj, *h);
				}

			  } else {
				subObj = SGMLBuild(dmi, tagMappingInfo,
					     	subTagName, localizingTagIDMap,
					     	obj,
						width, *h, &span, 
						tmi->inlineP);
				if (subObj)
				  if (!tmi->inlineP) {
					VObjList *olist = GET__children(obj);

					*h += span;
					SET__children(obj, 
					      appendObjToList(olist, subObj));
				  }
			  }
			}
		} break;
		case ')': {
			INPUT_LINE(lineBuff);

			if (obj) {
			  if (tmi->inlineP) {
				char *cp;
				Packet evalResult;
				nullPacket(&evalResult);

				dataBuff[dataBuffIdx] = '\0';
				if (GET_label(obj)) free(GET_label(obj));
				SET_label(obj, 
				  &dataBuff[dataBuffIdxStack[
					    dataBuffIdxStackIdx]]);

				sendMessage1chr_result(obj, 'i', &evalResult);

/*XXX DANGER*/			SET_label(obj, NULL);

				dataBuffIdx = dataBuffIdx_localStart;
				cp = PkInfo2Str(&evalResult);
				if (cp)
				  for (; *cp; cp++) 
				    dataBuff[dataBuffIdx++] = *cp;
/*
printf("INLINE1 obj=%s lidx=%d idx=%d DATA {%bs}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], dataBuffIdx, cp);
*/
				--dataBuffIdxStackIdx;
				clearPacket(&evalResult);
				return NULL;
			  } else {
				char *cp;

				size = dataBuffIdx - 
					dataBuffIdxStack[dataBuffIdxStackIdx];
				if (size > 0) {
					cp = (char*)malloc(sizeof(char) *
							   (size + 1));
					if (!cp) {
						perror("malloc failed");
						return 0;
					}
					strncpy(cp, 
						&dataBuff[dataBuffIdxStack[
							dataBuffIdxStackIdx]],
						size);
					cp[size] = '\0';
					if (GET_label(obj))
					  free(GET_label(obj));
					SET_label(obj, cp);
				}
/*
				dataBuff[dataBuffIdx] = '\0';
				SET_label(obj, 
			  &dataBuff[dataBuffIdxStack[dataBuffIdxStackIdx]]);
*/
/*
printf("obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], dataBuffIdx, GET_label(obj));
*/
				/* in case obj needs this information */
				SGMLBuildDoc_span = *h;

				span = getVSpan(obj, STR_D);

				if (span == 0) {
					/* destroy object -- it's useless */
/*
printf("DESTROYING obj=%s data={%s}\n", 
GET_name(obj), GET_label(obj));
*/
					obj = NULL;

				} else if (span != -1) {
					*h = span;
					SET_height(obj, *h);
				}
				*h += tmi->top + tmi->bottom;

				if (obj) 
				  if (sgml_verbose)
				    fprintf(stderr, 
				      "exiting %s. obj='%s' span=%d *h=%d\n", 
					"tag", GET_name(obj), span, *h);

				dataBuffIdx = dataBuffIdx_localStart;
				--dataBuffIdxStackIdx;
				return obj;
			  }
			}
		}
		break;
		case '\0':
			endP = 1;
		break;
		case 'N': 
		default:
		break;
		}
	} while (!endP);

	if (!tmi->inlineP) dataBuffIdx = dataBuffIdx_localStart;

	if (sgml_verbose)
		fprintf(stderr, 
			"exiting %s (end). obj='%s'\n", "tag", GET_name(obj));

	--dataBuffIdxStackIdx;
	return obj;
}


/* BINARY FILE VERSION 
 */
VObj *SGMLBuildDoc_B(srcp, url, parent, name, width, anchor)
	char *srcp;
	char *url;
	VObj *parent;
	char *name;
	int width;
	char *anchor;
{
	VObj *docObj;
	char *cp, *ccp;
	char *subTagp, subTag[100];
	SGMLDocMappingInfo *docMappingInfo = NULL;
	SGMLTagMappingInfo *tmi, *tagMappingInfo = NULL;
	SGMLDocMappingInfo *dmi = NULL;
	int h = 0;
	int y = 0;
	int i, j;
	char dtd[32];
	char data[100];
	int tagSize, tagID;
	FILE *fp;
	char c;
	int byteOrderHint;
	int version;
	Packet margv[3];
	Packet argv[1];
	char *itag;

	fp = fopen(url, "r");
	if (!fp) {
		fprintf(stderr, 
			"SGMLBuildDoc(): failed to open file '%s'\n", 
			url);
		return NULL;
	}

	INPUT_fp = fp;
 	INPUT_idxLow = 0;
 	INPUT_idxHigh = 0;

	/* expect:	version info in ASCII\n
	 */
	if (sgml_verbose)
		fprintf(stdout, "Header {");
	while (c = INPUTC()) {
		if (c == '\0') break;
		if (sgml_verbose)
			fprintf(stdout, "%c", c);
	}
	if (sgml_verbose)
		fprintf(stdout, "}\n");

	/* skip rest of zeros */
	skipByte();
	skipByte();
	skipByte();

	/* expect:	byteOrderHint(4b)
	 */
	byteOrderHint = readInt();

	if (sgml_verbose)
		fprintf(stdout, "byteOrderHint = %d\n", byteOrderHint);

	/* expect:	versionID(4b)
	 */
	version = readInt();
	if (sgml_verbose)
		fprintf(stdout, "version = %d\n", version);

	/* expect: 	DTDNameSize(4b)	DTDName
	 */
	if (version >= 2) {
		tagSize = readInt();
		INPUTS(tagSize, dtd);
		dtd[tagSize] = '\0';
		if (sgml_verbose) fprintf(stdout, "DTD = %s", dtd);
		trimEdgeSpaces(dtd);
	} else {
		strcpy(dtd, "HMML");
	}

	for (i = 0; SGMLForms[i].DTDName; i++) {
		if (!STRCMP(SGMLForms[i].DTDName, dtd)) {
			dmi = &SGMLForms[i];
			break;
		}
	}
	if (!dmi) {
		dmi = loadStyleSheet(dtd);
		if (!dmi) {
			printf("getSGMLStyle: unknown DTD \"%s\"\n", dtd);
			fclose(fp);
			return NULL;
		}
	}
	tagMappingInfo = dmi->tagMap;

	/* expect: 	TAGDICT(b)	tagCount(4b) 	
	 */
	readByte();
	tagDictCount = readInt();

	if (sgml_verbose)
		fprintf(stdout, "tagDictCount = %d\n", tagDictCount);

	/* expect:	tagID(4b)	size(4b) 	tagName
	 */
	for (i = 0; i < tagDictCount; i++) {
		int tmiIndex;

		tagID = readInt();
		tagSize = readInt();

		if (sgml_verbose)
			fprintf(stdout, 
				"tagID = %2d, tagSize=%3d  ", tagID, tagSize);

		INPUTS(tagSize, data);
		data[tagSize] = '\0';

		localizingTagIDMap[i] = -1;
		for (tmiIndex = 0; tagMappingInfo[tmiIndex].tag; tmiIndex++) {
			if (!STRCMP(data, tagMappingInfo[tmiIndex].tag)) {
				localizingTagIDMap[i] = tmiIndex;
				break;
			}
		}
		if (localizingTagIDMap[i] == -1) {
			fprintf(stdout, 
				"ERROR: unknown tag <%s>. Using <P>\n", 
				data);
		for (tmiIndex = 0; tagMappingInfo[tmiIndex].tag; tmiIndex++) {
			if (!STRCMP("P", tagMappingInfo[tmiIndex].tag)) {
				localizingTagIDMap[i] = tmiIndex;
				break;
			}
		}
		}
	}

/*
	localizingTagIDMap = (int*)malloc(sizeof(int)*tagDictCount);
	for (i = 0; i < tagDictCount; i++) {
		int tmiIndex;

		fprintf(stdout, "tagData = {%s}\n", tagDict[i]);

		itag = tagDict[i];
		localizingTagIDMap[i] = -1;
		for (tmiIndex = 0; tagMappingInfo[tmiIndex].tag; tmiIndex++) {
			if (!STRCMP(itag, tagMappingInfo[tmiIndex].tag)) {
				localizingTagIDMap[i] = tmiIndex;
				break;
			}
		}
	}
*/

	for (i = 0; i < tagDictCount; i++) {
		if (sgml_verbose) fprintf(stdout, "%d: [%d] --> {%s}\n", i, 
		localizingTagIDMap[i],
		tagMappingInfo[localizingTagIDMap[i]].tag);
	}

	/* make sure the the template object are loaded
	 */
	for (i = 0; i < tagDictCount; i++) {
		tmi = &tagMappingInfo[localizingTagIDMap[i]];
		if (!tmi->tag) break;
		if (!tmi->template) {
		  if (sgml_verbose)
			fprintf(stderr,
				"SGMLBuild_B: preloading object '%s'\n",
				tmi->type);

			argv[0].info.s = tmi->type;
			argv[0].type = PKT_STR;
			argv[0].canFree = 0;
			tmi->template = PkInfo2Obj(&argv[0]);
			if (!tmi->template) {
				fprintf(stderr,
					"SGMLBuild_B: can't get template object '%s'\n",
					tmi->type);
			}

			if (tmi->template) {
				tmi->cloneMeth = 
					findMeth(tmi->template, STR_clone);
				if (!tmi->template) {
					fprintf(stderr,
	"SGMLBuild_B: can't find clone method for template object '%s'\n",
						tmi->type);
				}
			}
		}
	}		


	/* expect:	TAG(b)	tagID(4b)
	 */
	readByte();
	tagID = readInt();

	dataBuffIdx = 0;

	docObj = SGMLBuild_B(dmi, tagMappingInfo, tagID, localizingTagIDMap,
				parent, width, y, &h, 0);
	if (!docObj) {
		if (sgml_verbose)
			fprintf(stdout, "SGMLBuildDoc_B: docObj = NULL\n");
		fclose(fp);
		return NULL;
	}
/*	free(localizingTagIDMap);*/

/*	margv[0].type = margv[1].type = margv[2].type = PKT_STR;
	margv[0].info.s = "nameAndURL";
	margv[0].canFree = 0;
	margv[1].info.s = name;
	margv[1].canFree = 0;
	margv[2].info.s = url;
	margv[2].canFree = 0;
	sendMessagePackets(docObj, margv, 3);
*/
/*	printf("############ done building '%s'\n", GET_name(docObj));
*/

	fclose(fp);
	return docObj;
}


VObj *SGMLBuild_B(dmi, tagMappingInfo, tagID, localizingTagIDMap, parent, 
		  width, y, h, PinlineP)
	SGMLDocMappingInfo *dmi;
	SGMLTagMappingInfo *tagMappingInfo;
	int tagID;
	int *localizingTagIDMap;
	VObj *parent;
	int width;
	int y;
	int *h;
	int PinlineP;
{
	int endP = 0;
	int token, size;
	int i, j;
	SGMLTagMappingInfo *tmi = NULL;
	VObj *obj, *template = NULL, *subObj;
	Packet argv[1];
	char c, c0, *s, *anchor, *subTagp, *datap;
	int span, height;
	char *name, *ref;
	char **srcp;
	int dataBuffIdx_localStart = dataBuffIdx;
	char *dtd;
	char attrName[64];
	int attrTypeID;
	extern int global_cloneID;

	dataBuffIdxStack[++dataBuffIdxStackIdx] = dataBuffIdx;

	tmi = &tagMappingInfo[localizingTagIDMap[tagID]];

	if (!tmi) {
		fprintf(stderr,
			"SGMLBuild_B: Don't know about tag (id=%d)\n", tagID);
		--dataBuffIdxStackIdx;
		return NULL;
	}
/*
  fprintf(stderr,
		"SGMLBuild_B'ing: tag '%s' .....................\n", tmi->tag);
*/
	template = tmi->template;
	if (!template) {
		fprintf(stderr,
			"SGMLBuild_B: Failed to get template object\n");
		--dataBuffIdxStackIdx;
		return NULL;
	}
	/* clone template object
	 */
	if (tmi->inlineP) {
		obj = template;
	} else {
		if (tmi->cloneMeth) {
			argv[0].info.i = global_cloneID++;
			argv[0].type = PKT_INT;
			argv[0].canFree = 0;
			tmi->cloneMeth(tmi->template, &scrapPk, 1, &argv[0]);
		}
		obj = scrapPk.info.o;
	}

	if (!obj) {
		argv[0].info.i = global_cloneID++;
		argv[0].type = PKT_INT;
		argv[0].canFree = 0;

		fprintf(stderr,
			"SGMLBuild_B: failed to clone template object '%s'\n",
			GET_name(template));
		fprintf(stderr,
			"SGMLBuild_B: STR_clone=%d\n", STR_clone);
		callMeth(template, &scrapPk, 1, &argv[0], STR_clone);
		obj = scrapPk.info.o;
		--dataBuffIdxStackIdx;
		return NULL;
	}


	/* modify template object attributes
	 */
	SET__parent(obj, parent);
	SET_parent(obj, GET_name(parent));

	SET_x(obj, tmi->left);

	y += tmi->top;
	SET_y(obj, y);

	width = width - tmi->left - tmi->right;
	SET_width(obj, width);

/*Not yet useful
	dtd = dmi->DTDName;
	SET__varList(obj,
		setVariable_id_STR(GET__varList(obj), STR_dtd, dtd, 0));
*/
	/* ick ick ick etc */
	SET__eventMask(obj, (GET__eventMask(obj) &
				~(EnterWindowMask | LeaveWindowMask))
				| MotionNotify); 

	*h = 0;

	do {
		token = readByte();

		switch (token) {
		case TOKEN_DATA: {

			READ_INT(size);

			INPUTS(size, &dataBuff[dataBuffIdx]);

			dataBuff[dataBuffIdx + size] = '\0';
/*
printf("DATA size===%d\n", size);
printf("...obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], 
dataBuffIdx, &dataBuff[dataBuffIdx]);
*/
			dataBuffIdx += size;
		}
		break;
		case TOKEN_ATTR: {
			char *cp;

			attrTypeID = readByte();

			READ_INT(size);

			if (size > 0) INPUTS(size, attrName);
			attrName[size] = '\0';
/*
			fprintf(stdout, 
				"Attr: type=%s name={%s}\n",
				sgmlAttrTypesStr[attrTypeID], 
				attrName);
*/
			/* attrValue */
			if (obj && attrTypeID) {
				READ_INT(size);
				cp = (char*)malloc(sizeof(char) * (size + 1));
				INPUTS(size, cp);
				cp[size] = '\0';

/*				  fprintf(stdout, 
				"Attr NOT IMPLIED: value={%s}\n", cp);
*/
#ifdef OBSOLETE
			  if (!STRCMP(attrName, "NAME")) {
			    if (obj) SET_content2(obj, cp);
			  } else if (!STRCMP(attrName, "HREF")) {
			    if (obj) sendMessage1N1str(obj, "setRef", cp);
			  }
#endif

				sendMessage1N2str(obj, "AA", attrName, cp);
			} else {
			/**
			 ** Implied attributes are not reported
			 ** in order to save message sending
			 ** overheads.
			 **/
			/*	fprintf(stdout, 
					"Attr IMPLIED\n");
				sendMessage1N1str(obj, "AI", attrName);
			*/
			}
		}
		break;
		case TOKEN_TAG: {
			int tagID;
			SGMLTagMappingInfo *ctmi;
			VObj *_insert_obj = insert_obj;

			READ_INT(tagID);

			ctmi = &tagMappingInfo[localizingTagIDMap[tagID]];

			if (obj && ctmi)
			  if (tmi->flushAlwaysP || ctmi->flushOnSubTagP) {
				char *cp;

				size = dataBuffIdx - 
					dataBuffIdxStack[dataBuffIdxStackIdx];
				if (size < 0) {
				  fprintf(stderr, 
			            "sgml parsing error occured.\n");
				  size = 0;
				}
				if (size) { 
					cp = (char*)malloc(sizeof(char) *
								(size + 1));
					strncpy(cp, 
						&dataBuff[dataBuffIdxStack[
							dataBuffIdxStackIdx]],
						size);
					cp[size] = '\0';
				} else {
					cp = (char*)malloc(sizeof(char));
					*cp = '\0';
				}
				if (GET_label(obj)) free(GET_label(obj));
				SET_label(obj, cp);
/*
printf("obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], 
dataBuffIdx, GET_label(obj));
*/
				/* in case obj needs this information */
				SGMLBuildDoc_span = *h;

				span = getVSpan(obj, STR_F);
				if (span == 0) {
					/* destroy object -- it's useless */
					obj = NULL;
				} else if (span != -1) {
					*h = span;
					SET_height(obj, *h);
				}
				/* if obj is a hpane, then postpone its and
				 * its children's geometry till the end of
				 * hpane is reached...
				 */
				if (!tmi->hpaneP)
					*h += tmi->top + tmi->bottom;

				if (obj)
				  if (sgml_verbose)
				    fprintf(stderr, 
				      "exiting %s. obj='%s' span=%d *h=%d\n", 
					"tag", GET_name(obj), span, *h);

				dataBuffIdx = dataBuffIdx_localStart;
			}
			if (obj) {
			  if (insert_obj) {
				VObjList *olist;
				int _width;

				_insert_obj = insert_obj;
				insert_obj = NULL;

				if (insert_width == -1) {
					_width = width;
				} else {
					_width = insert_width;
					insert_width = -1;
				}

				subObj = SGMLBuild_B(dmi, tagMappingInfo,
					     	tagID, localizingTagIDMap,
					     	_insert_obj,
						_width, 0, &span, 
						tmi->inlineP);

				olist = GET__children(obj);
				SET__children(obj, 
				      appendObjToList(olist, _insert_obj));

				if (subObj)
				  if (!tmi->inlineP) {
					olist = GET__children(_insert_obj);
					*h += span;
					SET__children(_insert_obj, 
					      appendObjToList(olist, subObj));
				   }

				/* in case this is needed, at _insert_obj's
				 * D time */
				SGMLBuildDoc_span = *h;

				span = getVSpan(_insert_obj, STR_D);
				if (span == 0) {
					/* destroy object -- it's useless XXX*/
					subObj = NULL;
				} else if (span != -1) {
					*h = span;
					SET_height(subObj, *h);
				}

			  } else {
				subObj = SGMLBuild_B(dmi, tagMappingInfo,
					     	tagID, localizingTagIDMap,
					     	obj,
						width, *h, &span,
						tmi->inlineP);
				if (subObj)
				  if (!tmi->inlineP) {
					VObjList *olist = GET__children(obj);

					*h += span;
					SET__children(obj, 
					      appendObjToList(olist, subObj));
				  }
			  }
			}
		} break;
		case TOKEN_END:
			if (obj) {
			  if (tmi->inlineP) {
				char *cp;
				Packet evalResult;

				nullPacket(&evalResult);

				dataBuff[dataBuffIdx] = '\0';
				if (GET_label(obj)) free(GET_label(obj));
				SET_label(obj, 
				  &dataBuff[dataBuffIdxStack[
					    dataBuffIdxStackIdx]]);

				sendMessage1chr_result(obj, 'i', &evalResult);

/*XXX DANGER?*/			SET_label(obj, NULL);

				dataBuffIdx = dataBuffIdx_localStart;
				cp = PkInfo2Str(&evalResult);
				if (cp) 
				  for (; *cp; cp++) 
				    dataBuff[dataBuffIdx++] = *cp;
/*
printf("INLINE1 obj=%s lidx=%d idx=%d DATA {%bs}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], dataBuffIdx, cp);
*/
				--dataBuffIdxStackIdx;
				clearPacket(&evalResult);
				return NULL;
			  } else {
				char *cp;

				size = dataBuffIdx - 
					dataBuffIdxStack[dataBuffIdxStackIdx];
				if (size < 0) {
					fprintf(stderr, 
				 		"sgml parsing error occured.\n");
					size = 0;
				}
				if (size) {
					cp = (char*)malloc(sizeof(char) *
							   (size + 1));
					if (!cp) {
						perror("malloc failed");
						return 0;
					}
					strncpy(cp, 
						&dataBuff[dataBuffIdxStack[
							dataBuffIdxStackIdx]],
						size);
					cp[size] = '\0';
					if (GET_label(obj))
						free(GET_label(obj));
					SET_label(obj, cp);
				}
/*
	dataBuff[dataBuffIdx] = '\0';
	SET_label(obj, 
	  &dataBuff[dataBuffIdxStack[dataBuffIdxStackIdx]]);
*/
/*
printf("obj=%s lidx=%d idx=%d DATA {%s}\n", 
GET_name(obj), dataBuffIdxStack[dataBuffIdxStackIdx], dataBuffIdx, GET_label(obj));
*/
				/* in case obj needs this information */
				SGMLBuildDoc_span = *h;

				span = getVSpan(obj, STR_D);

				if (span == 0) {
					/* destroy object -- it's useless */
printf("sgml.c: DESTROYING obj=%s data={%s}\n", 
	GET_name(obj), GET_label(obj));
					if (parent) {
					    SET__children(parent,
						removeVObjListNode(
						     GET__children(parent),
						     obj));
					}
					obj = NULL;

				} else if (span != -1) {
					*h = span;
					SET_height(obj, *h);
				}
				*h += tmi->top + tmi->bottom;

				if (obj) 
				  if (sgml_verbose)
				    fprintf(stderr, 
				      "exiting %s. obj='%s' span=%d *h=%d\n", 
					"tag", GET_name(obj), span, *h);

				dataBuffIdx = dataBuffIdx_localStart;
				--dataBuffIdxStackIdx;
				return obj;
			  }
			}
		break;
		case '\0':
		default:
			endP = 1;
		break;
		}
	} while (!endP);

	if (!tmi->inlineP) dataBuffIdx = dataBuffIdx_localStart;

	if (sgml_verbose)
		fprintf(stderr, 
			"exiting %s (end). obj='%s'\n", "tag", GET_name(obj));

	--dataBuffIdxStackIdx;
	return obj;
}

/*============================*/

int SGMLFindAnchorOffset(obj, anchor)
	VObj *obj;
	char *anchor;
{
	return 0;
}

int getVSpan(self, token)
	VObj *self;
	int token;
{
	/* equivalent script level statement: 	 h = send(self(), message); 
	 */
	Packet evalResult;
	int vspan;

	nullPacket(&evalResult);

	sendTokenMessage_result(self, token, &evalResult);

	vspan = evalResult.info.i;
	clearPacket(&evalResult);

	return vspan;

#ifdef BUGGY
	extern Packet reg1;
	extern long stackBaseIdx;
	extern long stackExecIdx;
	extern Packet *execStack;
	long save_stackBaseIdx = stackBaseIdx;
	long save_stackExecIdx = stackExecIdx;
	int vspan;

/*	clearPacket(&reg1);*/
	if (reg1.canFree & PK_CANFREE_STR) {
		free(reg1.info.s);
		reg1.canFree = 0;
	}
	reg1.info.i = 0;
	reg1.type = 0;

	execStack[++stackExecIdx].info.c = flag;
	execStack[stackExecIdx].type = PKT_CHR;
	execStack[stackExecIdx].canFree = 0;

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	vspan = reg1.info.i;

	if (callObjStack[callObjStackIdx].destroyed) freeArgListForDestroyed();
	else freeArgList(self);

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	freePossibleDangler(&reg1);

	return vspan;
#endif
}


int SGMLTileDoc(self, width, y)
	VObj *self;
	int width;
	int y;
{
	int h = 0;
	int height;
	VObjList *olist;

	int top = 3;
	int bottom = 3;
	int left = 10;
	int right = 10;

	SET_x(self, left);
	SET_y(self, y + top);

	width = GET_width(self) - left - right;
	SET_width(self, width);

	height = GET_height(self);
	if (height < 3) SET_height(self, 999);

	for (olist = GET__children(self); olist; olist = olist->next) {
/*
		int hh;
		hh = SGMLTileDoc(olist->o, width, h);
		printf("    obj %s: vspan=%d\n", GET_name(olist->o), hh);
		h += hh;
*/
		h += SGMLTileDoc(olist->o, width, h);
	}
/*	printf("obj %s: vspan=%d    (children accumulated)\n", 
		GET_name(self), h);
*/

	{
	/* equivalent script level statement: 	 h = send(self, 'D'); 
	 */

	Packet evalResult;
	int vspan;

	nullPacket(&evalResult);

	sendTokenMessage_result(self, STR_D, &evalResult);

	vspan = evalResult.info.i;
	clearPacket(&evalResult);

	h += vspan;

#ifdef BUGGY

	extern Packet reg1;
	extern long stackBaseIdx;
	extern long stackExecIdx;
	extern Packet *execStack;
	long save_stackBaseIdx = stackBaseIdx;
	long save_stackExecIdx = stackExecIdx;
	int vspan;


/*	clearPacket(&reg1);*/
/*	if (reg1.canFree & PK_CANFREE_STR) {
		free(reg1.info.s);
		reg1.canFree = 0;
	}
	reg1.info.i = 0;
	reg1.type = 0;
*/
	execStack[++stackExecIdx].info.c = 'D';
	execStack[stackExecIdx].type = PKT_CHR;
	execStack[stackExecIdx].canFree = 0;

	makeArgList(self, 1);

	stackBaseIdx = stackExecIdx;
	execObjScript(self);

	vspan = reg1.info.i;

	if (callObjStack[callObjStackIdx].destroyed) freeArgListForDestroyed();
	else freeArgList(self);

	stackExecIdx = save_stackExecIdx;
	clearPacket(&execStack[++stackExecIdx]);

	stackExecIdx = save_stackExecIdx;
	stackBaseIdx = save_stackBaseIdx;

	h += vspan;

/*	freePossibleDangler(&reg1);
*/
#endif
	}

/*	printf("obj %s: vspan=%d <<<<<<<\n", GET_name(self), h);*/

	SET_height(self, h);

	h += bottom;

	return h;
}

/* to provide access to style sheet. a sickie implementation...
 */
int getSGMLStyle(dtd, tagName, attrName, result)
	char *dtd;
	char *tagName;
	char *attrName;
	Packet *result;
{
	int i;
	SGMLTagMappingInfo *tmi = NULL;
	SGMLTagMappingInfo *tagMappingInfo = NULL;

	for (i = 0; SGMLForms[i].DTDName; i++) {
		if (!STRCMP(SGMLForms[i].DTDName, dtd)) {
			tagMappingInfo = SGMLForms[i].tagMap;
			break;
		}
	}
	if (!tagMappingInfo) {
		SGMLDocMappingInfo *dmi = loadStyleSheet(dtd);
		if (dmi) tagMappingInfo = dmi->tagMap;
		if (!tagMappingInfo) {
			printf("getSGMLStyle: unknown DTD \"%s\"\n", dtd);
			clearPacket(result);
			return 0;
		}
	}

	for (i = 0; tagMappingInfo[i].tag; i++) {
		if (!STRCMP(tagMappingInfo[i].tag, tagName)) {
			tmi = &tagMappingInfo[i];
			break;
		}
	}
	if (!tmi) {
		fprintf(stderr,
			"getSGMLStyle: Don't know about tag '%s'\n", tagName);
		clearPacket(result);
		return 0;
	}
	if (attrName) {
		if (!STRCMP(attrName, "top")) {
			result->info.i = tmi->top;
			result->type = PKT_INT;
			result->canFree = 0;
		} else if (!STRCMP(attrName, "bottom")) {
			result->info.i = tmi->bottom;
			result->type = PKT_INT;
			result->canFree = 0;
		} else if (!STRCMP(attrName, "left")) {
			result->info.i = tmi->left;
			result->type = PKT_INT;
			result->canFree = 0;
		} else if (!STRCMP(attrName, "right")) {
			result->info.i = tmi->right;
			result->type = PKT_INT;
			result->canFree = 0;
		} else {
			/* etcetra */
			clearPacket(result);
			return 0;
		}
	} else {
		if (!tmi->returnCache) {
			Attr *attrp;
			Packet *packet0 = makePacket(); /* top */
			Packet *packet1 = makePacket(); /* bottom */
			Packet *packet2 = makePacket(); /* left */
			Packet *packet3 = makePacket(); /* right */

			packet0->canFree = 
			packet1->canFree = 
			packet2->canFree = 
			packet3->canFree = PK_CANFREE_NOT;
			packet0->type = 
			packet1->type = 
			packet2->type = 
			packet3->type = PKT_INT;

			packet0->info.i = tmi->top;
			packet1->info.i = tmi->bottom;
			packet2->info.i = tmi->left;
			packet3->info.i = tmi->right;

			tmi->returnCache = attrp = makeAttr(0, packet0);
			attrp->next = makeAttr(1, packet1);
			attrp->next->next = makeAttr(2, packet2);
			attrp->next->next->next = makeAttr(3, packet3);
		}
		result->info.a = tmi->returnCache;
		result->type = PKT_ATR;
		result->canFree = 0;
	}
	return 1;
}

/* to provide dynamic modification to style sheet. Could be faster...
 */
int setSGMLStyle(dtd, tagName, attrName, value)
	char *dtd;
	char *tagName;
	char *attrName;
	Packet *value;
{
	int i;
	SGMLTagMappingInfo *tmi = NULL;
	SGMLTagMappingInfo *tagMappingInfo = NULL;

	for (i = 0; SGMLForms[i].DTDName; i++) {
		if (!STRCMP(SGMLForms[i].DTDName, dtd)) {
			tagMappingInfo = SGMLForms[i].tagMap;
			break;
		}
	}
	if (!tagMappingInfo) {
		printf("setSGMLStyle: unknown DTD \"%s\"\n", dtd);
		return 0;
	}

	for (i = 0; tagMappingInfo[i].tag; i++) {
		if (!STRCMP(tagMappingInfo[i].tag, tagName)) {
			tmi = &tagMappingInfo[i];
			break;
		}
	}
	if (!tmi) {
		fprintf(stderr,
			"getSGMLStyle: Don't know about tag '%s'\n", tagName);
		return 0;
	}
	if (!STRCMP(attrName, "top")) {
		tmi->top = PkInfo2Int(value);
	} else if (!STRCMP(attrName, "bottom")) {
		tmi->bottom = PkInfo2Int(value);
	} else if (!STRCMP(attrName, "left")) {
		tmi->left = PkInfo2Int(value);
	} else if (!STRCMP(attrName, "right")) {
		tmi->right = PkInfo2Int(value);
	} else {
		/* etcetra */
		return 0;
	}
	return 1;
}

SGMLDocMappingInfo *setSGMLStylesheet(dtd, stylesheet)
	char *dtd;
	char *stylesheet;
{
	char *cp, *retStrp, *path;
	char tagName[128], type[128];
	int inlineP, notifyOnCreationP, flushAlwaysP, flushOnSubTagP, hpaneP;
	int top, bttm, left, right, i;
	SGMLTagMappingInfo *tmi, *tagMappingInfo = NULL;
	SGMLDocMappingInfo *dmi = NULL;
	int tmiIdx, tmiIdxLimit;

	if (!stylesheet) return 0;

	/*XXX hardcoding HTML stylesheet, for now, to simplify distribution 
	 */
	if (!STRCMP(dtd, "HTML")) {
		retStrp = HTML_stylesheet_str;/* in HTML_style.c */
	} else {
			cp = SaveString(vl_expandPath(stylesheet, buff));
		if (!cp) return 0;
		if (loadFile(cp, &retStrp) == -1) return 0;
		if (!retStrp) return 0;
	}

	for (i = 0; SGMLForms[i].DTDName; i++) {
		if (!STRCMP(SGMLForms[i].DTDName, dtd)) {
			fprintf(stderr,
				"setSGMLStylesheet: already set for DTD %s\n",
				dtd);
			return 0;
		}
	}
	if (i >= MAX_SGML_FORMS) {
		fprintf(stderr,
		   "setSGMLStylesheet: exceeded MAX_SGML_FORMS. failed.\n");
		return 0;
	}
	dmi = &SGMLForms[i];
	SGMLForms[i+1].DTDName = NULL;
	SGMLForms[i+1].tagMap = NULL;

	/* guess the number of style entries, and allocate space for
	 */
	i = 0;
	tmiIdxLimit = 0;
	for (cp = retStrp; *cp; cp++) {
		buff[i++] = *cp;	/*XXX so unnecessarily necessary */
		if (*cp == '\n') {
			buff[i] = '\0';
			i = 0;
			if (buff[0] != '#' && !isspace(buff[0])) ++tmiIdxLimit;
		}
	}
	tagMappingInfo = dmi->tagMap = (SGMLTagMappingInfo*)malloc(
					sizeof(struct SGMLTagMappingInfo)
						 * (tmiIdxLimit + 1));
	if (!tagMappingInfo) {
		fprintf(stderr, "malloc failed\n");
		return 0;
	}	

	dmi->DTDName = saveString(dtd);

	/* set data */
	i = 0;
	tmiIdx = 0;
	for (cp = retStrp; *cp; cp++) {
		buff[i++] = *cp;
		if (*cp == '\n') {
			buff[i] = '\0';
			i = 0;
			if (buff[0] != '#' && !isspace(buff[0])) {
/*				printf("buff=>>%s<<\n", buff);
*/
				if (tmiIdx >= tmiIdxLimit) {
fprintf(stderr, "Argh! (tmiIdx >= tmiIdxLimit) for dtd=%s, stylesheet=%s\n",
						 dtd, stylesheet);
					return 0;
				}

				top = 0;
				bttm = 0;
				left = 0;
				right = 0;
				inlineP = 0;
				hpaneP = 0;
				flushAlwaysP = 0;
			        notifyOnCreationP = 0;

				sscanf(buff, 
					"%s %s %d %d %d %d %d %d %d %d %d",
					tagName, type,
					&inlineP,
				        &notifyOnCreationP,
				        &flushAlwaysP,
				        &flushOnSubTagP,
					&hpaneP,
					&top, &bttm, &left, &right);
/*
				printf(">> %s %s %d %d %d %d %d %d %d %d %d\n",
					tagName, type,
					inlineP, notifyOnCreationP,
					flushAlwaysP, flushOnSubTagP,
					hpaneP,
					top, bttm, left, right);
*/
/*use this only for overriding
				for (tmiIdx = 0; 
				     tagMappingInfo[tmiIdx].tag; 
				     tmiIdx++) {
					if (!STRCMP(tagName, 
					       tagMappingInfo[tmiIdx].tag)) {
						tmi = &tagMappingInfo[tmiIdx];
						tmi->type = SaveString(type);
						tmi->inlineP = inlineP;
						tmi->hpaneP = hapenP;
						tmi->top = top;
						tmi->bottom = bttm;
						tmi->left = left;
						tmi->right = right;
						goto done;
					}
				}
*/
				tmi = &tagMappingInfo[tmiIdx++];
				tmi->tag = saveString(tagName);
				tmi->type = saveString(type);
				tmi->inlineP = inlineP;
				tmi->notifyOnCreationP = notifyOnCreationP;
				tmi->flushAlwaysP = flushAlwaysP;
				tmi->flushOnSubTagP = flushOnSubTagP;
				tmi->hpaneP = hpaneP;
				tmi->template = NULL;
				tmi->cloneMeth = NULL;
				tmi->top = top;
				tmi->bottom = bttm;
				tmi->left = left;
				tmi->right = right;
				tmi->returnCache = NULL;
			}
		}
	}
	tmi = &tagMappingInfo[tmiIdx];
	tmi->tag = NULL;

	if (STRCMP(dtd, "HTML")) {
		free(retStrp);
	}
  	return dmi;
}

startHTML()
{
/*	dataBuffIdxStack[++dataBuffIdxStackIdx] = dataBuffIdx;*/
}
endHTML()
{
/*	--dataBuffIdxStackIdx;*/
}

SGMLDocMappingInfo *loadStyleSheet(dtd)
	char *dtd;
{
	char *stylesheetFN[128];
	SGMLDocMappingInfo *dmi = NULL;

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
		return NULL;
	}
	return dmi;
}
