include	title.inc
subttl	Misc Interrupts

.xlist
include external.inc
include	equates.inc

public	unexpected_interrupt
public	dummy_mouse
public	break
public	math
public	int71
public	boot
public	time
public	clock
public	alarm
public	ram_size_1
public	config_1
public	parity_error
IF BRIDGEBOARD
public	boot_msg_1
ENDIF

extrn	interrupt_flag:byte
extrn	data_rate:byte
extrn	hf_num:byte
extrn	timer_upper:word
extrn	timer_lower:word
extrn	timer_wrap:byte
extrn	motor_on:byte
extrn	update:near
extrn	start_clock:near
extrn	count_low:word
extrn	count_high:word
extrn	event_wait_flag:byte
extrn	event_flag_addr:dword
extrn	display:near
extrn	memory_size:word
extrn	equip_flag:word
extrn	video_mode:byte
extrn	floppy_parms:near
.list


zero	segment	at 0
zero	ends

dataa	segment	at 40h
dataa	ends

eproma	segment	byte public

	assume cs:eproma,ds:dataa,es:dataa

public aa_miscintr
aa_miscintr label byte

;******************************************************************************
;
;     THIS ROUTINE HANDLES UNEXPECTED INTERRUPTS
;
;******************************************************************************

UNEXPECTED_INTERRUPT PROC NEAR

	sti				;set interrupts
	push	ax			;
	call	save_all		;save all registers


;SET INTERRUPT FLAG

        MOV     byte ptr INTERRUPT_FLAG,-1

;CHECK ACTIVE INTERRUPT FROM INTERRUPT CONTROLLER #1

        MOV     DX,INT1
        CALL    F3                     ;GET THE ACTIVE INTERRUPT

;	MOV     BL,AL    		;orig routines
        MOV     BH,AL    		;save 1st ISR in BH, 12/8/86 jwy

        JZ      F2                     ;JUMP, NO HARDWARE INTERRUPT

;CHECK ACTIVE INTERRUPT FROM INTERRUPT CONTROLLER #2

;	MOV     dl,80h			;orig routines
        MOV     dl,0a0h			;2nd interrupt controller, 12/8/86 jwy

        CALL    F3       
        JZ      F1                     ;JUMP, NO #2 HARDWARE INTERRUPT
        MOV     BL,AL

;PROCESS INTERRUPT CONTROLLER #2 INTERRUPT

	inc	dx
        CALL    F4                     ;MASK OFF ACTIVE INTERRUPT
        MOV     AL,EOI
        OUT     INT1,AL                ;EOI TO #1, 12/8/86
	jmp	short $+2
	jmp	short $+2
        OUT     INT2,AL                ;EOI TO #2
	mov	bl,bh
        JMP     short F5		;12/8/86
;	JMP     short F2		;orig routines

;
;SUBROUTINE TO GET THE ACTIVE INTERRUPT 
;

F3	proc	near

	MOV     AL,0BH
        OUT     DX,AL                  ;REQUEST ACTIVE INTERRUPT
        CALL    DELAY                          
        IN      AL,DX                  ;GET ACTIVE INTERRUPT
        OR      AL,AL                  ;ACTIVE INTERRUPT ?
        RET
f3	endp


;
;SUBROUTINE TO MASK off INTERRUPT
;
F4	proc	near

        IN      AL,DX
        OR      AL,BL
        jmp	short $+2
        OUT     DX,AL
        RET
f4	endp




;PROCESS INTERRUPT CONTROLLER #1 INTERRUPT

F1:     mov	bl,bh			;restore ISR, 12/8/86 jwy
	and	bl,0fbh			;don't disable 2nd int controller
	MOV     DX,INT1+1                            
        CALL    F4                     ;MASK OFF THE INTERRUPT        
        MOV     AL,EOI
        OUT     INT1,AL                ;EOI
F5:
	MOV     INTERRUPT_FLAG,BL

;RETURN

F2:	
	call	restore_all
        POP     AX

; 	DUMMY_MOUSE -- This is a do-nothing routine so that mouse interrupt
;		       will not be uninitialized before mouse driver is 
;		       installed.

