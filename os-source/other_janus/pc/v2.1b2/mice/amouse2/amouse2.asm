	TITLE   AMouseWindowsDriver

	include cmacros.inc

	include janus\janusvar.inc
	include janus\services.inc
	include	..\amouse\mouseser.inc

assumes CS,CODE
assumes DS,DATA

sBegin   CODE

?WIN=0

ms_sd_np	dw	0,0
ms_rq_np	dw	0,0
DPCopyArea	dw	DPSize dup(0)
TimerOffset	dw	0
TimerSegment	dw	0
IntCallAddr	dw	0
IntCallSeg	dw	0
DataSegment	dw	0
old_count	db	0


;--------------------------Local-Routine-----------------------------;
; mouse_lock
;
; lock the mouse's shared memory area
;
; Entry:
;	None
; Returns:
;	None
; Error Returns:
;	None
; Registers Preserved:
;	ES,SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,FLAGS
; Calls:
;	None
; History:
;	Fri 21-Aug-1987 11:43:42 
;	Initial version
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Code
	assumes ds,Data


	page
;--------------------------Local-Routine-----------------------------;
; get_service
;
; get the amiga service assuming real mode
;
; Entry:
;	None
; Returns:
;	None
; Error Returns:
;	None
; Registers Preserved:
;	ES,SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,FLAGS
; Calls:
;	None
; History:
;	Fri 21-Aug-1987 11:43:42 
;	Initial version
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Code
	assumes ds,Data

get_service	PROC	NEAR

