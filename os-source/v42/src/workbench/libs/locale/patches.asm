	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/lists.i"
	INCLUDE "dos/dos.i"
	INCLUDE "utility/hooks.i"

	INCLUDE "localebase.i"

	LIST

;---------------------------------------------------------------------------

	XDEF	_ToLowerPatch
	XDEF	_ToUpperPatch
	XDEF	_GetDOSStringPatch
	XDEF	_StrnicmpPatch
	XDEF	_StricmpPatch
	XDEF	_RawDoFmtPatch
	XDEF	_DateToStrStub
	XDEF	_StrToDateStub
	XDEF	_LocBase

	XREF	_DateToStrPatch
	XREF	_StrToDatePatch

;---------------------------------------------------------------------------

_LVOGetCatalogStr EQU -72
_LVOFormatString  EQU -66

_LocBase DC.L 0

;---------------------------------------------------------------------------

_GetDOSStringPatch:
	move.l	d1,-(sp)		; save string number
	move.l	_LocBase(pc),a0		; get LocaleBase
	move.l	lb_OldGetDOSString(a0),a0 ; load old dos routine address
	jsr	(a0)			; get the string from dos

	move.l	d0,a1			; use returned ptr as default
	move.l	(sp)+,d0		; get string number back
	move.l	_LocBase(pc),a6		; get LocaleBase
	move.l	lb_DOSCatalog(a6),a0	; load dos catalog address
	CALL	GetCatalogStr

	move.l	lb_DosLib(a6),a6	; restore DOSBase
	rts

;---------------------------------------------------------------------------

_ToUpperPatch:
	move.l	_LocBase(pc),a6		; load LocaleBase
	move.b	d0,d1			; clear upper byte of char
	moveq.l	#0,d0
	move.b	d1,d0
	move.l	lb_DefaultLocale(a6),a0 ; load default locale
        move.l	el_ConvToUpper(a0),a0	; load address of language driver routine
	jsr	(a0)			; do the conversion

	move.l	lb_UtilityLib(a6),a6	; restore UtilityBase
	rts

;---------------------------------------------------------------------------

_ToLowerPatch:
	move.l	_LocBase(pc),a6         ; load LocaleBase
	move.b	d0,d1			; clear upper byte of char
	moveq.l	#0,d0
	move.b	d1,d0
	move.l	lb_DefaultLocale(a6),a0 ; load default locale
        move.l	el_ConvToLower(a0),a0   ; load address of language driver routine
	jsr	(a0)                    ; do the conversion

	move.l	lb_UtilityLib(a6),a6	; restore UtilityBase
	rts

;---------------------------------------------------------------------------

_StricmpPatch:
	moveq.l	#-1,d0			; ignore length

_StrnicmpPatch:
	move.l	a2,-(sp)		; save this sucker!
	move.l	_LocBase(pc),a6         ; load LocaleBase
	moveq.l	#SC_ASCII,d1		; type of compare operation
	move.l	a1,a2			; string #2 to a2
	move.l	a0,a1			; string #1 to a1
	move.l	lb_DefaultLocale(a6),a0 ; load default locale
        move.l	el_StrnCmp(a0),a0   	; load address of language driver routine
	jsr	(a0)                    ; do the conversion

	move.l	lb_UtilityLib(a6),a6	; restore UtilityBase
	move.l	(sp)+,a2		; hey sucker, glad you're back!
	rts

;---------------------------------------------------------------------------

_RawDoFmtPatch:
        movem.l	a2/a3,-(sp)		; save work register

	sub.l	#h_SIZEOF,sp		; reserve room on the stack for a Hook
	move.l	a2,h_SubEntry(sp)	; putCharProc in Hook
	move.l	a3,h_Data(sp)		; putCharData in Hook
	lea	RDFHook(pc),a6		; hook code
	move.l	a6,h_Entry(sp)		; hook code in Hook

	move.l	_LocBase(pc),a6         ; load LocaleBase
	move.l	a1,a2			; dataStream in a2
	move.l	a0,a1			; formatString in a1
	move.l	lb_DefaultLocale(a6),a0 ; load default locale
	move.l	sp,a3                   ; a3 has hook address
	jsr	_LVOFormatString(a6)	; all set, now do the work

	add.l	#h_SIZEOF,sp		; remove room allocated for Hook
	movem.l	(sp)+,a2/a3		; restore saved registers
	move.l	lb_SysLib(a6),a6	; restore SysBase
	rts				; bye

;	RawDoFmt(FormatString, DataStream, PutChProc, PutChData);
;                a0            a1          a2         a3

;APTR ASM FormatString(REG(a0) struct ExtLocale *locale,
;                      REG(a1) STRPTR string,
;                      REG(a2) UBYTE *dataStream,
;		       REG(a3) struct Hook *putCharFunc,
;                      REG(a6) struct LocaleBase *LocaleBase);
;

RDFHook:
	move.l	a3,-(sp)		; save a3
	move.l	a0,-(sp)		; save hook pointer
	move.l	a1,d0			; char in d0
	move.l	h_Data(a0),a3		; data in a3
	move.l	h_SubEntry(a0),a0       ; get routine to call
	move.l	lb_SysLib(a6),a6	; load SysBase
	jsr	(a0)
	move.l	(sp)+,a0		; restore hook pointer
	move.l	a3,h_Data(a0)		; preserve A3 from called routine
	move.l	(sp)+,a3		; restore a3
	move.l	_LocBase(pc),a6		; restore LocaleBase

	rts

;---------------------------------------------------------------------------

_DateToStrStub:
	move.l	a6,-(sp)
	move.l	_LocBase(pc),a6
	bsr	_DateToStrPatch
	move.l	(sp)+,a6
	rts

;---------------------------------------------------------------------------

_StrToDateStub:
	move.l	a6,-(sp)
	move.l	_LocBase(pc),a6
	bsr	_StrToDatePatch
	move.l	(sp)+,a6
	rts

;---------------------------------------------------------------------------

	END
