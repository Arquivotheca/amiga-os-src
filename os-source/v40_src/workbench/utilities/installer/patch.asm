
		section	patch.asm,CODE

		include 'graphics/rastport.i'

		xdef	_mySetSoftStyle
		xref	_LVOSetSoftStyle
		xref	_GfxBase

* SetSoftStyle(rp,style,enable)(a1,d0/d1)

_mySetSoftStyle
		move.l	a2,-(sp)
		move.l	8(sp),a1
		movem.l	12(sp),d0/d1
		move.l	rp_Font(a1),a2
		move.l	_GfxBase,a6
		jsr		_LVOSetSoftStyle(a6)
		move.l	(sp)+,a2
		rts

		end
