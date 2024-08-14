
;
;
;	CheckVideo
;
;		Check which Video card(s) is(are) present
;

CheckVideo	Proc	Near

	push	cx
	push	si

	xor	al,al
	mov	CardActive,al
	mov	CardFound,al

;	check for VGA

	mov	ax,1a00H
	int	10H				; video interrupt
	cmp	al,1aH
	jnz	NotVGA

	or	CardFound,VGAType

	cmp	bl,7
	jb	NotVGA
	or	CardActive,VGAType

NotVGA:

;	check for EGA

	mov	ah,12H
	mov	bl,10H
	int	10H			; video

	cmp	bl,10H
	jz	NotEGA
	test	CardFound,VGAType	; is it really VGA ?
	jnz	NotEGA
	or	CardFound,EGAType

NotEGA:
	test	CardFound,VGAType
	jnz	EGAPresent
	test	CardFound,EGAType
	jnz	EGAPresent
	jmp	EGANotPres
EGAPresent:

;	check monitor type and set register addresses accordingly

	mov	word ptr EGA_CRTC,3d4H	; store Monochrome monitor values
	mov	word ptr EGA_Feat,3daH
	or	bh,bh			; 0 => Colour monitor connected
	jnz	EGANotCol
	mov	word ptr EGA_CRTC,3b4H	; store Colour monitor values
	mov	word ptr EGA_Feat,3baH
	mov	EGAColour,1
EGANotCol:
	
;	ram size

	mov	EGARamSize,bl

	xor	ax,ax
	mov	es,ax
	test	byte ptr es:[487h],8
	jnz	EGANotActive		; EGA not active
	test	CardActive,VGAType
	jnz	EGANotActive
	mov	CardActive,EGAType
EGANotActive:
	
	test	EGAColour,1
	jz	EGANotPres
	cmp	cl,3
	jz	EGAEnhan
	cmp	cl,9
	jnz	EGANotPres
EGAEnhan:
	mov	EGAEnhanced,1
	jmp	EGANotPres

EGANotPres:

;	check for mono adaptor

	test	EGAColour,1
	jnz	MonoCheck
	test	CardFound,EGAType+VGAType
	jnz	TryCGA1			; EGAColour false 
MonoCheck:
	mov	dx,3b4h			; mono control port
	mov	al,0fh
	out	dx,al
	inc	dx
	in	al,dx			; status ? cursor position ?
	xchg	ah,al
	mov	al,100
	out	dx,al
	in	al,dx
	cmp	al,100
	jnz	TryCGA
	or	CardFound,MDAType
	xchg	ah,al
	out	dx,al			; restore cursor position

TryCGA:
	test	EGAColour,1
	jz	TryCGA1
	test	CardFound,EGAType+VGAType
	jnz	TryHerc
TryCGA1:				; jump to here if EGAColour false
	mov	dx,3d4h			; CGA control port
	mov	al,0fh
	out	dx,al
	inc	dx
	in	al,dx			; status ? cursor position ?
	xchg	ah,al
	mov	al,100
	out	dx,al
	in	al,dx
	cmp	al,100
	jnz	TryHerc			; EGAColour true but !(ega||vga) also true
	or	CardFound,CGAType
	xchg	ah,al
	out	dx,al			; restore cursor position

TryHerc:

;	see if a Hercules card is present

	mov	dx,3baH			; Hercules status port
	in	al,dx
	and	al,80H
	xchg	al,ah
	mov	cx,8000H
HercTLoop:
	in	al,dx
	and	al,80H
	cmp	al,ah
	jz	HercAgain
	or	CardFound,HerType
	xor	CardFound,MDAType
	jmp	WhoActive
HercAgain:
	loop	HercTLoop

;	determine who, if not EGA/VGA, is active

WhoActive:
	test	CardActive,EGAType+VGAType
	jnz	ExitNow

	cmp	ax,7
	jnz	MustBeCGA
	test	CardFound,HerType
	jz	MDAAct
	mov	CardActive,HerType
	jmp	ExitNow
