$PAGINATE
$TITLE(FARADAY AT 8X42 KEYBOARD CONTROLLER)
$PG PL=58
$PG PW=132
;
;	Keyboard Interface code for Faraday ATease Board........
;	Copyright Faraday Electronics, 743 Pastoria Ave, Sunnyvale Ca.
;					February 11 1985
;	Last update done.................Ver 1.0...............Feb 22 1985
;	Last update done.................Ver 1.1...............mAR 1 1985
;	Last update done.................Var 1.2.......Add PC KBD capability
;						..............3/15/85
;	Last update done.................Ver 1.3...enable Timer interrupts
;						..............4/9/85
;	Last update done.................Ver 1.4...enable A20 line on P2
;						..............5/7/85
;	Last update done.................Ver 1.5??.make this source match
;						   the 1F4B check sum of
;						   part #07016701 7/28/88
;	
;
	DEFSEG	ATCODE, ABSOLUTE
	SEG	ATCODE
$EJECT
	ORG  	0

;		Power on Reset interrupt handler

	jmp	reset


;Input Buffer full Interrupt handler

	ORG  3

	jmp	ibful		;input buffer full interrupt


;Timer Overflow Interrupt handler

	ORG  7

	jmp	timer		;timer overflow interrupt


TITLE:	DB	'COPYRIGHT (c) FARADAY ELECTRONICS'
	DB	'....  2/11/85     RLM'

RESET:
	IN	A,P1			;read input port
	mov	sts,a			;load the status register
	mov	a,#0C3h			;load mask
	OUTL	P2,A			;
	en	flags			;enable master flags

;	set status bits in the status register

	mov	a,#010h
	mov	STS,a
RESET1:
	JNIBF	RESET1			;wait here till we get command from main CPU
	JF1	RESET2			;if command then go check it
	JMP	RESET1			;otherwise wait again


;	Read input buffer and see if we have a `AA` command from CPU

RESET2:
	IN	A,DBB			;read input buffer
	XRL	A,#0AAH			;POD command
	JNZ	RESET1			;no, then wait again


;	Now do an initial Power on Diagnostics on the 8042 RAM

POD:
	MOV	A,#00H
	JZ	POD1			;check Z flag
	JMP	RESET1			;if error then system hangs here

POD1:
	MOV	A,#55H			;load a value in registers
	MOV	R0,A			;to be verified later
	MOV	R1,A			
	MOV	R2,A
	MOV	R3,A
	MOV	R4,A
	MOV	R5,A
	MOV	R6,A
	MOV	R7,A

;	Now check values back

	MOV	A,R0
	XRL	A,#055H			;
	JNZ	PODERR			;error if not equal
	MOV	A,R1
	XRL	A,#055H
	JNZ	PODERR			;error if R1 is not valid
	MOV	A,R2			;
	XRL	A,#055H
	JNZ	PODERR
	MOV	A,R3
	XRL	A,#055H
	JNZ	PODERR
	MOV	A,R4
	XRL	A,#055H
	JNZ	PODERR
	MOV	A,R5
	XRL	A,#055H
	JNZ	PODERR
	MOV	A,R6
	XRL	A,#055H
	JNZ	PODERR
	MOV	A,R7
	XRL	A,#055H
	JZ	PODOK

PODERR:
	JMP	RESET1			;error go hang 8042 and CPU

;	POD ok. Now set up initial values in the RAM and jmp to main loop

PODOK:

	CLR	A			;first zero out RAM
	MOV	R0,#3FH			;
	MOV	R1,#00
RESET4:
	MOV	@R1,A
	INC	R1
	DJNZ	R0,RESET4

	MOV	A,#060H
	MOV	STS,A

	MOV	R1,#020H		;get address of command byte
	MOV	@R1,#010H		;Initialize command byte
	INC	R1
	MOV	@R1,#01H		;LOC 21
	INC	R1		
	MOV	@R1,#06H		;LOC 22
	MOV	R1,#025H
	MOV	@R1,#01H		;LOC 25
	MOV	R1,#027H		;
	MOV	@R1,#0FBH		;LOC 27
	MOV	R1,#028H		;
	MOV	@R1,#0E0H		;LOC 28
	MOV	R1,#029H		;
	MOV	@R1,#06H		;LOC 29
	MOV	R1,#02AH
	MOV	@R1,#010H		;LOC 2A
	MOV	R1,#02BH
	MOV	@R1,#20H		;LOC 2B
	MOV	R1,#02CH		;
	MOV	@R1,#015H		;
	CLR	A

;	Now send 55 to CPU indicating POD ok


	MOV	R6,#00			;indicate no error
	MOV	R7,#055H		;
	CALL	SENDC			;send char in R7 to cpu
