/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1992 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* notify.c      - External cluster change notification functions
*
************************************************************************/

#include <exec/memory.h>
#include <string.h>
#include <stddef.h>

#include "camdlib.h"

/*========================================================================*/
/*                      PUBLIC INTERFACE FUNCTIONS                        */
/*========================================================================*/

#if 0
/****** camd.library/StartClusterNotify ************************************
*
*   NAME
*       StartClusterNotify -- Notify task of cluster changes
*
*   SYNOPSIS
*       void StartClusterNotify (struct ClusterNotifyNode *cn)
*                                       a0
*
*   FUNCTION
*		Allow an external task to receive notification via a signal that
*		CAMD's internal state has changed. This function is mostly useful
*		to patch editors, etc.
*
*   INPUTS
*       cn - a pointer to a ClusterNotifyNode
*
*   RESULTS
*       None
*
*   EXAMPLES
*
*		struct ClusterNotifyNode cnn;
*
*		cnn.cnn_Task = FindTask(NULL);
*		cnn.cnn_SigBit = AllocSignal(-1);
*       StartCluserNotify(&cnn);
*
*		/* later in code */
*
*		Wait(1 << cnn.cnn_SigBit);
*
*		/* someone changed the state of a cluster */
*
*   SEE ALSO
*       EndClusterNotify
****************************************************************************
*
*/
#endif

void __saveds __asm LIBStartClusterNotify (
register __a0 struct ClusterNotifyNode *cn)
{
    LockLinks();

	AddHead((struct List *)&CamdBase->NotifyList,(struct Node *)cn);

    UnlockLinks();
}

/****** camd.library/EndClusterNotify **************************************
*
*   NAME
*       EndClusterNotify -- Stop notification of cluster changes
*
*   SYNOPSIS
*       void EndClusterNotify (struct ClusterNotifyNode *cn)
*                                       a0
*
*   FUNCTION
*       Terminates notification of CAMD internal state changes
*
*   INPUTS
*       cn - a pointer to a ClusterNotifyNode
*
*   RESULTS
*       None
*
*   SEE ALSO
*       StartClusterNotify
****************************************************************************
*
*/

void __saveds __asm LIBEndClusterNotify (
register __a0 struct ClusterNotifyNode *cn)
{
    LockLinks();

	Remove((struct Node *)cn);

    UnlockLinks();
}


/*========================================================================*/
/*                      INTERNAL FUNCTIONS                                */
/*========================================================================*/

	/* notify_external must be called while links locked */

void notify_external(void)
{	struct ClusterNotifyNode	*cnn;

	for (cnn = (struct ClusterNotifyNode *)CamdBase->NotifyList.mlh_Head;
		cnn->cnn_Node.mln_Succ;
		cnn = (struct ClusterNotifyNode *)cnn->cnn_Node.mln_Succ)
	{
		Signal(cnn->cnn_Task,1 << cnn->cnn_SigBit);
	}
}
