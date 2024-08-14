page 60,132
include	title.inc
subttl  MISC   Miscellaneous Routines

.xlist
include	equates.inc

extrn	flag_1:byte
extrn	flag_2:byte
extrn	flag_3:byte
extrn	break:near
extrn	equip_flag:word
extrn	vir_mem_size:word

space	equ	20h

public	set_data
public	save_all
public	restore_all
public	start_clock
public	delay
public	nop_proc
public	beep
public	display
public	end_interrupt
public	move
public	wait_upi
public	read_switch
public	ccheck
public	readkey
public	convert
public	update
public	read_cmos
public	write_cmos
public	chk_cmos_good
public	shut
public	tvideo
public	convt_bin_dec
public	chk_clk
public	fixed_wait

page

.list


dataa	segment	  at	40h
dataa	ends

eproma	segment	 byte public

assume	cs:eproma,ds:dataa

;
;	start of main code for this module

start	equ	$+3A0h

page
;
;	Routine to see if clock is running or not. Set carry if
;	error.
;
chk_clk		proc	near
	push	ax
	call	save_all
	mov	dl,5
chk_clk_1:
	xor	cx,cx
chk_clk_2:
	mov	al,register_a+80h
	call	read_cmos		;go read status register
	test	ah,80h			;clock update
	jnz	chk_clk_3		;yes,test compl. state
	loop	chk_clk_2		;wait till done
	dec	dl
	jnz	chk_clk_1		;
	jmp	short chk_clk_err	;error return
chk_clk_3:
	mov	cx,1000h
chk_clk_4:
	dec	al			;point to right port
	call	read_cmos
	test	ah,080h			;
	jz	chk_clk_rtn		;changed state ok
	loop	chk_clk_4		;wait till done
chk_clk_err:
	stc				;error set carry
chk_clk_rtn:
	call	restore_all
	pop	ax
	ret
chk_clk	endp

;
;	set DS register routine
;

set_data	proc	near
	push	ax
	mov	ax,dataa
	mov	ds,ax
	pop	ax
	ret
set_data	endp

; save_all --	Saves all registers
;
; Input:	ax has already been saved before entering this routine
;
; Output:	bx,cx,dx,ds,es,si,di,bp are pushed in that order.
;		Then,	ds = 40h	dataa area
;			bp = top of stack
;		All other registers preserved.
;		After calling "save_all", the saved registers may be referenced
;		by "[bp + bp_<reg>]", where <reg> is a 2-letter register
;		designation in lower case.
;		In order to load half register (i.e al or ah) add 0 for lower
;		register and 1 for higher register.
;		To get al register reference it by "[bp+bp_ax]", and to get ah
;		register reference it by "[bp+bp_ax+1]"


save_all	proc	near
	pop	ax			; get return address
	push	bx			; save the rest of the registers
	push	cx
	push	dx
	push	ds
	push	es
	push	si
	push	di
	push	bp
	mov	bp,sp			; set bp to top of stack
	push	ax			; put return address
	mov	ax,dataa		;get dataa area
	mov	ds,ax			;
	mov	ax,[bp+bp_ax]		; restore ax
	ret
save_all	endp

;	Restore_all:	This routine will restore all registers except
;	ax register which will be restored after leaving this routine.
;

restore_all	proc	near
	pop	ax			; get return address
	pop	bp			; restore the registers
	pop	di
	pop	si
	pop	es
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	push	ax			; put return address
	ret
restore_all	endp

page


;******************************************************************************
;
;     	DELAY  ROUTINE:	a dummy routine to waste some time
;
;******************************************************************************

DELAY 	PROC 	NEAR
nop_proc	proc	near
        RET
nop_proc	endp	
DELAY 	ENDP

page
;
;******************************************************************************
;
;     BEEP
;
;     ENTER: BL = BEEP LENGTH
;
;******************************************************************************
;
BEEP PROC NEAR
;
;SAVE REGISTERS
        PUSH    AX 
        PUSH    BX
        PUSH    CX
;TURN SPEAKER ON AND OFF
        IN      AL,PIO                 ;GET CONTROL PORT 
BEEP0:  AND     AL,0FCH
        OUT     PIO,AL                 ;OFF SPEAKER
        MOV     CX,0CFH
