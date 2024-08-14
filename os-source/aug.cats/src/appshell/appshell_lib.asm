* appshell_lib.asm
* AppShell link library
* Copyright (C) 1989,1990,1991 Commodore-Amiga, Inc.
* Written by David N. Junod

_LVOHandleApp		EQU	-30
_LVOHandleAppAsync	EQU	-36
_LVOHandlerFuncA	EQU	-42
_LVOHandlerDataA	EQU	-48
_LVOGetText		EQU	-54
_LVOPrepText		EQU	-60
_LVOParseLine		EQU	-66
_LVOBuildParseLine	EQU	-72
_LVOFreeParseLine	EQU	-78
_LVOFindType		EQU	-84
_LVOMatchValue		EQU	-90
_LVOQStrCmpI		EQU	-96
_LVORemoveMsgPort	EQU	-102
_LVONotifyUser		EQU	-108
_LVOAPSHSetWaitPointer	EQU	-114
_LVOAPSHClearPointer	EQU	-120
_LVOPerfFunc		EQU	-126
_LVOFindNameI		EQU	-132
_LVOAddFuncEntry	EQU	-138
_LVOFreeFuncEntry	EQU	-144
_LVOAddFuncEntries	EQU	-150
_LVOFreeFuncEntries	EQU	-156
_LVOGetFuncEntry	EQU	-162
_LVOGetFuncID		EQU	-168
_LVOGetFuncName		EQU	-174
_LVOGetBaseInfo		EQU	-180
_LVONewProject		EQU	-186
_LVORenumProjects	EQU	-192
_LVORemoveProject	EQU	-198
_LVOAddProjects		EQU	-204
_LVOFreeProject		EQU	-210
_LVOFreeProjects	EQU	-216
_LVOGetProjNode		EQU	-222
_LVOSwapProjNodes	EQU	-228
_LVOIconFromWBArg	EQU	-234
_LVOsetup_arexxA	EQU	-240
_LVOsetup_dosA		EQU	-246
_LVOsetup_idcmpA	EQU	-252
_LVOAPSHGetGadgetInfo	EQU	-258
_LVOAPSHGetWindowInfo	EQU	-264
_LVOsetup_sipcA		EQU	-270
_LVOSendSIPCMessage	EQU	-276
_LVOSendSIPCMessageP	EQU	-282
_LVOOpenSIPC		EQU	-288
_LVOCloseSIPC		EQU	-294
_LVOsetup_toolA		EQU	-300
_LVOsetup_wbA		EQU	-306
_LVOLMatchFirst		EQU	-312
_LVOLMatchNext		EQU	-318
_LVOLMatchEnd		EQU	-324
_LVOLockAppInfo		EQU	-330
_LVOUnlockAppInfo	EQU	-336
_LVOExpandPattern	EQU	-342
_LVOUpdateProject	EQU	-348
_LVOAPSHSignal		EQU	-354
_LVONewAPSHObject	EQU	-360
_LVODisposeAPSHObject	EQU	-366
_LVOSetAPSHAttr		EQU	-372
_LVOGetAPSHAttr		EQU	-378


	xdef	_LVOHandleApp
	xdef	_LVOHandleAppAsync
	xdef	_LVOHandlerFuncA
	xdef	_LVOHandlerDataA
	xdef	_LVOGetText
	xdef	_LVOPrepText
	xdef	_LVOParseLine
	xdef	_LVOBuildParseLine
	xdef	_LVOFreeParseLine
	xdef	_LVOFindType
	xdef	_LVOMatchValue
	xdef	_LVOQStrCmpI
	xdef	_LVORemoveMsgPort
	xdef	_LVONotifyUser
	xdef	_LVOAPSHSetWaitPointer
	xdef	_LVOAPSHClearPointer
	xdef	_LVOPerfFunc
	xdef	_LVOFindNameI
	xdef	_LVOAddFuncEntry
	xdef	_LVOFreeFuncEntry
	xdef	_LVOAddFuncEntries
	xdef	_LVOFreeFuncEntries
	xdef	_LVOGetFuncEntry
	xdef	_LVOGetFuncID
	xdef	_LVOGetFuncName
	xdef	_LVOGetBaseInfo
	xdef	_LVONewProject
	xdef	_LVORenumProjects
	xdef	_LVORemoveProject
	xdef	_LVOAddProjects
	xdef	_LVOFreeProject
	xdef	_LVOFreeProjects
	xdef	_LVOGetProjNode
	xdef	_LVOSwapProjNodes
	xdef	_LVOIconFromWBArg
	xdef	_LVOsetup_arexxA
	xdef	_LVOsetup_dosA
	xdef	_LVOsetup_idcmpA
	xdef	_LVOAPSHGetGadgetInfo
	xdef	_LVOAPSHGetWindowInfo
	xdef	_LVOsetup_sipcA
	xdef	_LVOSendSIPCMessage
	xdef	_LVOSendSIPCMessageP
	xdef	_LVOOpenSIPC
	xdef	_LVOCloseSIPC
	xdef	_LVOsetup_toolA
	xdef	_LVOsetup_wbA
	xdef	_LVOLMatchFirst
	xdef	_LVOLMatchNext
	xdef	_LVOLMatchEnd
	xdef	_LVOLockAppInfo
	xdef	_LVOUnlockAppInfo
	xdef	_LVOExpandPattern
	xdef	_LVOUpdateProject
	xdef	_LVOAPSHSignal
	xdef	_LVONewAPSHObject
	xdef	_LVODisposeAPSHObject
	xdef	_LVOSetAPSHAttr
	xdef	_LVOGetAPSHAttr
	END

	xref	_AppShellBase

	xdef	_GetAPSHAttr
