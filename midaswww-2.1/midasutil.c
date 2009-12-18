#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include <string.h>
#include <stdlib.h>
#include "midasoperand.h"
#ifndef VMS
#include <sys/utsname.h>   
#endif
/*
 *  General utility functions
 */
 
static MidasOperand MidasGetenv(var)
char *var;
{  
  MidasOperand Temp;
  char *p= getenv(var); 

  Temp.Value.P = p?p:"";
  Temp.Type = MString;
  Temp.Dynamic = FALSE;
  
  return Temp;
}       
static MidasOperand MidasUname()
{  
  MidasOperand Temp;
#ifdef VMS
  Temp.Value.P = "VMS";
#else
  static struct utsname name;
  int rc = uname(&name);
  Temp.Value.P = name.sysname; 
#endif
  Temp.Type = MString;
  Temp.Dynamic = FALSE;
  
  return Temp;
}
static MidasOperand Locate(needle,haystack)
char *needle;
char *haystack;
{  
  MidasOperand Temp;
  char *p = strstr(haystack,needle);
  
  Temp.Value.I = p ? p-haystack : strlen(haystack);  
  Temp.Type = MInt;
  Temp.Dynamic = FALSE;
  
  return Temp;
}
static MidasOperand Extract(start,length,string)
int start;
int length;
char *string;
{ 
  MidasOperand Temp;
  int len = strlen(string);
  char *p;
 
  if (start>=len) p = NULL;
  else 
  {   
    p = string + start; 
    if (start+length < len) *(p+length) = '\0'; 
  }
  
  Temp.Value.P = XtNewString(p);
  Temp.Type = MString;
  Temp.Dynamic = TRUE;

  return Temp;
}
static MidasOperand Length(string)
char *string;
{
  MidasOperand Temp;

  Temp.Value.I = strlen(string);  
  Temp.Type = MInt;
  Temp.Dynamic = FALSE;
  
  return Temp;
}
static MidasOperand MidasSprintf(format,p1,p2,p3,p4,p5,p6,p7,p8)
char *format;
MidasOperand *p1,*p2,*p3,*p4,*p5,*p6,*p7,*p8;
{
  MidasOperand Temp;
  char buffer[1000];
   
  sprintf(buffer,format,p1->Value.P,p2->Value.P,p3->Value.P,p4->Value.P,p5->Value.P,p6->Value.P,p7->Value.P,p8->Value.P);

  Temp.Value.P = XtNewString(buffer);
  Temp.Type = MString;
  Temp.Dynamic = TRUE;

  return Temp;
}
static void MidasPrintf(format,p1,p2,p3,p4,p5,p6,p7,p8)
MidasOperand *p1,*p2,*p3,*p4,*p5,*p6,*p7,*p8;
{
  printf(format,p1->Value.P,p2->Value.P,p3->Value.P,p4->Value.P,p5->Value.P,p6->Value.P,p7->Value.P,p8->Value.P);
}
void MidasUtilInit()
{
   MidasDeclareFunction("GETENV(name)"         ,MidasGetenv); 
   MidasDeclareFunction("UNAME()"	       ,MidasUname);	

   MidasDeclareFunction("LENGTH(name)"         ,Length);
   MidasDeclareFunction("EXTRACT(Int,Int,name)",Extract);
   MidasDeclareFunction("LOCATE(name,name)"    ,Locate);
   MidasDeclareFunction("SPRINTF(name,{any},{any},{any},{any},{any},{any},{any},{any})",MidasSprintf);
   MidasDeclareVerb("PRINTF name {any} {any} {any} {any} {any} {any} {any} {any}",MidasPrintf);
}
