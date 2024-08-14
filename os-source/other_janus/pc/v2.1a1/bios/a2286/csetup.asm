include	title.inc
subttl	 ROM setup  
.xlist
include externals.inc
include equates.inc
.286P
.list
eproma	segment	public

	assume	cs:eproma

	PUBLIC	config_SETUP
	EXTRN	READ_CMOS: NEAR
	EXTRN	WRITE_CMOS: NEAR
	EXTRN	DISPLAY: NEAR
	EXTRN	CCHECK: NEAR	
	EXTRN	WD_CONFIG_TBL: BYTE
	extrn     flag_1:byte

;******************************************************************
BDATA	SEGMENT	AT	40H
	ORG	13H
MEMORY_SIZE	DW	?
VIR_MEM_SIZE	DW	?
	ORG	72H
RESET_KEY	DW	?

BDATA	ENDS


WORK_RAM	SEGMENT	AT	02000H		;Working area

public	equip
public	in_buff
public	in_buff_ptr
public	current_field	
public	floppy

		ORG	0

VIDEO_BASE		DW	?			;0, 1
CURRENT_FIELD	DB	?			;2
IN_BUFF_PTR	DB	?			;3
IN_BUFF		DB	8 DUP (?)		;4 -- B
NXT_HD_TYPE	DW	?			;5, 6

						
CMOS_IMAGE_TBL	LABEL	BYTE
SECOND		DB	?			
MINUTE		DB	?			
HOUR		DB	?			
CSDATE		DB	?			
CSMONTH		DB	?			
CSYEAR		DB	?			

FLOPPY		DB	?			;Byte 10H
		DB	?			;Byte 11H
HDISK		DB	?			;Byte 12H
		DB	?			;     13H	
EQUIP		DB	?			;     14H	
BASE_LOW		DB	?			;     15H	
BASE_HI		DB	?			;     16H	
EXT_LOW		DB	?			;     17H	
EXT_HI		DB	?			;     18H	
EXT_HD1		DB	?			;     19H	
EXT_HD2		DB	?			;     1AH	
		DB	13H DUP (?)		;Byte 1BH -- 2DH
CHECK_SUM		DB	?			;Btye 2EH
		DB	?			;Byte 2FH

		org	0400h
work_stack		dw	?
WORK_RAM	ENDS


;*******************************************************************	

include	setup_eq.inc
										       
HEADING		DB	0DH,0AH,'                  COMMODORE A2286 SETUP UTILITY V1.2',0DH,0AH,0AH,0AH

CONFIG_MSG		DB	' Date',0DH,0AH
		DB	' Time',0DH,0AH
		DB	' Diskette 1',0DH,0AH
		DB	' Diskette 2',0DH,0AH
		DB	' Hard Disk 1',0DH,0AH
		DB	' Hard Disk 2',0DH,0AH
		DB	' Video',0DH,0AH
		DB	' Coprocessor',0DH,0AH
		DB	' Base Memory',0DH,0AH
		DB	' Extended Memory',0DH,0AH,0AH
     		DB	' Base memory found:     ',0DH,0AH
		DB	' Extended memory found: ',0DH,0AH,0AH

HELP_MSG		DB	'Use ,  to select items',0DH,0AH
		DB	'Use ,  to select predefined values',0DH,0AH
		DB	'Press <V> to view more hard disk types',0DH,0AH
		DB	'Press <Q> to quit SETUP and reboot',0DH,0AH
		DB	'Press <E> to update CMOS and reboot',0DH,0AH,-1

HD_TBL_HEAD	DB	'ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸',-1
		DB	'³       Hard Disk Type Information     ³',-1
		DB	'ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´',-1
		DB	'³Type Cyln Head Sect W-pc L-zone Size  ³',-1,0
;			    1	306   4   17  none     0  xxx mb
HD_TBL_END	DB	'ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ',-1
     
NONE_MSG:
FLOPPY_TYPE 	DB	'NONE',-1
		DB	'360K',-1
		DB	'1.2M',-1
		DB	'720K',-1
		DB	'1.4M',-1

VIDEO_TYPE		DB	' SPECIAL',-1
		DB	'COLOR 40',-1
		DB	'COLOR 80',-1
		DB	'    MONO',-1

CPRO287		DB	'     NONE',-1
		DB	'INSTALLED',-1

KB_MSG		DB	' KB',-1
MB_MSG		DB	' MB',-1

;TABLE OF BCD MONTH LENGTHS TO CHECK OUT THE ENTERED DATE FOR LEGALLITY.
;THIS IS ACCESSED BY A BCD INDEX, SO APPROPRIATE 0 FILL IS NECESSARY

MON_TAB	DB	0,31H,29H,31H,30H,31H,30H,31H,31H,30H,0,0,0,0,0,0
	DB	31H,30H,31H

LEAVE_FIELD_TBL	LABEL	WORD
		DW	OFFSET	LEAVE_DATE
		DW	OFFSET	LEAVE_TIME
		DW	OFFSET	LEAVE_FLOPPY1
		DW	OFFSET	LEAVE_FLOPPY2
		DW	OFFSET	LEAVE_HD1
		DW	OFFSET	LEAVE_HD2
		DW	OFFSET	LEAVE_VIDEO
		DW	OFFSET	LEAVE_COPROC
		DW	OFFSET	LEAVE_BASE
		DW	OFFSET	LEAVE_EXT

INPUT_JMP_TBL	LABEL	WORD
		DW	OFFSET	INPUT_DATE
		DW	OFFSET	INPUT_TIME
		DW	OFFSET	INPUT_FLOPPY1
		DW	OFFSET	INPUT_FLOPPY2
		DW	OFFSET	INPUT_HD1
		DW	OFFSET	INPUT_HD2
		DW	OFFSET	INPUT_VIDEO
		DW	OFFSET	INPUT_COPROC
		DW	OFFSET	INPUT_BASE
		DW	OFFSET	INPUT_EXT	


start:
config_SETUP	PROC	NEAR
	cli

	mov	ax,bdata			; reset key flags
	mov	ds,ax
	mov	flag_1,0

	MOV	AX,SEG WORK_RAM
	MOV	DS,AX
	MOV	SS,AX
	mov	sp,offset work_stack
	sti

ASSUME	DS:WORK_RAM

; ----- Setup video

	INT	11H				;Get equipment byte
	AND	AX,VIDEO_MASK			;Mask out video bits
	MOV	VIDEO_BASE,COLOR_BASE		;Assume color
	CMP	AL,MONO				;Check video type
	MOV	AL,02H				;Will set to bw80 if color
	JNE	GOT_V_BASE			;All set if color
	MOV	VIDEO_BASE,MONO_BASE		;Mono.
	MOV	AL,07H				;Set to nomal mode
