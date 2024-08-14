/*
**	$Id: nipc_pragmas.h,v 1.12 93/09/03 17:46:18 kcd Exp $
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
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase AddRoute 2a 321004
#endif
#ifdef NIPC_PRIVATE_PRAGMAS
#pragma libcall NIPCBase DeleteRoute 30 001
#endif
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
/*--- (7 function slots reserved here) ---*/
/*---------------- Transactions -----------------------------------------------*/
#pragma libcall NIPCBase AllocTransactionA 7e 801
#ifdef __SASC_60
#pragma tagcall NIPCBase AllocTransaction 7e 801
#endif
#pragma libcall NIPCBase FreeTransaction 84 901
/*---------------- Entities ---------------------------------------------------*/
#pragma libcall NIPCBase CreateEntityA 8a 801
#ifdef __SASC_60
#pragma tagcall NIPCBase CreateEntity 8a 801
#endif
#pragma libcall NIPCBase DeleteEntity 90 801
#pragma libcall NIPCBase FindEntity 96 BA9804
#pragma libcall NIPCBase LoseEntity 9c 801
/*---------------- NIPC I/O ---------------------------------------------------*/
#pragma libcall NIPCBase DoTransaction a2 A9803
#pragma libcall NIPCBase BeginTransaction a8 A9803
#pragma libcall NIPCBase GetTransaction ae 801
#pragma libcall NIPCBase ReplyTransaction b4 901
#pragma libcall NIPCBase CheckTransaction ba 901
#pragma libcall NIPCBase AbortTransaction c0 901
#pragma libcall NIPCBase WaitTransaction c6 901
#pragma libcall NIPCBase WaitEntity cc 801
/*---------------- Network Information ----------------------------------------*/
#pragma libcall NIPCBase GetEntityName d2 09803
#pragma libcall NIPCBase GetHostName d8 09803
#pragma libcall NIPCBase NIPCInquiryA de 910804
#ifdef __SASC_60
#pragma tagcall NIPCBase NIPCInquiry de 910804
#endif
#pragma libcall NIPCBase PingEntity e4 0802
#pragma libcall NIPCBase GetEntityAttrsA ea 9802
#ifdef __SASC_60
#pragma tagcall NIPCBase GetEntityAttrs ea 9802
#endif
#pragma libcall NIPCBase SetEntityAttrsA f0 9802
#ifdef __SASC_60
#pragma tagcall NIPCBase SetEntityAttrs f0 9802
#endif
/*---------------- NIPC Buffer Management Routines ----------------------------*/
/*--- functions in V40 or higher (Release 3.1) ---*/
#pragma libcall NIPCBase AllocNIPCBuff f6 001
#pragma libcall NIPCBase AllocNIPCBuffEntry fc 00
#pragma libcall NIPCBase CopyNIPCBuff 102 109804
#pragma libcall NIPCBase CopyToNIPCBuff 108 09803
#pragma libcall NIPCBase CopyFromNIPCBuffer 10e 09803
#pragma libcall NIPCBase FreeNIPCBuff 114 801
#pragma libcall NIPCBase FreeNIPCBuffEntry 11a 801
#pragma libcall NIPCBase NIPCBuffLength 120 801
#pragma libcall NIPCBase AppendNIPCBuff 126 9802
#pragma libcall NIPCBase NIPCBuffPointer 12c 09803
