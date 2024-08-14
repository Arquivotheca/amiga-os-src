page 63,132
;*****************************************************************************
; Autoboot handlers:
;
; 
; Bill13 handles BIOS INT13 ......
; ------
;
; Will be called from PC. 
;
; Called from INT 13: 	Entry
;		     	- Jump to old INT13 if ......
;
;
;
; Bill19 handles BIOS INT19 ......
; ------
;
; Will be called from PC. 
;
; Called from INT 19: 	Entry
;		     	- Attempt boot off drive A:
;			- Attempt boot off drive B:
;			- Attempt boot off drive C: (our fake drive)
;			- Call bios int 18h Rom Basic (should not return)
;			- iret
;
;
;
; New code  :   25-Mar-88 Bill Koester
; 54.Update :  	27-jun-88 BK		load filename for autoboot drive
; 55.Update :  	28-jun-88 BK		no reset for harddisk
; 56.Update :  	18-jul-88 BK		allow multiple sector r/w for autoboot
; 57.Update :	20-jul-88 TB		no reset for 'strange' drives	    
;               29-nov-90 rsd		pcdisk to dosserv conversion
;
;******************************************************************************

public	       	Bill13, Bill19

cseg 		segment	para public 'code'

     		assume	cs:cseg,ss:cseg,ds:cseg,es:nothing

extrn	       	JanInt:near		; janus interupt entry

; external utilities

extrn	       	outhxw:near		; prints hex word in ax
extrn		outhxb:near		;prints hex byte in al
extrn	       	newline:near		; prints cr,lf
extrn	       	pstrng:near		; prints out string
extrn		change_int:near		
	
w	       	equ     word ptr
b		equ	byte ptr

include        	equ.inc
include		macros.inc
include        	vars_ext.inc
include 	abdata.inc		
include		abequ.inc
include		abblock.inc
;include		abmacros.inc
;include		janus\pcdisk.inc
include		janus\services.inc
include		janus\dosserv.inc
	
;*************************************************************************
;*
;*	bill13	       - Autoboot int 13 wedge
;*
;*************************************************************************
bill13	proc far

	pushf			;Save state
	pushall
	mov	bp,sp		; Set bp for structure access of stack frame

				; Dos redirects the janus int during boot
				; So we redirect it EVERY time
	push 	cs
	pop  	es		     	; set ES to load segment
	mov  	al,srv_int		; redirect IRQ3 
	mov  	di,offset JanInt 
	call 	change_int
	mov w 	cs:chain_vec+2,es 	     	; and save old pointer
	mov w 	cs:chain_vec,di


; *** kludge it !
	mov	ax,ss:StackFrame.RegAX[bp]	
	or	ah,ah
	jne	nokludge
	mov	dx,ss:StackFrame.RegDX[bp]
	cmp	dl,80h
	je	nokludge
	cmp	dl,3
	je	nokludge
	cmp	dl,2
	je	nokludge
	cmp	dl,1
	je	nokludge
	or	dl,dl
	je	nokludge
	mov 	dl,0
	mov	ss:StackFrame.RegDX[bp],dx
nokludge:

	if 	AB_INT13MSG
	push	bp
	      	push 	cs			;Print debug message
		pop 	ds
		mov	si,offset AB_billmsg0   ;func
	   	call 	pstrng		       
		mov	ax,ss:StackFrame.RegAX[bp]	
	   	mov	al,ah		  
	   	call 	outhxb		       
	   	mov	si,offset AB_billmsg1	;drive
	   	call 	pstrng		       
		mov	dx,ss:StackFrame.RegDX[bp]
		mov	al,dl
		call	outhxb
	   	mov	si,offset AB_billmsg2	;cyl
	   	call 	pstrng		       
		mov	cx,ss:StackFrame.RegCX[bp]
		mov	al,ch
		call	outhxb
	   	mov	si,offset AB_billmsg3	;head
	   	call 	pstrng
		mov	dx,ss:StackFrame.RegDX[bp]
 		mov	al,dh
		call	outhxb		       
	   	mov	si,offset AB_billmsg4	;sec
	   	call 	pstrng
		mov	cx,ss:StackFrame.RegCX[bp]
		mov	al,cl
		and	al,00111111b
		call	outhxb       
	   	mov	si,offset AB_billmsg5	;#secs
	   	call 	pstrng
		mov	ax,ss:StackFrame.RegAX[bp]
		call	outhxb		       
	   	call 	newline 		    
	pop	bp
	endif

	sti				;allow services requiring interrupts
					;to actually work.  amazing.


      	cmp	dl,80h		; Is this a req for drive 80H
	je	forme
	jmp	notme		; No. so call old handler