BEEP1:  LOOP    BEEP1
        OR      AL,2
        OUT     PIO,AL                 ;ON SPEAKER 
        MOV     CX,0CFH
BEEP2:  LOOP    BEEP2 
        DEC     BL
        JNZ     BEEP0
;RECOVER REGISTERS AN RETURN
        POP     CX
        POP     BX
        POP     AX
        RET                            ;RETURN
BEEP    ENDP

page
 
;******************************************************************************
;
;     SUBROUTINE TO DISPLAY MESSAGE 
;
;     ENTER: SI = BEGINNING OF MESSAGE                        
;
;******************************************************************************

DISPLAY 	PROC 	NEAR
        PUSH    BX
DISP1:  MOV     AL,CS:[SI]             ;GET CHARACTER TO BE TRANSMITTED
        CMP     AL,-1                  ;DONE ?
        JE      DISP2                  ;YES, JUMP
        MOV     AH,0EH                                    
        MOV     BL,7H
        INT     10H                    ;DISPLAY CHARACTER
        INC     SI 
        JMP     DISP1
DISP2:  POP     BX
        RET
DISPLAY ENDP


;******************************************************************************
;
;     SUBROUTINE TO MOVE A VECTOR TO INTERRUPT TABLE
;
;     ENTER:  DI = VECTOR DESTINATION, AX = VECTOR
;
;******************************************************************************

MOVE 	PROC 	NEAR

        PUSH    BX
        PUSH    ES
        XOR     BX,BX
        MOV     ES,BX                  ;ES = 0
        MOV     ES:[DI],AX             ;VECTOR TO TABLE 
        ADD     DI,2
        MOV     BX,CS
        MOV     ES:[DI],BX             ;SEGMENT TO VECTOR TABLE
        ADD     DI,2
        POP     ES
        POP     BX
        RET
MOVE 	ENDP   
page


;******************************************************************************
;
;     SUBROUTINE TO WAIT FOR UPI BUFFERS TO BE EMPTY
;
;     AH  INDICATES WHICH BUFFER TO CHECK
;
;     BIT 0 FOR OUTPUT BUFFER
;     BIT 1 FOR INPUT BUFFER
;
;******************************************************************************

WAIT_UPI 	PROC 	NEAR

        CLI
	push	cx
        XOR     CX,CX
WBUF1:  IN      AL,UPI+4               ;GET STATUS
        TEST    AL,AH                  ;BUFFERS BUSY
        JZ      WBUF2                  ;NO, RETURN
        LOOP    WBUF1
WBUF2:  
	pop	cx
;	sti
	nop
	RET

WAIT_UPI ENDP





;******************************************************************************
;
;     CHECK FOR CLOCK UPDATE IN PROGRESS
;
;     CY = 0 NO UPDATE IN PROGRESS
;          1 ERROR, UPDATE STUCK
;
;     AL DESTROYED
;
;******************************************************************************

UPDATE 	PROC 	NEAR

        CLI
        PUSH    CX      
        MOV     CX,0
UIP0:   MOV     AL,REGISTER_A
        CALL    READ_CMOS              ;READ UPDATE REGISTER
        TEST    AH,80H                 ;UPDATE IN PROGRESS ?
        JZ      UIP1                   ;NO, EXIT
        DEC     AL                     ;AL = UPDATE REGISTER
        LOOP    UIP0                   ;YES, TRY AGAIN
        STC                            ;INDICATE ERROR
UIP1:   POP     CX
        RET
UPDATE ENDP

page

;******************************************************************************
;
;     SUBROUTINE TO READ CMOS MEMORY
;     
;     ENTER: AL = ADDRESS
;
;     EXIT: AH = DATA, AL = ADDRESS + 1
;
;******************************************************************************

READ_CMOS PROC NEAR

        OUT     CMOS,AL                ;SEND ADDRESS TO CMOS
        XCHG    AH,AL
        CALL    DELAY
        IN      AL,CMOS+1              ;GET DATA
        XCHG    AH,AL
        INC     AL                     ;INCREMENT ADDRESS
        RET             
READ_CMOS ENDP


;******************************************************************************
;
;     SUBROUTINE TO WRITE CMOS MEMORY
;
;     ENTER:  AL = ADDRESS, AH = DATA
;
;     EXIT: AH = DATA, AL = ADDRESS + 1
;
;******************************************************************************

