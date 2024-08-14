*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** dos_lib.asm function offsets
	IDNT	dos_LVO
	SECTION	dos_LVO0
	XDEF	_LVOOpen
_LVOOpen EQU	-30
	XDEF	_LVOClose
_LVOClose EQU	-36
	XDEF	_LVORead
_LVORead EQU	-42
	XDEF	_LVOWrite
_LVOWrite EQU	-48
	XDEF	_LVOInput
_LVOInput EQU	-54
	XDEF	_LVOOutput
_LVOOutput EQU	-60
	XDEF	_LVOSeek
_LVOSeek EQU	-66
	XDEF	_LVODeleteFile
_LVODeleteFile EQU	-72
	XDEF	_LVORename
_LVORename EQU	-78
	XDEF	_LVOLock
_LVOLock EQU	-84
	XDEF	_LVOUnLock
_LVOUnLock EQU	-90
	XDEF	_LVODupLock
_LVODupLock EQU	-96
	XDEF	_LVOExamine
_LVOExamine EQU	-102
	XDEF	_LVOExNext
_LVOExNext EQU	-108
	XDEF	_LVOInfo
_LVOInfo EQU	-114
	XDEF	_LVOCreateDir
_LVOCreateDir EQU	-120
	XDEF	_LVOCurrentDir
_LVOCurrentDir EQU	-126
	XDEF	_LVOIoErr
_LVOIoErr EQU	-132
	XDEF	_LVOCreateProc
_LVOCreateProc EQU	-138
	XDEF	_LVOExit
_LVOExit EQU	-144
	XDEF	_LVOLoadSeg
_LVOLoadSeg EQU	-150
	XDEF	_LVOUnLoadSeg
_LVOUnLoadSeg EQU	-156
	XDEF	_LVOClearVec
_LVOClearVec EQU	-162
	XDEF	_LVONoReqLoadSeg
_LVONoReqLoadSeg EQU	-168
	XDEF	_LVODeviceProc
_LVODeviceProc EQU	-174
	XDEF	_LVOSetComment
_LVOSetComment EQU	-180
	XDEF	_LVOSetProtection
_LVOSetProtection EQU	-186
	XDEF	_LVODateStamp
_LVODateStamp EQU	-192
	XDEF	_LVODelay
_LVODelay EQU	-198
	XDEF	_LVOWaitForChar
_LVOWaitForChar EQU	-204
	XDEF	_LVOParentDir
_LVOParentDir EQU	-210
	XDEF	_LVOIsInteractive
_LVOIsInteractive EQU	-216
	XDEF	_LVOExecute
_LVOExecute EQU	-222
	XDEF	_LVOAllocDosObject
_LVOAllocDosObject EQU	-228
	XDEF	_LVOFreeDosObject
_LVOFreeDosObject EQU	-234
	XDEF	_LVODoPkt
_LVODoPkt EQU	-240
	XDEF	_LVOSendPkt
_LVOSendPkt EQU	-246
	XDEF	_LVOWaitPkt
_LVOWaitPkt EQU	-252
	XDEF	_LVOReplyPkt
_LVOReplyPkt EQU	-258
	XDEF	_LVOAbortPkt
_LVOAbortPkt EQU	-264
	XDEF	_LVOLockRecord
_LVOLockRecord EQU	-270
	XDEF	_LVOLockRecords
_LVOLockRecords EQU	-276
	XDEF	_LVOUnLockRecord
_LVOUnLockRecord EQU	-282
	XDEF	_LVOUnLockRecords
_LVOUnLockRecords EQU	-288
	XDEF	_LVOSelectInput
_LVOSelectInput EQU	-294
	XDEF	_LVOSelectOutput
_LVOSelectOutput EQU	-300
	XDEF	_LVOFGetC
_LVOFGetC EQU	-306
	XDEF	_LVOFPutC
_LVOFPutC EQU	-312
	XDEF	_LVOUnGetC
_LVOUnGetC EQU	-318
	XDEF	_LVOFRead
_LVOFRead EQU	-324
	XDEF	_LVOFWrite
_LVOFWrite EQU	-330
	XDEF	_LVOFGets
_LVOFGets EQU	-336
	XDEF	_LVOFPuts
_LVOFPuts EQU	-342
	XDEF	_LVOVFWritef
_LVOVFWritef EQU	-348
	XDEF	_LVOVFPrintf
_LVOVFPrintf EQU	-354
	XDEF	_LVOFlush