;

	MOV	R1,#2DH			;fixed on 3/1/85 ver 1.1
	CLR	A			;so as not to skip 1st char on power-up
	MOV	@R1,A			;
	MOV	R1,#3FH			;initialise diag address
	MOV	@R1,#040H		;To 40... ver 1.2 3/15/85
	MOV 	R0,#40H			;
RESET5:
	MOV 	@R0,#0H			;CLEAR MEMORY
	INC 	R0	
	DJNZ	R1,RESET5		;

	MOV 	A,#0BFH			;enable A20 line on soft reset
	OUTL	P2,A			;  VER 1.4   5/7/85


	EN  	TCNTI			;enable timer interrupts  4/09/85
	JMP 	MAINLP			;jump to main wait loop


;	Timer Overflow interrupt service routines.
;	This routine sets error bit 6 in AC and return to one level
;	high

TIMER:
	STOP	TCNT			;stop timer/counter
	CALL	DLYKL			;drop clock and delay

	MOV 	R7,A			;save Ac     ver 1.2  3/15/85

	MOV 	A,PSW			;Get current PSW
	MOV 	R1,A			;
	ANL 	A,#07			;Get stack pointer
	DEC 	A			;
	MOV 	R2,A			;
	MOV 	A,R1			;Get original PSW
	ANL 	A,#0F8H			;
	ORL 	A,R2			;
	MOV 	PSW,A			;Restore PSW

;	check for PC keyboard		3/15/85

	IN  	A,P1			;get input port
	JB3 	TIMER3			;AT keyboard, skip
	DEC 	R6				;true timeout
	JNZ 	TIMER3			;yes
	MOV 	A,#01H			;set flag
	RETR				;no error return

TIMER3:
	MOV 	A,#040H
	RETR				;Set error code in AC and return


;	Input Buffer full interrupt service routines

IBFUL:
	STOP	TCNT
	DIS 	I
	JMP 	MAINLP			;Go to main loop

$EJECT
	ORG	100h


;	This is the main wait loop where the processor 8042 waits for command
;	from the main CPU 80286
;


MAINLP:
	JNIBF	MAIN00			;jump if input buffer is empty
	JF1  	MAIN01			;input buffer full,jump if it is a comand
	JMP  	CKDATA			;data byte from CPU, go get it

MAIN00:
	JOBF	MAINLP			;jump if output buffer is full
	JNIBF	MAIN02			;jump if input buffer is empty
	JMP  	MAINLP			;

MAIN01:
	JMP  	CKCMD				;Command received from CPU, go analyse it

;
;	Come here when input and output buffers are empty, Now try to read from
;	the keyboard.
;

MAIN02:
	MOV  	R1,#020H		;look for keyboard disable bit
	MOV  	A,@R1			;
	JB4  	MAINLP			;do not read keyboard if disabled

	IN   	A,P1			;check for PC keyboard  ver 1.2  3/15/85
	CPL  	A			;
	JB3  	PCMODE			;PC keyboard if bit 3 = 0

	MOV  	R1,#02EH		;
	MOV  	A,@R1			;
	JZ   	MAIN03			;
	JNT1	MAINLP		 	;jump to main loop if KBD data is low
	JNT1	MAINLP			;
	MOV  	@R1,#00			;

MAIN03:
	ANL  	P2,#0B3H		;Raise KBD data and clk to indicate ready to
	NOP						;receive data from keyboard
	NOP
MAIN05:
	JNT0	MAIN04			;jump if keyboard responded by dropping clk
	JNIBF	MAIN05			;No command from CPU either, wait in a loop
	CALL	DLYKL			;CMD/DATA from CPU, tell KBD to hold off
	JMP  	MAINLP			;Go see what we got

MAIN04:
	JNT0	MAIN06			;jump if clk is still low
	CALL	DLYKL			;clk went high, must be transients, drop clk
	JMP  	MAIN03			;and wait for keyboard again

;	Keyboard ready to Xmit

MAIN06:
	CALL	KBDRY			;Go read from keyboard
	JB7  	MAIN07			;jump if Xmit error
	JB6  	MAIN08			;jump if timeout error

;	Char just read from keyboard is in R7.  Look at the command byte to
;	see if this scan code needs to be translated or not
;

MAIN20:
	call	STCMD			;
	MOV  	R1,#020H		;	
	MOV  	A,@R1
	JB6  	MAIN09			;PC compatibility mode, go Xlat scan code

MAIN10:
	MOV  	R6,#00			;indicate no error
	CALL	SENDC			;GO send char to CPU
	JMP  	MAINLP			;and wait in the main loop

