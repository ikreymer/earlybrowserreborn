/*		Access Manager					HTAccess.c
**		==============
*/


#include "HTParse.h"
#include "HTUtils.h"
#include "WWW.h"
#include "HTAnchor.h"
#include "HTFTP.h"
#include "HTTP.h"
#include "HTFile.h"
#include <errno.h>
#include <stdio.h>

#include "tcp.h"
#include "HText.h"
#include "HTNews.h"
#include "HTGopher.h"
#include "HTBrowse.h"		/* Need global HTClientHost */

#include "HTAccess.h"

#ifdef ERWISE
#include "Cl.h"
#endif

#define HT_NO_DATA -9999

PUBLIC int HTDiag = 0;		/* Diagnostics: load source as text */

/*	Telnet or "rlogin" access
**	-------------------------
*/
PRIVATE int remote_session ARGS2(char *, access, char *, host)
{
	char * user = host;
	char * hostname = strchr(host, '@');
	char * port = strchr(host, ':');
	char   command[256];
	BOOL rlogin = strcmp(access, "rlogin");
	
	if (hostname) {
	    *hostname++ = 0;	/* Split */
	} else {
	    hostname = host;
	    user = 0;		/* No user specified */
	}
	if (port) *port++ = 0;	/* Split */

#ifdef unix
	sprintf(command, "%s%s%s %s %s", access,
		user ? " -l " : "",
		user ? user : "",
		hostname,
		port ? port : "");
	if (TRACE) fprintf(stderr, "HTaccess: Command is: %s\n", command);
	system(command);
	return HT_NO_DATA;		/* Ok - it was done but no data */
#define TELNET_DONE
#endif

#ifdef MULTINET				/* VMS varieties */
	if (!rlogin) {			/* telnet */
	    if (user) printf("When you are connected, log in as %s\n", user);
	    sprintf(command, "TELNET %s%s %s",
		port ? "/PORT=" : "",
		port ? port : "",
		hostname);
	} else {
	    sprintf(command, "RLOGIN%s%s%s%s %s", access,
		user ? "/USERNAME=" : "",
		user ? user : "",
		port ? "/PORT=" : "",
		port ? port : "",
		hostname);
	}
	if (TRACE) fprintf(stderr, "HTaccess: Command is: %s\n", command);
	system(command);
	return HT_NO_DATA;		/* Ok - it was done but no data */
#define TELNET_DONE
#endif

#ifdef UCX
#define SIMPLE_TELNET
#endif
#ifdef VM
#define SIMPLE_TELNET
#endif
#ifdef SIMPLE_TELNET
	if (!rlogin) {			/* telnet only */
	    if (user) printf("When you are connected, log in as %s\n", user);
	    sprintf(command, "TELNET  %s",	/* @@ Bug: port ignored */
		hostname);
	    if (TRACE) fprintf(stderr, "HTaccess: Command is: %s\n", command);
	    system(command);
	    return HT_NO_DATA;		/* Ok - it was done but no data */
	}
#endif

#ifndef TELNET_DONE
	fprintf(stderr,
	"Sorry, this browser was compiled without the %s access option.\n",
		access);
	fprintf(stderr,
	"\nTo access the information you must %s to %s", access, hostname);
	if (port) fprintf(stderr," (port %s)", port);
	if (user) fprintf(stderr," logging in with username %s", user);
	fprintf(stderr, ".\n");
	return -1;
#endif
}

/*	Open a file descriptor for a document
**	-------------------------------------
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**
** On exit,
**	returns		<0	Error has occured.
**			>=0	Value of file descriptor or socket to be used
**				 to read data.
**	*pFormat	Set to the format of the file, if known.
**			(See WWW.h)
**
*/
PRIVATE int HTOpen ARGS3(
	CONST char *,addr1,
	HTFormat *,pFormat,
	HTParentAnchor *,anchor)
{
    char * access=0;	/* Name of access method */
    int status;
    char * gateway;
    char * gateway_parameter;
    char * addr = (char *)malloc(strlen(addr1)+1);
    
    if (addr == NULL) outofmem(__FILE__, "HTOpen");
    strcpy(addr, addr1);			/* Copy to play with */
    
    access =  HTParse(addr, "file:", PARSE_ACCESS);
    
    gateway_parameter = (char *)malloc(strlen(access)+20);
    if (gateway_parameter == NULL) outofmem(__FILE__, "HTOpen");
    strcpy(gateway_parameter, "WWW_");
    strcat(gateway_parameter, access);
    strcat(gateway_parameter, "_GATEWAY");
    gateway = getenv(gateway_parameter);
    free(gateway_parameter);

    if (gateway) {
	status = HTLoadHTTP(addr, gateway, anchor, HTDiag);
#ifndef CURSES
	if (status<0) fprintf(stderr,	/* For simple users */
	    "Cannot connect to information gateway %s\n", gateway);
#endif 
    } else if (0==strcmp(access, "http")) {
        status = HTLoadHTTP(addr, 0, anchor, HTDiag);
#ifndef CURSES
	if (status<0) fprintf(stderr,	/* For simple users */
		"Cannot connect to information server.\n");
#endif
    } else if (0==strcmp(access, "file")) {
        status = HTOpenFile(addr, pFormat, anchor);

    } else if (0==strcmp(access, "news")) {
        status = HTLoadNews(addr, anchor, HTDiag);
	if (status>0) status = HT_LOADED;

    } else if (0==strcmp(access, "gopher")) {
        status = HTLoadGopher(addr, anchor, HTDiag);
	if (status>0) status = HT_LOADED;

    } else if (!strcmp(access, "telnet") ||		/* TELNET */
    	!strcmp(access, "rlogin")) {			/* RLOGIN */
        char * host = HTParse(addr, "", PARSE_HOST);
	remote_session(access, host);
	free(host);
	
    } else if (0==strcmp(access, "wais")) {
        user_message(
"HTAccess: For WAIS access set WWW_wais_GATEWAY to gateway address.\n");
    } else {

        user_message(
	"HTAccess: name scheme `%s' unknown by this browser version.\n",
		 access);
        status = -1;
    }
    free(access);
    free(addr);
    return status;
}


