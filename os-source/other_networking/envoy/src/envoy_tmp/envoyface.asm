***
* $Id: envoyface.asm,v 1.1 92/09/10 14:44:50 gregm Exp Locker: kcd $
***

;---------------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/memory.i"
        INCLUDE "graphics/text.i"

        INCLUDE "envoy_rev.i"
        INCLUDE "envoybase.i"

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved

        xdef    _topaz8

;---------------------------------------------------------------------------

        XREF    _LVOAllocMem,_AbsExecBase
        XSYS    Remove
        XSYS    OpenLibrary
        XSYS    CloseLibrary
        XSYS    OpenFont
        XSYS    CloseFont

;---------------------------------------------------------------------------

        XREF    _LVOFreeMem
;---------------------------------------------------------------------------
        xref    _HostRequestA
        xref	_LoginRequestA
 	xref	_UserRequestA
;---------------------------------------------------------------------------


*       XREF    ENDCODE

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

LibName DC.B 'envoy.library',0

LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    eb_SIZE
        DC.L    LibFuncTable
        DC.L    LibDataTable
        DC.L    LibInit

V_DEF   MACRO
        dc.l     \1
        endm

LibFuncTable:
        V_DEF   LibOpen
        V_DEF   LibClose
        V_DEF   LibExpunge
        V_DEF   LibReserved

        V_DEF   _HostRequestA
        V_DEF	_LoginRequestA
	V_DEF	_UserRequestA

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
        move.l  a6,eb_SysBase(a3)
        move.l  a0,eb_SegList(a3)

        movem.l  (sp)+,d0-d7/a0-a6

        rts

        CNOP    0,4

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has EnvoyBase, task switching is disabled
; Returns 0 for failure, or NIPCBase for success.
LibOpen:
        addq.w      #1,LIB_OPENCNT(a6)

        movem.l     a0-a2/a5/d0-d1/d2/a6,-(sp)
        move.l      a6,a5
        move.l      eb_SysBase(a6),a6
        moveq.l     #0,d0
        move.l      d0,eb_IntuitionBase(a5)
        move.l      d0,eb_GfxBase(a5)
        move.l      d0,eb_DOSBase(a5)
        move.l      d0,eb_UtilityBase(a5)
        move.l      d0,eb_NIPCBase(a5)
        move.l      d0,eb_GadToolsBase(a5)
	move.l	    d0,eb_LayersBase(a5)

        move.l      #37,d2

        lea.l       DName,a1
        move.l      d2,d0
        SYS         OpenLibrary
        move.l      d0,eb_DOSBase(a5)
        beq	    9$

        lea.l       UName,a1
        move.l      d2,d0
        SYS         OpenLibrary
        move.l      d0,eb_UtilityBase(a5)
	beq.s	    9$

        lea.l       IName,a1
        move.l      d2,d0
        SYS         OpenLibrary
        move.l      d0,eb_IntuitionBase(a5)
        beq.s       9$

        lea.l       FName,a1
        move.l      d2,d0
        SYS         OpenLibrary
        move.l      d0,eb_GfxBase(a5)
        beq.s	    9$

        lea.l       NName,a1
        move.l      d2,d0
        SYS         OpenLibrary
        move.l      d0,eb_NIPCBase(a5)
        beq.s	    9$

	lea.l	    LName,a1
	move.l	    d2,d0
	SYS	    OpenLibrary
	move.l	    d0,eb_LayersBase(a5)
        beq.s	    9$

        lea.l       GName,a1
        move.l      d2,d0
        SYS         OpenLibrary
        move.l      d0,eb_GadToolsBase(a5)
        beq.s	    9$

        move.l      eb_GfxBase(a5),a6
        lea.l       TopazAttr,a0
        SYS         OpenFont
        move.l      d0,eb_TopazFont(a5)
        tst.l       d0
        beq.s       9$


        movem.l     (sp)+,a0-a2/a5/d0-d1/d2/a6

        move.l      a6,d0
        rts

9$      bsr.s       CloseEm
        movem.l     (sp)+,a0-a2/a5/d0-d2
        moveq.l     #0,d0
        rts


CloseEm:

        move.l      eb_GfxBase(a5),a6
        move.l      eb_TopazFont(a5),d0
        beq.s       0$
        move.l      d0,a1
        SYS         CloseFont

0$
        move.l      eb_SysBase(a5),a6

        move.l      eb_DOSBase(a5),d0
        beq.s       1$
        move.l      d0,a1
        SYS         CloseLibrary
1$

        move.l      eb_IntuitionBase(a5),d0
        beq.s       2$
        move.l      d0,a1
        SYS         CloseLibrary
2$

        move.l      eb_GfxBase(a5),d0
        beq.s       3$
        move.l      d0,a1
        SYS         CloseLibrary
3$

        move.l      eb_GadToolsBase(a5),d0
        beq.s       4$
        move.l      d0,a1
        SYS         CloseLibrary
4$

	move.l	    eb_LayersBase(a5),d0
	beq.s	    5$
	move.l	    d0,a1
	SYS	    CloseLibrary
5$

        move.l      eb_NIPCBase(a5),d0
        beq.s       6$
        move.l      d0,a1
        SYS         CloseLibrary
6$

        move.l      eb_UtilityBase(a5),d0
        beq.s       7$
        move.l      d0,a1
        SYS         CloseLibrary
7$

        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has EnvoyBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set, which won't happen for a ROM
;   module
LibClose:

        movem.l a0-a1/d0-d1/a5/a6,-(sp)
        move.l  a6,a5
        move.l  eb_SysBase(a5),a6
        bsr     CloseEm
        movem.l (sp)+,a0-a1/d0-d1/a5/a6

        subq.w  #1,LIB_OPENCNT(a6)
        bne.s   1$

        btst    #LIBB_DELEXP,LIB_FLAGS(a6)
        beq.s   1$
        bsr.s   DoExpunge

1$
        moveq.l #0,d0
        rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has NIPCBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
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
        move.l  eb_SysBase(a5),a6

        move.l  a5,a1                   * Pull our devbase from the list
        SYS     Remove

        move.l  eb_SegList(a5),d2

        move.l  a5,a1
        moveq.l #0,d0
        move.w  LIB_NEGSIZE(a5),d0      * Free the library base
        sub.l   d0,a1
        add.w   LIB_POSSIZE(a5),d0
        SYS     FreeMem

        move.l  d2,d0                   * Return our seglist

1$
        movem.l (sp)+,d2/a5/a6
        rts

;-----------------------------------------------------------------------

LibReserved:
        rts


IName   dc.b 'intuition.library',0
DName   dc.b 'dos.library',0
GName   dc.b 'gadtools.library',0
FName   dc.b 'graphics.library',0
UName   dc.b 'utility.library',0
NName   dc.b 'nipc.library',0
LName   dc.b 'layers.library',0

        cnop    0,2
_topaz8:
TopazAttr:
        dc.l TopazName
        dc.w 8
        dc.b FS_NORMAL
        dc.b FPF_ROMFONT
TopazName:
        dc.b 'topaz.font',0


         cnop        0,2
ENDCODE:

        END