MDAAct:
	mov	CardActive,MDAType
	jmp	ExitNow
MustBeCGA:
	mov	CardActive,CGAType

ExitNow:

	pop	si
	pop	cx
	ret

CheckVideo	EndP


EndOfResident:


; --------------	end of resident code & data	-----------------------


VMode	db	13,10,"VideoMode : $"
ModeNo	db	13,10,"Number    : $"
Columns	db	13,10,"Columns   : $"
Rows	db	13,10,"Rows      : $"
Height	db	13,10,"Char size : $"
XPage	db	13,10,"Page size : $"
Seq	db	13,10,"Sequencer : $"
Misc	db	13,10,"Misc. reg : $"
CRTC	db	13,10,"CRTC reg. : $"
Attr	db	13,10,"Attr reg. : $"
Graph	db	13,10,"Graphics  : $"
Buffer	db	16 dup (?)
Hex	db	"0123456789ABCDEF"
Monitor	db	"Monitor type : $"
ColoMon	db	"Colour$"
MonoMon	db	"Monochrome",13,10,"$"
Enhance	db	"(Enhanced)"
NewLine	db	13,10,"$"
VGACard	db	13,10,"VGA card  : $"
EGACard	db	13,10,"EGA card  : $"
CGACard	db	13,10,"CGA card  : $"
MDACard	db	13,10,"MDA card  : $"
HerCard	db	13,10,"Hercules card  : $"
HerWarn	db	13,10,"Hercules information unreliable for multi-mode card"
	db	13,10,"$"

Inactive	db	"in"
Active		db	"active",13,10,"$"

RamSize		db	"H Kb RAM installed : $"

DoOutput	db	0

String	Proc	Near

	test	cs:DoOutput,1
	jz	SSkip			; no output required

	push	ax
	push	ds
	push	cs
	pop	ds
	mov	ah,9
	int	21H
	pop	ds
	pop	ax

SSkip:
	ret

String	EndP

Write	Proc	Near

	test	cs:DoOutput,1
	jz	WSkip			; no output required

	push	ax
	push	bx
	push	ds
	push	cs
	pop	ds

	xor	bx,bx
	mov	bl,al
	push	bx

	pop	bx
	push	bx
	shr	bx,1
	shr	bx,1
	shr	bx,1
	shr	bx,1
	mov	dl,hex[bx]
	mov	ah,2
	int	21H

	pop	bx
	and	bl,0FH
	mov	dl,hex[bx]
	mov	ah,2
	int	21H			; output character in DL

	pop	ds
	pop	bx
	pop	ax
WSkip:
	ret

Write	EndP


Display	Proc	Near

;	input	ax		video mode

	test	DoOutput,1
	jnz	DCont
	jmp	DSkip			; no output required
DCont:
	push	ds
	xor	bx,bx
	mov	es,bx
	les	di,es:[04A8H]		; Save Ptr Table
	lds	si,es:[di] 		; Video Parameter Table

	lea	dx,ModeNo
	call	String
	call	Write

	mov	cl,6
	shl	ax,cl			; mode * 64
	add	si,ax			; offset for this mode

	lea	dx,Columns
	call	String
	lodsb
	call	Write

	lea	dx,Rows
	call	String
	lodsb
	call	Write

	lea	dx,Height
	call	String
	lodsb
	call	Write

	lea	dx,XPage
	call	String
	lodsw
	xchg	ah,al
	call	Write
	mov	al,ah
	call	Write

	mov	cx,4
	lea	dx,Seq
	call	String
	jmp	SeqL1
SeqLoop:
	mov	dl,","
	mov	ah,2
	int	21H
SeqL1:
	lodsb
	call	Write
	loop	SeqLoop

	lea	dx,Misc
	call	String
	lodsb
	call	Write

	mov	cx,25
	lea	dx,CRTC
	call	String
	jmp	MiscL1
MiscLoop:
	mov	dl,","
	mov	ah,2
	int	21H
MiscL1:
	lodsb
	call	Write		
	loop	MiscLoop

	mov	cx,20
	lea	dx,Attr
	call	String
	jmp	AttrL1
