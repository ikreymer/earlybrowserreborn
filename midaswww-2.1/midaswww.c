
/*
 *  These are the WWW - Midas Interface routines
 */ 

#include <signal.h>
#define EF 23
#define WWWMASTER
#include "midasnet.h"
#include "SGMLHyper.h"
#include "SGMLCompositeText.h"
#include "SGMLListText.h"
#include "SGMLAnchorText.h"
#include "SGMLContainerText.h"
#include "SGMLMarkerText.h"
#include "SGMLFormattedText.h"
#include "SGMLPlainText.h"
#include "GhostviewPlus.h"
#include "ps.h"

#include <Xm/MessageB.h> 
#include <X11/Xatom.h>
#include <string.h>

#define HASHSIZE 337
#define DEADNODE ((WWWNode *) -1)

static WWWNode *HashTable[HASHSIZE];
static WWWFile *WWWGateway; 
static WWWNode *WWWToLink = NULL;
WWWNode *WWWFromLink = DEADNODE; 
static Pixmap circlePixmap; 

/* Ugly static variable that keeps track of where we are in processing a document */

static int WWWnextid = 0;
static WWWNode *WWWlastnode;
static WWWNode *WWWparent;
static GC messageGC;

static XrmQuark localProtocol;
static XrmQuark httpProtocol;
static XrmQuark gopherProtocol;
static XrmQuark telnetProtocol;
static XrmQuark ftpProtocol;
static XrmQuark fileProtocol;
static XrmQuark historyProtocol;
static XrmQuark ingotProtocol;
static XrmQuark waisProtocol;

static XrmQuark getMethod;
static XrmQuark postMethod;

XrmQuark htmlDestination;
XrmQuark textDestination;
XrmQuark psDestination;
XrmQuark gifDestination;
XrmQuark xbmDestination;
XrmQuark fileDestination;
XrmQuark multimediaDestination;
XrmQuark unsupportDestination;

extern XtAppContext appl_context;

static List *cleanUp = NULL;  
static List *cleanUpTempfile = NULL;
static WWWNode *FirstVisitedNode = NULL; 
static WWWNode *historyNode;
static WWWNode *YouAreHereNode = NULL;

char password[64];

extern Widget WWWFetchDocumentHTTP1();
extern Widget WWWPostDocumentHTTP1();
extern Widget WWWFetchDocumentHTTP();
extern Widget WWWFetchDocumentGOPHER();
extern Widget WWWFetchDocumentFTP();

/*
 * Application resources
 * 
 */

#define WWWINHERIT_FONT    NULL
#define WWWINHERIT_UNDERLINE -999
#define WWWINHERIT_OUTLINE 2
#define WWWINHERIT_UNDERLINEHEIGHT 999
#define WWWINHERIT_UNDERLINESTYLE 999
#define WWWINHERIT_COLOR NULL  
#define WWWINHERIT_QUARK NULL
#define WWWINHERIT_SIZE 0

#define WWWNvisitedFontSize        "visitedFontSize"
#define WWWNvisitedFontSpacing     "visitedFontSpacing"
#define WWWNvisitedFontWidth       "visitedFontWidth"
#define WWWNvisitedFontSlant       "visitedFontSlant"
#define WWWNvisitedFontWeight      "visitedFontWeight"
#define WWWNvisitedFontRegistry    "visitedFontRegistry"
#define WWWNvisitedFontFamily      "visitedFontFamily"

#define WWWNvisitedColor           "visitedColor"
#define WWWNvisitedUnderline       "visitedUnderline"
#define WWWNvisitedOutline         "visitedOutline"
#define WWWNvisitedUnderlineHeight "visitedUnderlineHeight"
#define WWWNvisitedUnderlineStyle  "visitedUnderlineStyle"

#define WWWNsensitiveFontSize        "sensitiveFontSize"
#define WWWNsensitiveFontSpacing     "sensitiveFontSpacing"
#define WWWNsensitiveFontWidth       "sensitiveFontWidth"
#define WWWNsensitiveFontSlant       "sensitiveFontSlant"
#define WWWNsensitiveFontWeight      "sensitiveFontWeight"
#define WWWNsensitiveFontRegistry    "sensitiveFontRegistry"
#define WWWNsensitiveFontFamily      "sensitiveFontFamily"

#define WWWNsensitiveColor           "sensitiveColor"
#define WWWNsensitiveUnderline       "sensitiveUnderline"
#define WWWNsensitiveOutline         "sensitiveOutline"
#define WWWNsensitiveUnderlineHeight "sensitiveUnderlineHeight"
#define WWWNsensitiveUnderlineStyle  "sensitiveUnderlineStyle"

#define WWWNwaitingTimeOut	   "waitingTimeOut"
#define WWWNupdateTimeOut	   "updateTimeOut" 
#define WWWCTimeOut                "Timeout"				

#define WWWNdefaultHTTPPort        "defaultHTTPPort"
#define WWWNdefaultHTTPProtocol    "defaultHTTPProtocol"
#define WWWNdefaultGopherPort      "defaultGopherPort"
#define WWWNdefaultFTPPort         "defaultFTPPort"
#define WWWCHTTPProtocol           "Protocol"
#define WWWCPort                   "Port"
#define WWWNhelp                   "help"
#define WWWCHelp                   "Help"  

#define WWWNsignpostName           "signpostName"
#define WWWCSignpostName           "SignpostName"

#define WWWNwaisGatewayName        "waisGatewayName"
#define WWWCWaisGatewayName        "WaisGatewayName"
#define WWWNwaisGatewayPort        "waisGatewayPort"
#define WWWCWaisGatewayPort        "WaisGatewayPort"

#define WWWNokPorts                "okPorts"
#define WWWCOkPorts                "OkPorts"

#define WWWNfiletypes		   "filetypes"
#define WWWCFiletypes		   "Filetypes"		

#define WWWNmimetypes              "mimetypes"
#define WWWCMimetypes              "Mimetypes"

#define WWWNmimeencode             "mimeencode"
#define WWWCMimeencode             "Mimeencode"

#define WWWNusertypes              "usertypes"
#define WWWCUsertypes              "Usertypes"

#define WWWNuserMimetypes          "userMimetypes"
#define WWWCUserMimetypes          "UserMimetypes"
                
#define WWWNuserMimeencode         "userMimeencode"
#define WWWCUserMimeencode         "UserMimeencode"

#define WWWNforeground             "foreground"
#define WWWCForeground             "Foreground"
         
#define WWWNbackground             "background"
#define WWWCBackground             "Background"
 
#define WWWNfont                   "font"
#define WWWCFont                   "Font"
 
#define Offset(field) XtOffsetOf(WWWResources,field) 

static XtResource resources[] = { 

    {WWWNvisitedFontFamily, SGMLCFontFamily, SGMLRQuark, sizeof(XrmQuark),
    Offset (visited_rendition.family), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNvisitedFontWeight, SGMLCFontWeight, SGMLRQuark, sizeof(XrmQuark),
    Offset (visited_rendition.weight), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNvisitedFontSlant, SGMLCFontSlant, SGMLRQuark, sizeof(XrmQuark),
    Offset (visited_rendition.slant), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNvisitedFontWidth, SGMLCFontWidth, SGMLRQuark, sizeof(XrmQuark),
    Offset (visited_rendition.width), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNvisitedFontSpacing, SGMLCFontSpacing, SGMLRQuark, sizeof(XrmQuark),
    Offset (visited_rendition.spacing), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNvisitedFontRegistry, SGMLCFontRegistry, SGMLRQuark, sizeof(XrmQuark),
    Offset (visited_rendition.registry), XtRImmediate, SGMLINHERIT_QUARK},

    {WWWNvisitedFontSize, SGMLCFontSize, XtRInt, sizeof(int),
    Offset (visited_rendition.size), XtRImmediate, SGMLINHERIT_SIZE},

    {WWWNvisitedColor, SGMLCColor, XtRPixel, sizeof (Pixel),
    Offset(visited_rendition.color),XtRString, WWWINHERIT_COLOR},

    {WWWNvisitedOutline,SGMLCOutline,XtRBoolean,sizeof(Boolean),
    Offset(visited_rendition.outline),XtRImmediate,(XtPointer) WWWINHERIT_OUTLINE},

    {WWWNvisitedUnderline,SGMLCUnderline,XtRInt,sizeof(int),
    Offset(visited_rendition.underline),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINE},

    {WWWNvisitedUnderlineHeight, SGMLCUnderlineHeight, XtRDimension, sizeof(Dimension),
    Offset(visited_rendition.underline_height),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINEHEIGHT},

    {WWWNvisitedUnderlineStyle, SGMLCUnderlineStyle, SGMLRLineStyle, sizeof(int),
    Offset(visited_rendition.underline_style),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINESTYLE},
 
    {WWWNsensitiveFontFamily, SGMLCFontFamily, SGMLRQuark, sizeof(XrmQuark),
    Offset (sensitive_rendition.family), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNsensitiveFontWeight, SGMLCFontWeight, SGMLRQuark, sizeof(XrmQuark),
    Offset (sensitive_rendition.weight), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNsensitiveFontSlant, SGMLCFontSlant, SGMLRQuark, sizeof(XrmQuark),
    Offset (sensitive_rendition.slant), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNsensitiveFontWidth, SGMLCFontWidth, SGMLRQuark, sizeof(XrmQuark),
    Offset (sensitive_rendition.width), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNsensitiveFontSpacing, SGMLCFontSpacing, SGMLRQuark, sizeof(XrmQuark),
    Offset (sensitive_rendition.spacing), XtRImmediate, SGMLINHERIT_QUARK},
 
    {WWWNsensitiveFontRegistry, SGMLCFontRegistry, SGMLRQuark, sizeof(XrmQuark),
    Offset (sensitive_rendition.registry), XtRImmediate, SGMLINHERIT_QUARK},

    {WWWNsensitiveFontSize, SGMLCFontSize, XtRInt, sizeof(int),
    Offset (sensitive_rendition.size), XtRImmediate, SGMLINHERIT_SIZE},

    {WWWNsensitiveColor, SGMLCColor, XtRPixel, sizeof (Pixel),
    Offset(sensitive_rendition.color),XtRString, WWWINHERIT_COLOR},

    {WWWNsensitiveOutline,SGMLCOutline,XtRBoolean,sizeof(Boolean),
    Offset(sensitive_rendition.outline),XtRImmediate,(XtPointer) WWWINHERIT_OUTLINE},

    {WWWNsensitiveUnderline,SGMLCUnderline,XtRInt,sizeof(int),
    Offset(sensitive_rendition.underline),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINE},

    {WWWNsensitiveUnderlineHeight, SGMLCUnderlineHeight, XtRDimension, sizeof(Dimension),
    Offset(sensitive_rendition.underline_height),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINEHEIGHT},

    {WWWNsensitiveUnderlineStyle, SGMLCUnderlineStyle, SGMLRLineStyle, sizeof(int),
    Offset(sensitive_rendition.underline_style),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINESTYLE},
  
    {WWWNdefaultHTTPPort, WWWCPort, XtRInt, sizeof(int),
    Offset(default_HTTP_port),XtRImmediate,(XtPointer) 80},

    {WWWNdefaultHTTPProtocol, WWWCHTTPProtocol, XtRString, sizeof(char *),
    Offset(default_HTTP_protocol),XtRString,(XtPointer) "0.9"},

    {WWWNdefaultGopherPort, WWWCPort, XtRInt, sizeof(int),
    Offset(default_Gopher_port),XtRImmediate,(XtPointer) 70},

    {WWWNdefaultFTPPort, WWWCPort, XtRInt, sizeof(int),
    Offset(default_FTP_port),XtRImmediate,(XtPointer) 21},

    {WWWNokPorts, WWWCOkPorts, "List", sizeof(List *),
    Offset(ok_ports),XtRString,(XtPointer) "21 70 71 80 81 82 83 84 85 79 13 43"},

    {WWWNhelp, WWWCHelp, XtRString, sizeof(char *),
    Offset(help),XtRString,(XtPointer) "http://slacvx.slac.stanford.edu:80/midasv20/"},

    {WWWNwaitingTimeOut, WWWCTimeOut, XtRInt, sizeof(int),
    Offset(waiting_time_out),XtRImmediate,(XtPointer) 2000},

    {WWWNupdateTimeOut, WWWCTimeOut, XtRInt, sizeof(int),
    Offset(update_time_out),XtRImmediate,(XtPointer) 200},

    {WWWNfiletypes, WWWCFiletypes, "List", sizeof(List *),
    Offset(filetypes),XtRImmediate,(XtPointer) NULL},  

    {WWWNmimetypes, WWWCMimetypes, "List", sizeof(List *),
    Offset(mimetypes),XtRImmediate,(XtPointer) NULL},  

    {WWWNmimeencode, WWWCMimeencode, "List", sizeof(List *),
    Offset(mimeencode),XtRImmediate,(XtPointer) NULL},  

    {WWWNusertypes, WWWCUsertypes, "List", sizeof(List *),
    Offset(usertypes),XtRImmediate,(XtPointer) NULL},  

    {WWWNuserMimetypes, WWWCUserMimetypes, "List", sizeof(List *),
    Offset(usermimetypes),XtRImmediate,(XtPointer) NULL},

    {WWWNuserMimeencode, WWWCUserMimeencode, "List", sizeof(List *),
    Offset(usermimeencode),XtRImmediate,(XtPointer) NULL},

    {WWWNsignpostName, WWWCSignpostName, XtRAtom, sizeof(Atom),
    Offset(signpost),XtRString,(XtPointer) "midaswww"},  

    {WWWNwaisGatewayName, WWWCWaisGatewayName, SGMLRQuark, sizeof(XrmQuark),
    Offset(wais_gateway_node),XtRString,(XtPointer) "info.cern.ch"},  

    {WWWNwaisGatewayPort, WWWCWaisGatewayPort, XtRInt, sizeof(int),
    Offset(wais_gateway_port),XtRImmediate,(XtPointer) 8001},       

    {WWWNforeground, WWWCForeground, XtRPixel, sizeof(Pixel),
    Offset(foreground),XtRImmediate,(XtPointer) 0},      

    {WWWNbackground, WWWCBackground, XtRPixel, sizeof(Pixel),
    Offset(background),XtRImmediate,(XtPointer) 1},        

    {WWWNfont, WWWCFont, XtRFontStruct, sizeof(XFontStruct *),
    Offset(font),XtRString,(XtPointer) "fixed"}, 
};    

#undef Offset

static WWWFile *ParseFile();
 
/*
 * Calculates a hash id from a WWWFile
 * -----------------------------------
 */
static int WWWHash(file)
WWWFile *file;
{
   return ((int) file->protocol + 
           (int) file->node     + 
           (int) file->file     + 
           (int) file->anchor   + 
           file->port           ) % HASHSIZE; 
}
char *WWWAsciiFile(pfile)
WWWFile *pfile;
{
  char *buffer, *p;

  int l = 10 + strlen(XrmQuarkToString(pfile->protocol));
  if (pfile->file)   l += strlen(XrmQuarkToString(pfile->file));
  if (pfile->node)   l += strlen(XrmQuarkToString(pfile->node));
  if (pfile->port)   l += 6;
  if (pfile->anchor) l += strlen(XrmQuarkToString(pfile->anchor));

  p = buffer = XtMalloc(l);

  sprintf(buffer,"%s:",XrmQuarkToString(pfile->protocol));
  buffer += strlen(buffer);
  
  if (pfile->node)
    {
      sprintf(buffer,"//%s",XrmQuarkToString(pfile->node));
      buffer += strlen(buffer);
    }  

  if (pfile->port) 
    {  
      sprintf(buffer,":%d",pfile->port);
      buffer += strlen(buffer);
    }
  
  if (pfile->file)
    {    
      sprintf(buffer,"%s",XrmQuarkToString(pfile->file));
      buffer += strlen(buffer);
    }

  if (pfile->anchor)
    {
      sprintf(buffer,"#%s",XrmQuarkToString(pfile->anchor));
      buffer += strlen(buffer);
    }
  return p;
}

/*
 * Called to mark an anchor as visited or sensitive
 */
