TITLE	JI_HAN  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;****************************************************************************
; Function:
;
; JanusIntHandler  gets # of pending interrupt from jb_interrupts table,
; ---------------  adjusts ES:DI to correspondent interrupt parameter table
;		   and call interrupt handler on PC			
;
;****************************************************************************
;
public	       JanusIntHandler, SendJanusInt
public	       TestInt, Dummy
	
cseg segment   para public 'code'
assume         cs:cseg,ss:cseg,ds:cseg,es:nothing

extrn	       DosInt:near		     ; dos entry
extrn	       outhxw:near		     ; prints hex word in ax
extrn	       outuns:near		     ; prints decimal number in ax
extrn	       outint:near		     ; prints integer in ax
extrn	       newline:near		     ; prints cr,lf
extrn	       pstrng:near		     ; prints out string

include        equ.inc
include        macros.inc
include	       debug.inc
include        handler.inc
include        janus\janusvar.inc
include        vars_ext.inc
.list

JanusIntHandler     proc near

;;;;
;		push si
;		mov  si,offset mymsg
;		call pstrng
;		pop	si
;		jmp	around
;mymsg		db	"janusinthandler called",13,10,0
;around:
;;;;
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
	       mov  es,ax			  ; ES = para mem (=JanusAmiga)

	       mov  si,es:JanusAmiga.ja_Interrupts

	       if   DBG_IntTblMes	 	  ; print interrupt table
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
	       mov  cx,MaxInt-JanInt8		  ; count of SW interrupts
	xor	ax,ax
	mov	dx,ax
	mov	bl,0ffh
	
checkreq:      ;-----------------------------
	cmp	byte ptr es:[si],bl		;0ffh->NC, 07fh->C
	jz	shiftin				;no req here
	mov	byte ptr es:[si],bl		;got it - clear it.
shiftin:
	rcr	dx,1
	rcr	ax,1
	       add  si,2			  ; increment pointer
	       loop checkreq

int_req_enable: ;---------------------------	  ; any enabled interrupt ?
	and	ax,word ptr int_enable
	and	dx,word ptr int_enable+2

	mov	word ptr int_req,ax		;never ref'd, but wtf
	mov	word ptr int_req+2,dx

	mov	bx,ax
	or	bx,dx
	       jne  exec_int

	       if   DBG_NoIntMes
		push si
		mov  si,offset NoIntMes
		call pstrng
		pop  si
	       endif
   
	       jmp  done

exec_int:      ;-----------------------------	  : ES = parameter area
	       mov  si,es:JanusAmiga.ja_Parameters ; points to interrupt
						  ;	      parameter table
	       mov  bx,0			  ; Interrupt service offset 
	       mov  cx,MaxInt			  ; Interrupt service counter
exec_loop:
	rcr	dx,1
	rcr	ax,1
	       jnc  next_int
	       mov  di,es:[si][bx]		  ; es:di points to current
						  ;	  parameter block
	       if   DBG_ExeIntMes
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
	mov  	si,es:JanusAmiga.ja_Interrupts 	; send interrupt acknowledge
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
;
;------------------------------------------------------------------------------
;
; SendJanusInt:		Trigger Janus interrupt on Amiga side.
; 			Exspects Int# in BL.
;
;------------------------------------------------------------------------------
;
SendJanusInt	proc near 	  

	if 	DBG_SEND_JANUS_INT
	push	bp
      	push 	cs			;Print debug message
	pop 	ds
	mov	si,offset SEND_JANUS_INT_MSG ;func
   	call 	pstrng		       
   	call 	newline 		    
	pop	bp
	endif

	push	ax
	push    bx
	push	si
	push	es
	xor	bh,bh
	shl	bx,1
	mov	ax,cs:janus_param_seg
	mov    	es,ax   
	mov	si,es:JanusAmiga.ja_Interrupts	; points to interrupt table
	mov	es:byte ptr [si][bx],AckInt 	; fill message for Amiga
        mov  	al,0ffh					   
        out  	StatusReg,al	     		; trigger SYSINT on Amiga side
        pop	es
	pop     si
	pop     bx
	pop	ax
	ret

SendJanusInt	endp
;
;------------------------------------------------------------------------------
;
; Dummy:	Prints warning. Uninitialized should vectors point to here.
;
;------------------------------------------------------------------------------
;
Dummy	       proc near

	       if (infolevel-40 ge 0)  
		pushall
		push cs
		pop  ds 
		mov  si,offset NoHanMes
		call pstrng
		popall
	       endif

	       ret

Dummy	       endp
;
;------------------------------------------------------------------------------
;
; TestInt: counts interrupts from Amiga for HW diagnostics
;
;------------------------------------------------------------------------------
;
TestInt        proc near

	       if (infolevel-40 ge 0)  
		pushall
		push cs
		pop  ds 
		mov  si,offset TestIntMes
		call pstrng
		inc  IntCount
		mov  ax,15
		call outuns
		mov  ax,IntCount
		call outuns
		popall
	       endif
	       
	       ret

TestInt        endp


cseg	       ends
 
end 

