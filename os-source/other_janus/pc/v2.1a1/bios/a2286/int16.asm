page 	60,132
include	title.inc
subttl	INT 16h  (Keyboard Module)

.xlist
include externals.inc
include equates.inc
extrn	out_buffer:word
extrn	in_buffer:word
extrn	key_buffer_start:word
extrn	key_buffer_end:word
extrn	flag_1:byte
extrn	wait_upi:near
extrn	flag_3:byte
extrn	alt_buffer:byte
extrn	flag_2:byte
extrn	video_mode:byte
extrn	video_mode_reg:byte
extrn	key_buffer:near
extrn	reset_key:word
extrn	key_table_1:near
extrn	key_table_2:near
extrn	key_table_3:near
extrn	key_table_4:near
extrn	key_table_5:near
extrn	key_table_6:near
extrn	key_table_7:near
extrn	beep:near
extrn      config_setup:near
public	key
public	keyboard
extrn	test_1:near
.list


dataa	segment	at 40h
dataa	ends

zero	segment	at 0h
zero	ends

eproma	segment	byte public

	assume	cs:eproma,ds:dataa,es:dataa

.xlist
start_of_int16	equ	$+443h
.list

page
;******************************************************************************
;
;     KEYBOARD INPUT ROUTINE  16H
;
;     INPUT                      OUTPUT
;
;     (AH)=0 READ CHARACTER      (AL)=ASCII CHARACTER, (AH)=SCAN CODE
;
;     (AH)=1 CHARACTER STATUS    (ZF)=1, NO CODE
;                                (ZF)=0, CHARACTER IN (AX) AND BUFFER
;
;     (AH)=2 SHIFT STATUS        (AL)=SHIFT STATUS---FOR DEFINITION OF      
;                                SHIFT STATUS, SEE FLAG_1 OF KEYBOARD 
;                                ROUTINE
;   
;     REGISTERS CHANGED  (AX),AND FLAGS
;
;******************************************************************************

KEY PROC FAR 
        STI                            ;INTERRUPTS ON
        PUSH    DX
        PUSH    SI                     ;SAVE REGISTERS
        PUSH    DS
        MOV     SI,DATA
        MOV     DS,SI                  ;LOAD DATA SEGMENT
        CMP     AH,0
        JE      J0                     ;JUMP IF (AH)=0
        CMP     AH,1
        JE      J3                     ;JUMP IF (AH)=1
        CMP     AH,2
        JE      J4                     ;JUMP IF (AH)=2
        JMP     J5                     ;EXIT

;
;(AH)=0 GET CHARACTER
;
;ALLOW INTERRUPS TO OCCUR

J0:     STI                            ;INTERRUPTS ON
        NOP
        CLI                            ;INTERRUPTS OFF

;FIND OUT IF THERE IS ANYTHING IN THE BUFFER

        MOV     SI,OUT_BUFFER
        CMP     SI,IN_BUFFER           ;BUFFER EMPTY ?
        JNE     J1                     ;NO, JUMP

;WAIT FOR KEYBOARD

        MOV     AX,9002H
        INT     15H

;UPDATE LEDS

        XOR     DL,DL                  ;CLEAR EOI FLAG
        CALL    UPDATE_LED             ;UPDATE LEDS
        JMP     J0

;GET THE CHARACTER FROM THE BUFFER

J1:     MOV     AX,[SI]                ;BUFFER TO (AX)
        ADD     SI,2                                    
        CMP     SI,KEY_BUFFER_END      ;END OF BUFFER ?
        JNE     J2                     ;JUMP IF NOT BUFFER END
        MOV     SI,KEY_BUFFER_START
J2:     MOV     OUT_BUFFER,SI          ;UPDATE POINTER
        JMP     J5                     ;EXIT

;
;(AH)=1 GET STATUS AND CHARACTER IF IT IS AVAILABLE
;

