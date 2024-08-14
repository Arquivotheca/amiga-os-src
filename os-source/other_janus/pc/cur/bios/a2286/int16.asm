include	title.inc
subttl	INT 16h  (Keyboard Module)

;bug fix history:
;03/04/87	1. alt+ctrl+'Q','A' or 'Z' do not return code
;03/05/87	2. ctrl+shift+PrtSc do not return code (7200) 
;		3. alt + 2 + 5 + 5 will not return code 00FF
;07/24/87	1. break key did not set flag

.xlist
include external.inc
include equates.inc

extrn	out_buffer:word
extrn	in_buffer:word
extrn	key_buffer_start:word
extrn	key_buffer_end:word
extrn	reset_key:word

extrn	FLAG:byte
extrn	FLAG_1:byte
extrn	FLAG_2:byte
extrn	FLAG_3:byte

extrn	alt_buffer:byte
extrn	video_mode:byte
extrn	video_mode_reg:byte
extrn	break_key:byte
extrn	key_buffer:byte
extrn	key_table_1:byte
extrn	key_table_2:byte
extrn	key_table_2_1:byte
extrn	key_table_3:byte
extrn	key_table_4:byte
extrn	key_table_5:byte
extrn	key_table_6:byte
extrn	key_table_7:byte
extrn	rsetup:near

public	key
public	keyboard

extrn	wait_upi:near
extrn	beep:near
extrn	test_1:near
.list


dataa	segment	at 40h
dataa	ends

zero	segment	at 0h
zero	ends


;SYMBOL DEFITION

ENABLE_CMD	EQU	0AEH		;ENABLE KEYBOARD TO KEYBOARD
DISABLE_CMD	EQU	0ADH		;DISABLE KEYBOARD TO KEYBOARD

RESEND_CMD	EQU	0FEH		;RESEND COMMAND FROM KEYBOARD
ACK_CMD		EQU	0FAH		;ACK COMMAND FROM KEYBOARD
OVER_RUN_CODE	EQU	0FFH		;OVERRUN CHAR FROM KEYBOARD

;KEY SCAN CODE DEFINITION

R_SHIFT_KEY	EQU	54		;RIGHT SHIFT KEY
L_SHIFT_KEY	EQU	42		;LEFT SHIFT KEY

SCROLL_LOCK_KEY	EQU	70		;SCROLL LOCK KEY
NUM_LOCK_KEY	EQU	69		;NUM LOCK KEY
CAPS_LOCK_KEY	EQU	58		;CAPS LOCK KEY
INS_KEY		EQU	82		;INS KEY

CTRL_KEY	EQU	29		;CTRL KEY
ALT_KEY		EQU	56		;ALT KEY
SYS_REQ_KEY	EQU	84		;SYS REQ KEY

;FLAG

R_SHIFT		EQU	1		;RIGHT SHIFT KEY DEPRESSED/RELEASED
L_SHIFT		EQU	2		;LEFT SHIFT KEY DEPRESSED/RELEASED
CTRL_SHIFT	EQU	4		;CTRL SHIFT KEY DEPRESSED/RELEASED
ALT_SHIFT	EQU	8		;ALT SHIFT KEY DEPRESSED/RELEASED

SCROLL_LOCK_ON	EQU	10H		;SCROLL LOCK ON
NUM_LOCK_ON	EQU	20H		;NUM LOCK ON
CAPS_LOCK_ON	EQU	40H		;CAPS LOCK ON
INS_ON		EQU	80H		;INS ON

;FLAG_1

L_CTRL		EQU	1		;LEFT CTRL KEY DEPRESSED/RELEASED
L_ALT		EQU	2		;LEFT ALT KEY DEPRESSED/RELEASED
SYS_REQ		EQU	4		;SYS REQ DEPRESSED/RELEASED
HOLD_ON		EQU	8		;HOLD ON
SCROLL_LOCK	EQU	10H		;SCROLL LOCK KEY DEPRESSED/RELEASED
NUM_LOCK	EQU	20H		;NUM LOCK KEY DEPRESSED/RELEASED
CAPS_LOCK	EQU	40H		;CAPS LOCK DEPRESSED/RELEASED
INS_SHIFT	EQU	80H		;INS KEY DEPRESSED/RELEASED

;FLAG_2

SCROLL_LOCK_LED	EQU	1		;SCROLL LOCK LED ON/OFF
NUM_LOCK_LED	EQU	2		;NUM LOCK LED ON/OFF
CAPS_LOCK_LED	EQU	4		;CAPS LOCK LED ON/OFF
ACK_RECEIVED	EQU	10H		;ACK CMD FROM KEYBOARD RECEIVED
RESEND_RECEIVED	EQU	20H		;RESEND CMD RECEIVED FROM KEYBOARD
LED_UIP		EQU	40H		;LED UPDATE IN PROGRESS
ERROR_KB	EQU	80H		;KEYBOARD ERROR

;FLAG 3

E1_RECEIVED	EQU	1		;E1 + NN
E0_RECEIVED	EQU	2		;E0 + NN
R_CTRL		EQU	4		;RIGHT CTRL KEY DEPRESSED/RELEASED
R_ALT		EQU	8		;RIGHT ALT KEY DEPRESSED/RELEASED
KEY_101		EQU	10H		;101/102 KEYBOARD
S_NUM_LOCK	EQU	20H
ID_2ND		EQU	40H		;SECOND ID EXPECTED
ID_1ST		EQU	80H		;FIRST ID EXPECTED

eproma	segment	byte public

	assume	cs:eproma,ds:dataa,es:dataa

public aa_int16
aa_int16 label byte

;**********************************************************************
;
;     KEYBOARD INPUT ROUTINE  (INT 16H)
;
; (AH)=0	Read character from 84 keyboard
;	output	AL = character code
;		AH = scan code
;
; (AH)=1	Character status from 84 keyboard
;	output	ZF = 1 if no code available
;		ZF = 0, Character/scan code in AX
;
; (AH)=2	Status
;	output	AL bit 7 - Insert State
;		       6 - Caps Lock State
;		       5 - Num Lock State
;		       4 - Scroll Lock State
;		       3 - Alt State
;		       2 - Ctrl State
;		       1 - Left Shift State
;		       0 - Right Shift State
;
; (AH)=3 (AL)=5	Set typmatic delay & rate
;	input	BL bits 4-0 - rate
;			7-5 = 0
;		BH bits 1-0 - delay
;			7-2 = 0
;
; (AH)=5	Put char/scan into keyboard buffer
;	input	CL - character code
;		CH - scan code 
;	output	AL = 0 if successful
;		     1 if buffer full
;
; (AH)=10h	Read character from 101/102 keyboard
;	output	AL = character code
;		AH = scan code
;
; (AH)=11h	Character status from 101/102 keyboard
;	output	ZF = 1 if no code available
;		ZF = 0, Character/scan code in AX
;
; (AH)=12h	Extended status
;	output	AH bit 7 - System Shift
;		       6 - Caps Lock Make/Break
;		       5 - Num Lock  Make/Break
;		       4 - Scroll Lock Make/Break
;		       3 - Right Alt Make/Break
;		       2 - Right Ctrl Lock Make/Break
;		       1 - Left Alt Make/Break
;		       0 - Left Ctrl Make/Break
;		AL bit 7 - Insert State
;		       6 - Caps Lock State
;		       5 - Num Lock State
;		       4 - Scroll Lock State
;		       3 - Alt State
;		       2 - Ctrl State
;		       1 - Left Shift State
;		       0 - Right Shift State
;
;     REGISTERS CHANGED  (AX),AND FLAGS
;
;**********************************************************************

