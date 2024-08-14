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

UseSprite	db	0		; default is graphics on the PC side
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

DualPortArea	dw	8,0E000H	; Dual Port Ram Address
ParamArea	dw	8,0E000H	; Dual Port Ram Address

OldStackSeg	dw	0
OldStackPtr	dw	0
MyStack		dw	128 dup(?)
MyStackTop	equ	$


AmigaPCX		equ	0		; X Increment from Amiga
AmigaPCY		equ	2		; Y Increment from Amiga
AmigaPCLeftP		equ	4		; Left Pressed count f. A.
AmigaPCRightP		equ	6		; Right Pressed count f. A.
AmigaPCLeftR		equ	8		; Left Released count f. A.
AmigaPCRightR		equ	10		; Right Released count f. A.
AmigaPCStatus		equ	12		; Buttons Status f.A.
;PCAmigaX		equ	14		; X Pos as seen by PC for AMiga
;PCAmigaY		equ	16		; Y Pos as seen by PC for AMiga
PCVideoMode		equ	18		; Video mode as form int 10H/00

JIntNum			equ	17    
J_GET_SERVICE		equ	0
J_GET_BASE		equ	1
J_ALLOC_MEM		equ	2
J_FREE_MEM		equ	3
J_SET_PARAM		equ	4
J_SET_SERVICE		equ	5
J_STOP_SERVICE		equ	6
J_CALL_AMIGA		equ	7
J_WAIT_AMIGA		equ	8
J_CHECK_AMIGA		equ	9

J_NO_SERVICE		equ	0FFH
J_PENDING		equ	0
J_FINISHED		equ	1

;	Amiga parameters

VideoMode		equ	0
MouseImage		equ	1
MouseOn			equ	2
MouseOff		equ	3
MouseMove		equ	4
StandardImage		equ	5

DPSize		equ	8		; 8 words 16 bytes

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

GraphicsSave	db	48 dup(0)		; 3 x 16

DataSize	dw	$-DataStart

VersionNo	db	6,3		; emulating 6.03
CRMsg		dw	offset Customise
		dw	0,0
		dw	$+2
		dw	0		; end of chain
SaveSize	dw	0		; move DataSize into this
		dw	offset StateInfo

DPCopyArea	dw	DPSize dup(0)

ScreenMode	db	0
ScreenMaxX	dw	640
ScreenMaxY	dw	200

InTimer		db	0
;****************************************************************************
;*
;* MouseInt
;*
;****************************************************************************
MouseInt	PROC	NEAR
	test	cs:IntCallMask,-1
	jz	NextTimer

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

;	push	ss
;	pop	ax
;	mov	OldStackSeg,ax
;	mov	OldStackPtr,sp
;	push	cs
;	pop	ss
;	mov	sp,offset MyStackTop

	call	MouseRead		; read Mouse info from DPRam
	mov	ax,NewMask
	or	ax,ax
	jz	NoChange
	and	ax,IntCallMask		; does user want this one ?
	jz	NoChange
	mov	bx,ButtonStatus
	mov	cx,MouseX
	mov	dx,MouseY
	mov	di,MickeyX
	mov	si,MickeyY
	pushf
	sti
	call	dword ptr IntCallAddr
	popf
	mov	cs:NewMask,0
NoChange:
	
;	mov	ax,OldStackSeg
;	push	ax
;	pop	ss
;	mov	sp,OldStackPtr

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

	mov	cs:ScreenSegment,0B800H		; CGA address
	cmp	ax,1
	jg	VideoNot01
	mov	cs:XShift,4
	mov	cs:YShift,3
	jmp	TellVideo
VideoNot01:
	cmp	ax,3
	jg	VideoNot23
VideoText8x8:
	mov	cs:XShift,3
	mov	cs:YShift,3
	jmp	TellVideo
VideoNot23:
	cmp	ax,7
	jne	VideoGraphics
	mov	cs:ScreenSegment,0B000H		; mono address
	jmp	VideoText8x8
VideoGraphics:
	mov	cs:XShift,0
	mov	cs:YShift,0
