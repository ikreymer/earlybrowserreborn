/*
 * Copyright 1990-93 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * event_x.c
 *
 * Scott Silvey 6/21/93: broken up eventLoop() into smaller pieces.
 */
#include "utils.h"
#include "X11/Xlib.h"
#include "X11/cursorfont.h"
#include "X11/keysym.h"
#include "X11/keysymdef.h"
#include <sys/param.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <pwd.h>
#include <ctype.h>
#include "mystrings.h"
#include "error.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "vlist.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "glib.h"
#include "event.h"
#include "misc.h"

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#ifdef _AIX
#include <sys/select.h>
#endif

#ifndef SIGEMT
#define NO_SIGEMT
#define SIGEMT SIGUNUSED
#endif
#ifndef SIGSYS
#define NO_SIGSYS
#define SIGSYS SIGUNUSED
#endif

static int signal_fatal[] = {
	SIGBUS, SIGFPE, SIGEMT, SIGILL, SIGSEGV, SIGSYS, 0
};
static int signal_nonfatal[] = {
	SIGHUP, SIGINT, SIGPIPE, SIGQUIT, SIGTERM, SIGUSR1, SIGUSR2, 0
};

typedef struct TimeInfo {
	struct timeval time;
	struct TimeInfo *next;
	int (*func)();
	VObj *obj;
	int argc;
	Packet *argv;
} TimeInfo;

#define EVENT_MASK_ABSOLUTE \
	Button1MotionMask | StructureNotifyMask | ExposureMask 

#define EVENT_MASK_DEFAULT \
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
	PointerMotionMask | PointerMotionHintMask | EnterWindowMask | \
	LeaveWindowMask | Button1MotionMask | StructureNotifyMask | \
	ExposureMask 

int finish = 0;
int obj_track = 0;
int mouse_track = 0;
int objectPeekFlag = 0;
int flag_window_tracking = 0;
int mouseButtonPressedState = 0;
FILE *user_action_tracking = NULL;

IntPair mouse;

XEvent e;

#define KEYBUFSIZE 1
char keybuf[KEYBUFSIZE];

char keyStat_key;
int keyStat_control;
int keyStat_shift;

int max_socks = NOFILE;
fd_set read_mask;
VObj *objFDList[NOFILE]; /* list of association between fd and object */
TimeInfo *firstTimeInfo = NULL;

int async_event = 0;

/* forward decl */
char *eventChar();
void processPeekKeys();
int checkParam();
void process_event();
void signalHandler();

VObj *reparented_obj = NULL;
VObj *floatingObject = NULL;

VObj *xselectionObj = NULL; /* object that has data for x clip buffer */
char *sharedSelectionBuffer = NULL;

int modalState = 0;
Packet *modalResult;

char *eventStr[] = {
	"unknown event 0",
	"unknown event 1",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify",
	"LASTEvent"
};

/*
 * to be initialized by init_event 
 */
Packet *result;		/* scrap packets */
Packet *result2;

int intBuff[4]; 	/* temporary buffers */

int init_event()
{
	int i;

	/* initialize object-fileDescriptor list */
	for (i = 0; i < max_socks; i++) objFDList[i] = NULL;

	for (i = 0; signal_nonfatal[i]; i++)
		signal(signal_nonfatal[i], signalHandler);
	for (i = 0; signal_fatal[i]; i++)
		signal(signal_fatal[i], signalHandler);

	result = makePacket();
	result2 = makePacket();
	modalResult = makePacket();

	return 1;
}

void signalHandler(sig)
	int sig;
{
	static int exiting = 0;

	if (exiting == 1) exit(0); /* alright already */

	switch (sig) {
	/* fatal signals */
	case SIGBUS:
	case SIGFPE:
#ifndef NO_SIGEMT
	case SIGEMT:
#endif
	case SIGILL:
	case SIGSEGV:
#ifndef NO_SIGSYS
	case SIGSYS:
#endif
		fprintf(stderr,
			"signalHandler: caught fatal signal %d. Exiting.\n", 
			sig);
		fflush(stderr);
		exit(0);
	break;
	case SIGHUP:
	case SIGINT:
	case SIGPIPE:
	case SIGQUIT:
	case SIGTERM:
	case SIGUSR1:
	case SIGUSR2:
	default:
		exiting = 1;

		fprintf(stderr,
			"signalHandler: caught nonfatal signal %d.\n", 
			sig);
		fprintf(stderr, "signalHandler: freeing fonts.\n");
		fflush(stderr);

		free_fonts();

		fprintf(stderr, "signalHandler: freeing X resources.\n");
		fflush(stderr);

		freeAllObjects();

		fprintf(stderr, "signalHandler: exiting.\n");
		fflush(stderr);

		exit(0);
	break;
	}
}

/*
 * HANDLE_EVENT() ...
 *
 * For Viola in library mode where calling program has its own event loop.
 * This routine processes the event passed by the calling program.
 */
void violaProcessEvent(e)
    XEvent *e;
{
    VObj *topObj, *obj;
    HashEntry *hentry;
    Window w;
    
    w = e->xany.window;
    mouse.x = mouseX(e);
    mouse.y = mouseY(e);

    if (mouse_track)
	fprintf(stderr, "mouse.x=%d, mouse.y%d\n", mouse.x, mouse.y);

    hentry = window2Obj->get(window2Obj, (long)w);
    if (hentry) {
	obj = (VObj*)hentry->val;
	if (obj && validObjectP(obj)/*XXX probably unnecessary now */) {
	    VCurrentObj = obj;
	    if (flag_window_tracking &&
		VCurrentObj != VPrevObj) {
		fprintf(stderr, "CurrentObj=\"%s\"\n", GET_name(VCurrentObj));
	    }
	    if (GET_lock(VCurrentObj) && GET_window(VCurrentObj))
		process_event(e, ACTION_TOOL);
	    else
		process_event(e, currentTool);
	} else {
	  if (verbose) fprintf(stderr, "can't find object!\n");
	}
    } else {
	if (verbose) 
	    fprintf(stderr, "Can't find object associated with window.\n");
    }
}


static struct timeval noblocktimeval = {0,0};
/*
 * VIOLA_IDLE_EVENT() ... Called by the main program each time through its
 * event loop when there isn't an event waiting on the queue.
 *
 * This routine selects on all open sockets.  When one has read info, an
 * "input" message is send to the object that owns the socket.
 *
 * Derived from eventLoop().
 *
 */
