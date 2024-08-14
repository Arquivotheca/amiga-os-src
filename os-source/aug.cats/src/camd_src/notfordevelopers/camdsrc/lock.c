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
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* lock.c      - List locking
*
************************************************************************/

#include "camdlib.h"


/****** camd.library/LockCAMD ******************************************
*
*   NAME
*       LockCAMD -- Prevent other tasks from changing internal structures
*
*   SYNOPSIS
*       LockCAMD(locktype)
*                d0
*       APTR LockCAMD( ULONG );
*
*   FUNCTION
*       This routine will lock the internal sempahores in the CAMD library.
*       If they are already locked by another task, this routine will wait
*	until they are free.
*
*   INPUTS
*       locktype -- which internal list will be locked.
*         CL_LINKS -- locks the internal list of MidiInterfaces, MidiLinks
*             and MidiClusters. Any functions that create or delete
*             these structures (including SetMidiLinkAttrs) will be locked.
*
*   RESULT
*       If locktype is valid, returns a value that must be passed later
*	to UnlockCAMD.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       UnlockCAMD(), CreateMidi(), DeleteMidi(),
*               AddLink(), RemoveMidiLink(), SetMidiLinkAttrs();
*
*****************************************************************************
*
*/

APTR __saveds __asm LIBLockCAMD (register __d0 ULONG locktype)
{
    if (locktype < CD_NLocks)
    {	APTR ss;

	ObtainSemaphore (ss = &CamdBase->CamdSemaphores[locktype]);
	CamdBase->PublicLockBits |= (1 << locktype);
	return ss;
    }
    return NULL;
}


/****** camd.library/UnlockCAMD ******************************************
*
*   NAME
*       UnlockCAMD -- Unlock internal lists
*
*   SYNOPSIS
*       UnlockCAMD(lock)
*                   a0
*       void UnlockCAMD( APTR );
*
*   FUNCTION
*       Undoes the effects of LockCAMD().
*
*   INPUTS
*       lock -- value returned by LockCAMD(). Can be NULL.
*
*   EXAMPLE
*
*   SEE ALSO
*       LockCAMD()
*
*****************************************************************************
*
*/

void __saveds __asm LIBUnlockCAMD (register __a0 APTR lock)
{
    if (lock)
    {	LONG locktype =
	    (struct SignalSemaphore *)lock - CamdBase->CamdSemaphores;

	if (locktype < CD_NLocks)
	{
	    CamdBase->PublicLockBits &= ~(1 << locktype);
	    ReleaseSemaphore ((struct SignalSemaphore *)lock);
	}
    }
}