J3:     XOR     DL,DL                  ;CLEAR EOI FLAG
        CALL    UPDATE_LED             ;UPDATE LED INDICATORS
        CLI
        MOV     SI,OUT_BUFFER
        CMP     SI,IN_BUFFER           ;ZF = 1 IF BUFFER EMPTY
        MOV     AX,[SI]                ;BUFFER TO (AX)
        STI                            ;INTERRUPTS ON
        POP     DS                     ;RESTORE REGISTERS
        POP     SI
        POP     DX
        RET     2                      ;THROW AWAY OLD FLAGS, AND RETURN


;
;(AH)=2 GET SHIFT FLAG
;

J4:     MOV     AL,FLAG_1              ;STATUS TO (AL)

;RETURN    

J5:     POP     DS                     ;RECOVER REGISTERS
        POP     SI
        POP     DX
        IRET                           ;RETURN
KEY ENDP

;
;SUBROUTINE TO CHECK FOR KEYPAD KEY (CY=0 FOR KEYPAD CHARACTER)
;

Q51:    CMP     AL,71                  ;KEY < 71 ?
        JB      Q52                    ;YES, JUMP
        CMP     AL,83                  ;KEY > 82 ?
        JA      Q52                    ;YES, JUMP
        CLC                            ;CY = 0 FOR KEYPAD CHARACTER
        RET
Q52:    STC                            ;CY = 1 FOR NON KEYPAD CHARACTER
        RET



;
;SUBROUTINE TO SEND EOI AND ENABLE THE KEYBOARD
;

Q47:    TEST    DL,1                   ;EOI FLAG SET ?
        JZ      Q48                    ;NO, JUMP
        CLI
        MOV     AL,EOI
        OUT     INT1,AL       
        XOR     DL,DL                  ;CLEAR EOI FLAG

;ENTRY POINT TO ENABLE KEYBOARD

Q48:    MOV     AH,2
        CALL    WAIT_UPI               ;WAIT FOR INPUT BUFFER EMPTY
        MOV     AL,0AEH 
        OUT     UPI+4,AL               ;ENABLE KEYBOARD
        STI
        RET

page


;******************************************************************************
;
;     KEYBOARD ROUTINE
;     GET A CHARACTER FROM THE KEYBOARD AND INTERPET IT
;
;     BIT      FLAG_1               FLAG_2              FLAG_3
;     ---   ---------------     ----------------     ----------------
;      0    RSHFT DEPRESSED         ----             SCRL LED
;      1    LSHFT DEPRESSED         ----             NUM  LED
;      2    CTRL DEPRESSED      SYS REQ DEPRESSED    CAPS LED
;      3    ALT DEPRESSED       HOLD STATE             ----  
;      4    SCRL TOGGLED        SCRL DEPRESSED       ACK FROM KEYBOARD
;      5    NUM TOGGLED         NUM DEPRESSED        RESEND FROM KEYBOARD
;      6    CAPS TOGGLED        CAPS DEPRESSED       LED UPDATE IN PROGRESS
;      7    INS TOGGLED         INS DEPRESSED        KEYBOARD ERROR
;
;
;******************************************************************************

	org	08ch
	
KEYBOARD PROC NEAR

        STI
        PUSH    AX
        PUSH    BX
        PUSH    DI
        PUSH    DS
        PUSH    DX
        MOV     AX,DATA
        MOV     DS,AX

;
;DISABLE THE KEYBOARD
;
        MOV     AH,2 
        CALL    WAIT_UPI               ;WAIT FOR INPUT BUFFER TO BE EMPTY
        MOV     AL,0ADH
        OUT     UPI+4,AL               ;SEND DISABLE COMMAND

;
;GET THE CHARACTER FROM THE KEYBOARD
;

        IN      AL,UPI                 ;AL = SCAN CODE
        MOV     AH,AL                  ;AH = SCAN CODE
        MOV     DL,1                   ;SET EOI FLAG
        STI

;
;CHECK FOR OVERRUN
;

        CMP     AL,0FFH                ;OVERRUN ?
        JNE     Q1                     ;NO, JUMP
        JMP     Q44                    ;EXIT                    

;
;PROCESS RESEND
;

Q1:     CMP     AL,0FEH                ;RESEND ?
        JNE     Q2                     ;NO, JUMP
        OR      FLAG_3,20H             ;SET RESEND
        JMP     Q45                    ;EXIT

;
;PROCESS ACK FROM KEYBOARD
;