static void WWWBeenThere(w,visited)
Widget w; 
Boolean visited;
{
  Arg arglist[20];
  int n=0;
  SGMLRendition *rendition;
  
  if (visited) rendition = &appResources.visited_rendition;
  else         rendition = &appResources.sensitive_rendition;

  if (rendition->color != WWWINHERIT_COLOR) 
  {
    XtSetArg(arglist[n],SGMLNcolor,rendition->color); n++;
  }
  if (rendition->family != WWWINHERIT_QUARK) 
  {
    XtSetArg(arglist[n],SGMLNfontFamily,rendition->family); n++;
  }
   if (rendition->slant != WWWINHERIT_QUARK) 
  {
    XtSetArg(arglist[n],SGMLNfontSlant,rendition->slant); n++;
  }
   if (rendition->width != WWWINHERIT_QUARK) 
  {
    XtSetArg(arglist[n],SGMLNfontWidth,rendition->width); n++;
  }
   if (rendition->registry != WWWINHERIT_QUARK) 
  {
    XtSetArg(arglist[n],SGMLNfontRegistry,rendition->registry); n++;
  }
   if (rendition->weight != WWWINHERIT_QUARK) 
  {
    XtSetArg(arglist[n],SGMLNfontWeight,rendition->weight); n++;
  }
    if (rendition->spacing != WWWINHERIT_QUARK) 
  {
    XtSetArg(arglist[n],SGMLNfontSpacing,rendition->spacing); n++;
  }
   if (rendition->size != WWWINHERIT_SIZE) 
  {
    XtSetArg(arglist[n],SGMLNfontSize,rendition->size); n++;
  }
   if (rendition->outline != WWWINHERIT_OUTLINE) 
  {
    XtSetArg(arglist[n],SGMLNoutline,rendition->outline); n++;
  }
  if (rendition->underline != WWWINHERIT_UNDERLINE) 
  {
    XtSetArg(arglist[n],SGMLNunderline,rendition->underline); n++;
  }
  if (rendition->underline_height != WWWINHERIT_UNDERLINEHEIGHT) 
  {
    XtSetArg(arglist[n],SGMLNunderlineHeight,rendition->underline_height); n++;
  }    
  if (rendition->underline_style != WWWINHERIT_UNDERLINESTYLE) 
  {
    XtSetArg(arglist[n],SGMLNunderlineStyle,rendition->underline_style); n++;
  }
  if (n) XtSetValues(w,arglist,n); 
}
/*
 *  Delete a file structure
 *  -----------------------
 */
void FreeFile(file)
WWWFile *file;
{
  XtFree((char *)file);
}
/*
 *  Create a copy of a file structure
 *  ---------------------------------
 */
static WWWFile *CopyFile(file)
WWWFile *file;
{
  WWWFile *new = XtNew(WWWFile);
  
  *new = *file;
  return new; 
}
static int CompareFile(file1,file2)
WWWFile *file1;
WWWFile *file2;
{
  return memcmp(file1,file2,sizeof(WWWFile));
}
/*
 * Find a node
 * -----------------------
 */ 

static WWWNode *WWWFindNode(file)
WWWFile *file;
{
  int hashid = WWWHash(file);
  WWWNode *a = HashTable[hashid]; 
 
  for (; a != NULL; a = a->hashclash) if (!CompareFile(a->file,file)) return a;
 
  return NULL;
}
/*
 * Find (or create) a node
 * -----------------------
 */ 

static WWWNode *WWWCreateNode(file,parent)
WWWFile *file;
WWWNode *parent;
{
  int hashid;
  WWWNode *a = WWWFindNode(file);
 
  if (!a)
    {
      int hashid  = WWWHash(file);

      a = XtNew(WWWNode);

      a->file = CopyFile(file);
      a->title  = "Untitled";

      a->parent = parent ? parent : a;
      a->prev = NULL; 
      a->next = NULL;
      a->up   = NULL;
      a->down = NULL;
      a->chain= NULL;  
      a->route= NULL;
      a->visited  = FALSE;
      a->redirect = NULL;

      a->icon  = NULL;
      a->picture = NULL;

      a->widgets = NULL;
      a->num_widgets = 0;
      a->alloc_widgets = 0;
      a->id = 0;
   
      a->hashclash = HashTable[hashid];
      HashTable[hashid] = a;
    }
  return a; 
}
/*
 * Find (or create) a node and if necessary its parent
 * ---------------------------------------------------
 */ 
static WWWNode *WWWCreateNodeAndParent(file)
WWWFile *file;
{
   WWWNode *parent;

   if (file->anchor) 
     {
       XrmQuark temp = file->anchor;
       file->anchor = NULL;      
       parent = WWWCreateNode(file,NULL);
       file->anchor = temp;
     }
   else parent = NULL;
       
   return WWWCreateNode(file,parent);
}
/*
 * Locate a visited node in the history tree
 */
static Widget LocateVisitedNode(w,node)
Widget w;
WWWNode *node;
{
  Arg arglist[1];
  char *href;
  WWWFile *file;
  int result = 1;

  XtSetArg(arglist[0],SGMLNhref,&href);
  XtGetValues(w,arglist,1);

  if (href && *href)
    {  
      file = ParseFile(href,&WWWFileDefault);
      result = CompareFile(file,node->file);
      FreeFile(file);
    }
  return result ? NULL : w;
}
static Widget InsertBeforeWidget;
static int InsertBefore(w)
Widget w;
{
  Arg arglist[2];
  Cardinal num_children, p;
  WidgetList children;
           
  XtSetArg(arglist[0],XtNnumChildren,&num_children);
  XtSetArg(arglist[1],XtNchildren,&children);
  XtGetValues(XtParent(w),arglist,2);

  for ( p = 0 ; p < num_children ; p++) if (*children++ == InsertBeforeWidget) return p;

  return num_children;
}
/*
 * If the history node is visible, then add the new node to 
 * the history display.
 */ 

static void WWWUpdateHistory(node)
WWWNode *node;
{
  Arg arglist[2];
  int i; 

  node = node->parent;
  if (node == historyNode) return;  
  if (node == YouAreHereNode) return;
     
  /*
   * Get rid of old you are here marker
   */

  for (i = 0 ; i < historyNode->num_widgets ; i++)
    {
      Widget c, w = historyNode->widgets[i];
      if (w->core.being_destroyed) continue;
      c = MidasTraceWidgetTree(w,"...img");
      if (c) XtDestroyWidget(c); 
    }

  YouAreHereNode = node;

  if (!node->visited)
    {
     for (i = 0 ; i < historyNode->num_widgets ; i++)
        {
          Widget w = historyNode->widgets[i];
          Widget r, ul = NULL;

          /*
           * Start by trying to find the node's parent (if it has one) in the
           * history tree.  
           */

          if (node->up)
            {
              r = MidasScanWidgetTree(w,"...a",LocateVisitedNode,
                                        (XtPointer) node->up->parent); 
            }
          else r = NULL; 

          /*
           * If so the scan to find if the parent node already has a list of  
           * children. If not create one.
           */ 

          if (r) 
            {
              Widget parent = XtParent(r);
              int j, num_children;
              WidgetList children;
           
              XtSetArg(arglist[0],XtNnumChildren,&num_children);
              XtSetArg(arglist[1],XtNchildren,&children);
              XtGetValues(parent,arglist,2);

              for (j=0; children[j++] != r; );
              for (; j < num_children ; j++ )
                if (!strcmp(XtName(children[j]),"ul")) { ul = children[j]; break; }

              if (!ul)
                {
                  XtSetArg(arglist[0],SGMLNbulletType,SGMLBULLET_LINKEDARROW);
                  ul = XtCreateWidget("ul",sGMLListTextObjectClass,parent,arglist,1);
                  MidasSetupWidget(ul);
                  SGMLCompositeTextInsertChild(ul);
                }
            }
          else
            {
              ul = MidasTraceWidgetTree(w,"...ul");
            }
           /*
            * Now create the entry in the list for the new node.
            */
            {
              Widget m,a,q,c,h; 
              char *filename = WWWAsciiFile(node->file);
              
              WWWparent = historyNode;
              WWWlastnode = historyNode->down;
 
              m = SGMLCreateWidget("li",sGMLMarkerTextObjectClass,ul,NULL,0);

              XtSetArg(arglist[0],SGMLNpixmap,circlePixmap);
              c = SGMLCreateWidget("img",sGMLMarkerTextObjectClass,ul,arglist,1);
 
              XtSetArg(arglist[0],SGMLNhref,filename);
              a = SGMLCreateWidget("a",sGMLAnchorTextObjectClass,ul,arglist,1);
 
              XtSetArg(arglist[0],SGMLNtext,node->title);
              q = SGMLCreateWidget("",sGMLFormattedTextObjectClass,a,arglist,1);

              for (h = ul; !SGMLIsHyper(h) ; h = XtParent(h)); 
              SGMLHyperShowObject(h,c,FALSE);
              XtFree(filename);
            }
        } 
    }
  else /* node is already visited */
    {
      for (i = 0 ; i < historyNode->num_widgets ; i++)
        {
          Widget w = historyNode->widgets[i];
          Widget r, h;

          /*
           * Find the node 
           */

          r = MidasScanWidgetTree(w,"...a",LocateVisitedNode,(XtPointer) node); 
          if (r)
            {
              Widget c;
              InsertBeforeWidget = r;
              XtSetArg(arglist[0],XtNinsertPosition,InsertBefore);              
              XtSetValues(XtParent(r),arglist,1);
 
              XtSetArg(arglist[0],SGMLNpixmap,circlePixmap);
              c = SGMLCreateWidget("img",sGMLMarkerTextObjectClass,XtParent(r),arglist,1);

              XtSetArg(arglist[0],XtNinsertPosition,NULL);              
              XtSetValues(XtParent(r),arglist,1);
 
              for (h = r; !SGMLIsHyper(h) ; h = XtParent(h)); 
              SGMLHyperShowObject(h,c,FALSE);
           }
        }
    }
}
/*
 * Reset all visited node
 * ----------------------
 */
static void WWWReset()
{
  int i, nw;
  WidgetList w;
  WWWNode *n, *m;

  for (i=0;i<HASHSIZE;i++)
    {
      m = HashTable[i];  
      while (m) 
        {
          if (m->visited)
            {
              m->visited = FALSE;
              n = m->up;
              while(n)
              {
                w = n->widgets;
                nw = n->num_widgets;
                for (; nw-- > 0; w++) WWWBeenThere(*w,FALSE);
                n = n->chain;
              }
            }
          m = m->hashclash;
        } /* while */
    }    /* for */
}
/*
 * Search a given string
 */
static void WWWSearch(w,exp,sensitive,cont)
Widget w;
char *exp;
Boolean sensitive;
Boolean cont;
{
  Widget result;

  if (*exp == '\0') 
    {
      MidasBeep(50,w);
      return;
    } 
  result = SGMLHyperSearch((SGMLHyperWidget)w,exp,sensitive,cont);
  if (result)
   SGMLHyperShowObject((SGMLHyperWidget)w,result,TRUE);
  else MidasBeep(50,w);
}

/*
 * Mark a node as visited
 * ----------------------
 */
static void WWWVisitNode(node)
WWWNode *node;
{
  static WWWNode *LastVisitedNode = NULL; 

  if (!node->visited)
    {
      WWWNode *src = node->up;  

      for ( ; src != NULL; src = src->chain)
        {
           WidgetList w = src->widgets;
           int nw = src->num_widgets; 
           for (; nw-- > 0; w++) WWWBeenThere(*w,TRUE);
        }
      node->visited = TRUE;

      if (LastVisitedNode) LastVisitedNode->route = node;
      else                 FirstVisitedNode = node; 
      LastVisitedNode = node;
    }
}
static void WWWDetachWidgetFromNode(w,node)
Widget w;
WWWNode *node;
{
  int i;
  for (i=0 ; i < node->num_widgets; i++)
    if (node->widgets[i] == w) node->widgets[i] = node->widgets[--node->num_widgets];
}
static void WWWAttachWidgetToNode(node,w)
WWWNode *node;
Widget w;
{
   if (node->num_widgets == node->alloc_widgets) 
     node->widgets = (Widget *) XtRealloc((char *) node->widgets, (node->alloc_widgets += 5) * sizeof(Widget));
   node->widgets[node->num_widgets++] = w;
   XtAddCallback(w,XtNdestroyCallback,(XtCallbackProc) WWWDetachWidgetFromNode,(XtPointer)node);
}
/*
 * Initialize a DataSource structure
 * ---------------------------------
 */
void WWWInitDataSource(data,mb,buffer,bsize,readRoutine,s)
DataSource *data;
MessageBlock *mb;
char    *buffer;
int     bsize;
ReadRoutine readRoutine;
int     s;
{
  data->nleft = 0;
  data->flag = &mb->flag;
  data->read = 0;
  data->length = 0; 
  data->eof = FALSE;
  data->mb = mb;
  data->buffer = buffer;
  data->bsize = bsize; 
  data->socket = s;
  data->readRoutine = readRoutine; 
}

/*
 * Provides buffering for the TCP/IP packets
 * -----------------------------------------
 */