;	Error while receiving data from keyboard.  Must be parity or xmit
;	error

MAIN07:
	MOV  	R6,#080H		;Set error code
	MOV  	R7,#00			;and send a null char
	JMP  	MAIN20			;

;	Error in timeout. Data not received from keyboard within specified
;	time limit.

MAIN08:
	MOV  	R6,#040H		;set error code
	MOV  	R7,#00
	JMP  	MAIN20			;
;
;	Come here to translate scan code.  PC compatibility mode (bit 6) is
;	set in the command byte. Char just received from keyboard is in R7
;

MAIN09:
	MOV  	A,R7			;Get char
	XRL  	A,#0F0H			;Is it a Break char
	JNZ  	MAIN15			;NO, JUMP
	MOV  	R1,#02DH		;Set flag
	MOV  	@R1,#080H		;
	JMP  	MAINLP			;and return to main loop

MAIN15:
	MOV  	A,R7			;
	XRL  	A,#084H			;Convert scan code 84
	JNZ  	MAIN11			;Not 84
	MOV  	R7,#07FH		;Yes, then convert it to 7f
	JMP  	MAIN12

MAIN11:
	MOV  	A,R7			;
	XRL  	A,#083H			;
	JNZ  	MAIN12
	MOV  	R7,#02H			;Convert 83 scan code to 2

MAIN12:
	MOV  	A,R7			;
	JB7  	MAIN10			;Jump if high bit is set

;	check for keyboard disable bit in the command byte and also check
;	for key lock bit
;

MAIN14:
	MOV  	R1,#020H		;
	IN   	A,P1			;read input port
	JB7  	MAIN16			;jump if no key lock
	MOV  	A,@R1			;Get command byte
	JB3  	MAIN16			;key inhibit over ride
;	Keyboard inhibited; do not send char to CPU

	MOV	R1,#02DH		;
	MOV	@R1,#00			;initialise flag
	JMP	MAINLP			;Go to main wait loop

MAIN16:
	MOV	A,R7			;
	MOVP3	A,@A			;read translated scan code from page 3 table
	MOV	R7,A			;
	MOV	R1,#02DH		;
	MOV	A,@R1			;combine two scan codes
	ORL	A,R7			;
	MOV	R7,A			;
	MOV	@R1,#00		
	CLR	A
	JMP	MAIN10			;


;	Come here if it is a PC key board	ver 1.2  3/15/85

PCMODE:
	ANL	P2,#0BFH		;make sure data line is high
	ORL	P2,#080H		;
	JT0	PCMOD1			;wait for clk to go low
	JT0	PCMODE
	JMP	PCKBD			;start of transmission

PCMOD1:
	JNIBF	PCMODE			;jump if input buffer is empty
	CALL	DLYKL			;go drop data line, disable keyboard
	JMP	MAINLP			;and jump to main loop

;
;	Come here to read PC keyboard. Please note that some keyboards
;	send out 9 data bits and some 10 data bits. We will ignore the
;	first start bit and look for 9 additional data bits. If we get
;	only 8 bits, then the CPU will timeout for the 9th bit and this
;	will be corrected in the timeout routine.
;			ver 1.2  3/15/85


PCKBD:
	CALL	PCREAD			;go read data
	JB7	PCERR1			;transmission error
	JB6	PCERR2			;timeout error

	IN 	A,P1			;look for keyboard disabled
	JB7	PCKBD1			;O.K 
	MOV	R1,#20H			;look at the command byte
	MOV	A,@R1			;
	JB3	PCKBD1			;O.K
	ANL	P2,#07FH		;Drop data line
	JMP	MAINLP


PCKBD1:
	MOV	R6,#00H			;NO ERROR
	CALL	SENDC			;send it to CPU
	CALL	DLYRTN			;

PCERR1:
	ORL	P2,#080H		;
	JMP	MAINLP

PCERR2:
	MOV	R6,#040H		;error code
	MOV	R7,#0FFH		;send a null char
	CALL	SENDC			;
	JMP	PCERR1			;

;
;	subroutine to read data from PC keyboard
;

PCREAD:
	MOV	R6,#09H			;9 data bits to read
	CLR	A
	MOV	T,A			;set timer/counter

READ5:
	JNT0	READ5			;ignore start bit
	STRT	T		
	CLR	C

READ1:
	JT0	READ1			;
	JNT1	READ2			;sample data bit
	CPL	C
READ2:
	RRC	A			;shift bit just read
	CLR	C			;

READ3:
	JNT0	READ3			;wait for clock to go high
	DJNZ	R6,READ1		;read next bit
	ANL	P2,#073H		;drop data line