Q2:     CMP     AL,0FAH                ;ACK ?
        JNE     Q3                     ;NO, JUMP
        OR      FLAG_3,10H             ;SET ACK
        JMP     Q45                    ;EXIT

;
;UPDATE LED INDICATORS  
;

Q3:     CALL    UPDATE_LED

;
;CHECK FOR ILLEGAL SCAN CODE
;
        MOV     BL,AL 
        AND     BL,7FH                 ;MASK OFF BREAK BIT
        CMP     BL,55H                 ;ILLEGAL SCAN CODE ?
        JB      Q4                     ;NO, JUMP
        JMP     Q45                    ;EXIT

;
;PROCESS ALT DEPRESSED                                                 
;                                        

Q4:     CMP     AL,38H                 ;ALT DEPRESSED ?
        JNE     Q5                     ;NO, JUMP
        OR      FLAG_1,8               ;SET FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS ALT RELEASED
;

Q5:     CMP     AL,0B8H                ;ALT RELAESED ?
        JNE     Q7                     ;NO, JUMP
        AND     FLAG_1,0F7H            ;CLEAR FLAG
        MOV     AL,ALT_BUFFER
        MOV     ALT_BUFFER,0           ;CLEAR ALT BUFFER
        XOR     AH,AH
        CMP     AL,0                   ;ANYTHING IN ALT BUFFER ?
        JNE     Q6                     ;YES, JUMP
        JMP     Q45                    ;EXIT
Q6:     JMP     Q42                    ;PUT IT IN THE BUFFER

;
;PROCESS CTRL DEPRESSED
;

Q7:     CMP     AL,1DH                 ;CTRL DEPRESSED ?
        JNE     Q8                     ;NO, JUMP
        OR      FLAG_1,4               ;SET FLAG
        JMP     Q45                    ;EXIT 

;
;PROCOCESS CTRL RELEASED 
;

Q8:     CMP     AL,9DH                 ;CTRL RELEASED ?
        JNE     Q9                     ;NO, JUMP
        AND     FLAG_1,0FBH            ;CLEAR FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS LEFT SHIFT DEPRESSED
;

Q9:     CMP     AL,2AH                 ;LEFT SHIFT DEPRESSED ?
        JNE     Q10                    ;NO, JUMP
        OR      FLAG_1,2               ;SET FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS LEFT SHIFT RELEASED
;

Q10:    CMP     AL,0AAH                ;LEFT SHIFT RELEASED ?
        JNE     Q11                    ;NO, JUMP
        AND     FLAG_1,0FDH            ;CLEAR FLAG 
        JMP     Q45                    ;EXIT

;
;PROCESS RIGHT SHIFT DEPRESSED
;

Q11:    CMP     AL,036H                ;RIGHT SHIFT DEPRESSED ?
        JNE     Q12                    ;NO, JUMP
        OR      FLAG_1,1               ;SET FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS RIGHT SHIFT RELEASED 
;

Q12:    CMP     AL,0B6H                ;RIGHT SHIFT RELAESED ?
        JNE     Q13                    ;NO, JUMP
        AND     FLAG_1,0FEH            ;CLEAR FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS SYS REQ DEPRESSED
;

Q13:    CMP     AL,54H                 ;SYS REQ DEPRESSED ?
        JNE     Q15                    ;NO, JUMP
        TEST    FLAG_2,4               ;FLAG ALREADY SET ?
        JZ      Q14                    ;NO, JUMP 
        JMP     Q45                    ;EXIT
Q14:    OR      FLAG_2,4               ;SET FLAG
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     AX,8500H
        STI
        INT     15H                                                            
        JMP     Q46                    ;EXIT

;
;PROCESS SYS REQ RELEASED
;

Q15:    CMP     AL,0D4H                ;SYS REQ RELEASED ?
        JNE     Q16                    ;NO, JUMP
        AND     FLAG_2,0FBH            ;CLEAR FLAG 
        MOV     AL,2
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     AX,8501H                                      
        STI
        INT     15H
        JMP     Q46                    ;EXIT

;
;PROCESS INS DEPRESSED
;

