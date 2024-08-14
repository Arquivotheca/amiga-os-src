/*	system.c
 *  (c) Copyright 1991 by Ben Eng, All Rights Reserved
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <lib/misc.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <stdio.h>
#include <stdlib.h>

#define BTOC(bptr,ctype)    ((ctype *)((long)bptr << 2))
#define CTOB(ptr)           ((long)(ptr) >> 2)

extern struct Library *SysBase;

/*	Synchronous external command (wait for return)
 *	Uses your Input/Output unless you supply other handle
 *	Result will be return code of the command, unless the System() call
 *	itself fails, in which case the result will be -1
 */
long
doCommand( char *command, BPTR other )
{
	struct TagItem stags[3];

	stags[0].ti_Tag = SYS_Input;
	stags[0].ti_Data = other ? other : Input();
	stags[1].ti_Tag = SYS_Output;
	stags[1].ti_Data = other ? 0L : Output();
	stags[2].ti_Tag = TAG_DONE;
	return( System( command, stags ));
}

long
xsystem( char *cmd )
{
	long errcode = -1L;
	char *s;
	int sflag = 1;

	if( SysBase->lib_Version < 36L )
		sflag = 0;
	else if( s = getenv( "SYSTEM" ))
		sflag = (*s == 'n') ? 0 : 1;
	
	if( sflag ) {
		errcode = doCommand( cmd, 0L );
	}
	else {
		long r = Execute( cmd, (BPTR)0L, Output());
		/* if( r == -1L ) */
		errcode = IoErr();
	}
	return errcode;
}