forme:

	and b	cs:ActiveFlag,not Dos	; Clear DOS flag so janus int works

	cmp w	cs:AB_FileOpen,1	; Is our file open?
	je	dispatch		;yes - proceed.

	push	bp
	call	first_time		; No. so Attempt open
	pop	bp	

	cmp w	cs:AB_FileOpen,1		;If file not open pretend
	je	dispatch 			;were not here and call old
	jmp	notme				;handler

dispatch:  					;Dispatch req to proper func
	mov	ax,ss:StackFrame.RegAX[bp]
	or	ah,ah
	jne	dp1
	jmp	func0
dp1:	dec 	ah
	jne	dp2
	jmp	func1
dp2:	dec	ah
	jne	dp3
	jmp	func2
dp3:	dec	ah
	jne	dp4
	jmp	func3
dp4:	dec	ah
	jne	dp5
	jmp	func4
dp5:	dec	ah
	jne	dp6
	jmp	func5
dp6:	dec	ah
	jne	dp7
	jmp	func6
dp7:	dec	ah
	jne	dp8
	jmp	func7
dp8:	dec	ah
	jne	dp9
	jmp	func8
dp9:	dec	ah
	jne	dpa
	jmp	func9
dpa:	dec	ah
	jne	dpb
	jmp	funca
dpb:	dec	ah
	jne	dpc
	jmp	funcb
dpc:	dec	ah
	jne	dpd
	jmp	funcc
dpd:	dec	ah
	jne	dpe
	jmp	funcd
dpe:	dec	ah
	jne	dpf
	jmp	funce
dpf:	dec	ah
	jne	dp10
	jmp	funcf
dp10:	dec	ah
	jne	dp11
	jmp	func10
dp11:	dec	ah
	jne	dp12
	jmp	func11
dp12:	dec	ah
	jne	dp13
	jmp	func12
dp13:	dec	ah
	jne	dp14
	jmp	func13
dp14:	dec	ah
	jne	badexit
	jmp	func14

badexit:			; Generic exit point for returning
	popall			; an error with no params
	popf
	mov	ah,1
	stc
	iflagsi
	iret

okexit:				; Generic exit point for returning
	popall			; no error and no params
	popf
        xor	ah,ah
	clc
	iflagsi
	iret

notme:	popall			; Restore state and call old handler
	popf			; as if we didn't exist

	pushf
        call 	dword ptr cs:[AB_bill_int13]	
	iflagsi
	iret

;****************************************************************************
;
; All functions are entered with a pushall frame on the stack and BP
; pointing to the frame. Original parameters can be accessed via
;
;	mov	reg,ss:StackFrame.RegXX[bp]
;
; On exit, each routine should:
;				popall		; Clean up stack
;				popf
;				stuff return values
;				set or clear carry flag
;				iret
;
;****************************************************************************
;
; FUNCTION 0  - Reset the disk
;
; Input  Regs:	AH = 00
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func0:	
	jmp	okexit

;****************************************************************************
;
; FUNCTION 1  - Read status of last disk operation
;
; Input  Regs:	AH = 01
;		DL = Drive #
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func1:
	jmp	okexit

;****************************************************************************
;
; FUNCTION 2  - Read specific sectors to memory
;
; Input  Regs:	AH = 02
;	     ES:BX = Address of memory buffer
;		DL = Drive #
;		DH = Head
;		CX = Track or cylinder, sector
;		AL = # of sectors
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func2:				;Read specific sectors to memory
	;cnv_do, compute ofset and count
	;do read and transfer to dest es:bx

	mov	ax,JFUNC_GETBASE*100h + JSERV_AMIGASERVICE
	int 	JFUNC_JINT

	call	lock_dosserv

	call	rw_setup		;set up for read/write
	or	ax,ax
	jnz	read_err		;seek failed.

	;let the games begin!
	mov	ax,cs:AB_Secs		;ax=sectors he wants
	mov	cl,cs:AB_secs_per_buf	;cl=sectors we can do at a shot
	div	cl			;al=# of shots, ah=leftovers
	push	ax			;save it
	or	al,al			;any shots at all?
	je	last			;nope.  do leftovers.

	xor	ch,ch
	mov	cs:AB_Secs,cx		;do max num of sectors
	mov	cl,al			;cx = num of shots
