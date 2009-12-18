/*
 *  These are the WWW - Midas Interface routines
 */ 



#include "midasnet.h"

#include <string.h>

extern XtAppContext appl_context;
extern XrmQuark fileDestination;
extern XrmQuark htmlDestination;
extern XrmQuark textDestination;
extern XrmQuark psDestination;
extern XrmQuark gifDestination;
extern XrmQuark xbmDestination;
extern XrmQuark unsupportDestination;

#define DEADNODE ((WWWNode *) -1)
extern WWWNode *WWWFromLink;
extern char password[64];
extern void ClearFlag();

extern char *WWWAsciiFile();
void WWWAddReadInputEvent();
static int WWWUseHTTPVersion1();

static char *htrq = NULL;

char *WWWGetHTRQ()
{
  int size = 512;
  char *accept = "Accept: ";
  char *accept_encode = "Accept-Encoding: ";
  char *from = "From: ";
  char useragent[100];
  char *header = XtMalloc(size);
  ListItem *i = appResources.mimetypes->First;
  ListItem *j = appResources.mimeencode->First;
  ListItem *k = appResources.usermimetypes->First;
  ListItem *l = appResources.usermimeencode->First;

  sprintf(useragent,"User-Agent: MidasWWW/%s\r\n",VERSION);

  *header = '\0';
  for (; i ; i = i->Next)
    {
       if (strlen(header) > size - 128)
         header = XtRealloc(header, size += 128);
       strcat(header,accept);
       strcat(header,i->Entry);
       strcat(header,"\r\n");
     }
  for (;k ; k = k->Next)
    {
      if (MidasFindItemInList(appResources.mimetypes,k->Entry)) continue;
      if (strlen(header) > size - 128)
         header = XtRealloc(header, size += 128);
       strcat(header,accept);
       strcat(header,k->Entry);
       strcat(header,"\r\n");
    }
  for(; j ; j = j->Next)
    {
       if (strlen(header) > size - 128)
         header = XtRealloc(header, size += 128);
       strcat(header,accept_encode);
       strcat(header,j->Entry);
       strcat(header,"\r\n");
     }
  for (;l ; l = l->Next)
    {
      if (MidasFindItemInList(appResources.mimeencode,l->Entry)) continue;
      if (strlen(header) > size - 128)
         header = XtRealloc(header, size += 128);
       strcat(header,accept_encode);
       strcat(header,l->Entry);
       strcat(header,"\r\n");
    }

  header = XtRealloc(header, size += 128);
  strcat(header,from);
  strcat(header,password);
  strcat(header,"\r\n");
  strcat(header,useragent);
  return header;
}

Widget WWWPostDocumentHTTP1(w,file,mb)
Widget        w;
WWWFile      *file;
MessageBlock *mb;
{
  char *command, *copy, name[128];
  Widget result = NULL;
  DataSource data;
  int s, http1 = -1, fd = 0;
  WWWFiletype *filetype = NULL;  
  char *tmpf;
  char *referer, post[256];
  ListItem *item;
  char buffer[8096];
  char *idata = XrmQuarkToString(file->keyword);

  if (file->node == NULL)
    {
      strcat(mb->message,"No node specified for HTTP connection");
      mb->help_code = "http_no_node";
      return NULL; 
    }   
  if (file->port == 0) file->port = appResources.default_HTTP_port;

  s = TCPIPConnect(file,mb);
  if (s == 0) return result;

  /* use static so that we don't need to waste time to get it each time */

  if (htrq == NULL) htrq = WWWGetHTRQ();

  if (WWWFromLink != DEADNODE && WWWFromLink != NULL)
    {
      char *fromurl;
      char *refer = "Referer: ";
      fromurl = WWWAsciiFile(WWWFromLink->file); 
      referer = XtMalloc(strlen(fromurl)+30);
      strcpy(referer,refer);
      strcat(referer,fromurl);
      strcat(referer,"\r\n");
      XtFree(fromurl);
    }
  else referer = NULL;

  sprintf(post,"Content-type: application/x-www-form-urlencoded\r\nContent-length: %d\r\n\r\n",strlen(idata));

  command = XtMalloc(30 + strlen(htrq) + strlen(referer) + strlen(post) + (file->file ? strlen(XrmQuarkToString(file->file)) : 0));

  strcpy(command,"POST ");
  strcat(command,file->file ? XrmQuarkToString(file->file): "/"); 
  strcat(command," HTTP/1.0\r\n");

  strcat(command, htrq);
  if (referer) 
    {
      strcat(command,referer);
      XtFree(referer);
    } 
  strcat(command,post);

  if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
  XtFree(command);
   
  if (!TCPIPWriteAndWaitForResponse(s,file,mb,idata)) goto cleanup;

  WWWInitDataSource(&data,mb,buffer,sizeof(buffer),socket_read,s);

  http1 = WWWUseHTTPVersion1(&data,filetype,mb,name);
  if (mb->flag == 1) goto cleanup;
  
  data.read = 0; /* Some (bad) Load routines expect this */

  if (http1 == 1) 
    {
      filetype = WWWGetFiletype(name,appResources.httpDefaultFiletype);

      if (filetype->Destination == fileDestination)
        {
          /* First we need to ask the user where he wants to save this thing??
           */
          int managed = 0; 

          copy = strrchr(XrmQuarkToString(file->file), '/');
          if (copy) copy = XtNewString(copy+1);
          else      copy = XtNewString(XrmQuarkToString(file->file));

          if (copy && filetype->Filter)
            {
              char *ext = strrchr(copy,'.');
              if (ext) *ext = '\0';
             }
          filetype->UserData = copy;

          /* In case the time asking the user for file destination is  */
          /* longer than the time socket buffer is filled up, therefore */
          /* we write data to temp file and then re-load it back.       */ 

          if (!WWWWriteToTempfile(mb,GetCharacter,&data,&tmpf,".tar")) goto cleanup;
          if ((fd = open(tmpf,O_RDONLY,0)) < 0)
            {
              sprintf(mb->message,"Cannot open file <p>Reason: %s",strerror(errno));
              mb->help_code = "file_open_failed";
              goto cleanup;
            }
          data.socket = fd;

          *mb->message = '\0';
          if (managed = XtIsManaged(mb->widget)) XtUnmanageChild(mb->widget); 
          if (!WWWAskForFileDestination(w,mb,filetype,FALSE)) goto cleanup;
          if (managed) XtManageChild(mb->widget);  
        }
    }
  else  /* http1 < 0 : either the client error or server error */
    {
      if (http1 == 0)
        {
          strcat(mb->message,"Bad HTTP response from POST command");
          mb->help_code = "http_bad_post";
        }
      goto cleanup;
    }

  result = WWWLoadByFiletype(w,mb,filetype,GetCharacter,&data);

cleanup:
  
  if (filetype) WWWFreeFiletype(filetype);
  if (s) socket_close(s);
  if (fd) 
    {
      unlink(tmpf); 
      XtFree(tmpf);
    }
  return result;
}
/*
 * Fetch the document using TCP/IP and HTTP
 * ----------------------------------------
 */
