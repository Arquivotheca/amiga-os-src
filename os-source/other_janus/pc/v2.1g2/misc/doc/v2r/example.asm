;******************************************************************************
;
; Test Service		Steps thru the main functions of services.
; ------------		This is the brief example for using all  
;			Janus Services from PC's assembly language level.
;
; Copyright (c) 1988,   Commodore Amiga Inc.,  All rights reserved.
;
; HISTORY
; Date       name               Description
; ---------  -----------------  -------------------------------------------
; 07-26-88   Torsten Burgdorff  Created this file
; 07-28-88   Torsten Burgdorff  Cleanup for documentation
;
;******************************************************************************

include janus\services.inc 	; get this include files from 
include janus\servmac.inc	; Beta 1.0 Release disk
include janus\memory.inc

;******************************************************************************
;
;	Magic Constants:

DOS		=	21h
DOSPrintStrg	=	9

KEYB		=	16h
GetKey 		=	0
GetKeybStatus	=	1

reset		=	0

;******************************************************************************
;
;	Macro:

DOSPrt	macro	string
	push	ax
	push	dx
	mov	dx,offset string
	mov	ah,DOSPrintStrg
	int	DOS
	pop	dx
	pop	ax
	endm
	
WaitKey macro	string
	local	WK
	push	ax
	DOSPrt	KeyString
	DOSPrt	string
	mov	ah,GetKey
	int	KEYB
	cmp	ah,1
	pop	ax
	jne	WK
	ret
WK:
	endm

;******************************************************************************
;
;	Service ID's:

APP_ID_H 	=	0
APP_ID_L 	=	101

;******************************************************************************

SSEG	SEGMENT PARA STACK 'STACK'
    	db	12	DUP('STACKSEG')
SSEG	ENDS

;******************************************************************************

DSEG 	SEGMENT PARA 'DATA'

StartMsg	db	' Services Test Side  Demo 0.2 ',13,10,'$'
KeyString	db	13,10,' Hit any (ESC to quit) key to','$'

Add3M		db	' Add Service 101.3 ',13,10,'$'
Get3M		db	' Get Service 101.3 ',13,10,'$'
Call3M		db	' Call Service 101.3 ',13,10,'$'
Release3M 	db	' Release Service 101.3 ',13,10,'$'
Delete3M	db	' Delete Service 101.3 ',13,10,'$'
Lock3M		db	' Lock Service Data 101.3 ',13,10,'$'
Unlock3M	db	' Unlock Service Data 101.3 ',13,10,'$'

ErrorMsg	db	' Error during last operation !',13,10,'$'

SDATASEG	dw	0
SDATAOFF	dw	0
UserMemOff	dw	0

FLAG		dw	0

DSEG 	ENDS


; *** This is our main program *** *** *** -------------------------------

CSEG	SEGMENT PARA  'CODE'


START	PROC	FAR
	ASSUME	CS:CSEG,DS:DSEG,SS:SSEG,ES:NOTHING
	PUSH	DS	     	    	; prepare DOS return address
	SUB	AX,AX
	PUSH	AX
	MOV	AX,DSEG
	MOV	DS,AX			; setup data seg

	DOSPrt	StartMsg		; prints header

	GetBase JSERV_PCSERVICE		; wakeup services 


	; we are going to add a service	now ---------------------------------

	WaitKey Add3M			
	AddService App_ID_H,App_ID_L,3,123,MEMB_BUFFER,0,cs,dummy
	or	al,al
	jz	FoundService		; forget everything, if we run in an
	DOSPrt	ErrorMsg		;  error here
	jmp	NoService

FoundService:
	mov	SDataSeg,es		; save results
	mov	SDataOff,di
	mov	ax,es:ServiceData.sd_MemOffset[di]
	mov	UserMemOff,ax


	; play with the ServiceData semaphore -------------------------------

	WaitKey Lock3M			
	LockServiceData SDataOff	;  lock semaphore
		      
	WaitKey Unlock3M
	UnlockServiceData SDataOff	;  and unlock it again

					
	; link to just opened Service ---------------------------------------

	WaitKey Get3M			
	GetService App_ID_H,App_ID_L,3,0,cs,dummy
	or	al,al			
	jz	GotService		; forget this, if we run in an error
	DOSPrt	ErrorMsg
	jmp	GotNoService

GotService:
	mov	SDataSeg,es		; save results
	mov	SDataOff,di
	mov	ax,es:ServiceData.sd_MemOffset[di]
	mov	UserMemOff,ax
	

	; call this Service, means call our own Service Handler -------------

	WaitKey Call3M
	mov	FLAG,reset 	       
	CallService SDataOff		; trigger service now
	call	Wait_Action		; and wait for the answer


	; cleanup now -------------------------------------------------------

	WaitKey Release3M		; unlink from service
	ReleaseService SDataOff
		
GotNoService:
	WaitKey Delete3M		; and delete it finally
	DeleteService SDataOff

NoService:
	RET 				; here we go, simple, eh!


START	ENDP

; *** Main program ends here *** *** *** ---------------------------------


; ------------------------------------------------------------------------

Dummy	proc	far 		; Service Handler
				;  increments a simple counter
	push	ax		;  and returns
	push	ds
	mov	ax,DSEG
	mov	ds,ax
	inc	FLAG		; count hits
	pop	ds
	pop	ax
	ret

Dummy	endp

	
; ------------------------------------------------------------------------

Wait_Action proc	near		; waits until our service handler is called
				
WaitF:	cmp	FLAG,reset	; simple, but good for this example 
 	je	WaitF		
	ret

Wait_Action endp	     	

	
; ------------------------------------------------------------------------

CSEG	ENDS
	END	start

