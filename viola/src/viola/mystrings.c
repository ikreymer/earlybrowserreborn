/*
 * Copyright 1990 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * mystrings.c  -  Miscelaneous string functions.
 */

#include "utils.h"
#include "sys.h"
#include <ctype.h>
#include "mystrings.h"

#define CTRL_d 4
/*extern char *malloc();*/

char buff[BUFF_SIZE]; 
int buffi;

int numOfGBuffs = 0;
char *GBuff[NUM_OF_GBUFFS];
int GBuffIdx[NUM_OF_GBUFFS];
int GBuffSize[NUM_OF_GBUFFS];

/* Char strBuff[MAX_LINE_LENGTH]; */
char spaces[80] = {"                                                                              "};

long itemValArray[40];
long itemValArray2[40];

/* later, make into flags so no function call is necessary 
 */
int cmp_str(s1, s2)
	char *s1;
	char *s2;
{
	if (s1[0] == s2[0])
		if (!strcmp(s1, s2)) return 1;
	return 0;
}

int cmp_int(n1, n2)
	int n1;
	int n2;
{
	return n1 == n2;
}

void SkipBlanks(linep, i)
     char *linep;
     int *i;
{
  while (linep[*i]) {
    if (!ISSPACE(linep[*i])) return;
    ++(*i);
  }
}

int noCaseCharCmp(c1, c2)
     char c1, c2;
{
  if (c1 > 'Z') c1 -= 32; /* convert to lower case */
  if (c2 > 'Z') c2 -= 32;
  if (c1 == c2) return 1;
  else return 0;
}

/*
 * case insensitive strcmp
 */
int noCaseStrCmp(s1, s2)
     char *s1, *s2;
{
  while (*s1 && *s2) {
    if (!noCaseCharCmp(*s1++, *s2++)) return 0;
  }
  if (*s1 == '\0' && *s2 == '\0') return 1;
  return 0;
}

/*
 ** Stores the next word in linep in *wordp. Current postion in line is
 ** pointed to by index i.
 ** spaces around the word is trimed.
 *
 * PreCondition: i <= strlen(linep);
 *               *wordp must be large enough to hold any argument in *linep.
 * PostCondition: *wordp contains an argument string from *linep.
 */
int NextWord(linep, i, wordp)
     char *linep;
     int i;
     char *wordp;
{
  int j = 0;

  for (;;) {
    if (!linep[i]) {
      wordp[j] = '\0';
      return i;
    }
    if (!ISSPACE(linep[i])) break;
    ++i;
  }

  for (;;) {
    if (!linep[i]) break;
    if (ISSPACE(linep[i])) break;
    wordp[j++] = linep[i++];
  }
  wordp[j] = '\0';

  return i;
}

/*
 ** Skips the next word linep in *wordp. Current postion in line is
 ** pointed to by index i.
 ** will skip over spaces around the word.
 *
 * PreCondition: i <= strlen(linep);
 *               *wordp must be large enough to hold any argument in *linep.
 * PostCondition: *wordp contains an argument string from *linep.
 */
int SkipNextWord(linep, i)
     char *linep;
     int i;
{
  for (;;) {
    if (linep[i] == NULL) return i;
    if (!ISSPACE(linep[i])) break;
    ++i;
  }
  for (;;) {
    if (linep[i] == NULL) break;
    if (ISSPACE(linep[i])) break;
    ++i;
  }
  return i;
}

/*
 * copys the next phrase, up to the cutOffWord, onto destStr.
 * if no cutOffWord is found, the phrase string is copied anyway.
 */
int GetNextPhrase(str, i, destStr, cutOffWord)
     char   *str;
     int     i;
     char    *destStr;
     char    *cutOffWord;
{
  char c;
  int x, y, ci = i, cuti = 0, parenLevel = 0, quoteToggle = 0;
  int cutOffWordLen = strlen(cutOffWord);
  extern int hush;

  while (c = str[ci]) {
    if (parenLevel == 0 && quoteToggle == 0)
      if (c == cutOffWord[cuti]) {
	++cuti;
	if (cuti >= cutOffWordLen) {
	  ++ci;
	  goto gag;
	}
      } else {
	cuti = 0;
      }
    if (c == '\"') {
      if (quoteToggle) quoteToggle = 0;
      else quoteToggle = 1;
      cuti = 0;
    } else if (c == '(') {
      ++parenLevel;
      cuti = 0;
    } else if (c == ')') {
      --parenLevel;
      cuti = 0;
    }
    ++ci;
  }
  cuti = 0;
 gag:
  if (cuti) {
    for (y = 0, x = i; x < (ci - cuti); x++, y++) destStr[y] = str[x];
    destStr[y] = '\0';
    /* fprintf(stderr, "cuit=%d '%s' i=%d str='%s' ci=%d '%s'\n",cuti, cutOffWord, i, str, ci, destStr); */
    return ci;
  }
  for (y = 0, x = i; str[x]; x++, y++) destStr[y] = str[x];
  destStr[y] = '\0';
  /* fprintf(stderr, "'%s' i=%d str='%s' i=%d '%s'\n",cutOffWord, i, str, i, destStr);*/
  return i;
}

