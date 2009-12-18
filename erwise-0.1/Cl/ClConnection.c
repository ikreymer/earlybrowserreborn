/*
 * ClConnection.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Mon Apr 20 19:29:14 1992 tvr
 * Last modified: Wed May 13 00:53:24 1992 tvr
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#include "Cl.h"

#include "HTParse.h"
#include "HTFormat.h"
#include "HTAnchor.h"
#include "tcp.h"

#include "../HText/HText.h"

extern HText_t *HtLocalText;


/*
 * Poll connecting until connection completes.
 */
void
WWWErwiseConnect ()
{
  int status;

  CL_DEBUG (("Poll connect\n"));

  status = connect (WWWErwiseConnection->fd,
		    (struct sockaddr *) WWWErwiseConnection->addr,
		    WWWErwiseConnection->addr_size);
  if (status < 0 && (errno != EISCONN))
    {
      if ((errno == EALREADY) || (errno == EINPROGRESS))
	{
	  /*
           * Would block
           */
	  return;
	}

      CL_DEBUG (("Cannot connect(%d)\n", errno));

      /*
       * Cannot connect
       */
      WWWErwiseStatus = CL_FAILED;

      return;
    }

  /*
   * Connected. Get next function from the list of things to do.
   * Also, stop polling.
   */

  free (WWWErwiseConnection->addr);

  WWWErwiseConnection->addr = NULL;

  WWWErwiseConnection->function++;
}



/*
 * Instead of connect, this function is called. Store needed data to
 * poll connecting later.
 */
int
erwise_connect (fd, addr, size)
     int fd;
     struct sockaddr *addr;
     int size;
{
  int status;

  CL_DEBUG (("Try to Connect\n"));

  status = connect (fd, addr, size);

  if (status < 0)
    {
      switch (errno)
	{
	case EINPROGRESS:
	case EISCONN:
	case EALREADY:
	  break;

	default:
	  CL_DEBUG (("Cannot connect (first try %d)\n", errno));
	  return -1;
	}
    }

  /*
   * Duplicate what to connect
   */
  WWWErwiseConnection->addr = (void *) malloc (size);

  memcpy (WWWErwiseConnection->addr, addr, size);

  WWWErwiseConnection->addr_size = size;

  /*
   * OK
   */
  return 0;
}





/*
 * Send command to net
 */

void
WWWErwiseSendCommand ()
{
  int status;

  CL_DEBUG (("Send Command\n"));

  status = NETWRITE (WWWErwiseConnection->fd,
		     WWWErwiseConnection->command,
		     (int) strlen (WWWErwiseConnection->command));

  if (status == strlen (WWWErwiseConnection->command))
    {
      /*
       * Succeeded
       */
      free (WWWErwiseConnection->command);
      WWWErwiseConnection->command = 0;

      WWWErwiseConnection->function++;

    }
  else if (status < 0)
    {
      /*
       * Failed
       */
      CL_DEBUG (("SendCommand failed\n"));

      WWWErwiseStatus = CL_FAILED;
      return;
    }
  else
    {
      /*
       * Partial read
       */
      char *tmp = WWWErwiseConnection->command;

      WWWErwiseConnection->command = (char *) strdup (tmp + status);
      free (tmp);
    }
}


#define ERWISE_BLOCK 8192

/*
 * Read data until all data is read
 */

void
WWWErwiseReadData ()
{
  char tmp[ERWISE_BLOCK];

  int i;

  i = read (WWWErwiseConnection->fd,
	    tmp,
	    ERWISE_BLOCK);

  CL_DEBUG (("got %d bytes\n", i));

  /*
   * Append data to (memory) buffer or to file.
   */
  if (i > 0)
    {

      /*
       * Load directly to file ?
       */

      if (WWWErwiseConnection->load_to_file)
	{
	  int st;

	  st = write (WWWErwiseConnection->load_to_file_fd,
		      tmp,
		      i);

	  if (st != i)
	    {
	      WWWErwiseStatus = CL_FAILED;
	    }

	  return;
	}

      if (!WWWErwiseConnection->buffer_first)
	{

	  WWWErwiseConnection->buffer_first =
	    WWWErwiseConnection->buffer_last =
	    (cl_data_t *) malloc (sizeof (cl_data_t));

	  memset (WWWErwiseConnection->buffer_first, 0, sizeof (cl_data_t));

	  WWWErwiseConnection->buffer_first->data =
	    WWWErwiseConnection->buffer_first->freeptr =
	    (void *) malloc (i);

	  WWWErwiseConnection->buffer_first->size = i;

	  memcpy (WWWErwiseConnection->buffer_first->data, tmp, i);

	}
      else
	{

	  cl_data_t *p = (cl_data_t *) malloc (sizeof (cl_data_t));

	  memset (p, 0, sizeof (cl_data_t));

	  p->data = p->freeptr = (void *) malloc (i);

	  p->size = i;

	  memcpy (p->data, tmp, i);

	  p->prev = WWWErwiseConnection->buffer_last;

	  WWWErwiseConnection->buffer_last->next = p;

	  WWWErwiseConnection->buffer_last = p;
	}

      return;
    }

  if (i < 0 && (errno != EWOULDBLOCK))
    {
      CL_DEBUG (("ReadData failed\n"));
      WWWErwiseStatus = CL_FAILED;
    }

  if (i == 0)
    {
      WWWErwiseConnection->function++;
      return;
    }
}



/*
 * Parse data that has been read
 */

void
WWWErwiseParse ()
{
  /*
   * XXXXXX If saving to file, don't parse
   */

  HTParseFormat (WWWErwiseConnection->diag ? WWW_PLAINTEXT : WWW_HTML,
		 WWWErwiseConnection->anAnchor,
		 WWWErwiseConnection->fd);

  (void) HTClose (WWWErwiseConnection->fd);

  /*
   * XXXXX free connection structure
   */
  WWWErwiseStatus = CL_COMPLETED;
  WWWErwiseConnection->function++;
}



/*
 * If we are loading to file, nothing else needs to be done ...
 */
void
WWWErwiseTerminateIfLoadToFile ()
{
  if (WWWErwiseConnection->load_to_file)
    {
      HtLocalText = 0;

      while (*WWWErwiseConnection->function)
	{
	  WWWErwiseConnection->function++;
	}

      WWWErwiseStatus = CL_COMPLETED;

    }
  else
    {
      WWWErwiseConnection->function++;
    }
}
