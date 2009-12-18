/*
 *  These are the WWW - Midas Interface routines
 */ 

#include "midasnet.h"
#include <Xm/MessageB.h>
#include <string.h>

extern XrmQuark fileDestination;

/*
 * Fetch the document using TCP/IP and HTTP
 * ----------------------------------------
 */
Widget WWWFetchDocumentHTTP(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{
  char *command; 
  Widget result = NULL;
  DataSource data;
  int s;
  char *ext;
  WWWFiletype *filetype;  
  char buffer[8096];

  if (file->node == NULL)
    {
      strcat(mb->message,"No node specified for HTTP connection");
      mb->help_code = "http_no_node";
      return NULL; 
    }   
  if (file->port == 0) file->port = appResources.default_HTTP_port;

  filetype = WWWGetFiletype(XrmQuarkToString(file->file),appResources.httpDefaultFiletype);
  if (filetype->Destination == fileDestination)
    {
      /* First we need to ask the user where he wants to save this thing?? 
       */  
      if (!WWWAskForFileDestination(w,mb,filetype,FALSE)) goto cleanup;
    }

  s = TCPIPConnect(file,mb);
  if (s == 0) goto cleanup;
  
  command = XtMalloc(20 + (file->file ? strlen(XrmQuarkToString(file->file)) : 0));
  strcpy(command,"GET ");
  strcat(command,file->file ? XrmQuarkToString(file->file): "/");
  strcat(command,"\r\n");

  if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
  XtFree(command);

  WWWInitDataSource(&data,mb,buffer,sizeof(buffer),socket_read,s);

  mb->flag = -1;

  result = WWWLoadByFiletype(w,mb,filetype,GetCharacter,&data);

cleanup:
  
  WWWFreeFiletype(filetype);
  if (s) socket_close(s);
  return result;
}  