Q16:    CMP     AL,52H                 ;INS DEPRESSED ?
        JNE     Q17                    ;NO, JUMP
;       TEST    FLAG_1,20H             ;NUM LOCK ?
 	test	flag_1,2fh	;test for numlock, cntrl, alt, LR shift
        JNZ     Q17                    ;YES, JUMP
        MOV     AL,80H
        CALL    Q49                    ;SET AND TOGGLE FLAG      
        MOV     AX,5200H                                         
        JMP     Q42                    ;CHARACTER TO BUFFER

;
;PROCESS INS RELEASED
;

Q17:    CMP     AL,0D2H                ;INS RELEASED ?
        JNE     Q18                    ;NO, JUMP 
        AND     FLAG_2,7FH             ;CLEAR FLAG 
        JMP     Q45                    ;EXIT                      

;
;PROCESS CAP LOCK DEPRESSED
;

Q18:    CMP     AL,03AH                ;CAP LOCK DEPRESSED ?
        JNE     Q19                    ;NO ,JUMP
    	test	flag_1,4	        ;check for cntrl flag
	jz	Q18_1	        ;no cntrl 
	jmp	Q29	        ;yes, then do not change state
Q18_1:
        MOV     AL,40H
        CALL    Q49                    ;SET AND TOGGLE FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS CAP LOCK RELEASED  
;

Q19:    CMP     AL,0BAH                ;CAP LOCK RELEASED ?
        JNE     Q20                    ;NO, JUMP
        AND     FLAG_2,0BFH            ;CLEAR FLAG
Q19_1:  JMP     Q45                    ;EXIT

;
;PROCESS NUM LOCK DEPRESSED
;

Q20:    CMP     AL,045H                ;NUM LOCK DEPRESSED ?
        JNE     Q24                    ;NO, JUMP
        TEST    FLAG_1,4               ;CTL DEPRESSED ?
        JZ      Q23                    ;NO, PROCESS NUM LOCK FLAGS

	test	flag_2,8		;see if already in hold state
	jnz	Q19_1			;in hold state, exit

;PROCESS PAUSE

        OR      FLAG_2,8H              ;SET PAUSE
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        CMP     VIDEO_MODE,7           ;MONOCHROME ?
        JE      Q22                    ;YES, SKIP TURNING ON CRT
        PUSH    DX
        MOV     DX,3D8H                ;VIDEO ADAPTER ADDRESS
        MOV     AL,VIDEO_MODE_REG
        OUT     DX,AL                  ;TURN ON CRT
        POP     DX
Q22:    TEST    FLAG_2,8H              ;PAUSE ?
        JNZ     Q22                    ;YES, KEEP CHECKING
        JMP     Q46                    ;EXIT

;PROCESS NUM LOCK FLAGS

Q23:    MOV     AL,20H
        CALL    Q49                    ;SET AND TOGGLE FLAG       
        JMP     Q45                    ;EXIT

;
;PROCESS NUM LOCK RELEASED
;

Q24:    CMP     AL,0C5H                ;NUM LOCK RELAESED ?                  
        JNE     Q25                    ;NO, JUMP
        AND     FLAG_2,0DFH            ;CLEAR FLAG  
        JMP     Q45                    ;EXIT                  

;
;PROCESS SCROLL DEPRESSED
;

Q25:    CMP     AL,46H                 ;SCROLL DEPRESSED ?                 
        JNE     Q27                    ;NO, JUMP
        TEST    FLAG_1,4               ;CTRL MODE ?
        JNZ     Q26                    ;YES, JUMP                 
        MOV     AL,10H
        CALL    Q49                    ;TOGGLE AND SET FLAG
        JMP     Q45                    ;EXIT

;PROCESS BREAK

Q26:
	test	flag_2,8		;check for hold state
	jnz	Q29_1			;yes, then unhold it    
	mov	di,key_buffer_start
	mov	in_buffer,di			;fixed CNTRL Break problem
        MOV     OUT_BUFFER,DI
        MOV     byte ptr KEY_BUFFER,80H         ;SET BREAK FLAG
        CALL    Q48                    ;ENABLE KEYBOARD
        INT     1BH                    ;BREAK INTERRUPT
        XOR     AX,AX                  ;CODE
