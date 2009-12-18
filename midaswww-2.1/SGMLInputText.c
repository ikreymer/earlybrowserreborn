/*==================================================================*/
/*                                                                  */
/* SGMLInputTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a input text segment for the SGMLHyper widget            */
/*                                                                  */
/*==================================================================*/
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ScrolledW.h>
#include "SGMLInputTextP.h"
#include "SGMLFormText.h"
#include "SGMLCompositeTextP.h"   /* BAD */

/* 
  Private functions 
*/

#define Strlen(x) (x?strlen(x):0)

static Boolean option_selected, option = FALSE;
static char *option_value; 

/*
  Widget class methods
*/

static void    Destroy();
static void    Initialize();
static void    ClassInitialize();
static Boolean InheritChanges();
static Boolean SetValues();

static XrmQuark TypeText;
static XrmQuark TypeSubmit;
static XrmQuark TypeReset;
static XrmQuark TypeCheckbox;
static XrmQuark TypePassword;
static XrmQuark TypeRadio;
static XrmQuark TypeTextArea;
static XrmQuark TypeSelect;
static XrmQuark TypeHidden;
static XrmQuark TypeImage;
static XrmQuark TypeOption;
 
static XmString emptyLabel;

#define Offset(field) XtOffsetOf(SGMLInputTextRec,sgml_input_text.field)

