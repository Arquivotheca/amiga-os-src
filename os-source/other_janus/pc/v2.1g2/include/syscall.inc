;**********************************************************************
; (PC side file)
;
; syscall.inc -- interface definitions between amiga and commodore-pc
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;**********************************************************************


; All registers in this section are arranged to be read and written
; from the WordAccessOffset area of the shared memory.   If you really
; need to use the ByteAccessArea, all the words will need to be byte
; swapped.


; Syscall86 -- how the 8086/8088 wants its parameter block arranged

Syscall86   struc

    s86_AX   	dw 0
    s86_BX   	dw 0
    s86_CX   	dw 0
    s86_DX   	dw 0
    s86_SI   	dw 0
    s86_DS   	dw 0
    s86_DI   	dw 0
    s86_ES   	dw 0
    s86_BP   	dw 0
    s86_PSW  	dw 0
    s86_INT  	dw 0	; 8086 int # that will be called

Syscall86   ends



; Syscall68 -- the way the 68000 wants its parameters arranged

Syscall68   struc

   s68_D0         dd 0
   s68_D1         dd 0
   s68_D2         dd 0
   s68_D3         dd 0
   s68_D4         dd 0
   s68_D5         dd 0
   s68_D6         dd 0
   s68_D7         dd 0
   s68_A0         dd 0
   s68_A1         dd 0
   s68_A2         dd 0
   s68_A3         dd 0
   s68_A4         dd 0
   s68_A5         dd 0
   s68_A6         dd 0
   s68_PC         dd 0	; pc to start execution from
   s68_ArgStack   dd 0	; array to be pushed onto stack
   s68_ArgLength  dd 0	; number of bytes to be pushed (must be even)
   s68_MinStack   dd 0	; minimum necessary stack (0 = use default)
   s68_CCR        dd 0	; condition code register
   s68_Process    dd 0	; ptr to process for this block.
   s68_Command    dw 0	; special commands: see below
   s68_Status     dw 0	;
   s68_SigNum     dw 0	; internal use: signal to wake up process

Syscall68   ends


S68COM_DOCALL  = 0       ; normal case -- jsr to specified Program cntr
S68COM_REMPROC = 1       ; kill process
S68COM_CRPROC  = 2       ; create the proces, but do not call anything



