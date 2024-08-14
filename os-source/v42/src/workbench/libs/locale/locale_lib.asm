	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/alerts.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"libraries/dos.i"

	INCLUDE	"locale_rev.i"
	INCLUDE "localebase.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_ARexxHostLVO
	XREF	_CloseCatalogLVO
	XREF	_CloseLocaleLVO
	XREF	_ConvToLowerLVO
	XREF	_ConvToUpperLVO
	XREF	_FlushLib
	XREF	_FormatDateLVO
	XREF	_FormatStringLVO
	XREF	_GetCatalogStrLVO
	XREF	_GetLocaleStrLVO
	XREF	_IsAlNumLVO
	XREF	_IsAlphaLVO
	XREF	_IsCntrlLVO
	XREF	_IsDigitLVO
	XREF	_IsGraphLVO
	XREF	_IsLowerLVO
	XREF	_IsPrintLVO
	XREF	_IsPunctLVO
	XREF	_IsSpaceLVO
	XREF	_IsUpperLVO
	XREF	_IsXDigitLVO
	XREF	_OpenCatalogALVO
	XREF	_OpenLocaleLVO
	XREF	_ParseDateLVO
	XREF	_SetDefaultLocaleLVO
	XREF	_StrConvertLVO
	XREF	_StrnCmpLVO

	XREF	_LVOAlert
	XREF	_LVOCloseLibrary
	XREF	_LVOFreeMem
	XREF	_LVOInitSemaphore
	XREF	_LVOOpenLibrary

	XREF    ENDCODE

;---------------------------------------------------------------------------

	XDEF	LibInit
	XDEF	LibOpen
	XDEF	LibClose
	XDEF	LibExpunge
	XDEF	LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
	moveq   #-1,d0
	rts

;-----------------------------------------------------------------------

ROMTAG:
	DC.W    RTC_MATCHWORD		; UWORD RT_MATCHWORD
	DC.L    ROMTAG			; APTR  RT_MATCHTAG
	DC.L    ENDCODE			; APTR  RT_ENDSKIP
	DC.B    RTF_AUTOINIT		; UBYTE RT_FLAGS
	DC.B    VERSION			; UBYTE RT_VERSION
	DC.B    NT_LIBRARY		; UBYTE RT_TYPE
	DC.B    0			; BYTE  RT_PRI
	DC.L    LibName			; APTR  RT_NAME
	DC.L    LibId			; APTR  RT_IDSTRING
	DC.L    LibInitTable		; APTR  RT_INIT

LibName DC.B 'locale.library',0
LibId   VSTRING

	CNOP	0,4

LibInitTable:
	DC.L	LocaleLib_SIZEOF
	DC.L	LibFuncTable
	DC.L	LibDataTable
	DC.L	LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
	V_DEF	LibOpen
	V_DEF	LibClose
	V_DEF	LibExpunge
	V_DEF	LibReserved
	V_DEF	ARexxHost
	V_DEF	_CloseCatalogLVO
	V_DEF	_CloseLocaleLVO
	V_DEF	_ConvToLowerLVO
	V_DEF	_ConvToUpperLVO
	V_DEF	_FormatDateLVO
	V_DEF	_FormatStringLVO
	V_DEF	_GetCatalogStrLVO
	V_DEF	_GetLocaleStrLVO
	V_DEF	_IsAlNumLVO
	V_DEF	_IsAlphaLVO
	V_DEF	_IsCntrlLVO
	V_DEF	_IsDigitLVO
	V_DEF	_IsGraphLVO
	V_DEF	_IsLowerLVO
	V_DEF	_IsPrintLVO
	V_DEF	_IsPunctLVO
	V_DEF	_IsSpaceLVO
	V_DEF	_IsUpperLVO
	V_DEF	_IsXDigitLVO
	V_DEF	_OpenCatalogALVO
	V_DEF	_OpenLocaleLVO
	V_DEF	_ParseDateLVO
	V_DEF	_SetDefaultLocaleLVO
	V_DEF	_StrConvertLVO
	V_DEF	_StrnCmpLVO
	V_DEF	LocReserved0
	V_DEF	LocReserved1
	V_DEF	LocReserved2
	V_DEF	LocReserved3
	V_DEF	LocReserved4
	V_DEF	LocReserved5
	DC.W   -1

