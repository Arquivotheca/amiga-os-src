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

include		macros.inc
include        	vars_ext.inc
include        	equ.inc
include 	abdata.inc		
include		abequ.inc
include		abblock.inc
;include		abmacros.inc
include		janus\pcdisk.inc
include		janus\services.inc
	
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
	cmp	ah,0
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
	cmp	dl,0
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


      	cmp	dl,80h		; Is this a req for drive 80H
	je	forme
	jmp	notme		; No. so call old handler
forme:

	and b	cs:ActiveFlag,not Dos	; Clear DOS flag so janus int works

	cmp w	cs:AB_FileOpen,1	; Is our file open?
	je	going
	push	bp
	call	first_time		; No. so Attempt open
	pop	bp	
going:	

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
	iret

okexit:				; Generic exit point for returning
	popall			; no error and no params
	popf
        xor	ah,ah
	clc
	iret

notme:	popall			; Restore state and call old handler
	popf			; as if we didn't exist

	pushf
        call 	dword ptr cs:[AB_bill_int13]	
	iflags
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

	mov	ah,JFUNC_GETBASE			;Wake up services
	mov	al,JSERV_PCDISK
	int 	JFUNC_JINT

	mov	ax,ss:StackFrame.RegAX[bp]
	xor	ah,ah
	mov	cs:AB_Secs,ax

	mov	es,ss:StackFrame.RegES[bp]
	mov	cs:AB_dataseg,es
	mov	bx,ss:StackFrame.RegBX[bp]
	mov	cs:AB_dataoff,bx		;stash pointers

	xor	bh,bh
	mov	dx,ss:StackFrame.RegDX[bp]
	mov	bl,dh				;bx has head #
	mov	cs:AB_Head,bx

	mov	cx,ss:StackFrame.RegCX[bp]
	mov	ax,cx
	and	ax,03fh
	mov	cs:AB_Sec,ax
	and	cl,0c0h
	rol	cl,1
	rol	cl,1
	xchg	ch,cl
	mov	cs:AB_Cyl,cx

	mov	cx,cs:AB_Secs 
	mov	cs:AB_SecsDone,cx

	mov	bx,cs:AB_Head
	mov	ax,cs:AB_Cyl
	mov	cx,cs:AB_Sec	
	call	cnv_do
						;store offset into param mem
						;and compute count 
						;we assume pcdisks buffer
						;can handle 17 secs
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov	es:AmigaDiskReq.adr_Offset_h[si],dx	;offset high
	mov	es:AmigaDiskReq.adr_Offset_l[si],ax	;offset low

	mov	ax,cs:AB_Secs
	mov	cl,17
	div	cl
	push	ax
	cmp	al,0
	je	last

	mov	cs:AB_Secs,17
	xor	cx,cx	
	mov	cl,al
morer:
	push	cx
	call	DoReadCopy
	pop	cx
	loop	morer
last:
	pop	ax
	cmp	ah,0
	je	noremain
	xor	al,al
	xchg	ah,al
	mov 	cs:AB_Secs,ax
	call	DoReadCopy
noremain:

	mov	ax,cs:AB_SecsDone		; return value
	mov	ss:StackFrame.RegAX[bp],ax

	jmp	okexit				; normal exit


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

						;copy buff to mem pointer

	mov	ax,ss:StackFrame.RegAX[bp]
	xor	ah,ah
	mov	cs:AB_Secs,ax

	mov	es,ss:StackFrame.RegES[bp]
	mov	cs:AB_dataseg,es
	mov	bx,ss:StackFrame.RegBX[bp]
	mov	cs:AB_dataoff,bx		;stash pointers

	xor	bh,bh
	mov	dx,ss:StackFrame.RegDX[bp]
	mov	bl,dh				;bx has head #
	mov	cs:AB_Head,bx

	mov	cx,ss:StackFrame.RegCX[bp]
	mov	ax,cx
	and	ax,03fh
	mov	cs:AB_Sec,ax
	and	cl,0c0h
	rol	cl,1
	rol	cl,1
	xchg	ch,cl
	mov	cs:AB_Cyl,cx


	mov	cx,cs:AB_Secs
	mov	cs:AB_SecsDone,cx

	mov	bx,cs:AB_Head
	mov	ax,cs:AB_Cyl
	mov	cx,cs:AB_Sec
	push	bp
	call	cnv_do
	pop	bp
						;store offset into param mem
	mov	es,cs:AB_jparmseg 		;compute count
	mov	si,cs:AB_jparmoff
	mov	es:AmigaDiskReq.adr_Offset_h[si],dx	;offset high
	mov	es:AmigaDiskReq.adr_Offset_l[si],ax	;offset low
;888
	mov	ax,cs:AB_Secs
	mov	cl,17
	div	cl
	push	ax
	cmp	al,0
	je	last3

	mov	cs:AB_Secs,17
	xor	cx,cx	
	mov	cl,al
