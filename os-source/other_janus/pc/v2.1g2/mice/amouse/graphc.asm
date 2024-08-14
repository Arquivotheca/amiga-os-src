
SaveInc	equ	3		; Variable for testing

;****************************************************************************
;*
;*	Increment
;*
;****************************************************************************

;	input	ax	line number

Increment	PROC	NEAR

	push	ax
	and	al,cs:Bank		; Bank number
	cmp	al,cs:Bank		; last bank ?
	jne	IncNextBank		; no - add bank offset
	add	di,cs:OddOffset		; yes - subtract n*bank offset, add line offs.
	jmp	IncExit
IncNextBank:
	add	di,cs:EvenOffset
IncExit:
	pop	ax
	ret

Increment	ENDP

;****************************************************************************
;*
;*	DoCGACursor	-	CGA here means 'not EGA'
;*
;****************************************************************************

;	input	ax	line number for mouse image start
;		bl	byte width of area to receive image
;		bh	bit offset of start of image within first byte
;		dl	first byte of area to receive image (rel. to line)
;		cx	number of lines of image to draw
;		es:di	address of top,left of area to receive image
;		ds:si	address of mouse image definition

DoCGACursor	PROC	NEAR

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

	pop	ax			; line number

	call	Increment		; increment DI

	inc	ax			; next line number
	pop	bx
	pop	cx
	pop	dx
	loop	NextLine
	ret

DoCGACursor	ENDP


;****************************************************************************
;*
;*	DoEGACursor
;*
;****************************************************************************

;	input	ax	line number for mouse image start
;		bl	byte width of area to receive image
;		bh	bit offset of start of image within first byte
;		dl	first byte of area to receive image (rel. to line)
;		cx	number of lines of image to draw (ch = 0)
;		es:di	address of top,left of area to receive image
;		ds:si	address of mouse image definition


BytesToGo	dw	0
SaveArea	dw	7f80H		; needs at least 96 bytes
SaveTemp	dw	0

DoEGACursor	PROC	NEAR

	push	ax
	push	dx

	mov	dx,3ceh
	mov	ax,0ff08H		; pointer 8 data ff - bit mask
	out	dx,ax

	mov	ax,00F02H		; map mask select register
	mov	dx,3c4h
	out	dx,ax			; write to all planes

	mov	ax,SaveArea		; start of 96 (*4 planes) bytes of ..
	mov	SaveTemp,ax		;  ..  'unused' EGA RAM

	pop	dx			; restore
	pop	ax			; restore

EGA_NextLine:
	push	dx
	push	cx
	push	bx
	push	ax
	or	ax,ax			; check line number
	jns	EGA_RowLoopStart

	jmp	EGA_RowLoopEnd		; skip negative lines !!

EGA_RowLoopStart:
	mov	cl,bh			; cl = shift
	xor	bh,bh			; bx = count (bytes per line)
	mov	BytesToGo,bx
	mov	ch,dl			; ch = start byte
	mov	dx,3ceh
; byte 0
	
	mov	ah,-1
	mov	al,1[si]		; ax = screen mask byte 0 !
	ror	ax,cl			; ... shifted to align
	mov	bl,33[si]		; bx = cursor mask byte 0 !
	ror	bx,cl			; ... shifted to align
	or	ch,ch
	js	EGA_GCNoByte0		; negative - skip byte 0

	push	bx			; save cursor mask

;	read original from screen into latches and also copy to save area

	push	ax			; save screen maks
	mov	al,es:0[di]		; load the latches
	mov	bx,SaveTemp		; pointer to save/work area for this line
	mov	ax,0003H		; write normally
	out	dx,ax
	mov	ax,0105H		; write mode 1
	out	dx,ax
	mov	es:0[bx],al		; store original in save area
	mov	ax,0005H		; write mode 0
	out	dx,ax

;	and the screen mask with the contents of the latches and write to work

	mov	ax,0803H		; do AND when writing
	out	dx,ax
	pop	ax			; restore screen mask
	mov	es:32[bx],al

;	load the latches from the work area, xor cursor mask with this and
;	write to the screen

	push	ax
	mov	al,es:32[bx]		; load the latches
	mov	ax,1803H		; do XOR when writing
	out	dx,ax
	pop	ax
	pop	bx			; restore cursor mask
	mov	es:0[di],bl
EGA_GCNoByte0:	
	dec	cs:BytesToGo
	jz	EGA_SteppingStone

