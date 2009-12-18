/*
 * ClFTP.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Sun Apr 19 22:42:51 1992 tvr
 * Last modified: Mon May 11 23:30:15 1992 tvr
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include "Cl.h"

#include "HTParse.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HTAnchor.h"

#define IPPORT_FTP 21

/*
 * Erwise's own ftp load function. Setup all things to be done
 */

PUBLIC int HTFTP_open_file_read
ARGS2 (
	char *, name,
	HTParentAnchor *, anchor
)
{
  int status;

  /*
   * Set up a list of things to do
   */

  {
    static void (*functions[]) () =
    {
      WWWErwiseConnect,
      WWWErwiseSetSelect,
      WWWErwiseFtpGetCommand,	/* get junk */

      WWWErwiseSetPoll,
      WWWErwiseFtpUser,		/* Send user */
      WWWErwiseSendCommand,
      WWWErwiseSetSelect,
      WWWErwiseFtpGetCommand,

      WWWErwiseSetPoll,
      WWWErwiseFtpPass,		/* send pass */
      WWWErwiseSendCommand,
      WWWErwiseSetSelect,
      WWWErwiseFtpGetCommand,

      WWWErwiseSetPoll,
      WWWErwiseFtpBinary,	/* set binary mode */
      WWWErwiseSendCommand,
      WWWErwiseSetSelect,
      WWWErwiseFtpGetCommand,

      WWWErwiseSetPoll,
      WWWErwiseFtpPassive,	/* set passive mode */
      WWWErwiseSendCommand,
      WWWErwiseSetSelect,

      WWWErwiseFtpGetPassive,	/* Get reply and request a file */
      WWWErwiseSetPoll,
      WWWErwiseSendCommand,

      WWWErwiseFtpDataChannel,	/* make connection */
      WWWErwiseConnect,

      WWWErwiseFtpCheckForError,/* check for file not found etc ... */

      WWWErwiseSetSelect,	/* read data */
      WWWErwiseReadData,

      WWWErwiseSetPoll,		/* parse stuff */
      WWWErwiseTerminateIfLoadToFile,
      WWWErwiseParse,
      NULL,
    };
    WWWErwiseConnection->function = functions;
  }

  /*
   * Set up information needed later
   */

  WWWErwiseConnection->ftphost = HTParse (name, "", PARSE_HOST);

  WWWErwiseConnection->anAnchor = anchor;

  /*
   * Start connecting to data port
   */

  status = cl_start_connection (WWWErwiseConnection->ftphost,
				WWWErwiseConnection,
				IPPORT_FTP);

  /*
   * FTP mode on formatting is PLAINTEXT
   */

  WWWErwiseConnection->diag = 1;

  return status;
}


/*
 * Send misc commands to data flow
 */

void
WWWErwiseFtpUser ()
{
  CL_DEBUG (("FTP: send user\n"));

  WWWErwiseConnection->command = strdup ("USER ftp\r\n");

  WWWErwiseConnection->function++;
}

void
WWWErwiseFtpPass ()
{
  char tmp[8192], hostname[1024];

  CL_DEBUG (("FTP: send pass\n"));

  if (!gethostname (hostname, 1024))
    {
      strcpy (hostname, "noname.rupu");
    }

  sprintf (tmp, "PASS erwise@%s\r\n", hostname);

  WWWErwiseConnection->command = strdup (tmp);

  WWWErwiseConnection->function++;
}

void
WWWErwiseFtpPassive ()
{
  CL_DEBUG (("FTP: send pasv\n"));

  WWWErwiseConnection->command = strdup ("PASV\r\n");

  WWWErwiseConnection->function++;
}

void
WWWErwiseFtpBinary ()
{
  CL_DEBUG (("FTP: set binary\n"));

  WWWErwiseConnection->command = strdup ("TYPE I\r\n");

  WWWErwiseConnection->function++;
}


/*
 * Get junk commands return when the are accomplished
 */

#define ERWISE_BL_SIZE   8192

void
WWWErwiseFtpGetCommand ()
{
  char tmp[ERWISE_BL_SIZE];

  int i;

  i = NETREAD (WWWErwiseConnection->fd, tmp, ERWISE_BL_SIZE);

  if ((i == -1) && (errno == EWOULDBLOCK))
    {
      return;
    }

  CL_DEBUG (("FTP: got %s\n", tmp));

  if ((tmp[0] < '2') || (tmp[0] > '3'))
    {

      WWWErwiseStatus = CL_FAILED;

      return;

    }

  WWWErwiseConnection->function++;

}
/*
 * Get return code from passive command
 */

