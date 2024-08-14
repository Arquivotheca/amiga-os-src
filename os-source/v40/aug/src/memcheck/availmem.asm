;*************************************************************************************
; availmem.asm
;*************************************************************************************

	INCLUDE "exec/types.i"
	INCLUDE "globaldata.i"

;*************************************************************************************

	XDEF	_nAvailMem

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit

;*************************************************************************************

	XREF	_gd

;/*****************************************************************************/
;
;ULONG ASM newAvailMem (REG (d1) ULONG attrs, REG (a2) ULONG * stackptr)
;{
;    struct MemCheckInfo *mci = &gd->gd_MemCheckInfo;
;    ULONG retval, mungsize;
;
;    retval = (*gd->gd_AvailMem) (attrs, SysBase);
;    mungsize = INFOSIZE + mci->mci_PreSize + mci->mci_PostSize;
;    if (retval > mungsize)
;	retval -= mungsize;
;
;    return (retval);
;}

;*************************************************************************************
; ULONG ASM nAvailMem (REG (d1) ULONG attrs, REG (a6) struct Library *SysBase);
;*************************************************************************************

_nAvailMem:
	jsr	_LVOForbid(a6)
	movem.l	d2/a2,-(a7)

	move.l	_gd,a2			; get a pointer to our global data

	move.l	gd_AvailMem(a2),a0	; call the system AvailMem function
	jsr	(a0)

	move.l	#INFOSIZE,d2		; d2=mungsize = INFOSIZE +
	moveq.l	#0,d1			;               mci->mci_PreSize +
	move.w	mci_PreSize(a2),d1
	add.l	d1,d2
	moveq.l	#0,d1			;               mci->mci_PostSize +
	move.w	mci_PostSize(a2),d1
	add.l	d1,d2
	cmp.l	d2,d0			; if (d0=retval > d2=mungsize)
	bls.b	_Done
	sub.l	d2,d0			; d0=retval -= d2=mungsize
_Done:
	movem.l	(a7)+,d2/a2
	jmp	_LVOPermit(a6)