void violaIdleEvent()
{
	int i, bits;
	fd_set cur_read_mask;
	TimeInfo *tip;
	struct timeval timeout, timenow, *timeval=NULL;
	struct timeval time;
	VObj *topObj, *obj;

	FD_SET(ConnectionNumber(display), &read_mask); 
	
	if (firstTimeInfo) {
		tip = firstTimeInfo;
		gettimeofday(&timenow, (struct timezone*)NULL);
	
		timeout.tv_sec = firstTimeInfo->time.tv_sec -
			timenow.tv_sec;
		timeout.tv_usec = firstTimeInfo->time.tv_usec -
			timenow.tv_usec;
	
		if (timeout.tv_usec > 1000000) {
			timeout.tv_usec -= 1000000;
			timeout.tv_sec += 1;
		}
		if (timeout.tv_sec < 0) timeout.tv_sec = 0;
		if (timeout.tv_usec < 0) timeout.tv_usec = 0;
		timeval = &timeout;
	}
	
	bcopy(&read_mask, &cur_read_mask, sizeof(cur_read_mask));

/*	fprintf(stderr, "]\n");
*/
	bits = select(max_socks, &cur_read_mask, 
		      (fd_set*)NULL, (fd_set*)NULL, timeval);

	if (bits < 0) {
		perror("violaIdleEvent(): select() error: ");
		return;
	}
	
	if (timeval && !bits) {
		tip = firstTimeInfo;
		firstTimeInfo = firstTimeInfo->next;
		(tip->func)(tip->obj, tip->argv, tip->argc);
		if (tip->argv) free(tip->argv);
		free(tip);
	}

	if (!FD_ISSET(ConnectionNumber(display), &cur_read_mask)) {
		for (i = 0; bits && i < max_socks; i++) {
			if (!FD_ISSET(i, &cur_read_mask)) continue;
			/* tell object to handle socket data */
			if (obj = objFDList[i]) {
				if (verbose) {
					sprintf(buff, 
		"data in socket number = %d, obj = %s\n", i, GET_name(obj));
					fprintf(stderr, buff);
					messageToUser(NULL, 
						MESSAGE_DEBUG, buff);
				}
				sendMessage1(obj, "input");
			} else {
				sprintf(buff, 
		"data in fd = %d, but no associated object. closing it.\n", i);
				fprintf(stderr, buff);
				close(i);
			}
			bits--;
		}
	}
}

/*
 * 
 */
int eventLoop()
{
	VObj *topObj, *obj;
	HashEntry *hentry;
	TimeInfo *tip;
	int lock = 0;
	int i;
	Window w;
	fd_set cur_read_mask;
	struct timeval timeout, timenow, *timeval = NULL;
	int bits;

	FD_SET(ConnectionNumber(display), &read_mask);

	while (!finish) {

		if (async_event) {
			/* Handle async_event */
			async_event = 0;
		}
		/* changeToNormalMouseCursor();*/

		if (XPending(display)) {
			FD_SET(ConnectionNumber(display), &cur_read_mask);
		} else {
			struct timeval time;

			if (firstTimeInfo) {
				tip = firstTimeInfo;
				gettimeofday(&timenow, (struct timezone*)NULL);

				timeout.tv_sec = firstTimeInfo->time.tv_sec -
							timenow.tv_sec;
				timeout.tv_usec = firstTimeInfo->time.tv_usec -
							timenow.tv_usec;

				if (timeout.tv_usec > 1000000) {
					timeout.tv_usec -= 1000000;
					timeout.tv_sec += 1;
				}
				if (timeout.tv_sec < 0) timeout.tv_sec = 0;
				if (timeout.tv_usec < 0) timeout.tv_usec = 0;
				timeval = &timeout;
/*
				gettimeofday(&time, (struct timezone*)NULL);
				printf("before select: %d,%d now\n",
					time.tv_sec, time.tv_usec);
				printf("timeout : %d,%d \n",
					timeout.tv_sec, timeout.tv_usec);
*/
			}
			bcopy(&read_mask, &cur_read_mask,sizeof cur_read_mask);

			bits = select(max_socks, &cur_read_mask, 
					(fd_set*)NULL,
					(fd_set*)NULL, 
					(struct timeval*)timeval);

			if (timeval && !bits) {
				tip = firstTimeInfo;
				firstTimeInfo = firstTimeInfo->next;
				(tip->func)(tip->obj, tip->argv, tip->argc);
				if (tip->argv) {
					int i = tip->argc;
					while (--i >= 0) {
						clearPacket(&(tip->argv[i]));
					}
					free(tip->argv);
				}
				free(tip);
			}
			timeval = NULL;
		}

		if (!FD_ISSET(ConnectionNumber(display), &cur_read_mask)) {
			for (i = 0; bits && i < max_socks; i++) {
				if (!FD_ISSET(i, &cur_read_mask)) continue;
				/* tell object to handle socket data */
				if (obj = objFDList[i]) {

					if (verbose) {
						sprintf(buff,
							"data in socket number = %d, obj = %s\n", 
							i, GET_name(obj));
						fprintf(stderr, buff);

						messageToUser(NULL, MESSAGE_DEBUG, buff);
					}
					sendMessage1(obj, "input");

				} else {
					sprintf(buff,
						"data in fd = %d, but no associated object. closing it.\n",
						i);
/*
					messageToUser(NULL, MESSAGE_DEBUG, buff);
*/
					fprintf(stderr, buff);
					close(i);
				}
				bits--;
			}
		} else {
			XNextEvent(display, &e);

			/* super hack */
/*
			if (e.type == MapNotify) {
			  if (verbose) {
				w = eventWindow(e);
				printf("ignoring event %s. w=ox%x\n", 
					eventStr[e.type], w);
			  }
			  continue;
			}
*/
			w = eventWindow(e);
			mouse.x = mouseX(e);
			mouse.y = mouseY(e);

			if (mouse_track) {
				fprintf(stderr, "mouse.x=%d, mouse.y%d\n",
					 mouse.x, mouse.y);
			}
			obj = findWindowObject(w);
			if (obj) {
			 	VCurrentObj = obj;
  				if (flag_window_tracking &&
				    VCurrentObj != VPrevObj) {
					fprintf(stderr, 
						"CurrentObj=\"%s\"\n", 
						GET_name(VCurrentObj));
				}
				if (GET_lock(VCurrentObj) 
				    && GET_window(VCurrentObj)) {
					process_event(&e, ACTION_TOOL);
				} else {
					process_event(&e, currentTool);
				}
			} else {
/*				fprintf(stderr, "can't find object!\n");*/
			}
/*
			hentry = window2Obj->get(window2Obj, (long)w);
			if (hentry) {
				obj = (VObj*)hentry->val;
				if (obj) {
				 	VCurrentObj = obj;
	  				if (flag_window_tracking &&
					    VCurrentObj != VPrevObj) {
						fprintf(stderr, 
							"CurrentObj=\"%s\"\n", 
							GET_name(VCurrentObj));
					}
					if (GET_lock(VCurrentObj) 
					    && GET_window(VCurrentObj)) {
						process_event(&e, ACTION_TOOL);
					} else {
						process_event(&e, currentTool);
					}
				} else {
				  fprintf(stderr, 
					  "can't find object!\n");
				}
			} else {
			  if (verbose) 
			    fprintf(stderr, 
				"Can't find object associated with window.\n");
			}
*/
		}
	}
}