morer:
	push	cx
	call	DoReadCopy		;boink
	pop	cx
	or	ax,ax			;did we get any?
	jz	read_err		;no - get out.
	loop	morer
	
last:
	pop	ax			;restore shots/leftover count
	or	ah,ah			;any leftovers?
	je	noremain		;nope.
	mov	al,ah			;ax = leftovers
	xor	ah,ah
	mov 	cs:AB_Secs,ax
	call	DoReadCopy		;whap
	or	ax,ax			;get any?
	jz	read_err		;no. shucks.
noremain:

	mov	ax,cs:AB_SecsDone		; return value
	mov	ss:StackFrame.RegAX[bp],ax

	call	unlock_dosserv
	
	jmp	okexit				; normal exit

read_err:
	mov	ax,cs:AB_SecsDone		; return value
	mov	ss:StackFrame.RegAX[bp],ax

	call	unlock_dosserv
	
	jmp	badexit				; normal exit


;****************************************************************************
;
; FUNCTION 3  - Write specific sectors from memory
;
; Input  Regs:	AH = 03
;	     ES:BX = Address of memory buffer
;		DL = Drive #
;		DH = Head
;		CX = Track or cylinder, sector
;		AL = # of sectors
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func3:					
	;cnv_do, compute offset and count
	;do transfer to pcdisk_buf and write

	call	lock_dosserv

	call	rw_setup			;do math & seek
	or	ax,ax
	jnz	write_err			;seek failed.

	;start writing.
	mov	ax,cs:AB_Secs			;ax = secs to write
	mov	cl,cs:AB_secs_per_buf		;cl = max secs at a shot
	div	cl				;al=# of shots, ah=leftovers
	push	ax				;save it
	or	al,al				;only leftovers left?
	je	last3				;yes - do less than max

	xor	ch,ch
	mov	cs:AB_Secs,cx			;we're doing max secs/shot
	mov	cl,al				;cx = # of shots	
morer3:
	push	cx
	call	DoCopyWrite			;do a shot
	pop	cx
	or	ax,ax				;did we do any?
	jz	write_err			;no.
	add	cs:AB_SecsDone,ax		;add 'em up.
	loop	morer3
last3:
	pop	ax				;recall leftover value
	or	ah,ah				;any leftovers?
	je	noremain3			;no - we be done.
	mov	al,ah				;ax = leftovers
	xor	ah,ah
	mov 	cs:AB_Secs,ax			;do this many sectors
	call	DoCopyWrite			;foom.
	or	ax,ax
	jz	write_err			;oops.
	add	cs:AB_SecsDone,ax		;add 'em up.

noremain3:
	mov	ax,cs:AB_SecsDone
	mov	ss:StackFrame.RegAX[bp],ax

	call	unlock_dosserv
	
	jmp okexit

write_err:
	mov	ax,cs:AB_SecsDone
	mov	ss:StackFrame.RegAX[bp],ax

	call	unlock_dosserv
	
	jmp badexit

;****************************************************************************
;
; FUNCTION 4  - Verify specific sectors
;
; Input  Regs:	AH = 04
;		DL = Drive #
;		DH = Head
;		CX = Track or cylinder, sector
;		AL = # of sectors
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func4:
	jmp	okexit

;****************************************************************************
;
; FUNCTION 5  - Format specific track
;
; Input  Regs:	AH = 05
;		DL = Drive #
;		DH = Head
;		CX = Track or cylinder, sector
;		AL = # of sectors
;	     ES:BX = Address of format information
;		     (floppy only)
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func5:
	jmp 	okexit

;****************************************************************************
;
; FUNCTION 6  - Unused function
;
; Input  Regs:	AH = 06
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func6:
	jmp	badexit

;****************************************************************************
;
; FUNCTION 7  - Unused function
;
; Input  Regs:	AH = 07
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func7:
	jmp	badexit

;****************************************************************************
;
; FUNCTION 8  - Get the current drive parameters
;
; Input  Regs:	AH = 08
;		DL = Drive #
;
; Output Regs:	AH = Status
;		CL = Sectors per track (bits 0-5)
;		     Maximum cylinders (bits 6-7)
;		CH = Maximum cylinders
;		DH = Maximum heads
;		DL = # of drives
;		CY = 0	No Error
;		     1	Error
;
func8:				
	popall
	popf

	mov	ax,cs:AB_NumHeads
	dec	ax
	mov	dh,al
	mov	dl,1				;1 hard drive

