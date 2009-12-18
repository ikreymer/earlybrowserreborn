#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midaslist.h"
#include "midaserrors.h"
#include "midasoperand.h"
#include "midasconvert.h"
#include <X11/cursorfont.h>
#include <X11/CoreP.h>
#include <stdlib.h>

typedef Boolean (*cr)();

static List ConverterList;
static List MidasClassList;
Widget MidasGetActiveWidget();

ConvertBlock *MidasFindStringConverter(Type)
MidasType Type;
{
    ListItem *item;
    char buffer[256];
    strcpy(buffer,MString);
    strcat(buffer,".");
    strcat(buffer,Type);
    item = MidasFindItemInList(&ConverterList,buffer);
    if (item == 0) MidasError("Could not find converter for %s to %s",MString,Type);
    return item->Pointer;
}
static Boolean MidasConversionConverter(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    cr ConvertRtn = (cr) args[0].addr;
    MidasOperand FromOp, ToOp;
    XtPointer new;
    int size;
    
    FromOp.Value.P = from->addr;
    FromOp.Type = args[1].addr;
    FromOp.Dynamic = FALSE;

    ToOp.Dynamic = FALSE;
    ToOp.Type = args[2].addr;
     
    printf("Converting %s to %s rtn=%x\n",FromOp.Type,ToOp.Type,ConvertRtn);
    if (!ConvertRtn(&FromOp,&ToOp)) return FALSE;
    
    new = ToOp.Value.P;
    if (strcmp(ToOp.Type,"String")) size = sizeof(new);
    else size = strlen((char *) new) + 1;

    if (to->addr == 0)
      {
         to->addr = new;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         strcpy(to->addr,new);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
    
}
void MidasDeclareConverter(FromType,ToType,ConvertRtn)
     MidasType FromType;
     MidasType ToType;
     cr        ConvertRtn;
{
     char *name = XtMalloc(strlen(FromType)+strlen(ToType)+2);
     ListItem *item;

     strcpy(name,FromType);
     strcat(name,".");
     strcat(name,ToType);
     item = MidasFindItemInList(&ConverterList,name);
     if (item != 0) 
       {
         XtFree(name);
         MidasError("Duplicate converter for %s to %s ignored",FromType,ToType);
       }
     else
       {
         ConvertBlock *ab = XtNew(ConvertBlock);
/*
         XtConvertArgRec  ConvertArgs[] = {{XtAddress  ,(XtPointer)ConvertRtn ,sizeof(XtPointer)},
                                           {XtAddress  ,(XtPointer)FromType   ,strlen(FromType)+1} ,
                                           {XtAddress  ,(XtPointer)ToType     ,strlen(ToType)+1}};
*/
         item = MidasAddItemToList(&ConverterList,name);
         item->Pointer = ab;
         ab->ConvertRtn = ConvertRtn;
         ab->FromType = FromType;
         ab->ToType = ToType;
/*
         printf("Convert %s to %s rtn=%x\n",FromType ,ToType,ConvertRtn);
         XtSetTypeConverter(FromType,ToType,MidasConversionConverter,
                            &ConvertArgs,XtNumber(ConvertArgs),XtCacheNone);
*/
       }
     XtFree(name);
}
void MidasDeclareStringConverter(ToType,ConvertRtn)
     MidasType ToType;
     Boolean (*ConvertRtn)();
{
     MidasDeclareConverter(MString,ToType,ConvertRtn);
}
Boolean MidasConvertOperandInternal(Operand,Type)
MidasOperand *Operand;
MidasType Type;
{
  MidasOperand Temp;
  char name[256];
  ListItem *item;
  ConvertBlock *cb;
  Boolean ok;
   
  if (strcmp(Operand->Type,Type) == 0) return TRUE;
  
  if (strcmp(Type,"any") == 0)
    {
      MidasOperand *new = XtNew(MidasOperand);
      *new = *Operand;
      Operand->Type = "any";
      Operand->Dynamic = TRUE;
      Operand->Value.P = (XtPointer) new;
      return TRUE; 
    }

  strcpy(name,Operand->Type);
  strcat(name,".");
  strcat(name,Type);
  item = MidasFindItemInList(&ConverterList,name);

  if (item == 0) 
    {
      /*
       * Maybe the intrinsics can do the job for us?
       *
       */

      XrmValue from, to;
      Widget ActiveWidget = MidasGetActiveWidget();
      int result;

      if (strcmp(Operand->Type,MString) == 0) 
        {
          from.size = strlen(Operand->Value.P) + 1;
          from.addr = Operand->Value.P;
        }
      else
        {
          from.size = sizeof(Operand->Value.P);
          from.addr = Operand->Value.P;
        }
      to.addr = (XtPointer) &result;
      to.size = sizeof(result);

      MidasSuppressXtWarningMessages();

      ok = XtConvertAndStore(ActiveWidget,Operand->Type,&from,Type,&to);

      MidasReenableXtWarningMessages();

      if (ok) 
        { 
          if (to.size == sizeof(result)) Operand->Value.I = result;
          else 
            {
              short s;
              memcpy(&s,to.addr,to.size);
              Operand->Value.I = s;
            }
        }
      else if (to.size>sizeof(result))
        {
          to.addr = XtMalloc(to.size);
          ok = XtConvertAndStore(ActiveWidget,Operand->Type,&from,Type,&to);
          
          Operand->Value.P = to.addr;
          Operand->Dynamic = TRUE;
        }
      if (ok) 
        {
          Operand->Type = Type;
          return ok;
        }

      if (strcmp(Type,MString) == 0)
        {
          char *p = XtMalloc(12);
          sprintf(p,"%d",Operand->Value.I);

          if (Operand->Dynamic) XtFree(Operand->Value.P);
          Operand->Value.P = p;
          Operand->Dynamic = TRUE;
          Operand->Type = MString;
          return TRUE;
        }
      MidasError("No converter declared for %s to %s",Operand->Type,Type);
    }
 
  cb = item->Pointer;
    
  Temp.Dynamic = FALSE;
  Temp.Type = Type;

  ok = cb->ConvertRtn(Operand,&Temp);
   
  if (ok)
    {
      if (Operand->Dynamic) XtFree(Operand->Value.P);
      *Operand = Temp; 
    }

  return ok;
}
void MidasConvertOperand(Operand,Type)
MidasOperand *Operand;
MidasType Type;
{
  Boolean ok = MidasConvertOperandInternal(Operand, Type);
  if (ok) return;

  /* Try to report error as best we can */

  if (strcmp(Operand->Type,MString) == 0) 
    MidasError("Can not convert %s from %s to %s",Operand->Value.P,Operand->Type,Type);

  else if (strcmp(Type,MString) == 0)
    MidasError("Can not convert operand from %s to %s",Operand->Type,Type);

  else
    {
      ok = MidasConvertOperandInternal(Operand, MString);
      if (ok) 
        MidasError("Can not convert %s from %s to %s",Operand->Value.P,Operand->Type,Type);
      else
        MidasError("Can not convert operand from %s to %s",Operand->Type,Type);
    }
}
static MidasOperand MidasConvertForce(Operand,Type)
MidasOperand *Operand;
char *Type;
{
   MidasOperand Temp;
   Temp = *Operand;
   MidasConvertOperand(&Temp,XtNewString(Type)); /* bug ... never freed */
   return Temp; 
}
static Boolean MidasConvertBooleanString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   if (In->Value.I) Out->Value.P = "True";
   else             Out->Value.P = "False";
   return TRUE;
}
static Boolean MidasConvertIntString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   char *new = XtMalloc(12);
   sprintf(new,"%d",In->Value.P);
   Out->Value.P = new;
   return TRUE;
}
static Boolean MidasConvertFloatString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   char *new = XtMalloc(20);
   sprintf(new,"%f",In->Value.F);
   Out->Value.P = new;
   return TRUE;
}
static Boolean MidasConvertStringBoolean(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
  if      (strcmp(In->Value.P,"True" ) == 0) Out->Value.I = TRUE; 
  else if (strcmp(In->Value.P,"true" ) == 0) Out->Value.I = TRUE;
  else if (strcmp(In->Value.P,"TRUE" ) == 0) Out->Value.I = TRUE;
  else if (strcmp(In->Value.P,"False") == 0) Out->Value.I = FALSE;
  else if (strcmp(In->Value.P,"false") == 0) Out->Value.I = FALSE;
  else if (strcmp(In->Value.P,"FALSE") == 0) Out->Value.I = FALSE;
  else return FALSE;

  return TRUE;
}
static Boolean MidasConvertStringXmString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   Out->Value.P = (XtPointer) MidasCharToString(In->Value.P);
   Out->Dynamic = TRUE; /* Need special destructor for this ??? */
   return TRUE;
}
static Boolean MidasConvertStringUpperName(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   char *p = In->Value.P;

   for (; *p != '\0'; p++) *p = toupper(*p);

   Out->Value.P = In->Value.P;
   Out->Dynamic = In->Dynamic;
   In->Dynamic = FALSE;
   return TRUE;
}
static Boolean MidasConvertStringName(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   Out->Value.P = In->Value.P;
   Out->Dynamic = In->Dynamic;
   In->Dynamic = FALSE;
   return TRUE;
}
static Boolean MidasConvertStringInt(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   char *End;
 
   Out->Value.P = (XtPointer) strtol((char *)In->Value.P,&End,10);
   return (*End == '\0');
}    
static Boolean MidasConvertStringFloat(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   char *End;

   Out->Value.F = strtod(In->Value.P,&End);
   return (*End == '\0');
}
static Boolean MidasConvertStringWidget(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   Out->Value.P = (XtPointer) MidasFindWidget(In->Value.P);
   return TRUE;
}
static Boolean MidasConvertStringIcon(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   Out->Value.P = (XtPointer) MidasFetchIcon(In->Value.P,NULL);
   return TRUE;
}
static Boolean MidasConvertStringClass(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   ListItem *i = MidasFindItemInList(&MidasClassList,In->Value.P);
   if (i == 0) return FALSE;
   Out->Value.P = i->Pointer;
   return TRUE;
}
void MidasDeclareClass(Class)
CoreClassPart *Class;
{
    ListItem *i = MidasFindItemInList(&MidasClassList,Class->class_name);
    if (i==0) 
      {
        i = MidasAddItemToList(&MidasClassList,Class->class_name);
        i->Pointer = Class;
      }
}
static Boolean MidasConvertStringAny(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   return FALSE;
}
static Boolean MidasConvertIntNumber(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   *Out = *In;
   return TRUE;
}
static Boolean MidasConvertFloatNumber(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   *Out = *In;
   return TRUE;
}
static Boolean MidasConvertIntFloat(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   Out->Value.F = (float) In->Value.I;
   return TRUE;
}
/*
 *
 *  Converting shorts to Ints in a portable way is not trivial 
 *
 */ 
