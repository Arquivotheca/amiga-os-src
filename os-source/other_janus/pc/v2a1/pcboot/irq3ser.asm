page 63,132
;*****************************************************************************
; IRQ-3 handler:
; 
; ServiceIRQ3 handles shared DOS/BIOS functions on PC
; -----------
;
; Will be called from Janus or PC. 
;
; Called from INT B: 	JanInt entry
;		     	- Disable Janus interrupts
;		     	- Find out who calls
;
;			HW Int: JANUS is calling
;				- Set JanAktiv flag
;		     		- IF DosAktive   
;		       	     	  - Execute IRET
;		     		- ELSE 
;		       	     	  - Handle JanInt
;					- Reset IRQ3 on janus board
;			 		- Send EOI if all requests satisfied
;			 		- Get out requested functions
;			 		- Setup pointer	  
;			 		- Call requested functions
;			 		- Setup returning parameter block
;			 		- Send acknowledge for executed functions
;			 		- Enable Janus interrupts
;			 		- Send SYSINT to AMIGA for acknowledge
;			 		- Execute IRET
;
;			SW Int: PC is calling
;			  	- Execute requested function
;				- Enable Janus interrupts
; 			  	- Execute IRET
;
;	
; Called from INT 13:	DosInt entry
;		     		- Set DosAktiv flag 
;		     		- Call requested function
;		     		- Reset DosAktive flag
;		     		- IF JanAktiv 
;		       		  - Handle JanInt (see above)
;		     		- ELSE
;		       		  - Execute IRET 
;
;
;
; New code  :  	28-feb-86 TB
;  1.Update :  	23-apr-86 TB
;  2.Update :    9-sep-86 TB  
;  3.Update :  	23-oct-86 TB
;  4.Update :  	15-dec-86 TB 	2.16	keyboard problem
;  5.Update :  	17-feb-87 TB  	2.18	Janus service for PC
;  6.Update :  	29-oct-87 TB  	2.36	"7f"-search after every INT13
;  7.Update :  	17-nov-87 TB  	2.37	remap INT28 and update PIC with
;					 keyboard interrupt
; 41.Update :	 8-mar-88 TB	2.41	Alloc_Mem feature added		
; 44.Update :	30-mar-88 TB    2.44    Call services
; 45.Update :	 2-apr-88 TB    2.45    Call 2.generation of services 
; 48.Update :	26-apr-88 TB    2.48    late minute bug fixing
;
;******************************************************************************

public	       JanInt, DosInt, DOS_Idle

include        janusvar.inc
include        macros.inc
 

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing

;
; external utilities
;
extrn	       JanusIntHandler:near	     ; Dispatcher for janus ints
extrn	       JanIntDis:near		     ; disable janus interrupts
extrn	       JanIntEn:near		     ; enable janus interrupts
;
extrn	       GetService1:near
extrn	       GetBase:near
extrn	       AllocMem:near
extrn	       FreeMem:near
extrn	       SetParam:near		
extrn	       SetService:near
extrn	       StopService:near
extrn	       CallAmiga:near
extrn	       WaitAmiga:near
extrn	       CheckAmiga:near
extrn	       AddService:near		
extrn	       GetService:near
extrn	       CallService:near
extrn	       ReleaseService:near
extrn	       DeleteService:near
;
extrn	       outhxw:near		     ; prints hex word in ax
extrn	       outint:near		     ; prints integer in ax
extrn	       newline:near		     ; prints cr,lf
extrn	       pstrng:near		     ; prints out string
extrn	       change_int:near	 	     ; modify interrupt vector	
	
w	       equ     word ptr

include        vars_ext.inc
include        mes.inc
include        equ.inc
include	       service.inc	

irq3ser        proc far 

				
JanInt:        ;------------------------  IRQ3 points here
	cli			     	; disable all HW interrupts
;	call 	JanIntDis	     	; disable all janus interrupts 

	cmp	cs:FakeDosFlag,AlreadyCalled 	; were we called before ?
	je	FakeSkip

	push 	es			; DOS 3.xx catches our interrupt 
	push 	cs			; vector during boot, so let's 
	push	cs
	pop  	es			
	mov  	al,srv_int		; redirect IRQ3 again
 	mov  	di,offset JanInt 
	call 	change_int
	mov  	w cs:chain_vec+2,es 	; and save old pointer
	mov  	w cs:chain_vec,di
	pop  	es
	
        if   	(idle ge 1)
 	 mov  	al,idle_int		; redirect also INT28 again
 	 mov  	di,offset DOS_Idle 
	 call 	change_int
	 mov  	w cs:DOS_int28+2,es 	; and save old pointer
	 mov  	w cs:DOS_int28,di
	endif

	pop  	es
	mov	cs:FakeDosFlag,CalledFirst	; set flag now

FakeSkip:
       	push 	ax			; find out who is calling
        mov  	al,0bh		    	; read ISR !
        out  	pic_00,al
  	in   	al,pic_00      		; read interrupt controller		
 	and  	al,08h	          	; mask JANUS interrupt
 	pop	ax
	jnz   	IRQ3hard  		; is Amiga or PC calling ?
	cmp	cs:FakeDosFlag,CalledFirst
	je	FakeExit
	jmp	IRQ3soft

