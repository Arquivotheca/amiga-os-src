TITLE	OUTHXW  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;**************************************************************************
;* Module: outhxw    Entry: outhxw		  Type: Near		  *
;*									  *
;* Function: Print hex word on screen					  *
;*									  *
;* Parameters: AX - Hex word to print					  *
;*									  *
;* Returns: nothing							  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

public    outhxw,outhxb,outchr,newline,pstrng


cseg segment para public 'code'
assume cs:cseg

include	  equ.inc
include   debug.inc
	
outhxw	  proc near	      ; print hex word in ax
	  xchg al,ah
	  call outhxb
	  xchg al,ah
;**************************************************************************
;* Module: outhxw    Entry: outhxb		  Type: Near		  *
;*									  *
;* Function: Print hex byte on screen					  *
;*									  *
;* Parameters: AL - Hex byte to print					  *
;*									  *
;* Returns: nothing							  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************
outhxb: 		      ; print hex byte in al
	  push ax
	  shr  al,1
	  shr  al,1
	  shr  al,1
	  shr  al,1
	  call outhxn
	  pop  ax
	  and  al,0fh
outhxn:   add  al,'0'
	  cmp  al,'9'
	  jbe  outchr
	  add  al,7  
;**************************************************************************
;* Module: outhxw    Entry: outchr		  Type: Near		  *
;*									  *
;* Function: Print ASCII character on screen				  *
;*									  *
;* Parameters: AL - character to print					  *
;*									  *
;* Returns: nothing							  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

outchr: 		      ; print ascii char in al
	  if (video ge 1)     ; output on screen
	   push ax
	   mov	ah,write_tty
	   int	video_int
	   pop	ax  
	  endif

	  if (serial ge 1)    ; output on serial port
	   push ax 
	   push dx
	   mov	dx,0
	   mov	ah,1
	   int	serial_int
	   pop	dx
	   pop	ax
	  endif

	  ret

;**************************************************************************
;* Module: outhxw    Entry: newline		  Type: Near		  *
;*									  *
;* Function: prints cr/lf sequence on screen				  *
;*									  *
;* Parameters: none							  *
;*									  *
;* Returns: nothing							  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

newline:	   
	  push ax
	  mov  al,CR
	  call outchr
	  mov  al,LF
	  call outchr
	  pop  ax
	  ret
;**************************************************************************
;* Module: outhxw    Entry: pstrng		  Type: Near		  *
;*									  *
;* Function: Print string on screen					  *
;*									  *
;* Parameters: DS:[SI] - Pointer to an ASCIIZ string			  *
;*									  *
;* Returns: [SI] pointing to byte after the terminating zero		  *
;*									  *
;* All other Registers preserved.					  * 
;**************************************************************************

pstrng:
	  push ax
pst2:	  lodsb
	  or   al,al
	  jz   pst1
	  call outchr
	  jmp  pst2

pst1:	  pop  ax
	  ret

outhxw	  endp

cseg	  ends

	  end
