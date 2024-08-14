;********************************************************************
;
; dev.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


        INCLUDE "assembly.i"

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/io.i"

        INCLUDE "../janus/i86block.i"
        INCLUDE "../janus/services.i"
        LIST

        INCLUDE "jddata.i" 
        INCLUDE "asmsupp.i"
        INCLUDE "printf.mac"

        XDEF    JDOpen
        XDEF    JDClose
        XDEF    JDExpunge
        XDEF    JDNull
                
        XLVO    CloseLibrary
        XLVO    FreeMem

        XLVO    FreeJanusMem
        XLVO    SetParamOffset
        XLVO    SetJanusHandler

;DEBUG_CODE EQU 1

;------ jdisk.device/Open --------------------------------------------
JDOpen          move.l  d0,IO_UNIT(a1)
                bclr    #LIBB_DELEXP,LIB_FLAGS(a6)
                addq.w  #1,LIB_OPENCNT(a6)
                rts

;------ jdisk.device/Close -------------------------------------------
JDClose:
                ;------ clear device as a precaution against further use
                moveq   #-1,d0 
                move.l  d0,IO_DEVICE(a1)

                ;------ check if device should now be expunged
                subq.w  #1,LIB_OPENCNT(a6)
                bne.s   1$
                btst    #LIBB_DELEXP,LIB_FLAGS(a6)
                beq.s   1$
                jmp     LIB_EXPUNGE(a6)         ; call expunge for device

1$              moveq   #0,d0
                rts

;------ jdisk.device/Expunge -----------------------------------------
JDExpunge:
                tst.w   LIB_OPENCNT(a6)
                beq.s   realExpunge

                bset    #LIBB_DELEXP,LIB_FLAGS(a6)
                moveq   #0,d0
                rts

realExpunge     movem.l a2/a6,-(a7)
                move.l  a6,a2               ; cache device base

                moveq   #JSERV_HARDDISK,d0
                suba.l  a1,a1
                move.l  jd_JanusBase(a2),a6
                CALLLVO SetJanusHandler

                moveq   #JSERV_HARDDISK,d0
                moveq   #-1,d1
                CALLLVO SetParamOffset

                moveq   #AmigaDskReq_SIZEOF,d0
                move.l  jd_AmigaDskReq(a2),a1
                CALLLVO FreeJanusMem

                move.l  #TB_SIZE,d0
                move.l  jd_TrackBuffer(a2),a1
                CALLLVO FreeJanusMem

                move.l  jd_ExecBase(a2),a6
                move.l  a3,-(sp)                ; free up unit patches
                movea.l jd_Units(a2),a3         ; free up unit nodes
UnitFree        cmpa.w  #0,a3                   ; last unit ?
                beq.s   UnitFreeDone            ; yep, all memory freed
                movea.l a3,a1                   ; freeing this unit
                movea.l BB_Next(a3),a3          ; move to next first
                move.l  #BB_SIZEOF,d0
                CALLLVO FreeMem
                bra.s   UnitFree                ; go do the next
UnitFreeDone    move.l  (sp)+,a3

                move.l  jd_JanusBase(a2),a1
                CALLLVO CloseLibrary
                                                
                move.l  jd_Segment(a2),-(a7)

                ;------ remove device
                move.l  a2,a1
                REMOVE
    
                ;------ free up device node
                move.l  a2,a1
                CLEAR   d0
                move.w  LIB_NEGSIZE(a2),d0
                sub.w   d0,a1
                add.w   LIB_POSSIZE(a2),d0
                move.l  jd_ExecBase(a2),a6
                CALLLVO FreeMem

                move.l  (a7)+,d0
                movem.l (a7)+,a2/a6
                rts

;------ jdisk.device/Null --------------------------------------------
JDNull          moveq   #0,d0
                rts

        END