void process_event(ep, tool)
	XEvent *ep;
	int tool;
{
	static VObj *dragObj = NULL;
	static int resize_corner, mouseDown = 0, from_x, from_y;
/*	Window w = GET_window(VCurrentObj);
*/
	Window w;

/*	if (verbose) {
		XExposeEvent *eep = (XExposeEvent*)ep;
		VObj *obj;
		w = eep->window;
		obj = findWindowObject(w);

		fprintf(stderr, 
			"Event=%d %18s objw=0x%x obj=0x%x %s\n", 
			eventType(*ep), eventStr[eventType(*ep)], 
			w, obj, GET_name(obj));
	}
*/
	if (dragObj) {
		if (GET_window(dragObj) == 0) {
/*			if (verbose)
				fprintf(stderr, 
					"** w=0 in dragObj =%s\n", 
					GET_name(dragObj));
*/
			dragObj = NULL;
			return;
		}
	}
	switch (eventType(*ep)) {
	case KeyPress:
		handle_KeyPress(ep);
	break;

	case KeyRelease:
		handle_KeyRelease(ep);
	break;
			
	case EnterNotify:
		handle_EnterNotify(ep, &dragObj, tool, &mouseDown);
	break;

	case LeaveNotify:
		handle_LeaveNotify(ep, &dragObj, tool, &mouseDown);

		keyStat_control = 0;
		keyStat_shift = 0;

		/*processPeekKeys();*/
	break;

	case ButtonPress:
		handle_ButtonPress(ep, &dragObj, tool, &resize_corner,
				   &mouseDown, &from_x, &from_y);
	break;

	case ButtonRelease:
		handle_ButtonRelease(ep, &dragObj, tool, &resize_corner, 
				     &mouseDown);
		break;

	case MotionNotify:
		handle_MotionNotify(ep, &dragObj, tool, &resize_corner, 
				    &mouseDown, &from_x, &from_y);
		break;

	case MapNotify: {
		VObj *obj;
		XMapEvent *eep = (XMapEvent*)ep;

/*if (verbose) printf("MapNotify! w = 0x%x\n", (XMapEvent*)eep->window);*/

		obj = findWindowObject((XMapEvent*)eep->window);

		if (obj) {
/*
			if (GET_visible(obj)) {
if (verbose) printf("MapNotify! w = ox%x obj=%s\n",
				(XMapEvent*)eep->window, GET_name(obj));
				SET_window(obj, (XMapEvent*)eep->window);
			} else {
if (verbose) printf("MapNotify! w = ox%x obj=%s not IGNORED\n", 
				(XMapEvent*)eep->window, GET_name(obj));
				SET_window(obj, (XMapEvent*)eep->window);
			}
*/
			SET_visible(obj, 1);
		}
		if (reparented_obj) {
			GLDrawRubberFrame(reparented_obj,
					GET_x(reparented_obj) + 1, 
					GET_y(reparented_obj) + 1,
					GET_x(reparented_obj) 
					  + GET_width(reparented_obj) - 1, 
					GET_y(reparented_obj) 
					  + GET_height(reparented_obj) - 1);
			reparented_obj = NULL;
		}
		break;
	}
	case Expose: {
		XExposeEvent *eep = (XExposeEvent*)ep;
		VObj *obj;
		HashEntry *entry;

/*safer & slower:
		obj = findWindowObject((XMapEvent*)eep->window);
*/
		if (entry = window2Obj->get(window2Obj, 
					 (XMapEvent*)eep->window)) {
			obj = (VObj*)entry->val;
		} else {
			obj = NULL;
		}

		if (obj) {
/*
			if (GET_visible(obj)) {
if (verbose) printf("Expose! w = ox%x obj=%s\n",
		    (XExposeEvent*)eep->window, GET_name(obj));
				SET_window(obj, (XExposeEvent*)eep->window);
			} else {
if (verbose) printf("Expose! w = ox%x obj=%s NOT IGNORED\n", 
			(XExposeEvent*)eep->window, GET_name(obj));
				SET_window(obj, (XExposeEvent*)eep->window);

			}
*/
			SET_visible(obj, 1);

			/* supress drawing, if tool-draging is going on */
			if (!mouseDown) {
				intBuff[0] = eep->x;
				intBuff[1] = eep->y;
				intBuff[2] = eep->width;
				intBuff[3] = eep->height;
				sendTokenMessageAndInts(obj, STR_expose,
							intBuff, 4);
				SET_visible(obj, 1);
			}
		}
		break;
	}
	case ResizeRequest:
		if (!mouseDown) handle_ResizeRequest(ep);
		break;

	case ConfigureNotify:
		if (!mouseDown) handle_ConfigureNotify(ep);
		break;

	case UnmapNotify: {
		XUnmapEvent *eep = (XUnmapEvent*)ep;
		VObj *obj = findWindowObject((XUnmapEvent*)eep->window);

/*if (verbose) fprintf(stderr, "unmapNotify! w=ox%x obj=%s\n",
		 eep->window, GET_name(obj));
*/
		if (obj) {
			SET_window(obj, 0);
			SET_visible(obj, 0);
		}
		break;
	}
	case DestroyNotify: {
		XDestroyWindowEvent *eep = ep;
		VObj *obj = findWindowObject((XDestroyWindowEvent*)eep->window);

/*
if (verbose) fprintf(stderr, "destroyNotify! w=ox%x obj=%s\n",
		 eep->window, GET_name(obj));
*/
		if (obj) {
			SET_window(obj, 0);
			SET_visible(obj, 0);
		}
		break;
	}
	case SelectionClear:
		if (xselectionObj) {
/*			sendMessage1(xselectionObj, "clearSelection");
*/
			GLClearSelection();
		}
		break;

	case SelectionRequest:
		if (xselectionObj) {
			char *cp;

			callMeth(xselectionObj, result, 0, NULL,
					STR_getSelection);

			cp = PkInfo2Str(result);
			if (cp) {
				Packet *result = borrowPacket();
				XSelectionEvent reply;
				XSelectionRequestEvent *sep;
				int length;

				length = strlen(cp);

				XChangeProperty(ep->xselectionrequest.display,
					ep->xselectionrequest.requestor,
					ep->xselectionrequest.property,
					XA_STRING, 8, 
					PropModeReplace, 
					cp, length);

				sep = (XSelectionRequestEvent*)ep;

				reply.type = SelectionNotify;
				reply.serial = 0;
				reply.send_event = True;
				reply.display = sep->display;
				reply.requestor = sep->requestor;
				reply.selection = sep->selection;
				reply.target = sep->target;
				reply.time = sep->time;

				if (sep->property == None) 
					reply.property = sep->target;
				else 
					reply.property = sep->property;
			  

				XSendEvent(reply.display, reply.requestor, 
						False, 0, (XEvent *)&reply);
				returnPacket();
			}
		}
		break;
	}
}

