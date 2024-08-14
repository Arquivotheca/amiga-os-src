.286
	page 85,132
title	(C) Copyright 1987 Commodore Technology Group - West Chester, Pa.


include equates.inc
	
	;Segment Definitions
	;
STACK	SEGMENT	PARA STACK 'STACK'
STACK	ENDS

zero	SEGMENT AT 0h
zero	ENDS

dataa	SEGMENT AT 40h
dataa	ENDS

extrn	serial_base:word
extrn	parallel_table:word
extrn	equip_flag:word
extrn	special:byte
EXTRN	READ_CMOS:NEAR
extrn	chk_cmos_good:near

;some equates

mouse_base	equ	023ch
control_port	equ	0230h
data		equ	40h
CMOS_COMMODORE	EQU	20H
eproma	segment byte public

	assume	cs:eproma, ds:dataa, es:zero
	
	public	signon
	public	aconfig

signon	proc	near	;produce various & sundry sign on information
	mov	ax,data
	mov	ds,ax			;setup data segment
	mov	cx,comm_length
	mov	si,offset commodore
	MOV	AL,BYTE PTR EQUIP_FLAG
	AND	AL,30H
	CMP	AL,10H
	JNE	DISP_SIGN
	MOV	CX,COMM_LENGTH_S
	MOV	SI,OFFSET COMMODORE_S
DISP_SIGN:
	call	disp1			; produce Commodore (c) 
	ret
signon	endp

aconfig	proc	near	
	mov	ax,data
	mov	ds,ax			;setup data segment
	mov	ax,equip_flag		; check for 80287
	and	ax,0002h
	jz	cccc			; no 80287
	mov	cx,mess10_length
	mov	si,offset mess10
	call	disp1		
cccc:	mov	di,0
ccom:	mov	ax,serial_base[di]
	cmp	ax,00h
	jz	clpt0
	mov	cx,mess6_length
	mov	si,offset mess6
	call	disp1
	mov	cx,mess1_length
	mov	si,offset mess1
	call	disp1
	mov	ax,serial_base[di]
	call	print_ascii
ncom:	add	di,2
	cmp	di,8
	jl	ccom
clpt0:	mov	di,0
clpt:	mov	ax,parallel_table[di]
	cmp	ax,00h
	jz	cmouse
	mov	cx,mess6_length
	mov	si,offset mess6
	call	disp1
	mov	cx,mess2_length
	mov	si,offset mess2
	call	disp1
	mov	ax,parallel_table[di]
	call	print_ascii
nlpt:	add	di,2
	cmp	di,8
	jl	clpt
cmouse:
IF BRIDGEBOARD
ELSE
	mov	dx,mouse_base+2	;setup mouse base address
	mov	cx,10h
nmouse:	in	al,dx
	cmp	al,0deh
	jz	fmouse			;found new type Microsoft Mouse	
	loop	nmouse
	inc	dx			;mouse_base+3
	mov	al,091h			;now test for old type Microsoft Mouse
	out	dx,al			;initialize 8255 control register
	dec	dx
	dec	dx
	mov	al,066h
	out	dx,al
	dec	dx
	mov	al,00h
	out	dx,al			;output 00h
	inc	dx
	in	al,dx
	cmp	al,066h
	jnz	nomouse
fmouse:	mov	cx,mess6_length
	mov	si,offset mess6
	call	disp1
	mov	cx,mess3_length
	mov	si,offset mess3
	call	disp1
	mov	al,mouse_base mod 256
	mov	ah,mouse_base / 256
	call	print_ascii
	jmp	cclk
nomouse:

;   --- Check CMOS configuration byte see if we should enable internal mouse

	CALL	CHK_CMOS_GOOD			;If CMOS not good, skip
	JNZ	CCLK
	mov	al,cmos_commodore+80H		;CMOS Commodore config byte
	call	read_cmos			;Read it
	and	ah,40h				;Set enable mouse?
	jz	cclk				;No, skip
	
	mov	al,special
	or	al,10h
	mov	special,al
	mov	dx,control_port
	out	dx,al			;enable internal mouse
cimouse:
	mov	dx,mouse_base+1
	mov	al,066h
	out	dx,al
	jmp	short $+2
	in	al,dx
	cmp	al,066h
	jnz	cclk
	mov	cx,mess7_length
	mov	si,offset mess7
	call	disp1
	mov	cx,mess3_length
	mov	si,offset mess3
	call	disp1
	mov	al,mouse_base mod 256
	mov	ah,mouse_base / 256
	call	print_ascii
cclk:	
;
;  ---  See which address the on board COM should use

	CALL	CHK_CMOS_GOOD			;Is CMOS good?
	JNZ	sc0				;No, skip check CMOS setting
	MOV	AL,CMOS_COMMODORE+80H		;CMOS good, get config byte
	CALL	READ_CMOS
	AND	AH,30H				;Bits for COM port
	JNZ	DO_COM
	JMP	CHK_LPT				;skip enable if set to 0