;       JMP     Q45                    ;PUT IT IN THE BUFFER
	jmp	Q42		       ;put it in the buffer



;
;PROCESS SCROLL RELEASED
;

Q27:    CMP     AL,0C6H                ;SCROLL RELEASED ?
        JNE     Q28                    ;NO, JUMP
        AND     FLAG_2,0EFH            ;CLEAR FLAG
        JMP     Q45                    ;EXIT

;
;PROCESS RESET
;

Q28:    TEST    FLAG_1,4               ;CONTROL DEPRESSED ?
        JZ      Q29                    ;NO, JUMP
        TEST    FLAG_1,8               ;ALT DEPRESSED ? 
        JZ      Q29                    ;NO, JUMP
        CMP     AL,53H                 ;DEL DEPRESSED ?
        JNE     Q29_A    		;NO, JUMP
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     RESET_KEY,1234H
        XOR     AH,AH 
        JMP     test_1			;GO TO beginning of power on test
Q29_A:
	cmp	al,1			;see if ESC
	jne	Q29			;no
	call	Q47			;send EOI to keyboard
	jmp	config_setup		;

;
;PROCESS EXIT PAUSE
;

Q29:	TEST    FLAG_2,8               ;WAIT ?
        	JZ      Q30                    ;NO, JUMP
	test	al,80h		       
	jnz	Q30
Q29_1:
        AND     FLAG_2,0F7H            ;CLEAR FLAG  
        JMP     Q45                    ;EXIT

;
;PROCESS PRINT SCREEN         
;

Q30:    TEST    FLAG_1,3               ;LEFT OR RIGHT SHIFT ?
        JZ      Q31                    ;NO, JUMP
        CMP     AL,37H                 ;PTR SCREEN DEPRESSED ?
        JNE     Q31                    ;NO, JUMP 
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        INT     5                      ;PRINT SCREEN
        JMP     Q46                    ;EXIT

;
;PROCESS NORMAL KEY RELEASE
;

Q31:    TEST    AL,80H                 ;KEY RELEASED ?
        JZ      Q32                    ;NO, JUMP 
        JMP     Q45                    ;EXIT

;
;PROCESS ALT MODE, KEYPAD CHARACTER
;

Q32:    TEST    FLAG_1,8               ;ALT MODE ?     
        JZ      Q36                    ;NO, JUMP
        CALL    Q51                    ;KEYPAD CHARACTER ?
        JC      Q34                    ;NO, JUMP    

;XLAT KEYPAD SCAN CODE 

        SUB     AL,71        
        MOV     BX,OFFSET KEY_TABLE_1
        XLAT    byte ptr CS:KEY_TABLE_1
        MOV     BL,AL
        CMP     AL,-1                  ;NUMBER KEY ?
        JNE     Q33                    ;YES, JUMP
	mov	alt_buffer,0		;zero out alt key buffer
        JMP     Q45                    ;EXIT

;GENERATE NEW ALT BUFFER VALUE

Q33:    MOV     AL,ALT_BUFFER
        MOV     AH,10
        MUL     AH
        ADD     AL,BL
        MOV     ALT_BUFFER,AL
        JMP     Q45                    ;EXIT

;
;PROCESS NORMAL KEY ALT MODE
;
Q34:	MOV     BX,OFFSET KEY_TABLE_2
        XLAT    byte ptr CS:KEY_TABLE_2
        MOV     AH,AL
        XOR     AL,AL
        CMP     AH,39H                 ;SPACE KEY ?
        JNE     Q35                    ;NO, JUMP
        MOV     AL,20H
	jmp	short Q36_1		;ALT+SPACE
Q35:    mov	alt_buffer,0		;zero out alt key buffer
	JMP     short Q36_1             ;CHARACTER TO BUFFER

;
;PROCESS NORMAL KEY CTRL MODE
;

Q36:    TEST    FLAG_1,4               ;CTRL MODE ?
        JZ      Q37                    ;NO, JUMP

	mov	ah,al			; save scan code
        MOV     BX,OFFSET KEY_TABLE_3
        XLAT    byte ptr CS:KEY_TABLE_3
	xchg	al,ah			;al=scan code, ah=value from table
