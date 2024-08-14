;*************************************************************************************
; allocmem.asm
;*************************************************************************************

	INCLUDE "exec/types.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE "globaldata.i"

;*************************************************************************************

	XDEF	_newAllocMem

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_LVODateStamp

	XREF	_nAllocMem
	XREF	_MungMem
	XREF	_ShowProcessInfo
	XREF	_GetHunkInfo
	XREF	_GetProcessName

	XREF	_kprintf
	XREF	KPutStr

;*************************************************************************************

	XREF	_gd
	XREF	alloc0
	XREF	alloc1

;*************************************************************************************
; void *ASM newAllocMem (REG (d0) ULONG size, REG (d1) ULONG attrs, REG (a5) ULONG *stackptr, REG (a6) struct Library *);
;*************************************************************************************
;	A2	-- temporary --
;	A3	void *retval
;	A4	struct GlobalData *
;	A5	ULONG *stackptr
;	A6	struct Library *SysBase
;*************************************************************************************
;	D2	ULONG osize
;	D3	BOOL domemcheck
;	D4	ULONG size
;	D5	ULONG attrs
;	D6	-- temporary --
;*************************************************************************************
_newAllocMem:
	movem.l	d2-d7/a2-a4,-(a7)		; save the registers
	move.l	_gd,a4				; get a pointer to the GlobalData
	move.l	gd_Flags(a4),d6
	btst	#GDB_ACTIVE,d6			; see if MemCheck is active...
	beq.s	_newAllocMemFast		; not active, so use the fast code.
	move.l	(a5),a1				; get the caller address
	cmpa.l	gd_LStart(a4),a1		; if (caller < gd->gd_LStart)
	blt.s	_newAllocMemSlow
	cmpa.l	gd_LEnd(a4),a1			; if (caller > gd->gd_LEnd)
	bge.s	_newAllocMemSlow
_newAllocMemFast:
	move.l	gd_AllocMem(a4),a0		; call the system AllocMem function
	jsr	(a0)
	movem.l	(a7)+,d2-d7/a2-a4		; restore the registers
	rts

_newAllocMemSlow:
	; Get the values
	move.l	d1,d5				; save attributes

	; Clear the registers
	moveq.l	#0,d3				; domemcheck = FALSE
	sub.l	a3,a3				; retval = NULL

	; Check for zero size
	move.l	d0,d4				; save size
	move.l	d0,d2
	bne.s	_CheckBig			; if (size == 0)

	; Output the size of zero error message
	jsr	_LVOForbid(a6)
	lea	alloc0,a0
	jsr	KPutStr

	; Exit via error print
	bra	_ErrorExit

_CheckBig:
	; Check if size is too large for cookie
	andi.l	#$C0000000,d0
	bne.s	_AllocMem

	; Set domemcheck to TRUE
	moveq.l	#1,d3

	; Adjust the size for structure and the walls
	addi.l	#INFOSIZE,d4			; size = INFOSIZE +
	moveq.l	#0,d0				;        mci->mci_PreSize +
	move.w	mci_PreSize(a4),d0
	add.l	d0,d4
	moveq.l	#0,d0				;        mci->mci_PostSize +
	move.w	mci_PostSize(a4),d0
	add.l	d0,d4

