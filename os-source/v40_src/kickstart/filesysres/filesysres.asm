**
**	$Id: filesysres.asm,v 36.13 93/02/05 13:49:05 jesup Exp $
**
**      FileSystem resource module
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION	filesysres,code

**	Includes

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/initializers.i"

	INCLUDE	"filesysres.i"
	INCLUDE	"filesysres_rev.i"
	INCLUDE "src/kickstart/filesysres/fs_rev.i"

**	Macros

XLVO	MACRO
	XREF	_LVO\1
	ENDM

CALLLVO	MACRO
		jsr	_LVO\1(a6)
	ENDM

**	Imports

	XLVO	AddResource
	XLVO	AllocMem
	XLVO	OpenResource


**	Locals

DOS5TYPE	EQU	$444f5305		; DOS\5
DOS4TYPE	EQU	$444f5304		; DOS\4
DOS3TYPE	EQU	$444f5303		; DOS\3
DOS2TYPE	EQU	$444f5302		; DOS\2
DOS1TYPE	EQU	$444f5301		; DOS\1
		;	36.03 is highest L:FastFileSystem released, so this
		;	must be bigger than that.  The fs revision in 2.0
		;	was 36.223 when I chose 36.200 here.

; comes from filesystem revision automatically
DOS1VERSION	EQU	FS_REV			; version.revision
DOS1SIZEOF	EQU	fse_PatchFlags+4

UNI1TYPE	EQU	$554e4901		; UNI\1
UNI1VERSION	EQU	0			; version
UNI1SIZEOF	EQU	fse_Handler+4


 STRUCTURE  FSRData,FileSysResource_SIZEOF	; file system resource itself
    STRUCT  fsrd_DOS5,DOS1SIZEOF		; entry for DOS\5
    STRUCT  fsrd_DOS4,DOS1SIZEOF		; entry for DOS\4
    STRUCT  fsrd_DOS3,DOS1SIZEOF		; entry for DOS\3
    STRUCT  fsrd_DOS2,DOS1SIZEOF		; entry for DOS\2
    STRUCT  fsrd_DOS1,DOS1SIZEOF		; entry for DOS\1
    STRUCT  fsrd_UNI1,UNI1SIZEOF		; entry for UNI\1
    LABEL   FSRData_SIZEOF


**	Assumptions

	IFNE	UNI1VERSION
	FAIL	"UNI1VERSION not zero, recode to test version"
	ENDC
	IFNE	((fse_Handler-fse_Type)/4)/8
	FAIL	"fse_Handler not in low byte of fse_PatchFlags, recode"
	ENDC


******* FileSystem.resource/--background-- ***************************
*
*   PURPOSE
*
*	The FileSystem.resource is where boot disk drivers rendezvous
*	to share file system code segments for partitions specified by
*	dos type.  Prior to V36, it was created by the first driver
*	that needed to use it.  For V36, its creation is ensured by the
*	rom boot process.
*
*   CONTENTS
*
*	The FileSystem.resource is described in the include file
*	resources/filesysres.h.  The nodes on it describe how to
*	algorithmically convert the result of MakeDosNode (from the
*	expansion.library) to a node appropriate for the dos type.
*
*	FileSysEntry
*	    fse_Node		on fsr_FileSysEntries list
*				ln_Name is of creator of this entry
*	    fse_DosType		DosType of this FileSys: e.g. 0x444f5301
*				for the fast file system.
*	    fse_Version		high word is the version, low word is
*				the revision.
*	    fse_PatchFlags	bits set for those of the following that
*				need to be substituted into a standard
*				device node for this file system: e.g.
*				$180 for substitute SegList & GlobalVec
*	    fse_Type		device node type: zero
*	    fse_Task		standard dos "task" field
*	    fse_Lock		must be zero
*	    fse_Handler		for V36, if bit 31 is set, this is not
*				an AmigaDOS partition.
*	    fse_StackSize	stacksize to use when starting task
*	    fse_Priority	task priority when starting task
*	    fse_Startup		startup msg: FileSysStartupMsg for disks
*	    fse_SegList		segment of code to run to start new task
*	    fse_GlobalVec	BCPL global vector when starting task
*
*	no more entries need exist than those implied by fse_PatchFlags,
*	so entries do not have a fixed size.  For V36, for example, the
*	entry for the fast file system (fse_DosType 0x444f5301)
*	contains a zero fse_PatchFlags, and thus no entries beyond that.
*
*   NOTES
*	For V36, rom code will ensure that an entry for DOS\1 (0x444f5301)
*	exists with at least version 36.0, and for UNI\0 (0x554e4900) with
*	at least version 0.0.  The latter is marked as a non-AmigaDOS
*	partition.
*
**********************************************************************

residentPR:
		dc.w	RTC_MATCHWORD
		dc.l	residentPR
		dc.l	fsrEndModule
		dc.b	RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_RESOURCE
		dc.b	80
		dc.l	fsrName
		dc.l	fsrID
		dc.l	fsrInitCode

fsrName:
		dc.b	'FileSystem.resource',0

fsrID:
		VSTRING

