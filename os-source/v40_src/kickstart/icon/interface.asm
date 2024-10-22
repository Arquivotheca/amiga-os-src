*************************************************************************
*
* $Id: interface.asm,v 38.5 93/02/15 16:30:53 mks Exp $
*
* $Log:	interface.asm,v $
* Revision 38.5  93/02/15  16:30:53  mks
* Removed the alert call
* 
* Revision 38.4  92/07/28  13:48:18  mks
* Added the stubs to be FRead/FWrite rather than Read/Write
*
* Revision 38.3  92/04/20  11:13:47  mks
* Changed to use TaggedOpenLibrary() and downcoded the icon2wb.c routine.
*
* Revision 38.2  92/02/26  09:41:07  mks
* Added two stubs and changed to use small function table (words)
*
* Revision 38.1  91/06/24  19:01:45  mks
* Changed to V38 source tree - Trimmed Log
*
*************************************************************************

	SECTION	section

;****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE 'libraries/dos.i'

	INCLUDE	'internal/librarytags.i'

	INCLUDE	'asmsupp.i'
	INCLUDE	'messages.i'
	INCLUDE	'internal.i'


;****** Imported Names ***********************************************

*------ Data ---------------------------------------------------------

	XREF	subsysName
	XREF	VERNUM
	XREF	REVNUM
	XREF	VERSTRING

*------ Functions ----------------------------------------------------

	XREF	_IGetWBObject
	XREF	_IPutWBObject
	XREF	_IGetIcon
	XREF	_IPutIcon
	XREF	_IFreeFreeList
	XREF	_IFreeWBObject
	XREF	_IAllocWBObject
	XREF	_IAddFreeList
	XREF	_IGetDiskObject
	XREF	_IPutDiskObject
	XREF	_IFreeDiskObject
	XREF	_IFindToolType
	XREF	_IMatchToolValue
	XREF	_IBumpRevision
	XREF	_IFreeAlloc
	XREF	_IGetDefDiskObject
	XREF	_IPutDefDiskObject
	XREF	_IGetDiskObjectNew
	XREF	_IDeleteDiskObject

*------ Offsets ------------------------------------------------------

	XLIB	AddLibrary
	XLIB	Alert
	XLIB	FreeMem
	XLIB	MakeLibrary
	XLIB	TaggedOpenLibrary
	XLIB	CloseLibrary

;****** Exported Names ***********************************************

*------ Data ---------------------------------------------------------

	XDEF	Init
	XDEF	_SysBaseOffset
	XDEF	_DOSBaseOffset
	XDEF	_WorkbenchBaseOffset

_SysBaseOffset		EQU	il_SysBase
_DOSBaseOffset		EQU	il_DOSBase
_WorkbenchBaseOffset	EQU	il_WorkbenchBase

;****** Local Definitions ********************************************


Init:
	DC.L	il_Sizeof
	DC.L	funcTable
	DC.L	dataTable
	DC.L	initRoutine

*
* Build the function table (short size)
*
V_DEF		MACRO
		dc.W	\1+(*-funcTable)
		ENDM

funcTable:
	dc.w	-1
	V_DEF	libOpen
	V_DEF	libClose
	V_DEF	libExpunge
	V_DEF	libNull
	V_DEF	libGetWBObject
	V_DEF	libPutWBObject
	V_DEF	libGetIcon
	V_DEF	libPutIcon
	V_DEF	libFreeFreeList
	V_DEF	libFreeWBObject
	V_DEF	libAllocWBObject
	V_DEF	libAddFreeList
	V_DEF	libGetDiskObject
	V_DEF	libPutDiskObject
	V_DEF	libFreeDiskObject
	V_DEF	libFindToolType
	V_DEF	libMatchToolValue
	V_DEF	libBumpRevision
	V_DEF	libFreeAlloc
	V_DEF	libGetDefDiskObject
	V_DEF	libPutDefDiskObject
	V_DEF	libGetDiskObjectNew
	V_DEF	libDeleteDiskObject
	V_DEF	libIconReserved00
	V_DEF	libIconReserved01
	V_DEF	libIconReserved02
	V_DEF	libIconReserved03
	dc.w	-1

