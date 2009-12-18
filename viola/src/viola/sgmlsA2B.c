/*
 * sgmlsA2B.c
 *
 * usage: sgmlsA2B DTDname [sdecl] [file]
 *
 *    ie: sgmlsA2B HTML sdecl test.html
 *    ie: sgmlsA2B HMML test.hmml
 *
 * encodeFromASCIIToBinary(sgmls(SGML file))
 */
#include "utils.h"
#include <strings.h>
#include "mystrings.h"

int verbose = 0;

#define ATTR_STACK_SIZE 512
char *attrsStack[ATTR_STACK_SIZE];
int attrsStackIdx = 0;

char lbuff[512];

#define TAGBUFFSIZE 1024
char *tagDict[TAGBUFFSIZE];
int   tagDictCount = 0;

int version = 2;

int outd;

char *lspaces = "                                                        ";

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
		[	NAME(b)	size(4b)	name	]
		[	REF(b)	size(4b)	ref	]
			DATA(b)	size(4b)	data	
		[	GEOM(b) width(4b) height(4b)	]
			END
	]...


NEW VERSION, DUMBER BUT MORE GENERALIZED 

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
/*		[	GEOM(b) width(2b) height(2b)	]*/

#define TOKEN_TAGDICT 	1
#define TOKEN_TAG 	2
#define TOKEN_END 	3
#define TOKEN_ATTR 	4
#define TOKEN_DATA 	5

enum sgmlsAttributeTypes {
	SGMLS_ATTR_IMPLIED,
	SGMLS_ATTR_CDATA,
	SGMLS_ATTR_TOKEN,
	SGMLS_ATTR_ID,
	SGMLS_ATTR_IDREF,
	SGMLS_ATTR_ENTITY,
	SGMLS_ATTR_NOTATION
};

emitToken(i)
	int i;
{
	fprintf(stdout, "%c", (int)(i & 255));
}

emitStr(s, n)
	char *s;
	int n;
{
	int i;
	for (i = 0; i < n; i++) 
		fprintf(stdout, "%c", s[i]);
}

emitInt(i)
	int i;
{
	fprintf(stdout, "%c", (int)((i >> 24) & 255));
	fprintf(stdout, "%c", (int)((i >> 16) & 255));
	fprintf(stdout, "%c", (int)((i >> 8) & 255));
	fprintf(stdout, "%c", (int)(i & 255));
}

char *filterCtrl(inStr, size)
	char *inStr;
	int *size;
{
	char c, *cp, *outStr;

	outStr = cp = (char*)malloc(sizeof(char) * (strlen(inStr) + 1));

	while (c = *inStr++) {
		if (c == '\\') {
			switch (c = *inStr++) {
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
				  i += (int)(*inStr++ - '0') * 8;
				  i += (int)(*inStr++ - '0');

				  *cp = (char)i;

				  ++cp;
				  *cp = ' ';
				  ++cp;
				  *cp = ' ';
				  ++cp;
				} else {
				  *(cp - 1) = '\\';
				  ++cp;
				}
				continue;
			}
		} else {
			*cp++ = c;
		}
	}
	*cp = '\0';

	*size = (int)(cp - outStr);

	return outStr;
}

int findTagID(tagName)
	char *tagName;
{
	int i;

	for (i = 0; i < tagDictCount; i++) {
		if (!STRCMP(tagDict[i], tagName)) return i;
	}
	return -1; /* error! */
}

buildDict(tag, srcp, level)
	char *tag;
	char **srcp;
	int level;
{
	char tagName[100];
	char line[100000];
	char *end;
	int i, endP = 0;

	do {
		end = index(*srcp, '\n');
		if (!end) {
			end = index(*srcp, '\0');
			strncpy(line, *srcp, end - *srcp);
			line[end - *srcp] = '\0';
			*srcp = end;
			endP = 1;

		} else {
			strncpy(line, *srcp, end - *srcp);
			line[end - *srcp] = '\0';
			*srcp = end + 1;
		}

		if (line[0] == '(') {
			strcpy(tagName, line + 1);
/*			printf("OTAG=\"%s\"\n", tagName);*/

			for (i = 0; i < tagDictCount; i++) {
				if (!STRCMP(tagDict[i], tagName)) break;
			}
			if (i >= tagDictCount) {
				tagDict[tagDictCount] = saveString(tagName);
				++tagDictCount;
			}
			buildDict(tagName, srcp, level + 1);
		} else if (line[0] == ')') {
			return 1;
		}
	} while (!endP);

	return 1;
}