DO_COM:	MOV	AL,AH				
	SHR	AL,4				;Put to bit 1,0
	XOR	AH,AH				;AX -- index in COM table
	DEC	AL				;   of the port wanted
	SHL	AL,1				;2 bytes per entry
	MOV	SI,AX  				;Now check if address indicated
	XOR	DI,DI				;   by CMOS is used by external ports
	MOV	CX,4				
	MOV	AX,CS:[SI + OFFSET COM_TABLE]	;AX -- address in CMOS
SCOM:	CMP	AX,SERIAL_BASE[DI]		;Scan serial base table
	JE	COM_CONFLICT			;Conflict if there is a match
	ADD	DI,2				;Scan all 4 entries
	LOOP	SCOM
	JMP	ENABLE_COM			;No match found, go enable it
COM_CONFLICT:
	MOV	SI,OFFSET COM_ERR_MSG		;Display error message 
	MOV	CX,COM_ERR_LENGTH		
	CALL	DISP1				

sc0:	mov	si,00h				;Continue auto config
sc1:	mov	di,00h
scan_com:
	mov	ax,cs:[si + offset com_table]
	cmp	ax,00h
	JNZ	SC1A
	JMP	CHK_LPT
SC1A:	cmp	ax,serial_base[di]
	jz	sc3
	mov	ax,serial_base[di]
	cmp	ax,00h
	jnz	sc2
	jz	enable_com
sc3:	add	si,2
	jmp	sc1		
sc2:	add	di,2
	jmp	scan_com
enable_com:					;SI -- points to the address 
						;   that on board COM can use
	mov	ax,cs:[si + offset com_table]
	push	ax			;save addr of new com port
	mov	dx,control_port
	mov	al,special
	or	ax,cs:[si + offset com5720]
	mov	special,al
	out	dx,al			;enable internal com port	
	pop	ax
	push	ax			;get new com addr
	call	testcom			;test if uart is really there
	jnc	ec0			;jump if failed
	pop	ax			;clean up stack
	jmp	CHK_LPT
ec0:	mov	cx,mess7_length		;output internal com addr message
	mov	si,offset mess7		
	call	disp1
	mov	cx,mess1_length
	mov	si,offset mess1
	call	disp1
	pop	ax
	push	ax			;get new com addr
	call	print_ascii		
	mov	ax,equip_flag		;get equip_flag
	and	ax,0e00h		;isolate #serial bits
	add	ax,0200h		;increment # of com channels
	and	ax,0e00h
	push	ax
	mov	ax,equip_flag
	and	ax,0f1ffh
	mov	equip_flag,ax
	pop	ax
	or	ax,equip_flag
	mov	equip_flag,ax
	pop	ax			;get new com addr
	call	order_com		;re-order serial_base[]

;  ---  See which address the on board LPT should use

CHK_LPT:
	CALL	CHK_CMOS_GOOD			;Is CMOS good?
	JNZ	sl0				;No, skip check CMOS setting
	MOV	AL,CMOS_COMMODORE+80H		;CMOS good, get config byte
	CALL	READ_CMOS
	AND	AH,0CH				;Bits for lpt port
	JNZ	DO_LPT
	JMP	CHECK_GAME			;skip enable if set to 0
DO_LPT:	MOV	AL,AH				
	SHR	AL,2				;Put to bit 1,0
	XOR	AH,AH				;AX -- index in COM table
	DEC	AL				;   of the port wanted
	SHL	AL,1				;2 bytes per entry
	MOV	SI,AX  				;Now check if address indicated
	XOR	DI,DI				;   by CMOS is used by external ports
	MOV	CX,4				
	MOV	AX,CS:[SI + OFFSET LPT_TABLE]	;AX -- address in CMOS
SLPT:	CMP	AX,PARALLEL_TABLE[DI]		;Scan serial base table
	JE	LPT_CONFLICT			;Conflict if there is a match
	ADD	DI,2				;Scan all 4 entries
	LOOP	SLPT
	JMP	ENABLE_LPT			;No match found, go enable it
LPT_CONFLICT:
	MOV	SI,OFFSET LPT_ERR_MSG		;Display error message 
	MOV	CX,LPT_ERR_LENGTH		
	CALL	DISP1				

sl0:	mov	si,00h
sl1:	mov	di,00h
scan_lpt:
	mov	ax,cs:[si + offset lpt_table]
	cmp	ax,00h
	jnz	sl1a
	jmp	check_game
sl1a:	cmp	ax,parallel_table[di]
	jz	sl3
	mov	ax,parallel_table[di]
	cmp	ax,00h
	jnz	sl2
	jz	enable_lpt
sl3:	add	si,2
	jmp	sl1		
sl2:	add	di,2
	jmp	scan_lpt
enable_lpt:


LPT_OK:	mov	ax,cs:[si + offset lpt_table]
	push	ax			;save addr of new lpt port
	mov	dx,control_port
	mov	al,special
	or	ax,cs:[si + offset lpt5720]
	mov	special,al
	out	dx,al			;enable internal lpt port	
	pop	ax
	push	ax			;get new lpt addr
	call	testlpt			;test if ppc1 is really there
	jnc	el0         
	pop	ax
	jmp	check_game