/*	Close socket opened for reading a file
**	--------------------------------------
**
*/
#ifdef ERWISE
PUBLIC int HTClose ARGS1(int,soc)
#else
PRIVATE int HTClose ARGS1(int,soc)
#endif
{
    return HTFTP_close_file(soc);
}


/*		Load a document
**		---------------
**
**    On Entry,
**	  anchor	    is the node_anchor for the document
**        full_address      The address of the file to be accessed.
**
**    On Exit,
**        returns    YES     Success in opening file
**                   NO      Failure 
**
*/

PUBLIC BOOL HTLoadDocument ARGS3(HTParentAnchor *,anchor,
	CONST char *,full_address,
	BOOL,	filter)

{
    int	        new_file_number;
    HTFormat    format;
    HText *	text;

    if (text=(HText *)HTAnchor_document(anchor)) {	/* Already loaded */
#ifdef ERWISE
        /*
         * do NOT do this
         */
        fprintf(stderr, "HTBrowse: Document already in memory.\n");

        WWWErwiseStatus = CL_ALREADY_LOADED;

        return YES;
#else
        if (TRACE) fprintf(stderr, "HTBrowse: Document already in memory.\n");
        HText_select(text);
	return YES;
#endif
    }
    
#ifdef CURSES
      prompt_set("Retrieving document...");
#endif
    if (filter) {
        new_file_number = 0;
	format = WWW_HTML;
    } else {
	new_file_number = HTOpen(full_address, &format, anchor);
    }
/*	Log the access if necessary
*/
    if (logfile) {
	time_t theTime;
	time(&theTime);
	fprintf(logfile, "%24.24s %s %s %s\n",
	    ctime(&theTime),
	    HTClientHost ? HTClientHost : "local",
	    new_file_number<0 ? "FAIL" : "GET",
	    full_address);
	fflush(logfile);	/* Actually update it on disk */
	if (TRACE) fprintf(stderr, "Log: %24.24s %s %s %s\n",
	    ctime(&theTime),
	    HTClientHost ? HTClientHost : "local",
	    new_file_number<0 ? "FAIL" : "GET",
	    full_address);
    }
    

    if (new_file_number == HT_LOADED) {
	if (TRACE) {
	    printf("HTAccess: `%s' has been accessed.\n",
	    full_address);
	}
#ifdef ERWISE
        WWWErwiseStatus = CL_COMPLETED;
        WWWErwiseConnection->fd = -1;
#endif
	return YES;
    }
    
    if (new_file_number == HT_NO_DATA) {
	if (TRACE) {
	    printf("HTAccess: `%s' has been accessed, No data left.\n",
	    full_address);
	}
#ifdef ERWISE
        WWWErwiseStatus = CL_FAILED;
#endif
	return NO;
    }
    
    if (new_file_number<0) {		      /* Failure in accessing a file */

#ifdef CURSES
        user_message("Can't access `%s'", full_address);
#else
	printf("\nWWW: Can't access `%s'\n", full_address);
#endif
#ifdef ERWISE
        WWWErwiseStatus = CL_FAILED;
        return NO;
#endif
	if (!HTMainText){
            exit(2);				/* Can't get first page */
        } else {
            return NO;
        }
    }
    
    if (TRACE) {
	printf("WWW: Opened `%s' as fd %d\n",
	full_address, new_file_number);
    }

#ifdef ERWISE
    /*
     * Do the rest elsewhere (if this connection can be loaded
     * using non blocking transfer)
     */

    if(WWWErwiseConnection->function) {
        WWWErwiseConnection->fd = new_file_number;

        return YES;
    }
#endif

    HTParseFormat(HTDiag ? WWW_PLAINTEXT : format, anchor, new_file_number);
    
    HTClose(new_file_number);
    
    return YES;
    
} /* HTLoadDocument */


