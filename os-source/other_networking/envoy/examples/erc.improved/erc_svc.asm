        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "libraries/dos.i"

        INCLUDE "erc_rev.i"
        INCLUDE "ercbase.i"

        LIST

;---------------------------------------------------------------------------

        XREF    _StartService
        XREF    _Server
        XREF    _GetServiceAttrsA
        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved

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

LibName DC.B 'erc.service',0
LibId   VERSTAG

        CNOP    0,4

LibInitTable:
        DC.L    ERCSvc_SIZEOF
        DC.L    LibFuncTable
        DC.L    LibDataTable
        DC.L    LibInit

V_DEF   MACRO
        DC.L    \1
        ENDM

LibFuncTable:
        V_DEF   LibOpen
        V_DEF   LibClose
        V_DEF   LibExpunge
        V_DEF   LibReserved

        V_DEF   _RexxReserved
        V_DEF   _StartService
        V_DEF   _GetServiceAttrsA

        DC.L   -1

LibDataTable:
        INITBYTE   LN_TYPE,NT_LIBRARY
        INITLONG   LN_NAME,LibName
        INITBYTE   LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
        INITWORD   LIB_VERSION,VERSION
        INITWORD   LIB_REVISION,REVISION
        INITLONG   LIB_IDSTRING,LibId
        DC.W       0

        CNOP    0,4

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        movem.l a0/a5/a6/d7,-(sp)
        move.l  d0,a5
        move.l  d0,_ERCBase
        move.l  a6,ERC_SysBase(a5)
        move.l  a0,ERC_SegList(a5)

        move.l  #AO_DOSLib,d7
        lea     DOSName(pc),a1
        bsr.s   OpenLib
        move.l  d0,ERC_DOSBase(a5)

        move.l  #AO_UtilityLib,d7
        lea     UtilityName(pc),a1
        bsr.s   OpenLib
        move.l  d0,ERC_UtilityBase(a5)

        lea     ERC_OpenLock(a5),a0
        CALL    InitSemaphore

        clr.l   ERC_Entity(a5)
        clr.l   ERC_NIPCBase(a5)

        move.l  a5,d0

        movem.l (sp)+,a0/a5/a6/d7
        rts

OpenLib:
        moveq   #LIBVERSION,d0
        CALL    OpenLibrary
        move.l  (sp)+,a0        ; pop return address
        tst.l   d0              ; did lib open?
        beq.s   FailInit        ; nope, so exit
        jmp     (a0)            ; yes, so return

FailInit:
        bsr     CloseLibs
        or.l    #AG_OpenLib,d7
        CALL    Alert
        movem.l (sp)+,a0/a5/a6/d7
        moveq   #0,d0
        rts

LIBVERSION    EQU  37

DOSName         DC.B "dos.library",0
UtilityName     DC.B "utility.library",0
NIPCName        DC.B "nipc.library",0

        CNOP    0,4

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has ServicesBase, task switching is disabled
; Returns 0 for failure, or ServicesBase for success.
LibOpen:
        ; see if nipc.library is already open
        tst.l   ERC_NIPCBase(a6)
        bne.s   2$

        ; lock anyone else out for a while, since we need to Wait()
        lea.l   ERC_OpenLock(a6),a0
        move.l  a6,-(sp)
        move.l  ERC_SysBase(a6),a6
        CALL    ObtainSemaphore

        ; open nipc.library
        lea.l   NIPCName,a1
        clr.l   d0
        CALL    OpenLibrary
        move.l  (sp),a6
        move.l  d0,ERC_NIPCBase(a6)

        ; unlock the rest of the word.
        lea.l   ERC_OpenLock(a6),a0
        move.l  ERC_SysBase(a6),a6
        CALL    ReleaseSemaphore
        move.l  (sp)+,a6

        ; did nipc open?
        tst.l   ERC_NIPCBase(a6)
        bne.s   2$
        clr.l   d0  ; too bad...
        rts

        ; open okay...
2$      addq.w  #1,LIB_OPENCNT(a6)
        bclr    #LIBB_DELEXP,LIB_FLAGS(a6)
        move.l  a6,d0
        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has ServicesBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
; due to delayed expunge bit being set
LibClose:
        subq.w  #1,LIB_OPENCNT(a6)

        ; if delayed expunge bit set, then try to get rid of the library
        btst    #LIBB_DELEXP,LIB_FLAGS(a6)
        bne.s   CloseExpunge

        ; delayed expunge not set, so stick around
        moveq   #0,d0
        rts

CloseExpunge:
        ; if no library users, then just remove the library
        tst.w   LIB_OPENCNT(a6)
        beq.s   DoExpunge

        ; still some library users, so just flush unused resources
        bclr    #LIBB_DELEXP,LIB_FLAGS(a6)
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has ServicesBase, task switching is disabled
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
        move.l  ERC_SegList(a5),d2

        move.l  a5,a1
        REMOVE

        move.l  ERC_SysBase(a5),a6
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

CloseLibs:
        move.l  ERC_DOSBase(a5),a1
        CALL    CloseLibrary

        move.l  ERC_NIPCBase(a5),a1
        CALL    CloseLibrary

        move.l  ERC_UtilityBase(a5),a1
        GO      CloseLibrary

;-----------------------------------------------------------------------

_ERCBase       DC.L  0

;-----------------------------------------------------------------------

; Reserved Function for ARexx.

_RexxReserved:
        moveq.l #0,d0
        rts

;-----------------------------------------------------------------------

        END
