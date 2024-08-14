*
* io.asm
* 
* asm -iINCLUDE: -oobj/ io.asm
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "dos/dos.i"
        INCLUDE "dos/dosextens.i"
        INCLUDE "dos/dos_lib.i"

FUNCDEF	MACRO
	XREF _LVO\1
	ENDM

        INCLUDE "exec/exec_lib.i"

	INCLUDE "libhdr.i"

callsys macro
        CALLLIB _LVO\1
        endm
        
*
* blib, etc routines
*
BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM


SYSBASE	EQU	4

	XDEF	@rdch
	XDEF	_rdch
	XDEF	@wrch
	XDEF	_wrch
	XDEF	@myputch
	XDEF	_myputch

	BLIB	FGetC
	BLIB	FPutC

*
* rdch - downcoded from C to asm
*
@rdch:
_rdch:
	move.l	SYSBASE,a0
	move.l	ThisTask(a0),a0
	move.l	pr_CIS(a0),d1
	bra	@FGetC

*
* wrch - downcoded from C to asm
*
@wrch:
_wrch:
	move.l	d2,-(sp)
	move.l	d1,d2
	move.l	SYSBASE,a0
	move.l	ThisTask(a0),a0
	move.l	pr_COS(a0),d1
	bsr	@FPutC
	move.l	(sp)+,d2
	rts

* called by rawdofmt - which always writes a \0 on the end! */
*
*void ASM
*myputch (REG(d0) LONG ch, REG(a3) LONG *args)
*{
*	/* use a 1-character output buffer so we can avoid writing the null */
*	if (args[1]++ && !args[3])
*	{
*		if (FPutC(args[0],args[2]) == ENDSTREAMCH)
*			args[3] = -1;
*	}
*	args[2] = ch & 0xff;		/* passed in low byte */
*}
@myputch:
_myputch:
	move.l	d2,-(a7)
	moveq	#0,d1
	move.b	d0,d1		; character to print on next call
	move.l	8(a3),d2	; character to print this time
	move.l	d1,8(a3)	; for next call
	move.l	4(a3),d0	; count of characters written
	addq.l	#1,4(a3)	; is this the first char? increment count
	tst.l	d0
	beq.s	first_char	; if it is the first char, don't FPutC anything

	tst.l	$0c(a3)
	bne.s	first_char	; don't call FPutC after any error
	
	move.l	(a3),d1		; fh to write to
				; d2 already has character to print
	bsr	@FPutC
	moveq	#ENDSTREAMCH,d1
	cmp.l	d0,d1		; FPutC returns -1 on error
	bne.s	no_error

	move.l	d0,$0c(a3)	; set error flag

no_error:
first_char:
	move.l	(a7)+,d2
	rts


	END

