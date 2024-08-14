
/* *** lock.c ****************************************************************
 * 
 * Lock Display Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 3 Apr 86    =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"


	
BOOL	Lock(lock)
REGIST	struct	Lock *lock;
/* This guy *must* be re-entrant.  This is a quick and dirty semaphore
 * mechanism, boy you bet!  
 *
 * If the LockCount is zero, this task gets lock and the return value
 * from this routine is TRUE.
 * If the LockCount is non-zero, this task must wait until the count
 * becomes zero, or until the count becomes negative:
 *		- If the count becomes zero, this task gets the lock and the
 *		  return value from this routine is TRUE.
 *		- If the count goes negative, that means that the task that
 *		  had had the lock (presumably the owner of the lock memory)
 *		  intends on discarding the lock and is currently waiting for 
 *		  everyone who wants this lock to leave the lock alone.  It is
 *		  hoped that the owner of the lock, after everyone abandons wanting
 *		  it, will then immediately delink the lock from public access.
 *		  In any event, if the LockCount goes negative this routine returns
 *		  FALSE and the lock is not gotten.
 */
{
	if ((lock->LockCount == 0) && (lock->LockWanted NOT= 0)) WaitTOF();

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



VOID	DeathLock(lock)
REGIST	struct	Lock *lock;
{
	if (NOT Lock(lock))
		Alert(ALERT_BAD_UNLOCK_TASK);
	else
		{
		lock->LockCount = -32767;
		while (lock->LockWanted) WaitTOF();
		}
}

 
VOID	Unlock(lock)
REGIST	struct	Lock *lock;
{
	Forbid();

	if (lock->LockingTask != FindTask(0))
		Alert(ALERT_BAD_UNLOCK_TASK, NULL);
	else
		{
		if ((--lock->LockCount) == 0)
			lock->LockingTask = NULL;
		}

	Permit();
}


VOID	LifeUnlock(lock)
REGIST	struct	Lock *lock;
{
	Forbid();

	if (lock->LockingTask != FindTask(0))
		Alert(ALERT_BAD_UNLOCK_TASK);
	else
		{
		lock->LockCount = 0;
		lock->LockingTask = NULL;
		}

	Permit();
}

