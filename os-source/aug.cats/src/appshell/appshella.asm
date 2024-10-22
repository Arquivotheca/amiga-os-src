	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE	"appshell_rev.i"	; created by BumpRev

	SECTION text
	XDEF	_LibraryName,_LibraryID,_SysBase

	XREF	_CInit
	XREF	_LibOpen
	XREF	_LibClose
	XREF	_LibExpunge
	XREF	_LibReserved
	XREF	_HandleApp
	XREF	_HandleAppAsync
	XREF	_HandlerFuncA
	XREF	_HandlerDataA
	XREF	_GetText
	XREF	_PrepText
	XREF	_ParseLine
	XREF	_BuildParseLine
	XREF	_FreeParseLine
	XREF	_FindType
	XREF	_MatchValue
	XREF	_QStrCmpI
	XREF	_RemoveMsgPort
	XREF	_NotifyUser
	XREF	_APSHSetWaitPointer
	XREF	_APSHClearPointer
	XREF	_PerfFunc
	XREF	_FindNameI
	XREF	_AddFuncEntry
	XREF	_FreeFuncEntry
	XREF	_AddFuncEntries
	XREF	_FreeFuncEntries
	XREF	_GetFuncEntry
	XREF	_GetFuncID
	XREF	_GetFuncName
	XREF	_GetBaseInfo
	XREF	_NewProject
	XREF	_RenumProjects
	XREF	_RemoveProject
	XREF	_AddProjects
	XREF	_FreeProject
	XREF	_FreeProjects
	XREF	_GetProjNode
	XREF	_SwapProjNodes
	XREF	_IconFromWBArg
	XREF	_setup_arexxA
	XREF	_setup_dosA
	XREF	_setup_idcmpA
	XREF	_APSHGetGadgetInfo
	XREF	_APSHGetWindowInfo
	XREF	_setup_sipcA
	XREF	_SendSIPCMessage
	XREF	_SendSIPCMessageP
	XREF	_OpenSIPC
	XREF	_CloseSIPC
	XREF	_setup_toolA
	XREF	_setup_wbA
	XREF	_LMatchFirst
	XREF	_LMatchNext
	XREF	_LMatchEnd
	XREF	_LockAppInfo
	XREF	_UnlockAppInfo
	XREF	_ExpandPattern
	XREF	_UpdateProject
	XREF	_APSHSignal
	XREF	_NewAPSHObject
	XREF	_DisposeAPSHObject
	XREF	_SetAPSHAttr
	XREF	_GetAPSHAttr

Start:
	moveq	#-1,d0		; don't allow direct execution
	rts

InitDesc:
	; Resident structure
	dc.w	$4AFC		; romtag identifier (RTC_MATCHWORD)
	dc.l	InitDesc	; pointer to romtag identifier
	dc.l	EndCode		; pointer to end of code
	dc.b	0		; resident flags
	dc.b	VERSION		; release version number
	dc.b	9		; type of module (NT_LIBRARY)
	dc.b	0		; initialization priority
	dc.l	_LibraryName	; pointer to node name
	dc.l	_LibraryID	; pointer to ID string
	dc.l	Init		; pointer to init code

_LibraryID
	VSTRING
_LibraryName	dc.b	'appshell.library',0
	ds.w	0
