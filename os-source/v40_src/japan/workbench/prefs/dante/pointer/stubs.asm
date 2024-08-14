	include	"lvo.equ"

	XDEF _bltbmrp
	XDEF _bltbm
	XDEF _clipblit

_bltbmrp:
	jmp	_LVOBltBitMapRastPort(a6)

_bltbm:
	jmp	_LVOBltBitMap(a6)

_clipblit:
	jmp	_LVOClipBlit(a6)