READ4:
	STOP	TCNT			;STOP CLOCK
	JNT0	READ4			;wait for clock to go high
	MOV	R7,A			;put char just read in R7
	CLR	A			;no error
	RET				;	


$EJECT
	ORG	200h

;
;	Come here to get data from the CPU and send it to the keyboard
;

CKDATA:
	IN	A,DBB			;read data
	MOV	R7,A			;
	CALL	STCMD			;
	IN	A,P1			;look for PC keyboard  ver 1.2  3/15/85
	CPL	A			;
	JB3	DATA10			;bit 3 = 0 means PC keyboard

	MOV	R1,#020H		;
	MOV	A,@R1
	ANL	A,#0EFH
	MOV	@R1,A			;

	CALL	KBDTX			;send it to keyboard
	ORL	P2,#080H		;Raise keyboard data line high
	JB7	DATA00			;Jump if xmit eror
	JB6	DATA00			;jump if timeout error
	MOV	R6,#06			;Load timing constant
	CALL	KBDRD			;read from keyboard ack
	JB7	DATA01			;error in receive
	JB6	DATA02			;timeout error
	MOV	R6,#00			;no error status

DATA04:	
	CALL 	SENDC			;send char in R7 to CPU and set status in R6
	JMP	MAINLP			;go to main wait loop

DATA00:
	MOV	R6,#020H		;set error code
	MOV	R7,#0FEH		;send `RESEND` code to CPU
	JMP	DATA04

DATA01:
	MOV	R6,#0A0H
	MOV	R7,#0FEH		;set error code and RESEND code
	JMP	DATA04

DATA02:
	MOV	R6,#060H
	MOV	R7,#0FEH
	JMP	DATA04


;
;	Routine to send data to the keyboard.  In all total 11 bits are send as
;	follows: 1 start bit, 8 data bits, 1 parity bit and 1 stop bit. The
;	parity is set to odd parity.
;

KBDTX:
	MOV	R3,#02			;Outer loop count
	MOV	R4,#08			;# of data bits to send
	MOV	R5,#0			;timer constant
	MOV	R6,#0			;# of is data bits counter

	ANL	P2,#073H		;lower keyboard data line
	CALL	DLYKL			;lower clock line, and delay

	MOV	A,R5			;set timer/counter
	MOV	T,A			;
	STRT	T			;start T
	ANL	P2,#0B3H		;raise clock

KBD00:

	MOV	A,R7			;load char to be transmitted
	RRC	A			;rotate first bit in carry
	MOV	R7,A			;
KBD01:
	JT0	KBD01			;wait for clk to drop
	JNC	KBD02			;jump if data bit is 0
	ORL	P2,#080H		;raise KBD data line when data bit = 1
	INC	R6			;keep count of # of 1s
	JMP	KBD03			;

KBD02:
	ANL	P2,#073H		;drop KBD data line when data bit = 0

KBD03:
	JNT0	KBD03			;wait here for clk line to go high

	DJNZ	R4,KBD00		;send next data bit
	DJNZ	R3,KBD10A		;end of transmission
	JMP	KBD10

KBD10A:
	MOV	A,#01			;generate parity bit
	XRL	A,R6			;set bit 0 as parity bit
	ORL	A,#02H			;set stop bit
	MOV	R4,#2			;send only two bits
	MOV	R7,A			;
	JMP	KBD00			;

;	prepare keyboard to send an ack

KBD10:
	STOP	TCNT			;stop timer/counter
	MOV	A,#080H			;load timer constant
	MOV	T,A			
	STRT	T

KBD11:
	JT1	KBD11			;wait here till KBD data goes low
	NOP
	NOP
KBD12:
	JNT1	KBD12			;wait here till KBD data goes high
	STOP	TCNT			;
	CALL	DLYKL			;
	ANL	A,#00			;set error code
	RET

;
;	This routine drops KBD clock and does a delay
;

DLYKL:

	IN	A,P1			;check for PC keyboard  VER 1.3  3/15/85
	JB3	DLYKL2			;AC keyboard
	ANL	P2,#7FH			;drop data line for PC keyboard
	JMP	DLYKL3			;

DLYKL2:
	ORL	P2,#040H		;drop KBD clock line
DLYKL3:
	SEL	RB1			;
	MOV	R2,#0EH				
DLYKL1:
	DJNZ	R2,DLYKL1		;wait here till R2 goes 0
	SEL	RB0			;
	RET

;	Delay routines

DLYRTN:
	SEL	RB1			;select register bank 0
	MOV	R2,#060H		;get time constant
