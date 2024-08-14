**      $Id: mstartup.asm,v 1.11 90/12/14 09:08:59 ewout Exp Locker: ewout $
**
**      Mungwall startup code.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "hardware/cia.i"
	INCLUDE "dos/dosextens.i"
	INCLUDE "mungwall_rev.i"
	section code

	XREF	_new_AllocMem
	XREF	_new_FreeMem
	XREF	_intena
	XREF	_startup
	XREF    _FindLayers
	XREF    _HandleSignal
	XREF    _MakeTask
	XREF    _CreateTask
	XREF	_LVOAllocMem
	XREF	_LVOFreeMem
	XREF	_LVOSetFunction
	XREF	_LVOEnable
	XREF	_LVODisable
	XREF    _LVOForbid
	XREF    _LVOPermit

; ---
	XDEF	_SysBase
	XDEF	_grab_em
	XDEF	_free_em
	XDEF	_old_AllocMem
	XDEF	_old_FreeMem
	XDEF	_PreSize
	XDEF	_PostSize
	XDEF	_FillChar
	XDEF	_Snoop
	XDEF	_romtag
	XDEF    _LayersStart
	XDEF    _LayersEnd
	XDEF    _ErrorWait
	XDEF    _Taskname
;	XDEF    _StackPtr
	XDEF    _MungPort
	XDEF    _Except
	XDEF    _ConfigInfo
	XDEF    _VerTitle
	XDEF	_PreMunging
	XDEF	_PostMunging
	XDEF	_InitMunging

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
        move.l  #8000,-(sp)           ; stack for task
        move.l   #_HandleSignal,-(sp) ; initPC
        move.l  #0,-(sp)              ; priority
        pea     mname                 ; task name
        jsr     _CreateTask
        addq.l  #8,sp
        addq.l  #8,sp
        tst.l   d0
        beq     all_done
	bsr 	_grab_em
        bra     all_done

_grab_em:
	move.l	a6,-(sp)
	move.l	$4,a6

        jsr     _FindLayers

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

newAllocMem:
        jsr     _LVOForbid(a6)
        movem.l	a2,-(sp)
        lea		4(sp),a2
; _StackPtr
        jsr     _new_AllocMem
        movem.l (sp)+,a2
        jmp     _LVOPermit(a6)

newFreeMem:
        jsr     _LVOForbid(a6)
        movem.l	a2,-(sp)
        lea		4(sp),a2
; _StackPtr
        jsr     _new_FreeMem
        movem.l	(sp)+,a2
        jmp     _LVOPermit(a6)

_InitMunging:
		move.l #$ABADCAFE,a0
		bra.s Mung

_PreMunging:
		move.l #$DEADF00D,a0
		bra.s Mung

_PostMunging:
		move.l #$DEADBEEF,a0
Mung:
		;   A1=Base Address
		;   D0=raw size
		;
		move.l	a1,d1
		beq.s	BadCall
		move.l	d0,d1
		beq.s	BadCall
		move.l	a0,d1
		addq.l	#7,d0
		lsr.l	#3,d0
		beq.s	BadCall
		bra.s	inloop
loop:	move.l	d1,(a1)+
		move.l	d1,(a1)+
inloop: dbra	d0,loop
		sub.l	#$10000,d0
		bpl.s	loop
BadCall:
		rts

;_StackPtr		dc.l	0
_SysBase		dc.l	0
_old_FreeMem:	dc.l	0
_old_AllocMem:	dc.l	0
_ret_FreeMem:	dc.l	0
_ret_AllocMem:	dc.l	0
_PreSize:		dc.l	32
_PostSize:		dc.l	32
_LayersStart:   dc.l    0
_LayersEnd:     dc.l    0
_VerTitle:      dc.l    mid
_Taskname:      dc.l    0
_MungPort:      dc.l    0
_Except:        dc.w    0
_ErrorWait:     dc.w    0
_Snoop:			dc.w	0
_FillChar:		dc.b	$bb
_ConfigInfo:    dc.b    0

_romtag:
	DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
	DC.L	_romtag				;(52) APTR  RT_MATCHTAG
	DC.L	_end				;(56) APTR  RT_ENDSKIP
	DC.B	RTW_COLDSTART		;(5A) UBYTE RT_FLAGS
	DC.B	0					;(5B) UBYTE RT_VERSION
	DC.B	0					;(5C) UBYTE RT_TYPE
	DC.B	107					;(5D) BYTE  RT_PRI
	DC.L	mname				;(5E) APTR  RT_NAME
	DC.L	mid     			;(62) APTR  RT_IDSTRING
	DC.L	Entry				;(66) APTR  RT_INIT
								;(6A) LABEL RT_SIZE
mname:          DC.B    "Mungwall",0
mid:            VSTRING
mversion:       VERSTAG

_end:	; I don't care

        END