dataTable:
	INITBYTE	LN_TYPE,NT_LIBRARY
	INITLONG	LN_NAME,subsysName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERNUM
	INITWORD	LIB_REVISION,REVNUM
	INITLONG	LIB_IDSTRING,VERSTRING
	DC.L		0

libOpen:	ADDQ.W	#1,LIB_OPENCNT(A6)
		BCLR	#LIBB_DELEXP,LIB_FLAGS(A6)
		MOVE.L	A6,D0
		RTS

libClose:
		;------ set up our return value
		MOVEQ	#0,D0

		SUBQ.W	#1,LIB_OPENCNT(A6)
		BNE.S	Close_End

		BTST	#LIBB_DELEXP,LIB_FLAGS(A6)
		BEQ.S	Close_End

; --- Fall into the expunge routine...

libExpunge:
		TST.W	LIB_OPENCNT(A6)
		BNE.S	Expunge_NotNow

		;------ save our segment list
		MOVE.L	il_SegList(A6),-(SP)

		;------ pull ourselves off of the library list
		MOVE.L	A6,A1
		REMOVE

		;------ take our memory away...
		MOVE.L	A6,A1
		CLEAR	D0
		CLEAR	D1
		MOVE.W	LIB_NEGSIZE(A6),D0
		SUB.L	D0,A1
		MOVE.W	LIB_POSSIZE(A6),D1
		ADD.L	D1,D0

		LINKSYS	FreeMem

		;------ get back our return value
		MOVE.L	(SP)+,D0
		BRA.S	Expunge_End

Expunge_NotNow:
		BSET	#LIBB_DELEXP,LIB_FLAGS(A6)
		CLEAR	D0

Expunge_End:
Close_End:
		RTS


initRoutine:
		MOVE.L	A2,-(SP)
		MOVE.L	D0,A2

		MOVE.L	A0,il_SegList(A2)
		MOVE.L	A6,il_SysBase(A2)

		moveq.l	#OLTAG_DOS,d0	; Get DOS library tag...
		CALLSYS	TaggedOpenLibrary

		MOVE.L	D0,il_DOSBase(A2)
		BNE.S	init_OKDos

	;Never	;------ failed???
	;Never	ALERT	AN_IconLib!AG_OpenLib!AO_DOSLib
	;Never	CLEAR	D0
		BRA.S	init_end
init_OKDos:

		MOVE.L	A2,D0
init_end:
		MOVE.L	(SP)+,A2
		RTS

;GetWBObject(name)(A0)
libGetWBObject:
		MOVE.L	A0,-(SP)
		JSR	_IGetWBObject
		ADDQ.L	#4,SP
		RTS

;PutWBObject(name,object)(A0/A1)
libPutWBObject:
		MOVEM.L	A0/A1,-(SP)
		JSR	_IPutWBObject
		addq.l	#8,sp
		RTS

;* Stub for internal GetIcon calls...
		XREF	_LVOGetIcon
		XDEF	_GetIconStub
_GetIconStub:
		move.l	4(sp),a0
		move.l	8(sp),a1
		move.l	a2,-(sp)
		move.l	16(sp),a2
		jsr	_LVOGetIcon(a6)
		move.l	(sp)+,a2
		rts

;GetIcon(name,icon,freelist)(A0/A1/A2)
libGetIcon:
		MOVEM.L	A0/A1/A2,-(SP)
		JSR	_IGetIcon
		LEA	12(SP),SP
		RTS


;PutDiskObject(name,icon)(A0/A1)
libPutDiskObject:
		MOVEM.L	A0/A1,-(SP)
		JSR	_IPutDiskObject
		addq.l	#8,sp
		RTS

