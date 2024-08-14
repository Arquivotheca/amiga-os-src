TITLE	OUTDEC  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;**************************************************************************
;* Module: outdec    Entry: outint		     Type: Near 	  *
;*									  *
;* Function: print signed decimal integer word				  *
;*									  *
;* Parameters: AX - Integer to print					  *
;*									  *
;* Returns: nothing							  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

public 	outint,outuns

cseg segment para public 'code'
assume 	cs:cseg

include 	macros.inc

extrn		outchr:near

	     
outint	     proc near
	     pushall

	     mov    bl,' '
	     or     ax,ax
	     jns    out1  		; was outp
	     mov    bl,'-'
	     neg    ax
outp:	     push   ax
	     mov    al,bl
	     call   outchr
	     pop    ax
	     jmp    short out1
;**************************************************************************
;* Module: outdec    Entry: outuns		     Type: Near 	  *
;*									  *
;* Function: print unsigned decimal integer word			  *
;*									  *
;* Parameters: AX - Integer to print					  *
;*									  *
;* Returns: nothing							  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

outuns	      label near
	      pushall
out1:
	      sub   cx,cx
	      sub   dx,dx
	      mov   bx,10

out2:	      div   bx
	      push  dx
	      inc   cx
	      sub   dx,dx
	      or    ax,ax
	      jnz   out2

out3:	      pop   ax
	      add   al,'0'
	      call  outchr
	      loop  out3

	      popall
	      ret

outint	     endp

cseg ends

     end

