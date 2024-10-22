
BitChanged	equ	80H
BitSingle	equ	40H
BitProhibit	equ	8000H

EGAData:

;	current values

CRTRegs		dw	0000H,0001H,0002H,0003H,0004H,0005H,0006H,0007H
		dw	0008H,0009H,000AH,000BH,000CH,000DH,000EH,000FH
		dw	0010H,0011H,0012H,0013H,0014H,0015H,0016H,0017H
		dw	0018H,-1
SeqRegs		dw	0003H,BitProhibit+0001H,0002H,0003H,BitProhibit+0004H,-1
GraphRegs	dw	0000H,0001H,0002H,0003H,0004H
		dw	0005H,BitProhibit+0006H,0007H
		dw	0008H,-1
AttrRegs	dw	0000H,0001H,0002H,0003H,0004H,0005H,0006H,0007H
		dw	0008H,0009H,000AH,000BH,000CH,000DH,000EH,000FH
		dw	0010H,0011H,0012H,0013H,-1

;	default values

CRTDefs		db	25 dup(0)
SeqDefs		db	4 Dup(0)
GraphDefs	db	9 dup (0)
AttrDefs	db	20 dup (0)

EGARegs:
EGA_CRTC	ELTab	<3d4H,offset CRTRegs,offset CRTDefs,25,0>
EGA_Seq		ELTab	<3c4H,offset SeqRegs,offset SeqDefs,4,0>
EGA_Graph	ELTab	<3ceH,offset GraphRegs,offset GraphDefs,9,0>
EGA_Attr	ELTab	<3c0H,offset AttrRegs,offset AttrDefs,20,0>
EGA_Misc	ELTab	<3c2H,0a7H,0a7h,1,BitSingle>
EGA_Feat	ELTab	<3daH,0,0,1,BitSingle>
EGA_Gr1		ELTab	<3ccH,0,0,1,BitSingle>
EGA_Gr2		ELTab	<3caH,1,1,1,BitSingle>

EGADataSize	dw	$-EGAData


;	EGA registers, etc

CRTId		equ	0
SequencerId	equ	8
GraphId		equ	10h
AttrId		equ	18h
MiscId		equ	20h
FeatureId	equ	28h
Graph1Id	equ	30h
Graph2Id	equ	38h


;	set the EGA registers for the mouse pointer

EGAGraphSave		db	9 dup(0)
EGASeqSave		db	0
EGAGraphWant		db	0	; 0 - set/reset
			db	0	; 1 - enable set/reset
			db	0	; 2 - colour compare
			db	0	; 3 - data rotate
			db	0	; 4 - read map select
			db	0	; 5 - write mode 0
			db	0	; 6 - misc. - ignored
			db	15	; 7 - colour dont care
			db	-1	; 8 - bit mask

SaveUser	PROC	NEAR

	test	CardActive,EGAType
	jz	SUNotEGA		; not an EGA card

	push	es
	push	ax
	push	bx
	push	cx
	push	dx

;	save the current contents

	mov	ax,cs
	mov	es,ax
	mov	dx,GraphId
	mov	cx,9			; registers 0 to 8
	mov	bx,offset EGAGraphSave	; read into here
	mov	ah,0f2H			; read register range
	call	EGAProc

	mov	dx,SequencerId
	mov	bx,2			; read plane map mask
	mov	ah,0f0H			; read single register
	call	EGAProc
	mov	EGASeqSave,bl
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	es
SUNotEGA:
	ret

SaveUser	ENDP

SetMouse	PROC	NEAR

	test	CardActive,EGAType
	jz	SMNotEGA		; not an EGA card
	push	es

;	save the current contents

	call	SaveUser
	jmp	SMOut

;	write the values as needed by the mouse

	inc	ah			; f1H
	mov	bx,0f02H		; enable all planes
	call	EGAProc

	mov	ah,0f3H
	push	cs
	pop	es
	lea	bx,EGAGraphWant		; what the mouse wants
	mov	cx,9			; registers 0 to 8 (except 6)
	mov	dx,GraphId
	call	EGAProc			; set them
SMOut:
	pop	es

SMNotEGA:
	ret

SetMouse	ENDP

;	reset the user values in EGA registers

ResetUser	PROC	NEAR

	test	CardActive,EGAType
	jz	RUNotEGA		; not an EGA card
	push	es
	push	ax
	push	bx
	push	cx
	push	dx

