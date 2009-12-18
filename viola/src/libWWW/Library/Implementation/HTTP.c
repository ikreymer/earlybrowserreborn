/*	HyperText Tranfer Protocol	- Client implementation		HTTP.c
**	==========================
**
** Bugs:
**	Not implemented:
**		Forward
**		Redirection
**		Error handling
*/

/*	Module parameters:
**	-----------------
**
**  These may be undefined and redefined by syspec.h
*/

/*	MOSAIC_HACK2 is a kludge to guess the file type of trabsferred
**	file from the URL.  It is STRICTLY illegal to do this!
*/

/* Implements:
*/
#include "HTTP.h"
#define HTTP_VERSION	"HTTP/1.0"
#define HTTP2				/* Version is greater than 0.9 */

#define INIT_LINE_SIZE		1024	/* Start with line buffer this big */
#define LINE_EXTEND_THRESH	256	/* Minimum read size */
#define VERSION_LENGTH 		20	/* for returned protocol version */

int http_method = HTTP_METHOD_GET;
char *http_dataToPost = NULL;

/* Uses:
*/
#include "HTParse.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HTTCP.h"
#include "HTFormat.h"
#include <ctype.h>
#include "HTAlert.h"
#include "HTMIME.h"
#include "HTML.h"		/* SCW */
#include "HTInit.h"		/* SCW */
#include "HTAABrow.h"		/* Access Authorization */

struct _HTStream {
	HTStreamClass * isa;		/* all we need to know */
};


extern char * HTAppName;	/* Application name: please supply */
extern char * HTAppVersion;	/* Application version: please supply */

/*		Load Document from HTTP Server			HTLoadHTTP()
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
PUBLIC int HTLoadHTTP ARGS4 (
	CONST char *, 		arg,
/*	CONST char *,		gate, */
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    int s;				/* Socket number for returned data */
    char *command;			/* The whole command */
    char * eol = 0;			/* End of line if found */
    char * start_of_data;		/* Start of body of reply */
    int length;				/* Number of valid bytes in buffer */
    int status;				/* tcp return */
    char crlf[3];			/* A CR LF equivalent string */
    HTStream *	target = NULL;		/* Unconverted data */
    HTFormat format_in;			/* Format arriving in the message */
    char *auth = NULL;			/* Authorization information */

    CONST char* gate = 0;		/* disable this feature */
    SockA soc_address;			/* Binary network address */
    SockA * sin = &soc_address;
    BOOL had_header = NO;		/* Have we had at least one header? */
    char * text_buffer = NULL;
    char * binary_buffer = NULL;
    int buffer_length = INIT_LINE_SIZE;	/* Why not? */
    BOOL extensions = YES;		/* Assume good HTTP server */
    int server_status = 0;
    int offset = 0;

    int tries = 0;

    extern int http_progress_expected_total_bytes;  /*PYW*/
    extern int http_progress_subtotal_bytes;
    extern int http_progress_total_bytes;
    extern int http_progress_reporter_level;
    static int delimSize = 0;
    static char *delim = "Content-length: ";
    char delimBuf[32];
    int i, j, delimi = 0;
    char *cp, *buffp, buff[100];
    int foundContentLength = 0;

    if (delimSize == 0) delimSize = strlen(delim);


    if (!arg) return -3;		/* Bad if no name sepcified	*/
    if (!*arg) return -2;		/* Bad if name had zero length	*/

/*  Set up defaults:
*/
#ifdef DECNET
    sin->sdn_family = AF_DECnet;	    /* Family = DECnet, host order */
    sin->sdn_objnum = DNP_OBJ;          /* Default: http object number */
#else  /* Internet */
    sin->sin_family = AF_INET;	    /* Family = internet, host order */
    sin->sin_port = htons(TCP_PORT);    /* Default: http port    */
