;
;		lat_macros.i
;
;


Call		MACRO
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM

SaveM		macro
			movem.l	\1,-(sp)
			endm

RestoreM	macro
			movem.l	(sp)+,\1
			endm

DECLARED		macro
			xdef _\1
			xdef \1
\1
_\1
			ENDM

DECLAREA		macro
			xdef _\1
			xdef \1
_\1
			ENDM

DECLAREL		macro
			xdef _\1
_\1
			ENDM

Call_Lib	MACRO
			movea.l	_\2,a6
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM

*	CallEx FuncName

CallEx		MACRO
			movea.l	4,a6
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM

*	CallInt	Funcname

CallInt		MACRO
			movea.l	_IntuitionBase,a6		
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM

*	CallGfx	Funcname

CallGfx		MACRO
			movea.l	_GfxBase,a6
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM

*	CallDos	Funcname

CallDos		MACRO
			movea.l	_DOSBase,a6
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM
