	TITLE   emumouse

cseg	SEGMENT  BYTE PUBLIC 'CODE'
cseg	ENDS
	ASSUME  CS: cseg, DS: cseg, SS: cseg, ES: cseg
cseg      SEGMENT


		org	80H
ParamSize	db	1 dup (?)
Params		db	7fH dup (?)	

		org	100H		; ======
Start:
		jmp	main

DoTrace		db	0

TimerIntNo	db	8		; default is 8, alternative is 1CH
OtherIntNo	db	1CH		; default is 8, alternative is 1CH
TimerProc	dw	offset MouseInt
OtherProc	dw	offset Null

OldOffset	DW 01H DUP (?)
OldSegment	DW 01H DUP (?)
TimerOffset	DW 01H DUP (?)
TimerSegment	DW 01H DUP (?)
OtherOffset	DW 01H DUP (?)
OtherSegment	DW 01H DUP (?)
VideoOffset	DW 01H DUP (?)
VideoSegment	DW 01H DUP (?)

ms_rq_np	dw	0,0		; Dual Port Ram Address
ms_sd_np	dw	0,0		; Dual Port Ram Address


CardFound	db	0

CardActive	db	0

EGARamSize	db	0		; (x+1)*64Kb
EGAColour	db	0
EGAEnhanced	db	0

	include	janus\janusvar.inc
	include	janus\services.inc
	include	..\amouse\mouseser.inc

CursorWidth	equ	16		; make these variables ?????
CursorHeight	equ	16

DefaultMouse	dw	03FFFH,01FFFH,00FFFH,007FFH,003FFH,001FFH,000FFH
		dw	0007FH,0003FH,0001FH,001FFH,010FFH,030FFH,0F87FH
		dw	0F87FH,0FC3FH
		dw	00000H,04000H,06000H,07000H,07800H,07C00H,07E00H
		dw	07F00H,07F80H,07FC0H,07C00H,04600H,00600H,00300H
		dw	00300H,00180H
StateInfo:
DataStart	equ	$

CursorFlag	dw	-1
CursorSave	dw	0
SaveMask	db	3 dup (0)
CursorX		db	0
CursorY		db	0
CSValid		db	0
GraphValid	db	0
GraphCX		dw	0
GraphCY		dw	0
TextCurs	db	0
TCType		db	0
TCScrMask	dw	0FFFFH
TCCursMask	dw	07700H
GraphicsCDef	dw	32 dup (0)
ScreenOffset	dw	0
ScreenSegment	dw	0
NewMask		dw	0
IntCallMask	dw	0
IntCallAddr	dw	0
IntCallSeg	dw	0
XMPRatio	dw	8
YMPRatio	dw	16
XMinCursor	dw	0
XMaxCursor	dw	639
YMinCursor	dw	0
YMaxCursor	dw	199
CRTPageNumber	dw	0
LightPenMode	dw	0
ButtonStatus	dw	0

OddOffset	dw	50H-2000H	; offset between even and following odd line
EvenOffset	dw	2000H		; offset between odd and following even line
EGA		db	0
EGA350		db	0
Bank		db	1
LastFullMPos	dw	184

MouseX		dw	320
MouseY		dw	100
XShift		db	0
YShift		db	0
MickeyX		dw	0
MickeyY		dw	0
HotSpotX	dw	0
HotSpotY	dw	0

DoMove		db	0

CondFlag	db	0		; Conditional off flag
LeftX		dw	0		; Conditional off rectangle
RightX		dw	0
TopY		dw	0
BottomY		dw	0

Buttons		dw	16 dup (0)
PressCount	equ	0
PressX		equ	2
PressY		equ	4
ReleaseCount	equ	6
ReleaseX	equ	8
ReleaseY	equ	10

; EGAPlanes	db	4			; number of EGA planes

GraphicsSave	db	192 dup(0)		; 3 x 16 x 4


DataSize	dw	$-DataStart

Hercules	db	0

;---		don't split from here ...

VersionNo	db	6,21		; emulating 6.03
CRMsg		dw	offset Customise
		dw	0,0
		dw	$+2
		dw	0		; end of chain
SaveSize	dw	0		; move DataSize into this
		dw	offset StateInfo

;---		... to here

DPCopyArea	dw	DPSize dup(0)

ScreenMode	db	0
ScreenMaxX	dw	640
ScreenMaxY	dw	200

InTimer		db	0
old_count	db	0

