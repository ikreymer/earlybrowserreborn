#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midas.h"
#include "xgifload.h"
#include "SGMLText.h"

#if defined(VMS) & !defined(UCX)  
extern int socket_read();
#else
#define socket_read read
#define socket_write(a,b,c) write(a,b,c)
#define socket_close(a) close(a)
#define socket_ioctl(a,b,c) ioctl(a,b,c)
#define socket_errno errno
#endif

/*
 * Completion codes
 */
#ifdef VMS
#define FAILURE 0
#define SUCCESS 1
#else
#define SUCCESS 0
#define FAILURE 1
#endif

static char *AbortMessage = "<NoCache><h1>Cancelled</h1>Document fetch aborted at user request.<tryagain>";

/*
 *  Structures
 */

typedef struct  {

  XrmQuark protocol;
  XrmQuark node;
  int      port;
  XrmQuark file;      
  XrmQuark anchor; 
  XrmQuark keyword;
  XrmQuark method;

} WWWFile;  

static WWWFile WWWFileDefault;

/*
 * A node structure is created for every node discovered on the web.
 * A node may be a link source (<a href=xxx>) or a destination (<a name=xxx>) 
 * or both.
 *
 * The elements of the structure are:
 *
 *   hashclash     Chains together nodes with the same hash value   
 *
 *   parent        For destination nodes that are contained within a document 
 *                   (ie <a name=???> this points to the parent document node,
 *                   for other node points to the node itself. 
 *
 *   next          Within a document, next and previous chain together the
 *   prev            source anchors embedded in it.
 *
 *   up            One way you could have got here
 *   down          The document you get to if you follow this link, for a document
 *                   that is its own parent this points to the first source in the 
 *                   document (or NULL if there are none).  
 *
 *   chain         Chains together nodes that have the same destination (down)
 *
 */

struct _WWWNode {

  struct     _WWWNode *hashclash; 
  
  struct     _WWWNode *parent;
  struct     _WWWNode *next;
  struct     _WWWNode *prev;
  struct     _WWWNode *up;
  struct     _WWWNode *down;
  struct     _WWWNode *chain;
  struct     _WWWNode *route;
  struct     _WWWNode *redirect; /* Node resulted in a document has moved reposnse */  

  char       *title;
  WWWFile    *file;
  Boolean    visited;

  Pixmap     icon;     /* Set if this node is actually a pixmap */ 
  GIFImage   *picture; /* Set if the node is actually an image */

  WidgetList widgets;  /* current list of widgets corresponding to this node */
  int num_widgets;
  int alloc_widgets; 

  int id;              /* used to prevent loops when scanning web */ 

};
typedef struct _WWWNode WWWNode;   

struct _WWWLink {
    
  WWWNode *from;
  WWWNode *to;    

};
typedef struct _WWWLink WWWLink;   

typedef struct {

  int          flag;
  Widget       parent;
  Widget       widget;
  WWWFile      *file;
  XtIntervalId id;
  Boolean      newMessage;
  char         message[1024];
  char         *help_code;
  Widget       draw;
  Dimension    height;
  Dimension    ascent;
  Dimension    width;
  int          read;   /* bytes read so far */
  int          length; /* total number of bytes to be read */
  int          halffull; /* how much of document fetched */           

} MessageBlock;

typedef int (*ReadRoutine)();

typedef struct {

  int     nleft;       /* Number of characters left in buffer */
  char    *next;       /* Next character in buffer            */
  int     *flag;       /* Interupt flag                       */
  int     read;        /* Bytes read                          */
  int     length;      /* Total bytes (0 if not known)        */       
  ReadRoutine readRoutine; 
                       /* routine to read block of input      */
  int     socket;      /* argument to read routine            */  
  Boolean eof;         /* Reached end-of-file                 */
  char    *buffer;     /* Pointer to buffer                   */
  int     bsize;       /* Size of buffer                      */      
  MessageBlock *mb;
  
} DataSource;  

typedef struct {

   WWWNode *Node;
   Widget  Widget;

} ImageLink; 

typedef struct {

    char      *Command;
    char      *IconName;
    Boolean   Binary;
    Boolean   Filter;
    Boolean   Tempfile;
    Boolean   Shellflag;
    XrmQuark  Destination;
    XtPointer UserData;

} WWWFiletype, *WWWFiletypePtr;

typedef struct {

    SGMLRendition       sensitive_rendition;
    SGMLRendition       visited_rendition;

    List		*ok_ports;

    int                 default_HTTP_port;
    int                 default_Gopher_port;
    int                 default_FTP_port;

    String              default_HTTP_protocol;
    String              help;
    String		version;
 
    XrmQuark            wais_gateway_node;
    int                 wais_gateway_port;                                      

    int                 waiting_time_out;
    int                 update_time_out;

    Atom		signpost;

    List                *filetypes;
    List		*mimetypes;
    List		*mimeencode;
    List		*usertypes;
    List		*usermimetypes;
    List		*usermimeencode;

    WWWFiletype         *ftpDefaultFiletype;
    WWWFiletype         *localDefaultFiletype;
    WWWFiletype         *httpDefaultFiletype;
    WWWFiletype         *gopherDefaultFiletype;
    WWWFiletype         *defaultFiletype;
  
    Pixel foreground;
    Pixel background;
    XFontStruct *font;

} WWWResources;

#ifndef WWWMASTER
extern
#endif
WWWResources appResources;

WWWFiletype *WWWGetFiletype();
Boolean WWWAskForFileDestination();
int TCPIPConnect();
int TCPIPWriteAndWaitForResponse();
Widget WWWLoadByFiletype();
Widget WWWFetchDocumentTELNET();
Widget WWWFetchDocumentHTTP();
void WWWFreeFiletype();
Widget WWWFetchDocumentLOCAL();
int GetCharacter();