KEY	PROC	FAR 
        STI                            ;INTERRUPTS ON
        PUSH    SI                     ;SAVE REGISTERS
        PUSH    DS
        MOV     SI,DATA
        MOV     DS,SI                  ;LOAD DATA SEGMENT
        CMP     AH,0
        JE      J0                     ;JUMP IF (AH)=0
        CMP     AH,1
        JE      J1                     ;JUMP IF (AH)=1
        CMP     AH,2
        JE      J2                     ;JUMP IF (AH)=2
	CMP	AX,305H
	JE	J3			;JUMP IF (AX)=305
        CMP     AH,10H
        JE      J10                     ;JUMP IF (AH)=10
        CMP     AH,11H
        JE      J11                     ;JUMP IF (AH)=11
        CMP     AH,12H
        JE      J12                     ;JUMP IF (AH)=12
	CMP	AH,5
	JE	J5			;JUMP IF (AH)=5
	sub	ah,12h			;phd 10/26/87
;RETURN    

J6:     POP     DS                     ;RECOVER REGISTERS
        POP     SI
        IRET                           ;RETURN



;(AH)=10 GET CHARACTER from 101/102 keyboard

J10:	call	get_char		;get next char/scan code from buffer
	call	xlate_e			;translate code for 101/102 keyboard
        JMP     J6			;EXIT

;(AH)=0 GET CHARACTER from 84 kryboard

J0:	call	get_char		;get next char/scan code from buffer
	call	xlate_s			;get 84 keyboard compatible code
	jc	J0			;ignore this code & get next code
        JMP     J6			;EXIT

;(AH)=11 GET STATUS AND CHARACTER IF IT IS AVAILABLE from 101/102 keyboard

J11:	call	get_status		;see if buffer is empty
	jz	J1_1			;no char available

	pushf				;save ZF flag
	call	xlate_e			;translate code
	jmp	short J1_2		;return

;(AH)=1 GET STATUS AND CHARACTER IF IT IS AVAILABLE from 84 keyboard

J1:	call	get_status		;see if buffer is empty
	jz	J1_1			;no char available

	pushf				;save ZF flag
	call	xlate_s			;translate code
	jnc	J1_2			;return

	popf
	call	get_char		;discard this code
	jmp	J1			;get next char

J1_2:
	popf				;restore ZF flag
J1_1:
        STI                             ;INTERRUPTS ON
        POP     DS                      ;RESTORE REGISTERS
        POP     SI
        RET     2                       ;THROW AWAY OLD FLAGS, AND RETURN

KEY	ENDP

;(AH)=12 GET EXTENDED SHIFT FLAG

J12:	MOV     AH,FLAG_1               ;STATUS TO (AH)
	mov	al,ah			;save system flag
	and	ah,01110011b
	push	cx
	mov	cl,3
	ror	al,cl			;system flag bit 2 to bit 7
	and	al,10000000b		;clear out the bits except system flag
	or	ah,al
	mov	al,FLAG_3
	and	al,00001100b		;save r alt/ r ctrl flag
	or	ah,al
	pop	cx

;(AH)=2 GET SHIFT FLAG

J2:	MOV	AL,FLAG			;STATUS TO (AL)
	jmp	J6			;return
;(AH)=3 SET TYPMATIC RATE AND DELAY

J3:	cmp	bl,20h			;check the limit of rate
	jae	J6			;error
	cmp	bh,4			;check the limit of delay
	jae	J6			;error
	push	cx
	push	bx
	mov	cl,3
	ror	bh,cl			;delay 1-0 --> 6-5
	or	bh,bl			;add rate
	mov	al,0f3h			;set typematic rate/delay
	call	led4			;send command & wait for ACK
	mov	al,bh			;rate/delay
	call	led4			;send data & wait for ACK
	pop	bx
	pop	cx
	jmp	J6			;return to caller


;(AH)=5 PUT CHAR/SCAN INTO BUFFER

J5:	CLI
	push	bx
        MOV     SI,IN_BUFFER           ;IN BUFFER POINTER
        MOV     BX,SI
        ADD     BX,2
        CMP     BX,KEY_BUFFER_END       ;END OF BUFFER ?
        JNE     J5_1                    ;NO, JUMP
        MOV     BX,KEY_BUFFER_START
J5_1:	CMP     OUT_BUFFER,BX           ;BUFFER FULL ?
        JE      J5_2			;BUFFER FULL, RERURN

        MOV     [SI],CX                 ;CHARACTER TO BUFFER
        MOV     IN_BUFFER,BX            ;UPDATE POINTER
	XOR	AL,AL			;NO ERROR
	JMP	SHORT J5_3
J5_2:
	MOV	AL,1			;BUFFER FULL
J5_3:
	POP	BX
        STI
	jmp	J6			;return


get_char	proc	near
gc1:	STI                            ;INTERRUPTS ON
        NOP
        CLI                            ;INTERRUPTS OFF

;FIND OUT IF THERE IS ANYTHING IN THE BUFFER

        MOV     SI,OUT_BUFFER
        CMP     SI,IN_BUFFER            ;BUFFER EMPTY ?
        JNE     gc_ret			;NO, JUMP

;WAIT FOR KEYBOARD

        MOV     AX,9002H
        INT     15H

;UPDATE LEDS

	push	dx
        XOR     DL,DL                  ;CLEAR EOI FLAG
        CALL    UPDATE_LED             ;UPDATE LEDS
	pop	dx
        JMP     gc1

;GET THE CHARACTER FROM THE BUFFER

gc_ret:	MOV     AX,[SI]                ;BUFFER TO (AX)
        ADD     SI,2                                    
        CMP     SI,KEY_BUFFER_END      ;END OF BUFFER ?
        JNE     gc2			;JUMP IF NOT BUFFER END
        MOV     SI,KEY_BUFFER_START