/*
 * gets and returns the next specified lines in a string. 
 * unless preceded by '\\' lines are separated by '\n'
 *
 * "lines" specify how many lines to read.
 * returns: number of characters in returned string
 *          string containing the next number of "lines"
 */
char *NextLines(textpp, lines, size)
     char **textpp;
     int *lines, *size;
{
  char c = '\0', *textSavep, *cp;
  int linesToGet = *lines;

  *lines = 0;
  *size = 0;

  if (textpp == NULL) return NULL;

  textSavep = *textpp;

  while (**textpp) {
    /* ignore '\n' if preceded by '\\' */    
    if (**textpp == '\n' || **textpp == '\r') {
      if (c == '\\') {
	/* back up -- don't want to include '\\' */
	c = **textpp;
	--(*size); 
	++(*textpp);
      } else {
	if (++(*lines) >= linesToGet) {
	  ++(*textpp);
	  break;
	}
      }
    }
    if (*size > BUFF_SIZE) {
      /* exceeding line buffer size */
      *textpp = textSavep;
      *lines = 0;
      *size = 0;
      return NULL;
    }
    c = **textpp;
    buff[(*size)++] = **textpp;
    ++(*textpp);
  }
  buff[(*size)] = '\0';
  if ((cp = (char*)malloc(sizeof(char) * (*size + 1))) == NULL) {
    perror("NextLines(): malloc failed.");
    return NULL;
  }
  strcpy(cp, buff);
  return cp;
}

/*
 * skip the next specified lines in a string. 
 * unless preceded by '\\' lines are separated by '\n'
 *
 * "lines" specify how many lines to skip.
 * returns: current textpp value;
 */
char *SkipNextLines(textpp, lines, size)
     char **textpp;
     int *lines, *size;
{
  char c = '\0';
  int linesToGet = *lines;

  *lines = 0;
  *size = 0;

  if (textpp == NULL) return NULL;

  for (;;) {
    if (**textpp == '\0') return NULL;

    /* ignore '\n' if preceded by '\\' */
    if (**textpp == '\n' || **textpp == '\r') {
      if (c == '\\') {
	/* back up -- don't want to include '\\' */
	c = **textpp;
	--(*size); 
	++(*textpp);
      } else {
	if (++(*lines) >= linesToGet) {
	  ++(*textpp);
	  return *textpp;
	}
      }
    }
    c = **textpp;
    ++(*size);
    ++(*textpp);
  }
}

/*
 ** Gets a line of strings.
 *
 * PreCondition:  commandline must have atleat MAX_LINE_LENGTH characters.
 * PostCondition: commandline contains a line of strings ended with '\0'.
 *                terminate line with <return>.
 * Return: address of the string
 */
char *GetLine(commandline)
     char *commandline;
{
  char c;
  int i = 0;

  while (((c = getchar()) != '\n') && (i < (MAX_LINE_LENGTH - 1))) {
    if (c == CTRL_d) break;
    commandline[i++] = c;
  }
  commandline[i] = '\0';

  return commandline;
}

/*
 ** Determines if a line is all blank(without any printable characters
 ** for command). Control character are not accepted as command characters.
 *
 * PreCondition: str must end with a zero.
 */
int AllBlank(str)
     char *str;
{
  if (str) {
    char c;

    while (c = *(str++)) {
      if (!ISSPACE(c)) return 0;
    }
  }
  return 1;
}

/*
 ** Searches a char is within a string.
 *
 * RETURN: The index value of where the search char is on the string
 *         -1 if search character is not in the string.
 * PRECONDITION: str must end with null.
 */
int SearchChar(str,sc)
     char *str;
     char sc;
{
  char c;
  int i = 0;

  while (c = str[i]) {
    if (c == sc) return i;
    i++;
  }
  return -1;
}