morer3:
	push	cx
	call	DoCopyWrite
	pop	cx
	loop	morer3
last3:
	pop	ax
	cmp	ah,0
	je	noremain3
	xor	al,al
	xchg	ah,al
	mov 	cs:AB_Secs,ax
	call	DoCopyWrite
noremain3:
;888

	mov	ax,cs:AB_SecsDone
	mov	ss:StackFrame.RegAX[bp],ax

	jmp okexit

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

;	mov 	ss:w StackFrame.RegCX[bp],cx

	xor	ah,ah
	clc
	iret
;	jmp	okexit	

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
;*	cnv_do		-	Expects: BX	Head
;*			 		 AX     Cyl
;*			 		 CX	Sec
;*			 
;*			 	Returns: DX:AX	Offset
;*
;*************************************************************************
cnv_do	proc	near	      
	
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

;	cmp	cs:AB_SKIP512,1
;	je	skip	      
;	dec	cx
skip:	mul	cs:AB_NumHeads
	add	ax,bx
	mul	cs:AB_TrackSecs
	add	ax,cx
	adc	dx,0
	inc	cx
	mov	dh,dl
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

	ret
cnv_do	endp
;*************************************************************************
;*
;*	DoCopyWrite	-	Expects: adr_Offset
;*					 cs:dataoff set up
;*			 		 # of secs in cs:AB_Secs
;*			 
;*************************************************************************
DoCopyWrite	proc	near
	cld
	mov	ds,cs:AB_dataseg
	mov	si,cs:AB_dataoff
	mov	es,cs:AB_jbuffseg
	mov	di,cs:AB_jbuffoff
	mov	ax,cs:AB_Secs
	mov	cx,512
	mul	cx
	mov	cx,ax
	rep	movs es:byte ptr[di],ds:[si]
	mov	cs:AB_dataoff,si

	mov	ax,cs:AB_Secs
	mov	cx,512
	mul	cx
	mov	es,cs:AB_jparmseg 	       
	mov	si,cs:AB_jparmoff
	mov 	es:AmigaDiskReq.adr_Count_h[si],dx	;count high
	mov	es:AmigaDiskReq.adr_Count_l[si],ax	;count low = 512 bytes
	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_WRITE
	mov	ax,cs:AB_File
	mov	es:AmigaDiskReq.adr_File[si],ax		;File Number

	call	DoJanus
;	mov	ah,JFUNC_CALLAMIGA
;	mov	al,JSERV_PCDISK
;	mov	bx,0ffffh
;	int	JFUNC_JINT
;
;	mov	ah,JFUNC_WAITAMIGA
;	mov	al,JSERV_PCDISK
;	int	JFUNC_JINT

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

	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov  	es:AmigaDiskReq.adr_Count_h[si],dx	;count high
	mov	es:AmigaDiskReq.adr_Count_l[si],ax	;count low = 512 bytes
	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_READ
	mov	ax,cs:AB_File
	mov	es:AmigaDiskReq.adr_File[si],ax		;File Number

	call	DoJanus
						;copy buff to mem pointer
	cld
	mov	ds,cs:AB_jbuffseg
	mov	si,cs:AB_jbuffoff
	mov	es,cs:AB_dataseg
	mov	di,cs:AB_dataoff
  	mov 	ax,cs:AB_Secs
	mov	cx,512
	mul	cx

	mov	cx,ax
	rep	movs es:byte ptr[di],ds:[si]

	mov	cs:AB_dataoff,di
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

	mov	ah,JFUNC_GETBASE
	mov	al,JSERV_PCDISK
	int	JFUNC_JINT

	mov	ah,JFUNC_GETBASE
	mov	al,JSERV_PCDISK
	int	JFUNC_JINT

	cmp	di,-1				;pcdisk found?
	jne	get_ok
	jmp	exit				;not found so exit
						;file still flagged as
						;closed

get_ok:	mov 	cs:AB_jparmseg,es		;Save mem pointers
	mov 	cs:AB_jparmoff,di
	mov 	cs:AB_jbuffseg,dx

						;get buff off from parm mem
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov	ax,es:AmigaDiskReq.adr_BufferOffset[si]	
	mov	cs:AB_jbuffoff,ax

						;copy filename to buffer 
	cld
	push	cs
	pop	ds	
	mov	si,offset cs:AB_FileName
	mov	es,cs:AB_jbuffseg
	mov	di,cs:AB_jbuffoff
	mov	cx,50
	rep	movs es:byte ptr[di],ds:[si]

	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_OPEN_OLD 
 	
	call	DoJanus

	if	DBG_AB_FILEOPENMSG
	 push	ax
 	 call	outhxb
	 call	newline
	 pop	ax
	endif

	or	ax,ax
	je	opentok

	INFO	AB_COPENFAILMSG 
    	jmp	exit				;File remains closed!
						;and flag clear
		     
