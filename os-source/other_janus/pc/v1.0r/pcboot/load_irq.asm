;****************************************************************************
; Function:
;
; LoadIRQ3 loads and installs IRQ3SER 
; --------		   
;			     
;	   - will be loaded from AMIGA into janus buffer memory
;	   - will be found and executed during BIOS rom search 
;	   ( - downloads IRQ3SER into PC memory )
;	   ( - adjusts MEMSIZE			)
;	   - adjusts BIOS interrupt pointer to IRQ3SER
;
;
; 
; New code :  	28-feb-86  TB
; Update   :  	18-dec-86  TB  	2.16	Release 
; Update   :  	12-jan-87  TB  	2.17	 
; Update   :  	18-feb-87  TB  	2.18	Janus service for PC
; Update   :	23-feb-87  TB  	2.19	add AT segments			
;
;****************************************************************************

w	equ    word ptr

extrn	       JanInt:near		     ; janus interupt entry
extrn	       DosInt:near		     ; dos entry
extrn	       IdleInt:near		     ; DOS idle entry
extrn	       JanIntEn:near		     ; enable Janus interrupts
extrn	       hdpart:near		     ; init partition structure
extrn 	       FakeDosFlag:byte				
;
; external utilities
;
extrn	       change_int:near		     ; replace interrupt vector
extrn	       outhxw:near		     ; prints hex word in ax
extrn	       outhxb:near		     ; prints hex byte in al
extrn	       outint:near		     ; prints integer in ax
extrn	       outuns:near		     ; print unsigned integer in ax
extrn	       newline:near		     ; prints cr,lf
extrn	       pstrng:near		     ; prints out string

include        dskstruc.inc		     ; disk driver structures
include        janusvar.inc		     ; janus data structures
include        macros.inc		     ; helpfull macros
include        equ.inc			     ; equals
include	       pc_at.inc		     ; emulator type
include        mes.inc			     ; messageses
include        data.inc 		     ; variables
;
;-----------------------------------------------------------------------------
cseg	segment para public 'code'
					     
	assume cs:cseg,ds:cseg		 
  
	org    0000h
entry:
romlabel       dw  0aa55h		     ; rom search header

	org    0002h 
length	       db  08			     ; length of this routine
					     ; in 512k blocks
	org    0003h
start	proc   far
  
	jmp    run     
;
;
;------ data area for download routine ------

load_length    equ 02000h		     ; length in bytes
					     ; take  8kbyte (minimum is 1k)
loadmsg2       db  0ah,0dh,"Janus handler V2.20 found at segment ",0
 
Am_hd	       dsk_partition <0,100,200,80h,4,17>  ; fill partition table
						   ; for test
;
;--------------------------------------------
;
run:					     ; here we go
	  cli

	  pushall

	  mov  ah,0			     ; setup serial communications
	  mov  al,0e3h			     ; 9600,n,8,1
	  mov  dx,0
	  int  com 

	  mov  ax,cs			     ; setup our data segment
	  mov  ds,ax
 
	  if   (infolevel-10 ge 0)	
	   mov	si,offset loadmsg2     
	   call pstrng		       
	   mov	ax,cs		  
	   call outhxw		       
	   call newline 		    
	  endif

	  mov  ax,ja_seg		     ; init fixed segments of janus
	  mov  es,ax			     ; ES = para mem
	  mov  es:JanusBase.jpm_8088segment,ax
	  mov  janus_base_seg,ax
	  mov  janus_param_seg,ax

	  if   (infolevel-20 ge 0) 
	   mov	si,offset BaseMsg
	   call pstrng
	   call outhxw
	   call newline
	  endif

	  mov  ax,jb_seg		     ; buffer mem
	  mov  es:JanusBase.jbm_8088segment,ax 
	  mov  janus_buffer_seg,ax
  
	  if   (infolevel-20 ge 0)
	   mov	si,offset BufferMsg
	   call pstrng
	   call outhxw
	   call newline

	   mov	si,offset IntsMsg
	   call pstrng
	   mov	ax,es:JanusBase.jb_interrupts
	   call outhxw
	   call newline
    
	   mov	si,offset ParasMsg
	   call pstrng
	   mov	ax,es:JanusBase.jb_parameters
	   call outhxw
	   call newline
	  endif

	  mov  ax,offset Am_hd		     ; adjust pointer to janus 
	  mov  janus_part_base,ax	     ; partition block 

   	  mov  ah,0
	  int  disk			     ; reset disk system

  	  call newline
	  call hdpart			     ; init partition structure
  
	  mov  ax,load_length
	  mov  w sstack,ax		     ; init temp. stack pointer
	  mov  w sstack+2,cs
    
	  push cs
	  pop  es			     ; set ES to load segment
	  push es			     ; and save for later

	  mov  al,srv_int		     ; redirect IRQ3 
	  mov  di,offset JanInt 
	  call change_int
	  mov  w chain_vec+2,es 	     ; and save old pointer
	  mov  w chain_vec,di
	  pop  es


	  mov  FakeDosFlag,0		     ; reset flag

	  push ax
	  in   al,pic_01		     ; enable IRQ3  
	  and  al,irq3en
	  out  pic_01,al
	  pop  ax