Widget WWWFetchDocumentHTTP1(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{
  char *command, *copy, name[128];
  Widget result = NULL;
  DataSource data;
  int s, http1 = -1, fd = 0;
  WWWFiletype *filetype = NULL;  
  char *tmpf;
  char referer[256];
  ListItem *item;
  static List *http0List = NULL; 
  char buffer[8096];

  if (file->node == NULL)
    {
      strcat(mb->message,"No node specified for HTTP connection");
      mb->help_code = "http_no_node";
      return NULL; 
    }   
  if (file->port == 0) file->port = appResources.default_HTTP_port;

  if (http0List)
    {
      sprintf(name,"%s:%d",XrmQuarkToString(file->node),file->port);
      item = MidasFindItemInList(http0List,name);
      if (item) return WWWFetchDocumentHTTP(w,file,mb); 
    }

  s = TCPIPConnect(file,mb);
  if (s == 0) return result;

  /* use static so that we don't need to waste time to get it each time */
  if (htrq == NULL) htrq = WWWGetHTRQ();

  if (WWWFromLink != DEADNODE && WWWFromLink != NULL)
    {
      char *fromurl;
      char *refer = "Referer: ";
      fromurl = WWWAsciiFile(WWWFromLink->file); 
      strcpy(referer,refer);
      strcat(referer,fromurl);
      strcat(referer,"\r\n\r\n");
      XtFree(fromurl);
    }
  else *referer = '\0';

  command = XtMalloc(20 + strlen(htrq) + strlen(referer) + (file->file ? strlen(XrmQuarkToString(file->file)) : 0));

  strcpy(command,"GET ");
  strcat(command,file->file ? XrmQuarkToString(file->file): "/"); 
  strcat(command," HTTP/1.0\r\n");

  strcat(command, htrq);
  if (*referer)
    strcat(command,referer);
  else 
    strcat(command,"\r\n");

  if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
  XtFree(command);
 
  WWWInitDataSource(&data,mb,buffer,sizeof(buffer),socket_read,s);

  http1 = WWWUseHTTPVersion1(&data,filetype,mb,name);
  if (mb->flag == 1) goto cleanup;
  
  data.read = 0; /* Some (bad) Load routines expect this */

  if (http1 == 1) 
    {
      filetype = WWWGetFiletype(name,appResources.httpDefaultFiletype);

      if (filetype->Destination == fileDestination)
        {
          /* First we need to ask the user where he wants to save this thing??
           */
          int managed = 0; 

          copy = strrchr(XrmQuarkToString(file->file), '/');
          if (copy) copy = XtNewString(copy+1);
          else      copy = XtNewString(XrmQuarkToString(file->file));

          if (copy && filetype->Filter)
            {
              char *ext = strrchr(copy,'.');
              if (ext) *ext = '\0';
             }
          filetype->UserData = copy;

          /* In case the time asking the user for file destination is  */
          /* longer than the time socket buffer is filled up, therefore */
          /* we write data to temp file and then re-load it back.       */ 

          if (!WWWWriteToTempfile(mb,GetCharacter,&data,&tmpf,".tar")) goto cleanup;
          if ((fd = open(tmpf,O_RDONLY,0)) < 0)
            {
              sprintf(mb->message,"Cannot open file <p>Reason: %s",strerror(errno));
              mb->help_code = "file_open_failed";
              goto cleanup;
            }
          data.socket = fd;

          *mb->message = '\0';
          if (managed = XtIsManaged(mb->widget)) XtUnmanageChild(mb->widget); 
          if (!WWWAskForFileDestination(w,mb,filetype,FALSE)) goto cleanup;
          if (managed) XtManageChild(mb->widget);  
        }
    }
  else if (http1 == 0)
    {
      /* if the server does not understand HTTP/1.0, then use HTTP/0.9 */
      /*
       * Experience shows that if the server fails to identify itself as
       * HTTP/1.0 compatible, then best course of action is to close the 
       * socket and try again with HTTP/0.9 (otherwise various problems can arise).
       *
       * We cache the names of such servers so that future connections can use
       * HTTP/0.9 immediately. 
       */

      socket_close(s);
  
      if (!http0List) http0List = MidasCreateEmptyList("http0List");
      sprintf(name,"%s:%d",XrmQuarkToString(file->node),file->port);
      MidasAddItemToList(http0List,name);

      return WWWFetchDocumentHTTP(w,file,mb);
    } 
  else  /* http1 < 0 : either the client error or server error */
    {
      goto cleanup;
    }

  result = WWWLoadByFiletype(w,mb,filetype,GetCharacter,&data);

cleanup:
  
  if (filetype) WWWFreeFiletype(filetype);
  if (s) socket_close(s);
  if (fd) 
    {
      unlink(tmpf); 
      XtFree(tmpf);
    }
  return result;
}  
static int WWWUseHTTPVersion1(data,filetype,mb,ext)
DataSource *data;
WWWFiletype *filetype;
MessageBlock *mb;
char *ext;
{
  int save, i, n;
  char response[256], c_type[256], c_encode[256], location[256], error[256];
  char pheader[10];
  char *copy;
  float level = 0.0;
  int rcode = 200;

  /*
   * Note: We cannot allow GetCharacter to buffer characters
   *       internally, thus causing problems if the stream has to be 
   *       subsequently passed to a forked process that does not use
   *       GetCharacter
   */
  
  save = data->bsize;
  data->bsize = 1;

  for (i = 0; i < 8; i++) pheader[i] = GetCharacter(data);
  if (data->eof) goto fail;

  data->mb->newMessage = TRUE;
  strcpy(data->mb->message,"Reading HTTP header"); 

  sscanf(pheader, "HTTP/%f", &level);
  if (level < 0.99) goto fail;

  c_type[0] = c_encode[0] = '\0'; 

  for (n = i = 0; ; )
    {
      char c = response[i++] = GetCharacter(data);
      if (data->eof)
        {
          if (rcode/100 == 2) /* if things were ok , they are not now! */
            {
              rcode = 999;
            }
          break;
        }
      else if (c == '\n' || i > 253)  /* read a line each time */
        {
          response[i] = '\0';
          if (n == 0)
            {
              sscanf(response,"%d",&rcode);
              strcpy(error,response);
            }           
          else 
            {
              char *token = strtok(response,":");

              if (strlen(response) < 3) break; 
              else if (!strcasecmp(token,"encoding"))
                strcpy(c_encode,strtok(NULL," \r\n"));
              else if (!strcasecmp(response,"content-type")) 
                strcpy(c_type,strtok(NULL," \r\n"));
              else if (!strcasecmp(response,"location")) 
                strcpy(location,strtok(NULL," \r\n"));
              else if (!strcasecmp(response,"content-length"))
                sscanf(strtok(NULL," \r\n"),"%d",&data->length); 
            }  
          i = 0;
          n++; 
        }
    }

  strcpy(ext,"http1.");
  copy = strrchr(c_type,'/');
  if (copy) strcat(ext,copy+1);
  if (*c_encode) 
    {
      strcat(ext,".");
      strcat(ext,c_encode);
    }
  data->bsize = save;
  if (rcode == 301 || rcode == 302) 
    {
      sprintf(mb->message,"<h1>Moved</h1>Document has moved <a href=\"%s\" redirect=true>here</a>.",location);
      return -rcode;
    }
  else if (rcode == 999)
    {
      sprintf(mb->message,"Error reading document header.");
      mb->help_code = "header_error";       
      return -rcode;
    } 
  else if (rcode/100 != 2) 
    {
      mb->help_code = "request_error";
      sprintf(mb->message,"Document fetch failed.<p>Reason: %s",error);
      return -rcode;
    }  
  return 1;

fail:
 
  data->bsize = save;
  return 0;   
}
