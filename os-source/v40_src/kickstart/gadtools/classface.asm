*** classface.asm ********************************************************
*
*   classface.asm	-	Contains boopsi SSM routine
*
*   Copyright 1989-1992, Commodore-Amiga, Inc.
*
*   $Id: classface.asm,v 39.3 92/04/15 11:40:32 vertex Exp $
*
**************************************************************************

        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "utility/hooks.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_SSM

;---------------------------------------------------------------------------

* SSM( cl, o, msg )
_SSM:
	move.l	a2,-(sp)		; rely on a6 being preserved
	movem.l	8(sp),a0/a1/a2		; class, object, message
	exg	a1,a2			; swap message and object
	move.l	h_SIZEOF+4(a0),a0	; substitute superclass

invoke: ; --- performs call and restores a2
	pea.l	cmreturn(pc)
	move.l	h_Entry(a0),-(sp)
	rts
cmreturn:
	move.l	(sp)+,a2
	rts

;-----------------------------------------------------------------------

        END
