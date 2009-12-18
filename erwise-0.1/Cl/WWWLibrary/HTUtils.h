/*	Macros for general use					HTUtils.h
**
**	See also: the system dependent file "tcp.h"
*/

/* extern void *malloc(size_t size); */

#ifndef HTUTILS_H
#define HTUTILS_H

#ifdef SHORT_NAMES
#define WWW_TraceFlag HTTrFlag
#endif

/*	Debug message control.
*/
#ifdef DEBUG

#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

#define TRACE (WWW_TraceFlag)
#define PROGRESS(str) printf(str)
extern int WWW_TraceFlag;
#else
#define TRACE 0
#define PROGRESS(str) /* nothing for now */
#endif
#define CTRACE if(TRACE)fprintf
#define tfp stdout

/*	Standard C library for malloc() etc
*/
#ifdef vax
#ifdef unix
#define ultrix	/* Assume vax+unix=ultrix */
#endif
#endif

#ifndef VMS
#ifndef ultrix
#ifdef NeXT
#include <libc.h>	/* NeXT */
#endif
#ifndef MACH		/* Vincent.Cate@furmint.nectar.cs.cmu.edu */
#include <stdlib.h>	/* ANSI */
#endif
#else
#include <malloc.h>	/* ultrix */
#include <memory.h>
#include <stdio.h>
#endif

#else				/* VMS */
#include <stdio.h>
#include <ctype.h>
#endif

#define PUBLIC			/* Accessible outside this module     */
#define PRIVATE static		/* Accessible only within this module */

#ifdef __STDC__
#define CONST const		/* "const" only exists in STDC */
#define NOPARAMS (void)
#define PARAMS(parameter_list) parameter_list
#define NOARGS (void)
#define ARGS1(t,a) \
  (t a)
#define ARGS2(t,a,u,b) \
  (t a, u b)
#define ARGS3(t,a,u,b,v,c) \
  (t a, u b, v c)
#define ARGS4(t,a,u,b,v,c,w,d) \
  (t a, u b, v c, w d)
#define ARGS5(t,a,u,b,v,c,w,d,x,e) \
  (t a, u b, v c, w d, x e)
#define ARGS6(t,a,u,b,v,c,w,d,x,e,y,f) \
  (t a, u b, v c, w d, x e, y f)
#define ARGS7(t,a,u,b,v,c,w,d,x,e,y,f,z,g) \
  (t a, u b, v c, w d, x e, y f, z g)
#define ARGS8(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h) \
  (t a, u b, v c, w d, x e, y f, z g, s h)
#define ARGS9(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h,r,i) \
  (t a, u b, v c, w d, x e, y f, z g, s h, r i)

#else  /* not ANSI */

#define CONST
#define NOPARAMS ()
#define PARAMS(parameter_list) ()
#define NOARGS ()
#define ARGS1(t,a) (a) \
  t a;
#define ARGS2(t,a,u,b) (a,b) \
  t a; u b;
#define ARGS3(t,a,u,b,v,c) (a,b,c) \
  t a; u b; v c;
#define ARGS4(t,a,u,b,v,c,w,d) (a,b,c,d) \
  t a; u b; v c; w d;
#define ARGS5(t,a,u,b,v,c,w,d,x,e) (a,b,c,d,e) \
  t a; u b; v c; w d; x e;
#define ARGS6(t,a,u,b,v,c,w,d,x,e,y,f) (a,b,c,d,e,f) \
  t a; u b; v c; w d; x e; y f;
#define ARGS7(t,a,u,b,v,c,w,d,x,e,y,f,z,g) (a,b,c,d,e,f,g) \
  t a; u b; v c; w d; x e; y f; z g;
#define ARGS8(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h) (a,b,c,d,e,f,g,h) \
  t a; u b; v c; w d; x e; y f; z g; s h;
#define ARGS9(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h,r,i) (a,b,c,d,e,f,g,h,i) \
  t a; u b; v c; w d; x e; y f; z g; s h; r i;

#endif /* __STDC__ (ANSI) */

#ifndef NULL
#define NULL ((void *)0)
#endif


/* Note: GOOD and BAD are already defined (differently) on RS6000 aix */
/* #define GOOD(status) ((status)&1)	 VMS style status: test bit 0	      */
/* #define BAD(status)  (!GOOD(status))	 Bit 0 set if OK, otherwise clear   */

#ifndef BOOLEAN_DEFINED
typedef char	BOOLEAN;		/* Logical value */
#ifndef CURSES
#ifndef TRUE
#define TRUE	(BOOLEAN)1
#define	FALSE	(BOOLEAN)0
#endif
#endif
#define BOOLEAN_DEFINED
#endif

#ifndef BOOL
#define BOOL BOOLEAN
#endif
#ifndef YES
#define YES (BOOLEAN)1
#define NO (BOOLEAN)0
#endif

#define TCP_PORT 80		/* 80 Allocated by Jon Postel/ISI 24-Jan-92 */

/*	Is character c white space? */

#ifndef NOT_ASCII
#define WHITE(c) (((unsigned char)(c))<=' ')	/* Assumes ASCII but faster */
#else
#define WHITE(c) ( ((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r') )
#endif

#define HT_LOADED (29999)		/* Instead of a socket */

#include "HTString.h"  /* String utilities */

#ifndef ERWISE_UI
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#else
//#include <varargs.h>
#include <stdarg.h>
#endif /* ERWISE */

#ifdef CURSES
/* htbrowse.c; */
#include <curses.h>

extern        WINDOW  *w_top, *w_text, *w_prompt;
extern        void    user_message PARAMS((const char *fmt, ...));
extern        void    prompt_set PARAMS((CONST char * msg));
extern        void    prompt_count PARAMS((long kb));
#else
#define user_message printf
#endif

/*	Out Of Memory checking for malloc() return:
*/
#ifndef __FILE__
#define __FILE__ ""
#define __LINE__ ""
#endif

#define outofmem(file, func) \
 { fprintf(stderr, "%s %s: out of memory.\nProgram aborted.\n", file, func); \
  exit(1);}
/* extern void outofmem PARAMS((const char *fname, const char *func)); */


extern        void    msg_init PARAMS((int height));
extern        void    msg_printf PARAMS((int y, const char *fmt, ...));
extern        void    msg_exit PARAMS((int wait_for_key));

#endif /* HTUTILS_H */