; byte 1
	
	rol	ah,cl			; shift back
	mov	al,0[si]		; ax = screen mask byte 1 !
	ror	ax,cl
	rol	bh,cl			; shift back
	mov	bl,32[si]		; bx = cursor mask byte 1 !
	ror	bx,cl
	inc	ch
	js	EGA_GCNoByte1		; still negative - skip byte 1

	push	bx			; save cursor mask

;	read original from screen into latches and also copy to save area

	push	ax
	mov	al,es:1[di]		; load the latches
	mov	bx,SaveTemp		; pointer to save/work area for this line
	mov	ax,0003H		; write normally
	out	dx,ax
	mov	ax,0105H		; write mode 1
	out	dx,ax
	mov	es:1[bx],al		; store original in save area
	mov	ax,0005H		; write mode 0
	out	dx,ax

;	and the screen mask with the contents of the latches and write to work

	mov	ax,0803H		; do AND when writing
	out	dx,ax
	pop	ax
	mov	es:33[bx],al

;	load the latches from the work area, xor cursor mask with this and
;	write to the screen

	push	ax
	mov	al,es:33[bx]		; load the latches
	mov	ax,1803H		; do XOR when writing
	out	dx,ax
	pop	ax
	pop	bx			; restore cursor mask
	mov	es:1[di],bl

EGA_GCNoByte1:	
	dec	cs:BytesToGo
EGA_SteppingStone:
	jz	EGA_RowLoopEnd

; byte 2
	
	inc	ch
	js	EGA_GCNoByte2		; still negative - skip byte 2


	push	bx			; save cursor mask

;	read original from screen into latches and also copy to save area

	push	ax
	mov	al,es:2[di]		; load the latches
	mov	bx,SaveTemp		; pointer to save/work area for this line
	mov	ax,0003H		; write normally
	out	dx,ax
	mov	ax,0105H		; write mode 1
	out	dx,ax
	mov	es:2[bx],al		; store original in save area
	mov	ax,0005H		; write mode 0
	out	dx,ax

;	and the screen mask with the contents of the latches and write to work

	mov	ax,0803H		; do AND when writing
	out	dx,ax
	pop	ax
	mov	es:34[bx],ah

;	load the latches from the work area, xor cursor mask with this and
;	write to the screen

	push	ax
	mov	al,es:34[bx]		; load the latches
	mov	ax,1803H		; do XOR when writing
	out	dx,ax
	pop	ax
	pop	bx			; restore cursor mask
	mov	es:2[di],bh


EGA_GCNoByte2:			; needed ?????
	
EGA_RowLoopEnd:
	add	si,2			; cursor image pointer
	add	SaveTemp,SaveInc		; save/work pointer

	pop	ax			; line number

	call	Increment		; increment DI

	inc	ax			; next line number
	pop	bx
	pop	cx
	pop	dx
	loop	EGA_Loop

;	mov	dx,3ceh
;	mov	ax,0003H		; do straight write
;	out	dx,ax
	
	ret

EGA_Loop:
	jmp	EGA_NextLine

DoEGACursor	ENDP

;****************************************************************************
;*
;* ShowGraphCursor
;*
;****************************************************************************
ShowGraphCursor	PROC	NEAR

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

	xor	ax,ax
	mov	es,ax
	mov	ax,es:[44ch]		; screen page size
	mul	CRTPageNumber		; mouse page offset (need != 0:44e)
	mov	di,ax			; base for ScreenOffset
	mov	ax,MouseX
	sub	ax,HotSpotX
	cmp	ax,ScreenMaxX
	jae	GCQuit
	mov	GraphCX,ax
	sar	ax,1
	sar	ax,1
	sar	ax,1			; byte boundary
	add	di,ax
	mov	cl,4
	mov	ax,MouseY
	sub	ax,HotSpotY
	cmp	ax,ScreenMaxY
	jb	GCInRange
GCQuit:
	jmp	GCNoMove
GCInRange:
	mov	GraphCY,ax
	test	ax,1			; even /odd line ?
	jz	EvenLine
	add	di,EvenOffset
EvenLine:
	test	EGA,-1
	jnz	GCNoDiv
	sar	ax,1			; /2 ( drops odd bit )
	test	Hercules,-1		; 3 Banks = Hercules
	jz	GCNoDiv
	test	ax,1			; 2nd or 3rd Bank ?
	jz	GCHBank01
	add	di,EvenOffset
	add	di,EvenOffset		; add 4000H
GCHBank01:
	sar	ax,1			; /4