void ClearFlag(flag)
int *flag;
{
  *flag = 0;
}
int GetCharacter(data)
DataSource *data;
{
  XtInputMask mask; 

  if (data->eof) return EOF;
  if (data->nleft == 0)
    {
      /*
       * Check if there are any events to process
       */

#ifdef MULTINET 
      int rc;
      short iosb[4];
      XtInputId id;

      if (*data->flag == 1) goto UserAbort;  
      if (data->readRoutine == socket_read)
        {
          rc = SYS$QIO(EF,data->socket,IO$_RECEIVE,iosb,0,0,data->buffer,data->bsize,0,0,0,0);
          id = XtAppAddInput(appl_context,EF,(XtPointer) iosb,
                             (XtInputCallbackProc) ClearFlag,(XtPointer) data->flag);

          for (*data->flag = -1; *data->flag < 0;)
            {
              MidasFetchDispatchEvent();
            }
          XtRemoveInput(id);       

          if (*data->flag == 1)
            {
              Sys$Cancel(data->socket);
              goto UserAbort;
            }
          data->nleft = iosb[1];
        }
      else
        {
          data->nleft = (data->readRoutine)(data->socket,data->buffer,data->bsize);
          if (data->nleft < 0) printf("read error %s\n",strerror(errno));
        } 
#else
#ifdef ADDINPUTOK
      XtInputId id;
      if (*data->flag == 1) goto UserAbort;  
      id = XtAppAddInput(appl_context,data->socket,(XtPointer)XtInputReadMask,
                         (XtInputCallbackProc) ClearFlag,(XtPointer) data->flag);

      for (*data->flag = -1; *data->flag < 0;)
        {
          MidasFetchDispatchEvent();
        } 
      XtRemoveInput(id);       

      if (*data->flag == 1) goto UserAbort;

#endif
      data->nleft = (data->readRoutine)(data->socket,data->buffer,data->bsize);
      if (data->nleft < 0) printf("read error %s\n",strerror(errno));
#endif
      data->next = data->buffer;
      data->read += data->nleft;
      data->mb->newMessage = TRUE;
      data->mb->length = data->length;
      data->mb->read   = data->read;
      *data->mb->message = '@'; 
      while (XtAppPending(appl_context)) MidasFetchDispatchEvent();
      if (*data->flag == 1) goto UserAbort;
    }
  if (data->nleft-- > 0) return *(data->next++); 
  data->eof = TRUE;
  return EOF;

UserAbort:

  data->eof = TRUE;
  strcpy(data->mb->message,AbortMessage);
  return EOF; 
}
WWWFiletype *WWWGetFiletype(filename,defaultType)
char *filename;
WWWFiletype *defaultType;
/*
 * Given a filename, return a filetype structure based on the file's extension.
 * The returned structure must be freed using WWWFreeFiletype
 */
{
  char *copy; 
  WWWFiletype *result = XtNew(WWWFiletype);
  int i;

  if (filename) 
    {
      copy = strrchr(filename,'/');
      if (copy) copy = XtNewString(copy+1);
      else      copy = XtNewString(filename);
    }
  else copy = "";

  for (i=0; ; i++)
    {
      ListItem *item;
      WWWFiletype *temp;
      char *ext = strrchr(copy,'.');

      if (ext) 
        {

          /* Note: we first try a case sensitive search, but if that fails we convert
           *       the file extension to lowercase and try again.
           */

          ext++;
          item = MidasFindItemInList(appResources.filetypes,ext);
          if (!item)
            { 
              char *p;
              for (p = ext; *p; p++) *p = tolower(*p);
              item = MidasFindItemInList(appResources.filetypes,ext);
            }
        } 
      else item = NULL;

      if (item)  temp = (WWWFiletype *) item->Pointer;
      else       temp = defaultType;

      if (i == 0) 
        {
          *result = *temp;
          result->Command = XtNewString(temp->Command);
        }
      else 
        {
          result->IconName = temp->IconName;  
          result->Destination = temp->Destination;
          if (temp->Command)
            {
              result->Command = XtRealloc(result->Command, 
                                strlen(result->Command)+strlen(temp->Command)+1);
              strcat(result->Command,temp->Command);
            } 
        }
      if (!ext || !temp->Filter) break;
      *--ext = '\0';
    } 
  
  result->UserData = copy; 

  return result;
}
void WWWFreeFiletype(old)
WWWFiletype *old;
{
  XtFree(old->Command);
  XtFree(old->UserData);
  XtFree((char *)old);
}
static void DoCleanUp()
{
  ListItem *item;
  
  for (item = cleanUp->First; item ; item = item->Next)
    {
      unlink(item->Entry);
      GhostviewDisableInterpreter((Widget) item->Pointer);
    } 
}
static void DestroyGhostview(w,item)
Widget w;
ListItem *item;
{
  unlink(item->Entry);
  item->Pointer = NULL;
  MidasRemoveItemFromList(cleanUp,item);
}
static void MapGhostview(w,gv)
Widget w;
Widget gv;
{
   Arg arglist[2];
   int n = 0; 
   int pages;

   XtSetArg(arglist[n],XtNpages,&pages); n++;
   XtGetValues(gv,arglist,n); n = 0;

   XtSetArg(arglist[n],XtNuseBackingPixmap,TRUE); n++;
   if (pages == 1) { XtSetArg(arglist[n],XtNcurrentPage,&pages); n++; }
   XtSetValues(gv,arglist,n); n = 0;
  
   if (pages <= 0) GhostviewEnableInterpreter(gv);  
}   
static void UnmapGhostview(w,gv)
Widget w;
Widget gv;
{
   Arg arglist[2];
   int n = 0;

   GhostviewDisableInterpreter(gv);
 
   XtSetArg(arglist[n],XtNuseBackingPixmap,FALSE); n++;
   XtSetArg(arglist[n],XtNcurrentPage,0);  n++; 
   XtSetValues(gv,arglist,n);

   if (XtHasCallbacks(w,SGMLNmapCallback) == XtCallbackHasNone) 
     XtAddCallback(w,SGMLNmapCallback,(XtCallbackProc) MapGhostview,(XtPointer) gv);
}  
static void TrapGhostviewOutput(w,errors,buf)
Widget w;
char *buf;
Widget errors;
{
  Widget ge = MidasTraceWidgetTree(w,"^^^WWWMain###WWWGhostviewErrors");
  Widget text = MidasTraceWidgetTree(ge,"...WWWGhostscriptErrorText");

#ifndef DEBUG
  if (strstr(buf,"Warning:")) return; 
#endif   

  XmTextInsert(text,XmTextGetLastPosition(text),buf);
  MidasPopup(ge); 
}
static void TrapGhostviewMessage(w,errors,buf)
Widget w;
char *buf;
Widget errors;
{
   int n = 0 , pages;
   Arg arglist[10];

   XtSetArg(arglist[n],XtNpages,&pages); n++;
   XtGetValues(w,arglist,n);
   
   if (!strcmp(buf,"Page")) 
     {
       if (pages == 0) MidasQueueCommand(w,"Set Sensitive .^^^WWWMain...Next_Page True");
     }
   else if (!strcmp(buf,"Done"))
     {
       if (pages == 0) MidasQueueCommand(w,"Set Sensitive .^^^WWWMain...Next_Page False");
     }
   else TrapGhostviewOutput(w,errors,buf); 
}
Widget WWWLoadPS(w,mb,GetChar,data)
Widget w;
MessageBlock *mb;
int (*GetChar)();
DataSource *data;
{
  Widget result, gv, container;
  Arg arglist[10];
  int n=0;
  int c;
#ifndef VMS
  char buffer[L_tmpnam]; 
  char *temp = tmpnam(buffer); 
  FILE *tmp = fopen(temp,"w");
#else
  char buffer[256];
  FILE *tmp = fopen("sys$scratch:midaswww.tmp;","w");
  char *temp = fgetname(tmp,buffer);
#endif  
  int pages;
  struct document *doc;
  ListItem *item;
   
  if (!tmp)
    {
      sprintf(mb->message,"Error creating temporary file: %s\n",strerror(errno));
      mb->help_code = "temp_create_failed";
      return NULL;
    }
  /*
   * Some old http servers prepend <plaintext> to .ps documents
   * so scan for first % to start real postscript doc 
   */ 
  while ((c = GetChar(data)) != '%' && c != EOF);  
  if (c == EOF)
    {
      strcpy(mb->message,"No leading % found in .ps file");
      mb->help_code = "bad_ps";
      return NULL;
   }  
 
  fputc('%',tmp);
  while ((c = GetChar(data)) != EOF && putc(c,tmp) != EOF);
  if (ferror(tmp))
    {
      sprintf(mb->message,"Error writing temporary file: %s\n",strerror(errno));
      mb->help_code = "temp_write_failed";
      unlink(temp);
      fclose(tmp);
      return NULL;
    }
  fclose(tmp);

  if (!cleanUp)
    {
      cleanUp = MidasCreateEmptyList("cleanup");
      atexit(DoCleanUp);
    }
  item = MidasAddItemToList(cleanUp,temp);

  result = XtCreateWidget("gv_manager",sGMLCompositeTextObjectClass,w,NULL,0);

  XtSetArg(arglist[n],XtNfile,temp); n++;
  gv = XtCreateWidget("ghostview",ghostviewPlusWidgetClass,w,arglist,n);
  item->Pointer = (XtPointer) gv;  

  container = MidasTraceWidgetTree(result,".container");

  XtAddCallback(gv,XtNoutputCallback,(XtCallbackProc) TrapGhostviewOutput,(XtPointer) NULL);
  XtAddCallback(gv,XtNmessageCallback,(XtCallbackProc) TrapGhostviewMessage,(XtPointer) NULL);
  XtAddCallback(gv,XtNdestroyCallback,(XtCallbackProc) DestroyGhostview,(XtPointer) item);
  XtAddCallback(result,SGMLNunmapCallback,(XtCallbackProc) UnmapGhostview,(XtPointer) gv);
  MidasSetupWidget(result);

  n = 0;
  XtSetArg(arglist[n],XtNpages,&pages); n++;
  XtSetArg(arglist[n],XtNdoc,&doc); n++;
  XtGetValues(gv,arglist,n);

  if (pages>1)
    {
      WWWFile *here = CopyFile(data->mb->file); 
      Widget child;
      char *buffer = XtMalloc(pages*40+400);
      char *b = buffer;
      int i, page;
      struct page *p = doc->pages;
 
      sprintf(b,"This is a multipage Postscript document, select page:<ul>\n");
      b += strlen(b);
      
      for (page=1; page<=pages; page++, p++)
        {
          WWWNode *src;
          char temp[8];
          if (doc->pageorder == DESCEND) i = pages - page + 1;
          else                           i = page;
          
          if (strcmp(p->label,"?")) sprintf(b,"<li><a href=\"#%d\">Page %s</a>\n",i,p->label);
          else                      sprintf(b,"<li><a href=\"#%d\">Page %d</a>\n",i,i);
          b += strlen(b);
          
          sprintf(temp,"%d",i);
          here->anchor = XrmStringToQuark(temp); 
          
          src = WWWCreateNode(here,WWWparent);

          if (WWWlastnode) WWWlastnode->next = src; 
          src->prev = WWWlastnode;
          WWWlastnode = src; 
          WWWAttachWidgetToNode(src,container);
        }  
      FreeFile(here);

      sprintf(b,"</ul><h2>Note</h2>Once in the document you can use the <b>Next</b> and <b>Previous</b> buttons to go to ");
      b += strlen(b);   
      sprintf(b,"adjacent pages, and the <b>Go Back</b> button to return to this index.");      

      child = SGMLHyperSetText(result,buffer);
      SGMLCompositeTextInsertChild(child);
      XtFree(buffer);
    }     
  else if (pages == 1)
    {
      n = 0;
      XtSetArg(arglist[n],XtNcurrentPage,1); n++;
      XtSetValues(gv,arglist,n);
    }  

  return result; 
}
static Widget WWWLoadFile(w,mb,GetChar,data,filetype)
MessageBlock *mb;
Widget w;
int (*GetChar)();
WWWFiletype *filetype;
{
  int c;
  FILE *out;
  char *fname = filetype->UserData;

  out = fopen(fname,"w");
  if (out==0)
    {
      sprintf(mb->message,"Cannot open file %s",fname);
      mb->help_code = "file_open_failed";
      return NULL;
    }

  while ((c = GetChar(data)) != EOF && putc(c,out) != EOF);
  if (ferror(out))
    {
      sprintf(mb->message,"Error writing file %s: %s\n",fname,strerror(errno));
      mb->help_code = "file_write_failed";
      unlink(fname);
    }
  else sprintf(mb->message,"File saved as <b>%s</b>",fname);  

  fclose(out);
  return NULL; 
}
static void DestroyTempfile(w,i)
Widget w;
ListItem *i;
{
  if (i->Entry != NULL)
    unlink(i->Entry);
  MidasRemoveItemFromList(cleanUpTempfile,i);
}
static void DoCleanUpTempfile()
{
  ListItem *item;

  for (item = cleanUpTempfile->First; item; item = item->Next)
    {
      if(item->Entry == NULL)
        continue;
      unlink(item->Entry);
    }
}
static void ReLoadMultimedia(w,command)
Widget w;
char *command;
{
#ifdef VMS
  int flags = 1; /* nowait */
  int iss;
  struct {
     unsigned short len;
     unsigned char  type;
     unsigned char  class;
     char           *ptr;
   } command_D = {0,14,1,0};

  command_D.len = strlen(command);
  command_D.ptr = command;
  iss = Lib$Spawn(&command_D,&NULL,&NULL,&flags);
#else
  system(command);
#endif
}
static Widget WWWLoadMultimedia(w,mb,filetype,tmpf)
Widget w;
MessageBlock *mb;
WWWFiletype *filetype;
char *tmpf;
{
  char *buff = XtMalloc(128);
  char *name = XrmQuarkToString(mb->file->file); 
  char *command = filetype->Command;
  char *dest = XrmQuarkToString(filetype->Destination); 
  Widget result;
  Widget anchor;
  XtPointer state = SGMLHyperOpen(w);
  char temp[256];
  ListItem *item;
#ifdef VMS
  int flags = 1; /* nowait */
  int iss;
  struct {
     unsigned short len;
     unsigned char  type;
     unsigned char  class;
     char           *ptr;
   } command_D = {0,14,1,0};

  sprintf(buff,"%s %s",command,tmpf);
#else
  sprintf(buff,"%s %s &",command,tmpf);
#endif

  sprintf(temp,"<title>%s %s</title><h1><img pixmap=Movie>Multimedia<img pixmap=Sound></h1>%s <b>%s</b> file %s. ",command,name,command,dest,name); 
  SGMLHyperWrite(state,temp);
  sprintf(temp,"<p>Click <a href=\"#null\"><img pixmap=Movie></a>to Play again"); 
 
  SGMLHyperWrite(state,temp);
  result = SGMLHyperClose(state);

  if (!cleanUpTempfile)
    {
      cleanUpTempfile = MidasCreateEmptyList("cleanuptempfile");
      atexit(DoCleanUpTempfile);
    }
  if (filetype->Tempfile)
    item = MidasAddItemToList(cleanUpTempfile,tmpf);
  else
    item = MidasAddItemToList(cleanUpTempfile,NULL);
  item->Pointer = buff;
  XtAddCallback(result,XtNdestroyCallback,(XtCallbackProc) DestroyTempfile,(XtPointer)item); 

  anchor = MidasTraceWidgetTree(result,"...a."); 
  XtAddCallback(anchor,SGMLNactivateCallback,(XtCallbackProc) ReLoadMultimedia,buff);
#ifdef VMS
  command_D.len = strlen(buff);
  command_D.ptr = buff;
  iss = Lib$Spawn(&command_D,&NULL,&NULL,&flags);
  if (iss != 1)
    {
      sprintf(mb->message,"Error creating subprocess for playing multimedia");
      mb->help_code = "vms_multimedia_error";
      return NULL;
    } 
#else
  system(buff);
#endif
  return result; 
}

static Widget WWWLoadXBM(w,mb,GetChar,data)
MessageBlock *mb;
Widget w;
int (*GetChar)();
XtPointer data;
{
  int c, xhot, yhot, iss;  
  unsigned int width, height;
  Pixmap bitmap;
  Widget result;
  char *temp;
  

  if (!WWWWriteToTempfile(mb,GetChar,data,&temp,NULL)) return NULL;

  iss = XReadBitmapFile(XtDisplay(w),XtWindow(w),temp,&width,&height,&bitmap,&xhot,&yhot);
  if (iss != BitmapSuccess)
    {
      strcpy(mb->message,"Bitmap read failed");
      mb->help_code = "bitmap_read_failed";
      result = NULL; 
    } 
  else
   {
      result = (Widget) bitmap;
      mb->help_code = "pixmap";
   } 
  unlink(temp);   
  XtFree(temp);
  return result;
}
Widget WWWLoadGIF(w,mb,GetChar,data)
MessageBlock *mb;
Widget w;
int (*GetChar)();
XtPointer data;
{
  char buffer[256];
  Widget result;
  GIFImage *gifImage = GIFLoadFile(w,buffer,GetChar,data);

  if (gifImage) 
    {
      result = (Widget) gifImage;
      mb->help_code = "image";
    }
  else 
    {    
      sprintf(mb->message,"Error loading GIF file: %s<p>",buffer);
      mb->help_code = "gif_load_error";
      result = NULL;
    }

  return result;
}
static int WWWBuildFile(data)
DataSource *data;
{
  static char prefix[] = "<plaintext>\n";

  if (data->nleft == 0)
    {
      if (data->read == 0)
        {
          data->next = prefix;
          data->nleft = strlen(prefix);
          data->read += data->nleft;
        }  
      else
       {
          return GetCharacter(data);
       }
    }   
  if (data->nleft-- > 0) return *(data->next++); 
  data->eof = TRUE; 
  return EOF;
}
int WWWWriteToTempfile(mb,GetChar,data,tmp,ext)
MessageBlock *mb;
int (*GetChar)();
DataSource *data;
char **tmp;
char *ext;
{
  int c;
#ifndef VMS
  char buffer[L_tmpnam];
  char *tmpn = tmpnam(buffer);
  FILE *tmpfd;
  char *temp = XtMalloc(strlen(tmpn)+ (ext?strlen(ext):0) + 2);

  strcpy(temp,tmpn);
  if (ext)
     strcat(temp,ext);
  tmpfd = fopen(temp,"w");
#else
  char buffer[256];
  char *s = "sys$scratch:midaswww";
  char *tmpn = XtMalloc(strlen(s) + (ext?strlen(ext):4) + 2);
  FILE *tmpfd;
  char *temp;

  strcpy(tmpn,s);
  if (ext)
    {
      strcat(tmpn,ext);
      strcat(tmpn,";");
    }
  else
    strcat(tmpn,".tmp;");

  tmpfd = fopen(tmpn,"w");
  XtFree(tmpn); 
  temp = XtNewString(fgetname(tmpfd,buffer));
#endif

  if(!tmpfd)
    {
      sprintf(mb->message,"Error creating temporary file: %s\n",strerror(errno));
      mb->help_code = "temp_create_failed";
      return FALSE;
    }

  for ( c = GetChar(data); !data->eof ; c = GetChar(data))  putc(c,tmpfd); 
  if (ferror(tmpfd))
    {
      sprintf(mb->message,"Error writing temporary file: %s\n",strerror(errno));
      mb->help_code = "temp_write_failed";
      unlink(temp);
      fclose(tmpfd);
      return FALSE;
    }
  fclose(tmpfd);
  *tmp = temp;
  return TRUE;
}