_AllocMem:
	move.l	d4,d0				; restore size
	move.l	d5,d1				; restore attributes
	move.l	gd_AllocMem(a4),a0		; call the system AllocMem function
	jsr	(a0)
	move.l	d0,a3				; Save the memory block
	tst.l	d0				; (move to address reg doesn't affect flags)
	beq	_AllocFailed			; See if the allocation failed

	; Did they ask for a cleared block?
	btst	#MEMB_CLEAR,d5			; Are we MEMF_CLEAR?
	bne.s	_MakeMemChunk			; If yes, then jump ahead

	; Mung the memory
	move.l	a3,a0				; memory address
	move.l	d4,d0				; size
	move.l	#$DEADF00D,d1			; mung value
	jsr	_MungMem

_MakeMemChunk:
	tst.l	d3				; Are we supposed to build the MemCheck...
	beq	_ReallyExit			; structure?

	; Fill in the MemChunk structure
 	move.l	#$C0DE0000,d0			; Do the cookie in two steps so...
	move.w	#$F00D,d0			; it doesn't appear in this code
	move.l	d0,mc_Cookie(a3)

	move.l	d2,mc_Size(a3)			; Remember the size

	movem.l	a3-a4,-(a7)
	move.l	a4,a0				; GlobalData
	move.l	ThisTask(a6),a1			; SysBase->ThisTask
	move.l	a5,a2				; stackptr
	lea	mc_Offset(a3),a4
	lea	mc_Hunk(a3),a3
	jsr	_GetHunkInfo			; a0=gd a1=tc a2=stackptr a3=hunk a4=offset
	movem.l	(a7)+,a3-a4

	move.l	ThisTask(a6),a0
	move.l	a0,mc_Task(a3)			; Remember the allocating task address
	jsr	_GetProcessName			; a0=task
	move.l	d0,a0
	lea	mc_Name(a3),a1
	moveq.l	#15,d0
fillname:
	move.b	(a0)+,d1
	beq.s	donename
	move.b	d1,(a1)+
	dbra	d0,fillname
donename:
	move.b	d1,(a1)

	; Calculate the checksum
	move.l	mc_Cookie(a3),d0		; mc->mc_CheckSum = mc->mc_Cookie +
	add.l	d2,d0				;                   mc->mc_Size +
	add.l	mc_Hunk(a3),d0			;                   mc->mc_Hunk +
	add.l	mc_Offset(a3),d0		;                   mc->mc_Offset +
	add.l	mc_Task(a3),d0			;                   mc->mc_Task
	move.l	d0,mc_CheckSum(a3)

	; Get the current date stamp
	lea	mc_DateStamp(a3),a0
	move.l	a0,d1
	move.l	gd_DOSBase(a4),a6
	jsr	_LVODateStamp(a6)
	move.l	gd_SysBase(a4),a6		; restore SysBase

	; Copy 16 longwords from the stack
	moveq.l	#15,d0
	lea	mc_StackPtr(a3),a0
	move.l	a5,a1
fillstack:
	move.l	(a1)+,(a0)+
	dbra	d0,fillstack

	; Adjust the memory block
	add.l	#INFOSIZE,a3			; Skip past the MemChunk structure

	; Fill in the pre-wall
	moveq.l	#0,d0				; Get the size of the pre-wall
	move.w	mci_PreSize(a4),d0
	moveq.l	#0,d1				; Get the fill character
	move.b	mci_FillChar(a4),d1
	bra.s	fillprewall2
fillprewall:
	move.b	d1,(a3)+
fillprewall2:
	dbra	d0,fillprewall

	; Fill in the post-wall
	move.l	a3,a2				; get the current position
	add.l	d2,a2				; go to the start of the post-wall
	move.l	d2,d0				; round the size to...
	addq.l	#7,d0				; the nearest...
	and.b	#$F8,d0				; double-longword
	sub.l	d2,d0				; subtract the real size from the rounded-up
	add.w	mci_PostSize(a4),d0		; set the counter
	move.b	mci_FillChar(a4),d1		; set the fill character
	bra.s	fillpostwall2
fillpostwall:
	move.b	d1,(a2)+
fillpostwall2:
	dbra	d0,fillpostwall

	bra	_ReallyExit

_AllocFailed:
	jsr	_LVOForbid(a6)
	move.l	d5,-(a7)			; push attributes
	move.l	d4,-(a7)			; push size
	pea	alloc1				; push the fmt string
	jsr	_kprintf
	lea	12(a7),a7			; restore the stack
_ErrorExit:
	; Already shown the error, now show
	; the process information.
	move.l	a4,a0				; GlobalData
	move.l	a5,a1				; stack pointer
	sub.l	a2,a2				; clear mc
	jsr	_ShowProcessInfo		; a0=GlobalData a1=stackptr a2=mc
	jsr	_LVOPermit(a6)

_ReallyExit:
	move.l	a3,d0				; restore the return value
	movem.l	(a7)+,d2-d7/a2-a4
	rts