DLYRT1:	DJNZ	R2,DLYRT1
	SEL	RB0
	RET


;
;	Come here if you have PC keyboard. Read data from CPU and send
;	a dummy ACK.          ver 1.3   3/15/85
;

DATA10:
	CALL	DLYRTN			;delay
	MOV	A,R7			;
	MOV	R3,A			;save data just read
	XRL	A,#0EEH			;is it echo command to keyboard
	JZ	PCSND2			;yes
	MOV	R7,#0FAH		;no then send a dummy ACK
PCSND2:					
	MOV	R6,#00H			;no error
	CALL	SENDC			;
	MOV	A,R3			;get char back
	XRL	A,#0FFH			;is it reset code
	JZ	PCSND3			;yes
	CLR	A
	JMP	MAINLP			;otherwise jump to main loop
PCSND3:
	MOV	R1,#040H		;do a small delay

PCSND4:
	CALL	DLYRTN			;
	DJNZ	R1,PCSND4		;
	MOV	R7,#0AAH		;send ok to CPU
	MOV	R6,#00H
	CALL	SENDC
	ANL	P2,#0B3H		;raise clk line
	ANL	P2,#073H		;drop data line
	CLR	A			
	JMP	MAINLP			;go to main lp

$EJECT
	ORG		300H

;
;	Define scan code convertion table
;

SCNTBL:

	DB	0FFH,43H,41H,3FH,3DH,3BH,3CH,58H
	DB	64H,44H,42H,40H,3EH,0FH,29H,59H
	DB	65H,38H,2AH,70H,1DH,10H,02H,5AH
	DB	66H,71H,2CH,1FH,1EH,11H,03H,5BH
	DB	67H,2EH,2DH,20H,12H,05H,04H,5CH
	DB	68H,39H,2FH,21H,14H,13H,06H,5DH
	DB	69H,31H,30H,23H,22H,15H,07H,5EH
	DB	6AH,72H,32H,24H,16H,08H,09H,5FH
	DB	6BH,33H,25H,17H,18H,0BH,0AH,60H
	DB	6CH,34H,35H,26H,27H,19H,0CH,61H
	DB	6DH,73H,28H,74H,1AH,0DH,62H,6EH
	DB	3AH,36H,1CH,1BH,75H,2BH,63H,76H
	DB	55H,56H,77H,78H,79H,7AH,0EH,7BH
	DB	7CH,4FH,7DH,4BH,47H,7EH,7FH,6FH
	DB	52H,53H,50H,4CH,4DH,48H,01H,45H
	DB	57H,4EH,51H,4AH,37H,49H,46H,54H
;
;Routine KBDRD: Wait for keyboard to accept data
;		Input R6 = 6 = Time constant
;		Ouput R7 = data just read

KBDRD:
	CLR	A
	ANL	P2,#0B3H		;set KBD clk high

KBRD00:
	JNT0	KBRD05			;jump if keyboard clk is low
	INC	A			;
	JNZ	KBRD00			;wait in a loop
	JNT0	KBRD05			;did the clk go low yet
	DJNZ	R6,KBRD00		;wait till outer loop count is done
	CALL	DLYKL			;
	MOV	A,#080H			;set time out error
	RET				;

KBRD05:
	JT0	KBRD06			;jump if clk did not stay low long enough
	CALL	KBDRY			;clk is low, go read keyboard
	RET				;process error code in higher routines

KBRD06:
	CALL	DLYKL			;drop clk and delay
	JMP	KBDRD			;try again


;
;	Routine to read data from keyboard
;	On exit R7 = keyboard data just read
;		R6 = error code or 0

KBDRY:
	MOV	R4,#08H			;# of bits to read
	MOV	R6,#0			;init counter for # of 1s
	MOV	A,#0E0H			;init timer constant
	MOV	T,A			;set up timer/counter
	STRT	T			;and start it			
	CLR	A

KBRD10:
	JNT0	KBRD10			;wait till clk goes high
	NOP				;

KBRD11:
	JT0	KBRD11			;wait for clk to go low
	JT1	KBRD14			;jump if data bit is a 1
	ANL	A,#0FEH			;make low bit 0
	JMP	KBRD15			;read next data bit

KBRD14:
	ORL	A,#01			;make low bit 1
	INC	R6			;keep count of # of 1s for parity
KBRD15:
	RR	A			;
	DJNZ	R4,KBRD10		;GO read more data
	MOV	R7,A			;store data in R7
;
;	Now check the parity of byte just received
;
KBRD16:
	JNT0	KBRD16			;wait till clk goes high
	NOP

KBRD17:
	JT0	KBRD17			;wait for clk to go low
	JT1	KBRD18			;jump if the data bit is 1
	CLR	A
	JMP	KBRD19			;