gc2:	MOV     OUT_BUFFER,SI          ;UPDATE POINTER
	ret
get_char	endp

get_status	proc	near
	push	dx
	XOR     DL,DL                  ;CLEAR EOI FLAG
	CALL    UPDATE_LED             ;UPDATE LED INDICATORS
	pop	dx
        CLI
        MOV     SI,OUT_BUFFER
        CMP     SI,IN_BUFFER           ;ZF = 1 IF BUFFER EMPTY
        MOV     AX,[SI]                ;BUFFER TO (AX)
	sti
	ret
get_status	endp

; xxf0 --> xx00

xlate_e	proc	near
	or	ah,ah
	jz	xe1			;return
	cmp	al,0f0h
	jne	xe1			;return
	mov	al,0
xe1:
	ret	
xlate_e	endp

;e00d --> 1c0d cy=0
;e00a --> 1c0a cy=0
;e0xx --> 352f cy=0
;ah>84h        cy=1
;00f0          cy=0
;xxf0          cy=1
;00e0          cy=0
;xxe0 --> xx00 cy=0

xlate_s	proc	near
;	cmp	ah,84h			;see if F11 or F12
;	ja	xs2			;yes, discard this code
	cmp	ax,0e00dh		;see if new ENTER in keypad
	jne	xs3			;no
xs4:	mov	ah,1ch
	jmp	short xs1
xs3:
	cmp	ax,0e00ah		;see if ctrl + new ENTER
	je	xs4			;yes
;	cmp	ax,0e02fh		;see if '/' in keypad
	cmp	ah,0e0h
	jne	xs5			;no
	mov	ah,35h			;
	jmp	short xs1		;return
xs5:
	;Weipo, check this out...Sounds like the same bug from PC10-III.
	;jmf - 5/23/88
	cmp	ah,85h			;see if F11 or F12
	jae	xs2			;yes, discard this code
	or	ah,ah
	jz	xs1			;return
	cmp	al,0f0h
	je	xs2			;return
	cmp	al,0e0h
	jne	xs1
	mov	al,0
xs1:
	clc
	ret
xs2:
	stc
	ret	
xlate_s	endp

subttl	INT 9H (Keyboard Interrupt Handler)

;******************************************************************************
;
;     KEYBOARD ROUTINE (INT 9)
;     GET A CHARACTER FROM THE KEYBOARD AND INTERPET IT
;
;      FLAG            FLAG_1              FLAG_2                 FLAG_3
;     ------          --------            --------               --------
;0 RSHFT DEPRESSED  L CTRL DEPRESSED   SCRL LED                E1 RECEIVED
;1 LSHFT DEPRESSED  L ALT DEPRESSED    NUM  LED                E0 RECEIVED
;2 CTRL DEPRESSED   SYS REQ DEPRESSED  CAPS LED                R CTRL DEPRESSED
;3 ALT DEPRESSED    HOLD STATE         UNUSED                  R ALT DEPRESSED
;4 SCRL TOGGLED     SCRL DEPRESSED     ACK FROM KEYBOARD       84/101 KEYBOARD
;5 NUM TOGGLED      NUM DEPRESSED      RESEND FROM KEYBOARD    SET NUM LOCK LED
;6 CAPS TOGGLED     CAPS DEPRESSED     LED UPDATE IN PROGRESS  2ND ID BYTE EXPECTED
;7 INS TOGGLED      INS DEPRESSED      KEYBOARD ERROR          1ST ID BYTE EXPECTED
;
;******************************************************************************
	
KEYBOARD PROC NEAR

        STI
        PUSH    AX
        PUSH    BX
        PUSH    DI
        PUSH    DS
        PUSH    DX
        MOV     AX,DATA
        MOV     DS,AX

;DISABLE THE KEYBOARD

        MOV     AH,2 
        CALL    WAIT_UPI               ;WAIT FOR INPUT BUFFER TO BE EMPTY
        MOV     AL,0ADH
        OUT     UPI+4,AL               ;SEND DISABLE COMMAND
	jmp	short $+2

;GET THE CHARACTER FROM THE KEYBOARD

        IN      AL,UPI                 ;AL = SCAN CODE

	stc				;set carry flag
	mov	ah,4fh			;function 4fh of int 15h
	int	15h
	mov	dl,7			;send EOI, enable keyboard & clear flags
	jnc	Q3_3			;scan code handle by user, exit 

        MOV     AH,AL                  ;AH = SCAN CODE
        STI

;CHECK FOR OVERRUN

        CMP     AL,0FFH                ;OVERRUN ?
        JNE     Q1                     ;NO, JUMP
        JMP     Q44                    ;EXIT                    

;PROCESS RESEND

Q1:     CMP     AL,0FEH                ;RESEND ?
        JNE     Q2                     ;NO, JUMP
        OR      FLAG_2,20H             ;SET RESEND
	JMP     short Q3_3             ;EXIT

;PROCESS ACK FROM KEYBOARD

Q2:     CMP     AL,0FAH                ;ACK ?
        JNE     Q3                     ;NO, JUMP
        OR      FLAG_2,10H             ;SET ACK
        JMP     short Q3_3             ;EXIT

;UPDATE LED INDICATORS  

Q3:     CALL    UPDATE_LED

;process read id command

	test	flag_3,ID_1ST		;1st id byte expected?
	jz	Q3_1			;no, try 2nd byte

	and	flag_3,NOT ID_1ST	;clear bit for 1st ID byte
	cmp	al,0abh			;valid 1st ID byte?
	jne	Q3_3			;no, exit

	or	flag_3,ID_2ND		;2nd ID byte expected
	jmp	short Q3_3		;exit
Q3_1:
	test	flag_3,ID_2ND		;2nd ID byte expected?
	jz	Q4			;no, process normal scan code

	and	flag_3,NOT ID_2ND	;clear bit for 2nd ID byte
	or	flag_3,KEY_101		;assume 101/102 keyboard
	cmp	al,85h			;see if plain 101/102 keyboard
	je	Q3_3			;assumption is correct, exit

	cmp	al,41h			;see if unknown keyboard
	je	Q3_2			;yes, take care of NUM LOCK

	and	flag_3,NOT KEY_101	;this is not a 101/102 keyboard
	jmp	short Q3_3		;exit
Q3_2:
	test	flag_3,S_NUM_LOCK	;see if NUM LOCK LED need to be on
	jz	Q3_3			;no, exit

	or	flag,NUM_LOCK_ON	;NUM LOCK ON
	call	update_led		;update LED
Q3_3:
	jmp	Q45			;exit

Q4:	cmp	al,0e0h			;see if 2 byte scan code
	jne	Q4_1			;not e0

	or	flag_3,E0_RECEIVED+KEY_101
	jmp	short Q4_2		;exit
