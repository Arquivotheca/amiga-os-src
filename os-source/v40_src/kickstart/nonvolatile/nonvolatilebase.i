******************************************************************************
*
*	$Id: nonvolatilebase.i,v 40.6 93/05/06 14:06:20 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	nonvolatilebase.i,v $
* Revision 40.6  93/05/06  14:06:20  brummer
* Add nv_Flags to base to allow for multicommand option.
* 
* Revision 40.5  93/03/11  14:22:54  brummer
* Fix chkkillrequester/chkrestorerequester macros to use the value
* in nonvolatile.library base instead of opening it every time the
* macro is used.
*
* Revision 40.4  93/03/09  13:52:27  brummer
* Add the internal definitions and macros removed from
* nonvolatile.i
*
* Revision 40.3  93/03/08  14:04:51  brummer
* Add DiskInit semaphore to lib base.
*
* Revision 40.2  93/03/05  12:49:24  brummer
* Add storage location for lowlevel libraray base pointer
*
* Revision 40.1  93/03/01  15:10:13  brummer
* Added ptr for utility.library base.
* Added semaphore for NVRAM device access
*
* Revision 40.0  93/02/19  15:29:32  Jim2
* Changed EQU that prevents this file from being included multiple times.
*
* Revision 39.0  93/02/03  11:17:35  Jim2
* Placed nv_ in front of all fields.  Added nv_ExecBase, nv_NVRAMCopy and
* nv_DiskLock.  Removed nv_ExpansionBase and nv_Semaphore.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

        IFND    PRIVATE_NONVOLATILEBASE_I
PRIVATE_NONVOLATILEBASE_I      SET     1

    IFND    EXEC_TYPES_I
    include 'exec/types.i'
    ENDC

    IFND    EXEC_LIBRARIES_I
    include 'exec/libraries.i'
    ENDC

    IFND    EXEC_SEMAPHORES_I
    include 'exec/semaphores.i'
    ENDC

NONVOLATILELIBRARYNAME      MACRO
        dc.b    'nonvolatile.library',0
        ENDM


 STRUCTURE NVBase,LIB_SIZE			;(Word Sized)
	UBYTE	nv_Pad0				; ##
	UBYTE	nv_Flags			; ## misc flags
	APTR	nv_SegList			;
	APTR	nv_ExecBase			; exec.library base
	APTR	nv_DOSBase			; dos.library base
	APTR	nv_UTILBase			; utility.library base
	APTR	nv_LowLevelBase			; lowlevel.library base
	APTR	nv_NVRAMCopy			; ptr to NVRAM device RAM copy
	APTR	nv_DiskLock			;
	STRUCT	nv_Semaphore,SS_SIZE		; Semaphore used to for NVRAM access
	STRUCT	nv_DiskInitSema,SS_SIZE		; Semaphore for disk init
	LABEL     NVBASESIZE

;
; Offsets for JSR to lowlevel library
;
_LVOKillReq	equ	-$78			; kill requesters
_LVORestoreReq	equ	-$7E			; restore requesters

;
; nv_Flags :
;
	BITDEF	nv_Flags,MultiCommand,0		; ## NVRAM multi-command flag

******************************************************************************
*
*   Macro
*	ChkKillRequesters - Checks if requesters are to be killed and if so
*				calls lowlevel.library to kill them.
*
*   FUNCTION	This macro should be used with care, it will leave the
*		requester flag on the stack.  It should be paired with the
*		following macro ChkRestoreRequesters.
*
*   INPUTS	a5 = nonvolatile.library base
*		d1 = TRUE if requesters are to be killed
*		     FALSE if requesters are not to be killed
*
*   OUTPUTS	stack is pushed !!!
*
******************************************************************************
ChkKillRequesters	MACRO		;

		ext.l	d1				; are requesters allowed ?
		move.l	d1,-(sp)			; save KillRequesters param
		beq.s	5$				; if yes, j to skip
		tst.l	nv_LowLevelBase(a5)		; is lowlevel avail ?
		beq.s	5$				; if no, j to skip
		movem.l	d0-d1/a0-a1/a6,-(sp)		; save registers
		move.l	nv_LowLevelBase(a5),a6		;
		jsr	_LVOKillReq(a6)			; kill all requesters
		movem.l	(sp)+,d0-d1/a0-a1/a6		; restore registers
5$:							;
			ENDM

******************************************************************************
*
*   Macro
*	ChkRestoreRequesters - Checks if requesters were previously killed and
*				calls lowlevel.library to restore them.
*
*   FUNCTION	This macro should be used with care, it will pop the
*		requester flag off the stack.  It should be paired with the
*		previous macro ChkKillRequesters.
*
*   INPUTS	(sp)+ = TRUE if requesters are to be killed
*		     	FALSE if requesters are not to be killed
*
*   OUTPUTS	stack is popped !!!!
*
******************************************************************************
ChkRestoreRequesters	MACRO		;

		move.l	(sp)+,d1		; were requesters killed ?
		beq.s	7$			; if no, j to continue
		tst.l	nv_LowLevelBase(a5)	; is base avail ?
		beq.s	7$			; if no, j to skip
		movem.l	d0-d1/a0-a1/a6,-(sp)	; save registers
		move.l	nv_LowLevelBase(a5),a6	;
		jsr	_LVORestoreReq(a6)	; restore all requesters
		movem.l	(sp)+,d0-d1/a0-a1/a6	; restore registrers
7$:						;
			ENDM







        ENDC