Funcs:
	dc.w	$4EF9
	dc.l	_GetAPSHAttr
	dc.w	$4EF9
	dc.l	_SetAPSHAttr
	dc.w	$4EF9
	dc.l	_DisposeAPSHObject
	dc.w	$4EF9
	dc.l	_NewAPSHObject
	dc.w	$4EF9
	dc.l	_APSHSignal
	dc.w	$4EF9
	dc.l	_UpdateProject
	dc.w	$4EF9
	dc.l	_ExpandPattern
	dc.w	$4EF9
	dc.l	_UnlockAppInfo
	dc.w	$4EF9
	dc.l	_LockAppInfo
	dc.w	$4EF9
	dc.l	ALMatchEnd
	dc.w	$4EF9
	dc.l	ALMatchNext
	dc.w	$4EF9
	dc.l	ALMatchFirst
	dc.w	$4EF9
	dc.l	Asetup_wbA
	dc.w	$4EF9
	dc.l	Asetup_toolA
	dc.w	$4EF9
	dc.l	ACloseSIPC
	dc.w	$4EF9
	dc.l	AOpenSIPC
	dc.w	$4EF9
	dc.l	ASendSIPCMessageP
	dc.w	$4EF9
	dc.l	ASendSIPCMessage
	dc.w	$4EF9
	dc.l	Asetup_sipcA
	dc.w	$4EF9
	dc.l	AAPSHGetWindowInfo
	dc.w	$4EF9
	dc.l	AAPSHGetGadgetInfo
	dc.w	$4EF9
	dc.l	Asetup_idcmpA
	dc.w	$4EF9
	dc.l	Asetup_dosA
	dc.w	$4EF9
	dc.l	Asetup_arexxA
	dc.w	$4EF9
	dc.l	AIconFromWBArg
	dc.w	$4EF9
	dc.l	ASwapProjNodes
	dc.w	$4EF9
	dc.l	AGetProjNode
	dc.w	$4EF9
	dc.l	AFreeProjects
	dc.w	$4EF9
	dc.l	AFreeProject
	dc.w	$4EF9
	dc.l	AAddProjects
	dc.w	$4EF9
	dc.l	ARemoveProject
	dc.w	$4EF9
	dc.l	ARenumProjects
	dc.w	$4EF9
	dc.l	ANewProject
	dc.w	$4EF9
	dc.l	AGetBaseInfo
	dc.w	$4EF9
	dc.l	AGetFuncName
	dc.w	$4EF9
	dc.l	AGetFuncID
	dc.w	$4EF9
	dc.l	AGetFuncEntry
	dc.w	$4EF9
	dc.l	AFreeFuncEntries
	dc.w	$4EF9
	dc.l	AAddFuncEntries
	dc.w	$4EF9
	dc.l	AFreeFuncEntry
	dc.w	$4EF9
	dc.l	AAddFuncEntry
	dc.w	$4EF9
	dc.l	_FindNameI
	dc.w	$4EF9
	dc.l	APerfFunc
	dc.w	$4EF9
	dc.l	AAPSHClearPointer
	dc.w	$4EF9
	dc.l	AAPSHSetWaitPointer
	dc.w	$4EF9
	dc.l	ANotifyUser
	dc.w	$4EF9
	dc.l	ARemoveMsgPort
	dc.w	$4EF9
	dc.l	AQStrCmpI
	dc.w	$4EF9
	dc.l	AMatchValue
	dc.w	$4EF9
	dc.l	AFindType
	dc.w	$4EF9
	dc.l	AFreeParseLine
	dc.w	$4EF9
	dc.l	ABuildParseLine
	dc.w	$4EF9
	dc.l	AParseLine
	dc.w	$4EF9
	dc.l	APrepText
	dc.w	$4EF9
	dc.l	AGetText
	dc.w	$4EF9
	dc.l	AHandlerDataA
	dc.w	$4EF9
	dc.l	AHandlerFuncA
	dc.w	$4EF9
	dc.l	AHandleAppAsync
	dc.w	$4EF9
	dc.l	AHandleApp
	dc.w	$4EF9
	dc.l	ALibReserved
	dc.w	$4EF9
	dc.l	ALibExpunge
	dc.w	$4EF9
	dc.l	ALibClose
	dc.w	$4EF9
	dc.l	ALibOpen

lib:
	; Node structure
	dc.l	0		; next node
	dc.l	0		; previous node
	dc.b	9		; node type (NT_LIBRARY)
	dc.b	0		; priority
	dc.l	_LibraryName	; node name

	; Library structure
	dc.b	6		; flags (CHANGED | SUMUSED | DELEXP)
	dc.b	0		; padding
	dc.w	lib-Funcs	; number of bytes before LIB
	dc.w	_LibBase-lib	; number of bytes after LIB
	dc.w	VERSION		; major version number
	dc.w	REVISION	; minor version number
	dc.l	_LibraryID	; identification
	dc.l	0		; checksum
	dc.w	0		; open count