;	write the values as saved by SetMouse

	mov	ax,cs
	mov	es,ax
	mov	ah,0f1H			; write a register
	mov	bh,EGASeqSave
	mov	bl,02H			; enable planes
	mov	dx,SequencerId
	call	EGAProc

	mov	ah,0f3H
	push	cs
	pop	es
	lea	bx,EGAGraphSave		; what the mouse wants
	mov	cx,9			; registers 0 to 8 (except 6)
	mov	dx,GraphId
	call	EGAProc		; set them
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	es
RUNotEGA:
	ret

ResetUser	ENDP


EGAProc		PROC	NEAR

	push	bp
	mov	bp,sp

	push	ds			; bp-2
	push	es			; bp-4
	push	di			; bp-6
	push	si			; bp-8
	push	bx			; bp-10
	push	ax			; bp-12
	push	dx			; bp-14
	push	cx			; bp-16

	push	cs
	pop	ds

	call	ETrace			; trace if DoTrace = 2

;	the Attribute Controller should have been set to address state
;	by the caller. Just in case he hasn't done so do it now. This
;	should not cause problems since the spec. says that the A.C. is
;	always left in the Address state by EGA Lib.

	push	dx			; save DX
	mov	dx,word ptr EGA_Feat	; 3DAH or 3BAH
	in	al,dx			; reset Attr. Ctrlr flip/flop
	pop	dx			; restore DX

;	reason for call

	and	ah,0fH
	mov	bl,ah
	shl	bl,1			; *2
	xor	bh,bh			; bx = jump table index
	cmp	ah,5
	jb	EPUseDX
	cmp	ah,7
	je	EPUseDX
	jmp	EPNoDX			; DX is not a parameter
EPUseDX:
	mov	si,dx
	lea	si,[EGARegs+si]		; offset of this register
	cld				; df = 0
	mov	dx,[si].EGA_Port	; first word of EGAReg entry = port no.
	mov	ax,[si].EGA_Values	; value or list offset
EPNoDX:
	lea	bx,[EGATable+bx]
	jmp	word ptr [bx]		; jump to code for this function

;	Function F0 : Read one register

ReadAReg:
	test	[si].EGA_Status,BitSingle	; type of register
	jnz	RARSingle		; jump if single - value already in al
	mov	si,ax			; offset of table
	mov	bx,[bp-10]		; pointer
	sal	bx,1
	mov	al,[si+bx]		; get value
RARSingle:
	mov	[bp-10],al		; store for restoring
	jmp	EGAExit


;	Function F1 : Write one register

WriteAReg:
	or	[si].EGA_Status,BitChanged	; set changed for Reset Defaults
	mov	bx,[bp-10]		; (pointer and) value
	test	[si].EGA_Status,BitSingle	; type of register
	jnz	WARSingle		; jump if single - value already in al
	mov	si,ax			; offset of table
	mov	ax,bx			; pointer and value
	xor	bh,bh			; bx = pointer
	sal	bx,1
	test	[si+bx],BitProhibit
	jnz	WARSkip			; don't write this one
	out	dx,al			; pointer number set
	mov	al,ah			; value
	mov	[si+bx],al
	cmp	dx,word ptr EGA_Attr	; Attribute Controller ?
	jz	WARCommon		; flip/flop instead of diff. addr.
	inc	dx			; port for value
	jmp	WARCommon
WARSingle:
	mov	[si],bl
	mov	al,bl
WARCommon:
	out	dx,al			; write to the register
WARSkip:
	jmp	EGAExit

;	Function F2 : Read register range

ReadRegs:
	mov	di,[bp-10]		; register table offset
	test	[si].EGA_Status,BitSingle	; type of register
	jnz	RRSkip			; ignore for single register
	mov	si,ax			; register value table start
	mov	al,ch
	xor	ah,ah
	sal	ax,1
	add	si,ax			; offset of the first one wanted
	xor	ch,ch			; cx = number of registers
RRLoop:
	movsb				; move into user buffer
	inc	si			; skip properties byte
	loop	RRLoop
RRSkip:
	jmp	EGAExit

;	Function F3 : Write register range

WriteRegs:
	or	[si].EGA_Status,BitChanged	; set for Reset Defaults
	mov	di,[bp-10]		; register table offset
	test	[si].EGA_Status,BitSingle		; type of register
	jnz	WRSkip			; ignore for single register
	mov	si,ax			; register value table start
	mov	al,ch
	xor	ah,ah
;	sal	ax,1
	add	si,ax
	add	si,ax			; offset of the first one wanted
	xor	ch,ch			; cx = number of registers

	push	bp
	push	dx
	mov	bp,sp
	cmp	dx,word ptr EGA_Attr		; is it the Attr Ctrl ?
	jz	WRAttr			; yes
	inc	dx			; Data port address
