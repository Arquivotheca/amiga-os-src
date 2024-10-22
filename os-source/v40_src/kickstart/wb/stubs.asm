*************************************************************************
*
* $Id: stubs.asm,v 39.1 93/01/11 16:54:25 mks Exp $
*
* $Log:	stubs.asm,v $
* Revision 39.1  93/01/11  16:54:25  mks
* Now supports a string filter for the RENAME requester...
* No longer needs to check for ":" or "/" after the fact.
* 
* Revision 38.11  92/06/11  15:47:52  mks
* Added forbid and permit stubs
*
* Revision 38.10  92/05/31  00:58:49  mks
* Fixed the ThisTask() stub
*
* Revision 38.9  92/05/30  18:03:51  mks
* Added ThisTask() stub...
*
* Revision 38.8  92/05/30  16:36:07  mks
* Added a few stubs and cleaned up the order
*
* Revision 38.7  92/05/30  16:34:13  mks
* Added two new stubs
*
* Revision 38.6  92/05/14  18:04:15  mks
* Some new stubs to make life much better... (and smaller)
*
* Revision 38.5  92/04/14  12:13:03  mks
* Added the LockRamDisk routine (removed from global.c
*
* Revision 38.4  92/03/10  11:32:54  mks
* Added NameFromLockStub
*
* Revision 38.3  92/01/07  13:54:43  mks
* Removed the Max_Copy_Mem from here and it is now in abs.asm
*
* Revision 38.2  91/11/12  18:46:10  mks
* Added stub for the setfiledate so that copy can now preserve dates
*
* Revision 38.1  91/06/24  11:38:40  mks
* Initial V38 tree checkin - Log file stripped
*
*************************************************************************

		SECTION	code

		INCLUDE 'exec/types.i'
		INCLUDE	'exec/execbase.i'
		INCLUDE	'exec/memory.i'
		INCLUDE	'workbench.i'
		INCLUDE	'workbenchbase.i'
		INCLUDE	'utility/hooks.i'

*******************************************************************************
*
* Stub routines to save code in workbench...
*
*******************************************************************************
*
* A pair of routine that will save code in workbench by doing the
* AllocMyMsg and FreeMyMsg work here...
*
*******************************************************************************
*
* AllocMyMsg - Used in copy.c to allocate a copy message and count the bytes...
*
* This is the minimum size of the MEMF_LARGEST area of CHIP memory
* that must be available after the allocation.
*
Min_Chip_Free	EQU	30000			; Minimum free CHIP memory
*
* Now for the code to AllocMyMsg
*
		XREF	_LVOAvailMem
		XDEF	_AllocMyMsg
_AllocMyMsg:	tst.l	wb_ByteCount(a6)	; Check the byte count...
		bmi.s	NoAllocMsg		; If we used too much...
*
* Try to allocate from non-CHIP memory
*
		move.l	4(sp),d0		; Get the size...
		move.l	#MEMF_FAST!MEMF_CLEAR!MEMF_PUBLIC,-(sp)
		move.l	d0,-(sp)		; Get it onto the stack...
		bsr	_AllocVecStub		; Try the allocation
		addq.l	#8,sp			; Take back the stack...
		tst.l	d0			; Check if it worked
		bne.s	YesAllocMsg		; We allocated it
*
* Now, we check what the largest memory is...
*
		move.l	#MEMF_CHIP!MEMF_PUBLIC!MEMF_LARGEST,d1
		move.l	a6,-(sp)		; Save a6...
		move.l	wb_SysBase(a6),a6	; Get ExecBase
		jsr	_LVOAvailMem(a6)	; Using the above flags...
		move.l	(sp)+,a6		; Restore a6
		move.l	d0,d1			; Move d0 to d1...
		sub.l	#Min_Chip_Free,d1	; Subtract the minimum free
		bmi.s	NoAllocMsg		; If not enough, no allocation
		move.l	4(sp),d0		; Get size asked for...
		sub.l	d0,d1			; Subtract asked for size
		bmi.s	NoAllocMsg		l If not enough, no allocation
*
* Now, try to allocation from any place...
*
		move.l	#MEMF_CLEAR!MEMF_PUBLIC,-(sp)
		move.l	d0,-(sp)		; The size we want...
		bsr.s	_AllocVecStub		; Try the allocation
		addq.l	#8,sp			; Adjust stack back...
		tst.l	d0			; Check if it worked...
		bne.s	YesAllocMsg		; We allocated it...
*
NoAllocMsg:	moveq.l	#0,d0			; No allocation
		bra.s	DoneAllocMsg		; Alloc Done...
*
* Now, count the bytes of the allocated message.
* This uses the fact that AllocVec stores the size at -4 of the allocation.
*
YesAllocMsg:	move.l	d0,a0			; Get pointer...
		move.l	-(a0),d1		; Get size...
		sub.l	d1,wb_ByteCount(a6)	; Subtract from the byte count
DoneAllocMsg:	rts				; Return with d0=allocated mem
*
*******************************************************************************
*
* Forbid stub...
*
		XDEF	_ForbidStub
_ForbidStub:	move.l	wb_SysBase(a6),a0	; Get SysBase
		addq.b	#1,TDNestCnt(a0)	; Forbid()
		rts
*
*******************************************************************************
*
* Permit stub...
*
		XREF	_LVOPermit
		XDEF	_PermitStub
_PermitStub:	lea	_LVOPermit.w,a0		; Permit...
		bra.s	GeneralExec		; Do it...
*
*******************************************************************************
*
* ThisTask	Returns the current task pointer in d0
*
		XREF	_LVOFindTask
		XDEF	_ThisTask
_ThisTask:	sub.l	a1,a1			; Clear a1
		lea	_LVOFindTask.w,a0	; FindTask...
		bra.s	GeneralExec
*
*******************************************************************************
*
* OpenLibraryStub
*
		XREF	_LVOOpenLibrary
		XDEF	_OpenLibraryStub
_OpenLibraryStub:
		lea	_LVOOpenLibrary.w,a0	; Get offset
		bra.s	ArgInA1D0		; Go do the reset...
*
*******************************************************************************
*
* FreeMemStub
*
		XREF	_LVOFreeMem
		XDEF	_FreeMemStub
_FreeMemStub:	lea	_LVOFreeMem.w,a0	; Get offset
ArgInA1D0:	move.l	8(sp),d0		; Get second argument
		bra.s	ArgInA1			; Go do the rest...
*
*******************************************************************************
*
* CloseLibraryStub
*
		XREF	_LVOCloseLibrary
		XDEF	_CloseLibraryStub
_CloseLibraryStub:
		lea	_LVOCloseLibrary.w,a0	; Get offset
		bra.s	ArgInA1			; Go do the reset...
*
*******************************************************************************
*
* RemoveStub		(Note that this could be just inline Remove())
*
		XREF	_LVORemove
		XDEF	_RemoveStub
_RemoveStub:	lea	_LVORemove.w,a0		; Get offest
		bra.s	ArgInA1			; Go do the rest...
*
*******************************************************************************
*
* AllocMemStub
*
		XREF	_LVOAllocMem
		XDEF	_AllocMemStub
_AllocMemStub:	lea	_LVOAllocMem.w,a0	; Get offset
		bra.s	ArgInD0D1		; Go do the rest...
*
*******************************************************************************
*
* AllocVecStub
*
		XREF	_LVOAllocVec
		XDEF	_AllocVecStub
_AllocVecStub:	lea	_LVOAllocVec.w,a0	; Get offset
;		bra.s	GetArgInD0D1		; Do the rest...
*
*******************************************************************************
*
* Get the first two arguments from stack into D0/D1
*
ArgInD0D1:	move.l	4(sp),d0		; Get the arguments from
		move.l	8(sp),d1		; the stack...
*
*******************************************************************************
*
* GeneralExec call offset in a0
*
GeneralExec:	move.l	a6,-(sp)		; Save a6
		move.l	wb_SysBase(a6),a6	; Get ExecBase
		jsr	0(a6,a0.w)		; Call exec...
		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* FreeMyMsg - Used in copy.c to free a copy message and count the bytes...
*
		XDEF	_FreeMyMsg
_FreeMyMsg:	move.l	4(sp),d0		; Get message pointer
		beq.s	_FreeVecStub		; If no message, just free
		move.l	d0,a1			; Put pointer into a1
		move.l	d0,a0			; (and one into a0)
		move.l	-(a0),d0		; Get size of allocation
		add.l	d0,wb_ByteCount(a6)	; Add to the byte count
;		bra.s	_FreeVecStub		; And do the FreeVec...
*
*******************************************************************************
*
* FreeVecStub
*
		XREF	_LVOFreeVec
		XDEF	_FreeVecStub
_FreeVecStub:	lea	_LVOFreeVec.w,a0	; Get offset
;		bra.s	ArgInA1			; Go do the rest...
*
*******************************************************************************
*
* Get the first argument from stack into A1
*
ArgInA1:	move.l	4(sp),a1		; Get the argument...
		bra.s	GeneralExec
*
*******************************************************************************
*
* AlphaEnqueue - A much smaller version of the alpha-enqueue...
*
*	AlphaEnqueue(struct List *,struct Node *)
*
		XDEF	_AlphaEnqueue
		XREF	_LVOInsert
_AlphaEnqueue:	movem.l	a2/a3/a4,-(sp)	; Save these...
		move.l	4+12(sp),a2	; Get LIST
		move.l	8+12(sp),a3	; Get NODE
		move.l	a3,-(sp)	; Set NODE
		move.l	a2,-(sp)	; Set LIST
		bsr.s	_AddTailStub	; Add it to the list...
		addq.l	#8,sp		; Adjust stack...
		move.l	(a2),a4		; Start at first node
AlphaLoop:	cmp.l	a4,a3		; Check if we are at the end
		beq.s	AlphaDone	; If same node, we are done.
		move.l	LN_NAME(a3),a1	; Get insert string
		move.l	LN_NAME(a4),a0	; Get node string
		bsr.s	stricmp		; Check it...
		tst.b	d0		; Check result
		bpl.s	AlphaInsert	; Found the insert point.
		move.l	(a4),a4		; Get next node
		bra.s	AlphaLoop	; Loop back
AlphaInsert:	move.l	a3,-(sp)	; Set up node
		bsr.s	_RemoveStub	; Remove the node
		addq.l	#4,sp		; Clean up
		move.l	a2,a0		; Set up LIST
		move.l	a3,a1		; Set up NODE
		move.l	LN_PRED(a4),a2	; Set up LISTNODE
		move.w	#_LVOInsert,d0	; Set up INSERT lvo...
		bsr.s	ExecInD0	; Call EXEC Insert()...
AlphaDone:	movem.l	(sp)+,a2/a3/a4	; Restore
		rts
*
*******************************************************************************
*
* AddHeadStub
*
		XREF	_LVOAddHead
		XDEF	_AddHeadStub
_AddHeadStub:	move.w	#_LVOAddHead,d0		; Get offset...
GeneralA0A1:	move.l	4(sp),a0		; Get the arguments from
		move.l	8(sp),a1		; the stack...
ExecInD0:	move.l	a6,-(sp)		; Save a6
		move.l	wb_SysBase(a6),a6	; Get execbase
		jsr	0(a6,d0.w)		; AddHead
RestoreReturn:	move.l	(sp)+,a6		; Restore a6
		rts				; And we are done.
*
*******************************************************************************
*
* ReplyMsgStub
*
		XREF	_LVOReplyMsg
		XDEF	_ReplyMsgStub
_ReplyMsgStub:	lea	_LVOReplyMsg.w,a0	; Get offset...
		bra.s	ArgInA1		; Do the general routine.
*
*******************************************************************************
*
* AddTailStub
*
		XREF	_LVOAddTail
		XDEF	_AddTailStub
_AddTailStub:	move.w	#_LVOAddTail,d0		; Get offset...
		bra.s	GeneralA0A1		; Do the general routine.
*
*******************************************************************************
*
* PutMsgStub
*
		XREF	_LVOPutMsg
		XDEF	_PutMsgStub
_PutMsgStub:	move.w	#_LVOPutMsg,d0		; Get offset...
		bra.s	GeneralA0A1		; Do the general routine.
*
*******************************************************************************
*
* GetMsgStub
*
		XREF	_LVOGetMsg
		XDEF	_GetMsgStub
_GetMsgStub:	move.w	#_LVOGetMsg,d0		; Get offset...
		bra.s	GeneralA0A1		; A1 is not needed/ignored...
*
*******************************************************************************
*
* WaitPortStub
*
		XREF	_LVOWaitPort
		XDEF	_WaitPortStub
_WaitPortStub:	move.w	#_LVOWaitPort,d0	; Get offset...
		bra.s	GeneralA0A1		; A1 is not needed/ignored...
*
*******************************************************************************
*
* Now for utility.library
*
*******************************************************************************
*
* The stricmp() routine from utility...
*
		XREF	_LVOStricmp
		XDEF	_stricmp
_stricmp:	movem.l	4(sp),a0/a1		; a0=s1, a1=s2
stricmp:	move.l	a6,-(sp)		; Save a6...
		move.l	wb_UtilityBase(a6),a6	; Get utilitybase...
		jsr	_LVOStricmp(a6)		; Call it...
		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* __CXD33 replacement that uses utility.library
*
		XREF	_LVOSDivMod32
		XDEF	__CXD33			; Signed divide
__CXD33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	wb_UtilityBase(a6),a6	; Get utility base pointer
		jsr	_LVOSDivMod32(a6)	; Do the divide
		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* __CXD22 replacement that uses utility.library
*
		XREF	_LVOUDivMod32
		XDEF	__CXD22			; Unsigned divide
__CXD22:	move.l	a6,-(sp)		; Save your base pointer
		move.l	wb_UtilityBase(a6),a6	; Get utility base pointer
		jsr	_LVOUDivMod32(a6)	; Do the divide
		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* __CXM33 replacement that uses utility.library
*
		XREF	_LVOSMult32
		XDEF	__CXM33			; Signed multiply
__CXM33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	wb_UtilityBase(a6),a6	; Get utility base pointer
		jsr	_LVOSMult32(a6)		; Do the multiply
		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* __CXM22 replacement that uses utility.library
*
		XREF	_LVOUMult32
		XDEF	__CXM22			; Unsigned multiply
__CXM22:	move.l	a6,-(sp)		; Save your base pointer
		move.l	wb_UtilityBase(a6),a6	; Get utility base pointer
		jsr	_LVOUMult32(a6)		; Do the multply
		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* Now for the DOS stubs...
*
*******************************************************************************
*
* UnLockStub
*
*	Extra feature:  It check to see if the lock being unlocked is
*	the same as what the CurrentDir is.  If so, it clears the CurrentDir
*
		XREF	_LVOUnLock
		XDEF	_UnLockStub
_UnLockStub:	move.l	4(sp),d0		; Get lock...
		move.l	wb_SysBase(a6),a0	; Get ExecBase...
		move.l	ThisTask(a0),a0		; Get my task/process...
		cmp.l	pr_CurrentDir(a0),d0	; Check if the same...
		bne.s	DoUnLock		; If not, just unlock...
		moveq	#0,d0			; Clear d0...
		move.l	d0,-(sp)		; Stack it...
		bsr.s	_CurrentDirStub		; Clear the CurrentDir
		addq.l	#4,sp			; Adjust stack back...
;						  now do the unlock...
DoUnLock:	move.w	#_LVOUnLock,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* SameDeviceStub
*
		XREF	_LVOSameDevice
		XDEF	_SameDeviceStub
_SameDeviceStub:
		move.w	#_LVOSameDevice,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* ParentDirStub
*
		XREF	_LVOParentDir
		XDEF	_ParentDirStub
_ParentDirStub:	move.w	#_LVOParentDir,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* DupLockStub
*
		XREF	_LVODupLock
		XDEF	_DupLockStub
_DupLockStub:	move.w	#_LVODupLock,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* CreateDirStub
*
		XREF	_LVOCreateDir
		XDEF	_CreateDirStub
_CreateDirStub:	move.w	#_LVOCreateDir,d0	; Get offset
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
* CurrentDirStub
*
		XREF	_LVOCurrentDir
		XDEF	_CurrentDirStub
_CurrentDirStub:
		move.w	#_LVOCurrentDir,d0	; Get offset
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
* ReadStub
*
		XREF	_LVORead
		XDEF	_ReadStub
_ReadStub:	move.w	#_LVORead,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* WriteStub
*
		XREF	_LVOWrite
		XDEF	_WriteStub
_WriteStub:	move.w	#_LVOWrite,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* SeekStub
*
		XREF	_LVOSeek
		XDEF	_SeekStub
_SeekStub:	move.w	#_LVOSeek,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* OpenStub
*
		XREF	_LVOOpen
		XDEF	_OpenStub
_OpenStub:	move.w	#_LVOOpen,d0		; Get offset
;		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* DosStubArg -	Common stub that uses d0 as the offset and does a 3
*		argument DOS call...
*
DosStubArg:	movem.l	d2/d3/d4/a6,-(sp)	; Save d2/d3/d4 & a6
		movem.l	20(sp),d1/d2/d3/d4	; Get first arg
		move.l	wb_DOSBase(a6),a6	; Get DOSBase
		jsr	0(a6,d0.w)		; Call the routine
		movem.l	(sp)+,d2/d3/d4/a6	; Restore d2/d3/d4 & a6
		rts
*
*******************************************************************************
*
* CreateProcStub
*
		XREF	_LVOCreateProc
		XDEF	_CreateProcStub
_CreateProcStub:
		move.w	#_LVOCreateProc,d0	; Get offset
		bra.s	DosStubArg		; Do Common stub
*
*******************************************************************************
*
* RenameStub
*
		XREF	_LVORename
		XDEF	_RenameStub
_RenameStub:
		move.w	#_LVORename,d0		; Get offset
		bra.s	DosStubArg		; Do Common stub
*
*******************************************************************************
*
* CreateNewProcStub
*
		XREF	_LVOCreateNewProc
		XDEF	_CreateNewProcStub
_CreateNewProcStub:
		move.w	#_LVOCreateNewProc,d0	; Get offset
		bra.s	DosStubArg		; Do Common stub
*
*******************************************************************************
*
* SameLockStub
*
		XREF	_LVOSameLock
		XDEF	_SameLockStub
_SameLockStub:	move.w	#_LVOSameLock,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* ExamineStub
*
		XREF	_LVOExamine
		XDEF	_ExamineStub
_ExamineStub:	move.w	#_LVOExamine,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* ExNextStub
*
		XREF	_LVOExNext
		XDEF	_ExNextStub
_ExNextStub:	move.w	#_LVOExNext,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* DeleteFileStub
*
		XREF	_LVODeleteFile
		XDEF	_DeleteFileStub
_DeleteFileStub:
		move.w	#_LVODeleteFile,d0	; Get offset
		bra.s	DosStubArg
*
*******************************************************************************
*
* StrToLongStub
*
		XREF	_LVOStrToLong
		XDEF	_StrToLongStub
_StrToLongStub:	move.w	#_LVOStrToLong,d0	; Get offset
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
* FilePartStub
*
		XREF	_LVOFilePart
		XDEF	_FilePartStub
_FilePartStub:	move.w	#_LVOFilePart,d0	; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* AddPartStub
*
		XREF	_LVOAddPart
		XDEF	_AddPartStub
_AddPartStub:	move.w	#_LVOAddPart,d0		; Get offset
		bra.s	DosStubArg		; Do common stub
*
*******************************************************************************
*
* IoErrStub - Special stub that always returns an error...
*
		XREF	_LVOIoErr
		XDEF	_IoErrStub
_IoErrStub:	move.w	#_LVOIoErr,d0		; Get offset...
		bsr.s	DosStubArg		; Do common stub
		tst.l	d0			; Check for NULL IoErr()
		bne.s	IoErrRts		; We have a real error...
		moveq.l	#ERROR_REQUIRED_ARG_MISSING,d0
IoErrRts:	rts
*
*******************************************************************************
*
* atoi - This atoi uses the dos.library atoi routine...
*
		XDEF	_atoi
_atoi:		move.l	4(sp),a0		; Get string pointer...
		move.l	d0,-(sp)		; Space on the stack...
		move.l	sp,a1			; Get a pointer to the space...
		move.l	a1,-(sp)		; Save the pointer
		move.l	a0,-(sp)		; Save the string pointer...
		bsr.s	_StrToLongStub		; Do the StrToLong...
		addq.l	#8,sp			; Adjust stack...
		move.l	(sp)+,d0		; Get return result...
		rts
*
*******************************************************************************
*
* SetCommentStub
*
		XREF	_LVOSetComment
		XDEF	_SetCommentStub
_SetCommentStub:
		move.w	#_LVOSetComment,d0	; Get offset
		bra.s	DosStubNoReq		; Do non-requester stub...
*
*******************************************************************************
*
* SetFileDateStub
*
		XREF	_LVOSetFileDate
		XDEF	_SetFileDateStub
_SetFileDateStub:
		move.w	#_LVOSetFileDate,d0	; Get offset
		bra.s	DosStubNoReq		; Do non-requester stub...
*
*******************************************************************************
*
* SetProtectionStub
*
		XREF	_LVOSetProtection
		XDEF	_SetProtectionStub
_SetProtectionStub:
		move.w	#_LVOSetProtection,d0	; Get offset
;		bra.s	DosStubNoReq		; Do non-requester stub...
*
*******************************************************************************
*
* Version of the DOS stub without requesters...
*
DosStubNoReq:	move.l	wb_SysBase(a6),a0	; Get ExecBase...
		move.l	ThisTask(a0),a0		; Get current task...
		move.l	pr_WindowPtr(a0),-(sp)	; Save window pointer...
		moveq.l	#-1,d1			; We now set -1
		move.l	d1,pr_WindowPtr(a0)	;   as out window pointer...
		movem.l	d2/d3/d4/a0/a6,-(sp)	; Save d2/d3/d4 & a0/a6
		movem.l	28(sp),d1/d2/d3/d4	; Get first arg
		move.l	wb_DOSBase(a6),a6	; Get DOSBase
		jsr	0(a6,d0.w)		; Call the routine
		movem.l	(sp)+,d2/d3/d4/a0/a6	; Restore d2/d3/d4 & a0/a6
		move.l	(sp)+,pr_WindowPtr(a0)	; Restore window pointer...
		rts				; Return
*
*******************************************************************************
*
		XREF	_LVONameFromLock
		XDEF	_NameFromLockStub
_NameFromLockStub:
		move.w	#_LVONameFromLock,d0	; Get offset
		bra.s	DosStubNoReq		; Do non-requester stub...
*
*******************************************************************************
*
* Routine to set the IoErr value...
*
		XDEF	_SetResult2
_SetResult2:	move.l	wb_SysBase(a6),a1	; Get execbase...
		move.l	ThisTask(a1),a1		; Get current task/process
		move.l	4(sp),pr_Result2(a1)	; Set result...
		rts
*
*******************************************************************************
*
* LockRamDisk replacement
*
_LockRamDisk:	xdef	_LockRamDisk		; Grin
		moveq.l	#ACCESS_READ,d0		; Read mode
		move.l	d0,-(sp)		; Save...
		pea	RAM(pc)			; Pointer
		bsr	_LockStub		; Do lock
		addq.l	#8,sp			; Adjust back
		rts
*
*******************************************************************************
*
* strcat	- Smaller strcat for workbench...
*
		XDEF	_strcat
_strcat:	movem.l	4(sp),a0/a1		; Get destination & source
strcat_loop:	move.b	(a0)+,d0		; Loop for the start...
		bne.s	strcat_loop		; loop...
		subq.l	#1,a0			; Drop by one...
		bra.s	strcpy_loop		; go to the loop...
*
		XDEF	_strcpy			;
_strcpy:	movem.l	4(sp),a0/a1		; Get args...
strcpy_loop:	move.b	(a1)+,(a0)+		; Move the data...
		bne.s	strcpy_loop		; Keep going...
		rts
*
*******************************************************************************
*
* stccpy	- Smaller version of the copy routine...
*
		XDEF	_stccpy
_stccpy:	movem.l	4(sp),a0/a1		; Get destination & source
		move.l	12(sp),d0		; Get max chars
		subq.l	#1,d0			; One less than given...
stccpy_loop:	move.b	(a1)+,(a0)+		; Copy the data...
		dbeq.s	d0,stccpy_loop		; Loop for max or null...
		clr.b	-(a0)			; Make sure NULL terminated
		rts
*
*******************************************************************************
*
* strlen	- To remove the built-in version...
*
		XDEF	_strlen
_strlen:	move.l	4(sp),a0		; Get start...
		move.l	a0,d0			; Store it...
strlen_loop:	move.b	(a0)+,d1		; Check for NULL
		bne.s	strlen_loop		; Keep going...
		exg	a0,d0			; swap...
		sub.l	a0,d0			; subtract...
		subq.l	#1,d0			; Don't count the NULL
		rts
*
*******************************************************************************
*
RAM:		dc.b	'RAM:',0
*
*******************************************************************************
        end
