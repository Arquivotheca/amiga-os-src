;	NAME	LGHTPN
;	TITLE	Light Pen terminate and stay resident program
	PAGE	61,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
;------------------------------------------------------------------------------
;
; READ LIGHT PEN (CRT DSR Function AH=04h)
;
;	Input:	DS = ROM Data segment (0040h)
;	Output:	AH = 0 if light pen not triggered, 1 if it is
;		DH = Character row of light pen
;		DL = Character column of light pen
;		CH = Pixel row
;		BX = Pixel column, best estimate
;
;------------------------------------------------------------------------------
       IF	LIGHTPN
	EXTRN	GRFXCHK:NEAR		; Check if its a graphic mode
	PUBLIC	RDLGHT
;
CRTLTPN	PROC	NEAR
LGHTBL	DB	3,3,5,5,3,3,3,4		; Offset values for regen address
					;   calculation
RDLGHT:
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	BP,SP			; Get stack pointer so we can reference
					;   the returning values
	XOR	AH,AH			; Clear AH for return of no value
	PUSH	AX
	MOV	DX,CRTADDR		; CRT Base port
	ADD	DX,CRTST-CRTCAD		; DX = CRT Graphic Status Register port
	IN	AL,DX			; Get status
	INC	DX			; Set DX to clear light pen latch
	TEST	AL,LGHTSM		; Is Light Pen switch made ?
	JNZ	RDLGH2			;   No (it is active low)
RDLGH0:
	TEST	AL,LGHTRG		; Is Light Pen trigger made ?
	JZ	NOTRGR			;   No
;
	SUB	DX,LGHTRST-CRTCAD	; DX = CRT 6845 Select Register port
	MOV	AL,LPADRH		; AL = CRTC Light Pen register (MSB)
	OUT	DX,AL			; Select register
;
	MOV	BL,CRTMODE		; Get CRT mode
	XOR	BH,BH			; Delay for chip I/O recovery time
;
;;*****	CMP	BL,7			; Is it too big ?			;	YYYTEST
;;*****	JBE				;   No					;	YYYTEST
;;*****	CALL	WRTINL								;	YYYTEST
;;*****	DB	CR,LF,'Lightpen called with CRT mode above 7',CR,LF,0		;	YYYTEST
;
	INC	DX			; DX = CRTC Data register
	IN	AL,DX			; Get Light Pen register address (MSB)
	MOV	AH,AL			;   and save in AH
;
	DEC	DX			; Restore DX
	MOV	AL,LPADRL		; AL = CRTC Light Pen register (LSB)
	OUT	DX,AL			; Select register
	MOV	BL,[LGHTBL+BX]		; Get address offset
					; Delay for chip I/O recovery time
	INC	DX			; DX = CRTC Data register
	IN	AL,DX			; Get Light Pen register address (LSB)
;
	ADD	DX,LGHTRST-CRTCDT	; Reset Light Pen Latch
	OUT	DX,AL			;   output anything
;
MODEOK:
	SUB	AX,BX			;   and subtract to address
	JNC	NONEG			; If corrected value is less than 0						;stan 3/11
	XOR	AX,AX			; Clear it to 0 if so
NONEG:
	MOV	CL,3			; Set up CL to 3
;
	CALL	GRFXCHK			; Check if its a graphic mode
	JNC	RDLPGN			;   No, then go do non-graphics version
;
	MOV	BH,40			; Constant in BH
	DIV	BH			; Divide by screen width yields
					; AL = row #, AH = column #
	XCHG	AH,AL	
	MOV	BX,AX
;
	SHL	AH,1			; Mutiply row * 2 to get raster
					;   this will always be the even scan
					;   and hence vertical resolution is
					;   2 scan lines
	CMP	CRTMODE,6
	JNE	RDLPLW			; CRT is in low resolutiom mode
	INC	CX			; Use INC CX because it uses less bytes
					;   and is faster
	SHL	BL,1
RDLPLW:
	SHR	BH,1			; Divide by 4
	SHR	BH,1
	MOV	[BP+10],BX		; Return character row,column in DX
	JMP	SHORT RDLGH1
;
RDLPGN:
	DIV	BYTE PTR SCRNWID	; Divide by screen width yields
					; AL = row #, AH = column #
	XCHG	AH,AL	
	MOV	[BP+10],AX		; Return character row,column in DX
	SHL	AH,CL			; AH = Pixel row
					;   NOTE: CL still equals 3 from above
					;   a shift instruction does not destroy
					;   it
RDLGH1:
	MOV	[BP+13],AH		; Return Raster scan Pixel row in CH
	XOR	AH,AH
	SHL	AX,CL			; AX = Pixel column,
	MOV	[BP+14],AX		; Return Pixel column in BX
	MOV	BYTE PTR [BP-1],001h	; Return Flag in AH
;
	ADD	DX,LGHTRST-CRTCDT	; Reset Light Pen Latch
;
RDLGH2:
;
	OUT	DX,AL			;   output anything
NOTRGR:
	JMP	NEAR PTR COMXITA	; GO TO COMMON DSR EXIT
CRTLTPN	ENDP
	ENDIF
	PRGSIZ	<CRT DSR Light Pen code>	; Display program size
