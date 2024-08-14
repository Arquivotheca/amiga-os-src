
* *** sfuncs4.asm *************************************************************
* 
* Services Functions Implementation Algorithms and Routines
* =Robert J. Mical=
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 25-Oct-88  =RJ Mical=     Created this file!
* 
* *****************************************************************************


   INCLUDE "assembly.i"

   NOLIST
   INCLUDE "exec/types.i"

   INCLUDE "janus/janusbase.i"
   INCLUDE "janus/memory.i"
   INCLUDE "janus/services.i"

    INCLUDE "serviceint.i"
   LIST

   INCLUDE "asmsupp.i"


   XLIB   ObtainSemaphore
   XLIB   ReleaseSemaphore


   XDEF   LockServiceData
   XDEF   UnlockServiceData

   XLIB   MakeBytePtr
   XLIB   MakeWordPtr
   XLIB   JanusUnlock
   XLIB   JanusLock



* =============================================================================
* === IMPLEMENTATION ALGORITHMS ===============================================
* =============================================================================
* 
* This section is for Torsten and RJ.  These notes are *not* to be released 
* to the real world, please.




LockServiceData:
******* janus.library/LockServiceData ***************************************
*
*   NAME   
*     LockServiceData -- Lock the ServiceData for exclusive use
*
*   SYNOPSIS
*     VOID LockServiceData(ServiceData)
*                          A0
*
*     VOID LockServiceData(struct ServiceData *);
*
*        From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     Locks up a ServiceData for exclusive use.  Does everything you should 
*     do before using the ServiceData fields.  
* 
*     Currently, this function on the Amiga side does:
*        - ObtainSemaphore(ServiceData->sd_Semaphore);
*        - JanusLock(&ServiceData->sd_ServiceDataLock);
*     On the PC side this function does:
*        - JanusLock(&ServiceData->sd_ServiceDataLock);
*
*   INPUTS
*     ServiceData = address of a ServiceData structure (doesn't matter 
*                   what type of memory the pointer points to)
*
*   RESULT
*     None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     UnlockServiceData()
*
*****************************************************************************
*
*/

      MOVEM.L   A2-A3,-(SP)

   IFGE   INFOLEVEL-1
   MOVE.L   A0,-(SP)
   PUTMSG   1,<'%s/LockServiceData( $%lx )'>
   LEA.L   1*4(SP),SP
   ENDC


* - if we're the Amiga make sure the pointer is a word pointer
      CALLSYS   MakeWordPtr
	move.l	d0,a0
      MOVEA.L   A0,A2

      MOVEA.L   jb_ExecBase(A6),A3

* - if we're the Amiga lock the ServiceData semaphore
      MOVEA.L   sd_Semaphore(A0),A0
   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/LockServiceData() ObtainSemaphore( $%lx )'>
   LEA.L   1*4(SP),SP
   ENDC
      EXG.L   A6,A3
      CALLSYS   ObtainSemaphore
      EXG.L   A6,A3

* - lock the ServiceData lock
      MOVEA.L   A2,A0
      CALLSYS   MakeBytePtr
	move.l	d0,a0
   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/LockServiceData() LOCK( $%lx )'>
   LEA.L   1*4(SP),SP
   ENDC
      lea.l    sd_ServiceDataLock(A0),A0
      CALLSYS  JanusLock

      MOVEM.L   (SP)+,A2-A3

   PUTMSG   59,<'%s/LockServiceData done'>
      RTS



UnlockServiceData:
******* janus.library/UnlockServiceData *************************************
*
*   NAME   
*     UnlockServiceData -- Unlock a ServiceData locked with LockServiceData()
*
*   SYNOPSIS
*     VOID UnlockServiceData(ServiceData)
*                            A0
*
*     VOID UnlockServiceData(struct ServiceData *);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     Unlocks a ServiceData that had been locked by a call to 
*     LockServiceData().  Undoes everything that LockServiceData() does.  
* 
*     Currently, this function on the Amiga side does:
*        - JanusUnlock(&ServiceData->sd_ServiceDataLock);
*        - ReleaseSemaphore(ServiceData->sd_Semaphore);
*     On the PC side this function does:
*        - JanusUnlock(&ServiceData->sd_ServiceDataLock);
*
*   INPUTS
*     ServiceData = address of a ServiceData structure (doesn't matter 
*                   what type of memory the pointer points to)
*
*   RESULT
*     None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     LockServiceData()
*
*****************************************************************************
*
*/

      MOVEM.L   A2-A3,-(SP)

   IFGE   INFOLEVEL-1
   MOVE.L   A0,-(SP)
   PUTMSG   1,<'%s/UnlockServiceData( $%lx )'>
   LEA.L   1*4(SP),SP
   ENDC


* - if we're the Amiga make sure the pointer is a word pointer
      CALLSYS   MakeWordPtr
	move.l	d0,a0
      MOVEA.L   A0,A2

      MOVEA.L   jb_ExecBase(A6),A3

* - unlock the ServiceData lock
      CALLSYS   MakeBytePtr
	move.l	d0,a0 
   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/UnlockServiceData() UNLOCK( $%lx )'>
   LEA.L   1*4(SP),SP
   ENDC
      lea.l    sd_ServiceDataLock(A0),A0
      CALLSYS  JanusUnlock

* - if we're the Amiga unlock the ServiceData semaphore
      MOVEA.L   A2,A0
      MOVEA.L   sd_Semaphore(A0),A0
   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/UnlockServiceData() ReleaseSemaphore( $%lx )'>
   LEA.L   1*4(SP),SP
   ENDC
      EXG.L   A6,A3
      CALLSYS   ReleaseSemaphore
      EXG.L   A6,A3

      MOVEM.L   (SP)+,A2-A3

   PUTMSG   59,<'%s/UnlockServiceData done'>
      RTS



      END



