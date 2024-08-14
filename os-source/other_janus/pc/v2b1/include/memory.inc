;*************************************************************************
; (PC side file)
;
; Memory.inc -- Structures and defines for Janus memory
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************


;* magic constants for memory allocation 

MEM_TYPEMASK      =   000ffh   ; 8 memory areas
MEMB_PARAMETER    =   0        ; parameter memory
MEMB_BUFFER       =   1        ; buffer memory

MEMF_PARAMETER    =   (1 SHL 0)   ; parameter memory
MEMF_BUFFER       =   (1 SHL 1)   ; buffer memory

MEM_ACCESSMASK    =   03000h   ; bits that participate in access types
MEM_BYTEACCESS    =   00000h   ; pointer to byte access address space
MEM_WORDACCESS    =   01000h   ; pointer to word access address space
MEM_GRAPHICACCESS =   02000h   ; ptr to graphic access address space
MEM_IOACCESS      =   03000h   ; pointer to i/o access address space

TYPEACCESSTOADDR  =   5        ; # of bits to turn access mask to addr

; The amiga side of the janus board has four sections of its address space.
; Three of these parts are different arrangements of the same memory.  The
; fourth part has the specific amiga accesible IO registers (jio_??).
; The other three parts all contain the same data, but the data is arranged
; in different ways: Byte Access lets the 68k read byte streams written
; by the 8088, Word Access lets the 68k read word streams written by the
; 8088, and Graphic Access lets the 68k read medium res graphics memory
; in a more efficient manner (the pc uses packed two bit pixels; graphic
; access rearranges these data bits into two bytes, one for each bit plane).

ByteAccessOffsetH     =   00h
ByteAccessOffsetL     =   0000h
WordAccessOffsetH     =   02h
WordAccessOffsetL     =   0000h
GraphicAccessOffsetH  =   04h
GraphicAccessOffsetL  =   0000h
IoAccessOffsetH       =   06h
IoAccessOffsetL       =   0000h


; within each bank of memory are several sub regions.  These are the
; definitions for the sub regions

BufferOffsetH      =      00h
BufferOffsetL      =      0000h
ColorOffsetH       =      01h
ColorOffsetL       =      0000h
ParameterOffsetH   =      01h
ParameterOffsetL   =      08000h
MonoVideoOffsetH   =      01h
MonoVideoOffsetL   =      0c000h
IoRegOffsetH       =      01h
IoRegOffsetL       =      0e000h

BufferSizeH        =      01h
BufferSizeL        =      0000h
ParameterSizeH     =      00h
ParameterSizeL     =      04000h

; constants for sizes of various janus regions

JANUSTOTALSIZE    =   512*1024 ; 1/2 megabyte
JANUSBANKSIZE     =   128*1024 ; 128K per memory bank
JANUSNUMBANKS     =   4        ; four memory banks
JANUSBANKMASKH    =   06h      ; mask bits for bank region
JANUSBANKMASKL    =   0000h


; all bytes described here are described in the byte order of the 8088.
; Note that words and longwords in these structures will be accessed from
; the word access space to preserve the byte order in a word -- the 8088
; will access longwords by reversing the words : like a 68000 access to the
; word access memory

; JanusMemHead -- a data structure roughly analogous to an exec mem chunk.
; It is used to keep track of memory used between the 8088 and the 68000.


JanusMemHead   struc

    jmh_Lock        db 0     ; lock byte between processors
    jmh_pad0        db 0
    jmh_68000Base   dd 0     ; rptr's are relative to this
    jmh_8088Segment dw 0     ; segment base for 8088
    jmh_First       dw 0     ; offset to first free chunk
    jmh_Max         dw 0     ; max allowable index
    jmh_Free        dw 0     ; total number of free bytes -1

JanusMemHead   ends

JanusMemChunk  struc

     jmc_Next dw 0            ; rptr to next free chunk
     jmc_Size dw 0            ; size of chunk -1

JanusMemChunk  ends


;* === ===================================================================== 
;* === JanusRemember Structure ============================================= 
;* === ===================================================================== 
;* This is the structure used for the JRemember memory management routines 

JanusRemember  struc

   jrm_NextRemember  dw 0 ; Pointer to the next JanusRemember struct
   jrm_Offset        dw 0 ; Janus offset to this memory allocation
   jrm_Size          dw 0 ; Size of this memory allocation
   jrm_Type          dw 0 ; Type of this memory allocation

JanusRemember  ends


