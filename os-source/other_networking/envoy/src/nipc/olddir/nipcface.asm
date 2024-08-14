***
* $Id: nipcface.asm,v 1.6 92/06/08 10:15:29 kcd Exp $
***

;---------------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/memory.i"

        INCLUDE "nipc.library_rev.i"
        INCLUDE "nipcbase.i"

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved

;---------------------------------------------------------------------------

        XREF    _LVOAllocMem,_AbsExecBase

        XREF    @KillOffNIPC

        XREF    _BeginTransaction
        XREF    _DoTransaction
        XREF    _AbortTransaction
        XREF    _CreateEntityA
        XREF    _DeleteEntity
        XREF    _WaitEntity
        XREF    _FindEntity
        XREF    _LoseEntity
        XREF    _AllocTransactionA
        XREF    _FreeTransaction
        XREF    _GetTransaction
        XREF    _ReplyTransaction
        XREF    _WaitTransaction
        XREF    _CheckTransaction
        XREF    _GetEntityName
        XREF    _AddRoute
        XREF    _DeleteRoute
        XREF    _GetRouteInfo
        XREF    _StartStats
        XREF    _EndStats
;        XREF    _StartMonitor
;        XREF    _StopMonitor
        XREF    _GetHostName
        XREF    _NetQueryA

;---------------------------------------------------------------------------

        XREF    _LVOFreeMem
*       XREF    ENDCODE

;---------------------------------------------------------------------------

        xref    _InitLock,_StartLibrary

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
        moveq   #-1,d0
        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD              ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                     ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                    ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT               ; UBYTE RT_FLAGS
        DC.B    VERSION                    ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY                 ; UBYTE RT_TYPE
        DC.B    0                          ; BYTE  RT_PRI
        DC.L    LibName                    ; APTR  RT_NAME
        DC.L    LibId                      ; APTR  RT_IDSTRING
        DC.L    LibInitTable               ; APTR  RT_INIT

LibName DC.B 'nipc.library',0

LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    nb_SIZE
        DC.L    LibFuncTable
        DC.L    LibDataTable
        DC.L    LibInit

V_DEF   MACRO
        dc.l     \1
        endm

LibFuncTable:
*        DC.W    -1
        V_DEF   LibOpen
        V_DEF   LibClose
        V_DEF   LibExpunge
        V_DEF   LibReserved

        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved

;        V_DEF   _StartMonitor
;        V_DEF   _StopMonitor
        V_DEF   _AddRoute
        V_DEF   _DeleteRoute
        V_DEF   _StartStats
        V_DEF   _EndStats

        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved
        V_DEF   _NIPCReserved

        V_DEF   _AllocTransactionA
        V_DEF   _FreeTransaction

        V_DEF   _CreateEntityA
        V_DEF   _DeleteEntity
        V_DEF   _FindEntity
        V_DEF   _LoseEntity

        V_DEF   _DoTransaction
        V_DEF   _BeginTransaction
        V_DEF   _GetTransaction
        V_DEF   _ReplyTransaction
        V_DEF   _CheckTransaction
        V_DEF   _AbortTransaction
        V_DEF   _WaitTransaction
        V_DEF   _WaitEntity

        V_DEF   _GetEntityName
        V_DEF   _GetHostName
        V_DEF   _NetQueryA

        DC.l   -1

LibDataTable:
        INITBYTE   LN_TYPE,NT_LIBRARY
        INITLONG   LN_NAME,LibName
        INITBYTE   LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
        INITWORD   LIB_VERSION,VERSION
        INITWORD   LIB_REVISION,REVISION
        INITLONG   LIB_IDSTRING,LibId
        DC.W       0

        cnop       0,4

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:

        movem.l d0-d7/a0-a6,-(sp)

        move.l  d0,a3
        move.l  a6,nb_SysBase(a3)
        move.l  a0,nb_SegList(a3)

        lea.l   ANMPEntityList(a3),a1
        move.l  #nb_SIZE-ANMPEntityList-1,d1   /* Yes, there's no -1 for the dbra here.  There isn't supposed to be. */
1$      move.b  #0,(a1)+
        dbra    d1,1$

        move.l  a3,a6
        jsr     _InitLock

        movem.l  (sp)+,d0-d7/a0-a6

        rts

        CNOP    0,4

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has NIPCBase, task switching is disabled
; Returns 0 for failure, or NIPCBase for success.
LibOpen:
        movem.l     d0-d7/a0-a6,-(sp)


        addq.w      #1,LIB_OPENCNT(a6)

        jsr         _StartLibrary

        movem.l     (sp)+,d0-d7/a0-a6
        move.l      a6,d0
        tst.l       nipcprocess(a6)
        bne.s       1$
        subq.w      #1,LIB_OPENCNT(a6)
        clr.l       d0
1$      rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has NIPCBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set, which won't happen for a ROM
;   module
LibClose:

        subq.w  #1,LIB_OPENCNT(a6)
        bne.s   1$

        btst    #LIBB_DELEXP,LIB_FLAGS(a6)
        beq.s   1$
        bsr.s   DoExpunge

1$
        moveq.l #0,d0
        rts



*** FALLS THROUGH !!!

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has NIPCBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit. Won't happen since this is
; a ROM module
LibExpunge:


        tst.w   LIB_OPENCNT(a6)
        bne.s   1$
        bsr.s   DoExpunge
        bra.s   2$

1$
        bset    #LIBB_DELEXP,LIB_FLAGS(a6)
2$
        moveq.l   #0,d0
        rts

DoExpunge:
        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5
        move.l  nb_SegList(a5),d2

        move.l  a5,a1
        REMOVE
        move.l  d2,d0
        tst.l   nipcprocess(a6)
        beq.s   1$

        jsr     @KillOffNIPC
        clr.l   d0
1$
        movem.l (sp)+,d2/a5/a6
        rts

;-----------------------------------------------------------------------

_NIPCReserved:
LibReserved:
        moveq   #0,d0
        rts


;-----------------------------------------------------------------------

         cnop        0,2
ENDCODE:

        END
