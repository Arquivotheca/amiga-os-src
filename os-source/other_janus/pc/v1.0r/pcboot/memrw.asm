;**************************************************************************
;* Module: memrw     Entry: memrw		  Type: Near		  *
;*									  *
;* Function: Memory Read / Write service for AMIGA			  *
;*									  *
;* Parameters: ES:DI - Pointer to MemReadWrite Structure		  *
;*									  *
;* Returns: Stauts in ES:[DI].mrw_Status				  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

     include   memrw.inc
     include   words.inc
     include   macros.inc

w    equ  word ptr

cseg segment   para public 'code'

	  assume cs:cseg,ds:nothing,es:nothing,ss:nothing

	  include   data.inc	   

	  public    memrw

memrw	  proc near
	  pushall

	  mov  bx,es:[di].mrw_Command	     ; what shall we do ?
	  mov  ax,MRWS_BADCMD		     ; check for valid command
	  cmp  bx,5			     ; out of range ?
	  ja   mrwdone			     ; yes if above

	  shl  bx,1
	  mov  es:[di].mrw_Status,MRWS_INPROGRESS
	  mov  cx,es:[di].mrw_Count
	  cld
	  jmp  cs:jmptbl [bx]

write:	  push es
	  push di
	  mov  si,es:[di].mrw_Buffer 
	  mov  ds,janus_buffer_seg
	  les  di,es:[di].mrw_Address 
	  rep  movsb
	  pop  di
	  pop  es
	  cmp  bx,MRWC_WRITEREAD       ; read after write ?
	  jne  mrw_ok		       ; no if ne   
	  mov  cx,es:[di].mrw_Count    ; else restore count
				       ; and fall into read
read:	  lds  si,es:[di].mrw_Address
	  push es
	  push di
	  mov  di,es:[di].mrw_buffer
	  mov  es,janus_buffer_seg
	  rep  movsb
	  pop  di
	  pop  es
	  jmp  mrw_ok

readio:   mov  dx,w es:[di].mrw_Address+low_word
	  push es
	  push di
	  mov  di,es:[di].mrw_buffer
	  mov  es,janus_buffer_seg

rdiolp:   in   al,dx
	  inc  dx
	  stosb     
	  loop rdiolp

	  pop  di
	  pop  es
	  jmp  short mrw_ok   

writeio:  mov  dx,w es:[di].mrw_Address+low_word
	  mov  si,es:[di].mrw_buffer
	  mov  ds,janus_buffer_seg

wriolp:   lodsb      
	  inc  dx
	  out  dx,al
	  loop wriolp

mrw_ok:
	  sub  ax,ax
mrwdone:
	  mov  es:[di].mrw_status,ax
	  popall
	  ret

memrw	  endp

jmptbl	  dw   offset	 mrwdone	     ; 0
	  dw   offset	 read		     ; 1
	  dw   offset	 write		     ; 2
	  dw   offset	 readio 	     ; 3
	  dw   offset	 writeio	     ; 4
	  dw   offset	 write		     ; 5


cseg ends

     end