WRITE_CMOS PROC NEAR 

        OUT     CMOS,AL                ;SEND ADDRESS TO CMOS
        XCHG    AL,AH
        CALL    DELAY
        OUT     CMOS+1,AL              ;SEND DATA
        XCHG    AH,AL
        INC     AL                     ;INCREMENT ADDRESS
        RET
WRITE_CMOS ENDP


;	routine to test for cmos battery and checksum values
;	input:	al = port to be read from cmos
;	output:	Z flag = 0	cmos good
;		       = 1	cmos bad
;

chk_cmos_good	proc	near
	mov	al,0eh			;read cmos status byte .. 11/20/84
	call	read_cmos
	test	ah,0c0h			;check if these 2 bits are set
	ret				;error if set
chk_cmos_good	endp

page



;
;******************************************************************************
;
;    SUBROUTINE TO PREPARE FOR A SHUTDOWN (EXIT PROTECTED MODE)
;  
;    ENTER AH = SHUTDOWN NUMBER
;    
;******************************************************************************
;
SHUT PROC NEAR
;
;PUT SHUTDOWN NUMBER IN CMOS
        MOV      AL,SHUTDOWN
        CALL     WRITE_CMOS
	jmp	st2

;SET SHUTDOWM FLAG IN UPI
	MOV      AH,3
	CALL     WAIT_UPI              ;WAIT IN AND OUT BUFFERS
	MOV      AL,20H
	OUT      UPI+4,AL              ;REQUEST COMMAND BYTE
ST1:    IN       AL,UPI+4		;read status byte
	TEST     AL,1                  ;OUTPUT BUFFER FULL ?
	JZ       ST1                   ;NO, TRY AGAIN
	IN       AL,UPI                ;AL = COMMAND BYTE
	OR       AL,4                  ;SET SHUTDOWN BIT
	MOV      BL,AL                 ;SAVE IT
	MOV      AH,2
	CALL     WAIT_UPI              ;WAIT FOR INPUT BUFFER
	MOV      AL,60H
	OUT      UPI+4,AL              ;GET READY FOR COMMAND BYTE
	MOV      AH,2
	CALL     WAIT_UPI              ;WAIT FOR INPUT BUFFER
	MOV      AL,BL
	OUT      UPI,AL                ;SEND NEW COMMAND BYTE
st2:	ret				;return to caller
SHUT ENDP

page


;******************************************************************************
;
;     SUBROUTINE TO TEST FOR NO VIDEO PRESENT                             
;     IF THERE IS NONE, THE ROUTINE WILL:           
;     1.SET VIDEO ROUTINE TO RETURN
;     2.SET VIDEO BITS IN EQUIPMENT FLAG TO 0
;
;******************************************************************************

TVIDEO PROC NEAR

        PUSH    AX
        PUSH    DI
        PUSH    DS
        PUSH    SI
        MOV     SI,0B000H
        MOV     DS,SI                  ;DISPLAY SEGMENT

;TEST FOR MONOCHROME DISPLAY

        MOV     SI,1 
        CALL    TVD2     
        CMP     AX,0707H               ;VALID ATTRIBUTE ?
        JE      TVD1                   ;YES, JUMP

;TEST FOR COLOR GRAPHICS 

        MOV     SI,8001H
        CALL    TVD2      
        CMP     AX,0707H               ;VALID ATTRIBUTE ?
        JE      TVD1                   ;YES, JUMP

;SET VIDEO ROUTINE TO RETURN

        MOV     DI,40H
        MOV     AX,OFFSET BREAK
        CALL    MOVE

;CLEAR EQUIPMENT FLAG BITS

        MOV     AX,DATA
        MOV     DS,AX
        AND     EQUIP_FLAG,0FFCFH

;RETURN

TVD1:   POP     SI
        POP     DS
        POP     DI
        POP     AX
        RET

;
;SUBROUTINE TO TEST FOR VIDEO PRESENT
;

TVD2:   MOV 	AX,707H
	MOV	[SI],AL
	MOV	[SI+2],AH
	XOR	AX,AX
	MOV    AL,[SI]
        MOV    AH,[SI+2]
        RET
