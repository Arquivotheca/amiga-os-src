       page    ,132
;-----------------------------Module-Header-----------------------------;
; Module Name:	MOUSE.ASM
;
; Windows mouse driver data and initialization routines.
;
; rsd	10-25-90	merging of microsoft win3 & c-a dos mouse drivers
;
; Copyright (c) 1987, 1988, 1989  Microsoft Corporation
; Copyright (c) 1987, 1988, 1989  Commodore-Amiga
;
; Exported Functions:
;
; Public Functions:
;
; Public Data:
;
; General Description:
;
;   This segment contains all static data used by the mouse routines.  The
;   hardware interrupt routine is included in the static data so that it
;   has addressability to the static data through the CS register.  Thus
;   the data segment of this module MUST be fixed in memory, while the
;   code segment can be moveable and/or discardable.
;
;-----------------------------------------------------------------------;

	title	Mouse - Main Mouse Module

	.xlist
	include cmacros.inc
	include windefs.inc
	include windows.inc

	include amouse3.inc
	include janus\janusvar.inc
	include janus\services.inc
	include ..\amouse\mouseser.inc

	.list

TESTING	EQU	0
PRINT	EQU	0

	IF PRINT

DEB	MACRO	addr,data
	push	di
	push	cx
	mov	di,addr
	mov	cl,data
	call	debout
	pop	cx
	pop	di
ENDM

DEBW	MACRO	addr,data
	push	di
	push	cx
	mov	cx,data
	push	cx
	mov	di,addr+1
	call	debout
	pop	cx
	mov	cl,ch
	mov	di,addr
	call	debout
	pop	cx
	pop	di
ENDM

DEBSET	MACRO	addr,len
	push	di
	push	cx
	mov	di,addr
	mov	deb_addr,di
	mov	cx,len
	call	debwipe
	pop	cx
	pop	di
ENDM

DEBI	MACRO	data
	push	di
	push	cx
	mov	di,deb_addr
	inc	deb_addr
	mov	cl,data
	call	debout
	pop	cx
	pop	di
ENDM
	ELSE

DEB	MACRO	addr,data
ENDM

DEBSET	MACRO	addr
ENDM

DEBI	MACRO	data
ENDM
	ENDIF

	externFP CreateSystemTimer	;these are ms private...
	externFP KillSystemTimer	;...we have them cuz we need 'em.

	externFP AllocDStoCSAlias	;make a copy of a DS selector which
					;will have CS access rights
	externFP FreeSelector		;destroy a selector
	externFP AllocSelector		;duplicate a selector
	externFP SetSelectorBase	;change the base of a selector
	externFP SetSelectorLimit	;change the limit of a selector

	externFP GetWinFlags		;get windows env flags
;	externFP DOS3Call		;int 21h substitute

	externW	__0000h
	externW	__A000h
	externW	__B000h
	externW	__B800h
	externW	__C000h
	externW	__D000h
	externW	__E000h
	externW	__F000h

;OS2	equ	10			;Version number for OS/2

sBegin	Data

stupid		db	16 dup (0)

	even				;Want words on word boundary

;--- double

globalD 	event_proc,0		;Mouse event procedure when enabled
globalD		ms_sd_np,0		;mouse ServiceData native mode pointer
globalD		ms_sd_rp,0		;mouse ServiceData real mode pointer
globalD		ms_rq_np,0		;mouse MouseServReq native mode pointer
globalD		ms_rq_rp,0		;mouse MouseServReq real mode pointer

;--- word

DPCopyArea	dw	DPSize dup(0)	;copy of mouse related stuff
globalW		timer_handle,0		;win CreateSystemTimer() handle
globalW		interrupt_rate,19	;Maximum interrupt rate of mouse
globalW		DSEG_device_int,0	;data segment used by int handler
globalW		CSEG_device_int,0	;code segment used by int handler
globalW		device_int_sel,0	;manufactured selector for CS
globalW		ms_sd_sel,0		;mouse ServiceData native selector
globalW		ms_rq_sel,0		;mouse MouseServReq native selector
globalW		mode,0			;86/286/386

IF PRINT
globalW		deb_addr,0
ENDIF

;--- byte