;****************************************************	
;Warning Caution DANGER!!! Bill code to follow!
;If this looks screwed up... It probably is!
;additions also made to vars.inc and data.inc
				;Test for an existing hard drive 
	push 	ax
	push 	dx
	mov 	ah,10h 		;test drive ready
	mov 	dl,80h		;first hard drive
	int	DISK		;disk int
	pop	dx
	pop	ax
	jc 	nodisk		;No hard drive fopund so install fake
	jmp 	byebill		;Hard Card found so forget autoboot

nodisk:	   
	
	push 	es		; now redirect Int13
	mov  	al,13h			     
	mov  	di,offset bill13
	call 	change_int
	mov  w 	bill_int13+2,es
	mov  w 	bill_int13,di
 	pop  	es

	push	es		; now redirect int 19
	mov	al,19h	      
	mov	di,offset bill19
	call 	change_int
	pop  	es

 
	jmp 	byebill	     	; Interupt handlers follow
;New INT handlers go here (for now)      

bill13:
	pushf			;If first time called init and open file
	pushall
	cmp	cs:b_file_open,1
	je	going
      	mov	cs:b_file_open,1
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
	pushall			;Request is for me
	pushf
	      	push 	cs	;Print debug message
		pop 	ds	
		push	ax      
		mov	si,offset billmsg0    ;func
	   	call 	pstrng		       
	   	mov	al,ah		  
	   	call 	outhxb		       
	   	mov	si,offset billmsg1	;drive
	   	call 	pstrng		       
		mov	al,dl
		call	outhxb
	   	mov	si,offset billmsg2	;cyl
	   	call 	pstrng		       
		mov	al,ch
		call	outhxb
	   	mov	si,offset billmsg3	;head
	   	call 	pstrng
		mov	al,dh
		call	outhxb		       
	   	mov	si,offset billmsg4	;sec
	   	call 	pstrng
		mov	al,cl
		and	al,00111111b
		call	outhxb       
	   	mov	si,offset billmsg5	;#secs
	   	call 	pstrng
	 	pop	ax
		call	outhxb		       
	   	call 	newline 		    
	popf
	popall
;my routines implemented ***
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
	cmp cs:okflag,1
	je tok2
	mov ah,0	;** ROUTINE CHOPPED OUT FOR DEBUG
	mov al,1	;**
	clc		;**
	jmp	myexit	;**
tok2:
	;cnv_do, compute ofset and count
	;for count/pcdisk_buf_size + stray
	;do read and transfer to dest es:bx
	pushall
	mov	cs:dataseg,es
	mov	cs:dataoff,bx	;stash pointers
	mov	bh,0
	mov	bl,dh		;bx has head #
	mov	ah,0
	mov	al,ch		;got cyl in ax *** fix for 2 more bits **
	mov	ch,0
	and	cl,00111111b	;got sec # in cx
	call	cnv_do
	;store offset into param mem and count = 512
	mov	es,cs:jparmseg
	mov	si,cs:jparmoff
	mov	es:4[si],dx	;offset high
	mov	es:6[si],ax	;offset low
	 mov ax,es:4[si]
	 call outhxw
	 mov ax,es:6[si]
	 call outhxw
 	 call newline
	mov word ptr es:8[si],0	;count high
	mov	es:10[si],0200h	;count low = 512 bytes
	;fnctn = read
	mov word ptr es:[si],1 	
	;call J_CALL_AMIGA
	mov	ah,7
	mov	al,14
	mov	bx,0ffffh
	int	0bh
	;J_WAIT_AMIGA
	mov	ah,8
	mov	al,14
	int	0bh
	;copy buff to mem pointer
	cld
	mov	ds,cs:jbuffseg
	mov	si,cs:jbuffoff
	mov	es,cs:dataseg
	mov	di,cs:dataoff
	mov	cx,512
	rep	movs es:byte ptr[di],ds:[si]
	popall
	mov	ah,0
	mov	al,1
        clc
	jmp	myexit

i3:				;Write specific sectors from memory
	cmp cs:okflag,1
	je tok3
	mov	ah,0	;** ROUTINE CHOPPED OUT FOR DEBUG
	mov	al,1	;**
	clc		;**
	jmp	myexit 	;**
