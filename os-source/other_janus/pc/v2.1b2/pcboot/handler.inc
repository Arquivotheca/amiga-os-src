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
extrn	       ServiceDispatch:near
;
HandlerTab     	label	 word			;Int#
		dw	offset	dummy		;  0 Amiga HW int
	       	dw	offset  dummy		;  1	    "
	       	dw	offset  dummy		;  2	    "
	       	dw	offset  dummy		;  3	    "
	       	dw	offset  dummy		;  4	    "
	       	dw	offset  dummy		;  5	    "
	       	dw	offset  dummy		;  6	    "
	       	dw	offset  dummy		;  7 PC int. handler ready
	       	dw	offset  dummy		;  8 PC scrolling
	       	dw	offset 	Janus_disk_io  	;  9 Amiga reading PC harddisk
	       	dw	offset	dummy	  	; 10 PC reading Amiga ram
	       	dw	offset  memrw	  	; 11 Amiga reading PC ram
	       	dw	offset  dummy	  	; 12 PC calling Amiga funct.
	       	dw	offset  int86	  	; 13 AMIGA causing PC int.
	       	dw	offset  dummy	  	; 14 PC calls Amiga service
	       	dw	offset	ServiceDispatch	; 15 Amiga calls PC service
	       	dw	offset  dummy    	; 16 PCDisk
	       	dw	offset  dummy    	; 17 AMouse
	       	dw	offset  dummy    	; 18
	       	dw	offset  dummy    	; 19
	       	dw	offset  dummy    	; 20
	       	dw	offset  dummy    	; 21
	       	dw	offset  dummy    	; 22
	       	dw	offset  dummy    	; 23
	       	dw	offset  dummy    	; 24
	       	dw	offset  dummy    	; 25
	       	dw	offset  dummy    	; 26
	       	dw	offset  dummy    	; 27
	       	dw	offset  dummy    	; 28
	       	dw	offset  dummy    	; 29
	       	dw	offset  dummy    	; 30
	       	dw	offset  dummy    	; 31
	       	dw	offset  dummy    	; 32
;			
;------------------------------------------------------------------------------