;****************************************************************************
;*
;* MouseInt
;*
;****************************************************************************
MouseInt	PROC	NEAR
	test	cs:IntCallMask,-1
	jz	NextTimer

	cli				; disable interrupts - if not already
	test	cs:InTimer,-1
	jnz	NextTimer		; still running since last int.

	inc	cs:InTimer
	push	bp
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
	mov	ax,NewMask
	or	ax,ax
	jz	NoChange
	and	ax,IntCallMask		; does user want this one ?
	jz	NoChange
	mov	cl,XShift		; round MouseX to nearest 1<<XShift
	mov	bx,MouseX
	sar	bx,cl
	shl	bx,cl
	mov	cl,YShift		; round MouseY to nearest 1<<YShift
	mov	dx,MouseY
	sar	dx,cl
	shl	dx,cl
	mov	cx,bx			; want X co-ord in CX
	mov	bx,ButtonStatus
	mov	di,MickeyX
	mov	si,MickeyY
	pushf
	sti
	call	dword ptr IntCallAddr
	popf
	mov	cs:NewMask,0
NoChange:
	
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	bp
	dec	cs:InTimer
NextTimer:
	ret

MouseInt	ENDP
;****************************************************************************
;*
;* Null
;*
;****************************************************************************

Null		PROC	NEAR
	ret
Null		ENDP

;****************************************************************************
;*
;* Timer
;*
;****************************************************************************

Timer		PROC	FAR

	pushf
	call	cs:dword ptr TimerOffset
	call	cs:[TimerProc]
	iret

Timer		ENDP

;****************************************************************************
;*
;* Other
;*
;****************************************************************************
Other		PROC	FAR

	pushf
	call	cs:dword ptr OtherOffset
	call	cs:[OtherProc]
	iret

Other		ENDP


;****************************************************************************
;*
;* SetVideoPars
;*
;****************************************************************************
SetVideoPars	PROC	NEAR

	push	ds
	push	cs
	pop	ds
	mov	ScreenSegment,0B800H		; CGA address
	mov	ScreenMaxX,640			; EGA
	mov	ScreenMaxY,200			; EGA
	mov	OddOffset,50H-2000H		; EGA
	mov	EvenOffset,2000H		; EGA
	mov	EGA,0				; EGA
	mov	EGA350,0			; EGA
	cmp	ax,1
	jg	VideoNot01
	mov	XShift,4
	mov	YShift,3
	jmp	TellVideo
VideoNot01:
	cmp	ax,3
	jg	VideoNot23
VideoText8x8:
	mov	XShift,3
	mov	YShift,3
	jmp	TellVideo
VideoNot23:
	cmp	ax,7
	jne	VideoGraphics
	mov	ScreenSegment,0B000H		; mono address
	jmp	VideoText8x8
VideoGraphics:
	mov	XShift,0
	mov	YShift,0
	test	Hercules,-1
	jz	VGNotHerc
	mov	ScreenMaxX,720
	mov	ScreenMaxY,348
	mov	OddOffset,5AH-6000H
	mov	EvenOffset,2000H
	xor	ah,ah
	mov	al,Hercules
	sub	al,1
	mov	CRTPageNumber,ax
	or	ax,ax
	jnz	VGHercPg2
	mov	ScreenSegment,0B000H
VGHercPg2:					; B800 already set
	sub	al,6
	neg	ax				; 5 for page 1, 6 for page 0
	push	ax
	xor	ax,ax
	mov	ds,ax				; ds = 0
	pop	ax
	mov	ds:[449H],al
	jmp	TellVideo
VGNotHerc:
	cmp	ax,13				; EGA
	jb	TellVideo			; EGA
	mov	ScreenSegment,0A000H		; EGA
	mov	OddOffset,50H			; EGA
	mov	EvenOffset,50H			; EGA
	mov	EGA,-1				; EGA
	cmp	ax,15				; EGA
	jb	TellVideo			; EGA
	mov	ScreenMaxY,350			; EGA
	mov	EGA350,-1			; EGA
TellVideo:
	mov	ax,ScreenMaxY
	sub	ax,16
	mov	LastFullMPos,ax
	pop	ds
	sti
	ret

SetVideoPars	ENDP

;****************************************************************************
;*
;* Video
;*
;****************************************************************************
Video		PROC	FAR

	or	ah,ah
	jnz	VideoContinue
NewVideo:
	push	ax
	push	di
	push	es

	push	ax
	push	ds
	push	cs
	pop	ds			; ds = cs
	call	DoHideCursor
	mov	CursorFlag,-1
	pop	ds			; restore ds
	pop	ax			; restore ax
	pushf
	call	cs:dword ptr VideoOffset	; simulate interrupt
	xor	ax,ax
	mov	es,ax
	mov	al,es:[0449H]			; video mode
	and	al,7fH				; remove 'don't clear' bit
	cmp	cs:ScreenMode,al
	jz	SameVideo
	mov	cs:ScreenMode,al
	push	ax				; save for InitEGALib
	call	SetVideoPars
	pop	ax				; ax = screen mode
	call	InitEGALib			; init EGA, if it is EGA