;	mov 	ss:w StackFrame.RegDX[bp],dx


	mov	ax,cs:AB_TrackSecs
	and	al,00111111b


	mov	cx,cs:AB_Cyls
    	dec	cx				;adjust to reflect max cyl #
	xchg	ch,cl
	ror	cl,1
	ror	cl,1
	or	cl,al


	xor	ah,ah
	clc
	iflagsi
	iret

;****************************************************************************
;
; FUNCTION 9  - Initialize drive characteristics
;
; Input  Regs:	AH = 09
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func9:
	jmp	okexit

;****************************************************************************
;
; FUNCTION A  - Read long to memory (Read data and 4 ECC bytes)
;
; Input  Regs:	AH = 0A
;	     ES:BX = Address of memory buffer
;		DL = Drive #
;		DH = Head
;		CX = Track or cylinder, sector
;		AL = # of sectors
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
funca:
	jmp	badexit

;****************************************************************************
;
; FUNCTION B  - Write long from memory (Write data and 4 ECC bytes)
;
; Input  Regs:	AH = 0B
;	     ES:BX = Address of memory buffer
;		DL = Drive #
;		DH = Head
;		CX = Track or cylinder, sector
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
funcb:
	jmp	badexit

;****************************************************************************
;
; FUNCTION C  - Seek
;
; Input  Regs:	AH = 0C
;		DL = Drive #
;		DH = head
;		CX = Track or cylinder, sector
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
funcc:
	jmp	okexit

;****************************************************************************
;
; FUNCTION D  - Reset drive
;
; Input  Regs:	AH = 0D
;		DL = Drive #
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
funcd:
	jmp	okexit

;****************************************************************************
;
; FUNCTION E  - Unused function
;
; Input  Regs:	AH = 0E
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
funce:
	jmp	badexit

;****************************************************************************
;
; FUNCTION F  - Unused function
;
; Input  Regs:	AH = 0F
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
funcf:
	jmp	badexit

;****************************************************************************
;
; FUNCTION 10 - Test drive ready
;
; Input  Regs:	AH = 10
;		DL = Drive #
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func10:
	jmp	okexit

;****************************************************************************
;
; FUNCTION 11 - Recalibrate
;
; Input  Regs:	AH = 11
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func11:
	jmp	okexit

;****************************************************************************
;
; FUNCTION 12 - Unused function
;
; Input  Regs:	AH = 12
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func12:
	jmp	badexit

;****************************************************************************
;
; FUNCTION 13 - Unused function
;
; Input  Regs:	AH = 13
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func13:
	jmp 	badexit

;****************************************************************************
;
; FUNCTION 14 - Run controller diagnostic
;
; Input  Regs:	AH = 14
;
; Output Regs:	AH = Status
;		CY = 0	No Error
;		     1	Error
;
func14:
	jmp	okexit

Bill13	endp
	