static XtResource resources[] = {

    {SGMLNtype,SGMLCType,SGMLRQuark,sizeof(XrmQuark),
     Offset (type),SGMLRQuark,(XtPointer) &TypeText},
     
    {SGMLNname,SGMLCName,XtRString,sizeof(String),
     Offset (name),XtRImmediate,(XtPointer)0},

    {SGMLNvalue,SGMLCValue,XtRString,sizeof(String),
     Offset (value),XtRImmediate,(XtPointer)0},
 
    {SGMLNimage,SGMLCImage,"Image",sizeof(GIFImage *),
     Offset (image),XtRImmediate,(XtPointer)0},
 
    {SGMLNsize,SGMLCSize,XtRInt,sizeof(int),
     Offset (size),XtRImmediate,(XtPointer)0},

    {SGMLNrows,SGMLCRows,XtRInt,sizeof(int),
     Offset (rows),XtRImmediate,(XtPointer)0},

    {SGMLNcols,SGMLCCols,XtRInt,sizeof(int),
     Offset (cols),XtRImmediate,(XtPointer)0},
   
    {SGMLNmaxlength,SGMLCMaxlength,XtRInt,sizeof(int),
     Offset (maxlength),XtRImmediate,(XtPointer)0},

    {SGMLNchecked,SGMLCChecked,XtRBoolean,sizeof(Boolean),
     Offset (checked),XtRImmediate,(XtPointer)FALSE},
 
    {SGMLNmultiple,SGMLCMultiple,XtRBoolean,sizeof(Boolean),
     Offset (multiple),XtRImmediate,(XtPointer)FALSE},
   
    {SGMLNselected,SGMLCSelected,XtRBoolean,sizeof(Boolean),
     Offset (selected),XtRImmediate,(XtPointer)FALSE},

    {SGMLNautosubmit,SGMLCAutosubmit,XtRBoolean,sizeof(Boolean),
     Offset (auto_submit),XtRImmediate,(XtPointer)TRUE},  

    {SGMLNbombPixmap,SGMLCBombPixmap,"Icon",sizeof(Pixmap),
     Offset (bomb),XtRImmediate,(XtPointer)NULL},
 
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLInputTextClassRec  sGMLInputTextClassRec = {
    {
    (WidgetClass) &sGMLContainerTextClassRec,
                                         /* superclass            */
    "SGMLInputText",                     /* class_name            */
    sizeof(SGMLInputTextRec),            /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    Initialize,                          /* initialize            */
    NULL,                                /* initialize_hook       */
    NULL,                                /* obj1                  */
    NULL,                                /* obj2                  */
    0,                                   /* obj3                  */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    0,                                   /* obj4                  */
    0,                                   /* obj5                  */
    0,                                   /* obj6                  */
    0,                                   /* obj7                  */
    Destroy,	                         /* destroy               */
    NULL,                                /* obj8                  */
    NULL,                                /* obj9                  */
    SetValues,                           /* set_values            */
    NULL,                                /* set_values_hook       */
    NULL,                                /* obj10                 */
    NULL,                                /* get_values_hook       */
    NULL,                                /* obj11                 */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    NULL,                                /* obj12                 */
    NULL,                                /* obj13                 */
    NULL,                                /* obj14                 */
    NULL,                                /* extension             */
    },
    {
    SGMLInheritComputeSize,	    	 /* compute_size          */
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    SGMLInheritExpose,                   /* expose                */
    SGMLInheritActivate,                 /* activate              */
    SGMLInheritHilite,                   /* hilite                */
    SGMLInheritContains,                 /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    SGMLInheritCallMapCallback,          /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    NULL,                                /* sgml_set_values       */
    InheritChanges,                      /* inherit_changes       */
    SGMLInheritComputeChangeMask,        /* compute_change_mask   */
    SGMLInheritSearch,                   /* search                */
    SGMLInheritClearSelect,              /* clear_select          */
    SGMLInheritHiliteSelection,          /* hilite_selection      */
    SGMLInheritXYToPos,                  /* xy_to_pos             */
    SGMLInheritPosToXY,                  /* pos_to_xy             */
    SGMLInheritDumpText,                 /* dump_text             */
    NULL,                                /* extension             */
    },
    {
    SGMLInheritGeometryManager,          /* geometry_manager      */ 
    NULL,                                /* extension             */
    },
    { 
    NULL,                                /* extension             */
    },
};


WidgetClass sGMLInputTextObjectClass = (WidgetClass) &sGMLInputTextClassRec;

/*--------------------------------------------------------------*/
/* ClassInitialize:                                             */
/*--------------------------------------------------------------*/
 
static void ClassInitialize ()
{
  TypeText      = XrmPermStringToQuark("text");
  TypeSubmit    = XrmPermStringToQuark("submit"); 
  TypeReset     = XrmPermStringToQuark("reset");
  TypeCheckbox  = XrmPermStringToQuark("checkbox");
  TypePassword  = XrmPermStringToQuark("password");
  TypeRadio     = XrmPermStringToQuark("radio");
  TypeSelect    = XrmPermStringToQuark("select");
  TypeTextArea  = XrmPermStringToQuark("textarea");
  TypeHidden    = XrmPermStringToQuark("hidden");
  TypeImage     = XrmPermStringToQuark("image");
  TypeOption    = XrmPermStringToQuark("option");
  
  emptyLabel    = XmStringCreateSimple("");
}

/*--------------------------------------------------------------*/
/* check_passwd, echos *s in text widget, taken from Motif FAQ  */
/*--------------------------------------------------------------*/
/* Written by Dan Heller.  Copyright 1991, O'Reilly && Associates.
 * This program is freely distributable without licensing fees and
 * is provided without guarantee or warranty expressed or implied.
 * This program is -not- in the public domain.  This program appears
 * in the Motif Programming Manual, O'Reilly Volume 6.
 */

/* passwd.c -- prompt for a passwd.  Meaning, all input looks like
 * a series of *'s.  Store the actual data typed by the user in
 * an internal variable.  Don't allow paste operations.  Handle
 * backspacing by deleting all text from insertion point to the
 * end of text.
 */
 
void
check_passwd(text_w, buffer, cbs)
Widget        text_w;
char        **buffer;
XmTextVerifyCallbackStruct *cbs;
{
    char *new;
    int len;
    char *passwd = *buffer;  

    if (cbs->text->ptr == NULL) { /* backspace */
        cbs->endPos = Strlen(passwd); /* delete from here to end */
        if (cbs->endPos <= 0) return; /* catch null passwd - Mark Scoville */
        passwd[cbs->startPos] = 0; /* backspace--terminate */
        return;
    }

    if (cbs->text->length > 1) {
        cbs->doit = False; /* don't allow "paste" operations */
        return; /* make the user *type* the password! */
    }

    new = XtMalloc(cbs->endPos + 2); /* new char + NULL terminator */
    if (passwd) {
        strcpy(new, passwd);
        XtFree(passwd);
    } else
        new[0] = '\0';
    passwd = new;
    strncat(passwd, cbs->text->ptr, cbs->text->length);
    passwd[cbs->endPos + cbs->text->length] = 0;

    for (len = 0; len < cbs->text->length; len++)
        cbs->text->ptr[len] = '*';
    
    *buffer = new;
}

/*--------------------------------------------------------------*/
/* SetupImage:                                                  */
/*--------------------------------------------------------------*/
 
static void SetupImage(w)
SGMLInputTextObject w;
{
  Widget parent = (Widget) w;
  GIFImage *gifImage = w->sgml_input_text.image;
  char message[256];
  Pixmap pixmap;

  for ( ; !XtIsWidget(parent) ; parent = XtParent(parent));

  pixmap = GIFToPixmap(parent,gifImage,message);
  if (!pixmap) printf("Image conversion error: %s\n",message);  
  else 
   {
      Arg arglist[10];
      int n=0;

      XtSetArg(arglist[n],XmNlabelPixmap,pixmap); n++; 
      XtSetValues(w->sgml_input_text.child,arglist,n);
    } 
}
/*--------------------------------------------------------------*/
/* ResetAction:                                                 */
/*--------------------------------------------------------------*/
 static void ResetAction(w)
SGMLInputTextObject w;
{
  XrmQuark type  = w->sgml_input_text.type;
  Widget child = w->sgml_input_text.child;
 
  if (type == TypePassword || type == TypeText)
    {
      char *value = w->sgml_input_text.value;
      XmTextFieldSetString(child,value?value:"");
    }
  else if (type == TypeTextArea)
    {
      char *text = w->sgml_text.text;
      XmTextSetString(child,text?text:"");
    }
  else if (type == TypeSelect)
    {
      Boolean selected = w->sgml_input_text.selected;
      int n=0;
      Arg arglist[10];

      Widget list;
      Widget p = XtParent((Widget) w); 

      XtSetArg(arglist[n],SGMLNuserdata,&list); n++;
      XtGetValues(p,arglist,n); n = 0;

      if (XtIsSubclass(list,xmListWidgetClass))
        {
          XmString Text;
          char *q, *p, *text = XtNewString(w->sgml_text.text);

          /* Strip off leading and trailing white-space */

          for (p = text ; *p && isspace(*p) ; p++);
          for (q = p + Strlen(p) ; q-- > p && isspace(*q) ; ) *q = '\0';

          if (child) XmListDeselectAllItems(list); 

          Text = XmStringCreateSimple(p);
          if (selected)
            {
              int nsel, i;
              XmString *List, *NewList;
  
              n = 0;
              XtSetArg(arglist[n],XmNselectedItems,&List); n++;
              XtSetArg(arglist[n],XmNselectedItemCount,&nsel); n++;
              XtGetValues(list,arglist,n); n = 0;
              NewList = (XmString *) XtMalloc((nsel+1) * sizeof(XmString *));
              for (i = 0; i < nsel; i++) NewList[i] = List[i];
              NewList[nsel++] = Text; 

              XtSetArg(arglist[n],XmNselectedItems,NewList); n++;
              XtSetArg(arglist[n],XmNselectedItemCount,nsel); n++;
              XtSetValues(list,arglist,n);
              XtFree((char *) NewList);
            }

          XmStringFree(Text);
          XtFree(text); 
        }
      else if (selected) 
        {
          n = 0;
          XtSetArg(arglist[n],XmNmenuHistory,child); n++;
          XtSetValues(list,arglist,n);
        }
    }
  else if (type == TypeCheckbox || type == TypeRadio)
    {
      Boolean set  = w->sgml_input_text.checked;
      XmToggleButtonSetState(child,set,FALSE); 
    }     
}
static void Reset(reset,new)
Widget reset;
Widget new;
{
   if (SGMLIsCompositeText(new))
     {
        SGMLCompositeTextObject w = (SGMLCompositeTextObject) new;
        Widget *children = w->sgml_composite_text.children;
        int n = w->sgml_composite_text.num_children;

        for (; n--; ) Reset(reset,*children++); 
     }  
   else if (SGMLIsInputText(new)) ResetAction(new);
}
/*
 * This routine is used to encode a search string
 * ASSUMES ascii character set
 */
static char* Encode(search)
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

  if (!search) return;
  else
    {
      char *result = XtMalloc(3*Strlen(search) + 1);
      char *p, *s, *e, *q;

      for (s = search ; *s && isspace(*s); s++);
      for (e = s + Strlen(s) ; e>s && isspace(*(e-1)) ; e--);  
      for (q = result , p = s ; p<e ; p++ )
        {
          int c = (int) *p; 
          if      (isspace(c)) *q++ = '+';
          else if (c>= 32  && c<= 127 && isAcceptable[c-32]) *q++ = (char) c;
          else { sprintf(q,"%%%2x",c); q += 3; }
        }   
      *q = '\0';
      return result;
    }
}
static void PutResult(buffer,name,value)
char **buffer, *name, *value;
{
  char *old = *buffer;
  char *ename = Encode(name);
  char *evalue = Encode(value);
  char *new = (char *) XtMalloc(Strlen(old) + Strlen(ename) + Strlen(evalue) + 3);

  sprintf(new,"%s%s=%s&",old?old:"",ename,evalue); 

  XtFree(old);
  XtFree(ename);
  XtFree(evalue);
  *buffer = new; 
}
static void ActivateAction(w,ActivateRoutine,Closure)
SGMLInputTextObject w;
void (*ActivateRoutine)();
void *Closure;
{
  XrmQuark type  = w->sgml_input_text.type;
  Widget child = w->sgml_input_text.child;
  char *name = w->sgml_input_text.name; 

  if (type == TypeText)
    {
      char *value = XmTextFieldGetString(child);
      ActivateRoutine(Closure,name,value);
      XtFree(value);
    }      
  else if (type == TypePassword)
    {
      char *value = w->sgml_input_text.passwd;
      ActivateRoutine(Closure,name,value);
    }
  else if (type == TypeHidden)
    {
      char *value =  w->sgml_input_text.value;
      ActivateRoutine(Closure,name,value);
    }
  else if (type == TypeCheckbox || type == TypeRadio)
    {
      Boolean set = XmToggleButtonGetState(child);
      if (set)
        {
          char *value =  w->sgml_input_text.value;
          ActivateRoutine(Closure,name,value?value:"on");
        }
    }
  else if (type == TypeTextArea)
    {
      char *value = XmTextGetString(child);
      ActivateRoutine(Closure,name,value);
    }
  else if (type == TypeSelect)
    {
      Boolean selected = w->sgml_input_text.selected;
      int n=0;
      Arg arglist[10];

      Widget list;
      Widget p = XtParent((Widget) w); 

      XtSetArg(arglist[n],SGMLNuserdata,&list); n++;
      XtGetValues(p,arglist,n); n = 0;

      if (XtIsSubclass(list,xmListWidgetClass))
         {
           static int ns;
           static int *positions = NULL;
           static int position;
           char *name = XtName(list);
           char *text =  w->sgml_text.text; 
 
           if (child) /* Only the first item in the list has a child */
             {
               XtFree((char *) positions);
               position = 0;
               if (!XmListGetSelectedPos(list,&positions,&ns)) { positions = NULL; ns = 0; } 
             }
           if (text && *text && option)
             {   
               position++;
               for (n = 0; n < ns ; n++) 
                 if (position == positions[n])
                   { 
                     char *value = w->sgml_input_text.value;
                     ActivateRoutine(Closure,name,value);
                   } 
             }
        }                  
      else
        {
          Widget menuHistory;
          XtSetArg(arglist[n],XmNmenuHistory,&menuHistory); n++;
          XtGetValues(list,arglist,n); n = 0;
          if (menuHistory == child)
            {
              char *value = w->sgml_input_text.value;
              char *name = XtName(list);
              ActivateRoutine(Closure,name,value);
            }
        }
    }
  option = (type == TypeOption); 
} 
static void ActivateChain(activate,new,ActivateRoutine,Closure)
Widget activate;
Widget new;
void (*ActivateRoutine)();
void * Closure;
{
 
   if (SGMLIsCompositeText(new))
     {
        SGMLCompositeTextObject w = (SGMLCompositeTextObject) new;
        Widget *children = w->sgml_composite_text.children;
        int n = w->sgml_composite_text.num_children;

        for (; n--; ) ActivateChain(activate,*children++,ActivateRoutine,Closure); 
     }  
   else if (SGMLIsInputText(new)) ActivateAction(new,ActivateRoutine,Closure);
}
static void Activate(activate,form)
Widget activate;
Widget form;
{
  char *Buffer = NULL;
  ActivateChain(activate,form,PutResult,&Buffer); 
  if (*Buffer) *(Buffer + Strlen(Buffer) - 1) = '\0'; 
  SGMLFormSetResult(form,Buffer);
}  
static void ActivateImage(activate,form,cb)
Widget activate;
Widget form;
XmPushButtonCallbackStruct *cb;
{
  char *Buffer = NULL;
  ActivateChain(activate,form,PutResult,&Buffer); 
 
  if (cb->event->type == ButtonPress || cb->event->type == ButtonRelease)
    {
      XButtonEvent *be = (XButtonEvent *) cb->event;
      char *name = (char *) XtMalloc(Strlen(XtName(activate)) + 10);
      char value[20];
      Dimension hT, mH, mW, sT ,mL, mT; 
      Arg arglist[10];
      int n = 0;
      Position x, y;

      XtSetArg(arglist[n],XmNhighlightThickness, &hT); n++;
      XtSetArg(arglist[n],XmNmarginHeight,       &mH); n++;
      XtSetArg(arglist[n],XmNmarginWidth,        &mW); n++;
      XtSetArg(arglist[n],XmNshadowThickness,    &sT); n++;
      XtSetArg(arglist[n],XmNmarginLeft,         &mL); n++;
      XtSetArg(arglist[n],XmNmarginTop,          &mT); n++;
      XtGetValues(activate,arglist,n);
  
      x = be->x - hT - mW - sT - mL;
      y = be->y - hT - mH - sT - mT; 

      sprintf(name,"%s.x",XtName(activate));
      sprintf(value,"%d",x<0?0:x); 
      PutResult(&Buffer,name,value);
 
      sprintf(name,"%s.y",XtName(activate));
      sprintf(value,"%d",y<0?0:y); 
      PutResult(&Buffer,name,value);

      XtFree(name);
    } 

  if (*Buffer) *(Buffer + Strlen(Buffer) - 1) = '\0'; 
  SGMLFormSetResult(form,Buffer);
}
static void RadioAction(radio,w)
Widget radio;
SGMLInputTextObject w;
{
  Widget c2 = w->sgml_container_text.child;   
  
  if (w->sgml_input_text.type != TypeRadio) return;
  if (radio->core.xrm_name != c2->core.xrm_name) return;
  XmToggleButtonSetState(c2,(radio == c2),FALSE);
}
static void Radio(radio,new)
Widget radio;
Widget new;
{
   if (SGMLIsCompositeText(new))
     {
        SGMLCompositeTextObject w = (SGMLCompositeTextObject) new;
        Widget *children = w->sgml_composite_text.children;
        int n = w->sgml_composite_text.num_children;

        for (; n--; ) Radio(radio,*children++); 
     }  
   else if (SGMLIsInputText(new)) RadioAction(radio,new);
}
/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLInputTextObject request, new;
{
  Widget parent, child, ichild; 
  XrmQuark type   = new->sgml_input_text.type;
  char    *name   = new->sgml_input_text.name;
  char    *value  = new->sgml_input_text.value;
  Dimension sT, hT, mH, mB;
  XmFontList font = XmFontListCreate(new->sgml_text.font,XmSTRING_DEFAULT_CHARSET);
  int n=0;
  Arg arglist[20];
 
  if (name)   new->sgml_input_text.name   = XtNewString(name);
  if (value)  new->sgml_input_text.value  = XtNewString(value);
 
  new->sgml_input_text.passwd = NULL;

  for (parent = XtParent((Widget) new); 
       !XtIsWidget(parent) ; 
       parent = XtParent(parent));

  XtSetArg(arglist[n],XmNuserData,new); n++;
  XtSetArg(arglist[n],XmNfontList,font); n++;  
 
  if (type == TypeSubmit)
    {   
      Widget form;
      XmString Value;
      if (!value) value = "Submit Query";

      Value = XmStringCreateSimple(value);

      XtSetArg(arglist[n],XmNlabelString,Value); n++;

      ichild = child = XtCreateWidget(name,xmPushButtonWidgetClass,parent,arglist,n);
 
      n = 0;
      XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
      XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
      XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
      XtSetArg(arglist[n],XmNmarginBottom,      &mB); n++;
      XtGetValues(child,arglist,n);

      new->sgml_container_text.vertical_offset = -sT - hT - mH - mB - new->sgml_text.descent;  
 
      XmStringFree(Value);
 
      for (form = (Widget) new ; form ; form = XtParent(form))
        if (SGMLIsFormText(form)) 
          {    
            XtAddCallback(child,XmNactivateCallback,(XtCallbackProc) Activate,(XtPointer) form);
            break;
          }
    }  
  else if (type == TypeReset)
    {   
      Widget form;
      XmString Value;

      if (!value) value = "Reset";

      Value = XmStringCreateSimple(value);

      XtSetArg(arglist[n],XmNlabelString,Value); n++;

      ichild = child = XtCreateWidget(name,xmPushButtonWidgetClass,parent,arglist,n);
 
      n = 0;
      XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
      XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
      XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
      XtSetArg(arglist[n],XmNmarginBottom,      &mB); n++;
      XtGetValues(child,arglist,n);

      new->sgml_container_text.vertical_offset = -sT - hT - mH - mB - new->sgml_text.descent;  
 
      XmStringFree(Value);

      for (form = (Widget) new ; form ; form = XtParent(form))
        if (SGMLIsFormText(form)) 
          {    
            XtAddCallback(child,XmNactivateCallback,(XtCallbackProc) Reset,(XtPointer) form);
            break;
          }
    }
  else if (type == TypePassword || type == TypeText)
    {
      Widget form;
      int size  = new->sgml_input_text.size;
      int max   = new->sgml_input_text.maxlength;
      if (size == 0) size = 20;
  
      XtSetArg(arglist[n],XmNuserData,new); n++; 
      XtSetArg(arglist[n],XmNcolumns,size); n++;
      if (max)   { XtSetArg(arglist[n],XmNmaxLength,max); n++; }
      if (value) { XtSetArg(arglist[n],XmNvalue,value); n++;   }

      ichild = child = XtCreateWidget(name,xmTextFieldWidgetClass,parent,arglist,n);

      n = 0;
      XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
      XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
      XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
      XtGetValues(child,arglist,n);

      new->sgml_container_text.vertical_offset = -sT - hT - mH - new->sgml_text.descent;  
 
      if (type == TypePassword)
        { 
           new->sgml_input_text.passwd = XtNewString(value);
           XtAddCallback(child, XmNmodifyVerifyCallback,(XtCallbackProc) check_passwd, (XtPointer) &new->sgml_input_text.passwd);
        } 
      if (new->sgml_input_text.auto_submit)
        {
          for (form = (Widget) new ; form ; form = XtParent(form))
            if (SGMLIsFormText(form)) 
              {    
                XtAddCallback(child,XmNactivateCallback,(XtCallbackProc) Activate,(XtPointer) form);
                break;
              }
        }
    }         
  else if (type == TypeTextArea)
    {
      int rows  = new->sgml_input_text.rows;
      int cols  = new->sgml_input_text.cols;
      char *text = new->sgml_text.text; 

      if (rows == 0) rows = 20;
      if (cols == 0) cols = 20;

      XtSetArg(arglist[n],XmNeditMode,XmMULTI_LINE_EDIT); n++;
      XtSetArg(arglist[n],XmNcolumns,cols); n++;
      XtSetArg(arglist[n],XmNrows,rows); n++;
      XtSetArg(arglist[n],XmNvalue,text); n++;
      ichild = XmCreateScrolledText(parent,name,arglist,n);

      child = XtParent(ichild);  
      XtUnmanageChild(child);
      XtManageChild(ichild);
    }         
  else if (type == TypeImage)
    {
      Widget form;
      XtSetArg(arglist[n],XmNlabelType,XmPIXMAP); n++;
      XtSetArg(arglist[n],XmNmarginWidth,0); n++;
      XtSetArg(arglist[n],XmNmarginHeight,0); n++;
      XtSetArg(arglist[n],XmNlabelPixmap,new->sgml_input_text.bomb); n++;
      ichild = child = XtCreateWidget(name,xmPushButtonWidgetClass,parent,arglist,n); 
 
      n = 0;
      XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
      XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
      XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
      XtSetArg(arglist[n],XmNmarginBottom,      &mB); n++;
      XtGetValues(child,arglist,n);

      new->sgml_container_text.vertical_offset = -sT - hT - mH - mB - new->sgml_text.descent;  
 
      for (form = (Widget) new ; form ; form = XtParent(form))
        if (SGMLIsFormText(form)) 
          {    
            XtAddCallback(child,XmNactivateCallback,(XtCallbackProc) ActivateImage,(XtPointer) form);
            break;
          }
    } 
  else if (type == TypeOption)
    {
      option_selected = new->sgml_input_text.selected;
      option_value = new->sgml_input_text.value;
      child = ichild = NULL;
    }
  else if (type == TypeSelect)
    {
      char *text = XtNewString(new->sgml_text.text);
      Boolean multiple = new->sgml_input_text.multiple;
 
      int nn=0;
      Arg argl[10];

      Widget w;
      Widget p = XtParent((Widget) new); 

      XtSetArg(argl[nn],SGMLNuserdata,&w); nn++;
      XtGetValues(p,argl,nn); 
      if (!w)
        {
          int size = new->sgml_input_text.size;
          if (size > 1 || multiple)
            { 
              if (size) { XtSetArg(arglist[n],XmNvisibleItemCount,size); n++; }
              XtSetArg(arglist[n],XmNselectionPolicy,multiple?XmEXTENDED_SELECT:XmBROWSE_SELECT); n++;
              w = ichild = XmCreateScrolledList(parent,name,arglist,n);

              n = 0;
              XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
              XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
              XtSetArg(arglist[n],XmNlistMarginHeight,  &mH); n++;
              XtGetValues(ichild,arglist,n);

              new->sgml_container_text.vertical_offset = -sT - hT - 2*mH - new->sgml_text.descent;  
 
              n = 0;
              XtSetArg(arglist[n],SGMLNuserdata,w); n++;
              XtSetValues(p,arglist,n); 
 
              child = XtParent(ichild);  
              XtUnmanageChild(child);
              XtManageChild(ichild);           
            }
          else
            {
              Widget menu;
              menu = XmCreatePulldownMenu(parent,name,arglist,n); 
 
              XtSetArg(arglist[n],XmNlabelString,emptyLabel); n++;       
              XtSetArg(arglist[n],XmNsubMenuId,menu); n++;
              w = ichild = child = XmCreateOptionMenu(parent,name,arglist,n); 
 
              n = 0;
              XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
              XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
              XtGetValues(child,arglist,n);

              new->sgml_container_text.vertical_offset = -sT - 2*mH - new->sgml_text.descent;  
 
              n = 0;
              XtSetArg(arglist[n],SGMLNuserdata,w); n++;
              XtSetValues(p,arglist,n);
             }
        }
      else ichild = child = 0;
 
      if (text && *text)
        {
          char *q, *p, *r = XtNewString(text);

          /* Strip off leading and trailing white-space */

          for (p = r; *p && isspace(*p); p++);
          for (q = p + Strlen(p); q-- > p && isspace(*q); ) *q = '\0';
          
          if (option || *p) 
            { 
              XmString Text;
              if (!*p) p   = "";
              Text = XmStringCreateSimple(p);
      
              if (XtIsSubclass(w,xmListWidgetClass))
                {
                  XmListAddItem(w,Text,0);
                  if (option_selected) 
                    { 
                      int nsel, i;
                      XmString *List, *NewList;
                      n = 0;
                      XtSetArg(arglist[n],XmNselectedItems,&List); n++;
                      XtSetArg(arglist[n],XmNselectedItemCount,&nsel); n++;
                      XtGetValues(w,arglist,n); n = 0;
                      NewList = (XmString *) XtMalloc((nsel+1) * sizeof(XmString *));
                      for (i = 0; i < nsel; i++) NewList[i] = List[i];
                      NewList[nsel++] = Text; 

                      XtSetArg(arglist[n],XmNselectedItems,NewList); n++;
                      XtSetArg(arglist[n],XmNselectedItemCount,nsel); n++;
                      XtSetValues(w,arglist,n);
                      XtFree((char *) NewList);
                    }
                }
              else
                {
                  Widget menu;
                  n = 0;
                  XtSetArg(arglist[n],XmNsubMenuId,&menu); n++;
                  XtGetValues(w,arglist,n); n = 0;

                  XtSetArg(arglist[n],XmNlabelString,Text); n++;
                  XtSetArg(arglist[n],XmNfontList,font); n++;  
                  ichild = XtCreateManagedWidget(p,xmPushButtonWidgetClass,menu,arglist,n);
                  if (option_selected)
                    {
                      n = 0;
                      XtSetArg(arglist[n],XmNmenuHistory,ichild); n++;
                      XtSetValues(w,arglist,n);
                    } 
                } 
              XmStringFree(Text);
            }
          if (option_value && *option_value)
            {
              XtFree(r);
              new->sgml_input_text.value = XtNewString(option_value);
            }
          else new->sgml_input_text.value = p;
        }        
      new->sgml_input_text.selected = option_selected;
      XtFree(text);
    }
  else if (type == TypeCheckbox)
    {
      Boolean set  = new->sgml_input_text.checked;
      XtSetArg(arglist[n],XmNspacing,0); n++;
      XtSetArg(arglist[n],XmNmarginBottom,0); n++;      
      XtSetArg(arglist[n],XmNset,set); n++;
      XtSetArg(arglist[n],XmNlabelString,emptyLabel); n++; 

      ichild = child = XtCreateWidget(name,xmToggleButtonWidgetClass,parent,arglist,n);
 
      n = 0;
      XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
      XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
      XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
      XtGetValues(child,arglist,n);

      new->sgml_container_text.vertical_offset = -sT - hT - mH - new->sgml_text.descent;  
    }     
  else if (type == TypeRadio)
    {
      Widget form;
      Boolean set  = new->sgml_input_text.checked;

      XtSetArg(arglist[n],XmNset,set); n++;
      XtSetArg(arglist[n],XmNindicatorType,XmONE_OF_MANY); n++;
      XtSetArg(arglist[n],XmNlabelString,emptyLabel); n++;       
      XtSetArg(arglist[n],XmNspacing,0); n++;
      XtSetArg(arglist[n],XmNmarginWidth,0); n++;
      XtSetArg(arglist[n],XmNmarginHeight,0); n++;
      XtSetArg(arglist[n],XmNmarginLeft,0); n++;
      XtSetArg(arglist[n],XmNmarginRight,0); n++;
      XtSetArg(arglist[n],XmNmarginTop,0); n++;
      XtSetArg(arglist[n],XmNmarginBottom,0); n++;

      ichild = child = XtCreateWidget(name,xmToggleButtonWidgetClass,parent,arglist,n);
 
      n = 0;
      XtSetArg(arglist[n],XmNshadowThickness,   &sT); n++; 
      XtSetArg(arglist[n],XmNhighlightThickness,&hT); n++; 
      XtSetArg(arglist[n],XmNmarginHeight,      &mH); n++;
      XtGetValues(child,arglist,n);

      new->sgml_container_text.vertical_offset = -sT - hT - mH - new->sgml_text.descent;  

      for (form = (Widget) new ; form ; form = XtParent(form))
        if (SGMLIsFormText(form)) 
          {    
            XtAddCallback(child,XmNvalueChangedCallback,(XtCallbackProc) Radio, (XtPointer) form);
            break;
          }
    }
  else ichild = child = NULL;

  XmFontListFree(font);   

  new->sgml_container_text.child = child;
  new->sgml_input_text.child = ichild;
  if (new->sgml_input_text.image)  SetupImage(new); 
  option = (type == TypeOption); 
} 
/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLInputTextObject w;
{
    XtFree(w->sgml_input_text.name);
    XtFree(w->sgml_input_text.value);
    XtFree(w->sgml_input_text.passwd);
    if (w->sgml_input_text.image) GIFFreeFile(w->sgml_input_text.image);
}    
 
