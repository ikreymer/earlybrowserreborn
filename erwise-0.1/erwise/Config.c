#include "Includes.h"

#define BUFFER_SIZE 79
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define INDENT_STR  "    "

static char *strdupmax(char *str, int length);
static char *parseid(char *str);
static char *parsevalue(char *str);
static char *readline(FILE * fp);

static int ConfigLines = 0;


static Config_t Fonts[] =
{
    {RESOURCE, C_FONT_NORMAL, (void *) "-adobe-times-medium-r-normal--12-120-75-75-p-64-iso8859-1"},
    {RESOURCE, C_FONT_LIST, (void *) "8x13"},
    {RESOURCE, C_FONT_LISTCOMPACT, (void *) "6x13"},
    {RESOURCE, C_FONT_GLOSSARY, (void *) "-adobe-times-medium-r-normal--12-120-75-75-p-64-iso8859-1"},
    {RESOURCE, C_FONT_GLOSSARYCOMPACT, (void *) "-adobe-times-medium-r-normal--12-120-75-75-p-64-iso8859-1"},
    {RESOURCE, C_FONT_EXAMPLE, (void *) "7x13"},
    {RESOURCE, C_FONT_LISTING, (void *) "7x13"},
    {RESOURCE, C_FONT_ADDRESS, (void *) "8x13"},
    {RESOURCE, C_FONT_HEADER1, (void *) "-adobe-times-medium-r-normal--20-140-100-100-p-96-iso8859-1"},
    {RESOURCE, C_FONT_HEADER2, (void *) "-adobe-times-medium-r-normal--18-180-75-75-p-94-iso8859-1"},
    {RESOURCE, C_FONT_HEADER3, (void *) "-adobe-times-medium-r-normal--17-120-100-100-p-84-iso8859-1"},
    {RESOURCE, C_FONT_HEADER4, (void *) "-adobe-times-medium-r-normal--14-100-100-100-p-74-iso8859-1"},
    {RESOURCE, C_FONT_HEADER5, (void *) "-adobe-times-medium-r-normal--14-100-100-100-p-74-iso8859-1"},
    {RESOURCE, C_FONT_HEADER6, (void *) "-adobe-times-medium-r-normal--14-100-100-100-p-74-iso8859-1"},
    {RESOURCE, C_FONT_HEADER7, (void *) "-adobe-times-medium-r-normal--14-100-100-100-p-74-iso8859-1"},
    {EOC, (char *) NULL, (void *) NULL}
};


static Config_t GlobalSettings[] =
{
    {RESOURCE, C_TOPMARGIN, (void *) "0"},
    {RESOURCE, C_BOTTOMMARGIN, (void *) "0"},
    {RESOURCE, C_LEFTMARGIN, (void *) "0"},
    {RESOURCE, C_RIGHTMARGIN, (void *) "0"},
    {RESOURCE, C_FIXEDWIDTHMODE, (void *) "Off"},
    {RESOURCE, C_FIXEDWIDTH, (void *) "560"},
    {RESOURCE, C_ONEPAGEMODE, (void *) "Off"},
    {RESOURCE, C_WIDTH, (void *) "560"},
    {RESOURCE, C_HEIGHT, (void *) "600"},
    {RESOURCE, C_DOUBLECLICKTIME, (void *) "500"},
    {RESOURCE, C_SEARCH, (void *) "Fixed"},
    {RESOURCE, C_CONTROLPANEL, (void *) "Fixed"},
    {RESOURCE, C_LIST, (void *) "Fixed"},
    {RESOURCE, C_RECALL, (void *) "Fixed"},
    {RESOURCE, C_PAGESETTINGS, (void *) "Fixed"},
    {EOC, (char *) NULL, (void *) NULL}
};


static Config_t Print[] =
{
    {RESOURCE, C_COMMAND, (void *) "lpr"},
    {RESOURCE, C_FILENAME, (void *) "print.out"},
    {RESOURCE, C_PRINTTOFILE, (void *) "False"},
    {RESOURCE, C_TOPMARGIN, (void *) "0"},
    {RESOURCE, C_BOTTOMMARGIN, (void *) "0"},
    {RESOURCE, C_LEFTMARGIN, (void *) "0"},
    {RESOURCE, C_RIGHTMARGIN, (void *) "0"},
    {RESOURCE, C_WIDTH, (void *) "80"},
    {EOC, (char *) NULL, (void *) NULL}
};


