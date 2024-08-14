include	title.inc
subttl	INT 14H  (Serial_IO)

.xlist
include external.inc
include equates.inc
public	serial_io
extrn	serial_base:near
extrn	serial_time:near
extrn	break_key:byte
.list

zero	segment	at 0h
zero	ends


dataa	segment	at 40h
dataa	ends

eproma	segment	byte public

	assume cs:eproma,ds:dataa,es:dataa

public aa_int14
aa_int14 label byte




;******************************************************************************
;     THIS ROUTINE PROVIDES THE INTERFACE TO THE SERIAL PORTS.
;               
;     INPUT TO ROUTINE :
;      AH = COMMAND TO PERFORM
;
;       0 - INITIALIZE THE SERIAL PORT WITH PARAMETERS CONTAINED
;           IN REGISTER AL
;       1 - TRANSMIT THE CHARACTER IN REGISTER AL OVER THE SERIAL LINE
;       2 - RECEIVE THE CHARACTER IN REGISTER AL FROM THE SERIAL LINE
;       3 - RETURN STATUS OF THE SERIAL PORT IN REGISTER AX
;
;      AL = CONTAINS EITHER THE PARAMETER TO INITIALIZE THE SERIAL PORT 
;           OR THE CHRACTER TO TRANSMIT DEPENDING ON COMMAND.
;
;       FORMAT OF PARAMETERS IN AL FOR INITIALIZATION
;                              # OF    
;       BAUD RATE    PARITY    STOPBITS WORD LENGTH
;       7 6 5 ------ 4 3 ------ 2 ----- 1 0 -------------REGISTER AL
;       0 0 0 - 110  0 0 -NONE  0 -1    1 0 -7 BITS
;       0 0 1 - 150  0 1 -ODD   1 -2    1 1 -8 BITS
;       0 1 0 - 300  1 0 -NONE
;       0 1 1 - 600  1 1 -EVEN
;       1 0 0 -1200
;       1 0 1 -2400
;       1 1 0 -4800
;       1 1 1 -9600
;
;      DX - CONTAINS THE SERIAL PORT NUMBER ( 0 OR 1 )
;       
;      DATA VARIABLE SERIAL_BASE CONTAINS THE BASE ADDRESS FOR
;      SERIAL PORT 0 , SERIAL_BASE+1 CONTAINS BASE ADDR FOR
;      SERIAL PORT 1 ETC.
;
;     OUTPUT FROM ROUTINE :
;     DEPENDING ON THE COMMAND AH,AL CONTAIN THE FOLLOWING
;      -COMMAND-    -----------AH------        --------------AL---------     
;      INITIALIZE   LINE CONTROL STATUS        MODEM STATUS  (SEE BELOW)
;      TRANSMIT     (NOTE 1)                   TRANSMITTED CHAR
;      RECEIVE      (NOTE 2)                   RECEIVED CHAR
;      STATUS       LINE CONTROL STATUS        MODEM STATUS  (SEE BELOW)
;
;      NOTE 1 - BIT 7 OF AH IS SET IF A TIMEOUT ERROR OCCURS
;      NOTE 2 - BIT 7 OF AH IS SET IF A TIMEOUT ERROR OCCURS
;               OTHER ERRORS INDICATED IN BITS 1,2,3,4,7 (AS IN STATUS)
;               
;      LINE CONTROL STATUS                  MODEM STATUS
;      7 - TIMEOUT                          7 - RECEIVED LINE SIGNAL DETECT
;      6 - TRANSMITTER SHIFT REG EMPTY      6 - RING INDICATOR
;      5 - TRANSMITTER HOLDING REG EMPTY    5 - DATA SET READY (DSR)
;      4 - BREAK DETECTED                   4 - CLEAR TO SEND (CTS)
;      3 - FRAMING ERROR                    3 - DELTA RECEIVE LINE SIGNL DETECT
;      2 - PARITY ERROR                     2 - TRAILING EDGE RING DETECT
;      1 - OVERRUN ERROR                    1 - DELTA DATA SET READY
;      0 - DATA READY                       0 - DELTA CLEAR TO SEND
;******************************************************************************
;-------------------------------------------------------------------------|
;	THIS IS THE ENTRY POINT FOR THIS MODULE. MAKE SURE YOU DO NOT     |
;	CHANGE THE ORIGIN.............   RLM   03/05/85           	  |
;-------------------------------------------------------------------------|