;* Stub for internal PutIcon calls...
		XREF	_LVOPutIcon
		XDEF	_PutIconStub
_PutIconStub:
		move.l	4(sp),a0
		move.l	8(sp),a1
		jmp	_LVOPutIcon(a6)

;PutIcon(name,icon)(A0/A1)
libPutIcon:
		MOVEM.L	A0/A1,-(SP)
		JSR	_IPutIcon
		addq.l	#8,sp
		RTS

;* Stub for internal FreeFreeList calls...
		XREF	_LVOFreeFreeList
		XDEF	_FreeFreeListStub
_FreeFreeListStub:
		move.l	4(sp),a0
		jmp	_LVOFreeFreeList(a6)

;FreeFreeList(freelist)(A0)
libFreeFreeList:
		MOVE.L	A0,-(SP)
		JSR	_IFreeFreeList
		ADDQ.L	#4,SP
		RTS

;* Stub for internal FreeWBObject calls...
_LVOFreeWBObject EQU	-60			; From SFD
		XDEF	_FreeWBObjectStub
_FreeWBObjectStub:
		move.l	4(sp),a0
		jmp	_LVOFreeWBObject(a6)

;FreeWBObject(object)(A0)
libFreeWBObject:
		MOVE.L	A0,-(SP)
		JSR	_IFreeWBObject
		ADDQ.L	#4,SP
		RTS

;* Stub for internal AllocWBObject calls...
_LVOAllocWBObject EQU	-66			; From SFD
		XDEF	_AllocWBObjectStub
_AllocWBObjectStub:
		jmp	_LVOAllocWBObject(a6)

;AllocWBObject()()
libAllocWBObject:
		JMP	_IAllocWBObject

;* Stub for internal AddFreeList calls...
		XREF	_LVOAddFreeList
		XDEF	_AddFreeListStub
_AddFreeListStub:
		move.l	4(sp),a0
		move.l	8(sp),a1
		move.l	a2,-(sp)
		move.l	16(sp),a2
		jsr	_LVOAddFreeList(a6)
		move.l	(sp)+,a2
		rts

;AddFreeList(free,mem,size)(A0/A1/A2)
libAddFreeList:
		MOVEM.L	A0/A1/A2,-(SP)
		JSR	_IAddFreeList
		LEA	12(SP),SP
		RTS

;* Stub for internal GetDiskObject calls...
		XREF	_LVOGetDiskObject
		XDEF	_GetDiskObjectStub
_GetDiskObjectStub:
		move.l	4(sp),a0
		jmp	_LVOGetDiskObject(a6)

;GetDiskObject(object)(A0)
libGetDiskObject:
		MOVE.L	A0,-(SP)
		JSR	_IGetDiskObject
		ADDQ.L	#4,SP
		RTS

;* Stub for internal FreeDiskObject calls...
		XREF	_LVOFreeDiskObject
		XDEF	_FreeDiskObjectStub
_FreeDiskObjectStub:
		move.l	4(sp),a0
		jmp	_LVOFreeDiskObject(a6)

;FreeDiskObject(object)(A0)
libFreeDiskObject:
		MOVE.L	A0,-(SP)
		JSR	_IFreeDiskObject
		ADDQ.L	#4,SP
		RTS

;DeleteDiskObject(object)(A0)
libDeleteDiskObject:
		MOVE.L	A0,-(SP)
		JSR	_IDeleteDiskObject
		ADDQ.L	#4,SP
		RTS

;FindToolType(toolTypeArray,typeName)(A0/A1)
libFindToolType:
		MOVEM.L	A0/A1,-(SP)
		JSR	_IFindToolType
		ADDQ.L	#8,SP
		RTS

;MatchToolValue(typeString,value)(A0/A1)
libMatchToolValue:
		MOVEM.L	A0/A1,-(SP)
		JSR	_IMatchToolValue
		ADDQ.L	#8,SP
		RTS

