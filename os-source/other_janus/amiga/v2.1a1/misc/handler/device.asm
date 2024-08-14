;******************************************************************************
;* 
;*	AmigaDos/MSDOS Handler 
;*
;*	This routine has stolen timeserve's ID number.  When it grows up
;*	it should get one of its own.  Also, the code is littered with 
;*	vestiges of old approaches to various problems  and now-obsolete bug
;*	workarounds.  Keeping this in mind, I suggest that if you do lift 
;*	a of piece code, understand what it does or else you could end up with
;*	something that is seriously non-optimal.  
;*
;******************************************************************************

include janus\timeserv.inc
include janus\servmac.inc
include janus\services.inc
include janus\pc.inc
;******************************************************************************

LOCKDONE 	= 	2001
EXAMDONE	=	2003
OLOCKDONE	=	2004
READDONE	=	2006
PAREDONE	=	2007
SEEKDONE	=	2008
PCLODONE	=	2009
OPEN2		=	2010
WRITDONE	=	2011

OPEN_FILE	=	3DH
CHDIR		=	3BH
CLSE		=	3EH
FIND_FIRST	=	4EH
FIND_NEXT	=	4FH
SET_DTA		=	1AH
READ_FILE	=	3FH
WRIT_FILE	=	40H
SEEK		=	42H
CRET_FILE	=	3CH

TERMINATOR	=	'$'
NAMESIZE	=	512


;******************************************************************************

;******************************************************************************


DOS		=	21h
DOSPrintStrg	=	9

DOSPrt	macro	string
	push	ax
	push	dx
	mov	dx,offset string
	mov	ah,DOSPrintStrg
	int	DOS
	pop	dx
	pop	ax
	endm

SSEG	SEGMENT PARA STACK 'STACK'
    	db	32	DUP('STACKSEG')
SSEG	ENDS

;******************************************************************************

DSEG 	SEGMENT PARA 'DATA'

STARTMSG	db	'Service Test Starting!',13,10,'$'
;;ACALLMSG	db	'CallService Called!',13,10,'$'
;;ARETMSG	db	'CallService Returns!',13,10,'$'
SDATASEG	dw	0
SDATAOFF	dw	0
SPTRSEG		dw	0
SPTROFF		dw	0
FLAG		dw	0
WaitStatus	db	GETS_WAIT

TMPBUF		db	1024	dup (0)
OpenCreat	db	0
DSEG 	ENDS

;******************************************************************************

CSEG	SEGMENT PARA  'CODE'


START	PROC	FAR
	ASSUME	CS:CSEG,DS:NOTHING,SS:SSEG,ES:NOTHING
	PUSH	DS
	SUB	AX,AX
	PUSH	AX
	MOV	AX,DSEG
	MOV	DS,AX			; setup data seg

sm:
	DOSPrt	StartMsg		; prints header
					

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
;	DOSPrt	ErrorMsg
	jmp	sm				;loop...


FUNTBL	DW	GETLOCK
	DW	EXAMINE
	DW	EXNEXT
	DW	OPENLK
	DW	READD
	DW	PARENT
	DW	ASEEK
	DW	PRECLOS
	DW	OPENNX
	DW	CLOZE
	DW	WRIT
	DW	CREATE
gotserv:
	cmp	FLAG,0				;wait for int routine to change
	je	gotserv				;wait flag

	
;;	mov	dx,offset ACALLMSG		;print  call service msg
;;	mov	ah,09h
;;	int 	21h

	mov	FLAG,0				;reset flag	


	mov	ax,SPTROFF			;set up for structure access
	mov	di,ax


	MOV	AX,0				
	MOV	ES:PCDosServ.f_error[DI],AX	;clear error

;	Set up DTA Address
;
;	PUSH	DS
;	PUSH	ES
;	POP	DS
;	MOV	DX, WORD PTR ES:PCDosServ.Xfer[DI]	
;	MOV	AH,SET_DTA			;function = set DTA
;	INT	DOS	
;	POP	DS


;	MOV	SI, CS:FUNTBL[SI]

	MOV	SI, OFFSET CS:FUNTBL		
	MOV	AX,ES:PCDosServ.f_function[DI]	;Get function to perform
	ADD	SI,AX				;disp table offset of function
	ADD	SI,AX				;do twice to get word not byte
	JMP	CS:[SI]				;Do function

;
; Get Lock
; 
; Get a  PC file/directory "lock".     
; Output:
;
;	PCDosServ.f_error	error code (0 = no error)
;	PCDosServ.f_filehandle	filehandle if file opened,  0 if dir found 
;							 
;	
OPENNX:
	MOV	AX,OPEN2
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	JMP	ENTRY1
CREATE:
	MOV	AX,OLOCKDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	MOV	AL,CRET_FILE
	MOV	OpenCreat,AL
	JMP	ENTRY2

