;******************************************************************************
;
;  Locking macros for JanusMemory
;
;  Locks access to janus data structures from AMIGA side
;
;******************************************************************************

JLOCK	  macro semaphore
	  local jlstart,jlexit

	mov	al,0ffh
jlstart:
	  stc	
    lock  xchg byte ptr semaphore,al
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  nop
	  or	al,al
	  js	jlstart
jlexit:
	  endm


UNLOCK	  macro semaphore

	  mov  semaphore,7fh

	  endm

LockTest	struc
	lb	db	0
LockTest	ends

include janus\timeserv.inc
include janus\servmac.inc
include janus\services.inc

;******************************************************************************

DBG_STARTMSG	=	1
DBG_GETMSG	=	1
DBG_CALLMSG	=	1
DBG_RETMSG	=	1
DBG_HANDMSG	=	1

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
HelpMSG		db	'ASetTime Ver. 1.0',13,10
		db	'ASetTime /w waits until service come available',13,10,'$' 
SDATASEG	dw	0
SDATAOFF	dw	0
SPTRSEG		dw	0
SPTROFF		dw	0
FLAG		dw	0
WaitStatus	db	0

DSEG 	ENDS

;******************************************************************************

CSEG	SEGMENT PARA  'CODE'


START	PROC	FAR
	ASSUME	CS:CSEG,DS:NOTHING,SS:SSEG,ES:NOTHING
	PUSH	DS
	SUB	AX,AX
	PUSH	AX
					
	xor	cx,cx
	mov	si,80h
	or	cl,[si]				; read length of command line
	jz	NoSwitch

ReadLine:
	inc	si
	mov	bl,[si]     			; read command line now
	cmp	bl,'/'
	je	hit
	loop	ReadLine			; loop end and nothing found
						;  => help user
Help:
	mov	dx,offset HelpMSG		; Print help message
	mov 	ah,09h
	int	21h  
	jmp	short Switch

Hit:						; found our switch
	inc	si
	mov	bl,[si]				; read it

	assume  ds:DSEG				; set our data segment now
	MOV	AX,DSEG
	MOV	DS,AX

	or	bl,20h				; not case sensitive
	cmp	bl,'w'				; it's wait ?
	jne	Help				; no => help user
	mov	WaitStatus,GETS_WAIT		

NoSwitch:
	assume  ds:DSEG
	MOV	AX,DSEG
	MOV	DS,AX

	if	DBG_STARTMSG
	mov	dx,offset STARTMSG		;Print startup message
	mov 	ah,09h
	int	21h  
	endif

Switch:
	GETBASE JSERV_PCDISK

	GETSERVICE Timeserv_Application_ID_H,Timeserv_Application_ID_L,Timeserv_Local_ID,WaitStatus,cs,Handler

	push	ax
	mov	SDATASEG,es			;Stash service data ptr	
	mov	SDATAOFF,di
	mov	ax,es:ServiceData.sd_MemOffset[di]
	mov	SPTROFF,ax
	pop	ax
		
	cmp	al,JSERV_OK			;If Get fails print msg and exit
	je	gotserv
	mov	dx,offset SERVERRMSG
	mov	ah,09h
	int	21h
	ret

gotserv:
	if	DBG_GETMSG
	mov	dx,offset AGETMSG		;Print after GS message
	mov 	ah,09h
	int	21h	
	endif

	mov	FLAG,1		       		;Init our WAIT flag

;	CALLSERVICE SDATAOFF
	
	if	DBG_CALLMSG
	mov	dx,offset ACALLMSG		;print after call service msg
	mov	ah,09h
	int 	21h
	endif

;tWait:	cmp	FLAG,1				;wait for int routine to change
;	je	tWait				;wait flag

	mov	ax,SPTROFF			;set up for structure access
	mov	di,ax

;	mov	ah,2bh				;set date
;	mov	cx,es:TimeServReq.tsr_Year[di]
;	mov	dh,es:TimeServReq.tsr_Month[di]
;	mov	dl,es:TimeServReq.tsr_Day[di]

;	int	21h

;	mov	ah,2dh		   		;Set time
;	mov	ch,es:TimeServReq.tsr_Hour[di]
;	mov	cl,es:TimeServReq.tsr_Minutes[di]
;	mov	dh,es:TimeServReq.tsr_Seconds[di]
;	mov	dl,0
;	int	21h

;	mov	ah,09h		      		;Print Date and Time
;	push	ds
;	push	es
;	pop	ds
;	lea	dx,es:TimeServReq.tsr_String[di]
;	int	21h	
;	pop	ds


hosehead:
	JLOCK	es:LockTest.lb[di]
	UNLOCK	es:LockTest.lb[di]
	mov 	ah,02h
	mov	dl,44
	int 	21h
	jmp	hosehead

 	if	DBG_RETMSG
	mov	dx,offset ARETMSG		;print funtion end msg
	mov	ah,09h
	int 	21h
	endif

	RELEASESERVICE SDATAOFF

	RET
START	ENDP

Handler	proc	far				;Routine for handling function
	push	ax				; completion. After a call
	push	ds				; service this routine will  
	push	dx				; be called to indicate the 
	mov	ax,DSEG				; requested funtion has 
	mov	ds,ax				; completed.

	if	DBG_HANDMSG
	mov	dx,offset HANDMSG
	mov	ah,09h
	int	21h
	endif

	mov  	ds:FLAG,0ffffh	    		;Set our wait flag to wake up
	pop	dx				;the main program
	pop	ds
	pop	ax
	ret
Handler	endp
CSEG	ENDS
	END	start