_LVOFlush EQU	-360
	XDEF	_LVOSetVBuf
_LVOSetVBuf EQU	-366
	XDEF	_LVODupLockFromFH
_LVODupLockFromFH EQU	-372
	XDEF	_LVOOpenFromLock
_LVOOpenFromLock EQU	-378
	XDEF	_LVOParentOfFH
_LVOParentOfFH EQU	-384
	XDEF	_LVOExamineFH
_LVOExamineFH EQU	-390
	XDEF	_LVOSetFileDate
_LVOSetFileDate EQU	-396
	XDEF	_LVONameFromLock
_LVONameFromLock EQU	-402
	XDEF	_LVONameFromFH
_LVONameFromFH EQU	-408
	XDEF	_LVOSplitName
_LVOSplitName EQU	-414
	XDEF	_LVOSameLock
_LVOSameLock EQU	-420
	XDEF	_LVOSetMode
_LVOSetMode EQU	-426
	XDEF	_LVOExAll
_LVOExAll EQU	-432
	XDEF	_LVOReadLink
_LVOReadLink EQU	-438
	XDEF	_LVOMakeLink
_LVOMakeLink EQU	-444
	XDEF	_LVOChangeMode
_LVOChangeMode EQU	-450
	XDEF	_LVOSetFileSize
_LVOSetFileSize EQU	-456
	XDEF	_LVOSetIoErr
_LVOSetIoErr EQU	-462
	XDEF	_LVOFault
_LVOFault EQU	-468
	XDEF	_LVOPrintFault
_LVOPrintFault EQU	-474
	XDEF	_LVOErrorReport
_LVOErrorReport EQU	-480
	XDEF	_LVOCli
_LVOCli EQU	-492
	XDEF	_LVOCreateNewProc
_LVOCreateNewProc EQU	-498
	XDEF	_LVORunCommand
_LVORunCommand EQU	-504
	XDEF	_LVOGetConsoleTask
_LVOGetConsoleTask EQU	-510
	XDEF	_LVOSetConsoleTask
_LVOSetConsoleTask EQU	-516
	XDEF	_LVOGetFileSysTask
_LVOGetFileSysTask EQU	-522
	XDEF	_LVOSetFileSysTask
_LVOSetFileSysTask EQU	-528
	XDEF	_LVOGetArgStr
_LVOGetArgStr EQU	-534
	XDEF	_LVOSetArgStr
_LVOSetArgStr EQU	-540
	XDEF	_LVOFindCliProc
_LVOFindCliProc EQU	-546
	XDEF	_LVOMaxCli
_LVOMaxCli EQU	-552
	XDEF	_LVOSetCurrentDirName
_LVOSetCurrentDirName EQU	-558
	XDEF	_LVOGetCurrentDirName
_LVOGetCurrentDirName EQU	-564
	XDEF	_LVOSetProgramName
_LVOSetProgramName EQU	-570
	XDEF	_LVOGetProgramName
_LVOGetProgramName EQU	-576
	XDEF	_LVOSetPrompt
_LVOSetPrompt EQU	-582
	XDEF	_LVOGetPrompt
_LVOGetPrompt EQU	-588
	XDEF	_LVOSetProgramDir
_LVOSetProgramDir EQU	-594
	XDEF	_LVOGetProgramDir
_LVOGetProgramDir EQU	-600
	XDEF	_LVOSystemTagList
_LVOSystemTagList EQU	-606
	XDEF	_LVOAssignLock
_LVOAssignLock EQU	-612
	XDEF	_LVOAssignLate
_LVOAssignLate EQU	-618
	XDEF	_LVOAssignPath
_LVOAssignPath EQU	-624
	XDEF	_LVOAssignAdd
_LVOAssignAdd EQU	-630
	XDEF	_LVORemAssignList
_LVORemAssignList EQU	-636
	XDEF	_LVOGetDeviceProc
_LVOGetDeviceProc EQU	-642
	XDEF	_LVOFreeDeviceProc
_LVOFreeDeviceProc EQU	-648
	XDEF	_LVOLockDosList
_LVOLockDosList EQU	-654
	XDEF	_LVOUnLockDosList
_LVOUnLockDosList EQU	-660
	XDEF	_LVOAttemptLockDosList
_LVOAttemptLockDosList EQU	-666
	XDEF	_LVORemDosEntry
_LVORemDosEntry EQU	-672
	XDEF	_LVOAddDosEntry
