
*  $Id: classface.asm,v 40.0 94/02/15 17:46:58 davidj Exp $

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'

	INCLUDE 'utility/hooks.i'

	;INCLUDE 'intuall.i'		; TEST TEST TEST 

	xref	_Debug

	xdef	_CM
	xdef	_CoerceMessage
	xdef	_SM
	xdef	_SendMessage
	xdef	_SSM
	xdef	_SendSuperMessage

*
* C interface to message sending
* CM( a0: cl, a2: o, a1: msg )
*
_CM:
	move.l	a2,-(a7)	; rely on a6 being preserved
	movem.l	8(sp),a0/a2	; get class and object
	move.l	a2,d0
	beq.s	cmnullreturn
	move.l	a0,d0
	beq.s	cmnullreturn
	move.l	16(sp),a1	; get msg
	; --- registers ready, now call hook

invoke: ; --- performs call and restores a2
	pea.l	cmreturn(pc)
	move.l	h_Entry(a0),-(sp)
	rts
cmnullreturn:
	moveq.l	#0,d0
cmreturn:
	move.l	(sp)+,a2
	rts
	
*
* C interface to message sending
* CoerceMessage( cl, o, arg1, arg2, arg3 )
*
_CoerceMessage:
	move.l	a2,-(a7)	; rely on a6 being preserved
	movem.l	8(sp),a0/a2	; get hook and object
	move.l	a2,d0
	beq.s	cmnullreturn
	move.l	a0,d0
	beq.s	cmnullreturn
	lea	16(sp),a1	; varargs version
	; --- registers ready, now call hook
	bra.s	invoke(pc)
	; ----- don't return here

* SM( o, msg )
_SM:
	move.l	a2,-(a7)	; rely on a6 being preserved
	move.l	8(sp),a2	; object
	move.l	a2,d0
	beq.s	cmnullreturn
	move.l	12(sp),a1	; message
	move.l	-4(a2),a0	; object class precedes object data

	bra.s	invoke(pc)	; will cleanup a2
	; ----- don't return here

* SendMessage( o, arg1, arg2, ... )
_SendMessage:
	move.l	a2,-(a7)	; rely on a6 being preserved
	move.l	8(sp),a2	; object
	move.l	a2,d0
	beq.s	cmnullreturn
	lea	12(sp),a1	; message
	move.l	-4(a2),a0	; object class precedes object data

	bra.s	invoke(pc)	; will cleanup a2
	; ----- don't return here

* SSM( cl, o, msg )
_SSM:
	move.l	a2,-(a7)	; rely on a6 being preserved
	movem.l	8(sp),a0/a2	; class, object
	move.l	16(sp),a1	; message
	move.l	h_SIZEOF+4(a0),a0	; substitute superclass

	bra.s	invoke(pc)	; will cleanup a2
	; ----- don't return here

* SendSuperMessage( cl, o, arg1, arg2 )
_SendSuperMessage:
	move.l	a2,-(a7)	; rely on a6 being preserved
	movem.l	8(sp),a0/a2	; class, object
	lea	16(sp),a1	; message
	move.l	h_SIZEOF+4(a0),a0	; substitute superclass

	bra.s	invoke(pc)	; will cleanup a2
	; ----- don't return here


	end