build(tag, srcp, level, attrsIdxBegin, attrsCount)
	char *tag;
	char **srcp;
	int level;
	int attrsIdxBegin;
	int attrsCount;
{
	char tagName[100];
	char line[100000];
	char *end;
	int i, j, endP = 0;
	int subAttrsIdxBegin = attrsIdxBegin + attrsCount;
	int subAttrsCount = 0;
	char *name, *ref;
	char c, c0, *s, *anchor, *subTagp, *datap;
	int tagID;
	int width, height;
	char *cp;
	int ai, si, len;
	int attrTypeID;
	char attrName[128];
	char attrType[32];
	char attrValue[500];

/*	printf("%s", lspaces + strlen(lspaces) - (level * 4));*/
/*	fprintf(stderr, "######## TAG=\"%s\"\n", tag);
*/

	tagID = findTagID(tag);
	if (tagID != -1) {
		emitToken(TOKEN_TAG);
		emitInt(tagID);
	}

	name = NULL;
	ref = NULL;
	width = height = 0;

	for (i = attrsIdxBegin, j = i + attrsCount; i < j; i++) {
/*
		fprintf(stderr,
			"  a[%d]={%s}\n", 
			i, 
			attrsStack[i]);
*/
		cp = attrsStack[i];

		ai = 0; si = 1;
		while (c = cp[si]) {
			if (c == ' ') { c = cp[si++]; break;}
			if (c == '\0') break;
			attrName[ai++] = cp[si];
			++si;
		}
		attrName[ai] = '\0';

		attrType[0] = '\0';
		if (c != '\0') {
			ai = 0;
			while (c = cp[si]) {
				if (c == ' ') { c = cp[si++]; break;}
				if (c == '\0') break;
				attrType[ai++] = cp[si];
				++si;
			}
			attrType[ai] = '\0';
		}

		attrValue[0] = '\0';
		if (c != '\0') {
			ai = 0;
			while (c = cp[si]) {
				if (c == '\0') break;
				attrValue[ai++] = cp[si];
				++si;
			}
			attrValue[ai] = '\0';
		}

		if (!STRCMP(attrType, "IMPLIED")) {
			attrTypeID = SGMLS_ATTR_IMPLIED;
		} else if (!STRCMP(attrType, "CDATA")) {
			attrTypeID = SGMLS_ATTR_CDATA;
		} else if (!STRCMP(attrType, "TOKEN")) {
			attrTypeID = SGMLS_ATTR_TOKEN;
		} else if (!STRCMP(attrType, "ID")) {
			attrTypeID = SGMLS_ATTR_ID;
		} else if (!STRCMP(attrType, "IDREF")) {
			attrTypeID = SGMLS_ATTR_IDREF;
		} else if (!STRCMP(attrType, "ENTITY")) {
			attrTypeID = SGMLS_ATTR_ENTITY;
		} else if (!STRCMP(attrType, "NOTATION")) {
			attrTypeID = SGMLS_ATTR_NOTATION;
		} else {
			attrTypeID = -1;
		}
/*		fprintf(stderr,
			">> attr name={%s} type={%s}=%d value={%s}\n",
			attrName, attrType, attrTypeID, attrValue);
*/
		emitToken(TOKEN_ATTR);
		emitToken(attrTypeID);
		len = strlen(attrName);
		emitInt(strlen(attrName));
		emitStr(attrName, len);
		if (attrTypeID > 0) {
			len = strlen(attrValue);
			emitInt(strlen(attrValue));
			emitStr(attrValue, len);
		}
	}

	do {
		end = index(*srcp, '\n');
		if (!end) {
			end = index(*srcp, '\0');
			strncpy(line, *srcp, end - *srcp);
			line[end - *srcp] = '\0';
			*srcp = end;
			endP = 1;

		} else {
			strncpy(line, *srcp, end - *srcp);
			line[end - *srcp] = '\0';
			*srcp = end + 1;
		}

		if (line[0] == '(') {
			strcpy(tagName, line + 1);
			if (verbose)
			  fprintf(stderr,"%s", lspaces + strlen(lspaces) - (level * 4));

			build(tagName, srcp, level + 1, 
				subAttrsIdxBegin, subAttrsCount);

			subAttrsIdxBegin = attrsIdxBegin + attrsCount;
			subAttrsCount = 0;

		} else if (line[0] == ')') {

		  if (verbose) {
			fprintf(stderr, "%s", lspaces + strlen(lspaces) - (level * 4));
			fprintf(stderr, "ETAG=\"%s\"\n", line + 1);
		      }
			emitToken(TOKEN_END);
			return 1;
		} else if (line[0] == '-') {
			char *s;
			int size;

		  if (verbose) {
			fprintf(stderr,"%s", lspaces + strlen(lspaces) - (level * 4));
			fprintf(stderr,"LINE=...\n");
			fprintf(stderr,"LINE=\"%s\"\n", line + 1);
		      }
			s = filterCtrl(line + 1, &size);

			if (verbose) {
			  fprintf(stderr, "DATA %d \n{%s}\n", size, s);
			}
			emitToken(TOKEN_DATA);
			emitInt(size);
			emitStr(s, size);

		} else if (line[0] == 'A' || line[0] == 'I') {
			datap = s = line;
			attrsStack[subAttrsIdxBegin + subAttrsCount] = 
				saveString(datap);
			++subAttrsCount;
			while (*s) {
				if (*s++ == '\n') {
					*(s - 1) = '\0';
					break;
				}
			}
/*
			printf("   suba[%d]={%s}\n",
			subAttrsIdxBegin + subAttrsCount - 1,
			attrsStack[subAttrsIdxBegin + subAttrsCount - 1]);
*/
		} else {
			/* ignore */
/*			printf("%s", lspaces + strlen(lspaces) - (level * 4));
			printf("?=\"%s\"\n", line);
*/
		}
	} while (!endP);

	return 1;
}

