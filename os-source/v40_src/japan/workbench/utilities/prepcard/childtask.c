#include "prepcard.h"

/*
 * Interrupts which signal us on insert/remove of card
 */

void __asm InsertInt(register __a1 struct cmdVars *cv)
{

	cv->cv_IsInserted = TRUE;
	cv->cv_RetryCard = 1;
	Signal(cv->cv_ChildTask,1L<<cv->cv_ChildSignal);


}

/*
 * Interrupts which signal us on remove of card
 */

void __asm RemovedInt(register __a1 struct cmdVars *cv)
{

	cv->cv_IsRemoved = TRUE;
	Signal(cv->cv_ChildTask,1L<<cv->cv_ChildSignal);


}

VOID __asm PrepCardTask( register __a2 struct cmdVars *cv, register __a3 struct MsgPort *childport)
{
register struct TupleMsg *tm;
struct PrepMsg *pm;
struct CardHandle *ch;
ULONG mask,sigs;
BOOL isBUSY;
struct MsgPort *tport;
struct timerequest *tr;
BOOL isTIMER;
BOOL isTIMING;


	ch = &cv->cv_CardHandle;

	tr = NULL;
	tport = NULL;
	isTIMER = FALSE;
	isTIMING = FALSE;

	if(childport)
	{
		if(tport = CreateMsgPort())
		{
			if(tr = CreateIORequest(tport,sizeof(struct timerequest)))
			{
				if(!(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)tr,0L)))
				{
					isTIMER = TRUE;
				}
			}
		}

		childport->mp_Node.ln_Name = CHILD_PORT;
		childport->mp_Node.ln_Pri = 0;
		AddPort(childport);

		cv->cv_ChildPort = childport;

		cv->cv_ChildSignal = AllocSignal(-1L);

		cv->cv_ChildTask = FindTask(NULL);

/* obtain card ownership */

		cv->cv_CardInUse = TRUE;

		OwnCard( ch );

/* Force a card change if possible - this may force the current owner
 * to let go, or cause THIS owner to let go once, and own again - no
 * problem.  The idea is to make sure every other handle thinks its
 * a newly inserted card upon release, and obtain ownership NOW if possible
 */

		if(CardForceChange())
		{
			cv->cv_CardInUse = FALSE;
		}

	}

/* tell parent we started */

	Signal(cv->cv_Task, 1L<<cv->cv_Signal);

/* do work if child started */

	if(childport)
	{
		mask = (1L<<cv->cv_ChildSignal) | (1L<<childport->mp_SigBit);

		if(isTIMER) mask |= (1L<<tport->mp_SigBit);

		isBUSY = TRUE;

		while(isBUSY)
		{

			sigs = Wait(mask);

/* notify parent of card change */

			if(sigs & 1L<<cv->cv_ChildSignal )
			{

				Signal(cv->cv_Task, 1L<<cv->cv_Signal);
			}

			if(cv->cv_IsInserted)
			{
/* indicate card is not in use by some other device, and cannot be released */

				cv->cv_CardInUse = FALSE;
			}

			if(cv->cv_IsRemoved)
			{

/* do not release while Parent task is busy accessing.  If already
 * owned by parent task, then parent task is responsible for releasing
 * ownership.
 */

				ObtainSemaphore(&cv->cv_CardSemaphore);

/* ok to clear both flags again - won't be changed again until after
 * ReleaseCard(), if ever.
 */

				cv->cv_NoCard = TRUE;
				cv->cv_IsInserted = FALSE;
				cv->cv_IsRemoved = FALSE;

				ReleaseCard(ch,0L);

				ReleaseSemaphore(&cv->cv_CardSemaphore);
			}

			while(pm = (struct PrepMsg *)GetMsg(childport))
			{

				if(pm->pm_Command == CHILD_COPYTUPLE)
				{

					tm = (struct TupleMsg *)pm->pm_Data;

					tm->tm_result = CopyTuple(
						tm->tm_ch,
						tm->tm_tuplebuf,
						tm->tm_tuplecode,
						tm->tm_tuplesize);

				}

				if(pm->pm_Command == CHILD_QUIT)
				{ 

					ReleaseCard( ch, CARDF_REMOVEHANDLE );
					cv->cv_IsRemoved = FALSE;
					FreeSignal(cv->cv_ChildSignal);
					isBUSY = FALSE;
				}

				if(pm->pm_Command == CHILD_WAKEME)
				{
					if(isTIMER)
					{
						if(!(isTIMING))
						{


							isTIMING = TRUE;
							tr->tr_node.io_Command = TR_ADDREQUEST;
							tr->tr_time.tv_secs = 0L;
							tr->tr_time.tv_micro = 500000L;

							SendIO( (struct IORequest *)tr );
						}
					}
				}
				ReplyMsg((struct Message *)pm);			
			}


/* wake event loop if timeout (used for delayed card checking) */

			if(isTIMER)
			{
				if(GetMsg(tport))
				{

					isTIMING = FALSE;
					Signal(cv->cv_Task,1L<<cv->cv_Signal);				
				}
			}
		}

	}

	if(tport)
	{
		if( tr )
		{
			if(isTIMER)
			{
				if(isTIMING)
				{

					WaitPort( tport);
					GetMsg( tport );
				}

				CloseDevice( (struct IORequest *)tr );
			}
			DeleteIORequest( tr );
		}
		DeleteMsgPort( tport );
	}

	RemPort(childport);
	DeleteMsgPort(childport);

	
}

