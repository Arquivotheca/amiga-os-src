;*****************************************************************************/
; freemem.asm
;*****************************************************************************/

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE "globaldata.i"

;*****************************************************************************/

	XDEF	_newFreeMem

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_LVODeallocate
	XREF	_LVOTypeOfMem
	XREF	_LVOSignal
	XREF	_MemoryDump
	XREF	_ShowProcessInfo
	XREF	_MungMem
	XREF	_kprintf
	XREF	KPutStr

;*************************************************************************************

	XREF	free
	XREF	free0
	XREF	free1
	XREF	free2
	XREF	free3
	XREF	free4
	XREF	free5
	XREF	free6
	XREF	free7
	XREF	free8
	XREF	free9

	XREF	_gd

;*************************************************************************************
; void ASM newFreeMem (REG (a1) UBYTE * memb, REG (d0) ULONG size, REG (a2) ULONG *stackptr);
;*************************************************************************************
;	D2	ULONG size
;	D3	-- temporary --
;	D4	UBYTE *omemb
;	D5	ULONG osize
;	D6	ULONG errorcnt
;
;	A3	UBYTE *memb
;	A4	struct MemCheck *
;	A5	struct GlobalData *
;	A6	struct Library *SysBase
;*************************************************************************************
_newFreeMem:
	movem.l	d2-d6/a3-a5,-(a7)
	moveq.l	#0,d6				; clear the error counter
	sub.l	a4,a4				; clear MemCheck
	move.l	a1,d4				; Copy address to omemb
	move.l	d0,d5				; Copy size to osize
	move.l	_gd,a5				; Get a copy of the GlobalData

	; if (memb == NULL)
	move.l	a1,a3				; Copy address
	move.l	a1,d1
	beq	_FreeMem_Zero			; Attempt to free address 0

	; if ((ULONG) memb != (((ULONG) memb + 7) & ~0x07))
	add.l	#7,d1				; See if memory...
	and.l	#$FFFFFFF8,d1			; is misaligned...
	cmp.l	a1,d1
	bne	_FreeMem_Misaligned		; Misaligned pointer

	; if (size == 0)
	move.l	d0,d2				; Copy size
	beq	_FreeMem_SizeZero		; Attempt to free size of zero
	jsr	_LVOTypeOfMem(a6)
	tst.l	d0
	beq	_FreeMem_Invalid		; Attempt to free invalid memory

	; Restore variables
	move.l	a3,a1				; restore memb
	move.l	d2,d0				; restore size

	; See if we have a cookie
	move.l	a3,a0				; Get a pointer to the MemCheck structure
	suba.l	#INFOSIZE,a0			; mc = memb - INFOSIZE -
	moveq.l	#0,d1				;      mci->mci_PreSize
	move.w	mci_PreSize(a5),d1
	sub.l	d1,a0
 	move.l	#$C0DE0000,d1			; do it in two steps so magic cookie...
	move.w	#$F00D,d1			; doesn't appear in this code
	cmp.l	(a0),d1				; Do we have a cookie here?
	bne	_FreeMem			; No cookie here

	; Set new memory block
	move.l	a0,a4
	move.l	a4,a3

	; Generate cookie checksum
	moveq.l	#0,d1				; clear errors
	move.l	a4,a0
	add.l	(a0)+,d1
	add.l	(a0)+,d1
	add.l	(a0)+,d1
	add.l	(a0)+,d1
	add.l	(a0),d1
	cmp.l	mc_CheckSum(a4),d1
	beq.s	_FreeMem_MC_1

	; show checksum failure
	move.l	d4,a3				; use original block
	move.l	d5,d2				; use original size
	sub.l	a4,a4				; clear the MemCheck structure
	bra	_FreeMem