Widget WWWLoadByFiletype(w,mb,filetype,GetChar,data)
Widget w;
MessageBlock *mb;
WWWFiletype *filetype;
int (*GetChar)();
DataSource *data;
{
  Widget result;
  FILE *ptr;  
  int pds[2];
  int stderrds;
  int fd;
  char *tempfile;

  if (filetype->Tempfile)
    {
      if (!WWWWriteToTempfile(mb,GetChar,data,&tempfile,NULL)) return NULL;

      if (filetype->Shellflag)
        {
          if ((fd = open(tempfile,O_RDONLY,0)) < 0)
            {
              sprintf(mb->message,"Cannot open temporary file <p> Reason: %s\n",strerror(errno));
              mb->help_code = "file_open_failed";
              result = NULL;
              goto cleanup;
            }
          else
            {
              socket_close(data->socket);
              data->socket = fd;
              data->nleft = 0;
              data->read = 0;
              data->eof = FALSE; 
            }
        }
    }
  else
      tempfile = XrmQuarkToString(mb->file->file);
 
  if (filetype->Command && filetype->Shellflag)
    { 
#ifndef VMS
       if (pipe(pds) <0)
         {
           sprintf(mb->message,"pipe error: can't create a pipe");
           mb->help_code = "pipe_fail";
           result = NULL;
           goto cleanup;
          }

       close(0);          /* close stdin */
       dup(data->socket); /* data socket becomes stdin */

       close(2);                    /* close stderr */
       stderrds = dup(pds[1]);      /* write end of pipe pds becomes stderr */

       ptr = popen(filetype->Command, "r");

       close(pds[1]);   /* we don't need the write-end of pipe in the parent process */
       close(stderrds); /* close the stderr descriptor */
       dup(1);          /* stderr become stdout */
       if (ptr == NULL)
         {
           sprintf(mb->message,"Call to popen failed for command: <tt>%s</tt>",filetype->Command);
           mb->help_code = "pipe_fail";    
           close(pds[0]);
           result = NULL;
           goto cleanup;
         } 
       data->socket = fileno(ptr); /* ??? */
#else
       mb->help_code = "no_vms_pipes";    
       sprintf(mb->message,"Sorry, pipes not supported (yet) under VMS.<p>Unable to execute command: <tt>%s</tt>",filetype->Command);
       result = NULL;
       goto cleanup;
#endif  
    }
  else ptr = NULL; 

  if      (filetype->Destination == psDestination  ) result = WWWLoadPS(w,mb,GetChar,data);
  else if (filetype->Destination == htmlDestination) result = SGMLHyperLoadText(w,GetChar,data);
  else if (filetype->Destination == gifDestination ) result = WWWLoadGIF(w,mb,GetChar,data); 
  else if (filetype->Destination == textDestination) result = SGMLHyperLoadText(w,WWWBuildFile,data); /* VERY SILLY */
  else if (filetype->Destination == fileDestination) result = WWWLoadFile(w,mb,GetChar,data,filetype); 
  else if (filetype->Destination == xbmDestination ) result = WWWLoadXBM(w,mb,GetChar,data); 
  else if (filetype->Destination == multimediaDestination) result = WWWLoadMultimedia(w,mb,filetype,tempfile);
  else if (filetype->Destination == unsupportDestination) 
    {
      sprintf(mb->message,"The file type is not supported by HTTP/1.0 at this moment");
      mb->help_code = "unknown_destination";
      result = NULL;
    }
  else
    {
      sprintf(mb->message,"Unknown destination %s",XrmQuarkToString(filetype->Destination));
      mb->help_code = "unknown_destination";
      result = NULL;
     }

#ifndef VMS
  if (ptr)
    {
      int iss = pclose(ptr); 

      if (iss && result == NULL) 
        {
          int n, i;
          char buff[512], reason[100];

          if      (WIFSTOPPED(iss) ) sprintf(reason, "Stopped, signal %d"     ,WSTOPSIG(iss));
          else if (WIFEXITED(iss)  ) sprintf(reason, "Exited, status = %d"    ,WEXITSTATUS(iss));
          else if (WIFSIGNALED(iss)) sprintf(reason, "Terminated, signal = %d",WTERMSIG(iss));
          else                       strcpy(reason,"Unknown");  

          if ((n = read(pds[0],buff,sizeof(buff))) > 0)
            {
              buff[n > 511 ? 511 : n] = '\0';

              sprintf(mb->message+strlen(mb->message),"<p>Error while processing file <p>Command: <tt>%s</tt><p>Reason: %s<p>Messages: <pre>%s</pre>",
                      filetype->Command,reason,buff);                                       
              mb->help_code = "file_process_fail";
            }
          else
            {
              sprintf(mb->message+strlen(mb->message),"<p>Error while processing file <p>Command: <tt>%s</tt><p>Reason: %s",
                      filetype->Command,reason);
              mb->help_code = "unknown";
            }
        }
    } 
  if (filetype->Command) close(pds[0]);
#endif

cleanup:
  if (filetype->Tempfile && filetype->Shellflag)
    {
      close(fd);
      unlink(tempfile);
      XtFree(tempfile);
    }
  return result;
}
/*
 * Fetch the local document
 * ------------------------
 */

Widget WWWFetchDocumentLOCAL(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{
  Widget result;
  WWWFiletype *filetype; 
  char *name = XrmQuarkToString(file->file); 
  DataSource data;
  char buffer[8096];

  int f = open(name,O_RDONLY,0);
  if (f < 0)   
    {
      sprintf(mb->message,"Cannot open file %s <p> Reason: %s",name,strerror(errno));
      mb->help_code = "file_open_failed";
      return NULL;
    }  
  WWWInitDataSource(&data,mb,buffer,sizeof(buffer),read,f);
  
  mb->flag = -1;  

  filetype = WWWGetFiletype(name,appResources.localDefaultFiletype);
  /* Since it is Local, don't need to use any tempfile */
  filetype->Tempfile = FALSE; 
  result = WWWLoadByFiletype(w,mb,filetype,GetCharacter,&data);

  WWWFreeFiletype(filetype); 
  close(f);
  return result;
}  
/* 
 * Make a TCPIP connection
 *
 * If this routine fails it returns 0 with an error message in mb->message
 * Otherwise it returns the socket number on which the connection has been made. 
 */ 
int TCPIPConnect(file,mb)
WWWFile *file;
MessageBlock *mb;
{
  static struct sockaddr_in server;
  static XrmQuark prevnode = NULL;
  char *node = XrmQuarkToString(file->node);
  int port = file->port;
  int s = 0; 
  int enable = 1;
  int disable = 0;
  XtInputId id,id2;

/*
 * Check that the port is valid
 */

  if (port < 1024)
    {
      char buf[20];
      List *ok = appResources.ok_ports;
      ListItem *item;

      sprintf(buf,"%d",port);
      item = MidasFindItemInList(ok,buf);

      if (!item)
        {
          sprintf(mb->message,"Connection to %s port %d disallowed",node,port);
          mb->help_code = "secure_port";
          return 0;
        } 
    }
  /*
   * For sites with overloaded name servers (such as SLAC), name translation
   * can be very slow, so we add some caching here. 
   */
  if (file->node != prevnode || prevnode == NULL) /* common case */
    {
      static List *nodeList = NULL;
      struct _cache { int length ; XtPointer data; } *cache; 
      ListItem *item;

      if (!nodeList) nodeList = MidasCreateEmptyList("nodeList"); 

      item = MidasFindItemInList(nodeList,node);
      if (!item)
        {
          struct hostent *hp;

          hp = gethostbyname(node);
          if (hp == 0)
            {
              sprintf(mb->message,"Host %s unknown",node);
              mb->help_code = "unknown_host";
              return 0;
            }

          item = MidasAddItemToList(nodeList,node);
          cache = (struct _cache *) XtMalloc(hp->h_length + sizeof(int));
          cache->length = hp->h_length; 
          bcopy(hp->h_addr, &cache->data, hp->h_length);
          item->Pointer = (XtPointer) cache; 
        }
      cache = (struct _cache *) item->Pointer;
      bcopy(&cache->data,&server.sin_addr, cache->length);
      prevnode = file->node;
    } 

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  
  /*
   * Sockets 0,1,2 are no good, just get another one 
   */

  for (;s >= 0 && s <= 2; ) s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s < 0)
    {
      sprintf(mb->message,"Socket create failed<p> Reason: %s",strerror(errno));
      mb->help_code = "socket_create_failed";
      return 0;
    }                                   


  mb->newMessage = TRUE;
  sprintf(mb->message,"Waiting to connect to: %s port %d",node,port); 

#ifdef MULTINET
    {
      int rc;
      short iosb[4];
      XtInputId id; 

      mb->flag = -1;
      rc = SYS$QIO(EF,s,IO$_CONNECT,iosb,0,0,&server,sizeof(server),0,0,0,0);
      id = XtAppAddInput(appl_context,EF,(XtPointer) iosb,(XtInputCallbackProc) ClearFlag,(XtPointer) &mb->flag);

      for (; mb->flag < 0;)
        {
          MidasFetchDispatchEvent();
        }
      XtRemoveInput(id); 

      if (mb->flag == 1)
        {
          Sys$Cancel(s);
        }
      else if (iosb[0] != 1) mb->flag = -1;
    }
#else /* MULTINET */
#ifndef NO_NBIO
  socket_ioctl(s,FIONBIO,&enable);
#endif
  mb->flag = connect(s,&server,sizeof(server));
  if (mb->flag<0 && (socket_errno == EWOULDBLOCK || socket_errno == EINPROGRESS))
    {
#ifdef ADDINPUTOK
      XtInputId id = XtAppAddInput(appl_context,s,(XtPointer) XtInputWriteMask,(XtInputCallbackProc) ClearFlag,(XtPointer) &mb->flag);
      for (;mb->flag < 0;)
        {
          MidasFetchDispatchEvent();
        } 
      XtRemoveInput(id); 
#endif
    }
#ifndef NO_NBIO
  socket_ioctl(s,FIONBIO,&disable);
#endif
#endif
  if (mb->flag == 1)
    {
      strcpy(mb->message,AbortMessage);
      socket_close(s);
      return 0; 
    }
  if (mb->flag < 0)  
    {
      sprintf(mb->message,"Cannot connect to %s port %d",node,port);
      mb->help_code = "cannot_connect";
      socket_close(s);
      return 0;
    }
  return s;
}
/*
 * This routine writes a message over the TCPIP connection, and then waits until a
 * response is available (or the user aborts) before returning.
 *
 * If this routine fails it returns 0 with an error message in mb->message and closes the socket 
 * Otherwise it returns the socket number on which the connection has been made. 
 */
int TCPIPWriteAndWaitForResponse(s,file,mb,command)
int s;
WWWFile *file;
MessageBlock *mb;
char *command;
{
  char *node = XrmQuarkToString(file->node);
  int port = file->port;
  XtInputId id;

  mb->newMessage = TRUE;
  sprintf(mb->message,"Waiting to write to %s port %d",node,port);

#ifdef MULTINET
    { 
      int rc;
      short iosb[4];

      mb->flag = -1;

      rc = SYS$QIO(EF,s,IO$_SEND,iosb,0,0,command,strlen(command),0,0,0,0);
      id = XtAppAddInput(appl_context,EF,(XtPointer) iosb,(XtInputCallbackProc) ClearFlag,(XtPointer) &mb->flag);
      for (; mb->flag < 0;)
        {
          MidasFetchDispatchEvent();
        }
      XtRemoveInput(id);
      if (mb->flag == 1)
        {
          Sys$Cancel(s);
          strcpy(mb->message,AbortMessage);
          socket_close(s);
          return 0;
        }
      else if (iosb[0] != 1) mb->flag = -1;
    } 
#else /* MULTINET */
    {  
      void (*oldhandler)() =  signal(SIGPIPE,SIG_IGN);

      mb->flag = -1;

#ifdef ADDINPUTOK
      id = XtAppAddInput(appl_context,s,(XtPointer) XtInputWriteMask,(XtInputCallbackProc) ClearFlag,(XtPointer) &mb->flag);

      for (;mb->flag < 0;)
        {
           MidasFetchDispatchEvent();
        }
      XtRemoveInput(id);

      if (mb->flag == 1)
        {
          strcpy(mb->message,AbortMessage);
          socket_close(s);
          return 0;
        }
#endif
      mb->flag = socket_write(s,command,strlen(command));
      signal(SIGPIPE,oldhandler);
    }
#endif

  if (mb->flag < 0)  
    {
      sprintf(mb->message,"Cannot connect to %s port %d",node,port);
      mb->help_code = "cannot_connect";
      socket_close(s);
      return 0;
    }
  mb->newMessage = TRUE;
  sprintf(mb->message,"Waiting for response from %s port %d",node,port); 

  mb->flag = -1;

#ifdef ADDINPPUTOK
  id = XtAppAddInput(appl_context,s,(XtPointer) XtInputReadMask,(XtInputCallbackProc) ClearFlag,(XtPointer) &mb->flag);

  for (;mb->flag < 0;)
     {
       MidasFetchDispatchEvent();
     } 
  XtRemoveInput(id);  
#endif

  if (mb->flag == 1)
    {
      strcpy(mb->message,AbortMessage);
      socket_close(s);
      return 0; 
    }

  return 1; 
}
Boolean WWWAskForFileDestination(w,mb,filetype,binaryAllowed)
Widget w;
MessageBlock *mb;
WWWFiletype *filetype;
Boolean binaryAllowed;
{
  Arg arglist[10];
  int n = 0;
  char *fname;
  MidasShell *ms = MidasGetShell(w);
  Widget fd = MidasTraceWidgetTree(ms->Widget,".WWWMain###WWWSaveFileAs");
  Widget binary = MidasTraceWidgetTree(fd,"...WWWSaveAsBinary");
      
  if (binaryAllowed)
    {
      XtSetArg(arglist[n],XmNsensitive,TRUE); n++;
      XtSetArg(arglist[n],XmNset,filetype->Binary); n++;
      XtSetValues(binary,arglist,n);
    }
  else XtSetSensitive(binary,FALSE);  

  MidasSetIngotString(fd,"file",filetype->UserData); 
      
  fname = MidasQueryUser(fd);

  if (!fname || *fname == '\0')
    {
      strcpy(mb->message,AbortMessage);
      XtFree(fname);
      return FALSE;
    }
      
  filetype->UserData = (XtPointer) fname;
  if (binaryAllowed)
    {
      n = 0;
      XtSetArg(arglist[n],XmNset,&filetype->Binary); n++;
      XtGetValues(binary,arglist,n);
    }   
  return TRUE;
}

/*
 * Generate a history listing
 */

/*
 *  Dump the hash table
 *  -------------------
 */
static void WWWDumpTree(a,unique_id,state)
WWWNode *a;
int unique_id;
XtPointer state;
{
  WWWNode *child; 
  WWWFile *pfile = a->file; 
  char *p = WWWAsciiFile(pfile);
  int sublist = 0;

  a->id = unique_id;

  if (a == YouAreHereNode) SGMLHyperWrite(state,"<li><img pixmap=circle><a href=\"");
  else                     SGMLHyperWrite(state,"<li><a href=\"");
  SGMLHyperWrite(state,p);
  SGMLHyperWrite(state,"\">");
  SGMLHyperWrite(state,a->title);
  SGMLHyperWrite(state,"\n</a>"); 
  XtFree(p);    
 
  for (child = a->down; child; child = child->next) 
    {
      WWWNode *b = child->down;
      if (!b) continue;
      b = b->parent;
      if (b->visited && b->id != unique_id) 
        {
          if (!sublist++) SGMLHyperWrite(state,"<ul \"bulletType\"=linkedArrow>\n");
          WWWDumpTree(b,unique_id,state);
        } 
    } 
  if (sublist) SGMLHyperWrite(state,"</ul>\n");
}

static void WWWDumpHistory(state,first)
XtPointer state;
WWWNode *first;
{
  static int unique_id = 0;
  WWWNode *a;

  unique_id++; /* Unique id for this scan */  
 
  for (a = first; a ; a = a->route)    
    {
      WWWNode *b = a->parent;
      if (b->id != unique_id && b != historyNode) WWWDumpTree(b,unique_id,state);
    } 
}