TellVideo:
	sti
	mov	ax,VideoMode
	call	Janus
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
	les	di,cs:dword ptr DualPortArea
	cmp	es:PCVideoMode[di],ax
	jz	SameVideo
	mov	es:PCVideoMode[di],ax
	call	SetVideoPars
	jmp	VideoRestore
SameVideo:
	pushf
	sti
	mov	ax,MouseOff
	call	Janus
	popf
VideoRestore:
	pop	es
	pop	di
	pop	ax
	iret

VideoContinue:
	jmp	cs:dword ptr VideoOffset

Video		ENDP


;****************************************************************************
;*
;* Janus
;*
;****************************************************************************
Janus		PROC	NEAR
	test	cs:UseSprite,-1
	jz	NoJanus
	push	ax
JLoop:
	mov	ax,(J_CHECK_AMIGA *256) + JIntNum
	int	0BH			; check if Amiga service busy
	or	al,al
	je	JLoop
	pop	ax			; get parameter back
	push	di
	push	es
	les	di,cs:dword ptr ParamArea
	mov	es:2[di],ax		; store for the Amiga
	pop	es
	pop	di
	mov	ax,(J_CALL_AMIGA * 256) + JIntNum
	push	bx
	mov	bx,-1
	int	0BH
	pop	bx
NoJanus:
	ret
Janus		ENDP

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
Trace		PROC	NEAR
	test	cs:DoTrace,1
	jz	NoTrace
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
	mov	ax,MouseOff
	call	Janus
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

;****************************************************************************
;*
;* ShowGraphCursor
;*
;****************************************************************************
ShowGraphCursor	PROC	NEAR

	test	UseSprite,-1
	jz	ShowGraph
	ret
ShowGraph:
	pushf
	cli

	push	ax
	push	bx
	push	cx
	push	dx
	push	es
	push	di
	push	ds
	push	si

	mov	ax,MouseX
	sub	ax,HotSpotX
	cmp	ax,ScreenMaxX
	jae	GCQuit
	mov	GraphCX,ax
	sar	ax,1
	sar	ax,1
	sar	ax,1			; byte boundary
	mov	di,ax
	mov	cl,4
	mov	ax,MouseY
	sub	ax,HotSpotY
	cmp	ax,ScreenMaxY
	jb	GCInRange
GCQuit:
	jmp	GCNoMove
GCInRange:
	mov	GraphCY,ax
	test	ax,1
	jz	EvenLine
	add	di,2000H		; odd line
EvenLine:
	sar	ax,1			; /2 ( drops odd bit )
	shl	ax,cl			; *16
	add	di,ax
	shl	ax,1
	shl	ax,1			; *64
	add	di,ax			; di = MouseX + (MouseY/2)*80;
	mov	es,ScreenSegment	; es:di points at cursor position
	mov	ScreenOffset,di		; save for Save/TidyGraphCursor
	call	SaveGraphCursor		; save region under cursor
;	lds	si,dword ptr DualPortArea
;	add	si,32			; MouseData
	lea	si,GraphicsCDef
	mov	cx,10H			; 16 lines of mouse image
	mov	ax,cs:GraphCY		; check for end of screen
	sub	ax,184
	js	XCheck
	sub	cx,ax
	js	GCOutOfRange		; jump if negative
	jnz	XCheck			; jump if positive and > 0
GCOutOfRange:
	jmp	GCNoMove
XCheck:
	push	cx
	mov	cx,3
	mov	bx,cs:GraphCX
	ror	bx,cl			; bl = byte number
	mov	dl,bl
	rol	bh,cl			; bh = bit offset
	and	bh,7			; in case negative
	sub	bl,77
	js	GCMove
	sub	cl,bl			; cx = no of bytes affected
GCMove:
	mov	bl,cl
	pop	cx
	mov	ax,cs:GraphCY		; line number
	cld
NextLine:
	push	dx
	push	cx
	push	bx
	push	ax
	or	ax,ax			; check line number
	js	RowLoopEnd		; skip negative lines !!
	mov	cl,bh			; cl = shift
	xor	bh,bh			; bx = count
	mov	ch,dl			; ch = start byte

