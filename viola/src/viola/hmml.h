/* 
 * DO NOT FREELY RELEASE THIS MODULE PUBLIC -- TO BE IN COMMERCIAL VERSION.
 *
 * SGMLS interface, an optimized hmml.v. Hopefully generalized, too.
 */
/*	tag		class		styleInfo
 */
/* TODO: make a style sheet out of this */

typedef struct SGMLTagMappingInfo = {
	char 	*tag;
	char 	*type;
	int 	inline;
	char 	*BDColor;	/* should be in seperate visual style sheet? */
	char 	*BGColor;
	char 	*FGColor;
	char 	*font;
	int 	format;
	int 	verbatim;
	VObj 	*typeObj;	/* place holder for binary 'type' */
	int 	fontID;		/* place holder for binary 'font' */
} SGMLTagMappingInfo;

typedef struct SGMLDocMappingInfo = {
	char *DTDName;
	SGMLTagMappingInfo *tagMap;
} SGMLDocMappingInfo;

int SGMLInit();
VObj *SGMLMakeDocObj();