/*
 ** Cut the trailing spaces.  ie. move the '\0' toward the front,
 ** to fill up the tail spaces.
 *
 * PRECONDITION: str must end with null.
 * RETURN: Number of spaces cut.
 *         0 if strings is set to NULL.
 */
int CutTailSpace(str)
     char *str;
{
  if (str) {
    int l, i = 0;

    l = strlen(str);
    if (l <= 0) return 0;
    
    for (i = l - 1; (i >= 0) && (str[i] == ' '); i--) str[i] = '\0';
    
    return l - i - 1;
  }
  return 0;
}

/*
 * cuts of the spaces, if any, at the beginning and the end of a string. 
 */
char *trimEdgeSpaces(str)
     char *str;
{
  int i;
  char c, *cp, *cp2;

  if (!str) return str;
  if (!str[0]) return str;

  for (cp = str; c = *cp; cp++) {
    if (!ISSPACE(c)) {
      cp2 = str;
      if (cp > str) {
        /* shift string toward left to write over leading spaces */
        i = cp - str;
        while (*cp) *(cp2++) = *(cp++);
        *cp2 = '\0';
      } else {
        while (*(++cp2)); /* travel to end of string */
      }
      /* go backward and replace blanks with 0 */
      for(--cp2; cp2 >= str; cp2--) {
        c = *cp2;
        if (!ISSPACE(c)) {
          *(cp2+1) = '\0';
          return str;
        }
      }
      return str;
    }
  }
  return str;
}

/*
 * rids of the leading spaces
 * shift string toward left to write over leading spaces
 */
char *trimFrontSpaces(str)
     char *str;
{
  int i = 0, j = 0;

  do {
    if (!ISSPACE(str[i])) {
      if (i > 0) {
	/* shift string toward left to write over leading spaces */
	while (str[i]) str[j++] = str[i++];
	str[j] = '\0';
      }
      return str;
    }
  } while (str[i++]);

  return str;
}

/*
 ** Convert a string to signed int number value
 *
 * PRECONDITION: string must be unsigned and contain only digits with
 *               possibly sign(first char).
 *               Camas and decimal points are ignored.
 * RETURN: proper value. or 0 if str is NULL
 * NOTE: Spaces at edges are ok. ex: "   ---2345.3555 " -> (-2345)
 */
int strToVal(str)
     char *str;
{
  int i, j = 1, val = 0, negate = 1;

  if (str == NULL) return 0;

  for (i = strlen(str) - 1; i >= 0; i--) {
    /* fprintf(stderr, "%d %d %d %d\n",(*(str+i)-'0'), val,*(str+i),j);*/
    if (*(str+i) == '-') negate *= -1;
    if (*(str+i) == '.') {
      j = 1;
      val = 0;
      continue;
    }
    if (isdigit(*(str + i))) {
      val += (*(str + i) - '0') * j;
      j *= 10;
    }
  }
  val *= negate;
  return val;
}

/*
 ** puts the signed int val in str form
 *
 * PRECONDITION: val must be integer.
 */
char *valToStr(val, str)
     long val;
     char *str;
{
  long i, j = 0, digit;

  if (val > MAX_LONG) val = MAX_LONG;

  if (val == 0) {
    str[0] = '0';
    str[1] = '\0';
    return str;
  }

  if (val < 0) {  /* if value if negative */
    val *= -1;
    *(str + j++) = '-';
  }

  /* skip all non-zero digit*/
  for (i = MAX_LONG; i > 0 && (val / i) == 0 ; i /= 10);

  for (; i > 0; i /= 10) {
    digit = val / i;
    *(str + j) = digit + '0';
    val -= digit * i;
    j++;
/* 
  fprintf(stderr, "%ld %ld %ld %ld %ld %ld\n",
  (long)digit, (long)(*(str+j)-'0'), (long)val,
  (long)*(str + j), (long)j,(long)i);
*/
  }
  str[j++] = 0;

  return str;
}

/*
 * find where the common character is
 * RETURN: -1 if none.
 */
int commonCharAt(str, set)
     char *str, *set;
{
  int i = 0;

  while (*set) {
    if (charIsInStr(*set++, str)) return i;
    ++i;
  }
  return -1;
}

/*
 * see if str contains any characters in set.
 */
int anyCommonChar(str, set)
    char *str, *set;
{
  while (*str)
    if (charIsInStr(*str++, set)) return 1;
  return 0;
}

/*
 * see if character ch is in string str
 */
