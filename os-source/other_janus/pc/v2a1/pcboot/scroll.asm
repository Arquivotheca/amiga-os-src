page 63,132
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

PC_Scroll      equ	8			; should be at another place		
	
include        vars_ext.inc
include        mes.inc
include        equ.inc


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
						; Send PC_Scroll
	push	ax
	push	bx
	push	si
	push	es
	mov	bx,PC_Scroll		    	; service # in BX
	mov	byte ptr cs:ServStatTab[bx],0 	; set status = pending
	shl	bx,1

	if   	DBG_Scroll  
	 mov	ax,1234h
	 call	outhxw
	endif

	mov	ax,cs:janus_param_seg
	mov    	es,ax   
	mov	si,es:JanusBase.jb_Parameters	; points to parameter table
	mov	es:word ptr [si][bx],1234h	; fill message for Amiga
 	mov	si,es:JanusBase.jb_Interrupts	; points to interrupt table
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
	mov	bx,PC_Scroll		     	; service # in BX
	mov	byte ptr cs:ServStatTab[bx],0 	; set status = pending
	shl	bx,1
	mov	ax,cs:janus_param_seg
	mov    	es,ax   
	mov	si,es:JanusBase.jb_Parameters	; points to parameter table
	mov	es:word ptr [si][bx],0		; fill message for Amiga
 	mov	si,es:JanusBase.jb_Interrupts	; points to interrupt table
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


UpdateInt:       	;-----------------------  PC_INT16 points here
	push	ax
	dec	cs:ticks
	jnz	no_update_I
	mov	cs:ticks,3000
	in	al,pic_01
	test	al,08h
	jz	no_update_I	; channel still enabled

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
	and	al,0f7h
	out	pic_01,al
no_update_I:
	pop	ax
	sti
	pushf
	call	dword ptr cs:[BIOS_int16]
	iflags 				; save system flags
	iret

Scroll_INT  endp	


cseg	ends
 
end 