Q4_1:
	cmp	al,0e1h			;see if 2 byte scan code
	jne	Q4_3			;not e1

	or	flag_3,E1_RECEIVED+KEY_101
Q4_2:	mov	dl,5			;send EOI & enable keyboard
	jmp	Q45			;exit

;CHECK FOR ILLEGAL SCAN CODE

Q4_3:	MOV     BL,AL 
        AND     BL,7FH                 ;MASK OFF BREAK BIT
        CMP     BL,59H                 ;ILLEGAL SCAN CODE ?
        jae     Q3_3                   ;exit

;PROCESS ALT DEPRESSED                                                 

	cmp	bl,38h			;alt key?
	jne	Q7			;no, see if ctrl key

	call	check_e1		;check if e1+alt
					;if so, ignore this code
	test	al,80h			;see if ALT break
        jnz     Q5                      ;yes, ALT break

        OR      FLAG,8                  ;SET FLAG
	test	flag_3,E0_RECEIVED	;see if right alt key
	jz	Q4_6			;left alt key

	or	flag_3,8		;right alt key pressed
	jmp	Q3_3  			;exit
Q4_6:
	or	flag_1,2		;left alt key pressed
	JMP     Q3_3			;EXIT

;PROCESS ALT RELEASED

Q5:     AND     FLAG,0F7H               ;CLEAR FLAG
	test	flag_3,E0_RECEIVED	;see if right alt key
	jz	Q5_1			;left alt key

	and	flag_3,0f7h		;right alt key released
	test	flag_1,2		;see if left alt on
	jnz	Q5_3			;yes, set flag on
	jmp	short Q5_2		;exit
Q5_1:
	and	flag_1,0fdh		;left alt key released
	test	flag_3,8		;see if right alt is on
	jz	Q5_2			;no
Q5_3:
	or	flag,8			;alt flag on
Q5_2:
        MOV     AL,ALT_BUFFER
        MOV     ALT_BUFFER,0           ;CLEAR ALT BUFFER
        XOR     AH,AH
        CMP     AL,0                   ;ANYTHING IN ALT BUFFER ?
        JE	Q8_2                   ;no, JUMP
;alt + 2 + 5 + 5 problem, 03/05/87 jwy
	JMP     Q42_1                  ;PUT IT IN THE BUFFER
;	JMP     Q42                    ;PUT IT IN THE BUFFER

;PROCESS CTRL DEPRESSED

Q7:	cmp	bl,1dh			;ctrl key?
	jne	Q9			;no, see if left shift key

	call	check_e1		;check if e1+ctrl
					;if so, ignore this code
	test	al,80h			;see if break
        jnz     Q8                      ;yes, JUMP

        OR      FLAG,4                  ;SET FLAG
	test	flag_3,E0_RECEIVED	;see if right ctrl key
	jz	Q7_2			;left ctrl key

	or	flag_3,4		;right ctrl key pressed
	jmp	short Q8_2		;exit
Q7_2:
	or	flag_1,1		;left ctrl key pressed
	JMP     short Q8_2		;EXIT

;PROCOCESS CTRL RELEASED 

Q8:     AND     FLAG,0FBH               ;CLEAR FLAG
	test	flag_3,E0_RECEIVED	;see if right ctrl key
	jz	Q8_1			;left ctrl key

	and	flag_3,0fbh		;right ctrl key released
	test	flag_1,1		;see if left ctrl on
	jnz	Q8_3			;yes, ctrl on
	jmp	short Q8_2		;exit
Q8_1:
	and	flag_1,0feh		;left ctrl key released
	test	flag_3,4		;see if right ctrl on
	jz	Q8_2			;no
Q8_3:
	or	flag,4			;ctrl on
Q8_2:
	;send EOI and clear flags
        JMP     Q45                    ;EXIT

;PROCESS LEFT SHIFT DEPRESSED

Q9:	cmp	bl,2ah			;see if left shift key
	jne	Q11			;no, process right shift

	call	check_e0		;check if e0+left shift
					;if so, ignore this code and reset e0 & e1
	call	check_e1		;check if e1+left shift
					;if so, ignore this code
	test	al,80h			;see if break code
        jnz	Q10			;yes, this is break left shift key

        OR      FLAG,2                  ;SET FLAG
        JMP     short Q10_1             ;EXIT

;PROCESS LEFT SHIFT RELEASED

Q10:    AND     FLAG,0FDH            ;CLEAR FLAG 
Q10_1:
	;send EOI and clear flags
        JMP     Q45                    ;EXIT

;PROCESS RIGHT SHIFT DEPRESSED

Q11:	cmp	bl,36h			;see if right shift key
	jne	Q13			;no, process sys req

	call	check_e0		;check if e0+right shift
					;if so, ignore this code and reset e0 & e1
	call	check_e1		;check if e1+right shift
					;if so, ignore this code
	test	al,80h			;see if break code
        jnz	Q12			;yes, this is break right shift key

        OR      FLAG,1                  ;SET FLAG
        JMP     Q10_1                   ;EXIT

;PROCESS RIGHT SHIFT RELEASED 

Q12:    AND     FLAG,0FEH               ;CLEAR FLAG
        JMP     Q10_1                   ;EXIT

Q13:	test	flag_3,E1_RECEIVED	;see if E1 + NN
	jz	Q13_1			;no

	cmp	al,45h			;NUM LOCK on 101 keyboard depressed?
	jne	Q13_2			;reset E0, E1 bits & send EOI
	jmp	Q20_1			;hold state ON
Q13_1:

;PROCESS SYS REQ DEPRESSED

	CMP     bl,54H                 ;SYS REQ KEY?
	jne	Q16			;no, handle INS key

	test	al,80h			;break key?
	jnz	Q15			;yes

        TEST    FLAG_1,4               ;FLAG ALREADY SET ?
        JZ      Q14                    ;NO, JUMP 
Q13_2:  JMP     Q45                    ;EXIT

Q14:    OR      FLAG_1,4               ;SET FLAG
	mov	dl,5
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     AX,8500H
        STI
        INT     15H                                                            
        JMP     Q46                    ;EXIT

;PROCESS SYS REQ RELEASED

Q15:    AND     FLAG_1,0FBH            ;CLEAR FLAG 
	mov	dl,5
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     AX,8501H                                      
        STI
        INT     15H
        JMP     Q46                    ;EXIT

;PROCESS INS DEPRESSED

Q16:	cmp	bl,52h			;see if INS key
	jne	Q18			;no, process Caps Lock

	test	al,80h			;see if break key
	jnz	Q17			;yes, break key

;       TEST    FLAG,20H             ;NUM LOCK ?
; 	test	FLAG,2fh	;test for numlock, cntrl, alt, LR shift
 	test	FLAG,0ch		;test for cntrl, alt
	jz	Q16_1			;neither alt nor ctrl