int charIsInStr(ch, str)
    char ch, *str;
{
  while (*str) {
    if (*str++ == ch)
      return 1;        /* change 1 to I will make if-then not work...*/
  }
  return 0;
}

int numOfChar(str,sc)
     char str[];
     char sc;
{
  char c;
  int i = 0, j = 0;

  while (c = str[i]) {
    if (c == sc) j++;
    i++;
  }
  return j;
}

/* 
 * trim off the enclosing quotes of a string
 */
char *trimQuote(str)
     char *str;
{
  char *s, *h = str;

  while (*str != '\0') {
    if (*str == '\"') {
      s = str;
      while (*s != '\0') {
	*s = *(s + 1);
	s++;
      }
    }
    str++;
  }
  return h;
}

/* allocates bigger space, and append to it.. the original string is freed*/
char *append(orig, append)
     char *orig, *append;
{
  char *sp;
  
  if (sp = (char*)malloc(sizeof(char)*(strlen(orig) + strlen(append) + 1))) {
    strcpy(sp, orig);
    strcat(sp, append);
    if (orig) free(orig);
    return sp;
  } else {
    fprintf(stderr, "malloc failed.");
    return NULL;
  }
}

/* same as append, with a carriage appended at the end */
char *appendLine(orig, append)
     char *orig, *append;
{
  char *sp;

  if (sp = (char*)malloc(sizeof(char) * (strlen(orig) + strlen(append) + 2))) {
    strcpy(sp, orig);
    strcat(sp, append);
    strcat(sp, "\n");
    if (orig) free(orig);
    return sp;
  } else {
    fprintf(stderr, "malloc failed.");
    return NULL;
  }
}

char *saveString(str)
     char *str;
{
  char *sp;

  if (!str) return NULL;/*bad*/

  if (sp = (char*)malloc(sizeof(char) * (strlen(str) + 1))) {
    strcpy(sp, str);
    return sp;
  }
  fprintf(stderr, "malloc failed.");
  return NULL;
}

char *saveStringN(str, size)
     char *str;
     int size;
{
  char *sp = (char*)malloc(sizeof(char) * size);

  if (sp) {
    strncpy(sp, str, size);
    return sp;
  }
  fprintf(stderr, "malloc failed.");
  return NULL;
}

char *VsaveString(group, str)
     MemoryGroup *group;
     char *str;
{
  char *sp;

  if (!str) return NULL;

  if (sp = (char*)Vmalloc(group, sizeof(char) * (strlen(str) + 1))) {
    strcpy(sp, str);
    return sp;
  } else {
    fprintf(stderr, "malloc failed.");
    return NULL;
  }
}

int eqStr(cmpStr, fixedStr)
     char *cmpStr, *fixedStr;
{
  char *sp = (char*)malloc(sizeof(char) * (strlen(cmpStr) + 1));

  strcpy(sp, cmpStr);
  trimEdgeSpaces(sp);
  if (!STRCMP(sp, fixedStr)) {
    free(sp);
    return 1;
  }
  free(sp);
  return 0;
}

char *listSum2Str(list1, list2, listLength, str)
     int list1[], list2[], listLength;
     char *str;
{
  int i = 0;
  char localBuff[40];

  str[0] = '\0';
  for (; i < listLength; i++) {
    strcat(str, valToStr((long)(list1[i] + list2[i]), localBuff));
    if (i < listLength - 1) strcat(str, ",");
  }
  return str;
}

void insertChar(eStr, col, c)
    char *eStr;
    int   col;
    char  c;
{
  shiftStr(eStr, col, 1);
  eStr[col] = c;
}

int shiftStr(strp, starti, shift)
     char *strp;
     int starti, shift;
{
  int length, shifts = 0;
  int i, j = 0;

  length = strlen(strp);
  if (shift > 0) {
    strp[length+shift] = '\0';
    for (i = length-starti; i > 0; i--) {
      j++;
      strp[length+shift-j] = strp[length - j];
      ++shifts;
    }
  } else {
    i = length - starti;
    strp[length - shift + j] = '\0';
    while (i >= 0) {
      strp[starti + j] = strp[starti - shift + j];
      --shifts;
      j++;
      i--;
    }
  }
  return shifts;
}

/*
 * Line numbering start at 0
 *
 * Up to the user to free returned data.
 */