OPENLK:
	MOV	AX,OLOCKDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	JMP	ENTRY1

PARENT:
	MOV	AX,PAREDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	JMP	ENTRY1


GETLOCK:
	

	MOV	AX,LOCKDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
ENTRY1:
	MOV	AL,OPEN_FILE
	MOV	OpenCreat,AL
ENTRY2:

	push	ds
	push	es
	pop	ds
	
;;	lea	dx,PCDosServ.f_name1[DI]
;;	mov	ah,DOSPrintStrg
;;	mov	bx,dx
;;	mov	[bx+114],BYTE PTR TERMINATOR
;;	int	DOS				;print name

	pop	ds



	
	LEA	DX,ES:PCDosServ.f_name1[DI]	;path\name
	CALL	GETNSTRING			;put path\name in TMPBUF
	MOV	DX,OFFSET TMPBUF		
	

;	MOV	AX,ES:PCDosServ.f_access[DI]	;get mode (only using AL)
	mov	al,0	;temporary 
	MOV	AH,OpenCreat			;function = open or create file
	INT	DOS	
	JC	LK_ERR				;open failed

	MOV	ES:PCDosServ.f_filehandle[DI],AX	;return filehandle
	JMP	LK_END

LK_ERR:						;See if directory--do CD
	MOV	ES:PCDosServ.f_error[DI],AX	;if error, will return file (not CD) error
	LEA	DX,ES:PCDosServ.f_name1[DI]		;path\name
	CALL	GETNSTRING			;put path\name in TMPBUF
	MOV	DX,OFFSET TMPBUF		
	MOV	AH,CHDIR			;function = change directory
	INT	DOS
	MOV	BX,0
	MOV	ES:PCDosServ.f_filehandle[DI],BX ;filehandle 0 regardless of outcome
	JC	LK_END				;can't get to directory
	MOV	ES:PCDosServ.f_error[DI],BX	;no error
	
LK_END:
	JMP	ANSWER

EXAMINE:




;	PUSH	DS
;	PUSH	ES
;	POP	DS
;	MOV	DX, WORD PTR ES:PCDosServ.Xfer[DI]	
;	MOV	AH,SET_DTA			;function = set DTA
;	INT	DOS	
;	POP	DS



	MOV	AX,EXAMDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done

	LEA	DX,ES:PCDosServ.f_name1[DI]		;path\name
	CALL	GETNSTRING			;put path\name in TMPBUF
	MOV	DX,OFFSET TMPBUF		
	MOV	AH,FIND_FIRST			;function = find first
	MOV	CX,0FFH				;all attributes
	INT	DOS	
	MOV	ES:PCDosServ.f_error[DI],AX	;return error 
FOUND:
	JMP	COPYDTA
		
EXNEXT:

	MOV	AX,EXAMDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
;	LEA	DX,PCDosServ.f_name1[DI]		;path\name
	MOV	AH,FIND_NEXT			;function = find next
	INT	DOS	
	MOV	ES:PCDosServ.f_error[DI],AX	;return error 
	JMP	FOUND
	
	
COPYDTA:
	push 	di
	push	ds
	push	es
	mov	ah,2fH				;Get DTA
	int	DOS
	push 	es
	pop	ds
	pop	es
	mov	cx,2BH				;num bytes to move
	mov	si,bx
	add	di,PCDosServ.Xfer	
	cld
	rep	movsb
	
	pop	ds
	pop	di

ANSWER:


	push	es
	push	ds
	
	mov	ah,2fH
	int	DOS

	push	es
	pop	ds


;	lea	dx,PCDosServ.Xfer[DI]

;;	mov	ah,DOSPrintStrg
;;	mov	dx,bx
;;	mov	[bx+42],BYTE PTR TERMINATOR
;;	int	DOS				;print name

	pop	ds
	pop 	es


ANSWER1:

	CALLSERVICE SDATAOFF 

	JMP	gotserv				;done

READD:
	MOV	AX,READDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
;	PUSH	DS
;	PUSH	ES
;	POP	DS
	
	MOV	BX,ES:PCDosServ.f_filehandle[DI]	
	MOV	CX,ES:PCDosServ.f_access[DI]	;number of bytes to read
;	LEA	DX,ES:PCDosServ.f_name1[DI]		;buffer address
	MOV	DX,OFFSET TMPBUF
	MOV	AH,READ_FILE			;function = read  file
	INT	DOS	
	JC	RD_ERR				;read failed

	MOV	ES:PCDosServ.f_access[DI],AX	;return bytes read

	push 	di
	mov	cx,ax				;num bytes to move
	mov	si,dx
	add	di,PCDosServ.f_name	
	cld
	rep	movsb
	

	pop	di




	JMP	RD_END

RD_ERR:						
	MOV	ES:PCDosServ.f_error[DI],AX	;if error, return code
