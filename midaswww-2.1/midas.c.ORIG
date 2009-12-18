#include <stdio.h>                              /* For printf and so on. */
#include <setjmp.h>                             /* For setjmp and so on. */
#include <stdlib.h>                             /* for system() etc.     */
#include <string.h>
#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include <X11/CoreP.h> 
#include <X11/Xatom.h> 
#include <X11/cursorfont.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>    
#include <Xm/Command.h>    
#include <Xm/BulletinB.h>
#include <Xm/MessageB.h>
#include "SGMLHyper.h"     
#include "SGMLCompositeText.h"     
#include "midaslist.h"     
#include "midasoperand.h"
#include "midaswidget.h"
#include "midasshell.h"
#include "Tree.h"

typedef Widget (*MidasWidgetScanProc)();

#ifdef VMS
#define FAILURE 0
#define SUCCESS 1
#else
#define SUCCESS 0
#define FAILURE 1
#endif

#if !defined(__STDC__) && !defined(_NO_PROTO)
#define _NO_PROTO
#endif
#if defined(__STDC__) && defined(_NO_PROTO)
#undef _NO_PROTO
#endif

#ifdef __STDC__
# ifndef MISSING_STDARG_H
#  include <stdarg.h>
# endif
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

#ifndef VMS
#include <pwd.h>
#endif

#ifdef EDITRES
	extern void _XEditResCheckMessages();
#endif

MidasOperand MidasGetIngot();
MidasOperand MidasGetCreateCallback();
void MidasLoadImage();

void MidasDispatchCommandCallback();
MidasOperand MidasEvaluateExpression();

#ifndef VMS
WidgetList DXmChildren();
int DXmNumChildren();
#endif

/*
 * Global data            
 */                                  

Display         *display;		/* Display variable */
XtAppContext    app_context;		/* application context */

static Widget WidgetTree = NULL;        
static Widget MidasMain;                /* Root widget ID of main */
                                        /* MRM fetch */

#define HASHTABLESIZE 137
static MidasWidget *HashTable[HASHTABLESIZE];

struct QueuedCommand {
    Widget                Widget;
    char                 *Command;
    struct QueuedCommand *Next; 
};

typedef struct QueuedCommand QueuedCommand;

static struct {
    QueuedCommand *Head;
    QueuedCommand *Tail;
} CommandQueue;

struct MidasFile {
    MrmHierarchy         Hierarchy;
    Widget               DefaultParent;
    Pixel                foreground;
    Pixel                background;
};

typedef struct MidasFile MidasFile;

struct ConvertPair {
    char    *Name;
    char    Value;
};

typedef struct ConvertPair ConvertPair;

static ConvertPair ConvertBoolean[] = {{"true",1}, 
                                       {"false",0},
                                       {0,0}};

static XtConvertArgRec ConvertBooleanArg[] =
{{XtAddress,(XtPointer) ConvertBoolean,sizeof(XtPointer)}};

static ConvertPair ConvertPacking[] = {{"XmPACK_TIGHT" ,1},
                                       {"XmPACK_COLUMN",2},
                                       {"XmPACK_NONE"  ,3},
                                       {0,0}};
static XtConvertArgRec ConvertPackingArg[] =
{{XtAddress,(XtPointer) ConvertPacking,sizeof(XtPointer)}};
static XtConvertArgRec ConvertWidgetArg[] = {{XtWidgetBaseOffset,0,sizeof(Widget)}};

static List   *MidasWidgetList;
static List   *MidasFileList;
static List   *MidasApplicationList;
static List   *MidasShellList;

static XtTranslations MidasTranslations;

Widget ActiveWidget;
XmAnyCallbackStruct *ActiveCallback;
Boolean IfState;

static char ProblemStrings[2000];
static char *ProblemPtr = ProblemStrings;

/* we need some protection here against exceeding the buffer size */

jmp_buf JmpEnv[100];
int NumJump = 0; 

static Widget MidasFindName();
void *MidasConvertExpression();

static XtErrorMsgHandler OldHandler;

void MidasDummyHandler(){} 
void MidasDefaultWarningHandler(msg)
char *msg;
{
   fprintf(stderr,"%s \n",msg);
}   
/*
void MidasSuppressXtWarningMessages()
{
  OldHandler = XtAppSetWarningMsgHandler(app_context,MidasDummyHandler);
}
void MidasReenableXtWarningMessages()
{
  XtAppSetWarningMsgHandler(app_context,OldHandler);
}  
*/           
void MidasSuppressXtWarningMessages() 
{              
   XtAppSetWarningHandler(app_context,(XtErrorHandler) MidasDummyHandler);
}
void MidasReenableXtWarningMessages() 
{
   XtAppSetWarningHandler(app_context,MidasDefaultWarningHandler);
}