SameVideo:
	pop	es
	pop	di
	pop	ax
	iret

VideoContinue:
	test	cs:CardActive,EGAType
	jz	VideoNormal
	cmp	ah,0f0h
	jb	VideoNormal
	call	EGAProc
	iret					; don't jump to SameVideo

VideoNormal:
	jmp	cs:dword ptr VideoOffset

Video		ENDP


;****************************************************************************
;*
;* Print
;*
;****************************************************************************
Print		PROC	NEAR
	pushf
	sti
	mov	ah,5
	int	21H
	popf
	ret
Print		ENDP

;****************************************************************************
;*
;* Trace
;*
;****************************************************************************
ETrace		PROC	NEAR
	test	cs:DoTrace,2
	jz	NoTrace
	push	ax
	push	dx
	
	mov	dx,1			; EGA trace identifier

	jmp	TraceCommon

ETrace		ENDP

Trace		PROC	NEAR
	test	cs:DoTrace,1
	jz	NoTrace
	push	ax
	push	dx
	
	xor	dx,dx			; mouse trace identifier

TraceCommon:
	xchg	dh,dl
	call	Print
	xchg	dh,dl
	call	Print

	pop	dx
	pop	ax
	push	ax
	push	dx

	push	di
	push	di
	push	si
	push	si
	push	dx
	push	dx
	push	cx
	push	cx
	push	bx
	push	bx
	push	ax
	push	ax
	
	pop	dx
	mov	dl,dh
	call	Print		; ah
	pop	dx
	call	Print		; al
	pop	dx
	mov	dl,dh
	call	Print		; bh
	pop	dx
	call	Print		; bl
	pop	dx
	mov	dl,dh
	call	Print		; ch
	pop	dx
	call	Print		; cl
	pop	dx
	mov	dl,dh
	call	Print		; dh
	pop	dx
	call	Print		; dl
	pop	dx
	mov	dl,dh
	call	Print		; si h
	pop	dx
	call	Print		; si l
	pop	dx
	mov	dl,dh
	call	Print		; di h
	pop	dx
	call	Print		; di l

	pop	dx
	pop	ax
NoTrace:
	ret

Trace		ENDP

;****************************************************************************
;*
;* DoCondOff
;*
;****************************************************************************
;	returns with C flag set if cursor has been turned off

DoCondOff	PROC	NEAR
	push	ax
	test	CondFlag,-1
	jz	NotCondOff
	mov	ax,MouseX		; compare co-ords with corners of
	sub	ax,HotSpotX
	cmp	ax,RightX		; conditional off area
	jg	NotCondOff
	add	ax,CursorWidth
	cmp	ax,LeftX
	jl	NotCondOff
	mov	ax,MouseY
	sub	ax,HotSpotY
	cmp	ax,BottomY
	jg	NotCondOff
	add	ax,CursorHeight
	cmp	ax,TopY
	jl	NotCondOff
	call	DoHideCursor
	stc
	jmp	CondOffExit
NotCondOff:
	clc
CondOffExit:
	pop	ax
	ret

DoCondOff	ENDP

;****************************************************************************
;*
;* DoHideCursor
;*
;****************************************************************************
DoHideCursor	PROC	NEAR

	test	CursorFlag,-1
	jnz	DoHCexit		; cursor not shown
	test	XShift,-1		; if non-zero then in text mode
	jz	HideGraphics
	test	TextCurs,-1		; zero is software cursor
	jnz	HideHardware
	call	TidySoftCursor
	jmp	DoHCexit
HideHardware:
	mov	cx,2000H		; cursor off
	mov	ah,01			; set cursor type ( = no display )
	int	10H			; do it
	jmp	DoHCexit
HideGraphics:
	call	TidyGraphCursor
DoHCexit:
	dec	CursorFlag
	ret

DoHideCursor	ENDP

;****************************************************************************
;*
;* MouseToCursor
;*
;****************************************************************************
MouseToCursor	PROC	NEAR

	mov	ax,MouseY
	mov	cl,YShift
	shr	ax,cl
	mov	CursorY,al
	mov	ax,MouseX
	mov	cl,XShift
	shr	ax,cl
	mov	CursorX,al
	ret

MouseToCursor	ENDP

	include	GraphC.asm

