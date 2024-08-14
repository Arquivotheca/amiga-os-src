*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** exec_lib.asm function offsets
	IDNT	exec_LVO
	SECTION	exec_LVO0
	XDEF	_LVOSupervisor
_LVOSupervisor EQU	-30
	XDEF	_LVOExitIntr
_LVOExitIntr EQU	-36
	XDEF	_LVOSchedule
_LVOSchedule EQU	-42
	XDEF	_LVOReschedule
_LVOReschedule EQU	-48
	XDEF	_LVOSwitch
_LVOSwitch EQU	-54
	XDEF	_LVODispatch
_LVODispatch EQU	-60
	XDEF	_LVOException
_LVOException EQU	-66
	XDEF	_LVOInitCode
_LVOInitCode EQU	-72
	XDEF	_LVOInitStruct
_LVOInitStruct EQU	-78
	XDEF	_LVOMakeLibrary
_LVOMakeLibrary EQU	-84
	XDEF	_LVOMakeFunctions
_LVOMakeFunctions EQU	-90
	XDEF	_LVOFindResident
_LVOFindResident EQU	-96
	XDEF	_LVOInitResident
_LVOInitResident EQU	-102
	XDEF	_LVOAlert
_LVOAlert EQU	-108
	XDEF	_LVODebug
_LVODebug EQU	-114
	XDEF	_LVODisable
_LVODisable EQU	-120
	XDEF	_LVOEnable
_LVOEnable EQU	-126
	XDEF	_LVOForbid
_LVOForbid EQU	-132
	XDEF	_LVOPermit
_LVOPermit EQU	-138
	XDEF	_LVOSetSR
_LVOSetSR EQU	-144
	XDEF	_LVOSuperState
_LVOSuperState EQU	-150
	XDEF	_LVOUserState
_LVOUserState EQU	-156
	XDEF	_LVOSetIntVector
_LVOSetIntVector EQU	-162
	XDEF	_LVOAddIntServer
_LVOAddIntServer EQU	-168
	XDEF	_LVORemIntServer
_LVORemIntServer EQU	-174
	XDEF	_LVOCause
_LVOCause EQU	-180
	XDEF	_LVOAllocate
_LVOAllocate EQU	-186
	XDEF	_LVODeallocate
_LVODeallocate EQU	-192
	XDEF	_LVOAllocMem
_LVOAllocMem EQU	-198
	XDEF	_LVOAllocAbs
_LVOAllocAbs EQU	-204
	XDEF	_LVOFreeMem
_LVOFreeMem EQU	-210
	XDEF	_LVOAvailMem
_LVOAvailMem EQU	-216
	XDEF	_LVOAllocEntry
_LVOAllocEntry EQU	-222
	XDEF	_LVOFreeEntry
_LVOFreeEntry EQU	-228
	XDEF	_LVOInsert
_LVOInsert EQU	-234
	XDEF	_LVOAddHead
_LVOAddHead EQU	-240
	XDEF	_LVOAddTail
_LVOAddTail EQU	-246
	XDEF	_LVORemove
_LVORemove EQU	-252
	XDEF	_LVORemHead
_LVORemHead EQU	-258
	XDEF	_LVORemTail
_LVORemTail EQU	-264
	XDEF	_LVOEnqueue
_LVOEnqueue EQU	-270
	XDEF	_LVOFindName
_LVOFindName EQU	-276
	XDEF	_LVOAddTask
_LVOAddTask EQU	-282
	XDEF	_LVORemTask
_LVORemTask EQU	-288
	XDEF	_LVOFindTask
_LVOFindTask EQU	-294
	XDEF	_LVOSetTaskPri
_LVOSetTaskPri EQU	-300
	XDEF	_LVOSetSignal
_LVOSetSignal EQU	-306
	XDEF	_LVOSetExcept
_LVOSetExcept EQU	-312
	XDEF	_LVOWait
_LVOWait EQU	-318
	XDEF	_LVOSignal
_LVOSignal EQU	-324
	XDEF	_LVOAllocSignal
_LVOAllocSignal EQU	-330
	XDEF	_LVOFreeSignal
_LVOFreeSignal EQU	-336
	XDEF	_LVOAllocTrap
_LVOAllocTrap EQU	-342
	XDEF	_LVOFreeTrap
_LVOFreeTrap EQU	-348
	XDEF	_LVOAddPort
_LVOAddPort EQU	-354
	XDEF	_LVORemPort
_LVORemPort EQU	-360
	XDEF	_LVOPutMsg
_LVOPutMsg EQU	-366
	XDEF	_LVOGetMsg
_LVOGetMsg EQU	-372
	XDEF	_LVOReplyMsg
_LVOReplyMsg EQU	-378
	XDEF	_LVOWaitPort
_LVOWaitPort EQU	-384
	XDEF	_LVOFindPort
_LVOFindPort EQU	-390
	XDEF	_LVOAddLibrary
_LVOAddLibrary EQU	-396
	XDEF	_LVORemLibrary
_LVORemLibrary EQU	-402
	XDEF	_LVOOldOpenLibrary
_LVOOldOpenLibrary EQU	-408
	XDEF	_LVOCloseLibrary
_LVOCloseLibrary EQU	-414
	XDEF	_LVOSetFunction
_LVOSetFunction EQU	-420
	XDEF	_LVOSumLibrary
_LVOSumLibrary EQU	-426
	XDEF	_LVOAddDevice
_LVOAddDevice EQU	-432
	XDEF	_LVORemDevice
_LVORemDevice EQU	-438
	XDEF	_LVOOpenDevice
