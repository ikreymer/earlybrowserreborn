#include "defhell.h"
/*
 #if defined(sun4) && defined(__GCC__)
 #define wchar_t	wchar_t_xxxxx
 #include <X11/Xlib.h>
 #undef  wchar_t
 #include <stdlib.h>
*/
/*#define X_WCHAR 1*/

#include <stdio.h>
#include "../libWWW/Library/Implementation/HTUtils.h"

/* Override Sun's stddef.h with GCC's stdtypes.h (?) */

#define print printf

#if defined(SYSV) || defined(SVR4) || defined(__svr4__) || defined(VMS)
#define bcopy(source, dest, count) memcpy(dest, source, count)
#define bzero(b, len) memset(b, 0, len)
#endif
