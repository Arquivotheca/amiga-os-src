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
; New code :   25-Mar-88 Bill Koester
; 1.Update :  
; 2.Update :    
;
;******************************************************************************

public	       Bill13, Bill19

include        janusvar.inc
include        macros.inc
 

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing

extrn	       JanInt:near		     ; janus interupt entry
extrn	       JanusIntHandler:near	     ; Dispatcher for janus ints
extrn	       JanIntDis:near		     ; disable janus interrupts
extrn	       JanIntEn:near		     ; enable janus interrupts
;
; external utilities
;
extrn	       outhxw:near		     ; prints hex word in ax
extrn		outhxb:near			;prints hex byte in al
extrn	       outint:near		     ; prints integer in ax
extrn	       newline:near		     ; prints cr,lf
extrn	       pstrng:near		     ; prints out string
extrn		change_int:near		
	
w	       equ     word ptr

	
include        vars_ext.inc
include        mes.inc
include        equ.inc
;include        data.inc
include 	abdata.inc		
include		abequ.inc
include		service.inc
	
;New INT handlers go here (for now)      

bill13		proc far

	pushf			;If first time called init and open file
	pushall
	cmp	cs:AB_FileOpen,1
	je	going
	call	first_time	
going:	popall
	popf

	pushf			;If not for drive 80 then call old handler		
	pushall
      	cmp	dl,80h
	je	kludge
       	jmp	notme
kludge:
	and	cs:ActiveFlag,not Dos
	popall
	popf
	if 	AB_INT13MSG
	pushf			;Request is for me
	pushall
	      	push 	cs	;Print debug message
		pop 	ds	
		push	ax      
		mov	si,offset AB_billmsg0    ;func
	   	call 	pstrng		       
	   	mov	al,ah		  
	   	call 	outhxb		       
	   	mov	si,offset AB_billmsg1	;drive
	   	call 	pstrng		       
		mov	al,dl
		call	outhxb
	   	mov	si,offset AB_billmsg2	;cyl
	   	call 	pstrng		       
		mov	al,ch
		call	outhxb
	   	mov	si,offset AB_billmsg3	;head
	   	call 	pstrng
		mov	al,dh
		call	outhxb		       
	   	mov	si,offset AB_billmsg4	;sec
	   	call 	pstrng
		mov	al,cl
		and	al,00111111b
		call	outhxb       
	   	mov	si,offset AB_billmsg5	;#secs
	   	call 	pstrng
	 	pop	ax
		call	outhxb		       
	   	call 	newline 		    
	popall
	popf
	endif
;my routines implemented ***
	pushf
	pushall
	push cs
	pop  es			     		; set ES to load segment
	push es			     		; and save for later
	cli
	mov  al,srv_int		     		; redirect IRQ3 
	mov  di,offset JanInt 
	call change_int
	mov  w chain_vec+2,es 	     		; and save old pointer
	mov  w chain_vec,di
	sti
	pop  es
	popall
	popf

	cmp	cs:AB_FileOpen,1		;If file not open return
	je	cont				;error
	mov	ah,1
	stc
	jmp	myexit

cont:
	cmp	ah,00h	;Reset the disk
	je	i0
	cmp	ah,01h	;Read status of last disk operation
	je	i1
	cmp	ah,02h	;Read specific sectors to memory
	je	i2
	cmp	ah,03h	;Write specific sectors from memory
	jne	jhg
	jmp	i3
jhg:	cmp	ah,04h	;Verify specific sectors
	je	i4
	cmp	ah,05h	;Format specific track
	je	i5
	cmp	ah,08h	;Get current drive parameters
	jne	ghj
	jmp	i8
ghj:	cmp	ah,09h	;Initialize drive characteristics
	je	i9
	cmp	ah,0ch	;Seek
	je	ic
	cmp	ah,0dh	;Reset drive
	je	id
	cmp	ah,10h	;Test drive ready
	je	i10
	cmp	ah,11h	;Recalibrate
	je	i11
	cmp	ah,14h	;Run controller diagnostic
	je	i14
	mov	ah,1	;Uniplemented funcs return 01 and carry set
	stc
	jmp	myexit
			
i0:				;Functions that always return OK
        or   	cs:ActiveFlag,Dos	; set flag
	sti
	pushf
        call 	dword ptr cs:[AB_bill_int13]	
	iflags
	iret
i1:
i4:
i5:
i9:
ic:
id:
i10:
i11:
i14:
	mov	ah,0
	clc
	jmp	myexit	       