;BumpRevision(newstring,oldstring)(A0/A1)
libBumpRevision:
		MOVEM.L	A0/A1,-(SP)
		JSR	_IBumpRevision
		ADDQ.L	#8,SP
		RTS

;FreeAlloc(free,len,type)(A0/A1/A2)
libFreeAlloc:
		MOVEM.L	A0/A1/A2,-(SP)
		JSR	_IFreeAlloc
		LEA	12(SP),SP
		RTS

;* Stub for internal GetDiskObject calls...
		XREF	_LVOGetDefDiskObject
		XDEF	_GetDefDiskObjectStub
_GetDefDiskObjectStub:
		move.l	4(sp),d0
		jmp	_LVOGetDefDiskObject(a6)

;GetDefDiskObject(type)(D0)
libGetDefDiskObject:
		MOVE.L	D0,-(SP)
		JSR	_IGetDefDiskObject
		ADDQ.L	#4,SP
		RTS

;PutDefDiskObject(struct DiskObject *)(A0)
libPutDefDiskObject:
		MOVE.L	A0,-(SP)
		JSR	_IPutDefDiskObject
		ADDQ.L	#4,SP
		RTS

;GetDiskObjectNew(object)(A0)
libGetDiskObjectNew:
		MOVE.L	A0,-(SP)
		JSR	_IGetDiskObjectNew
		ADDQ.L	#4,SP
		RTS

;IconReserved00() thru IconReserved03()
libIconReserved00:
libIconReserved01:
libIconReserved02:
libIconReserved03:
libNull:	MOVEQ.L	#0,D0
		RTS

*******************************************************************************
*
* Stub routines to save code in workbench...
*
*******************************************************************************
*
* AllocMemStub
*
		XREF	_LVOAllocMem
		XDEF	_AllocMemStub
_AllocMemStub:	move.l	4(sp),d0		; Get the arguments from
		move.l	8(sp),d1		; the stack...
		move.l	a6,-(sp)		; Save a6
		move.l	il_SysBase(a6),a6	; Get execbase
		jsr	_LVOAllocMem(a6)	; AllocMem
		move.l	(sp)+,a6		; Restore a6
		rts				; And we are done.
*
*******************************************************************************
*
* FreeMemStub
*
		XREF	_LVOFreeMem
		XDEF	_FreeMemStub
_FreeMemStub:	move.l	4(sp),a1		; Get the arguments from
		move.l	8(sp),d0		; the stack...
		move.l	a6,-(sp)		; Save a6
		move.l	il_SysBase(a6),a6	; Get execbase
		jsr	_LVOFreeMem(a6)		; FreeMem
		move.l	(sp)+,a6		; Restore a6
		rts				; And we are done.
*
*******************************************************************************
*
* AllocVecStub
*
		XREF	_LVOAllocVec
		XDEF	_AllocVecStub
_AllocVecStub:	move.l	4(sp),d0		; Get the arguments from
		move.l	8(sp),d1		; the stack...
		move.l	a6,-(sp)		; Save a6
		move.l	il_SysBase(a6),a6	; Get execbase
		jsr	_LVOAllocVec(a6)	; AllocVec
		move.l	(sp)+,a6		; Restore a6
		rts				; And we are done.
*
*******************************************************************************
*
* FreeVecStub
*
		XREF	_LVOFreeVec
		XDEF	_FreeVecStub
_FreeVecStub:	move.l	4(sp),a1		; Get the arguments from
		move.l	a6,-(sp)		; Save a6
		move.l	il_SysBase(a6),a6	; Get execbase
		jsr	_LVOFreeVec(a6)		; FreeVec
		move.l	(sp)+,a6		; Restore a6
		rts				; And we are done.
*
*******************************************************************************
*
* Now for the DOS stubs...
*
*******************************************************************************
*
* ParentDitStub
*
		XREF	_LVOParentDir
		XDEF	_ParentDirStub