Q16_0:
	jmp	Q29
Q16_1:
	test	flag_3,E0_RECEIVED	;see if E0 + INS (new insert key)
	jnz	Q16_3			;yes, new INSERT key

;process 0/INS key

 	test	FLAG,23h		;test for numlock, LR shift
	jz	Q16_3			;Ins function

	test	flag,20h		;see if Num Lock state
	jz	Q16_0			;no, numeric '0'

	test	flag,3			;see if LR shift
	jz	Q16_0			;no, numeric '0'
Q16_3:
        MOV     AL,80H
        CALL    Q49                     ;SET AND TOGGLE FLAG      
        MOV     AX,5200H		;return code for 0/INS key
	test	flag_3,E0_RECEIVED	;see if E0 + INS (new insert key)
	jz	Q16_4			;no, 0/INS key
	mov	al,0e0h			;return code for new INSERT key
Q16_4:
        JMP     Q42_1                   ;CHARACTER TO BUFFER

;PROCESS INS RELEASED

Q17:    AND     FLAG_1,7FH             ;CLEAR FLAG 
        JMP     Q45                    ;EXIT                      

;PROCESS CAP LOCK DEPRESSED

Q18:	cmp	bl,3ah			;see if Caps Lock
	jne	Q20			;no, process Num Lock

	test	al,80h			;see if break key
	jnz	Q19			;yes, break key

    	test	FLAG,4		        ;check for cntrl flag
	jz	Q18_1			;no cntrl 
	jmp	Q29			;yes, then do not change state
Q18_1:
        MOV     AL,40H
        CALL    Q49                    ;SET AND TOGGLE FLAG
        JMP     Q45                    ;EXIT

;PROCESS CAP LOCK RELEASED  

Q19:    AND     FLAG_1,0BFH            ;CLEAR FLAG
Q19_1:  JMP     Q45                    ;EXIT

;PROCESS NUM LOCK DEPRESSED

Q20:	cmp	bl,45h			;see if Num Lock
	jne	Q25			;no, process Scroll Lock

	test	al,80h			;see if break key
	jnz	Q24			;yes, break key

        TEST    FLAG,4                  ;CTL DEPRESSED ?
        JZ      Q23                     ;NO, PROCESS NUM LOCK FLAGS

	test	flag,8			;alt also?
	jz	Q20_2			;no, ctrl only
	jmp	Q32			;handle alt mode
Q20_2:
	test	FLAG_1,8		;see if already in hold state
	jnz	Q19_1			;in hold state, exit

	test	flag_3,KEY_101		;see if 101/102 keyboard
	jz	Q20_1			;ctrl + Num Lock on 84 key, hold
	jmp	Q29			;ctrl + Num Lock on 101 key, do not hold

;PROCESS PAUSE

Q20_1:	OR      FLAG_1,8H              ;SET PAUSE
	MOV	DL,5			;send EOI & enable keyboard
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        CMP     VIDEO_MODE,7           ;MONOCHROME ?
        JE      Q22                    ;YES, SKIP TURNING ON CRT
        PUSH    DX
        MOV     DX,3D8H                ;VIDEO ADAPTER ADDRESS
        MOV     AL,VIDEO_MODE_REG
        OUT     DX,AL                  ;TURN ON CRT
        POP     DX
Q22:    TEST    FLAG_1,8H              ;PAUSE ?
        JNZ     Q22                    ;YES, KEEP CHECKING
        JMP     Q45                    ;EXIT

;PROCESS NUM LOCK FLAGS

Q23:    MOV     AL,20H
        CALL    Q49                    ;SET AND TOGGLE FLAG       
        JMP     Q45                    ;EXIT

;PROCESS NUM LOCK RELEASED

Q24:    AND     FLAG_1,0DFH            ;CLEAR FLAG  
        JMP     Q45                    ;EXIT                  

;PROCESS SCROLL DEPRESSED

Q25:	cmp	bl,46h			;see if Scroll Lock
	jne	Q28			;no, process soft reset

	test	al,80h			;see if break key
	jnz	Q27			;yes, break key

        TEST    FLAG,4                  ;CTRL MODE ?
        JNZ     Q26                     ;YES, JUMP                 

        MOV     AL,10H
        CALL    Q49                    ;TOGGLE AND SET FLAG
        JMP     Q45                    ;EXIT
;PROCESS BREAK

Q26:	test	flag,8			;alt also?
	jz	Q26_2			;no, ctrl only
	jmp	Q32			;handle alt mode
Q26_2:
	test	FLAG_1,8		;check for hold state
	jz	Q26_3			;no
	jmp	Q29_1			;yes, then unhold it

Q26_3:
	test	flag_3,KEY_101		;see if 101/102 keyboard
	jz	Q26_1			;no, process key break

	test	flag_3,E0_RECEIVED	;E0 + NN?
	jz	Q28			;ctrl+ScrllLock on 101, not a break

;ctrl + Scroll Lock on 84 keyboard or ctrl + Pause/Break on 101/102 keyboard

Q26_1:
;	mov	di,key_buffer_start
	mov	di,out_buffer
	mov	in_buffer,di		;fixed CNTRL Break problem
;       MOV     OUT_BUFFER,DI
;       MOV     byte ptr KEY_BUFFER,80H ;SET BREAK FLAG
	mov	break_key,80h
        CALL    Q48                    ;ENABLE KEYBOARD
        INT     1BH                    ;BREAK INTERRUPT
        XOR     AX,AX                  ;CODE
;       JMP     Q45                    ;PUT IT IN THE BUFFER
	jmp	Q42_1		       ;put it in the buffer

;PROCESS SCROLL RELEASED

Q27:    AND     FLAG_1,0EFH            ;CLEAR FLAG
        JMP     Q45                    ;EXIT

;PROCESS RESET

Q28:    TEST    FLAG,4               ;CONTROL DEPRESSED ?
        JZ      Q29                    ;NO, JUMP
        TEST    FLAG,8               ;ALT DEPRESSED ? 
        JZ      Q29                    ;NO, JUMP
	test	flag_3,KEY_101		;see if 101/102 keyboard
	jz	Q28_1			;no

	test	flag_1,SYS_REQ		;see if system req state
	jnz	Q29			;yes, not a real alt shift state
Q28_1:
        CMP     AL,53H                 ;DEL DEPRESSED ?
        JNE     Q28_2                  ;NO, JUMP
;        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     RESET_KEY,1234H
        XOR     AH,AH 
        JMP     test_1			;GO TO beginning of power on test
Q28_2:
	cmp	al,1			;see if ESC
	jne	Q29_A			;no
	call	Q47			;send EOI to keyboard
	jmp	RSETUP		;
