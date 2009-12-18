/*
 * Cl.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Thu Apr 16 22:08:27 1992 tvr
 * Last modified: Wed May 13 13:27:58 1992 tvr
 *
 */

#include <stdio.h>

#include "Cl.h"

#include "HTStyle.h"
#include "HTParse.h"
#include "HText.h"
#include "tcp.h"

#include "../HText/HText.h"

extern HText_t *HtLocalText;

int WWWErwiseStatus;
ClConnection_t *WWWErwiseConnection;

void cl_free_connection ();

/*
 * Where to load this file ?
 */

char *WWWErwiseFileLoadName = 0;

/*
 * Open connection. Allocate strutures.
 */

ClConnection_t *
ClOpenConnection (address)
     char *address;
{
  int status;
  ClConnection_t *p = (ClConnection_t *) malloc (sizeof (*p));

  /*
   * Some validation checks
   */

  if (!p)
    return p;

  if (!address)
    return NULL;

  memset (p, 0, sizeof (*p));

  p->address = (char *) strdup (address);

  /*
   * If we wish to load to file, open fd here
   */

  if (WWWErwiseFileLoadName)
    {

      p->load_to_file_fd =
	open (WWWErwiseFileLoadName, O_WRONLY | O_CREAT, 0666);

      free (WWWErwiseFileLoadName);

      WWWErwiseFileLoadName = 0;

      if (p->load_to_file_fd < 0)
	{

	  printf ("ClOpenConnection: Cannot load to file '%s'\n",
		  WWWErwiseFileLoadName);

	  ClCloseConnection (p);

	  return 0;
	}

      p->load_to_file = 1;
    }


  /*
   * Kludge #1:
   * Call common code to get socket fd. Also set state to some value so
   * that ReadData calls some function that makes sense.
   */

  WWWErwiseConnection = p;

  WWWErwiseStatus = CL_CONTINUES;

  p->status = HTLoadAbsolute (address, 0);

  if ((WWWErwiseStatus == CL_FAILED) ||
      (WWWErwiseStatus == CL_ALREADY_LOADED))
    {

      ClCloseConnection (p);

      return NULL;
    }

  return p;
}


/*
 * Read data or poll connection opening
 */
struct HText *
ClReadData (connection, how_done, fd)
     ClConnection_t *connection;
     int *how_done;
     int *fd;
{
  int continues;

  void (*tmpf) ();

  WWWErwiseStatus = CL_CONTINUES;

  WWWErwiseConnection = connection;

  /*
   * If loading was on non-blocking mode ?
   */
  if (!connection->function)
    {

      *fd = connection->fd;

      if (connection->status)
	{

	  *how_done = CL_COMPLETED;

	  ClCloseConnection (connection);

	  return HtLocalText;

	}
      else
	{

	  *how_done = CL_FAILED;

	  ClCloseConnection (connection);

	  return 0;

	}
    }

  /*
   * Try max 3 pollings at one call. This makes loading faster but does not
   * get too much cpu
   */
  for (continues = 3; continues > 0; continues--)
    {

      tmpf = *connection->function;

      if (*connection->function)
	(void) (*connection->function) ();

      /*
       * If mode is going to change to poll, set it now
       */
      if (*connection->function == WWWErwiseSetPoll)
	(void) WWWErwiseSetPoll ();

      /*
       * Can we try next polling?
       */
      if ((*connection->function == tmpf) || (*how_done != CL_CONTINUES) ||
	  (connection->select_fd))
	{

	  continues = 0;
	}
    }

  *how_done = WWWErwiseStatus;

  *fd = connection->select_fd;

  if (*how_done == CL_FAILED)
    {

      ClCloseConnection (connection);

      return NULL;
    }

  if (*how_done == CL_COMPLETED)
    {

      ClCloseConnection (connection);

      return HtLocalText;	/* global variable because @#$#$ <censored> */
    }

  return NULL;
}


/*
 * User wants to terminate a connection
 */

void
ClCloseConnection (connection)
     ClConnection_t *connection;
{
  if (connection->load_to_file)
    {

      close (connection->load_to_file_fd);
    }

  if (connection->fd)
    {

      shutdown (connection->fd, 2);

      close (connection->fd);
    }

  if (connection->secondary_fd)
    {

      shutdown (connection->secondary_fd, 2);

      close (connection->secondary_fd);
    }

  cl_free_connection (connection);
}



/*
 * read data from local buffer. If no data on buffer, make normal read
 */
int
cl_read_data (fd, data, length)
     int fd;
     char *data;
     int length;
{
  ClConnection_t *p = WWWErwiseConnection;

  cl_data_t *b = p->buffer_first;

  int howmuch = 0;

  if (p->buffer_last)
    {
      if (!b)
	return 0;

      if (length < b->size)
	{

	  memcpy (data, b->data, length);

	  howmuch = length;

	  b->size -= length;

	  b->data += length;
	}
      else
	{

	  memcpy (data, b->data, b->size);

	  howmuch = b->size;

	  free (b->freeptr);

	  if (b->next)
	    {
	      b->next->prev = 0;
	      p->buffer_first = b->next;
	    }
	  else
	    {
	      p->buffer_first = 0;
	    }
	  free (b);
	}
    }
  else
    {
      return NETREAD (fd, data, length);
    }
  return howmuch;
}



/*
 * Free everything connection structure has malloced (and not yet freed)
 */

void
cl_free_connection (connection)
     ClConnection_t *connection;
{
  if (connection->address)
    free (connection->address);

  if (connection->addr)
    free (connection->addr);

  if (connection->command)
    free (connection->command);

  if (connection->buffer_first)
    {
      cl_data_t *p = connection->buffer_first;

      while (p)
	{

	  cl_data_t *p2 = p;

	  free (p->freeptr);

	  free (p);

	  p = p2->next;
	}
    }

  /*
   * NOTE!
   * anAnchor or diag should not be freed (at least they are not on
   * original common code.
   */
}


/*
 * Return true if loading to file is supported with this address
 */
int
ClCanLoadToFile (address)
     char *address;
{
  char *access;

  if (!address)
    {
      return 0;
    }

  access = HTParse (address, "", PARSE_ACCESS);

  if (!strcmp (access, "html"))
    {

      free (access);

      return 1;
    }

  if (!strcmp (access, "file"))
    {

      free (access);

      return 1;
    }

  free (access);

  return 0;
}


/*
 * Are we loading this connection to file ?
 */
int
ClConnectionOnLoadToFileMode (connection)
     ClConnection_t *connection;
{
  if (!connection)
    return 0;

  return connection->load_to_file;
}


void
ClSetFileNameForLoadingToFile (char *filename)
{
  if (filename)
    WWWErwiseFileLoadName = (char *) strdup (filename);
  else
    WWWErwiseFileLoadName = NULL;
}
