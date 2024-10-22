	OPTIMON
	IFD BETA_VERSION

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"exec/types.i"

	LIST

;---------------------------------------------------------------------------

	XDEF	@BetaAlert

;---------------------------------------------------------------------------

	XREF	_LVOTimedDisplayAlert
	XREF	_LVOTaggedOpenLibrary
	XREF	_IntuitionBase

;---------------------------------------------------------------------------

@BetaAlert:
	moveq.l	#0,d0			 ; Clear d0 (recovery)
	lea	BetaString(pc),a0	 ; Get beta string
	moveq.l	#46,d1			 ; Height...
	move.l	#8*60,a1		 ; Timeout (8 seconds)
	move.l	a6,-(sp)
	move.l	_IntuitionBase,a6
	jsr	_LVOTimedDisplayAlert(a6)
	move.l	(sp)+,a6
	rts

BetaString:
	dc.b	$00,$90,$10
	dc.b	'BETA release for registered developers only!',$00,$01
	dc.b	$00,$F8,$20
	dc.b	'DO NOT DISTRIBUTE!',$00,$00

;---------------------------------------------------------------------------

	ENDC

	END