SERIAL_IO       PROC    NEAR 

        STI                            	;RENABLE INTERRUPTS
	cmp	dx,1			;printer # 0, 1 allowed
	ja	serial_rtn		;return

	push	ax			;
	call	save_all
	push	dx
	pop	si
        MOV     BX,DX                  ;CALCULATE OFFSET TO GET PORT ADDR
        ADD     BX,BX
        MOV     DX,word ptr SERIAL_BASE[BX]     ;GET BASE ADDR OF SERIAL PORT
        AND     DX,DX
        JZ      SERIAL_DONE            ;RETURN IF ADDRESS IS ZERO
        TEST    AH,0FCH                ;CHECK IF ILLEGAL COMMAND
        JNZ     SERIAL_DONE            ;RETURN IF ILLEGAL COMMAND
        XOR     BH,BH                  ;CALCULATE BRANCH TABLE OFFSET
        MOV     BL,AH
        SAL     BX,1
        CALL    WORD PTR CS:[BX+OFFSET S1]  ;CALL APPROPRIATE ROUTINE
SERIAL_DONE:
	mov	[bp+bp_ax],ax		;save ax for return status
	call	restore_all
	pop	ax
serial_rtn:
        IRET



;******************************************************************************
;
;       INITIALIZE SERIAL PORT
;
;******************************************************************************

SERIAL_INIT     PROC    NEAR
        PUSH    AX                     ;SAVE PARAMETERS
        AND     AL,01FH                ;ISOLATE PARITY,STOPBIT,WORD LENGTH
        MOV     BL,AL                  ;SAVE IN BL
        OR      AL,80H                 ;SET DLAB BIT
        ADD     DX,3                   ;POINT TO LINE CTRL REG
        OUT     DX,AL                  ;OUTPUT TO LINE CTRL REG
        POP     AX                     ;RESTORE PARAMETERS
        MOV     CX,3                   ;SETUP ROTATE COUNT
        ROL     AL,CL                  ;ROTATE BAUD RATE INTO BITS 0-2
        AND     AL,07H                 ;ISOLATE BAUD RATE
        JNZ     S5                     ;JUMP IF NOT 110 BAUD
        MOV     AX,1047                ;GET DIVISOR FOR 110 BAUD
        JMP     SHORT S6               ;GO LOAD THE DIVISOR LATCHES
S5:     MOV     CL,AL                  ;USE BAUD RATE FOR SHIFT OPERATION
        MOV     AX,1536                ;SETUP AX WITH 75 BAUD DIVISOR
        SHR     AX,CL                  ;ADJUST DIVISOR BASED ON BAUD RATE
S6:     SUB     DX,3                   ;POINT TO DIVISOR LATCHES
        OUT     DX,AL                  ;OUTPUT TO DIVISOR LATCHES(LSB)
        MOV     AL,AH                  ;GET MSB
        INC     DX                     ;POINT TO DIV LATCHES MSB
	call	delay			;
        OUT     DX,AL                  ;OUTPUT TO DIVISOR LATCHES(MSB)
        ADD     DX,2                   ;POINT TO LINE CTRL REG
        MOV     AL,BL                  ;TURN OFF DLAB
	call	delay
        OUT     DX,AL                  
        SUB     DX,2                   ;POINT TO INT ENABLE REG
        XOR     AL,AL			;
	call	delay			;
        OUT     DX,AL                  ;CLEAR INT ENABLE REG (DISABLE INTRPTS) 
        DEC     DX                     ;POINT TO BASE ADDR
        JMP     SERIAL_STATUS          ;TO STATUS SECTION
SERIAL_INIT     ENDP



;******************************************************************************
;	FUNCTION CODE = 1; SEND CHAR IN AL OVER COMMUNICATION LINE
;       TRANSMIT CHARACTER
;
;******************************************************************************

SERIAL_XMIT     PROC    NEAR
        push	ax			;SAVE XMIT CHAR 
        MOV     AL,3                   ;PREPARE TO SET DTR,RTS
        ADD     DX,4                   ;POINT TO MODEM CTRL REG
        OUT     DX,AL                  ;OUTPUT TO MODEM CTRL REG
        ADD     DX,2                   ;POINT TO MODEM STATUS REG
	mov	bx,2030h		;load mask bits
	call	wait_4_status		;go wait for status from serial port
	jnz	ser_timoutx		;error 