static Config_t Defaults[] =
{
    {DYNAMIC_PTR, C_DEFAULTSTABLE, (void *) NULL},
    {EOC, (char *) NULL, (void *) NULL}
};


static Config_t KeyBindings[] =
{
    {RESOURCE, C_GETPAGE, (void *) "Return"},
    {RESOURCE, C_PREVWORD, (void *) "Left"},
    {RESOURCE, C_NEXTWORD, (void *) "Right"},
    {RESOURCE, C_PREVTAG, (void *) "Up"},
    {RESOURCE, C_NEXTTAG, (void *) "Down"},
    {RESOURCE, C_CLOSE, (void *) "^c"},
    {RESOURCE, C_SEARCH, (void *) "^s"},
    {RESOURCE, C_SETTINGS, (void *) "S"},
    {RESOURCE, C_PRINTDOC, (void *) "p"},
    {RESOURCE, C_LIST, (void *) "L"},
    {EOC, (char *) NULL, (void *) NULL}
};


Config_t Config[] =
{
    {COMMENT, C_COMMENT, (void *) "Erwise configuration file"},
    {BLOCK, C_FONTS, (void *) &Fonts},
    {BLOCK, C_KEYBINDINGS, (void *) &KeyBindings},
    {BLOCK, C_PRINT, (void *) &Print},
    {BLOCK, C_GLOBALSETTINGS, (void *) &GlobalSettings},
    {DYNAMIC, C_DEFAULTS, (void *) &Defaults},
    {EOC, (char *) NULL, (void *) NULL}
};


/*
 * misc routines
 */
static char *
 strdupmax(str, length)
char *str;
int length;
{
    char *dup = (char *) NULL;

    if (str) {
	dup = (char *) malloc((length + 1) * sizeof(char));
	strncpy(dup, str, length);
	dup[length] = '\0';
    }
    return dup;
}


static char *
 stripnewline(str)
char *str;
{
    int length;

    if (str) {
	length = strlen(str);
	if (length && (str[length - 1] == '\n'))
	    str[length - 1] = '\0';
    }
    return str;
}


/*
 * config's code starts here
 */
static char *
 readline(fp)
FILE *fp;
{
    char buffer[BUFFER_SIZE + 1];
    char *line = (char *) NULL;
    char *data;
    int length = 1;

    line = (char *) malloc(sizeof(char));
    line[0] = '\0';

    while (!feof(fp) && !strrchr(line, '\n')) {
	if (data = fgets(buffer, BUFFER_SIZE, fp)) {
	    line =
		(char *) realloc(line, (strlen(data) + length) * sizeof(char));
	    strcat(line, data);
	    length = strlen(line) + 1;
	}
    }
#ifdef DEBUG_CONFIG
    printf("read line: '%s'\n", line);
#endif
    ConfigLines++;
    return line;
}


/*
 * returns a copy of id
 */
static char *
 parseid(str)
char *str;
{
    char *ptr, *id = (char *) NULL;
    int length = 0;

    while (str && *str && isspace(*str))
	str++;
    ptr = str;
    while (str && *str && !isspace(str[length]))
	length++;
    id = strdupmax(ptr, length);
#ifdef DEBUG_CONFIG
    printf("id: '%s'\n", id);
#endif
    return id;
}


/*
 * returns a copy of value
 */
static char *
 parsevalue(str)
char *str;
{
    char *ptr;
    char *value = (char *) NULL;

    if (ptr = strchr(str, '=')) {
	ptr++;
	while (ptr && *ptr && isspace(*ptr))
	    ptr++;
	value = stripnewline(strdup(ptr));
    }
    return value;
}


static Config_t *
 getitem(id, table)
char *id;
Config_t *table;
{
    Config_t *walker;

    walker = table;
    while (walker && (walker->type != EOC)) {
	if (!strcmp(id, walker->id))
	    return (walker);
	walker++;
    }
    return ((Config_t *) NULL);
}