i2:				;Read specific sectors to memory
	cmp cs:AB_okflag,1
	je tok2
	mov ax,0100h	;** ROUTINE CHOPPED OUT FOR DEBUG
	stc		;**
	jmp	myexit	;**
tok2:
	;cnv_do, compute ofset and count
	;for count/pcdisk_buf_size + stray
	;do read and transfer to dest es:bx
	pushall

	pushall
	mov ax,0110h 		;Wake up services
	int 0bh
	popall

	pushall
	xor	ah,ah
	mov	cs:AB_Secs,ax
	mov	cs:AB_dataseg,es
	mov	cs:AB_dataoff,bx	;stash pointers
	xor	bh,bh
	mov	bl,dh		;bx has head #
	mov	cs:AB_Head,bx
	mov	al,ch		;got cyl in ax *** fix for 2 more bits **
	mov	cs:AB_Cyl,ax
	xor	ch,ch
	and	cl,00111111b	;got sec # in cx
	mov	cs:AB_Sec,cx
	popall

	mov	cx,cs:AB_Secs 
	mov	cs:AB_SecsDone,0
Readm:	pushall
	mov	bx,cs:AB_Head
	mov	ax,cs:AB_Cyl
	mov	cx,cs:AB_Sec	
	call	cnv_do
	;store offset into param mem and count = 512
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov	es:4[si],dx	;offset high
	mov	es:6[si],ax	;offset low
	mov word ptr es:8[si],0	;count high
	mov	es:10[si],0200h	;count low = 512 bytes
	;fnctn = read
	mov word ptr es:[si],1 	;Funtion = Read
	mov	ax,cs:AB_File
	mov	es:2[si],ax	;File Number
	;call J_CALL_AMIGA
	mov	ax,0710h
	mov	bx,0ffffh
	int	0bh
;	call	outhxb
	;J_WAIT_AMIGA
aga:	mov	ax,0810h
	int	0bh
;	call	outhxb
;	call	newline
;	cmp 	al,1
;	jne	aga    
	;copy buff to mem pointer
	cld
	mov	ds,cs:AB_jbuffseg
	mov	si,cs:AB_jbuffoff
	mov	es,cs:AB_dataseg
	mov	di,cs:AB_dataoff
	mov	cx,512
	rep	movs es:byte ptr[di],ds:[si]
	mov	cs:AB_dataoff,di
	inc	AB_Sec
	inc	AB_SecsDone
	popall
	dec	cx
	je	fini     
	jmp	Readm

fini:	popall
	mov	ax,cs:AB_SecsDone
	xor	ah,ah
        clc
	jmp	myexit

i3:				;Write specific sectors from memory
	cmp cs:AB_okflag,1
	je tok3
	mov	ax,0100h	;** ROUTINE CHOPPED OUT FOR DEBUG
	stc		;**
	jmp	myexit 	;**
tok3:	;cnv_do, compute offset and count
	;for count/pcdisk_buf_size + stray
	;do transfer to pcdisk_buf and write
	pushall

	;copy buff to mem pointer

	xor	ah,ah
	mov	cs:AB_Secs,ax
	mov	cs:AB_dataseg,es
	mov	cs:AB_dataoff,bx	;stash pointers
	xor	bh,bh
	mov	bl,dh		;bx has head #
	mov	cs:AB_Head,bx
	mov	al,ch		;got cyl in ax *** fix for 2 more bits **
	mov	cs:AB_Cyl,ax
	xor	ch,ch
	and	cl,00111111b	;got sec # in cx
	mov	cs:AB_Sec,cx


	mov	cx,cs:AB_Secs
	mov	cs:AB_SecsDone,0
rdmore:	pushall
	cld
	mov	ds,cs:AB_dataseg
	mov	si,cs:AB_dataoff
	mov	es,cs:AB_jbuffseg
	mov	di,cs:AB_jbuffoff
	mov	cx,512
	rep	movs es:byte ptr[di],ds:[si]
	mov	cs:AB_dataoff,si

	mov	bx,cs:AB_Head
	mov	ax,cs:AB_Cyl
	mov	cx,cs:AB_Sec
	call	cnv_do
	;store offset into param mem and count = 512
	mov	es,cs:AB_jparmseg 	
	mov	si,cs:AB_jparmoff
	mov	es:4[si],dx	;offset high
	mov	es:6[si],ax	;offset low
	mov word ptr es:8[si],0	;count high
	mov	es:10[si],0200h	;count low = 512 bytes
	;fnctn = write
	mov word ptr es:[si],2 	
	mov	ax,cs:AB_File
	mov	es:2[si],ax	;File Number
	;call J_CALL_AMIGA
	mov	ax,0710h	;J_CALL_AMIGA
	mov	bx,0ffffh
	int	0bh
	;J_WAIT_AMIGA
	mov	ax,0810h
	int	0bh
	inc	cs:AB_Sec
	inc	cs:AB_SecsDone
 	popall
	dec	cx
	je	fin
	jmp	rdmore

