/* "nipc.library" */
/*--- functions in V39 or higher (distributed as Release 3.0) ---*/
/*---------------- Private NIPC Diagnostic/Configuration calls ----------------*/
/*pragma libcall NIPCBase StartMonitor 1e 00*/
/*pragma libcall NIPCBase StopMonitor 24 00*/
/*pragma libcall NIPCBase AddRoute 2a 4321005*/
/*pragma libcall NIPCBase DeleteRoute 30 001*/
#pragma libcall NIPCBase StartStats 36 801
#pragma libcall NIPCBase EndStats 3c 00
/*--- (8 function slots reserved here) ---*/
/*---------------- Transactions -----------------------------------------------*/
#pragma libcall NIPCBase AllocTransactionA 72 801
#pragma libcall NIPCBase FreeTransaction 78 901
/*---------------- Entities ---------------------------------------------------*/
#pragma libcall NIPCBase CreateEntityA 7e 801
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
#pragma libcall NIPCBase NetQueryA d2 910804
