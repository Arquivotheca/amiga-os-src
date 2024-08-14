	SECTION text,code
	XREF	_AppShellBase
	XDEF	_HandleApp
	XDEF	_HandleAppAsync
	XDEF	_HandlerFuncA
	XDEF	_HandlerDataA
	XDEF	_GetText
	XDEF	_PrepText
	XDEF	_ParseLine
	XDEF	_BuildParseLine
	XDEF	_FreeParseLine
	XDEF	_FindType
	XDEF	_MatchValue
	XDEF	_QStrCmpI
	XDEF	_RemoveMsgPort
	XDEF	_NotifyUser
	XDEF	_APSHSetWaitPointer
	XDEF	_APSHClearPointer
	XDEF	_PerfFunc
	XDEF	_FindNameI
	XDEF	_AddFuncEntry
	XDEF	_FreeFuncEntry
	XDEF	_AddFuncEntries
	XDEF	_FreeFuncEntries
	XDEF	_GetFuncEntry
	XDEF	_GetFuncID
	XDEF	_GetFuncName
	XDEF	_GetBaseInfo
	XDEF	_NewProject
	XDEF	_RenumProjects
	XDEF	_RemoveProject
	XDEF	_AddProjects
	XDEF	_FreeProject
	XDEF	_FreeProjects
	XDEF	_GetProjNode
	XDEF	_SwapProjNodes
	XDEF	_IconFromWBArg
	XDEF	_setup_arexxA
	XDEF	_setup_dosA
	XDEF	_setup_idcmpA
	XDEF	_APSHGetGadgetInfo
	XDEF	_APSHGetWindowInfo
	XDEF	_setup_sipcA
	XDEF	_SendSIPCMessage
	XDEF	_SendSIPCMessageP
	XDEF	_OpenSIPC
	XDEF	_CloseSIPC
	XDEF	_setup_toolA
	XDEF	_setup_wbA
_HandleApp:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0-D1/A0-A1
	jsr	-30(a4)
	move.l	(sp)+,A4
	rts
_HandleAppAsync:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-36(a4)
	move.l	(sp)+,A4
	rts

_HandlerFuncA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-42(a4)
	move.l	(sp)+,A4
	rts
_HandlerDataA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-48(a4)
	move.l	(sp)+,A4
	rts
_GetText:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-54(a4)
	move.l	(sp)+,A4
	rts
_PrepText:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-60(a4)
	move.l	(sp)+,A4
	rts
_ParseLine:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-66(a4)
	move.l	(sp)+,A4
	rts
_BuildParseLine:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-72(a4)
	move.l	(sp)+,A4
	rts
_FreeParseLine:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	move.l 8(sp),D0
	jsr	-78(a4)
	move.l	(sp)+,A4
	rts
_FindType:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-84(a4)
	move.l	(sp)+,A4
	rts
_MatchValue:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-90(a4)
	move.l	(sp)+,A4
	rts
_QStrCmpI:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-96(a4)
	move.l	(sp)+,A4
	rts
_RemoveMsgPort:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	move.l 8(sp),D0
	jsr	-102(a4)
	move.l	(sp)+,A4
	rts
_NotifyUser:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-108(a4)
	move.l	(sp)+,A4
	rts
_APSHSetWaitPointer:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-114(a4)
	move.l	(sp)+,A4
	rts
_APSHClearPointer:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-120(a4)
	move.l	(sp)+,A4
	rts
_PerfFunc:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-126(a4)
	move.l	(sp)+,A4
	rts
_FindNameI:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),A0/A1
	jsr	-132(a4)
	move.l	(sp)+,A4
	rts
_AddFuncEntry:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-138(a4)
	move.l	(sp)+,A4
	rts
_FreeFuncEntry:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-144(a4)
	move.l	(sp)+,A4
	rts
_AddFuncEntries:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-150(a4)
	move.l	(sp)+,A4
	rts
_FreeFuncEntries:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	move.l 8(sp),D0
	jsr	-156(a4)
	move.l	(sp)+,A4
	rts
_GetFuncEntry:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-162(a4)
	move.l	(sp)+,A4
	rts
_GetFuncID:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-168(a4)
	move.l	(sp)+,A4
	rts
_GetFuncName:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-174(a4)
	move.l	(sp)+,A4
	rts
_GetBaseInfo:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-180(a4)
	move.l	(sp)+,A4
	rts
_NewProject:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-186(a4)
	move.l	(sp)+,A4
	rts
_RenumProjects:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-192(a4)
	move.l	(sp)+,A4
	rts
_RemoveProject:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-198(a4)
	move.l	(sp)+,A4
	rts
_AddProjects:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-204(a4)
	move.l	(sp)+,A4
	rts
_FreeProject:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	move.l 8(sp),D0
	jsr	-210(a4)
	move.l	(sp)+,A4
	rts
_FreeProjects:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-216(a4)
	move.l	(sp)+,A4
	rts
_GetProjNode:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-222(a4)
	move.l	(sp)+,A4
	rts
_SwapProjNodes:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-228(a4)
	move.l	(sp)+,A4
	rts
_IconFromWBArg:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	move.l 8(sp),D0
	jsr	-234(a4)
	move.l	(sp)+,A4
	rts
_setup_arexxA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-240(a4)
	move.l	(sp)+,A4
	rts
_setup_dosA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-246(a4)
	move.l	(sp)+,A4
	rts
_setup_idcmpA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-252(a4)
	move.l	(sp)+,A4
	rts
_APSHGetGadgetInfo:
	movem.l D2/A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 12(sp),D0/D1/A0/A1
	move.l	28(sp),D2
	jsr	-258(a4)
	movem.l (sp)+,D2/A4
	rts
_APSHGetWindowInfo:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-264(a4)
	move.l	(sp)+,A4
	rts
_setup_sipcA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-270(a4)
	move.l	(sp)+,A4
	rts
_SendSIPCMessage:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0
	jsr	-276(a4)
	move.l	(sp)+,A4
	rts
_SendSIPCMessageP:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1/A0/A1
	jsr	-282(a4)
	move.l	(sp)+,A4
	rts
_OpenSIPC:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	move.l 12(sp),A0
	jsr	-288(a4)
	move.l	(sp)+,A4
	rts
_CloseSIPC:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-294(a4)
	move.l	(sp)+,A4
	rts
_setup_toolA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-300(a4)
	move.l	(sp)+,A4
	rts
_setup_wbA:
	move.l	A4,-(sp)
	move.l	_AppShellBase,a4
	movem.l 8(sp),D0/D1
	jsr	-306(a4)
	move.l	(sp)+,A4
	rts

	END