GOT_V_BASE:
	XOR	AH,AH				;Now set CRT mode
	INT	10H

	MOV	AX,0600H			;Blank screen
	XOR	CX,CX
	MOV	DX,184FH
	MOV	BH,07H
	INT	10H

	MOV	DX,VIDEO_BASE			;Turn cursor off
	MOV	AL,0AH
	OUT	DX,AL
	INC	DX
	MOV	AL,32H
	OUT	DX,AL

; ----- Set CMOS_IMAGE_TBL to default values

	MOV	DI,OFFSET CMOS_IMAGE_TBL	;ES:DI -- index to the table
	PUSH	DS
	POP	ES
	CLD
	MOV	CX,26H			;38 bytes
	XOR	AL,AL
	REP 	STOSB			;Clear all entries
	MOV	AX,40H
	MOV	ES,AX
	MOV	AX,ES:MEMORY_SIZE		;Set default memory size to
	MOV	WORD PTR BASE_LOW,AX	;  the amount found by POST
	MOV	AX,ES:VIR_MEM_SIZE		;Same way for extended memory
	MOV	WORD PTR EXT_LOW,AX
		
; ----- Check if CMOS is valid

	MOV	AL,CMOS_DIAG		;Diag byte
	CALL	READ_CMOS			;Read CMOS
	AND	AH,0C0H			;Check if error indicated by POST
	JNZ	DISP_MENU 		;If so, use default values
		
; ----- CMOS valid, read configration information in to CMOS_IMAGE_TBL

	CALL	READ_ALL			;Read all info from CMOS

; ----- Set values in DISP_TBL for display

DISP_MENU:
	CALL	DISP_FIXED_STRINGS		;Display all fixed strings in SETUP menu
	CALL	DISP_HD_TBL 		;Read and display hard disk type table
	CALL	DISP_CMOS_VALUES		;Display current CMOS setting
	CALL	DISP_MEM_FOUND		;Display amount of mem found by POST
	XOR	AL,AL			;Default field 0 (date)
	MOV	CURRENT_FIELD,AL		;Init current field to default
	CALL	INV_VIDEO			;High light it
	MOV	IN_BUFF_PTR,0

WAIT_KEY:
	MOV	CX,2700H			;Wait keyboard input loop count
WAIT_KEY1:
	MOV	AH,1			;See if any key is pressed
	INT	16H				
	JNZ	GOT_KEY			;Yes, process it
	LOOP	WAIT_KEY1			;Keep waiting for a while
	CMP	CURRENT_FIELD,1		;See if we are at time field
	JE	WAIT_KEY			;If so, don't update time display
	MOV	AH,03
	XOR	BX,BX
	INT	10H
	PUSH	DX
	CALL	DISP_TIME			;Update time display
	POP	DX
	MOV	AH,02
	INT	10H
	JMP	WAIT_KEY			;Go back wait for keyboard input
GOT_KEY:					
	XOR	AH,AH			;Got key input, read it
	INT	16H
	CMP	AL,'q'		;Exit if it's <q> key
	JE	EXIT
	CMP	AL,'Q'		; and also if it's <Q> key
	JE	EXIT		
	CMP	Al,'e'		;Update CMOS and exit if it's <e> key
	JE	SAVE_EXIT
	CMP	AL,'E'	   	; or the <E> key 				
	JNE	NOT_EXIT

SAVE_EXIT:
	MOV	BX,OFFSET LEAVE_FIELD_TBL
	MOV	al,CURRENT_FIELD
	SHL	AL,1
	xor	ah,ah
	ADD	bx,ax		 ; ToBu: do 16bit addition
	CALL	CS:[BX]
	CALL	UPDATE_CMOS

EXIT:	
	MOV	ES:RESET_KEY,1234H		
	DB	0EAH			; far jump RESET
	DW	0FFF0H
	DW	0F000H	
	hlt

NOT_EXIT:
	CMP	AH,UP_KEY			;Up arrow key?
	JNE	NOT_UP_KEY		;No
	CMP	AL,'8'
	JE	NOT_UP_KEY
	MOV	al,CURRENT_FIELD 		;Have to change to previous field
	PUSH	AX
	MOV	BL,07H
	CALL	BLANK_LINE		;Change the current field
	POP	AX			; to normal attribute
	SHL	AL,1			
	MOV	BX,OFFSET LEAVE_FIELD_TBL	;Get display routine jmup tbl
	xor	ah,ah		 	; ToBu: do 16bit addition
	ADD	bx,ax			;Adjust to the right routine
	CALL	CS:[BX]			;Display current display
	MOV	AL,CURRENT_FIELD
	OR	AL,AL			;Are we at the top field?
	JZ	SKIP_DEC	
	DEC	AL			;Dec field ptr
	JMP	SHORT SAVE_C_F
SKIP_DEC:
	MOV	AL,09
SAVE_C_F:
	MOV	CURRENT_FIELD,AL		;Save it
	CALL	INV_VIDEO			;High light the new field
	JMP	WAIT_KEY			;Loop back wait for more keys
NOT_UP_KEY:
	CMP	AL,CR
	JE	DOWN_KEY
	CMP	AH,DW_KEY			;Down arrow key?
	JNE	NOT_UP_DOWN
	CMP	AL,'2'
	JE	NOT_UP_DOWN
DOWN_KEY:
	MOV	al,CURRENT_FIELD		;
	PUSH	AX
	MOV	BL,07H
	CALL	BLANK_LINE
	POP	AX
	MOV	BX,OFFSET LEAVE_FIELD_TBL
	SHL	AL,1
	xor	ah,ah
	ADD	bx,ax		 	; ToBu: do 16bit addition
	CALL	CS:[BX]
	MOV	AL,CURRENT_FIELD
	CMP	AL,09
	JE	SKIP_INC
	INC	AL
	JMP	SHORT SAVE_C_F1
SKIP_INC:
	XOR	AL,AL
SAVE_C_F1:
	MOV	CURRENT_FIELD,AL
	CALL	INV_VIDEO
	JMP	WAIT_KEY
NOT_UP_DOWN:
	CMP	AL,'v'
	JE	ViewHD
	CMP	AL,'V'
	JNE	OTHER_KEY
ViewHD:
	MOV	SI,NXT_HD_TYPE
	CMP	SI,max_hd_typ   
	JNE	TB1
	MOV	SI,1
	MOV	NXT_HD_TYPE,SI
TB1:	CALL	DISP_TBL
	MOV	NXT_HD_TYPE,SI
	JMP	WAIT_KEY