static char *MidasClassName(w)
Widget w;
{
   CoreClassPart *Class = (CoreClassPart *) XtClass(w);
   return Class->class_name; 
}
Widget MidasGetActiveWidget()
{
  return ActiveWidget;
}
XmAnyCallbackStruct *MidasGetActiveCallback()
{
  return ActiveCallback;
}
XmString MidasCharToString(C)  /*---Convert a char * to an XmString */
      char *C;
{
    return XmStringCreateSimple(C);
}
#ifndef _NO_PROTO
void MidasError(char *fmt,...)
#else
/* VARARGS2*/
void MidasError(fmt,va_alist)
char *fmt;
va_dcl
#endif
{
    va_list args;
    Va_start(args, fmt);
     
    vsprintf(ProblemPtr, fmt, args);
    printf("Error: %s\n",ProblemPtr);
    ProblemPtr += strlen(ProblemPtr);
    *ProblemPtr++ = '\n';
    va_end(args);

    if (NumJump > 0) longjmp(JmpEnv[--NumJump],1);

    *--ProblemPtr = '\0';
      {
        Widget w;
        XmString string = XmStringCreateLtoR(ProblemStrings,
                                             XmSTRING_DEFAULT_CHARSET);   
        int n = 0; 
        Arg args[10];
        XtSetArg(args[n],XmNmessageString, string); n++;
 
        w = MidasFindName("MidasErrorBox");
        if (w != 0)
          {
            XtSetValues(w,args,n);
            XmStringFree(string);
            XtManageChild(w);
          }
      }
    ProblemPtr = ProblemStrings;
};
char *MidasStringToChar(string) /*----Convert an XmString to a char */
     XmString string;
{
    XmStringContext   context;
    char              *result;
    XmStringCharSet   charset;
    XmStringDirection direction;
    Boolean           separator;

    XmStringInitContext(&context,string); 
    XmStringGetNextSegment(context,&result,&charset,&direction,&separator); 
    XmStringFreeContext(context);

    return result;
};
void MidasScanChildren(w,Rtn,Closure1,Closure2)
      Widget w;
      void (*Rtn)();
      XtPointer Closure1;
      XtPointer Closure2;
{
   int n;
   Widget *list;  

   if (XtIsSubclass(w,compositeWidgetClass))
      {
        list = DXmChildren(w);
        n = DXmNumChildren(w);
      
        for (; n > 0; n-- , list++) Rtn(*list,Closure1,Closure2); 
      }

   if (SGMLIsCompositeText(w))
      {
        Arg arglist[10];
        int nn = 0;
  
        XtSetArg(arglist[nn],XtNchildren,&list); nn++;
        XtSetArg(arglist[nn],XtNnumChildren,&n); nn++;
        XtGetValues(w,arglist,nn);
        
        for (; n > 0; n-- , list++) Rtn(*list,Closure1,Closure2); 
      }

   if (XtIsWidget(w) && w->core.screen != 0)
     {
       list = w->core.popup_list;
       n = w->core.num_popups;       
 
       if (list != NULL) for (; n > 0; n-- , list++) Rtn(*list,Closure1,Closure2); 
     }
}
void MidasScanAncestors(w,Rtn,Closure1)
      Widget w;
      void (*Rtn)();
      XtPointer Closure1;
{
   int n;
   Widget *list;  

   Rtn(w,Closure1);
   MidasScanChildren(w,MidasScanAncestors,(XtPointer) Rtn,Closure1);
}
void MidasCloseUidFile(w,mfile)
    Widget w;
    MidasFile *mfile;
{
    ListItem *item;
    MrmCloseHierarchy(mfile->Hierarchy);
    
    for (item = MidasFileList->First; item != NULL; item = item->Next)
      {
        if (mfile == (MidasFile *) item->Pointer) 
          {
            MidasRemoveItemFromList(MidasFileList,item);
            return;
          }  
      } 
}
static MrmHierarchy MidasOpenUidFile(name,parent)
char *name;
Widget parent;
{
    MrmHierarchy hierarchy; 
    MidasFile *mfile;
    ListItem *item;
    Cardinal rc;
    caddr_t version;
    MrmCode type; 
        
    MidasSuppressXtWarningMessages();
    if (MrmOpenHierarchy(1, &name, NULL, &hierarchy)!= MrmSUCCESS)
    { 
#ifndef VMS
       char *newname = name;
       MidasReenableXtWarningMessages();

       if (MidasMakeUid(&newname))
         {
           rc = MrmOpenHierarchy(1, &newname, NULL, &hierarchy); 
           unlink(newname);  
           XtFree(newname);
           if (rc != MrmSUCCESS) 
             {
               fprintf(stderr,"Fatal error opening self-contained uid file %s\n",name);
               exit(FAILURE);
             }
         }
       else
#endif
         { 
#ifdef VMS
           fprintf(stderr,"Unable to open uid file %s\n",name);
#endif
           exit(FAILURE);
         }
     }                          
   
    rc = MrmFetchLiteral(hierarchy,"MidasVersion",display,&version,&type);
    if (rc != MrmSUCCESS || strcmp((char *)version,VERSION))
      { 
        fprintf(stderr,"Version mismatch for uidfile %s\n",name);
        if (rc == MrmSUCCESS) fprintf(stderr,"Expected %s, found %s\n",VERSION,version);
        fprintf(stderr,"Please see installation instructions\n");
        exit(FAILURE);
      }
    MidasReenableXtWarningMessages();

    item = MidasAddItemToList(MidasFileList,name);
    mfile = XtNew(MidasFile);
    mfile->DefaultParent = parent;
    mfile->Hierarchy = hierarchy;
    mfile->foreground = XBlackPixelOfScreen(XtScreen(parent));
    mfile->background = XWhitePixelOfScreen(XtScreen(parent));
    item->Pointer = mfile;

    XtAddCallback(parent,XtNdestroyCallback,(XtCallbackProc) MidasCloseUidFile,(XtPointer) mfile);

    return hierarchy;
}
void MidasWidgetBeingDestroyed(w,mw,reason)
    Widget w;
    MidasWidget *mw;
    MidasWidget **reason;
{
    /* remove from hash chain */ 
 
    int hash = ((int) w) % HASHTABLESIZE;
    MidasWidget *loop = HashTable[hash];
  
    if (loop == mw) HashTable[hash] = mw->HashChain;
    else 
      { 
        for ( ; loop->HashChain != mw; loop = loop->HashChain);
        loop->HashChain = mw->HashChain;  
      }  

    if (mw->Map       != NULL) XtDestroyWidget(mw->Map);
    if (mw->IngotList != NULL) MidasDestroyIngots(mw);
    if (mw->NameEntry != NULL) MidasRemoveItemFromList(MidasWidgetList,mw->NameEntry);
    else XtFree((char *) mw);
}
MidasWidget *MidasWidgetToMW(w)
Widget w;
/*
 *  This routine returns a pointer to the MidasWidget structure for any widget.
 */ 
{
    int hash = ((int) w) % HASHTABLESIZE;
    MidasWidget *mw = HashTable[hash];
  
    for ( ; mw != NULL; mw = mw->HashChain)
      if (mw->Widget == w) return mw;
  
    mw = XtNew(MidasWidget);
    mw->Widget       = w;
    mw->ShowChildren = False;
    mw->Map          = NULL;
    mw->IngotList    = NULL;
    mw->Parent       = NULL;
    mw->NameEntry    = NULL;
    mw->HashChain    = HashTable[hash];
    mw->Inited       = FALSE;
    HashTable[hash]  = mw;

    return mw;
}
static void MidasRegisterName(name,w)
char *name;
Widget w;
{
    ListItem *item = MidasFindItemInList(MidasWidgetList,name);
    if (item == NULL || ((MidasWidget *) item->Pointer)->Widget != w) 
      {
        MidasWidget *mw = MidasWidgetToMW(w);

        item = MidasAddItemToList(MidasWidgetList,name);
        item->Pointer = mw;
        mw->NameEntry = item;
      }
}
static void MidasAddChildTree(Child,mw)
Widget Child;
MidasWidget *mw;
{
  Widget w;
  MidasWidget *newmw = MidasWidgetToMW(Child);
  XmString label; 
  Arg arglist[3];
  int n = 0;
  
  if (WidgetTree == NULL) return; 

  label = MidasCharToString(XtName(Child)); 
   
  if (mw != NULL) { XtSetArg(arglist[n],XtNtreeParent,mw->Map); n++; }
  XtSetArg(arglist[n],XmNuserData,Child); n++;
  XtSetArg(arglist[n],XmNlabelString,label); n++;

  w = XmCreatePushButton(WidgetTree,"TreeLeaf",arglist,n);

  XmStringFree(label);

  XtManageChild(w);
  newmw->Map = w;
}   
static void MidasRemoveChildTree(Child)
Widget Child;
{
  MidasWidget *mw = MidasWidgetToMW(Child);
 
  if (mw->ShowChildren) MidasScanChildren(Child,MidasRemoveChildTree,NULL,NULL);

  if (mw->Map != NULL) 
    {
      XtDestroyWidget(mw->Map);
      mw->Map = NULL;
    }
  mw->ShowChildren = FALSE;   
}
static void MidasToggleChildrenTree(w)
Widget w;
{
  MidasWidget *mw = MidasWidgetToMW(w);
 
  if (WidgetTree == NULL) return;

  mw->ShowChildren = !mw->ShowChildren;
  if (mw->ShowChildren) MidasScanChildren(w,MidasAddChildTree,(XtPointer) mw,NULL);   
  else                  MidasScanChildren(w,MidasRemoveChildTree,NULL,NULL);    
}
void MidasSetupWidget(w)
Widget w;
{
    typedef struct _MR { char *Ingots; char *CreateCallback; } MidasResources;
    MidasResources result;
    MidasWidget *mw = MidasWidgetToMW(w);
    MidasWidget *parent = MidasWidgetToMW(XtParent(w));
    
#define Offset(field) XtOffsetOf(MidasResources,field)

    static XtResource resources[] = 
      {{"midasIngots"           ,"MidasIngots"         ,XtRString ,sizeof(char *),
         Offset(Ingots)         ,XtRImmediate          ,NULL},
       {"midasCreateCallback"   ,"MidasCreateCallback" ,XtRString ,sizeof(char *),
         Offset(CreateCallback) ,XtRImmediate          ,NULL}};

#undef Offset
    /*
     *  Check if widget already initialized
     */       

    if (mw->Inited) return; 
    mw->Inited = TRUE;
     
    /*
     * If the widgets parent has show children (in the widget tree)
     * then make it so
     */
     
    if (parent->ShowChildren) MidasAddChildTree(w,parent);
         
    /*  
     *  Add the special midas translation to all Widget children
     */

    if (XtIsWidget(w)) XtOverrideTranslations(w,MidasTranslations);

 
    XtAddCallback(w,XtNdestroyCallback,(XtCallbackProc) MidasWidgetBeingDestroyed,(XtPointer) mw);

    XtGetApplicationResources(w,(XtPointer)&result,resources,XtNumber(resources),NULL,0);
    if (result.Ingots != NULL) MidasSetupIngots(mw,result.Ingots);
    if (result.CreateCallback != NULL) 
      {
        MidasSetIngotString(w,"midasCreateCallback",result.CreateCallback);
        MidasDispatchCommandCallback(w,result.CreateCallback,NULL);
      }
}
Widget MidasCreateWidget(Parent,Class,Name)
Widget Parent;
WidgetClass Class;
char *Name;
{
    Widget w = XtCreateManagedWidget(Name,Class,Parent,NULL,0);
    MidasRegisterName(Name,w);
    MidasSetupWidget(w);
    return w;
}
Widget MidasFetch(name,parent)
char *name;
Widget parent;
{
    MrmType class;
    Widget actualParent, widget = 0;
    MidasFile *mfile;
    ListItem *i;
    Boolean Dialog = MidasGetQualifier("DIALOG");
    static LookingForParent = FALSE;

    if (!LookingForParent)
      { 
        LookingForParent = TRUE;
        MidasGetQualifier("PARENT",&parent);
        LookingForParent = FALSE;
      }

    for (i=MidasFileList->First; i != 0; i=i->Next)
      {
        mfile = i->Pointer;
        if (parent==0) actualParent = mfile->DefaultParent;
        else           actualParent = parent;
        if (Dialog) 
          {
            Arg args[10];
            int n = 0;
            char *dialogName = strcat(strcpy(XtMalloc(strlen(name)+7),name),"Dialog");
            Widget Dw = XmCreateDialogShell(actualParent,dialogName,NULL,0);
            XtSetArg(args[n],XmNx, 0); n++;
            XtSetArg(args[n],XmNy, 0); n++;
            if (MrmFetchWidgetOverride(mfile->Hierarchy,name,Dw,NULL,
                                       args,n,&widget,&class)
                ==MrmSUCCESS) 
              {
                MidasRegisterName(name,widget);
                MidasScanAncestors(Dw,MidasSetupWidget,NULL); 
                return widget;
              }
            XtDestroyWidget(Dw);
          }
        else 
          {
            if (MrmFetchWidget(mfile->Hierarchy,name,actualParent,&widget,&class) == MrmSUCCESS) 
              {
                /* 
                 * This might be a dialog box, in which case its parent is also created by
                 * this call.
                 */        
                Widget parent = XtParent(widget);

                MidasRegisterName(name,widget);
                if (XtIsShell(parent)) MidasScanAncestors(parent,MidasSetupWidget,NULL); 
                else                   MidasScanAncestors(widget,MidasSetupWidget,NULL);
                return widget;
              }
          }
      }
    return 0;
}
Pixmap MidasFetchIcon(name,ww)
char *name;
Widget ww;
{
    Arg arglist[1]; 
    Pixel f,b;
    Pixmap PixMap;
    ListItem *i;
    MidasSuppressXtWarningMessages();

    for (i=MidasFileList->Last; i != 0; i=i->Prev)
      {
        MidasFile *mfile = i->Pointer;
        Widget w = mfile->DefaultParent;
        Display *dpy = XtDisplay(w);
        Screen *screen = XtScreen(w);
        Cardinal depth;
        
        XtSetArg(arglist[0],XtNdepth,&depth);
        XtGetValues(w,arglist,1);
        
        if (depth == 1)
          {
            f = 1;
            b = 0; 
          }
        else if (ww)
          {
            XtSetArg(arglist[0],XtNbackground,&b);
            XtGetValues(ww,arglist,1);
            f = mfile->foreground;
           }
        else
          {
            f = mfile->foreground;
            b = mfile->background;
          }
  
        if (MrmFetchIconLiteral(mfile->Hierarchy,name,
                                screen, dpy, f, b, &PixMap)  
            == MrmSUCCESS)
        { 
          MidasReenableXtWarningMessages();
          return PixMap;
        } 
      }
    MidasReenableXtWarningMessages();
    return 0;
}
static Widget MidasFindName(name)
char *name;
{
    ListItem *item = MidasFindItemInList(MidasWidgetList,name);
    if (item != 0) 
      {
        MidasWidget *mw = item->Pointer;
        return mw->Widget;
      }
    else 
      {
        return MidasFetch(name,0);
      }
}
void MidasFetchDispatchEvent()
{
    XtAppProcessEvent(app_context,XtIMAll); 
}
void MidasGetCallbacks(L,w)
List *L;
Widget w;
{
    unsigned int numResource;
    XtResource *Resource, *R;

    WidgetClass Class = XtClass(w);

    MidasEmptyList(L);
    MidasAddItemToList(L,"midasCreateCallback");

    XtGetResourceList(Class,&Resource,&numResource);
    
    for (R=Resource; numResource-- > 0; R += 1)
      if (strcmp(R->resource_type,"Callback") == 0)
        {
           ListItem *item = MidasAddItemToList(L,R->resource_name);
        }
}
void MidasGetResources(L,w)
List *L;
Widget w;
{
    unsigned int numResource;
    XtResource *Resource, *R;

    WidgetClass Class = XtClass(w);

    MidasEmptyList(L);

    XtGetResourceList(Class,&Resource,&numResource);
    
    for (R=Resource; numResource-- > 0; R += 1)
      if (strcmp(R->resource_type,"Callback") != 0)
        {
           ListItem *item = MidasAddItemToList(L,R->resource_name);
        }
}
MidasOperand MidasModifier(modifierName)
char *modifierName;
{
  Boolean result = FALSE;
  MidasOperand Temp;
  XEvent *event = ActiveCallback ? ActiveCallback->event : NULL;

  if (event)
    { 
      if (event->type == ButtonPress || event->type == ButtonRelease)
        {
          XButtonEvent *be = (XButtonEvent *) event;
          if (!strcmp(modifierName,"shift")   && be->state & ShiftMask)   result = TRUE; 
          if (!strcmp(modifierName,"lock")    && be->state & LockMask)    result = TRUE; 
          if (!strcmp(modifierName,"control") && be->state & ControlMask) result = TRUE; 
        }
      if (event->type == KeyPress || event->type == KeyRelease)
        {
          XKeyEvent *ke = (XKeyEvent *) event;
          if (!strcmp(modifierName,"shift")   && ke->state & ShiftMask)   result = TRUE; 
          if (!strcmp(modifierName,"lock")    && ke->state & LockMask)    result = TRUE; 
          if (!strcmp(modifierName,"control") && ke->state & ControlMask) result = TRUE; 
        } 
    }
  Temp.Value.I = result;
  Temp.Dynamic = FALSE;
  Temp.Type = MBoolean;

  return Temp;
}
MidasOperand MidasGetAppResource(w,ResourceName)
Widget w;
char *ResourceName;
{
    static XtResource resources = 
      {"userData","UserData",XtRString,sizeof(char *),0,XtRString,""};

    char *result="";
    MidasOperand Temp;
    XtResource *rt = XtNew(XtResource);

    *rt = resources;
    rt->resource_name = XtNewString(ResourceName);
    XtGetApplicationResources(w,(XtPointer) &result,rt,1,NULL,0);

    Temp.Value.P = (XtPointer)result;
    Temp.Dynamic = FALSE;
    Temp.Type = MString;

    return Temp;
}
MidasOperand MidasGetValue(w,ResourceName)
Widget w;
char *ResourceName;
{
    MidasOperand Temp;
    char result[200], *ret;
    unsigned int numResource;
    int notfound = 1;
    XtResource *Resource, *R;
    WidgetClass Class = XtClass(w);


    /*  Special handling for midasCreateCallback resource
     */

    if (strcmp(ResourceName,"midasCreateCallback") == 0) return MidasGetCreateCallback(w);
      
    XtGetResourceList(Class,&Resource,&numResource);
    
    for (R=Resource; numResource-- > 0; R += 1)
      {
        if (strcmp(R->resource_name,ResourceName) == 0)
          {
            notfound = 0;
            if (strcmp(R->resource_type,XtRString) == 0)
              {
                char *ptr;
                int n=0;
                Arg args[10];
                XtSetArg(args[n],ResourceName,&ptr); n++;
                XtGetValues(w,args,n);
                ret = XtNewString(ptr);
              }
            else if (strcmp(R->resource_type,"XmString") == 0)  
              {
                String ptr;
                int n=0;
                Arg args[10];
                XtSetArg(args[n],ResourceName, &ptr); n++;
                XtGetValues(w,args,n);
                if (ptr == 0) ret = XtNewString("");
                else          ret = MidasStringToChar(ptr);
              }
            else
              {
                XrmValue from,to;
                if (strcmp(R->resource_type,"Callback") == 0)  
                  {
                    int n = 0;
                    Arg args[10];
                    XtSetArg(args[n],ResourceName, &from.addr); n++;
                    XtGetValues(w,args,n);
                  }
                else from.addr = ((char *) w) + R->resource_offset;  

                from.size = R->resource_size; 
                to.size = 200;
                to.addr = result;
                XtConvertAndStore(w,R->resource_type,&from,XtRString,&to);
                if (to.size > 200)
                  {
                    ret = XtMalloc(to.size);
                    to.addr = ret;
                    XtConvertAndStore(w,R->resource_type,&from,XtRString,&to);

                  }
                else ret = XtNewString(result);
             }
          }
       }
    XtFree((char *)Resource);
    if (notfound) MidasError("Resource %s not found",ResourceName);
    Temp.Value.P = ret;
    Temp.Dynamic = TRUE;
    Temp.Type = MString;
    return Temp;
}
MidasOperand MidasGetValueRaw(w,ResourceName)
Widget w;
char *ResourceName;
{
    MidasOperand Temp;
    unsigned int numResource;
    int notfound = 1;
    XtResource *Resource, *R;
    WidgetClass Class = XtClass(w);


    /*  Special handling for midasCreateCallback resource
     */

    if (strcmp(ResourceName,"midasCreateCallback") == 0) return MidasGetCreateCallback(w);
      
    XtGetResourceList(Class,&Resource,&numResource);
    
    for (R=Resource; numResource-- > 0; R += 1)
      {
        if (strcmp(R->resource_name,ResourceName) == 0)
          {
            char *addr =  (char *) w;
            addr += R->resource_offset;
            if (R->resource_size <= sizeof(Temp.Value.P))
              { 
                Temp.Value.P = NULL;
                memcpy(&Temp.Value.P,addr,R->resource_size); 
              }
            else Temp.Value.P = addr;         
            Temp.Type = R->resource_type;
            Temp.Dynamic = False;
            notfound = 0;
         }
       }
    XtFree((char *)Resource);
    if (notfound) MidasError("Resource %s not found",ResourceName);
    return Temp;
}
MidasShell *MidasGetShell(w)
Widget w;
{
    MidasShell *ms;
    ListItem *i;

    for (; ; w = XtParent(w))
      for (i = MidasShellList->First; i != 0; i = i->Next)
        {
          ms = i->Pointer;
          if (ms->Widget == w) return ms; 
        }
}      
void MidasDispatchAndTrapErrors(command)
char *command;
{
    if (setjmp(JmpEnv[NumJump++]) == 0)
      {
        MidasDispatchCommand(command);
        NumJump--;
      }
    else MidasError("Error interpreting command: %s\n       from widget %s (class %s)"
                    ,command,XtName(ActiveWidget),MidasClassName(ActiveWidget));
}
static void midas_command_proc(w,tag,reason)
    Widget w;
    char *tag;
    XmCommandCallbackStruct *reason;
{
    char *command = MidasStringToChar(reason->value);
    Widget Stack = ActiveWidget;
    Boolean StackIf = IfState;

    ActiveWidget = w;
    IfState = FALSE;
    
    MidasDispatchAndTrapErrors(command);

    ActiveWidget = Stack;
    IfState = StackIf;
    XtFree(command);
};
void MidasEchoCommand(command)
char *command;
{
    Widget w = MidasFindName("MidasCommand");
    if (w && XtIsRealized(w) && XtIsManaged(w))
      {   
        XmString s = MidasCharToString(command);

        if (w != 0) w = XmCommandGetChild(w,XmDIALOG_HISTORY_LIST);
        if (w != 0) 
          {  
            XmListAddItem(w,s,0);
            XmListSetBottomPos(w,0);
          }
        XmStringFree(s);
      }
}
void MidasDispatchCommandCallback(w,tag,reason)
    Widget w;
    char *tag;
    XmAnyCallbackStruct *reason;
{
    char buffer[2000];
    char *p = tag , *q = buffer;
    Boolean quotes = FALSE;
    Widget Stack = ActiveWidget;
    XmAnyCallbackStruct *CStack = ActiveCallback;

    ActiveWidget = w;
    if (reason) ActiveCallback = reason;    

    for (;; p++)
      {
        if (!quotes && (*p == '\n' || *p == ';')) 
          {
            *q = '\0';
            if (*buffer != '\0')
              {
                MidasEchoCommand(buffer);
                MidasDispatchAndTrapErrors(buffer);
              }
            q = buffer;
          }
        else if (*p == '\0') 
          {
            *q = '\0';
            if (*buffer != '\0')
              {
                MidasEchoCommand(buffer);
                MidasDispatchAndTrapErrors(buffer);
              }
            break;
          }
        else if (*p == '%') MidasForceEvaluateExpression(&p,&q);
        else 
          { 
            *q++ = *p;
            if (*p == '"') quotes = !quotes;
          }
      }
    ActiveWidget = Stack;
    ActiveCallback = CStack;
};
void MidasSendActionProc(w,event,params,num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    MidasDispatchCommandCallback(w,*params,NULL);    
}
void MidasSpecialActionProc(w,event,params,num_params)
    Widget w;
    XButtonEvent *event;
    String *params;
    Cardinal *num_params;
{
/*
 * Check to see if the event really came from some child gadget
 * or insenstive widget
 */    
    Widget OldWidget = 0;
 
    for ( ;w != OldWidget && XtIsComposite(w); )
      {
        Widget *list = DXmChildren(w);
        int n = DXmNumChildren(w);
       
        OldWidget = w;

        for (; n>0; n--, list++)
          {
            if (!XtIsWidget(*list) || !XtIsSensitive(*list))
              {
                Dimension width, height;
                Position x,y;
                Arg arglist[4];
                XtSetArg(arglist[0],XmNx,&x);
                XtSetArg(arglist[1],XmNy,&y);
                XtSetArg(arglist[2],XmNwidth ,&width);
                XtSetArg(arglist[3],XmNheight,&height);
                XtGetValues(*list,arglist,4);
                if (event->x >= x && event->x < x+width &&
                    event->y >= y && event->y < y+height) 
                  {
                    w = *list;
                    break;      
                  }
              }
          }
      }
    MidasDispatchCommandCallback(w,*params,NULL);    
}
void MidasQueueCommand(w,Command)
Widget w;
char *Command;
{
    QueuedCommand *QC = XtNew(QueuedCommand);
    QC->Widget = w;
    QC->Command = XtNewString(Command); 
    QC->Next = 0;
    if (CommandQueue.Head == 0) CommandQueue.Head = QC;
    else                        CommandQueue.Tail->Next = QC;
    CommandQueue.Tail = QC;
}
void MidasDispatchQueuedCommands()
{
    /*
     * Care is needed, reentrant routine 
     * (if queued command is Dispatch Queued Comands).
     */
    QueuedCommand *QC;
    for (QC = CommandQueue.Head; QC != 0; QC = CommandQueue.Head)
      {
        CommandQueue.Head = QC->Next;
        if (!QC->Next) CommandQueue.Tail = NULL;

        MidasDispatchCommandCallback(QC->Widget, QC->Command, 0);
        XtFree(QC->Command);
        XtFree((char *)QC);
      }
}
void MidasIgnore(Command)
char *Command;
{
    MidasDispatchCommandCallback(ActiveWidget,Command,NULL);
}
static void MidasIf(Doit,Command)
Boolean Doit;
char *Command;
{
    if (Doit) MidasDispatchCommandCallback(ActiveWidget,Command,ActiveCallback);
    IfState = Doit;
}
static void MidasElseIf(Doit,Command)
Boolean Doit;
char *Command;
{
    if (!IfState) MidasIf(Doit,Command);
}
static void MidasElse(Command)
char *Command;
{
    if (!IfState) MidasDispatchCommandCallback(ActiveWidget,Command,ActiveCallback);
}
void MidasBeep(pcent,w)
int pcent;
Widget w;
{
    if (w == NULL) w = ActiveWidget;
    XBell(XtDisplay(w),pcent);
}

