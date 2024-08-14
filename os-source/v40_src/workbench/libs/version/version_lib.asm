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

        INCLUDE "version_rev.i"

        LIST

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved

;-----------------------------------------------------------------------

   STRUCTURE VersionLib,LIB_SIZE
	ULONG	vl_SysBase
	ULONG	vl_SegList
   LABEL VersionLib_SIZEOF

;-----------------------------------------------------------------------

CALL MACRO <Function_Name>
	xref _LVO\1
 	jsr _LVO\1(A6)
     ENDM

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

LibName DC.B 'version.library',0
LibId   VSTRING

        CNOP    0,2

LibInitTable:
        DC.L    VersionLib_SIZEOF
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

        DC.W   -1

        CNOP    0,2

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        exg	a5,d0
        move.l  a6,vl_SysBase(a5)
        move.l  a0,vl_SegList(a5)
        move.w	#REVISION,LIB_REVISION(a5)
        exg	d0,a5
	rts

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has VersionBase, task switching is disabled
; Returns 0 for failure, or VersionBase for success.
LibOpen:
        addq.w  #1,LIB_OPENCNT(a6)
        move.l  a6,d0
        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has VersionBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,LIB_OPENCNT(a6)
	bne.s	1$

        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5
        move.l  vl_SegList(a5),d2

        move.l  a5,a1
        REMOVE

	move.l	vl_SysBase(a5),a6
        move.l  a5,a1
        moveq   #0,d0
        move.w  LIB_NEGSIZE(a5),d0
        sub.l   d0,a1
        add.w   LIB_POSSIZE(a5),d0
        CALL    FreeMem

        move.l  d2,d0
        movem.l (sp)+,d2/a5/a6

1$:
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has VersionBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------

ENDCODE:

;-----------------------------------------------------------------------

        END