AttrLoop:
	mov	dl,","
	mov	ah,2
	int	21H
AttrL1:
	lodsb
	call	Write
	loop	AttrLoop

	mov	cx,9
	lea	dx,Graph
	call	String
	jmp	GraphL1
GraphLoop:
	mov	dl,","
	mov	ah,2
	int	21H
GraphL1:
	lodsb
	call	Write
	loop	GraphLoop
	pop	ds
DSkip:
	ret

Display	EndP


;
;
;	ShowVideo
;
;		Check which Video card(s) is(are) present and
;		print Video information if DoOutput is set
;

ShowVideo	Proc	Near

	push	cx
	push	si

	call	CheckVideo

;	check for VGA


	test	CardFound,VGAType
	jz	SV_NotVGA
	lea	dx,VGACard
	call	String

	test	CardActive,VGAType
	jz	SV_VGANotActive
	lea	dx,Active
	jmp	SV_VGADisp
SV_VGANotActive:
	lea	dx,Inactive
SV_VGADisp:
	call	String

SV_NotVGA:

;	check for EGA

	test	CardFound,EGAType
	jz	SV_NotEGA
	lea	dx,EGACard
	call	String

SV_NotEGA:
	test	CardFound,VGAType+EGAType
	jz	SV_EGANotPres

;	ram size

	xor	ah,ah
	mov	al,EGARamSize
	inc	al
	mov	cl,6
	shl	ax,cl			; *64
	xchg	ah,al
	call	Write
	xchg	ah,al
	call	Write
	lea	dx,RamSize
	call	String

	lea	dx,Inactive
	test	CardActive,EGAType
	jz	SV_EGANotActive		; EGA not active
	lea	dx,Active
SV_EGANotActive:
	call	String
	
	lea	dx,Monitor
	call	String
	test	EGAColour,1
	jz	SV_EGAMono
	lea	dx,ColoMon
	call	String
	lea	dx,NewLine
	test	EGAEnhanced,1
	jz	SV_EGANotEnhan
SV_EGAEnhan:
	lea	dx,Enhance
	jmp	SV_EGANotEnhan
SV_EGAMono:
	lea	dx,MonoMon
SV_EGANotEnhan:
	call	String

SV_EGANotPres:

;	check for mono adaptor

	test	CardFound,MDAType
	jz	SV_TryCGA
	lea	dx,MDACard
	call	String
	lea	dx,Inactive
	test	CardActive,MDAType
	jz	SV_MDANotActive
	lea	dx,Active
SV_MDANotActive:
	call	String
	lea	dx,NewLine
	call	String

;	check for Colour adaptor

SV_TryCGA:
	test	CardFound,CGAType
	jz	SV_TryHerc
	lea	dx,CGACard
	call	String
	lea	dx,Inactive
	test	CardActive,CGAType
	jz	SV_CGANotActive
	lea	dx,Active
SV_CGANotActive:
	call	String
	lea	dx,NewLine
	call	String


;	see if a Hercules card is present

SV_TryHerc:
	test	CardFound,HerType
	jz	SV_WhatMode
	lea	dx,HerWarn
	call	String
	lea	dx,HerCard
	call	String
	lea	dx,Inactive
	test	CardActive,HerType
	jz	SV_HerNotActive
	lea	dx,Active
SV_HerNotActive:
	call	String
	lea	dx,NewLine
	call	String

;	Video mode, index and initial register values

SV_WhatMode:

	xor	ax,ax
	mov	es,ax
	mov	al,es:[449H]		; Video Mode
	and	al,7fH			; remove 'don't clear' bit

	lea	dx,VMode
	call	String
	call	Write
	lea	dx,NewLine
	call	String
	call	WhatIndex		; VMode in AX to Index in AX
	call	Display			; display register values
	lea	dx,NewLine
	call	String

	pop	si
	pop	cx
	ret

ShowVideo	EndP


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

ms_app_id	equ	MOUSESERV_APPLICATION_ID
ms_local_id	equ	MOUSESERV_LOCAL_ID
ms_flags	equ	GETS_WAIT+GETS_ALOAD_A