;
;	Break:	This is a dummy interrupt entry
;
break	proc	near

	iret

break	endp


UNEXPECTED_INTERRUPT ENDP    

DUMMY_MOUSE	PROC	NEAR

	PUSH	AX
	MOV	AL,EOI
	OUT	INT1,AL
	POP	AX
	IRET
DUMMY_MOUSE	ENDP

;******************************************************************************
;
;     SEND COPROCESSOR INTERRUPT TO INTERRUPT 2
;
;******************************************************************************

MATH PROC NEAR
        PUSH    AX
        MOV     AL,EOI
        OUT     INT2,AL                ;EOI TO SLAVE
	jmp	short $+2
	jmp	short $+2
        OUT     INT1,AL                ;EOI TO MASTER
        XOR     AX,AX
        OUT     COPROCESSOR_CLR,AL     ;GET RID OF THE REQUEST
        POP     AX
        INT     2
        IRET 
MATH ENDP


;
;	Intr71:	    This Routine redirects hardware interrupt level 9
;		to interrupt level 2
;

Int71	proc	near
	push	ax
	sti				;
	mov	al,int1
	out	int2,al
	pop	ax			;restore register
	int	0ah			;send it to level 2
	iret				;return to caller
Int71	endp

;******************************************************************************
;
;     TIMER INTERRUPT  08H  UPDATE TIME OF DAY
;
;     REGISTERS CHANGED,NONE
;
;******************************************************************************

TIME PROC FAR 
        STI                            ;INTERRUPTS ON
;	PUSH    AX
;	call	save_all		;
	push	ds			;1/15/86
	push	ax
	push	dx
	mov	ax,dataa
	mov	ds,ax

	INC     TIMER_LOWER            ;UPDATE TIME
	jz	I1			;0,increment high count
	dec	timer_upper		;

I1:
        INC     TIMER_UPPER
        CMP     TIMER_LOWER,0B0H       ;TEST FOR 24 HOURS
        JNZ     I2
        CMP     TIMER_UPPER,018H      
        JNZ     I2
	xor	ax,ax			;3/11/85
        MOV     TIMER_LOWER,ax
        MOV     TIMER_UPPER,ax
        MOV     TIMER_WRAP,1
I2:     
	DEC     MOTOR_TIMOUT           ;DECREMENT FLOPPY MOTOR TIMEOUT
        JNZ     I3
	mov	dx,dor			;3/11/85
        MOV     AL,0CH
        OUT     DX,AL
	call	delay			;3/11/85
        AND     MOTOR_ON,0F0H          ;TURN OFF MOTOR

;	execute interrupt 1C at every timer tick

I3:     
	INT     1CH                    ;USER INTERRUPT
        MOV     AL,EOI                 ;END OF INTERRUPT TO TIMER
        OUT     INT1,AL 
;	call	restore_all		;restore registers
;	POP     AX
	pop	dx
	pop	ax
	pop	ds			;1/15/86
	IRET                           ;RETURN
TIME ENDP

;******************************************************************************
;
;     			TIMER  1AH
;    
;    (AH)=0  READ THE TIMER CLOCK CX = HIGH COUNT
;                                 DX = LOW COUNT
;                                 AL = 0 IF TIMER HAS NOT PASSED 24 HOURS
;                                        SINCE LAST READ
;
;    (AH)=1  SET THE TIMER CLOCK  CX = HIGH COUNT
;                                 DX = LOW COUNT
;
;    (AH)=2  READ THE CMOS CLOCK  CH = HOURS (BCD)
;                                 CL = MINUTES (BCD)
;                                 DH = SECONDS (BCD)
;                                 CY = 0 CLOCK OK     
;                                      1 CLOCK FAILED
;
;    (AH)=3  SET THE CMOS CLOCK   CH = HOURS (BCD)
;                                 CL = MINUTES (BCD)
;                                 DH = SECONDS (BCD)
;                                 DL =1 IF DAYLIGHT SAVINGS
;
;    (AH)=4 READ DATE FROM CMOS   CH = CENTURY (BCD)
;                                 CL = YEAR (BCD)
;                                 DH = MONTH (BCD)
;                                 DL = DAY (BCD)
;                                 CY = 0 CLOCK OK
;                                 CY = 1 CLOCK FAILED
;
;    (AH)=5 SET THE DATE TO CMOS  CH = CENTURY (BCD)
;                                 CL = YEAR (BCD)
;                                 DH = MONTH (BCD)
;                                 DL = DAY (BCD)
;
;    (AH)=6 SET THE ALARM         CH = HOURS (BCD)
;                                 CL = MINUTES (BCD)
;                                 DH = SECONDS
;                                 
;                                 ON EXIT  CY = 0 CLOCK OK
;                                          CY = 1 CLOCK FAILED
;                                          AX = 0 ALARM ALREAD ENABLED
;
;    (AH)=7 RESET THE ALARM
;
;
;******************************************************************************