;------	findEntry ----------------------------------------------------
;
;   INPUTS
;   d2	entry type
;   a5	filesysres
;
;   RESULTS
;   ccr	non-zero indicates found
;   a2	entry if found
;
findEntry:
		;-- see if an entry for d2 already exists
		move.l	fsr_FileSysEntries(a5),a2
feSearchType:
		move.l	(a2),d0
		beq.s	feFail
		cmp.l	fse_DosType(a2),d2
		beq.s	feSuccess
		move.l	d0,a2
		bra.s	feSearchType

feSuccess:
		moveq	#1,d0

feFail:
		rts


;------	createEntry --------------------------------------------------
;
;   INPUTS
;   d0	entry size
;   a5	filesysres
;   a6  SysBase
;
;   RESULTS
;   ccr	non-zero indicates created
;   a2	entry if created
;
createEntry:
		move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1
		CALLLVO	AllocMem
		tst.l	d0
		beq.s	ceDone

		move.l	d0,a2
		;--	addtail this node to the resource
		lea	fsr_FileSysEntries+MLH_TAILPRED(a5),a0
		move.l	(a0),a1		; (get prev last node)
		move.l	a2,(a0)		; patch list tail back to this node
		subq.l	#4,a0
		movem.l	a0/a1,(a2)	; patch this node to list tail and
					;   to prev last node
		move.l	a2,(a1)		; patch this to prev last node succ

ceDone:
		rts


;------	updateEntry --------------------------------------------------
;
;   INPUTS
;   d1	entry version
;   d2	entry type
;   a2	entry
;
updateEntry:
		move.l	d2,fse_DosType(a2)
		move.l	d1,fse_Version(a2)
		clr.l	fse_PatchFlags(a2)
		lea	fsrID(pc),a0
		move.l	a0,LN_NAME(a2)
		rts


;------ fsrInitCode --------------------------------------------------
fsrInitCode:
		movem.l	d2/a2-a3/a5,-(a7)
		;-- see if the resource already exists
		lea	fsrName(pc),a1
		CALLLVO	OpenResource

		tst.l	d0
		beq.s	ficCreateResource
		move.l	d0,a5

		;-- ensure DOS\1 entry >= 36.0 exists
		move.l	#DOS5TYPE,d2
fic_dosloop:
		bsr.s	findEntry
		bne.s	ficFoundDOS1

		moveq	#DOS1SIZEOF,d0
		bsr.s	createEntry
		beq.s	ficUNI1		; (unexpected) memory failure

		;--	have DOS\1 entry, compare versions
ficFoundDOS1:
		move.l	#DOS1VERSION,d1
		cmp.l	fse_Version(a2),d1
		ble.s	fic_looptest	; (newly created always drops thru)

		;--	overwrite this entry in-place with better one from rom
ficUpdateDOS1:
		bsr.s	updateEntry

fic_looptest:	subq.b	#1,d2		; down to DOS\1
		bne.s	fic_dosloop

		;-- ensure UNI\1 entry >= 0.0 exists
ficUNI1:
		move.l	#UNI1TYPE,d2
		bsr	findEntry
		bne.s	ficDoneNC

		moveq	#UNI1SIZEOF,d0
		bsr.s	createEntry
		beq.s	ficDoneNC	; (unexpected) memory failure

		moveq	#UNI1VERSION,d1
		bsr	updateEntry
		bset	#((fse_Handler-fse_Type)/4),fse_PatchFlags+3(a2)
		bset	#7,fse_Handler(a2)

ficDoneNC:
		moveq	#0,d0		; show no resource created here

ficDone:
		movem.l	(a7)+,d2/a2-a3/a5
		rts


		;-- create the FileSystem.resource
ficCreateResource:
		move.l	#FSRData_SIZEOF,d0
		move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1
		CALLLVO	AllocMem

		tst.l	d0
		beq.s	ficDoneNC

		move.l	d0,a5

		move.b	#NT_RESOURCE,LN_TYPE(a5)

		lea	fsrName(pc),a0
		move.l	a0,LN_NAME(a5)

		lea	fsrID(pc),a0
		move.l	a0,fsr_Creator(a5)

		lea	fsr_FileSysEntries(a5),a3
		NEWLIST	a3			; save a3 for AddTails

		lea	fsrd_DOS5(a5),a2
		move.l	#DOS5TYPE,d2
dosloop:
		move.l	#DOS1VERSION,d1
		bsr	updateEntry
		move.l	a3,a0
		move.l	a2,a1
		ADDTAIL				; add to resource list
		add.w	#DOS1SIZEOF,a2		; next entry
		subq.b	#1,d2			; dos5->dos1
		bne.s	dosloop

		lea	fsrd_UNI1(a5),a2
		move.l	#UNI1TYPE,d2
		moveq	#UNI1VERSION,d1
		bsr	updateEntry
		move.l	a3,a0
		move.l	a2,a1
		ADDTAIL				; add to resource list
		bset	#((fse_Handler-fse_Type)/4),fse_PatchFlags+3(a2)
		bset	#7,fse_Handler(a2)

		move.l	a5,a1
		CALLLVO	AddResource

		move.l	a5,d0
		bra	ficDone


fsrEndModule:

	END
