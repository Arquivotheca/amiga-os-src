/* :ts=4
*
*	tester.c
*
*	William A. Ware						D309
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <proto/exec.h>

#include <dos/dos.h>
#include <proto/dos.h>

#include "title.h"
#include "animation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


main(int argc,char **argv)
{
	struct Task 	*task;

#ifdef NONTASK
	Title();
#else
	if (!StartAnimation())
	{
		printf( "Cannot open animation" );
		exit(0);
	}
	Delay(60*11);
	SendAnimMessage( ANIMMSG_BOOTING );

	Delay(60*4);
	SendAnimMessage( ANIMMSG_SHUTDOWN );
	Delay(60L);
	
#endif

	exit(0);
}
