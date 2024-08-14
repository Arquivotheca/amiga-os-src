TITLE	INT86  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;**************************************************************************
;* Module: int86     Entry: int86		  Type: Near		  *
;*									  *
;* Function: Execute a PC Software INT from Amiga			  *
;*									  *
;* Parameters: ES:DI - Pointer to a syscall structure			  *
;*									  *
;* Returns: results in syscall structure				  *
;*									  *
;* All Registers preserved.						  * 
;*									  *
;* Note: This code is not reentrant and must run in RAM.		  *
;**************************************************************************

public	  int86

cseg segment   para public 'code'
assume cs:cseg,ds:nothing   

include   janus\syscall.inc
include   macros.inc

extrn	  parmptr:word
extrn	  sstack:word


int86	  proc near
	  pushall			     ; keep clean ...

	  mov  	cs:parmptr,di		     ; save parameter pointer
	  mov	cs:parmptr+2,es

	  mov   si,di			     ; swap pointers
	  push 	es			     ; and segments
	  pop  	ds

	  mov  	al,byte ptr [si].s86_int     ; interrupt number
	  mov  	byte ptr cs:intinst+1,al

	  mov  	ax,[si].s86_ax		     ; registers
	  mov  	bx,[si].s86_bx
	  mov  	cx,[si].s86_cx
	  mov  	dx,[si].s86_dx
	  mov  	di,[si].s86_di
	  mov  	es,[si].s86_es
	  mov  	bp,[si].s86_bp
	  push 	[si].s86_psw
	  popf
	  lds  	si,dword ptr [si].s86_si     ; last fetch
	     
intinst:  int  	255			     ; do it
					     ; with some luck we come here ...
	  pushf 			     ; temp save
	  push 	ds			     ; save them first
	  push 	si

	  mov  	byte ptr cs:intinst+1,255    ; restore old value for checksum
	  lds  	si,dword ptr cs:[parmptr]    ; parameter pointer fetch

	  mov  	[si].s86_ax,ax		     ; result store
	  mov  	[si].s86_bx,bx		     ; result store
	  mov  	[si].s86_cx,cx		     ; result store
	  mov  	[si].s86_dx,dx		     ; result store
	  mov  	[si].s86_di,di		     ; result store
	  mov  	[si].s86_es,es		     ; result store
	  mov  	[si].s86_bp,bp		     ; result store

	  pop  	ax			     ; was si
	  mov  	[si].s86_si,ax		     ; result store
	  pop  	ax			     ; was ds
	  mov  	[si].s86_ds,ax		     ; result store
	  pop  	ax			     ; were flags
	  mov  	[si].s86_psw,ax		     ; result store

	  popall			     ; as they were b4
	  ret

int86	  endp

cseg	  ends

	  end
