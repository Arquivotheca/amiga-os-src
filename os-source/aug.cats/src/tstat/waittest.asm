
* CrashTest

	INCLUDE "exec/types.i"
	INCLUDE "dos/dos.i"

_LVOWait	EQU	-318

     SECTION CODE

     movea.l sp,a3

     move.l #$00010203,-(sp)
     move.l #$04050607,-(sp)
     move.l #$08090a0b,-(sp)
     move.l #$0c0d0e0f,-(sp)
     move.l #$10111213,-(sp)
     move.l #$14151617,-(sp)
     move.l #$18191a1b,-(sp)
     move.l #$1c1d1e1f,-(sp)
     move.l #$20212223,-(sp)
     move.l #$24252627,-(sp)
     move.l #$28292a2b,-(sp)
     move.l #$2c2d2e2f,-(sp)
     move.l #$30313233,-(sp)
     move.l #$34353637,-(sp)
     move.l #$38393a3b,-(sp)
     move.l #$3c3d3e3f,-(sp)

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

     move.l sp,a4
     lea.l   next(PC),a5
next:
     movea.l 4,a6
     move.l  #SIGBREAKF_CTRL_C,d0
     jsr     _LVOWait(a6)
     movea.l a3,sp     
     rts


     CNOP 0,2

one  DC.B '1234'
two  DC.B '5678'


     END