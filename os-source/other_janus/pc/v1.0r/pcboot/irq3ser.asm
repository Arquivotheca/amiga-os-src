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
; New code :  28-feb-86  TB
; 1.Update :  23-apr-86  TB
; 2.Update :   9-sep-86  TB  
; 3.Update :  23-oct-86  TB
; 4.Update :  15-dec-86  TB  2.16	keyboard problem
; 5.Update :  17-feb-87  TB  2.18	Janus service for PC
;******************************************************************************

public	       JanInt, DosInt, IdleInt

include        janusvar.inc
include        macros.inc
 

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing

extrn	       JanusIntHandler:near	     ; Dispatcher for janus ints
extrn	       JanIntDis:near		     ; disable janus interrupts
extrn	       JanIntEn:near		     ; enable janus interrupts
;
; external utilities
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


irq3ser        proc far 

JanInt:        ;------------------------  IRQ3 points here
	cli			     	; disable all HW interrupts
;	call 	JanIntDis	     	; disable all janus interrupts 

	cmp	cs:FakeDosFlag,AlreadyCalled 	; were we called before ?
	je	FakeExit

;	push	ax     			; check where interrupt	vector
;	push	ds			; points to
;	xor	ax,ax
;	mov	ds,ax			; DS points to interrupt table
;	cmp	IntB_seg,Dos_seg 	; Int vector still points to DOS
;	jne	FakeExit		; no, skip redirection

	push 	es			; DOS 3.xx catches our interrupt 
	push 	cs			; vector during boot, so let's 
	pop  	es			; redirect IRQ3 again
	mov  	al,srv_int		
 	mov  	di,offset JanInt 
	call 	change_int
	mov  	w cs:chain_vec+2,es 	; and save old pointer
	mov  	w cs:chain_vec,di
	pop  	es
	mov	cs:FakeDosFlag,AlreadyCalled	; set flag now

FakeExit:
;	pop	ds

       	push 	ax			; find out who is calling
        mov  	al,0bh		    	; read ISR !
        out  	pic_00,al
  	in   	al,pic_00      		; read interrupt controller		
 	and  	al,08h	          	; mask JANUS interrupt
 	pop	ax
	jnz   	IRQ3hard  		; is Amiga or PC calling ?
	jmp	IRQ3soft

IRQ3hard: 	;----------------------   Amiga is calling !
	or   	cs:ActiveFlag,Janus     ; set flag
   
       if   	(infolevel-40 ge 0) 
	pushf
	push 	si
	push 	ds
	push 	cs
	pop  	ds
	mov  	si,offset HWMsgP
	call 	pstrng
	call 	newline
	pop  	ds
	pop  	si
	popf
       endif

       if   	(idle ge 1)
        else
	jns  	IntExe		     	; Dos interrupt still in process ?
       endif

	iret			     	; return to function

IdleInt:       	;-----------------------  PC_INT28 points here
       if   (idle ge 1)   
        and  	cs:ActiveFlag,0ffh
	jnz  	IntExe
	iret
       endif

DosInt:        	;-----------------------  PC_INT13 points here
        or   	cs:ActiveFlag,Dos	; set flag

       if   	(infolevel-50 ge 0) 
	push 	si
       	push 	ds
       	push 	cs
       	pop  	ds
       	mov  	si,offset DosMsgA
       	call 	pstrng
       	call 	newline
       	pop  	ds
       	pop  	si
       endif
 
        sti			     	; reenable ints
        pushf			     	; prepare IRET from called funct.
        call 	dword ptr cs:[bios_int13]	
        iflags			     	; save system flags
  
        and  	cs:ActiveFlag,not Dos   ; clear flag

       if   	(infolevel-50 ge 0) 
       	pushf
       	push 	si
       	push 	ds
       	push 	cs
       	pop  	ds
       	mov  	si,offset DosMsgN
       	call 	pstrng
       	call 	newline
       	pop  	ds
       	pop  	si
       	popf
       endif

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
	 
       if   	(infolevel-48 ge 0) 
	pushf
	push 	si
	push 	ds
	push 	cs
	pop  	ds
	push 	ax
	mov  	si,offset IntCtrlMes
	call 	pstrng
	in   	al,pic_00		; read interrupt controller again
	xor  	ah,ah
	call 	outhxw
	call 	newline
	pop  	ax
	pop  	ds
	pop  	si
	popf
       endif
  
       if   	(infolevel-40 ge 0) 
	pushf
	push 	si
	push 	ds
	push 	cs
	pop  	ds
	mov  	si,offset HWMsgR
	call 	pstrng
	call 	newline
	pop  	ds
	pop  	si
	popf
       endif

        cli			     	; disable interrupt until IRET
        push 	ax
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
       if   	(infolevel-40 ge 0) 
	pushf
	push    ax
	push 	si
	push 	ds
	push 	cs
	pop  	ds
	mov  	si,offset HWMsgPC
	call 	pstrng
 	call	newline
	call	outhxw
	call 	newline
	pop  	ds
	pop  	si
	pop	ax
  	popf
       endif
 
	cmp	ah,J_CHECK_AMIGA	; out of range ?
	jle	exe_service