ms_app_id	equ	MOUSESERV_APPLICATION_ID
ms_local_id	equ	MOUSESERV_LOCAL_ID
ms_flags	equ	GETS_WAIT+GETS_ALOAD_A

	push	ds
	mov	ax,(ms_app_id SHR 16) AND 0FFFFH	;app id high
	mov	ds,ax
	mov	si,ms_app_id AND 0FFFFH	;app id low
	mov	cx,ms_local_id		;local id
	xor	ax,ax			;handler seg
	mov	es,ax
	mov	di,ax			;handler offs
	mov	ah,JFUNC_GETSERVICE	;get a service
	mov	al,ms_flags		;flags
	int	JFUNC_JINT
	pop	ds
	cmp	al,JSERV_OK
	jnz	getr_failed		;(can't happen if GETS_WAIT)

	;es:di = mouseserv ServiceData pointer

	;lock and unlock the servicedata structure (waits until fully initted)
	mov	AH,JFUNC_LOCKSERVICEDATA
;	mov	DI,word ptr ms_sd_np
	int	JFUNC_JINT
	mov	AH,JFUNC_UNLOCKSERVICEDATA
;	mov	DI,word ptr ms_sd_np
	int	JFUNC_JINT

	mov	word ptr ms_sd_np,di	;save ptr to ServiceData struct
	mov	word ptr ms_sd_np+2,es
	
	mov	ax,word ptr es:sd_PCMemPtr[di]	;save ptr to MouseServReq struct
	mov	word ptr ms_rq_np,ax
	mov	ax,word ptr es:sd_PCMemPtr+2[di]
	mov	word ptr ms_rq_np+2,ax

	xor	ax,ax			;return 0 for success
	jmp	getr_done

getr_failed:
	mov	ax,1			;return 1 for failure

getr_done:
	ret				;return 0 for success.

get_service	ENDP


	page
;--------------------------Local-Routine-----------------------------;
; free_service
;
; release the amiga mouse service
;
; Entry:
;	None
; Returns:
;	None
; Error Returns:
;	None
; Registers Preserved:
;	ES,SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,FLAGS
; Calls:
;	None
; History:
;	Fri 21-Aug-1987 11:43:42 
;	Initial version
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Code
	assumes ds,Data

free_service	PROC	NEAR

	mov	ax,word ptr ms_sd_np
	or	ax,ax
	jz	free_done

	mov	di,ax
	mov	ah,JFUNC_RELEASESERVICE
	int	JFUNC_JINT		;do the int

free_done:
	ret				;return 0 for success.

free_service	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; tickle_janus
;
; try to verify that INT 0BH is patched in to janus stuff by trying
; to do a GetBase function.
;
; Entry:
;	None
; Returns:
;	None
; Error Returns:
;	None
; Registers Preserved:
;	ES,SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,FLAGS
; Calls:
;	None
; History:
;	Fri 21-Aug-1987 11:43:42 
;	Initial version
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Code
	assumes ds,Data

tickle_janus	PROC	NEAR

	mov	cx,3			;knock three times.

tickle_loop:
	push	cx
	
	mov	ah,JFUNC_GETBASE	;try to get the base of pcdisk
	mov	al,JSERV_AMIGASERVICE
	int	JFUNC_JINT		;real mode int 0b

	pop	cx
	loop	tickle_loop

	cmp	al,JSERV_OK
	jz	tickle_ok

	xor	ax,ax			;failed.
	jmp	tickle_done

tickle_ok:
	mov	ax,1			;succeeded.

tickle_done:
	ret

tickle_janus	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; MouseRead
;
; Information regarding the mouse is returned to the caller.
;
; Entry:
;	None
; Returns:
;	AX = status
;	BX = delta X
;	CX = delta Y
; Error Returns:
;	None
; Registers Preserved:
;	ES,SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,FLAGS
; Calls:
;	None
; History:
;	Fri 21-Aug-1987 11:43:42 
;	Initial version
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Code
	assumes ds,Data

MouseRead	PROC FAR

	push	di
	push	es

	les	di,dword ptr ms_rq_np	;find out if the counter changed
	mov	al,es:ChangeCount[di]
	cmp	al,old_count		;did it?
	jnz	change			;yes.

	xor	ax,ax			;no, indicate no change
	jmp	MRNoMove		;get out

change:
	mov	old_count,al		;save new counter value

	; read out of D.P.RAM into local area
;	les	di,dword ptr ms_rq_np
	xor	bx,bx

;	move data up to but excluding button status (not used by Windows)
;	into local copy area with lock on, release lock and then examine
;	data.
				
	mov	cx,(AmigaPCStatus/2)	; convert to words
MoveLoop:
	xor	ax,ax			; zero
	xchg	ax,es:0[di+bx]		; destructive read from D.P.RAM
	mov	DPCopyArea[bx],ax	; store in local area
	inc	bx
	inc	bx
	loop	MoveLoop

;	les	di,dword ptr ms_rq_np	;find out if the counter changed
	mov	al,es:ChangeCount[di]
	cmp	al,old_count		;did it?
	jnz	change			;yes.

	lea	di,DPCopyArea	;point to copy area
	xor	ax,ax		; initialise to no change

				; get X change
	mov	bx,AmigaPCX[di]
	or	bx,bx
	jz	GetY		; no change in X co-ord.
	sar	bx,1
	or	ax,1		; cursor position change
GetY:
				; get Y change
	mov	cx,AmigaPCY[di]
	or	cx,cx
	jz	GetButtons	; no change in Y co-ord.
	sar	cx,1
	or	ax,1		; cursor position change
GetButtons:
	test	word ptr AmigaPCLeftP[di],-1
	jz	GetLRel
	or	ax,2		; Left pressed
GetLRel:
	test	word ptr AmigaPCLeftR[di],-1
	jz	GetRPrs
	or	ax,4		; Left released
GetRPrs:
	test	word ptr AmigaPCRightP[di],-1
	jz	GetRRel
	or	ax,8		; Right pressed
GetRRel:
	test	word ptr AmigaPCRightR[di],-1
	jz	GetStatus
	or	ax,16		; Right released
GetStatus:

MRNoMove:
	pop	es
	pop	di
	ret

MouseRead	ENDP


InTimer		db	0

cProc	MouseInt, NEAR

cBegin	MouseInt

	test	cs:InTimer,-1
	jnz	NextTimer		; still running since last int.

	inc	cs:InTimer
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es
	push	cs
	pop	ds

	call	MouseRead		; read Mouse info from DPRam
	or	ax,ax
	jz	NoChange
	mov	dx,cs:DataSegment
	mov	ds,dx
	mov	dx,ds:[2]
	call	dword ptr cs:IntCallAddr
NoChange:
	
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	dec	cs:InTimer
NextTimer:

cEnd	MouseInt

cProc	Timer, FAR
cBegin	Timer

	pushf
	call	cs:dword ptr TimerOffset
	call	MouseInt
	iret

cEnd	Timer

cProc	Init, NEAR
cBegin	Init

	call	tickle_janus		;tickle janus
	or	ax,ax
	jnz	tickled			;it laughed.

	mov	ax,1			;janus is NOT amused.  fail.
	jmp	NoService

tickled:
	call	get_service		;find dpram & param ram ptrs

	mov	ax,351cH
	int	21H			; read current Timer interrupt handler
	mov	TimerSegment,es		; save
	mov	TimerOffset,bx
	mov	dx,OFFSET Timer
	push	cs
	pop	ds
	mov	ax,251cH		; set new Timer interrupt handler
	int	21H
NoService:

cEnd	Init

Print		PROC	NEAR
	pushf
	sti
	push	ax
	mov	ah,5
	int	21H
	pop	ax
	popf
	ret
Print		ENDP

?WIN=1

cProc	Inquire	, <PUBLIC,FAR,di,si>
	parmD	Area
cBegin	Inquire

	les	di,Area
	mov	si,0
	mov	ax,14
	mov	cx,ax
  repz	movsb
	sub	ax,cx

cEnd	Inquire



cProc	Enable, <PUBLIC,FAR>
	parmD	IntAddr
cBegin	Enable


	push	ds
	push	cs
	pop	ds
	mov	DataSegment,ax
	les	bx,IntAddr		; windows interrupt routine
	mov	ax,IntCallAddr		;for test.
	or	ax,IntCallSeg
	mov	IntCallAddr,bx
	mov	IntCallSeg,es

	jnz	initted_already
	call	Init

initted_already:
	xor	ax,ax			; return value ?

cEnd	Enable

cProc	Disable, <PUBLIC, FAR>
cBegin	Disable

	mov	ax,cs:TimerSegment
	mov	dx,cs:TimerOffset
	mov	ds,ax
	or	ax,dx
	jz	NotEn
	mov	ax,251cH		; set new Timer interrupt handler
	int	21H
	xor	ax,ax			; zero
	mov	cs:TimerSegment,ax
	mov	cs:TimerOffset,ax
	call	free_service
NotEn:

cEnd	Disable

cProc	GetVector, <PUBLIC, FAR>
cBegin	GetVector

	mov	ax,-1

cEnd	GetVector

sEnd

sBegin	DATA


	db	001h, 000h, 003h, 000h, 000h, 000h, 002h, 000h
	db	002h, 000h, 000h, 000h, 000h, 000h, 000h, 000h   
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 0FFh, 0FFh, 0F8h, 002h, 00Bh
	db	0F7h, 002h, 000h, 0F8h, 003h, 00Ch, 0EFh, 000h 
	db	000h, 000h, 030h, 06fh, 0ffh	 

sEnd
	END