_FreeMem_MC_1:

	; See if correct size specified
	cmp.l	mc_Size(a4),d5
	beq.s	_FreeMem_MC_2

	; show incorrect size
	jsr	_ShowFreeHeader			; Show the common portion
	move.l	mc_Size(a4),-(a7)		; push the alloc size
	move.l	d5,-(a7)			; push the free size
	pea	free6				; push the error string
	jsr	_kprintf			; print it
	lea	12(a7),a7			; restore the stack
	move.l	mc_Size(a4),d0			; set the right size
	moveq.l	#1,d6				; set the error flag

_FreeMem_MC_2:
	; Adjust the size
	add.l	#INFOSIZE,d0			; size = size + INFOSIZE +
	moveq.l	#0,d1				;        mci->mci_PreSize +
	move.w	mci_PreSize(a5),d1
	add.l	d1,d0
	moveq.l	#0,d1				;        mci->mci_PostSize +
	move.w	mci_PostSize(a5),d1
	add.l	d1,d0
	move.l	d0,d2				; This is now the new size

_FreeMem_MC_3:
	; Check the pre-wall for damage
	moveq.l	#0,d3				; Clear the error counter
	move.l	a4,a0				; Where are we
	add.l	#INFOSIZE,a0			; Jump to the beginning of the pre-wall
	move.b	mci_FillChar(a5),d0		; Get the fill character
	move.w	mci_PreSize(a5),d1		; Get the presize
	bra.s	__PreW_End
__PreW_Loop:
	cmp.b	(a0)+,d0
	beq.s	__PreW_End
	addq.l	#1,d3				; Increment the error counter
__PreW_End:
	dbra	d1,__PreW_Loop
	tst.l	d3				; Were there any errors
	beq.s	_FreeMem_MC_4			; No there wasn't so go to the next step

	; show bad pre-wall
	jsr	_ShowFreeHeader			; Show the common portion
	move.l	d3,-(a7)			; push the number of errors
	pea	free7				; push the error string
	jsr	_kprintf			; print it
	lea	8(a7),a7			; restore the stack
	moveq.l	#1,d6				; set the error flag

	; need to do the memory dump now
	lea	wall,a0				; get the prefix
	move.l	a0,d0
	move.l	a5,a0				; get the GlobalData
	move.l	a4,a1				; get the pointer
	add.l	#INFOSIZE,a1
	moveq.l	#0,d1
	move.w	mci_PreSize(a5),d1
	jsr	_MemoryDump			; a0=gd, d0=prefix, a1=ptr, d1=size

_FreeMem_MC_4:
;	bra	_FreeMem_MC_5
	; Check the post-wall for damage
	moveq.l	#0,d3				; Clear the error counter
	move.l	d4,a0				; start with the beginning of the memory block
	add.l	mc_Size(a4),a0			; add the memory block size
	move.b	mci_FillChar(a5),d0		; Get the fill character
	move.w	mci_PostSize(a5),d1		; Get the post size
	bra.s	__PostW_End
__PostW_Loop:
	cmp.b	(a0)+,d0
	beq.s	__PostW_End
	addq.l	#1,d3				; Increment the error counter
__PostW_End:
	dbra	d1,__PostW_Loop
	tst.l	d3				; Were there any errors
	beq.s	_FreeMem_MC_5			; No there wasn't so go to the next step

	; show bad post-wall
	jsr	_ShowFreeHeader			; Show the common portion
	move.l	d3,-(a7)			; push the number of errors
	pea	free8				; push the error string
	jsr	_kprintf			; print it
	lea	8(a7),a7			; restore the stack
	moveq.l	#1,d6				; set the error flag

	; need to do the memory dump now
	lea	wall,a0				; get the prefix
	move.l	a0,d0				; get it into the appropriate register
	move.l	a5,a0				; get the GlobalData
	move.l	d4,a1				; start with the beginning of the memory block
	add.l	mc_Size(a4),a1			; add in the size
	moveq.l	#0,d1				; clear the upper word of the size
	move.w	mci_PostSize(a5),d1		; load the lower word of the size
	jsr	_MemoryDump			; a0=gd, d0=prefix, a1=ptr, d1=size