Ill_function:				; yes, report error
	mov	al,J_ILL_FNCTN
        jmp	exit_int

exe_service:
	or      ah,ah
	je	Get_service
	dec     ah
	je	Get_base
	dec     ah
	je	Alloc_mem
	dec     ah
	je	Free_mem
	dec     ah
	je	Set_param
	dec     ah
	je	Set_service
	dec     ah
	je	Stop_service
	dec     ah
	je	Call_amiga 
	dec     ah
	jne	Next_service
	inc	ah  			; call this shared Wait_Amiga routine 
	jmp	Wait_amiga		; with AH=1 as a flag
Next_service:
	dec     ah
	je	Check_amiga   	
	jmp	Ill_function  		; expand functions here


Get_service: 				; Service 0 -----------------
	jmp	No_service

Get_base:				; Service 1 -----------------
	push	si			; save it
	push	bx
	push	ax
	xor	bh,bh	
	mov 	bl,al  				; service # in BX
	mov	dx,cs:janus_buffer_seg		; 1.Buffer seg
	mov	ax,cs:janus_param_seg
	mov    	es,ax				; 2.Para seg
	mov	si,es:JanusBase.jb_Parameters	; points to parameter table
	shl	bx,1
	mov	di,es:[si][bx]			; 3.Para offset
	pop	ax
	pop 	bx	
	pop	si
	jmp	OK

Alloc_mem:			   	; Service 2 -----------------		
	mov	al,J_NO_MEMORY
        jmp	exit_int

Free_mem:				; Service 3 -----------------
	jmp	No_service

Set_param:				; Service 4 -----------------
	jmp	No_service

Set_service:				; Service 5 -----------------
	jmp	No_service

Stop_service:				; Service 6 -----------------
	jmp	No_service


Call_amiga:				; Service 7 -----------------
	cmp	bx,0ffffh
	je	Def_Mem
	jmp	No_service		; cannot handle this feature now

Def_Mem:
	push	ax
	push	bx
	push	si
	push	es
	xor	bh,bh
	mov	bl,al					; service # in BX
	mov	byte ptr cs:ServStatTab[bx],J_PENDING 	; set status = pending
	shl	bx,1
	mov	ax,cs:janus_param_seg
	mov    	es,ax   
	mov	si,es:JanusBase.jb_Interrupts	; points to interrupt table
	mov	es:byte ptr [si][bx],AckInt 	; fill message for Amiga
        mov  	al,0ffh					   
        out  	StatusReg,al	     		; trigger SYSINT on Amiga side
        pop	es
	pop     si
	pop     bx  				; fall into Check routine
	pop	ax				; with AH=0


Check_Amiga:				; Service 9 -----------------
					; AH=0 on this entry

Wait_Amiga:				; Service 8 -----------------
					; AH=1 on this entry 
;	call 	JanIntEn		; enable janus interrupts
	sti				; enable ints now
	push	bx
	xor	bh,bh
	mov	bl,al 			; service # in BX

Wait_Amiga_Ready:
 	mov	al,byte ptr cs:ServStatTab[bx] 	; read service status
	or	ah,ah				; called via Wait_Amiga ?
	jz	Wait_Amiga_Exit			; AH=1 means yes !		
	cmp	al,J_PENDING
	je	Wait_Amiga_Ready	; loop if pending

Wait_Amiga_Exit:
	pop	bx
	jmp	exit_int


OK:					; common exits --------------
	mov	al,J_OK
        jmp	exit_int

No_service:
	mov	al,J_NO_SERVICE
exit_int:
;	call 	JanIntEn		; enable janus interrupts
 	iret


irq3ser endp	


cseg	ends
 
end 

