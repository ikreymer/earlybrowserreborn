/*
 * HText.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Wed Feb 26 15:59:04 1992 tvr
 * Last modified: Mon Apr 27 00:41:22 1992 tvr
 *
 */

#include <stdio.h>

#include "HTStyle.h"
#include "HText.h"


/*
 * This file interacts with WWWLibrary and builds one hypertext page.
 * For every hypertext object on this page will be set
 *  - data, length
 *  - anchor information
 *  - style
 * Empty object will be generated for paragraph endings, newlines and
 * tabulators.
 */


/*
 * Debug module (temporary)
 */

#define HT_DEBUG(a)		/*printf a*/


/*
 * Local prototype
 */

void new_htext_object ();
void make_paragraph_object ();
void HText_setStyle ();


/*
 * Local text (temporary)
 */

struct HText *HtLocalText;


/*
 * When constructing data, save it first here. Then malloc needed space
 * and copy it
 */

char localdata[HTEXT_MAX_OBJECT_SIZE];


/*
 * Set same anchor information to all objects belonging to same
 * anchor.
 */

HTChildAnchor *local_anchor = 0;

/*
 * Default style
 */
extern HTStyleSheet *styleSheet;


/*
 * This file has functions that WWWLibrary uses to construct an hypertext
 * object.
 */

/*                      Creation Method
**                      ---------------
*/
struct HText *
HText_new (anchor)
     HTParentAnchor *anchor;
{
  struct HText *self;

  /*
   * Create hypertext object.
   */

  self = (struct HText *) malloc (sizeof (struct HText));

  if (!self)
    return self;

  HTAnchor_setDocument (anchor, (HyperDoc *) self);

  /*
   * Initialize object.
   */

  memset (self, 0, sizeof (struct HText));

  /*
   * Set some WWWLibrary stuff ...
   */
  self->node_anchor = anchor;

  /*
   * Setup hashing code
   */

  HtLocalText = self;

  return self;
}


/*      Free Entire Text
**      ----------------
*/
void 
HText_free (self)
     struct HText *self;
{
  struct HTextObject *p, *p2;
  struct HTextAnchor *a, *a2;

  if (!self)
    return;

  p = self->first;

  /*
   * Set some WWWLibrary stuff ...
   */
  if (self->node_anchor)
    HTAnchor_setDocument (self->node_anchor, (HyperDoc *) 0);


  /*
   * Free all data allocated on this file.
   */
  while (p)
    {
      p2 = p;
      p = p->next;

      /*
       * Avoid loop if something goes wrong
       */
      p2->next = p2->prev = NULL;

      if (p2->data)
	free (p2->data);

      free (p2);

    }

  a = self->anchorlist;

  while (a)
    {
      a2 = a;
      a = a->next;

      free (a2);
    }

  free (self);
}

/*
 * Get default style
 */
HTStyle *
ht_get_default_style ()
{
  HTStyle *p = styleSheet->styles;

  while (p && p->next)
    {
      if (!strcmp (p->SGMLTag, "XMP"))
	return p;
      p = p->next;
    }

  /*
   * Return last if xmp not found
   */
  return p;
}



/*                      Object Building methods
**                      -----------------------
**
**      These are used by a parser to build the text in an object
*/
void 
HText_beginAppend (text)
     struct HText *text;
{
  /*
   * Set object to accept text
   */

  new_htext_object (text);

  if (!text->first)
    {
      HT_DEBUG ((stderr, "htext begin append failed?\n"));
    }

  /*
   * Set default style. Some pages cause no setStyle.
   */
  HText_setStyle (text, ht_get_default_style ());
}


/*      New paragraph in current style
**      ------------------------------
** See also: setStyle.
*/

void 
HText_appendParagraph (text)
     struct HText *text;
{
  HT_DEBUG (("\nHText_appendParagraph\n"));

  /*
   * Create paragraph object
   */

  make_paragraph_object (text, HTEXT_PARAGRAPH);
}