ConfigType_t
getidtype(id, table)
char *id;
Config_t *table;
{
    Config_t *walker;

    if (!strncmp(id, "{", 1))
	return (START_OF_BLOCK);
    if (!strncmp(id, "}", 1))
	return (END_OF_BLOCK);
    if (!strncmp(id, "", 1))
	return (EMPTY);
    if (!strncmp(id, "#", 1))
	return (COMMENT);

    if (walker = getitem(id, table))
	return (walker->type);
    return (UNKNOWN);
}


static char *
 parsedynamicobject(line)
char *line;
{
    char *ptr;

    ptr = line;
    while (ptr && *ptr && isspace(*ptr))
	ptr++;
    return (stripnewline(strdup(ptr)));
}


static int restoredynamicblock(fp, table)
FILE *fp;
Config_t *table;
{
    char *line;
    char *id;
    int done = FALSE;
    char **items = (char **) NULL;
    int count = 0;

    do {
	line = readline(fp);
	if (line[0] == '\0')
	    done = TRUE;
	else {
	    id = parseid(line);
	    switch (getidtype(id, table)) {
	    case END_OF_BLOCK:
		done = TRUE;
	    case START_OF_BLOCK:
		break;
	    default:
		if (!count) {
		    count = 1;
		    items = (char **) malloc(2 * sizeof(char *));
		} else {
		    count++;
		    items =
			(char **) realloc(items, (count + 1) * sizeof(char *));
		}
		items[count - 1] = parsedynamicobject(line);
		items[count] = (char *) NULL;
#ifdef DEBUG_CONFIG
		printf("added to count[%d] value '%s'\n",
		       count - 1, items[count - 1]);
#endif
		break;
	    }
	    free(id);
	}
	free(line);
    } while (!done);
    table->value = (void *) items;
    return 0;
}


static void *
 setitemvalue(item, value)
Config_t *item;
void *value;
{
    void *oldvalue = (void *) NULL;

    switch (item->type) {
    case DYNAMIC_PTR:
	oldvalue = item->value;
	item->value = value;
	break;
    case RESOURCE:
	if (item->value)
	    free(item->value);
	item->value = value;
	break;
    default:
	/* error recovery - not needed? */
	fprintf(stderr, "unknown type for given item\n");
    }
    return oldvalue;
}


static int restoreblock(fp, table)
FILE *fp;
Config_t *table;
{
    char *id, *value;
    char *line;
    int done = FALSE;
    int ret;
    Config_t *item;
    int blockcount = 0;
    int unknown = FALSE;
    char *syncline;
    int syncdone;

#ifdef DEBUG_CONFIG
    printf("parsing block\n");
#endif
    while (!feof(fp) && !done) {
	line = readline(fp);
	if (line[0] == '\0') {
	    id = (char *) NULL;
	} else {
	    id = parseid(line);
	    item = getitem(id, table);
	    switch (getidtype(id, table)) {
	    case RESOURCE:
		value = parsevalue(line);
		/* we don't care about this return value... */
		value = setitemvalue(item, (void *) strdup(value));
		unknown = FALSE;
		break;
	    case BLOCK:
#ifdef DEBUG_CONFIG
		printf("block: '%s'\n", id);
#endif
		ret = restoreblock(fp, (Config_t *) item->value);
		unknown = FALSE;
		break;
	    case START_OF_BLOCK:
		if (!unknown)
		    blockcount++;
		else {
#ifdef DEBUG_CONFIG
		    printf("syncing\n");
#endif
		    syncdone = FALSE;
		    while (!syncdone && !feof(fp) && (syncline = readline(fp))) {
			if (strchr(syncline, '}'))
			    syncdone = TRUE;
			free(syncline);
		    }
		}
		unknown = FALSE;
		break;
	    case END_OF_BLOCK:
		blockcount--;
		if (blockcount == 0)
		    done = TRUE;
		else
		    fprintf(stderr, "parse error at line %d\n", ConfigLines);
		unknown = FALSE;
		break;
	    case DYNAMIC:
#ifdef DEBUG_CONFIG
		printf("dynamic: '%s'\n", id);
#endif
		ret = restoredynamicblock(fp, (Config_t *) item->value);
		unknown = FALSE;
		break;
	    case UNKNOWN:
		fprintf(stderr, "Unknown tag type '%s'\n", id);
		unknown = TRUE;
		break;
	    case EMPTY:
	    case COMMENT:
	    default:
		unknown = FALSE;
		break;
	    }
	}
	free(line);
    }
    if (blockcount) {
	if (blockcount < 0)
	    fprintf(stderr, "missing { at %d\n", ConfigLines);
	if (blockcount > 0)
	    fprintf(stderr, "missing } at %d\n", ConfigLines);
    }
#ifdef DEBUG_CONFIG
    printf("end of block\n");
#endif
}