globalB 	vector,-1		;Vector # of mouse interrupt
globalB 	mouse_flags,0		;Various flags
globalB 	mouse_type,0		;Type of mouse (inport/bus/serial/etc.)
globalB		InTimer,0		;reentrancy flag for mouse "int"
globalB		old_count,0
globalB		open_flags,0

IF PRINT
globalB		testcount,0
globalB		oa_count,0
globalB		ow_count,0
ENDIF


;masks for open_flags
OF_AMIGA	EQU	1
OF_WINDOWS	EQU	2

;params for GetService() to get MouseServ
ms_app_id	equ	MOUSESERV_APPLICATION_ID
ms_local_id	equ	MOUSESERV_LOCAL_ID
ms_flags	equ	GETS_WAIT+GETS_ALOAD_A


;DPMI "simulate real interrupt" stuff
	even
r_first	equ	this byte
r_edi	dd	0		;EDI		00
r_esi	dd	0		;ESI		04
r_ebp	dd	0		;EBP		08
r_rsvd	dd	0		;reserved	0C
r_ebx	dd	0		;EBX		10
r_edx	dd	0		;EDX		14
r_ecx	dd	0		;ECX		18
r_eax	dd	0		;EAX		1C
r_flag	dw	0		;flags		20
r_es	dw	0		;ES		22
r_ds	dw	0		;DS		24
r_fs	dw	0		;FS		26
r_gs	dw	0		;GS		28
r_ip	dw	0		;IP		2A
r_cs	dw	0		;CS		2C
r_sp	dw	0		;SP		2E
r_ss	dw	0		;SS		30
;DPMI "simulate real interrupt" stuff

page
;--------------------------Interrupt-Routine----------------------------;
;
; device_int - Mouse Specific Interrupt Handler
;
; The mouse specific interrupt code will follow.  It will be copied
; into the reserved area as initialization time, and executed from
; here.
;
; Entry:
;	DS = Data
;	CS = Data
; Returns:
;	calls event_proc if there is one and something changed with:
;		AX = status
;		BX = delta X
;		CX = delta Y
;		DX = number of mouse buttons
; Error Returns:
;	None
; Registers Preserved:
;	All
; Registers Destroyed:
;	None
; Calls:
;	MouseRead
;	indirect through event_proc
; History:
;	Fri 21-Aug-1987 11:43:42 
;	Initial version
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Data
	assumes ds,Data
	assumes es,nothing
	assumes ss,nothing

	even

	public	device_int

device_int	proc	far

	push	ds			;save DS

	push	cs:DSEG_device_int	;pfm.
	pop	ds

	test	InTimer,-1		;are we violating ourselves?
	jnz	NextTimer		; yes.  still running since last int.

	inc	InTimer			;we're here!

	push	ax			;save regs
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	es

	call	FAR PTR MouseRead	; read Mouse info from DPRam
	or	ax,ax
	jz	NoChange

	mov	dx,NUMBER_BUTTONS	;Return number of buttons
	call	dword ptr event_proc	;call user routine
NoChange:
	
	pop	es
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax

	dec	InTimer
NextTimer:
	pop	ds

	ret

device_int	endp

sEnd	Data

page

sBegin	Code
assumes cs,Code

;----------------------------------------------------------------------------
IF PRINT
assumes cs,Code
assumes ds,Data

debout	proc	far
	push	es
	push	ax

	cmp	mode,86
	jz	debreal
	mov	ax,offset __D000h
	jmp	debit
debreal:
	mov	ax,0D000h
debit:
	mov	es,ax
	pop	ax
	add	di,3900h
	mov	byte ptr es:[di],cl
	pop	es

	ret
debout	endp

debwipe	proc	far
	push	es
	push	ax

	cmp	mode,86
	jz	debwreal
	mov	ax,offset __D000h
	jmp	debwit
debwreal:
	mov	ax,0D000h
debwit:
	mov	es,ax
	pop	ax
	add	di,3900h
debwl:
	mov	byte ptr es:[di],0
	inc	di
	loop	debwl
	pop	es

	ret
debwipe	endp
ENDIF
;----------------------------------------------------------------------------

	page
;--------------------------Local-Routine-----------------------------;
; sim_int0b
;
; do simulated INT 0BH
;
; Entry:
;	None (well, *you* must fill in the r_??? stuff before calling)
; Returns:
;	None (examine r_??? stuff)
; Error Returns:
;	None
; Registers Preserved:
;	DS, CS, IP, SS, SP :)
; Registers Destroyed:
;	All others
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

