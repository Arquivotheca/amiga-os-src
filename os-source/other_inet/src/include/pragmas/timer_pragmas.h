/*
**	$Id: timer_pragmas.h,v 1.6 91/01/25 15:46:51 rsbx Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "Timer.Device" */
#pragma libcall TimerBase AddTime 2a 9802
#pragma libcall TimerBase SubTime 30 9802
#pragma libcall TimerBase CmpTime 36 9802
#pragma libcall TimerBase ReadEClock 3c 801
#pragma libcall TimerBase GetSysTime 42 801
