        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"

        INCLUDE "realtime_rev.i"
        INCLUDE "realtimebase.i"

        LIST

;---------------------------------------------------------------------------

        XREF	_LIBLockRealTime
        XREF	_LIBUnlockRealTime

        XREF	_LIBCreatePlayerA
        XREF	_LIBDeletePlayer
        XREF	_LIBSetPlayerAttrsA
        XREF	_LIBSetConductorState
        XREF	_LIBExternalSync
        XREF	_LIBNextConductor
        XREF	_LIBFindConductor
        XREF	_LIBGetPlayerAttrsA

        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF	LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
        moveq   #-1,d0
        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD           ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                  ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                 ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT            ; UBYTE RT_FLAGS
        DC.B    VERSION                 ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY              ; UBYTE RT_TYPE
        DC.B    0                       ; BYTE  RT_PRI
        DC.L    LibName                 ; APTR  RT_NAME
        DC.L    LibId                   ; APTR  RT_IDSTRING
        DC.L    LibInitTable            ; APTR  RT_INIT

LibName DC.B 'realtime.library',0
LibId   VSTRING

        CNOP    0,2

LibInitTable:
        DC.L    RealTimeLib_SIZEOF
        DC.L    LibFuncTable
        DC.L    0
        DC.L    LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
        V_DEF	LibOpen
        V_DEF	LibClose
        V_DEF	LibExpunge
        V_DEF	LibReserved

        ; Locks
        V_DEF  _LIBLockRealTime
        V_DEF  _LIBUnlockRealTime

        ; Conductor
        V_DEF  _LIBCreatePlayerA
        V_DEF  _LIBDeletePlayer
        V_DEF  _LIBSetPlayerAttrsA
        V_DEF  _LIBSetConductorState
        V_DEF  _LIBExternalSync
        V_DEF  _LIBNextConductor
        V_DEF  _LIBFindConductor
        V_DEF  _LIBGetPlayerAttrsA

        ; for the future...
        V_DEF  LibReserved
        V_DEF  LibReserved
        V_DEF  LibReserved
        V_DEF  LibReserved
        V_DEF  LibReserved

        DC.W   -1

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        movem.l a0/a5/a6/d7,-(sp)
        move.l  d0,a5
        move.l  a6,rtl_SysBase(a5)
        move.l  a0,rtl_SegList(a5)

	move.w	#REVISION,LIB_REVISION(a5)

        move.l  #AG_OpenLib!AO_UtilityLib,d7
        lea     UtilityName(pc),a1
        moveq   #LIBVERSION,d0
        CALL    OpenLibrary
        move.l  d0,rtl_UtilityBase(a5)
        beq.s   FailInit

	lea	rtl_ConductorList(a5),a0
	NEWLIST a0

        lea	rtl_ConductorLock(a5),a0
        CALL	InitSemaphore

	lea	rtl_PlayerListLock(a5),a0
	CALL	InitSemaphore

        move.l  a5,d0
        movem.l (sp)+,a0/a5/a6/d7
        rts

FailInit:
        CALL	Alert
        bsr     CloseLibs
        movem.l (sp)+,a0/a5/a6/d7
        moveq   #0,d0
        rts

LIBVERSION    EQU  37
UtilityName   DC.B "utility.library",0

        CNOP    0,2

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has RealTimeBase, task switching is disabled
; Returns 0 for failure, or RealTimeBase for success.
LibOpen:
        addq.w  #1,LIB_OPENCNT(a6)
        bclr    #LIBB_DELEXP,LIB_FLAGS(a6)
        move.l  a6,d0
        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has RealTimeBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,LIB_OPENCNT(a6)

	; if delayed expunge bit set, then try to get rid of the library
	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	bne.s	CloseExpunge

	; delayed expunge not set, so stick around
	moveq	#0,d0
	rts

CloseExpunge:
	; if no library users, then just remove the library
	tst.w	LIB_OPENCNT(a6)
	beq.s	DoExpunge

	; still some library users, so forget about flushing
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has RealTimeBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
        tst.w   LIB_OPENCNT(a6)
        beq.s   DoExpunge

        bset    #LIBB_DELEXP,LIB_FLAGS(a6)
        moveq   #0,d0
        rts

DoExpunge:
        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5
        move.l  rtl_SegList(a5),d2

        move.l  a5,a1
        REMOVE

        move.l  rtl_SysBase(a5),a6
        bsr.s   CloseLibs

        move.l  a5,a1
        moveq   #0,d0
        move.w  LIB_NEGSIZE(a5),d0
        sub.l   d0,a1
        add.w   LIB_POSSIZE(a5),d0
        CALL    FreeMem

        move.l  d2,d0
        movem.l (sp)+,d2/a5/a6
        rts

;-----------------------------------------------------------------------

LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------

CloseLibs:
        move.l  rtl_UtilityBase(a5),a1
        GO	CloseLibrary

;-----------------------------------------------------------------------

        END