void
WWWErwiseFtpGetPassive ()
{
  char tmp[1024], *p;
  int a1, a2, a3, a4;
  int p1, p2, port;

  int i;

  i = NETREAD (WWWErwiseConnection->fd, tmp, 1024);

  if ((i == -1) && (errno == EWOULDBLOCK))
    {
      return;
    }

  tmp[i] = 0;

  CL_DEBUG (("FTP: got passive %s\n", tmp));

  /*
   * Get to the line which is reply from PASV command.
   * Check for errors on the way.
   */

  p = tmp;

  i = 1;

  while (i)
    {
      if ((*p >= '0') && (*p <= '9'))
	{
	  if (!strncmp ("227", p, 3))
	    {
	      i = 0;
	      continue;
	    }

	  if ((p[0] < '2') || (tmp[0] > '3'))
	    {
	      WWWErwiseStatus = CL_FAILED;
	      return;
	    }
	}

      /*
       * Get next line
       */
      while (*p && (*p != '\n'))
	p++;

      p++;

      if (!*p)
	{
	  /*
           * no line but no errors yet
           */
	  return;
	}
    }

  /*
   * Correct line, get port
   */

  while (*p && (*p != '('))
    p++;

  if (!*p)
    {
      /*
       * 227 but no address / port ??
       */

      WWWErwiseStatus = CL_FAILED;

      return;
    }

  sscanf (p, "(%d,%d,%d,%d,%d,%d)", &a1, &a2, &a3, &a4, &p1, &p2);

  port = p1 * 256 + p2;

  printf ("ErwiseFTP: got connection to %d.%d.%d.%d on port %d\n",
	  a1, a2, a3, a4, port);

  /*
   * send receive command too
   */

  {
    char *filename = HTParse (WWWErwiseConnection->address, "",
			      PARSE_PATH + PARSE_PUNCTUATION);

    sprintf (tmp, "RETR %s\r\n", filename);

    WWWErwiseConnection->command = strdup (tmp);
  }

  WWWErwiseConnection->port = port;

  WWWErwiseConnection->function++;
}


/*
 * Make data channel connection
 */
void
WWWErwiseFtpDataChannel ()
{
  /*
   * Set data flow port to secondary so that it can be closed properly
   */

  WWWErwiseConnection->secondary_fd = WWWErwiseConnection->fd;

  cl_start_connection (WWWErwiseConnection->ftphost,
		       WWWErwiseConnection,
		       WWWErwiseConnection->port);

  WWWErwiseConnection->function++;
}



/*
 * Start connection. Make sure it is nonblocking one.
 */

int
cl_start_connection (host, connection, port)
     char *host;
     ClConnection_t *connection;
     int port;
{
  /*
   * Code partly from get_connection()
   */

  int s;

  struct hostent *phost;	/* Pointer to host -- See netdb.h */
  struct sockaddr_in soc_address;	/* Binary network address */
  struct sockaddr_in *sin = &soc_address;

/*  Set up defaults:
*/
  sin->sin_family = AF_INET;	/* Family, host order  */
  sin->sin_port = htons (port);

/* Get node name:
*/
  {
    if (*host >= '0' && *host <= '9')
      {				/* Numeric node address: */
	sin->sin_addr.s_addr = inet_addr (host);	/* See arpa/inet.h */

      }
    else
      {				/* Alphanumeric node name: */
	phost = gethostbyname (host);	/* See netdb.h */
	if (!phost)
	  {
	    if (TRACE)
	      printf (
		       "FTP: Can't find internet node name `%s'.\n",
		       host);

	    WWWErwiseStatus = CL_FAILED;

	    return -1;
	  }
	memcpy (&sin->sin_addr, phost->h_addr, phost->h_length);
      }

    if (TRACE)
      printf (
	       "FTP: Parsed remote address as port %d, inet %d.%d.%d.%d\n",
	       (unsigned int) ntohs (sin->sin_port),
	       (int) *((unsigned char *) (&sin->sin_addr) + 0),
	       (int) *((unsigned char *) (&sin->sin_addr) + 1),
	       (int) *((unsigned char *) (&sin->sin_addr) + 2),
	       (int) *((unsigned char *) (&sin->sin_addr) + 3));
  }				/* scope of p1 */


  {
    int status;

    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s < 0)
      {
	WWWErwiseStatus = CL_FAILED;

	return -1;
      }

    connection->fd = s;

    /*
     * Must NOT select connection now. Must poll.
     */
    connection->select_fd = 0;

    (void) fcntl (s, F_SETFL, O_NONBLOCK);
    (void) fcntl (s, F_SETFL, FNDELAY);

    status = erwise_connect (s,
			     (struct sockaddr *) & soc_address,
			     sizeof (soc_address));
  }

  /*
   * everything done
   */

  return s;
}



/*
 * Check for errors when retr is sent
 */

void
WWWErwiseFtpCheckForError ()
{
  fd_set readfds;
  fd_set exceptionfds;

  struct timeval tv;

  int fd, fd2;

  fd = WWWErwiseConnection->fd;
  fd2 = WWWErwiseConnection->secondary_fd;

  tv.tv_sec = tv.tv_usec = 0;

  FD_ZERO (&readfds);
  FD_SET (fd, &readfds);
  FD_SET (fd2, &readfds);

  FD_ZERO (&exceptionfds);
  FD_SET (fd, &exceptionfds);
  FD_SET (fd2, &exceptionfds);

  select ((fd > fd2 ? fd : fd2) + 1, &readfds, NULL, &exceptionfds, &tv);

  /*
   * Error on either data or command channel
   */
  if (FD_ISSET (fd, &exceptionfds) || FD_ISSET (fd2, &exceptionfds))
    {

      WWWErwiseStatus = CL_FAILED;

      return;
    }

  /*
   * Data is flowing (OK)
   */
  if (FD_ISSET (fd, &readfds))
    {

      WWWErwiseConnection->function++;

      return;
    }

  /*
   * Anything here is considered an error
   */
  if (FD_ISSET (fd2, &readfds))
    {

      WWWErwiseStatus = CL_FAILED;

      return;
    }
}