/*      Append a character to the text object
**      -------------------------------------
*/
void 
HText_appendCharacter (text, ch)
     struct HText *text;
     char ch;
{
  struct HTextObject *p;
  int l;
  static char lastchar = 0;

  HT_DEBUG (("%c", ch));

  /*
   * safety
   */

  if (!text)
    return;

  /*
   * Parse special characters
   */
  if (ch == '\n')
    {

      make_paragraph_object (text, HTEXT_NEWLINE);

      return;

    }

  if (ch == '\t')
    {

      make_paragraph_object (text, HTEXT_TAB);

      return;

    }

  /*
   * Append space(s) at the end of objects
   */
  if (lastchar == ' ' && ch != ' ')
    {

      new_htext_object (text);

      /*
       * Don't loose this character
       */
      lastchar = ch;

      HText_appendCharacter (text, ch);

      return;
    }

  lastchar = ch;

  p = text->last;

  /*
   * If local buffer is full, make continued hypertext object.
   * This should not happen (is extreamely rare).
   */

  if (p->length >= HTEXT_MAX_OBJECT_SIZE)
    {

      p->paragraph = HTEXT_CONTINUE;

      new_htext_object (text);

      HText_appendCharacter (p, ch);

      /* return */

    }
  else
    {

      localdata[p->length++] = ch;
    }
}


HTextAnchor_t *
htext_new_anchor ()
{
  HTextAnchor_t *p;

  p = (HTextAnchor_t *) malloc (sizeof (HTextAnchor_t));

  if (!p)
    {
      HT_DEBUG (("cannot malloc on htext_new_anchor(). tough.\n"));

      exit (-1);
    }

  memset (p, 0, sizeof (HTextAnchor_t));

  return p;
}


/*              Anchor handling
**              ---------------
*/
/*      Start an anchor field
*/
void 
HText_beginAnchor (text, anc)
     struct HText *text;
     HTChildAnchor *anc;
{
  HT_DEBUG (("\nHText_beginAnchor [%s]\n", anc->tag));

  /*
   * Set up anchor
   */

  if (!anc)
    return;

  if (HTAnchor_followMainLink ((HTAnchor *) anc))
    {
      /*
       * Anchor is to be shown on the screen
       */
      new_htext_object (text);

      local_anchor = anc;
    }
  /*
   * Anchor is HREF anchor only
   */
  if (!text->anchorlist)
    {

      text->anchorlist = htext_new_anchor ();

      text->anchorlist->object = text->last;

      text->anchorlist->anchor = anc;

    }
  else
    {

      HTextAnchor_t *p = text->anchorlist;

      for (; p->next; p = p->next) ;

      p->next = htext_new_anchor ();

      p->next->anchor = anc;

      p->next->object = text->last;
    }
}



/*
 * End anchor creation
 */

void 
HText_endAnchor (text)
     struct HText *text;
{
  HT_DEBUG (("\nHText_endAnchor\n"));

  /*
   * Create this anchor an object and create new
   */

  new_htext_object (text);

  local_anchor = NULL;

}



void 
HText_appendText (text, str)
     struct HText *text;
     char *str;
{
  register char *p;

  HT_DEBUG (("HText_appendText by characters\n"));

  /*
   * Append characters (or better, words)
   */

  for (p = str; *p; p++)
    HText_appendCharacter (text, *p);

}

/*
 * End text appending
 */

void 
HText_endAppend (text)
     struct HText *text;
{
  HT_DEBUG (("HText_endAppend"));

  new_htext_object (text);
}


void 
HText_setStyle (text, style)
     struct HText *text;
     HTStyle *style;
{
  HT_DEBUG (("HText_setStyle %s\n", style->name));

  if (!style)
    {
      HT_DEBUG (("HText bug on HText_setStyle ... not changing style\n"));
      return;
    }

  /*
   * Begin style
   */
  new_htext_object (text);

  /*
   * If many headers in a row, append paragraph
   */
  if (text->last->style == style)
    {
      HText_appendParagraph (text);
    }

  text->last->style = style;
}



BOOL 
HText_select (text)
     struct HText *text;
{
  HT_DEBUG (("\nHText_select ??????\n"));
  /*
   * Do what ???
   */

  HtLocalText = text;
}



BOOL 
HText_selectAnchor (text, anchor)
     struct HText *text;
     struct HTChildAnchor *anchor;
{
  HT_DEBUG (("\nHText_selectAnchor\n"));