#endif

    sprintf(crlf, "%c%c", CR, LF);	/* To be corect on Mac, VM, etc */
    
    if (TRACE) {
        if (gate) fprintf(stderr,
		"HTTPAccess: Using gateway %s for %s\n", gate, arg);
        else fprintf(stderr, "HTTPAccess: Direct access for %s\n", arg);
    }
    
/* Get node name and optional port number:
*/
    {
	char *p1 = HTParse(gate ? gate : arg, "", PARSE_HOST);
	int status = HTParseInet(sin, p1);  /* TBL 920622 */
        free(p1);
	if (status) return status;   /* No such host for example */
    }
    
retry:

/*
** Compose authorization information (this was moved here
** from after the making of the connection so that the connection
** wouldn't have to wait while prompting username and password
** from the user).				-- AL 13.10.93
*/
#ifdef ACCESS_AUTH
#define FREE(x)	if (x) {free(x); x=NULL;}
    {
	char *docname;
	char *hostname;
	char *colon;
	int portnumber;

	docname = HTParse(arg, "", PARSE_PATH);
	hostname = HTParse((gate ? gate : arg), "", PARSE_HOST);
	if (hostname &&
	    NULL != (colon = strchr(hostname, ':'))) {
	    *(colon++) = '\0';	/* Chop off port number */
	    portnumber = atoi(colon);
	}
	else portnumber = 80;
	
	auth = HTAA_composeAuth(hostname, portnumber, docname);

	if (TRACE) {
	    if (auth)
		fprintf(stderr, "HTTP: Sending authorization: %s\n", auth);
	    else
		fprintf(stderr, "HTTP: Not sending authorization (yet)\n");
	}
	FREE(hostname);
	FREE(docname);
    }
#endif /* ACCESS_AUTH */
   
/*	Now, let's get a socket set up from the server for the data:
*/      
#ifdef DECNET
    s = socket(AF_DECnet, SOCK_STREAM, 0);
#else
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    status = connect(s, (struct sockaddr*)&soc_address, sizeof(soc_address));
    if (status < 0) {
	    if (TRACE) fprintf(stderr, 
	      "HTTP: Unable to connect to remote host for `%s' (errno = %d).\n", arg, errno);
	    /* free(command);   BUG OUT TBL 921121 */
	    return HTInetStatus("connect");
      }
    
    if (TRACE) fprintf(stderr, "HTTP connected, socket %d\n", s);

/*	Ask that node for the document,
**	omitting the host name & anchor if not gatewayed.
*/        
    if (gate) {
        command = malloc(4 + strlen(arg)+ 2 + 31);
        if (command == NULL) outofmem(__FILE__, "HTLoadHTTP");

	/*PYW*/
	if (http_method == HTTP_METHOD_POST) strcpy(command, "POST ");
	else strcpy(command, "GET");

	strcat(command, arg);
    } else { /* not gatewayed */
	char * p1 = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION);
        command = malloc(4 + strlen(p1)+ 2 + 31);
        if (command == NULL) outofmem(__FILE__, "HTLoadHTTP");

	/*PYW*/
	if (http_method == HTTP_METHOD_POST) strcpy(command, "POST ");
	else strcpy(command, "GET ");

	strcat(command, p1);
	free(p1);
    }
#ifdef HTTP2
    if (extensions) {
        strcat(command, " ");
        strcat(command, HTTP_VERSION);
    }
#endif

    strcat(command, crlf);	/* CR LF, as in rfc 977 */

    if (extensions) {

	int n;
	int i;
        HTAtom * present = WWW_PRESENT;
	char line[256];    /*@@@@ */

	if (!HTPresentations) HTFormatInit();
	n = HTList_count(HTPresentations);

	for(i=0; i<n; i++) {
	    HTPresentation * pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep_out == present) {
	      if (pres->quality != 1.0) {
                 sprintf(line, "Accept: %s q=%.3f%c%c",
			 HTAtom_name(pres->rep), pres->quality, CR, LF);
	      } else {
                 sprintf(line, "Accept: %s%c%c",
			 HTAtom_name(pres->rep), CR, LF);
	      }
	      StrAllocCat(command, line);

	    }
	}
	
	sprintf(line, "User-Agent:  %s/%s  libwww/%s (modified)%s",
		HTAppName ? HTAppName : "unknown",
		HTAppVersion ? HTAppVersion : "0.0",
		HTLibraryVersion, crlf);
	StrAllocCat(command, line);