static Boolean MidasConvertIntShort(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   if (In->Value.I > 32767 || In->Value.I < -32768) return FALSE;
   Out->Value.I = In->Value.I;
   return TRUE;
}
static Boolean MidasConvertShortInt(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   printf("%x\n",In->Value.I);
   printf("%x\n",In->Value.S);
   Out->Value.I = (int) In->Value.S;
   printf("%x\n",Out->Value.I);
   return TRUE;
}
static Boolean MidasConvertStringNumber(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   Boolean ok;

   ok = MidasConvertStringInt(In,Out);
   if (ok) { Out->Type = MInt;  return ok; }

   ok = MidasConvertStringFloat(In,Out);
   if (ok) { Out->Type = MFloat;  return ok; }
   
   return ok;
}
static Boolean MidasConvertStringCursor(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   int r;
   if      (strcmp(In->Value.P,""     ) == 0) r = 0;
   else if (strcmp(In->Value.P,"watch") == 0) r = XC_watch;
   else if (strcmp(In->Value.P,"trek" ) == 0) r = XC_trek;
   else return False;

   Out->Value.I = r; 
   return True;
}
static Boolean MidasConvertStringAtom(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
   XrmValue source, dest;
   Atom result; 
   Widget ActiveWidget = MidasGetActiveWidget();

   source.size = strlen(In->Value.P) + 1;
   source.addr = In->Value.P;

   dest.size = sizeof(Atom);
   dest.addr = (XtPointer) &result;  
 
   XtConvertAndStore(ActiveWidget,XtRString,&source,XtRAtom,&dest);
    
   Out->Value.P = (XtPointer) result;
   return True;
}
void MidasConvertInit()
{
    ConverterList = NullList;
    MidasClassList = NullList;

    MidasDeclareStringConverter("name",       MidasConvertStringName);
    MidasDeclareStringConverter("upname",     MidasConvertStringUpperName);
    MidasDeclareStringConverter("Widget",     MidasConvertStringWidget);   
    MidasDeclareStringConverter("list",       MidasConvertStringList);
    MidasDeclareStringConverter("Int",        MidasConvertStringInt);     
    MidasDeclareStringConverter("Float",      MidasConvertStringFloat);
    MidasDeclareStringConverter("Icon",       MidasConvertStringIcon);
    MidasDeclareStringConverter("Boolean",    MidasConvertStringBoolean);  
    MidasDeclareStringConverter("any",        MidasConvertStringAny);
    MidasDeclareStringConverter("XmString",   MidasConvertStringXmString); 
    MidasDeclareStringConverter("MenuWidget", MidasConvertStringWidget);
    MidasDeclareStringConverter("Pixmap",     MidasConvertStringIcon);
    MidasDeclareStringConverter("Cursor",     MidasConvertStringCursor);   
    MidasDeclareStringConverter("Class",      MidasConvertStringClass);
    MidasDeclareStringConverter("Atom",       MidasConvertStringAtom);
    
    /* 
     * Although the intrinsics have converters for Shell*Dim they rely
     * an ACTUALLY getting a shell widget. Thus we need these two extra   
     * lines here (otherwise MidasWWW show source can bomb).
     */
 
    MidasDeclareStringConverter("ShellVertDim",  MidasConvertStringInt);     
    MidasDeclareStringConverter("ShellHorizDim", MidasConvertStringInt);     

    MidasDeclareConverter("Boolean","String", MidasConvertBooleanString);
    MidasDeclareConverter("Boolean","name",   MidasConvertBooleanString);
    MidasDeclareConverter("Int","String",     MidasConvertIntString);
    MidasDeclareConverter("Int","name",       MidasConvertIntString);
    MidasDeclareConverter("Float","String",   MidasConvertFloatString);
    MidasDeclareConverter("String","Number",  MidasConvertStringNumber);
    MidasDeclareConverter("Int","Number",     MidasConvertIntNumber);
    MidasDeclareConverter("Float","Number",   MidasConvertFloatNumber);
    MidasDeclareConverter("Int","Float",      MidasConvertIntFloat);
    MidasDeclareConverter("Int","Short",      MidasConvertIntShort);
    MidasDeclareConverter("Int","HorizontalDimension",MidasConvertIntShort);
    MidasDeclareConverter("Int","VerticalDimension"  ,MidasConvertIntShort);
    MidasDeclareConverter("Int","ShellHorizPos"      ,MidasConvertIntShort);
    MidasDeclareConverter("Int","ShellVertPos"       ,MidasConvertIntShort);
    MidasDeclareConverter("HorizontalDimension","Int",MidasConvertShortInt);
    MidasDeclareConverter("VerticalDimension","Int"  ,MidasConvertShortInt);
    MidasDeclareConverter("HorizontalDimension","Number",MidasConvertShortInt);
    MidasDeclareConverter("VerticalDimension","Number"  ,MidasConvertShortInt);
 
    MidasDeclareFunction("CONVERT(any,name)",MidasConvertForce);

    MidasClassInit();
}