_LVOAddDosEntry EQU	-678
	XDEF	_LVOFindDosEntry
_LVOFindDosEntry EQU	-684
	XDEF	_LVONextDosEntry
_LVONextDosEntry EQU	-690
	XDEF	_LVOMakeDosEntry
_LVOMakeDosEntry EQU	-696
	XDEF	_LVOFreeDosEntry
_LVOFreeDosEntry EQU	-702
	XDEF	_LVOIsFileSystem
_LVOIsFileSystem EQU	-708
	XDEF	_LVOFormat
_LVOFormat EQU	-714
	XDEF	_LVORelabel
_LVORelabel EQU	-720
	XDEF	_LVOInhibit
_LVOInhibit EQU	-726
	XDEF	_LVOAddBuffers
_LVOAddBuffers EQU	-732
	XDEF	_LVOCompareDates
_LVOCompareDates EQU	-738
	XDEF	_LVODateToStr
_LVODateToStr EQU	-744
	XDEF	_LVOStrToDate
_LVOStrToDate EQU	-750
	XDEF	_LVOInternalLoadSeg
_LVOInternalLoadSeg EQU	-756
	XDEF	_LVOInternalUnLoadSeg
_LVOInternalUnLoadSeg EQU	-762
	XDEF	_LVONewLoadSeg
_LVONewLoadSeg EQU	-768
	XDEF	_LVOAddSegment
_LVOAddSegment EQU	-774
	XDEF	_LVOFindSegment
_LVOFindSegment EQU	-780
	XDEF	_LVORemSegment
_LVORemSegment EQU	-786
	XDEF	_LVOCheckSignal
_LVOCheckSignal EQU	-792
	XDEF	_LVOReadArgs
_LVOReadArgs EQU	-798
	OFFSET	0
dos_FlushLVO0	ds.w	1
	SECTION	dos_LVO1
	XDEF	_LVOFindArg
_LVOFindArg EQU	-804
	XDEF	_LVOReadItem
_LVOReadItem EQU	-810
	XDEF	_LVOStrToLong
_LVOStrToLong EQU	-816
	XDEF	_LVOMatchFirst
_LVOMatchFirst EQU	-822
	XDEF	_LVOMatchNext
_LVOMatchNext EQU	-828
	XDEF	_LVOMatchEnd
_LVOMatchEnd EQU	-834
	XDEF	_LVOParsePattern
_LVOParsePattern EQU	-840
	XDEF	_LVOMatchPattern
_LVOMatchPattern EQU	-846
	XDEF	_LVODosNameFromAnchor
_LVODosNameFromAnchor EQU	-852
	XDEF	_LVOFreeArgs
_LVOFreeArgs EQU	-858
	XDEF	_LVOFilePart
_LVOFilePart EQU	-870
	XDEF	_LVOPathPart
_LVOPathPart EQU	-876
	XDEF	_LVOAddPart
_LVOAddPart EQU	-882
	XDEF	_LVOStartNotify
_LVOStartNotify EQU	-888
	XDEF	_LVOEndNotify
_LVOEndNotify EQU	-894
	XDEF	_LVOSetVar
_LVOSetVar EQU	-900
	XDEF	_LVOGetVar
_LVOGetVar EQU	-906
	XDEF	_LVODeleteVar
_LVODeleteVar EQU	-912
	XDEF	_LVOFindVar
_LVOFindVar EQU	-918
	XDEF	_LVOCliInit
_LVOCliInit EQU	-924
	XDEF	_LVOCliInitNewcli
_LVOCliInitNewcli EQU	-930
	XDEF	_LVOCliInitRun
_LVOCliInitRun EQU	-936
	XDEF	_LVOWriteChars
_LVOWriteChars EQU	-942
	XDEF	_LVOPutStr
_LVOPutStr EQU	-948
	XDEF	_LVOVPrintf
_LVOVPrintf EQU	-954
	XDEF	_LVOParsePatternNoCase
_LVOParsePatternNoCase EQU	-966
	XDEF	_LVOMatchPatternNoCase
_LVOMatchPatternNoCase EQU	-972
	XDEF	_LVODosGetString
_LVODosGetString EQU	-978
	XDEF	_LVOSameDevice
_LVOSameDevice EQU	-984
	XDEF	_LVOExAllEnd
_LVOExAllEnd EQU	-990
	XDEF	_LVOSetOwner
_LVOSetOwner EQU	-996
	END
