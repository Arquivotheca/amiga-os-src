/* loadproc.c
 *
 */
/*****************************************************************************/

#include "classbase.h"
#include "classdata.h"

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

/*****************************************************************************/

void LoadHandler (void)
{
#ifdef	SysBase
#undef	SysBase
#endif
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct localData *lod;
    struct StartupMsg *sm;
    struct ClassBase *cb;
    struct Process *pr;
    ULONG sigs;
    ULONG sigr;
    Class *cl;
    Object *o;

    struct adtFrame *alf;
    ULONG frame;

    BOOL preload;
    BOOL load;

    /* What registers do we think should be in nodes */
    register struct SignalSemaphore *sem;
    register struct FrameNode *fn;
    register struct List *dlist;

    /* Find out who we are */
    pr = (struct Process *) FindTask (NULL);

    /* Get the startup message */
    WaitPort (&pr->pr_MsgPort);
    sm = (struct StartupMsg *) GetMsg (&pr->pr_MsgPort);

    /* Pull all the information from the startup message */
    cb = sm->sm_CB;
    cl = sm->sm_Class;
    o = sm->sm_Object;
    lod = INST_DATA (cl, o);

    /* All the signals that we're waiting on */
    sigs = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F;

    /* Show that we are RUNNING */
    lod->lod_Flags |= LODF_LRUNNING;

    /* Don't start out loading */
    preload = load = FALSE;
    frame = 0;

    /* Cache a pointer to the semaphore */
    dlist = &lod->lod_DiscardList;
    sem = &lod->lod_DLock;

    /* Keep going until we are told to stop */
    while (lod->lod_Flags & LODF_LGOING)
    {
	if (load || preload)
	{
	    /* Get a load message */
	    fn = NULL;
	    ObtainSemaphore (sem);		/* 642 : 332 */
	    if (lod->lod_InList <= 10)
	    {
		if (fn = (struct FrameNode *) RemHead (dlist))
		    lod->lod_OutList--;
		else
		    fn = AllocVec (sizeof (struct FrameNode), MEMF_CLEAR);
	    }
	    else
		preload = FALSE;
	    ReleaseSemaphore (sem);		/* 502 : 343 */

	    /* Make sure we have a load message */
	    if (fn)
	    {
		/* Get the current frame number */
		if (lod->lod_Frame <= frame)
		    frame++;
		else
		    frame = lod->lod_Frame;

		/* Clear the type field */
		fn->fn_Node.ln_Type = 0;
		if (frame >= lod->lod_NumFrames)
		{
		    frame = lod->lod_NumFrames - 1;
		    fn->fn_Node.ln_Type = 1;
		    load = preload = FALSE;
		}

		/* Make sure it is a frame that we should try loading */
#if 1
		if (1)
#else
		if (load || preload)
#endif
		{
		    /* Load the frame */
		    alf = &fn->fn_Frame;
		    alf->MethodID = ADTM_LOADFRAME;
		    alf->alf_TimeStamp = frame;
		    alf->alf_Frame = frame;
		    if (DoMethodA (o, (Msg) alf))
		    {
			/* Add the node to the end of the list (should be sorted by timestamp) */
			ObtainSemaphore (&lod->lod_FLock);
			lod->lod_InList++;
			AddTail (&lod->lod_FrameList, (struct Node *) fn);
			ReleaseSemaphore (&lod->lod_FLock);
		    }
		    else
		    {
			switch (IoErr ())
			{
				/* These error types will not abort the load procedure */
			    case ERROR_NO_FREE_STORE:
			    case ERROR_OBJECT_NOT_FOUND:
				break;

				/* Any other error type will cause the preload to abort */
			    default:
				preload = load = FALSE;
				frame = 0;
				break;
			}

			/* Unload the frame */
			alf->MethodID = ADTM_UNLOADFRAME;
			DoMethodA (o, (Msg) alf);

			/* Free our portion */
			FreeVec (fn);
			fn = NULL;
		    }
		}
		else
		{
		    /* Frame number is out of bounds, so discard and stop loading */
		    ObtainSemaphore (sem);	/* 0 */
		    lod->lod_OutList++;
		    AddTail (dlist, (struct Node *) fn);
		    ReleaseSemaphore (sem);	/* 0 */
		    preload = load = FALSE;
		    frame = 0;
		}
	    }

	    /* Only remove if we have several on the list */
	    if (AttemptSemaphore (sem))		/* 856 : 1148 */
	    {
		if (lod->lod_OutList > 5)
		{
		    if (fn = (struct FrameNode *) RemHead (dlist))
		    {
			lod->lod_OutList--;

			/* Unload the frame */
			alf = &fn->fn_Frame;
			alf->MethodID = ADTM_UNLOADFRAME;
			DoMethodA (o, (Msg) alf);

			/* Free	our portion */
			FreeVec (fn);
		    }
		}
		ReleaseSemaphore (sem);		/* 262 */
	    }

	    /* Check to see if anything happened */
	    sigr = CheckSignal (sigs);
	}
	else
	{
	    sigr = NULL;

	    /* Empty the discard list */
	    if (AttemptSemaphore (sem))		/* 0 */
	    {
		while ((fn = (struct FrameNode *) RemHead (dlist)) && !sigr)
		{
		    lod->lod_OutList--;

		    /* Unload the frame	 */
		    alf = &fn->fn_Frame;
		    alf->MethodID = ADTM_UNLOADFRAME;
		    DoMethodA (o, (Msg) alf);

		    /* Free our	portion	 */
		    FreeVec (fn);

		    /* Check to	see if anything	happened */
		    sigr = CheckSignal (sigs);
		}
		ReleaseSemaphore (sem);		/* 0 */
	    }

	    /* Wait for something to happen */
	    if (!sigr)
		sigr = Wait (sigs);
	}

	/* No need to check each signal if nothing came in */
	if (sigr)
	{
	    /* Dispose of the object */
	    if (sigr & SIGBREAKF_CTRL_C)
	    {
		lod->lod_Flags &= ~LODF_LGOING;
	    }

	    /* Start the load process */
	    if (sigr & SIGBREAKF_CTRL_D)
	    {
		lod->lod_Flags |= LODF_LOADING;
		load = TRUE;
	    }

	    /* Start the pre-load process */
	    if (sigr & SIGBREAKF_CTRL_E)
	    {
		lod->lod_Flags |= LODF_LOADING;
		preload = TRUE;
	    }

	    /* Stop the load process */
	    if (sigr & SIGBREAKF_CTRL_F)
	    {
		lod->lod_Flags &= ~LODF_LOADING;
		preload = load = FALSE;
		frame = 0;
	    }
	}
    }

    /* Exit now */
    Forbid ();

    /* Show that we aren't running anymore */
    lod->lod_Flags &= ~LODF_LRUNNING;
}