;*************************************************************************
;*
;*	rw_setup
;*
;*	input:		bp points to stack frame of caller
;*	output:		AB_Secs, AB_dataseg, AB_dataoff, AB_head,
;*			AB_Sec, AB_Cyl all set up from bp input
;*			AB_SecsDone = 0
;*			seek performed
;*	returns:	ax = error from seek function (0=ok)
;*
;*************************************************************************
rw_setup	proc	near

	;get # of sectors to write 
	mov	ax,ss:StackFrame.RegAX[bp]
	xor	ah,ah
	mov	cs:AB_Secs,ax

	;get source memory addr
	mov	es,ss:StackFrame.RegES[bp]
	mov	cs:AB_dataseg,es
	mov	bx,ss:StackFrame.RegBX[bp]
	mov	cs:AB_dataoff,bx		;stash pointers

	;get head number
	mov	dx,ss:StackFrame.RegDX[bp]
	mov	bl,dh				;bx has head #
	xor	bh,bh
	mov	cs:AB_Head,bx

	;get cylinder and sector number
	mov	cx,ss:StackFrame.RegCX[bp]
	mov	ax,cx
	and	ax,03fh
	mov	cs:AB_Sec,ax
	and	cl,0c0h
	rol	cl,1
	rol	cl,1
	xchg	ch,cl
	mov	cs:AB_Cyl,cx

	mov	cs:AB_SecsDone,0		;no work done yet

	mov	bx,cs:AB_Head
	mov	ax,cs:AB_Cyl
	mov	cx,cs:AB_Sec

	if	DBG_AB_CNV_IN
	 pushall
	 call	outhxw
	 mov	ax,bx
	 call	outhxw
	 mov	ax,cx
	 call	outhxw
	 call	newline
	 popall	     
	endif

	mul	cs:AB_NumHeads		;ax = cyl*hds
	add	ax,bx			;ax = cyl*hds+hd
	mul	cs:AB_TrackSecs		;dx:ax = (cyl*hds+hd)*spt
	add	ax,cx			;dx:ax = (cyl*hds+hd)*spt+sec
	adc	dx,0
	mov	dh,dl			;dx:ax = dx:ax * 512
	mov	dl,ah
	mov	ah,al
	sub	al,al
	shl	ax,1
	rcl	dx,1

	if	DBG_AB_CNV_OUT
	 pushall
	 push	ax
	 mov	ax,dx
	 call	outhxw
	 pop	ax
	 call	outhxw
	 call	newline
	 popall
	endif

	les	si,cs:AB_ds_rq
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_SEEK_BEGINING
	mov 	es:DOSServReq.dsr_Arg2_h[si],dx		;offs high
	mov	es:DOSServReq.dsr_Arg2_l[si],ax		;offs low
	mov	ax,word ptr cs:AB_File
	mov	es:DOSServReq.dsr_Arg1_l[si],ax
	mov	ax,word ptr cs:AB_File+2
	mov	es:DOSServReq.dsr_Arg1_h[si],ax
	call	DoJanus

	ret
rw_setup	endp
;*************************************************************************
;*
;*	DoCopyWrite	-	Expects: adr_Offset
;*					 cs:dataoff set up
;*			 		 # of secs in cs:AB_Secs
;*	returns		AX = number of sectors written
;*	destroys	everything except BP,AX
;*************************************************************************
DoCopyWrite	proc	near

	cld
	mov	ds,cs:AB_dataseg
	mov	si,cs:AB_dataoff
	les	di,cs:AB_ds_buf_ptr
	mov	ax,cs:AB_Secs
	mov	cx,512
	mul	cx
	mov	cx,ax
if	(0 ge 1)
	rep	movs es:byte ptr[di],ds:[si]
else
	shr	cx,1
	rep	movsw
endif
	mov	cs:AB_dataoff,si

	les	si,cs:AB_ds_rq
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_WRITE
	mov	es:DOSServReq.dsr_Arg2_l[si],ax
	mov  	es:DOSServReq.dsr_Arg2_h[si],dx
	mov	ax,word ptr cs:AB_File
	mov	es:DOSServReq.dsr_Arg1_l[si],ax
	mov	ax,word ptr cs:AB_File+2
	mov  	es:DOSServReq.dsr_Arg1_h[si],ax
	call	DoJanus
	or	ax,ax
	jz	dowrite_ok

	xor	ax,ax			;fail - return 0
	jmp	dowrite_exit

dowrite_ok:
	mov	ax,cs:AB_Secs

dowrite_exit:
	ret

DoCopyWrite	endp
;*************************************************************************
;*
;*	DoReadCopy	-	Expects: adr_Offset
;*					 cs:dataoff set up
;*			 		 # of secs in cs:AB_Secs
;*			 
;*************************************************************************
DoReadCopy	proc	near

	mov 	ax,cs:AB_Secs
	mov	cx,512
	mul	cx
	mov	cx,ax		;save for later

	les	si,cs:AB_ds_rq
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_READ
	mov	es:DOSServReq.dsr_Arg2_l[si],ax
	mov  	es:DOSServReq.dsr_Arg2_h[si],dx
	mov	ax,word ptr cs:AB_File
	mov	es:DOSServReq.dsr_Arg1_l[si],ax
	mov	ax,word ptr cs:AB_File+2
	mov  	es:DOSServReq.dsr_Arg1_h[si],ax
	call	DoJanus
	or	ax,ax
	jz	doread_ok

	xor	ax,ax			;fail - return 0
	jmp	doread_exit

doread_ok:
	;copy buff to mem pointer
	cld
	lds	si,cs:AB_ds_buf_ptr
	mov	es,cs:AB_dataseg
	mov	di,cs:AB_dataoff
