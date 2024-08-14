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

;
; Structure for 'PUSHALL'-stack
;
StackFrame	struc

RegAX		dw	0
RegBX		dw	0
RegCX		dw	0
RegDX		dw	0
RegBP		dw	0
RegDI		dw	0
RegSI		dw	0
RegDS		dw	0
RegES		dw	0

StackFrame	ends

;******************************************************************************


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

;******************************************************************************


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

JLOCK	  macro semaphore
	  local jlstart,jlexit
jlstart:
	  stc	
    lock  rcl  semaphore,1		; !!! should use lock prefix
	  jnc  jlexit
	  nop
	  nop
	  jmp  short jlstart
jlexit:

	  endm


UNLOCK	  macro semaphore

	  mov  semaphore,7fh

	  endm

;******************************************************************************
;
;  INFO macros for printing debug messageses on screen
;
;******************************************************************************

INFO	macro	message	       		; macro to print message
         if   	DBG_&message 
	  pushf
	  push 	si
	  push 	ds
	  push 	cs
	  pop  	ds
	  mov  	si,offset message
	  call 	pstrng
	  call 	newline
	  pop  	ds
	  pop  	si
	  popf
         endif
	endm 	


INFO_AX	macro	message			; macro to print message and
         if   	DBG_&message 		;  a hex value in AX
	  pushf
	  push	ax
	  push 	si
	  push 	ds
	  push 	cs
	  pop  	ds
	  mov  	si,offset message
	  call 	pstrng
	  call 	outhxw
	  call 	newline
	  pop  	ds
	  pop  	si
	  pop	ax	
	  popf
         endif
	endm 	


INFO_AX_BX 	macro	message		; macro to print message and
         if   	DBG_&message 		;  decimal values in AX and BX
	  pushf
	  push	ax
	  push 	si
	  push 	ds
	  push 	cs
	  pop  	ds
	  mov  	si,offset message
	  call 	pstrng
	  call 	outint
	  mov  	al,'.'
	  call 	outchr
	  mov	ax,bx
	  call 	outint
	  call 	newline
	  pop  	ds
	  pop  	si
	  pop	ax	
	  popf
         endif
	endm 	

.list