#ifdef ACCESS_AUTH
	if (auth != NULL) {
	    sprintf(line, "%s%s", auth, crlf);
	    StrAllocCat(command, line);
	}
#endif /* ACCESS_AUTH */

	if (http_dataToPost) { /*PYW -- this is kludgy */
	  sprintf(line, "Content-type: application/x-www-form-urlencoded%s",
		  crlf);
	  StrAllocCat(command, line);

	  sprintf(line,"Content-length: %d%s", 
		  strlen(http_dataToPost), crlf);
	  StrAllocCat(command, line);
	  
	  StrAllocCat(command, crlf);	/* Blank line means "end" */

	  StrAllocCat(command, http_dataToPost);
	}
    }

    StrAllocCat(command, crlf);	/* Blank line means "end" */

    if (TRACE) fprintf(stderr, "HTTP Tx: %s\n", command);

/*	Translate into ASCII if necessary
*/
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
    if (status<0) {
	if (TRACE) fprintf(stderr, "HTTPAccess: Unable to send command.\n");
	    return HTInetStatus("send");
    }


/*	Read the first line of the response
**	-----------------------------------
**
**	HTTP0 servers must return ASCII style text, though it can in
**	principle be just text without any markup at all.
**	Full HTTP servers must return a response
**	line and RFC822 style header.  The response must therefore in
**	either case have a CRLF somewhere soon.
**
**	This is the theory.  In practice, there are (1993) unfortunately
**	many binary documents just served up with HTTP0.9.  This
**	means we have to preserve the binary buffer (on the assumption that
**	conversion from ASCII may lose information) in case it turns
**	out that we want the binary original.
*/

    {
    
    /* Get numeric status etc */

	BOOL end_of_file = NO;
	HTAtom * encoding = HTAtom_for("7bit");
	
	binary_buffer = (char *) malloc(buffer_length * sizeof(char));
	if (!binary_buffer) outofmem(__FILE__, "HTLoadHTTP");
	text_buffer = (char *) malloc(buffer_length * sizeof(char));
	if (!text_buffer) outofmem(__FILE__, "HTLoadHTTP");
	length = 0;

	http_progress_subtotal_bytes = 0;
	if (http_progress_reporter_level == 0) { /*PYW*/
	  http_progress_total_bytes = 0;
	  http_progress_expected_total_bytes = 0;
	}
	++http_progress_reporter_level;
	
	do {	/* Loop to read in the first line */
	    
	   /* Extend line buffer if necessary for those crazy WAIS URLs ;-) */
	   
	    if (buffer_length - length < LINE_EXTEND_THRESH) {
	        buffer_length = buffer_length + buffer_length;
		binary_buffer = (char *) realloc(
			binary_buffer, buffer_length * sizeof(char));
		if (!binary_buffer) outofmem(__FILE__, "HTLoadHTTP");
		text_buffer = (char *) realloc(
			text_buffer, buffer_length * sizeof(char));
		if (!text_buffer) outofmem(__FILE__, "HTLoadHTTP");
	    }
	    status = NETREAD(s, binary_buffer + length,
 	    			buffer_length - length -1);
	    if (status < 0) {
	        HTAlert("Unexpected network read error on response");
		NETCLOSE(s);
		return status;
	    }

	    if (TRACE) fprintf(stderr, "HTTP: read returned %d bytes.\n",
	    	 status);

#ifdef VIOLA  /* KLUDGE ALERT */
	    http_progress_notify(status);
#endif

	    if (status == 0) {
	        end_of_file = YES;
		break;
	    }
	    binary_buffer[length+status] = 0;


/*	Make an ASCII *copy* of the buffer
*/
#ifdef NOT_ASCII
	    if (TRACE) fprintf(stderr, "Local codes CR=%d, LF=%d\n", CR, LF);
#endif
	    {
		char * p;
		char * q;
		for(p = binary_buffer+length, q=text_buffer+length;
			*p; p++, q++) {
		    *q = FROMASCII(*p);
		}

		*q++ = 0;
	    }

/* Kludge to trap binary responses from illegal HTTP0.9 servers.
** First time we have enough, look at the stub in ASCII
** and get out of here if it doesn't look right.
**
** We also check for characters above 128 in the first few bytes, and
** if we find them we forget the html default.
**
** Bugs: A HTTP0.9 server returning a document starting "HTTP/"
**	will be taken as a HTTP 1.0 server.  Failure.
**	An HTTP 0.9 server returning a binary document with
**	characters < 128 will be read as ASCII.
*/
#define STUB_LENGTH 20
	    if (length < STUB_LENGTH && length+status >= STUB_LENGTH) {
		if(strncmp("HTTP/", text_buffer, 5)!=0) {
		    char *p;
		    start_of_data = text_buffer; /* reparse whole reply */
		    for(p=binary_buffer; p <binary_buffer+STUB_LENGTH;p++) {
		        if (((int)*p)&128) {
			    format_in = HTAtom_for("www/unknown");
			    length = length + status;
			    goto copy; /* out of while loop */
			}
		    }
		}
	    }
/* end kludge */

	    
	    eol = strchr(text_buffer + length, 10);	    
	    if (eol) {
	        *eol = 0;		/* Terminate the line */
	        if (eol[-1] == CR) eol[-1] = 0;	/* Chop trailing CR */
		                                /* = corrected to ==  -- AL  */
            }

	    length = length + status;

	} while (!eol && !end_of_file);		/* No LF */	    
    } /* Scope of loop variables */

    