_seglist
	dc.l	0		; pointer to the SegList
_LibBase
	dc.l	0		; pointer to library base

Init:
	move.l	4,_SysBase	; open ExecBase
	lea	_seglist,a1
	move.l	a0,(a1)
	lea	lib,a1
	move.l	a1,-(sp)
	jsr	_CInit
	addq.l	#4,sp
	rts

ALibOpen:
	move.l	d0,-(sp)
	move.l	a6,-(sp)
	jsr	_LibOpen
	addq.l	#8,sp
	rts

ALibClose:
	move.l	a6,-(sp)
	jsr	_LibClose
	addq.l	#4,sp
	rts

ALibExpunge:
	move.l	a6,-(sp)
	jsr	_LibExpunge
	addq.l	#4,sp
	rts

ALibReserved:
	clr.l	d0
	rts
AHandleApp:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_HandleApp
	add.l	#16,sp
	rts
AHandleAppAsync:
	movem.l	D1-D0,-(sp)
	jsr	_HandleAppAsync
	add.l	#8,sp
	rts
AHandlerFuncA:
	movem.l	D1-D0,-(sp)
	jsr	_HandlerFuncA
	add.l	#8,sp
	rts
AHandlerDataA:
	movem.l	D1-D0,-(sp)
	jsr	_HandlerDataA
	add.l	#8,sp
	rts
AGetText:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_GetText
	add.l	#16,sp
	rts
APrepText:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_PrepText
	add.l	#16,sp
	rts
AParseLine:
	movem.l	D1-D0,-(sp)
	jsr	_ParseLine
	add.l	#8,sp
	rts
ABuildParseLine:
	movem.l	A0/D1-D0,-(sp)
	jsr	_BuildParseLine
	add.l	#12,sp
	rts
AFreeParseLine:
	movem.l	D0,-(sp)
	jsr	_FreeParseLine
	add.l	#4,sp
	rts
AFindType:
	movem.l	A0/D1-D0,-(sp)
	jsr	_FindType
	add.l	#12,sp
	rts
AMatchValue:
	movem.l	D1-D0,-(sp)
	jsr	_MatchValue
	add.l	#8,sp
	rts
AQStrCmpI:
	movem.l	D1-D0,-(sp)
	jsr	_QStrCmpI
	add.l	#8,sp
	rts
ARemoveMsgPort:
	movem.l	D0,-(sp)
	jsr	_RemoveMsgPort
	add.l	#4,sp
	rts
ANotifyUser:
	movem.l	A0/D1-D0,-(sp)
	jsr	_NotifyUser
	add.l	#12,sp
	rts
AAPSHSetWaitPointer:
	movem.l	D1-D0,-(sp)
	jsr	_APSHSetWaitPointer
	add.l	#8,sp
	rts
AAPSHClearPointer:
	movem.l	D1-D0,-(sp)
	jsr	_APSHClearPointer
	add.l	#8,sp
	rts
APerfFunc:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_PerfFunc
	add.l	#16,sp
	rts
AFindNameI:
;	movem.l	D1-D0,-(sp)
	move.l	d0,a0
	move.l	d1,a1
	jsr	_FindNameI
;	add.l	#8,sp
	rts
AAddFuncEntry:
	movem.l	D1-D0,-(sp)
	jsr	_AddFuncEntry
	add.l	#8,sp
	rts
AFreeFuncEntry:
	movem.l	D1-D0,-(sp)
	jsr	_FreeFuncEntry
	add.l	#8,sp
	rts
AAddFuncEntries:
	movem.l	D1-D0,-(sp)
	jsr	_AddFuncEntries
	add.l	#8,sp
	rts
AFreeFuncEntries:
	movem.l	D0,-(sp)
	jsr	_FreeFuncEntries
	add.l	#4,sp
	rts
AGetFuncEntry:
	movem.l	A0/D1-D0,-(sp)
	jsr	_GetFuncEntry
	add.l	#12,sp
	rts
AGetFuncID:
	movem.l	D1-D0,-(sp)
	jsr	_GetFuncID
	add.l	#8,sp
	rts
AGetFuncName:
	movem.l	D1-D0,-(sp)
	jsr	_GetFuncName
	add.l	#8,sp
	rts