public other_key
OTHER_KEY:
	PUSH	AX
	MOV	al,CURRENT_FIELD
	MOV	BX,OFFSET INPUT_JMP_TBL
	SHL	AL,1
	xor	ah,ah
	ADD	bx,ax		         	; ToBu: do 16bit addition
	POP	AX
	CALL	CS:[BX]
	JMP	WAIT_KEY

config_SETUP	ENDP


READ_ALL	PROC	NEAR

	MOV	AL,CMOS_FD_TYPE 		;Read floppy types
	CALL	READ_CMOS
	MOV	FLOPPY,AH			;Save floppy types
	INC	AL
	CALL	READ_CMOS			;Read hard disk types
	MOV	HDISK,AH
	INC	AL				
	CALL	READ_CMOS			;Read equipment byte
	MOV	EQUIP,AH
	CALL	READ_CMOS			;Read base memory size (low)
	MOV	BASE_LOW,AH
	CALL	READ_CMOS			;Base memory size (high)
	MOV	BASE_HI,AH
	CALL	READ_CMOS			;Read extended memory size (low)
	MOV	EXT_LOW,AH		
	CALL	READ_CMOS			;Extended memory size (high)
	MOV	EXT_HI,AH
	CALL	READ_CMOS			;Read extended HD1 type byte
	MOV	EXT_HD1,AH
	CALL	READ_CMOS			;Extended HD2 type
	MOV	EXT_HD2,AH
	RET
READ_ALL	ENDP


DISP_FIXED_STRINGS	PROC	NEAR

	MOV	AH,02			;Set cursor position to (0,16)
	MOV	DX,0010H
	XOR	BH,BH
	INT	10H
	MOV	SI,OFFSET HEADING		;Do display
	CALL	DISPLAY
	RET

DISP_FIXED_STRINGS	ENDP


; ----- Routine to display hard disk table

DISP_HD_TBL	PROC	NEAR
	MOV	NXT_HD_TYPE,0
	PUSH	DS
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET HD_TBL_HEAD	;Display table head
	MOV	DX,0328H 			;Starting from position (3,41)
DISP_0:	MOV	AH,02
	XOR	BH,BH
	INT	10H
	CALL	DISPLAY
	INC	SI
	INC	DH
	CMP	BYTE PTR [SI],0
	JNE	DISP_0

	PUSH	DX			;Display table frame
P_VLINE:					;  print left vertical line
	MOV	AH,02			;  first, then print the right one
	INT	10H
	MOV	AL,'³'
	CALL	DSP_CHAR
	INC	DH 			;Next line
	CMP	DH,17H			;To bottom of screen
	JB	P_VLINE
	POP	DX			;Restore starting position
	CMP	DL,4FH			;Did we do the right side yet?
	JE	P_END			;Done if yes
	MOV	DL,4FH			;Do it otherwise
	PUSH	DX
	JMP	P_VLINE
P_END:					;Now display the bottom line
	MOV	DX,1728H
	MOV	AH,02
	INT	10H
	MOV	SI,OFFSET HD_TBL_END
	CALL	DISPLAY

	MOV	SI,1			;Display contents of the 
	CALL	DISP_TBL			;  table starting from thpe 1
	POP	DS
	MOV	NXT_HD_TYPE,SI
	RET

DISP_HD_TBL	ENDP


; ----- Routine to display entries of predefined hard disk table
;	Input:	SI -- Starting hard disk type number
;		DS -- 0f000H

DISP_TBL	PROC	NEAR

	push	ds
	push	cs	 ;*******************
	pop	ds	 ;*******************

	MOV	AH,03
	XOR	BH,BH
	INT	10H
	PUSH	DX

	XOR	CH,CH
	MOV	DI,OFFSET WD_CONFIG_TBL	;Get offset of ROM HD table
	MOV	CL,4			;adjust to the right starting type
	SHL	SI,CL
	ADD	DI,SI
	SUB	DI,10H
	SHR	SI,CL
	MOV	DX,0729H			;This is where to start display
P_TBL:
	PUSH	DX			;Save row number
	PUSH	SI			;Save type number
	MOV	AH,02
	INT	10H			;Set cursor position
	MOV	AX,SI			;Display type number
	MOV	CL,3			;This is the size of print field
	CALL	HEX_DEC_DISP		;Convert and display
	MOV	AX,HD_PTR.HD_CYL		;Display # cyl
	MOV	CL,6
	CALL	HEX_DEC_DISP
	MOV	AL,HD_PTR.HD_HDS		;Display # heads
	XOR	AH,AH
	MOV	CL,4
	CALL	HEX_DEC_DISP
	MOV	AL,HD_PTR.HD_SEC		;Display # sectors
	XOR	AH,AH
	MOV	CL,5
	CALL	HEX_DEC_DISP
	MOV	AX,HD_PTR.HD_PCMP	 	;Display write pc cyl
	CMP	AX,0FFFFH
	JNE	TB_0
	MOV	AL,' '
	CALL	DSP_CHAR
	CALL	DSP_CHAR
	MOV	SI,OFFSET NONE_MSG
	CALL	DISPLAY
	JMP	SHORT LAND
TB_0:	MOV	CL,6
	CALL	HEX_DEC_DISP
LAND:	MOV	AX,HD_PTR.HD_LAND		;Display landing zone
	MOV	CL,6
	CALL	HEX_DEC_DISP
	MOV	AX,HD_PTR.HD_CYL		;Now calculate the capacity
	XOR	DX,DX		 	;  in MB
	MOV	CL,HD_PTR.HD_HDS
	MUL	CX
	MOV	CL,HD_PTR.HD_SEC
	MUL	CX
	MOV	CL,0BH
	SHR	AX,CL
	ROR	DX,CL
	OR	AX,DX
	MOV	CL,4
	CALL	HEX_DEC_DISP
	MOV	SI,OFFSET MB_MSG
	CALL	DISPLAY
	POP	SI	   		;Restore type number
	POP	DX	   		;Restore starting cursor pos
	ADD	DI,10H	   		;DI points to next table entry
	INC	SI	   		;Next type
	INC	DH	   		;Display at the next line
	CMP	SI,max_hd_typ		;Are we passed the last type?
	JE	STOP_DISP			;If so, no more to display
	CMP	DH,17H	   		;Are at the last line of CRT?
	JNE	P_TBL	   		;Display next type if not
STOP_DISP:
	CMP	DH,17H
	JE	NOT_LAST

	MOV	DL,29H
	MOV	AH,02
	XOR	BH,BH
	INT	10H
	MOV	CX,38
	XOR	BH,BH
	MOV	AX,0A20H
	INT	10H
	INC	DH
	MOV	AH,02
	INT	10H
	MOV	AX,0A20H
	INT	10H