get_service	PROC	NEAR

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


;****************************************************************************
;*
;* GetParams
;*
;****************************************************************************

GPQuit:
	jmp	NoParams

GetParams	PROC	NEAR

	mov	cl,ParamSize
	xor	ch,ch
	jcxz	GPQuit
	lea	si,Params
PLoop:
	lodsb
;	cmp	al," "
;	je	PSkip
	cmp	al,"-"			; start of parameter
	jne	PXSkip
	dec	cx
	jcxz	GPQuit
	lodsb
	cmp	al,"i"
	jne	NotInt
	mov	al,TimerIntNo
	xchg	al,OtherIntNo
	mov	TimerIntNo,al
PXSkip:
	jmp	PSkip
NotInt:
	mov	Hercules,0
	cmp	al,"h"			; hercules
	jne	NotHercules
	lodsb
	and	al,3			; page number 1 or 2
	mov	Hercules,al
	mov	Bank,3			; 4 banks
	jmp	PSkip
NotHercules:
	cmp	al,"t"
	jne	NotTrace
	mov	DoOutput,1		; print video params at start
	dec	cx	
	jcxz	PSkip			; no more parameters
	lodsb
	and	al,0fH
	mov	DoTrace,al
	jmp	PSkip
NotTrace:
	cmp	al,"c"
	jne	NoParams
	dec	cx
	jcxz	NoParams
	xor	bx,bx
	lea	di,Customise
	push	ds
	pop	es
	lodsb
	mov	ah," "			; default terminator
	cmp	al,22H			; <">
	jne	SpaceTerminates
	mov	ah,22H			; quoted string
	dec	cx
	jcxz	NoParams
CLoop:
	lodsb				; read a character
	cmp	al,ah
	je	PSKip
	cmp	al,0DH
	je	PSKip
SpaceTerminates:
	cmp	bx,40
	je	MSkip			; too long
	stosb				; store a character
	inc	bx
MSkip:
	loop	CLoop
	jmp	NoParams		; reached end of parameters

PSkip:
	loop	PXLoop
NoParams:
	ret

PXLoop:					; needed because of relative jump limit
	jmp	PLoop

GetParams	ENDP

WaitForAmiga	db	1		; report error if amiga AMouse not active
ToldUser	db	0		; already output the waiting msg
KeyOffset	dw	0
KeySegment	dw	0

NoSMsg	db	"Amiga could not load MouseServ - AMOUSE not installed.",0DH,0AH,"$"
VersNo	db	"AMouse version 1.7"
;	include	Date.inc
	db	" installed",0DH,0AH,"$"
Passed	db	"Parameters passed to previously installed driver",0DH,0AH,"$"
Already	db	"A mouse driver is already installed",0DH,0AH,"$"
;Waiting	db	"Waiting for AMouse on Amiga",0dh,0ah
;	db	"Press a key to abandon",0dh,0ah,"$"

;	this routine replaces the keyboard interrupt routine
;	while initialising. It sets WaitForAmiga false when a key is pressed

; KeyInt:
; 	mov	cs:WaitForAmiga,0	; stop on any key
;	iret

;	service not available on Amiga. Decide what to do next

NoService:

;	mov	dl,255			; no output
;	mov	ah,6			; keyboard/display i/o
;	int	21H
;	jnz	NoServMsg		; ZF=0 => got input
;
;	test	ToldUser,-1
;	jnz	UserTold
;	mov	ToldUser,1
;	mov	ah,9
;	mov	dx,offset Waiting
;	int	21H
;UserTold:
;	jmp	TryJanus		; keep on trying

;	give up on amiga

NoServMsg:

	mov	ah,9
	mov	dx,offset NoSMsg
	int	21H
	mov	ax,4C02H
	int	21H			; terminate with return code


;	already got a mouse driver. Whose is it ?

AlreadyIn:
	mov	cx,40			; CopyRight length
	lea	di,CopyRight
	mov	si,di
	rep cmpsb
	jcxz	ItsMine

