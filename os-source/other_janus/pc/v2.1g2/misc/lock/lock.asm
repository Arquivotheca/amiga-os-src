include janus\janus.inc
;include janus\timeserv.inc
;include janus\servmac.inc
;include janus\services.inc
include lockserv.inc

;******************************************************************************
;
;  Locking macros for JanusMemory
;
;  Locks access to janus data structures from AMIGA side
;
;******************************************************************************
;
;JLOCK	  macro semaphore
;	  local jlstart,jlexit
;
;	mov	al,0ffh
;jlstart:
;	  stc	
;    lock  xchg byte ptr semaphore,al
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  nop
;	  or	al,al
;	  js	jlstart
;jlexit:
;
;jlstart:
;	stc	
;        lock  rcl byte ptr semaphore,1
;	jnc	jlexit
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	nop
;	jmp 	jlstart
;jlexit:
;
;	  endm
;
;
;UNLOCK	  macro semaphore
;
;	  mov  semaphore,7fh
;
;	  endm


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
SERVERRMSG	db	'Error LockServ service not available',13,10,'$'
HelpMSG		db	'Locktest Ver. 3.0 (library only)',13,10
		db	'Locktest [n] [/w]',13,10
		db	'        n = number of times to perform loop.  If 0 or omitted,',13,10
		db	'            loop runs forever, printing time and date every 1000',13,10
		db	'            passes.  Otherwise, the number of BIOS clock ticks the',13,10
		db	'            loop takes is displayed.  NOTE: n is multiplied by 100',13,10
		db	'            before use, so n=1 means 100 loops.',13,10
		db	'        /w = wait for the service to become available.',13,10
		db	'$'
CNTMSG1		db	'The test took $'
CNTMSG2		db	' (hex) BIOS ticks.',13,10,'$'
SDATASEG	dw	0
SDATAOFF	dw	0
SPTRSEG		dw	0
SPTROFF		dw	0
FLAG		dw	0
WaitStatus	db	0
loopcount	dw	0
decbuf		db	'?????$'

DSEG 	ENDS

;******************************************************************************

CSEG	SEGMENT PARA  'CODE'


START	PROC	FAR
	ASSUME	CS:CSEG,DS:NOTHING,SS:SSEG,ES:NOTHING
	PUSH	DS
	SUB	AX,AX
	PUSH	AX

	mov	ax,DSEG
	mov	es,ax
	ASSUME	ES:DSEG
						
	xor	cx,cx
	mov	es:loopcount,cx			;init loopcount to 0

	mov	si,80h				;point to cmd line start
	or	cl,[si]				; read length of command line
	jz	endcmdline			;no cmd line.

	inc	si				;point past count

	call	skipspace			;skip whitespace
	jz	endcmdline			;if nothing there.

	;loop to get a number from 0..65535 for loop count
Readcount:
	mov	bl,[si]     			; read command line now

	;try for ascii 0..9.  if not, go to tryslash.
	sub	bl,'0'
	jc	tryslash
	cmp	bl,10
	jnc	tryslash

       	inc	si				;we got a 0..9
	xor	bh,bh				;nuke upper byte

	push	bx				;save it
	mov	ax,es:loopcount			;get running value
	mov	bx,10				;mul by 10
	mul	bx
	pop	bx
	add	ax,bx				;add this digit
	mov	es:loopcount,ax			;save it

	loop	Readcount			; loop until nothing found

	jmp	endcmdline			;if we get here, no switches

	;here we try to parse /w and /h.
tryslash:
	call	skipspace			;eat whitespace
	jz	endcmdline			;end of line.

	mov	bl,[si]     			; read command line now
	cmp	bl,'/'
	jnz	Help				;if it's not a slash, error.

	inc	si				;eat the slash
	dec	cx
	jz	Help				;oops - nothing after it

	mov	bl,[si]				; read char after slash
	inc	si				;point past it.
	or	bl,20h				; not case sensitive

	cmp	bl,'w'				; it's wait ?
	jz	waitflag			;yeah, go.

	jmp	Help   				;the number you have reached...

waitflag:
	mov	WaitStatus,GETS_WAIT		
	jmp	endslash

endslash:
	loop	tryslash
	jmp	endcmdline

Help:
	assume  ds:DSEG				; set our data segment now
	MOV	AX,DSEG
	MOV	DS,AX

	mov	dx,offset HelpMSG		; Print help message
	mov 	ah,09h
	int	21h  
	jmp	getout

;-------------------------------------------------------------------------
; real work starts here.

