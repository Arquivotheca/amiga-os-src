        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"devices/prtbase.i"

	INCLUDE "postscript_rev.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_CommandTable
	XREF	_ExtendedCharTable

	XREF	_DriverOpen
	XREF	_DriverClose
	XREF    _DriverRender
	XREF	_DriverDoSpecial
	XREF	_DriverConvert

	XREF	_LVOOldOpenLibrary
	XREF	_LVOCloseLibrary

	XREF	_PD
	XREF	_SysBase
	XREF    _DOSBase
	XREF	_GfxBase
	XREF	_UtilityBase

;---------------------------------------------------------------------------

	XDEF	_PED

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
	moveq	#-1,d0
	rts

;---------------------------------------------------------------------------

	DC.W	35  			; VERSION
	DC.W	0			; REVISION

_PED:
	DC.L	DriverName
	DC.L	DriverInit
	DC.L	DriverExpunge
	DC.L	_DriverOpen
	DC.L	_DriverClose
	DC.B	PPC_BWGFX		; Printer class
	DC.B	PCC_BW			; Color class
	DC.B	0			; MaxColumns
	DC.B	8			; NumCharSets
	DC.W	1			; NumRows
	DC.L	0       		; MaxXDots
	DC.L	0	    		; MaxYDots
	DC.W	300			; XDotsInch
	DC.W	300			; YDotsInch
	DC.L	_CommandTable		; Commands
	DC.L	_DriverDoSpecial
	DC.L	_DriverRender
	DC.L	300
	DC.L	_ExtendedCharTable	; 8BitChars
	DC.L	1
	DC.L	_DriverConvert

;---------------------------------------------------------------------------

DriverInit:
	move.l	4(sp),_PD

	move.l	a6,-(sp)
	move.l	4,a6
	move.l	a6,_SysBase

	cmp.w	#39,LIB_VERSION(a6)
	bge.s	1$

	; not running in V39, quit
	move.l	(sp)+,a6
	moveq	#1,d0
	rts

1$:	; open DOS
	lea	DOSName(pc),a1
	jsr	_LVOOldOpenLibrary(a6)
	move.l	d0,_DOSBase

	; open gfx
	lea	GfxName(pc),a1
	jsr	_LVOOldOpenLibrary(a6)
	move.l	d0,_GfxBase

	; open utility
	lea	UtilityName(pc),a1
	jsr	_LVOOldOpenLibrary(a6)
	move.l	d0,_UtilityBase

	move.l	(sp)+,a6
	moveq	#0,d0
	rts

;---------------------------------------------------------------------------

DriverExpunge:
	move.l	a6,-(sp)
	move.l	_SysBase,a6

	move.l	_DOSBase,a1
	jsr	_LVOCloseLibrary(a6)

	move.l	_GfxBase,a1
	jsr 	_LVOCloseLibrary(a6)

	move.l  _UtilityBase,a1
	jsr	_LVOCloseLibrary(a6)

	move.l	(sp)+,a6
	moveq.l	#0,d0
	rts

*---------------------------------------------------

DriverName  VERSTAG
DOSName     DC.B 'dos.library',0
GfxName     DC.B 'graphics.library',0
UtilityName DC.B 'utility.library',0

;---------------------------------------------------------------------------

	END
