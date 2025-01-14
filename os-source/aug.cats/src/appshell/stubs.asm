*************************************************************************
*									*
*	Copyright (C) 1990, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* stubs.asm
*
* Source Control
* ------ -------
*
* $Id: stubs.asm,v 1.4 1992/09/07 18:04:56 johnw Exp johnw $
*
* $Log: stubs.asm,v $
* Revision 1.4  1992/09/07  18:04:56  johnw
* Minor changes.
*
* Revision 1.1  91/12/12  15:01:19  davidj
* Initial revision
* 
* Revision 37.3  91/01/22  21:20:55  mks
* Added the CXD replacement routines.
* 
* Revision 37.2  91/01/17  11:03:10  mks
* Added Rename stub.
*
* Revision 37.1  91/01/04  20:07:46  mks
* Re-checked in under the V37 source tree...
*
* Revision 36.9  90/12/18  17:28:59  mks
* Added StrToLongStub support...
*
* Revision 36.8  90/12/11  07:08:32  mks
* Removed non-flashing ants...
*
* Revision 36.7  90/12/10  22:53:24  mks
* Added a DrawBox() routine that does not draw the corners twice
*
* Revision 36.6  90/12/02  07:16:03  mks
* Added the stricmp routine from Utility.library
*
* Revision 36.5  90/12/02  05:41:20  mks
* Added stubs for OpenLibrary and CloseLibrary
*
* Revision 36.4  90/12/01  21:40:04  mks
* Added stubs for various dos functions
*
* Revision 36.3  90/11/19  16:31:16  mks
* Added stubs for SameLock(), Examine(), and ExNext()
*
* Revision 36.2  90/11/12  19:13:10  mks
* Re-activated the SetResult2() code.
*
* Revision 36.1  90/11/07  15:20:57  mks
* Major optimizations of the stub routines.
*
* Revision 1.6  90/11/06  19:19:52  mks
* Changed things around to save space.  Also added a feature
* to IoErr() such that it will never return a NULL.
*
* Revision 1.5  90/10/22  16:42:44  mks
* Needed to restore the ALLOCMEM and FREEMEM stubs...
*
* Revision 1.4  90/10/21  20:15:28  mks
* Commented out the AllocMem and FreeMem stubs...
*
* Revision 1.3  90/10/18  15:11:28  mks
* Added AllocVec and FreeVec to the stubs...
*
* Revision 1.2  90/10/13  13:54:07  mks
* New stubs for various DOS calls...
*
* Revision 1.1  90/10/11  16:42:57  mks
* Initial revision
*
*
*************************************************************************

		SECTION	code

		INCLUDE 'exec/types.i'
		INCLUDE	'exec/execbase.i'
		INCLUDE	'workbench.i'
		INCLUDE	'workbenchbase.i'


*******************************************************************************
*
* Stub routines to save code in workbench...
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
		move.l	(sp)+,a6		; Restore a6
		rts				; And we are done...
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
		move.l	a6,-(sp)		; Save a6
		move.l	wb_SysBase(a6),a6	; Get execbase
		jsr	0(a6,d0.w)		; AddHead
		move.l	(sp)+,a6		; Restore a6
		rts				; And we are done.
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
* Now for utility.library
*
*******************************************************************************
*
* The stricmp() routine from utility...
*
		XREF	_LVOStricmp
		XDEF	_stricmp
_stricmp:	movem.l	4(sp),a0/a1		; a0=s1, a1=s2
		move.l	a6,-(sp)		; Save a6...
		move.l	wb_UtilityBase(a6),a6	; Get utilitybase...
		jsr	_LVOStricmp(a6)		; Call it...
		move.l	(sp)+,a6		; Restore workbenchbase
		rts
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
		move.l	(sp)+,a6		; Restore base register
		rts
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
		move.l	(sp)+,a6		; Restore base register
		rts
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
		move.l	(sp)+,a6		; Restore base register
		rts
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
		move.l	(sp)+,a6		; Restore base register
		rts
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
* UnLockStub
*
		XREF	_LVOUnLock
		XDEF	_UnLockStub
_UnLockStub:	move.w	#_LVOUnLock,d0		; Get offset
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

        end
