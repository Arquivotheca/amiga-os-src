TITLE	IRQ3SER  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
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
; 54.Update :	24-jun-88 TB	2.54	use new set of include files
; 59.Update :	26-jul-88 TB	2.59	add service 15,16 to dispatcher
; 71.Update :	28-oct-89 TB	2.71	added JFunction 21-25 to dispatcher
;
;******************************************************************************

public	       JanInt, DosInt

cseg segment   para public 'code'
assume	       cs:cseg,ss:cseg,ds:cseg,es:nothing

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
extrn	       LockServiceData:near
extrn	       UnlockServiceData:near
extrn	       JanusInitLock:near
extrn	       JanusLockAttempt:near
extrn          JanusLock:near
extrn          JanusUnlock:near
extrn	       AllocJRemember:near
extrn	       AttachJRemember:near
extrn	       AllocServiceMem:near
extrn          FreeJRemember:near
extrn          FreeServiceMem:near
;
extrn	       outhxw:near		     ; prints hex word in ax
extrn	       outint:near		     ; prints integer in ax
extrn	       newline:near		     ; prints cr,lf
extrn	       pstrng:near		     ; prints out string
extrn	       change_int:near	 	     ; modify interrupt vector	
	
w	       equ     word ptr

include        equ.inc
include        vars_ext.inc
include	       debug.inc
include        macros.inc
include	       janus\services.inc	

pchar	macro	c
	endm
xyz	macro	c
	push	ax

;;	push	dx		;serial
;;	xor	dx,dx		;serial
;;	mov	ax,1*100h+c	;serial
;;	int	14h		;serial
;;	pop	dx

	push	bx		;video
	xor	bx,bx		;video
	mov	ax,0eh*100h+c	;video
	int	10h		;video
	pop	bx		;video

	pop	ax
	endm

irq3ser        proc far 
				
JanInt:        ;------------------------  IRQ3 points here
	cli			     	; disable all HW interrupts
;	call 	JanIntDis	     	; disable all janus interrupts 
if	(counters ge 1)
	cmp	cs:irq3_count,0
	jz	irq3_count_ok
	inc	cs:irq3_reenter
irq3_count_ok:
	inc	cs:irq3_count	
endif

IF 0
;;
;repatch IRQ3, since dos's silly stack multiplexer (which conveniently
;assumes that IRQ3 is a *hardware* interrupt - and why would a *hardware*
;interrupt need to see the same registers that were present when the
;interrupt occurred??) has wrecked the registers for us.  who knows
;what software function that'll be....
	push	ax
	push	di
	push 	es			; DOS 3.xx catches our interrupt 
	push 	cs			; vector during boot, so let's 
	pop  	es			
	mov  	al,srv_int		; redirect IRQ3 again
 	mov  	di,offset JanInt 
	call 	change_int
	mov  	w cs:chain_vec+2,es 	; and save old pointer
	mov  	w cs:chain_vec,di
	pop  	es
	pop	di
	pop	ax
;;
ENDIF

	cmp	cs:FakeDosFlag,AlreadyCalled 	; were we called before ?
	je	FakeSkip

	push	ax
	push	di

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

	pop	di
	pop	ax

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
	pchar	'*'
	mov	cs:FakeDosFlag,AlreadyCalled	; set flag now
	or   	cs:ActiveFlag,Janus     	; set flag

	pushf
	push	ax
	mov	al,cs:ActiveFlag
	mov	ah,0
;	xor	ah,ah 			; this changes the flags !!!
	INFO_AX HWMsgP 
   	pop	ax
	popf

        if   	(idle ge 1)
        else
	js	Hard_exit		; Dos interrupt still in process ?
	jmp	IntExe		     	
        endif
Hard_exit:
	pchar	'%'
if	(counters ge 1)
	inc	cs:irq3_hardexit
endif
	iret			     	; return to function


DosInt:        	;-----------------------  PC_INT13 points here
	pchar	'['
        or   	cs:ActiveFlag,Dos	; set flag

	push	ax
	mov	al,ah
	lahf
	xchg 	al,ah
	INFO_AX	DosMsgA			; print flags and function code
 	pop	ax

	clc				; clear carry to be on save side	
        pushf			     	; prepare IRET from called funct.
        call 	dword ptr cs:[bios_int13]	
        iflagsi			     	; save system flags

	push	ax
	mov	al,ah
	lahf
	xchg 	al,ah
	INFO_AX	DosMsgN			; print flags and error code
	pop	ax

        and  	cs:ActiveFlag,not Dos   ; clear flag
        jnz  	IntExex		     	; JanInt pending ?

	pchar	']'
        iret			    	; return to DOS
   
IntExex: 	;-----------------------
	pchar	'$'
IntExe: 	;-----------------------
if	(counters ge 1)
	cmp	cs:irq3_hw_count,0
	jz	irq3_hw_count_ok
	inc	cs:irq3_hw_reenter
irq3_hw_count_ok:
	inc	cs:irq3_hw_count	
endif
	push	ax
	push 	dx  
	
;;; irq3 a (sidecar)
;	mov  	dx,03b0h	     	; reset IRQ3 on janus board 
;;; irq3 b (2088, 2286, 2386)
	mov  	dx,02f8h	     	; reset IRQ3 on janus board 
;;;

;;; normal
;	mov  	dx,IRQ3Reset	     	; reset IRQ3 on janus board 
;;;
	in   	al,dx		     	; by reading garbage from there
	pop  	dx
	pop  	ax
	sti

	pchar	'#'
	call 	JanusIntHandler

	and  	cs:ActiveFlag,not Janus	; reset flag

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
if	(counters ge 1)
	dec	cs:irq3_count
	dec	cs:irq3_hw_count	
