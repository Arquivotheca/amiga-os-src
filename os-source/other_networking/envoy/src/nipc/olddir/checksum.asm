
         xdef     _CalcChecksum

* CalcChecksum(ptr to data, # of bytes in this segment, previous checksum);

_CalcChecksum:
         movem.l  d3/d2,-(sp)
         move.l   04+8(sp),a0
         move.l   08+8(sp),d0
         move.l   12+8(sp),d1

         moveq    #0,d2          * a nice 0 for adding in those trailing X's.
         asr.w    #1,d0
         subq     #1,d0          * for the dbra
         add.w    d2,d2          * clears X
1$       move.w   (a0)+,d3
         addx.w   d3,d1
         dbra     d0,1$
         addx     d2,d1

         btst     #0,11+8(sp)
         beq      2$
         move.b   (a0)+,d0
         lsl.w    #8,d0
         add.w    d2,d2          * clears X
         addx.w   d0,d1
         addx.w   d2,d1

2$       ext.l    d1
         move.l   d1,d0
         movem.l  (sp)+,d3/d2
         rts