NOT_LAST:
	POP	DX
	MOV	AH,02
	XOR	BH,BH
	INT	10H
	pop	ds
	RET					;Done

DISP_TBL	ENDP
	
	
; ----- Routine to convert hex number to decimal and disply it with leading
;	zero blanked out
;		Input:	AX -- hex number to be converted
;			CX -- number of char in display field
;		Output: Decimal number displayed at current cursor position	
;		  	AX,CX,DX changed

HEX_DEC_DISP	PROC	NEAR

	PUSH	BX			;Save BX
	PUSH	SI
	XOR	BH,BH			;BH -- flag indicating leading 0
	MOV	SI,CX			;Save field size
GET_DIGIT:
	XOR	DX,DX			;Clear DX for remaiders
	MOV	BX,0AH			;BL -- devider
	DIV	BX			;Split out a digit (decimal)
	PUSH	DX			;Stack it
	LOOP	GET_DIGIT			;Do for whatever number of digits
	MOV	CX,SI			;Restore field size 
DISP_DIGIT:				;Now it's time to display it
	POP	AX			;Get one digit back
	OR	AX,AX			;Is it a 0?
	JNZ	WRITE_DIGIT		;No, go ahead convert to ASCII
	OR	BH,BH			;It's a 0, see if it's a leading 0
	JNZ	WRITE_DIGIT		;No, go ahead convert it
	CMP	CX,1	   		;Write digit 0 out if it is
	JE	WRITE_DIGIT		;  the last digit
	MOV	AL,20H			;Leading 0, blank it out
	JMP	SHORT GOT_ASCII		;Go display
WRITE_DIGIT:	
	MOV	BH,1			;Indicate the next 0 is not a leading 0
	OR	AL,30H			;Convert to ASCII
GOT_ASCII:
	CALL	DSP_CHAR			;Write it out
	LOOP	DISP_DIGIT		;Do for all digits we stacked
	POP	SI
	POP	BX			;Restore BX
	RET

HEX_DEC_DISP	ENDP

; ----- Routine to convert BCD to ASCII
;		Input:	AL -- BCD number
;		Output	AX -- converted ASCII number

BCD_ASCII	PROC	NEAR

	MOV	AH,AL				;Save the BCD number
	MOV	CL,4
	SHR	AL,CL				;Do high nibble first
	OR	AL,30H				;Convert to ASCII
	XCHG	AH,AL				;Store converted high nibble in AH
	AND	AL,0FH				;Now do low nibble
	OR	AL,30H				;Convert it to ASCII
	RET					;Return with 2-char ASCII in AX
BCD_ASCII	ENDP



DSP_CHAR	PROC	NEAR
	PUSH	AX
	MOV	AH,0EH
	INT	10H
	POP	AX
	RET
DSP_CHAR	ENDP

; ----- Routine to display all relavent CMOS values

DISP_CMOS_VALUES	PROC	NEAR

	CALL	DISP_DATE  		;Show date
	CALL	DISP_TIME			;Show time
	CALL	DISP_FLOPPY1		;Show FD1
	CALL	DISP_FLOPPY2		;Show FD2
	CALL	DISP_HD1			;Show HD1
	CALL	DISP_HD2			;Show HD2
	CALL	DISP_VIDEO		;Show vedeo
	CALL	DISP_COPROC		;Show coprocessor
	CALL	DISP_BASE			;Show base memory
	CALL	DISP_EXT			;Show extended memory
	RET
DISP_CMOS_VALUES	ENDP

DISP_MEM_FOUND	PROC	NEAR

;	PUSH	ES
;	MOV	AX,40H
;	MOV	ES,AX
	MOV	AH,02
	MOV	DX,0F18H
	XOR	BH,BH
	INT	10H
	MOV	DX,ES:VIR_MEM_SIZE
	MOV	AX,ES:MEMORY_SIZE
;	POP	ES
	PUSH	DX
	MOV	CX,5
	CALL	HEX_DEC_DISP
	MOV	SI,OFFSET KB_MSG
	CALL	DISPLAY
	MOV	AH,02
	MOV	DX,1018H
	INT	10H
	POP	AX
	MOV	CX,5
	CALL	HEX_DEC_DISP
	MOV	SI,OFFSET KB_MSG
	CALL	DISPLAY
	RET
DISP_MEM_FOUND	ENDP


LEAVE_DATE	PROC	NEAR
	MOV	SI,OFFSET IN_BUFF
	CALL	GET_BCD				;Get date
	JC	SKIP_NEW_DATE			;Has to be a number
	OR	AL,AL				;Date has to be non-zero
	JZ	SKIP_NEW_DATE
	MOV	DL,AL				;DL -- date
	CALL	GET_BCD				;Get month
	JC	SKIP_NEW_DATE			;Has to be a number
	OR	AL,AL				;Can not be zero
	JZ	SKIP_NEW_DATE
	CMP	AL,12H				;Can not be above 12
	JA	SKIP_NEW_DATE
	MOV	DH,AL				;DH -- month
	MOV	BL,DH
	XOR	BH,BH				;BX -- index to month table
	CMP	DL,CS:MON_TAB[BX]		;See if the date is valid
	JA	SKIP_NEW_DATE			;  for the month
	CALL	GET_BCD				;Get year
	JC	SKIP_NEW_DATE			;Has to be a number
	CMP	AL,99H				;Less than 99
	JA	SKIP_NEW_DATE
	MOV	CL,AL
	MOV	CH,19H
	MOV	AH,05H
	INT	1AH				;Set RTC date
SKIP_NEW_DATE:
	CALL	DISP_DATE
	RET
LEAVE_DATE	ENDP


LEAVE_TIME	PROC	NEAR

	MOV	SI,OFFSET IN_BUFF
	CALL	GET_BCD
	JC	SKIP_NEW_TIME
	CMP	AL,23H
	JA	SKIP_NEW_TIME
	MOV	CH,AL
	CALL	GET_BCD
	JC	SKIP_NEW_TIME
	CMP	AL,59H
	JA	SKIP_NEW_TIME
	MOV	CL,AL
	XOR	DX,DX
	CALL	GET_BCD
	JC	SET_TIME
	CMP	AL,59H
	JA	SET_TIME
	MOV	DH,AL
SET_TIME:
	MOV	AH,03
	INT	1AH
SKIP_NEW_TIME:
	CALL	DISP_TIME
	RET
LEAVE_TIME	ENDP


LEAVE_FLOPPY1	PROC	NEAR
	CALL	DISP_FLOPPY1
	RET