/*
 * This procedure will ensure that, if a dialog window is being mapped,
 * its contents become visible before returning.  It is intended to be
 * used just before a bout of computing that doesn't service the display.
 * You should still call XmUpdateDisplay() at intervals during this
 * computing if possible.
 *
 * The monitoring of window states is necessary because attempts to map
 * the dialog are redirected to the window manager (if there is one) and
 * this introduces a significant delay before the window is actually mapped
 * and exposed.  This code works under mwm, twm, uwm, and no-wm.  It
 * doesn't work (but doesn't hang) with olwm if the mainwindow is iconified.
 *
 * The argument to ForceDialog is any widget in the dialog (often it
 * will be the BulletinBoard child of a DialogShell).
 */

void MidasForceDialog(w)
Widget w;
{
  Widget diashell, topshell;
  Window diawindow, topwindow;
  Display *dpy;
  XWindowAttributes xwa;
  XEvent event;
  XtAppContext cxt;

/* Locate the shell we are interested in.  In a particular instance, you
 * may know these shells already.
 */

  for (diashell = w;
       !XtIsShell(diashell);
       diashell = XtParent(diashell))
    ;

/* Locate its primary window's shell (which may be the same) */

  for (topshell = diashell;
       !XtIsTopLevelShell(topshell);
       topshell = XtParent(topshell))
    ;

  if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
    dpy = XtDisplay(topshell);
    diawindow = XtWindow(diashell);
    topwindow = XtWindow(topshell);
    cxt = XtWidgetToApplicationContext(diashell);

/* Wait for the dialog to be mapped.  It's guaranteed to become so unless... */

    while (XGetWindowAttributes(dpy, diawindow, &xwa),
           xwa.map_state != IsViewable) {

/* ...if the primary is (or becomes) unviewable or unmapped, it's
   probably iconified, and nothing will happen. */

      if (XGetWindowAttributes(dpy, topwindow, &xwa),
          xwa.map_state != IsViewable)
        break;

/* At this stage, we are guaranteed there will be an event of some kind.
   Beware; we are presumably in a callback, so this can recurse. */

      XtAppNextEvent(cxt, &event);
      XtDispatchEvent(&event);
    }
  }

