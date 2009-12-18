/* Scott */
/* 
 * Include file for all programs that wish to link with Viola.
 */

#include <X11/Xlib.h>
#include "hash.h"
#include "obj.h"
#include "slotaccess.h"

#define EVENT_MASK_ABSOLUTE \
    Button1MotionMask | StructureNotifyMask | ExposureMask 
    
#define EVENT_MASK_DEFAULT \
    KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
    PointerMotionMask | PointerMotionHintMask | EnterWindowMask | \
    LeaveWindowMask | Button1MotionMask | StructureNotifyMask | \
    ExposureMask 

#define VIOLA_EVENT_MASK EVENT_MASK_ABSOLUTE

extern int perishableActiveHelp;

char *initViola();
void violaProcessEvent();
void violaIdleEvent();
void freeViolaResources();