/*------------------------------------------------------------------*/
/* SetValues :                                                      */
/*------------------------------------------------------------------*/

static Boolean SetValues (current, request, new)
SGMLInputTextObject current, request, new;
{
 
#define HAS_CHANGED(a)    (new->sgml_input_text.a != current->sgml_input_text.a)
#define REJECT(a) if HAS_CHANGED(a) new->sgml_input_text.a != current->sgml_input_text.a

    REJECT(name); 
    REJECT(value); 
    REJECT(passwd); 
    if (HAS_CHANGED(image)) 
      { 
        SetupImage(new);
        if (current->sgml_input_text.image) GIFFreeFile(current->sgml_input_text.image); 
      }
#undef HAS_CHANGED
#undef REJECT
  return FALSE; 
} 
/*--------------------------------------------------------------*/
/* Inherit Changes                                              */
/*--------------------------------------------------------------*/

static Boolean InheritChanges(w,inputMask)
SGMLInputTextObject w;
int inputMask;
{
    if (inputMask & gcMask) 
      {
        int n=0;
        Arg arglist[10];
        XmFontList font = XmFontListCreate(w->sgml_text.font,XmSTRING_DEFAULT_CHARSET);

        XtSetArg(arglist[n],XmNfontList,font); n++;  
        if (w->sgml_input_text.child) XtSetValues(w->sgml_input_text.child,arglist,n);
        XmFontListFree(font);        
      }
  return FALSE; 
}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLInputTextObject                                      */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateInputText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLInputTextObjectClass,parent,al,ac);
}