/* The next XSync() will get an expose event if the dialog was unmapped. */

  XmUpdateDisplay(topshell);
}
void MidasSetCursor(w,c)
Widget w;
int c;
{
    Cursor C;
 
    if (c!=0) C = XCreateFontCursor(XtDisplay(w),c);
    else      C = None;

    XDefineCursor(XtDisplay(w),XtWindow(w),C);
    XFlush(XtDisplay(w));
}
void MidasSetSensitive(w,Sensitive)
Widget w;
Boolean Sensitive;
{
    XtSetSensitive(w,Sensitive);
}
char *MidasGetWidgetName(w)
Widget w;
{
    char *parent, *thisone, *new;
    int pl, tl;
    MidasShell *ms = MidasGetShell(w);

    thisone = XtName(w);
    if (*thisone == '\0' || strchr(thisone,'-') != 0)
      MidasError("Can not STORE RESOURCE for unnamed widget");

    parent = XtName(ms->Widget);
    pl = strlen(parent);
    tl = strlen(thisone);
    new = XtMalloc(pl+tl+2);

    strcpy(new,parent);
    if (ms->Widget != w)
      {
        strcat(new,"*");
        strcat(new,thisone);
      }

    return new;
}
void MidasSetValue(w,ResourceName,Op)
Widget w;
char *ResourceName;
MidasOperand *Op;
{
    char *result;
    unsigned int numResource;
    int found = FALSE;
    XtResource *Resource, *R;
    WidgetClass Class = XtClass(w);

    if (strcmp(ResourceName,"midasCreateCallback") == 0) 
      {
        MidasSetIngotP(w,ResourceName,Op);
        return;
      }

    XtGetResourceList(Class,&Resource,&numResource);

    for (R=Resource; numResource-- > 0; R += 1)
      {    
        if (strcmp(R->resource_name,ResourceName) == 0)
          {
            Arg arglist[1];

            found = TRUE;
                 
            MidasConvertOperand(Op,R->resource_type);
            XtSetArg(arglist[0],ResourceName,Op->Value.P);
            XtSetValues(w,arglist,1);              

            break;
          }
      }

    XtFree((char *) Resource);
    if (!found) MidasError("Resource %s not found",ResourceName);
}
static void MidasStoreAppResource(w,resource,value)
Widget w;
char *resource;
char *value;
{
    char *name = MidasGetWidgetName(w);
    char *new = XtMalloc(strlen(name)+strlen(resource)+2);
    MidasShell *ms = MidasGetShell(w);
    MidasApplication *ma = ms->Application;

    strcpy(new,name);
    strcat(new,".");
    strcat(new,resource);

    XrmPutStringResource(&ma->Database,new,value);
    if (ma->Changes++ == 0 && ma->AutoStore) 
      MidasQueueCommand(w,"Midas Save Stored Resources .");

    XtFree(new);
    XtFree(name);
}
static void MidasStoreNamedResource(w,name,resource)
Widget w;
char *name;
char *resource;
{
    MidasShell *ms = MidasGetShell(w);
    MidasApplication *ma = ms->Application;

    XrmPutStringResource(&ma->Database,name,resource);

    if (ma->Changes++ == 0 && ma->AutoStore) 
      MidasQueueCommand(w,"Midas Save Stored Resources .");
}
static void MidasUpdateNamedResource(w,name,resource)
Widget w;
char *name;
char *resource;
{
    XrmDatabase db = XtDatabase(XtDisplay(w));
    XrmPutStringResource(&db,name,resource);
}
static void MidasStoreResource(w,resource)
Widget w;
char *resource;
{
    MidasOperand Temp;
    Temp = MidasGetValue(w,resource);
 
    if (strcmp(Temp.Type,MString)) MidasConvertOperand(Temp,MString);
   
    MidasStoreAppResource(w,resource,Temp.Value.P);
    if (Temp.Dynamic) XtFree(Temp.Value.P);
}
static void MidasSaveStoredResources(w)
Widget w;
{
    MidasShell *ms = MidasGetShell(w);
    XrmPutFileDatabase(ms->Application->Database, ms->Application->Filename);
    ms->Application->Changes = 0; 
}
static void MidasSaveGeometry(w)
Widget w;
{
    MidasStoreResource(w,XmNx);
    MidasStoreResource(w,XmNy);
    MidasStoreResource(w,XmNwidth);
    MidasStoreResource(w,XmNheight);
    MidasSaveStoredResources(w);        
}
static MidasOperand MidasInt(f)
float f;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MInt;
    Temp.Value.I = (int) f;
    return Temp;
}
static void MidasSetFlag(w,flag)
Widget w;
int *flag;
{
  *flag = 1;
}
static MidasOperand MidasHasChildren(w)
Widget w;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MBoolean;
    Temp.Value.I = FALSE;
  
    MidasScanChildren(w,MidasSetFlag,(XtPointer) &Temp.Value.I,NULL); 
    return Temp;
}
static MidasOperand MidasChildrenInTree(w)
Widget w;
{
    MidasOperand Temp;
    MidasWidget *mw = MidasWidgetToMW(w);
    Temp.Dynamic = FALSE;
    Temp.Type = MBoolean;
    Temp.Value.I = mw->ShowChildren;
    
    return Temp; 
}
static MidasOperand MidasStrnspn(s,t)
char *s;
char *t;
/*
    Exceedingly temporary
*/
{
    MidasOperand Temp;
    char *p = s;
    for (;*p != '\0' && *p == ' '; p++);
    for (;*p != '\0' && *p != ' '; p++);
    *p = '\0';

    Temp.Dynamic = TRUE;
    Temp.Type = MString;
    Temp.Value.P = (XtPointer) XtNewString(s);

    return Temp;
}
static MidasOperand MidasGetWidgetId(w)
Widget w;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MInt;
    Temp.Value.I = (int) w;
    return Temp;
}
static MidasOperand MidasIsManaged(w)
Widget w;
{
    MidasOperand Temp;
    int result = XtIsManaged(w);
    Temp.Dynamic = FALSE;
    Temp.Type = MBoolean;
    Temp.Value.I = (result != 0); 
    return Temp;
}
static MidasOperand MidasIsRealized(w)
Widget w;
{
    MidasOperand Temp;
    int result = XtIsRealized(w);
    Temp.Dynamic = FALSE;
    Temp.Type = MBoolean;
    Temp.Value.I = (result != 0); 
    return Temp;
}
static MidasOperand MidasGetOperandType(Op)
MidasOperand *Op;
{
    MidasOperand Temp;
 
    Temp.Dynamic = FALSE;
    Temp.Type = MString;
    Temp.Value.P = Op->Type;
    return Temp;
}
static MidasOperand MidasWidgetFunction(w)
Widget w;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MWidget;
    Temp.Value.P = (XtPointer) w;
    return Temp;
}   
static MidasOperand MidasFetchFunction(name,parent)
char *name;
Widget parent;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MWidget;
    Temp.Value.P = (XtPointer) MidasFetch(name,parent);
    return Temp;
}
static MidasOperand MidasParent(w)
Widget w;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MWidget;
    Temp.Value.P = (XtPointer) XtParent(w);
    return Temp;
}
static MidasOperand MidasShellFunction(w)
Widget w;
{
    MidasOperand Temp;
    MidasShell *ms = MidasGetShell(w);
 
    Temp.Dynamic = FALSE;
    Temp.Type = MWidget;
    Temp.Value.P = (XtPointer) ms->Widget;
    return Temp;
}
static MidasOperand MidasWidgetName(w)
Widget w;
{
    MidasOperand Temp;
    char *name = XtName(w); 
  
    if (*name == '\0') 
      {
        name = XtMalloc(24);
        sprintf(name,"No Name (%d)",w);
        Temp.Dynamic = TRUE;
      }
    else Temp.Dynamic = FALSE;

    Temp.Value.P = name;
    Temp.Type = MString;
    return Temp;
}
static void MidasGetSelectionCallback(w,closure,selection,type,value,length,format)
Widget w;
XtPointer closure;
Atom *selection;
Atom *type;
XtPointer value;
unsigned long *length;
int *format;
{
  MidasOperand *Temp = (MidasOperand *) closure;
  int size = *length * *format; 

  if (*type == XA_STRING)
    {
      Temp->Value.P = (XtPointer) memcpy(XtMalloc(size+1),value,size);
      ((char *)Temp->Value.P)[size] = '\0';
      Temp->Type = MString;  
      Temp->Dynamic = TRUE;
    } 
  else if (*type == XA_INTEGER)
    {
      Temp->Value.I = *((int *) value);
      Temp->Type = MInt;
      Temp->Dynamic = FALSE;  
    }
  else 
    {
      Temp->Value.P = ""; 
      Temp->Type = MString;
      Temp->Dynamic = FALSE;
    } 
  XtFree(value);     
}
static MidasOperand MidasGetSelection(w,selection,target)
Widget w;
Atom selection;
Atom target;
{
    MidasOperand Temp;
  
    Temp.Value.P = NULL;

    /* w must be a real,realized widget */
    for (; w && (!XtIsWidget(w) || !XtIsRealized(w)); w = XtParent(w));  
    if (!w) MidasError("MidasGetSelection requires realized widget");

    XtGetSelectionValue(w,selection,target,(XtSelectionCallbackProc)MidasGetSelectionCallback,(XtPointer)&Temp,
                        XtLastTimestampProcessed(XtDisplay(w)));

    for (; !Temp.Value.P; ) MidasFetchDispatchEvent();
    return Temp;
}
static MidasOperand MidasGetProperty(w,property)
Widget w;
Atom property;
{
   Display *dpy = XtDisplay(w);
   Window  wind = XtWindow(w);
   Atom type;
   int format;
   unsigned long nitems;
   unsigned long bytes;  
   unsigned char *result;
   Boolean rc;
   MidasOperand Temp;
 
   rc = XGetWindowProperty(dpy,wind,property,0,1024,FALSE,AnyPropertyType,
                           &type,&format,&nitems,&bytes,&result);

   if (rc != Success) MidasError("Could not get property %s from window %x",XmGetAtomName(dpy,property),wind);
   if (type == XA_STRING)
     { 
       Temp.Value.P = (XtPointer) result;
       Temp.Type = MString;
       Temp.Dynamic = TRUE; 
     }
   else
     {
       XFree(result);
       MidasError("Property %s on window %x has unknown type: %s",
                  XmGetAtomName(dpy,property),wind,XmGetAtomName(dpy,type));
                                                                   
     }
   return Temp;
}
static MidasOperand MidasWidgetClass(w)
Widget w;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MString;
    Temp.Value.P = MidasClassName(w);
    return Temp;
}
static MidasOperand MidasGetCommand()
{
    MidasOperand Temp;
    XmCommandCallbackStruct *cb = (XmCommandCallbackStruct *) ActiveCallback;

    if (ActiveCallback->reason != XmCR_COMMAND_ENTERED) 
      MidasError("GETCOMMAND requires callback reason XmCR_COMMAND_ENTERED");

    Temp.Dynamic = TRUE;
    Temp.Type = MString;
    Temp.Value.P = MidasStringToChar(cb->value);
    return Temp;
}   
static MidasOperand MidasGetConfirmExit(w)
Widget w;
{
    MidasShell *ms = MidasGetShell(w);
    MidasApplication *ma = ms->Application;
    MidasOperand Temp;

    Temp.Dynamic = FALSE;
    Temp.Type = MBoolean;
    Temp.Value.I = ma->ConfirmExit;
    return Temp;
}
static MidasOperand MidasGetClickCount()
{
    MidasOperand Temp;
    XmPushButtonCallbackStruct *cb = (XmPushButtonCallbackStruct *) ActiveCallback;
    if (ActiveCallback->reason != XmCR_ACTIVATE) 
      MidasError("GETCLICKCOUNT requires callback reason XmCR_ACTIVATE");
  
    Temp.Dynamic = FALSE;
    Temp.Type = MInt;
    Temp.Value.I = cb->click_count;
    return Temp;
}
static MidasOperand MidasHyperGetText()
{
    MidasOperand Temp;
    SGMLHyperCallbackStruct *cb = (SGMLHyperCallbackStruct *) ActiveCallback;
/*
    if (ActiveCallback->reason != SGMLHYPER_REASON)
      MidasError("HYPERGETTEXT requires callback reason SGMLHYPER_REASON");
*/
    Temp.Dynamic = TRUE;
    Temp.Type = MString;
    Temp.Value.P = XtNewString(cb->text);
    return Temp;
}
static MidasOperand MidasHyperGetHidden()
{
    MidasOperand Temp;
    SGMLHyperCallbackStruct *cb = (SGMLHyperCallbackStruct *) ActiveCallback;
/*
    if (ActiveCallback->reason != SGMLHYPER_REASON)
      MidasError("HYPERGETTEXT requires callback reason SGMLHYPER_REASON");
*/
    Temp.Dynamic = TRUE;
    Temp.Type = MString;
    Temp.Value.P = XtNewString(cb->hidden);
    return Temp;
}
static MidasOperand MidasHyperGrep(w,string)
Widget w;
char *string;
{
    MidasOperand Temp;
    Boolean result = SGMLHyperGrep(w,string,TRUE,FALSE,TRUE);
 
    Temp.Value.I = result;
    Temp.Type = MBoolean;
    Temp.Dynamic = FALSE;
    return Temp;
}
#ifndef VMS
static String MidasGetRootDirName(buf)
String buf;
{
#ifndef X_NOT_POSIX
     uid_t uid;
#else
     int uid;
     extern int getuid();
#ifndef SYSV386
     extern struct passwd *getpwuid(), *getpwnam();
#endif
#endif
     struct passwd *pw;
     static char *ptr = NULL;

     if (ptr == NULL) {
	if (!(ptr = getenv("HOME"))) {
	    if (ptr = getenv("USER")) pw = getpwnam(ptr);
	    else {
		uid = getuid();
 		pw = getpwuid(uid);
	    }
	    if (pw) ptr = pw->pw_dir;
	    else {
		ptr = NULL;
		*buf = '\0';
	    }
	}
     }

     if (ptr)
 	(void) strcpy(buf, ptr);

     buf += strlen(buf);
     *buf = '/';
     buf++;
     *buf = '\0';
     return buf;
}
#endif
static char* MidasGetAppUserDefaults(dpy,name)
Display *dpy;
char *name;
{
    char *filename;
#ifdef VMS

    static char file[] = "DECW$USER_DEFAULTS:", ext[] = ".DAT"; 
    int totlen;

    totlen = strlen(file) + strlen(ext) + strlen(name) + 1;
    filename = XtMalloc(totlen);
    strcpy(filename,file);
    strcat(filename,name);
    strcat(filename,ext);

#else

    char* path;
    Boolean Alloc=False;
    char homedir[256];
    MidasGetRootDirName(homedir);

    if (!(path = getenv("XUSERFILESEARCHPATH"))) {
	char *old_path;
	if (!(old_path = getenv("XAPPLRESDIR"))) {
	    char *path_default = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
	    path = XtMalloc(6*strlen(homedir) + strlen(path_default));
	    sprintf( path, path_default,
		    homedir, homedir, homedir, homedir, homedir, homedir );
	} else {
	    char *path_default = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
	    path = XtMalloc( 6*strlen(old_path) + 2*strlen(homedir) + strlen(path_default));
	    sprintf(path, path_default, old_path, old_path, old_path, homedir,
		    old_path, old_path, old_path, homedir );
	}
        Alloc=True;
    }
    filename = XtResolvePathname(dpy,NULL,name,NULL,path,NULL,0,NULL);
    if (!filename) 
      {
         filename = XtMalloc(strlen(homedir)+strlen(name)+1);
         strcpy(filename,homedir);
         strcat(filename,name); 
      }
    if (Alloc) XtFree(path);
 
#endif
   return filename;
}
Widget MidasCreateDialog(name)
char *name;
{
    Widget parent, dialog;
    if (MidasGetQualifier("PARENT",&parent) == 0) MidasError("Parent required");
    dialog = XmCreateDialogShell(parent,name,NULL,0);
    MidasRegisterName(name,dialog);
    return dialog;
}
void MidasDestroyApplication(ma)
MidasApplication *ma;
{
  /* 
   * /* BUG - What if the database hasn't been written out?
   */

  if (ma->Help) XtDestroyWidget(ma->Help);
  XtCloseDisplay(ma->Display);
  XtFree(ma->AppName);
  XtFree(ma->AppClass);
  MidasRemoveItemFromList(MidasApplicationList,ma->Item);
}
void MidasDestroyShell(w,ms)
Widget     w;
MidasShell *ms;
{
  MidasApplication *ma = ms->Application;
  MidasRemoveItemFromList(MidasShellList,ms->Item);
 
  ma->Shells--;
  if (ma->Shells == 0) MidasDestroyApplication(ma); 
}
MidasApplication *MidasCreateApplication(name,argv,argc)
char *name;
String *argv;
Cardinal *argc;
{
    MidasApplication *ma = XtNew(MidasApplication);
    char *ShellName = XtNewString(name);
    char *ShellClass = XtNewString(name);
    char *new; 
    ListItem *item;

    *ShellName = tolower(*ShellName);
    *ShellClass = toupper(*ShellClass);

    display = XtOpenDisplay(app_context,NULL,NULL,ShellClass,
                                        NULL, 0, argc, argv);
    if (display == 0) { MidasError("Could not open display"); exit(FAILURE); }

    new = MidasGetAppUserDefaults(display,ShellClass);

    item = MidasAddItemToList(MidasApplicationList,name);
    item->Pointer = ma;

    ma->Display     = display;
    ma->Database    = XrmGetFileDatabase(new);
    ma->Filename    = new;
    ma->Changes     = 0; 
    ma->Item        = item;
    ma->Help        = NULL;
    ma->AppClass    = ShellClass;
    ma->AppName     = ShellName; 
    ma->Shells      = 0;
    ma->Argc        = *argc;
    ma->Argv        = argv; 
    ma->ConfirmExit = TRUE;
    ma->AutoStore   = TRUE;  

    return ma;
}
MidasApplication *MidasGetApplication(name)
char *name;
{
  ListItem *item = MidasFindItemInList(MidasApplicationList,name);
  if (item) return item->Pointer;
  else      return NULL; 
}
Widget MidasCreateShell(name)
char *name;
{
  MidasApplication *ma = MidasGetApplication(name);
  MidasShell       *ms = XtNew(MidasShell);
  ListItem *item; 
  Widget w;
  char junk[12]; 
  int i; 

  w = XtAppCreateShell(ma->AppName,ma->AppClass,
                       applicationShellWidgetClass,
                       ma->Display, NULL, 0);
   
  item = MidasAddItemToList(MidasShellList,name);
  item->Pointer = ms;

  ma->Shells++; 

  ms->Widget = w;
  ms->Application = ma;
  ms->Item = item;

  XtAddCallback(w,XtNdestroyCallback,(XtCallbackProc) MidasDestroyShell,(XtPointer)ms); 

  MidasSetupWidget(w);
  MidasRegisterName(name,w);

#ifdef EDITRES
    XtAddEventHandler(w, (EventMask)0, True,
	              _XEditResCheckMessages, (XtPointer)NULL);
#endif

    /* Shells are always displayed in the Widget Tree */
    
    MidasAddChildTree(w,NULL);
 
    /* Put the command line options into ingots so they can be access by the application */

    sprintf(junk,"%d",ma->Argc);
    MidasSetIngotString(w, "argc", junk);
    for (i=0; i < ma->Argc; i++)
      {
        sprintf(junk,"argv[%d]",i);
        MidasSetIngotString(w,junk,ma->Argv[i]); 
      }
    return w;
}
static void MidasSetConfirmExit(w,value)
Widget w;
Boolean value;
{
  MidasShell *ms = MidasGetShell(w);
  MidasApplication *ma = ms->Application;
  ma->ConfirmExit = value; 
}
static MidasOperand MidasTreeToWidget(w)
Widget w;
{
  MidasOperand Temp;
  Arg arglist[1];
  XtSetArg(arglist[0],XmNuserData,&Temp.Value.P);
  XtGetValues(w,arglist,1);
  
  Temp.Type = MWidget;
  Temp.Dynamic = FALSE;
   
  return Temp;
}
void MidasCreateWidgetTree(Tree)
Widget Tree;
{
  ListItem *item;
 
  WidgetTree = Tree;

  for (item = MidasShellList->First; item != 0; item = item->Next)
    {
      MidasShell *ms = (MidasShell *) item->Pointer;
      MidasAddChildTree(ms->Widget,NULL);   
    }   
}
static void MidasInvertWidget(w)
Widget w;
{ 
  Arg arglist[2];
  Pixel fg; 
  Pixel bg;  

  XtSetArg(arglist[0],XmNforeground,&fg);
  XtSetArg(arglist[1],XmNbackground,&bg);
  XtGetValues(w,arglist,2);

  XtSetArg(arglist[0],XmNforeground,bg);
  XtSetArg(arglist[1],XmNbackground,fg);
  XtSetValues(w,arglist,2);
}
static void MidasCurrentWidgetInTree(w)
Widget w;
{
  static Widget OldWidget = NULL;
  Widget TreeW; 
  MidasWidget *mw, *parent;
  
  if (OldWidget != NULL)
    {
      mw = MidasWidgetToMW(OldWidget);
      TreeW = mw->Map;
      if (TreeW != NULL) MidasInvertWidget(TreeW);
    }
  OldWidget = w;
  
  mw = MidasWidgetToMW(w); 

  /* if this widget isnt in the tree, make it so */ 

  for (; mw->Map  == NULL; )
    {
      for (parent = mw; parent->Map == NULL ; parent = MidasWidgetToMW(XtParent(parent->Widget)) );
      MidasToggleChildrenTree(parent->Widget); 
    }

  TreeW = mw->Map;
  MidasInvertWidget(TreeW); 

  /* it may be necessary to scroll the tree to make this widget visible */

  {
     Arg arglist[10];
     int n=0;
     Widget h_scroll, v_scroll;
  
     Widget Tree = XtParent(TreeW);
     Widget Clip = XtParent(Tree);
     Widget Swin = XtParent(Clip);

     Position    x_button,y_button;
     Dimension   h_button,w_button;
     Position    x_clip,y_clip;
     Dimension   h_clip,w_clip;
     Position    dv=0,dh=0;
     int min,max;
     int v_val,v_size,v_inc,v_page;
     int h_val,h_size,h_inc,h_page;
     Position x,y;

    /* Get window scroll bars */
 
     XtSetArg(arglist[n],XmNhorizontalScrollBar, &h_scroll); n++;
     XtSetArg(arglist[n],XmNverticalScrollBar,   &v_scroll); n++;
     XtGetValues(Swin,arglist,n);
      
    /* Get size of clip window and button */

     n = 0;
     XtSetArg(arglist[n],XmNwidth,  &w_clip); n++;
     XtSetArg(arglist[n],XmNheight, &h_clip); n++;
     XtGetValues(Clip,arglist,n);

     n = 0;
     XtSetArg(arglist[n],XmNwidth,  &w_button); n++;
     XtSetArg(arglist[n],XmNheight, &h_button); n++;
     XtGetValues(TreeW,arglist,n);

    /* Get global coordinates of clip and selection rect */

     XtTranslateCoords(Clip,0,0,&x_clip,&y_clip);
     XtTranslateCoords(TreeW,0,0,&x_button,&y_button);

    /* offset of selection within clip window */

     x = x_button - x_clip;
     y = y_button - y_clip;

    /* selection y coordinate is not visible */

     if( y < 0 || y + h_button > h_clip)
     {
        /* the widget must be moved verticaly by dv pixels */

        dv = (y + h_button / 2)  - h_clip / 2;

        n = 0;
        XtSetArg(arglist[n],XmNminimum,&min); n++;
        XtSetArg(arglist[n],XmNmaximum,&max); n++;

        XtGetValues(v_scroll,arglist,n);

        XmScrollBarGetValues(v_scroll,&v_val,&v_size,&v_inc,&v_page);

        max -= v_size;

        if( dv + v_val > max ) dv = max - v_val;
        if( dv + v_val < min ) dv = min - v_val;
     }

    /* selection x coordinate is not visible */

     if( x < 0 || x + w_button > w_clip)
     {
        /* the widget must be moved horizontaly by dh pixels */

        dh = (x + w_button / 2)  - w_clip / 2;

        n = 0;
        XtSetArg(arglist[n],XmNminimum,&min); n++;
        XtSetArg(arglist[n],XmNmaximum,&max); n++;
        XtGetValues(h_scroll,arglist,n);

        XmScrollBarGetValues(h_scroll,&h_val,&h_size,&h_inc,&h_page);

        max -= h_size;

        if( dh + h_val > max ) dh = max - h_val;
        if( dh + h_val < min ) dh = min - h_val;

     }

    /* if the widget must be moved */

     if(dv || dh)
     {
        Position x,y;
  
        n = 0;
        XtSetArg(arglist[n],XmNx,&x); n++;
        XtSetArg(arglist[n],XmNy,&y); n++;
        XtGetValues(Tree,arglist,n);
 
        x -= dh;
        y -= dv;

        /* move it */

        n = 0;
        XtSetArg(arglist[n],XmNx,x); n++;
        XtSetArg(arglist[n],XmNy,y); n++;
        XtSetValues(Tree,arglist,n);

        /* update scroll bars */

        if(dv) XmScrollBarSetValues(v_scroll,v_val+dv,v_size,v_inc,
            v_page,TRUE);
        if(dh) XmScrollBarSetValues(h_scroll,h_val+dh,h_size,h_inc,
            h_page,TRUE);

      }
    }

}
Widget MidasScanWidgetTree(root,path,scnrtn,closure)
Widget root;
char *path;
MidasWidgetScanProc scnrtn;
XtPointer closure;
{
    Widget w = root;

    if (strcmp(path,"") ==  0) 
      {
        if (scnrtn) return (*scnrtn)(w,closure);
        else        return w;
      }
    else if (strncmp("^^^",path,3) == 0) 
      {
        path += 3;
        for (w = XtParent(w); w != 0; w = XtParent(w)) 
          {
            Widget result = MidasScanWidgetTree(w,path,scnrtn,closure);
            if (result != 0) return result;
          }
        return 0;
      }
    else if (*path == '^') 
      {
        path += 1;
        w = XtParent(w);
        return MidasScanWidgetTree(w,path,scnrtn,closure);
      }
    else if (strncmp("###",path,3) == 0)
      {
         char t,*p;
         Widget result; 
         strncpy(path,"...",3);   
         result = MidasScanWidgetTree(w,path,scnrtn,closure);
         strncpy(path,"###",3);
         if (result) return result;
         
         for (p = path+3 ; *p != '\0' && *p != '.' && *p != '#' && *p != '^' ;p++);
         
         t = *p;
         *p = '\0'; 
         result= MidasFetch(path+3,w);
         if (!result) return result;
         
         *p = t;
         return MidasScanWidgetTree(result,p,scnrtn,closure);
      }
    else if (strncmp("...",path,3) == 0) 
      {
        Widget *list, *save;  
        int n , nsave;
      
        if (XtIsSubclass(w,compositeWidgetClass) != 0)
          { 

            save = DXmChildren(w);  
            nsave = DXmNumChildren(w);

            path += 3; 
            for (list = save , n = nsave; n > 0; n-- , list++) 
              { 
                Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                if (result != 0) return result;
              }
            path -= 3;
            for (list = save , n = nsave; n > 0; n-- , list++) 
              { 
                Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                if (result != 0) return result;
              }
          }
        if (XtIsWidget(w))
          {
            save = w->core.popup_list;
            nsave = w->core.num_popups;       
       
            path += 3; 
            for (list = save , n = nsave; n > 0; n-- , list++) 
              { 
                Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                if (result != 0) return result;
              }
            path -= 3;
            for (list = save , n = nsave; n > 0; n-- , list++) 
              { 
                Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                if (result != 0) return result;
              }            
          }
	if (SGMLIsCompositeText(w))
	   {
             Arg arglist[10];
             int nn = 0;

             XtSetArg(arglist[nn],XtNchildren,&save); nn++;
             XtSetArg(arglist[nn],XtNnumChildren,&nsave); nn++;
             XtGetValues(w,arglist,nn);

             path += 3; 
             for (list = save , n = nsave ; n > 0; n-- , list++) 
               {
                 Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                 if (result != 0) return result;
               }  
             path -= 3;
             for (list = save , n = nsave ; n > 0; n-- , list++) 
               {
                 Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                 if (result != 0) return result;
               }  
	   }
        return 0;
      }
    else if (*path == '.') 
      {
        Widget *list, *save;  
        int n, nsave;  
        path++;
      
        if (XtIsSubclass(w,compositeWidgetClass))
          { 
            list = DXmChildren(w); 
            n = DXmNumChildren(w);   

            for (; n > 0; n-- , list++) 
              { 
        	Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
        	if (result != 0) return result;
              }
           }   
        if (XtIsWidget(w))
          {
            save = w->core.popup_list;
            nsave = w->core.num_popups;       
       
            for (list = save , n = nsave; n > 0; n-- , list++) 
              { 
                Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                if (result != 0) return result;
              }
          }
	if (SGMLIsCompositeText(w))
	   {
             Arg arglist[10];
             int nn = 0;

             XtSetArg(arglist[nn],XtNchildren,&list); nn++;
             XtSetArg(arglist[nn],XtNnumChildren,&n); nn++;
             XtGetValues(w,arglist,nn);

             for (; n > 0; n-- , list++) 
               {
                 Widget result = MidasScanWidgetTree(*list,path,scnrtn,closure);
                 if (result != 0) return result;
               }  
 	   }

        path--;
        return 0;
      }
    else
      {
        char temp, *p, *e = 0;

        for (p=path; *p != '.' && *p != '^' && *p != '#' && *p != '\0'; p++) 
          {
            if (*p == '=') e = p;
          }

        temp = *p;
        *p = '\0';               


        if (e == 0)
          {
            if (strcmp(path,"*") == 0) 
              {
                *p = temp;
                return MidasScanWidgetTree(w,p,scnrtn,closure);
              }
            else
              {
                if (strcmp(XtName(w),path) == 0)
                  {
                    *p = temp;
                    return MidasScanWidgetTree(w,p,scnrtn,closure);
                  }
                else if (strcmp(MidasClassName(w),path) == 0)
                  {
                    *p = temp;
                    return MidasScanWidgetTree(w,p,scnrtn,closure);
                  }
                else
                  {
                    *p = temp;
                    return 0;
                  }
              }
          }
        else
          {
            unsigned int numResource;
            XtResource *Resource, *R;
            WidgetClass Class = XtClass(w);

            XtGetResourceList(Class,&Resource,&numResource);
    
            *e = '\0';

            for (R=Resource; numResource-- > 0; R += 1)
              if (strcmp(R->resource_name,path) == 0)
                {
                  XtPointer ptr , result;
                  XrmValue from,to;
                  int n = 0;
                  Arg args[10];
                  XtSetArg(args[n],path,&result); n++;

                  XtGetValues(w, args, n);
                  *e = '=';
                  *p = temp;

                  from.size = strlen(e+1)+1; 
                  from.addr = e+1;
                  to.size = sizeof(XtArgVal);
                  to.addr = (XtPointer) &ptr;
                  XtConvertAndStore(w,XtRString,&from,R->resource_type,&to);
                  if (to.size > sizeof(XtArgVal)) 
                  {
                    Widget res;
                    XtPointer ptr = XtMalloc(to.size);
                    to.addr = ptr;
                    XtConvertAndStore(w,XtRString,&from,R->resource_type,&to);

                    if (memcmp(to.addr,result,to.size) == 0) 
                      res = MidasScanWidgetTree(w,p,scnrtn,closure);
                    else res = NULL;

                    XtFree((char *) ptr);
                    return res;
                  }
                else
                  {
                    if (memcmp(to.addr,result,to.size) == 0) 
                      return MidasScanWidgetTree(w,p,scnrtn,closure);
                    else return 0;
                  }
                } 

            *e = '=';
            *p = temp;
 
            return 0;
          }
      }
}
Widget MidasTraceWidgetTree(root,path)
Widget root;
char *path;
{
  return MidasScanWidgetTree(root,path,NULL,NULL);
}
static MidasOperand MidasTraceWidgetTreeFunction(w,path)
Widget w;
char *path;
{
    MidasOperand Temp;
    Temp.Dynamic = FALSE;
    Temp.Type = MWidget;
    Temp.Value.P = (XtPointer) MidasTraceWidgetTree(w,path);
    return Temp;
}
Widget MidasFindWidget(name)
char *name;
{
    Widget w, root;
    char temp, *p, *path = XtNewString(name);

    for (p = path+1; *p != '.' && *p != '^' && *p != '\0'; p++);

    temp = *p;
    *p = '\0';

    if (strcmp(path,".") == 0) root = ActiveWidget;
    else                       root = MidasFindName(path);

    if (root == 0) MidasError("Could not find widget %s",path);

    *p = temp;
    w = MidasTraceWidgetTree(root,p);
 
    if (w == 0) MidasError("Could not follow path %s",path);

    XtFree(path);
    return w;
}
static MidasWidgetScanProc MidasForAllScan(w,command)
Widget w;
char *command;
{
  MidasDispatchCommandCallback(w,command,NULL);
  return NULL;
}
static void MidasForAll(name,command)
char *name;
char *command;
{
    Widget w, root;
    char temp, *p, *path = XtNewString(name);

    for (p = path+1; *p != '.' && *p != '^' && *p != '\0'; p++);

    temp = *p;
    *p = '\0';

    if (strcmp(path,".") == 0) root = ActiveWidget;
    else                       root = MidasFindName(path);

    if (root == 0) MidasError("Could not find widget %s",path);

    *p = temp;
    MidasScanWidgetTree(root,p,MidasForAllScan,command);

    XtFree(path);
}
static void MidasDestroy(widget)
Widget widget;
{
    XtDestroyWidget(widget);
}
static void MidasQuitMidas()
/*
 * Like totally
 */
{
    exit(SUCCESS);
}
void MidasClearWindow(w)
Widget w;
{
    XClearArea(XtDisplay(w),XtWindow(w),0,0,0,0,TRUE);
}
void MidasPopup(widget)
Widget widget;
{
    if (XtIsManaged(widget)==0) XtManageChild(widget);
    else if (XtIsRealized(widget)) 
      {
         Widget parent = XtParent(widget);
         if (XtIsShell(parent)) 
           {
             Display *dpy = XtDisplay(parent);
             Window  wind = XtWindow(parent);
             XWindowAttributes attr;

             XGetWindowAttributes(dpy,wind,&attr); 
             if (attr.map_state == IsUnmapped) XMapWindow(dpy,wind);
             else                              XRaiseWindow(dpy , wind);
           }         }

    if (XtIsRealized(widget)==0)
      {
        MidasShell *ms = MidasGetShell(widget);
        XtRealizeWidget(ms->Widget);
      }
}
static void MidasPopdown(widget)
Widget widget;
{
    MidasShell *ms = MidasGetShell(widget);

    if (ms->Widget == widget) widget = MidasTraceWidgetTree(widget,".");
    if (XtIsManaged(widget)) XtUnmanageChild(widget);
    if (XtParent(widget) == ms->Widget) XtUnrealizeWidget(ms->Widget); 

}
static void MidasToggle(widget)
Widget widget;
{
    if (XtIsManaged(widget)) MidasPopdown(widget);
    else                     MidasPopup(widget);
}
static void MidasCenterHorizontal(w,position)
Widget w;
int position;
{
    Dimension width;
    Position  offset;
    Widget parent = XtParent(w);
    int n = 0;
    Arg args[10];

    XtSetArg(args[n],XmNwidth, &width); n++;

    if (XmIsForm(parent) == 0)
      MidasError("Can only center widgets whose parent is FORM");

    XtGetValues(w,args,n);

    offset = -((int) width)/2;
      {
        int n = 0;
        Arg args[10];
        XtSetArg(args[n],XmNleftAttachment, XmATTACH_POSITION); n++;
        XtSetArg(args[n],XmNleftOffset,     offset);   n++, 
        XtSetArg(args[n],XmNleftPosition,   position); n++;

        XtSetValues(w,args,n);
      }
}
static void Set1Flag(w,flag)
Widget w;
int *flag; 
{
  *flag = 1;
}
static void Set2Flag(w,flag)
Widget w;
int *flag; 
{
  *flag = 2;
}
static void ClearFlag(w,flag)
Widget w;
int *flag; 
{
  *flag = 0;
}

