/*
**	$Id: nipc_pragmas.h,v 1.15 94/03/17 22:09:41 jesup Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "nipc.library" */
/*--- functions in V39 or higher (Release 3) ---*/
/*---------------- Private NIPC Diagnostic/Configuration calls ----------------*/
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase StartMonitor 1e 00
#endif
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase StopMonitor 24 00
#endif
#pragma libcall NIPCBase AddRoute 2a 321004
#pragma libcall NIPCBase DeleteRoute 30 001
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase StartStats 36 801
#endif
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase EndStats 3c 00
#endif
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase SetSvcsBase 42 801
#endif
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase LockPool 48 00
#endif
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase UnlockPool 4e 00
#endif
/*--- (5 function slots reserved here) ---*/
/*---------------- Transactions -----------------------------------------------*/
#pragma libcall NIPCBase AllocTransactionA 72 801
#ifdef __SASC_60
#pragma tagcall NIPCBase AllocTransaction 72 801
#endif
#pragma libcall NIPCBase FreeTransaction 78 901
/*---------------- Entities ---------------------------------------------------*/
#pragma libcall NIPCBase CreateEntityA 7e 801
#ifdef __SASC_60
#pragma tagcall NIPCBase CreateEntity 7e 801
#endif
#pragma libcall NIPCBase DeleteEntity 84 801
#pragma libcall NIPCBase FindEntity 8a BA9804
#pragma libcall NIPCBase LoseEntity 90 801
/*---------------- NIPC I/O ---------------------------------------------------*/
#pragma libcall NIPCBase DoTransaction 96 A9803
#pragma libcall NIPCBase BeginTransaction 9c A9803
#pragma libcall NIPCBase GetTransaction a2 801
#pragma libcall NIPCBase ReplyTransaction a8 901
#pragma libcall NIPCBase CheckTransaction ae 901
#pragma libcall NIPCBase AbortTransaction b4 901
#pragma libcall NIPCBase WaitTransaction ba 901
#pragma libcall NIPCBase WaitEntity c0 801
/*---------------- Network Information ----------------------------------------*/
#pragma libcall NIPCBase GetEntityName c6 09803
#pragma libcall NIPCBase GetHostName cc 09803
#pragma libcall NIPCBase NIPCInquiryA d2 910804
#ifdef __SASC_60
#pragma tagcall NIPCBase NIPCInquiry d2 910804
#endif
#pragma libcall NIPCBase PingEntity d8 0802
#pragma libcall NIPCBase GetEntityAttrsA de 9802
#ifdef __SASC_60
#pragma tagcall NIPCBase GetEntityAttrs de 9802
#endif
#pragma libcall NIPCBase SetEntityAttrsA e4 9802
#ifdef __SASC_60
#pragma tagcall NIPCBase SetEntityAttrs e4 9802
#endif
/*---------------- NIPC Buffer Management Routines ----------------------------*/
/*--- functions in V40 or higher (Release 3.1) ---*/
#pragma libcall NIPCBase AllocNIPCBuff ea 001
#pragma libcall NIPCBase AllocNIPCBuffEntry f0 00
#pragma libcall NIPCBase CopyNIPCBuff f6 109804
#pragma libcall NIPCBase CopyToNIPCBuff fc 09803
#pragma libcall NIPCBase CopyFromNIPCBuffer 102 09803
#pragma libcall NIPCBase FreeNIPCBuff 108 801
#pragma libcall NIPCBase FreeNIPCBuffEntry 10e 801
#pragma libcall NIPCBase NIPCBuffLength 114 801
#pragma libcall NIPCBase AppendNIPCBuff 11a 9802
#pragma libcall NIPCBase NIPCBuffPointer 120 09803
