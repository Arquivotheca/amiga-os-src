	       page 63,130

include        syscall.inc
include        macros.inc
include        equ.inc
	       

cseg segment   para public 'code'

	  assume cs:cseg,ds:nothing   

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
extrn	  parmptr:word
extrn	  sstack:word

int86	  proc near
	  pushall			     ; keep clean ...

;	  in   al,pic_01
;	  or   al,2			     ; mask out keyb. interrupt
;	  out  pic_01,al

	  mov  cs:parmptr,di		     ; save parameter pointer
	  mov  cs:parmptr+2,es

	  mov  si,di			     ; swap pointers
	  push es			     ; and segments
	  pop  ds

	  mov  al,byte ptr [si].int	     ; interrupt number
	  mov  byte ptr cs:intinst+1,al

	  mov  ax,[si].r_ax		     ; registers
	  mov  bx,[si].r_bx
	  mov  cx,[si].r_cx
	  mov  dx,[si].r_dx
	  mov  di,[si].r_di
	  mov  es,[si].r_es
	  mov  bp,[si].r_bp
	  push [si].psw
	  popf
	  lds  si,dword ptr [si].r_si	     ; last fetch
	     
intinst:  int  255			     ; do it
					     ; with some luck we come here ...
	  pushf 			     ; temp save
	  push ds			     ; save them first
	  push si

	  mov  byte ptr cs:intinst+1,255     ; restore old value for checksum
	  lds  si,dword ptr cs:[parmptr]     ; parameter pointer fetch

	  mov  [si].r_ax,ax		     ; result store
	  mov  [si].r_bx,bx		     ; result store
	  mov  [si].r_cx,cx		     ; result store
	  mov  [si].r_dx,dx		     ; result store
	  mov  [si].r_di,di		     ; result store
	  mov  [si].r_es,es		     ; result store
	  mov  [si].r_bp,bp		     ; result store

	  pop  ax			     ; was si
	  mov  [si].r_si,ax		     ; result store
	  pop  ax			     ; was ds
	  mov  [si].r_ds,ax		     ; result store
	  pop  ax			     ; were flags
	  mov  [si].psw,ax		     ; result store

;	  in   al,pic_01
;	  and  al,0fdh			     ; enable keyb. interrupt
;	  out  pic_01,al
 
	  popall			     ; as they were b4
	  ret

int86	  endp

cseg	  ends

	  end