handle_KeyPress(ep)
	XKeyEvent *ep;
{
	VObj *obj = findWindowObject(ep->window);
	char *keyInfo;

	keyInfo = eventChar(ep);

	if (obj) {
		if ((unsigned int)keyInfo <= (char*)1) {
			sendMessage1(obj, "keyPress");
		} else {
			sendMessage1(obj, keyInfo);
		}
	}
	/* printf("control=%d shift=%d\n", keyStat_control, keyStat_shift);*/
}

handle_KeyRelease(ep)
	XKeyEvent *ep;
{
	VObj *obj = findWindowObject(ep->window);
	if (((int)eventChar(ep) <= 1) && obj) {
		sendMessage1(obj, "keyRelease");
	}
}

handle_EnterNotify(ep, dragObjp, tool, mouseDown)
	XEnterWindowEvent *ep;
	VObj **dragObjp;
	int tool;
	int *mouseDown;
{
	if (VCurrentObj == NULL) return;
	if (ep->detail == NotifyVirtual || 
	    ep->detail == NotifyNonlinearVirtual) return;

	if (validObjectP(VCurrentObj) == 0) {
		VCurrentObj = NULL;
		return;
	}
	
	if ((tool != ACTION_TOOL) && (*mouseDown == 0)) {
		GLDrawRubberFrame(VCurrentObj,
				GET_x(VCurrentObj) + 1, 
				GET_y(VCurrentObj) + 1,
				GET_x(VCurrentObj) 
				  + GET_width(VCurrentObj) - 1, 
				GET_y(VCurrentObj) 
				  + GET_height(VCurrentObj) - 1);
	} else {
		sendMessage1(VCurrentObj, "enter");
	}
}

handle_LeaveNotify(ep, dragObjp, tool, mouseDown)
	XLeaveWindowEvent *ep;
	VObj **dragObjp;
	int tool;
	int *mouseDown;
{
	if (VCurrentObj == NULL) return;
	if (ep->detail == NotifyVirtual ||
	    ep->detail == NotifyNonlinearVirtual) return;

	if (validObjectP(VCurrentObj) == 0) {
		VCurrentObj = NULL;
		return;
	}

	/*
	 * the cursor has moved off the top window... onto root.
	 */
	if ((tool != ACTION_TOOL) && (*mouseDown == 0)) {
		GLDrawRubberFrame(VCurrentObj,
				GET_x(VCurrentObj) + 1, 
				GET_y(VCurrentObj) + 1,
				GET_x(VCurrentObj) 
				  + GET_width(VCurrentObj) - 1, 
				GET_y(VCurrentObj) 
				  + GET_height(VCurrentObj) - 1);
	} else {
		sendMessage1(VCurrentObj, "leave");
	}
}

