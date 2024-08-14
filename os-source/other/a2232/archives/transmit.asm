
*********** Exports

	    XDEF    _SerialTest

***********


	    INCLUDE "exec/ables.i"
	    INCLUDE "hardware/custom.i"
	    INT_ABLES

***********
index	    EQUR    a6

BAUDRATE    EQU     19200
SERPERVAL   EQU     (3579545/BAUDRATE)-1
***********



	    move.l  #$dff180,a0
	    move.w  #SERPERVAL,serper(a0)


_SerialTest:
	    ;[Fall-thru]
	    move.l  4,a6
	    DISABLE	    ;We're taking over, and not giving it back!

;--Pump out a test pattern
Start:	    lea.l   pattern(pc),index
TopLoop:
FlushSer:   move.w  $DFF018,D1	;Custom chip access must be word aligned
	    btst    #13,D1	;Flight 777 to tower. All clear?
	    beq.s   FlushSer	;Roger, all clear.  Procede w/next byte.
	    moveq   #0,d0
	    move.b  (index)+,d0 ;Get character to be transmitted
	    beq.s   ExitLoop
	    or.w    #$0100,d0	;Add in a stop bit
	    move.w  d0,$DFF030
	    bra.s   TopLoop

ExitLoop:   move.l  counter,d0
	    addq.l  #1,d0
	    move.l  d0,counter
	    and.b   #$0f,d0
	    bne.s   noblink
		bchg.b	#1,$bfe001  ;Blink LED
noblink     btst.b  #6,$bfe001	;Test
	    bne.s   Start

	    move.l  counter,d0
	    rts





;-----------------------------------------
counter dc.l	0	;How many iterations

;
; Test pattern common to both sides.
;
;
pattern:    dc.b    $f0
	    dc.b    $0f
	    dc.b    $01
	    dc.b    $23
	    dc.b    $45
	    dc.b    $67
	    dc.b    $89
	    dc.b    $ab
	    dc.b    $cd
	    dc.b    $ef
	    dc.b    $55
	    dc.b    $aa
	    dc.b    $cc
	    dc.b    $66
	    dc.b    $ff
	    dc.b    0	    ;End marker