CLOCK PROC FAR  
	sti				;set interrupts on again
	push	ax
	call	save_all		;save registers

;	JUMP TO ROUTINE

        CMP     AH,8                   ;ILLEGAL VALUE ?
        JB      G0                     ;NO, JUMP
        JMP     G16                    ;YES, EXIT
G0:     
	XOR     BX,BX
        MOV     BL,AH
        SHL     BX,1                   ;ADJUST FOR WORD
        JMP     WORD PTR CS:[BX + OFFSET G1]

;
;	Function code 0:  (AH)=0 READ CLOCK
;

G2:     
	mov	cx,timer_upper
	mov	dx,timer_lower
	MOV     word ptr [bp+bp_cx],cx		;return high word in cx
        MOV     word ptr [bp+bp_dx],dx		;and low word in dx
        MOV     AL,TIMER_WRAP		;
	mov	byte ptr [bp+bp_ax],al	;
        MOV     TIMER_WRAP,0 
        JMP     G16      

;
;	Function code 1: (AH)=1 SET CLOCK
;

G3:     MOV     TIMER_UPPER,CX         ;SET TIMER
        MOV     TIMER_LOWER,DX
        MOV     TIMER_WRAP,0
        JMP     G16

;
;	Function code 2:  (AH)=2 READ CMOS CLOCK TIME
;
;	First CHECK TO SEE IF UPDATE IS IN PROGRESS

G4:     CALL    UPDATE                                 
        JNC     G5                     ;JUMP IF NO ERROR
        JMP     G17                    ;ERROR, JUMP

;	READ HOURS, MINUTES, SECONDS

G5:     
	cli				;interrupts off during read
	MOV     AL,HOURS                           
        CALL    READ_CMOS
        MOV     CH,AH                  ;CH = HOURS
        MOV     AL,MINUTES   
        CALL    READ_CMOS                  
        MOV     CL,AH                  ;CL = MINUTES
        MOV     AL,SECONDS
        CALL    READ_CMOS
        MOV     DH,AH                  ;DH = SECONDS
        XOR     DL,DL			;

	mov	[bp+bp_dx],dx		;
	mov	[bp+bp_cx],cx		;
        JMP     G16

;	Function code 3:  (AH)=3 SET CMOS CLOCK TIME
;
;	CHECK TO SEE IF UPDATE IS IN PROGRESS

G6:     
	CALL    UPDATE
        JNC     G7                     ;JUMP IF CLOCK IS RUNNING
        CALL    START_CLOCK            ;SETUP CMOS CLOCK

;SET HOURS, MINUTES, SECONDS

G7:     MOV     AL,HOURS
        MOV     AH,CH                  
        CALL    WRITE_CMOS             ;SET HOURS
        MOV     AL,MINUTES
        MOV     AH,CL                               
        CALL    WRITE_CMOS             ;SET MINUTES
        MOV     AL,SECONDS              
        MOV     AH,DH
        CALL    WRITE_CMOS             ;SET SECONDS

;SET DAYLIGHT SAVINGS TIME

        MOV     AL,REGISTER_B
        CALL    READ_CMOS
        AND     AH,23H
        OR      AH,DL
        DEC     AL                     ;AL = REGISTER B ADDRESS
        CALL    WRITE_CMOS             ;SET DAYLIGHT SAVINGS
        JMP     G16