handle_ButtonPress(ep, dragObjp, tool, resize_corner, mouseDown, from_x,from_y)
	XButtonEvent *ep;
	VObj **dragObjp;
	int tool;
	int *resize_corner;
	int *mouseDown;
	int *from_x, *from_y;
{
	int root_x, root_y;

	if (validObjectP(VCurrentObj) == 0) {
		VCurrentObj = NULL;
		return;
	}

	mouseButtonPressedState |= 1<<(ep->button);
/*printf("press button#=%d\n", ep->button);*/

	/* only the combination of control-key & middle mouse button
	 * will get you the CLI 
	 */
	if (keyStat_control) {
		execScript(VCurrentObj, result, "shell(\"visible\", 1);");
		execScript(VCurrentObj, result, "shell(\"raise\");");
		return;
	}

	switch (tool) {
	case ACTION_TOOL: { /* press */

		if (cmd_history) {
			sprintf(buff, "%s;mouseDown\n", 
				GET_name(VCurrentObj));
			messageToUser(NULL, MESSAGE_HISTORY, buff);
		}
		*dragObjp = VCurrentObj;
		sendMessage1(VCurrentObj, "buttonPress");
	}
	break;

	case TARGET_TOOL: /* press */
		VTargetObj = VCurrentObj;
		sendMessage1(VResourceObj, "targetSet");
	break;

	case REPARENT_TOOL: {/* press */

		VObj *stack;
		Window w = GET_window(VCurrentObj);
		int child_x, child_y;

		*dragObjp = VCurrentObj;
		if (!*dragObjp) {
			IERROR("reparent tool: can't find the object! abort.\n");
		}
		if (!w) {
			MERROR(*dragObjp, "reparent to root window failed.\n");
			*dragObjp = NULL;
			return;
		}
		if (GET__classInfo(VCurrentObj) != &class_glass) {
			XSetWindowAttributes attrs;
			attrs.save_under = True;
			XChangeWindowAttributes(display, w, CWSaveUnder, 
						&attrs);
		}

#ifdef dsdsgs
		if (stack = ObjStack(*dragObjp)) {
			Window stackw = GET_window(stack);
			int stack_x, stack_y;

			w = GET_window(*dragObjp);
			GLRootPosition(w, &child_x, &child_y);
			GLRootPosition(stackw, &stack_x, &stack_y);

			/* 
			 * dragObject's param is relative to the stack object.
			 * from_{x,y} is relative to the root window.
			 */
			GLDrawRubberFrame(*dragObjp,
					GET_x(*dragObjp) + 1, 
					GET_y(*dragObjp) + 1,
					GET_x(*dragObjp) 
					  + GET_width(*dragObjp) - 1, 
					GET_y(*dragObjp) 
					  + GET_height(*dragObjp) - 1);

			SET_x(*dragObjp, child_x - stack_x);
			SET_y(*dragObjp, child_y - stack_y);

			callMethod(stack, "self", result2);
			callMethod(*dragObjp, "set", result, "parent", 
					result2, NULL);

			callMethod(VResourceObj, "self", result2);
			callMethod(*dragObjp, "set", result, "parent", 
					result2, NULL);
	
			if (GET__classInfo(VCurrentObj) != &class_glass) {
				XSetWindowAttributes attrs;
				attrs.save_under = True;
				XChangeWindowAttributes(display, w, 
							CWSaveUnder, &attrs);
				XSelectInput(display, w,
					KeyPressMask | 
					ButtonPressMask | 
					ButtonReleaseMask | 
					Button1MotionMask| 
					PointerMotionMask | 
					PointerMotionHintMask |
					EnterWindowMask | 
					LeaveWindowMask |
					SubstructureRedirectMask);
			}
			*mouseDown = 1;
			GLQueryMouse(rootWindow, &root_x, &root_y, &(mouse.x),
					&(mouse.y));

			*from_x = root_x - GET_x(*dragObjp);
			*from_y = root_y - GET_y(*dragObjp);
		} else {
			*dragObjp = NULL;
			IERROR("stack attribute is not set for object `%s'\n", GET_name(stack));
		}
#endif

printf("*	from %d,%d\n", *from_x, *from_y);
printf("	 x,y %d,%d\n", GET_x(*dragObjp), GET_y(*dragObjp));

	}
	break;

	case MOVE_TOOL: { /* press */

		Window w = GET_window(VCurrentObj);

		if (!w) return;

		/* restrict user from moving non-field (stack, card...) */
		*dragObjp = VCurrentObj;
		if (*dragObjp == NULL) {
			IERROR("move tool: can't find the object! abort.\n");
		}
		GLDrawRubberFrame(*dragObjp,
				GET_x(*dragObjp) + 1, 
				GET_y(*dragObjp) + 1,
				GET_x(*dragObjp) 
				  + GET_width(*dragObjp) - 1, 
				GET_y(*dragObjp) 
				  + GET_height(*dragObjp) - 1);

		if (GET__classInfo(VCurrentObj) != &class_glass) {

			XSetWindowAttributes attrs;
			attrs.save_under = True;
			XChangeWindowAttributes(display, w, CWSaveUnder, 
						&attrs);
			XSelectInput(display, w,
					KeyPressMask |
					ButtonPressMask | 
					ButtonReleaseMask |
					Button1MotionMask | 
					PointerMotionMask | 
					PointerMotionHintMask |
					EnterWindowMask | 
					LeaveWindowMask |
					SubstructureRedirectMask);
		}
		*mouseDown = 1;
		GLQueryMouse(rootWindow, &root_x, &root_y, 
				&(mouse.x), &(mouse.y));
		*from_x = root_x;
		*from_y = root_y;
	}
	break;

	case RESIZE_TOOL: { /* press */

		Window w = GET_window(VCurrentObj);

		if (!w) return;

		*dragObjp = VCurrentObj;

		if (GET__classInfo(VCurrentObj) != &class_glass) {

			XSetWindowAttributes attrs;

			attrs.save_under = True;
			XChangeWindowAttributes(display, w, CWSaveUnder, 
						&attrs);
			XSelectInput(display, w,
					KeyPressMask | 
					ButtonPressMask | 
					ButtonReleaseMask |
					Button1MotionMask | 
					PointerMotionMask | 
					PointerMotionHintMask |
					EnterWindowMask | 
					LeaveWindowMask |
					SubstructureRedirectMask);
		}
		*mouseDown = 1;
		GLQueryMouse(w, &root_x, &root_y, &(mouse.x), &(mouse.y));
		*from_x = root_x;
		*from_y = root_y;
			
		if (mouse.x < (int)(GET_width(*dragObjp) / 2)) {
			if (mouse.y < (int)(GET_height(*dragObjp) / 2))
				*resize_corner = RC_UPPER_LEFT;
			else
				*resize_corner = RC_LOWER_LEFT;
		} else {
			if (mouse.y < (int)(GET_height(*dragObjp) / 2))
				*resize_corner = RC_UPPER_RIGHT;
			else
				*resize_corner = RC_LOWER_RIGHT;
		}
	}
	break;
	}
}