_FreeMem_MC_5
	; Get rid of the cookie
	move.l	#$C0DEFEED,(a4)

_FreeMem:
	; If there were any errors, than show
	; process information
	jsr	_SPI
_FreeMem1:
	; If we're not forbidden, then mung memory
	tst.b	TDNestCnt(a6)
	beq.s	_FreeMem2

	; We are in forbid, so add the memory to our list
	move.l	gd_MemHeader(a5),a0		; set the MemHeader
	move.l	a3,a1				; set the MemChunk
	move.l	d2,d0				; set the size
	jsr	_LVODeallocate(a6)		; a0=MemHeader, a1=MemChunk, d0=Size

	; Tell our parent process about this
	move.l	gd_Process(a5),a1
	move.l	#SIGBREAKF_CTRL_F,d0
	jsr	_LVOSignal(a6)
	bra	_FreeMem_End

_FreeMem2:
	; Mung the memory block
	move.l	a3,a0				; memory block
	move.l	d2,d0				; size
	move.l	#$DEADBEEF,d1			; fill value
	jsr	_MungMem			; a0=ptr, d0=size, d1=value

	; Search for the MemHeader the block
	; belongs in.
	move.l	a3,a1				; restore memb
	move.l	d2,d0				; restore size
	lea	MemList(a6),a0			; beginning of free list array

_FreeMem_Next:
	TSTNODE a0,a0				; Get the next MemHeader
	beq	_CorruptList			; Corrupt free list or bad args

	; See if address belongs to this
	; MemHeader
	cmp.l   MH_LOWER(a0),a1
	bcs.s   _FreeMem_Next
	cmp.l   MH_UPPER(a0),a1
	bcc.s   _FreeMem_Next

	; Found the MemHeader
	jsr	_LVODeallocate(a6)		; a0=MemHeader, a1=MemChunk, d0=Size

_FreeMem_End:
	jsr	_SPI
	movem.l	(a7)+,d2-d6/a3-a5
	rts

;*************************************************************************************

_SPI:
	; If there were any errors, than show
	; process information
	move.l	d6,d0
	beq.s	_SPI_Exit
	move.l	a2,-(a7)			; save A2
	move.l	a5,a0				; GlobalData
	move.l	a2,a1				; stack pointer
	move.l	a4,a2				; set mc
	jsr	_ShowProcessInfo		; a0=GlobalData a1=stackptr a2=mc
	move.l	(a7)+,a2			; restore A2
	moveq.l	#0,d6				; clear the error flag
_SPI_Exit:
	rts

;*************************************************************************************

_FreeMem_Zero:
	lea	free0,a0
	bra	_ShowError

_FreeMem_SizeZero:
	lea	free1,a0
	bra	_ShowError

_FreeMem_Invalid:
	lea	free2,a0
	bra	_ShowError

_FreeMem_Misaligned:
	lea	free3,a0
	bra	_ShowError

_FreeMem_Checksum:
	lea	free4,a0
	bra	_ShowError

_FreeMem_OldCookie:
	lea	free5,a0
	bra	_ShowError

_CorruptList:
	lea	free9,a0
	bra	_ShowError

_ShowError:
	jsr	_ShowFreeHeader			; Show the common portion
	jsr	KPutStr				; Show the error string
	moveq.l	#1,d6				; set error flag
	bra	_FreeMem_End

;*************************************************************************************
; void ASM ShowFreeHeader (REG (d4) UBYTE *ptr, REG (d5) ULONG size);
;*************************************************************************************
_ShowFreeHeader:
	movem.l	d0/d1/a0/a1,-(a7)
	move.l	d5,-(a7)			; push the size
	move.l	d4,-(a7)			; push the memory block
	pea	free				; push the header string
	jsr	_kprintf			; kprintf
	lea	12(a7),a7			; restore the stack
	movem.l	(a7)+,d0/d1/a0/a1
	rts

;*************************************************************************************

wall	dc.b	'Wall:',0

