/*** mightily hacked up library version of xloadimage.c 
 *** PYW
 ***/

/* xloadimage.c:
 *
 * generic image loader for X11
 *
 * jim frost 09.27.89
 *
 * Copyright 1989, 1990, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xloadimage.h"
#include "patchlevel"
#include <signal.h>

extern Display *Disp;
extern int Scrn;
int           dispcells;
Colormap      theCmap;

/*PYW begin*/
typedef struct ImgNode {
  char *id;
  XImageInfo *ximageinfo;
  int width, height;
  int refc;
  struct ImgNode *next;
} ImgNode;

typedef struct WinObj {
  Window w;
  ImgNode *node;
} WinObj;

ImgNode *imgNodes = NULL;

WinObj winObj[10];
int winObjIdx = 0;

GC gc;

char *saveString(str)
{
  char *sp;

  if (!str) return NULL;/*bad*/

  if (sp = (char*)malloc(sizeof(char) * (strlen(str) + 1))) {
    strcpy(sp, str);
    return sp;
  }
  fprintf(stderr, "malloc failed.");
  return NULL;
}

void purgeImg()
{
  ImgNode *ip = imgNodes;

  for (ip = imgNodes; ip; ip = ip->next) {
    /* free ximageinfo */
    ip->ximageinfo = NULL;
  }
}

ImgNode *imgNodeRefInc(id)
     char *id;
{
  ImgNode *ip = imgNodes;
  int stat;
  if (ip) {
    for (; ip; ip = ip->next) {
      if (!strcmp(id, ip->id)) {
	if (ip->ximageinfo == NULL) {
	  stat = loadImg(ip->id, &(ip->ximageinfo), 
			 &(ip->width), &(ip->height));
	  if (stat ==0) return NULL;
	}
	ip->refc++;
	return ip;
      }
    }
  }
  /* make new node */
  ip = (ImgNode*)malloc(sizeof(struct ImgNode));
  ip->id = saveString(id);
  ip->ximageinfo = NULL;
  ip->refc = 1;
  if (imgNodes) {
    ip->next = imgNodes;
  } else {
    ip->next = NULL;
  }
  stat = loadImg(ip->id, &(ip->ximageinfo), &(ip->width), &(ip->height));
  if (stat ==0) return NULL;
  imgNodes = ip;
  return ip;
}

int imgNodeRefDec(id)
     char *id;
{
  ImgNode *last_ip = NULL, *ip, *next_ip;
  
  for (ip = imgNodes; ip; ip = ip->next) {
    if (!strcmp(id, ip->id)) {
      if (--(ip->refc) <= 0) {
	next_ip = ip->next;
	/* destroy ip*/
	/* free ip->ximageinfo*/
	free(ip);
	if (last_ip) last_ip->next = next_ip;
	else imgNodes = NULL;
	return 0;
      }
    }
    last_ip = ip;
  }
  return 0;
}

int makeImg(filename)
     char *filename;
{
  int i, stat;
  XSetWindowAttributes attrs;
  ImgNode *ip;

  attrs.background_pixmap = NULL;/*purify hush*/

  ip = imgNodeRefInc(filename);
  
  printf("IP=%d\n", ip);
  if (ip == 0) return 0;
  
  printf("MADE imgObj (%d,%d)\n", ip->width, ip->height);
  
  attrs.event_mask = StructureNotifyMask | KeyPressMask |
    ButtonPressMask | ButtonReleaseMask | 
      ExposureMask;

  winObj[winObjIdx].node = ip;
  winObj[winObjIdx].w = XCreateWindow(Disp, RootWindow(Disp, Scrn),
				      (int)10 + (i * 50), (int)10,
				      (int)ip->width, (int)ip->height,
				      (int)1, /* border thickness */
				      CopyFromParent,
				      CopyFromParent,
				      CopyFromParent,
				      CWEventMask,
				      &attrs);
  XMapRaised(Disp, winObj[winObjIdx].w);
  XFlush(Disp);
  
  winObjIdx++;
  return 1;
}

void main(argc, argv)
	int argc;
	char *argv[];
{
  XEvent event;
  void processEvent();
  int ForegroundPixel, BackgroundPixel;
  int i = 1;
  
  /* INIT */
  initImgLib();
  
  dispcells = DisplayCells(Disp, Scrn);
  theCmap   = DefaultColormap(Disp, Scrn);
  
  ForegroundPixel = BlackPixel(Disp, Scrn);
  BackgroundPixel = WhitePixel(Disp, Scrn);
  gc = DefaultGC(Disp, Scrn);

  for (i = 1; i < argc; i++) {
    printf("makeImg(%s)\n", argv[i]);
    makeImg(argv[i]);
  }
/*
  makeImg("/usr/work/htest/marvin2.gif");
  makeImg("/usr/work/htest/resdogs2.gif");
  makeImg("vl.gif");

  makeImg("funny.gif");
  makeImg("good.gif");
    makeImg("/usr/users/pei/pics/mona.gif");
    makeImg("good.gif");
    makeImg("/usr/users/pei/pics/candle1.gif");
    makeImg("good.gif");
    makeImg("/usr/users/pei/pics/candle2.gif");
    makeImg("good.gif");
    makeImg("/usr/users/pei/pics/anchor.gif");
    */
  
  printf("Press right button to exit.\n");
  while(1){
    XNextEvent(Disp, &event);
    processEvent(&event);
  }
}

void processEvent(event)
     XEvent *event;
{
  int i, x,y;
  Window w = event->xany.window;
  
  printf("window=x%x\n", w);
  
  switch(event->type) {
  case ButtonPress :
    x = ((XButtonEvent *)event)->x;
    y = ((XButtonEvent *)event)->y;
    printf("x=%d, y=%d\n", x, y);
    
    switch(((XButtonPressedEvent *)event)->button) {
    case 1: /* left button */
      printf("left button press\n");
      for (i = 0; i < winObjIdx; i++) {
	if (winObj[i].w == w) {
	  printf("DESTROYING winObj[%d]\n", i);
	  
	  imgNodeRefDec(winObj[i].node->id);
	  /*
	    freeXImage(winObj[i].image, winObj[i].ximageinfo);
	    freeImage(winObj[i].image);
	    */
	  XDestroyWindow(Disp, winObj[i].w);
	  winObj[i].w = 0;
	  winObj[i].node = NULL;
	  break;
	}
      }
      break;

    case 2: /* middle button */
      printf("middle button press\n");
      for (i = 0; i < winObjIdx; i++) {
	if (winObj[i].w == w) {
	  makeImg(winObj[i].node->id);
	  break;
	}
      }
      break;

    case 3: /* right button */
      printf("right button press: \n");
      {
	ImgNode *ip;
	for (ip = imgNodes; ip; ip = ip->next) {
	  printf("node=%x  refc=%d id=%s\n", ip, ip->refc, ip->id);
	}
      }
    }
    break;

  case Expose :
    {
      int x1, y1, x2, y2;

      printf("expose\n");

      for (i = 0; i < winObjIdx; i++) {
	if (winObj[i].w == w) {
	  printf("EXPOSURE FOR winObj[%d]\n", i);
	  XPutImage(Disp, w, gc, winObj[i].node->ximageinfo->ximage, 0, 0, 
		    0, 0,
		    winObj[i].node->width,
		    winObj[i].node->height);
	  XFlush(Disp);
	  break;
	}
      }
    }
  }
}