handle_ButtonRelease(ep, dragObjp, tool, resize_corner, mouseDown)
	XButtonEvent *ep;
	VObj **dragObjp;
	int tool;
	int *resize_corner;
	int *mouseDown;
{
	Window w;

	if (validObjectP(VCurrentObj) == 0) {
		VCurrentObj = NULL;
		return;
	}
	w = GET_window(VCurrentObj);

	mouseButtonPressedState &= ~(1<<(ep->button));

	switch (tool) {
	case ACTION_TOOL: /* release */
		if (VCurrentObj == *dragObjp) {
			if (cmd_history) {
				sprintf(buff, 
					"%s;mouseDown\n", GET_name(*dragObjp));
				messageToUser(NULL, MESSAGE_HISTORY, buff);
			}
			sendMessage1(*dragObjp, "buttonRelease");
		}
		*dragObjp = NULL;
		*mouseDown = 0;
	break;

	case MOVE_TOOL:	{ /* release */
		if (*dragObjp) {
			if (GET__classInfo(VCurrentObj) != &class_glass) {
				XSetWindowAttributes attrs;

				attrs.save_under = False;
				XChangeWindowAttributes(display, w, 
							CWSaveUnder, &attrs);
				XSelectInput(display, w, 
						KeyPressMask | 
						KeyReleaseMask | 
						ButtonPressMask | 
						ButtonReleaseMask |
						PointerMotionMask | 
						PointerMotionHintMask |
						EnterWindowMask | 
						LeaveWindowMask | 
						Button1MotionMask |
						StructureNotifyMask | 
						ExposureMask);
				callMeth(*dragObjp, result, 0, result2, 
					STR_render);
			}
			GLDrawRubberFrame(*dragObjp,
					GET_x(*dragObjp) + 1, 
					GET_y(*dragObjp) + 1,
					GET_x(*dragObjp) 
					  + GET_width(*dragObjp) - 1, 
					GET_y(*dragObjp) 
					  + GET_height(*dragObjp) - 1);
		}
		*dragObjp = NULL;
		*mouseDown = 0;
		break;
	}
	case REPARENT_TOOL: /* release */
		if (*dragObjp) {
			VObj parent;
			Window pw, w = GET_window(*dragObjp);
			int parent_rx, parent_ry, child_rx, child_ry;

			if (GET__classInfo(VCurrentObj) != &class_glass) {

				XSetWindowAttributes attrs;
				attrs.save_under = False;
				XChangeWindowAttributes(display, w, 
							CWSaveUnder, &attrs);

				XSelectInput(display, w, 
						KeyPressMask | 
						KeyReleaseMask | 
						ButtonPressMask | 
						ButtonReleaseMask |
						PointerMotionMask | 
						PointerMotionHintMask |
						EnterWindowMask | 
						LeaveWindowMask | 
						Button1MotionMask |
						StructureNotifyMask | 
						ExposureMask);
			}
			GLRootPosition(w, &child_rx, &child_ry);
			unMapObject(*dragObjp);

			if (pw = GLQueryWindow()) {
				GLRootPosition(pw, &parent_rx, &parent_ry);
			} else {
				MERROR(*dragObjp, 
					"reparent tool can't find the new parent window.\n");
			}
/*
			if (parent = findWindowObject(pw)) {
				if (ScanAttr(parent, "G") && 
				    !ScanAttr(*dragObjp, "G")) {
					MERROR(*dragObjp, 
"Sorry, X11R4 does not allow visible inferiors to InputOnly windows.\n");
					} else {
						SET_x(*dragObjp, 
							child_rx - parent_rx);
						SET_y(*dragObjp, 
							child_ry - parent_ry); 

						callMethod(parent, "self", result2);
						callMethod(*dragObjp, "set", result, "parent", result2, NULL);

						callMethod(*dragObjp, "set", result, "visible", packetConst_1, NULL);
					}
				reparented_obj = *dragObjp;
			} else {
				MERROR(*dragObjp, 
					"reparent tool can't figure out the new parent\n");
			}
*/
		}
		*dragObjp = NULL;
		*mouseDown = 0;
		floatingObject = NULL;
	break;

	case RESIZE_TOOL: /* release */

		if (*dragObjp) {
			if (GET__classInfo(VCurrentObj) != &class_glass) {
				XSetWindowAttributes attrs;
		
				attrs.save_under = 0;
				XChangeWindowAttributes(display, w, 
							CWSaveUnder, &attrs);
				XSelectInput(display, w, 
						KeyPressMask | 
						KeyReleaseMask | 
						ButtonPressMask | 
						ButtonReleaseMask |
						PointerMotionMask | 
						PointerMotionHintMask |
						EnterWindowMask | 
						LeaveWindowMask | 
						Button1MotionMask |
						StructureNotifyMask | 
						ExposureMask);
			}
			intBuff[0] = GET_x(*dragObjp);
			intBuff[1] = GET_y(*dragObjp);
			intBuff[2] = GET_width(*dragObjp);
			intBuff[3] = GET_height(*dragObjp);
			sendTokenMessageAndInts(*dragObjp, STR_config, 
						intBuff, 4);

			callMeth(*dragObjp, result, 0, 0, STR_render);
			GLDrawRubberFrame(*dragObjp,
				GET_x(*dragObjp) + 1, 
				GET_y(*dragObjp) + 1,
				GET_x(*dragObjp) 
				  + GET_width(*dragObjp) - 1, 
				GET_y(*dragObjp) 
				  + GET_height(*dragObjp) - 1);
		}
		*dragObjp = NULL;
		*mouseDown = 0;
		break;
	}
}