tok3:	;cnv_do, compute offset and count
	;for count/pcdisk_buf_size + stray
	;do transfer to pcdisk_buf and write
	pushall
	;copy buff to mem pointer
	mov	cs:dataseg,es
	mov	cs:dataoff,bx	;stash pointers
	pushall
	cld
	mov	ds,cs:dataseg
	mov	si,cs:dataoff
	mov	es,cs:jbuffseg
	mov	di,cs:jbuffoff
	mov	cx,512
	rep	movs es:byte ptr[di],ds:[si]
	popall
	mov	bh,0
	mov	bl,dh		;bx has head #
	mov	ah,0
	mov	al,ch		;got cyl in ax *** fix for 2 more bits **
	mov	ch,0
	and	cl,00111111b	;got sec # in cx
	call	cnv_do
	;store offset into param mem and count = 512
	mov	es,cs:jparmseg 	
	mov	si,cs:jparmoff
	mov	es:4[si],dx	;offset high
	mov	es:6[si],ax	;offset low
	 mov ax,es:4[si]
	 call outhxw
	 mov ax,es:6[si]
	 call outhxw
 	 call newline
	mov word ptr es:8[si],0	;count high
	mov	es:10[si],0200h	;count low = 512 bytes
	;fnctn = write
	mov word ptr es:[si],2 	
	;call J_CALL_AMIGA
	mov	ah,J_CALL_AMIGA
	mov	al,14
	mov	bx,0ffffh
	int	0bh
	;J_WAIT_AMIGA
	mov	ah,8
	mov	al,14
	int	0bh
	popall
	mov	ah,0
	mov	al,1
	clc
	jmp	myexit
i8:				;Get the current drive parameters
	mov	dh,4		;4 heads
	mov	dl,1		;1 hard drive
	mov	cl,17		;17 sectors/trk
	mov	ch,1		;40 cyls
	mov	ah,0
	clc
	jmp	myexit
	
myexit:
	iret
cnv_do	proc	near		;Given head in dh
				;cyl and sec cx
				;# of sectors in al
				;compute start offset and byte count
	dec	cx
	mul	cs:bk_NumHeads
	add	ax,bx
	mul	cs:bk_TrackSecs
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

first_time	proc	near
	pushall
pcloop:	mov	ah,J_GET_BASE
	mov	al,14	;AMIGAFUN
	int	srv_int	;JANUS_INT
	cmp	al,J_OK	;J_OK ?
	jne	pcloop
	cmp 	di,0ffffh
	je	pcloop
	cmp	di,0		;don't ask me why... It works!
	je 	pcloop
	mov 	cs:jparmseg,es
	mov 	cs:jparmoff,di
	mov 	cs:jbuffseg,dx
	popall

;get buffer offset from parameter memory
	pushall
	mov	es,cs:jparmseg
	mov	si,cs:jparmoff
	mov	ax,es:12[si]	;buffer offset
	mov	cs:jbuffoff,ax
	popall
;copy filename to buffer and do open old clear drive not ready
	pushall
	cld
	push	cs
	pop	ds	
	mov	si,offset cs:AB_file
	mov	es,cs:jbuffseg
	mov	di,cs:jbuffoff
	mov	cx,50
	rep	movs es:byte ptr[di],ds:[si]
	popall

	pushall
	mov	es,cs:jparmseg
	mov	si,cs:jparmoff
	mov word ptr	es:[si],0005h	;ADR_FNCTN_OPEN_OLD
	mov	ah,J_CALL_AMIGA
	mov	al,14	;AMIGAFUN
	mov	bx,0ffffh
	int	srv_int
	mov	ah,J_WAIT_AMIGA
	mov	al,14
	int	srv_int
	popall
	ret

first_time	endp

notme:	popall
	popf

	pushf
        call 	dword ptr cs:[bill_int13]	
	iflags
	iret
	
bill19:	

;** Attempt floppy boot from a: then b:

	pushall
	SUB	DX,DX			; Head=0, Drive = A:
BOOTNXT:
	MOV	CX,3			; retry
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



	  
byebill:
	push	cs
	pop	es

;END of Bill code (thank god!)	    
;****************************************************
	  mov  al,hd_int		     ; now redirect Int13
	  mov  di,offset DosInt
	  call change_int
	  mov  w bios_int13+2,es
	  mov  w bios_int13,di
 
	  if   (idle ge 1)
	   mov	al,idle_int		      ; now redirect Int28
	   mov	di,offset IdleInt
	   call change_int
	   mov	w DOS_int28+2,es
	   mov	w DOS_int28,di
	  endif       

	  mov  ax,ja_seg		     ; send ready message 
	  mov  es,ax			     ; to Amigas DJMOUNT command
	  mov  es:JanusBase.jpm_pad0,'B'     ; set "here I am"

	  mov  FakeDosFlag,0		     ; reset flag

	  popall
	  ret				     ; back to BIOS power up
	
start	  endp

;--------------------------------------------			       
cseg	  ends

	  end  entry