opentok:
	mov	ax,es:AmigaDiskReq.adr_File[si]
	mov	cs:AB_TFile,ax			;save temp handle for close

					;store offset and count into parm mem
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov 	es:AmigaDiskReq.adr_Offset_h[si],0	;offset high
	mov 	es:AmigaDiskReq.adr_Offset_l[si],0	;offset low
	mov 	es:AmigaDiskReq.adr_Count_h[si],0	;cnt high
	mov	es:AmigaDiskReq.adr_Count_l[si],0200h	;cnt low = 512 bytes

	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_READ
	mov	ax,cs:AB_TFile
	mov	es:AmigaDiskReq.adr_File[si],ax	

	call	DoJanus

	or	ax,ax
	je	readcok
	
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_CLOSE
	mov	ax,cs:AB_TFile
	mov	es:AmigaDiskReq.adr_File[si],ax	

	call	DoJanus
	jmp	exit

readcok:

	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_CLOSE
	mov	ax,cs:AB_TFile
	mov	es:AmigaDiskReq.adr_File[si],ax	

	call	DoJanus

	mov	cx,es:AmigaDiskReq.adr_Count_L[si]
	push	es
	push	si
	mov	es,cs:AB_jbuffseg
	mov	si,cs:AB_jbuffoff
		  
more: ;	cmp	cx,0
      ;	je	endfound
	cmp b	es:[si],00ah
	je	endfound
	inc	si
	dec	cx
	jmp	more
	
endfound:
	mov b	es:[si],0
	pop	si
	pop	es

	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_OPEN_OLD 
 	
	call	DoJanus

	if	DBG_AB_FILEOPENMSG
	 push	ax
	 call	outhxb
	 call 	newline
	 pop	ax
	endif

	or ax,ax
	je	openok
	INFO	AB_OPENFAILMSG 
    	jmp	exit				;File remains closed!
						;and flag clear
		  
openok:	
	mov 	ax,es:AmigaDiskReq.adr_File[si] 
	mov 	cs:AB_File,ax
      	mov	cs:AB_FileOpen,1

					;Check for ABOOT file and read parms

					;store offset and count into parm mem
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov 	es:AmigaDiskReq.adr_Offset_h[si],0	;offset high
	mov 	es:AmigaDiskReq.adr_Offset_l[si],0	;offset low
	mov 	es:AmigaDiskReq.adr_Count_h[si],0	;cnt high
	mov	es:AmigaDiskReq.adr_Count_l[si],0200h	;cnt low = 512 bytes

	mov 	es:AmigaDiskReq.adr_Fnctn[si],ADR_FNCTN_READ
	mov	ax,cs:AB_File
	mov	es:AmigaDiskReq.adr_File[si],ax	

	call	DoJanus
	

	mov	es,cs:AB_jbuffseg
	mov	si,cs:AB_jbuffoff
;	cmp	es:AB_BLOCK.ab_name[si],'A'	;check file format
;	jne	exit				;accept default as hard
						;coded file

						;get params
	INFO	AB_READPARMS
;	mov	cs:AB_SKIP512,1

	mov	ax,es:AB_BLOCK.Heads[si]
	xchg	ah,al
	mov	cs:AB_NumHeads,ax

	mov	ax,es:AB_BLOCK.SecTrk[si]
	xchg	ah,al
	mov	cs:AB_TrackSecs,ax

	mov	ax,es:AB_BLOCK.Cyl[si]
	xchg	ah,al
	mov	cs:AB_Cyls,ax

exit: 	
	INFO	AB_FTEXITMSG
	popall
	ret

first_time	endp
;*************************************************************************
;*
;*	DoJanus	       - Initiate and wait for a pcdisk request
;*
;*************************************************************************
DoJanus	proc	near


	mov	ah,JFUNC_CALLAMIGA
	mov	al,JSERV_PCDISK
	int	JFUNC_JINT

	cmp	al,JSERV_FINISHED
	je	doexit
	cmp	al,JSERV_PENDING
	jne	error

	mov	ah,JFUNC_WAITAMIGA
	mov	al,JSERV_PCDISK
	int	JFUNC_JINT

	cmp	al,JSERV_FINISHED
	jne	error

doexit:
	push	es
	push	si
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov	ax,es:AmigaDiskReq.adr_Err[si]
	pop	si
	pop	es
	or	ax,ax
	jne	error
	ret

error: 
	mov	ax,-1
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
	mov 	ax,0
	mov	es,ax
	mov	bx,7c00h		; read boot code here
	MOV	CX,0001H		; CH = Track 0, CL = sector 1
	MOV	AX,0201H		; AH = read command, AL = read 1 sector
	INT	13h			; do the read
	POP	CX			; count restore
	JC	BOOTDK1 		; failed start again

	MOV	AX,0
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