;****************************************************************************
;*
;* ShowSoftCursor
;*
;****************************************************************************
ShowSoftCursor	PROC	NEAR

	test	TextCurs,-1		; hard or soft cursor ?
	jnz	SSCReturn		; hardware cursor
	call	MouseToCursor
	push	es
	push	di
	xor	ax,ax
	mov	es,ax			; Segment 0
	mov	di,es:[044EH]		; Page offset
	mov	ax,es:[044AH]		; Screen width < 255 ????
	mul	CursorY
	mov	bl,CursorX
	xor	bh,bh
	add	ax,bx			; character offset
	shl	ax,1			; characters occupy 2 bytes
	add	di,ax			; offset into screen buffer
	mov	ScreenOffset,di
	mov	es,ScreenSegment	; B000 or B800
	mov	ax,es:0[di]		; get character and attributes
	mov	CursorSave,ax		; save to restore later
	mov	CSValid,1
	and	ax,TCScrMask
	xor	ax,TCCursMask
	mov	es:0[di],ax		; rewrite cursor
	pop	di
	pop	es
SSCReturn:
	ret

ShowSoftCursor	ENDP

;****************************************************************************
;*
;* TidySoftCursor
;*
;****************************************************************************
TidySoftCursor	PROC	NEAR

	test	TextCurs,-1		; hard or soft cursor ?
	jnz	TSCReturn		; hardware cursor
	test	CSValid,-1		; got something to restore ?
	jz	TSCReturn		; no
	mov	CSValid,0
	push	es
	push	di
	les	di,dword ptr ScreenOffset
	mov	ax,CursorSave		; previously saved by ShowSoftCursor
	mov	es:0[di],ax		; restore character
	pop	di
	pop	es
TSCReturn:
	ret
	
TidySoftCursor	ENDP

;****************************************************************************
;*
;* MoveCursor
;*
;****************************************************************************
MoveCursor	PROC	NEAR

	pushf
	cli				; inhibit interrupts
	
	call	MouseToCursor
	mov	ch,CursorY
	mov	cl,CursorX
	xor	ax,ax
	push	ds
	mov	ds,ax
	mov	al,ds:[044AH]		; columns per line
	mul	ch			; Y * columns
	xor	ch,ch
	add	cx,ax			; add in X offset
	add	cx,ds:[044EH]		; page offset ???????
	mov	dx,ds:[0463H]		; CRT Register Base

	mov	ah,ch	
	mov	al,0EH
	out	dx,ax			; send cursor pos "hi"

	mov	ah,cl	
	mov	al,0FH
	out	dx,ax			; send cursor pos "lo"

 	pop	ds			; restore ds
	popf				; restore interrupt state

	ret

MoveCursor	ENDP


;****************************************************************************
;*
;* MouseRead
;*
;****************************************************************************
MouseRead	PROC	NEAR

	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es

	les	di,dword ptr ms_rq_np	;find out if the changecount changed
	mov	al,es:ChangeCount[di]
	cmp	al,old_count
	jnz	changed			;yes - do all the work.
	jmp	MRNoMove		;no - get out.

changed:
	mov	old_count,al

	push	ds
	mov	ax,ds
	mov	es,ax
	lea	di,DPCopyArea			; es:di points at copy area
	lds	si,dword ptr ms_rq_np		; ds:si - DP area
	mov	cx,DPSize			; copy DPSize words
	rep movsw
	pop	ds

	les	di,dword ptr ms_rq_np
					; zeroise up to but excluding
					; button status
	mov	cx,(AmigaPCStatus/2)	; convert to words
	xor	ax,ax
	rep stosw

	les	di,dword ptr ms_rq_np	;find out if the changecount changed
	mov	al,es:ChangeCount[di]
	cmp	al,old_count
	jnz	changed			;yes - do all the work.

	xor	dl,dl		; if changes then will be set non-zero 
	lea	si,DPCopyArea	; point at copy of DualPort values

				; get new MouseX (within limits )
	mov	ax,AmigaPCX[si]
	or	ax,ax
	jz	GetY		; no change in X co-ord.
	or	NewMask,1	; cursor position change
	mov	dl,1		; move the cursor
	add	MickeyX,ax
	push	dx
	shl	ax,1
	shl	ax,1
	shl	ax,1		; *8
	cwd			; sign extend into DX
	idiv	XMPRatio
	pop	dx
	add	ax,MouseX
	cmp	ax,XMinCursor
	jge	XMinOK
	mov	ax,XMinCursor
XMinOK:
	cmp	ax,XMaxCursor
	jle	XMaxOK
	mov	ax,XMaxCursor
XMaxOK:
	mov	MouseX,ax
GetY:
				; get new MouseY (within limits )
	mov	ax,AmigaPCY[si]
	or	ax,ax
	jz	GetButtons	; no change in Y co-ord.
	or	NewMask,1	; cursor position change
	mov	dl,1		; move the cursor
	add	MickeyY,ax
	push	dx
	shl	ax,1
	shl	ax,1
	shl	ax,1		; *8
	cwd			; sign extend into DX
	idiv	YMPRatio
	pop	dx
	add	ax,MouseY
	cmp	ax,YMinCursor
	jge	YMinOK
	mov	ax,YMinCursor
