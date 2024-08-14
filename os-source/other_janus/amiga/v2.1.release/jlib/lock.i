; macro to lock access to janus data structures from PC side

LOCK     MACRO    ; ( \1 -- effective address of lock byte )
begin\@:
      tas      \1
      bpl.s    exit\@
      nop
      nop
      bra.s    begin\@
exit\@:
      endm

*; macro to try once to lock access to janus data structures from PC side
*; changes D0 to 1 if lock gotten, 0 if lock not gotten
*
*LOCKTRY     MACRO    ; ( \1 -- effective address of lock byte )
*begin\@:
*      MOVEQ.L  #1,D0
*      TAS      \1
*      BPL.S    exit\@
*      MOVEQ.L  #0,D0
*exit\@:
*      ENDM
*
*UNLOCK      MACRO
*      MOVE.B   #$7f,\1
*      ENDM