TVIDEO ENDP

page


;******************************************************************************
;
;     SUBROUTINE TO INITILIZE CMOS CLOCK
;
;******************************************************************************

START_CLOCK PROC NEAR

        MOV     AL,REGISTER_A
        MOV     AH,26H
        CALL    WRITE_CMOS             ;32KHZ, PERIODIC INTERRUPT
        MOV     AH,02H
        CALL    WRITE_CMOS             ;24 HOUR MODE, BCD MODE
        CALL    READ_CMOS              ;CLEAR REGISTER C
        CALL    READ_CMOS              ;CLEAR REGISTER D
        RET

START_CLOCK ENDP

PAGE

;******************************************************************************
;     SUBROUTINE TO CONVER BCD NUMBER TO HEX NUMBER
;     MAXIMUM NUMBER THAT CAN BE CONVERTED = 99
;
;     ENTRY: AH = NUMBER TO CONVERT
;
;     EXIT: AH = CONVERTED NUMBER              
;
;     REGISTERS DESTROYED: AL
;
;******************************************************************************

CONVERT PROC NEAR

        PUSH    BX
	mov	al,0ffh			;return code for invalid BCD number
	mov	bh,ah			;input value
	and	bh,0f0h			;mask off low nibble
	cmp	bh,0a0h			;check high nibble
	jae	CONV1			;invalid BCD number
	mov	bh,ah			;input value
	and	bh,0fh			;mask off high nibble
	cmp	bh,0ah			;check low nibble
	jae	CONV1			;invalid BCD number

        XOR     AL,AL                  ;START WITH 0
        XOR     BX,BX

;CONVERT THE NUMBER 

CONV0:  CMP     AH,BL                  ;DONE ?
        JE      CONV1                  ;YES, JUMP
        INC     AL
        INC     BL
        MOV     BH,BL
        AND     BH,0FH
        CMP     BH,0AH                 ;CARRY ?
        JNE     CONV0                  ;NO, CONTINUE
        AND     BL,0F0H                ;LOW BITS TO 0
        ADD     BL,10H                 ;ADD IN CARRY
        JMP     CONV0

;RETURN

CONV1:  MOV     AH,AL                  ;AH = CONVERTED NUMBER
        POP     BX
        RET
CONVERT ENDP


                               
;******************************************************************************
;
;     SUBROUTINE TO GET A NUMBER FROM THE KEYBOARD AND DISPLAY IT
;
;     ENTER:  BH = UPPER RANGE 0F VALID NUMBER
;             BL = LOWER RANGE OF NUMBER
;     EXIT:   AH = NUMBER RECEIVED FROM KEYBOARD (IN HEX)
;             AL = NUMBER RECEIVED FROM KEYBOARD (IN BCD)
;
;     NOTE: IF THE FIRST CHARACTER RECEIVED FROM THE KEYBOARD IS A CR,
;           A ZERO WILL BE RETURNED IN AH AND AL.
;
;
;******************************************************************************

READKEY PROC NEAR

        PUSH    CX
        PUSH    DX

;INITILIZE PARAMETERS

RK0:    MOV     DH,-1                  ;SECOND CHARACTER 
        MOV     DL,-1                  ;FIRST CHARACTER
        MOV     CH,1                   ;COLUMN COUNTER

;GET A NUMBER FROM THE KEYBOARD

RK1:    CALL    RK8                    ;GET THE CHARACTER
        CMP     AL,CR                  ;CR ?
        JE      RK3                    ;YES, JUMP
        CMP     CH,2                   ;FIRST COLUMN   
        JE      RK2                    ;YES, JUMP
        MOV     DH,AL                  ;SECOND CHARACTER TO DH
        JMP     RK1
RK2:    MOV     DL,AL                  ;FIRST CHARACTER TO DL
        JMP     RK1

;COVERT THE NUMBER      

RK3:    MOV     AX,DX
        CMP     AX,0FFFFH              ;DEFAULT CHARACTER
        JNE     RK4                    ;NO, JUMP
        XOR     AX,AX
        JMP     RK7                    ;EXIT
RK4:    CMP     AH,-1                  ;SECOND CHARACTER ?    
        JNE     RK5                    ;YES, JUMP
        XOR     AH,AH
        XCHG    AH,AL