static Widget WWWFetchDocumentHISTORY(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{ 
  XtPointer state = SGMLHyperOpen(w);

  SGMLHyperWrite(state,"<title>History</title><nocache>\n");
  SGMLHyperWrite(state,"<h1>Documents visited this session</h1><ul \"bulletType\"=arrow>\n");
  WWWDumpHistory(state,FirstVisitedNode);
  SGMLHyperWrite(state,"</ul>");
  return SGMLHyperClose(state);
}
static Widget WWWFetchDocumentINGOT(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{ 
  MidasOperand result;

  result = MidasGetIngot(w,XrmQuarkToString(file->file));
  return SGMLHyperSetText(w,(char *) result.Value.P);
}
 
/*
 * Open a telnet connection
 */

Widget WWWFetchDocumentTELNET(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{ 
   char buffer[256];
   char *ptr = XrmQuarkToString(file->node);
   char *legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_";
   char *cpt;
   char *hostname;
   char *username;
#ifdef VMS
   int flags = 1; /* nowait */ 
   int iss; 
   short len;
   char result[256];
   struct { 
     unsigned short len;
     unsigned char  type;
     unsigned char  class;
     char           *ptr;
   } command_D = {0,14,1,0}, 
     device_D = {0,14,1,0}, 
     input_D = {9,14,1,"SYS$INPUT"}, 
     msg_D = {sizeof(buffer),14,1,buffer};
#endif
   if (!ptr || *ptr == '\0') 
     {
        strcpy(mb->message,"No node specifed for TELNET conection");
        mb->help_code = "telnet_no_node";
        return NULL;
     }  
   cpt = strchr(ptr,'@');
   hostname = XtMalloc(strlen(ptr)+1);
   username = XtMalloc(strlen(ptr)+1);

   if (cpt)
     {
       int i;
       cpt++;
       strcpy(hostname,cpt);
       for (i=0;i<=strlen(ptr);i++)
         if ((username[i] = ptr[i]) == '@') break;
       username[i] = '\0';
      }
   else strcpy(hostname,ptr);

   /* 
    * Security fix, check for bad characters that can cause shell shock
    */

   if (strspn(hostname,legal) != strlen(hostname))
     {
        strcpy(mb->message,"Illegal character in telnet node specification");
        mb->help_code = "telnet_illegal_char";
        return NULL;
     }  
#ifdef VMS
   /*
    * These commands are right for MULTINET, others?
    */

   sprintf(buffer,"*title : TELNET %s\n",XrmQuarkToString(file->node));

   iss = DECwTermPort(NULL,NULL,buffer,result,&len);
   if (iss != 1) 
     {
        Lib$Sys_GetMsg(&iss,&len,&msg_D);
        buffer[len] = '\0';
        sprintf(mb->message,"Error creating DECTERM for telnet connection:<p>%s",buffer);
        mb->help_code = "vms_telnet_error";
        return NULL;
     }

   if (file->port) sprintf(buffer,"telnet/port=%d %s",file->port,XrmQuarkToString(file->node)); 
   else            sprintf(buffer,"telnet %s"        ,XrmQuarkToString(file->node));

   command_D.len = strlen(buffer);
   command_D.ptr = buffer;

   device_D.len = len;
   device_D.ptr = result;

   Sys$Alloc(&device_D,0,0,0,0);
   iss = Lib$Spawn(&command_D,&device_D,&device_D,&flags);
   Sys$Dalloc(&device_D,0);
   if (iss != 1) 
     {
        Lib$Sys_GetMsg(&iss,&len,&msg_D);
        buffer[len] = '\0';
        sprintf(mb->message,"Error creating subprocess for telnet connection:<p>%s",buffer);
        mb->help_code = "vms_telnet_error";

        return NULL;
     }
#else  
   if (file->port) sprintf(buffer,"xterm -e telnet %s %d &",hostname,file->port); 
   else            sprintf(buffer,"xterm -e telnet %s &",hostname);

   system(buffer);
#endif

   if (cpt)
    {
      if (file->port) sprintf(mb->message,"<nocache><title>Telnet %s</title><h1><img pixmap=telnet>Telnet %s</h1>Opened telnet connection to <b>%s port %d</b><p>Login Username <b>%s</b>",hostname,hostname,hostname,file->port,username);
      else sprintf(mb->message,"<nocache><title>Telnet %s</title><h1><img pixmap=telnet>Telnet %s</h1> Opened telnet connection to <b>%s</b><p>Login Username <b>%s</b>",hostname,hostname,hostname,username);
     }
   else
    {
      if (file->port) sprintf(mb->message,"<nocache><title>Telnet %s</title><h1><img pixmap=telnet>Telnet %s</h1> Opened telnet connection to <b>%s port %d</b>",hostname,hostname,hostname,file->port);
       else sprintf(mb->message,"<nocache><title>Telnet %s</title><h1><img pixmap=telnet>Telnet %s</h1> Opened telnet connection to <b>%s</b>",hostname,hostname,hostname);
     }

   XtFree(hostname);
   XtFree(username);
   return NULL;
}
 /*
 * Fetch the document using appropriate protocol
 * ---------------------------------------------
 */
static Widget WWWFetchDocumentByProtocol(w,infile,mb,title)
Widget w;
WWWFile *infile;
MessageBlock *mb;
char *title;
{
  WWWFile temp;           /* When the port is 0 it is changed to the default port by  */
  WWWFile *file = &temp;  /* the fetch document routines, so need to make copy, or    */
  temp = *infile;         /* it causes problems with the hashing of nodes             */

  if      (file->protocol == httpProtocol   ) 
    {
      if (file->method == getMethod)
        {
          if (!strcmp(appResources.default_HTTP_protocol,"1.0"))
            return WWWFetchDocumentHTTP1(w,file,mb);
          else return WWWFetchDocumentHTTP(w,file,mb);
        }
      else if (file->method == postMethod) return WWWPostDocumentHTTP1(w,file,mb);
      else
        {
           sprintf(mb->message,"Unknown method %s",XrmQuarkToString(file->method));
           mb->help_code = "bad_method";
           return NULL;
        }  
    }
  else if (file->protocol == localProtocol  ) return WWWFetchDocumentLOCAL   (w,file,mb);
  else if (file->protocol == gopherProtocol ) return WWWFetchDocumentGOPHER  (w,file,mb,title); 
  else if (file->protocol == ftpProtocol    ) return WWWFetchDocumentFTP     (w,file,mb);
  else if (file->protocol == fileProtocol   ) return WWWFetchDocumentFTP     (w,file,mb); 
  else if (file->protocol == historyProtocol) return WWWFetchDocumentHISTORY (w,file,mb);
  else if (file->protocol == ingotProtocol  ) return WWWFetchDocumentINGOT   (w,file,mb);
  else if (file->protocol == telnetProtocol ) return WWWFetchDocumentTELNET  (w,file,mb);
  else if (file->protocol == waisProtocol   )
    {
      WWWFile actual;
      char *p =  WWWAsciiFile(file);
      char *new = p + 6; /* +6 to remove wais:/ */
      
      actual.protocol = httpProtocol;
      actual.node     = appResources.wais_gateway_node;
      actual.port     = appResources.wais_gateway_port;
      actual.file = XrmStringToQuark(new); 
      actual.keyword = NULL;
      actual.anchor = NULL; 
      actual.method = getMethod;
      
      XtFree(p);
      
      if (!strcmp(appResources.default_HTTP_protocol,"1.0")) 
        return WWWFetchDocumentHTTP1(w,&actual,mb);
      else return WWWFetchDocumentHTTP(w,&actual,mb);
    }   
  else 
    {
      WWWFile actual;   
      char *p =  WWWAsciiFile(file);
      char *new = XtMalloc(strlen(p)+2);
      
      strcpy(new,"/");
      strcat(new,p);
      
      actual.protocol = WWWGateway->protocol;
      actual.node     = WWWGateway->node;
      actual.port     = WWWGateway->port;
      actual.file = XrmStringToQuark(new); 
      actual.keyword = NULL;
      actual.anchor = NULL; 
      actual.method = getMethod;
      
      XtFree(p);
      XtFree(new);
        
      if (!strcmp(appResources.default_HTTP_protocol,"1.0")) 
        return WWWFetchDocumentHTTP1(w,&actual,mb);
      else return WWWFetchDocumentHTTP(w,&actual,mb);
    }
}  
/*
 * Sets up handling for the popup message box
 */
static void SetFlag(w,flag)
Widget w;
int *flag;  
{
  *flag = 1;
}   
static void DrawMessage(w,mb,reason)
Widget w;
XmDrawingAreaCallbackStruct *reason;
MessageBlock *mb;
{
  char name[512];
  char *p;
  Boolean update = (reason == NULL);
  Display *dpy = XtDisplay(w);
  Window wind = XtWindow(w);
 
  if (mb->height == 0)
    {
      int ascent, descent, dir;
      XCharStruct overall;
 
      XTextExtents(appResources.font,mb->message,strlen(mb->message),&dir,&ascent,&descent,&overall);
      mb->height = ascent + descent;
      mb->ascent = ascent;
      mb->width =  overall.width;
    }
  if (!update)
    {
      p = WWWAsciiFile(mb->file); 
      if (strlen(p) > 150) strcpy(p+140," ...");

      sprintf(name,"Getting %s",p);
      XDrawImageString(dpy,wind,messageGC,0,mb->ascent,name,strlen(name));
    }
  if (*mb->message != '@') 
    {  
      p = mb->message;
      if (mb->halffull > 0) XClearArea(dpy,wind,5,mb->ascent+2*mb->height,400,mb->height,FALSE); 
      mb->halffull = 0;
    }
  else if (mb->length == 0)
    {
      p = name;
      sprintf(name,"Read %d bytes",mb->read);
      if (mb->halffull > 0) XClearArea(dpy,wind,5,mb->ascent+2*mb->height,400,mb->height,FALSE); 
      mb->halffull = 0;
    }  
  else
    {  
      int halffull = mb->read < mb->length ? 400*mb->read/mb->length : 400;
      int halfempty = 400 - halffull;

      sprintf(name,"Read %d of %d bytes",mb->read,mb->length);
      p = name;
     
      if (halffull)  XFillRectangle(dpy,wind,messageGC,5,mb->ascent+2*mb->height,halffull,mb->height);
      if (halfempty) 
        {
          if (halffull < mb->halffull) 
            XClearArea(dpy,wind,5+halffull,mb->ascent+2*mb->height,halfempty,mb->height,FALSE);
          XDrawRectangle(dpy,wind,messageGC,5,mb->ascent+2*mb->height,400,mb->height);
        }
      mb->halffull = halffull;
    }
  if (update)
    {
      int l = strlen(p); 
      int width = XTextWidth(appResources.font,p,l);
      XDrawImageString(dpy,wind,messageGC,0,mb->ascent+mb->height,p,l);
      if (width < mb->width) 
        XClearArea(dpy,wind,width,mb->height,mb->width - width,mb->height,FALSE);   
      mb->width = width;  
    }
  else XDrawImageString(dpy,wind,messageGC,0,mb->ascent+mb->height,p,strlen(p));
  XFlush(dpy);
}
static void SetMessage(mb)
MessageBlock *mb;
{ 
  if (mb->newMessage)
    {
      Widget w = mb->draw;
      mb->newMessage = FALSE;
      if (mb->height)
        {
          DrawMessage(w,mb,NULL);
        } 
    }  
  mb->id = XtAppAddTimeOut(appl_context,appResources.update_time_out,(XtTimerCallbackProc) SetMessage,(XtPointer) mb);
}
static void PopupMessage(mb)
MessageBlock *mb;
{
  if (*mb->message != '\0')
    {
      SetMessage(mb);
      MidasPopup(mb->widget);
      XFlush(XtDisplay(mb->widget));
    }
  else 
    { 
      int time =  appResources.waiting_time_out;
      if (time<100) time = 100;
      mb->id = XtAppAddTimeOut(appl_context,time,(XtTimerCallbackProc) PopupMessage,(XtPointer) mb);    
    }
}
static Widget WWWFetchDocument(w,file,include,title)
Widget w;
WWWFile *file;
List *include;
char *title;
{
  MessageBlock mb;
  MidasShell *ms = MidasGetShell(w);
  MidasOperand Popup;
  Widget result;
  Widget info, draw;
  static Boolean first = TRUE;

  Popup = MidasGetIngot(ms->Widget,"Popup"); 

  if (!Popup.Value.P)
    { 
      ListItem *item;
      char *fixedMessage = (char *) XtMalloc(sizeof(mb.message));

      info = MidasFetch("WWWWaiting",ms->Widget); 

      Popup.Value.P = (XtPointer) info;
      Popup.Type = "Widget";
      Popup.Dynamic = FALSE;
      MidasSetIngot(ms->Widget,"Popup",Popup);
    }
  else info = (Widget) Popup.Value.P;
 
  draw = MidasTraceWidgetTree(info,".XmDrawingArea");
  XtAddCallback(draw,XmNexposeCallback,(XtCallbackProc) DrawMessage,(XtPointer) &mb); 
  XtAddCallback(info,XmNunmapCallback,(XtCallbackProc) SetFlag,(XtPointer) &mb.flag);
  XtAddGrab(info,TRUE,FALSE);

  mb.flag = -1;
  mb.file = file;
  mb.parent = ms->Widget; 
  mb.help_code = ""; 
  strcpy(mb.message,"");
  mb.widget = info;
  mb.draw = draw;
  mb.height = 0;
  mb.width = 0;
  mb.halffull = 0;

  if (appResources.waiting_time_out < 0) mb.id = NULL; 
  else 
    {
      int timeout = first ? 10000 : appResources.waiting_time_out;         
      mb.id = XtAppAddTimeOut(appl_context,timeout,(XtTimerCallbackProc) PopupMessage,(XtPointer) &mb);
    }
  first = FALSE;  

  result = WWWFetchDocumentByProtocol(w,file,&mb,title);

  if (!result)
    {
      if (!*mb.help_code) result = SGMLHyperSetText(w,mb.message);
      else 
        {
          XtPointer t = SGMLHyperOpen(w);
          SGMLHyperWrite(t,"<nocache><h2>Error:</h2>");
          SGMLHyperWrite(t,mb.message);
          SGMLHyperWrite(t,"<p>For more information see <a href=");
          SGMLHyperWrite(t,appResources.help);
          SGMLHyperWrite(t,mb.help_code);
          SGMLHyperWrite(t,".html>help</a>.");
          result = SGMLHyperClose(t);         
        } 
    } 
  else 
    {
       ListItem *item;

       /* 
        * Make sure the document is in the expected format, if not make it so
        *
        */

       if (strcmp(mb.help_code,""))
         {
           Widget child;
           Arg arglist[10];
           int n=0;      

           if (!strcmp(mb.help_code,"image"))  
             { 
               GIFImage *image = (GIFImage *) result;
               XtSetArg(arglist[n],SGMLNimage, image); n++; 
             }  
           else if (!strcmp(mb.help_code,"pixmap"))
             { 
               Pixmap pm = (Pixmap) result;
               XtSetArg(arglist[n],SGMLNpixmap, pm); n++; 
             }  
           XtSetArg(arglist[n],SGMLNdeleteImageWhenDone, TRUE); n++;

           result = XtCreateWidget("default",sGMLCompositeTextObjectClass,(Widget)w,NULL,0);
           child = XtCreateWidget("image",sGMLMarkerTextObjectClass,(Widget)result,arglist,n); 
           SGMLCompositeTextInsertChild(child);
         }

       /*
        * If the document was successfully fetched, then maybe there are included documents
        * to deal with (currently this only supports IMG tags).
        */
       for (item = include->First; item && mb.flag != 1 ; item = item->Next)
         {
            Arg arglist[1];
            ImageLink *il = (ImageLink *) item->Pointer;
            WWWNode *node = il->Node;

            if (!node->icon && !node->picture) 
              {
                WWWFile *incf = node->file;
                Widget xxx;

                mb.help_code = ""; 
                xxx = WWWFetchDocumentByProtocol(w,incf,&mb,NULL);

                if (!xxx) printf("Error processing <img> tag: %s\n",mb.message); 
                else if (!strcmp(mb.help_code,"image") ) node->picture = (GIFImage *) xxx;
                else if (!strcmp(mb.help_code,"pixmap")) node->icon    = (Pixmap) xxx;
                else
                  {
                    char *name = WWWAsciiFile(incf);
                    printf("Document: %s\nnot in format suitable for <img> tag\n",name);
                    XtDestroyWidget(xxx);
                    XtFree(name);
                  }   
                if (mb.flag == 1) break; /* User aborted document fetch */ 
              }  
            if (node->icon)
              {
                unsigned int depth; 

                XtSetArg(arglist[0],SGMLNpixmap,node->icon);
                XtSetValues(il->Widget,arglist,1);
 
                XtSetArg(arglist[0],SGMLNpixmapDepth,&depth);
                XtGetValues(il->Widget,arglist,1); 
  
                if (depth == 1) 
                  {
                    XtSetArg(arglist[0],SGMLNoutline,0);
                    XtSetValues(il->Widget,arglist,1);
                  }
              }
            else if (node->picture)
              {
                XtSetArg(arglist[0],SGMLNimage,node->picture);
                XtSetValues(il->Widget,arglist,1);
              }   
         }
    }
  MidasEmptyList(include); /* Always tidy up after you have finished playing */
  XtRemoveCallback(info,XmNunmapCallback,(XtCallbackProc) SetFlag,(XtPointer) &mb.flag);
  XtRemoveCallback(draw,XmNexposeCallback,(XtCallbackProc) DrawMessage,(XtPointer) &mb); 
  if (mb.id) XtRemoveTimeOut(mb.id);
  
  if (XtIsManaged(info))
    {
      /* We have sometimes seen problems where managing and the rapidly 
       * unmanaging a window cause protocol errors (or worse) due to expose events 
       * arriving after the window has been unmanaged. To overcome this problem
       * we now wait here until we receive the UnmapNotify event from the shell 
       * We also ignore any expose events for the info or draw widget while waiting
       * since there is no point in processing them.  
       */
      Boolean unmap;

      XtUnmanageChild(info);
      XFlush(XtDisplay(info));
      for (unmap = TRUE; unmap ;) /* Wait until we get the UnMapNotify event */ 
        { 
          XEvent event;
          XtAppNextEvent(appl_context,&event);
          if (event.type == Expose) 
            {
              if (event.xexpose.window != XtWindow(info) && 
                  event.xexpose.window != XtWindow(draw))
                XtDispatchEvent(&event); 
            }
          else if (event.type == UnmapNotify)
            {
              if (event.xunmap.window == XtWindow(XtParent(info))) unmap = FALSE; 
              XtDispatchEvent(&event); 
            } 
          else XtDispatchEvent(&event); 
        } 
    }
  XtRemoveGrab(info);
  return result;
}
static void WWWSetTitle(node,title)
WWWNode *node;
char *title;
{
  node->parent->title = XtNewString(title);
  WWWUpdateHistory(node); 
  /*
   * The logic (sic) here is that if you have visited a sub-node you have 
   * also visited the node's parent, but NOT vice_versa 
   */
  WWWVisitNode(node);
  if (node != node->parent) WWWVisitNode(node->parent);
}
static void WWWZapNode(w,node)
Widget w;
WWWNode *node;
{
   int n=0;
/* 
 * Remove a cached document
 */

  node = node->parent;
  
  for (n = 0; n < node->num_widgets; n++)
    {
      Widget t = node->widgets[n];
      if (SGMLIsCompositeText(t) && XtParent(t) == w)
        {     
           WWWDetachWidgetFromNode(t,node);  
           XtDestroyWidget(t);
           break;   
        }
    }    
}
/*
 * Display a new node in a widget
 * ------------------------------
 */ 
static Widget WWWDisplayNode(w,node,include,title)
Widget w;
WWWNode *node;
List *include;
char *title;
{
   Widget new;
   int n=0;
/*
 *  The document may already be cached in the widget?
 */    
  
  for (n = 0; n < node->num_widgets; n++)
    {
      Widget t = node->widgets[n];
      if (SGMLIsCompositeText(t) && XtParent(t) == w)
        {     
           SGMLHyperManageChild(t);
           return t;   
        }
    }    
/*
 *  Otherwise we have to fetch it.
 */
 
  WWWlastnode = NULL;
  WWWparent = node;

  new = WWWFetchDocument(w,node->file,include,title);
  WWWAttachWidgetToNode(node,new);
/*
 * Check for a redirection request. 
 */
  if (node->redirect)
    {
      MidasOperand Temp;
      Temp.Value.P = (XtPointer) node->redirect;
      Temp.Dynamic = FALSE;
      Temp.Type = "WWWNode";
      MidasSetIngot(w,"WWWFileBase",Temp);
      new = WWWDisplayNode(w,node->redirect,include,title);
    }
  
  SGMLHyperManageChild(new);
  return new;
}
/*
 * Simplify a file by dealing with /. and /..
 * ----------------------------------------------------------------
 */ 
static void SimplifyFile(filename)
char *filename;
{
    char * p;
    char * q;
    if (filename[0] && filename[1])	/* Bug fix 12 Mar 93 TBL */
     for(p=filename+2; *p; p++) {
        if (*p=='/') {
	    if ((p[1]=='.') && (p[2]=='.') && (p[3]=='/' || !p[3] )) {
		for (q=p-1; (q>=filename) && (*q!='/'); q--); /* prev slash */
		if (q[0]=='/' && 0!=strncmp(q, "/../", 4)
			&&!(q-1>filename && q[-1]=='/')) {
	            strcpy(q, p+3);	/* Remove  /xxx/..	*/
		    if (!*filename) strcpy(filename, "/");
		    p = q-1;		/* Start again with prev slash 	*/
		}		/*   xxx/.. leave it!	*/
	    } else if ((p[1]=='.') && (p[2]=='/' || !p[2])) {
	        strcpy(p, p+2);			/* Remove a slash and a dot */
	    }
	}
    }
}
/*
 *  parse a character file specification to create a file structure
 * ----------------------------------------------------------------
 */  
static WWWFile *ParseFile(infile,def)
char *infile;
WWWFile *def; 
{
  char *p, *node, *oldfile, *newfile, *anchor, *protocol;
  char *directory, *port;
  WWWFile *result = CopyFile(def);
  char *file = XtNewString(infile); 
  char *orig = file;
   
  result->anchor = NULL; /* NOT inherited from default */
  result->method = getMethod;

  /* strip leading and trailing space`s */  

  if (!file) file = "";
  else
    { 
      for (; isspace(*file); ) file++;
      for (p = file + strlen(file); isspace(*--p); ) *p = '\0'; 
    }

  node = strstr(file,"/");
  protocol = strchr(file,':');
  if (protocol && (protocol < node || node == 0)) 
    {
      *protocol = '\0';
      result->protocol = XrmStringToQuark(file);
      if (result->protocol != def->protocol)
        {    
          result->node = NULL;
          result->port = 0;
        }  
      file = protocol + 1;
    } 
  else protocol = NULL; 
    
  if (strncmp(file,"//",2)) 
    { 
      if (*file == '/' || protocol) 
        {
          SimplifyFile(file);

          if (anchor = strchr(file,'#')) 
          {
            *anchor++ = '\0';
            result->anchor = XrmStringToQuark(anchor);
          }
          result->file = XrmStringToQuark(file); 
        }
      else
        {
          Boolean absolute;
          /*
           * This is kind of a mess, comes from trying to support the UW home page
           * without breaking access to local files from the command line. Maybe we shouldn't
           * worry about the UW home page?? Problem stems from fact that relative local addresses should
           * be left unchanges, while relative global addresses must be converted to absolute addresses.
           */
          if (result->protocol != localProtocol) oldfile = XtNewString(result->file?XrmQuarkToString(result->file):"/");
          else                                   oldfile = XtNewString(result->file?XrmQuarkToString(result->file):"");
          absolute = (*oldfile == '/');

          if (anchor = strchr(file,'#')) 
          {
            *anchor++ = '\0';
            result->anchor = XrmStringToQuark(anchor);
          }
 
          for (;*file;file += 3)
            {
              char *p = strrchr(oldfile,'/'); 
              if (!p)  p = oldfile; 
              *p = '\0';
              if (strncmp(file,"../",3)) break;
            }
          newfile = strcpy((char *) XtMalloc(strlen(oldfile) + strlen(file) + 2),oldfile);
          if ((absolute || *oldfile) && *file)strcat(newfile,"/");
          strcat(newfile,file);
          SimplifyFile(newfile);
          
          result->file = XrmStringToQuark(newfile);
          XtFree(newfile);
          XtFree(oldfile);
        }
    }
  else
    {
      char *p;

      node = file + 2;
     
      directory = strchr(node,'/');  
      if (directory) *(directory) = '\0';
    
      port = strchr(node,':');
      if (port)
        {
          *(port++) = '\0';
          result->port = atoi(port);   
        }
      else result->port = 0; /* Not inherited if node present */

      for (p=node; *p != '\0'; p++) if (isupper(*p)) *p = tolower(*p);  
      result->node = XrmStringToQuark(node);

      if (directory)
        {
          *(directory) = '/';

          if (anchor = strchr(directory,'#')) 
            {
              *anchor++ = '\0';
              result->anchor = XrmStringToQuark(anchor);
            }
        }
      
      result->file = XrmStringToQuark(directory); 
      
    }

  XtFree(orig);
  return result;
}
static void WWWFreeUnusedColors(id)
XtIntervalId *id;
{
  *id = NULL;
  fastAllocFreeUnusedColors();
}
static void WWWGet(ww,node,include,title)
Widget ww;
WWWNode *node;
List *include;
char *title;
{
  static XtIntervalId id = NULL;
  WWWLink *link;
  Widget new;

  if (id) XtRemoveTimeOut(id);

  new = WWWDisplayNode(ww,node->parent,include,title); 
  WWWToLink = node;

  id = XtAppAddTimeOut(appl_context,10000,(XtTimerCallbackProc) WWWFreeUnusedColors,(XtPointer) &id);

  if (strcmp(XtName(new),"gv_manager"))
    {
      if (node != node->parent) 
        {
          WidgetList w = node->widgets;
          int nw = node->num_widgets; 

          for (; nw-- > 0; w++) 
            {
               Widget p;
               for (p = *w; p != NULL; p = XtParent(p)) 
                 if (p == ww)
                   {
                     SGMLHyperShowObject(ww,*w,FALSE);
                     return; 
                   }
               SGMLHyperShowObject(ww,NULL,FALSE); 
            }
        }
      else SGMLHyperShowObject(ww,NULL,FALSE); 
    }
  else
    {  
      if (node != node->parent) 
        {
          WidgetList w = node->widgets;
          int nw = node->num_widgets; 

          for (; nw-- > 0; w++) 
            {
               Widget p;
               for (p = *w; p != NULL; p = XtParent(p)) 
                 if (p == ww)
                   {
                     if (SGMLIsContainerText(*w))
                       { 
                         Arg arglist[1];
                         char *page = XrmQuarkToString(node->file->anchor); 
                         Widget gv;
                     
                         XtSetArg(arglist[0],SGMLNchild,&gv);
                         XtGetValues(*w,arglist,1);

                         XtSetArg(arglist[0],XtNcurrentPage,atoi(page));
                         XtSetValues(gv,arglist,1);

                         XtSetArg(arglist[0],SGMLNmanaged,TRUE);
                         XtSetValues(*w,arglist,1);
                       }
                     else 
                       {
                         Widget ct = *w;
                         for (; SGMLIsCompositeText(XtParent(ct)) ; ct = XtParent(ct))
                           {
                             Arg arglist[1];
                      
                             XtSetArg(arglist[0],SGMLNmanaged,TRUE);
                             XtSetValues(ct,arglist,1);
                           }
                         SGMLHyperShowObject(ww,*w,FALSE);
                       }  
                     return;
                   }
            }
        }
      else
        {
          Widget Index = MidasTraceWidgetTree(new,".default");
          if (Index)
            {
              Arg arglist[1];
              XtSetArg(arglist[0],SGMLNmanaged,TRUE);
              XtSetValues(Index,arglist,1);
              SGMLHyperShowObject(ww,NULL,FALSE);
            }
        }
    }    
}
static void WWWRedirect(node1,node2)
WWWNode *node1,*node2;
{
   node1->redirect = node2;
}
static void WWWDeleteHistory(list)
List *list;
{
  ListItem *item = MidasFindItemInListPos(list,0);
  if (item) MidasRemoveItemFromList(list,item);
}
static void WWWAddHistory(list,title)
List *list;
char *title;
{
  ListItem *item;
  WWWLink *link = XtNew(WWWLink);
  link->from = WWWFromLink;
  link->to = WWWToLink;

  if (WWWFromLink == DEADNODE)
    {
      item = MidasFindItemInListPos(list,0);
      if (item) link->from = ((WWWLink *) item->Pointer)->to;
      else      link->from = NULL;
    }
  if (WWWFromLink)
    {
      item = MidasAddItemToListPos(list,title,0);
      item->Pointer = (XtPointer) link; 
    }   

  WWWFromLink = DEADNODE;
  WWWToLink = NULL;
}
static void WWWLinkHistory(node)
WWWNode *node;
{
  WWWFromLink = node;
}
static void WWWPut(w,stuff)
Widget w;
char *stuff;
{
  Widget t = SGMLHyperSetText(w,stuff);
  SGMLHyperManageChild(t); 
}
static void WWWDump(w,file)
Widget w;
char *file;
{
  char *dump;
  FILE *out = fopen(file,"w");
  if (out==NULL) MidasError("Could not open file %s",file);

  dump = SGMLHyperGetText(w,TRUE);
  fputs(dump,out);
  fclose(out); 
  XtFree(dump);
}
/*
 * Called whenever a new node is created as the result of some user initiated action.
 * Examples includee typing a keyword in the ISINDEX area, clicking on an ISMAP image
 * or filling in a form.
 *
 */
static MidasOperand WWWCreateNodeFromKeyword(parent,reference,keyword,method)
WWWNode *parent;
WWWNode *reference;
char *keyword;
char *method;
{
  MidasOperand Temp;
  WWWNode *src, *dest, *p; 
  WWWFile *new = CopyFile(reference->file); 
  WWWFile *pnew;
  XrmQuark Method = XrmStringToQuark(method);
  char buffer[12], *q;

  parent = parent->parent; /* Make sure we really have parent */
  pnew = CopyFile(parent->file);
  
  sprintf(buffer,"@@%d",WWWnextid++);  
  pnew->anchor = XrmStringToQuark(buffer);

  src = WWWCreateNode(pnew,parent);

  if (Method == getMethod)
    { 
      char *buffer = (char *) XtMalloc(strlen(XrmQuarkToString(new->file)) + strlen(keyword) + 5);
      strcpy(buffer,XrmQuarkToString(new->file));
      if (q = strchr(buffer,'?')) *q = '\0';
      strcat(buffer,"?");
      strcat(buffer,keyword);
      new->file = XrmStringToQuark(buffer); 
      new->anchor = NULL;
      XtFree(buffer);
    }
  else
    {
      new->keyword = XrmStringToQuark(keyword); /* bad idea, creates large number of quarks */
      new->anchor = NULL;
      new->method = Method;
    }
  dest = WWWCreateNodeAndParent(new); 
  FreeFile(new);
  FreeFile(pnew);

  src->next = parent->down;
  parent->down = src;  
  src->down = dest;

  p = dest->up; 
  for (; p != NULL; p = p->chain) if (p==src) break;
  if (p == NULL) 
    {
      src->chain = dest->up; 
      dest->up = src;
    } 
 
  Temp.Value.P = (XtPointer) src; 
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
 
  return Temp;
}
/*
 *  Called when a new anchor is created
 *  ----------------------------------- 
 */
static void WWWCreateAnchor(w,doc)
Widget w;
WWWFile *doc;
{
  char *href, *name, buffer[12];
  WWWNode *src  = NULL;
  WWWNode *dest = NULL;
  WWWFile *here = CopyFile(doc); 

  Arg arglist[10];
  int n=0;
  
  XtSetArg(arglist[n],SGMLNhref,&href); n++;
  XtSetArg(arglist[n],SGMLNname,&name); n++;
  XtGetValues(w,arglist,n); 

  /*
   *  Create a source node, if the node has no name then make
   *  one up.
   */

  if (!name || !(*name))
    {
      name = buffer;
      sprintf(buffer,"@@%d",WWWnextid++);  
    } 

  here->anchor = XrmStringToQuark(name); 

  /*
   *  Problem .. what if multiple named anchors in document have same name.
   *  solution....make up a new name.
   */  

  src = WWWFindNode(here);
  if (src)
    {
      Widget parent = XtParent(w); 
      Widget *ww = src->widgets;
      int nw = src->num_widgets;

      for ( ; !XtIsWidget(parent) ; parent = XtParent(parent) );
      for ( ; nw-- ; ww++)
        {
          Widget p2 = XtParent(*ww);
          for ( ; !XtIsWidget(p2) ; p2 = XtParent(p2) );
          if (p2 == parent) 
            {
              sprintf(buffer,"@@%d",WWWnextid++);  
              here->anchor = XrmStringToQuark(buffer);
              break; 
            } 
        }  
    }

  src = WWWCreateNode(here,WWWparent);
  FreeFile(here);
    
  if (!WWWparent->down) WWWparent->down = src;  
  if (WWWlastnode) WWWlastnode->next = src;
  src->prev = WWWlastnode;
  WWWlastnode = src; 
  WWWAttachWidgetToNode(src,w);

  n=0;
  XtSetArg(arglist[n],SGMLNuserdata,(XtPointer) src); n++;
  XtSetValues(w,arglist,n);
  
  /*
   * If this node points somewhere then create a destination node
   */  

  if (href && *href)
    { 
      WWWNode *p;  
      here = ParseFile(href,doc);

      dest = WWWCreateNodeAndParent(here);

      FreeFile(here); 

      src->down = dest;

      p = dest->up; 
      for (; p != NULL; p  = p->chain) if (p==src) break;
      if (p == NULL) 
        {
          src->chain = dest->up; 
          dest->up = src;
        } 
      if (dest->visited) WWWBeenThere(w,TRUE);
      else               WWWBeenThere(w,FALSE); 
    }
}
/*
 * It should be possible to do this directly from MIDAS so this routine
 * should go away as soon as it actually is.
 */ 
static void WWWCopyBackgroundColor(src,dest)
Widget src;
Widget dest;
{ 
  Pixel bg;
  Arg arglist[1];
 
  XtSetArg(arglist[0],XmNbackground,&bg);
  XtGetValues(src,arglist,1);
  XtSetArg(arglist[0],XmNbackground,bg);
  XtSetValues(dest,arglist,1);
}
static MidasOperand WWWSource(w)
Widget w;
{
  MidasOperand Temp;
  char *dump = SGMLHyperGetText(w,TRUE);

  Temp.Value.P = dump;
  Temp.Dynamic = TRUE;
  Temp.Type = MString;
  
  return Temp; 
}
static MidasOperand WWWBack(list)
List *list;
{
  MidasOperand Temp;

  ListItem *item = MidasFindItemInListPos(list,0);
  WWWLink *link = (WWWLink *) item?item->Pointer:NULL;
  Temp.Value.P = link ? (XtPointer) link->from : NULL;
  
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp;
}
static MidasOperand WWWPrev(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->prev : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWNext(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->next : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWParent(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->parent : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWDest(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->down : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 
} 
static MidasOperand WWWRedirectLookup(node)
WWWNode *node;
{
  MidasOperand Temp;

  for ( ; node->redirect; node = node->redirect);

  Temp.Value.P = (XtPointer) node;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 
}
static MidasOperand WWWGetNode(w)
Widget w;
{
  MidasOperand Temp;
  Arg arglist[10];
  int n=0;
  
  XtSetArg(arglist[n],SGMLNuserdata,&Temp.Value.P); n++;
  XtGetValues(w,arglist,n);

  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 
}
static MidasOperand WWWParse(file,def)
char *file;
WWWFile *def;
{
  MidasOperand Temp;
  
  WWWFile *pfile = ParseFile(file,def);

  Temp.Value.P = (XtPointer) pfile;
  Temp.Dynamic = TRUE;
  Temp.Type = "WWWFile";
       
  return Temp;
}

/*-----------------------------------------------------------------------*/
/* convert a string to lower case                                        */
/*-----------------------------------------------------------------------*/
 
static void lowcase(p)
register char *p;
{
    while(*p)
    {
        *p = tolower(*p); 
        p++;
    }
}

/*-----------------------------------------------------------------------*/
/* code for dealing with ISMAP option n IMG tag                          */
/*-----------------------------------------------------------------------*/

static MidasOperand WWWIsMap(w)
Widget w;
{
  MidasOperand Temp;

  Temp.Value.P = "";
  Temp.Dynamic = FALSE;
  Temp.Type = MString;    

  if (SGMLIsMarkerText(w))
    {
      char *string;
      Arg arglist[1];
      XtSetArg(arglist[0],SGMLNparam,&string);
      XtGetValues(w,arglist,1);

      lowcase(string);

      if (strstr(string," ismap")) /* Should be done better!!!! */
        {
          char buffer[12];
          SGMLMarkerTextCallbackStruct *cb = (SGMLMarkerTextCallbackStruct *) MidasGetActiveCallback();
          sprintf(buffer,"%d,%d",cb->x,cb->y);

          Temp.Value.P = XtNewString(buffer);
          Temp.Dynamic = TRUE;
        } 
    }
  return Temp;
}

/*
 * Finds and returns a named parameter from a parameter string
 * Note: Duplicates code in SGMLHyper.c   .. should be tidied up
 */
static MidasOperand WWWParam(w,name)
Widget w;
char *name;
{
  Arg arglist[1];
  char *string;
  MidasOperand Temp;
  int l;
  char *p, *q, *r;
  char *copy; 
  char *whiteSpace = " \t\n\r";
  char *whiteSpaceOrEqual = " \t=\n\r";
  Boolean found = FALSE, done; 
 
  XtSetArg(arglist[0],SGMLNparam,&string);
  XtGetValues(w,arglist,1);
 
  if (!string || *string =='\0') 
    { 
      Temp.Type = MString;
      Temp.Value.P = "";
      Temp.Dynamic = FALSE;
      return Temp;
    }
      
  r = copy = XtNewString(string);

  for (;;)
    { 
       p = r + strspn(r,whiteSpace);
       q = p + strcspn(p,whiteSpaceOrEqual);
       r = q + strspn(q,whiteSpace);

       if (*r == '\0') break;
                
       *q = '\0';
       if (*p != '"') lowcase(p);
       else 
         {
           p++;
           if (*(q-1)=='"') *(q-1) = '\0'; 
         }  

      found = !strcmp(p,name);         

      r++;
      r +=  strspn(r,whiteSpace);
      if (*r == '"') 
        { 
          r++;
          l = strcspn(r,"\"");
        }     
      else
        {
          l = strcspn(r,whiteSpace);
        }
      p = r + l;
      done = (*p == '\0');
      if (!done) *p = '\0';
       
      if (found) break;
      r = p + 1;  
      if (done) break;
    }          

  Temp.Type = MString;

  if (found)
    {
      Temp.Value.P = XtNewString(r);  
      Temp.Dynamic = TRUE;
    }
  else
    {
      Temp.Value.P = "";
      Temp.Dynamic = FALSE;
    }
   XtFree(copy);
   return Temp;
} 
/* 
 *  Converters
 *  ----------
 */
static Boolean WWWConvertStringFile(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  WWWFile *pfile = ParseFile(In->Value.P,&WWWFileDefault);  

  Out->Value.P = (XtPointer) pfile;
  Out->Dynamic = TRUE;
       
  return TRUE;
}
static Boolean WWWConvertStringNode(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  WWWFile *pfile = ParseFile(In->Value.P,&WWWFileDefault);  
  WWWNode *node = WWWCreateNodeAndParent(pfile);
  
  FreeFile(pfile); 
 
  Out->Value.P = (XtPointer) node;
  Out->Dynamic = FALSE;
       
  return TRUE;
}
static Boolean WWWConvertFileNode(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  if (In->Value.P)
    {
      WWWNode *node = WWWCreateNodeAndParent((WWWFile *) In->Value.P);  
      Out->Value.P = (XtPointer) node;
    }
  else Out->Value.P = NULL;   
  Out->Dynamic = FALSE;
       
  return TRUE;
}
static Boolean WWWConvertNodeFile(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  WWWNode *node = (WWWNode *) In->Value.P;
 
  Out->Value.P = node ? (XtPointer) node->file : NULL;
  Out->Dynamic = FALSE;
       
  return TRUE;
}
static Boolean WWWConvertPageString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
  if      (In->Value.I ==   0) Out->Value.P = "Portrait";
  else if (In->Value.I ==  90) Out->Value.P = "Landscape"; 
  else if (In->Value.I == 180) Out->Value.P = "Upside Down"; 
  else if (In->Value.I == 270) Out->Value.P = "Seascape"; 
  else return FALSE;

  Out->Dynamic = FALSE;
  return TRUE;
}
static Boolean WWWConvertNodeString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
  WWWNode *node = (WWWNode *) In->Value.P;
  WWWFile *file = node ? node->file : NULL;
  if (file)
    {
      char *p = WWWAsciiFile(file);

      Out->Value.P = (XtPointer) p;
      Out->Dynamic = TRUE;
    }
  else
    {
      Out->Value.P = (XtPointer) "0";
      Out->Dynamic = FALSE;
    } 
  return TRUE;
}
static Boolean WWWConvertFileString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
  WWWFile *pfile = (WWWFile *) In->Value.P;
  char *p = WWWAsciiFile(pfile);  

  Out->Value.P = (XtPointer) p;
  Out->Dynamic = TRUE;
       
  return TRUE;
}
static void WWWSetGateway(gateway)
char *gateway;
{
  WWWGateway = ParseFile(gateway,&WWWFileDefault);
}
static void WWWSetWaitingTime(time)
int time;
{
  appResources.waiting_time_out = time; 
}
static void WWWSetUpdateTime(time)
int time;
{
  appResources.update_time_out = time; 
}   
static void WWWSetHTTPLevel(level)
char *level;
{
  appResources.default_HTTP_protocol = XtNewString(level); 
}
static MidasOperand WWWGetWaitingTime()
{
  MidasOperand Temp;

  Temp.Value.I = appResources.waiting_time_out; 
  Temp.Dynamic = FALSE;
  Temp.Type = MInt;

  return Temp;
}
static MidasOperand WWWGetUpdateTime()
{
  MidasOperand Temp;

  Temp.Value.I = appResources.update_time_out; 
  Temp.Dynamic = FALSE;
  Temp.Type = MInt;

  return Temp; 
}    
static MidasOperand WWWGetHTTPLevel()
{
  MidasOperand Temp;

  Temp.Value.P = (XtPointer) appResources.default_HTTP_protocol; 
  Temp.Dynamic = FALSE;
  Temp.Type = MString;

  return Temp; 
}
static void WWWInclude(include,w,node)
List *include;
Widget w;
WWWNode *node;
{
  /*
   * Maybe we fetched this before and its all done??
   */
  if (node->icon)
    {
      unsigned int depth;
      Arg arglist[1];

      XtSetArg(arglist[0],SGMLNpixmap,node->icon);
      XtSetValues(w,arglist,1);

      XtSetArg(arglist[0],SGMLNpixmapDepth,&depth);
      XtGetValues(w,arglist,1); 
  
      if (depth == 1) 
        {
          XtSetArg(arglist[0],SGMLNoutline,0);
          XtSetValues(w,arglist,1);
        } 
    }
  else if (node->picture)
    {
      Arg arglist[1];

      XtSetArg(arglist[0],SGMLNimage,node->picture);
      XtSetValues(w,arglist,1);
    }
  /* 
   * Otherwise add it to the list of files to be fetched
   */
  else 
    {
       ImageLink *il = XtNew(ImageLink);
       ListItem *item = MidasAddItemToListPos(include,NULL,0);   

       item->Pointer = il;
       il->Widget = w;
       il->Node = node;  
    }
}
MidasOperand WWWTempFile()
{
  MidasOperand Temp;
#ifndef VMS
  char buffer[L_tmpnam];
  char *tmpn = tmpnam(buffer);
#else
  char *tmpn = "sys$scratch:midaswww.tmp;"; /* Semi-colon needed so delete command will work */
#endif

  Temp.Value.P = XtNewString(tmpn);
  Temp.Dynamic = TRUE;
  Temp.Type = MString;

  return Temp;
}
/*
 * This routine is used to encode a search string
 * ASSUMES ascii character set
 */
static MidasOperand WWWEncode(search)
char *search;
{
  static Boolean isAcceptable[96] =

  /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
  {    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	 /* 2x   !"#$%&'()*+,-./	 */
       1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	 /* 3x  0123456789:;<=>?	 */
       1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	 /* 4x  @ABCDEFGHIJKLMNO         */
       1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	 /* 5X  PQRSTUVWXYZ[\]^_	 */
       0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	 /* 6x  `abcdefghijklmno	 */
       1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };/* 7X  pqrstuvwxyz{\}~	DEL 	 */

  char *result = XtMalloc(3*strlen(search) + 1);
  char *p, *s, *e, *q;
  MidasOperand Temp;

  for (s = search ; *s && isspace(*s); s++);
  for (e = s + strlen(s) ; e>s && isspace(*(e-1)) ; e--);  
  for (q = result , p = s ; p<e ; p++ )
    {
      int c = (int) *p; 
      if      (isspace(c)) *q++ = '+';
      else if (c>= 32  && c<= 127 && isAcceptable[c-32]) *q++ = (char) c;
      else { sprintf(q,"%%%2x",c); q += 3; }
    }   
  *q = '\0';

  Temp.Dynamic = TRUE;
  Temp.Value.P = (XtPointer) result;  
  Temp.Type = MString;
  return Temp;
}
static void WWWRegisterWindow(w,delete)
Widget w;
Boolean delete; /* If true delete the entry associated with this window */ 
{
  Display *dpy = XtDisplay(w);
  Window window = XtWindow(w);    
  int scrno = XScreenNumberOfScreen(XtScreen(w));
  Window root = XRootWindow(dpy,scrno);
  Atom actualType;
  unsigned long numItems, bytesLeft;
  int actualFormat, rc;
  Window *windowList;
  static List *registerList = NULL;

  /* 
   * To allow Midas WWW to be controlled by a remote application we put a signpost 
   * on the root window. The name of the signpost is controlled by the signpostName
   * resource. The signpost property contains a list of window ids of currently
   * active MidasWWW windows. Each of these windows has an attached property that allows
   * them to be controlled by another process.
   */

  XGrabServer(dpy); /* Can't allow intervening changes to property */
  rc = XGetWindowProperty(dpy,root,appResources.signpost,0,100,FALSE,XA_WINDOW,
                          &actualType,&actualFormat,&numItems,&bytesLeft,(unsigned char **)&windowList);

  if (rc == Success) 
    {
      int i, found = -1;
      for (i=0 ; i < numItems; i++) if (windowList[i] == window) found = i;

      if (found >= 0 && delete) 
        {
          numItems--; 
          for (i = found  ; i < numItems; i++) windowList[i] = windowList[i+1];
        }
      else if (found < 0 && !delete)
        { 
          Window *newWindowList = (Window *) XtMalloc(sizeof(Window) * (numItems + 1));
          for (i=0 ; i < numItems; i++) newWindowList[i] = windowList[i];
          newWindowList[numItems++] = window;
          XFree((char *) windowList);
          windowList = newWindowList;
          XtAddCallback(w,XtNpopdownCallback,(XtCallbackProc) WWWRegisterWindow,(XtPointer) TRUE);
        }
      else goto done;
    }
  else  
    {
      if (delete) goto done;
      numItems = 1;
      windowList = &window;
      XtAddCallback(w,XtNpopdownCallback,(XtCallbackProc) WWWRegisterWindow,(XtPointer) TRUE);
    }

  if (numItems) XChangeProperty(dpy,root,appResources.signpost,
                   XA_WINDOW,32,PropModeReplace,(unsigned char *)windowList,numItems);
  else XDeleteProperty(dpy,root,appResources.signpost);
done:
  XUngrabServer(dpy);
  XFlush(dpy);
  if (rc == Success) XtFree((char *) windowList);
}
static void WWWInit(shell)
Widget shell;
{
  XtResource *fResources;
  ListItem *filetype, *usertype;
  int i,j,n = 0;
  char **cleanup;
  WWWFiletype *types;
  static char bits[] = {0x01, 0x02};
  
  XtGCMask valueMask = GCBackground | GCForeground | GCFont | GCLineWidth | GCFillStyle | GCStipple;  
  XGCValues values;  

  circlePixmap = MidasFetchIcon("circle",shell); 
 
  XtInitializeWidgetClass(sGMLTextObjectClass); /* Needed to install String to Quark converter */

  XtGetApplicationResources(shell,(XtPointer) &appResources,resources,XtNumber(resources),NULL,0);
  if (!appResources.filetypes) appResources.filetypes = MidasCreateEmptyList("FileList");
  if (!appResources.mimetypes) appResources.mimetypes = MidasCreateEmptyList("MIMEList");
  if (!appResources.mimeencode) appResources.mimeencode = MidasCreateEmptyList("MIMEEncodeList");
  if (!appResources.usertypes) appResources.usertypes = MidasCreateEmptyList("UsertypesList");
  if (!appResources.usermimetypes) appResources.usermimetypes = MidasCreateEmptyList("UserMIMEList");
  if (!appResources.usermimeencode) appResources.usermimeencode = MidasCreateEmptyList("UserMIMEEncodeList");

  values.foreground = appResources.foreground; 
  values.background = appResources.background; 
  values.line_width = 2;
  values.fill_style = FillStippled;
  values.stipple = XCreateBitmapFromData(XtDisplay(shell),XtWindow(shell),bits,2,2);
  values.font = appResources.font->fid; 
  messageGC =  XtGetGC (shell, valueMask, &values);

  /* Merge the usertype to the filetypes */ 

  for (usertype = appResources.usertypes->First; usertype; usertype = usertype->Next)
    {
      if (!MidasFindItemInList(appResources.filetypes,usertype->Entry))
        MidasAddItemToList(appResources.filetypes,usertype->Entry);
    }

  for (filetype = appResources.filetypes->First; filetype; filetype = filetype->Next) 
    {
       n++;
    }

  fResources = (XtResource *) XtMalloc(sizeof(XtResource) * n * 7);
  cleanup = (char **) XtMalloc(sizeof(char *) * n * 7);
  types = (WWWFiletype *) XtMalloc(sizeof(WWWFiletype) * n); 
 
  for (i = 0 , j = 0, filetype = appResources.filetypes->First; filetype; j++, i++ , filetype = filetype->Next) 
    {
      char *name1 = XtMalloc(strlen(filetype->Entry) + 25);
      char *name2 = XtMalloc(strlen(filetype->Entry) + 25);
      char *name3 = XtMalloc(strlen(filetype->Entry) + 25);
      char *name4 = XtMalloc(strlen(filetype->Entry) + 25);
      char *name5 = XtMalloc(strlen(filetype->Entry) + 25);
      char *name6 = XtMalloc(strlen(filetype->Entry) + 25);
      char *name7 = XtMalloc(strlen(filetype->Entry) + 25);

      filetype->Pointer = types + j;

      strcpy(name1,"filetype_");
      strcat(name1,filetype->Entry);
      strcpy(name2,name1);
      strcat(name2,"_icon");
      strcpy(name3,name1);
      strcat(name3,"_filter");
      strcpy(name4,name1);
      strcat(name4,"_binary");
      strcpy(name5,name1);
      strcat(name5,"_destination");
      strcpy(name6,name1);
      strcat(name6,"_tempfile");
      strcpy(name7,name1);
      strcat(name7,"_shellflag");
      strcat(name1,"_command");

      fResources[i].resource_name   = name1; 
      fResources[i].resource_class  = "Command"; 
      fResources[i].resource_type   = XtRString; 
      fResources[i].resource_size   = sizeof(char *);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,Command) + j*sizeof(WWWFiletype);  
      fResources[i].default_type    = XtRImmediate;
      fResources[i].default_addr    = (XtPointer) NULL;
      cleanup[i] = name1;
      i++;
      fResources[i].resource_name   = name2; 
      fResources[i].resource_class  = "IconName"; 
      fResources[i].resource_type   = XtRString; 
      fResources[i].resource_size   = sizeof(char *);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,IconName) + j*sizeof(WWWFiletype);  
      fResources[i].default_type    = XtRString;
      fResources[i].default_addr    = (XtPointer) "document";
      cleanup[i] = name2;
      i++;
      fResources[i].resource_name   = name3; 
      fResources[i].resource_class  = "Filter"; 
      fResources[i].resource_type   = XtRBoolean; 
      fResources[i].resource_size   = sizeof(Boolean);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,Filter) + j*sizeof(WWWFiletype);  
      fResources[i].default_type    = XtRImmediate;
      fResources[i].default_addr    = (XtPointer) FALSE;
      cleanup[i] = name3;
      i++;
      fResources[i].resource_name   = name4; 
      fResources[i].resource_class  = "Binary"; 
      fResources[i].resource_type   = XtRBoolean; 
      fResources[i].resource_size   = sizeof(Boolean);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,Binary) + j*sizeof(WWWFiletype);  
      fResources[i].default_type    = XtRImmediate;
      fResources[i].default_addr    = (XtPointer) FALSE;
      cleanup[i] = name4;
      i++;
      fResources[i].resource_name   = name5; 
      fResources[i].resource_class  = "Destination"; 
      fResources[i].resource_type   = "Quark"; 
      fResources[i].resource_size   = sizeof(XrmQuark);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,Destination) + j*sizeof(WWWFiletype);  
      fResources[i].default_type    = XtRImmediate;
      fResources[i].default_addr    = (XtPointer) NULL;
      cleanup[i] = name5;
      i++;
      fResources[i].resource_name   = name6;
      fResources[i].resource_class  = "Tempfile";
      fResources[i].resource_type   = XtRBoolean;
      fResources[i].resource_size   = sizeof(Boolean);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,Tempfile) + j*sizeof(WWWFiletype);
      fResources[i].default_type    = XtRImmediate;
      fResources[i].default_addr    = (XtPointer) FALSE;
      cleanup[i] = name6;
      i++;
      fResources[i].resource_name   = name7;
      fResources[i].resource_class  = "Shellflag";
      fResources[i].resource_type   = XtRBoolean;
      fResources[i].resource_size   = sizeof(Boolean);
      fResources[i].resource_offset = XtOffset(WWWFiletypePtr,Shellflag) + j*sizeof(WWWFiletype);
      fResources[i].default_type    = XtRImmediate;
      fResources[i].default_addr    = (XtPointer) TRUE;
      cleanup[i] = name7;
      
      types[j].UserData = NULL;
    }

  XtGetApplicationResources(shell,(XtPointer)types,fResources,7*n,NULL,0);
  XtFree((char *)fResources);
  for (i = 0; i < n*7; i++) XtFree(cleanup[i]);
  XtFree((char *)cleanup);

  appResources.ftpDefaultFiletype    = NULL;
  appResources.localDefaultFiletype  = NULL;
  appResources.gopherDefaultFiletype = NULL;
  appResources.httpDefaultFiletype   = NULL;
  appResources.defaultFiletype       = NULL;

  for (filetype = appResources.filetypes->First; filetype; filetype = filetype->Next) 
    {
       WWWFiletype *temp = (WWWFiletype *) filetype->Pointer;

       if      (!strcmp(filetype->Entry,"$ftp"    )) appResources.ftpDefaultFiletype    = temp;
       else if (!strcmp(filetype->Entry,"$local"  )) appResources.localDefaultFiletype  = temp;
       else if (!strcmp(filetype->Entry,"$gopher" )) appResources.gopherDefaultFiletype = temp;
       else if (!strcmp(filetype->Entry,"$http"   )) appResources.httpDefaultFiletype   = temp;
       else if (!strcmp(filetype->Entry,"$default")) appResources.defaultFiletype       = temp;
    }

  if (!appResources.defaultFiletype)
    {
      WWWFiletype *Default = XtNew(WWWFiletype);
      appResources.defaultFiletype = Default;

      Default->IconName= "document";
      Default->Command = NULL;
      Default->Binary  = FALSE;
      Default->Filter  = FALSE;
      Default->Tempfile = FALSE;
      Default->Shellflag = TRUE;
      Default->Destination = textDestination;
      Default->UserData = NULL;

      filetype = MidasAddItemToList(appResources.filetypes,"default");
      filetype->Pointer = Default; 
    }
  if (!appResources.ftpDefaultFiletype   ) appResources.ftpDefaultFiletype    = appResources.defaultFiletype;
  if (!appResources.localDefaultFiletype ) appResources.localDefaultFiletype  = appResources.defaultFiletype;
  if (!appResources.gopherDefaultFiletype) appResources.gopherDefaultFiletype = appResources.defaultFiletype;
  if (!appResources.httpDefaultFiletype  ) appResources.httpDefaultFiletype   = appResources.defaultFiletype;

}
/*
 * Initialization
 */ 