FakeExit:
	mov	cs:FakeDosFlag,AlreadyCalled	; set flag now
	jmp	ILL_Function			;  and return with error set


IRQ3hard: 	;----------------------   Amiga is calling !
	mov	cs:FakeDosFlag,AlreadyCalled	; set flag now
	or   	cs:ActiveFlag,Janus     	; set flag

	push	ax
	mov	al,cs:ActiveFlag
	mov	ah,0
;	xor	ah,ah 			; this changes the flags !!!
	INFO_AX HWMsgP 
   	pop	ax

        if   	(idle ge 1)
        else
	 js     Hard_exit		; Dos interrupt still in process ?
	 jmp  	IntExe		     	
        endif
Hard_exit:
	iret			     	; return to function

DOS_Idle: 	;----------------------   DOS Int 28
        if   	(idle ge 1)
;	 or   	cs:ActiveFlag,IdleDOS      ; set flag
	 test  	cs:ActiveFlag,janus
    	 jz	Idle1
	 jmp  	IntExe
Idle1:
	INFO	IdleMsg

	 sti
	 pushf
	 call	dword ptr cs:[DOS_int28]
	 iflags 				; save system flags
        endif	;idle

        iret			     	; return to function


DosInt:        	;-----------------------  PC_INT13 points here
        or   	cs:ActiveFlag,Dos	; set flag

	INFO	DosMsgA
 
        sti			     	; reenable ints
        pushf			     	; prepare IRET from called funct.
        call 	dword ptr cs:[bios_int13]	
        iflags			     	; save system flags

	INFO	DosMsgN

        and  	cs:ActiveFlag,not Dos   ; clear flag
;       jmp  	IntExe		     	; JanInt pending ?
        jnz  	IntExe		     	; JanInt pending ?

        iret			    	; return to DOS
   
IntExe: 	;-----------------------
	and  	cs:ActiveFlag,not Janus	; reset flag
	push	ax
	push 	dx  
	mov  	dx,IRQ3Reset	     	; reset IRQ3 on janus board 
	in   	al,dx		     	; by reading garbage from there
	pop  	dx
	pop  	ax
	sti

	call 	JanusIntHandler

	push	ax
	in   	al,pic_00		; read interrupt controller again
	xor  	ah,ah
        INFO_AX IntCtrlMes 
  
	INFO	HWMsgR

        cli			     	; disable interrupt until IRET
;	call 	JanIntEn		; enable janus interrupts
        mov  	al,0ffh					   
        out  	StatusReg,al	     	; the way to trigger a
				     	; SYSINT on AMIGA side
        mov  	al,0bh		    	; read ISR !
        out  	pic_00,al
        in   	al,pic_00		; clear interrupt controller now
        or   	al,al
        jz   	already_clr
        mov  	al,20h		     	; send unspecific clear to 
        out  	pic_00,al		; interrupt controller
already_clr:
        in   	al,pic_00		; read interrupt controller again
        pop  	ax
        iret			     	; return to DOS


IRQ3soft:	;-----------------------  PC is calling
        INFO_AX HWMsgPC 
 
	cmp	ah,J_Delete_Service	; out of range ?
	jle	exe_service
	jmp	Ill_function		; yes, report error

exe_service:
	or      ah,ah
	jne	es1
	call	GetService1
	jmp	exit_int
es1:	dec     ah
	jne	es2
	call	GetBase
	jmp	exit_int
es2:	dec     ah
	jne	es3
	call	AllocMem
	jmp	exit_int
es3:	dec     ah
	jne	es4
	call	FreeMem
	jmp	exit_int
es4:	dec     ah
	jne	es5
	call	SetParam
	jmp	exit_int
es5:	dec     ah
	jne	es6
	call	SetService
	jmp	exit_int
es6:	dec     ah
	jne	es7
	call	StopService
	jmp	exit_int
es7:	dec     ah
	jne	es8
	call	CallAmiga 
	jmp	exit_int
es8:	dec     ah
	jne	es9
	inc	ah  			; call this shared Wait_Amiga routine 
	call	WaitAmiga		; with AH=1 as a flag
	jmp	exit_int
es9:	dec     ah
	jne	es10
	call	CheckAmiga 
	jmp	exit_int
es10:	dec     ah
	jne	es11
	call	AddService
	jmp	exit_int
es11:	dec     ah
	jne	es12
	call	GetService
	jmp	exit_int
es12:	dec     ah
	jne	es13
	call	CallService
	jmp	exit_int
es13:	dec     ah
	jne	es14
	call	ReleaseService 
	jmp	exit_int
es14:	dec     ah
	jne	Ill_function  		; expand functions here
	call	DeleteService 
        jmp	exit_int

Ill_function:				; yes, report error
	mov	al,J_ILL_FNCTN

exit_int:				; common exits --------------
;	call 	JanIntEn		; enable janus interrupts
	mov	ah,0ffh			; destroy register to make at least
 	iret				;  this happened

irq3ser endp	


cseg	ends
 
end 

