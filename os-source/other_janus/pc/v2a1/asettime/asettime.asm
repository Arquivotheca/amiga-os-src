include c:\include\janus\service.inc
include timeserv.inc
;******************************************************************************
DBG_STARTMSG	=	0
DBG_GETMSG	=	0
DBG_CALLMSG	=	0
DBG_RETMSG	=	0
DBG_HANDMSG	=	0
;******************************************************************************

;******************************************************************************

SSEG	SEGMENT PARA STACK 'STACK'
    	db	12	DUP('STACKSEG')
SSEG	ENDS

;******************************************************************************

DSEG 	SEGMENT PARA 'DATA'
STARTMSG	db	'Service Test Starting!',13,10,'$'
AGETMSG		db	'After GetService Called!',13,10,'$'
ACALLMSG	db	'After CallService Called!',13,10,'$'
ARETMSG		db	'After CallService Returns!',13,10,'$'
HANDMSG		db	'GetService Handler Called',13,10,'$'
SERVERRMSG	db	'Error TimeServ service not available',13,10,'$'
SDATASEG	dw	0
SDATAOFF	dw	0
SPTRSEG		dw	0
SPTROFF		dw	0
FLAG		dw	0
DSEG 	ENDS

;******************************************************************************

CSEG	SEGMENT PARA  'CODE'


START	PROC	FAR
	ASSUME	CS:CSEG,DS:DSEG,SS:SSEG,ES:NOTHING
	PUSH	DS
	SUB	AX,AX
	PUSH	AX
	MOV	AX,DSEG
	MOV	DS,AX

;******	CODE STARTS HERE ! *************

	if	DBG_STARTMSG
	mov	dx,offset STARTMSG	;Print startup message
	mov 	ah,09h
	int	21h  
	endif

	mov	ah,1			;Wake up int b, Must call at least
	mov	al,16			;once before using to set up janus
	int	0bh

	
					;Set up for GetService
	mov	si,APPLICATION_ID_H	;Set Application ID
	mov	dx,APPLICATION_ID_L
	mov	cx,TIMESERV_LOCAL_ID	;Local service ID
	mov	ah,J_GET_SERVICE	;Function code
	mov	al,0			;Don't wait for service to
					;become available
	push	cs
	pop	es			;get code seg into es to set function
	mov	di,offset cs:Handler	;set offset of our handler
	int	0bh

	push	ax
	mov	SDATASEG,es		;Stash service data ptr	
	mov	SDATAOFF,di
	mov	ax,es:ServiceData.MemOffset[di]
	mov	SPTROFF,ax
	pop	ax
		
	cmp	al,J_OK			;If Get fails print msg and exit
	je	gotserv
	mov	dx,offset SERVERRMSG
	mov	ah,09h
	int	21h
	ret

gotserv:
	if	DBG_GETMSG
	mov	dx,offset AGETMSG	;Print after GS message
	mov 	ah,09h
	int	21h	
	endif

	mov	FLAG,1
	mov	ah,J_CALL_SERVICE      ;Set up for call service
	mov	di,SDATAOFF
	int	0bh

	if	DBG_CALLMSG
	mov	dx,offset ACALLMSG	;print after call service msg
	mov	ah,09h
	int 	21h
	endif

tWait:	cmp	FLAG,1			;wait for int routine to change
	je	tWait			;wait flag

	mov	ax,SPTROFF		;set up for structure access
	mov	di,ax

	mov	ah,2bh			;set date
	mov	cx,es:TimeServReq.tsr_Year[di]
	mov	dh,es:TimeServReq.tsr_Month[di]
	mov	dl,es:TimeServReq.tsr_Day[di]

	int	21h

	mov	ah,2dh
	mov	ch,es:TimeServReq.tsr_Hour[di]
	mov	cl,es:TimeServReq.tsr_Minutes[di]
	mov	dh,es:TimeServReq.tsr_Seconds[di]
	mov	dl,0
	int	21h

	mov	ah,09h
	push	ds
	push	es
	pop	ds
	lea	dx,es:TimeServReq.tsr_String[di]
	int	21h	
	pop	ds

 	if	DBG_RETMSG
	mov	dx,offset ARETMSG	;print funtion end msg
	mov	ah,09h
	int 	21h
	endif

	mov	es,SDATASEG		;Release The Service
	mov	di,SDATAOFF
	mov	ah,J_RELEASE_SERVICE	
	int	0bh

	RET
START	ENDP

Handler	proc	far			;Routine for handling function
	push	ax			;completion. After a call service
	push	ds			;this routine will be called to 
	push	dx			;indicate the requested funtion
	mov	ax,DSEG			;has completed.
	mov	ds,ax

	if	DBG_HANDMSG
	mov	dx,offset HANDMSG
	mov	ah,09h
	int	21h
	endif

	mov  	ds:FLAG,0ffffh
	pop	dx
	pop	ds
	pop	ax
	ret
Handler	endp
CSEG	ENDS
	END	start