int ConfigRestore(fp)
FILE *fp;
{
    ConfigLines = 0;
    restoreblock(fp, Config);
}


static void indent(fp, depth)
FILE *fp;
int depth;
{
    while (depth-- > 0)
	fprintf(fp, INDENT_STR);
}


int saveblock(fp, table, depth)
FILE *fp;
Config_t *table;
int depth;
{
    Config_t *walker;
    char **dynamic_object;
    int ret;

    walker = table;
    while (walker && (walker->type != EOC)) {
	switch (walker->type) {
	case COMMENT:
	    fprintf(fp, "# %s\n", (char *) walker->value);
	    break;
	case RESOURCE:
	    indent(fp, depth);
	    fprintf(fp, "%s = %s\n", walker->id, (char *) walker->value);
	    break;
	case DYNAMIC_PTR:
	    if (dynamic_object = (char **) walker->value)
		while (*dynamic_object) {
		    indent(fp, depth);
		    fprintf(fp, "%s\n", (char *) *dynamic_object);
		    dynamic_object++;
		}
	    break;
	case BLOCK:
	case DYNAMIC:
	    fprintf(fp, "\n");
	    indent(fp, depth);
	    fprintf(fp, "%s\n", walker->id);
	    indent(fp, depth);
	    fprintf(fp, "{\n");
	    if (walker->type == BLOCK)
		ret = saveblock(fp, (Config_t *) walker->value, depth + 1);
	    else
		ret = saveblock(fp, (Config_t *) walker->value, depth + 1);
	    indent(fp, depth);
	    fprintf(fp, "}\n");
	    break;
	    /*
             * case DYNAMIC: indent(fp, depth - 1); fprintf(fp, "%s\n",
             * walker->id); indent(fp, depth - 1); fprintf(fp, "{\n"); ret =
             * saveblock(fp, (Config_t *)walker->value, depth + 1);
             * indent(fp, depth - 1); fprintf(fp, "}\n"); break;
             */
	default:
	    fprintf(stderr, "unknown type for '%s'\n", walker->id);
	}
	walker++;
    }
}


int ConfigSave(fp)
FILE *fp;
{
    saveblock(fp, Config, 0);
    fflush(fp);
}


static void configinitloop(table)
Config_t *table;
{
    Config_t *walker;

    walker = table;
    while (walker && (walker->type != EOC)) {
	switch (walker->type) {
	case RESOURCE:
	    walker->value = (void *) strdup((char *) walker->value);
#ifdef DEBUG_CONFIG
	    printf("init: '%s': '%s'\n", walker->id, (void *) walker->value);
#endif
	    break;
	case BLOCK:
#ifdef DEBUG_CONFIG
	    printf("init: table = %s\n", walker->id);
#endif
	    configinitloop((Config_t *) walker->value);
	    break;
	}
	walker++;
    }
}


void ConfigInit(void)
{
    configinitloop(Config);
}


void *
 ConfigGetValue(tableptr, id)
void *tableptr;
char *id;
{
    Config_t *table = (Config_t *) tableptr;
    Config_t *item;

    if (!table)
	table = Config;
    if (item = getitem(id, table))
	return (item->value);
    return ((void *) NULL);
}


void *
 ConfigSetValue(tableptr, id, value)
void *tableptr;
char *id;
void *value;
{
    Config_t *table = (Config_t *) tableptr;
    void *ret = (void *) NULL;
    Config_t *item;

    if (!table)
	table = Config;
    if (item = getitem(id, table))
	switch (item->type) {
	case RESOURCE:
	    if (item->value)
		free(item->value);
	    item->value = value;
	    break;
	case DYNAMIC_PTR:
	    ret = item->value;
	    item->value = value;
	    break;
	default:
	    ;
	}
    return ret;
}
