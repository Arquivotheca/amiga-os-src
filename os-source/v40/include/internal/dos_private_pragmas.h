/*pragma libcall DOSBase Open 1e 2102*/
/*pragma libcall DOSBase Close 24 101*/
/*pragma libcall DOSBase Read 2a 32103*/
/*pragma libcall DOSBase Write 30 32103*/
/*pragma libcall DOSBase Input 36 0*/
/*pragma libcall DOSBase Output 3c 0*/
/*pragma libcall DOSBase Seek 42 32103*/
/*pragma libcall DOSBase DeleteFile 48 101*/
/*pragma libcall DOSBase Rename 4e 2102*/
/*pragma libcall DOSBase Lock 54 2102*/
/*pragma libcall DOSBase UnLock 5a 101*/
/*pragma libcall DOSBase DupLock 60 101*/
/*pragma libcall DOSBase Examine 66 2102*/
/*pragma libcall DOSBase ExNext 6c 2102*/
/*pragma libcall DOSBase Info 72 2102*/
/*pragma libcall DOSBase CreateDir 78 101*/
/*pragma libcall DOSBase CurrentDir 7e 101*/
/*pragma libcall DOSBase IoErr 84 0*/
/*pragma libcall DOSBase CreateProc 8a 432104*/
/*pragma libcall DOSBase Exit 90 101*/
/*pragma libcall DOSBase LoadSeg 96 101*/
/*pragma libcall DOSBase UnLoadSeg 9c 101*/
/*pragma libcall DOSBase ClearVec a2 2102*/
/*pragma libcall DOSBase NoReqLoadSeg a8 101*/
/*pragma libcall DOSBase DeviceProc ae 101*/
/*pragma libcall DOSBase SetComment b4 2102*/
/*pragma libcall DOSBase SetProtection ba 2102*/
/*pragma libcall DOSBase DateStamp c0 101*/
/*pragma libcall DOSBase Delay c6 101*/
/*pragma libcall DOSBase WaitForChar cc 2102*/
/*pragma libcall DOSBase ParentDir d2 101*/
/*pragma libcall DOSBase IsInteractive d8 101*/
/*pragma libcall DOSBase Execute de 32103*/
/* New entry points for 1.4*/
/************************************************************************/
/************************************************************************/
/***             Object creation/deletion                            ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase AllocDosObject e4 2102*/
/*pragma libcall DOSBase FreeDosObject ea 2102*/
/************************************************************************/
/************************************************************************/
/***             Packet Level routines                               ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase DoPkt f0 65432106*/
/*pragma libcall DOSBase SendPkt f6 32103*/
/*pragma libcall DOSBase WaitPkt fc 0*/
/*pragma libcall DOSBase ReplyPkt 102 32103*/
/*pragma libcall DOSBase AbortPkt 108 101*/
/************************************************************************/
/************************************************************************/
/***             Record Locking                                      ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase LockRecord 10e 5432105*/
/*pragma libcall DOSBase LockRecords 114 2102*/
/*pragma libcall DOSBase UnLockRecord 11a 32103*/
/*pragma libcall DOSBase UnLockRecords 120 101*/
/************************************************************************/
/************************************************************************/
/***             Buffered File I/O                                   ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase SelectInput 126 101*/
/*pragma libcall DOSBase SelectOutput 12c 101*/
/*pragma libcall DOSBase FGetC 132 101*/
/*pragma libcall DOSBase FPutC 138 2102*/
/*pragma libcall DOSBase UnGetC 13e 2102*/
/*pragma libcall DOSBase FRead 144 432104*/
/*pragma libcall DOSBase FWrite 14a 432104*/
/*pragma libcall DOSBase FGets 150 32103*/
/*pragma libcall DOSBase FPuts 156 2102*/
/*pragma libcall DOSBase VFWritef 15c 2102*/
/*pragma libcall DOSBase VFPrintf 162 32103*/
/*pragma libcall DOSBase Flush 168 101*/
/*pragma libcall DOSBase SetVBuf 16e 32103*/
/************************************************************************/
/************************************************************************/
/***             Object Management                                   ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase DupLockFromFH 174 101*/
/*pragma libcall DOSBase OpenFromLock 17a 2102*/
/*pragma libcall DOSBase ParentOfFH 180 101*/
/*pragma libcall DOSBase ExamineFH 186 2102*/
/*pragma libcall DOSBase SetFileDate 18c 2102*/
/*pragma libcall DOSBase NameFromLock 192 32103*/
/*pragma libcall DOSBase NameFromFH 198 32103*/
/*pragma libcall DOSBase SplitName 19e 5432105*/
/*pragma libcall DOSBase SameLock 1a4 2102*/
/*pragma libcall DOSBase SetMode 1aa 2102*/
/*pragma libcall DOSBase ExAll 1b0 5432105*/
/*pragma libcall DOSBase ReadLink 1b6 5432105*/
/*pragma libcall DOSBase MakeLink 1bc 32103*/
/*pragma libcall DOSBase ChangeMode 1c2 32103*/
/*pragma libcall DOSBase SetFileSize 1c8 32103*/
/************************************************************************/
/************************************************************************/
/***             Error Handling                                      ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase SetIoErr 1ce 101*/
/*pragma libcall DOSBase Fault 1d4 432104*/
/*pragma libcall DOSBase PrintFault 1da 2102*/
/*pragma libcall DOSBase ErrorReport 1e0 832104*/
/*pragma libcall DOSBase Requester 1e6 432104*/
/************************************************************************/
/************************************************************************/
/***             Process Management                                  ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase Cli 1ec 0*/
/*pragma libcall DOSBase CreateNewProc 1f2 101*/
/*pragma libcall DOSBase RunCommand 1f8 432104*/
/*pragma libcall DOSBase GetConsoleTask 1fe 0*/
/*pragma libcall DOSBase SetConsoleTask 204 101*/
/*pragma libcall DOSBase GetFileSysTask 20a 0*/
/*pragma libcall DOSBase SetFileSysTask 210 101*/
/*pragma libcall DOSBase GetArgStr 216 0*/
/*pragma libcall DOSBase SetArgStr 21c 101*/
/*pragma libcall DOSBase FindCliProc 222 101*/
/*pragma libcall DOSBase MaxCli 228 0*/
/*pragma libcall DOSBase SetCurrentDirName 22e 101*/
/*pragma libcall DOSBase GetCurrentDirName 234 2102*/
/*pragma libcall DOSBase SetProgramName 23a 101*/
/*pragma libcall DOSBase GetProgramName 240 2102*/
/*pragma libcall DOSBase SetPrompt 246 101*/
/*pragma libcall DOSBase GetPrompt 24c 2102*/
/*pragma libcall DOSBase SetProgramDir 252 101*/
/*pragma libcall DOSBase GetProgramDir 258 0*/
/*pragma libcall DOSBase System 25e 2102*/
/************************************************************************/
/************************************************************************/
/***             Device List Management                              ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase AssignLock 264 2102*/
/*pragma libcall DOSBase AssignLate 26a 2102*/
/*pragma libcall DOSBase AssignPath 270 2102*/
/*pragma libcall DOSBase AssignAdd 276 2102*/
/*pragma libcall DOSBase RemAssignList 27c 2102*/
/*pragma libcall DOSBase GetDeviceProc 282 2102*/
/*pragma libcall DOSBase FreeDeviceProc 288 101*/
/*pragma libcall DOSBase LockDosList 28e 101*/
/*pragma libcall DOSBase UnlockDosList 294 101*/
/*pragma libcall DOSBase AttemptLockDosList 29a 101*/
/*pragma libcall DOSBase RemDosEntry 2a0 101*/
/*pragma libcall DOSBase AddDosEntry 2a6 101*/
/*pragma libcall DOSBase FindDosEntry 2ac 32103*/
/*pragma libcall DOSBase NextDosEntry 2b2 2102*/
/*pragma libcall DOSBase MakeDosEntry 2b8 2102*/
/*pragma libcall DOSBase FreeDosEntry 2be 101*/
/*pragma libcall DOSBase IsFileSystem 2c4 101*/
/************************************************************************/
/************************************************************************/
/***             Handler Interface                                   ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase Format 2ca 32103*/
/*pragma libcall DOSBase Relabel 2d0 2102*/
/*pragma libcall DOSBase Inhibit 2d6 2102*/
/*pragma libcall DOSBase AddBuffers 2dc 2102*/
/************************************************************************/
/************************************************************************/
/***             Date Time Routines                                  ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase CompareDates 2e2 2102*/
/*pragma libcall DOSBase DateToStr 2e8 101*/
/*pragma libcall DOSBase StrToDate 2ee 101*/
/************************************************************************/
/************************************************************************/
/***             Image Management                                    ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase InternalLoadSeg 2f4 a98004*/
/*pragma libcall DOSBase InternalUnLoadSeg 2fa 9102*/
/*pragma libcall DOSBase NewLoadSeg 300 2102*/
/*pragma libcall DOSBase AddSegment 306 32103*/
/*pragma libcall DOSBase FindSegment 30c 32103*/
/*pragma libcall DOSBase RemSegment 312 101*/
/************************************************************************/
/************************************************************************/
/***             Command Support                                     ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase CheckSignal 318 101*/
/*pragma libcall DOSBase ReadArgs 31e 32103*/
/*pragma libcall DOSBase FindArg 324 2102*/
/*pragma libcall DOSBase ReadItem 32a 32103*/
/*pragma libcall DOSBase StrToLong 330 2102*/
/*pragma libcall DOSBase MatchFirst 336 2102*/
/*pragma libcall DOSBase MatchNext 33c 101*/
/*pragma libcall DOSBase MatchEnd 342 101*/
/*pragma libcall DOSBase ParsePattern 348 32103*/
/*pragma libcall DOSBase MatchPattern 34e 2102*/
/*pragma libcall DOSBase DosPrivateFunc4 354 0*/
/* ##public*/
/*pragma libcall DOSBase FreeArgs 35a 101*/
/*pragma libcall DOSBase DosPrivateFunc5 360 0*/
/* ##public*/
/*pragma libcall DOSBase FilePart 366 101*/
/*pragma libcall DOSBase PathPart 36c 101*/
/*pragma libcall DOSBase AddPart 372 32103*/
/************************************************************************/
/************************************************************************/
/***             Notification                                        ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase StartNotify 378 101*/
/*pragma libcall DOSBase EndNotify 37e 101*/
/************************************************************************/
/************************************************************************/
/***             Environment Variable functions                      ****/
/************************************************************************/
/************************************************************************/
/*pragma libcall DOSBase SetVar 384 432104*/
/*pragma libcall DOSBase GetVar 38a 432104*/
/*pragma libcall DOSBase DeleteVar 390 2102*/
/*pragma libcall DOSBase FindVar 396 2102*/
#pragma libcall DOSBase CliInit 39c 801
#pragma libcall DOSBase CliInitNewcli 3a2 801
#pragma libcall DOSBase CliInitRun 3a8 801
/*pragma libcall DOSBase WriteChars 3ae 2102*/
/*pragma libcall DOSBase PutStr 3b4 101*/
/*pragma libcall DOSBase VPrintf 3ba 2102*/
/*pragma libcall DOSBase MatchReplace 3c0 0*/