YMinOK:
	cmp	ax,YMaxCursor
	jle	YMaxOK
	mov	ax,YMaxCursor
YMaxOK:
	mov	MouseY,ax
GetButtons:
	lea	bx,Buttons
	mov	ax,AmigaPCLeftP[si]
	or	ax,ax
	jz	GetLRel
	or	NewMask,2
	add	PressCount[bx],ax		; Left Count
	mov	ax,MouseX
	mov	PressX[bx],ax
	mov	ax,MouseY
	mov	PressY[bx],ax
GetLRel:
	mov	ax,AmigaPCLeftR[si]
	or	ax,ax
	jz	GetRPrs
	or	NewMask,4
	add	ReleaseCount[bx],ax		; Left Count
	mov	ax,MouseX
	mov	ReleaseX[bx],ax
	mov	ax,MouseY
	mov	ReleaseY[bx],ax
GetRPrs:
	add	bx,16
	mov	ax,AmigaPCRightP[si]
	or	ax,ax
	jz	GetRRel
	or	NewMask,8
	add	PressCount[bx],ax		; right Count
	mov	ax,MouseX
	mov	PressX[bx],ax
	mov	ax,MouseY
	mov	PressY[bx],ax
GetRRel:
	mov	ax,AmigaPCRightR[si]
	or	ax,ax
	jz	GetStatus
	or	NewMask,16
	add	ReleaseCount[bx],ax
	mov	ax,MouseX
	mov	ReleaseX[bx],ax
	mov	ax,MouseY
	mov	ReleaseY[bx],ax
GetStatus:
	mov	ax,AmigaPCStatus[si]
	mov	ButtonStatus,ax
	test	dl,1			; cursor position change
	jz	MRNoMove
	call	MoveMouse
MRNoMove:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

MouseRead	ENDP

;****************************************************************************
;*
;* MoveMouse
;*
;****************************************************************************
MoveMouse	PROC	NEAR

	test	CursorFlag,-1
	jnz	MMoveExit		; if non-zero cursor is off
	call	DoCondOff		; check for conditional off
	jb	MMoveExit		; jump if Carry flag is set (jcc!!)
	test	XShift,-1
	jz	MMGraphics		; if non-zero then in text mode
	test	TextCurs,-1		; hard or soft cursor ?
	jnz	MMHardware		; hardware cursor
	call	TidySoftCursor
	call	ShowSoftCursor
	jmp	MMoveExit
MMHardware:
	call	MoveCursor		; hardware cursor
	jmp	MMoveExit

MMGraphics:
	pushf
	cli
	call	TidyGraphCursor
	call	ShowGraphCursor
	popf

MMoveExit:
	ret

MoveMouse	ENDP

CopyRight:
	db	"===    Copyright (c) 1988 Commodore Amiga by Bill Holohan   ==="
Customise:
	db	"****************************************"

MouseDriver	PROC NEAR
	jmp	MainCode

BasicMouse	PROC FAR
	push	bp			; for bloody BASIC
	mov	bp,sp
	mov	si,[bp+12]
	mov	ax,[si]
	mov	si,[bp+10]
	mov	bx,[si]
	mov	si,[bp+8]
	mov	cx,[si]
	mov	si,[bp+6]
	mov	dx,si
	cmp	ax,9			; graphics cursor block
	jz	UseDXValue
	cmp	ax,18			; ?????
	jz	UseDXValue
	cmp	ax,12			; Interrupt call ...
	jz	UseDXValue
	cmp	ax,20			; Swap int....
	jnz	UseDXasPtr
	mov	es,bx
	or	bx,bx
	jnz	UseDXValue
	push	ds
	pop	es
	jmp	UseDXValue
UseDXasPtr:
	mov	dx,[si]
	cmp	ax,16
	jnz	UseDXValue
	mov	cx,[si]
	mov	dx,[si+02]
	mov	di,[si+06]
	mov	si,[si+04]
UseDXValue:
	push	ax
	int	33H
	pop	si
	cmp	si,20
	jnz	Check9
	mov	bx,es
	push	ds
	pop	es
Check9:
	cmp	si,9
	jz	NoDX
	cmp	si,12
	jz	NoDX
	cmp	si,16
	jz	NoDX
	mov	si,[bp+6]
	mov	[si],dx
NoDX:
	mov	si,[bp+8]
	mov	[si],cx
	mov	si,[bp+10]
	mov	[si],bx
	mov	si,[bp+12]
	mov	[si],ax
	pop	bp
	ret	8

BasicMouse	ENDP