RK5:    MOV     CL,4
        SHL     AL,CL
        OR      AH,AL                  ;MERGE THE NUMBERS TOGATHER
        MOV     CL,AH                  ;SAVE THE BCD NUMBER
        CALL    CONVERT                ;CONVER TO HEX
        MOV     AL,CL                  ;RECOVER THE BCD NUMBER

;CHECK FOR VALID NUMBER

        CMP     AH,BH                  ;IN RANGE ?
        JA      RK6                    ;NO, JUMP
        CMP     AH,BL                  ;IN, RANGE ?
        JB      RK6                    ;NO, JUMP
        JMP     RK7
RK6:    MOV     SI,OFFSET READKEY_MSG_1
        CALL    DISPLAY                ;SEND ERROR MESSAGE
        JMP     RK0

;RETURN

RK7:    PUSH    AX
        MOV     SI,OFFSET READKEY_MSG_2
        CALL    DISPLAY                ;SEND CR, LF
        POP     AX
        POP     DX
        POP     CX
        RET
readkey	endp



;
;GET CHARACTER FROM KEYBOARD AND DISPLAY IT
;

RK8 PROC NEAR

RK9:    XOR     AH,AH
        INT     16H                    ;READ THE KEYBOARD

;CHECK FOR CR

        CMP     AL,CR                  ;CR ?
        JE      RK12                   ;YES, EXIT 

;PROCESS BACKSPACE

        CMP     AL,BS                  ;BACK SPACE?
        JNE     RK10                   ;NO, JUMP
        CMP     CH,1                   ;FIRST COLUMN ?
        JE      RK9                    ;YES, TRY AGAIN
        DEC     CH 
        MOV     AH,14
        INT     10H                    ;SEND BACKSPACE
        JMP     RK9                          

;PROCESS SPACE

RK10:   CMP     AL,SPACE
        JNE     RK11
        CMP     CH,1                   ;FIRST COLUMN   
        JNE     RK9                    ;NO, TRY AGAIN 
        INC     CH
        MOV     AH,14
        INT     10H
        JMP     RK9

;PROCESS NUMBER

RK11:   CMP     CH,3                   ;COLUMN 3 ?
        JE      RK9                    ;YES, TRY AGAIN
        CMP     AL,30H                 ;VALID NUMBER
        JB      RK9                    ;NO, TRY AGAIN
        CMP     AL,39H                 ;VALID NUMBER
        JA      RK9                    ;NO, TRY AGAIN
        INC     CH
        PUSH    AX
        MOV     AH,14
        INT     10H                    ;DISPLAY THE NUMBER
        POP     AX
        SUB     AL,30H                 ;CONVER TO BCD

;RETURN

RK12:   RET        
RK8 ENDP

page

;******************************************************************************
;
;     SUBROUTINE TO GENERATE CHECKSUM ON CMOS MEMORY
;
;     EXIT: BX = CHECKSUM
;           ZF = 0, CHECKSUM OK
;           ZF = 1, CHECKSUM ERROR
;
;******************************************************************************

CCHECK PROC NEAR

;GENERATE CHECKSUM

        MOV     AL,CMOS_START+80H
        MOV     CX,CMOS_SIZE
        XOR     BX,BX
CK0:    CALL    READ_CMOS
        PUSH    AX
        XCHG    AH,AL
        XOR     AH,AH
        ADC     BX,AX
        POP     AX
        LOOP    CK0

;CHECK FOR ERROR

        MOV     AL,CHECKSUM_ADR
        CALL    READ_CMOS
        MOV     CH,AH
        CALL    READ_CMOS
        MOV     CL,AH
        CMP     CX,BX                  ;ERROR ?
        RET
CCHECK ENDP



;******************************************************************************
;
;     THIS SUBROUTINE READ THE SWITCH SETTINGS AND RETURNS THE RESULTS
;     IN AL
;
;     AL BIT     SWITCH
;     ------     -------------
;      0-3       USER DEFINED
;       4           ----     
;       5        -MANUFACTURING TEST MODE
;       6        +MONOCHROME ADAPTER
;       7        -KEYBOARD INHIBIT (KEY SWITCH OFF)
;
;******************************************************************************

