/*------ misc ---------------------------------------------------------*/
#pragma syscall Supervisor 1e d01
/*------ special patchable hooks to internal exec activity ------------*/
/*pragma syscall ExitIntr 24 0*/
/*pragma syscall Schedule 2a 0*/
/*pragma syscall Reschedule 30 0*/
/*pragma syscall Switch 36 0*/
/*pragma syscall Dispatch 3c 0*/
/*pragma syscall Exception 42 0*/
/*------ module creation ----------------------------------------------*/
#pragma syscall InitCode 48 1002
#pragma syscall InitStruct 4e a903
#pragma syscall MakeLibrary 54 10a9805
#pragma syscall MakeFunctions 5a a9803
#pragma syscall FindResident 60 901
#pragma syscall InitResident 66 1902
/*------ diagnostics --------------------------------------------------*/
#pragma syscall Alert 6c 701
#pragma syscall Debug 72 1
/*------ interrupts ---------------------------------------------------*/
#pragma syscall Disable 78 0
#pragma syscall Enable 7e 0
#pragma syscall Forbid 84 0
#pragma syscall Permit 8a 0
#pragma syscall SetSR 90 1002
#pragma syscall SuperState 96 0
#pragma syscall UserState 9c 1
#pragma syscall SetIntVector a2 9002
#pragma syscall AddIntServer a8 9002
#pragma syscall RemIntServer ae 9002
#pragma syscall Cause b4 901
/*------ memory allocation --------------------------------------------*/
#pragma syscall Allocate ba 802
#pragma syscall Deallocate c0 9803
#pragma syscall AllocMem c6 1002
#pragma syscall AllocAbs cc 9002
#pragma syscall FreeMem d2 902
#pragma syscall AvailMem d8 101
#pragma syscall AllocEntry de 801
#pragma syscall FreeEntry e4 801
/*------ lists --------------------------------------------------------*/
#pragma syscall Insert ea a9803
#pragma syscall AddHead f0 9802
#pragma syscall AddTail f6 9802
#pragma syscall Remove fc 901
#pragma syscall RemHead 102 801
#pragma syscall RemTail 108 801
#pragma syscall Enqueue 10e 9802
#pragma syscall FindName 114 9802
/*------ tasks --------------------------------------------------------*/
#pragma syscall AddTask 11a ba903
#pragma syscall RemTask 120 901
#pragma syscall FindTask 126 901
#pragma syscall SetTaskPri 12c 902
#pragma syscall SetSignal 132 1002
#pragma syscall SetExcept 138 1002
#pragma syscall Wait 13e 1
#pragma syscall Signal 144 902
#pragma syscall AllocSignal 14a 1
#pragma syscall FreeSignal 150 1
#pragma syscall AllocTrap 156 1
#pragma syscall FreeTrap 15c 1
/*------ messages -----------------------------------------------------*/
#pragma syscall AddPort 162 901
#pragma syscall RemPort 168 901
#pragma syscall PutMsg 16e 9802
#pragma syscall GetMsg 174 801
#pragma syscall ReplyMsg 17a 901
#pragma syscall WaitPort 180 801
#pragma syscall FindPort 186 901
/*------ libraries ----------------------------------------------------*/
#pragma syscall AddLibrary 18c 901
#pragma syscall RemLibrary 192 901
#pragma syscall OldOpenLibrary 198 901
#pragma syscall CloseLibrary 19e 901
#pragma syscall SetFunction 1a4 8903
#pragma syscall SumLibrary 1aa 901
/*------ devices ------------------------------------------------------*/
#pragma syscall AddDevice 1b0 901
#pragma syscall RemDevice 1b6 901
#pragma syscall OpenDevice 1bc 190804
#pragma syscall CloseDevice 1c2 901
#pragma syscall DoIO 1c8 901
#pragma syscall SendIO 1ce 901
#pragma syscall CheckIO 1d4 901
#pragma syscall WaitIO 1da 901
#pragma syscall AbortIO 1e0 901
/*------ resources ----------------------------------------------------*/
#pragma syscall AddResource 1e6 901
#pragma syscall RemResource 1ec 901
#pragma syscall OpenResource 1f2 901
/*------ private diagnostic support -----------------------------------*/
/*pragma syscall RawIOInit 1f8 0*/
/*pragma syscall RawMayGetChar 1fe 0*/
/*pragma syscall RawPutChar 204 0*/
/*------ misc ---------------------------------------------------------*/
#pragma syscall RawDoFmt 20a ba9804
#pragma syscall GetCC 210 0
#pragma syscall TypeOfMem 216 901
#pragma syscall Procure 21c 9802
#pragma syscall Vacate 222 801
#pragma syscall OpenLibrary 228 902
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
/*------ signal semaphores (note funny registers)----------------------*/
#pragma syscall InitSemaphore 22e 801
#pragma syscall ObtainSemaphore 234 801
#pragma syscall ReleaseSemaphore 23a 801
#pragma syscall AttemptSemaphore 240 801
#pragma syscall ObtainSemaphoreList 246 801
#pragma syscall ReleaseSemaphoreList 24c 801
#pragma syscall FindSemaphore 252 901
#pragma syscall AddSemaphore 258 901
#pragma syscall RemSemaphore 25e 901
/*------ kickmem support ----------------------------------------------*/
#pragma syscall SumKickData 264 0
/*------ more memory support ------------------------------------------*/
#pragma syscall AddMemList 26a 9821005
#pragma syscall CopyMem 270 9803
#pragma syscall CopyMemQuick 276 9803
/*------ cache --------------------------------------------------------*/
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
#pragma syscall CacheClearU 27c 1
#pragma syscall CacheClearE 282 1
#pragma syscall CacheControl 288 1002
/*------ misc ---------------------------------------------------------*/
#pragma syscall CreateIORequest 28e 802
#pragma syscall DeleteIORequest 294 801
#pragma syscall CreateMsgPort 29a 0
#pragma syscall DeleteMsgPort 2a0 801
#pragma syscall ObtainSemaphoreShared 2a6 801
/*------ even more memory support -------------------------------------*/
#pragma syscall AllocVec 2ac 1002
#pragma syscall FreeVec 2b2 901
#pragma syscall CreatePrivatePool 2b8 21003
#pragma syscall DeletePrivatePool 2be 801
#pragma syscall AllocPooled 2c4 8002
#pragma syscall FreePooled 2ca 8902
/*------ misc ---------------------------------------------------------*/
#pragma syscall SetFunction8 2d0 981004
#pragma syscall ColdReboot 2d6 0
#pragma syscall StackSwap 2dc 81003
/*------ task trees ---------------------------------------------------*/
#pragma syscall ChildFree 2e2 1
#pragma syscall ChildOrphan 2e8 1
#pragma syscall ChildStatus 2ee 1
#pragma syscall ChildWait 2f4 1
/*------ future expansion ---------------------------------------------*/
/*pragma syscall ExecReserved00 2fa 1*/
/*pragma syscall ExecReserved01 300 1*/
/*pragma syscall ExecReserved02 306 1*/
/*pragma syscall ExecReserved03 30c 1*/
