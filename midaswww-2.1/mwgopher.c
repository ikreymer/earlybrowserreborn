/*
 *  These are the WWW - Midas Interface routines
 */ 

#include "midasnet.h"
#include "SGMLHyper.h"

extern XrmQuark fileDestination; 
extern XrmQuark textDestination; 

#include <string.h>

extern Widget WWWLoadGIF();
extern Widget WWWLoadPS();


/*
 * Fetch the document using TCP/IP and Gopher
 * ------------------------------------------
 */
static int GopherBuildFile(data)
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
  return  EOF;
}
static int GopherBuildIndex(data)
DataSource *data;
{
  static char newbuffer[8096];
  static DataSource newdata;
  int ip; 

  if (data->nleft == 0)
    {
      int type;
      if (data->read == 0) 
        {
           newdata = *data;
           newdata.buffer = newbuffer;
           newdata.bsize = sizeof(newdata); 
        }
      /*
       * Each line contains 
       *   1) A single character that gives the gopher "type"
       *   2) A User readable name (terminated by a tab)
       *   3) A Closure String (possibly null) (terminated by a tab)
       *   4) A Node name (terminated by a tab)
       *   5) A Port number (terminated by /r/n)
       */

      type = GetCharacter(&newdata);
      while (type == ' ' || type == '\n') type = GetCharacter(&newdata); /* ??? */

      if (type == '.' || type == EOF) 
        { 
          return EOF; 
        }
      else
        { 
          char buffer[900];
          char *name;
          char *closure;
          char *node;
          char *port;
          char *icon;
          char *p;

          name = buffer;
          for (p = buffer; (ip = GetCharacter(&newdata)) != '\t' && ip != EOF; p++) *p = ip;
          *p++ = '\0';
 
          closure = p;
          for ( ; (ip = GetCharacter(&newdata)) != '\t' && ip != EOF; ) 
            {
              if (ip==' ' || ip=='"' || ip=='%' || ip == '?') { sprintf(p,"%%%2x",ip); p += 3; }   
              else *p++ = ip;
            }
          *p++ = '\0';
  
          node = p;
          for ( ; (ip = GetCharacter(&newdata)) != '\t' && ip != EOF; p++) *p = ip;   
          *p++ = '\0';

          port = p;
          for ( ; (ip = GetCharacter(&newdata)) != '\t' && ip != EOF && ip != '\r'; p++) *p = ip;   
          for ( ; ip != '\r' && ip != EOF; ip = GetCharacter(&newdata));
          *p++ = '\0';
               
          GetCharacter(&newdata); /* Skip /n */ 

          if      (type == '0') 
            {
               WWWFiletype *ft = WWWGetFiletype(closure,appResources.gopherDefaultFiletype); 
               icon = ft->IconName; 
               WWWFreeFiletype(ft);
            }
          else if (type == '1') icon = "directory";
          else if (type == '7') icon = "search";
          else if (type == 'I') icon = "graphics";
          else if (type == '8') icon = "telnet";
          else if (type == 'g') icon = "graphics";
          else if (type == 'h') icon = "web";
          else if (type == 'p') icon = "ps";
          else                  icon = NULL; 

          if (icon) sprintf(data->buffer,"<li pixmap=%s><a href=\"gopher://%s:%s/%c%s\" title=\"%s\">%s</a>\n",
                                         icon,node,port,type,closure,name,name); 
          else      sprintf(data->buffer,"<li pixmap=noEntry>%s\n",name);

          data->next = data->buffer;
          data->nleft = strlen(data->buffer);
          data->read += data->nleft;
        }
    }
  if (data->nleft-- > 0) return (ip = *(data->next++)) ? ip : EOF; 
  return EOF;
}
Widget WWWFetchDocumentGOPHER(w,file,mb,title)
Widget w;
WWWFile *file;
MessageBlock *mb;
char *title;
{
  char command[256]; 
  char *q, *p, gopher_type;
  Widget result;
  DataSource data;
  int s;
  char buffer[8096];

  if (file->node == NULL)
    {
      strcat(mb->message,"No node specified for Gopher connection");
      mb->help_code = "gopher_no_node";
      return NULL; 
    }   
  p = strcpy(command,file->file ? XrmQuarkToString(file->file) : "/");
  if (*p == '/' ) p++;
  if (*p != '\0') gopher_type = *p++;
  else            gopher_type = '1';

  if (gopher_type == '8') return WWWFetchDocumentTELNET(w,file,mb);

  if (file->port == 0) file->port = appResources.default_Gopher_port;

  if (gopher_type == '7')
    {
      char *key;

      /*
       * Look for keyword (delimited by ?)
       */

      key = strchr(p,'?');
      if (key) *key = '\t';  
      else 
        {
          XtPointer doc = SGMLHyperOpen(w); 
          if (title && *title) 
            {
              SGMLHyperWrite(doc,"<title>");
              SGMLHyperWrite(doc,title);
              SGMLHyperWrite(doc,"</title><h1><img pixmap=gopher>");
              SGMLHyperWrite(doc,title);
              SGMLHyperWrite(doc,"</h1>");
            }
          SGMLHyperWrite(doc,"Give search term:<isindex>");
          return SGMLHyperClose(doc);
        }
    }  

  s = TCPIPConnect(file,mb);
  if (s == 0) return NULL;

  /*
   * Gopher addresses use %xx as an escape sequence
   * Deal with it here. (assumes machine is using ASCII!)
   */ 

  for (q = p ; *q != '\0' ; q++ ) 
    if (*q == '%') 
    { 
      int x; 
      sscanf(q,"%%%2x",&x);
      *q = (char) x;
      strcpy(q+1,q+3);
    }        
  strcat(command, "\r\n");
      
  if (!TCPIPWriteAndWaitForResponse(s,file,mb,p)) return NULL;

  WWWInitDataSource(&data,mb,buffer,sizeof(buffer),socket_read,s);

  if      (gopher_type == '1'  || gopher_type == '7') 
    {
      XtPointer doc = SGMLHyperOpen(w); 
      if (title && *title) 
        {
          SGMLHyperWrite(doc,"<title>");
          SGMLHyperWrite(doc,title);
          SGMLHyperWrite(doc,"</title><h1><img pixmap=gopher>");
          SGMLHyperWrite(doc,title);
          SGMLHyperWrite(doc,"</h1>");
        }
      else SGMLHyperWrite(doc,"<h1><img pixmap=gopher></h1>");
      SGMLHyperWrite(doc,"Select one of:<menu>");
      SGMLHyperWriteData(doc,GopherBuildIndex,&data);     
      SGMLHyperWrite(doc,"</menu>");
      result = SGMLHyperClose(doc);
    }
  else if (gopher_type == 'g' || gopher_type == 'I')
                               result = WWWLoadGIF(w,mb,GetCharacter,&data);  
  else if (gopher_type == 'p') result = WWWLoadPS(w,mb,GetCharacter,&data);
  else if (gopher_type == 'h') result = SGMLHyperLoadText(w,GetCharacter,&data);
  else                         
    {
      WWWFiletype *filetype;
      p = strcpy(command,file->file ? XrmQuarkToString(file->file) : "/");
      if (*p == '/' ) p++;
      if (*p != '\0') p++;      
      filetype = WWWGetFiletype(p,appResources.gopherDefaultFiletype);

      if (filetype->Destination == fileDestination)
        {
          /* First we need to ask the user where he wants to save this thing?? */

          if (!WWWAskForFileDestination(w,mb,filetype,FALSE)) goto cleanup;
        }
      if (filetype->Destination == textDestination)
        {
          XtPointer doc = SGMLHyperOpen(w); 
          if (title && *title) 
            {
              SGMLHyperWrite(doc,"<title>");
              SGMLHyperWrite(doc,title);
              SGMLHyperWrite(doc,"</title>");
            }
          SGMLHyperWrite(doc,"<plaintext>");
          SGMLHyperWriteData(doc,GetCharacter,&data);
          result = SGMLHyperClose(doc);
        }
      else
        {
          result = WWWLoadByFiletype(w,mb,filetype,GetCharacter,&data);
        }
      WWWFreeFiletype(filetype);
    }
cleanup:
  socket_close(s);
  return result;
}  
