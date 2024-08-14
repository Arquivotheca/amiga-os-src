   IFND  JANUS_JANUSVAR_I
JANUS_JANUSVAR_I  SET   1

;*************************************************************************
; (Amiga side file)
;
; janusvar.i -- the software data structures for the janus board
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
; Date       Name
; --------   -------------   -------------------------------------------
; 07-15-88 - Bill Koester  - Modified for self inclusion of required files
; 07-26-88 - Bill Koester  - Added ja_Reserved to JanusAmiga
;                            Added ja_AmigaState, ja_PCState and related
;                            flags to JanusAmiga
;*************************************************************************

   IFND  JANUS_MEMORY_I
   INCLUDE "janus/memory.i"
   ENDC

; all bytes described here are described in the byte order of the 8088.
; Note that words and longwords in these structures will be accessed from
; the word access space to preserve the byte order in a word -- the 8088
; will access longwords by reversing the words : like a 68000 access to the
; word access memory

 STRUCTURE  JanusAmiga,0

   UBYTE    ja_Lock                   ; also used to handshake at 8088 reset
   UBYTE    ja_8088Go                 ; unlocked to signal 8088 to initialize
   STRUCT   ja_ParamMem,JanusMemHead_SIZEOF
   STRUCT   ja_BufferMem,JanusMemHead_SIZEOF
   RPTR     ja_Interrupts
   RPTR     ja_Parameters
   UWORD    ja_NumInterrupts

   ; This field is used by janus.library to communicate Amiga states
   ; to the PC.  The bits of this field may be read by anyone, but
   ; may be written only by janus.library.

   USHORT   ja_AmigaState

   ; This field is used by the PC to communicate PC states
   ; to the Amiga.  The bits of this field may be read by anyone, but
   ; may be written only by the PC operating system.
   ;
   USHORT   ja_PCState

   STRUCT   ja_Reserved,4*4

   LABEL    JanusAmiga_SIZEOF

; === AmigaState Definitions ===
AMIGASTATE_RESERVED  EQU $FFFC
AMIGA_NUMLOCK_SET    EQU $0001
AMIGA_NUMLOCK_SETn   EQU   0
AMIGA_NUMLOCK_RESET   EQU   $0002
AMIGA_NUMLOCK_RESETn   EQU   1

; === PCState Definitions ===
PCSTATE_RESERVED     EQU $FFFF

;------ constant to set to indicate a pending software interrupt

JSETINT  EQU   $7f

   ENDC  ;End of JANUS_JANUSVAR_I conditional assembly
