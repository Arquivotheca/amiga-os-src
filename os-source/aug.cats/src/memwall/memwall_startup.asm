        INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "hardware/cia.i"
	INCLUDE "dos/dosextens.i"

	section code

	XREF	_MyAllocMem
	XREF	_MyFreeMem
	XREF	_intena
	XREF	_startup

	XREF	_LVOAllocMem
	XREF	_LVOFreeMem
	XREF	_LVOSetFunction
	XREF	_LVOEnable
	XREF	_LVODisable

	XDEF	_SysBase
	XDEF	_grab_em
	XDEF	_free_em
	XDEF	_old_AllocMem
	XDEF	_old_FreeMem
	XDEF	_presize
	XDEF	_postsize
	XDEF	_fillchar
	XDEF	_process
	XDEF	_snoop
	XDEF	_supersnoop
	XDEF	_romtag

Entry:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a0,-(a7)	; save args

	;-- may be executed or resident
	move.l	$4,a6		; a6 = execbase already if resident, else
				; I'm a process and can trash it.
	move.l	a6,_SysBase


	;--- if not a process (started from reboot?) just wedge in
	move.l	ThisTask(a6),a0
	cmp.b	#NT_PROCESS,LN_TYPE(a0)
	bne.s	reinstall

	;-- else call startup code in C (ptr to args on stack)
	jsr	_startup
	tst.l	d0
	bra	all_done


reinstall:
	bsr 	_grab_em
 	bra	all_done

_grab_em:
	move.l	a6,-(sp)
	move.l	$4,a6

	jsr	_LVODisable(a6)
	lea	newAllocMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOAllocMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_old_AllocMem

	lea	newFreeMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOFreeMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_old_FreeMem

	jsr	_LVOEnable(a6)
	move.l	(sp)+,a6
	rts


_free_em:
	move.l	a6,-(sp)
	move.l	$4,a6

	jsr	_LVODisable(a6)
	move.l	_old_AllocMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOAllocMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_ret_AllocMem

	move.l	_old_FreeMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOFreeMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_ret_FreeMem

	cmp.l	#newFreeMem,d0
	bne.s	nofree
	move.l	_ret_AllocMem,d0
	cmp.l	#newAllocMem,d0
	bne.s	nofree
	bra.s	freeok

nofree:
	* we didn't get back our wedges, so we'll restore what was in there
	move.l	_ret_AllocMem,a0
	move.l	a0,d0
	move.l	#_LVOAllocMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)

	move.l	_ret_FreeMem,a0
	move.l	a0,d0
	move.l	#_LVOFreeMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)

	move.l	#0,d0		; failure
	bra.s	outahere
freeok:
	move.l	#1,d0		; success
outahere:
	jsr	_LVOEnable(a6)
	move.l	(sp)+,a6
	rts

	
all_done:
	addq.l	#4,a7
	movem.l	(sp)+,d2-d7/a2-a6
	moveq	#0,d0
	rts

* give return address for supersnoop
newAllocMem:
	move.l	(a7),a0
	jsr	_MyAllocMem(PC)
	rts

newFreeMem:
	move.l	(a7),a0
	jsr	_MyFreeMem(PC)
	rts

_SysBase	dc.l	0
_old_FreeMem:	dc.l	0
_old_AllocMem:	dc.l	0
_ret_FreeMem:	dc.l	0
_ret_AllocMem:	dc.l	0

_presize:	dc.l	32
_postsize:	dc.l	32
_process	dc.l	0
_snoop:		dc.l	0
_supersnoop:	dc.l	0
_fillchar:	dc.b	$bb

*
	CNOP	0,2
_romtag:
	DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
	DC.L	_romtag			;(52) APTR  RT_MATCHTAG
	DC.L	_end			;(56) APTR  RT_ENDSKIP
	DC.B	RTW_COLDSTART		;(5A) UBYTE RT_FLAGS
	DC.B	0			;(5B) UBYTE RT_VERSION
	DC.B	0			;(5C) UBYTE RT_TYPE
	DC.B	107			;(5D) BYTE  RT_PRI
	DC.L	NAME			;(5E) APTR  RT_NAME
	DC.L	idtag			;(62) APTR  RT_IDSTRING
	DC.L	Entry			;(66) APTR  RT_INIT
					;(6A) LABEL RT_SIZE

NAME:	DC.B	"memwall",0
idtag:	DC.B	"memwall 36.11 (6/18/90)",13,10,0

_end:	; I don't care

        END