MainCode:
	push	bp
	mov	bp,sp

	push	ds			; bp-2
	push	es			; bp-4
	push	di			; bp-6
	push	si			; bp-8
	push	bx			; bp-10		save it - M2%
	push	ax			; bp-12

	push	cs
	pop	ds

	call	MouseRead

	sti

	call	Trace

	or	ax,ax
	jns	check2			; jump if not negative
failcheck:
	jmp	exit
check2:
	cmp	ax,30
	jg	Specials		; jump if > 30
	shl	ax,1			; *2
	mov	bx,ax
	lea	bx,[JTable+bx]
	jmp	word ptr [bx]

Specials:
	cmp	ax,6DH			; get pointer to version ????
	je	SpVers
;	cmp	ax,024h			;***
;	je	GetInfo			;***

;;; rsd
	cmp	ax,024h
	je	GetInfo
;;; rsd

	cmp	ax,4DH
	jne	failcheck		; not a special
	lea	di,Customise+12
	mov	[bp-6],di		; return di
	jmp	SpSetES
SpVers:
	lea	di,VersionNo
	mov	[bp-6],di		; return di
SpSetES:
	mov	[bp-4],cs		; return es
	jmp	exit

;;; rsd
GetInfo:
	mov	ah,cs:VersionNo[0]
	mov	al,cs:VersionNo[1]
	mov	[bp-10],ax
	jmp	exit
;;; rsd

;GetInfo:		  		;***
;	mov	[bp-10],707h		;
;	jmp	exit			;

ResetAndStatus:	; Function 0 : Mouse Reset and Status
	pushf
	cli				; disable interrupts - Int Call & Cursor
	call	TidySoftCursor		; just in case - Hardware ???
	call	TidyGraphCursor		; just in case

	xor	ax,ax
	push	cs
	pop	es
	lea	di,StateInfo
	push	cx
	mov	cx,DataSize		; mouse state data size
	rep stosb			; zeroise everything
	pop	cx
	popf				; restore interrupts state

	call	CheckVideo		; what card is it now !
	
	xor	ax,ax
	mov	es,ax			; segment zero
	mov	al,es:[449H]
	and	al,7fH			; remove 'don't clear' bit
	call	SetVideoPars
;	mov	EGAPlanes,4		; +++++ temp +++++
	mov	CursorFlag,-1		; hidden
	mov	ax,ScreenMaxX		; reset values
	sar	ax,1
	mov	MouseX,ax
	mov	ax,ScreenMaxY
	sar	ax,1
	mov	MouseY,ax
	mov	TCScrMask,0FFFFH
	mov	TCCursMask,07700H
	mov	XMPRatio,8
	mov	YMPRatio,16
	mov	ax,ScreenMaxX
	dec	ax
	mov	XMaxCursor,ax
	mov	ax,ScreenMaxY
	dec	ax
	mov	YMaxCursor,ax
	mov	Bank,1			; 2 banks
	test	Hercules,-1
	jz	RasNotHerc
	mov	Bank,3			; 4 banks
RASNotHerc:

	push	cs
	pop	es	
	mov	cx,32
	lea	si,DefaultMouse
	lea	di,GraphicsCDef
	rep movsw

	call	MoveMouse		; Position the pointer

	mov	word ptr [bp-12],-1	; ax = -1 => installed
	mov	word ptr [bp-10],2	; bx = 2  => 2 buttons
exit2:
	jmp	exit
ShowCursor:	; Function 1 : Show Cursor
	mov	CondFlag,0		; no longer conditional off
	test	CursorFlag,-1		; check for zero
	jz	exit2			; do nothing if zero
	inc	CursorFlag
	jnz	exit2			; exit if not now zero
	test	XShift,-1		; if non-zero then in text mode
	jz	ShowGraphics
	test	TextCurs,-1		; zero is software cursor
	jnz	ShowHardware
	call	ShowSoftCursor
	jmp	exit
ShowHardware:
	mov	ch,byte ptr TCScrMask	; start line
	mov	cl,byte ptr TCCursMask	; stop line
	and	cx,0F0FH		; mask to keep in range.
	mov	ah,01			; set cursor type
	int	10H			; do it
	call	MoveCursor
	jmp	exit
ShowGraphics:
	call	ShowGraphCursor
	jmp	exit
HideCursor:	; Function 2 : Hide Cursor
	mov	CondFlag,0		; no longer just conditional
	call	DoHideCursor
	jmp	exit
	
ButtonAndPos:	; Function 3 : Get Button Status and Mouse Position
	mov	bx,MouseX
	mov	cl,XShift
	sar	bx,cl
	shl	bx,cl
	mov	dx,MouseY
	mov	cl,XShift
	sar	dx,cl
	shl	dx,cl
	mov	cx,bx			; Mouse X
	mov	bx,ButtonStatus
	mov	[bp-10],bx
	jmp	exit