handle_MotionNotify(ep, dragObjp, tool, resize_corner,mouseDown, from_x,from_y)
	XEvent *ep;
	VObj **dragObjp;
	int tool;
	int *resize_corner;
	int *mouseDown;
	int *from_x, *from_y;
{
	if (((XMotionEvent*)ep)->is_hint != NotifyHint) return;

	if (validObjectP(VCurrentObj) == 0) {
		VCurrentObj = NULL;
		return;
	}

	switch (tool) {
	case ACTION_TOOL: {
		int root_x, root_y, dx, dy;
		Window w = GET_window(VCurrentObj);

		if (VCurrentObj && w) {
			GLQueryMouse(w, &root_x, &root_y, 
					&(mouse.x), &(mouse.y));
			intBuff[0] = mouse.x;
			intBuff[1] = mouse.y;
			sendMessageAndInts(VCurrentObj, 
					"mouseMove", intBuff, 2);
		}
	} break;

	case MOVE_TOOL:

		if (*mouseDown && *dragObjp) {
			Window w = GET_window(*dragObjp);
			int root_x, root_y, dx, dy;

/*			if (*dragObjp != findWindowObject(w)) return;
*/
			GLQueryMouse(rootWindow, &root_x, &root_y, 
				     &(mouse.x), &(mouse.y));
		
			printf("** from %d,%d root %d,%d\n", 
					*from_x, *from_y, root_x, root_y);
			dx = root_x - *from_x;
			dy = root_y - *from_y;
			*from_x = root_x;
			*from_y = root_y;

			SET_x(*dragObjp, GET_x(*dragObjp) + dx);
			SET_y(*dragObjp, GET_y(*dragObjp) + dy);
			XMoveWindow(display, w, 
					GET_x(*dragObjp), GET_y(*dragObjp));
		}
	break;

	case REPARENT_TOOL:

		if (*mouseDown && *dragObjp) {
			Window w;
			int root_x, root_y, dx, dy;
			XSetWindowAttributes attrs;

			w = GET_window(*dragObjp);
			GLQueryMouse(rootWindow, &root_x, &root_y, 
				   &(mouse.x), &(mouse.y));

			SET_x(*dragObjp, root_x - *from_x);
			SET_y(*dragObjp, root_y - *from_y);

			printf("*from %d,%d\n", *from_x, *from_y);
			printf(" param %d,%d\n", GET_x(*dragObjp), GET_y(*dragObjp));

			attrs.override_redirect = True; 
			XChangeWindowAttributes(display, w, CWOverrideRedirect,
						&attrs);
			if (GET__parent(*dragObjp))
				GLUpdatePosition(0, w, 
					GET_x(*dragObjp), GET_y(*dragObjp));
			else
				GLUpdatePosition(1, w, 
					GET_x(*dragObjp), GET_y(*dragObjp));

			attrs.override_redirect = 0; 
			XChangeWindowAttributes(display, w, CWOverrideRedirect,
						&attrs);
		}
		break;

	case RESIZE_TOOL: /* motion */

		if (*mouseDown && *dragObjp) {
			Window pw, w = eventWindow(*((XEvent*)ep));
			int root_x, root_y, dx, dy;

			if (*dragObjp != findWindowObject(w)) return;

			GLDrawRubberFrame(*dragObjp,
					GET_x(*dragObjp) + 1,
					GET_y(*dragObjp) + 1,
					GET_x(*dragObjp) 
					  + GET_width(*dragObjp) - 1, 
					GET_y(*dragObjp) 
					  + GET_height(*dragObjp) - 1);

			GLQueryMouse(rootWindow, &root_x, &root_y, 
				   &(mouse.x), &(mouse.y));
			
			dx = root_x - *from_x;
			dy = root_y - *from_y;
			*from_x = root_x;
			*from_y = root_y;

			switch (*resize_corner) {
			case RC_UPPER:
				SET_y(*dragObjp, GET_y(*dragObjp) + dy);
				SET_height(*dragObjp, 
						GET_height(*dragObjp) - dy);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XMoveResizeWindow(display, w,
						GET_x(*dragObjp), 
						GET_y(*dragObjp),
						GET_width(*dragObjp), 
						GET_height(*dragObjp));
			break;

			case RC_LOWER:
				SET_height(*dragObjp, 
						GET_height(*dragObjp) + dy);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XMoveResizeWindow(display, w,
						GET_x(*dragObjp), 
						GET_y(*dragObjp),
						GET_width(*dragObjp), 
						GET_height(*dragObjp));
			break;

			case RC_LEFT:
				SET_x(*dragObjp, GET_x(*dragObjp) + dx);
				SET_width(*dragObjp,
					GET_width(*dragObjp) - dx);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XMoveResizeWindow(display, w,
						GET_x(*dragObjp), 
						GET_y(*dragObjp),
						GET_width(*dragObjp), 
						GET_height(*dragObjp));
			break;
		
			case RC_RIGHT:
				SET_width(*dragObjp, 
					GET_width(*dragObjp) + dx);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XResizeWindow(display, w, 
						GET_width(*dragObjp), 
						GET_height(*dragObjp));
			break;
		
			case RC_UPPER_LEFT:
				SET_x(*dragObjp, GET_x(*dragObjp) + dx);
				SET_y(*dragObjp, GET_y(*dragObjp) + dy);
				SET_width(*dragObjp, 
						GET_width(*dragObjp) - dx);
				SET_height(*dragObjp, 
						GET_height(*dragObjp) - dy);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XMoveResizeWindow(display, w,
						GET_x(*dragObjp), 
						GET_y(*dragObjp),
						GET_width(*dragObjp),
						GET_height(*dragObjp));
			break;
		
			case RC_LOWER_LEFT:
				SET_x(*dragObjp, GET_x(*dragObjp) + dx);
				SET_y(*dragObjp, GET_y(*dragObjp));
				SET_width(*dragObjp, 
						GET_width(*dragObjp) - dx);
				SET_height(*dragObjp, 
						GET_height(*dragObjp) + dy);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XMoveResizeWindow(display, w,
						GET_x(*dragObjp),
						GET_y(*dragObjp),
						GET_width(*dragObjp),
						GET_height(*dragObjp));
			break;
		
			case RC_UPPER_RIGHT:
				SET_x(*dragObjp, GET_x(*dragObjp));
				SET_y(*dragObjp, GET_y(*dragObjp) + dy);
				SET_width(*dragObjp, 
						GET_width(*dragObjp) + dx);
				SET_height(*dragObjp, 
						GET_height(*dragObjp) - dy);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XMoveWindow(display, w, GET_x(*dragObjp), 
						GET_y(*dragObjp));
				XResizeWindow(display, w, 
						GET_width(*dragObjp), 
						GET_height(*dragObjp));
			break;
		
			case RC_LOWER_RIGHT:
				SET_width(*dragObjp, 
						GET_width(*dragObjp) + dx);
				SET_height(*dragObjp, 
						GET_height(*dragObjp) + dy);
				*resize_corner = 
					checkParam(*dragObjp, *resize_corner);
				XResizeWindow(display, w, 
						GET_width(*dragObjp), 
						GET_height(*dragObjp));
			break;
			}
			GLDrawRubberFrame(*dragObjp,
					GET_x(*dragObjp) + 1, 
					GET_y(*dragObjp) + 1,
					GET_x(*dragObjp) 
					  + GET_width(*dragObjp) - 1, 
					GET_y(*dragObjp) 
					  + GET_height(*dragObjp) - 1);
		}
		break;
	}
}

#ifdef INLINED_handle_ExposeNotify
handle_ExposeNotify(ep)
	XExposeEvent *ep;
{
	VObj *obj = findWindowObject(ep->window);

/*	while(XCheckTypedWindowEvent(display, ep->window, Expose, ep));*/
/*
	if (GET__classInfo(obj) != &class_txtDisp &&
	    GET__classInfo(obj) != &class_HTML) {
		while(XCheckTypedWindowEvent(display, ep->window, Expose, ep));
	}
*/
	if (obj) {
		intBuff[0] = ep->x;
		intBuff[1] = ep->y;
		intBuff[2] = ep->width;
		intBuff[3] = ep->height;
		sendTokenMessageAndInts(obj, STR_expose, intBuff, 4);
	}
	SET_visible(obj, 1);
}
#endif

handle_ResizeRequest(ep)
	XResizeRequestEvent *ep;
{
	VObj *obj = findWindowObject(ep->window);

	if (obj) {
		intBuff[0] = GET_x(obj);
		intBuff[1] = GET_y(obj); 
		intBuff[2] = ep->width;
		intBuff[3] = ep->height;
		sendTokenMessageAndInts(obj, STR_config, intBuff, 4);
	}
}

