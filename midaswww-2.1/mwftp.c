/*
 *  These are the WWW - Midas Interface routines
 */ 


#include "midasnet.h"
#include "SGMLHyper.h"

#include <string.h>

extern XrmQuark fileDestination;
extern char password[64];
extern XtAppContext appl_context;
extern void ClearFlag();

/*
 * Fetch the document using TCP/IP and FTP
 * ---------------------------------------
 */
static int TCPIPListenConnect(mb,command,control_sock,port)
MessageBlock *mb;
char *command;
int control_sock;
int *port;
{
  struct sockaddr_in server;
  struct sockaddr_in *sin = &server;
  int addr_len;
  int sockfd = 0;

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY; 
 
  /*
   * Sockets 0,1,2 are no good, just get another one 
   */

  for (;sockfd >= 0 && sockfd <= 2;) sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0)
    {
      sprintf(mb->message,"Can't open socket stream<p> Reason: %s",strerror(errno));
      mb->help_code = "connection_fail";
      return 0;
    }

  addr_len = sizeof(server);
  if (getsockname(control_sock,(struct sockaddr *)&server,&addr_len) < 0)
    {
      sprintf(mb->message,"Can't retrieve locally bound address of control socket <p> Reason: %s",strerror(errno));
      mb->help_code = "connection_fail";
      return 0;
    }

  server.sin_port = 0;   /* This must be assigned after getsockname */
  if (bind(sockfd,(struct sockaddr *)&server,sizeof(server)) < 0)
    { 
      sprintf(mb->message,"Can't bind local address<p> Reason: %s",strerror(errno));
      mb->help_code = "connection_fail";
      return 0;
    }

  if (getsockname(sockfd,(struct sockaddr *)&server,&addr_len) < 0)
    {
      sprintf(mb->message,"Can't retrieve locally bound address of data socket <p> Reason: %s",strerror(errno));
      mb->help_code = "connection_fail";
      return 0;
    }

  if (listen(sockfd,1) < 0)
    {
      sprintf(mb->message,"Listen Failure <p> Reason: %s",strerror(errno));
      mb->help_code = "connection_fail";
      return 0;
    }
  *port = server.sin_port;
  sprintf(command,"PORT %d,%d,%d,%d,%d,%d\r\n",
             (int) *((unsigned char *)(&sin->sin_addr)+0),
             (int) *((unsigned char *)(&sin->sin_addr)+1),
             (int) *((unsigned char *)(&sin->sin_addr)+2),
             (int) *((unsigned char *)(&sin->sin_addr)+3),
             (int) *((unsigned char *)(&sin->sin_port)+0),
             (int) *((unsigned char *)(&sin->sin_port)+1)); 
  return sockfd;
}
static int FTPResponse(data,port,messages)
DataSource *data;
int *port;   
char **messages;
{
  char *command, temp[256]; 
  int i = 0; 
  int j = 0;
  int rc, size = 256;
  char cont;
   
  command = XtMalloc(size);
  while (1) 
    {
      command[i] = GetCharacter(data);
      temp[j] = command[i];
      if (command[i] == '\n')
        {            
          temp[j] = '\0';
          if (isdigit(*temp))
            {
              sscanf(temp,"%d%c",&rc,&cont);
              if (cont != '-') break; 
            }  
          j = -1;
  	  size = size + 256;
          command = XtRealloc(command, size);
        }     
      else if (command[i] == (char)EOF) 
        { 
          XtFree(command);
          return -1;
        }
      i++; j++;    
    }            
  command[i+1] = '\0';
  *messages = command; 
/*
 * Handle some special return codes here
 */
  if (rc==227 && port) 
    {
      int reply, h0, h1, h2, h3, p0, p1;	/* Parts of reply */
      char *p;
      for (p=command; *p ; p++) if ((*p<'0')||(*p>'9')) *p = ' '; /* Keep only digits */
      sscanf(command,"%d%d%d%d%d%d%d",&reply,&h0,&h1,&h2,&h3,&p0,&p1);
      *port = (p0<<8) + (p1&255); 
    }
  if (rc==150 && port)
    {
      char check[6],*skip;
      int size = 0;
      skip = strrchr(command,'(');
      if (skip)
        {  
          sscanf(skip,"(%d%5s",&size,check);
          if (size && !strcmp(check,"bytes")) *port = size;  
        } 
    } 
  return rc;  
}
static int FTPBuildDirectory(data)
DataSource *data;
{
  static char newbuffer[8096];
  static char postfix[] = "</menu>"; 
  static DataSource newdata;
  int ip; 

  if (data->nleft == 0)
    {
      char *current = XtNewString(XrmQuarkToString(data->mb->file->file));
      char *node = XrmQuarkToString(data->mb->file->node);

      if (data->read == 0)
        {
          newdata = *data;
          newdata.buffer = newbuffer;
          newdata.bsize = sizeof(newbuffer);  

          sprintf(data->buffer,"<title>%s %s</title>\n<h1>%s</h1>\n See<a href=\"ftp://%s/$$login\"> login messages </a><menu>\n",node,current?current:"",current?current:"",node); 
          if (current && *current && strcmp(current,"/")) strcat(data->buffer,"<li pixmap=directory><a href=\"../\">&lt;parent directory&gt;</a>\n");
          data->next = data->buffer;
          data->nleft = strlen(data->buffer);
          data->read += data->nleft;
        }  
      else
        { 
          /*
           * Each line contains a file specification
           */

          char buffer[900];
          char *p, *q, *type;
          char *icon; 
          int ip;
          char *system = data->mb->help_code; /* this was stashed here by FetchDocumentFTP */ 

          /* Remove any trailing / from current, remembering current could be empty (or null) */
  
          if (current && strrchr(current,'/') == current + strlen(current) - 1) *strrchr(current,'/') = '\0';
 
          next:

          for (q = p = buffer; (ip = GetCharacter(&newdata)) != '\r' && ip != EOF; p++) *p = ip;
          if (GetCharacter(&newdata) == EOF) /* Skip /n */
           { 
             data->next = postfix;
             data->nleft = strlen(postfix)+1; /* include the trailing null */ 
             data->read += data->nleft;
            }
          else
            {
              *p = '\0';
     
              if (*(p-1) == '*') *--p = '\0';
              if (*(p-1) == '@') *--p = '\0';
 
              if (!strcmp(system,"VMS"))
                {
                  char *t;
                  if (*buffer == '[') for ( ; *q != '\0' && *q++ != ']' ; );
                  for (t = q ; *t != '\0'; t++) *t = tolower(*t);
                }
              else if (!strcmp(system,"OS/2"))
                {
                  char *t, *size, *date, *time;

                  size = strtok(buffer," ");
                  type = strtok(NULL," ");
                  date = strtok(NULL," ");
                  time = strtok(NULL," ");
                  q = strtok(NULL," "); 
                  for (t = q ; *t != '\0'; t++) *t = tolower(*t);
                }
              else
                {
                  if (*(p-1) == '*') *--p = '\0';
                  if (*(p-1) == '@') *--p = '\0';
                }

              if (*(p-1) == '/') 
                {
                  *--p = '\0';
                  icon = "directory";
                } 
              else if (!strcmp(system,"VMS") && !strcmp(p-4,".dir"))
                {
                  icon = "directory";
                  p -= 4; 
                  *p = '\0';
                } 
              else if (!strcmp(system,"OS/2") && !strcmp(type,"DIR"))
                {
                  if (!strcmp(q,".") || !strcmp(q,"..")) goto next;
                  icon = "directory";
                }
              else 
                {
                   WWWFiletype *ft = WWWGetFiletype(q,appResources.ftpDefaultFiletype); 
                   icon = ft->IconName; 
                   WWWFreeFiletype(ft);
                }   

              if (icon) sprintf(data->buffer,"<li pixmap=%s><a href=\"%s/%s\">%s</a>\n",icon,current?current:"",q,q); 
              else      sprintf(data->buffer,"<li pixmap=noEntry>%s\n",q);

              data->next = data->buffer;
              data->nleft = strlen(data->buffer);
              data->read += data->nleft;
            }
        }
      XtFree(current);
    }
  if (data->nleft-- > 0) return (ip = *(data->next++)) ? ip : EOF; 
  return EOF;
}
Widget WWWFetchDocumentFTP(w,file,mb)
Widget w;
WWWFile *file;
MessageBlock *mb;
{
  Widget result = NULL;
  DataSource data, new_data;
  char buffer[1024], new_buffer[8096];
  int s,  rc, new_socket = 0, port;
  WWWFile new;
  char command[256];
  char *ext;
  char mode;
  char *msg;
  char *filename = XrmQuarkToString(file->file);
  char *save_message = NULL;
  WWWFiletype *filetype = NULL;
  struct sockaddr_in soc_addr;
  int soc_addrlen = sizeof(soc_addr);
  XtInputId id;

  static char     *active_msg1;
  static char     *active_msg2;
  static int      active_port   = 0;
  static XrmQuark active_node   = (XrmQuark)NULL;
  static int      active_socket = 0;
  static char     active_mode   = ' ';
  static char     active_system[5];
  new = *file;
 
  /*
   * If no node was specified, treat as local file spec
   */
  if (file->node == NULL) return WWWFetchDocumentLOCAL(w,file,mb);
 
  if (file->port == 0) file->port = appResources.default_FTP_port;

  filetype = WWWGetFiletype(filename,appResources.ftpDefaultFiletype);
  if (filetype->Destination == fileDestination)
    {
      /* First we need to ask the user where he wants to save this thing?? 
       * Also since this is an FTP transfer we need to ask if it is a binary
       * file.
       */  
      if (!WWWAskForFileDestination(w,mb,filetype,TRUE)) goto cleanup;
    }

/* connect label is used for time-out goto */
connect:

  if (active_socket == 0 || file->port != active_port || file->node != active_node)
    {
      char *user = "anonymous";

      if (active_socket) 
        {
          socket_close(active_socket); 
          active_socket = 0;
          XtFree(active_msg1);
          XtFree(active_msg2); 
        }   
    
      s = TCPIPConnect(file,mb);
      if (s == 0) return NULL;

      if (!TCPIPWriteAndWaitForResponse(s,file,mb,"")) goto cleanup;

      WWWInitDataSource(&data,mb,buffer,sizeof(buffer),socket_read,s);

      rc = FTPResponse(&data,NULL,&msg);
      if (rc < 0) goto cleanup;
      active_msg1 = msg;

      if (rc/100 == 2)
        {
          sprintf(command,"USER %s\r\n",user);
          if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;  

          rc = FTPResponse(&data,NULL,&msg);
          if (rc < 0) goto cleanup;
          active_msg2 = msg;
        }
      if (rc/100 == 3)
        {
          XtFree(active_msg2);
          sprintf(command,"PASS %s\r\n",password);
	  if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup; 

	  rc = FTPResponse(&data,NULL,&msg);
          if (rc < 0) goto cleanup;
          active_msg2 = msg;
        }
      if (rc/100 == 3) 
         {
          XtFree(active_msg2);
	  if (!TCPIPWriteAndWaitForResponse(s,file,mb,"ACCT noaccount\r\n")) goto cleanup;

	  rc = FTPResponse(&data,NULL,&msg);
          if (rc < 0) goto cleanup;
          active_msg2 = msg;
        }

      if (rc/100 != 2) 
        {
          int maxlen = sizeof(mb->message) - 200; 
          socket_close(s);
          if (strlen(msg) > maxlen) strcpy(msg + maxlen,"...(message too long...truncated)");
          sprintf(mb->message,"Cannot login to <i>%s</i> port %d as user <i>%s</i> with password <i>%s</i> <p> Reason: <pre>\n%s</pre>",
             XrmQuarkToString(file->node),file->port,user,password,msg);
          mb->help_code = "cannot_login";
          goto cleanup;
        }

      if (!TCPIPWriteAndWaitForResponse(s,file,mb,"SYST\r\n")) goto cleanup;
      rc =FTPResponse(&data,NULL,&msg);
      if (rc < 0) goto cleanup;
      if (rc/100 == 2)
        {
          sscanf(msg,"%*d%4s",active_system);
        }  
      else active_system[0] = '\0';
      XtFree(msg);
 
      active_socket = s;
      active_port = file->port;
      active_node = file->node;
      active_mode = 'A';
    }  
  else
   { 
      s = active_socket; 
      WWWInitDataSource(&data,mb,buffer,sizeof(buffer),socket_read,s);
    }

  /*
   * Display the login messages with /$$login as anchor
   */
  if (file->file == XrmStringToQuark("/$$login"))
  {  
     char c[256];
     XtPointer temp = SGMLHyperOpen(w);

     sprintf(c,"<title> %s /Login Messages </title>",XrmQuarkToString(file->node));
     SGMLHyperWrite(temp,c);
     SGMLHyperWrite(temp,"<h1> Login Messages </h1>");
     SGMLHyperWrite(temp,"<xmp>");
     SGMLHyperWrite(temp,active_msg1);
     SGMLHyperWrite(temp,active_msg2); 
     SGMLHyperWrite(temp,"</xmp>");
     return SGMLHyperClose(temp);
  }
  
#ifdef PASV
  /*
   * Tell the other guy to shut up and listen
   */
   
  if (!TCPIPWriteAndWaitForResponse(s,file,mb,"PASV\r\n")) goto cleanup;
  rc = FTPResponse(&data,&port,&msg);
  if (rc < 0) goto cleanup;

  /* if it is a time-out, connect again */
  if (rc == 421)
    {
    active_socket = 0;
    XtFree(active_msg1);
    XtFree(active_msg2);
    XtFree(msg);
    goto connect; 
   } 
  if (rc/100 != 2)
    {
      if (strlen(msg) > 512) *(msg+512) = '\0';
      sprintf(mb->message,"Server at %s port %d does not understand PASV command <p> Reason: %s",XrmQuarkToString(file->node),file->port,msg);
      mb->help_code = "brain_dead";
      XtFree(msg);
      goto cleanup;
    }
  XtFree(msg);
  new.port = port;
   
  new_socket = TCPIPConnect(&new,mb);
  if (new_socket == 0) goto cleanup;
 
#else /* Use Listen */

  new_socket = TCPIPListenConnect(mb,command,s,&port);
  if (new_socket == 0) goto cleanup;
  if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
  rc = FTPResponse(&data,NULL,&msg);
  if (rc < 0) goto cleanup;

  /* if it is a time-out, connect again */

  if (rc == 421)
    {
      active_socket = 0;
      XtFree(active_msg1);
      XtFree(active_msg2);
      XtFree(msg);
      goto connect;
    }

  if (rc/100 != 2)
    {
       if (strlen(msg) > 512) *(msg+512) = '\0';
       sprintf(mb->message,"Server at %s does not understand PORT command <p> Reason: %s",XrmQuarkToString(file->node),msg);
       mb->help_code = "brain_dead";
       XtFree(msg);
       goto cleanup;
    }
  XtFree(msg);

#endif
  WWWInitDataSource(&new_data,mb,new_buffer,sizeof(new_buffer),socket_read,new_socket);
  
  /*
   * Ok we are ready to get the file at last
   */

  if (filetype->Binary) mode = 'I';
  else                  mode = 'A';

  if (mode != active_mode)
    {
      active_mode = mode;
      sprintf(command,"TYPE %c\r\n",mode);
      if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
      rc = FTPResponse(&data,NULL,&msg);
      if (rc < 0) goto cleanup;
      XtFree(msg);
    }
  if (filename && *filename)
    {  
      if (!strcmp(active_system,"VMS"))
        {
          char buf[521];
          char *l, *t = buf;
          char *f = filename;    

          if (*f == '/') f = ++filename; /* chop off leading / */ 
          l = strrchr(f,'/');
          if (l) { *t++ = '['; *t++ = '.'; }
          for ( ; *f != '\0' ; f++, t++) if ( *f == '/') *t = '.'; else *t = *f; 
          *t = '\0'; 
          if (l) *(buf+(l-filename)+2) = ']';
          sprintf(command,"RETR %s\r\n",buf);
        }
      else if (!strcmp(active_system,"OS/2")) 
        {
          if (*filename == '/') filename++; /* chop off leading / */ 
          sprintf(command,"RETR %s\r\n",filename);
        }
      else sprintf(command,"RETR %s\r\n",filename);
    } 
  else strcpy(command,"RETR\r\n");
  
  if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
  rc = FTPResponse(&data,&new_data.length,&msg);
  if (rc < 0) goto cleanup;
  XtFree(msg);
  if (rc/100 == 1) 
    {

#ifndef PASV

      mb->newMessage = TRUE;
      sprintf(mb->message,"Waiting for connection from %s port %d",XrmQuarkToString(file->node),port);

      mb->flag = -1;

#ifndef NO_NBIO
      id = XtAppAddInput(appl_context,new_socket,(XtPointer) XtInputReadMask,ClearFlag,(XtPointer) &mb->flag);
      for (;mb->flag < 0;)
         {
           MidasFetchDispatchEvent();
         }
      XtRemoveInput(id);
#endif

      if (mb->flag == 1)
        {
          strcpy(mb->message,AbortMessage);
          goto cleanup;
        }

      new_data.socket = accept(new_socket,(struct sockaddr *)&soc_addr,&soc_addrlen);
      if (new_data.socket < 0)
         {
           sprintf(mb->message,"Can't accept connect request on socket %d",new_socket);
           mb->help_code = "connection_fail";
           goto cleanup;
         }

#endif /* PASV */

      result = WWWLoadByFiletype(w,mb,filetype,GetCharacter,&new_data);
      if (result == NULL) 
        {
          if (*mb->help_code) goto cleanup;
          else                save_message = XtNewString(mb->message); 
        }
    }
  else 
    {  
      /* we assume null is unix (nic.merit.edu for example) */      
      if (!strcmp(active_system,"UNIX") || !*active_system) 
        {
          if (filename && *filename) sprintf(command, "CWD %s\r\n",filename);
          else strcpy(command,"CWD /\r\n");
          if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
          rc = FTPResponse(&data,NULL,&msg);
          if (rc < 0) goto cleanup;
          if (rc/100 == 2)
            {
              XtFree(msg);            /* CWD msg */
              if (!TCPIPWriteAndWaitForResponse(s,file,mb,"NLST -F\r\n")) goto cleanup;
              rc = FTPResponse(&data,NULL,&msg);
              if (rc < 0) goto cleanup;
              XtFree(msg);
              if (rc/100 != 1)
                { 
                  if (!TCPIPWriteAndWaitForResponse(s,file,mb,"NLST\r\n")) goto cleanup;
                  rc = FTPResponse(&data,NULL,&msg);
                  if (rc < 0) goto cleanup;
                }
             }
        }   
      else if (!strcmp(active_system,"VMS"))
        {
          if (!filename || !*filename) sprintf(command,"NLST\r\n");
          else                   
            { 
              char *t;
              sprintf(command,"NLST [.%s]\r\n",filename); 
              for (t = command; *t != '\0' ; t++) if (*t == '/') *t = '.';
            }
          if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
          rc = FTPResponse(&data,NULL,&msg);
          if (rc < 0) goto cleanup;
        }
      else if (!strcmp(active_system,"OS/2")) /* NLST under OS/2 doesn't show the directories! */
        {
          if (filename && *filename) sprintf(command,"LIST %s\r\n",filename); 
          else strcpy(command,"LIST\r\n");
          if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
          rc = FTPResponse(&data,NULL,&msg); 
          if (rc < 0) goto cleanup;
        }
      else  /* just hope for the best! */ 
        {
          if (filename && *filename) sprintf(command,"NLST %s\r\n",filename);
          else strcpy(command,"NLST\r\n");
          if (!TCPIPWriteAndWaitForResponse(s,file,mb,command)) goto cleanup;
          rc = FTPResponse(&data,NULL,&msg);
          if (rc < 0) goto cleanup;
        }
#ifndef PASV

      if (rc/100 == 1)
        { 
          mb->newMessage = TRUE;
          sprintf(mb->message,"Waiting for connection from %s port %d",XrmQuarkToString(file->node),port);

          mb->flag = -1;
#ifndef NO_NBIO
          id = XtAppAddInput(appl_context,new_socket,(XtPointer) XtInputReadMask,ClearFlag,(XtPointer) &mb->flag);
          for (;mb->flag < 0;)
            {
              MidasFetchDispatchEvent();
            }
          XtRemoveInput(id);
#endif
          if (mb->flag == 1)
            {
              strcpy(mb->message,AbortMessage);
              goto cleanup;
            }

          new_data.socket = accept(new_socket,(struct sockaddr *)&soc_addr,&soc_addrlen);
          if (new_data.socket < 0)
            {
              sprintf(mb->message,"Can't accept connect request on socket %d",new_socket);
              mb->help_code = "connection_fail";
              goto cleanup;  
            }

#endif /* PASV */

          mb->help_code = active_system; /* ugly way to get this into FTPBuildDirectory */  
          result = SGMLHyperLoadText(w,FTPBuildDirectory,&new_data);
          if (result) mb->help_code = ""; /* Get rid of temporary value */
        }
   }

  socket_close(new_data.socket);
  socket_close(new_socket);
  new_socket = 0;
  WWWFreeFiletype(filetype);
  filetype = NULL;

  if (rc/100 == 1) 
   {
    rc = FTPResponse(&data,NULL,&msg);
    if (rc < 0) goto cleanup;
    if (rc != 226 && result)
    {
      XtDestroyWidget(result);
      if (strlen(msg) > 512) *(msg+512) = '\0';
      sprintf(mb->message,"Transfer incomplete at %s port %d <p> Reason: %s",XrmQuarkToString(file->node),file->port,msg);
      mb->help_code = "transfer_fail";
      XtFree(msg);
      goto cleanup;
    } 
    XtFree(msg); 
   }
  else      
   {
      sprintf(mb->message,"File access failed <p> Reason: %s",msg);
      mb->help_code = "file_access_fail";
      XtFree(msg);
      goto cleanup;
   } 

  if (save_message)
    {
      strcpy(mb->message,save_message);
      XtFree(save_message);
    }
  return result;

cleanup:

  if (new_socket) socket_close(new_socket); 
  if (active_socket)
    {
      socket_close(active_socket); 
      active_socket = 0; 
    } 
  XtFree(save_message);
  if (filetype) WWWFreeFiletype(filetype);
  return NULL;   
}  
