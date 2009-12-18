/*	HyperText Tranfer Protocol	- Client implementation		HTTP.c
**	==========================
*/

/*	Module parameters:
**	-----------------
**
**  These may be undefined and redefined by syspec.h
*/
#include "HTParse.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HTTCP.h"
#include "HTFormat.h"


#ifndef ERWISE
/*		Load Dcoument from HTTP Server			HTLoadHTTP()
**		==============================
**
**	Given a hypertext address, this routine loads a document.
**
**
** On entry,
**	arg	is the hypertext reference of the article to be loaded.
**	gate	is nill if no gateway, else the gateway address.
**
** On exit,
**	returns	>=0	If no error, a good socket number
**		<0	Error.
**
**	The socket must be closed by the caller after the document has been
**	read.
**
*/
PUBLIC int HTLoadHTTP ARGS4 (CONST char *, arg,
	CONST char *,	gate, 
	HTAnchor *,	anAnchor,
	int,		diag)
{
    int s;				/* Socket number for returned data */
    char *command;			/* The whole command */
    int status;				/* tcp return */
 
    struct sockaddr_in soc_address;	/* Binary network address */
    struct sockaddr_in* sin = &soc_address;

    if (!arg) return -3;			/* Bad if no name sepcified	*/
    if (!*arg) return -2;			/* Bad if name had zero length	*/

/*  Set up defaults:
*/
    sin->sin_family = AF_INET;	    		/* Family, host order  */
    sin->sin_port = htons(TCP_PORT);	    	/* Default: new port,    */

    if (TRACE) {
        if (gate) fprintf(stderr,
		"HTTPAccess: Using gateway %s for %s\n", gate, arg);
        else fprintf(stderr, "HTTPAccess: Direct access for %s\n", arg);
    }
    
/* Get node name and optional port number:
*/
    {
	char *p1 = HTParse(gate ? gate : arg, "", PARSE_HOST);
	HTParseInet(sin, p1);
        free(p1);
    }
    
   
/*	Now, let's get a socket set up from the server for the sgml data:
*/      
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    status = connect(s, (struct sockaddr*)&soc_address, sizeof(soc_address));
    if (status<0){
	if (TRACE) printf(
	    "HTTP: Unable to connect to remote host for `%s'.\n",
	    arg);
	free(command);
	return HTInetStatus("connect");
    }
    
    if (TRACE) printf("HTTP connected, socket %d\n", s);

/*	Ask that node for the document,
**	omitting the host name & anchor if not gatewayed.
*/        
    if (gate) {
        command = malloc(4 + strlen(arg)+ 2 + 1);
        if (command == NULL) outofmem(__FILE__, "HTLoadHTTP");
        strcpy(command, "GET ");
	strcat(command, arg);
    } else { /* not gatewayed */
	char * p1 = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION);
        command = malloc(4 + strlen(p1)+ 2 + 1);
      if (command == NULL) outofmem(__FILE__, "HTLoadHTTP");
        strcpy(command, "GET ");
	strcat(command, p1);
	free(p1);
    }
    strcat(command, "\r\n");		/* Include CR for telnet compat. */
	    

    if (TRACE) printf("HTTP writing command `%s' to socket %d\n", command, s);
    
#ifdef NOT_ASCII
    {
    	char * p;
	for(p = command; *p; p++) {
	    *p = TOASCII(*p);
	}
    }
#endif

    status = NETWRITE(s, command, (int)strlen(command));
    free(command);
    if (status<0){
	if (TRACE) printf("HTTPAccess: Unable to send command.\n");
	    return HTInetStatus("send");
    }

/*	Now load the  date
*/
    HTParseFormat(diag ? WWW_PLAINTEXT : WWW_HTML,
                (HTParentAnchor *) anAnchor, s);
    
    if (TRACE) printf("HTTP: close socket %d.\n", s);
    status = NETCLOSE(s);

    return HT_LOADED;			/* Good return */
}
#endif /* ERWISE */