handle_ConfigureNotify(ep)
	XConfigureEvent *ep;
{ 
	VObj *obj = findWindowObject(ep->window);

	if (obj) {
		if (ep->above != 0) {
			intBuff[0] = ep->x;
			intBuff[1] = ep->y;
		} else {
			intBuff[0] = GET_x(obj);
			intBuff[1] = GET_y(obj); 
		}
		intBuff[2] = ep->width;
		intBuff[3] = ep->height;
		sendTokenMessageAndInts(obj, STR_config, intBuff, 4);
	}
}

char *eventChar(e)
	XKeyEvent *e;
{
	KeySym ks;

	keybuf[0] = '\0';
	XLookupString (e, keybuf, KEYBUFSIZE, &ks, NULL);

	if (ks != NoSymbol) {
		/* printf("e=%ld, key[0]='%c'\n", (long)e, keybuf[0]);*/

		switch (ks) {
		case XK_Shift_L:
		case XK_Shift_R:
			if (keyStat_shift) keyStat_shift = 0;
			else keyStat_shift = 1;
			keyStat_key = '\0';
			return 0;
		case XK_Control_L:
			if (keyStat_control) keyStat_control = 0;
			else keyStat_control = 1;
			keyStat_key = '\0';
			return 0;
		case XK_Home:				/* major ickyness */
			return "key_home";
		case XK_Up:
			return "key_up";
		case XK_Down:
			return "key_down";
		case XK_Left:
			return "key_left";
		case XK_Right:
			return "key_right";
		case XK_Begin:
			return "key_begin";
		case XK_End:
			return "key_end";
		case XK_Prior:
			return "key_prior";
		case XK_Next:
			return "key_next";
		default:
			keyStat_key = keybuf[0];
		}
		if (keyStat_key != '\0') 
			return (char)1; /*XXX 1 has special meaning. ick. */
	}
	return NULL;
}

void processPeekKeys()
{
	if (keyStat_control && keyStat_shift) {
/*
		method_generic_renderObjectParam(CurrentObj);
*/
		objectPeekFlag = 1;
	} else {
		if (objectPeekFlag) {
			objectPeekFlag = 0;
/*
			method_generic_renderObjectParam(CurrentObj);
*/
		}
	}
}

int checkParam(self, resize_corner)
	VObj *self;
	int resize_corner;
{
	if (GET_width(self) < 1) {
		SET_width(self, 1);
		if ((resize_corner == RC_LEFT) ||
		    (resize_corner == RC_UPPER_LEFT) ||
		    (resize_corner == RC_LOWER_LEFT))
			resize_corner = RC_RIGHT;
		else 
			resize_corner = RC_LEFT;

	} else if (GET_height(self) < 1) {
		SET_height(self, 1);
		if ((resize_corner == RC_UPPER) ||
		    (resize_corner == RC_UPPER_RIGHT) ||
		    (resize_corner == RC_UPPER_LEFT))
			resize_corner = RC_LOWER;
		else 
			resize_corner = RC_UPPER;
	}
	return resize_corner;
}

long scheduleEvent(delay, func, obj, argc, argv)
	int delay;	/* milliseconds */
	int (*func)();
	VObj *obj;
	int argc;
	Packet *argv;
{
	TimeInfo *ltip = NULL, *tip, *new;

	/* later, do recycling by making a circular linked list... 
	 */
	if (!(new = (TimeInfo*)malloc(sizeof(struct TimeInfo)))) return 0;
	gettimeofday(&new->time, (struct timezone*)NULL);
	new->time.tv_sec += delay / 1000;
	new->time.tv_usec += (delay % 1000) * 1000;
	if (new->time.tv_usec > 1000000) {
		new->time.tv_usec -= 1000000;
		new->time.tv_sec += 1;
	}
	new->func = func;
	new->obj = obj;
	new->argc = argc;
	new->argv = argv;

	if (!firstTimeInfo) {
		firstTimeInfo = new;
		new->next = NULL;
		return (long)new;
	}
	for (tip = firstTimeInfo; tip; tip = tip->next) {
		if (new->time.tv_sec < tip->time.tv_sec) {
			if (ltip) ltip->next = new;
			else firstTimeInfo = new;
			new->next = tip;
			return (long)new;
		} else if (new->time.tv_sec == tip->time.tv_sec) {
			if (new->time.tv_usec <= tip->time.tv_usec) {
				if (ltip) ltip->next = new;
				else firstTimeInfo = new;
				new->next = tip;
				return (long)new;
			}
		}
		ltip = tip;
	}
	ltip->next = new;
	new->next = NULL;

	return (long)new;
}

int cancelEvent(timeInfoID)
	long timeInfoID;
{
	TimeInfo *ltip = NULL, *tip;

	for (tip = firstTimeInfo; tip; tip = tip->next) {
		if ((long)tip == timeInfoID) {
			if (ltip) ltip->next = tip->next;
			else firstTimeInfo = tip->next;
			if (tip->argv) free(tip->argv);
			free(tip);
			return 1;
		}
		ltip = tip;
	}
	return 0;
}

int cancelEventByObject(obj)
	VObj *obj;
{
	TimeInfo *ltip = NULL, *tip, *ntip;

	for (tip = firstTimeInfo; tip; tip = ntip) {
		if (tip->obj == obj) {
			if (ltip) ltip->next = tip->next;
			else firstTimeInfo = tip->next;
			if (tip->argv) free(tip->argv);
			ntip = tip->next;
			free(tip);
			/* don't exit yet, there may be more than one events 
			 * for this object
			 */
		} else {
			ltip = tip;
			ntip = tip->next;
		}
	}
	return 0;
}

void dumpSchedule()
{
	TimeInfo *tip;
	struct timeval time;
	int i;

	gettimeofday(&time, (struct timezone*)NULL);
	printf("%d,%d now\n", time.tv_sec, time.tv_usec);

	for (tip = firstTimeInfo; tip; tip = tip->next) {
		printf("%d,%d: obj=%s \t", 
			tip->time.tv_sec,
			tip->time.tv_usec,
			GET_name(tip->obj));
		for (i = 0; i < tip->argc; i++) {
			dumpPacket(&(tip->argv[i]));
		}
		printf("\n");
	}
}


void modalLoop()
{
	modalState = 1;

	/* loop in this state, to implement modal ness */
	while (modalState) {
		XEvent e;
		VObj *obj;

		XNextEvent(display, &e);
		obj = findWindowObject(eventWindow(e));
		if (obj) {
			process_event(&e, ACTION_TOOL);
		}
	}
}