SetMCurPos:	; Function 4 : Set Mouse Cursor Position
	mov	MouseX,cx
	mov	MouseY,dx
	call	MoveMouse
	jmp	exit

GetBPressInfo:	; Function 5 : Get Button Press Information
;	mov	cx,[bp-10]		; 0 or 1
;	and	cx,1			; Norton Editor passes 0x2000
;	mov	ax,1
;	shl	ax,cl
;	and	ax,ButtonStatus
	mov	ax,ButtonStatus
	mov	[bp-12],ax
	mov	cl,4
	mov	bx,[bp-10]		; M2%
	and	bx,1			; Norton Editor passes 0x2000
	shl	bx,cl			; * 16
	lea	bx,Buttons[bx]
	xor	dx,dx			; zero
	xchg	dx,PressCount[bx]	; load and zeroise
	mov	cx,PressX[bx]
	mov	bx,PressY[bx]
	xchg	bx,dx
	mov	[bp-10],bx
exit1:					; because exit too far away
	jmp	exit

GetBRelInfo:	; Function 6 : Get Button Release Information
;	mov	cx,[bp-10]		; 0 or 1
;	and	cx,1			; Norton Editor passes 0x2000
;	mov	ax,1
;	shl	ax,cl
;	and	ax,ButtonStatus
	mov	ax,ButtonStatus
	mov	[bp-12],ax
	mov	cl,4
	mov	bx,[bp-10]		; M2%
	and	bx,1			; Norton Editor passes 0x2000
	shl	bx,cl			; * 8
	lea	bx,Buttons[bx]
	xor	dx,dx			; zero
	xchg	dx,ReleaseCount[bx]	; load and zeroise
	mov	cx,ReleaseX[bx]
	mov	bx,ReleaseY[bx]
	xchg	bx,dx
	mov	[bp-10],bx
	jmp	exit

SetMInMaxHoriz:	; Function 7 : Set Minimum and Maximum Horizontal Cursor Pos.
	mov	XMinCursor,cx
	mov	XMaxCursor,dx
	jmp	exit

SetMInMaxVert:	; Function 8 : Set Minimum and Maximum Vertical Cursor Pos.
	mov	YMinCursor,cx
	mov	YMaxCursor,dx
	jmp	exit

SetGCurBlock:	; Function 9 : Set Graphics Cursor Block
	
	pushf
	cli				; disable interrupts - Int Call ...

	test	CursorFlag,-1
	jnz	SetGCNotVis1
	test	XShift,-1		; if non-zero then in text mode
	jnz	SetGCNotVis1
	call	TidyGraphCursor
SetGCNotVis1:
	mov	bx,[bp-10]		; M2%
	mov	HotSpotX,bx
	mov	HotSpotY,cx

	push	si
	push	di
	push	es		; part 1 of swap es to ds
	push	ds		; part 2 of swap es to ds
	mov	si,dx
	lea	di,GraphicsCDef
	pop	es		; part 3 of swap es to ds
	pop	ds		; part 2 of swap es to ds
	mov	cx,32
	cld			; to increment
	rep movsw	
	push	cs
	pop	ds		; restore ds
	pop	di		; etc 
	pop	si
	test	CursorFlag,-1
	jnz	SetGCNotVis2
	test	XShift,-1		; if non-zero then in text mode
	jnz	SetGCNotVis2
	call	ShowGraphCursor
SetGCNotVis2:
	popf
	jmp	exit
SetTextCur:	; Function 10 : Set Text Cursor
	pushf
	cli				; disable interrupts - Int Call ...
