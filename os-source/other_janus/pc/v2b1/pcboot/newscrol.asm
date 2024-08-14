TITLE	SCROLL  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;*****************************************************************************
; SCROLL handler:
; 
; SCROLL handles BIOS INT10 function on PC
; ------
;
; Will be called from PC. 
;
; Called from INT 10: 	Entry
;		     	- Jump to old INT10 if we don't scroll the last line
;		       	- Send PC_Scroll interrupt
;			- Call old INT10
;			- Send PC_Stop_Scroll interrupt
;			- Execute IRET 
;
;
;
; New code :   8-feb-88  TB
; 1.Update :  
; 2.Update :    
;
;******************************************************************************

public	       UpdateInt, Scroll_INT


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

include        janus\janusvar.inc
include	       janus\services.inc
include        macros.inc
include        vars_ext.inc
include        equ.inc
include	       debug.inc

Scroll_INT      proc far 
	
	cmp	ah,scrolling			; scroll up?
	jne	no_scroll
	cmp	al,1				; one row?
	jne	no_scroll
	cmp	cx,0				; scroll complete screen?	
	jne	no_scroll
	cmp	dh,24				; the last row row?
	jne	no_scroll

	mov	cs:ScrollFlag,1			; I think, we should scroll!	
						; Send JSERV_SCROLL
	push	ax
	push	bx
	push	si
	push	es
	mov	bx,JSERV_SCROLL		    	; service # in BX
	mov	byte ptr cs:ServStatTab[bx],0 	; set status = pending
	shl	bx,1

	if   	DBG_Scroll  
	 mov	ax,1234h
	 call	outhxw
	endif

	mov	ax,cs:janus_param_seg
	mov    	es,ax   
	mov	si,es:JanusAmiga.ja_Parameters	; points to parameter table
	mov	es:word ptr [si][bx],1234h	; fill message for Amiga
 	mov	si,es:JanusAmiga.ja_Interrupts	; points to interrupt table
	mov	es:byte ptr [si][bx],AckInt 	; fill message for Amiga
        mov  	al,0ffh					   
        out  	StatusReg,al	     		; trigger SYSINT on Amiga side
        pop	es
	pop     si
	pop     bx  				; fall into Check routine
	pop	ax				; with AH=0

no_scroll:
	sti
	pushf
	call	dword ptr cs:[BIOS_int10]
	iflags 					; save system flags
					
	cmp	cs:ScrollFlag,1			; are we in scroll condition?
	jne 	exit_scroll			; no -> exit
	dec	cs:ScrollFlag			; yes -> clear flag
						; Send PC_Stop_Scroll
	push	ax
	push	bx
	push	si
	push	es
	mov	bx,JSERV_SCROLL		     	; service # in BX
	mov	byte ptr cs:ServStatTab[bx],0 	; set status = pending
	shl	bx,1
	mov	ax,cs:janus_param_seg
	mov    	es,ax   
	mov	si,es:JanusAmiga.ja_Parameters	; points to parameter table
	mov	es:word ptr [si][bx],0		; fill message for Amiga
 	mov	si,es:JanusAmiga.ja_Interrupts	; points to interrupt table
	mov	es:byte ptr [si][bx],AckInt 	; fill message for Amiga
        mov  	al,0ffh					   
        out  	StatusReg,al	     		; trigger SYSINT on Amiga side
        pop	es
	pop     si
	pop     bx  				

	if   	DBG_Scroll 
	 mov	ax,0
	 call	outhxw
	endif

	pop	ax				

exit_scroll:
	iret

Scroll_INT  endp	


UpdateInt	proc	far	;---------------  PC_INT16 points here

	push	ax
	dec	cs:ticks
	jnz	no_update_I
	mov	cs:ticks,UpdateRate

	push	es
	mov	al,HW_Keyb_int		      	; redirect Int9 periodically
	mov	di,offset NumLockUpdate
	call	change_int
	mov	w HW_irq1+2,es
	mov	w HW_irq1,di
	pop  	es
		
	in	al,pic_01			; check interrupt channel
	test	al,08h
	jz	no_update_I			; channel 3 still enabled ?

	if   	DBG_UpdateMsg 
	 pushf
	 push 	si
	 push 	ds
	 push 	cs
	 pop  	ds
	 mov  	si,offset UpdateMsg
	 call 	pstrng
	 call 	newline
	 pop  	ds
	 pop  	si
	 popf
       	endif

	cli					
	and	al,0f7h				; reenable channel 3
	out	pic_01,al

no_update_I:
	pop	ax
	sti
	pushf					; go ahead with INT16
	call	dword ptr cs:[BIOS_int16]
	iflags 					; save system flags
	iret

Update_INT  endp	


NumLockUpdate	proc	far    	;----------------  PC_IRQ1 points here

	push	ax

	pop	ax
	sti
	pushf					; go ahead with IRQ1
	call	dword ptr cs:[HW_irq1]
	iflags 					; save system flags
	iret

NumLockUpdate	endp	

	
cseg	ends
 
end 