/*		Load a document from absolute name
**		---------------
**
**    On Entry,
**        relative_name     The relative address of the file to be accessed.
**
**    On Exit,
**        returns    YES     Success in opening file
**                   NO      Failure 
**
**
*/

PUBLIC BOOL HTLoadAbsolute ARGS2(CONST char *,addr, BOOL, filter)
{
   return HTLoadDocument(
       		HTAnchor_parent(HTAnchor_findAddress(addr)),
       		addr, filter);
}


/*		Load a document from relative name
**		---------------
**
**    On Entry,
**        relative_name     The relative address of the file to be accessed.
**
**    On Exit,
**        returns    YES     Success in opening file
**                   NO      Failure 
**
**
*/

PUBLIC BOOL HTLoadRelative ARGS1(CONST char *,relative_name)
{
    char * 		full_address = 0;
    BOOL       		result;
    char * 		mycopy = 0;
    char * 		stripped = 0;
    char *		current_address =
    				HTAnchor_address((HTAnchor*)HTMainAnchor);

    StrAllocCopy(mycopy, relative_name);

    stripped = HTStrip(mycopy);
    full_address = HTParse(stripped,
	           current_address,
		   PARSE_ACCESS|PARSE_HOST|PARSE_PATH|PARSE_PUNCTUATION);
    result = HTLoadAbsolute(full_address, NO);
    free(full_address);
    free(current_address);
    return result;
}


/*		Load if necessary, and select an anchor
**		--------------------------------------
**
**    On Entry,
**        destination      	    The child or parenet anchor to be loaded.
**
**    On Exit,
**        returns    YES     Success
**                   NO      Failure 
**
*/

PUBLIC BOOL HTLoadAnchor ARGS1(HTAnchor *,destination)
{
    HTParentAnchor * parent;

    if (!destination) return NO;	/* No link */
    
    parent  = HTAnchor_parent(destination);
    
    if ( /* HTAnchor_document (parent) == NULL) { */ parent != HTMainAnchor)	{	/* If not already loaded */
        BOOL result;
        char * address = HTAnchor_address((HTAnchor*) parent);
	result = HTLoadDocument(parent, address, NO);
	free(address);
	if (!result) return NO;
    }
    
    if (destination != (HTAnchor *)parent)	/* If child anchor */
        HText_selectAnchor(HTMainText, 
		(HTChildAnchor*)destination); /* Double display? @@ */
		
    return YES;
	
} /* HTLoadAnchor */


#ifndef ERWISE
/*		Search
**		------
**  Performs a keyword search on word given by the user. Adds the keyword to 
**  the end of the current address and attempts to open the new address.
**
**  On Entry,
**       *keywords  	space-separated keyword list or similar search list
**	HTMainAnchor	global must be valid.
*/

PUBLIC BOOL HTSearch ARGS1(char *,keywords)

{
    char * p;	          /* pointer to first non-blank */
    char * q, *s;
    char * address = HTAnchor_address((HTAnchor*)HTMainAnchor);
    BOOL result;
    
    p = HTStrip(keywords);
    for (q=p; *q; q++)
        if (WHITE(*q)) {
	    *q = '+';
	}

    s=strchr(address, '?');		/* Find old search string */
    if (s) *s = 0;			        /* Chop old search off */

    StrAllocCat(address, "?");
    StrAllocCat(address, p);

    result = HTLoadRelative(address);
    free(address);
    return result;
    
}
#else /* ERWISE */

/*
 * Why everything is so hardcoded ???? 
 */

PUBLIC char *HTSearchAddress ARGS1(char *,keywords)
{
    char * p;             /* pointer to first non-blank */
    char * q, *s;
    char * address = HTAnchor_address((HTAnchor*)HTMainAnchor);
    char * current_address;
    char * mycopy;
    char * stripped;
    char * full_address;

    
    p = HTStrip(keywords);
    for (q=p; *q; q++)
        if (WHITE(*q)) {
            *q = '+';
        }

    s=strchr(address, '?');             /* Find old search string */
    if (s) *s = 0;                              /* Chop old search off */

    StrAllocCat(address, "?");
    StrAllocCat(address, p);

    StrAllocCopy(mycopy, address);
    
    current_address = HTAnchor_address((HTAnchor*)HTMainAnchor);

    stripped = HTStrip(mycopy);

    full_address = HTParse(stripped,
                   current_address,
                   PARSE_ACCESS|PARSE_HOST|PARSE_PATH|PARSE_PUNCTUATION);

    free(address);
    
    return full_address;
}

#endif /* ERWISE */

