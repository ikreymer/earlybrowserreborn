/*
 * sgmlsB2B.c
 *
 * encodeFromBinaryToASCII(encodeFromASCIIToBinary(sgmls(SGML file)))
 */
#include "utils.h"
#include <strings.h>
#include "mystrings.h"

#define ATTR_STACK_SIZE 512
char *attrsStack[ATTR_STACK_SIZE];
int attrsStackIdx = 0;

char lbuff[512];

#define TAGBUFFSIZE 1024
char *tagDict[TAGBUFFSIZE];
int   tagDictCount = 0;

int version = 2;

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

int skipInt(fp)
	FILE *fp;
{
	return fgetc(fp);
}

int readByte(fp)
	FILE *fp;
{
	int i;
	i = fgetc(fp) & 255;
	return i;
}

int readInt(fp)
	FILE *fp;
{
	int i;
	i = fgetc(fp) << 24;
	i |= fgetc(fp) << 16;
	i |= fgetc(fp) << 8;
	i |= fgetc(fp);
	return i;
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

char *findTagName(tagID)
	int tagID;
{
	int i;

	if (tagID > tagDictCount) return NULL;
	return tagDict[tagID];
}

build(fp, tagID)
	FILE *fp;
	int tagID;
{
	char line[100000];
	char attrName[64];
	int attrTypeID;
	int i, endP = 0;
	int token, size;

/*	fprintf(stdout, "Tag: %d\n", tagID);*/
	fprintf(stdout, "Tag: %s\n", findTagName(tagID));

/*	printf("%s", lspaces + strlen(lspaces) - (level * 4));
	printf("######## TAG=\"%s\"\n", tag);
*/
	do {
		token = readByte(fp);

/*	printf("######## TOKEN=%d\n", token);*/

		switch (token) {
		case TOKEN_TAG: {
			int tagID;
			tagID = readInt(fp);
			build(fp, tagID);
		} break;
		case TOKEN_ATTR: {
			attrTypeID = readByte(fp);

			size = readInt(fp);
			for (i = 0; i < size; i++) {
				attrName[i] = fgetc(fp);
			}
			attrName[i] = '\0';

			line[0] = '\0';
			if (attrTypeID > 0) {
				size = readInt(fp);
				for (i = 0; i < size; i++) {
					line[i] = fgetc(fp);
				}
				line[i] = '\0';

				fprintf(stdout, 
					"Attr: type=%s name={%s} value={%s}\n",
					sgmlAttrTypesStr[attrTypeID], 
					attrName,
					line);
			} else {
				fprintf(stdout, 
					"Attr: type=%s name={%s}\n",
					sgmlAttrTypesStr[attrTypeID], 
					attrName);
			}
		}
		break;
		case TOKEN_DATA: {
			size = readInt(fp);
			for (i = 0; i < size; i++) {
				line[i] = fgetc(fp);
			}
			line[i] = '\0';
		
			fprintf(stdout, 
				"Data: size=%d {%s}\n",
				size, line);
		}
		break;
		case TOKEN_END:
			return 1;
		break;
		}
	} while (!endP);

	return 1;
}

main(argc, argv)
     int argc;
     char **argv;
{
	int c;
	char cmd[256];
	char dtd[256];
	int i, j;
	int byteOrderHint;
	int version;
	FILE *fp = stdin;
	char data[1000];
	int tagSize, tagID;

	/* expect:	version info in ASCII\n
	 */
	fprintf(stdout, "Header {");
	while (c = fgetc(fp)) {
		if (c == '\0') break;
		fprintf(stdout, "%c", c);
	}
	fprintf(stdout, "}\n");

	fgetc(fp); /* skip rest of zeros */
	fgetc(fp);
	fgetc(fp);

	/* expect:	byteOrderHint(4b)
	 */
	byteOrderHint = readInt(fp);

	fprintf(stdout, "byteOrderHint = %d\n", byteOrderHint);

	/* expect:	versionID(4b)
	 */
	version = readInt(fp);
	fprintf(stdout, "version = %d\n", version);

	/* expect: 	DTDNameSize(4b)	DTD
	 */
	if (version >= 2) {
		tagSize = readInt(fp);
		for (j = 0; j < tagSize; j++) dtd[j] = fgetc(fp);
		dtd[j] = '\0';
		fprintf(stdout, "DTD = %s\n", dtd);
	}

	/* expect: 	TAGDICT(b)	tagCount(4b) 	
	 */
	readByte(fp);
	tagDictCount = readInt(fp);
	fprintf(stdout, "tagDictCount = %d\n", tagDictCount);

	/* expect:	tagID(4b)	size(4b) 	tagName
	 */
	for (i = 0; i < tagDictCount; i++) {
		tagID = readInt(fp);
		tagSize = readInt(fp);
/*		fprintf(stdout, "tagID = %2d, tagSize=%3d  ", tagID, tagSize);
*/
		for (j = 0; j < tagSize; j++) data[j] = fgetc(fp);
		data[j] = '\0';
		tagDict[i] = saveString(data);
/*		fprintf(stdout, "tagData = {%s}\n", tagDict[i]);
*/
	}

	printf("==== TAG Dict ======\n");
	for (i = 0; i < tagDictCount; i++) {
		printf("tagID=%d, {%s}\n", i, tagDict[i]);
	}

	/* expect:	TAG(b)	tagID(4b)
	 */
	readByte(fp);
	tagID = readInt(fp);
	build(fp, tagID);

	exit(0);
}

