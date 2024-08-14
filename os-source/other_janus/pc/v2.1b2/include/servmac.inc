;*****************************************************************************
; (PC side file)
;
; SERViceMACros.inc	is a set of macro definitions, which allow 
;			an easy integration of Janus Services in 
;			PC's assembler language programs.	       
;
; Copyright (c) 1988,   Commodore Amiga Inc.,  All rights reserved.
;
; HISTORY
; Date       name               Description
; ---------  -----------------  -------------------------------------------
; 07-16-88   Torsten Burgdorff  Created this file
;
;*****************************************************************************

;NOTE: Needs Services.inc


GETBASE	macro	Service

	mov	ah,JFUNC_GETBASE
	mov	al,Service
	int	JFUNC_JINT

	endm	 		; effects AX, ES, DI, DX	 


ALLOCMEM macro	Size,Type

	mov	ah,JFUNC_ALLOCMEM
	mov	al,Type
	mov	bx,Size
	int	JFUNC_JINT
				
	endm			; effects AX, BX


FREEMEM	macro	Offset,Type

	push	bx
	mov	ah,JFUNC_FREEMEM
	mov	al,Type
	mov	bx,Offset
	int	JFUNC_JINT
	pop	bx

	endm  			; effects AX


SETPARAM macro	Service,Offset

	push	bx
	mov	ah,JFUNC_SETPARAM
	mov	al,Service
	mov	bx,Offset
	int	JFUNC_JINT
	pop	bx

	endm			; effects AX


CALLAMIGA macro	Service

	mov	ah,JFUNC_CALLAMIGA
	mov	al,Service
	int	JFUNC_JINT

	endm			; effects AX


WAITAMIGA macro	Service

	mov	ah,JFUNC_WAITAMIGA
	mov	al,Service
	int	JFUNC_JINT

	endm		   	; effects AX


CHECKAMIGA macro Service

	mov	ah,JFUNC_CHECKAMIGA
	mov	al,Service
	int	JFUNC_JINT

	endm	       		; effects AX


ADDSERVICE macro AppID_high,AppID_low,LocalID,Size,Type,Flags,HandlerSeg,HandlerOff

	push	bx
	push	cx
	push	dx
	push	si
	push	ds
	mov	ax,AppID_high
	push	ax
	mov	si,appID_low
	mov	cx,LocalID
	mov	bx,Size
	mov	dx,Type	
	mov	ax,HandlerSeg
	mov	es,ax
	mov	di,offset HandlerSeg:HandlerOff
	mov	ah,JFUNC_ADDSERVICE
	mov	al,Flags
	pop	ds
	int	JFUNC_JINT
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	bx	

	endm	   		; effects AX, ES, DI


GETSERVICE macro AppID_high,AppID_low,LocalID,Flags,HandlerSeg,HandlerOff

	push	cx
	push	si
	push	ds
	mov	ax,AppID_high
	push	ax
	mov	si,appID_low
	mov	cx,LocalID
	mov	ax,HandlerSeg
	mov	es,ax
	mov	di,offset Handlerseg:HandlerOff
	mov	ah,JFUNC_GETSERVICE
	mov	al,Flags
	pop	ds
	int	JFUNC_JINT
	pop	ds
	pop	si
	pop	cx

	endm			; effects AX, ES, DI


CALLSERVICE macro ServiceData

	push	di
	mov	di,ServiceData
	mov	ah,JFUNC_CALLSERVICE
	int	JFUNC_JINT
	pop	di

	endm			; effects AX


RELEASESERVICE macro ServiceData

	push	di
	mov	di,ServiceData
	mov	ah,JFUNC_RELEASESERVICE
	int	JFUNC_JINT
	pop	di

	endm			; effects AX


DELETESERVICE macro ServiceData

	push	di
	mov	di,ServiceData
	mov	ah,JFUNC_DELETESERVICE
	int	JFUNC_JINT
	pop	di

	endm			; effects AX


LOCKSERVICEDATA	macro ServiceData

	push	di
	mov	di,ServiceData
	mov	ah,JFUNC_LOCKSERVICEDATA
	int	JFUNC_JINT
	pop	di

	endm			; effects AX


UNLOCKSERVICEDATA macro	ServiceData

	push	di
	mov	di,ServiceData
	mov	ah,JFUNC_UNLOCKSERVICEDATA
	int	JFUNC_JINT
	pop	di

	endm			; effects AX