;
; 5/30/88 - jmf - keyboard speed change
;
Q29_A:
	cmp	al,31			;check for cntrl alt S
	je	Q29_B			;yes, then do speed change to 6 Mhz
	cmp	al,20			;check for cntrl alt T
	je	Q29_C			;yes, then do speed change to 8 Mhz
	cmp	al,32			;check for cntrl alt D
	jne	Q29			;No normal key process 
	push	ax			;
	mov	al,3			;set speed change to 12 Mhz
Q29_D:
	push	dx
	mov	dx,180H			;speed change port
	out	dx,al			;
	nop				;
	mov	dl,al	  		;save speed change pattern
	mov	al,cmos_sp_flag+80h	;get speed flag in AH
	call	read_cmos
	and	ah,80h			;only leave the bit for hd IO
	or	ah,dl			;merge in speed change pattern
	mov	al,cmos_sp_flag+80h	;write back to cmos
	call	write_cmos
	push	bx
	mov	bx,0080h
	call	beep
;	jmp	Q45			;return
	pop	bx
	pop	dx
	pop	ax			;restore scan code and process
	jmp	short Q29		;normal key
Q29_B:
	push	ax			;
	mov	al,0			;
	jmp	short Q29_D		;
Q29_C:
	push	ax			;save scan code
	mov	al,1			;
	jmp	short Q29_D		;

;PROCESS EXIT PAUSE

Q29:    TEST    FLAG_1,8               ;WAIT ?
        JZ      Q30                    ;NO, JUMP
	test	al,80h		       
	jnz	Q30
Q29_1:
        AND     FLAG_1,0F7H            ;CLEAR FLAG  
        JMP     Q45                    ;EXIT

;PROCESS PRINT SCREEN         

Q30:	test	flag,4			;see if ctrl shift
	jnz	Q31			;yes, do not process PrtSc

	test	flag_3,KEY_101		;see if 101/102 keyboard
	jz	Q30_1			;no, check shift state

	test	flag_3,E0_RECEIVED	;see if new key
	jnz	Q30_2			;new key, do not check shift state
	jmp	short Q31
Q30_1:
	TEST    FLAG,3               ;LEFT OR RIGHT SHIFT ?
        JZ      Q31                    ;NO, JUMP
Q30_2:
        CMP     AL,37H                 ;PTR SCREEN DEPRESSED ?
        JNE     Q31                    ;NO, JUMP 
	MOV	DL,5			;send EOI & enable keyboard
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        INT     5                      ;PRINT SCREEN
	MOV	DL,2			;CLEAR FLAGS
        JMP     Q45                    ;EXIT

;PROCESS NORMAL KEY RELEASE

Q31:    TEST    AL,80H                 ;KEY RELEASED ?
        JZ      Q32                    ;NO, JUMP 
        JMP     Q45                    ;EXIT

;PROCESS ALT MODE, KEYPAD CHARACTER

Q32:    TEST    FLAG,8               ;ALT MODE ?     
	jnz	Q32_0			;yes, process alt + key
	jmp	Q36
Q32_0:
	test	flag_3,KEY_101		;see if 101/102 keyboard
	jz	Q32_1			;no

	test	flag_1,SYS_REQ		;see if system req state
	jnz	Q36			;yes, not a real alt shift state
Q32_1:
        CALL    Q51                    ;KEYPAD CHARACTER ?
        JC      Q34                    ;NO, JUMP    

;XLAT KEYPAD SCAN CODE 

	mov	ah,al			;save scan code
        SUB     AL,71        
        MOV     BX,OFFSET KEY_TABLE_1
        XLAT    byte ptr CS:KEY_TABLE_1
        MOV     BL,AL
	mov	al,ah			;restore scan code
        CMP     BL,-1                  ;NUMBER KEY ?
	je	Q34			;no, del, - or +

	test	flag_3,E0_RECEIVED	;see if E0 + NN
	jnz	Q34			;one of new key

;GENERATE NEW ALT BUFFER VALUE

Q33:    MOV     AL,ALT_BUFFER
        MOV     AH,10
        MUL     AH
        ADD     AL,BL
        MOV     ALT_BUFFER,AL
        JMP     Q45                    ;EXIT

;PROCESS NORMAL KEY ALT MODE

Q34:	cmp	al,39h			;see if space
	je	Q34_1			;yes
	cmp	al,0fh			;see if tab key
	je	Q34_1			;yes
	cmp	al,4ah			;see if '-'
	je	Q34_1			;yes
	cmp	al,4eh			;see if '+'
	je	Q34_1			;yes
	mov	alt_buffer,0		;zero out alt key buffer
Q34_1:
	mov	ah,al			; save scan code
        MOV     BX,OFFSET KEY_TABLE_2
        XLAT    byte ptr CS:KEY_TABLE_2
	xchg	al,ah			;al=scan code, ah=value from table
        MOV     BX,OFFSET KEY_TABLE_2_1
        XLAT    byte ptr CS:KEY_TABLE_1 ;AL = CODE

	test	flag_3,E0_RECEIVED	;see if E0 + NN
	jz	Q35			;no

	cmp	ah,1ch			;new ENTER?
	jne	Q34_2			;no
	mov	ax,0a600h
	jmp	short Q35
Q34_2:
	cmp	ah,35h			;new '/'
	jne	Q34_3			;no
	mov	ax,0a400h
	jmp	short Q35
Q34_3:
	cmp	ah,53h			;new Delete?
	jne	Q35			;no
	mov	ax,0a300h
Q35:
	jmp	short Q36_1

;PROCESS NORMAL KEY CTRL MODE

Q36:    TEST    FLAG,4               ;CTRL MODE ?
        JZ      Q37                    ;NO, JUMP

	cmp	al,35h			;?/ on 84, / in keypad on 101
	jne	Q36_2			;no

	test	flag_3,E0_RECEIVED	;see if new key
	jz	Q36_2			;no

	mov	ax,9500h		;/ in keypad on 101
	jmp	Q42_1			;save code
Q36_2:
	push	bx			;save scan code in bl
	mov	ah,al			; save scan code
        MOV     BX,OFFSET KEY_TABLE_3
        XLAT    byte ptr CS:KEY_TABLE_3
	xchg	al,ah			;al=scan code, ah=value from table
        MOV     BX,OFFSET KEY_TABLE_4
        XLAT    byte ptr CS:KEY_TABLE_4 ;AL = CODE
	pop	bx			;restore scan code in bl
	cmp	ah,1ch			;see if ENTER key
	jne	Q36_3			;no

	test	flag_3,E0_RECEIVED	;see if new ENTER key
	jz	Q36_1			;no, old ENTER key, return 1c0ah

	mov	ah,0e0h			;new ENTER key
	jmp	short Q36_1
