/*  Amiga includes  */
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>

/*  Amiga prototypes and pragmas:  */
#ifdef AZTEC_C
#include <functions.h>
#endif
#ifdef SAS
#include <proto/all.h>
#include <dos.h>
#endif

/*  Socket library includes:  */
#include <sys/types.h>
#include <sys/socket.h>
/* #include <netinet/in.h>  included by ss/socket.h */
#include <netdb.h>
#include <ss/socket.h>

/*  ANSI includes:  */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>  /* only here because used by ss/socket.h  */

/*  Unix compatibility functions  */
#define bzero(x,y)   setmem(x,y,0)
#define bcopy(x,y,z) memcpy(y,x,z)
#define perror(x) printf("%s: %s\n", x, strerror(errno));


/*
**  Length of buffer, also max length of filenames.
**  Should be at least 255 so that long filenames can be handled.
**  Ncopy buffers currently eat stack, so very large buffers should be
**  avoided or the implementation should be changed to utilize non-stack
**  buffers.
*/
#define LENGTH 1024