;
;	Function code 4:  (AH)=4 READ CMOS CLOCK DATE
;
;	CHECK TO SEE IF UPDATE IS IN PROGRESS

G8:     
	CALL    UPDATE                 ;CHECK FOR UPDATE
        JNC     G9                     ;JUMP IF NO ERROR
        JMP     G17

;READ CENTURY, YEAR, MONTH, DAY    

G9:     MOV     AL,CENTURY
        CALL    READ_CMOS
        MOV     CH,AH                  ;AH = CENTURY
        MOV     AL,YEAR 
        CALL    READ_CMOS 
        MOV     CL,AH                  ;CL = YEAR
        MOV     AL,MONTH
        CALL    READ_CMOS
        MOV     DH,AH                  ;DH = MONTH 
        MOV     AL,DATE  
        CALL    READ_CMOS
        MOV     DL,AH                  ;DL = DAY of the month
	mov	[bp+bp_cx],cx		;save it for return
	mov	[bp+bp_dx],dx		;  rlm  3/22/85
        JMP     G16

;
;	Function code 5: (AH)=5 SET THE CMOS CLOCK DATE
;
;	CHECK TO SEE IF UPDATE IS IN PROGRESS

G10:    
	CALL    UPDATE
        JNC     G11                    ;JUMP IF CLOCK IS RUNNING
        CALL    START_CLOCK            ;SETUP CLOCK

;SET THE CENTURY, YEAR, MONTH, DATE, DAY

G11:    MOV     AL,CENTURY
        MOV     AH,CH
        CALL    WRITE_CMOS             ;SET CENTURY
        MOV     AL,YEAR
        MOV     AH,CL
        CALL    WRITE_CMOS             ;SET YEAR
        MOV     AL,MONTH
        MOV     AH,DH     
        CALL    WRITE_CMOS             ;SET MONTH
        MOV     AL,DATE
        MOV     AH,DL
        CALL    WRITE_CMOS             ;SET DATE
        MOV     AL,DAY
        XOR     AH,AH
        CALL    WRITE_CMOS             ;SET DAY 

;ENABLE UPDATE CYCLES

        MOV     AL,REGISTER_B
        CALL    READ_CMOS
        AND     AH,07FH
        DEC     AL                     ;AL = REGISTER B ADDRESS
        CALL    WRITE_CMOS             ;ALLOW UPDATE CYCLES
        JMP     G16

;	Function code 6: (AH)=6 SET THE CMOS CLOCK ALARM
;
;	CHECK TO SEE IF ALARM IS ALREADY ENABLED

G12:    
	MOV     AL,REGISTER_B
        CALL    READ_CMOS
        TEST    AH,20H                 ;ALARM ENABLED ?
        JZ      G13                    ;NO, JUMP
        XOR     AX,AX
        JMP     G17

;CHECK TO SEE IF UPDATE IS IN PROGRESS

G13:    CALL    UPDATE
        JNC     G14                    ;JUMP IF CLOCK IS RUNNING
        CALL    START_CLOCK

;SET HOUR, MINUTE, SECOND OF ALARM

G14:    MOV     AL,HOUR_ALARM
        MOV     AH,CH
        CALL    WRITE_CMOS             ;SET ALARM HOURS
        MOV     AL,MIN_ALARM
        MOV     AH,CL          
        CALL    WRITE_CMOS             ;SET ALARM MINUTES
        MOV     AL,SEC_ALARM
        MOV     AH,DH
        CALL    WRITE_CMOS             ;SET ALARM SECONDS

;TURN ALARM ON IN CMOS CLOCK

        MOV     AL,REGISTER_B
        CALL    READ_CMOS
        AND     AH,07FH
        OR      AH,20H
        DEC     AL                     ;AL = REGISTER B ADDRESS
        CALL    WRITE_CMOS             ;TURN ALARM ON

;TURN ON ALARM INTERRUPT

        IN      AL,INT2+1              ;GET INTERRUPT MASK
        AND     AL,0FEH
        CALL    DELAY
        OUT     INT2+1,AL              ;ENABLE INTERRUPT
        JMP     G16

;
;	Function code 7: (AH)=7 RESET THE CMOS CLOCK ALARM
;

