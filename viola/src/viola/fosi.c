#include <ctype.h>
#include "utils.h"
#include "HTFont.h"

#undef HTSTRING_H
#include "../libWWW/Library/Implementation/HTString.h"
#include "../libWWW/Library/Implementation/HTTCP.h"
#include "../libWWW/Library/Implementation/HTAnchor.h"
#include "../libWWW/Library/Implementation/HTParse.h"
#include "../libWWW/Library/Implementation/HTAccess.h"
#include "../libWWW/Library/Implementation/HTHistory.h"
#include "../libWWW/Library/Implementation/HText.h"
#include "../libWWW/Library/Implementation/SGML.h"
#include "../libWWW/Library/Implementation/FOSI.h"
#include "../libWWW/Library/Implementation/FOSIDTD.h"

#undef HTWRITE_H
#include "../libWWW/Library/Implementation/HTWriter.h"
#include "../libWWW/Library/Implementation/HTFile.h"

void CB_FOSI_new()
{
	printf("CB_FOSI_new() called\n");
}

void CB_FOSI_data(str, size)
	char *str;
	int size;
{
	printf("CB_FOSI_data() called\n");
}

void CB_FOSI_stag(element_number, present, value, tagInfo)
	int element_number;
	BOOL *present;
	char **value;
	HTTag *tagInfo;
{
	char *tag = FOSI_dtd.tags[element_number].name;

	printf("CB_FOSI_stag() called\n");
	fprintf(stderr, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FOSI\t(%s\n", tag);

	switch (element_number) {
	case FOSI_E_I_C:

  if (present[FOSI_E_I_C_GI])
    fprintf(stderr, "### E-I-C GI=%s\n", value[FOSI_E_I_C_GI]);

    	break;
	case FOSI_CHARLIST:

    	break;
	case FOSI_FONT:

  if (present[FOSI_FONT_BGCOL])
    fprintf(stderr, "### FONT BGCOL=%s\n", value[FOSI_FONT_BGCOL]);
  if (present[FOSI_FONT_FGCOL])
    fprintf(stderr, "### FONT FGCOL=%s\n", value[FOSI_FONT_FGCOL]);
  if (present[FOSI_FONT_SIZE])
    fprintf(stderr, "### FONT SIZE=%s\n", value[FOSI_FONT_SIZE]);

	break;
	case FOSI_INDENT:

  if (present[FOSI_INDENT_LEFTIND])
    fprintf(stderr, "### INDENT LEFTIND=%s\n", value[FOSI_INDENT_LEFTIND]);
  if (present[FOSI_INDENT_RIGHTIND])
    fprintf(stderr, "### INDENT RIGHTIND=%s\n", value[FOSI_INDENT_RIGHTIND]);
  if (present[FOSI_INDENT_FIRSTLN])
    fprintf(stderr, "### INDENT FIRSTIND=%s\n", value[FOSI_INDENT_FIRSTLN]);

	break;
	default:
	break;
	}

	fprintf(stderr, "### FOSI\t(%s\n",
	FOSI_dtd.tags[element_number].name);
}

void CB_FOSI_etag(element_number)
	int element_number;
{
	printf("CB_FOSI_etag() called\n");
}

void CB_FOSI_end()
{
	printf("CB_FOSI_end() called\n");
}