static Widget WhyDoIHaveToDoThis(w,closure)
Widget w;
XtPointer closure;
{
  XtRemoveAllCallbacks(w,XtNdestroyCallback);
  return NULL;
}
/*
 * Quit applcation closes a complete application. It:
 *   a) Checks if there are any unsaved changes in the resource database, and if so
 *      asks the user whether to save them.
 *   b) If confirm exit is set for the shell, asks the user if he really wants to exit.
 *   c) If there are no more active applications, exits the program.       
 *   d) Destroys all the shells in the application.
 */
static void MidasQuitApplication(w)
Widget w;
{
  MidasShell *ms;
  MidasApplication *ma;
  int mods;  
  ListItem *item;
  int Nshell = 0;

  if (w == NULL) w = ActiveWidget;
  ms = MidasGetShell(w);
  ma = ms->Application; 
  
  if (ma->Changes > 0 || ma->ConfirmExit)
    {
      char message[255];
      XmString Message; 
      XmString Yes    = MidasCharToString("Yes");
      XmString No     = MidasCharToString("No");
      XmString Cancel = MidasCharToString("Cancel");
      XmString ConfirmExit = MidasCharToString("Confirm Exit");
      int ExitResponse = -1;              
      Arg arglist[10];
      int n = 0;
      Widget w;

      if (ma->Changes > 0) sprintf(message,"Save changes before quitting %s?",ma->AppClass);
      else                 sprintf(message,"Do you really want to quit %s?"  ,ma->AppClass);

      Message = MidasCharToString(message);  
      XtSetArg(arglist[n],XmNmessageString,Message); n++;  
      XtSetArg(arglist[n],XmNdialogTitle,ConfirmExit); n++;
      XtSetArg(arglist[n],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); n++;
      XtSetArg(arglist[n],XmNmessageAlignment,XmALIGNMENT_CENTER); n++;
      XtSetArg(arglist[n],XmNokLabelString,Yes); n++;
      XtSetArg(arglist[n],XmNcancelLabelString,No); n++;
      XtSetArg(arglist[n],XmNhelpLabelString,Cancel); n++;

      w = XmCreateQuestionDialog(ms->Widget,"ExitDialog",arglist,n);

      XtAddCallback(w,XmNokCallback    ,(XtCallbackProc) Set1Flag  ,(XtPointer) &ExitResponse);
      XtAddCallback(w,XmNcancelCallback,(XtCallbackProc) ClearFlag ,(XtPointer) &ExitResponse);
      XtAddCallback(w,XmNhelpCallback  ,(XtCallbackProc) Set2Flag  ,(XtPointer) &ExitResponse);

      if (ma->Changes == 0)
        {
          MidasPopdown(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
        }

      XtManageChild(w);

      for (;ExitResponse == -1;) MidasFetchDispatchEvent();

      XmStringFree(Message);
      XmStringFree(Yes);
      XmStringFree(No);
      XmStringFree(Cancel);
      XmStringFree(ConfirmExit);

      if (ExitResponse == 2)                    /* Cancel */   
        {
          XtDestroyWidget(w);
          return;    
        }
      if (ExitResponse == 1 && ma->Changes > 0) /* Yes    */
        {
          XrmPutFileDatabase(ma->Database, ma->Filename);
          ma->Changes = 0; 
        }
    }
  /*
   * Ok, we really want to quit, are we the only active application?
   */
  for (item = MidasShellList->First; item != NULL; item = item->Next) 
    {
      MidasShell *s = (MidasShell *) item->Pointer;
      if (s->Application != ma && XtIsRealized(s->Widget)) Nshell++;
    } 

  if (Nshell == 0) 
    {
      XtCallCallbacks(ms->Widget,XtNpopdownCallback,NULL);
      exit(SUCCESS);
    }
  for (item = MidasShellList->First; item != NULL; item = item->Next)
    {   
      MidasShell *s = (MidasShell *) item->Pointer;
      if (s->Application  == ma) 
        {
          XtCallCallbacks(s->Widget,XtNpopdownCallback,NULL);
          MidasPopdown(s->Widget);
          MidasScanWidgetTree(s->Widget,"...XmMainWindow.XmDrawingArea.",WhyDoIHaveToDoThis,NULL); 
          MidasScanWidgetTree(s->Widget,"...XmMainWindow.",WhyDoIHaveToDoThis,NULL); 
          MidasQueueCommand(s->Widget,"Destroy .");
        }
    }
} 
/*
 *  Midas Quit Shell destroys a specific shell. If it is the only shell in the 
 *  application then it closes the application.    
 */
static void MidasQuitShell(w)
Widget w;
{
  MidasShell *ms;

  if (w == NULL) w = ActiveWidget;
  ms = MidasGetShell(w);
  if (ms->Application->Shells == 1) 
    {
      MidasQuitApplication(w);
      return;
    }
  /* We usurp the unused PopDonw callback of the applicationShell as a way of providing
   * a Destroy callback for the shell.
   */
 
  XtCallCallbacks(ms->Widget,XtNpopdownCallback,NULL);

  /* Some versions of the toolkit seem to have problems destroying main windows 
   * The following seems to circumvent the problems.
   */ 

  MidasPopdown(ms->Widget); 
  MidasScanWidgetTree(ms->Widget,"...XmMainWindow.XmDrawingArea.",WhyDoIHaveToDoThis,NULL); 
  MidasScanWidgetTree(ms->Widget,"...XmMainWindow.",WhyDoIHaveToDoThis,NULL); 
  MidasQueueCommand(ms->Widget,"Destroy .");
}
static Widget MidasInvokeApplication(command)
char *command;
{
    Widget toplevel_widget;
    MidasOperand Temp;
    char *argv[20];
    int  argc;

    MidasTokenizeCommand(command,argv,&argc,XtNumber(argv)," ");
    
    MidasCreateApplication(argv[0],argv,&argc);
    toplevel_widget = MidasCreateShell(argv[0]);

    Temp = MidasGetAppResource(toplevel_widget,"startup");

    if (*((char *)Temp.Value.P) != '\0') MidasQueueCommand(toplevel_widget,Temp.Value.P);
    return toplevel_widget;
}                         
/*
 * This procedure is useful for kicking widgets that refuse to notice
 * that their children have changed size
 */ 
static void MidasKick(w)
Widget w;
{ 
    if (XtIsWidget(w)) 
      {
        WidgetClass class = XtClass(w);
        if (class->core_class.resize) (*class->core_class.resize)(w);
      } 
}
static void MidasHelpOnContext(w)
Widget w;
{
    MidasShell *ms = MidasGetShell(w);
    Cursor C =  XCreateFontCursor(XtDisplay(w),XC_question_arrow);
    Widget new = XmTrackingLocate(ms->Widget,C,FALSE);

    for (; new != NULL; new = XtParent(new))
      if (XtHasCallbacks(new,XmNhelpCallback) == XtCallbackHasSome)
        {
          XtCallCallbacks(new,XmNhelpCallback,NULL);
          break;
        }
    XFreeCursor(XtDisplay(w),C);
} 
/*-------------------------------------------------------------
** Trap any errors that occur as a result of trying to 
** communicate with MidasWWW  
*/
static Boolean ErrorFlag; 
static int (*OldErrorHandler)();
 
static int MidasHelpErrorHandler(dpy,event)
Display *dpy;
XErrorEvent *event;
{
  if (event->error_code == BadWindow) ErrorFlag = True;
  else OldErrorHandler(dpy,event); 
  return 0; 
}
static void MidasHelp(w,url)
Widget w;
char *url;
{
    Display *dpy = XtDisplay(w);
    Window root  = XRootWindowOfScreen(XtScreen(w));
    Atom urlAtom = XInternAtom(dpy,"URL",FALSE); 
    Atom prop    = XInternAtom(dpy,"midaswww",FALSE);

    Atom type;
    int format, rc;
    unsigned long items, bytes;
    Window *windowList; 
    char *startCommand = "midaswww";
    char command[500];
#ifdef VMS
    int flags = 1; /* NOWAIT */
    int d[2]; 
#endif
    /*
     * Get the list of registered MidasWWW windows by accessing the appropriate 
     * property on the root window.
     */
  
    XGrabServer(dpy);
    rc = XGetWindowProperty(dpy,root,prop,0,100,FALSE,XA_WINDOW,
                          &type,&format,&items,&bytes,(unsigned char **)&windowList);
    if (rc == Success) 
      {

      /*
       * OK, property already exists, but beware, window may not actually exist, so be
       * ready to trap error. (This can happen if MidasWWW client somehow exited without
       * cleaning up the root window property). 
       */
      int try; /* window to try */

      OldErrorHandler = XSetErrorHandler(MidasHelpErrorHandler);
    
      for (try = 0; try < items ; try++)
        {        
          ErrorFlag = FALSE;
          XChangeProperty(dpy,windowList[try],urlAtom,XA_STRING,8,
                          PropModeReplace,(unsigned char *) url,strlen(url));
          XSync(dpy,FALSE); /* wait for any error to come back */
     
          if (!ErrorFlag) break;
        }

      /* to be sociable we will clean up any spurious entries we found */
      
      if (try > 0)
        {
          if (ErrorFlag) XDeleteProperty(dpy,root,prop);
          else XChangeProperty(dpy,root,prop,XA_WINDOW,32,PropModeReplace,
                               (unsigned char *) windowList + try, items - try);
        }
      XUngrabServer(dpy);
      XFlush(dpy);       
      XSetErrorHandler(OldErrorHandler); 
      XFree((char *) windowList);
      if (!ErrorFlag) return;                   
    }
  else XUngrabServer(dpy);
     
  /*
   * Have to start a new client 
   */  
#ifdef VMS
  sprintf(command,"%s %s",startCommand,url);
  d[0] = strlen(command);
  d[1] = command;
  rc = Lib$Spawn(d,0,0,&flags);
#else       
  sprintf(command,"%s %s &",startCommand,url);
  system(command); 
#endif
} 
static void MidasGrabFocus(w)
Widget w;
{
  XmProcessTraversal(w,XmTRAVERSE_CURRENT);  
}
static MidasOperand QueryUser(w)
Widget w;
{
    int flag = 0;
    Arg arglist[10];  

    if (!XmIsBulletinBoard(w))
      MidasError("QUERYUSER requires a widget of subclass BulletinBoard");

    XtAddCallback(w,XmNunmapCallback,(XtCallbackProc) MidasSetFlag,(XtPointer) &flag);
    XtSetArg(arglist[0],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL);
    XtSetValues(w,arglist,1);

    MidasPopup(w);

    for (;flag == 0;) 
      {
        MidasDispatchQueuedCommands();
        MidasFetchDispatchEvent(); 
      }

    return MidasGetIngot(w,"result");
}
char *MidasQueryUser(w)
Widget w;
{
  MidasOperand Temp;
  Temp = QueryUser(w);
  return (char *)MidasConvertToString(Temp);
}
/* The names and addresses of Things that Mrm.has to bind.  The names do
 * not have to be in alphabetical order.  */

static MrmRegisterArg reglist[] = { 
    {"midas_command_proc", (caddr_t) midas_command_proc},
    {"SendMidas", (caddr_t) MidasDispatchCommandCallback}};

static XtActionsRec actions[] = {
    {"SendMidas"   , (XtActionProc) MidasSendActionProc},
    {"MidasSpecial", (XtActionProc) MidasSpecialActionProc}};

static Boolean MidasConvertStringToCallback(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    int size = sizeof(XtPointer);
    XtPointer new;
    XtCallbackRec *Callback;

    if (*from->addr == '\0') Callback = 0;
    else 
      { 

    /* This stuff is currently never freed....need to implement destructor??? */

        Callback = (XtCallbackRec *) XtMalloc(sizeof(XtCallbackRec)*2);
        Callback[0].callback = (XtCallbackProc) MidasDispatchCommandCallback;
        Callback[0].closure  = (XtPointer) XtNewString((char *) from->addr);
        Callback[1].callback = NULL;
        Callback[1].closure  = NULL;
      }

    new = (XtPointer) &Callback;

    if (to->addr == 0)
      {
         to->addr = new;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         memcpy(to->addr,new,size);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}
static Boolean MidasConvertStringToList(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    int size = sizeof(List *);
    static List *new;
    char *in = XtNewString((char *) from->addr);
    char *token;

    new = MidasCreateEmptyList("");
    for (token = strtok(in," "); token ; token = strtok(NULL," "))    
      {
        MidasAddItemToList(new,token);
      }
    XtFree(in);
   
    /* WARNING: Memory leak....There is no destructor to delete this List */ 

    if (to->addr == 0)
      {
         to->addr = (XtPointer) &new;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         memcpy(to->addr,&new,size);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}
static Boolean MidasConvertStringToPixmap(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    int size = sizeof(Pixmap);
    static Pixmap new;
    Widget w = *((Widget *) args[0].addr);

    new = MidasFetchIcon((char *) from->addr,w);
   
    /* WARNING: Memory leak....There is no destructor to delete this Pixmap */ 

    if (to->addr == 0)
      {
         to->addr = (XtPointer) &new;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         memcpy(to->addr,&new,size);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}
static Boolean MidasConvertCallbackToString(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    XtCallbackRec *Callback = (XtCallbackRec *) from->addr;
    int size;
    XtPointer new;
    static char empty[] = "";

    if (Callback == 0)
      {
        new = empty;
      }
    else if (Callback[0].callback == (XtCallbackProc) MidasDispatchCommandCallback)
      {
        new = Callback[0].closure;
      }
    else 
      {
        new = empty;
      }
    size = strlen((char *) new) + 1;

    if (to->addr == 0)
      {
         to->addr = new;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         memcpy(to->addr,new,size);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}
static Boolean MidasConvertStringTable(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    XmString **string = (XmString **) from->addr;
    char *new;
    int size;

    if (*string == 0 || **string == 0)
      {
        new = XtNewString("");
        size = 1;
      }
    else
      {
        new = MidasStringToChar(**string);
        size = strlen(new) + 1;
      } 
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
         XtFree(new);
         return TRUE;
      }
    else
      {
         to->size = size;
         XtFree(new);
         return FALSE;
      }
}
static Boolean MidasConvertEnum(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    char *val = from->addr;
    char *new;
    int size;
    ConvertPair *cp = (ConvertPair *) args->addr;

    for (; cp->Name != 0; cp++)
      if (cp->Value == *val) { new = cp->Name; break; }

    if (cp->Name == 0) return FALSE;
    size =strlen(new);

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
static Boolean MidasConvertInteger(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    static char buffer[12];
    int size;

    if (from->size == sizeof(short int))
      {
        int i;
        short int s; 
        memcpy(&s,from->addr,sizeof(s));
        i = s;
        sprintf(buffer,"%d",i);
        size = 1 + strlen(buffer);
      }
    else
      {
        int s;
        memcpy(&s,from->addr,sizeof(s)); 
        sprintf(buffer,"%d",s);
        size = 1 + strlen(buffer);
      }


    if (to->addr == 0)
      {
         to->addr = buffer;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         strcpy(to->addr,buffer);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}
static Boolean MidasConvertQuark(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    char *buffer;
    int size;
    XrmQuark s;

    memcpy(&s,from->addr,sizeof(s)); 
    buffer = XrmQuarkToString(s);
    size = 1 + strlen(buffer);

    if (to->addr == 0)
      {
         to->addr = buffer;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         strcpy(to->addr,buffer);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
} 
static Boolean MidasConvertFloat(display,args,nargs,from,to,converter_data)
    Display    *display;
    XrmValue   *args;
    Cardinal   *nargs;
    XrmValue   *from;
    XrmValue   *to;
    XtPointer  *converter_data;
{
    static char buffer[12];
    int size;

    float s;
    memcpy(&s,from->addr,sizeof(s)); 
    sprintf(buffer,"%f",s);
    size = 1 + strlen(buffer);

    if (to->addr == 0)
      {
         to->addr = buffer;
         to->size = size;
         return TRUE;
      }
    else if  (to->size >= size)
      {
         strcpy(to->addr,buffer);
         to->size = size;
         return TRUE;
      }
    else
      {
         to->size = size;
         return FALSE;
      }
}
void MidasMainLoop()
{
    for (;;)
      {
        MidasDispatchQueuedCommands();
        MidasFetchDispatchEvent();
      }
}
void MidasInvokeAction(w,name)
Widget w;
char *name;
{
    XtCallActionProc(w,name,NULL,NULL,0);
}
void MidasSystem(name)
char *name;
{
    system(name);
}
void MidasInvokeCallback(w,name)
Widget w;
char *name;
{
    XtCallCallbacks(w,name,NULL);
}
void MidasFlush(w)
Widget w;
{
    XFlush(XtDisplay(w));
}
void MidasUpdate(w)
Widget w;
{
    XmUpdateDisplay(w);
}
Widget MidasInitializeFallback(argc,argv,fallback)
Cardinal *argc;
char *argv[];
char *fallback[];
{
    MrmHierarchy MidasHierarchy;
    MrmType *dummy_class;
    Pixmap PixMap;
    Pixel foreground,background;
    Widget MidasMenu;
    MidasOperand Temp;
    Widget toplevel_widget;
    int i;
    
    for (i=0; i < HASHTABLESIZE; i++) HashTable[i] = NULL;

    MidasWidgetList       = MidasCreateEmptyList("MidasWidgets");
    MidasFileList         = MidasCreateEmptyList("MidasFiles");
    MidasShellList        = MidasCreateEmptyList("MidasShells");
    MidasApplicationList  = MidasCreateEmptyList("MidasApplications");

    CommandQueue.Head = 0;
    CommandQueue.Tail = 0;

    MrmInitialize();                 /* Initialize MRM before initializing */
                                     /* the X Toolkit. */

    /* If we had user-defined widgets, we would register them with Mrm.here. */

    /* Set up initial command parsing stuff */

    XtSetTypeConverter("XmStringTable","String",MidasConvertStringTable,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("ShellVertPos","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("ShellHorizPos","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("ShellVertDim","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("ShellHorizDim","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("VerticalPosition","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("HorizontalPosition","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("VerticalDimension","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("HorizontalDimension","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Dimension","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Int","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Short","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Cardinal","String",MidasConvertInteger,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Float","String",MidasConvertFloat,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Quark","String",MidasConvertQuark,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("String","Callback",MidasConvertStringToCallback,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Callback","String",MidasConvertCallbackToString,
                       NULL,0,XtCacheNone,NULL);
    XtSetTypeConverter("Boolean","String",MidasConvertEnum,
                       ConvertBooleanArg,1,XtCacheNone,NULL);
    XtSetTypeConverter("Packing","String",MidasConvertEnum,
                       ConvertPackingArg,1,XtCacheNone,NULL);
    XtSetTypeConverter("String","Icon",MidasConvertStringToPixmap,
                       ConvertWidgetArg,1,XtCacheByDisplay,NULL);
    XtSetTypeConverter("String","List",MidasConvertStringToList,
                       NULL,0,XtCacheNone,NULL);

    MidasDeclareInit();
    MidasConvertInit();
    MidasDeclareQualifier("PARENT","Widget");
    MidasDeclareQualifier("UIDFILE","name");
    MidasDeclareQualifier("ICON","Icon");
    MidasDeclareQualifier("DIALOG","");
    MidasDeclareFunction("STRNSPN(name,name)"               ,MidasStrnspn);
    MidasDeclareFunction("GETWIDGETID(Widget)"              ,MidasGetWidgetId);
    MidasDeclareFunction("HASCHILDREN(Widget)"              ,MidasHasChildren);
    MidasDeclareFunction("CHILDRENINTREE(Widget)"           ,MidasChildrenInTree);
    MidasDeclareFunction("ISMANAGED(Widget)"                ,MidasIsManaged);
    MidasDeclareFunction("ISREALIZED(Widget)"               ,MidasIsRealized);
    MidasDeclareFunction("GETVALUE(Widget,name)"            ,MidasGetValue);
    MidasDeclareFunction("GETVALUERAW(Widget,name)"         ,MidasGetValueRaw);
    MidasDeclareFunction("GETAPPRESOURCE(Widget,name)"      ,MidasGetAppResource);
    MidasDeclareFunction("EVALU8(name)"                     ,MidasEvaluateExpression);
    MidasDeclareFunction("INT(Float)"                       ,MidasInt);
    MidasDeclareFunction("FETCH(name,{Widget})"             ,MidasFetchFunction);
    MidasDeclareFunction("TRACEWIDGETTREE(Widget,name)"     ,MidasTraceWidgetTreeFunction);
    MidasDeclareFunction("WIDGET(Widget)"                   ,MidasWidgetFunction);
    MidasDeclareFunction("WIDGETNAME(Widget)"               ,MidasWidgetName);
    MidasDeclareFunction("WIDGETCLASS(Widget)"              ,MidasWidgetClass);
    MidasDeclareFunction("PARENT(Widget)"                   ,MidasParent);
    MidasDeclareFunction("SHELL(Widget)"                    ,MidasShellFunction);
    MidasDeclareFunction("GETSELECTION(Widget,Atom,Atom)"   ,MidasGetSelection);
    MidasDeclareFunction("TYPE(any)"                        ,MidasGetOperandType);
    MidasDeclareFunction("GETPROPERTY(Widget,Atom)"         ,MidasGetProperty);
    MidasDeclareFunction("GETCOMMAND()"                     ,MidasGetCommand);
    MidasDeclareFunction("GETCONFIRMEXIT(Widget)"           ,MidasGetConfirmExit);
    MidasDeclareFunction("GETCLICKCOUNT()"                  ,MidasGetClickCount);
    MidasDeclareFunction("MODIFIER(name)"                   ,MidasModifier);
    MidasDeclareFunction("HYPERGETTEXT()"                   ,MidasHyperGetText);
    MidasDeclareFunction("HYPERGETHIDDEN()"                 ,MidasHyperGetHidden);
    MidasDeclareFunction("QUERYUSER(Widget)"                ,QueryUser);
    MidasDeclareFunction("HYPERGREP(Widget,name)"           ,MidasHyperGrep);
    MidasDeclareFunction("TREETOWIDGET(Widget)"             ,MidasTreeToWidget);
    MidasDeclareVerb("MIDAS *name..."                       ,MidasIgnore);
    MidasDeclareVerb("POPUP        Widget"                  ,MidasPopup);
    MidasDeclareVerb("POPDOWN      Widget"                  ,MidasPopdown);
    MidasDeclareVerb("TOGGLE       Widget"                  ,MidasToggle);
    MidasDeclareVerb("FETCH        name {Widget}"           ,MidasFetch);
    MidasDeclareVerb("DESTROY      Widget"                  ,MidasDestroy);
    MidasDeclareVerb("OPEN UID     name Widget"             ,MidasOpenUidFile);
    MidasDeclareVerb("CREATE SHELL name"                    ,MidasCreateShell);
    MidasDeclareVerb("CREATE WIDGET TREE Widget"            ,MidasCreateWidgetTree);
    MidasDeclareVerb("CREATE DIALOG name"                   ,MidasCreateDialog);
    MidasDeclareVerb("CREATE WIDGET Widget Class name"      ,MidasCreateWidget);
    MidasDeclareVerb("QUIT {Widget}"                        ,MidasQuitApplication);
    MidasDeclareVerb("QUIT APPLICATION {Widget}"            ,MidasQuitApplication);
    MidasDeclareVerb("QUIT SHELL {Widget}"                  ,MidasQuitShell);
    MidasDeclareVerb("QUIT MIDAS"                           ,MidasQuitMidas);
    MidasDeclareVerb("REGISTER NAME name Widget"            ,MidasRegisterName);
    MidasDeclareVerb("TOGGLE CHILDREN Widget"               ,MidasToggleChildrenTree);
    MidasDeclareVerb("GET CALLBACKS list Widget"            ,MidasGetCallbacks);
    MidasDeclareVerb("GET RESOURCES list Widget"            ,MidasGetResources);
    MidasDeclareVerb("SET VALUE Widget name any..."         ,MidasSetValue);    
    MidasDeclareVerb("SET SENSITIVE Widget Boolean"         ,MidasSetSensitive);    
    MidasDeclareVerb("SAVE GEOMETRY Widget"                 ,MidasSaveGeometry);
    MidasDeclareVerb("STORE RESOURCE Widget name"           ,MidasStoreResource);
    MidasDeclareVerb("STORE NAMED RESOURCE Widget name name",MidasStoreNamedResource);
    MidasDeclareVerb("UPDATE NAMED RESOURCE Widget name name"
                                                            ,MidasUpdateNamedResource);
    MidasDeclareVerb("STORE APP RESOURCE Widget name name"  ,MidasStoreAppResource);
    MidasDeclareVerb("SAVE STORED RESOURCES Widget"         ,MidasSaveStoredResources);
    MidasDeclareVerb("CENTER HORIZONTAL Widget Int"         ,MidasCenterHorizontal);
    MidasDeclareVerb("CLEAR WINDOW Widget",                  MidasClearWindow);
    MidasDeclareVerb("CURRENT WIDGET IN TREE Widget"        ,MidasCurrentWidgetInTree);
    MidasDeclareVerb("IF Boolean *name...",                  MidasIf);
    MidasDeclareVerb("ELSE IF Boolean *name...",             MidasElseIf);
    MidasDeclareVerb("ELSE *name...",                        MidasElse);
    MidasDeclareVerb("INVOKE ACTION Widget name",            MidasInvokeAction);
    MidasDeclareVerb("INVOKE CALLBACK Widget name",          MidasInvokeCallback);
    MidasDeclareVerb("LOAD IMAGE name",                      MidasLoadImage);
    MidasDeclareVerb("FLUSH Widget",                         MidasFlush);
    MidasDeclareVerb("UPDATE Widget",                        MidasUpdate);
    MidasDeclareVerb("FORCE DIALOG Widget",                  MidasForceDialog); 
    MidasDeclareVerb("SYSTEM name...",                       MidasSystem);
    MidasDeclareVerb("INVOKE APPLICATION name...",           MidasInvokeApplication);
    MidasDeclareVerb("SET CURSOR Widget Cursor",             MidasSetCursor);
    MidasDeclareVerb("ERROR name...",                        MidasError);
    MidasDeclareVerb("BEEP {Int} {Widget}",                  MidasBeep);
    MidasDeclareVerb("HYPER LOAD FILE Widget name",          SGMLHyperLoadFile);
    MidasDeclareVerb("LIST SELECT ITEM Widget XmString {Boolean}",XmListSelectItem);
    MidasDeclareVerb("HELP ON CONTEXT Widget",               MidasHelpOnContext);
    MidasDeclareVerb("HELP Widget name",                     MidasHelp);
    MidasDeclareVerb("GRAB FOCUS Widget",                    MidasGrabFocus); 			
    MidasDeclareVerb("FOR ALL name name...",                 MidasForAll); 
    MidasDeclareVerb("KICK Widget",			     MidasKick);
    MidasDeclareVerb("QUEUE Widget name..." ,		     MidasQueueCommand);
    MidasDeclareVerb("DISPATCH QUEUED COMMANDS",	     MidasDispatchQueuedCommands);
    MidasDeclareVerb("SET CONFIRM EXIT Widget Boolean",	     MidasSetConfirmExit);

    MidasUtilInit();
    MidasListInit();
    MidasInitializeIngots();

    /* Initialize the X Toolkit. We get back a top level shell widget. */

    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();

    /* declare fallback resources */

    XtAppSetFallbackResources(app_context,fallback); 
  
    /* declare SendMidas action routine */

    XtAppAddActions(app_context,actions,XtNumber(actions));

    /* Parse the special midas translations */

    MidasTranslations = XtParseTranslationTable("Ctrl <Btn1Down> : MidasSpecial(\"Set Ingot WidgetList currentWidget Widget(.):dialog;Popup MidasInfo:parent=WidgetList;Register Name WidgetName(.) .\")");
                                                                  
    MidasCreateApplication("Midas",argv,argc);
    toplevel_widget = MidasCreateShell("Midas");
    
    /* Open the UID files (the output of the UIL compiler) in the hierarchy*/

    MidasHierarchy = MidasOpenUidFile("midas.uid",toplevel_widget);
 
    /* Register the items MRM needs to bind for us. */

    MrmRegisterNames(reglist,XtNumber(reglist));

    /* Go get the midas pixmap */

    PixMap = MidasFetchIcon("MidasIcon",NULL);

    { int n = 0;
      Arg args[10];
      XtSetArg(args[n],XmNiconPixmap, PixMap); n++;
      XtSetValues(toplevel_widget, args, n);
    }

    /* Initialize Customization stuff */

    CustomInit();
    return toplevel_widget;
}
    /* Invoke the users profile */

Widget MidasInitializeUser(AppName,argv,argc)
Cardinal *argc;
char *argv[];
char *AppName;
{
    MidasCreateApplication(AppName,argv,argc);
    return MidasCreateShell(AppName);
};
void MidasStartInterpreter(top)
Widget top;
{ 
    MidasOperand Temp;

    Temp = MidasGetAppResource(top,"startup");

    if (*((char *)Temp.Value.P) != '\0') MidasQueueCommand(top,Temp.Value.P);
    else                                 MidasQueueCommand(top,"Popup Midas_Main");
}
XtAppContext MidasInitialize(argc,argv)
Cardinal argc;
char *argv[];
{
    Widget top = MidasInitializeFallback(&argc,argv,NULL);
    char *AppName, *AppClass;
    XtGetApplicationNameAndClass(XtDisplay(top),&AppName,&AppClass); 

    if (strcmp(AppName,"midas") && strcmp(AppName,"MIDAS") && strcmp(AppName,"Midas"))
       top = MidasInitializeUser(AppName,argv,&argc);

    MidasStartInterpreter(top);
    return XtWidgetToApplicationContext(top); 
}
