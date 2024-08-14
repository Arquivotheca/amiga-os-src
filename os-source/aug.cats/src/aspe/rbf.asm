;-----------------------------------------------
; Amiga Serial Performance Evaluator RBF handler
; written by Darius Taghavy
; (c) 1990 Commodore Business Machines, Inc.
;-----------------------------------------------
   SECTION CODE
   XDEF    _RBFHand                   ; C addressable entry point

   XREF    _value                     ; holds current velocity etc
   XREF    _receive_count             ; total number of received bytes

   SECTION DATA
SERDATR  equ   $018                   ; Serial Port Data/status read
INTREQ   equ   $09c                   ; interrupt request bits (clear or set)
INTF_RBF equ   $0800                  ; = 2048 dec (bit 11)

   SECTION CODE

_RBFHand move.w   SERDATR(a0),d1         ; Get data byte
                                         ; (a0 points to custom chip base)
         move.w   #INTF_RBF,INTREQ(a0)   ; Reset request bit
;        move.b   d1,_value              ; save data
         addq.l   #1,_receive_count      ; increment receive byte counter
         moveq    #0,d0                  ; return
         rts

   END