/*	We now have a terminated unfolded line. Parse it.
**	-------------------------------------------------
*/

    if (TRACE)fprintf(stderr, "HTTP: Rx: %.70s\n", text_buffer);

    {
	int fields;
	char server_version [VERSION_LENGTH+1];


/* Kludge to work with old buggy servers.  They can't handle the third word
** so we try again without it.
*/
	if (extensions &&
		0==strcmp(text_buffer,		/* Old buggy server? */
		"Document address invalid or access not authorised")) {
	    extensions = NO;
	    if (binary_buffer) free(binary_buffer);
	    if (text_buffer) free(text_buffer);
	    if (TRACE) fprintf(stderr,
		"HTTP: close socket %d to retry with HTTP0\n", s);
	    NETCLOSE(s);
	    goto retry;		/* @@@@@@@@@@ */
	}
/* end kludge */

	fields = sscanf(text_buffer, "%20s%d",
	    server_version,
	    &server_status);

	if (fields < 2 || 
	       strncmp(server_version, "HTTP/", 5)!=0) { /* HTTP0 reply */
	    format_in = WWW_HTML;
	    start_of_data = text_buffer;	/* reread whole reply */
	    if (eol) *eol = '\n';		/* Reconstitute buffer */
	    
	} else {				/* Full HTTP reply */
	
	/*	Decode full HTTP response */
	
	    format_in = HTAtom_for("www/mime");
	    start_of_data = eol ? eol + 1 : text_buffer + length;

	    switch (server_status / 100) {
	    
	    default:		/* bad number */
		HTAlert("Unknown status reply from server!");
		break;
		
	    case 3:		/* Various forms of redirection */
		HTAlert(
	"Redirection response from server is not handled by this client");
		break;
		
	    case 4:		/* Access Authorization problem */
#ifdef ACCESS_AUTH
		switch (server_status) {
		  case 401:
		    length -= start_of_data - text_buffer;
		    if (HTAA_shouldRetryWithAuth(start_of_data, length, s)) {
			/* Clean up before retrying */
			if (binary_buffer) free(binary_buffer);
			if (text_buffer) free(text_buffer);
			if (TRACE) 
			    fprintf(stderr, "%s %d %s\n",
				    "HTTP: close socket", s,
				    "to retry with Access Authorization");
			(void)NETCLOSE(s);
			goto retry;
			break;
		    }
		    else {
			/* FALL THROUGH */
		    }
		  default:
		    {
			char *p1 = HTParse(gate ? gate : arg, "", PARSE_HOST);
			char * message;

			if (!(message = (char*)malloc(strlen(text_buffer) +
						      strlen(p1) + 100)))
			    outofmem(__FILE__, "HTTP 4xx status");
			sprintf(message,
				"HTTP server at %s replies:\n%s\n\n%s\n",
				p1, text_buffer,
				((server_status == 401) 
				 ? "Access Authorization package giving up.\n"
				 : ""));
			status = HTLoadError(sink, server_status, message);
			free(message);
			free(p1);
			goto clean_up;
		    }
		} /* switch */
		goto clean_up;
		break;
#else
		/* case 4 without Access Authorization falls through */
		/* to case 5 (previously "I think I goofed").  -- AL */
#endif /* ACCESS_AUTH */

	    case 5:		/* I think you goofed */
		{
		    char *p1 = HTParse(gate ? gate : arg, "", PARSE_HOST);
		    char * message = (char*)malloc(
			strlen(text_buffer)+strlen(p1) + 100);
		    if (!message) outofmem(__FILE__, "HTTP 5xx status");
		    sprintf(message,
		    "HTTP server at %s replies:\n%s", p1, text_buffer);
		    status = HTLoadError(sink, server_status, message);
		    free(message);
		    free(p1);
		    goto clean_up;
		}
		break;
		
	    case 2:		/* Good: Got MIME object */
/*printf("XXXXXXXXX retrying to read header text_buffer={%s}\n", 
      text_buffer);
*/
		/* don't exit until the whole header is read */
/*		if (strstr(text_buffer, "\n\n")) break;
		if (strstr(text_buffer, "\n\r\n")) break;
		if (tries++ < 3) goto retry;
*/
		break;
	    } /* switch on response code */
	
	}		/* Full HTTP reply */
	
    } /* scope of fields */