;	MOV     AH,AL                  ;AH = PSEUDO SCAN CODE
        MOV     BX,OFFSET KEY_TABLE_4
        XLAT    byte ptr CS:KEY_TABLE_4 ;AL = CODE
Q36_1:  JMP     short Q42              ;CHARACTER TO BUFFER

;
;PROCESS NORMAL KEY WITH NO LEFT OR RIGHT SHIFT                 
;

Q37:    TEST    FLAG_1,3H              ;SHIFT ?
        JNZ     Q40                    ;YES, JUMP

;HANDLE KEYPAD CHARACTERS

        CALL    Q51                    ;KEYPAD CHARACTER ?
        JC      Q38                    ;NO, JUMP
        TEST    FLAG_1,20H             ;NUM LOCK ?
        JNZ     Q41                    ;YES, JUMP

;XLATE CHARACTER

Q38:    MOV     BX,OFFSET KEY_TABLE_5
        XLAT    byte ptr CS:KEY_TABLE_5         ;AL = CODE

	cmp	ah,52h			;is it insert key
	jne	Q38_1			;no
	push	ax			;
	mov	al,80h			;toggle flag
	call	Q49			;
	pop	ax

Q38_1:

;HANDLE LETTERS IN CAP LOCK MODE

        CMP     AL,"a"                 ;CHARACTER < "a"        
        JL      Q39                    ;YES, JUMP               
        CMP     AL,"z"                 ;CHARACTER > "z"     
        JA      Q39                    ;YES, JUMP
        TEST    FLAG_1,40H             ;CAPS ?
        JZ      Q39                    ;NO, JUMP
        AND     AL,0DFH                ;CONVER LOWER CASE TO UPPER CASE
Q39:    JMP     Q42                    ;CHARACTER TO BUFFER

;       
;PROCESS KEY WITH LEFT OR RIGHT SHIFT
;
;HANDLE KEYPAD CHARACTERS

Q40:    CALL    Q51                    ;KEYPAD CHARACTER
        JC      Q41                    ;NO, JUMP
        TEST    FLAG_1,20H             ;NUM LOCK ?
        JNZ     Q38                    ;YES, JUMP  

;XLATE CHARACTER

Q41:    MOV     BX,OFFSET KEY_TABLE_6
        XLAT    byte ptr CS:KEY_TABLE_6
        XCHG    AH,AL                                  
        MOV     BX,OFFSET KEY_TABLE_7
        XLAT    byte ptr CS:KEY_TABLE_7

;HANDLE LETTERS IN CAP LOCK MODE

        CMP     AL,"A"                 ;CHARACTER < "A" ?       
        JB      Q42                    ;YES, JUMP                   
        CMP     AL,"Z"                 ;CHARACTER > "Z" ?
        JA      Q42                    ;YES, JUMP
        TEST    FLAG_1,40H             ;CAPS
        JZ      Q42                    ;NO, JUMP
        OR      AL,20H                 ;CONVERT UPPER TO LOWER CASE

;                  
;CHARACTER TO BUFFER
;

Q42:    CMP     AL,-1                  ;ILLEGAL CHARACTER ?
        JE      Q45                    ;YES, JUMP                      
        CMP     AH,-1                  ;ILLEGAL CHARACTER ?
        JE      Q45                    ;YES, JUMP                

;UPDATE BUFFER       

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
        CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD
        MOV     AX,9102H
        INT     15H      
        JMP     Q46                    ;EXIT

;ERROR, SEND BEEP TO SPEAKER

Q44:    STI
        MOV     BL,80                  ;BEEP LENGTH
        CALL    BEEP

;
;EXIT
;

Q45:    CALL    Q47                    ;SEND EOI AND ENABLE KEYBOARD 
Q46:    POP     DX
        POP     DS
        POP     DI
        POP     BX
        POP     AX
        IRET

;
;SUBROUTINE TO SET AND TOGGLE FLAG
;