main(argc, argv)
     int argc;
     char **argv;
{
	int c;
	FILE *fp;
	char *buffp, buff[200000];
	char cmd[256];
	char dtd[256];
	int i;
	char *sdecl = "";
	char *doc = "";

	strcpy(dtd, argv[1]);
	if (argc == 3) {
		doc = argv[2];
	} else {
		sdecl = argv[2];
		doc = argv[3];
	}
/*	sprintf(cmd, "%s %s %s", "/usr/local/bin/sgmls", sdecl, doc);*/
	sprintf(cmd, "%s %s %s", "sgmls", sdecl, doc);
	fp = popen(cmd, "r");
	if (!fp) {
	  fprintf(stderr, "popen() failed\n");
	  exit(0);
	}

	buffp = buff;
	while ((c = fgetc(fp)) != EOF) {
		*buffp = c;
		buffp++;
	}
	*buffp = '\0';
	pclose(fp);

/*	printf(">>>%s<<<\n", buff);*/

	printf("Binary SGMLS file format, version=%d\n", version);
	emitInt(0);
	emitInt(0x12345678);
	emitInt(version);

	if (version >= 2) {
		emitInt(strlen(dtd));
		printf("%s", dtd);
	}

	buffp = buff;
	buildDict("top", &buffp, 0);

/*	printf("==== TAG Dict ======\n");*/

	emitToken(TOKEN_TAGDICT);
	emitInt(tagDictCount);

	for (i = 0; i < tagDictCount; i++) {
/*		fprintf(stderr, "  %4d  size=%4d  %s\n", 
			i, strlen(tagDict[i]), tagDict[i]);
*/
		emitInt(i);
		emitInt(strlen(tagDict[i]));
		emitStr(tagDict[i], strlen(tagDict[i]));
	}

	buffp = buff;
	build("top", &buffp, 0, 0, 0);
}