/*	Set up the stream stack to handle the body of the message
*/

copy:

    target = HTStreamStack(format_in,
			format_out,
			sink , anAnchor);

    if (!target) {
	char buffer[1024];	/* @@@@@@@@ */
	if (binary_buffer) free(binary_buffer);
	if (text_buffer) free(text_buffer);
	sprintf(buffer, "Sorry, no known way of converting %s to %s.",
		HTAtom_name(format_in), HTAtom_name(format_out));
	fprintf(stderr, "HTTP: %s", buffer);
	status = HTLoadError(sink, 501, buffer);
	goto clean_up;
    }

    
/*	Push the data down the stream
**	We have to remember the end of the first buffer we just read
*/
    if (format_in == WWW_HTML) {
        target = HTNetToText(target);	/* Pipe through CR stripper */
    }

    if (http_progress_reporter_level == 1) { 
        cp = strstr(binary_buffer, "Content-length: ");
        if (!cp) cp = strstr(binary_buffer, "Content-Length: ");
        if (cp) {
	      cp += strlen("Content-length: ");
	      for (buffp = buff; *cp != '\n'; buffp++, cp++) *buffp = *cp;
	      *buffp = '\0';
	      http_progress_expected_total_bytes = atoi(buff);
        }
	foundContentLength = 1;
    }

