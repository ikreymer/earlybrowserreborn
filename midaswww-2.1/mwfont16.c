#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "SGMLTextP.h"

#define isch16(c) ((c)&0x80)

extern void ExtDrawString(w,img,dpy,wind,gc,x,y,copy,len)
Widget w;
Boolean img;
Display *dpy;
Drawable wind;
GC gc;
int x, y;
char *copy;
int len;
{

   SGMLTextObject tw = (SGMLTextObject) w;
   char *np, *p = copy;
   int i, l;
   Boolean mode, lastmode;
   XFontStruct *font = tw->sgml_text.font;
   GC sgc = tw->sgml_text.font16_gc;
   GC invert_sgc = tw->sgml_text.font16_invert_gc;
   short swidth = tw->sgml_text.font16->max_bounds.width;

   if (isch16(*p)) mode = lastmode = TRUE;
   else  mode = lastmode = FALSE;
   for (i=0, np=p;i < len;i++,p++)
     {
       if (isch16(*p)) mode = TRUE;
       else mode = FALSE;

       if (lastmode != mode && lastmode)
         {
           l = (int)( (p - np)/2);

           if (img)
             XDrawImageString16(dpy,wind,invert_sgc,x,y,(XChar2b *) np,l);
           else
             XDrawString16(dpy,wind,sgc,x,y,(XChar2b *) np,l);
           x += (l * swidth);
           np = p;
           lastmode = mode;
         }
       if (lastmode != mode && !lastmode)
         {
           l = p - np;

           if (img)
             XDrawImageString(dpy,wind,gc,x,y,np,l);
           else
             XDrawString(dpy,wind,gc,x,y,np,l);
           x += XTextWidth(font,np,l);
           np = p;
           lastmode = mode;
         }
       if (mode) { i++; p++; }
     }

   lastmode = mode;
   if (lastmode)
     {
       l = (int)( (p - np)/2);

       if (img)
         XDrawImageString16(dpy,wind,invert_sgc,x,y,(XChar2b *) np,l);
       else
         XDrawString16(dpy,wind,sgc,x,y,(XChar2b *) np,l);
       x += (l * swidth);
     }
   else
     {
       l = p - np;
       if (img)
         XDrawImageString(dpy,wind,gc,x,y,np,l);
       else
         XDrawString(dpy,wind,gc,x,y,np,l);
       x += XTextWidth(font,np,l);
     }
}


extern int ExtTextWidth(w,copy,len,byref)
Widget w;
char **copy;
int len;
Boolean byref;
{
   SGMLTextObject tw = (SGMLTextObject) w;
   XFontStruct *font = tw->sgml_text.font;
   char *np, *p = *copy;
   Boolean mode, lastmode;
   int i, nc, l = 0;
   short swidth = tw->sgml_text.font16->max_bounds.width;

   if (len == 1)
    {
      if (isch16(*p)) 
      {
        if (byref)
          *copy = p+1;
        return swidth;
      }
      else return XTextWidth(font,p,1);
    }

   if (isch16(*p)) mode = lastmode = TRUE;
   else  mode = lastmode = FALSE;
   for (i=0, np=p;i < len;i++,p++)
     {
       if (isch16(*p)) mode = TRUE;
       else mode = FALSE;

       if (lastmode != mode && lastmode)
         {
           nc = (int)( (p - np)/2);
           l += ( nc * swidth);
           np = p;
           lastmode = mode;
         }
       if (lastmode != mode && !lastmode)
         {
           nc = p - np;
           l += XTextWidth(font,np,nc);
           np = p;
           lastmode = mode;
         }
       if (mode) { i++; p++; }
     }
   lastmode = mode;
   if (lastmode)
     {
       nc = (int)( (p - np)/2);
       l += ( nc * swidth);
     }
   else
     {
       nc = p - np;
       l += XTextWidth(font,np,nc);
     }

   return l;
}
