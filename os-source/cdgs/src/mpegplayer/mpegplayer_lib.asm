        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "internal/librarytags.i"

        INCLUDE "mpegplayer_rev.i"
        INCLUDE "mpegplayerbase.i"

        LIST

;---------------------------------------------------------------------------

	XREF	_CDPlayerLVO
	XREF	ENDCODE

	XREF	_LVOTaggedOpenLibrary
	XREF	_LVOOpenLibrary
	XREF	_LVOCloseLibrary
	XREF	_LVOFreeMem

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
; Not useful for a ROM module
;Start:
;        moveq   #-1,d0
;        rts

;---------------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD              ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                     ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                    ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT 		   ; UBYTE RT_FLAGS
        DC.B    VERSION                    ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY                 ; UBYTE RT_TYPE
        DC.B    0                          ; BYTE  RT_PRI
        DC.L    MPEGPlayerName             ; APTR  RT_NAME
        DC.L    MPEGPlayerId               ; APTR  RT_IDSTRING
        DC.L    LibInitTable	           ; APTR  RT_INIT

MPEGPlayerName DC.B 'mpegplayer.library',0
MPEGPlayerId   VSTRING

	CNOP	0,2

;-----------------------------------------------------------------------

LibInitTable:
	DC.L	MPEGPlayerLib_SIZEOF
	DC.L	LibFuncTable
	DC.L	0
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
	V_DEF	_CDPlayerLVO
	DC.W    -1

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
	move.l	a1,-(sp)			; preserve
	move.l	d0,a1				; put lib base into an A register
	move.l	a6,mp_SysLib(a1)		; save exec base
	move.l	a0,mp_SegList(a1)		; save seg list for expunge...
	move.w  #REVISION,LIB_REVISION(a1)	; set this up, Exec can't
	move.l  (sp)+,a1			; restore original
	rts					; return with D0=libBase

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has MPEGPlayerBase, task switching is disabled
; Returns 0 for failure, or MPEGPlayerBase for success.
LibOpen:
	tst.w	LIB_OPENCNT(a6)		; only allow one opener at a time
	bne.s	FailOpen_2

	move.l	a5,-(sp)
	move.l	a6,a5
	move.l	mp_SysLib(a5),a6

	moveq	#OLTAG_GRAPHICS,d0
	CALL	TaggedOpenLibrary
	move.l	d0,mp_GfxLib(a5)
	beq.s   FailOpen_1

	moveq	#OLTAG_INTUITION,d0
	CALL	TaggedOpenLibrary
	move.l	d0,mp_IntuitionLib(a5)
	beq.s   FailOpen_1

	lea	LowLevelName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,mp_LowLevelLib(a5)
	beq.s   FailOpen_1

	lea	DeBoxName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,mp_DeBoxLib(a5)
	beq.s   FailOpen_1

	lea	UtilityName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,mp_UtilityLib(a5)
	beq.s   FailOpen_1

	lea	CDGName(pc),a1
	moveq	#0,d0
	CALL	OpenLibrary
	move.l	d0,mp_CDGLib(a5)
	beq.s   FailOpen_1

	move.l	a5,a6
	move.l	(sp)+,a5

	addq.w	#1,LIB_OPENCNT(a6)
	move.l	a6,d0
	rts

FailOpen_1:
	bsr.s	CloseLibs
	move.l	a5,a6
	move.l	(sp)+,a5

FailOpen_2:
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has MPEGPlayerBase, task switching is disabled
; Returns 0 if there are any openers, or the library seglist if there
;   are no more.
LibClose:
	subq.w	#1,LIB_OPENCNT(a6)

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has MPEGPlayerBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
	tst.w	LIB_OPENCNT(a6)
	beq.s	DoExpunge

	moveq	#0,d0		; can't go, someone's using us
	rts

DoExpunge:
	movem.l	d2/a5/a6,-(sp)
	move.l	a6,a5
	move.l	mp_SysLib(a5),a6
	move.l	mp_SegList(a5),d2

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
	move.l	mp_UtilityLib(a5),a1
	CALL	CloseLibrary

	move.l	mp_CDGLib(a5),a1
	CALL	CloseLibrary

	move.l	mp_DeBoxLib(a5),a1
	CALL	CloseLibrary

	move.l	mp_LowLevelLib(a5),a1
	CALL	CloseLibrary

	move.l	mp_IntuitionLib(a5),a1
	CALL	CloseLibrary

	move.l	mp_GfxLib(a5),a1
	GO	CloseLibrary

;-----------------------------------------------------------------------

LibReserved:
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

LIBVERSION    EQU  39
DeBoxName     DC.B "debox.library",0
LowLevelName  DC.B "lowlevel.library",0
UtilityName   DC.B "utility.library",0
CDGName       DC.B "cdg.library",0

;-----------------------------------------------------------------------

	END