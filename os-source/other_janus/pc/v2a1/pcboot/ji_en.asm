page 63,132
;******************************************************************************
; Functions:
;
; JanIntDis  disables interrupts from janus board		   
; --------- 
;
; JanIntEn  enables interrupts from janus board 		 
; -------- 
;
;*****************************************************************************

public	       JanIntEn, JanIntDis

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing

include        equ.inc

  
JanIntEn       proc near

	       pushf
	       push ax 
	       push dx
	       mov  dx,IntEn_reg 
	       mov  al,not JanPCInt
	       out  dx,al
	       pop  dx
	       pop  ax
	       popf
	       ret

JanIntEn       endp


JanIntDis      proc near

	       pushf
	       push ax 
	       push dx
	       mov  dx,IntEn_reg 
	       mov  al,JanPCInt
	       out  dx,al
	       pop  dx
	       pop  ax
	       popf
	       ret

JanIntDis      endp


cseg	       ends
 
end 