; byte 0
	
	mov	ah,-1
	mov	al,1[si]		; ax = screen mask byte 0 !
	ror	ax,cl
	and	al,es:0[di]		; and with screen contents
	xor	dh,dh
	mov	dl,33[si]		; ax = cursor mask byte 0 !
	ror	dx,cl
	xor	al,dl
	or	ch,ch
	js	GCNoByte0		; negative - skip byte 0
	mov	es:0[di],al		; store in screen
GCNoByte0:	
	dec	bx
	jz	RowLoopEnd

; byte 1
	
	rol	ah,cl			; shift back
	mov	al,0[si]		; ax = screen mask byte 1 !
	ror	ax,cl
	and	al,es:1[di]		; and with screen contents
	rol	dh,cl			; shift back
	mov	dl,32[si]		; ax = cursor mask byte 1 !
	ror	dx,cl
	xor	al,dl
	inc	ch
	js	GCNoByte1		; still negative - skip byte 1
	mov	es:1[di],al		; store in screen
GCNoByte1:	
	dec	bx
	jz	RowLoopEnd

; byte 2
	
	and	ah,es:2[di]		; and with screen contents
	xor	ah,dh
	inc	ch
	js	GCNoByte2		; still negative - skip byte 2
	mov	es:2[di],ah		; store in screen
GCNoByte2:			; needed ?????
	
RowLoopEnd:
	add	si,2			; cursor image pointer

;	test	di,2000H		; if set then is odd line
;	jz	MEvenLine
	pop	ax			; line number
	test	ax,1			; was it odd ?
	jz	MEvenLine		; no - even !!
	add	di,50H-2000H
	jmp	LineLoopEnd
MEvenLine:
	add	di,2000H
LineLoopEnd:
	inc	ax			; next line number
	pop	bx
	pop	cx
	pop	dx
	loop	NextLine
	mov	cs:GraphValid,-1
GCNoMove:
	pop	si
	pop	ds
	pop	di
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	
	popf

NoPCGraph:
	ret

ShowGraphCursor	ENDP

;****************************************************************************
;*
;* TidyGraphCursor
;*
;****************************************************************************
TidyGraphCursor	PROC	NEAR

	pushf
	cli

	test	UseSprite,-1
	jnz	TGCexit

	test	GraphValid,-1
	jz	TGCexit
	mov	GraphValid,0

	push	ax
	push	bx
	push	cx
	push	dx
	push	es
	push	di
	push	si

	les	di,dword ptr ScreenOffset ; es:di points at cursor position
	lea	si,GraphicsSave
	mov	cx,10H			; 16 lines of mouse image
	mov	ax,GraphCY		; check for end of screen
	mov	bx,ax
	sub	ax,184
	js	TGCNext
	sub	cx,ax
	cld
TGCNext:
	push	cx
	push	bx
	or	bx,bx
	js	TGCSkip
	mov	cx,3
	mov	dx,GraphCX		; pixel x
	sar	dx,cl			; byte x
	xor	bx,bx
TGCNextByte:
	or	dx,dx
	js	TGCByteSkip
	mov	ah,SaveMask[bx]		; valid bits in saved byte
	not	ah			; valid bits in screen byte
	mov	al,es:[di+bx]		; screen byte
	and	al,ah			; valid bits
	or	al,[si+bx]		; add in the saved bits
	mov	es:[di+bx],al		; store the byte
TGCByteSkip:
	inc	bx
	inc	dx
	loop	TGCNextByte
TGCSkip:
	pop	bx
	pop	cx
;	test	di,2000H		; if set then is odd line
	test	bx,1			; if set then is odd line
	jz	TGCEvenLine
	add	di,50H-2000H
	jmp	TGCLoopEnd
TGCEvenLine:
	add	di,2000H
TGCLoopEnd:
	inc	bx
	add	si,3
	loop	TGCNext

	pop	si
	pop	di
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
TGCexit:
	popf
	ret

TidyGraphCursor	ENDP

;****************************************************************************
;*
;* SaveGraphCursor
;*
;****************************************************************************
SaveGraphCursor	PROC	NEAR