LEAVE_FLOPPY1	ENDP

LEAVE_FLOPPY2	PROC	NEAR
	CALL	DISP_FLOPPY2
	RET
LEAVE_FLOPPY2	ENDP

LEAVE_HD1	PROC	NEAR
	CMP	IN_BUFF_PTR,0
	JE	NO_H1_IN
	MOV	SI,OFFSET IN_BUFF
	CALL	GET_BIN
	CMP	AX,max_hd_typ
	JA	NO_H1_IN
	CMP	AX,0FH
	JE	NO_H1_IN
	JB	STORE_H1
	MOV	AH,AL
	MOV	EXT_HD1,AH
	MOV	AL,0FH
STORE_H1:
	SHL	AL,4
	AND	HDISK,0FH
	OR	HDISK,AL
NO_H1_IN:
	CALL	DISP_HD1
	RET
LEAVE_HD1	ENDP

LEAVE_HD2	PROC	NEAR
	CMP	IN_BUFF_PTR,0
	JE	NO_H2_IN
	MOV	SI,OFFSET IN_BUFF
	CALL	GET_BIN
	CMP	AX,max_hd_typ
	JA	NO_H2_IN
	CMP	AX,0FH
	JE	NO_H2_IN
	JB	STORE_H2
	MOV	AH,AL
	MOV	EXT_HD2,AH
	MOV	AL,0FH
STORE_H2:
	AND	HDISK,0F0H
	OR	HDISK,AL
NO_H2_IN:
	CALL	DISP_HD2
	RET
LEAVE_HD2	ENDP

LEAVE_VIDEO	PROC	NEAR
	CALL	DISP_VIDEO
	RET
LEAVE_VIDEO	ENDP

LEAVE_COPROC	PROC	NEAR
	CALL	DISP_COPROC
	RET
LEAVE_COPROC	ENDP

LEAVE_BASE	PROC	NEAR
	CMP	IN_BUFF_PTR,0
	JE	NO_BASE_IN
	MOV	SI,OFFSET IN_BUFF
	CALL	GET_BIN
;	PUSH	ES
;	MOV	BX,40
;	MOV	ES,BX
	MOV	BX,ES:MEMORY_SIZE
;	POP	ES
	CMP	AX,BX
	JE	BASE_OK
;	CALL	DISP_WARNNING
BASE_OK:
	MOV	WORD PTR BASE_LOW,AX
NO_BASE_IN:
	CALL	DISP_BASE
	RET
LEAVE_BASE	ENDP

LEAVE_EXT	PROC	NEAR
	CMP	IN_BUFF_PTR,0
	JE	NO_EXT_IN
	MOV	SI,OFFSET IN_BUFF
	CALL	GET_BIN
;	PUSH	ES
;	MOV	BX,40
;	MOV	ES,BX
	MOV	BX,ES:VIR_MEM_SIZE
;	POP	ES
	CMP	AX,BX
	JE	EXT_OK
;	CALL	DISP_WARNNING
EXT_OK:	MOV	WORD PTR EXT_LOW,AX
NO_EXT_IN:
	CALL	DISP_EXT
	RET
LEAVE_EXT	ENDP


DISP_DATE	PROC	NEAR

	MOV	DX,0413H			;Set cursor position
	XOR	BH,BH
	MOV	AH,02
	INT	10H

	MOV	AH,04				;Get date
	INT	1AH
	MOV	WORD PTR CSDATE,DX		;Save date & month
	MOV	CSYEAR,CL				;Save year

	MOV	BH,3				;3 fields (xx/xx/xx)
	MOV	AL,CSDATE			;Get date
DATE_0:	CALL	BCD_ASCII			;Convert BCD (in AL) to ASCII (in AX)
	XCHG	AH,AL				;Display them
	CALL	DSP_CHAR
	XCHG	AH,AL
	CALL	DSP_CHAR
	DEC	BH				;Dec counter
	JZ	DONE				;All done
	MOV	AL,'.'				;Display '.'
	CALL	DSP_CHAR
	MOV	AL,CSMONTH			;Get month
	CMP	BH,2   				;See if we did date already
	JE	DATE_0				;No
	MOV	AL,CSYEAR			;Yes, then do year
	JMP	SHORT DATE_0
DONE:	RET	

DISP_DATE	ENDP


DISP_TIME	PROC	NEAR

	MOV	DX,0513H			;Set cursor position
	XOR	BH,BH
	MOV	AH,02
	INT	10H

	MOV	AH,02				;Read time
	INT	1AH
	MOV	SECOND,DH			;Save second
	MOV	WORD PTR MINUTE,CX		;Save minute & hour

	MOV	BH,3				;3 fields (xx:xx:xx)
	MOV	SI,OFFSET HOUR			;Start with hour
TIME_0:	LODSB
	CALL	BCD_ASCII			;Convert BCD to ASCII
	XCHG	AH,AL				;AL -- char to display
	CALL	DSP_CHAR
	XCHG	AH,AL				;Next char
	CALL	DSP_CHAR
	DEC	BH				;See if all done
	JZ	T_DONE				;Yes
	MOV	AL,':'				;Display ":"
	CALL	DSP_CHAR
	DEC	SI
	DEC	SI
	JMP	SHORT TIME_0
T_DONE:	RET
DISP_TIME	ENDP

DISP_FLOPPY1	PROC	NEAR
	XOR	AL,AL
	CALL	DISP_FLOPPY
	RET
DISP_FLOPPY1	ENDP

DISP_FLOPPY2	PROC	NEAR
	MOV	AL,1
	CALL	DISP_FLOPPY
	RET
DISP_FLOPPY2	ENDP


; ----- Routine to display current diskette drive type in SETUP menu
;		Input:	AL = 0 -- display first diskette drive
;			AL = 1 -- display second diskette drive


DISP_FLOPPY	PROC	NEAR
	push	ax
	MOV	DX,0617H
	OR	AL,AL
	JZ	FD_0
     	MOV	DX,0717H
FD_0:	MOV	AH,02
	XOR	BH,BH
	INT	10H
	pop	ax
	OR	AL,AL
	MOV	AL,FLOPPY			;Load floppy types
	JNZ	FD_1
	MOV	CL,4				
	SHR	AL,CL				;First floppy type