KBRD18:
	MOV	A,#01H			;set AC to 1

KBRD19:
	JNT0	KBRD19			;wait for stop bit
	NOP

KBRD20:
	JT0	KBRD20
	NOP

;	Now check parity..must be odd
	
	STOP	TCNT			;
	NOP				;
	XRL	A,R6
	JB0	KBRD21			;if bit set means parity ok

;	Parity error
	
	CALL 	DLYKL
	MOV	A,#080H			;set error code
	RET				;

KBRD21:
	CALL	DLYKL
	MOV	A,#00			;no error
	RET


;
;	Subroutine SENDC:	Send a char in R7 and status in R6 to the CPU
;

SENDC:
	CALL	STCMD
	IN	A,P1			;get inhibit switch from input port P1
	SWAP	A
	RL	A			;align bit 4
	ANL	A,#010H			;
	ORL	A,R6			;merge status bits
	MOV	STS,A			;set status register
	MOV	A,R7			;get char
	OUT	DBB,A			;send it to CPU
	RET				;

$EJECT
	ORG 	0400H

;
;	Come here to handle command from the CPU to 8042
;


CKCMD:
	IN  	A,DBB			;read input buffer
	MOV 	R7,A			;
	CALL	STCMD			;
	MOV	A,R7			;restore command
	JB6	CMD20			;
	JB7	CMD30			;

;	Come here if the command code is 00 - 3F

	JB5	CMD10
	MOV	R0,#02BH
	ADD	A,@R0

CMD10:
	MOV	R1,A			;
	MOV	A,@R1			;get contents of RAM location

CMD10A:
	MOV	R7,A
CMD11:
	MOV	R6,#00
	CALL	SENDC			;send R6 and R7 to CPU
	CLR	A
	JMP	MAINLP			;go wait in main loop

;
;	Handling of commands 40 - 7F (except 60)
;

CMD27:
	MOV	A,R1			;restore original command
	ANL	A,#03FH			;zero out bit 6
	JB5	CMD27A			;
	MOV	R0,#02BH
	ADD	A,@R0

CMD27A:
	MOV	R1,A			;
	IN	A,DBB			;get input from CPU
	MOV	@R1,A			;store it in that location
	JMP	MAINLP			;go wait in main loop

;
;	Handling of commands 80 - BF. Valid commands are AA - AE
;

CMD30:
	JB5	CMD30B

CMD30A:
	JMP	MAINLP			;invalid command, jump to main loop

CMD30B:
	JB4	CMD30A			;

;	valid command byte

	MOV	R1,#020H		;get command byte
	MOV	R0,A			;save command
	XRL	A,#0AEH			;is it AE
	JNZ	CMD31			;no.skip

;	Command is AE : clear keyboard disable (bit 4) in the command byte

	MOV	A,@R1			;get current command byte	
	ANL	A,#0EFH			;mask off bit 4
CMD30C:		
	MOV	@R1,A			;save byte back in RAM
	JMP	MAINLP			;

CMD31:
	MOV	A,R0			;restore command
	XRL	A,#0ADH			;is it AD command
	JNZ	CMD32			;no skip


;
;	Command is AD: set keyboard disable bit (bit 4) of command byte
;
	MOV	A,@R1			;get current value
	ORL	A,#010H			;set bit
	JMP	CMD30C			;

CMD32:
	MOV	A,R0 			;		
	XRL	A,#0ACH			;is it AC
	JNZ	CMD33			;no skip

;	Command is AC: Do a diagnostic dump to CPU

	CALL	DIAGDP
	JMP	MAINLP			;

CMD33:
	MOV	A,R0
	XRL	A,#0ABH			;is it AB command
	JNZ	CMD34			;no skip

;	Command is AB: Do the interface test
	CALL	IFTEST
	JMP	CMD11


CMD34:
	MOV	A,R0
	XRL	A,#0AAH			;is it AA command
	JNZ	CMD35			;no skip

;	Command is AA: Do internal diagnostics
	
	JMP	POD
CMD35:
	JMP	MAINLP


;
;	Handling of commands C0 - FF. Valid commands are C0,C2,C3,D0,D1	
;	E0 & F0 - FF
;
CMD40:
	JB5	CMD60			;jump if EX or EX command
	JB4	CMD50			;jump if DX command

;	Command is C0 - CF

	JB3	CMD35			;jump if invalid command
	JB2	CMD35			;jump if invalid command
	JB1	CMD41			;jump if command is C2
	JB0	CMD42			;jump if command is C1

;	command is C0

	IN	A,P1			;read input buffer
	JMP	CMD10A			;go send it to CPU

