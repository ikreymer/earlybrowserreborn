/*
 * Network specific header file for midaswww
 */
#ifdef UCX                                    
#include <types.h>
#include <socket.h>
#include <errno.h>
#include <in.h>
#define bcopy(a, b, c) (void) memmove ((void *) (b), (void *) (a), (size_t) (c))
#else
#ifdef VMS
#define MULTINET
#include "multinet_root:[multinet.include.vms]inetiodef.h"
#else
#define ADDINPUTOK
#endif
#include <sys/types.h>
#define __TIME_T /* keeps VAX compiler happy */ 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#endif /* UCX */  

#include <netdb.h>
#include <stdio.h>

#ifndef VMS
#include <unistd.h>
#include <fcntl.h>
#ifdef AIXV3
#define _H_M_WAIT
#endif
#include <sys/wait.h>
#else
#include <unixio.h>
#endif

#ifdef sun
#define atexit(x) on_exit(x,NULL)
extern int sys_nerr;
extern char *sys_errlist[];
#define strerror(n) (n<sys_nerr ? sys_errlist[n] : "Unknown error code")
#endif

#ifdef VMS
#define NO_NBIO
#define unlink(a) delete(a)
#define CADDR_T
#define __TYPES_LOADED
#include ctype
#endif  

#include "midaswww.h"
