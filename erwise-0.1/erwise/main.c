static char *rcsid = "$Id: main.c,v 1.1 1992/05/18 21:43:03 tvr Exp $";

#include "Includes.h"



void main(argc, argv)
int argc;
char *argv[];
{
    FILE *fp;

    ConfigInit();

    if (fp = OpenErwiserc("r")) {
	ConfigRestore(fp);
	fclose(fp);
    }
    if (UiInitialize(argc, argv, ConfigGetValue, ConfigSetValue) != UI_OK)
	DisplayFatal("Error initializing Ui-toolkit");

    AttachCallbacks();
    BindKeys();
    BindVariables();

    UiMainLoop();
}

#define RCPOSTFIX ".old"
FILE *
 OpenErwiserc(type)
char *type;
{
    char *tmpstr;
    char *bustr;
    FILE *fp;

    if (!getenv("HOME"))
	return (FILE *) NULL;

    tmpstr = strdup(getenv("HOME"));

    tmpstr = (char *) ReAlloc((void *) tmpstr, strlen(tmpstr) +
			      strlen(ERWISERC) + 2);
    strcat(tmpstr, "/");
    strcat(tmpstr, ERWISERC);
    if (!strcmp(type, "w")) {
	bustr = (char *) Malloc((strlen(tmpstr) + strlen(RCPOSTFIX) + 1) *
				sizeof(char));
	strcpy(bustr, tmpstr);
	strcat(bustr, RCPOSTFIX);
	rename(tmpstr, bustr);
	free(bustr);
    }
    fp = fopen(tmpstr, type);
    Free(tmpstr);

    return fp;
}


#ifdef mips
char *
 strdup(s)
char *s;
{
    char *p = (char *) malloc(strlen(s) + 1);

    strcpy(p, s);

    return p;
}

#endif