;	a different mouse driver - exit

	mov	ah,9
	mov	dx,offset Already
	int	21H
	mov	ax,4C03H
	int	21H			; terminate with return code

;	AMouse already resident. Pass on the parameters

ItsMine:
	lea	di,Customise
	mov	si,di
	mov	cx,40
	rep movsb
	mov	al,TimerIntNo
	push	es
	pop	ds
	cmp	al,TimerIntNo
	je	SameInts		; interrupts as previously
	sti
	mov	TimerIntNo,al
	mov	al,cs:OtherIntNo
	mov	OtherIntNo,al
	mov	ax,TimerProc
	xchg	ax,OtherProc
	mov	TimerProc,ax
	cli
SameInts:
	mov	al,cs:Hercules
	mov	Hercules,al
	mov	al,cs:DoTrace
	mov	DoTrace,al
	mov	al,cs:Bank
	mov	Bank,al
	mov	ax,cs
	mov	ds,ax
	mov	ah,9
	mov	dx,offset Passed
	int	21H
	mov	ax,4C00H
	int	21H			; terminate with return code
	
	

main:
	push	cs
	pop	ds

	call	GetParams
	call	ShowVideo		; displays Video info, if req.

	mov	ax,DataSize
	mov	SaveSize,ax

TryJanus:
	call	tickle_janus
	or	ax,ax
	jnz	GotService
	jmp	NoService

GotService:

;	it's not enough to call GetBase three times and check the result of
;	the last one. It's possible that only the last one was successful
;	in the case where AMouse on the Amiga side is called during the loop.
;	So I've set the loop to 1 and as soon as a call is successful it will
;	drop through to here.

	call	get_service
	or	ax,ax
	jz	ReallyGotService
	jmp	NoService

ReallyGotService:
	mov	ax,3533H		
	int	21H			; read current interrupt 51 handler
	mov	ax, es			; Check segment and
	or	ax, bx			; offset of int 33
	jz	NotIn			; vector.  If 0 or pointing to
	mov	al, es:[bx]		; an IRET driver not installed
	cmp	al, 0cfh
	je	NotIn			; not already installed
	jmp	AlreadyIn		; already installed
NotIn:
	mov	OldSegment,es		; save
	mov	OldOffset,bx
	mov	dx,OFFSET MouseDriver
	push	cs
	pop	ds
	mov	ax,2533H		; set new interrupt 51 handler
	int	21H

	mov	ah,35H
	mov	al,TimerIntNo
	int	21H			; read current Timer interrupt handler
	mov	TimerSegment,es		; save
	mov	TimerOffset,bx
	mov	dx,OFFSET Timer
	push	cs
	pop	ds
	mov	ah,25H			; set new Timer interrupt handler
	mov	al,TimerIntNo
	int	21H


	mov	ah,35H
	mov	al,OtherIntNo
	int	21H			; read current Other interrupt handler
	mov	OtherSegment,es		; save
	mov	OtherOffset,bx
	mov	dx,OFFSET Other
	push	cs
	pop	ds
	mov	ah,25H			; set new Other interrupt handler
	mov	al,OtherIntNo
	int	21H


	mov	ax,3510H		
	int	21H			; read current interrupt 10H handler
	mov	VideoSegment,es		; save
	mov	VideoOffset,bx
	mov	dx,OFFSET Video
	push	cs
	pop	ds
	mov	ax,2510H		; set new interrupt 10H handler
	int	21H

	les	di,dword ptr ms_rq_np
	mov	cx,DPSize
	xor	ax,ax
	rep stosw			; zeroise Dual Port Ram variables.

	mov	es,ax			; segement zero
	mov	al,es:[449H]
	and	al,7fH			; remove 'don't clear' bit
	mov	ScreenMode,al
	call	SetVideoPars		; also tells Amiga what video mode
	mov	ah,9
	mov	dx,offset VersNo
	int	21H
	mov	ax,3100H
	lea	dx,EndOfResident+10FH	; round up + program header
	mov	cl,4
	sar	dx,cl
	int	21H			; terminate but stay resident