GCNoDiv:
	shl	ax,cl			; *16
	add	di,ax
	shl	ax,1
	shl	ax,1			; *64
	add	di,ax			; di = (MouseX/8) + (MouseY/2)*80;
					; EGA  (MouseX/8) + MouseY*80
	test	Hercules,-1
	jz	GCNotHerc
	dec	cl			; 3
	shr	ax,cl			; *8
	add	di,ax
	dec	cl			; 2
	shr	ax,cl			; *2
	add	di,ax			; Hercules  (MouseX/8) + (MouseY/4)*90
GCNotHerc:
	mov	es,ScreenSegment	; es:di points at cursor position
	mov	ScreenOffset,di		; save for Save/TidyGraphCursor
	call	SaveGraphCursor		; save region under cursor

	call	SetMouse		; set EGA registers, if EGA

	lea	si,GraphicsCDef
	mov	cx,10H			; 16 lines of mouse image
	mov	ax,cs:GraphCY		; check for end of screen
	sub	ax,cs:LastFullMPos
	js	XCheck
	sub	cx,ax			; only room for ... lines
	js	GCOutOfRange		; jump if negative
	jnz	XCheck			; jump if positive and > 0
GCOutOfRange:
	jmp	GCResetUser
XCheck:
	push	cx			; save mouse image line count
	mov	cx,3
	mov	bx,cs:GraphCX
	ror	bx,cl			; bl = byte number
	mov	dl,bl
	rol	bh,cl			; bh = bit offset
	and	bh,7			; in case negative
	sub	bl,77			; 77 = width-3 -- check for right edge
	js	GCMove
	sub	cl,bl			; cx = no of bytes affected
GCMove:
	mov	bl,cl
	pop	cx			; number of lines in mouse image
	mov	ax,cs:GraphCY		; line number
	mov	cs:GraphValid,-1	; point of no return

	test	cs:EGA,-1
	jz	GCNotEGA
	call	DoEGACursor
	jmp	GCResetUser
GCNotEGA:
	call	DoCGACursor

GCResetUser:
	mov	ax,cs
	mov	ds,ax
	call	ResetUser		; restore EGA registers, if EGA

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
;* TidyCGA	-	CGA here means 'not EGA'
;*
;****************************************************************************
;
;	input	bx	line number
;		cx	number of lines of mouse image
;		es:di	pointer to screen area
;		ds:si	saved original contents

TidyCGA		Proc	Near

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
	pop	ax
	pop	cx
	call	Increment
	mov	bx,ax
	inc	bx
	add	si,3		; save area pointer
	loop	TGCNext

	ret

TidyCGA		EndP

;****************************************************************************
;*
;* TidyEGA
;*
;****************************************************************************
;
;	input	bx	line number
;		cx	number of lines of mouse image
;		es:di	pointer to screen area
;		ds:si	saved original contents (non EGA only )

TidyEGA		Proc	Near

	push	ds

	call	SaveUser		; save EGA Lib user values

	push	bx			; save line number
	push	cx			; save mouse image line count
	mov	cx,3
	mov	bx,GraphCX		; pixel X
	ror	bx,cl			; bl = byte number
	mov	al,bl
	rol	bh,cl			; bh = bit offset
	and	bh,7			; in case negative
	sub	al,77			; 77 = width-3 -- check for right edge
	js	TEGA_Count
	sub	cl,al			; cx = no of bytes affected
TEGA_Count:
	mov	bh,cl
	pop	cx			; number of lines in mouse image

	mov	si,SaveArea		; start of 96 (*4 planes) bytes of ..
					;  ..  'unused' EGA RAM

	mov	ax,es
	mov	ds,ax			; point ds at screen buffer

	mov	ax,00F02H		; map mask select register
	mov	dx,3c4h
	out	dx,ax			; write to all planes

	mov	dx,3ceh
	mov	ax,0ff08H		; pointer 8 data ff - bit mask
	out	dx,ax

	mov	ax,0105H		; write mode 1
	out	dx,ax

	pop	ax			; line number

TEGA_NextLine:
	push	ax			; save line number
	push	bx			; save byte count/number
	or	ax,ax			; check line number
	js	TEGA_RowLoopEnd		; skip negative lines !!

; byte 0
	
	or	bl,bl			; byte no. on line
	js	TEGA_NoByte0		; negative - skip byte 0

;	read original from save area into latches and copy to screen

	mov	al,0[si]		; load the latches
	mov	0[di],al		; store original back in screen

TEGA_NoByte0:	
	dec	bh			; bytes to do on line
	jz	TEGA_RowLoopEnd

; byte 1
	
	inc	bl
	js	TEGA_NoByte1		; still negative - skip byte 1