char *getLines(low, high, text, size)
	int low, high;
	char *text;
	int *size;
{
	char *cp;
	char *begin = text;
	char *end;
	int i = 0;

	if (i < low)
		for (; *begin; begin++)
			if (*begin == '\n')
				if (++i >= low) {
					begin++;
					break;
				}
	end = begin;
	if (i <= high)
		for (; *end; end++)
			if (*end == '\n') 
				if (i++ >= high) {
					end++;
					break;
				}
	*size = end - begin;
	if (*size > 0) {
		cp = (char*)malloc(sizeof(char) * (*size + 1));
		strncpy(cp, begin, *size);
		cp[*size] = '\0';
	} else {
		cp = (char*)malloc(sizeof(char));
		cp[0] = '\0';
		*size = 1;
	}
	return cp;
}

/* UNUSED */
char *enQuote(str)
     char *str;
{
  char *buffp = buff;

  *buffp++ = '\"';
  while (*str) *buffp++ = *str++;
  *buffp++ = '\"';
  *buffp = '\0';

  return SaveString(buff);
}

char *enBracket(list)
     char *list;
{
  char *buffp = buff;

  *buffp++ = '{';
  while (*list) *buffp++ = *list++;
  *buffp++ = '}';
  *buffp = '\0';

  return SaveString(buff);
}

/* not properly done - merely replaces {...} with space 
 * may bomb if list string is empty ""
 */
char *deBracket(list)
     char *list;
{
  if (!list[0]) {
    return list;
  } else {
    int i;
    char c;

    for (i = 0; c = list[i]; i++) {
      if (c == '{') {
	list[i] = ' ';
	break;
      }
    }
    for (i = strlen(list); i >= 0; i--) {
      if (list[i] == '}') {
	list[i] = '\0';
	break;
      }
    }
    trimFrontSpaces(list);
    return list;
  }
}

/*
 * grabs item(s) out of a {list}
 */
char *listItem(list, li, hi)
     char *list;
     int li, hi;
{
  char c;
  int i, bi = 0, itemNum = 1, paren = 0;

  /* skip the first left bracket */
  for (i = 0; (c = *(list + i)) != '\0'; i++) {
    if (c == '{') {
      paren = 1;
      ++i;
      break;
    }
  }
  buff[bi++] = '{';

  for (; (c = *(list + i)) != '\0'; i++) {
    if (itemNum > hi) break;
    switch (c) {
    case '{':
      if (++paren >= 1)
	if (itemNum >= li) buff[bi++] = '{';
      break;
    case '}':
      if (--paren >= 1)
	if (itemNum >= li) buff[bi++] = '}';
      break;
    case ',':
      if (paren == 1) ++itemNum;
      if (paren >= 1) {
	if (itemNum == li) {
	  if (paren >= 2) buff[bi++] = ',';
	} else if ((itemNum > li) && (itemNum <= hi)) buff[bi++] = ',';
      }
      break;
    default:
      if (itemNum >= li) buff[bi++] = c;
    }
  }
  buff[bi++] = '}';
  buff[bi] = '\0';
  return SaveString(buff);
}


/*
 * individual items must be less than 32 characters long.
 */
int getItemVals(li, hi, itemStr)
     int li, hi;
     char *itemStr;
{
  int itemNum= 1;
  int bi = 0, i, flag = 0, ai=1;
  char c, str[32]; /* dangerous... */

  if (itemStr == NULL) {
    return NULL;
  }
  for (i = 0; (c = *(itemStr + i)) != '\0'; i++) {
    if (itemNum > hi) break;
    if (c == ',') {
      itemNum++;
      if (flag) {
	str[bi] = '\0';
	itemValArray[ai++] = (long)strToVal(str);
	bi = 0;
      }
    } else if (itemNum >= li) {
      if (c != '{' && c != '}') str[bi++] = c;
      flag = 1;
    }
  }
  str[bi] = '\0';
  itemValArray[ai++] = (long)strToVal(str);
  itemValArray[ai] = (long)ai;

  return ai;
}

char *extractWord(text, li, hi, retStr)
	char *text;
	int li, hi;
	char *retStr;
{
	int bi = 0, i = 0, itemNum = 1;
	char c;

	if (!isalnum(*text)) itemNum = 0;	/* head needs skiping */

	while ((c = *(text + i)) != '\0') {
		if (!isalnum(c)) {
			if (itemNum >= hi) goto done;
			do {
				if (itemNum >= li) retStr[bi++] = c;
				if ((c = *(text + (++i))) == '\0') goto done;
			} while (!isalnum(c));
			++itemNum;
		}
		if (itemNum >= li) retStr[bi++] = c;
		++i;
	}
 done:
	retStr[bi] = '\0';

	return retStr;
}