  /*
   * Something stupid ?
   */
}



/*
 * Allocate new object. Set up modes from previous. Set up data -area.
 */

struct HTextObject *
malloc_ht_object (prev)
     struct HTextObject *prev;
{
  struct HTextObject *p;

  /*
   * New object
   */

  p = (struct HTextObject *) malloc (sizeof (struct HTextObject));

  if (!p)
    {
      HT_DEBUG (("cannot malloc hypertextobject\n"));
      exit (-1);
    }

  if (prev)
    {

      memcpy (p, prev, sizeof (struct HTextObject));

      p->data = 0;

      p->length = 0;

      p->anchor = 0;

      p->paragraph = 0;

    }
  else
    {

      memset (p, 0, sizeof (struct HTextObject));

    }


  return p;
}




/*
 * Create new hypertext object.
 */

void 
new_htext_object (text)
     struct HText *text;
{
  struct HTextObject *p;

  /*
   * First object ?
   */
  if (text->first == 0)
    {
      text->first = text->last = malloc_ht_object (NULL);

      return;
    }

  /*
   * Temporary assignment
   */

  p = text->last;

  /*
   * Strip all null objects exept paragraph marks
   */
  if ((p->length == 0) && !(p->paragraph))
    return;

  /*
   * Check paragraph objects
   */
  if (p->paragraph && (p->paragraph != HTEXT_CONTINUE) && p->length)
    {
      HT_DEBUG (("non null paragraph object ?\n"));
      exit (-1);
    }

  /*
   * If constructing object, malloc and copy local data
   */

  if (p->length)
    {

      p->data = (char *) malloc (p->length + 1);

      if (!p->data)
	{
	  HT_DEBUG (("cannot malloc data on new_htext_object()\n"));
	  exit (-1);
	}

      memcpy (p->data, localdata, p->length);

      p->data[p->length] = 0;
    }

  /*
   * Set anchorinformation to object
   */

  p->anchor = local_anchor;

  /*
   * Create new object
   */

  p = malloc_ht_object (text->last);

  /*
   * Set it last
   */

  p->prev = text->last;

  text->last->next = p;

  text->last = p;
}



/*
 * Make empty object only stating paragraph, tab or newline
 */

void 
make_paragraph_object (text, type)
     struct HText *text;
     int type;
{
  new_htext_object (text);

  text->last->paragraph = type;

  new_htext_object (text);

}


/*
 * Malloc and copy data
 */
char *
ht_memdup (data, len)
     char *data;
     int len;
{
  char *p;

  p = (char *) malloc (len);

  if (!p)
    return 0;

  memcpy (p, data, len);

  return p;
}


/*
 * Duplicate object information of this hypertext-page
 */

HText_t *
HtDuplicate (text)
     HText_t *text;
{
  HText_t *newtext;
  HTextObject_t *o, *p;
  HTextObject_t *p_old = 0;

  newtext = (HText_t *) malloc (sizeof (*newtext));

  if (!newtext)
    return 0;

  memset (newtext, 0, sizeof (*newtext));

  o = text->first;

  /*
   * Allocate new htext -page info
   */
  if (o)
    {
      p = newtext->first = (HTextObject_t *) malloc (sizeof (*p));

      if (!p)
	{

	  free (newtext);

	  return 0;
	}
    }

  while (o)
    {
      memset (p, 0, sizeof (*p));

      p->paragraph = o->paragraph;

      p->length = o->length;

      p->style = o->style;

      p->data = (char *) ht_memdup (o->data, o->length);

      if (!p->data)
	{

	  goto free_allocated_error;
	}

      p->prev = p_old;

      if (o->next)
	{

	  p_old = p;

	  p = (HTextObject_t *) malloc (sizeof (*p));

	  if (!p)
	    {

	      goto free_allocated_error;
	    }

	  p_old->next = p;
	}

      o = o->next;
    }

  newtext->last = p;

  return newtext;

free_allocated_error:

  o = newtext->first;

  while (o)
    {

      if (o->data)
	free (o->data);

      free (o);
    }

  free (newtext);

  return 0;
}
