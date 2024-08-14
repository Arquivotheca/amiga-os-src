
*************************************************************************
*                                                                       *
*       Copyright (C) 1992, Commodore Amiga Inc.  All rights reserved.  *
*                                                                       *
*************************************************************************


        INCLUDE "exec/types.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/ables.i"
        INCLUDE "playerlibrary_rev.i"

        INCLUDE "playerlibrary.i"
        INCLUDE "internal.i"
        INCLUDE "cdtv:include/cdtv/cdtv.i"

        XREF    _Main
        XREF    _LibOpen
        XREF    _LibClose
        XREF    _LibExpunge
        XREF    _GetOptions
        XREF    _SetOptions
        XREF    _GetPlayerState
        XREF    _ModifyPlayList
        XREF    _ObtainPlayList
        XREF    _SubmitKeyStroke
        XREF    _Analyze
        XREF    __EndCode

        XDEF    _initFunc
        XDEF    _initStruct

        INT_ABLES


        SECTION PlayerLibrary

        moveq.l #-1,d0          ; Not Executable
        rts


ROMTAG:                         ;STRUCTURE RT,0
        dc.w    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
        dc.l    ROMTAG          ; APTR  RT_MATCHTAG
        dc.l    __EndCode       ; APTR  RT_ENDSKIP
        dc.b    RTF_COLDSTART   ; UBYTE RT_FLAGS
        dc.b    VERSION         ; UBYTE RT_VERSION
        dc.b    NT_LIBRARY      ; UBYTE RT_TYPE
        dc.b    0               ; BYTE  RT_PRI
        dc.l    PlayerName      ; APTR  RT_NAME
        dc.l    VERSTRING       ; APTR  RT_IDSTRING
        dc.l    InitModule      ; APTR  RT_INIT
                                ; LABEL RT_SIZE

PlayerName:
                PLAYERLIBRARYNAME

VERSTRING:
                VSTRING
                VERSTAG

VERNUM: EQU     VERSION
REVNUM: EQU     REVISION

        ds.w    0



*------ Functions Offsets -------------------------------------

_initFunc:
        dc.l    _LibOpen
        dc.l    _LibClose
        dc.l    _LibExpunge
        dc.l    Reserved
        dc.l    _GetOptions
        dc.l    _SetOptions
        dc.l    _GetPlayerState
        dc.l    _ModifyPlayList
        dc.l    _ObtainPlayList
        dc.l    _SubmitKeyStroke
        dc.l    _Analyze
        dc.l    -1


*------ Initializaton Table -----------------------------------

_initStruct:
        INITLONG        LIB_IDSTRING,VERSTRING
        INITLONG        LN_NAME,PlayerName
        INITWORD        LIB_VERSION,VERNUM
        INITWORD        LIB_REVISION,REVNUM
        INITBYTE        LN_TYPE,NT_LIBRARY
        INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED

        DC.L            0



InitModule:
        movem.l d2-d7/a2-a6,-(sp)

        jsr     _Main

        movem.l (sp)+,d2-d7/a2-a6
        rts



Reserved:
        moveq   #0,d0
        rts



        section __MERGED,data

        end
        

