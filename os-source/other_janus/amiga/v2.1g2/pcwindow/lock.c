
/**** lock.c ****************************************************************
 * 
 * Lock Display Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 3 Apr 86    =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"
#include <proto/exec.h>
#include <proto/graphics.h>

#define  DBG_MY_LOCK_ENTER       0
#define  DBG_DEATH_LOCK_ENTER    1
#define  DBG_UNLOCK_ENTER        0
#define  DBG_LIFE_UNLOCK_ENTER   1

/****i* PCWindow/MyLock ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
* This guy *must* be re-entrant.  This is a quick and dirty semaphore
* mechanism, boy you bet!  
*
* If the LockCount is zero, this task gets lock and the return value
* from this routine is TRUE.
* If the LockCount is non-zero, this task must wait until the count
* becomes zero, or until the count becomes negative:
*      - If the count becomes zero, this task gets the lock and the
*        return value from this routine is TRUE.
*      - If the count goes negative, that means that the task that
*        had had the lock (presumably the owner of the lock memory)
*        intends on discarding the lock and is currently waiting for 
*        everyone who wants this lock to leave the lock alone.  It is
*        hoped that the owner of the lock, after everyone abandons wanting
*        it, will then immediately delink the lock from public access.
*        In any event, if the LockCount goes negative this routine returns
*        FALSE and the lock is not gotten.
*
*/
   
BOOL MyLock(struct Lock *lock)
{
#if (DEBUG1 & DBG_MY_LOCK_ENTER)
   kprintf("lock.c:       MyLock(lock=0x%lx)\n",lock);
#endif

   if ((lock->LockCount == 0) && (lock->LockWanted != 0)) 
      WaitTOF();

   Forbid();
   if (lock->LockCount)
   {
      if (lock->LockingTask != FindTask(0))
      {
         lock->LockWanted++;
         while (lock->LockCount) 
         {
            if (lock->LockCount < 0)
            {
               lock->LockWanted--;
               return(FALSE);
            }
            WaitTOF();
         }
         lock->LockWanted--;
      }
   }
   lock->LockCount++;
   lock->LockingTask = FindTask(0);
   Permit();
   return(TRUE);
}

/****i* PCWindow/DeathLock ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID DeathLock(struct Lock *lock)
{
#if (DEBUG1 & DBG_DEATH_LOCK_ENTER)
   kprintf("lock.c:       DeathLock(lock=0x%lx)\n",lock);
#endif

   if (NOT MyLock(lock))
      MyAlert(ALERT_BAD_UNLOCK_TASK,NULL);
   else
   {
      lock->LockCount = -32767;
      while (lock->LockWanted) 
	     WaitTOF();
   }
}

/****i* PCWindow/Unlock ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
 
VOID Unlock(struct Lock *lock)
{
#if (DEBUG1 & DBG_UNLOCK_ENTER)
   kprintf("lock.c:       Unlock(lock=0x%lx)\n",lock);
#endif

   Forbid();

   if (lock->LockingTask != FindTask(0))
      MyAlert(ALERT_BAD_UNLOCK_TASK, NULL);
   else
   {
      if ((--lock->LockCount) == 0)
         lock->LockingTask = NULL;
   }

   Permit();
}

/****i* PCWindow/LifeUnlock ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID LifeUnlock(struct Lock *lock)
{
#if (DEBUG1 & DBG_LIFE_UNLOCK_ENTER)
   kprintf("lock.c:       LifeUnlock(lock=0x%lx)\n",lock);
#endif

   Forbid();

   if (lock->LockingTask != FindTask(0))
      MyAlert(ALERT_BAD_UNLOCK_TASK,NULL);
   else
   {
      lock->LockCount = 0;
      lock->LockingTask = NULL;
   }

   Permit();
}
