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
; Update   :	21-may-87  TB  	2.22	add AT segments	automatically		
; Update   :	 4-nov-87  TB  	2.23	update PIC automatically with INT1C		
; Update   :	16-nov-87  TB  	2.24	update PIC automatically with INT16		
; Update   :	17-nov-87  TB  	2.25	remap INT28 after DOS boot		
;
; Update   :	 8-feb-88  TB  	2.40	Fast scroll feature implemented	
; Update   :	 8-mar-88  TB  	2.41	Allocate Mem feature implemented	
; Update   :	10-mar-88  TB  	2.41L	2.41, but using LOCK prefix
; Update   :	24-mar-88  TB   2.42    Enable JanusInts before boot 			
; Update   :	29-mar-88  TB   2.43    Auto segment load, 12k length 			
; Update   :	30-mar-88  TB   2.44    Call services
; Update   :	 2-apr-88  TB   2.45    2.generation of services
; Update   :	10-apr-88  TB   2.46    change structures according to RJ
; Update   :	14-apr-88  TB   2.47    change memory locking, add serviceses
; Update   :	26-apr-88  TB   2.48    late minute bug fixing
; Update   :	26-apr-88  TB   2.49    forget channeling
;
;****************************************************************************
 
w	equ    word ptr

extrn	       JanInt:near		     ; janus interupt entry
extrn	       DosInt:near		     ; dos entry
extrn	       UpdateInt:near		     ; DOS keyboard check entry
extrn	       DOS_Idle:near		     ; DOS idle entry
extrn	       Scroll_INT:near		     ; Scroll entry
extrn	       Bill13:near		     ; autoboot disk I/O handler
extrn          Bill19:near		     ; autoboot boot handler			
extrn	       JanIntEn:near		     ; enable Janus interrupts
extrn	       InitServicePtr:near	     ; init Service structures		
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

include        janusvar.inc		     ; janus data structures
include        macros.inc		     ; helpfull macros
include        equ.inc			     ; equals
include        mes.inc			     ; messageses
include        vars_ext.inc 		     ; variables
include	       abequ.inc
include	       abdata.inc
;
;-----------------------------------------------------------------------------
cseg	segment para public 'code'
					     
	assume cs:cseg,ds:cseg		 
  
	org	0000h
entry:
ROMlabel	dw  0aa55h		     ; rom search header

ROMlength      	db  16			     ; length of PC.BOOT code
					     ;  in 512k blocks
start	proc   	far			     ; execution starts at off 3
  
	jmp    	run     
;
;
;------ data area for download routine ------

load_length    	equ (12 * 1024)	  	     ; length of code + data in bytes
					     ;  take 12kbyte (minimum is 1k)
loadmsg       	db  0ah,0dh,"Janus handler V2.49 found at segment ",0
;												 
;--------------------------------------------
;
run:					     ; here we go
	cli
	
	pushall

;	  mov  ah,0			     ; setup serial communications
;	  mov  al,0e3h			     ; 9600,n,8,1
;	  mov  dx,0
;	  int  com 

	mov  	ax,cs			     ; setup our data segment
	mov  	ds,ax
 		
	INFO_AX LoadMsg
	
	mov  	ax,f_seg				; look for Janus Parameter
	mov  	es,ax				; segment at F000
	cmp  	es:JanusBase.jpm_8088segment,ax
	je   	FoundBase
	mov  	ax,d_seg				; look for Janus Parameter
	mov  	es,ax				; segment at D000
	cmp  	es:JanusBase.jpm_8088segment,ax
	je   	FoundBase
	
	INFO	NoBaseMsg			; No Base found
	jmp 	NoBase
			  	
FoundBase:
	mov  	janus_base_seg,ax
	mov  	janus_param_seg,ax
	INFO_AX BaseMsg

	mov  	ax,es:JanusBase.jbm_8088segment 	; autoset
	mov  	janus_buffer_seg,ax
	INFO_AX BufferMsg
	  
	mov	ax,es:JanusBase.jb_interrupts
	INFO_AX IntsMsg

	mov	ax,es:JanusBase.jb_parameters
	INFO_AX ParasMsg    

;	cmp	cs:FakeDosFlag,AlreadyCalled   	; were we called before ?
;	je	NoPartRead

 	mov  	ah,0
	int  	disk			     	; reset disk system
  	call 	newline
	call 	hdpart			     	; init partition structure

NoDiskRead:  
	mov  	ax,load_length
	mov  	w sstack,ax		     	; init temp. stack pointer
	mov  	w sstack+2,cs

	push 	cs
	pop  	es			     		; set ES to load segment
	push 	es			     		; and save for later
	mov  	al,srv_int		     		; redirect IRQ3 
	mov  	di,offset JanInt 
	call 	change_int
	mov  	w chain_vec+2,es 	     		; and save old pointer
	mov  	w chain_vec,di
	pop  	es
	     	

;****************************************************	
;Warning Caution DANGER!!! Bill code to follow!
;If this looks screwed up... It probably is!
;additions also made to abvars.inc and abdata.inc
	if	AB_ACTIVATE

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
	mov  	w AB_bill_int13+2,es
	mov  	w AB_bill_int13,di
 	pop  	es

	push	es		; now redirect int 19
	mov	al,19h	      
	mov	di,offset bill19
	call 	change_int
	pop  	es

byebill: 
	endif
;END of Bill code (thank god!)	    
;****************************************************
;

	push 	es
	mov 	al,hd_int			; now redirect Int13
	mov  	di,offset DosInt
	call 	change_int
	mov  	w bios_int13+2,es
	mov  	w bios_int13,di
 	pop  	es

	if   	(scroll ge 1)
	 push	es
	 mov	al,video_int		      	; now redirect Int10
	 mov	di,offset Scroll_INT
	 call 	change_int
	 mov	w BIOS_int10+2,es
	 mov	w BIOS_int10,di
	 pop  	es
        endif       

	mov  	ticks,100			; init keyboard ticks counter
	push	es
	mov	al,keyb_int		      	; now redirect Int16
	mov	di,offset UpdateInt
	call	change_int
	mov	w BIOS_int16+2,es
	mov	w BIOS_int16,di
	pop  	es
	
	if   (idle ge 1)
	 push	es
	 mov	al,idle_int		      	; now redirect Int28
	 mov	di,offset DOS_Idle
	 call 	change_int
	 mov	w DOS_int28+2,es
	 mov	w DOS_int28,di
	 pop  	es
	endif       
					
	mov 	ax,janus_param_seg	 	; send ready message 
	mov 	es,ax			     	; to Amigas DJMOUNT command
	mov  	es:JanusBase.jpm_pad0,'T'     	; set "here I am"
	mov  	es:JanusBase.jbm_pad0,'B'     	; set "here I am"
	
	call	InitServicePtr
	or	al,al
	jz	ServiceGoes
	INFO	NoServiceBaseMsg		; No Base found

ServiceGoes:	
	push 	ax  				
	in   	al,pic_01		     	; enable IRQ3  
	and  	al,irq3en
	out  	pic_01,al
	pop  	ax

	call 	JanIntEn      			; enable Janus interrupts
	mov  	FakeDosFlag,0	     		; reset flag
		 			
	if   	(scroll ge 1)
  	 mov  	si,es:JanusBase.jb_Parameters   ; write init value for
	 mov  	es:word ptr[si+16],55aah	; scroll communication	
	endif

NoBase:
	popall
	ret				     	; back to BIOS power up
	
start	endp

;--------------------------------------------			       
cseg	ends

	end  entry
