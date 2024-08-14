/*
**	$Id: battclock_pragmas.h,v 1.3 90/05/03 11:57:44 rsbx Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "battclock.resource" */
#pragma libcall BattClockBase ResetBattClock 6 00
#pragma libcall BattClockBase ReadBattClock c 00
#pragma libcall BattClockBase WriteBattClock 12 001
#ifdef BATTCLOCK_PRIVATE_PRAGMAS
#pragma libcall BattClockBase ReadBattClockMem 18 2102
#endif
#ifdef BATTCLOCK_PRIVATE_PRAGMAS
#pragma libcall BattClockBase WriteBattClockMem 1e 21003
#endif