fin:	popall
	mov	ax,cs:AB_SecsDone
	xor 	ah,ah
	clc
	jmp	myexit
i8:				;Get the current drive parameters
	mov	dx,0301h	; 4Heads, 1 Hard Drive
	mov	cx,0a811h	;168 Cyls, 17 secs/trk
	xor	ah,ah
	clc
	jmp	myexit
	
myexit:
	iret

notme:	popall
	popf

	pushf
        call 	dword ptr cs:[AB_bill_int13]	
	iflags
	iret
	
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
			      
	dec	cx
	mul	cs:AB_NumHeads
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
	ret
cnv_do	endp
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
	and	cs:ActiveFlag,not Dos		;We are in DOS

	mov	ax,0110h			;GetBase(pcdisk)
	int 	srv_int

	mov	ax,0110h       			;Call again
	int	srv_int	

	cmp	al,J_OK				;pcdisk found?
	je	get_ok
	jmp	exit				;not found so exit
						;file still flagged as
						;closed

get_ok:	mov 	cs:AB_jparmseg,es			;Save mem pointers
	mov 	cs:AB_jparmoff,di
	mov 	cs:AB_jbuffseg,dx

;get buffer offset from parameter memory
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov	ax,es:12[si]          		;buffer offset
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

;Do open old
	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	mov word ptr	es:[si],0005h	;ADR_FNCTN_OPEN_OLD
	mov	ax,0710h	;J_CALL_AMIGA
	mov	bx,0ffffh
	int	srv_int

	if	DBG_AB_FILEOPENMSG
 	 call	outhxb
	endif

	mov	ax,0810h	;J_WAIT_AMIGA
	int	srv_int

	if	DBG_AB_FILEOPENMSG
	 call	outhxb
	 call 	newline
	endif

	mov	es,cs:AB_jparmseg
	mov	si,cs:AB_jparmoff
	cmp byte ptr es:14[si],0
	je	openok
	INFO	AB_OPENFAILMSG 
    	jmp	exit		;File remains closed!

openok:	mov 	ax,es:2[si]   ;file number
	mov 	cs:AB_File,ax
      	mov	cs:AB_FileOpen,1

;Check for ABOOT file and read parms
	

exit: 	
	INFO	AB_FTEXITMSG
	popall
	ret

first_time	endp
;*************************************************************************
;*
;*	bill19	       - Attempt boot from drive a:
;*						 b:
;*						 c:
;*
;*************************************************************************
bill19:	

;** Attempt floppy boot from a: then b:

	pushall
	SUB	DX,DX			; Head=0, Drive = A:
BOOTNXT:
	MOV	CX,1			; retry
BOOTDK0:
	PUSH	CX
	STI				; enable interrupts
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

	popall
	db	0eah	; far jump to 0:7c00h
	dw	7c00h
	dw	0
BOOTDK1:

	LOOP	BOOTDK0
	INC	DL			; Next Drive
	CMP	DL,2			; All drives done ?
	JB	BOOTNXT 		; no if below


	mov	dx,0080h		; Head=0, Drive = C:
	MOV	CX,3			; retry
BOOTDK01:
	PUSH	CX
	STI				; enable interrupts
	XOR	AH,AH			; Reset diskette
	INT	13h			; ....
	mov 	ax,0
	mov	es,ax
	mov	bx,7c00h		; read boot code here
	MOV	CX,0001H		; CH = Track 0, CL = sector 1
	MOV	AX,0201H		; AH = read command, AL = read 1 sector
	INT	13h			; do the read
	POP	CX			; count restore
	JC	BOOTDK11 		; failed start again

	popall
	db	0eah	; far jump to 0:7c00h
	dw	7c00h
	dw	0
BOOTDK11:

	LOOP	BOOTDK01

	popall
	INT	18H			; Call INT 18h but don't return
;
	IRET

Bill13  endp	


cseg	ends
 
end 