G15:    
	MOV     AL,REGISTER_B
        CALL    READ_CMOS
        AND     AH,57H                            
        DEC     AL                     ;AL = REGISTER B ADDRESS
        CALL    WRITE_CMOS             ;ALARM OFF

;
;NORMAL RETURN
;

G16:    STI
	clc				;clear carry for normal return
	call	restore_all
	pop	ax			;
        IRET     

;
;ERROR RETURN 
;

G17:    STI
	stc				;set carry for error return
	call	restore_all
	pop	ax
        RET     2

;
;	jump table
;

G1 LABEL WORD
	DW 	OFFSET G2
	DW 	OFFSET G3
	DW 	OFFSET G4
	DW 	OFFSET G6
	DW 	OFFSET G8
	DW 	OFFSET G10
	DW 	OFFSET G12
	DW 	OFFSET G15

CLOCK ENDP   

;******************************************************************************
;
;     			ALARM    70H 
;
;     THIS INTERRUPT COMES FROM THE CMOS CLOCK CALENDAR.        
;     THERE ARE TWO CONDITIONS WHICH MAY CAUSE IT:
;
;     1. PERIODIC--ABOUT 1024 INTERRUPTS PER SECOND
;
;     2. EVENT--CAUSED BY THE ALARM OF THE CALENDAR CHIP
;
;******************************************************************************

ALARM PROC NEAR

        STI
        PUSH    AX
	call	save_all		;

;TEST FOR CMOS PERIODIC INTERRUPT

        MOV     AL,REGISTER_B
        CALL    READ_CMOS
        MOV     BH,AH
        CALL    READ_CMOS
        AND     BH,AH
        TEST    BH,40H                 ;PERIODIC CMOS INTERRUPT ?
        JZ      PI0                    ;NO, JUMP

;DECREMENT PERIODIC INTERRUPT COUNTS

        SUB     COUNT_LOW,3D0H
        SBB     COUNT_HIGH,0
        JA      PI0                    ;JUMP IF NOT END OF COUNT

;TURN OFF PERIODIC INTERRUPT

        MOV     AL,REGISTER_B
        CALL    READ_CMOS           
        AND     AH,0BFH
        DEC     AL                     ;AL = REGISTER B ADDRESS
        CALL    WRITE_CMOS             ;TURN OFF PERIODIC INTERRUPT

;SET USER FLAG

        MOV     event_wait_flag,0
        LDS     DI,dword ptr event_flag_addr
        MOV     BYTE PTR[DI],80H      ;USER FLAG

;TEST FOR ALARM INTERRUPT        

PI0:    
	TEST    BH,20H                 ;ALARM INTERRUPT ?
        JZ      PI1                    ;NO, JUMP
        INT     4AH                    ;USER INTERRUPT

;END INTERRUPT

PI1:    MOV     AL,EOI
        OUT     INT2,AL                ;EOI TO INTERRUPT CONTROLLER #2
	jmp	short $+2
	jmp	short $+2
        OUT     INT1,AL                ;EOI TO INTERRUPT CONTROLLER #1
	call	restore_all
        POP     AX                      
        IRET
ALARM ENDP

;	Intr 12h:  Get ram size
;		  On output Ax = memory size in 1 k bytes
;

ram_size_1	proc	near

	sti
	push	ds
	call	set_data
	mov	ax,memory_size
	pop	ds
	iret

ram_size_1	endp


;
;	Int 11h:  Configuration word
;		On output AX contains configuration word defined as 
;		follows:
;
;	Bit 0 = 		bit 0 = floppy present
;	Bit 1 =			bit 1 = 80287 present
;	Bits 2 -3 = 0
;	bits 5 - 4 = Monitor type	00 = no monitor
;					01 = 40X25
;					10 = 80X25
;					11 = 80X25
;	Bits 6 - 7 = # of disks
;	Bit 8	   = 0
;	Bits 9 - 11 = # of RS-232 ports
;	Bit  12	    = Game IO
;	Bit 13	    = 0
;	Bit 14 - 15 = # of printers
;