WRAttr:
	
WRLoop:
	test	0[si],BitProhibit
	jnz	WRLSkip

	xchg	dx,0[bp]		; get address port
	out	dx,al			; pointer out
	push	ax
	mov	al,es:0[di]		; get user value
	mov	0[si],al		; store in my table
	xchg	dx,0[bp]		; get data port
	out	dx,al			; set value
	pop	ax

WRLSkip:
	inc	di
	add	si,2
	inc	ax			; next pointer
	loop	WRLoop
	cmp	dx,word ptr EGA_Attr
	jnz	WRNotAttr		; not the Attr.Ctrlr.
	mov	al,20H
	out	dx,al
WRNotAttr:
	pop	dx			; tidy up stack ..
	pop	bp			; .. and reset BP
WRSkip:
	jmp	EGAExit

;	Function F4 : Read register set

ReadSet:
	mov	si,[bp-10]		; pointer to user table
	mov	ax,[bp-4]		; user es
	mov	ds,ax			; ds = user es
RSLoop:
	lodsw
	mov	dx,ax			; port number
	lodsb
	mov	bl,al
	xor	bh,bh
	mov	ah,0f0H
	call	EGAProc			; read a register into BL
	mov	0[si],bl		; store in user table
	inc	si			; next lot
	loop	RSLoop
	jmp	EGAExit

;	Function F5 : Write register set

WriteSet:
	mov	si,[bp-10]		; pointer to user table
	mov	ax,[bp-4]		; user es
	mov	ds,ax			; ds = user es
WSLoop:
	lodsw
	mov	dx,ax			; port number
	mov	di,ax
	lea	di,[EGARegs+di]		; offset of this register
	lodsw
	test	cs:[di].EGA_Status,BitSingle
	jz	WSCommon
	mov	al,ah
WSCommon:
	mov	bx,ax			; data (and pointer)
	mov	ah,0f1H
	call	EGAProc			; write a register into BX
	loop	WSLoop
	jmp	EGAExit

;	Function F6 : Revert to default registers


RevertToDefault:
	mov	cx,4			; 4 multi-registers
	mov	dx,0			; port number
	mov	ax,cs
	mov	es,ax			; es = this segment
	lea	si,EGARegs
MultiLoop:
	push	cx			; save loop count
	test	[si].EGA_Status,BitChanged
	jz	MLNotChanged
	mov	cl,[si].EGA_Count	; register count (ch = 0)
	cmp	dx,SequencerId
	jnz	MLNotSeq
	mov	ch,1			; start at 1
MLNotSeq:
	mov	bx,[si].EGA_Defs	; defaults address
	mov	ah,0f3H			; write register range
	call	EGAProc			; write them
	xor	[si].EGA_Status,BitChanged
MLNotChanged:
	add	si,8
	add	dx,8			; I restore DX
	pop	cx
	loop	MultiLoop

	mov	cx,4

SingleLoop:
	test	[si].EGA_Status,BitChanged
	jz	SLNotChanged
	xor	[si].EGA_Status,BitChanged

	mov	bx,[si].EGA_Defs	; value into bl
	mov	ah,0f1H			; write a register
	call	EGAProc			; write it
SLNotChanged:
	add	si,8
	add	dx,8			; I restore DX
	loop	SingleLoop

	jmp	EGAExit

;	Function F7 : Define default register table

DefineDefault:
	or	[si].EGA_Status,BitChanged
	test	[si].EGA_Status,BitSingle
	jnz	DDSingle		; it's a single

	mov	ax,cs
	mov	es,ax
	mov	di,[si].EGA_Defs	; es:di points at defaults area

	mov	cl,[si].EGA_Count
	xor	ch,ch

	mov	si,[bp-10]		; pointer to user table
	mov	ax,[bp-4]		; user es
	mov	ds,ax			; ds:si points at user values

DDMove:

    rep movsb				; move 

	jmp	EGAExit

DDSingle:
	add	si,EGA_Defs		; low byte of EGA_Defs
	mov	cx,1
	jmp	DDMove

;	Function FA : Interrogate driver

EGAVersion	db	1,80

Interrogate:
	push	cs
	pop	ax
	lea	bx,EGAVersion
	mov	[bp-10],bx
	mov	[bp-4],ax
;	jmp	EGAExit

;	Functions F8,F9, and FB to FF : Reserved