endcmdline:
	assume  ds:DSEG
	MOV	AX,DSEG
	MOV	DS,AX

	if	DBG_STARTMSG
	mov	dx,offset STARTMSG		;Print startup message
	mov 	ah,09h
	int	21h  
	endif

	GETBASE JSERV_PCDISK

	GETSERVICE Lockserv_Application_ID_H,Lockserv_Application_ID_L,Lockserv_Local_ID,WaitStatus,cs,Handler

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

	mov	cx,loopcount
	jcxz	forever

	mov	ah,0		;ask bios for current ticks
	int	1ah
	push	cx		;cx has upper word
	push	dx		;dx has lower word

	mov	cx,loopcount
	lea	di,LockServReq.lsr_b1[di]

hosehead:
	push	cx
	mov	cx,100
inner:
;	JLOCK	es:LockServReq.lsr_b1[di]
;	UNLOCK	es:LockServReq.lsr_b1[di]

	mov	ah,JFUNC_LOCK
	int	JFUNC_JINT

	mov	ah,JFUNC_UNLOCK
	int	JFUNC_JINT

	loop	inner
	pop	cx
	loop	hosehead

	mov	ah,0		;get bios ticks after test
	int	1ah
	pop	bx		;get lower word of original count
	sub	dx,bx		;diff
	pop	bx   		;get upper word of original count
	sbb	cx,bx		;diff

	push	dx		;save lower word
	push	cx		;save upper word

	mov	dx,offset CNTMSG1
	mov	ah,9
	int	21h

	pop	ax
	call	dumphexword
	pop	ax
	call   	dumphexword

	mov	dx,offset CNTMSG2
	mov	ah,9
	int	21h

	jmp	alldone

forever:
	call	printtime
	mov	al,13
	call	dumpascii
	mov	al,10
	call	dumpascii

;	JLOCK	es:LockServReq.lsr_b1[di]
	mov	ah,JFUNC_LOCK
	lea	di,LockServReq.lsr_b1[di]
	int	JFUNC_JINT
	
for2:
	call	printtime

	mov	al,13
	call	dumpascii

	mov	cx,1000

for1:
;	UNLOCK	es:LockServReq.lsr_b1[di]
	mov	ah,JFUNC_UNLOCK
	int	JFUNC_JINT

;	JLOCK	es:LockServReq.lsr_b1[di]
	mov	ah,JFUNC_LOCK
	int	JFUNC_JINT

	loop	for1

       	jmp	for2

alldone:

 	if	DBG_RETMSG
	mov	dx,offset ARETMSG		;print funtion end msg
	mov	ah,09h
	int 	21h
	endif

	RELEASESERVICE SDATAOFF

getout:	RET
START	ENDP

printtime	proc	near
	mov	ah,2ah			;get dos date
	int	21h

	push	cx

	xor	ah,ah			;month
	mov	al,dh
	mov	cx,2
	call	dumpdecword

	mov	al,'/'
	call	dumpascii

	mov	al,dl			;day of month
	mov	cx,2
	call	dumpdecword

	mov	al,'/'
	call	dumpascii

	pop	ax			;year

	mov	cx,4
	call	dumpdecword

	mov	al,' '
	call	dumpascii

	mov	ah,2ch			;get dos time
	int	21h

	push	cx

	xor	ah,ah			;hour
	mov	al,ch
	mov	cx,2
	call	dumpdecword

	mov	al,':'
	call	dumpascii

	pop	cx

	mov	al,cl			;minute
	mov	cx,2
	call	dumpdecword

	mov	al,':'
	call	dumpascii

	mov	al,dh			;seconds
	mov	cx,2
	call	dumpdecword

	ret
printtime	endp

dumpdecword	proc	near
	push	ax
	push	bx
	push	cx
	push	dx
	push	di

	mov	bx,10
	lea	di,decbuf+5

divloop:
	xor	dx,dx
	div	bx
	add	dl,'0'
	dec	di
	mov	[di],dl
	loop	divloop

	mov	ah,9
	mov	dx,di
	int	21h

	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
dumpdecword	endp

dumpascii	proc	near
	push	ax
	push	dx
	mov	ah,2
	mov	dl,al
	int	21h
	pop	dx
	pop	ax
	ret
dumpascii	endp

dumphexword	proc	near
	push	ax
	mov	al,ah
	call	dumphexbyte
	pop	ax
	call	dumphexbyte
	ret
dumphexword	endp

dumphexbyte	proc	near
	push	ax
	mov	cl,4
	shr	al,cl
	call	dumphexnib
	pop	ax
	call	dumphexnib
	ret
dumphexbyte	endp

dumphexnib	proc	near
	and	al,0fh
	cmp	al,10
	jnc	letter
	add	al,'0'
	jmp	doit
letter:	add	al,'A'-10
doit:	mov	dl,al
	mov	ah,2
	int	21h
	ret
dumphexnib	endp

skipspace	proc	near
skipit:	cmp	byte ptr [si],' '
	jnz	quit
	inc	si
	loop	skipit
	xor	ax,ax
quit:	ret
skipspace	endp

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