Q36_3:
	cmp	bl,47h			;see if key pad key
	jb	Q36_4			;no

	test	flag_3,E0_RECEIVED	;see if new key pad key
	jz	Q36_1			;no

	mov	al,0e0h			;new key pad key
	jmp	short Q36_1
Q36_4:
	cmp	ax,7200h		;see if ctrl+PrintScreen or ctrl+PrtSc
					; or ctrl+* in key pad 
	jne	Q36_1			;no

;ctrl+PrtSc = 7200h
;ctrl+PrintScreen = 7200h
;ctrl+* in key pad = 9600h

	test	flag_3,KEY_101 		;see if 101/102 keyboard
	jz	Q36_1			;no, this is ctrl+PrtSc

	test	flag_3,E0_RECEIVED	;see if PrintScreen
	jnz	Q36_1			;yes, ctrl+PrintScreen

	mov	ax,9600h		;ctrl+* on 101
Q36_1:  JMP     Q42              	;CHARACTER TO BUFFER

;PROCESS NORMAL KEY WITH NO LEFT OR RIGHT SHIFT                 

Q37:	cmp	al,35h			;?/ on 84, / in keypad on 101
	jne	Q37_1			;no

	test	flag_3,E0_RECEIVED	;see if new key
	jz	Q37_1			;no

	mov	ax,0e02fh		;e0+37h '/' on 101
	jmp	Q42_1