sim_int0b	PROC NEAR

	mov	ax,300h			;DPMI real mode int simulator
	mov	bx,1*256+JFUNC_JINT	;put A20 normal, do int 0bh
;	mov	bx,0*256+53h		;don't touch A20, do int 0bh
			;(don't ask me - 53h = 0bh in the windows universe.)
	push	ds			;es = seg of r_??? stuff
	pop	es
	mov	di,offset r_first	;first of the list
	mov	cx,0			;copy nothing to real mode stack
	int	31h			;zoom.
	ret

sim_int0b	ENDP

	page
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

mouse_lock	PROC NEAR

	push	es
	push	di

	cmp	mode,86			;real mode?
	jnz	lck_prot		;no, do prot.

;real mode
	mov	ah,JFUNC_LOCK		;try to lock
	les	di,dword ptr ms_rq_np	;point to param area
	lea	di,WriteLock[di]	;es:di = &(ms_rq_np->WriteLock)
	int	JFUNC_JINT		;do it (return val in al)

	jmp	lck_done

lck_prot:
	push	bx
	push	cx

	mov	ah,JFUNC_LOCK		;try to lock
	mov	word ptr r_eax,ax	;(pass in AH)
	mov	ax,word ptr ms_rq_rp+2	;point to param area
	mov	r_es,ax			;	(seg in ES)
	mov	ax,word ptr ms_rq_rp	;get offset to param area
	add	ax,WriteLock		;add WriteLock field index
	mov	word ptr r_edi,ax	;offs in DI
	call	sim_int0b		;poof.
	mov	ax,word ptr r_eax	;return value in AL

	pop	cx
	pop	bx

lck_done:
	pop	di
	pop	es

	ret

mouse_lock	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; mouse_unlock
;
; unlock the mouse's shared memory area
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

mouse_unlock	PROC NEAR
	push	es
	push	di

	cmp	mode,86			;real mode?
	jnz	unl_prot		;no, do prot.

;real mode
	mov	ah,JFUNC_UNLOCK		;unlock
	les	di,dword ptr ms_rq_np	;point to param area
	lea	di,WriteLock[di]	;es:di = &(ParamArea->WriteLock)
	int	JFUNC_JINT		;do it

	jmp	unl_done

unl_prot:
	push	bx
	push	cx

	mov	ah,JFUNC_UNLOCK		;unlock
	mov	word ptr r_eax,ax	;(pass in AH)
	mov	ax,word ptr ms_rq_rp+2	;point to param area
	mov	r_es,ax			;	(seg in ES)
	mov	ax,word ptr ms_rq_rp	;get offset to param area
	add	ax,WriteLock		;add WriteLock field index
	mov	word ptr r_edi,ax	;offs in DI
	call	sim_int0b		;poof.

	pop	cx
	pop	bx

unl_done:
	pop	di
	pop	es

	ret

mouse_unlock	ENDP

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

	call	mouse_lock

	; read out of D.P.RAM into local area
	les	di,dword ptr ms_rq_np
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

	call	mouse_unlock

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

	page

;--------------------------Local-Routine-----------------------------;
determine_mode	PROC NEAR

	cCall	GetWinFlags
	test	ax,WF_ENHANCED
	jnz	win_386
	test	ax,WF_STANDARD
	jnz	win_286

	mov	mode,86
	jmp	win_done

win_286:
	mov	mode,286
	jmp	win_done

win_386:
	mov	mode,386

win_done:
	ret

determine_mode	ENDP

	page
;
; table of segments exported by windows for our convenience.
; we try to use one of these if possible when we need to make a
; selector:offset16 pair from a segment:offset16 pair, rather
; than actually creating a new descriptor.
;
import_seg_table:
	dw	00000h, offset __0000h
	dw	0a000h, offset __A000h
	dw	0b000h, offset __B000h
	dw	0b800h, offset __B800h
	dw	0c000h, offset __C000h
	dw	0d000h, offset __D000h
	dw	0e000h, offset __E000h
	dw	0f000h, offset __F000h
	dw	0ffffh, 0ffffh		;end marker

;--------------------------Local-Routine-----------------------------;
; make_addressable
;
; adjust DX:AX so that DX fits one of the normal X000 segments
;
; Entry:
;	DX:AX = seg:offset of real address to make a sel for
;	CX = length of area
; Returns:
;	DX = new selector
;	AX = new offset
; Error Returns:
;	None
; Registers Preserved:
;	ES,SI,DI,DS,BP,BX
; Registers Destroyed:
;	AX,CX,DX,FLAGS
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

make_addressable	PROC	NEAR

	push	si
	push	bx

	push	ax		;save offset

	mov	si,offset import_seg_table	;point to seg xlate table
	cld				;forward.
scan_xlate:
	lods	word ptr cs:[si]	;get real mode seg value
	mov	bx,ax		;into bx
	lods	word ptr cs:[si]	;get prot mode descriptor value
	cmp	bx,0ffffh	;was real seg FFFF? (end of table)
	jz	create_a_seg	;yes, try dirty method
	cmp	bx,dx		;got it?
	jnz	scan_xlate	;no, keep looking.

	mov	dx,ax		;prot descriptor into dx
	pop	ax		;restore offset

	stc			;non-deleteable (borrowed) selector.

	jmp	seg_done	;finito.

create_a_seg:
	push	dx		;save seg

	;... use windows stuff to create descriptors ...
	push	ds		;create a new DS type selector
	call	AllocSelector

	pop	dx		;pop seg

	push	ax		;save sel

	rol	dx,1		;segment ABCD -> BCDA
	rol	dx,1
	rol	dx,1
	rol	dx,1
	mov	ax,dx		;AX,DX = BCDA
	and	ax,0fh		;PHYSh = 000A
	and	dx,0fff0h	;PHYSl = BCD0

	pop	cx		;get sel
	push	cx		;save sel

	push	cx		;pass sel
	push	ax		;pass PHYSh
	push	dx		;pass PHYSl
	call	SetSelectorBase	;change base addr of selector

	pop	cx		;get sel
	push	cx		;save sel

	push	cx		;pass sel
	mov	ax,00000h	;LIMITh = 0000
	push	ax
	mov	ax,0ffffh	;LIMITl = FFFF
	push	ax
	call	SetSelectorLimit	;change limit of selector

	pop	dx		;return selector
	pop	ax		;return offset
	clc			;deleteable selector.

seg_done:
	pop	bx
	pop	si

	ret

make_addressable	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; get_service_real
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

get_service_real	PROC	NEAR

	xor	ax,ax			;no need for selectors.
	mov	ms_sd_sel,ax
	mov	ms_rq_sel,ax

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

	mov	word ptr ms_sd_np,di	;save ptr to ServiceData struct
	mov	word ptr ms_sd_rp,di
	mov	word ptr ms_sd_np+2,es
	mov	word ptr ms_sd_rp+2,es
	
	mov	ax,word ptr es:sd_PCMemPtr[di]	;save ptr to MouseServReq struct
	mov	word ptr ms_rq_np,ax
	mov	word ptr ms_rq_rp,ax
	mov	ax,word ptr es:sd_PCMemPtr+2[di]
	mov	word ptr ms_rq_np+2,ax
	mov	word ptr ms_rq_rp+2,ax

	xor	ax,ax			;return 0 for success
	jmp	getr_done

getr_failed:
	mov	ax,1			;return 1 for failure

getr_done:
	ret				;return 0 for success.

get_service_real	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; get_service_prot
;
; get the amiga service assuming protected mode
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

get_service_prot	PROC	NEAR

	xor	ax,ax			;no selectors yet.
	mov	ms_sd_sel,ax
	mov	ms_rq_sel,ax

	mov	r_ds,(ms_app_id SHR 16) AND 0FFFFH	;app id high
	mov	word ptr r_esi,ms_app_id AND 0FFFFH	;app id low
	mov	word ptr r_ecx,ms_local_id	;local id
	mov	r_es,0			;handler seg
	mov	word ptr r_edi,0	;handler offs
	mov	ah,JFUNC_GETSERVICE	;get a service
	mov	al,ms_flags		;flags
	mov	word ptr r_eax,ax
	call	sim_int0b

	;es:di = mouseserv ServiceData pointer
	mov	ax,word ptr r_eax
	cmp	al,JSERV_OK
	jnz	getp_failed		;(can't happen if GETS_WAIT)

	mov	dx,r_es			;get ServiceData real mode ptr
	mov	ax,word ptr r_edi
	mov	word ptr ms_sd_rp,ax	;save sd real mode ptr
	mov	word ptr ms_sd_rp+2,dx
	call	make_addressable
	mov	word ptr ms_sd_np,ax	;save native ptr to ServiceData struct
	mov	word ptr ms_sd_np+2,dx
	jc	no_del_sd_sel
	mov	word ptr ms_sd_sel,dx	;save selector if not borrowed
no_del_sd_sel:

	mov	es,dx			;use the native sd pointer
	mov	di,ax

	mov	ax,word ptr es:sd_PCMemPtr[di]	;get the real MouseServReq ptr
	mov	dx,word ptr es:sd_PCMemPtr+2[di]
	mov	word ptr ms_rq_rp,ax	;save sd real mode ptr
	mov	word ptr ms_rq_rp+2,dx
	call	make_addressable
	mov	word ptr ms_rq_np,ax	;save native ptr to MouseServReq struct
	mov	word ptr ms_rq_np+2,dx
	jc	no_del_rq_sel
	mov	word ptr ms_rq_sel,dx	;save selector if not borrowed
no_del_rq_sel:

	xor	ax,ax			;return 0 for success
	jmp	getp_done

getp_failed:
	mov	ax,1			;return 1 for failure

getp_done:
	ret				;return 0 for success.

get_service_prot	ENDP

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

	mov	ax,word ptr ms_sd_rp
IF PRINT
	DEBW	2,ax
ENDIF
	or	ax,ax
	jz	free_done

	mov	di,ax
	mov	ah,JFUNC_RELEASESERVICE

	cmp	mode,86
	jz	free_real

	mov	word ptr r_eax,ax	;pass ah value
	mov	word ptr r_edi,di	;pass offset of servicedata
	call	sim_int0b
	jmp	free_done

free_real:
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

	cmp	mode,86			;intel - ya gotta love 'em.
	jz	tickle_real

	mov	word ptr r_eax,ax
	call	sim_int0b
	mov	ax,word ptr r_eax

	jmp	tickle_next

tickle_real:
	int	JFUNC_JINT		;real mode int 0b

tickle_next:
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
; get_service
;
; try to locate mouse service.
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

	cmp	mode,86
	jz	get_real

	call	get_service_prot
	jmp	get_done

get_real:
	call	get_service_real

get_done:

IF PRINT
	DEBW	10h,<word ptr ms_sd_rp+2>
	DEBW	12h,<word ptr ms_sd_rp>

	DEBW	14h,<word ptr ms_sd_np+2>
	DEBW	16h,<word ptr ms_sd_np>

	DEBW	18h,<word ptr ms_sd_sel>

	DEBW	20h,<word ptr ms_rq_rp+2>
	DEBW	22h,<word ptr ms_rq_rp>

	DEBW	24h,<word ptr ms_rq_np+2>
	DEBW	26h,<word ptr ms_rq_np>

	DEBW	28h,<word ptr ms_rq_sel>
ENDIF
	ret

get_service	ENDP

	page
;---------------------------Private-Routine-----------------------------;
; tickle janus and find the mouse service.  AX=0=fail, <>0=ok.
;-----------------------------------------------------------------------;
open_amiga	PROC NEAR
	call	tickle_janus		;tickle janus
	or	ax,ax
	jz	oa_fail			;nobody's home.

	call	get_service		;find dpram & param ram ptrs

	or	ax,ax
	jz	okay

oa_fail:
	xor	ax,ax			;failure.
	jmp	init_done

okay:
IF PRINT
	inc	oa_count
	DEB	0,oa_count
ENDIF
	mov	ax,1			;Successful initialization

init_done:
	ret

open_amiga	ENDP

	page
;---------------------------Private-Routine-----------------------------;
; give take the DS and make a CS out of it
;-----------------------------------------------------------------------;
create_ds	PROC NEAR

	;now we convert DS to a CS for the interrupt routine
	push	si
	push	di
	push	ds

	cmp	mode,86
	jz	create_real

	push	ds
	call	AllocDStoCSAlias
	mov	CSEG_device_int,ax
	mov	device_int_sel,ax
	mov	ax,ds
	mov	DSEG_device_int,ax
	jmp	create_done

create_real:
	xor	ax,ax
	mov	device_int_sel,ax
	mov	ax,ds
	mov	CSEG_device_int,ax
	mov	DSEG_device_int,ax

create_done:
	pop	ds
	pop	di
	pop	si

	ret

create_ds	ENDP
	page
;---------------------------Private-Routine-----------------------------;
; do any special magic required to prepare for "enhanced" 386 mode
;-----------------------------------------------------------------------;
init_enh386_windows	PROC NEAR

;
;	If running under Windows/386 then tell the Win386 mouse driver what
;	type of mouse we found.
;
	push	ax
	push	bx
	push	di
	push	es

	xor	di, di
	mov	es, di
	mov	ax, 1684h		;Get device API entry point
	mov	bx, VMD_DEVICE_ID	;for the Virtual Mouse Device
	int	2Fh
	mov	ax, es
	or	ax, di			;Q: Does VMD have API entry point?
	jz	vmd_call_done		;   N: Done

	push	cs			;Return to here after call to Win386
	mov	bx, offset vmd_call_done;virtual mouse driver
	push	bx
	push	es			;Call this SEG:OFF by doing a far
	push	di			;return

	mov	ax, 100h		;Set mouse type & int VECTOR API call
	mov	bl, mouse_type
	mov	bh, vector

BogusFarRetProc PROC FAR
	ret				;"Return" to VMD's API entry point
BogusFarRetProc ENDP

vmd_call_done:
	call	create_ds

	pop	es
	pop	di
	pop	bx
	pop	ax

	ret

init_enh386_windows	ENDP

	page
;---------------------------Private-Routine-----------------------------;
; do any special magic required to prepare for real mode
;-----------------------------------------------------------------------;
init_real_windows	PROC NEAR

	call	create_ds
	ret

init_real_windows	ENDP

	page
;---------------------------Private-Routine-----------------------------;
; do any special magic required to prepare for protected mode
;-----------------------------------------------------------------------;
init_prot_windows	PROC NEAR

	call	create_ds
	ret

init_prot_windows	ENDP


;--------------------------Local-Routine-----------------------------;
open_windows	PROC NEAR

IF PRINT
	inc	ow_count
	DEB	1,ow_count
ENDIF
	cmp	mode,86
	jnz	not_86
	call	init_real_windows	;8088!  ugh!
	jmp	ow_done
not_86:

	cmp	mode,286
	jnz	not_286
	call	init_prot_windows	;80286!  double ugh!
	jmp	ow_done

not_286:
	call	init_enh386_windows	;80386!  *almost* a nice cpu.

ow_done:
	ret

open_windows	ENDP

;--------------------------Local-Routine-----------------------------;
close_amiga	PROC NEAR

IF PRINT
	dec	oa_count
	DEB	0,oa_count
ENDIF
	call	free_service		;let the amiga go.
	ret

close_amiga	ENDP

close_windows	PROC NEAR
	push	si
	push	di

IF PRINT
	dec	ow_count
	DEB	1,ow_count
ENDIF
	cmp	mode,86
	jz	wep_done

	mov	ax,device_int_sel	;manufactured selector for CS
	or	ax,ax
	jz	skip_devint
	push	ax
	call	FreeSelector
	mov	device_int_sel,0
skip_devint:

	mov	ax,word ptr ms_sd_sel	;manufactured selector for ServiceData
	or	ax,ax
	jz	skip_sd
	push	ax
	call	FreeSelector
	mov	ms_sd_sel,0
skip_sd:

	mov	ax,word ptr ms_rq_sel	;manufactured selector for MouseServReq
	or	ax,ax
	jz	skip_rq
	push	ax
	call	FreeSelector
	mov	ms_rq_sel,0
skip_rq:

wep_done:
	mov	ax,1

	pop	di
	pop	si

	ret

close_windows	ENDP

;--------------------------Local-Routine-----------------------------;
close_stuff	PROC NEAR

	test	open_flags,OF_AMIGA
	jz	amiga_closed
	call	close_amiga
	and	open_flags,NOT OF_AMIGA
amiga_closed:

	test	open_flags,OF_WINDOWS
	jz	windows_closed
	call	close_windows
	and	open_flags,NOT OF_WINDOWS
windows_closed:

	ret

close_stuff	ENDP

;--------------------------Local-Routine-----------------------------;
open_stuff	PROC NEAR

	call	determine_mode

	test	open_flags,OF_AMIGA
	jnz	amiga_open
	call	open_amiga
	or	ax,ax
	jz	open_stuff_fail
	or	open_flags,OF_AMIGA
amiga_open:

	test	open_flags,OF_WINDOWS
	jnz	windows_open
	call	open_windows
	or	open_flags,OF_WINDOWS
windows_open:

	jmp	open_stuff_done

open_stuff_fail:
	call	close_stuff

open_stuff_done:
	xor	ax,ax
	mov	al,open_flags
	not	al
	and	al,OF_WINDOWS OR OF_AMIGA
;zero means both are open
	ret

open_stuff	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; enable_proc
;
; start the polling of the mouse
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

enable_proc	PROC NEAR

	call	open_stuff
	or	ax,ax
	jnz	ena_failed
;if AX<>0, we have an error.  but we can't do shit about it here.

	mov	ax,54			;54 milliseconds (18.2Hz)
	push	ax

	mov	ax,CSEG_device_int
	push	ax			;call the mouse poller
	mov	ax,offset device_int
	push	ax

	cCall	CreateSystemTimer	;fwhap

	mov	timer_handle,ax		;save ref returned for killing it

ena_failed:
	ret

enable_proc	ENDP

	page
;--------------------------Local-Routine-----------------------------;
; disable_proc
;
; stop polling the mouse
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

disable_proc	PROC NEAR

	mov	ax,timer_handle		;get saved timer ref
	push	ax

	cCall	KillSystemTimer		;fwhap

	call	close_stuff

	ret

disable_proc	ENDP

	page
;--------------------------Exported-Routine-----------------------------;
; int Inquire(lp_mouse_info);
;
; Information regarding the mouse is returned to the caller.
;
; Entry:
;	None
; Returns:
;	AX = # bytes returned in lp_mouse_info
; Error Returns:
;	None
; Registers Preserved:
;	SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,ES,FLAGS
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

cProc	Inquire,<FAR,PUBLIC,WIN,PASCAL>,<di>

	parmD	lp_mouse_info

cBegin
	les	di,lp_mouse_info	;Get far pointer of destination area
	assumes es,nothing

	cld				;forward.

	mov	al,0ffh			;AL = 0FFh if mouse exists, 00 if not
	mov	ah,0ffh 		;AH = 0FFh if relative coordinates
	stosw				;  (but is currently ignored!)
	.errnz	msExists
	.errnz	msRelative-msExists-1

	mov	ax,NUMBER_BUTTONS	;Return number of buttons
	stosw
	.errnz	msNumButtons-msRelative-1

	mov	ax,interrupt_rate	;Return maximum interrupt rate
	stosw
	.errnz	msRate-msNumButtons-2

	mov	ax,X_SPEED		;Return threshold before acceleration
	stosw
	.errnz	msXThresh-msRate-2

if Y_SPEED ne X_SPEED			;Generally they're the same
	mov	ax,Y_SPEED		;Return threshold before acceleration
endif
	stosw
	.errnz	msYThresh-msXThresh-2

	xor	ax,ax			;Return useless x,y resolution info
	stosw
	stosw
	.errnz	msXRes-msYThresh-2
	.errnz	msYRes-msXRes-2
	.errnz	msYRes+2 - size MOUSEINFO

	mov	ax,size MOUSEINFO	;Return size of info

cEnd

	page
;--------------------------Exported-Routine-----------------------------;
; int MouseGetIntVect();
;
; The interrupt vector used by the mouse is returned to the caller.
; If no mouse is found, then -1 is returned.
;
; Entry:
;	None
; Returns:
;	AX = interrupt vector
;	AX = -1 if no mouse was found
; Error Returns:
;	None
; Registers Preserved:
;	SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,ES,FLAGS
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

cProc	MouseGetIntVect,<FAR,PUBLIC,WIN,PASCAL>

cBegin

;	NOTE! vector must be less than 7Fh for the sign extension to work

; um, also note: the VMD in the microsoft examples thinks that a vector
; of -1 means "there ain't no mouse."  we must teach it otherwise.

	mov	al,vector		;Will be -1 if mouse wasn't found
	cbw				;AX = -1 if no mouse was found
cEnd

	page
;--------------------------Exported-Routine-----------------------------;
; void Enable(lp_event_proc);
;
; Enable hardware mouse interrupts, with the passed procedure address
; being the target of all mouse events.
;
; This routine may be called while already enabled.  In this case the
; passed event procedure should be saved, and all other initialization
; skipped.
;
; Entry:
;	None
; Returns:
;	None
; Error Returns:
;	None
; Registers Preserved:
;	SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,ES,FLAGS
; Calls:
;	enable_proc
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

cProc	Enable,<FAR,PUBLIC,WIN,PASCAL>,<si,di>

	parmD	new_event_proc

cBegin

;	The new event procedure is always saved regardless of the
;	mouse already being enabled.  This allows the event proc
;	to be changed as needed.

	cli				;Protect against interrupt while
	mov	ax,off_new_event_proc	;  changing the vector
	mov	wptr event_proc[0],ax
	mov	ax,seg_new_event_proc
	mov	wptr event_proc[2],ax
	sti

;	If the mouse is already enabled, or it doesn't exist, then
;	we're all done with the enable call.

	mov	al,mouse_flags		;If enabled or mouse doesn't exist,
	test	al,MMF_ENABLED
	jnz	enable_done
	call	enable_proc		;Mouse specific initialization
	or	mouse_flags,MMF_ENABLED	;Show enabled now

enable_done:

cEnd

	page
;--------------------------Exported-Routine-----------------------------;
; void Disable();
;
; Disable hardware mouse interrupts, restoring the previous mouse
; interrupt handler and 8259 interrupt enable mask.
;
; This routine may be called while already disabled.  In this case the
; disabling should be ignored.
;
; Entry:
;	None
; Returns:
;	None
; Error Returns:
;	None
; Registers Preserved:
;	SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,ES,FLAGS
; Calls:
;	disable_proc
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

cProc	Disable,<FAR,PUBLIC,WIN,PASCAL>,<si,di>

cBegin
	test	mouse_flags,MMF_ENABLED
	jz	disable_done		;Mouse is already disabled
	call	disable_proc		;Disable as needed
	and	mouse_flags,not MMF_ENABLED

disable_done:

cEnd

	page
;--------------------------Exported-Routine-----------------------------;
; WORD WEP();
;
; Generic WEP.
;
; Entry:
;	None
; Returns:
;	AX = 1
; Error Returns:
;	None
; Registers Preserved:
;	all
; Registers Destroyed:
;	none
; Calls:
;	nothing
; History:
;  Wed 18-Oct-1989 11:44:39  
; Wrote it!
;-----------------------------------------------------------------------;

;------------------------------Pseudo-Code------------------------------;
; {
; }
;-----------------------------------------------------------------------;

	assumes cs,Code
	assumes ds,Data

cProc	WEP,<FAR,PUBLIC,WIN,PASCAL>
;	parmW	stuff
cBegin nogen

	mov	ax,1
	ret	2

cEnd nogen

	page
;---------------------------Public-Routine-----------------------------;
; Initialize
;
; All boot time initialization will be performed.
;
; After a mouse handler has been found, the Data segment will be
; resized to the minimum needed to support the mouse in use.
;
; Entry:
;	CX = size of heap
;	DI = module handle
;	DS = automatic data segment
;	ES:SI = address of command line (not used)
; Returns:
;	AX <> 0 to show success
; Error Returns:
;	None
; Registers Preserved:
;	SI,DI,DS,BP
; Registers Destroyed:
;	AX,BX,CX,DX,ES,FLAGS
; Calls:
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

cProc	Initialize,<FAR,PUBLIC,WIN,PASCAL>,<si,di>

cBegin

	mov	open_flags,0

IF 0
	call	determine_mode

	call	open_stuff
	or	ax,ax
	jz	proceed

	xor	ax,ax			;failure.
	jmp	winit_done
ENDIF
proceed:
	;we're initialized.
	or	mouse_flags,MMF_MOUSE_EXISTS

	mov	ax,1			;Successful initialization

winit_done:

cEnd

sEnd	Code
end	Initialize