;	read original from screen into latches and copy to screen

	mov	al,1[si]		; load the latches
	mov	1[di],al		; store original back in screen

TEGA_NoByte1:	
	dec	bh			; bytes to do on line
	jz	TEGA_RowLoopEnd

; byte 2
	
	inc	bl
	js	TEGA_NoByte2		; still negative - skip byte 2

;	read original from screen into latches and copy to screen

	mov	al,2[si]		; load the latches
	mov	2[di],al		; store original back in screen

TEGA_NoByte2:			; needed ?????
	
TEGA_RowLoopEnd:
	add	si,SaveInc		; next line in save area

	pop	bx			; byte count/number
	pop	ax			; line number
	call	Increment		; update di to point ot next line
	inc	ax			; next line number
	loop	TEGA_NextLine

	pop	ds	
	call	ResetUser		; save EGA Lib user values

	ret

TidyEGA		EndP

;****************************************************************************
;*
;* TidyGraphCursor
;*
;****************************************************************************
TidyGraphCursor	PROC	NEAR

	pushf
	cli

	test	GraphValid,-1		; got something to tidy ?
	jnz	TGCCont			; yes
	jmp	TGCexit			; no
TGCCont:
	mov	GraphValid,0

	push	ax
	push	bx
	push	cx
	push	dx
	push	es
	push	di
	push	si

	lea	si,GraphicsSave		; ds:si points at saved data

;	mov	cx,1
;	test	EGA,-1
;	jz	TGCNotEGA
;	mov	cl,EGAPlanes		; ch = 0 from cx=1 above
;	call	SaveUser		; save user settings
;	mov	ax,00005H		; pointer 5 data 0
;	mov	dx,3ceh
;	out	dx,ax
;	mov	ax,0ff08H		; pointer 8 data ff
;	out	dx,ax
;TGCPlane:
;	mov	ah,80H
;	rol	ah,cl			; set bit corresponding to plane
;	mov	al,2			; map mask select register
;	mov	dx,3c4H
;	out	dx,ax	

;	mov	ah,cl			; plane number starting at 1
;	dec	ah			; plane number starting at 0
;	mov	al,4			; read map select register
;	mov	dx,3ceH
;	out	dx,ax	
;
;TGCNotEGA:

;	push	cx			; plane loop

	les	di,dword ptr ScreenOffset ; es:di points at cursor position
	mov	cx,10H			; 16 lines of mouse image
	mov	ax,GraphCY		; check for end of screen
	mov	bx,ax
	sub	ax,LastFullMPos
	js	TGCDoIt
	sub	cx,ax
TGCDoIt:
	test	EGA,-1
	jz	TGCNotEGA
	call	TidyEGA
	jmp	TGCFin
TGCNotEGA:
	call	TidyCGA
TGCFin:
;	pop	cx			; plane counter
;	loop	TGCPlane		; only >1 if EGA

;	call	ResetUser		; restore EGA registers, if EGA

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

	test	EGA,-1
	jz	SGC_OK
	ret
SGC_OK:
	push	ax
	push	bx
	push	cx
	push	dx
	push	ds
	push	si
	push	es
	push	di

	lea	si,GraphicsSave		; ds:si points at save area

;	mov	cx,1
;	test	EGA,-1
;	jz	SGCNotEGA
;	mov	cl,cs:EGAPlanes		; ch = 0 from cx=1 above
;SGCPlane:
;	mov	ah,cl
;	dec	ah
;	mov	al,4			; read map select register
;	mov	dx,3ceh
;	out	dx,ax	
;SGCNotEGA:
;	push	es
;	push	di

;	push	cx			; plane loop

	mov	cx,GraphCX		; x co-ordinate
	and	cx,7			; bit offset
	mov	ax,-1			; all bits set
	shr	ax,cl			; mask for bytes 0 and 1
	mov	SaveMask,ah
	mov	SaveMask+1,al
	not	ah			; mask for byte 2
	mov	SaveMask+2,ah
	mov	cx,10H			; 16 lines of mouse image
	mov	ax,GraphCY		; check for end of screen
	mov	bx,ax			; save line number
	sub	ax,cs:LastFullMPos
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
	pop	ax
	pop	cx
	call	Increment		; increment DI
	mov	bx,ax			; want line number in BX
	inc	bx
	add	si,3
	loop	SGCNext

;	pop	cx			; plane loop for EGA
;	pop	di
;	pop	es
;	loop	SGCPlane

	pop	di
	pop	es
	pop	si
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
SGCexit:
	ret

SaveGraphCursor	ENDP
