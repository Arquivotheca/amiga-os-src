************************************************************************
***
***  start.asm
***
*** CDTV Bootstrap startup module.
***
************************************************************************

    INCLUDE "exec/types.i"
    INCLUDE "exec/exec.i"
    INCLUDE "exec/execbase.i"
    INCLUDE "exec/resident.i"
    INCLUDE "hardware/cia.i"
    INCLUDE "hardware/intbits.i"

    INCLUDE "defs.i"
    INCLUDE "cdstrap_rev.i"
    INCLUDE "cdtv:include/internal/copyright.i"


    SECTION text,CODE

            CNOP    4
            moveq   #0,d0
            rts

************************************************************************

    XREF    _Main
    XREF    __EndCode

    XDEF    _ModIdent
    XDEF    _FakeOWB
    XDEF    _UnlinkRomTag

_ciaa   EQU     $bfe001 
_ciab   EQU     $bfd000

ResTag:     dc.w    RTC_MATCHWORD
            dc.l    ResTag
            dc.l    __EndCode       ; end skip
            dc.b    RTF_COLDSTART   ; flags
            dc.b    VERSION         ; version
            dc.b    0               ; type
            dc.b    -58             ; pri
            dc.l    CDStrapName
            dc.l    _ModIdent
            dc.l    InitModule

CDStrapName:
            dc.b    'cdstrap',0
            COPYRIGHT
            ds.w    0

_ModIdent:  VSTRING
            ds.w    0
ModName:    dc.b    'CDStrap',0
            ds.w    0

************************************************************************

InitModule:
        movem.l d2-d7/a2-a6,-(sp)

        jsr _Main

        clr.l   d0
        movem.l (sp)+,d2-d7/a2-a6
        rts


_FakeOWB:
        move.l  #0,d0
        rts


************************************************************************
***
***  ChkSumFD -- CheckSum Floppy Disk
***
*** Returns zero for valid check sum.
*** Code taken from Strap 1.3 module.
***
************************************************************************

BUF_SIZE    equ (512*2)

    XDEF    _ChkSumFD
_ChkSumFD:
        move    #(BUF_SIZE>>2)-1,d1
        move.l  4(sp),a0
        moveq   #0,d0
2$:
        add.l   (a0)+,d0
        bcc.s   1$
        addq.l  #1,d0
1$:     dbf d1,2$
        not.l   d0
        rts




_UnlinkRomTag:
; a0 - Rom tag pointer

        movem.l a2/a6,-(sp)

        move.l  4,a6              ; load exec base pointer

        move.l  KickTagPtr(a6),d0 ; d0 now points to array of rom tag pointers
        beq.s   doResModules      ; if the pointer is NULL, leave
        move.l  d0,a1             ; a1 now points to array of rom tag pointers

        move.l  (a1)+,d0          ; d0 has first ROM Tag
        cmp.l   d0,a0             ; is this us?
        bne.s   scanKickArray     ; not first one, so scan array

        move.l  (a1),d0           ; d0 has pointer to next array
        bclr    #31,d0            ; clear upper bit
        move.l  d0,KickTagPtr(a6) ; store pointer to next array in ExecBase
        bra.s   doResModules

scanKickArray:
        tst.l   d0                ; set condition codes
        bmi.s   skaDoLink         ; was this first rom tag a link pointer?

skaLoop:
        move.l  (a1)+,d0          ; d0 has next rom tag
        beq.s   doResModules      ; end of list, didn't find it...
        bge.s   skaCmp            ; if bit 31 is set, then d0 holds pointer to next array of pointers

skaDoLink:
        bclr    #31,d0            ; clear magical high bit
        move.l  a1,a2             ; save address of "next array" pointer
        move.l  d0,a1             ; a1 has address of next array
        bra.s   skaLoop           ; so try again

skaCmp:
        cmp.l   d0,a0             ; is this our lucky winner?
        bne.s   skaLoop           ; nope, so try again

        move.l  (a1),-(a2)        ; our next into the previous' next pointer



doResModules:
        move.l  ResModules(a6),d0 ; d0 now points to array of rom tag pointers
        beq.s   exit              ; if the pointer is NULL, leave
        move.l  d0,a1             ; a1 now points to array of rom tag pointers

1$:     move.l  (a1)+,d0          ; d0 has next rom tag
        beq.s   exit              ; end of list, didn't find it...
        bge.s   2$                ; if bit 31 is set, then d0 holds pointer to next array of pointers
        bclr    #31,d0            ; clear magical high bit
        move.l  d0,a1             ; a1 has address of next array
        bra.s   1$                ; so try again

2$:     cmp.l   d0,a0             ; is this our lucky winner?
        bne.s   1$                ; nope, so try again

3$:     move.l  a1,d0             ; now make our rom tag pointer a link
        bset    #31,d0            ; pointer instead, and point it to the
        move.l  d0,-(a1)          ; next slot in the array

exit:   movem.l (sp)+,a2/a6
        rts

        section __MERGED,data

        end
        

