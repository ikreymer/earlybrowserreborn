/*
 * ClHTTP.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Fri Apr 17 23:43:01 1992 tvr
 * Last modified: Mon May 11 23:29:10 1992 tvr
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include "Cl.h"

#include "HTParse.h"
#include "HTFormat.h"
#include "HTAnchor.h"
#include "tcp.h"



/*
 * We want that loading won't kill the whole f*king client.
 *
 * This is a kludge ;)
 */

PUBLIC int HTLoadHTTP
ARGS4 (CONST char *, arg,
       CONST char *, gate,
       HTAnchor *, anAnchor,
       int, diag)
{
  /*
   * Most code from old loading function
   */

  int s;
  int status;
  char *command;

  struct sockaddr_in soc_address;	/* Binary network address */
  struct sockaddr_in *sin = &soc_address;

  if (!arg)
    return -3;			/* Bad if no name sepcified	*/
  if (!*arg)
    return -2;			/* Bad if name had zero length	*/

/*  Set up defaults:
*/
  sin->sin_family = AF_INET;	/* Family, host order  */
  sin->sin_port = htons (TCP_PORT);	/* Default: new port,    */

  if (TRACE)
    {
      if (gate)
	fprintf (stderr,
		 "HTTPAccess: Using gateway %s for %s\n", gate, arg);
      else
	fprintf (stderr, "HTTPAccess: Direct access for %s\n", arg);
    }

/* Get node name and optional port number:
*/
  {
    char *p1 = HTParse (gate ? gate : arg, "", PARSE_HOST);
    HTParseInet (sin, p1);
    free (p1);
  }


/*	Now, let's get a socket set up from the server for the sgml data:
*/
  s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  (void) fcntl (s, F_SETFL, O_NONBLOCK);
  (void) fcntl (s, F_SETFL, FNDELAY);

  /*
   * Now. Do first attempt to connect.
   */
  status = erwise_connect (s,
			   (struct sockaddr *) & soc_address,
			   sizeof (soc_address));

  if (s < 0)
    {
      /*
       * Fail
       */
      /*free(command);*/
      return -1;
    }

  if (TRACE)
    printf ("HTTP connected, socket %d\n", s);

/*	Ask that node for the document,
**	omitting the host name & anchor if not gatewayed.
*/
  if (gate)
    {
      command = malloc (4 + strlen (arg) + 1 + 1);
      strcpy (command, "GET ");
      strcat (command, arg);
    }
  else
    {				/* not gatewayed */
      char *p1 = HTParse (arg, "", PARSE_PATH | PARSE_PUNCTUATION);
      command = malloc (4 + strlen (p1) + 1 + 1);
      strcpy (command, "GET ");
      strcat (command, p1);
      free (p1);
    }
  strcat (command, "\r\n");	/* Include CR for telnet compat. */


  if (TRACE)
    printf ("HTTP writing command `%s' to socket %d\n", command, s);

#ifdef NOT_ASCII
  {
    char *p;
    for (p = command; *p; p++)
      {
	*p = TOASCII (*p);
      }
  }
#endif

  /*
   * Ok. Everything is now set up. Set functionpointers so that
   * the rest of loading works ok.
   */

  {
    static void (*functions[]) () =
    {
      WWWErwiseConnect,
      WWWErwiseSendCommand,
      WWWErwiseSetSelect,
      WWWErwiseReadData,
      WWWErwiseSetPoll,
      WWWErwiseTerminateIfLoadToFile,
      WWWErwiseParse,
      NULL,
    };
    WWWErwiseConnection->function = functions;
  }

  WWWErwiseConnection->command = command;

  WWWErwiseConnection->diag = diag;

  WWWErwiseConnection->anAnchor = anAnchor;

  /*
   * Cheat HTOpen()
   */
  return s;
}