;	es:di points at the screen block

	test	UseSprite,-1
	jnz	SGCexit

	push	ax
	push	bx
	push	cx
	push	es
	push	di
	push	ds
	push	si

	mov	cx,GraphCX		; x co-ordinate
	and	cx,7			; bit offset
	mov	ax,-1			; all bits set
	shr	ax,cl			; mask for bytes 0 and 1
	mov	SaveMask,ah
	mov	SaveMask+1,al
	not	ah			; mask for byte 2
	mov	SaveMask+2,ah
	lea	si,GraphicsSave		; ds:si points at save area
	mov	cx,10H			; 16 lines of mouse image
	mov	ax,GraphCY		; check for end of screen
	mov	bx,ax			; save line number
	sub	ax,184
	js	SGCNext
	sub	cx,ax
	cld
SGCNext:
	push	cx
	push	bx
	or	bx,bx
	js	SGCSkip
	mov	cx,3
	mov	dx,GraphCX		; pixel x
	sar	dx,cl			; byte x
	xor	bx,bx
SGCNextByte:
	xor	al,al			; clear for Skip
	or	dx,dx
	js	SGCByteSkip
	mov	al,es:[di+bx]		; get screen buffer byte
	and	al,SaveMask[bx]		; remove invalid bits
SGCByteSkip:
	mov	[si+bx],al		; store the byte
	inc	bx
	inc	dx
	loop	SGCNextByte
SGCSkip:
	pop	bx
	pop	cx
	test	bx,1			; if set then is odd line
	jz	SGCEvenLine
	add	di,50H-2000H
	jmp	SGCLoopEnd
SGCEvenLine:
	add	di,2000H
SGCLoopEnd:
	inc	bx
	add	si,3
	loop	SGCNext

	pop	si
	pop	ds
	pop	di
	pop	es
	pop	cx
	pop	bx
	pop	ax
SGCexit:
	ret

SaveGraphCursor	ENDP

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

	les	di,dword ptr ParamArea
Lock1:
	lock inc byte ptr es:4[di]
	js	MRGotLock
	jmp	MRNoMove		; exit no change if locked
MRGotLock:

	mov	ax,ds				; set es to ds
	mov	es,ax
	lea	di,DPCopyArea			; di points at copy area
	lds	si,dword ptr DualPortArea	; ds:si - DP area
	mov	cx,DPSize			; copy DPSize words
	rep movsw

	mov	ax,cs				; restore ds
	mov	ds,ax
	les	di,dword ptr DualPortArea
					; zeroise 6 words
	mov	cx,6
	xor	ax,ax
	rep stosw

	les	di,dword ptr ParamArea
Lock2:
	lock mov byte ptr es:4[di],07fh	; unlock


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
	sub	ax,HotSpotX
;	mov	PCAmigaX[si],ax
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
	sub	ax,HotSpotY
;	mov	PCAmigaY[si],ax
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

	test	cs:UseSprite,-1
	jz	MMoveExit

	push	es
	push	di

	les	di,dword ptr DualPortArea
	mov	ax,MouseX
	sub	ax,HotSpotX
;	mov	es:PCAmigaX[di],ax	
	mov	ax,MouseY
	sub	ax,HotSpotY
;	mov	es:PCAmigaY[di],ax
	mov	ax,MouseMove
	call	Janus

	pop	di
	pop	es
MMoveExit:
	ret

MoveMouse	ENDP

CopyRight:
	db	"===    Copyright 1988 Bill Holohan   ==="
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

	mov	es,ax			; segment zero
	mov	al,es:[449H]
	call	SetVideoPars
	mov	ax,MouseOff		; hide cursor
	call	Janus
	mov	CursorFlag,-1		; hidden
	mov	ax,ScreenMaxX		; reset values
	sar	ax,1
	mov	MouseX,ax
	mov	ax,ScreenMaxY
	sar	ax,1
	mov	MouseY,ax
	mov	ax,StandardImage	; do Janus call now to minimise delay
	call	Janus			; on the MouseMove below
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
	mov	ax,MouseOn
	call	Janus
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
	mov	cx,[bp-10]		; 0 or 1
	mov	ax,1
	shl	ax,cl
	and	ax,ButtonStatus
	mov	[bp-12],ax
	mov	cl,4
	mov	bx,[bp-10]		; M2%
	shl	bx,cl			; * 8
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
	mov	cx,[bp-10]		; 0 or 1
	mov	ax,1
	shl	ax,cl
	and	ax,ButtonStatus
	mov	[bp-12],ax
	mov	cl,4
	mov	bx,[bp-10]		; M2%
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
;	les	di,dword ptr DualPortArea
;	add	di,32	
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
	mov	ax,MouseImage
	call	Janus
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
	xor	cx,cx
	xchg	MickeyX,cx
	xor	dx,dx
	xchg	MickeyY,dx
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
	cli
	mov	di,dx
	lea	si,StateInfo
	mov	cx,DataSize
	cld
	rep movsb
	sti
	jmp	exit
