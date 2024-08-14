
**** Exports
	    XDEF    _SerialTest
****


	    INCLUDE "exec/types.i"
	    INCLUDE "exec/nodes.i"
	    INCLUDE "exec/ables.i"
	    INCLUDE "libraries/dos.i"
	    INCLUDE "hardware/custom.i"
	    INCLUDE "hardware/intbits.i"
JSRLIB	    MACRO
	    XREF    _LVO\1
	    jsr     _LVO\1(a6)
	    ENDM


BAUDRATE    EQU     19200
SERPER	    EQU     (3579545/BAUDRATE)-1



	    move.l  #$dff180,a5
	    move.w  #SERPER,serper(a5)


	    XDEF    _SerialTest
_SerialTest:
	    ;[Fall-thru]

;Set our new serial interrupt vector
	    move.l  4,a6
	    move.l  #INTB_RBF,d0
	    lea.l   RBF_IS(pc),a1
	    JSRLIB  SetIntVector
	    move.l  d0,a2
	    ;[A2=old RBF vector]


;enable the interrupt

	    move.w  #INTF_RBF,intreq(a5)                ;clear any pending
	    move.w  #INTF_SETCLR!INTF_RBF,intena(a5)    ;enable RBF

;wait for CTRL-C
	    move.l  #SIGBREAKF_CTRL_C,d0
	    JSRLIB  Wait

	    move.w  #INTF_SETCLR!INTF_RBF,intena(a5)    ;disable RBF
	    move.w  #INTF_RBF,intreq(a5)                ;clear any pending


;Restore peace and tranquility
	    move.l  #INTB_RBF,d0
	    move.l  a2,a1	    ;restore previous
	    JSRLIB  SetIntVector
	    moveq   #20,d1
	    lea.l   RBF_IS(pc),a1
	    cmp.l   d0,d1
	    bne.s   bad_compare
	    moveq   #0,d1
bad_compare:
	    move.l  d1,d0
	    rts


;
;The interrupt code itself!
;

RBF_int:    move.w  serdatr(a0),d0          ;Get status & data (same word)
	    move.w  d0,0
	    bpl.s   all_ok
	    move.w  #$0f00,$dff180
all_ok	    move.w  #INTF_RBF,intreq(a0)    ;Ack the byte & clear the IRQ
	     ;and.w   #$00ff,d0
	     ;or.w    #$0100,d0   ;Add in a stop bit
	     ;move.w  d0,serdat(a0)  ;Echo character back
	    bchg.b  #1,$bfe001
	    rts


;-----------------------------------------

RBF_IS	    dc.l    0		    ;LN_SUCC
	    dc.l    0		    ;LN_PRED
	    dc.b    NT_INTERRUPT    ;LN_TYPE
	    dc.b    0		    ;LN_PRI
	    dc.l    MyName	    ;LN_NAME
	    dc.l    $12345678	    ;IS_DATA
	    dc.l    RBF_int	    ;IS_CODE

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

MyName	    dc.b    'error rate tester',0