if	(0 ge 1)
	rep	movs es:byte ptr[di],ds:[si]
else
	shr	cx,1
	rep	movsw
endif

	mov	cs:AB_dataoff,di

	mov	ax,cs:AB_Secs		;success - return # of secs read

doread_exit:
	ret

DoReadCopy	endp
;*************************************************************************
;*
;*	first_time	-	Entered if AB_FileOpen flag = 0
;*				Call GetBase(pcdisk) once to wake up
;*				services.
;*				If Second call fails - return
;*				PCDisk found.
;*				stash mem pointers.
;*
;*
;*************************************************************************
first_time	proc	near
	pushall

	mov	cx,3
first_loop:
	mov	ax,JFUNC_GETBASE*100h+JSERV_AMIGASERVICE
	int	JFUNC_JINT
	loop	first_loop

	cmp	di,-1				;amigaservice found?
	jne	get_dosserv

	if 	DBG_AB_NOPCDISKMSG
	push	bp
	      	push 	cs			;Print debug message
		pop 	ds
		mov	si,offset AB_NOPCDISKMSG   ;func
	   	call 	pstrng		       
	   	call 	newline 		    
	pop	bp
	endif
	jmp	exit1				;not found so exit
						;file still flagged as

get_dosserv:
	push	ds
	mov	ax,DOSSERV_APPLICATION_ID shr 16
	mov	DS,ax
	mov	SI,DOSSERV_APPLICATION_ID and 0FFFFH
	mov	CX,DOSSERV_LOCAL_ID
	xor	ax,ax
	mov	ES,ax
	mov	DI,ax
	mov	AX,JFUNC_GETSERVICE*100h + GETS_WAIT+GETS_ALOAD_A
	int	JFUNC_JINT
	pop	ds
	cmp	al,JSERV_OK
	jz	got_dosserv

	;... error msg here ...
	jmp	exit1

got_dosserv:
	;lock and unlock the servicedata structure (waits until fully initted)
	mov	AH,JFUNC_LOCKSERVICEDATA
;	mov	DI,word ptr cs:AB_sd_ds
	int	JFUNC_JINT
	mov	AH,JFUNC_UNLOCKSERVICEDATA
;	mov	DI,word ptr cs:AB_sd_ds
	int	JFUNC_JINT

	;es:di -> ServiceData - save in ds_sd
	mov	word ptr cs:AB_sd_ds,di
	mov	word ptr cs:AB_sd_ds+2,es

	;get DOSServReq struct ptr from ServiceData struct
	les	di,es:ServiceData.sd_PCMemPtr[di]
	mov	word ptr cs:AB_ds_rq,di
	mov	word ptr cs:AB_ds_rq+2,es

	;get buffer seg, offset, and length from DOSServReq struct
	mov	ax,es:DOSServReq.dsr_Buffer_Off[di]
	mov	word ptr cs:AB_ds_buf_ptr,ax
	mov	ax,es:DOSServReq.dsr_Buffer_Seg[di]
	mov	word ptr cs:AB_ds_buf_ptr+2,ax
	mov	ax,es:DOSServReq.dsr_Buffer_Size[di]
	mov	cs:AB_ds_buf_len,ax

	;calculate how many sectors the buffer will hold
	xor	dx,dx
	mov	cx,512
	div	cx		;ax = dx:ax / cx, dx = remainder
	or	ah,ah		;result exceeds 255?
	jnz	toobig		;oops.
	cmp	al,128		;>127?
	jc	use_it
toobig:
	mov	al,127		;yes!  must fit in a byte!
use_it:
	mov	cs:AB_secs_per_buf,al
	
get_ok:	
	call	lock_dosserv

	if DBG_AB_PCDISKMSG
		push	bp
		push	cs
		;Print debug message
		pop	ds
		mov	si,offset AB_PCDISKMSG	;func call pstrng call newline
		pop	bp
	endif


						;copy filename to buffer 
	cld
	push	cs
	pop	ds	
	mov	si,offset cs:AB_FileName
	les	di,cs:AB_ds_buf_ptr
	mov	cx,50
	rep	movs es:byte ptr[di],ds:[si]

	les	si,cs:AB_ds_rq
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_OPEN_OLD 
 	
	call	DoJanus

	if	DBG_AB_FILEOPENMSG
	 push	ax
 	 call	outhxb
	 call	newline
	 pop	ax
	endif

	if 	AB_CTRLOPENERRMSG
	push	bp
	      	push 	cs			;Print debug message
		pop 	ds
		mov	si,offset AB_billmsg6   ;func
	   	call 	pstrng		       
		push 	es
		push	si
		les	si,cs:AB_ds_rq
		mov 	ax,es:DOSServReq.dsr_Err[si]
		pop	si
		pop	es
	   	call 	outhxw	     
	   	call 	newline 		    
	pop	bp
	endif

	or	ax,ax
	je	opentok

	INFO	AB_COPENFAILMSG 
    	jmp	exit				;File remains closed!
						;and flag clear
		     