_GetAPSHAttr:
	movem.l	A2/A6,-(sp)
	move.l	12(sp),A0
	movem.l	16(sp),D0/A1/A2
	move.l	_AppShellBase(A4),A6
	jsr	-378(A6)
	movem.l	(sp)+,A2/A6
	RTS

	xdef	_SetAPSHAttr
_SetAPSHAttr:
	movem.l	A2/A6,-(sp)
	movem.l	12(sp),A0/A1/A2
	move.l	_AppShellBase(A4),A6
	jsr	-372(A6)
	movem.l	(sp)+,A2/A6
	RTS

	xdef	_DisposeAPSHObject
_DisposeAPSHObject:
	move.l	A6,-(sp)
	movem.l	8(sp),A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-366(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_NewAPSHObject
_NewAPSHObject:
	move.l	A6,-(sp)
	move.l	8(sp),A0
	movem.l	12(sp),D0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-360(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_APSHSignal
_APSHSignal:
	move.l	A6,-(sp)
	move.l	8(sp),A1
	move.l	12(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-354(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_UpdateProject
_UpdateProject:
	movem.l	A2/A3/A6,-(sp)
	movem.l	16(sp),A1/A2/A3
	move.l	_AppShellBase(A4),A6
	jsr	-348(A6)
	movem.l	(sp)+,A2/A3/A6
	RTS

	xdef	_ExpandPattern
_ExpandPattern:
	movem.l	A2/A3/A6,-(sp)
	movem.l	16(sp),A1/A2/A3
	move.l	_AppShellBase(A4),A6
	jsr	-342(A6)
	movem.l	(sp)+,A2/A3/A6
	RTS

	xdef	_UnlockAppInfo
_UnlockAppInfo:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-336(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_LockAppInfo
_LockAppInfo:
	move.l	A6,-(sp)
	move.l	8(sp),A1
	move.l	_AppShellBase(A4),A6
	jsr	-330(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_LMatchEnd
_LMatchEnd:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-324(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_LMatchNext
_LMatchNext:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-318(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_LMatchFirst
_LMatchFirst:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-312(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_setup_wbA
_setup_wbA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-306(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_setup_toolA
_setup_toolA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-300(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_CloseSIPC
_CloseSIPC:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-294(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_OpenSIPC
_OpenSIPC:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-288(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_SendSIPCMessageP
_SendSIPCMessageP:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-282(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_SendSIPCMessage
_SendSIPCMessage:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-276(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_setup_sipcA
_setup_sipcA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-270(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_APSHGetWindowInfo
_APSHGetWindowInfo:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-264(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_APSHGetGadgetInfo
_APSHGetGadgetInfo:
	movem.l	D2/A6,-(sp)
	movem.l	12(sp),D0/D1/D2/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-258(A6)
	movem.l	(sp)+,D2/A6
	RTS

	xdef	_setup_idcmpA
_setup_idcmpA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-252(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_setup_dosA
_setup_dosA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-246(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_setup_arexxA
_setup_arexxA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-240(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_IconFromWBArg
_IconFromWBArg:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-234(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_SwapProjNodes
_SwapProjNodes:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-228(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_GetProjNode
_GetProjNode:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-222(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FreeProjects
_FreeProjects:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-216(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FreeProject
_FreeProject:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-210(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_AddProjects
_AddProjects:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-204(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_RemoveProject
_RemoveProject:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-198(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_RenumProjects
_RenumProjects:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-192(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_NewProject
_NewProject:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-186(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_GetBaseInfo
_GetBaseInfo:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-180(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_GetFuncName
_GetFuncName:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-174(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_GetFuncID
_GetFuncID:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-168(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_GetFuncEntry
_GetFuncEntry:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-162(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FreeFuncEntries
_FreeFuncEntries:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-156(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_AddFuncEntries
_AddFuncEntries:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-150(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FreeFuncEntry
_FreeFuncEntry:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-144(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_AddFuncEntry
_AddFuncEntry:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-138(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FindNameI
_FindNameI:
	move.l	A6,-(sp)
	movem.l	8(sp),A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-132(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_PerfFunc
_PerfFunc:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-126(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_APSHClearPointer
_APSHClearPointer:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-120(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_APSHSetWaitPointer
_APSHSetWaitPointer:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-114(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_NotifyUser
_NotifyUser:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-108(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_RemoveMsgPort
_RemoveMsgPort:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-102(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_QStrCmpI
_QStrCmpI:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-96(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_MatchValue
_MatchValue:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-90(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FindType
_FindType:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-84(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_FreeParseLine
_FreeParseLine:
	move.l	A6,-(sp)
	move.l	8(sp),D0
	move.l	_AppShellBase(A4),A6
	jsr	-78(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_BuildParseLine
_BuildParseLine:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0
	move.l	_AppShellBase(A4),A6
	jsr	-72(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_ParseLine
_ParseLine:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-66(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_PrepText
_PrepText:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-60(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_GetText
_GetText:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-54(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_HandlerDataA
_HandlerDataA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-48(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_HandlerFuncA
_HandlerFuncA:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-42(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_HandleAppAsync
_HandleAppAsync:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1
	move.l	_AppShellBase(A4),A6
	jsr	-36(A6)
	move.l	(sp)+,A6
	RTS

	xdef	_HandleApp
_HandleApp:
	move.l	A6,-(sp)
	movem.l	8(sp),D0/D1/A0/A1
	move.l	_AppShellBase(A4),A6
	jsr	-30(A6)
	move.l	(sp)+,A6
	RTS

	END