_ParentDirStub:	move.w	#_LVOParentDir,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* UnLockStub
*
		XREF	_LVOUnLock
		XDEF	_UnLockStub
_UnLockStub:	move.w	#_LVOUnLock,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* DeleteFileStub
*
		XREF	_LVODeleteFile
		XDEF	_DeleteFileStub
_DeleteFileStub:
		move.w	#_LVODeleteFile,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* CloseStub
*
		XREF	_LVOClose
		XDEF	_CloseStub
_CloseStub:	move.w	#_LVOClose,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* LockStub
*
		XREF	_LVOLock
		XDEF	_LockStub
_LockStub:	move.w	#_LVOLock,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* OpenStub
*
		XREF	_LVOOpen
		XDEF	_OpenStub
_OpenStub:	move.w	#_LVOOpen,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* FWriteStub
*
		XREF	_LVOFWrite
		XDEF	_FWriteStub
_FWriteStub:	move.w	#_LVOFWrite,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* FReadStub
*
		XREF	_LVOFRead
		XDEF	_FReadStub
_FReadStub:	move.w	#_LVOFRead,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* SetVBufStub
*
		XREF	_LVOSetVBuf
		XDEF	_SetVBufStub
_SetVBufStub:	move.w	#_LVOSetVBuf,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* SetProtectionStub
*
		XREF	_LVOSetProtection
		XDEF	_SetProtectionStub
_SetProtectionStub:
		move.w	#_LVOSetProtection,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* IoErrStub
*
		XREF	_LVOIoErr
		XDEF	_IoErrStub
_IoErrStub:	move.w	#_LVOIoErr,d0		; Get offset
;		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* DosStubArg -	Common stub that uses d0 as the offset and does up to 4
*		argument DOS call...
*
DosStubArg:	movem.l	d2/d3/d4/a6,-(sp)	; Save d2, d3, d4 and a6
		movem.l	20(sp),d1-d4		; Get args...
		move.l	il_DOSBase(a6),a6	; Get DOSBase
		jsr	0(a6,d0.w)		; Call the routine
		movem.l	(sp)+,d2/d3/d4/a6	; Restore all...
		rts
*
*******************************************************************************
*
* FilePartStub
*
		XREF	_LVOFilePart
		XDEF	_FilePartStub
_FilePartStub:	move.w	#_LVOFilePart,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* PathPartStub
*
		XREF	_LVOPathPart
		XDEF	_PathPartStub
_PathPartStub:	move.w	#_LVOPathPart,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* UpdateWorkbenchStub
*
		XDEF	_MyUpdateWorkbench
_MyUpdateWorkbench:
		move.l	4(sp),d0		; Get name...
		move.l	d0,-(sp)		; Save it...
		bsr.s	_FilePartStub		; Get FilePart...
		move.l	d0,(sp)			; Save it again...
		move.l	a6,-(sp)		; Save a6...
		moveq.l	#OLTAG_WORKBENCH,d0	; Get Workbench tag...
		move.l	il_SysBase(a6),a6	; Get execbase...
		CALLSYS	TaggedOpenLibrary	; Try to open workbench...
		move.l	d0,a6			; Get workbench base...
		tst.l	d0			; Did it open?
		beq.s	uw_Exit			; Exit now if NULL...
		move.l	4(sp),a0		; Get file name
		move.l	16(sp),a1		; Get lock
		move.l	20(sp),d0		; Get flag
		jsr	-30(a6)			; UpdateWorkbench (private)
		move.l	a6,a1			; Get ready for CloseLibrary
		move.l	(sp),a6			; Get IconBase...
		move.l	il_SysBase(a6),a6	; Get ExecBase...
		CALLSYS	CloseLibrary		; Close Workbench...
uw_Exit:	move.l	(sp)+,a6		; Restore icon base...
		addq.l	#4,sp			; clean up stack...
		rts				; and we are done...
*
*******************************************************************************
		END
