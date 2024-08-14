/*
**	$Id: realtime_pragmas.h,v 40.1 93/03/16 11:12:57 vertex Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "realtime.library" */
/*--- functions in V37 or higher (Release 2.04) ---*/

/* Locks */

#pragma libcall RealTimeBase LockRealTime 1e 001
#pragma libcall RealTimeBase UnlockRealTime 24 801

/* Conductor */

#pragma libcall RealTimeBase CreatePlayerA 2a 801
#ifdef __SASC_60
#pragma tagcall RealTimeBase CreatePlayer 2a 801
#endif
#pragma libcall RealTimeBase DeletePlayer 30 801
#pragma libcall RealTimeBase SetPlayerAttrsA 36 9802
#ifdef __SASC_60
#pragma tagcall RealTimeBase SetPlayerAttrs 36 9802
#endif
#pragma libcall RealTimeBase SetConductorState 3c 10803
#pragma libcall RealTimeBase ExternalSync 42 10803
#pragma libcall RealTimeBase NextConductor 48 801
#pragma libcall RealTimeBase FindConductor 4e 801
#pragma libcall RealTimeBase GetPlayerAttrsA 54 9802
#ifdef __SASC_60
#pragma tagcall RealTimeBase GetPlayerAttrs 54 9802
#endif