RD_END:
;	POP	DS
	JMP	ANSWER1
WRIT:
	MOV	AX,WRITDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	
	MOV	BX,ES:PCDosServ.f_filehandle[DI]	
	MOV	AX,ES:PCDosServ.f_access[DI]	;number of bytes to write
	MOV	DX,OFFSET TMPBUF


	push 	di
	push	ds
	push	es
	push	ds
	push	es
	pop	ds
	pop	es
	
	mov	si,di
	mov	cx,ax
	mov	di,dx
	add	si,PCDosServ.f_name	
	cld
	rep	movsb
	pop	es
	pop	ds
	pop	di

	MOV	CX,AX				;bytes to write
	MOV	AH,WRIT_FILE			;function = write  file
	INT	DOS	
	JC	WR_ERR				;write failed

	MOV	ES:PCDosServ.f_access[DI],AX	;return bytes written

	JMP	WR_END

WR_ERR:						
	MOV	ES:PCDosServ.f_error[DI],AX	;if error, return code
WR_END:
	JMP	ANSWER1

;
; Seek to Beginning of file.  
;	
;	This serves as a file close if we still have a LOCK on the file.
;	This prevents a file  disappearing while its still LOCKed.
 

PRECLOS:
	MOV	AX,PCLODONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	MOV	DX,0				;seek offset lo
	MOV	CX,0				;seek offset hi
	MOV	AL,1				;seek from current
	MOV	AH,SEEK
	MOV	BX,ES:PCDosServ.f_filehandle[DI]	
	INT	DOS				;get current location
	JMP	ANSWER1

CLOZE:
	MOV	AH,CLSE
	MOV	BX,ES:PCDosServ.f_filehandle[DI]	
	INT	DOS				;close filehandle
	JMP	ANSWER1

	
ASEEK:
;
; Find current position
; 
	MOV	AX,SEEKDONE
	MOV	ES:PCDosServ.f_function[DI],AX	;Tell Amiga we're done
	MOV	DX,0				;seek offset lo
	MOV	CX,0				;seek offset hi
	MOV	AL,1				;seek from current
	MOV	AH,SEEK
	MOV	BX,ES:PCDosServ.f_filehandle[DI]	
	INT	DOS				;get current location
	JC	SK_ERR				;seek error
	MOV	BX,DX				;hold for a sec
	MOV	DX,ES:PCDosServ.f_offset[DI]	;get seek offset lo
	MOV	CX,ES:PCDosServ.f_offhi[DI]	;get seek offset hi
	MOV	ES:PCDosServ.f_offhi[DI],BX	;return current position hi
	MOV	ES:PCDosServ.f_offset[DI],AX	;return current position lo
;
; Seek
;
	MOV	AX,ES:PCDosServ.f_access[DI]	;get mode
	MOV	AH,SEEK
	MOV	BX,ES:PCDosServ.f_filehandle[DI]	
	INT	DOS				;get current location
	JC	SK_ERR				;seek error
	JMP	ANSWER1


SK_ERR:						
	MOV	ES:PCDosServ.f_error[DI],AX	
	JMP	ANSWER1
		






	mov	cx,es:TimeServReq.tsr_Year[di]
	mov	dh,es:TimeServReq.tsr_Month[di]
	mov	dl,es:TimeServReq.tsr_Day[di]


	mov	ah,2dh		   		;Set time
	mov	ch,es:TimeServReq.tsr_Hour[di]
	mov	cl,es:TimeServReq.tsr_Minutes[di]
	mov	dh,es:TimeServReq.tsr_Seconds[di]
	mov	dl,0


	mov	ah,09h		      		;Print Date and Time
	push	ds
	push	es
	pop	ds
	lea	dx,es:TimeServReq.tsr_String[di]
	pop	ds



	RELEASESERVICE SDATAOFF

	RET
START	ENDP

;
; PC File functions 
;

GETNSTRING	PROC
	PUSH	DS
	PUSH	ES
	PUSH	DS
	PUSH	ES
	POP	DS
	POP	ES
	PUSH 	di
	mov	cx,NAMESIZE				;num bytes to move
	mov	si,dx					;source
	MOV	DI,OFFSET TMPBUF			;destination
	cld
	rep	movsb
	POP	DI
	POP	ES
	POP	DS
	ret
GETNSTRING	ENDP



Handler	proc	far				;Routine for handling function
	push	ax				; completion. After a call
	push	ds				; service this routine will  
	push	dx				; be called to indicate the 
	mov	ax,DSEG				; requested funtion has 
	mov	ds,ax				; completed.
	mov  	ds:FLAG,0ffffh	    		;Set our wait flag to wake up
	pop	dx				;the main program
	pop	ds
	pop	ax
	ret
Handler	endp
CSEG	ENDS
	END	start