void WWWMidasInit(argc,argv)
int argc;
char *argv[];
{
  int i;
  char hostname[32];
  struct hostent *h;
  WWWFile *pfile; 

  strcpy(password,cuserid(NULL));
  gethostname(hostname,32);
  h = gethostbyname(hostname);
  strcat(password,"@");
  strcat(password,h->h_name);
  
  for (i=0; i < HASHSIZE; i++) HashTable[i] = NULL; 

  localProtocol   = XrmPermStringToQuark("local");
  httpProtocol    = XrmPermStringToQuark("http");
  gopherProtocol  = XrmPermStringToQuark("gopher");
  ftpProtocol     = XrmPermStringToQuark("ftp");
  fileProtocol    = XrmPermStringToQuark("file");
  telnetProtocol  = XrmPermStringToQuark("telnet");
  historyProtocol = XrmPermStringToQuark("history");
  telnetProtocol  = XrmPermStringToQuark("telnet");
  ingotProtocol   = XrmPermStringToQuark("ingot"); 
  waisProtocol    = XrmPermStringToQuark("wais"); 

  getMethod       = XrmPermStringToQuark("get");  
  postMethod      = XrmPermStringToQuark("post");  

  htmlDestination = XrmPermStringToQuark("html");
  psDestination   = XrmPermStringToQuark("ps");
  gifDestination  = XrmPermStringToQuark("gif");
  textDestination = XrmPermStringToQuark("text");
  fileDestination = XrmPermStringToQuark("file");
  xbmDestination  = XrmPermStringToQuark("xbm"); 
  multimediaDestination = XrmPermStringToQuark("multimedia");
  unsupportDestination = XrmPermStringToQuark("unsupport");

  WWWFileDefault.protocol = localProtocol;
  WWWFileDefault.port   = 0;
  WWWFileDefault.node   = NULL; 
  WWWFileDefault.file   = NULL; 
  WWWFileDefault.anchor = NULL; 
  WWWFileDefault.method = getMethod; 

  pfile = ParseFile("history:",&WWWFileDefault);  
  historyNode = WWWCreateNodeAndParent(pfile);
  FreeFile(pfile); 

/*
 * MIDAS BUG confusion between name and String
 */

  MidasDeclareConverter("String","WWWFile"        ,WWWConvertStringFile);
  MidasDeclareConverter("name","WWWFile"          ,WWWConvertStringFile);
  MidasDeclareConverter("String","WWWNode"        ,WWWConvertStringNode);
  MidasDeclareConverter("name","WWWNode"          ,WWWConvertStringNode);
  MidasDeclareConverter("WWWFile","String"        ,WWWConvertFileString);
  MidasDeclareConverter("WWWFile","name"          ,WWWConvertFileString);
  MidasDeclareConverter("WWWFile","WWWNode"       ,WWWConvertFileNode);
  MidasDeclareConverter("WWWNode","WWWFile"       ,WWWConvertNodeFile);
  MidasDeclareConverter("WWWNode","String"        ,WWWConvertNodeString); 
  MidasDeclareConverter("PageOrientation","String",WWWConvertPageString); 

  MidasDeclareVerb("WWW INIT Widget"              ,WWWInit);
  MidasDeclareVerb("WWW SET GATEWAY name"         ,WWWSetGateway);
  MidasDeclareVerb("WWW SET TITLE WWWNode name"   ,WWWSetTitle);
  MidasDeclareVerb("WWW REGISTER Widget Boolean"  ,WWWRegisterWindow);
  MidasDeclareVerb("WWW DUMP Widget name"         ,WWWDump); 
  MidasDeclareVerb("WWW ADD HISTORY list name"    ,WWWAddHistory); 
  MidasDeclareVerb("WWW REDIRECT WWWNode WWWNode" ,WWWRedirect); 
  MidasDeclareVerb("WWW DELETE HISTORY list"      ,WWWDeleteHistory); 
  MidasDeclareVerb("WWW GET Widget WWWNode list name"  
                                                  ,WWWGet); 
  MidasDeclareVerb("WWW ZAP Widget WWWNode"       ,WWWZapNode); 
  MidasDeclareVerb("WWW LINK {WWWNode}"           ,WWWLinkHistory);
  MidasDeclareVerb("WWW PUT Widget name"          ,WWWPut); 
  MidasDeclareVerb("WWW ANCHOR Widget WWWFile"    ,WWWCreateAnchor); 
  MidasDeclareVerb("WWW COPYB Widget Widget"      ,WWWCopyBackgroundColor); 
  MidasDeclareVerb("WWW NEXT PAGE Widget"         ,GhostviewNextPage); 
  MidasDeclareVerb("WWW FIRST PAGE Widget"        ,GhostviewEnableInterpreter); 
  MidasDeclareVerb("WWW INCLUDE list Widget WWWNode"
                                                  ,WWWInclude);
  MidasDeclareVerb("WWW RESET"                    ,WWWReset);
  MidasDeclareVerb("WWW SEARCH Widget name Boolean Boolean"
                                                  ,WWWSearch);

  MidasDeclareVerb("WWW SET WAITING TIME Int"     ,WWWSetWaitingTime);
  MidasDeclareVerb("WWW SET UPDATE TIME Int"      ,WWWSetUpdateTime);
  MidasDeclareVerb("WWW SET HTTP LEVEL name"      ,WWWSetHTTPLevel);
  MidasDeclareFunction("WWWWAITINGTIME()"         ,WWWGetWaitingTime); 
  MidasDeclareFunction("WWWUPDATETIME()"          ,WWWGetUpdateTime); 
  MidasDeclareFunction("WWWHTTPLEVEL()"           ,WWWGetHTTPLevel);

  MidasDeclareFunction("WWWTEMPFILE()"            ,WWWTempFile);
  MidasDeclareFunction("WWWPARSE(name,WWWFile)"   ,WWWParse); 
  MidasDeclareFunction("WWWSOURCE(Widget)"        ,WWWSource); 
  MidasDeclareFunction("WWWBACK(list)"            ,WWWBack); 
  MidasDeclareFunction("WWWPREV(WWWNode)"         ,WWWPrev); 
  MidasDeclareFunction("WWWNEXT(WWWNode)"         ,WWWNext); 
  MidasDeclareFunction("WWWPARENT(WWWNode)"       ,WWWParent); 
  MidasDeclareFunction("WWWDEST(WWWNode)"         ,WWWDest); 
  MidasDeclareFunction("WWWREDIRECT(WWWNode)"     ,WWWRedirectLookup); 
  MidasDeclareFunction("WWWNODE(Widget)"          ,WWWGetNode);
  MidasDeclareFunction("WWWPARAM(Widget,name)"    ,WWWParam);
  MidasDeclareFunction("WWWISMAP(Widget)"         ,WWWIsMap); 
  MidasDeclareFunction("WWWENCODE(name)"          ,WWWEncode);
  MidasDeclareFunction("WWWCREATENODEFROMKEYWORD(WWWNode,WWWNode,name,name)"
                                                  ,WWWCreateNodeFromKeyword);

  /* Enable tear off-menu's (doesnt work yet) */
   
#if XmVersion >= 1002
/*  XmRepTypeInstallTearOffModelConverter(); */
#endif
}
