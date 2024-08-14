;************************************************************
; File: syscall.inc
;************************************************************
; Parameter Block for 68000 calling 8086 software interrupt
;************************************************************
syscall86      struc		      ; all members overridable
r_ax	       dw      0
r_bx	       dw      0
r_cx	       dw      0
r_dx	       dw      0
r_si	       dw      0
r_ds	       dw      0
r_di	       dw      0
r_es	       dw      0
r_bp	       dw      0
psw	       dw      0	       ; Status register (returned)
int_nr	       dw      0	       ; Interrupt #, low byte used
syscall86      ends
;*************************************************************
; Parameter Block for 8086 calling 68000 routines
; Note: Memory is defined in double words, but
; 68000 reads the memory in long words. This means the
; following dd's have low and high word exchanged.
; The 68000 reads it as a long word through the
; word access area of the JANUS interface.
; Seel also words.inc
; All size specifiers are given in bytes, and must
; be even.
;************************************************************
syscall68      struc

d0	       dd      0       ; Data Registers
d1	       dd      0
d2	       dd      0
d3	       dd      0
d4	       dd      0
d5	       dd      0
d6	       dd      0
d7	       dd      0

a0	       dd      0       ; Address Registers
a1	       dd      0
a2	       dd      0
a3	       dd      0
a4	       dd      0
a5	       dd      0
a6	       dd      0

pc_68k	       dd      0       ; Call Address
sp_68k	       dd      0       ; Stack Area for Arguments
sp_len_68k     dd      0       ; Length of 68000 Stack Area (in bytes,
			       ; must be even). 0 indicates nothing on stack.
sp_depth_68k   dd      0       ; Minimum stack depth required on 68000 (bytes)
			       ; 0 indicates: Use default stack size.
psw_68k        dd      0       ; status
syscall68      ends