READ_SWITCH PROC NEAR

        MOV     AH,3
        CALL    WAIT_UPI               ;WAIT FOR IN AND OUT BUFFERS
	jz	SW3
	in	al,upi			;flush output buffer
SW3:    MOV     AL,0C0H
        OUT     UPI+4,AL               ;READ SWITCH COMMAND
	call	delay
	push	cx
	mov	cx,0			;inner loop counter
	mov	ah,2			;outer loop counter
SW10:	dec	cx			;decrement inner loop counter
	jcxz	SW11			;handle outer loop counter
	IN      AL,UPI+4		;get status register
        TEST    AL,1                    ;OUTPUT BUFFER FULL ?
        JNZ     SW12                    ;OBF full, get data
	jmp	SW10			;
SW11:	dec	ah			;decrememt outer loop count
	jnz	SW10			;wait more
SW12:   IN      AL,UPI                 ;GET SWITCH SETTINGS
	pop	cx
        RET
READ_SWITCH ENDP


page


;******************************************************************************
;
;     SUBROUTINE TO END INTERRUPTS
;
;******************************************************************************

END_INTERRUPT PROC NEAR
	mov	al,0FFh			;
        OUT     INT1+1,AL              ;MASK INTERRUPTS OFF
        MOV     AL,EOI
        CALL    DELAY
        OUT     INT1,AL                 ;EOI TO INTERRUPT CONTROLLER
        RET
END_INTERRUPT ENDP





;
;	Routine to convert a binary number in ax to decimal and display
;	on the screen one digit at a time. The number can be from 0 to
;	64k. No registers are saved. Leading zeros are converted to spaces
;

convt_bin_dec	proc	near

	push	ax			;save the original #
	mov	dx,0
	mov	bx,10000
	div	bx			;get first high order digit
	call	prt_num			;go display it
	mov	bx,1000			;next digit
	mov	dx,0			;
	div	bx			;
	call	prt_num
	mov	bx,100
	mov	dx,0
	div	bx			;
	call	prt_num			;
	mov	bx,10			;
	mov	dx,0
	div	bx			;
	call	prt_num			;
	call	prt_num			;
	and	vir_mem_size,07fffh	;
	pop	ax			;
	ret
convt_bin_dec	endp

page

;
;	Routine prt_num:  print a decimal digit in al. Convert leading
;	zeros to spaces.
;

prt_num		proc	near
	push	dx
	cmp	al,0			;is it zero
	jnz	prt_num_1		;no
	test	vir_mem_size,8000h	;is this leading zero
	jnz	prt_num_1		;no, then print it
	mov	al,20h			;yes, then change it to space
	call	prt_char		;go send it screen
	pop	ax			;put dx into ax
	ret				;and return

prt_num_1:
	or	vir_mem_size,8000h	;set flag
	add	al,30h			;convert it to ascii digit
	call	prt_char		;
	pop	ax			;put dx into ax
	ret				;
prt_num	endp


;
;	send char in al to video screen
;

prt_char	proc	near
	mov	ah,14			;function code for Int 10
	mov	bl,7
	int	10h			;
	mov	dx,0			;
	ret				;
prt_char	endp


;
;	This routine is used to measure time in microseconds. It is
;	hardware dependant and not processor speed dependant. It
;	measures time by checking the change of status of refresh
;	bit in port 61h. It takes 15 micro seconds to go through
;	the routine once.
;
fixed_wait	proc	near
	push	ax
	push	dx
	mov	dx,61h		;
	mov	ah,10h		;assume refresh bit is set
fixed_wait_1:
	in	al,dx		;get input 
	and	al,10h		;check for refresh bit
	cmp	ah,al		;has the bit changed state
	je	fixed_wait_1	;not yet
	mov	ah,al		;yes, set new state in ah
	loop	fixed_wait_1	;
	pop	dx		;
	pop	ax		;
	ret
fixed_wait	endp



READKEY_MSG_1  LABEL BYTE     

	DB CR, LF, LF
	DB "Illegal selection, try again", CR, LF, -1

READKEY_MSG_2 LABEL BYTE

	DB CR, LF, LF, -1


page

;******************Please take note of this **************************
;
;	Size of this module must be 03A0h
;

empty_space	db	114 dup (0)

	if  ($ - start)
;	error
	endif

eproma	ends
	end


