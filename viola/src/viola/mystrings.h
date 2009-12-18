/*
 *     mystrings.h
 */
#define SaveString saveString
#define VSaveString(group, s) saveString(s)

#ifndef BOOL
#define BOOL
#endif

/************************************************************************/
#define LINE_FEED 13

#define MALLOCERR printf("Error: Malloc failed.");
/************************************************************************/

typedef struct strNIntPair {
	int  i;
	char *s;
} strNIntPair;

/*
 * size of string used to hold slot content in buffer object...
 */
#define BUFF_SIZE 64000

extern char buff[];
extern int buffi;

#define NUM_OF_GBUFFS 64
extern int numOfGBuffs;
extern char *GBuff[];
extern int GBuffIdx[];
extern int GBuffSize[];

#define MALLOCSIZE(str) (char*)malloc(sizeof(char) * strlen(str))
#define MAX_LINE_LENGTH 1000
#define MAX_ARGUMENTS 100
/* MAX_LONG must be multiples of 10.b/c some functions use that property...*/
#define MAX_LONG 1000000000

/* strcmp alias:
 * compares the first two characters first, then if both match, call strcmp().
 */
#define STRCMP(a,b) ((*a == *b) ? ((*(a+1) == *(b+1)) ? (strcmp(a,b)) : 1) : 1)
#define PUTSTR(str) fputc(str, stdout)

#define ISSPACE(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')

#define trimBackSpaces(str, strLength)\
  for (buffi = strLength; buffi >= 0 && ISSPACE(str[buffi]); str[buffi--] = '\0')

int cmp_str();
int cmp_int();

char *trimFrontSpaces();

/*
 * cuts of the spaces, if any, at the beginning and the end of a string.
 */
char *trimEdgeSpaces();

void SkipBlanks();

/*
 * case insensitive strcmp
 */
int noCaseCharCmp();
int noCaseStrCmp();

/*
 ** Stores the next word in linep in *wordp. Current postion in line is
 ** pointed to by index i.
 ** spaces around the word is trimed.
 *
 * PreCondition: i <= strlen(linep);
 *               *wordp must be large enough to hold any argument in *linep.
 * PostCondition: *wordp contains an argument string from *linep.
 */
int NextWord();

/*
 ** Skips the next word linep in *wordp. Current postion in line is
 ** pointed to by index i.
 ** will skip over spaces around the word.
 *
 * PreCondition: i <= strlen(linep);
 *               *wordp must be large enough to hold any argument in *linep.
 * PostCondition: *wordp contains an argument string from *linep.
 */
int SkipNextWord();

/*
 * copys the next phrase, before cutOffWord, onto destStr.
 */
int GetNextPhrase();

char *NextLines();

/*
 ** Gets a line of strings.
 *
 * PreCondition:  commandline must have atleat MAX_LINE_LENGTH characters.
 * PostCondition: commandline contains a line of strings ended with '\0'.
 *                terminate line with <return>.
 * Return: address of the string
 */
char *GetLine();

/*
 ** Determines if a line is all blank(without any printable characters
 ** for command). Control character are not accepted as command characters.
 *
 * PreCondition: inline must end with a zero.
 */
int AllBlank();

/*
 ** Searches a char is within a string.
 *
 * RETURN: The index value of where the search char is on the string
 *         -1 if search character is not in the string.
 * PRECONDITION: str must end with null.
 */
int SearchChar();

/*
 ** Cut the trailing spaces.  ie. move the '\0' toward the front,
 ** to fill up the tail spaces.
 *
 * PRECONDITION: str must end with null.
 * RETURN: Number of spaces cut.
 */
int CutTailSpace();

/* convert a string to int number value */
int strToVal();

/* puts the int val in str form*/
char *valToStr();

/*
 * see if str contains any characters in set.
 */
int anyCommonChar();

/*
 * see if character ch is in string str
 * returns the index of ch in str
 */
int charIsInStr();

int numOfChar();

/* 
 * trim off the enclosing quotes of a string
 */
char *trimQuote();

/* allocates bigger space, and append to it.. the original string is freed*/
char *append();

/* same as append, with a carriage appended at the end */
char *appendLine();

char *saveString();
char *saveStringN();
char *VsaveString();

int eqStr();

char *listSum2Str();

void insertChar();
int shiftStr();

char *getLines();
char *enQuote();
char *enBracket();
char *deBracket();
char *listItem();
char *extractWord();