endif
        iret			     	; return to DOS

FuncTable:
	dw	GetService1		; 0
	dw	GetBase			; 1
	dw	AllocMem		; 2
	dw	FreeMem			; 3
	dw	SetParam		; 4
	dw	SetService		; 5
	dw	StopService		; 6
	dw	CallAmiga 		; 7
	dw	WaitAmiga		; 8 This is actually a special case
	dw	CheckAmiga 		; 9
	dw	AddService		; 10
	dw	GetService		; 11
	dw	CallService		; 12
	dw	ReleaseService 		; 13
	dw	DeleteService		; 14
	dw	LockServiceData 	; 15
	dw	UnlockServiceData 	; 16
	dw	JanusInitLock		; 17
	dw	JanusLockAttempt	; 18
	dw	JanusLock		; 19
	dw	JanusUnlock		; 20
	dw	AllocJRemember		; 21
	dw	AttachJRemember		; 22
	dw	FreeJRemember		; 23
	dw	AllocServiceMem		; 24
	dw	FreeServiceMem		; 25

jumper	proc	near
	push	bx			;save bx
	push	ax			;save ax
	mov	al,ah			;put index into ax
	xor	ah,ah
	mov	bx,offset FuncTable	;get ptr to jumptable
	add	bx,ax			;add 2*index
	add	bx,ax
	pop	ax			;restore ax
	mov	bx,cs:[bx]		;fetch addr from table
	push	bp
	mov	bp,sp
	xchg	bx,[bp+2]		;addr to stack, saved bx back to bx
	pop	bp
	xor	ah,ah
	ret				;"jump" to the table proc
jumper	endp

IRQ3soft:	;-----------------------  PC is calling
        INFO_AX HWMsgPC 
        pchar	'+'
 
if	(counters ge 1)
	cmp	cs:irq3_sw_count,0
	jz	irq3_sw_count_ok
	inc	cs:irq3_sw_reenter
irq3_sw_count_ok:
	inc	cs:irq3_sw_count	
endif
	cmp	ah,JFUNC_Max		; out of range ?
	jle	exe_service
	jmp	Ill_function		; yes, report error

exe_service:
	cmp	ah,08
	jne	around
	mov 	ah,1
	call 	WaitAmiga
	jmp	exit_int
around:
	call	jumper
	jmp	exit_int

;	or      ah,ah
;	jne	es1
;	call	GetService1
;	jmp	exit_int
;es1:	dec     ah
;	jne	es2
;	call	GetBase
;	jmp	exit_int
;es2:	dec     ah
;	jne	es3
;	call	AllocMem
;	jmp	exit_int
;es3:	dec     ah
;	jne	es4
;	call	FreeMem
;	jmp	exit_int
;es4:	dec     ah
;	jne	es5
;	call	SetParam
;	jmp	exit_int
;es5:	dec     ah
;	jne	es6
;	call	SetService
;	jmp	exit_int
;es6:	dec     ah
;	jne	es7
;	call	StopService
;	jmp	exit_int
;es7:	dec     ah
;	jne	es8
;	call	CallAmiga 
;	jmp	exit_int
;es8:	dec     ah
;	jne	es9
;	inc	ah  			; call this shared Wait_Amiga routine 
;	call	WaitAmiga		; with AH=1 as a flag
;	jmp	exit_int
;es9:	dec     ah
;	jne	es10
;	call	CheckAmiga 
;	jmp	exit_int
;es10:	dec     ah
;	jne	es11
;	call	AddService
;	jmp	exit_int
;es11:	dec     ah
;	jne	es12
;	call	GetService
;	jmp	exit_int
;es12:	dec     ah
;	jne	es13
;	call	CallService
;	jmp	exit_int
;es13:	dec     ah
;	jne	es14
;	call	ReleaseService 
;	jmp	exit_int
;es14:	dec     ah
;	jne	es15
;	call	DeleteService
;	jmp	exit_int
;es15:	dec     ah
;	jne	es16
;	call	LockServiceData 
;	jmp	exit_int
;es16:	dec     ah
;	jne	es17
;	call	UnlockServiceData 
;	jmp	exit_int
;es17:	dec     ah
;	jne	es18
;	call	JanusInitLock
;	jmp	exit_int
;es18:	dec     ah
;	jne	es19
;	call	JanusLockAttempt
;	jmp	exit_int
;es19:	dec     ah
;	jne	es20
;	call	JanusLock
;	jmp	exit_int
;es20:	dec     ah
;	jne	es21
;	call	JanusUnlock
;	jmp	exit_int
;es21:	dec     ah
;	jne	es22
;	call	AllocJRemember
;	jmp	exit_int
;es22:	dec     ah
;	jne	es23
;	call	AttachJRemember
;	jmp	exit_int
;es23:	dec     ah
;	jne	es24
;	call	FreeJRemember
;	jmp	exit_int
;es24:	dec     ah
;	jne	es25
;	call	AllocServiceMem
;	jmp	exit_int
;es25:	dec     ah
;	jne	Ill_function  		; expand functions here
;	call	FreeServiceMem
;	jmp	exit_int

Ill_function:				; yes, report error
	mov	al,JSERV_ILLFUNCTION

exit_int:				; common exits --------------
;	call 	JanIntEn		; enable janus interrupts
	mov	ah,0ffh			; destroy register to make at least
if	(counters ge 1)
	dec	cs:irq3_count
	dec	cs:irq3_sw_count
endif
 	iret				;  this happened

irq3ser endp	


cseg	ends
 
end 

