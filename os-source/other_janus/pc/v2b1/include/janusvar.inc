;*************************************************************************
; (PC side file)
;
; janusvar.inc -- the software data structures for the janus board
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
; Date       Name
; --------   -------------	---------------------------------------------
; 07-15-88 - Bill Koester  - Modified for self inclusion of required files
; 07-26-88 - Bill Koester  - Added ja_Reserved to JanusAmiga
;                            Added ja_AmigaState, ja_PCState and related
;										flags to JanusAmiga
;
;*************************************************************************

;NOTE Needs Memory.inc

; all bytes described here are described in the byte order of the 8088.
; Note that words and longwords in these structures will be accessed from
; the word access space to preserve the byte order in a word -- the 8088
; will access longwords by reversing the words : like a 68000 access to the
; word access memory

JanusAmiga  struc

   ja_Lock           db 0        ; also used to handshake at 8088 reset
   ja_8088Go         db 0        ; unlocked to signal 8088 to initialize
;------------------------------- JanusMemHead of ParamMem
   jpm_Lock          db 0
   jpm_pad0          db 0
   jpm_68000Base     dd 0
   jpm_8088Segment   dw 0
   jpm_First         dw 0
   jpm_Max           dw 0
   jpm_Free          dw 0
;------------------------------- JanusMemHead of BufferMem
   jbm_Lock          db 0
   jbm_pad0          db 0
   jbm_68000Base     dd 0
   jbm_8088Segment   dw 0
   jbm_First         dw 0
   jbm_Max           dw 0
   jbm_Free          dw 0
;-------------------------------
   ja_Interrupts     dw 0
   ja_Parameters     dw 0
   ja_NumInterrupts  dw 0

   ; This field is used by Janus.library to communicate Amiga states
   ; to the PC. The bits of this field may be read by anyone, but
   ; may be written only by janus.library.
   
   ja_AmigaState	dw 0
   ; This field is used by the PC to communicate PC states
   ; to the Amiga. The bits of this field may be read by anyone, but
   ; may be written only by the PC operating system.
   
   ja_PCState	dw 0
   
   ja_Reserved dd 4 dup (0)

JanusAmiga  ends

; === AmigaState Definitions === 
AMIGASTATE_RESERVED	= 0FFFEh
AMIGA_NUMLOCK_SET	= 0001h
AMIGA_NUMLOCK_SETn      = 0
AMIGA_NUMLOCK_RESET     = 0002h
AMIGA_NUMLOCK_RESETn    = 1

; === PCState Definitions === 
PCSTATE_RESERVED	= 0FFFFh


;------ constant to set to indicate a pending software interrupt

JSETINT  	=   07fh