_LVOOpenDevice EQU	-444
	XDEF	_LVOCloseDevice
_LVOCloseDevice EQU	-450
	XDEF	_LVODoIO
_LVODoIO EQU	-456
	XDEF	_LVOSendIO
_LVOSendIO EQU	-462
	XDEF	_LVOCheckIO
_LVOCheckIO EQU	-468
	XDEF	_LVOWaitIO
_LVOWaitIO EQU	-474
	XDEF	_LVOAbortIO
_LVOAbortIO EQU	-480
	XDEF	_LVOAddResource
_LVOAddResource EQU	-486
	XDEF	_LVORemResource
_LVORemResource EQU	-492
	XDEF	_LVOOpenResource
_LVOOpenResource EQU	-498
	XDEF	_LVORawIOInit
_LVORawIOInit EQU	-504
	XDEF	_LVORawMayGetChar
_LVORawMayGetChar EQU	-510
	XDEF	_LVORawPutChar
_LVORawPutChar EQU	-516
	XDEF	_LVORawDoFmt
_LVORawDoFmt EQU	-522
	XDEF	_LVOGetCC
_LVOGetCC EQU	-528
	XDEF	_LVOTypeOfMem
_LVOTypeOfMem EQU	-534
	XDEF	_LVOProcure
_LVOProcure EQU	-540
	XDEF	_LVOVacate
_LVOVacate EQU	-546
	XDEF	_LVOOpenLibrary
_LVOOpenLibrary EQU	-552
	XDEF	_LVOInitSemaphore
_LVOInitSemaphore EQU	-558
	XDEF	_LVOObtainSemaphore
_LVOObtainSemaphore EQU	-564
	XDEF	_LVOReleaseSemaphore
_LVOReleaseSemaphore EQU	-570
	XDEF	_LVOAttemptSemaphore
_LVOAttemptSemaphore EQU	-576
	XDEF	_LVOObtainSemaphoreList
_LVOObtainSemaphoreList EQU	-582
	XDEF	_LVOReleaseSemaphoreList
_LVOReleaseSemaphoreList EQU	-588
	XDEF	_LVOFindSemaphore
_LVOFindSemaphore EQU	-594
	XDEF	_LVOAddSemaphore
_LVOAddSemaphore EQU	-600
	XDEF	_LVORemSemaphore
_LVORemSemaphore EQU	-606
	XDEF	_LVOSumKickData
_LVOSumKickData EQU	-612
	XDEF	_LVOAddMemList
_LVOAddMemList EQU	-618
	XDEF	_LVOCopyMem
_LVOCopyMem EQU	-624
	XDEF	_LVOCopyMemQuick
_LVOCopyMemQuick EQU	-630
	XDEF	_LVOCacheClearU
_LVOCacheClearU EQU	-636
	XDEF	_LVOCacheClearE
_LVOCacheClearE EQU	-642
	XDEF	_LVOCacheControl
_LVOCacheControl EQU	-648
	XDEF	_LVOCreateIORequest
_LVOCreateIORequest EQU	-654
	XDEF	_LVODeleteIORequest
_LVODeleteIORequest EQU	-660
	XDEF	_LVOCreateMsgPort
_LVOCreateMsgPort EQU	-666
	XDEF	_LVODeleteMsgPort
_LVODeleteMsgPort EQU	-672
	XDEF	_LVOObtainSemaphoreShared
_LVOObtainSemaphoreShared EQU	-678
	XDEF	_LVOAllocVec
_LVOAllocVec EQU	-684
	XDEF	_LVOFreeVec
_LVOFreeVec EQU	-690
	XDEF	_LVOCreatePool
_LVOCreatePool EQU	-696
	XDEF	_LVODeletePool
_LVODeletePool EQU	-702
	XDEF	_LVOAllocPooled
_LVOAllocPooled EQU	-708
	XDEF	_LVOFreePooled
_LVOFreePooled EQU	-714
	XDEF	_LVOAttemptSemaphoreShared
_LVOAttemptSemaphoreShared EQU	-720
	XDEF	_LVOColdReboot
_LVOColdReboot EQU	-726
	XDEF	_LVOStackSwap
_LVOStackSwap EQU	-732
	XDEF	_LVOChildFree
_LVOChildFree EQU	-738
	XDEF	_LVOChildOrphan
_LVOChildOrphan EQU	-744
	XDEF	_LVOChildStatus
_LVOChildStatus EQU	-750
	XDEF	_LVOChildWait
_LVOChildWait EQU	-756
	XDEF	_LVOCachePreDMA
_LVOCachePreDMA EQU	-762
	XDEF	_LVOCachePostDMA
_LVOCachePostDMA EQU	-768
	XDEF	_LVOAddMemHandler
_LVOAddMemHandler EQU	-774
	XDEF	_LVORemMemHandler
_LVORemMemHandler EQU	-780
	XDEF	_LVOObtainQuickVector
_LVOObtainQuickVector EQU	-786
	XDEF	_LVOExecReserved04
_LVOExecReserved04 EQU	-792
	OFFSET	0
exec_FlushLVO0	ds.w	1
	SECTION	exec_LVO1
	XDEF	_LVOExecReserved05
_LVOExecReserved05 EQU	-798
	XDEF	_LVOExecReserved06
_LVOExecReserved06 EQU	-804
	XDEF	_LVOTaggedOpenLibrary
_LVOTaggedOpenLibrary EQU	-810
	XDEF	_LVOReadGayle
_LVOReadGayle EQU	-816
	XDEF	_LVOExecReserved08
_LVOExecReserved08 EQU	-822
	END
