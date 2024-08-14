
;*************************************************************************
;
; janusvar.i -- the software data structures for the janus board
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************

; all bytes described here are described in the byte order of the 8088.
; Note that words and longwords in these structures will be accessed from
; the word access space to preserve the byte order in a word -- the 8088
; will access longwords by reversing the words : like a 68000 access to the
; word access memory

; JanusMemHead -- a data structure roughly analogous to an exec mem chunk.
; It is used to keep track of memory used between the 8088 and the 68000.


 STRUCTURE JanusMemHead,0
	UBYTE	jmh_Lock	    ; lock byte between processors
	UBYTE	jmh_pad0
	APTR	jmh_68000Base	    ; rptr's are relative to this
	UWORD	jmh_8088Segment     ; segment base for 8088
	RPTR	jmh_First	    ; offset to first free chunk
	RPTR	jmh_Max 	    ; max allowable index
	UWORD	jmh_Free	    ; total number of free bytes -1
	LABEL	JanusMemHead_SIZEOF

 STRUCTURE JanusMemChunk,0
	RPTR	jmc_Next	    ; rptr to next free chunk
	UWORD	jmc_Size	    ; size of chunk -1
	LABEL	JanusMemChunk_SIZEOF

 STRUCTURE JanusBase,0
	UBYTE	jb_Lock 	    ; also used to handshake at 8088 reset
	UBYTE	jb_8088Go	    ; unlocked to signal 8088 to initialize
	STRUCT	jb_ParamMem,JanusMemHead_SIZEOF
	STRUCT	jb_BufferMem,JanusMemHead_SIZEOF
	RPTR	jb_Interrupts
	RPTR	jb_Parameters
	UWORD	jb_NumInterrupts
	LABEL	JanusBase_SIZEOF


;------ constant to set to indicate a pending software interrupt
JSETINT 	EQU	$7f