config_1	proc	near
	sti
	push	ds
	call	set_data
	mov	ax,equip_flag
	pop	ds
	iret
config_1	endp

;******************************************************************************
;
;     PARITY ERROR  NMI
;
;     DETERMINE CAUSE OF PARITY ERROR AND DISPLAY ON CONSOLE
;
;******************************************************************************

PARITY_ERROR PROC FAR 

        PUSH    AX
        PUSH    BX
        PUSH    CX
        PUSH    DS
        PUSH    DX
        PUSH    SI

;DETERMINE IF THE INTERRUPT WAS CAUSE BY ERROR

        IN      AL,PIO                 ;ERROR TO (AL)
        TEST    AL,0C0H                ;ERROR ?
        JZ      B5                     ;NO, EXIT              

;DISPLAY INTERNAL ERROR

;        IN      AL,PIO  
	MOV     SI,OFFSET ERROR_MSG_2	;Parity error on expansion board, CR, LF
        TEST    AL,80H                 ;INTERNAL MEMORY CHECK
        JZ      B0                     ;NO, JUMP

        MOV     SI,OFFSET ERROR_MSG_1	;Parity error on main circuit board, CR, LF

;CLEAR SCREEN

B0:	MOV     BX,DATA
        MOV     DS,BX
        MOV     AL,VIDEO_MODE
        XOR     AH,AH
        INT     10H

        CALL    DISPLAY			;DISPLAY ERROR MESSAGES

;TURN OFF THE NMI AND CLEAR THE ERRORS

B1:     MOV     AL,80H
        CALL    READ_CMOS              ;OFF NMI
        IN      AL,PIO  
        OR      AL,0CH
        CALL    DELAY
        OUT     PIO,AL                 ;CLEAR ERRORS
        AND	AL,03H  
        CALL    DELAY
        OUT     PIO,AL                 ;ENABLE ERRORS

;SEARCH MEMORY FOR ERROR

        MOV     BX,MEMORY_SIZE
        XOR     DX,DX
B2:     MOV     DS,DX
        XOR     SI,SI
        MOV     CX,8000H
        REPZ    LODSW
        IN      AL,PIO  
        TEST    AL,0C0H                ;ERROR ?
        JZ      B3                     ;NO, CONTINUE TESTING

        MOV     SI,OFFSET ERROR_MSG_3	;Fatal Error --  Computer stopped
        CALL    DISPLAY
        CLI
        HLT                            ;HALT, FATAL ERROR

B3:     ADD     DX,800H
        SUB     BX,64                  ;ALL MEMORY TESTED ?
        JNZ     B2                     ;NO, CONTINUE TESTING

;NO ERROR FOUND, CONTINUE

        MOV     SI,OFFSET ERROR_MSG_4	;Press F1 key to continue
        CALL    DISPLAY
 B4:    XOR     AH,AH
        INT     16H                    ;GET CHARACTER FROM KEYBOARD
        CMP     AX,3B00H               ;"F1" KEY ?
        JNE     B4                     ;NO, TRY AGAIN

;TURN NMI BACK ON

B5:	XOR     AL,AL    
        CALL    READ_CMOS              ;NMI ON

;EXIT
	POP     SI
        POP     DX
        POP     DS
        POP     CX
        POP     BX
        POP     AX
        IRET
PARITY_ERROR ENDP

;******************************************************************************
;
;	BOOT STRAP LOADER	19H
;
;	READ SYSTEM FROM FLOPPY DISK OR HARD DISK (IF PRESENT)
;	AND JUMP TO OPERATING SYSTEM
;
;******************************************************************************

BOOT PROC FAR

;SETUP STACK FOR BOOT

	MOV	AX,stack_seg		;setup stack segment
	MOV	SS,AX
	MOV	SP,top_of_stack

;	move the address of floppy disk parameters in the low memory

	mov	si,offset CRLF_MSG	;CR LF
	call	display

	mov	ax,0
	mov	ds,ax			;setup DS
	assume	ds:zero

	mov	bx,offset floppy_parms
	mov	word ptr dsk_parm_ptr,bx	;set diskette parameter vector
	mov	word ptr dsk_parm_ptr+2,cs

	MOV	AX,DATA
	MOV	DS,AX
	assume	ds:dataa