LibDataTable:
	INITBYTE   LN_TYPE,NT_LIBRARY
	INITLONG   LN_NAME,LibName
	INITBYTE   LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
	INITWORD   LIB_VERSION,VERSION
	INITWORD   LIB_REVISION,REVISION
	INITLONG   LIB_IDSTRING,LibId
	DC.W       0

	CNOP	0,4

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
	movem.l	a0/a5/d7,-(sp)
	move.l	d0,a5
	move.l	a6,lb_SysLib(a5)
	move.l	a0,lb_SegList(a5)

	move.l	#AG_OpenLib!AO_DOSLib,D7
	lea	DOSName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,lb_DosLib(a5)
	beq.s   FailInit

	move.l	#AG_OpenLib!AO_UtilityLib,d7
	lea	UtilityName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,lb_UtilityLib(a5)
	beq.s   FailInit

	move.l	#AG_OpenLib!AO_Intuition,d7
	lea	IntuitionName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,lb_IntuitionLib(a5)
	beq.s   FailInit

	lea	lb_LibLock(a5),a0
	CALL	InitSemaphore
	lea	lb_Catalogs(a5),a0
	NEWLIST	a0

	move.l	a5,d0
	movem.l	(sp)+,a0/a5/d7
	rts

FailInit:
	bsr	CloseLibs
        CALL	Alert
	movem.l	(sp)+,a0/a5/d7
	moveq	#0,d0
	rts


LIBVERSION    EQU  37
UtilityName   DC.B "utility.library",0
DOSName       DC.B "dos.library",0
IntuitionName DC.B "intuition.library",0

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has LocaleBase, task switching is disabled
; Returns 0 for failure, or LocaleBase for success.
LibOpen:
	addq.w	#1,lb_UsageCnt(a6)
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	move.l	a6,d0
	rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has LocaleBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,lb_UsageCnt(a6)

	; if delayed expunge bit set, then try to get rid of the library
	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	bne.s	CloseExpunge

	; delayed expunge not set, so stick around
	moveq	#0,d0
	rts

CloseExpunge:
	; if no library users, then just remove the library
	tst.w	lb_UsageCnt(a6)
	beq.s	DoExpunge

	; still some library users, so just flush unused resources
	bsr	_FlushLib
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has LocaleBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
	bsr	_FlushLib

	tst.w	lb_UsageCnt(a6)
	beq.s	DoExpunge

	bset	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

DoExpunge:
	movem.l	d2/a5/a6,-(sp)
	move.l	a6,a5
	move.l	lb_SysLib(a5),a6
	move.l	lb_SegList(a5),d2

	move.l  a5,a1
	REMOVE

	bsr.s	CloseLibs

	move.l	a5,a1
	moveq	#0,d0
	move.w	LIB_NEGSIZE(a5),d0
	sub.l	d0,a1
	add.w	LIB_POSSIZE(a5),d0
	CALL	FreeMem

	move.l	d2,d0
	movem.l	(sp)+,d2/a5/a6
	rts

;-----------------------------------------------------------------------

CloseLibs:
	move.l	lb_DosLib(a5),a1
	CALL	CloseLibrary

	move.l	lb_UtilityLib(a5),a1
	CALL	CloseLibrary

	move.l	lb_IntuitionLib(a5),a1
	GO	CloseLibrary

;-----------------------------------------------------------------------

LibReserved:
LocReserved0:
LocReserved1:
LocReserved2:
LocReserved3:
LocReserved4:
LocReserved5:
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

ARexxHost:
	subq.l	#4,sp		; room where to put result
	move.l	sp,a1
	bsr	_ARexxHostLVO	; call the real function
	movea.l	(sp)+,a0	; put the result string into A0
	rts			; bye

;-----------------------------------------------------------------------

	END