#ifdef WRONG
    http_progress_subtotal_bytes = 0;
    if (server_status && http_progress_reporter_level == 0) { /*PYW*/
      http_progress_total_bytes = 0;
      http_progress_expected_total_bytes = 0;

      if (server_status / 100 == 2) {
	char *sp = binary_buffer + (start_of_data - text_buffer);
	char lc = '\0', llc = '\0', lllc = '\0';
	for (;;) {
 	  if (*sp == delim[delimi]) {
	    if (++delimi >= delimSize) {
	      for (i = 0, j = strchr(sp, '\n') - sp; i < j; i++) 
		delimBuf[i] = *(sp+i);
	      delimBuf[i] = '\0';
	      http_progress_expected_total_bytes = atoi(delimBuf);
	      delimi = 0;
	      break;
	    }
	  } else delimi = 0;
	  if (llc == '\n' && lc == '\n') {
	    break;
	  } else if (lllc == '\n' && llc == '\r' && lc == '\n') {
	    break;
	  }
	  lllc = llc;
	  llc = lc;
	  lc = *sp++;
	}
      }
    }
#endif
    /* strip MIME headers */
    if (server_status) {
      if (server_status / 100 == 2 && format_out == WWW_SOURCE) {
	char *sp, *end;
	char lc = '\0', llc = '\0', lllc = '\0';

	if (foundContentLength == 0 && http_progress_reporter_level == 1) { 
            cp = strstr(binary_buffer, "Content-length: ");
            if (!cp) cp = strstr(binary_buffer, "Content-Length: ");
            if (cp) {
	          cp += strlen("Content-length: ");
	          for (buffp = buff; *cp != '\n'; buffp++, cp++) *buffp = *cp;
	          *buffp = '\0';
	          http_progress_expected_total_bytes = atoi(buff);
            }
        }

	{
	  sp = binary_buffer + (start_of_data - text_buffer);
	  while (*sp) {
	    if (llc == '\n' && lc == '\n') {
	      offset = sp - binary_buffer - (start_of_data - text_buffer);
	      goto wheee;
	    } else if (lllc == '\n' && llc == '\r' && lc == '\n') {
	      offset = sp - binary_buffer - (start_of_data - text_buffer);
	      goto wheee;
	    }
	    lllc = llc;
	    llc = lc;
	    lc = *sp++;
	  }
	}
	{
	  for (;;) {
	    status = NETREAD(s, binary_buffer, buffer_length-1);
	    if (status < 0) {
	      HTAlert("Unexpected network read error on response");
	      NETCLOSE(s);
	      return status;
	    }
	    sp = binary_buffer;
	    for (;;) {
	      if (llc == '\n' && lc == '\n') {
		offset = sp - binary_buffer;
		goto dofus;
	      } else if (lllc == '\n' && llc == '\r' && lc == '\n') {
		offset = sp - binary_buffer;
		goto dofus;
	      }
	      lllc = llc;
	      llc = lc;
	      lc = *sp++;
	    }
	  }
	dofus:
	  (*target->isa->put_block)(target,
				    binary_buffer + offset,
				    status - offset);
	  HTCopy(s, target);
	  (*target->isa->free)(target);
	  status = HT_LOADED;

	  goto clean_up;
	}
      }
    }
  wheee:
    (*target->isa->put_block)(target,
		binary_buffer + (start_of_data - text_buffer) + offset,
		length - (start_of_data - text_buffer) - offset);
    HTCopy(s, target);
    (*target->isa->free)(target);
    status = HT_LOADED;

/*	Clean up
*/
    
clean_up: 

    --http_progress_reporter_level;
    if (http_progress_reporter_level == 0) { 
        http_progress_total_bytes = 0;
        http_progress_expected_total_bytes = 0;
#ifdef VIOLA  /* KLUDGE ALERT */
        http_progress_notify(-1);/* to say "it's done" */
#endif
    }
    http_progress_subtotal_bytes = 0;

    if (binary_buffer) free(binary_buffer);
    if (text_buffer) free(text_buffer);

    if (TRACE) fprintf(stderr, "HTTP: close socket %d.\n", s);
    (void) NETCLOSE(s);

    return status;			/* Good return */

}


/*	Protocol descriptor
*/

GLOBALDEF PUBLIC HTProtocol HTTP = { "http", HTLoadHTTP, 0 };
