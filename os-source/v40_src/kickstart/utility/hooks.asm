        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"hooks.i"

	LIST

;---------------------------------------------------------------------------

	XDEF	CallHookPkt

;---------------------------------------------------------------------------

CallHookPkt:
	move.l	h_Entry(a0),-(sp)
	rts