Reserved:
	mov	ax,[bp-2]		; restore ax, ds, si
	mov	ds,ax
	mov	ax,[bp-12]
	mov	si,[bp-8]
	pushf
	call	cs:dword ptr VideoOffset	; call the normal int 10H handler
	mov	[bp-12],ax
	mov	[bp-8],si
	mov	ax,ds
	mov	[bp-2],ax
;	jmp	EGAExit

EGAExit:

;	the Attribute Controller is always left in the Address state by
;	this routine.

	mov	dx,word ptr cs:EGA_Feat		; 3DAH or 3BAH
	in	al,dx			; reset Attr. Ctrlr flip/flop

	pop	cx
	pop	dx
	pop	ax
	pop	bx
	pop	si
	pop	di
	pop	es
	pop	ds
	mov	sp,bp
	pop	bp

	ret

EGAProc	ENDP

EGATable:
	dw	ReadAReg		;	F0
	dw	WriteAReg		;	F1
	dw	ReadRegs		;	F2
	dw	WriteRegs		;	F3
	dw	ReadSet			;	F4
	dw	WriteSet		;	F5
	dw	RevertToDefault		;	F6
	dw	DefineDefault		;	F7
	dw	Reserved		;	F8
	dw	Reserved		;	F9
	dw	Interrogate		;	FA
	dw	Reserved		;	FB
	dw	Reserved		;	FC
	dw	Reserved		;	FD
	dw	Reserved		;	FE
	dw	Reserved		;	FF


CopyDefaults	PROC	NEAR

;	input	es:di		points at my area for defaults
;		ds:si		points at EGA cards area for defaults
;		cx		loop count

CDLoop:
	mov	ax,es:0[di]		; get status byte + value
	lodsb				; get default value into AL
	stosw				; store status and value
	loop	CDLoop
	ret

CopyDefaults	ENDP

;	convert Video Mode (in AX) to index in defaults list (in AX)

WhatIndex	Proc	Near
	
	cmp	ax,14
	ja	Check64K		; index for 16&15 dependent on RAM size
	cmp	ax,3
	ja	IndexAsIs		; modes 4-14, index == mode

;	check enhanced colour display

	test	cs:EGAEnhanced,1
	jz	IndexAsIs
	add	ax,13H			; enhanced colour display
	jmp	IndexAsIs

Check64K:

;	check for more than 64K RAM

	test	cs:EGARamSize,-1
	jz	IndexAsIs		; 0 = 64K RAM
	add	ax,2			; > 64K Ram

IndexAsIs:
	ret

WhatIndex	EndP


InitEGALib	PROC	NEAR

;	input	ax	Video Mode

;	check if EGA card present

	test	cs:CardActive,EGAType
	jnz	InitEGA
	jmp	InitNotEGA		; not an EGA card

InitEGA:

;	save registers

	push	es
	push	di
	push	ds
	push	si
	push	cx
	push	ax

;	convert Video Mode to index in defaults list
	
	call	WhatIndex

	xor	cx,cx
	mov	ds,cx
	lds	si,ds:[04A8H]		; Save Ptr Table
	lds	si,ds:[si] 		; Video Parameter Table

	mov	cx,6			; shift 6 == * 64
	shl	ax,cl			; index
	add	si,ax			; pointer to defaults for this mode
	add	si,5			; pointer to Sequencer defaults
	
	cld				; direction for lods	

	mov	ax,cs
	mov	es,ax			; point ES at this code segment
	lea	di,cs:SeqRegs+2		; register 0 has no default ????
	mov	cx,4
	push	cx
	push	si
	call	CopyDefaults
	lea	di,cs:SeqDefs
	pop	si
	pop	cx
    rep movsb

	lodsb				; Miscellaneous
	mov	byte ptr cs:EGA_Misc+2,al
	mov	byte ptr cs:EGA_Misc+4,al	; naughty !!!!!!

	mov	cx,25
	lea	di,cs:CRTRegs
	push	cx
	push	si
	call	CopyDefaults
	lea	di,cs:CRTDefs
	pop	si
	pop	cx
    rep movsb

	mov	cx,20
	lea	di,cs:AttrRegs
	push	cx
	push	si
	call	CopyDefaults
	lea	di,cs:AttrDefs
	pop	si
	pop	cx
    rep movsb

	mov	cx,9
	lea	di,cs:GraphRegs
	push	cx
	push	si
	call	CopyDefaults
	lea	di,cs:GraphDefs
	pop	si
	pop	cx
    rep movsb

;	restore registers

	pop	ax
	pop	cx
	pop	si
	pop	ds
	pop	di
	pop	es

InitNotEGA:
	ret

InitEGALib	ENDP
