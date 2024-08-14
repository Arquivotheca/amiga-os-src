;-------------------------------------------------------------------
; transmit_sysx.asm
;-------------------------------------------------------------------
; assemble under MANX: as -o transmit_sysx.o transmit_sysx.asm
; assemble under LATTICE: asm -iinclude: transmit_sysx.asm
;-------------------------------------------------------------------
; written by Darius Taghavy, CATS
; (c)1989 by Commodore Business Machines, Inc.
; All rights reserved
;-------------------------------------------------------------------
                        SECTION DATA
; Custom Chip Base Address, registers and flags
;----------------------------------------------
CUSTOM_CHIPS   equ      $dff000  ;Base address
TBE            equ      5        ;Transmitt Buffer Empty

;READ ONLY REGISTER
SERDATR        equ      $018     ;Serial Port Data/status read 

;WRITE ONLY REGISTER
SERDAT         equ      $030     ;Serial Data Output Reg


;--------------------------------------------------------------------
                        SECTION CODE

	XDEF	_trans

;trans( APTR data, LONG count );
;---------------------------------------------------------------------
_trans                        ; Send N number of bytes.
   movem.l  a0/a1,-(a7)       ; push registers on stack
   lea      CUSTOM_CHIPS,a0   ; Custom Chip base pointer

   movea.l  4+8(a7),a1        ; Get pointer to byte data.
   move.l   8+8(a7),d0        ; Get count.

1$ move.b   (a1)+,d1          ; get data 
   andi.w   #$ff,d1           ; set all flags for subsequent or		
   ori.w    #$100,d1          ; Set Stop Bit.

2$ btst.b   #TBE,SERDATR(a0)  ; wait until serial port is ready
   beq      2$                ; to be written to

   move.w   d1,SERDAT(a0)     ; Send byte out.
   subq.l   #1,d0             ; dec count
   bne      1$                ; loop until done

   movem.l  (a7)+,a0/a1       ; pop registers off stack 
   rts                        ; return to main()

   END
