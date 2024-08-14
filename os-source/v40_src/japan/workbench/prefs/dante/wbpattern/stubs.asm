	include	"lvo.equ"

	XDEF _bltbmrp
	XDEF _bltbm

_bltbmrp:
	jsr	_LVOBltBitMapRastPort(a6)
	rts

_bltbm
	jsr	_LVOBltBitMap(a6)
	rts
