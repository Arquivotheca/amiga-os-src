
;**********************************************************************
;
; janus.i -- software conventions for janus.i
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved
;
;***********************************************************************


	IFND	EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC	EXEC_TYPES_I

	IFND	EXEC_LIBRARIES_I
	INCLUDE "exec/libraries.i"
	ENDC	EXEC_LIBRARIES_I

	IFND	EXEC_INTERRUPTS_I
	INCLUDE "exec/interrupts.i"
	ENDC	EXEC_INTERRUPTS_I


; JanusResource -- an entity which keeps track of the reset state of the 8088
;   if this resource does not exist, it is assumed the 8088 can be reset

 STRUCTURE  JanusResource,LN_SIZE
    APTR	jr_BoardAddress ; address of JANUS board
    UBYTE	jr_Reset	; non-zero indicates 8088 is held reset
    LABEL	JanusResource_SIZEOF


; As a coding convenience, we assume a maximum of 32 handlers.
; People should avoid using this in their code, because we want
; to be able to relax this constraint in the future.  All the
; standard commands' syntactically support any number of interrupts,
; but the internals are limited to 32.

MAXHANDLER	EQU	32

; JanusAmiga -- amiga specific data structures for janus project

 STRUCTURE JanusAmiga,LIB_SIZE
    ULONG	ja_IntReq		; software copy of outstanding requests
    ULONG	ja_IntEna		; software copy of enabled interrupts
    APTR	ja_ParamMem		; ptr to (word arranged) param mem
    APTR	ja_IoBase		; ptr to base of io register region
    APTR	ja_ExpanBase		; ptr to start of shared memory
    APTR	ja_ExecBase		; ptr to exec library
    APTR	ja_DOSBase		; ptr to DOS library
    APTR	ja_SegList		; holds a pointer to our code segment
    APTR	ja_IntHandlers		; base of array of int server ptrs
    STRUCT	ja_IntServer,IS_SIZE	; INTB_PORTS server
    STRUCT	ja_ReadHandler,IS_SIZE	; JSERV_READAMIGA handler
    LABEL	JanusAmiga_SIZEOF


; hide a byte quantity in the lib_pad field
ja_SpurriousMask    EQU     LIB_pad

; magic constants for memory allocation
MEM_TYPEMASK	    EQU     $00ff   ; 8 memory areas
     BITDEF    MEM,PARAMETER,0	    ; parameter memory
     BITDEF    MEM,BUFFER,1	    ; buffer memory

MEM_ACCESSMASK	    EQU     $3000   ; bits that participate in access types
MEM_BYTEACCESS	    EQU     $0000   ; return base suitable for byte access
MEM_WORDACCESS	    EQU     $1000   ; return base suitable for word access
MEM_GRAPHICACCESS   EQU     $2000   ; return base suitable for graphic access
MEM_IOACCESS	    EQU     $3000   ; return base suitable for io access

TYPEACCESSTOADDR    EQU     5	    ; # of bits to change access mask into addr



; macro to lock access to janus data structures from PC side
LOCK		MACRO	; ( \1 -- effective address of lock byte )
begin\@:
		tas	\1
		beq.s	exit\@
		nop
		nop
		bra.s	begin\@
exit\@:
		endm

UNLOCK		MACRO	; ( \1 -- effective address of lock byte )
		move.b	#0,\1
		ENDM

JANUSNAME	MACRO
		dc.b	'janus.library',0
		ENDM