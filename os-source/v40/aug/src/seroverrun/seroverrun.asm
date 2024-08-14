*
*       Peter's crazy buffer-overflow sampler
*
*       Or Berlitz's "Learn to speak 68000-assembly in 30 minutes a day" :-)
*
*       (aka "Pardon my Accent")
*
*
*       Adds an interrupt server that pulls data from the serial bus
*       as quickly as it becomes available, then checks for the overrun
*       condition.  If so, it stores the return address from the interrupt,
*       in hopes of figuring out who's been disabling for so long.
*
*


                INCLUDE 'hardware/custom.i'
                INCLUDE 'hardware/intbits.i'

                XREF    _custom
                XDEF    _handler

RBFULLB         EQU     14              ; serdatr read-buffer-full flag
OVERRUNB        EQU     15              ; serdatr overrun flag

                section code

* Where is the interrupt return address?
* Well, count 8*4 for pushed registers in exec (incl. return address)
* and 2 into the stack frame:

STACKOFFSET     EQU     34
* cribbed $42 from angel
*STACKOFFSET    EQU     $42

*       Register usage:
*       d0: read from serdatr register
*       d1: temp
*       a0: custom chip base
*       a1: is_Data region
*       a5: table location to write to
*       a6: 1st location past end of table

*       data area (a1) defined as:
*       ULONG next writable address in table
*       ULONG address of longword past end-of-table
*       ULONG [] table of return addresses
*

switchaddr      equ     $f012f8         ; switch address Bryce is interested in
                                        ; correct for A2000 KS 37.59.f0

_handler:
                lea.l   _custom,a0
                moveq   #0,d1

again:          move.w  serdatr(a0),d0
                btst.l  #RBFULLB,d0     ; was there data?
                beq.s   nodata          ; nope

                moveq   #1,d1           ; flag that I've seen a byte
                move.w  #INTF_RBF,intreq(a0)    ; clear the interrupt

                btst.l  #OVERRUNB,d0    ; did we overrun?
                beq.s   again           ; nope

                bchg.b  #1,$bfe001      ; toggle the LED
                                        ; a1 contains our data area
                move.l  (a1)+,a5        ; a5 = next writable location
                move.l  (a1),a6         ; a6 = first illegal location
                cmpa.l  a5,a6           ; a6-a5 had better be > 0
                bcs.s   nomoreroom      ; no more room in the table
                subq.l  #4,a1           ; back up to the 'next writable location'

                move.l  STACKOFFSET(sp),(a5)+   ; fill the stack buffer with return address
                move.l  a5,(a1)         ; update 'next writable location'
       ;         cmpi.l  #switchaddr,STACKOFFSET(sp)     ; the address of interest?
       ;         bne.s   again           ; no
                move.l  #$555,$2FC      ; Trigger analyzer in the standard way
                bra.s   again

nomoreroom:
                clr.l   0               ; enforcer away...

nodata:
                move.w  d1,d0           ; Set Z flag to indicate "not mine", or not
                rts