RestoreMDS:		;	23
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
	sti
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
EndOfResident:


;****************************************************************************
;*
;* GetParams
;*
;****************************************************************************
GetParams	PROC	NEAR

	mov	cl,ParamSize
	xor	ch,ch
	jcxz	NoParams
	lea	si,Params
PLoop:
	lodsb
;	cmp	al," "
;	je	PSkip
	cmp	al,"-"			; start of parameter
	jne	PSkip
	dec	cx
	jcxz	NoParams
	lodsb
	cmp	al,"i"
	jne	NotInt
	mov	al,TimerIntNo
	xchg	al,OtherIntNo
	mov	TimerIntNo,al
	jmp	PSkip
NotInt:
	cmp	al,"x"
	jne	NotXLock
	mov	al,90H			; NOP
	mov	byte ptr Lock1,al
	mov	byte ptr Lock2,al
	jmp	PSkip
	cmp	al,"c"
	jne	NoParams
NotXLock:
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
	jmp	NoParams
PSkip:
	loop	PLoop
NoParams:
	ret

GetParams	ENDP

NoSMsg	db	"AMouse not active on Amiga",0DH,0AH,"$"
VersNo	db	"AMouse version 1.2 installed",0DH,0AH,"$"
Passed	db	"Parameters passed to previously installed driver",0DH,0AH,"$"
Already	db	"A mouse driver is already installed",0DH,0AH,"$"

NoService:
	mov	ah,9
	mov	dx,offset NoSMsg
	int	21H
	mov	ax,4C02H
	int	21H			; terminate with return code

AlreadyIn:
	mov	cx,40			; CopyRight length
	lea	di,CopyRight
	mov	si,di
	rep cmpsb
	jcxz	ItsMine
	mov	ah,9
	mov	dx,offset Already
	int	21H
	mov	ax,4C03H
	int	21H			; terminate with return code
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
	mov	al,byte ptr cs:Lock1		; set lock on or off
	mov	byte ptr Lock1,al
	mov	byte ptr Lock2,al
	mov	ah,9
	mov	dx,offset Passed
	int	21H
	mov	ax,4C00H
	int	21H			; terminate with return code
	
	

main:
	push	cs
	pop	ds

	call	GetParams

	mov	ax,DataSize
	mov	SaveSize,ax
	mov	cx,3			; Janus problem - try 3 times
					; before giving up on Amiga
RetryJanus:
	push	cx
	mov	ax,(J_GET_BASE*256)+JIntNum	; get base for service
	int	0BH			; do it
;	or	al,al
;	jnz	NoService		; one check (at time of writing useless)
	mov	ax,di			; Parameter Offset
	or	ax,ax			; the real check
	jns	GotService		; Offset should be positive
	pop	cx
	loop	RetryJanus
	jmp	NoService
GotService:
	mov	ax,es:0[di]		; Offset of Buffer area
	mov	DualPortArea,ax
	mov	DualPortArea+2,dx	; segment of Buffer area
	mov	ParamArea,di
	mov	ParamArea+2,es		; parameter area address

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

	les	di,dword ptr DualPortArea
	mov	cx,DPSize
	xor	ax,ax
	rep stosw			; zeroise Dual Port Ram variables.
	mov	es,ax			; segement zero
	mov	al,es:[449H]
	les	di,dword ptr DualPortArea
	mov	es:PCVideoMode[di],ax
	call	SetVideoPars		; also tells Amiga what video mode
	mov	ah,9
	mov	dx,offset VersNo
	int	21H
	mov	ax,3100H
	lea	dx,EndOfResident+10FH	; round up + program header
	mov	cl,4
	sar	dx,cl
	int	21H			; terminate but stay resident

cseg	ENDS
END	Start
