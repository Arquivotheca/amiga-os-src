;*****************************************************************************
;
; macros.inc	       
;
;*****************************************************************************
.xlist
;
popall	macro				; pop all regs
	popac
	pop	bp
	pop	di
	pop	si
	pop	ds
	pop	es
	endm

pushall macro				; push all regs
	push	es
	push	ds
	push	si
	push	di
	push	bp
	pushac
	endm

pushac	macro				; push accumulators
	push	dx
	push	cx
	push	bx
	push	ax
	endm

popac	macro				; pop accumulators
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	endm

icall	macro	pointer 		; CALL an INT- Entry
	pushf				; later iret expects flags on stack
	call	dword ptr pointer	; then segm/offset
	endm

iflags	macro				; move flags to - 4[sp]
					; on stack:
					; cc		   [8]
					; ret segment	   [6]
					; ret offset	   [4]
					;
	push	bp			; bp		   [2]
	push	ax			; ax  <- [sp]	   [0]

	pushf				; flags to ax
	pop	ax
	mov	bp,sp			; store current flags
	mov	8[bp],ax		; for later iret
	pop	ax			; restore used regs
	pop	bp

	endm
;******************************************************************************
; infolevel may be overwritten later in routine
;
infolevel = 10	      ; print all messages
;
;******************************************************************************
;
;  Locking macros for JanusMemory
;
;  Locks access to janus data structures from AMIGA side
;
;******************************************************************************

LOCK	  macro pointer

start:
     lock or   pointer,40h
	  jnl  exit
	  nop
	  nop
	  jmp  short start
exit:

	  endm


UNLOCK	  macro pointer

	  mov  pointer,0

	  endm
.list