opentok:
	;save temp handle for close
	mov	ax,es:DOSServReq.dsr_Arg1_l[si]
	mov	word ptr cs:AB_TFile,ax
	mov	ax,es:DOSServReq.dsr_Arg1_h[si]
	mov	word ptr cs:AB_TFile+2,ax

	;read 512 bytes from the file
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_READ
;	mov	ax,word ptr cs:AB_TFile
;	mov	es:DOSServReq.dsr_Arg1_l[si],ax
;	mov	ax,word ptr cs:AB_TFile+2
;	mov	es:DOSServReq.dsr_Arg1_h[si],ax
	mov 	es:DOSServReq.dsr_Arg2_h[si],0		;cnt high
	mov	es:DOSServReq.dsr_Arg2_l[si],512	;cnt low = 512 bytes
	call	DoJanus

	push	ax
	;close file.	
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_CLOSE
	mov	ax,word ptr cs:AB_TFile
	mov	es:DOSServReq.dsr_Arg1_l[si],ax
	mov	ax,word ptr cs:AB_TFile+2
	call	DoJanus
	pop	ax
	or	ax,ax			;did the read fail?
	je	readcok			;no.  keep going.

	jmp	exit				;run away!

readcok:
	mov	cx,es:DOSServReq.dsr_Arg3_l[si]	;how many did we get?
	jcxz	open_fail			;none!  panic.
	
	les	si,cs:AB_ds_buf_ptr		;point to data
more:	cmp b	es:[si],00ah			;scan for '\n'
	je	endfound
	inc	si
	loop	more
	
endfound:
	mov b	es:[si],0			;terminate with null

	;open the file whose name is in the buffer
	les	si,cs:AB_ds_rq
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_OPEN_OLD 
	call	DoJanus

	if	DBG_AB_FILEOPENMSG
	 push	ax
	 call	outhxb
	 call 	newline
	 pop	ax
	endif

	or ax,ax
	je	openok

open_fail:
	INFO	AB_OPENFAILMSG 
    	jmp	exit				;File remains closed!
						;and flag clear
		  
openok:	
	;save handle
	mov	ax,es:DOSServReq.dsr_Arg1_l[si]
	mov	word ptr cs:AB_File,ax
	mov	ax,es:DOSServReq.dsr_Arg1_h[si]
	mov	word ptr cs:AB_File+2,ax

	;read 1st 512 bytes from "partition"
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_READ
;	mov	ax,word ptr cs:AB_File
;	mov	es:DOSServReq.dsr_Arg1_l[si],ax
;	mov	ax,word ptr cs:AB_File+2
;	mov	es:DOSServReq.dsr_Arg1_h[si],ax
	mov 	es:DOSServReq.dsr_Arg2_h[si],0		;cnt high
	mov	es:DOSServReq.dsr_Arg2_l[si],512	;cnt low = 512 bytes
	call	DoJanus
	or	ax,ax				;read errors?
	jnz	ab_read_fail
	cmp	es:DOSServReq.dsr_Arg3_l[si],512	;got 512 bytes?
	jnz	ab_read_fail

	les	si,cs:AB_ds_buf_ptr
	cmp	byte ptr es:AB_BLOCK.ab_name[si],'A'	;check file format
	jne	ab_read_fail			;accept default as hard
						;get params
	INFO	AB_READPARMS

	mov	ax,es:AB_BLOCK.Heads[si]
	xchg	ah,al
	mov	cs:AB_NumHeads,ax

	mov	ax,es:AB_BLOCK.SecTrk[si]
	xchg	ah,al
	mov	cs:AB_TrackSecs,ax

	mov	ax,es:AB_BLOCK.Cyl[si]
	xchg	ah,al
	mov	cs:AB_Cyls,ax

	;flag aboot file as open - success!
      	mov	cs:AB_FileOpen,1
	jmp	exit

