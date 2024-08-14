;******************************************************************************
;
; handler.inc  contains offsets of PC handlers for AMIGA requests.
;	       Requests for not installed handler points to dummy
;	       routine and responds with error message on PC screen
;
;******************************************************************************
;
extrn	       janus_disk_io:near
extrn	       memrw:near
extrn	       int86:near
;
HandlerTab     label	 word			  ;Int#
	       dw	 offset     dummy	  ; 0 Amiga HW int
	       dw	 offset     dummy	  ; 1	    "
	       dw	 offset     dummy	  ; 2	    "
	       dw	 offset     dummy	  ; 3	    "
	       dw	 offset     dummy	  ; 4	    "
	       dw	 offset     dummy	  ; 5	    "
	       dw	 offset     dummy	  ; 6	    "
	       dw	 offset     dummy	  ; 7 PC int. handler ready
	       dw	 offset     dummy	  ; 8 PC scrolling
	       dw	 offset    Janus_disk_io  ; 9 Amiga reading PC harddisk
	       dw	 offset     dummy	  ;10 PC reading Amiga ram
	       dw	 offset    memrw	  ;11 Amiga reading PC ram
	       dw	 offset     dummy	  ;12 PC calling Amiga funct.
	       dw	 offset    int86	  ;13 AMIGA causing PC int.
	       dw	 offset     dummy	  ;14 PC inits a new service
	       dw	 offset    TestInt	  ;15 Amiga inits a new service
	       dw	 offset     dummy    
;
dummy	       proc near

	       if (infolevel-40 ge 0)  
		pushall
		push cs
		pop  ds 
		mov  si,offset NoHanMes
		call pstrng
		popall
	       endif

	       ret

dummy	       endp
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

IntCount       dw   ?

TestInt        endp
;
;------------------------------------------------------------------------------