FD_1:	
	AND	AL,0FH
	MOV	AH,AL				;Save for later
	SHL	AL,1				;Multiply by 5 (each string
	SHL	AL,1				;  for floppy types is 5 byte
	ADD	AL,AH				;  long
	XOR	AH,AH		
	MOV	SI,OFFSET FLOPPY_TYPE		;SI -- beginning of strings
	ADD	SI,AX				;Adjust for the right type
	CALL	DISPLAY				;Display it
	RET
DISP_FLOPPY	ENDP

DISP_HD1	PROC	NEAR
	XOR	AL,AL
	CALL	DISP_HD
	RET
DISP_HD1	ENDP

DISP_HD2	PROC	NEAR
	MOV	AL,1
	CALL	DISP_HD
	RET
DISP_HD2	ENDP


; ----- Routine to display current hard disk type in SETUP menu
;		Input:	AL = 0 -- display first hard disk
;			AL = 1 -- display second hard disk

DISP_HD	PROC	NEAR
	push	ax
	MOV	DX,0817H			;Set cursor position for HD1
	OR	AL,AL			;See if it's right
	JZ	HD_0			;Yes
	MOV	DX,0917H			;2nd drive, set cursor for it
HD_0:	MOV	AH,02
	XOR	BH,BH
	INT	10H
	pop	ax
	MOV	AH,AL
	OR	AL,AL 			;Check which drive
	MOV	AL,HDISK			;Get hard disk types
	JNZ	HD_1			;Drive 2
	MOV	CL,4			;Drive 1, get the high nibble
	SHR	AL,CL
HD_1:
	AND	AL,0FH			;AL -- type for the right drive
	CMP	AL,0FH			;Type 15?
	JNE	GOT_HD_TYPE		;If not, then we got type
	OR	AH,AH
	MOV	AH,EXT_HD1		;Type 15 indicate ext type
	JZ	HD_2
	MOV	AH,EXT_HD2
HD_2:
	OR	AH,AH			;Ext type 0?
	JZ	GOT_HD_TYPE		;Yes, assume type 15
	MOV	AL,AH			;Get ext type otherwise
GOT_HD_TYPE:
	OR	AL,AL			;Is it 0?
	JNZ	SHOW_TYPE			;Display type number if not
	MOV	SI,OFFSET NONE_MSG		;Say "NONE" if 0
	CALL	DISPLAY
	JMP	HD1_DONE
SHOW_TYPE:
	XOR	AH,AH			;AX -- Type number to be
	MOV	CX,4			;  converted to decimal
	CALL	HEX_DEC_DISP		;Convert and display
HD1_DONE:
	RET
DISP_HD	ENDP


DISP_VIDEO	PROC	NEAR

	MOV	DX,0A13H			;Set cursor position
	MOV	AH,COPROC_MASK
	XOR	BH,BH
	INT	10H

	MOV	AL,EQUIP			;Get equipment byte
	AND	AL,30H			;Stap out video bits
	MOV	AH,AL			;Save for later
	MOV	CL,4				
	SHR	AL,CL			;AL -- video type
	SHR	AH,1			;Multiply by 9 for display index
	ADD	AL,AH
	XOR	AH,AH
	MOV	SI,OFFSET VIDEO_TYPE	;Get beginning of video strings
	ADD	SI,AX			;Adjust for right type
	CALL	DISPLAY			;Display it
	RET
DISP_VIDEO	ENDP

DISP_COPROC	PROC	NEAR

	MOV	DX,0B12H
	XOR	BH,BH
	MOV	AH,02
	INT	10H

	MOV	AL,EQUIP
	AND	AL,02H
	SHR	AL,1
	MOV	BL,0AH
	MUL	BL
	MOV	SI,OFFSET CPRO287
	ADD	SI,AX
	CALL	DISPLAY
	RET
DISP_COPROC	ENDP

	
	
; -----	Routine to display base memory setup 

DISP_BASE	PROC	NEAR

	MOV	DX,0C13H			;Set cursor position
	MOV	AH,02
	XOR	BH,BH
	INT	10H

	MOV	AX,WORD PTR BASE_LOW		;Get memory size from CMOS_IMAGE_TBL
	MOV	CX,5				;Display it as a 5-digit number
	CALL	HEX_DEC_DISP			;Convert and display
	MOV	SI,OFFSET KB_MSG		;Display "KB"
	CALL	DISPLAY
	RET

DISP_BASE	ENDP

; ----- Routine to display extended memory setup

DISP_EXT	PROC	NEAR

	MOV	DX,0D13H			;Set cursor position
	MOV	AH,02
	XOR	BH,BH
	INT	10H

	MOV	AX,WORD PTR EXT_LOW		;Get size form CMOS table
	MOV	CX,5			;Display it as a 5-digit number
	CALL	HEX_DEC_DISP		;Convert and display
	MOV	SI,OFFSET KB_MSG		;Display "KB"
	CALL	DISPLAY
	RET

DISP_EXT	ENDP

INPUT_DATE	PROC	NEAR
INPUT_TIME:
	XOR	BP,BP			;Indicate non-num char allowed
	CALL	PROCESS_CHAR
	RET

INPUT_DATE	ENDP



INPUT_FLOPPY1	PROC	NEAR

	MOV	BL,FLOPPY
	SHR	BL,4
	MOV	BH,4
	CALL	INPUT_ARROW
	SHL	BL,4
	AND	FLOPPY,0FH
	OR	FLOPPY,BL
	CALL	DISP_FLOPPY1
	RET
INPUT_FLOPPY1	ENDP

INPUT_FLOPPY2	PROC	NEAR

	MOV	BL,FLOPPY
	AND	BL,0FH
	MOV	BH,4
	CALL	INPUT_ARROW
	AND	FLOPPY,0F0H
	OR	FLOPPY,BL
	CALL	DISP_FLOPPY2
	RET
INPUT_FLOPPY2	ENDP


INPUT_ARROW	PROC	NEAR
	CMP	AH,LEFT_KEY
	JE	LEFT_ARROW
	CMP	AH,RIGHT_KEY
	JE	RIGHT_ARROW
	JMP	SHORT IN_F_DONE
LEFT_ARROW:
	TEST	AL,0FH
	JNZ	IN_F_DONE
	OR	BL,BL
	JZ	IN_F0
	DEC	BL
	JMP	SHORT IN_F_DONE
IN_F0:	MOV	BL,BH
	JMP	SHORT IN_F_DONE
RIGHT_ARROW:
	TEST	AL,0FH
	JNZ	IN_F_DONE
	CMP	BL,BH
	JE	IN_F1
	INC	BL
	JMP	SHORT IN_F_DONE
IN_F1:	XOR	BL,BL
IN_F_DONE:
	RET
INPUT_ARROW	ENDP


INPUT_HD1	PROC	NEAR

	CMP	AH,LEFT_KEY
	JE	H1_ARROW
	CMP	AH,RIGHT_KEY
	JNE	H1_NOT_ARROW
H1_ARROW:
	TEST	AL,0FH
	JNZ	H1_NOT_ARROW
	CMP	IN_BUFF_PTR,0
	JNE	I_H1_RET
	MOV	BL,HDISK
	SHR	BL,4
	CMP	BL,0FH
	JNE	I_H1_0
	MOV	BL,EXT_HD1
I_H1_0:	MOV	BH,max_HD_typ-1
	CALL	INPUT_ARROW
	CMP	BL,0FH
	JA	STORE_EXT
	JNE	NOT_15
	CMP	AH,LEFT_KEY
	JNE	INC_1
	DEC	BL
NOT_15:	SHL	BL,4
	AND	HDISK,0FH
	OR	HDISK,BL
	JMP	SHORT I_H1_DONE
INC_1:	INC	BL
STORE_EXT:
	OR	HDISK,0F0H
	MOV	EXT_HD1,BL
I_H1_DONE:
	CALL	DISP_HD1
I_H1_RET:
	RET
H1_NOT_ARROW:
	MOV	BP,1
	CALL	PROCESS_CHAR
	RET
	
INPUT_HD1	ENDP

INPUT_HD2	PROC	NEAR

	CMP	AH,LEFT_KEY
	JE	H2_ARROW
	CMP	AH,RIGHT_KEY
	JNE	H2_NOT_ARROW
H2_ARROW:
	TEST	AL,0FH
	JNZ	H2_NOT_ARROW
	CMP	IN_BUFF_PTR,0
	JNE	I_H2_RET
	MOV	BL,HDISK
	AND	BL,0FH
	CMP	BL,0FH
	JNE	I_H2_0
	MOV	BL,EXT_HD2
I_H2_0:	MOV	BH,max_HD_typ-1
	CALL	INPUT_ARROW
	CMP	BL,0FH
	JA	STORE_EXT_2
	JNE	H2_NOT_15
	CMP	AH,LEFT_KEY
	JNE	H2_INC_1
	DEC	BL
H2_NOT_15:
	AND	HDISK,0F0H
	OR	HDISK,BL
	JMP	SHORT I_H2_DONE
H2_INC_1:
	INC	BL
STORE_EXT_2:
	OR	HDISK,0FH
	MOV	EXT_HD2,BL
I_H2_DONE:
	CALL	DISP_HD2
I_H2_RET:
	RET
H2_NOT_ARROW:
	MOV	BP,1
	CALL	PROCESS_CHAR
	RET

INPUT_HD2	ENDP

INPUT_VIDEO	PROC	NEAR

	MOV	BL,EQUIP
	AND	BL,VIDEO_MASK
	SHR	BL,4
	MOV	BH,3
	CALL	INPUT_ARROW
	SHL	BL,4
	AND	EQUIP,NOT VIDEO_MASK
	OR	EQUIP,BL
	CALL	DISP_VIDEO
	RET

INPUT_VIDEO	ENDP

public input_coproc
INPUT_COPROC	PROC	NEAR

	MOV	BL,EQUIP
	CMP	AH,LEFT_KEY
	JE	TOGGLE
	CMP	AH,RIGHT_KEY
	JNE	I_C_DONE
TOGGLE:	TEST	AL,0FH
	JNZ	I_C_DONE
	XOR	BL,COPROC_MASK
	MOV	EQUIP,BL
I_C_DONE:
	CALL	DISP_COPROC
	RET

INPUT_COPROC	ENDP


INPUT_BASE	PROC	NEAR
	MOV	BP,1
	CALL	PROCESS_CHAR
	RET
INPUT_BASE	ENDP

INPUT_EXT	PROC	NEAR
	MOV	BP,1
	CALL	PROCESS_CHAR
	RET
INPUT_EXT	ENDP



INV_VIDEO	PROC	NEAR

	MOV	DX,0412H			;Top field
	ADD	DH,AL				;Adjust to the field to be hi lighted
NEXT_CHAR:
	MOV	AH,02				;Set cursor position
	XOR	BH,BH
	INT	10H

	MOV	CX,1				;Read attr/char from current 
	MOV	AH,08H				;  cursor position
	INT	10H
	MOV	BL,70H				;Reverse video
	MOV	AH,09H				;Write attr/char
	INT	10H
	INC	DL				;Next position
	CMP	DL,1BH				;End of the field?
	JBE	NEXT_CHAR			;Continue until done

	MOV	DL,13H				;  cursor position to the
	MOV	AH,02				;  beginning of the field
	INT	10H				
	MOV	IN_BUFF_PTR,0
	RET
INV_VIDEO	ENDP

; ----- Routine to display blank line in field AL 
;		Input:	AL -- field index to blank
;			BL -- attribute for the blank line

BLANK_LINE	PROC	NEAR
	
	MOV	DX,0412H			;Cursor pos of the first field
	ADD	DH,AL				;Adjust to the right field
	MOV	AH,02				;Set cursor position
	XOR	BH,BH
	INT	10H
	MOV	CX,0AH				;10 char long
	MOV	AX,0920H			;Now display with BL attribute
	INT	10H
	MOV	DL,13H
	MOV	AH,02
	INT	10H
	RET
BLANK_LINE	ENDP


PROCESS_CHAR	PROC	NEAR
	MOV	BL,IN_BUFF_PTR
	XOR	BH,BH
	MOV	SI,BX
	CMP	AL,'A'		;NO ALPHA ALLOWED AT ALL
	JAE	CHAR_DONE
	CMP	AL,BS		;CHECK FOR A BACKSPACE
	JNE	NOT_BS
	OR	SI,SI		;SEE IF AT BEGINNING ALREADY
	JE	CHAR_DONE		;DON'T BACK UP TOO FAR
	DEC	SI
	CALL	DSP_CHAR		;DO THE BACKSPACE
	MOV	AL,' '		;BLANK IT OUT
	CALL	DSP_CHAR
	MOV	AL,BS
	CALL	DSP_CHAR
	JMP	CHAR_DONE		;BACK TO NORMAL CHARACTER PROCESSING NOW

NOT_BS:   
	CMP	SI,08		;only seven chars allowed! 			
	JE	CHAR_DONE
	
	OR	SI,SI		;First char has to be a number!
	JZ	NUM_ONLY

	or	BP,BP		; ONLY NUMBER ALLOWED ?
	jnz	num_only		; yes -> jump
				; no, also special allowed !
	cmp	al,'.'
	je	fill_buffer
	cmp	al,':'
	je	fill_buffer
	cmp	al,'-'
	je	fill_buffer
	cmp	al,'/'
	je	fill_buffer	
				; no special found
NUM_ONLY:				; check for numbers
       	CMP	AL,'0'
	JB	CHAR_DONE
	CMP	AL,'9'
	JA	CHAR_DONE
	PUSH	AX
	OR	SI,SI
	JNZ	SKIP_BLANK_LINE
	MOV	AL,CURRENT_FIELD
	MOV	BL,70H
	CALL	BLANK_LINE
SKIP_BLANK_LINE:
	POP	AX

Fill_Buffer:
	MOV	IN_BUFF[SI],AL
	CALL	DSP_CHAR
	INC	SI
CHAR_DONE:
	MOV	BX,SI
	MOV	IN_BUFF_PTR,BL
	RET
PROCESS_CHAR	 ENDP

GET_BCD PROC NEAR		;THIS PROCEDURE SCANS FROM [SI] LOOKING FOR
				; A NUMBER, DELIMITED BY ANY NON-NUMBER.
				;WILL SCAN OVER LEADING BLANKS.
	PUSH	DX		;Save user DX register
	PUSH	BX		;SAVE USER BL REGISTER
	MOV	BL,1		;SET 'NO NUMBER' FLAG TO START WITH
	MOV	DL,IN_BUFF_PTR
	OR	DL,DL
	JZ	SET_FLAG
	XOR	DH,DH
	ADD	DX,OFFSET IN_BUFF
	XOR	AX,AX		;START NUMBER OUT WITH A ZERO
NXT_LET: CMP	BYTE PTR [SI],' '	;IF IT IS A BLANK, JUST SKIP IT
	JNE	GOT_NON_BLNK
	INC	SI
	JMP	NXT_LET
GOT_NON_BLNK: 
	CMP	SI,DX
	JE	SET_FLAG
	CMP	BYTE PTR [SI],'0'	;SEE IF IT IS A NUMBER NOW
	JB	NUM_DONE	;NON-NUMBERS GET US OUT OF THIS MESS
	CMP	BYTE PTR [SI],'9'
	JA	NUM_DONE
	AND	BYTE PTR [SI],0FH	;JUST KEEP THE NUMBER PART
	SHL	AX,1		;SHIFT WHAT WE GOT UP IN SIGNIFICANCE
	SHL	AX,1
	SHL	AX,1
	SHL	AX,1
	OR	AL,BYTE PTR [SI]	;AND BRING IN THE NEW DIGIT
	INC	SI
	MOV	BL,0		;SET 'NUMBER FOUND' FLAG
	JMP	GOT_NON_BLNK	;AND GO FOR THE NEXT CHARACTER
NUM_DONE: 
	INC	SI		;SKIP ANY OTHER TERMINATOR SO THE NEXT SCAN
SET_FLAG:			; WILL START OUT GOOD
	RCR	BL,1		;MAKE NUMBER FOUND FLAG IN THE CARRY BIT 
	POP	BX		;RESTORE REG
	POP	DX
	RET
GET_BCD ENDP

GET_BIN PROC NEAR		;THIS PROCEDURE SCANS FROM [SI] LOOKING FOR
				; A NUMBER, DELIMITED BY ANY NON-NUMBER.
				;WILL SCAN OVER LEADING BLANKS. CONVERTS THE
				; NUMBER FOUND INTO BINARY
	PUSH	DX
	MOV	DL,IN_BUFF_PTR
	OR	DL,DL
	JZ	BIN_DONE
	XOR	DH,DH
	ADD	DX,OFFSET IN_BUFF
	XOR	AX,AX		;START NUMBER OUT WITH A ZERO
BIN_NXT_LET:
	CMP	BYTE PTR [SI],' '	;IF IT IS A BLANK, JUST SKIP IT
	JNE	BIN_GOT_NON_BLNK
	INC	SI
	JMP	BIN_NXT_LET
BIN_GOT_NON_BLNK:
	CMP	SI,DX
	JE	BIN_DONE
	CMP	BYTE PTR [SI],'0'	;SEE IF IT IS A NUMBER NOW
	JB	BIN_NUM_DONE		;NON-NUMBERS GET US OUT OF THIS MESS
	CMP	BYTE PTR [SI],'9'
	JA	BIN_NUM_DONE
	AND	BYTE PTR [SI],0FH	;JUST KEEP THE NUMBER PART
	PUSH	BX		;SHIFT WHAT WE GOT UP IN SIGNIFICANCE
	MOV	BX,AX
	SHL	AX,1		;HARD MULTIPLY BY 10
	SHL	AX,1
	ADD	AX,BX
	SHL	AX,1
	MOV	BL,BYTE PTR [SI]	;AND BRING IN THE NEW DIGIT
	MOV	BH,0
	ADD	AX,BX			;THAT MAKES IT ALL KOSHER
	POP	BX			;RESTORE THE SAVED REG
	INC	SI
	JMP	BIN_GOT_NON_BLNK	;AND GO FOR THE NEXT CHARACTER
BIN_NUM_DONE:
	INC	SI		;SKIP ANY OTHER TERMINATOR SO THE NEXT SCAN
BIN_DONE:			; WILL START OUT GOOD
	POP	DX
	RET			
GET_BIN ENDP



UPDATE_CMOS	PROC	NEAR

	MOV	AL,EQUIP  		;First make the equip byte right
	AND	AL,3EH			;Clear floppy drive indicator bits
	TEST	FLOPPY,0FFH		;Any drive set?
	JZ	NO_FLOPPY		;No, no floppy drive
	TEST	FLOPPY,0F0H		;First drive present?
	JZ	ONE_F			;No, must have one drive
	TEST	FLOPPY,0FH		;2nd drive present?
	JZ	ONE_F			;No, only one drive
	OR	AL,40H			;Indicate there are 2 floppy drives set
ONE_F:	OR	AL,01H			;Indicate floppy drive installed
NO_FLOPPY:
	MOV	EQUIP,AL		;Write to equip byte (image)
	MOV	CX,1EH			;Now copy the image table to CMOS
	MOV	SI,OFFSET FLOPPY	;Start from byte 10h
	MOV	AL,CMOS_FD_TYPE		;Get CMOS address
PUT_CMOS:
	MOV	AH,[SI]			;Get from image table
	CALL	WRITE_CMOS		;Write to CMOS
	INC	SI
	LOOP	PUT_CMOS		;Do for all 1EH bytes
	CALL	CCHECK			;Gernerate checksum
	MOV	AL,0AEH			;Write checksum
	MOV	AH,BH
	CALL	WRITE_CMOS
	MOV	AH,BL
	CALL	WRITE_CMOS
	RET
UPDATE_CMOS	ENDP

public end_setup
end_setup	db	0AAh

;****************************************************

EPROMA	ENDS
	END
