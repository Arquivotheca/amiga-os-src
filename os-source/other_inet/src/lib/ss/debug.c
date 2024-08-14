/* -----------------------------------------------------------------------
 * debug.c    function included only for debugging. Normal compile
 *             should omit this !
 *
 * $Locker:  $
 *
 * $Id: debug.c,v 1.2 91/08/07 14:15:49 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	debug.c,v $
 * Revision 1.2  91/08/07  14:15:49  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/debug.c,v 1.2 91/08/07 14:15:49 bj Exp $
 *
 *------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdarg.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include "sslib.h"


void Dprintf(char *fmt, ...)
{

    va_list args;

    if (debugfh==NULL) {
		debugfh = Open("con:0/0/640/100/SOCKET.LIBRARY/AUTO/WAIT", MODE_NEWFILE);
    }

    va_start(args,fmt);
	VFPrintf(debugfh,fmt,args);

    va_end(args);


}