;	command is C2

CMD41:
	IN	A,P1			;execute C2 command
	MOV	STS,A			;read P1 and store in status reg.
	JNIBF	CMD41			;wait until input buffer is full
	JMP	MAINLP			;

;	Command is C1

CMD42:					
	IN	A,P1			;execute command C3
	SWAP	A
	MOV	STS,A			;
	JNIBF	CMD42
	JMP	MAINLP
;	Command is D0 - DF

CMD50:
	ANL	A,#0FH			;isolate bits 0-3
	JNZ	CMD51			;

;	D0 Command

	IN	A,P2			;read output port
	JMP	CMD10A			;send it to CPU

CMD51:
	DEC	A
	JNZ	CMD35

;	Command is D1

CMD52:
	JNIBF	CMD52			;wait here for CPU to send a byte
	JF1	CKCMD			;if command go execute it
	IN	A,DBB			;read data byte
	OUTL	P2,A			;write to output port
	JMP	CMD35

;	Execute commands EX & FX
CMD60:
	JB4	CMD70			;jump if command is FX

;	Command is E0 - EF

	ANL	A,#0FH			;isolate bits 0-3
	JNZ	CMD35			;jump if not E0

;	Command is E0: read test inputs and send to CPU
	
	JNT0	CMD61
	XRL	A,#01H
CMD61:
	JNT1	CMD10A
	XRL	A,#02H			;set bit 1 if KBD data is high
	JMP	CMD10A

;
;	Command is F0 - FF: Pulse output port P2
;

CMD70:
	MOV	R0,A			;save original command
	IN	A,P2			;read output port and save it
	MOV	R2,A
	MOV	R1,#020H		;
	MOV	A,@R1			;read current value of command byte
	ANL	A,#03			;isolate bits 0,1
	SWAP	A				;
	ORL	A,R2			;merge bits with output port P2 value
	ANL	A,R0			;set selected bits
	OUTL	P2,A			;output it to P2
	ORL	A,#0FH			;
	OUTL	P2,A	
	JMP	CMD35


CMD20:
	JB7	CMD40

;	Execute commands 40 - 7F

CMD21:
	JNIBF	CMD21			;wait for CPU to send a byte
	JF1	CKCMD			;jump if it is a command
	MOV	R1,A			;save original command
	XRL	A,#060H			;is it command 60
	JNZ	CMD27			;NO

;	Execute command 60
	MOV	R1,#020H		;
	MOV	A,@R1			;get current value of command byte
	MOV	R2,A			;save it
	IN	A,DBB			;read input byte from CPU
	MOV	@R1,A			;write new value in the command byte
	JB4	CMD22B			;jump if bit 4 set
	MOV	A,R2			;
	ANL	A,#010H			;set bit if not previously set
	JZ	CMD22A			;
	MOV	R0,#02EH		;save previous value
	MOV	@R0,#01H		;

CMD22A:
	MOV	A,@R1
	JMP	CMD22C

CMD22B:
	ORL	P2,#040H		;keyboard disable, set clk low

CMD22C:
	JB2	CMD23A			;is sys flag on?
	CLR	F0    			;clear it
	JMP	CMD23B

CMD23A:
	JF0	CMD23B
	CPL	F0

CMD23B:
	JB1	CMD24A			;is bit on
	ANL	P2,#0DFH		;
	JMP	CMD24B			;
		
CMD24A:
	ORL	P2,#020H		;set bit 5 of P2

CMD24B:
	JB0	CMD25A
	ANL	P2,#0EFH		;reset bit 4 (OBF) of P2
	JMP	CMD25B

CMD25A:
	ORL	P2,#010H		;set bit 4 (OBF) of P2

CMD25B:	
	JB5	CMD26A
	ORL	P2,#040H		;set KBD clk low
	ORL	P2,#080H		;set KBD clk high
	JMP	MAINLP

CMD26A:
	ANL	P2,#07FH		;set KBD clk high
	ANL	P2,#0B4H		;set KBD data low
	JMP	MAINLP


$EJECT
	ORG	500H

	DB	0BH,02H,03H,04H,05H,06H,07H,08H
	DB	09H,0AH,1EH,30H,2EH,20H,12H,21H
;
;	Execute diagnostic dump of 8042 RAM to CPU. In all send 20 bytes
;

DIAGDP:
	MOV	A,#010H			;
	MOV	STS,A			;
	MOV	R1,#030H		;Read P1,P2T0T1 & PSW
	CALL	DIAG6
	MOV	R1,#20H			;start of dump address
	CLR	A
	MOV	R6,#014H		;counter for 20 bytes

