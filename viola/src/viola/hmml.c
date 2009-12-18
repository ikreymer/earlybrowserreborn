#include "sgml.h"

SGMLDocMappingInfo SGMLForms = {
	HMMLDocMappingInfo,
	NULL;
};

SGMLDocMappingInfo HMMLDocMappingInfo = {
	"HMML",
	HMMLTagMappingInfo;
};

/***** XXX remember to map this to a ASCII style sheet 
 */
/*	tag,			type,
 *	BDColor,		BGColor,		FGColor
 *	font,						format, verbatim
 *	[typeObjectLink, fontID] 
 */
SGMLTagMappingInfo *HMMLTagMappingInfo[] = {
{
	"HMML",			"doc",
	"LemonChiffon1",	"LemonChiffon1",	"LemonChiffon1",
	"",						0,0,
	0,0
},{
	"SECTION",		"fld",
	"LemonChiffon1",	"LemonChiffon1",	"LemonChiffon1",
	"",						0,0,
	0,0
},{
	"H1",			"txt",
	"LemonChiffon1",	"LemonChiffon1",	"black",
	"bold_largest",					1,0,
	0,0
},{
	"H2",			"txt",
	"LemonChiffon1",	"LemonChiffon1",	"black",
	"bold_large",					1,0,
	0,0
},{
	"H3",			"txt",
	"LemonChiffon1",	"LemonChiffon1",	"black",
	"bold",						1,0,
	0,0
},{
	"P",			"txt",
	"LemonChiffon1",	"LemonChiffon1",	"black",
	"normal",					1,0,
	0,0
},{
	"A",			"txtAnchor",
	"LemonChiffon1",	"LemonChiffon1",	"black",
	"normal",					1,0,
	0,0
},{
	"EXAMPLE",		"txt",
	"black",		"LemonChiffon2",	"black",
	"fixed",					1,1,
	0,0
},{
	"XBM",			"XBM",
	"black",		"LemonChiffon1",	"black",
	"",						0,0,
	0,0
},{
	"XBMF",			"XBMF",
	"black",		"LemonChiffon1",	"black",
	"",						0,0,
	0,0
},{
	NULL
}
};

int SGMLInit()
{

}

VObj *SGMLMakeDocObj(source, parent, name, width)
	char *source;
	VObj *parent;
	char *name;
	int width;
{
	char *srcp;
	char *cp, *ccp;
	char *subTagp, subTag[100];
	SGMLDocMappingInfo *docMappingInfo = NULL;

	if (!source) return NULL;

	strp = source;

	++strp;
	subTagp = strp;
	while (*strp) {
		if (*strp++ == '\n') {
			*strp = '\0';
			++strp;
			break;
		}
	}
	trimEdge(subTagp);

	for (i = 0; SGMLForms[i]; i++) {
		if (!STRCMP(SGMLForms[i]->DTDName, subTagp)) {
			docMappingInfo = SGMLForms[i];
			break;
		}
	}
	if (!docMappingInfo) {
		return NULL;
	}
	docObj = send(docMappingInfo->DTDName, "clone", arg[3]);

	tweak(docObj, concatenate("set(\"name\", \"", arg[3], "\");"));
	docObj = send(arg[3], 'B', 
				arg[2], arg[4], arg[1], arg[3]);

		printf("############ done building ########\n");
		GB_free(0);

		print("hmml docObj=", docObj, ", arg[1]=", arg[1], "\n");

		printf("############ done freeing ########\n");
		return docObj;
	}
}

VObj *SGMLBuild(tag, parent, width)
	char *tag;
	VObj *parent;
	int width;
{
	
}
