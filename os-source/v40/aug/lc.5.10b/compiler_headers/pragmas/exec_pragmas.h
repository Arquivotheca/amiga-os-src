/*------ misc ---------------------------------------------------------*/
#pragma libcall SysBase Supervisor 1e d01
/*------ special patchable hooks to internal exec activity ------------*/
/*pragma libcall SysBase ExitIntr 24 0*/
/*pragma libcall SysBase Schedule 2a 0*/
/*pragma libcall SysBase Reschedule 30 0*/
/*pragma libcall SysBase Switch 36 0*/
/*pragma libcall SysBase Dispatch 3c 0*/
/*pragma libcall SysBase Exception 42 0*/
/*------ module creation ----------------------------------------------*/
#pragma libcall SysBase InitCode 48 1002
#pragma libcall SysBase InitStruct 4e a903
#pragma libcall SysBase MakeLibrary 54 10a9805
#pragma libcall SysBase MakeFunctions 5a a9803
#pragma libcall SysBase FindResident 60 901
#pragma libcall SysBase InitResident 66 1902
/*------ diagnostics --------------------------------------------------*/
#pragma libcall SysBase Alert 6c 701
#pragma libcall SysBase Debug 72 1
/*------ interrupts ---------------------------------------------------*/
#pragma libcall SysBase Disable 78 0
#pragma libcall SysBase Enable 7e 0
#pragma libcall SysBase Forbid 84 0
#pragma libcall SysBase Permit 8a 0
#pragma libcall SysBase SetSR 90 1002
#pragma libcall SysBase SuperState 96 0
#pragma libcall SysBase UserState 9c 1
#pragma libcall SysBase SetIntVector a2 9002
#pragma libcall SysBase AddIntServer a8 9002
#pragma libcall SysBase RemIntServer ae 9002
#pragma libcall SysBase Cause b4 901
/*------ memory allocation --------------------------------------------*/
#pragma libcall SysBase Allocate ba 802
#pragma libcall SysBase Deallocate c0 9803
#pragma libcall SysBase AllocMem c6 1002
#pragma libcall SysBase AllocAbs cc 9002
#pragma libcall SysBase FreeMem d2 902
#pragma libcall SysBase AvailMem d8 101
#pragma libcall SysBase AllocEntry de 801
#pragma libcall SysBase FreeEntry e4 801
/*------ lists --------------------------------------------------------*/
#pragma libcall SysBase Insert ea a9803
#pragma libcall SysBase AddHead f0 9802
#pragma libcall SysBase AddTail f6 9802
#pragma libcall SysBase Remove fc 901
#pragma libcall SysBase RemHead 102 801
#pragma libcall SysBase RemTail 108 801
#pragma libcall SysBase Enqueue 10e 9802
#pragma libcall SysBase FindName 114 9802
/*------ tasks --------------------------------------------------------*/
#pragma libcall SysBase AddTask 11a ba903
#pragma libcall SysBase RemTask 120 901
#pragma libcall SysBase FindTask 126 901
#pragma libcall SysBase SetTaskPri 12c 902
#pragma libcall SysBase SetSignal 132 1002
#pragma libcall SysBase SetExcept 138 1002
#pragma libcall SysBase Wait 13e 1
#pragma libcall SysBase Signal 144 902
#pragma libcall SysBase AllocSignal 14a 1
#pragma libcall SysBase FreeSignal 150 1
#pragma libcall SysBase AllocTrap 156 1
#pragma libcall SysBase FreeTrap 15c 1
/*------ messages -----------------------------------------------------*/
#pragma libcall SysBase AddPort 162 901
#pragma libcall SysBase RemPort 168 901
#pragma libcall SysBase PutMsg 16e 9802
#pragma libcall SysBase GetMsg 174 801
#pragma libcall SysBase ReplyMsg 17a 901
#pragma libcall SysBase WaitPort 180 801
#pragma libcall SysBase FindPort 186 901
/*------ libraries ----------------------------------------------------*/
#pragma libcall SysBase AddLibrary 18c 901
#pragma libcall SysBase RemLibrary 192 901
#pragma libcall SysBase OldOpenLibrary 198 901
#pragma libcall SysBase CloseLibrary 19e 901
#pragma libcall SysBase SetFunction 1a4 8903
#pragma libcall SysBase SumLibrary 1aa 901
/*------ devices ------------------------------------------------------*/
#pragma libcall SysBase AddDevice 1b0 901
#pragma libcall SysBase RemDevice 1b6 901
#pragma libcall SysBase OpenDevice 1bc 190804
#pragma libcall SysBase CloseDevice 1c2 901
#pragma libcall SysBase DoIO 1c8 901
#pragma libcall SysBase SendIO 1ce 901
#pragma libcall SysBase CheckIO 1d4 901
#pragma libcall SysBase WaitIO 1da 901
#pragma libcall SysBase AbortIO 1e0 901
/*------ resources ----------------------------------------------------*/
#pragma libcall SysBase AddResource 1e6 901
#pragma libcall SysBase RemResource 1ec 901
#pragma libcall SysBase OpenResource 1f2 901
/*------ private diagnostic support -----------------------------------*/
/*pragma libcall SysBase RawIOInit 1f8 0*/
/*pragma libcall SysBase RawMayGetChar 1fe 0*/
/*pragma libcall SysBase RawPutChar 204 0*/
/*------ misc ---------------------------------------------------------*/
#pragma libcall SysBase RawDoFmt 20a ba9804
#pragma libcall SysBase GetCC 210 0
#pragma libcall SysBase TypeOfMem 216 901
#pragma libcall SysBase Procure 21c 9802
#pragma libcall SysBase Vacate 222 801
#pragma libcall SysBase OpenLibrary 228 902
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
/*------ signal semaphores (note funny registers)----------------------*/
#pragma libcall SysBase InitSemaphore 22e 801
#pragma libcall SysBase ObtainSemaphore 234 801
#pragma libcall SysBase ReleaseSemaphore 23a 801
#pragma libcall SysBase AttemptSemaphore 240 801
#pragma libcall SysBase ObtainSemaphoreList 246 801
#pragma libcall SysBase ReleaseSemaphoreList 24c 801
#pragma libcall SysBase FindSemaphore 252 901
#pragma libcall SysBase AddSemaphore 258 901
#pragma libcall SysBase RemSemaphore 25e 901
/*------ kickmem support ----------------------------------------------*/
#pragma libcall SysBase SumKickData 264 0
/*------ more memory support ------------------------------------------*/
#pragma libcall SysBase AddMemList 26a 9821005
#pragma libcall SysBase CopyMem 270 9803
#pragma libcall SysBase CopyMemQuick 276 9803
/*------ cache --------------------------------------------------------*/
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
#pragma libcall SysBase CacheClearU 27c 1
#pragma libcall SysBase CacheClearE 282 1
#pragma libcall SysBase CacheControl 288 1002
/*------ misc ---------------------------------------------------------*/
#pragma libcall SysBase CreateIORequest 28e 802
#pragma libcall SysBase DeleteIORequest 294 801
#pragma libcall SysBase CreateMsgPort 29a 0
#pragma libcall SysBase DeleteMsgPort 2a0 801
#pragma libcall SysBase ObtainSemaphoreShared 2a6 801
/*------ even more memory support -------------------------------------*/
#pragma libcall SysBase AllocVec 2ac 1002
#pragma libcall SysBase FreeVec 2b2 901
#pragma libcall SysBase CreatePrivatePool 2b8 21003
#pragma libcall SysBase DeletePrivatePool 2be 801
#pragma libcall SysBase AllocPooled 2c4 8002
#pragma libcall SysBase FreePooled 2ca 8902
/*------ misc ---------------------------------------------------------*/
#pragma libcall SysBase SetFunction8 2d0 981004
#pragma libcall SysBase ColdReboot 2d6 0
#pragma libcall SysBase StackSwap 2dc 81003
/*------ task trees ---------------------------------------------------*/
#pragma libcall SysBase ChildFree 2e2 1
#pragma libcall SysBase ChildOrphan 2e8 1
#pragma libcall SysBase ChildStatus 2ee 1
#pragma libcall SysBase ChildWait 2f4 1
/*------ future expansion ---------------------------------------------*/
/*pragma libcall SysBase ExecReserved00 2fa 1*/
/*pragma libcall SysBase ExecReserved01 300 1*/
/*pragma libcall SysBase ExecReserved02 306 1*/
/*pragma libcall SysBase ExecReserved03 30c 1*/