DIAG1:
	JNIBF	DIAG2			;jump if input buffer is empty
DIAGRT:
	RET				;input buffer full, abort operation
DIAG2:
	JOBF	DIAG1			;jump if output buffer is full

;	input buffer & output buffer are empty

	CALL	DELAY			;
	MOV	A,@R1			;get byte to send
	CALL	SEND			;
	JB0	DIAGRT			;abort operation if error
	INC	R1				;next byte
	DJNZ	R6,DIAG1		;
	RET

;
;	Routine to read P1, P2 T0T1 & PSW and send it to CPU
;

DIAG6:
	IN	A,P1			;get input port
	MOV	@R1,A			;save it at LOC 30h
	INC	R1
	IN	A,P2			;get output port
	MOV	@R1,A			;save it at LOC 31
	INC	R1
	CLR	A
	JNT0	DIAG4			;jump if clk is low
	MOV	A,#01H			;clk high set bit
DIAG4:
	JNT1	DIAG5			;jumpif KBD data is low
	ADD	A,#02H			;data line high, set bit
DIAG5:
	MOV	@R1,A			;save state of T0 & T1
	INC	R1
	MOV	A,PSW			;send status
	MOV	@R1,A
	RET				;

;
;	Routine SEND: send a byte to CPU in system scan code format.
;

SEND:
	MOV	R2,A			;save char
	SWAP	A
	ANL	A,#0FH			;send high nibble first
	MOVP	A,@A			;translate it
	OUT	DBB,A			;send it to CPU
	CALL	DELAY			;

SEND1:
	JNIBF	SEND2			;jump if input buffer is empty
	JMP	SENDRT			;input buffer full, abort operation
SEND2:
	JOBF	SEND1			;wait for CPU to pick up last byte

	MOV	A,R2			;get char back
	ANL	A,#0FH			;
	MOVP	A,@A			;translate it
	OUT	DBB,A			;send it to CPU
	CALL	DELAY

SEND3:
	JNIBF	SEND4			;jump if iput buffer is empty
	JMP	SENDRT			;error if input buffer is full
SEND4:
	JOBF	SEND3			;wait for CPU to pick up last byte

	MOV	A,#39H			;
	OUT	DBB,A			;send a delimiter
	CALL	DELAY			;
	CLR	A
	RET				;

SENDRT:
	MOV	A,#01H			;set error code
	RET				;

DELAY:
	MOV	R0,#00
	MOV	R4,#040H
DELAY1:
	DJNZ	R0,DELAY1		;inner wait loop
	DJNZ	R4,DELAY1
	RET


;
;	Interface test for 8042
;

IFTEST:
	ANL	P2,#0B3H		;raise clock
	ORL	P2,#80H			;raise data
	CLR	A

TEST2:
	JT0	TEST1			;jump if clock is high
	DEC	A
	JNZ	TEST2			;wait in a loop for a while
	MOV	R7,#01H			;error, clock not high
	RET				;send error code to CPU

TEST1:
	ORL	P2,#040H		;drop clock
	NOP
	NOP
	JNT0	TEST3			;make sure clock went lot
	JNT0	TEST3			;check 1 more time
	ANL	P2,#0B3H		;
	MOV	R7,#02			;set error code
	RET

TEST3:
	CLR	A
TEST31:
	JT1	TEST4			;jump if data line is high
	DEC	A
	JNZ	TEST31      		;try again     
	MOV	R7,#03			;set error code	
	RET

TEST4:
	ANL	P2,#073H		;drop data line
	NOP
	NOP
	JNT1	TEST5			;jump if data line dropped
	JNT1	TEST5			;
	ORL	P2,#080H		;
	MOV	R7,#04			;send error code to CPU
	RET				;

TEST5:
	ORL	P2,#080H		             
	MOV	R7,#00			;no error
	RET
$EJECT
	ORG	600H

;	Come here to store last 40 commands given to 8042.  Buffer starts
;	from 40hex to 7F.  Location 3F contains the current pointer
;	                  corrected   5/7/85  ver 1.4

STCMD:
	MOV	A,#03FH
	MOV	R0,A			;
	MOV	A,@R0			;get current pointer
	XRL	A,#07FH			;end of buffer
	JNZ	STCMD2			;no
	MOV	A,#40H			;
	MOV	@R0,A			;yes, reset to start of buffer
STCMD2:
	MOV	A,@R0			;
STCMD1:
	MOV	R1,A			;
	MOV	A,R7			;get current command
	MOV	@R1,A			;store it
	MOV	A,@R0			;
	INC	A			;update pointer
	MOV	@R0,A			;
	RET



	END

