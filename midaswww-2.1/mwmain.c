/* 
 * Main routine for MidasWWW
 */
#include "midaswww.h"
#include <stdio.h>
extern Widget MidasInitializeUser();
extern Widget MidasInitializeFallback();

XtAppContext appl_context;

#define WWWNversion                "version"
#define WWWCVersion                "Version"

static XtResource resources[] = {
    {WWWNversion, WWWCVersion, XtRString, sizeof(char *),
    XtOffsetOf(WWWResources,version),XtRString,(XtPointer) "0"},
};

static int DummyErrorHandler(dpy,ee)
Display *dpy;
XErrorEvent *ee;
{
  return 0;
}
int main(argc,argv)
int argc;
char *argv[];
{ 
  int depth, n=0;
  Arg arglist[10];

  static char *midas_resources[] = { 
#include "Midas.adh"
  "",NULL};
  static char *midaswww_common_resources[] = { 
#include "Midaswww_common_part.adh"
  "",NULL};
  static char *midaswww_color_resources[] = { 
#include "Midaswww_color_part.adh"
  "",NULL};
  static char *midaswww_mono_resources[] = { 
#include "Midaswww_mono_part.adh"
  "",NULL};   
  static char **midaswww_resources;
  int none = 0;  

  Widget  top = MidasInitializeFallback(&none,NULL,midas_resources);
  
  appl_context = XtWidgetToApplicationContext(top);  
  
  XtGetApplicationResources(top,(XtPointer) &appResources,resources,XtNumber(resources),NULL,0);
  if (strcmp(appResources.version,VERSION))
    {
      fprintf(stderr,"Wrong version of Midas resource file found:\n");
      fprintf(stderr,"Midas Version = %s ; Resource file Version = %s\n",VERSION,appResources.version);
      fprintf(stderr,"Please consult installation instructions\n");
      exit(FAILURE);
    }

  XtSetArg(arglist[n],XtNdepth,&depth); n++;
  XtGetValues(top,arglist,n);
 
  if (depth > 4) 
   {
      char **temp = midaswww_resources = (char **) XtMalloc(sizeof(char *) * 
                              (XtNumber(midaswww_common_resources) + 
                               XtNumber(midaswww_color_resources ))); 
      memcpy(temp,midaswww_common_resources,sizeof(char *) * XtNumber(midaswww_common_resources));
      temp += XtNumber(midaswww_common_resources) - 2;
      memcpy(temp,midaswww_color_resources ,sizeof(char *) * XtNumber(midaswww_color_resources ));
   }
  else
   {
      char **temp = midaswww_resources = (char **) XtMalloc(sizeof(char *) * 
                              (XtNumber(midaswww_common_resources) + 
                               XtNumber(midaswww_mono_resources ))); 
      memcpy(temp,midaswww_common_resources,sizeof(char *) * XtNumber(midaswww_common_resources));
      temp += XtNumber(midaswww_common_resources) - 2;
      memcpy(temp,midaswww_mono_resources  ,sizeof(char *) * XtNumber(midaswww_mono_resources ));
   } 

  XtAppSetFallbackResources(appl_context,midaswww_resources);
  top = MidasInitializeUser("midaswww",argv,&argc);
  XtFree((char *)midaswww_resources);  

  XtGetApplicationResources(top,(XtPointer) &appResources,resources,XtNumber(resources),NULL,0);
  if (strcmp(appResources.version,VERSION))
    {
      fprintf(stderr,"Wrong version of Midas WWW resource file found:\n");
      fprintf(stderr,"Midas WWW Version = %s ; Resource file Version = %s\n",VERSION,appResources.version);
      fprintf(stderr,"Please consult installation instructions\n");
      exit(FAILURE);
    }

  /*  
   * Unless running in debug mode we will silently ignore any X errors
   */

#ifndef DEBUG
  XSetErrorHandler(DummyErrorHandler);
#endif

  MidasStartInterpreter(top);

  WWWMidasInit(argv,argc);
  MidasMainLoop();
}
