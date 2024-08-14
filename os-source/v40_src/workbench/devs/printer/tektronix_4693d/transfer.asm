**********************************************************************
*
* Transfer routine for Tektronix 4693D
*
* Bill Koester - Mar/88
*
**********************************************************************
*
*
*
**********************************************************************

    INCLUDE "exec/types.i"

    INCLUDE "../intuition.i"
    INCLUDE "devices/printer.i"
    INCLUDE "devices/prtbase.i"
    INCLUDE "devices/prtgfx.i"

    XREF    _PD

    XDEF    _Transfer

_Transfer:
; Transfer(PInfo, y, ptr)
; struct PrtInfo *PInfo
; UWORD y;
; UBYTE *ptr;
;
;  a0 =  PInfo
;  a1 =  ColorInt ptr
;  a2 =  PD
;  a3 =  ptr

;  d0 =  width
;  d1 =  dvalue
;  d2 =  scratch

      link     A5,#$FFF8
      movem.l  d2-d6/a2-a3,-(sp)    ;save regs used

      movea.l  $8(a5),a0            ;PInfo
      movea.l  _PD,a2               ;PD
      movea.l  $10(a5),a3           ;ptr


*   ColorInt = PInfo->pi_ColorInt;  /* ptr to array of int */
      movea.l  pi_ColorInt(a0),a1

*   width = PInfo->pi_width;        /* source width        */
      move.w   pi_width(a0),d0
      subq.w   #$1,d0


      cmpi.w   #SHADE_BW,pd_Preferences+pf_PrintShade(a2)
      beq      bw

* color or grey scale dump
mc:
      move.b   PCMRED(a1),(a3)+       ;red
      move.b   PCMGREEN(a1),d2       ;green
      asl.b    #$4,d2
      or.b     PCMBLUE(a1),d2         ;blue
      move.b   d2,(a3)+
      addq.l   #ce_SIZEOF,a1

      dbf      d0,mc
      bra.s    exit

* black and white dump

bw:
      moveq.l  #$0,d1
      move.w   pi_threshold(a0),d1
      eori.l   #$f,d1
mbw:
      move.b   PCMBLACK(a1),d2
      addq.l   #ce_SIZEOF,a1
      cmp.b    d1,d2
      ble.s    wht
      move.b   #$0F,(a3)+
      move.b   #$ff,(a3)+
      bra      loop
wht:
      move.b   #$00,(a3)+
      move.b   #$00,(a3)+
loop:
      dbf      d0,mbw

exit:
      movem.l  (sp)+,d2-d6/a2-a3   ;restore regs used
      moveq.l  #0,d0               ;flag all ok
      unlk     a5
      rts                         ;goodbye

    END
