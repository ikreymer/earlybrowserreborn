/*
 * HText.h --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Wed Feb 26 15:57:03 1992 tvr
 * Last modified: Mon Apr 27 23:55:47 1992 tvr
 *
 */


/*
 * Maximum Htext object data size
 */

#define HTEXT_MAX_OBJECT_SIZE	1024

/*
 * Region changes on hypertext
 */
#define HTEXT_PARAGRAPH		1
#define	HTEXT_TAB		2
#define	HTEXT_NEWLINE		3
#define	HTEXT_CONTINUE		4


/*
 * Hypertext object structure.
 */

typedef struct HTextObject
{
  /*
   * Pointers to next and previous objects
   */

  struct HTextObject *prev;
  struct HTextObject *next;

  /*
   * All Hypertext specific fields here
   */

  /*
   * If object is an anchor, have anchordata here
   */
  HTChildAnchor *anchor;

  /*
   * Style of this object.
   */
  HTStyle *style;

  /*
   * what data on this object
   */
  char *data;

  /*
   * How many bytes data
   */
  int length;

  /*
   * Marks paragraph change at the end of this object
   */
  int paragraph;

  /*
   * All Xl specific data here
   */

  /*
   * Position of an object on a virtual screen. These will be set when
   * object is being positioned.
   */
  long x;
  long y;

  /*
   * Size of an object. These are calculated once (as these does
   * not change).
   */
  long width;
  long height;

  /*
   * Xl specific data of this object
   */
  struct XlObjectData *xl_data;
} HTextObject_t;


/*
 * Hypertext structure of a page
 */

typedef struct HText
{
  /*
   * First and last hypertext objects
   */

  struct HTextObject *first;
  struct HTextObject *last;

  /*
   * Node anchor of this page
   */
  HTParentAnchor *node_anchor;

  /*
   * Xl specific global data for a page
   */
  struct XlGlobalData *xl_global;

  /*
   * HREF pointer list
   */
  struct HTextAnchor *anchorlist;

  /*
   * Cursor object of this page
   */
  struct HTextObject *cursor;

} HText_t;


/*
 * HREF pointer list on a page
 */
typedef struct HTextAnchor
{
  /*
   * Pointer to next
   */
  struct HTextAnchor *next;

  /*
   * Anchor data
   */
  HTChildAnchor *anchor;

  /*
   * To which hypertextobject is this connected to
   */
  struct HTextObject *object;
} HTextAnchor_t;



/*
 * Prototype
 */

HText_t *HtDuplicate (HText_t * text);
