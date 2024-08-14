/*
**	$Id: battmem_pragmas.h,v 1.5 91/03/04 14:24:45 rsbx Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "battmem.resource" */
#pragma libcall BattMemBase ObtainBattSemaphore 6 00
#pragma libcall BattMemBase ReleaseBattSemaphore c 00
#pragma libcall BattMemBase ReadBattMem 12 10803
#pragma libcall BattMemBase WriteBattMem 18 10803
