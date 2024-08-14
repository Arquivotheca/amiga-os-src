
/*** intuall.h **************************************************************
 *
 *  intuall.h, general includer for intuition
 *
 *  $Id: intuall.h,v 38.3 92/05/27 11:52:34 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 *                  		Modification History
 *	date	    author :   	Comments
 *	------      ------      -------------------------------------
 *	1-30-85     -=RJ=-      created this file!
 *
 ****************************************************************************/

#include <exec/types.h>

#include <graphics/videocontrol.h>

#include "iprefs.h"
#include "sc.h"
#include "i.h"

#include "sghooks.h"
#include "cghooks.h"

#include "intuinternal.h"
#include "ism.h"
#include "ifunctions.h"

#define printf kprintf

/* some shorthand	*/
#define GDID	GetDisplayInfoData

/*
 * don't forget to define LOCKDEBUG in lockstub.asm
 *
 * LOCKDEBUG = 0: Lock-debugging disabled
 * LOCKDEBUG = 1: "Light" lock-debugging.  Enforcer hits and recoverable alerts.
 * LOCKDEBUG = 2: Full lock-debugging with enforcer hits and serial printing.
 */
#define LOCKDEBUG 0

#if LOCKDEBUG==0
#define assertLock( a, b )	;
#define checkLock( a, b )	;
#define lockFree( a, b )	;
#define enableLockCheck()	;
#else
#define enableLockCheck()	SETFLAG( IBase->DebugFlag, IDF_LOCKDEBUG )
#if LOCKDEBUG==1
#define assertLock( a, b )	AssertLock( NULL, b )
#define checkLock( a, b )	CheckLock( NULL, b )
#define lockFree( a, b )	LockFree( NULL, b )
#else /* LOCKDEBUG==2 */
#define assertLock( a, b )	AssertLock( a, b )
#define checkLock( a, b )	CheckLock( a, b )
#define lockFree( a, b )	LockFree( a, b )
#endif
#endif