el0:	mov	cx,mess7_length		;output internal lpt addr message
	mov	si,offset mess7		
	call	disp1
	mov	cx,mess2_length
	mov	si,offset mess2
	call	disp1
	pop	ax
	push	ax			;get new lpt addr
	call	print_ascii		
	mov	ax,equip_flag		;get equip_flag
	and	ax,0c000h		;isolate #parallel bits
	add	ax,04000h		;increment # of lpt channels
	push	ax
	mov	ax,equip_flag
	and	ax,03fffh
	mov	equip_flag,ax
	pop	ax
	or	ax,equip_flag
	mov	equip_flag,ax
	pop	ax			;get new lpt addr
	call	order_lpt		;re-order parallel_table[]
check_game:
	mov	ax,equip_flag
	and	ax,1000h
	jz	signon_done
	mov	cx,mess6_length		;output external game port msg
	mov	si,offset mess6		
	call	disp1
	mov	cx,mess8_length
	mov	si,offset mess8
	call	disp1
	mov	ax,0201h
	call	print_ascii		

signon_done:
ENDIF

IF BRIDGEBOARD
	call	crlf			;carriage return/line feed
ENDIF

	ret
aconfig	endp
;
;
print_ascii	proc	near
	push	ax			;save the value to be output
	and	ax,0f000h		;isolate ms nibble
	mov	cl,0ch			;set shift count
	shr	ax,cl
	call	outchar
	pop	ax
	push	ax
	and	ax,0f00h
	mov	al,ah
	mov	ah,00h
	call	outchar
	pop	ax
	push	ax
	and	ax,00f0h
	mov	cl,04h
	shr	ax,cl
	call	outchar
	pop	ax
	and	ax,000fh
outchar		proc	near
	mov	si,ax
	mov	al,cs:[si+offset ascii_tab]
	mov	ah,0eh
	mov	bh,0
	int	10h
	ret
outchar		endp
print_ascii	endp
ascii_tab:	db	'0123456789ABCDEF'
;
;
crlf	proc	near
	mov	al,0dh
	mov	ah,0eh
	mov	bx,07h
	int	10h
	mov	al,0ah
	mov	ah,0eh
	mov	bx,07h
	int	10h
	ret
crlf	endp
;
;
disp1	proc	near  
	MOV     AL,CS:[SI]             ;GET CHARACTER TO BE TRANSMITTED
        MOV     AH,0EH                                    
        MOV     BX,7H
        INT     10H                    ;DISPLAY CHARACTER
        INC     SI 
        LOOP    DISP1
        RET
disp1   endp
;
;

IF BRIDGEBOARD
ELSE
lpt_table:
	dw	3bch,378h,278h,000h		;
lpt5720:
	dw	0001h,0002h,0003h,0000h		;

com_table:
	dw	3f8h,2f8h,000h,000h		;
com5720:
	dw	0008h,0004h,0000h,0000h		;
;
	
testcom	proc near
	mov	dx,ax		       ;set up address
	add	dx,2
        in      al,dx                  ;READ SERIAL INTERRUPT ID
        test    al,0F0h                ;UART PRESENT ?
        jz 	tc2                    ;yes, then jump
	stc			       ;if not present, set carry flag
	jmp	tc2+1
tc2:    clc
	ret
testcom	endp
;
;
testlpt	proc near
	mov	dx,ax		       ;set up address
        MOV     AL,0AAH
        OUT     DX,AL                  ;WRITE PARALLEL PORT  
	jmp	short $+2
        MOV     CX,1FH
ts4:    LOOP    ts4                    ;DELAY
        IN      AL,DX                  ;READ PARALLEL PORT  
        CMP     AL,0AAH                ;PARALLEL PORT INSTALLED ?
        JNE     TS1		       ;no, jump
	clc
	ret
ts1:	stc
ts2:	ret
testlpt	endp
;
;
order_com	proc near
	push	ax
	mov	ax,equip_flag
	and	ax,0e00h
	mov	cl,9
	shr	ax,cl
	mov	cx,ax
	pop	ax
	xor	si,si
	cmp	cx,2
	je	oc
	mov	serial_base[si],ax
	ret
oc:	mov	ax,cs:[si + offset com_table]
	mov	serial_base[si],ax
	add	si,2
	loop	oc
	ret
order_com	endp
;
;
order_lpt	proc near
	PUSH	AX				;AX -- address of on board lpt
	mov	ax,equip_flag
	and	ax,0c000h
	mov	cl,14
	shr	ax,cl
	mov	cx,ax
	pop	ax
	xor	si,si
	CMP	CX,3
	je	ol
	cmp	ax,parallel_table[si]
	jb	ol0	
	mov	cx,parallel_table[si]
	mov	parallel_table[si],ax
	mov	parallel_table[si+2],cx
	ret
ol0:	mov	parallel_table[si+2],ax
	ret

ol:	mov	ax,cs:[si + offset lpt_table]
	mov	parallel_table[si],ax
	add	si,2
	loop	ol
	ret
order_lpt	endp

ENDIF
;
;
	include		mess.asm	;include messages

eproma	ends

	end

