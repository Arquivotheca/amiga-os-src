TITLE	LOAD_IRQ  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
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
; Released :	28-apr-88  Version 2.49 released as V2.0 Alpha 1.0
;
; Update   :	16-jun-88  TB   2.51    fix `waiting for service' function
; Update   :	21-jun-88  TB   2.52    fix wrong segment in autoboot
; Update   :	23-jun-88  TB   2.53	cleanup and fix service 14
; Update   :	24-jun-88  TB   2.54	use new set of include files
; Update   :    28-jun-88  TB   2.55	add service 15,16
; Released :  	15-jul-88  Version 2.55 released as V2.0 Beta 0.3
; Update   :    18-jul-88  TB   2.56	add flags, init pad and semaphores
; Update   :    20-jul-88  TB   2.57	fix DOS 3.3 boot on AT, range check
; Update   :   	21-jul-88  TB	2.58	RJ fixed UserCount
; Update   :   	25-jul-88  TB	2.59	add Amiga/PC UserCount, fix service 12
; Update   :   	27-jul-88  TB	2.60	fixed Seg/Off order of PCMemPtr	and
;					 init AmigaMemPtr in service 10
; Released :	27-jul-88  Version 2.60 released as V2.0 Beta 1.0
;
; Update   :   	29-seb-88  TB	2.61	implement version check, structure
;					change, set HandlerLoaded flag
; Update   :   	 6-oct-88  TB	2.63	implement PCWait flag
; Update   :   	28-jul-88  TB	2.64	fix 'ZeroMem allocation' problem
;
; 65.Update :   02-Aug-89 BK	Fixed remove service to use the MemType
;				instead of MEMF_PARMTER when freeing
;				service mem.
; 66.Update :	10-Aug-89 BK	Changed MakeServiceData to init the lock
;				to 7f and JRememberKey to -1
; 67.Update :	10-Aug-89 BK	Changed AddService to set mem ptrs to NULL
;				if no mem allocated.
; 68.Update :	24-Aug-89 BK	Fixed AddService() to set the SERVICE_ADDED
;				flag in sd_Flags so the Amiga can do 
;				GETS_WAIT
; 69.Update :	28-Aug-89 BK	Fixed Release Service to return the 
;				sd_FirstPCCustomer field to -1 rather than 0
; 70.Update :	28-Aug-89 BK	Fixed Delete Service to return the 
;				sd_FirstPCCustomer field to -1 rather than 
;				leaving it at some random value.
;
; 71.Update :	28-Oct-89 TB	Fixed Delete Service to return the 
; 72.Update :   03-Oct-90 BK    Fixed library version check. Used to check
;                               for specific Version and equal or higher
;                               Revision. Now Checks Revision only if
;                               Version is exact match and allows higher
;                               Versions than what is requested.                               
;
;****************************************************************************

cseg	segment para public 'code'
assume 	cs:cseg,ds:cseg		 
  
w	       equ    word ptr

extrn	       JanInt:near		     	; janus interupt entry
extrn	       DosInt:near		     	; dos entry
extrn	       Bill13:near		     	; autoboot disk I/O handler
extrn          Bill19:near		     	; autoboot boot handler			
extrn	       JanIntEn:near		     	; enable Janus interrupts
extrn	       InitServicePtr:near	     	; init Service structures		
extrn	       CallAmiga:near	     		; Call Amiga Service		
extrn	       hdpart:near		     	; init partition structure
extrn 	       FakeDosFlag:byte				
;
; external utilities
;
extrn	       change_int:near		     	; replace interrupt vector
extrn	       outhxw:near		     	; prints hex word in ax
extrn	       outhxb:near		     	; prints hex byte in al
extrn	       outint:near		     	; prints integer in ax
extrn	       outuns:near		     	; print unsigned integer in ax
extrn	       outchr:near		     	; prints ascii char in al
extrn	       newline:near		     	; prints cr,lf
extrn	       pstrng:near		     	; prints out string


include        equ.inc			     	; equals
include        janus\janusvar.inc	     	; janus data structures
include	       janus\services.inc	
include        macros.inc		     	; helpfull macros
include	       debug.inc
include        vars_ext.inc 		     	; variables
include	       abequ.inc
include	       abdata.inc

if		(kludge16 ge 1)
extrn	       UpdateInt:near		     	; DOS keyboard check entry
endif
if		(scroll ge 1)
extrn	       Scroll_INT:near		     	; Scroll entry
endif

	org	0000h
entry:
ROMlabel	dw  0aa55h		     	; rom search header

ROMlength      	db  16			     	; length of PC.BOOT code
					     	;  in 512k blocks
start	proc   	far			     	; execution starts at off 3
  
	jmp    	run     
;
;
;------ data area for download routine ------

load_length    	equ (12 * 1024)	  	     	; length of code + data in bytes
					     	;  take 12kbyte (minimum is 1k)
 
romheader	db	" Janus Handler by Torsten Burgdorff ",0
copymsg		db  	" Copyright (c) 1988, Commodore Amiga Inc.",0ah,0dh
		db  	"         All rights reserved.   ",0

JHandlerVer	=	36			; Version of Janus Handler
JHandlerRev	=	81	
JLibVer		=	36			; Version of Janus.Library	
JLibRev		=	52			;  that is requiered 

;												 
;--------------------------------------------
;
run:					     	; here we go
	cli
	
	pushall			       		
	mov  	ax,cs			     	; setup our data segment
	mov  	ds,ax				

;;; shadow if we can

	mov	dx,0ech		;read configreg 3
	mov	al,3
	out	dx,al
	mov	dx,0edh
	in	al,dx
	and	al,10h		;bit 4 set?
	jnz	noshadow	;yes - REMAP384 is on, can't shadow.

	mov	ax,cs		;at d400?
	cmp	ax,0d400h
	jnz	noshadow	;no.

	pushf
	cli
	push	ds

	mov	dx,0ech		;can shadow.  address configreg 16
	mov	al,10h
	out	dx,al
	mov	dx,0edh
	in	al,dx
	and	al,0f3h		;clear the d400 bits
	or	al,004h		;read slot, write system
	out	dx,al

	mov	ax,0d400h
	mov	ds,ax
	mov	si,0
	mov	cx,16384

loopie:
	mov	al,[si]
	mov	[si],al
	inc	si
	loop	loopie

	mov	dx,0ech		;can shadow.  address configreg 16
	mov	al,10h
	out	dx,al
	mov	dx,0edh
	in	al,dx
	and	al,0f3h		;clear the d400 bits
	or	al,00ch		;read system, write system
	out	dx,al

	pop	ds
	popf

noshadow:

;;;
 
	mov	ax,JHandlerVer
	mov	bx,JHandlerRev
	INFO_AX_BX HandlerVersionMsg		; print our version
	INFO	CopyMsg

	mov	ax,cs
	INFO_AX	LoadMsg				; print our segment 
			   
	mov  	ax,f_seg			; look for Janus Parameter
	mov  	es,ax				; segment at F000
	cmp  	es:JanusAmiga.jpm_8088segment,ax
	je   	FoundBase
	mov  	ax,d_seg			; look for Janus Parameter
	mov  	es,ax				; segment at D000
	cmp  	es:JanusAmiga.jpm_8088segment,ax
	je   	FoundBase
	
	INFO	NoBaseMsg			; No Base found
	jmp 	NoBase
			  	
FoundBase:
	mov  	janus_base_seg,ax
	mov  	janus_param_seg,ax
	INFO_AX BaseMsg

	mov  	ax,es:JanusAmiga.jbm_8088segment	; autoset
	mov  	janus_buffer_seg,ax
	INFO_AX BufferMsg
	  
	mov	ax,es:JanusAmiga.ja_interrupts
	INFO_AX IntsMsg

	mov	ax,es:JanusAmiga.ja_parameters
	INFO_AX ParasMsg    

	mov	es:JanusAmiga.ja_JHandlerVer,JHandlerVer	; fill version
	mov	es:JanusAmiga.ja_JHandlerRev,JHandlerRev	;  fields
	
	mov	ax,es:JanusAmiga.ja_JLibVer	; check and print version and
	mov	bx,es:JanusAmiga.ja_JLibRev	;  revision of JANUS.LIBRARY
	INFO_AX_BX LibVersionMsg			  
	cmp	ax,JLibVer
	jb	Library_Bad
	ja	Library_OK
	cmp	bx,JLibRev
	jge	Library_OK

Library_Bad:
	INFO	WrongVersionMsg			; wrong version found
	jmp 	NoBase 				; -> exit
		
Library_OK:
;	cmp	cs:FakeDosFlag,AlreadyCalled   	; were we called before ?
;	je	NoPartRead

 	mov  	ah,0
	int  	disk_int		     	; reset disk system
  	call 	newline
	call 	hdpart			     	; init partition structure

NoDiskRead:  
	mov  	ax,load_length
	mov  	w sstack,ax		     	; init temp. stack pointer
	mov  	w sstack+2,cs

	push 	cs
	pop  	es			     	; set ES to load segment
	push 	es			     	; and save for later
	mov  	al,srv_int		     	; redirect IRQ3 
	mov  	di,offset JanInt 
	call 	change_int
	mov  	w chain_vec+2,es 	     	; and save old pointer
	mov  	w chain_vec,di
	pop  	es
	     	

;****************************************************	
;Warning Caution DANGER!!! Bill code to follow!
;If this looks screwed up... It probably is!
;additions also made to abvars.inc and abdata.inc
	if	AB_ACTIVATE

						;Test for an existing  
	push 	ax				; hard drive
	push 	dx
	mov 	ah,10h 				;test drive ready
	mov 	dl,80h				;first hard drive
	int	disk_int			;disk int
	pop	dx
	pop	ax
	jc 	nodisk				;No hard drive found,
						; so install fake
	jmp 	byebill				;Hard Card found,
						; so forget autoboot

nodisk:	   
	
	push 	es				; now redirect Int13
	mov  	al,disk_int			     
	mov  	di,offset bill13
	call 	change_int
	mov  	w AB_bill_int13+2,es
	mov  	w AB_bill_int13,di
 	pop  	es

	push	es				; now redirect int 19
	mov	al,boot_int	      
	mov	di,offset bill19
	call 	change_int
	pop  	es

byebill: 
	endif
;END of Bill code (thank god!)	    
;****************************************************
;

	push 	es
	mov 	al,disk_int			; now redirect Int13
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

if	(kludge16 ge 1)
	mov  	ticks,100			; init keyboard ticks counter
	push	es
	mov	al,keyb_int		      	; now redirect Int16
	mov	di,offset UpdateInt
	call	change_int
	mov	w BIOS_int16+2,es
	mov	w BIOS_int16,di
	pop  	es
endif

	mov 	ax,janus_param_seg	 	 
	mov 	es,ax			     	
	mov	es:JanusAmiga.ja_Lock,MemUnLock	; init main semaphore
	mov	es:JanusAmiga.ja_8088Go,0	; clear pad
	
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

	mov  	FakeDosFlag,0	     		; reset flag
	call 	JanIntEn      			; enable Janus interrupts
	
;	mov	al,JSERV_PCBOOTED 		; tell Amiga: now we are ready
;	call	CallAmiga			;  to handle services 

	if   	(scroll ge 1)
  	 mov  	si,es:JanusAmiga.ja_Parameters  ; write init value for
	 mov  	es:word ptr[si+16],55aah	; scroll communication	
	endif

	mov  	es:JanusAmiga.ja_HandlerLoaded,'TB' 	; set "here I am" 
							;  to DJMOUNT
NoBase:
	popall
	ret				     	; back to BIOS power up
	
start	endp

;--------------------------------------------			       
cseg	ends

	end  entry
