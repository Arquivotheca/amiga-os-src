/***********************************************************************
 * GetFileSize.c - sets the size on a file                             *
 * Written August 1988 by Talin                                        *
 ***********************************************************************/
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include "functions.h"
#include "xfunctions.h"
#include "macros.h"

#define SCAT	goto ex

LONG GetFileSize(char *filename)
{	register BPTR					fl=NULL;
	register struct FileInfoBlock	*fib;
	register LONG					result = -1,
									success;

	if (!MakeStruct(fib)) SCAT;			/* create the file info block */
	if (!(fl = Lock(filename,ACCESS_READ))) SCAT;

	if (success = Examine(fl,fib)) result = fib->fib_Size;	/* object size */
ex:
	if (fl) UnLock(fl);									/* get rid of lock */	
	UnMakeStruct(fib);									/* de-alloc fib */
	return result;
}