;	TRY TO READ FLOPPY

BT1:	MOV	CX,5			;RETRY COUNT
BT2:	xor	dx,dx			;diskette
	XOR	AX,AX			;reset
	INT	13H			;CLEAR THE DISK
	JC	BT3			;JUMP IF RESET ERROR
	CALL	BT10			;READ THE FLOPPY
BT3:	JNC	BT4			;JUMP IF NO ERROR
	TEST	AH,80H			;SEE IF TIME OUT ERROR
	JNZ	BT5			;YES, GO TRY FIXED DISK
	LOOP	BT2
	JMP	SHORT BT5		;GO TRY HARD DISK

BT4:
	cmp	byte ptr es:[bx],06	;check valid opcode
	jb	bt9			;bad read
	mov	ax,word ptr es:[bx]	;check first 4 bytes
	cmp	ax,word ptr es:[bx+2]	;if all equal, then not a system
	je	bt9			;disk,otherwise jump to boot blk

					;if we get here, the first byte
					;of the boot block was a valid jump
good_block:
	db	0eah			;make a far jump to 0:7C00h
	dw	07C00h			;offset
	dw	00h			;segment

;	CHECK TO SEE IF HARD DISK IS PRESENT

BT5:
	TEST	data_rate,1		;see if fixed disk/diskette card
	JZ	BT9			;NO, JUMP

;fix for seagate disk controller
;	cmp	hf_num,0		;HARD DISK PRESENT ?
;	JZ	BT9			;NO, JUMP
;end fix for seagate fixed disk controller

	mov	al,cmos_status_1	;get cmos status byte
	call	read_cmos
	test	ah,hd_fail		;check pod error flag
	jnz	BT9			;there was an error on pod

;	TRY TO BOOT FROM HARD DISK

	MOV	CX,5
BT6:	MOV	DX,80H
	XOR	AX,AX
	INT	13H			;RESET THE DISK
	JC	BT7			;JUMP IF RESET ERROR
	CALL	BT10			;READ THE HARD DISK
BT7:	JNC	BT8			;JUMP IF NO ERROR
	LOOP	BT6
	JMP	SHORT BT9

;	CHECK TO SEE IF DATA IN MEMORY IS CORRECT

BT8:	CMP	es:[bx+1feh],0AA55H	;valid partition table?
	je	good_block		;make a far jump to location 0:7C00h

;SEND ERROR MESSAGE TO SCREEN AND WAIT FOR KEYBOARD INPUT

BT9:	int	18h

	MOV	SI,OFFSET BOOT_MSG_1	;Boot failure, check disk and hit any key to try again",CR,LF
	CALL	DISPLAY
	XOR	AX,AX
	INT	16H			;WAIT FOR KEYBOARD INPUT
	JMP	BT1			;TRY TO BOOT AGAIN

;SUBROUTINE TO READ THE DISK
;input	dh - head #
;	dl - drive #
;output	ES:BX - data transfer addr.
;	CY = 1 if error 

BT10 PROC NEAR
	push	cx			;save retry count
	xor	ax,ax			;setup dta addr.
	mov	es,ax
	MOV	BX,7C00H		;ADDRESS
	MOV	AX,0201H		;READ ONE SECTOR COMMAND
	MOV	CX,1			;TRACK 0, sector 1
	INT	13H			;READ THE DISK
	pop	cx			;restore retry count
	RET
BT10 ENDP
BOOT ENDP


BOOT_MSG_1	LABEL BYTE
	DB "Boot failure, check disk and hit any key to try again",CR,LF,-1
ERROR_MSG_1	LABEL BYTE
	DB "Parity error on main circuit board", CR, LF, -1
ERROR_MSG_2	LABEL BYTE
	DB "Parity Error on Expansion Bus", CR, LF, -1
ERROR_MSG_3	LABEL BYTE
	DB "Non-recoverable error -- Processor halted", -1
ERROR_MSG_4	LABEL BYTE
	DB "Press F1 key to continue"
CRLF_MSG	LABEL	BYTE
	DB	CR, LF, -1

eproma	ends
	end

