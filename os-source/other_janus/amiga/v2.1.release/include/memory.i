   IFND  JANUS_MEMORY_I
JANUS_MEMORY_I  SET  1

;*************************************************************************
; (Amiga side file)
;
; Memory.i -- Structures and defines for Janus emeory
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
; 7-15-88 - Bill Koester - Modified for self inclusion of required files
;*************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

* magic constants for memory allocation 

MEM_TYPEMASK      EQU   $00ff    ; 8 memory areas
MEMB_PARAMETER    EQU   $0       ; parameter memory
MEMB_BUFFER       EQU   $1       ; buffer memory

MEMF_PARAMETER    EQU   $1       ; parameter memory
MEMF_BUFFER       EQU   $2       ; buffer memory

MEM_ACCESSMASK    EQU   $3000    ; bits that participate in access types
MEM_BYTEACCESS    EQU   $0000    ; pointer to byte access address space
MEM_WORDACCESS    EQU   $1000    ; pointer to word access address space
MEM_GRAPHICACCESS EQU   $2000    ; ptr to graphic access address space
MEM_IOACCESS      EQU   $3000    ; pointer to i/o access address space

TYPEACCESSTOADDR  EQU   5        ; # of bits to turn access mask to addr

; The amiga side of the janus board has four sections of its address space.
; Three of these parts are different arrangements of the same memory.  The
; fourth part has the specific amiga accesible IO registers (jio_??).
; The other three parts all contain the same data, but the data is arranged
; in different ways: Byte Access lets the 68k read byte streams written
; by the 8088, Word Access lets the 68k read word streams written by the
; 8088, and Graphic Access lets the 68k read medium res graphics memory
; in a more efficient manner (the pc uses packed two bit pixels; graphic
; access rearranges these data bits into two bytes, one for each bit plane).

ByteAccessOffset     EQU   $00000
WordAccessOffset     EQU   $20000
GraphicAccessOffset  EQU   $40000
IoAccessOffset       EQU   $60000


; within each bank of memory are several sub regions.  These are the
; definitions for the sub regions

BufferOffset      EQU      $00000
ColorOffset       EQU      $10000
ParameterOffset   EQU      $18000
MonoVideoOffset   EQU      $1c000
IoRegOffset       EQU      $1e000

BufferSize        EQU      $10000
ParameterSize     EQU      $04000

; constants for sizes of various janus regions

JANUSTOTALSIZE    EQU   512*1024 ; 1/2 megabyte
JANUSBANKSIZE     EQU   128*1024 ; 128K per memory bank
JANUSNUMBANKS     EQU   4        ; four memory banks
JANUSBANKMASK     EQU   $60000   ; mask bits for bank region


; all bytes described here are described in the byte order of the 8088.
; Note that words and longwords in these structures will be accessed from
; the word access space to preserve the byte order in a word -- the 8088
; will access longwords by reversing the words : like a 68000 access to the
; word access memory

; JanusMemHead -- a data structure roughly analogous to an exec mem chunk.
; It is used to keep track of memory used between the 8088 and the 68000.


 STRUCTURE  JanusMemHead,0

   UBYTE    jmh_Lock             ; lock byte between processors
   UBYTE    jmh_pad0
   APTR     jmh_68000Base        ; rptr's are relative to this
   UWORD    jmh_8088Segment      ; segment base for 8088
   RPTR     jmh_First            ; offset to first free chunk
   RPTR     jmh_Max              ; max allowable index
   UWORD    jmh_Free             ; total number of free bytes -1

   LABEL    JanusMemHead_SIZEOF



 STRUCTURE  JanusMemChunk,0

   RPTR     jmc_Next             ; rptr to next free chunk
   UWORD    jmc_Size             ; size of chunk

   LABEL    JanusMemChunk_SIZEOF

* === ===================================================================== 
* === JanusRemember Structure ============================================= 
* === ===================================================================== 
* This is the structure used for the JRemember memory management routines 

 STRUCTURE  JanusRemember,0

   RPTR     jrm_NextRemember   ; Pointer to the next JanusRemember struct
   RPTR     jrm_Offset         ; Janus offset to this memory allocation 
   USHORT   jrm_Size           ; Size of this memory allocation
   USHORT   jrm_Type           ; Type of this memory allocation

   LABEL    JanusRemember_SIZEOF


   ENDC  ;End of JANUS_MEMORY_I conditional assembly
