TITLE	CHGINT  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;**************************************************************************
;* Module: chgint    Entry: change_int		  Type: Near		  *
;*									  *
;* Function:   Set up Interrupt Vector					  *
;*									  *
;* Parameters: AL    - Interrupt # to change				  *
;*	       ES:DI - New Interrupt Vector				  *
;*									  *
;* Returns:    ES:DI - Old Interrupt Vector				  *
;*	       Interrupt Flag in CCR clear				  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

public	change_int

w	equ	word ptr

cseg segment   para public 'code'
assume  cs:cseg,ds:cseg,ss:cseg,es:nothing

change_int	proc near

	push 	ax
	push 	ds
	push 	si		; save what we use

	sub  	ah,ah 	      	; calc vector offset
	shl  	ax,1
	shl  	ax,1

	sub  	si,si 	      	; point to vector segment
	mov  	ds,si
 
	mov  	si,ax 	      	; int vector offset
	cli		      	; quiet now ..
	push 	es
	push 	di
	les  	di,[si]	      	; get old vector
	pop  	[si]		; store new      
	pop  	[si]+2

	pop  	si	      	; restore preserved regs
	pop  	ds
	pop  	ax
	ret
change_int 	endp

cseg ends

     end
	    