Q49:    CLI
        TEST    FLAG_2,AL              ;KEY DEPRESSED ?
        JNZ     Q50                    ;YES, RETURN
        OR      FLAG_2,AL              ;SET FLAG
        XOR     FLAG_1,AL              ;TOGGLE FLAG
        STI
        CALL    UPDATE_LED             ;UPDATE LEDS
Q50:    RET

page

;******************************************************************************
;
;     SUBROUTINE TO UPDATE THE KEYBOARD LEDS IF NECESSARY
;
;     ENTER: DL = 1 SEND EOI TO INTERRUPT CONTROLLER
;            DL = 0 SKIP SENDING EOI
;       
;******************************************************************************

UPDATE_LED PROC NEAR

;
;CHECK FOR CHANGE IN LED INDICATORS

        CLI
        PUSH    AX
        PUSH    CX
        MOV     AL,FLAG_1
        MOV     CL,4
        SHR     AL,CL
        AND     AL,07H 
        MOV     CL,AL                  ;SHIFT STATE IN BITS 0-2  
        MOV     AH,FLAG_3
        AND     AH,07H                 ;LED BITS IN BITS 0-2
        XOR     AL,AH                  ;ANY CHANGE ?
        JZ      LED3                   ;NO, EXIT

;TEST FOR UPDATE IN PROGRESS

        TEST    FLAG_3,40H             ;UPDATE IN PROGRESS ?
        JNZ     LED3                   ;YES, EXIT
        OR      FLAG_3,40H             ;SET BUSY

;SEND EOI IF NEEDED

        TEST    DL,1                   ;EOI FLAG ?
        JZ      LED1                   ;NO, JUMP
        MOV     AL,EOI
        OUT     INT1,AL                ;SEND EOI
        XOR     DL,DL                  ;CLEAR EOI FLAG

;UPDATE LEDS IN KEYBOARD

LED1:   MOV     AL,0EDH 
        CALL    LED4                   ;SEND LED COMMAND TO KEYBOARD
        JNZ     LED2                   ;JUMP IF ERROR
        MOV     AL,CL                  ;CL = LED BITS 
        CALL    LED4                   ;SEND LED BITS
        JZ      LED0                   ;JUMP IF NO ERROR
LED2:   MOV     AL,0F4H 
        CALL    LED4                   ;SEND ENABLE TO KEYBOARD

;UPDATE THE LED BITS IN FLAG_3  

        CLI
LED0:   AND     FLAG_3,038H            ;CLEAR THE ERROR, LED BITS AND BUSY
        OR      FLAG_3,CL              ;SET THE LED BITS

;RETURN

LED3:   STI
        POP     CX
        POP     AX
        RET

;
;SUBROUTINE TO TRANSMITT TO THE KEYBOARD 
;
;SEND BYTE TO KEYBOARD

LED4:   PUSH    CX
	mov	bl,3
led41:	cli
	AND     FLAG_3,0CFH            ;CLEAR ACK, AND RESEND FLAGS
        PUSH    AX
        MOV     AH,2
        CALL    WAIT_UPI               ;WAIT FOR INPUT BUFFER
        POP     AX
        OUT     UPI,AL                 ;SEND BYTE TO KEYBOARD 

;WAIT FOR KEYBOARD TO RESPOND

        STI
LED5:   MOV     CX,01A00H              ;10 MSEC WAIT FOR KEYBOARD RESPONCE
LED6:   TEST    FLAG_3,10H             ;ACK ?               
        JNZ     LED8                   ;YES, EXIT 
        TEST    FLAG_3,20H             ;RESEND ?
        JNZ     LED7                   ;YES, TRY AGAIN
        LOOP    LED6
LED7:   DEC     BL                     ;DECREMENT RETRY COUNT
        JNZ     LED41                  ;JUMP IF RETRY NOT ZERO
        OR      FLAG_3,80H             ;SET ERROR
LED8:   TEST    FLAG_3,80H             ;TEST FOR ERROR
        POP     CX
        RET

;
UPDATE_LED ENDP

keyboard	endp


page

;*****************Please Take note of this ***********************
;
;	Size of this module must be 0443h
;

empty_space	db	12 dup (0)

end_of_int16	label	byte

	if	(end_of_int16 - start_of_int16)

;	error

	endif

eproma	ends
	end




