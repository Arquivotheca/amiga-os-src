
* Crash3m

     SECTION CODE

     move.l #$CACACACA,-(sp)
     moveq #0,d0
     moveq #1,d1
     moveq #2,d2
     moveq #3,d3
     moveq #4,d4
     moveq #5,d5
     moveq #6,d6
     moveq #7,d7
     movea.l #16,a0
     movea.l #17,a1
     movea.l #18,a2
     movea.l #19,a3
     movea.l #20,a4
     movea.l #21,a5
     movea.l #22,a6
     move.l  a7,a3
     lea.l   next(PC),a2
next:
     move.l  1,-(sp)
     rts


     CNOP 0,2

one  DC.B '1234'
two  DC.B '5678'

     END
