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

#include "animation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"

#ifndef NONTASK
#include "title_pragmas.h"

struct Library 	*TitleBase;
#endif

int StartAnimation(void);

/*******************************************************************************
 *  																		   *
 * SendAnimMessage - Send a message to animation task						   *
 *  																		   *
 *******************************************************************************/

void SendAnimMessage(int msg)
{
	struct MsgPort *AnimPort;
	struct MsgPort *ReplyPort;
	struct Message *ShutdownMsg;
	
	/* See if animation is running */
	Forbid();
	if (AnimPort = FindPort("Startup Animation"))
	{
		/* Create a message reply port */
		if (ReplyPort = CreateMsgPort())
		{
			/* Allocate a message to send  */
			if (ShutdownMsg = (struct Message *)
						AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR))
			{
				/* Send this message to anim   */
				ShutdownMsg->mn_Length    = msg;

				ShutdownMsg->mn_ReplyPort = ReplyPort;
				PutMsg(AnimPort, ShutdownMsg);
				Permit();
				
				
				/* Wait for message to return  */
				WaitPort(ReplyPort);

				while(GetMsg(ReplyPort));

				/* Free the message 		   */
				FreeMem(ShutdownMsg, sizeof(struct Message));
			
			}
			else Permit();
			/* Delete MsgPort			   */
			DeleteMsgPort(ReplyPort);
		}
		else Permit();
	}
	else Permit();
}
//===============================================================================


main(int argc,char **argv)
{
	struct Task 	*task;
	struct MsgPort	*animport;


#ifdef NONTASK
	Title();
#else
	if (argc < 2) 
	{
		printf("Usage: tester BOOT | HOLD | FALSE | DOWN\n");
		exit(0);
	}

	if (!(animport = FindPort("Startup Animation")))
	{
		if (TitleBase = OpenLibrary( "title.library",0 ))
		{	
			if (!StartAnimation()) printf("Cannot start animation\n");
			CloseLibrary( TitleBase );
		}
		else printf("Cannot open library \n");
	}
	
	if (animport = FindPort("Startup Animation"))
	{
		if (!stricmp( argv[1],"BOOT" ) )
			SendAnimMessage( ANIMMSG_BOOTING );
		if (!stricmp( argv[1],"SPIN" ) )
			SendAnimMessage( ANIMMSG_HOLDOFFANIM );
		if (!stricmp( argv[1],"FALSE" ) )
			SendAnimMessage( ANIMMSG_FALSEALARM );
		if (!stricmp( argv[1],"DOWN" ) )
			SendAnimMessage( ANIMMSG_SHUTDOWN );
	}
	else printf("Cannot Open find \"Startup Animation\" Port\n" );
#endif

	exit(0);
}