;	inc	InTimer
	test	CursorFlag,-1
	jnz	STNotVis1		; cursor not visible
	call	TidySoftCursor		; remove software cursor if it`s there
STNotVis1:
	mov	bx,[bp-10]		; M2%
	mov	TextCurs,bl		; cursor select
	mov	TCScrMask,cx		; screen mask / start line
	mov	TCCursMask,dx		; cursor mask / stop line
	test	CursorFlag,-1
	jnz	STNotVis2
	test	TextCurs,-1		; zero is software cursor
	jz	STSoft
	dec	CursorFlag
	mov	ch,byte ptr TCScrMask	; start line
	mov	cl,byte ptr TCCursMask	; stop line
	and	cx,0F0FH		; mask to keep in range.
	mov	ah,01			; set cursor type
	popf
	int	10H			; do it
	inc	CursorFlag
	jmp	STExit
STSoft:
	call	ShowSoftCursor		; show software cursor, if required
STNotVis2:
	popf
STExit:
;	dec	InTimer
	jmp	exit
ReadMotCounts:	; Function 11 : Read Mouse Motion Counters (taking the mickey)

; -----------------------------
	mov	cx,200H			; speed related bug ! on AT
xxLoop:
	loop	xxLoop
; -----------------------------

	xor	cx,cx
	xchg	MickeyX,cx		; read MickeyX and zeroise
	xor	dx,dx
	xchg	MickeyY,dx		; read MickeyY and zeroise
	jmp	exit
SetISMaskAndAdd:; function 12 : Set Interrupt Subroutine Call Mask and Address
	pushf
	cli				; disable interrupts
	mov	IntCallMask,cx
	mov	IntCallAddr,dx
	mov	IntCallSeg,es
	popf				; restore previous interrupts state
	jmp	exit
LightPenEmuOn:	; Function 13 : Light Pen Emulation Mode On
	xor	ax,ax			; zero = on
LightPenEmuOff:	; Function 14 : Light Pen Emulation Mode Off
	mov	LightPenMode,ax		; non-zero = off
	jmp	exit
SetMPRatio:	; Function 15 : Set Mickey/Pixel Ratio
	mov	XMPRatio,cx
	mov	YMPRatio,dx
	jmp	exit
CondOff:	; Function 16 : Conditional Off
	mov	CondFlag,-1		; set conditional off flag
	mov	bx,[bp-10]		; M2%
	mov	si,[bp-8]
	mov	di,[bp-6]
	mov	LeftX,cx
	mov	TopY,dx
	mov	RightX,si
	mov	BottomY,di
	call	DoCondOff
	jmp	exit
SetDoubleSpThres:;Function 19 : Set Double-Speed Threshold
	jmp	exit
SwapIntSubrs:	; Function 20 : Swap Interrupt Subroutines
	pushf
	cli					; disable interrupts
	xchg	dx,IntCallAddr
	xchg	cx,IntCallMask
	push	cx
	mov	cx,es
	xchg	cx,IntCallSeg
	mov	[bp-4],cx
	pop	cx
	popf					; restore interrupts state
	jmp	exit
	
GetMDSSReq:	; Function 21 : Get Mouse Driver State Storage Requirements
	mov	bx,[DataSize]
	mov	[bp-10],bx
	jmp	exit
SaveMDS:	; Function 22 : Save Mouse Driver State
	pushf
	cli
	mov	di,dx
	lea	si,StateInfo
	mov	cx,DataSize
	cld
	rep movsb
	popf
	jmp	exit
RestoreMDS:		;	23
	pushf
	cli
	mov	cx,DataSize
	push	ds			; exchange es and ds
	mov	ax,es
	mov	ds,ax
	pop	es
	mov	si,dx			; ds:si = source in user area
	lea	di,StateInfo		; es:di = target in mouse area
	cld
	rep movsb
	popf
	jmp	exit
SetCRTPageNo:	; Function 29 : Set CRT Page Number
	mov	bx,[bp-10]		; M2%
	mov	CRTPageNumber,bx
	jmp	exit
GetCRTPageNo:	; Function 30 : Get CRT Page Number
	mov	bx,CRTPageNumber
	mov	[bp-10],bx		; M2%
	jmp	exit
	
exit:
	pop	ax
	pop	bx
	pop	si
	pop	di

	call	Trace

	pop	es
	pop	ds
	mov	sp,bp
	pop	bp
	iret	


JTable:
	dw	ResetAndStatus		;	0
	dw	ShowCursor		;	1
	dw	HideCursor		;	2
	dw	ButtonAndPos		;	3
	dw	SetMCurPos		;	4
	dw	GetBPressInfo		;	5
	dw	GetBRelInfo		;	6
	dw	SetMInMaxHoriz		;	7
	dw	SetMInMaxVert		;	8
	dw	SetGCurBlock		;	9
	dw	SetTextCur		;	10
	dw	ReadMotCounts		;	11
	dw	SetISMaskAndAdd		;	12
	dw	LightPenEmuOn		;	13
	dw	LightPenEmuOff		;	14
	dw	SetMPRatio		;	15
	dw	CondOff			;	16
	dw	exit			;	17
	dw	exit			;	18
	dw	SetDoubleSpThres	;	19
	dw	SwapIntSubrs		;	20
	dw	GetMDSSReq		;	21
	dw	SaveMDS			;	22
	dw	RestoreMDS		;	23
	dw	exit			;	24
	dw	exit			;	25
	dw	exit			;	26
	dw	exit			;	27
	dw	exit			;	28
	dw	SetCRTPageNo		;	29
	dw	GetCRTPageNo		;	30


MouseDriver	ENDP

	include	EGALib.asm
	include	Init.asm

cseg	ENDS
END	Start