AGetBaseInfo:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_GetBaseInfo
	add.l	#16,sp
	rts
ANewProject:
	movem.l	A0/D1-D0,-(sp)
	jsr	_NewProject
	add.l	#12,sp
	rts
ARenumProjects:
	movem.l	A0/D1-D0,-(sp)
	jsr	_RenumProjects
	add.l	#12,sp
	rts
ARemoveProject:
	movem.l	A0/D1-D0,-(sp)
	jsr	_RemoveProject
	add.l	#12,sp
	rts
AAddProjects:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_AddProjects
	add.l	#16,sp
	rts
AFreeProject:
	movem.l	D0,-(sp)
	jsr	_FreeProject
	add.l	#4,sp
	rts
AFreeProjects:
	movem.l	D1-D0,-(sp)
	jsr	_FreeProjects
	add.l	#8,sp
	rts
AGetProjNode:
	movem.l	A0/D1-D0,-(sp)
	jsr	_GetProjNode
	add.l	#12,sp
	rts
ASwapProjNodes:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_SwapProjNodes
	add.l	#16,sp
	rts
AIconFromWBArg:
	movem.l	D0,-(sp)
	jsr	_IconFromWBArg
	add.l	#4,sp
	rts
Asetup_arexxA:
	movem.l	D1-D0,-(sp)
	jsr	_setup_arexxA
	add.l	#8,sp
	rts
Asetup_dosA:
	movem.l	D1-D0,-(sp)
	jsr	_setup_dosA
	add.l	#8,sp
	rts
Asetup_idcmpA:
	movem.l	D1-D0,-(sp)
	jsr	_setup_idcmpA
	add.l	#8,sp
	rts
AAPSHGetGadgetInfo:
	movem.l D2,-(sp)
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_APSHGetGadgetInfo
	add.l	#20,sp
	rts
AAPSHGetWindowInfo:
	movem.l	A0/D1-D0,-(sp)
	jsr	_APSHGetWindowInfo
	add.l	#12,sp
	rts
Asetup_sipcA:
	movem.l	D1-D0,-(sp)
	jsr	_setup_sipcA
	add.l	#8,sp
	rts
ASendSIPCMessage:
	movem.l	A0/D1-D0,-(sp)
	jsr	_SendSIPCMessage
	add.l	#12,sp
	rts
ASendSIPCMessageP:
	movem.l	A1-A0/D1-D0,-(sp)
	jsr	_SendSIPCMessageP
	add.l	#16,sp
	rts
AOpenSIPC:
	movem.l	A0/D1-D0,-(sp)
	jsr	_OpenSIPC
	add.l	#12,sp
	rts
ACloseSIPC:
	movem.l	D1-D0,-(sp)
	jsr	_CloseSIPC
	add.l	#8,sp
	rts
Asetup_toolA:
	movem.l	D1-D0,-(sp)
	jsr	_setup_toolA
	add.l	#8,sp
	rts
Asetup_wbA:
	movem.l	D1-D0,-(sp)
	jsr	_setup_wbA
	add.l	#8,sp
	rts
ALMatchFirst:
	movem.l	A0/D1-D0,-(sp)
	jsr	_LMatchFirst
	add.l	#12,sp
	rts
ALMatchNext:
	movem.l	D0,-(sp)
	jsr	_LMatchNext
	add.l	#4,sp
	rts
ALMatchEnd:
	movem.l	D0,-(sp)
	jsr	_LMatchEnd
	add.l	#4,sp
	rts

EndCode:
	xdef	__oserr,__OSERR,__FPERR,__SIGFPE
	xdef	__SIGINT,__ONERR,__ONEXIT,__ONBREAK,__ECS
	xdef	__ProgramName

_SysBase	dc.l	0
__oserr:
__OSERR	dc.l	0
__FPERR	dc.l	0
__SIGFPE	dc.l	0
__SIGINT	dc.l	0
__ONERR	dc.l	0
__ONEXIT	dc.l	0
__ONBREAK	dc.l	0
__ECS		dc.l	0
__ProgramName	dc.l	0
	END