ab_read_fail:
;;;
		push	si
		mov	si,offset rf_msg
	   	call 	pstrng
	   	pop	si
	   	jmp	rf_around
rf_msg	   	db	"firsttime: read failed",13,10,0
rf_around:
;;;
	;file couldn't be read, or was garbage.  close it down.
	les	si,cs:AB_ds_rq
	mov 	es:DOSServReq.dsr_Function[si],DSR_FUNC_CLOSE
	mov	ax,word ptr cs:AB_File
	mov	es:DOSServReq.dsr_Arg1_l[si],ax
	mov	ax,word ptr cs:AB_File+2
	mov	es:DOSServReq.dsr_Arg1_h[si],ax
	call	DoJanus

exit:
	call	unlock_dosserv

exit1:	
	INFO	AB_FTEXITMSG
	popall
	ret

first_time	endp
						;closed
;*************************************************************************
;*
;*	lock_dosserv		make DOSServReq our very own
;*
;*************************************************************************
lock_dosserv	proc near
	les	di,cs:AB_ds_rq
	lea	di,es:DOSServReq.dsr_Lock[di]
	mov	ah,JFUNC_LOCK
	int	JFUNC_JINT
	ret
lock_dosserv	endp
	
;*************************************************************************
;*
;*	unlock_dosserv		free DOSServReq
;*
;*************************************************************************
unlock_dosserv	proc near
	les	di,cs:AB_ds_rq
	lea	di,es:DOSServReq.dsr_Lock[di]
	mov	ah,JFUNC_UNLOCK
	int	JFUNC_JINT
	ret
unlock_dosserv	endp

;*************************************************************************
;*
;*	DoJanus	       - Initiate and wait for a pcdisk request
;*
;*************************************************************************
DoJanus	proc	near

	push	es
	push	di

	mov	AH,JFUNC_CALLSERVICE	;call a service
	les	di,cs:AB_sd_ds		;(dosserv) - es used later on
	int	JFUNC_JINT

	cmp	al,JSERV_OK		;did it work?
	jnz	error			;no.

;	sti

	;wait for service to complete
waitloop:
	test	word ptr es:ServiceData.sd_Flags[di],SERVICE_PCWAIT
	jnz	waitloop

	les	di,cs:AB_ds_rq		;now check for an error in the call
	mov	ax,es:DOSServReq.dsr_Err[di]
	or	ax,ax
	jz	exitd			;no problem. ax is 0.

error:
	mov	ax,-1			;oops.

exitd:
	pop	di
	pop	es

	ret
	
DoJanus	endp	
;*************************************************************************
;*
;*	bill19	       - Attempt boot from drive a:
;*						 b:
;*						 c:
;*
;*************************************************************************
bill19	proc	far	
	
	pushall
	SUB	DX,DX			; Head=0, Drive = A:
BOOTNXT:
	MOV	CX,3			; retry
BOOTDK0:
	PUSH	CX
;	STI				; enable interrupts
	XOR	AH,AH			; Reset diskette
	INT	13h			; ....
	xor	ax,ax
	mov	es,ax
	mov	bx,7c00h		; read boot code here
	MOV	CX,0001H		; CH = Track 0, CL = sector 1
	MOV	AX,0201H		; AH = read command, AL = read 1 sector
	INT	13h			; do the read
	POP	CX			; count restore
	JC	BOOTDK1 		; failed start again

	xor	ax,ax
	MOV	ES,AX
	CMP	ES:[7DFEH],0AA55H	; Valid boot block?
	JNE	BOOTFAIL

	popall
	db	0eah			; far jump to 0:7c00h
	dw	7c00h
	dw	0
BOOTDK1:

	LOOP	BOOTDK0			; Loop for CX retrys
	INC	DL			; Next Drive
	CMP	DL,81H			; If 81h then last was c: 80h
	JE	BOOTFAIL
	CMP	DL,2			; All drives done ?
	JB	BOOTNXT 		; no if below
	MOV	DL,80H			; Floppys done try hard drive
	MOV	CX,3			; 3 retrys
	JMP	BOOTDK0


BOOTFAIL:
	popall
	INT	18H			; Call INT 18h but don't return

	IRET

Bill19	endp

cseg	ends
 
end 





