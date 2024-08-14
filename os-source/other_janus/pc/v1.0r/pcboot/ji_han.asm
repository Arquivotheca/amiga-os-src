page 63,132

;****************************************************************************
; Function:
;
; JanusIntHandler  gets # of pending interrupt from jb_interrupts table,
; ---------------  adjusts ES:DI to correspondent interrupt parameter table
;		   and call interrupt handler on PC			
;
;****************************************************************************
;
public	       JanusIntHandler

extrn	       DosInt:near		     ; dos entry
;
; external utilities
;
extrn	       outhxw:near		     ; prints hex word in ax
extrn	       outuns:near		     ; prints decimal number in ax
extrn	       outint:near		     ; prints integer in ax
extrn	       newline:near		     ; prints cr,lf
extrn	       pstrng:near		     ; prints out string

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing

include        macros.inc
include        data.inc
include        equ.inc
include        handler.inc
include        janusvar.inc
include        mes.inc
include        vars_ext.inc
.list

JanusIntHandler     proc near

	       cli				  ; init our own stack
	       mov  cs:ustack+2,ss
	       mov  cs:ustack,sp
	       mov  ss,cs:sstack+2
	       mov  sp,cs:sstack
	       pushall
	       sti		     

	       push cs
	       pop  ds				  ; DS = CS
	       mov  ax,cs:janus_base_seg		 
	       mov  es,ax			  ; ES = para mem (=JanusBase)

	       mov  si,es:JanusBase.jb_Interrupts

	       if   (infolevel-55 ge 0) 	  ; print interrupt table
		push si
		mov  si,offset IntTblMes
		call pstrng
		pop  ax
		push ax
		call outhxw			  ; print the offset
		call newline
		pop  si
		push si
		mov  cx,MaxInt
Table0: 	mov  ax,es:[si]
		call outhxw			  ; print the entries
		call newline
		add si,2
		loop Table0
		pop  si
	       endif

	       add  si,JanInt8*2+1		  ; points to 1st SW interrupt
	       mov  ax,JanInt8			  ; current interrupt #
	       mov  cx,MaxInt-JanInt8		  ; count of SW interrupts
	       mov  bx,0100h			  ; set bit8   
	       mov  word ptr int_req,0		  ; clear interrupt 
	       mov  word ptr int_req+2,0	  ; request register

checkreq:      ;-----------------------------
	       stc 
	       rcl  byte ptr es:[si],1		  ; test and clear interrupt #
	       jc   noreq			  ; no pending interrupt
	       cmp  cx,16
	       jle  ir_high_word
	       or   word ptr int_req,bx 	  ; set correspondent bit 
	       jmp  short ir_low_word		  ;	in int_req_low 
ir_high_word:
	       or   word ptr int_req+2,bx	  ; set correspondent bit
						  ;	in int_req_high
ir_low_word:
	       if   (infolevel-46 ge 0)
		push si
		mov  si,offset FoundIntMes
		call pstrng
		call outint			  ; print interrupt #
		call newline
		pop  si
	       endif
noreq:
	       inc  ax				  ; increment counter
	       rol  bx,1			  ; shift pattern
	       add  si,2			  ; increment pointer
	       loop checkreq

int_req_enable: ;---------------------------	  ; any enabled interrupt ?
	       mov  bx,word ptr int_enable
	       and  word ptr int_req,bx
	       mov  bx,word ptr int_req
	       mov  ax,word ptr int_enable+2
	       and  word ptr int_req+2,ax
	       or   bx,word ptr int_req+2	  ; any enabled interrupt ?
	       jne  exec_int

	       if   (infolevel-47 ge 0)
		push si
		mov  si,offset NoIntMes
		call pstrng
		pop  si
	       endif
   
	       jmp  done

exec_int:      ;-----------------------------	  : ES = parameter area
	       mov  si,es:JanusBase.jb_Parameters ; points to interrupt
						  ;	      parameter table
	       mov  ax,word ptr int_req 	  ; load ir_low
	       mov  dx,word ptr int_req+2	  ; load ir_high
	       mov  bx,0			  ; Interrupt service offset 
	       mov  cx,MaxInt			  ; Interrupt service counter
exec_loop:
	       cmp  cx,16
	       ja   ei_low_word 
	       ror  dx,1
	       jmp  short ei_high_word
ei_low_word:
	       ror  ax,1
ei_high_word:
	       jnc  next_int
	       mov  di,es:[si][bx]		  ; es:di points to current
						  ;	  parameter block
	       if   (infolevel-47 ge 0)
		push ax
		push bx
		push si
		push di
		push es
		mov  si,offset ExeIntMes
		call pstrng
		mov  ax,bx
		shr  ax,1			  ; ax/2
		call outint			  ; print interrupt #
		call newline
		mov  si,offset ESDI_Mes
		call pstrng
		pop  ax
		call outhxw			  ; print es
		pop  ax
		call outhxw			  ; print di
		call newline
		pop  si
		pop  bx
		pop  ax
	       endif
 
        call 	word ptr HandlerTab [bx]        ; action
	       
        push 	bx
        shr  	bx,1				; service # in BX
        inc  	byte ptr cs:ServStatTab[bx]	; PC service installed ?
						; also sets status to finished
	jnz  	service_found			; yes	
        dec  	byte ptr cs:ServStatTab[bx]	; reset status
	pop 	bx
	push 	si	
	mov  	si,es:JanusBase.jb_Interrupts 	; send interrupt acknowledge
	mov  	es:byte ptr [si][bx],AckInt   	; message to AMIGA
	pop  	si
	jmp	short next_int
service_found:
	pop	bx
next_int:
	       add  bx,2			  ; next offset
	       loop exec_loop			  ; next interrupt service
done:
	       cli				  ; free our own stack
	       popall
	       mov  ss,cs:ustack+2
	       mov  sp,cs:ustack
	       sti		     
	       ret

JanusIntHandler     endp

cseg	       ends
 
end 