S14:    SUB     DX,5                   ;POINT TO TRANSMIT HOLDING REG
        pop	ax			;GET SAVED CHARACTER
        OUT     DX,AL                  ;OUTPUT CHARACTER
        XOR     AH,AH                  ;STATUS TO ZERO
        RET
SER_TIMOUTX:
        pop	dx			;RESTORE XMIT CHAR
	mov	al,dl			;
        or	AH,80H                 ;SET TIMEOUT BIT
        RET
SERIAL_XMIT     ENDP




;******************************************************************************
;	FUNCTION CODE = 2, RECEIVE CHAR FROM COMUNICATION LINE AND RETURN IT
;			IN AL.
;       RECEIVE CHARACTER
;
;******************************************************************************

SERIAL_RCV      PROC    NEAR
        MOV     AL,1                   ;PREPARE TO SET DSR
        ADD     DX,4                   ;POINT TO MODEM CTRL REG
        OUT     DX,AL                  ;OUTPUT TO MODEM CTRL REG
        ADD     DX,2                   ;POINT TO MODEM STATUS
	mov	bx,0120h		;
	call	wait_4_status		;
	jnz	ser_timoutr		;timeout error
S23:    
        AND     AH,01EH                ;STRIP OFF NON-ERROR BITS
        SUB     DX,5                   ;POINT TO RECEIVE BUFFER
        IN      AL,DX                  ;READ CHARACTER
        RET
SER_TIMOUTR:
        or	AH,80H                 ;SET TIMEOUT BIT
        RET
SERIAL_RCV      ENDP




;******************************************************************************
;
;       GET STATUS
;
;******************************************************************************

SERIAL_STATUS   PROC    NEAR           
        ADD     DX,6                   ;POINT TO MODEM STATUS REG
        IN      AL,DX                  ;READ MODEM STATUS REG
        MOV     AH,AL
        DEC     DX                     ;POINT TO LINE STATUS REG
        IN      AL,DX                  ;READ LINE  STATUS
        XCHG    AL,AH                  ;LINE STATUS IN AH, MODEM STATUS IN AL
        RET
SERIAL_STATUS ENDP


       
S1      LABEL   WORD
        DW      OFFSET SERIAL_INIT     ;TABLE OF OFFSETS TO SERIAL ROUTINES
        DW      OFFSET SERIAL_XMIT
        DW      OFFSET SERIAL_RCV
        DW      OFFSET SERIAL_STATUS



SERIAL_IO ENDP


;***************************************************************************
;		WAIT_4_STATUS	ROUTINE
;	This subroutine waits for coomunication line to be ready before
;	sending or receiving the data.
;	On entry BL contains tha mask bits to be looked for while testing
;	the modem status register and BH contains the mask bits to look for
;	while testing the line status register
;*****************************************************************************

wait_4_status	proc	near
	push	cx
	push	bp			;save bp register
	mov	cl,byte ptr serial_time[si]	;get timeout value
	mov	ch,0			;
	rcl	cx,1			;multiple cx by 4
	rcl	cx,1			;
	push	cx			;
	pop	bp			;put cx value in bp
	push	bp			;save it for later use
	call	wait_loop		;
	cmp	bp,0			;timeout error
	pop	bp
	je	wait_err_return		
wait_loop_4:
	dec	dx			;point to line status register
	mov	bl,bh			;
	call	wait_loop
	cmp	al,bl
	je	wait_status_rtn		;
wait_err_return:
	cmp	bl,0			;set flag

wait_status_rtn:

	pop	bp
	pop	cx
	ret

wait_4_status	endp

wait_loop	proc	near

wait_loop_1:
	xor	cx,cx			;start of loop
wait_loop_2:
	in	al,dx			;get status
	mov	ah,al			;save it to be returned
	and	al,bl			;isolate bits
	cmp	al,bl			;are they equal
	je	wait_rtn		;yes,test next port
	loop	wait_loop_2		;no try again
	dec	bp			;wait more
	jnz	wait_loop_1		;
wait_rtn:
	ret				;
wait_loop	endp

eproma	ends
	end