Q37_1:
;	cmp	al,37h			;see if PrtSc/* or new '*'
;	jne	Q37_3			;no

;	test	flag_3,E0_RECEIVED	;new key?
;	jz	Q37_3			;no

;	mov	ax,372ah
;	jmp	Q42_1
Q37_3:

	TEST    FLAG,3H              ;SHIFT ?
        JNZ     Q40                    ;YES, JUMP

;HANDLE KEYPAD CHARACTERS

        CALL    Q51                    ;KEYPAD CHARACTER ?
        JC      Q38                    ;NO, JUMP

	test	flag_3,E0_RECEIVED	;see if new key
	jnz	Q37_2			;yes, do not check num lock

        TEST    FLAG,20H             ;NUM LOCK ?
        JNZ     Q41                    ;YES, JUMP
Q37_2:
;	cmp	al,4ch			;5?
;	jne	Q37_4			;no

;	mov	ah,0f0h
;	xchg	al,ah
;	jmp	Q42

Q37_4:
	test	flag_3,E0_RECEIVED	;see if one of new key
	jz	Q38			;no

	mov	al,0e0h
	jmp	short Q38_2
	
;XLATE CHARACTER

Q38:    MOV     BX,OFFSET KEY_TABLE_5
        XLAT    byte ptr CS:KEY_TABLE_5         ;AL = CODE

	cmp	ah,57h			;F11?
	je	Q38_1			;yes
	cmp	ah,58h			;F12?
	jne	Q38_2			;no
Q38_1:
	mov	ah,0
	xchg	al,ah
	jmp	short Q42_1		;put it into buffer
Q38_2:
;	cmp	ah,52h			;is it insert key
;	jne	Q38_3			;no
;	push	ax			;
;	mov	al,80h			;toggle flag
;	call	Q49			;
;	pop	ax
Q38_3:

;HANDLE LETTERS IN CAP LOCK MODE

        CMP     AL,"a"                 ;CHARACTER < "a"        
        JL      Q39                    ;YES, JUMP               
        CMP     AL,"z"                 ;CHARACTER > "z"     
        JA      Q39                    ;YES, JUMP
        TEST    FLAG,40H             ;CAPS ?
        JZ      Q39                    ;NO, JUMP
        AND     AL,0DFH                ;CONVER LOWER CASE TO UPPER CASE
Q39:    JMP     short Q41_1            ;CHARACTER TO BUFFER

;PROCESS KEY WITH LEFT OR RIGHT SHIFT
;HANDLE KEYPAD CHARACTERS

Q40:	CALL    Q51                    ;KEYPAD CHARACTER
        JC      Q41                    ;NO, JUMP

	test	flag_3,E0_RECEIVED	;see if new key
	jnz	Q37_2			;yes, do not check num lock

        TEST    FLAG,20H             ;NUM LOCK ?
        JNZ     Q38                    ;YES, JUMP  

;XLATE CHARACTER

Q41:    MOV     BX,OFFSET KEY_TABLE_6
        XLAT    byte ptr CS:KEY_TABLE_6
        XCHG    AH,AL                                  
        MOV     BX,OFFSET KEY_TABLE_7
        XLAT    byte ptr CS:KEY_TABLE_7

;HANDLE LETTERS IN CAP LOCK MODE

        CMP     AL,"A"                 ;CHARACTER < "A" ?       
        JB      Q41_1                  ;YES, JUMP                   
        CMP     AL,"Z"                 ;CHARACTER > "Z" ?
        JA      Q41_1                  ;YES, JUMP
        TEST    FLAG,40H               ;CAPS
        JZ      Q41_1                  ;NO, JUMP
        OR      AL,20H                 ;CONVERT UPPER TO LOWER CASE
Q41_1:
	cmp	ah,1ch			;see if ENTER
	jne	Q42			;no

	test	flag_3,E0_RECEIVED	;see if new ENTER key
	jz	Q42			;no

	mov	ah,0e0h			;new ENTER inkeypad region

;CHARACTER TO BUFFER

Q42:    CMP     AL,-1                  ;ILLEGAL CHARACTER ?
        JE      Q45                    ;YES, JUMP                      
        CMP     AH,-1                  ;ILLEGAL CHARACTER ?
        JE      Q45                    ;YES, JUMP                

;UPDATE BUFFER       

Q42_1:
        CLI
        MOV     DI,IN_BUFFER           ;IN BUFFER POINTER
        MOV     BX,DI
        ADD     BX,2
        CMP     BX,KEY_BUFFER_END      ;END OF BUFFER ?
        JNE     Q43                    ;NO, JUMP
        MOV     BX,KEY_BUFFER_START
Q43:    CMP     OUT_BUFFER,BX          ;BUFFER FULL ?
        JE      Q44                    ;YES, SEND BEEP TO SPEAKER
        MOV     [DI],AX                ;CHARACTER TO BUFFER
        MOV     IN_BUFFER,BX           ;UPDATE POINTER
        STI
	MOV	DL,5			;send EOI & enable keyboard
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     AX,9102H
        INT     15H     
	MOV	DL,2			;CLEAR FLAGS 
        JMP     Q45                    ;EXIT

;ERROR, SEND BEEP TO SPEAKER

Q44:    STI
        MOV     BL,80                  ;BEEP LENGTH
        CALL    BEEP
	mov	dl,5			;send EOI & enable keyboard
;EXIT

Q45:    CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD 
Q46:    POP     DX
        POP     DS
        POP     DI
        POP     BX
        POP     AX
        IRET

;SUBROUTINE TO SEND EOI AND ENABLE THE KEYBOARD
;DL bit 0 = 1 if eoi
;       1 = 1 if reset flag

Q47:    TEST    DL,2                   ;reset e0 & e1 flag?
        JZ      Q47_1                  ;NO, JUMP

	and	flag_3, NOT E0_RECEIVED+E1_RECEIVED
	AND	DL,0FDH			;CLEAR FLAG BIT
Q47_1:  TEST    DL,1                   ;EOI FLAG SET ?
        JZ      Q47_2                    ;NO, JUMP

        CLI
        MOV     AL,EOI
        OUT     INT1,AL       
        AND     DL,0FEH                  ;CLEAR EOI FLAG

Q47_2:	TEST	DL,4			;SEE IF ENABLE KEYBOARD
	JZ	Q48_1			;NO, BYPASS

;ENTRY POINT TO ENABLE KEYBOARD

Q48:    MOV     AH,2
        CALL    WAIT_UPI               ;WAIT FOR INPUT BUFFER EMPTY
        MOV     AL,0AEH 
        OUT     UPI+4,AL               ;ENABLE KEYBOARD
Q48_1:
	STI
        RET

;SUBROUTINE TO SET AND TOGGLE FLAG

Q49	proc	near
        CLI
        TEST    FLAG_1,AL              ;KEY DEPRESSED ?
        JNZ     Q49_1                  ;YES, RETURN

        OR      FLAG_1,AL              ;SET FLAG
        XOR     FLAG,AL                ;TOGGLE FLAG
        STI
        CALL    UPDATE_LED             ;UPDATE LEDS
Q49_1:
        RET
Q49	endp

;SUBROUTINE TO CHECK FOR KEYPAD KEY (CY=0 FOR KEYPAD CHARACTER)

Q51	proc	near
        CMP     AL,71                  ;KEY < 71 ?
        JB      Q51_1                  ;YES, JUMP
        CMP     AL,83                  ;KEY > 82 ?
        JA      Q51_1                  ;YES, JUMP
        CLC                            ;CY = 0 FOR KEYPAD CHARACTER
        RET
Q51_1:  STC                            ;CY = 1 FOR NON KEYPAD CHARACTER
        RET
Q51	endp

;see if E0 + NN, if so, reset E0, E1 received flags and exit

check_e0	proc	near
	test	flag_3,E0_RECEIVED	;see if E0 + NN
	jz	chk_e0_ret		;no, return to caller

	pop	ax			;do not return to caller
	;reset flags & send EOI
	jmp	Q45			;exit
chk_e0_ret:
	ret
check_e0	endp

;see if E1 + NN, if so, exit

check_e1	proc	near
	test	flag_3,E1_RECEIVED	;see if E1 + NN
	jz	chk_e1_ret		;no, return to caller

	pop	ax			;do not return to caller
	mov	dl,5			;send EOI  enable keyboard
	jmp	Q45			;exit
chk_e1_ret:
	ret
check_e1	endp


;******************************************************************************
;
;     SUBROUTINE TO UPDATE THE KEYBOARD LEDS IF NECESSARY
;
;     ENTER: DL = 1 SEND EOI TO INTERRUPT CONTROLLER
;            DL = 0 SKIP SENDING EOI
;       
;******************************************************************************

UPDATE_LED PROC NEAR

;CHECK FOR CHANGE IN LED INDICATORS

        CLI
        PUSH    AX
        PUSH    CX
        MOV     AL,FLAG
        MOV     CL,4
        SHR     AL,CL
        AND     AL,07H 
        MOV     CL,AL                  ;SHIFT STATE IN BITS 0-2  
        MOV     AH,FLAG_2
        AND     AH,07H                 ;LED BITS IN BITS 0-2
        XOR     AL,AH                  ;ANY CHANGE ?
        JZ      LED3                   ;NO, EXIT

;TEST FOR UPDATE IN PROGRESS

        TEST    FLAG_2,40H             ;UPDATE IN PROGRESS ?
        JNZ     LED3                   ;YES, EXIT
        OR      FLAG_2,40H             ;SET BUSY

;SEND EOI IF NEEDED

        TEST    DL,1                   ;EOI FLAG ?
        JZ      LED1                   ;NO, JUMP
        MOV     AL,EOI
        OUT     INT1,AL                ;SEND EOI
	AND	DL,0FEH			;CLEAR EOI FLAG

;UPDATE LEDS IN KEYBOARD

LED1:   MOV     AL,0EDH 
        CALL    LED4                   ;SEND LED COMMAND TO KEYBOARD
        JNZ     LED2                   ;JUMP IF ERROR
        MOV     AL,CL                  ;CL = LED BITS 
        CALL    LED4                   ;SEND LED BITS
        JZ      LED0                   ;JUMP IF NO ERROR
LED2:   MOV     AL,0F4H 
        CALL    LED4                   ;SEND ENABLE TO KEYBOARD

;UPDATE THE LED BITS IN FLAG_2  

        CLI
LED0:   AND     FLAG_2,038H            ;CLEAR THE ERROR, LED BITS AND BUSY
        OR      FLAG_2,CL              ;SET THE LED BITS

;RETURN

LED3:   STI
        POP     CX
        POP     AX
        RET

;SUBROUTINE TO TRANSMITT TO THE KEYBOARD 
;SEND BYTE TO KEYBOARD

LED4:   PUSH    CX
	mov	bl,3
led41:	cli
	AND     FLAG_2,0CFH            ;CLEAR ACK, AND RESEND FLAGS
        PUSH    AX
        MOV     AH,2
        CALL    WAIT_UPI               ;WAIT FOR INPUT BUFFER
        POP     AX
        OUT     UPI,AL                 ;SEND BYTE TO KEYBOARD 

;WAIT FOR KEYBOARD TO RESPOND

        STI
LED5:   MOV     CX,01A00H              ;10 MSEC WAIT FOR KEYBOARD RESPONCE
LED6:   TEST    FLAG_2,10H             ;ACK ?               
        JNZ     LED8                   ;YES, EXIT 
        TEST    FLAG_2,20H             ;RESEND ?
        JNZ     LED7                   ;YES, TRY AGAIN
        LOOP    LED6
LED7:   DEC     BL                     ;DECREMENT RETRY COUNT
        JNZ     LED41                  ;JUMP IF RETRY NOT ZERO
        OR      FLAG_2,80H             ;SET ERROR
LED8:   TEST    FLAG_2,80H             ;TEST FOR ERROR
        POP     CX
        RET

;
UPDATE_LED ENDP

keyboard	endp

eproma	ends
	end

