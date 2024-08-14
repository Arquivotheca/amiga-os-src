/* :ts=4
*
*	task.c
*
*	William A. Ware 					D309
*
*****************************************************************************
*	This information is CONFIDENTIAL and PROPRIETARY						*
*	Copyright 1993, Silent Software, Incorporated.  						*
*	All Rights Reserved.													*
****************************************************************************/


#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <proto/exec.h>

#include "color.h"
#include "spriteanim.h"
#include "title.h"

#include "animation.h"

/*************************************************************************
 * CreateTask() / DeleteTask()
 */

#define	ME_TASK		0
#define	ME_STACK	1
#define	NUMENTRIES	2


struct FakeMemEntry {
	ULONG	fme_Reqs;
	ULONG	fme_Length;
};

struct FakeMemList {
	struct Node			fml_Node;
	UWORD				fml_NumEntries;
	struct FakeMemEntry	fml_ME[NUMENTRIES];
};

struct FakeMemList	__far TaskMemTemplate = {
	{ 0 },
	NUMENTRIES,
	{
		{ MEMF_REVERSE | MEMF_PUBLIC | MEMF_CLEAR, sizeof (struct Task) },
		{ MEMF_PUBLIC | MEMF_REVERSE, 0 }
	}
};



struct Task *CreateTask( STRPTR name,long pri,APTR initPC,unsigned long stackSize)
{
	register struct MemList	*ml;
	register struct Task	*new;
	struct FakeMemList		fakememlist;

	stackSize = (stackSize + 3) & ~3;

	CopyMem ((char *) &TaskMemTemplate,
			 (char *) &fakememlist,
			 sizeof (fakememlist));
	fakememlist.fml_ME[ME_STACK].fme_Length = stackSize;

	if (!(ml = (struct MemList *) AllocEntry
								   ((struct MemList *) &fakememlist)))
		return (NULL);

	new = (struct Task *) ml->ml_ME[ME_TASK].me_Addr;

	new->tc_SPLower	= ml->ml_ME[ME_STACK].me_Addr;
	new->tc_SPReg	=
	new->tc_SPUpper	= (APTR) ((ULONG) new->tc_SPLower + stackSize);

	new->tc_Node.ln_Type = NT_TASK;
	new->tc_Node.ln_Pri = pri;
	new->tc_Node.ln_Name = name;

	NewList (&new->tc_MemEntry);
	AddHead (&new->tc_MemEntry, ml);

	AddTask (new, initPC, NULL);
	return (new);
}


void DeleteTask ( struct Task	*task )
{
	RemTask (task);
}



/***********************************************************************************************
 *  																						   *
 * Set up task data in a form that will allow it to be freed automatically by RemTask() 	   *
 *  																						   *
 ***********************************************************************************************/

#define STACK_SIZE		8192*2

char __far TaskTitle[] = "Startup Animation";

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



/*******************************************************************************************
 *  																					   *
 * StartAnimation - Start Animation Task												   *
 *  																					   *
 *******************************************************************************************/

int StartAnimation(void)
{
	if (!CreateTask(TaskTitle,5,Title,STACK_SIZE)) return 0;
	
	return 1;
}



