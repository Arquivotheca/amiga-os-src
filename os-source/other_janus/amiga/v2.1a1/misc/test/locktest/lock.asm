   INCLUDE "janus/janus.i"

      XDEF     _MyLock
;     XDEF     _MyUnlock
;  	  XDEF     _MyInitLock
;	  XDEF     _MySoftLock


_MyLock:
      move.l  d0,-(sp)
1$:
      bsr     _MyLockAttempt
      tst.l    D0
      beq      1$
      move.l  (sp)+,d0
      rts
          

_MyLockAttempt:
      MOVEQ.L  #1,D0
      TAS      (a0)
      BPL.S    1$
      MOVEQ.L  #0,D0
1$:
      RTS



;_MyUnlock: 
;_MyInitLock:
;      move.b   #$7f,(A0)
;      rts
;
;_MySoftLock:
;      move.l  d0,-(sp)
;1$:
;      bsr     _MySoftLockAttempt
;      tst.l    D0
;      beq      1$
;      move.l  (sp)+,d0
;      rts
;
;_MySoftLockAttempt:
;      move.l   a2,-(sp)
;      MOVEQ.L  #1,D0
;      move.l   jb_ParamMem(a6),a2      * pointer to JanusAmiga
;      move.b   #$01,ja_AmigaFlag(a2)
;      move.b   #$00,ja_Turn(a2)
;1$:
;      cmp.b    #1,ja_PCFlag(a2)
;      bne      4$
;      cmp.b    #0,ja_Turn(a2)
;      beq      1$
;4$
;;                                       * Master lock gotten
;      cmp.b    #$ff,(a0)
;      beq      2$
;      move.b   #$ff,(a0)
;      bra      3$
;2$
;      MOVEQ.L  #0,D0
;3$
;      move.b   #0,ja_AmigaFlag(a2)     * release master lock
;      move.l   (sp)+,a2
;      rts
;
      END

