include	title.inc
subttl	Power On Diagnostics (CBM-BSW)

;------------------------------------------------------------------------------
;
;		POWER ON DIAGNOSTICS
;
; Rev	who	when	   comments
;
; 1.00	llj	12/01/84   Initially coded
; 1.01	rlm	01/02/85   Generated this module
; 1.02	rlm	01/09/85   added messages to this module
; 1.03	rlm	01/22/85   Removed some msgs to fix module size
; 1.10	rlm	02/15/85   resetkey flag etc.
;		^----- 2.0 Release -----^
;
; 1.20	rlm	02/25/85   A20 line in test_26
; 1.20	rlm	02/26/85   fixed TEST-14 keyboard err
;			   also fix ROM scan routine test_21
; 1.22	rlm	03/05/85   fixed 8259 int in test3
; 1.23	rlm	03/05/85   fixed UPI in test_9
;		^----- 2.20 Release -----^
;
; 1.30	rlm	3/11/85	   dated minor changes
; 1.31	rlm	3/21/85	   fixed Power Up hang in test_9
;		^----- 2.40 Release -----^
;
; 1.40	rlm	4/8/85	   version #not displayed
;		^----- 2.50 Release -----^
;
; 1.50	rlm     4/19/85    ECN # 047
;		^----- 2.52 Release 5/7/85 -----^
;
; 2.53	jwy	6/7/85	   1. Don't turn off interrupt on reset
;			   2. Do checksum on ROM BIOS after checking the shutdown byte
;	    	   	   3. Initialize interrupt ctrl before accessing IMR (Init Video)
;			   4. Set the system flag in 8042 before using the shutdown
;			   5. Don't set the system flag in "shut" routine
;			   6. Disable the NMI on system reset
;
; 2.54	jwy	6/13/85	   1. Fixed the # of diskette on POD (POD)
;			   2. Fixed the expansion memory on POD (POD)
;			   3. Fixed VDISK/E (INT 15H)
;			   4. Fixed LED problem (INT 9H) 
;
; 		^--- ECN # 049  2.54 Release 06135 ---^
;
; 2.55	jwy	6/27/85    1. Fixed the LED problem at warm boot (POD) - Do the 
;			      keyboard reset when ctrl-alt-del
;			   2. Fixed intermittent problem with parallel port
;			      ( # of parallel port) (POD)
;			   3. Changed error checking code in prt_scr 
;			      (INT 5H) (25H to 29H)
;			   4. Fixed the Ctrl-PrtSc key problem (TOP8K)
;			      - scan code table was wrong for that key
;			   5. Fixed the scan code of Ctrl-Func Key
;			      (INT 9H)
;
; 		^--- ECN # 052  2.55 Release 07015 or 06275 ---^
;
; 2.56 	jwy	8/28/85	   1. Implement PC DOS 3.1 patch (INT 13)
;			      - Disable INT during REP INSW 
;			   2. System hanged when disk reset command (INT 13)
;			      - Save the # of disk (BL) & drive # (DL) 
;			   3. Turn the motor on again if it hasn't enough time
;			      to reach to stable status (INT 40)
;			   4. Parallel printer printed the garbage or missed the char 
;			      with 8 MHz clock (INT 17) - Put some delay during STROBE 
;			   5. Fixed Cherry keyboard problem (POD) - Enable and
;			      then disable the kb before sending out the reset cmd
;			   6. Remove CLI from "READ_CMOS", "WRITE_CMOS" (ROUTINES)
;			   7. HP Fixed disk controller board problem - If there
;			      is no fixed disk, INT 13H have to point the floppy_io (POD)
;			   8. Run test_26 if memory on the mother board is
;			      512k or more instead of 640k or more (POD)
;			   9. Main routines - change the drive code
;			      comparison from 81 to 81h (int 13h)
;			  10. On error change the error code from 86 to 86h (int 15h)
;			  11. Put Board ID code in Loc 3 (POD)
;			      - A-Tease  = 0ffh
;			      - BUS AT   = 0fdh
;			      - MICRO AT = 0feh
;			  12. Service the on-board ROM at E000:0000 (POD)
;			  13. Fixed the problem with Paradise video board (POD)
;			      - Setup int 11h (equipment_determination) vector
;				before servicing the video ROM
;			      - Set equip_flag 0 if video ROM found
;			  14. Fixed the problem with FE diskette controller board (MISCINTR)
;			      - Set drive code when trying to boot from diskette
;			  15. Fixed the problem with TOPVIEW (INT 10)
;			      - func 10,11 & 12h was incorrect
;			  16. It takes long time to boot from disk 
;			      - if error code from diskette is time out, don't retry
;				the diskette again
;			  17. Fixed Superkey problem (POD)
;			      - Setup KEY_BUFFER_END (40:82) correctly
;			  18. Fixed the problem with Framework 'Scroll Lock' (INT 16)
;			      - Some bugs in handling LED routines
;			  19. For some diskless workstation, before displaying error msg,
;			      call int 18h when disk or diskette boot failure. (MISCINTR)
;
; 		^--- ECN # 059  2.56 Release 08285 ---^
;
;
; 2.57	 rlm  11/05/85	  1. Fixed Insert Key problem in INT16
;			  2. New algorithm for 80287 testing
;			  3. Fixed INT40 for function range checking
;			  4. Do Not turn off hold state if break key is detected  INT16
;		 	  5. Fixed INT15 for function range checking
;			  6. Fixed LED4 handling routine in INT16
;			  7. Remove infinite loop from Read_switch routine
;			  8. Check Convert routine for invalid BCD input
;			  9. Disregard NUM Lock key while in hold state INT16
;			 10. Fixed CNTRL Break problem in INT16
;			 11. Fixed CNTRL+CAPS LOCK problem in INT16
;			 12. Fixed ALT+Numeric key problem in INT16 
;
; 		^--- ECN # 067  2.57 Release 11225 ---^
;
;
; 2.58	 rlm  12/19/85	1. New INT13 Routines
;
; 		^--- ECN #      2.58 Release 12195 ---^
;
;
; 2.59	 rj    7/7/86   1. Fix invalid function call in int40  DOS 3.2 problem
;			2. Copy CMOS mem config in RAM if unequal after 
;			    alt/cntl/del.  Topview problem
;			3. Add extra int7 transition in parallel loopback for 
;			   FE2000/A. ATease only
;
; 		^--- ECN #      2.59 Release 08016 ---^
;
; 2.60	rlm  9/23/86	1. Fix Timer test for 3010 chip
;			2. Increment timing loop in test_24 (Tektronics)
;			3. Initialize Interrupt controller 2 properly. Fixed
;			   problem with external interrupt on IRQ9
;
; 		^--- ECN #      1.10 release for Bus-AT--12/16/86---^
;
; 3.0   Torsten Burgdorff   	Changes were made to run Micro-AT with Faradays
;       2/11/87		FE3010 in AMIGA2000 slot system:
;
;			1. Add diagnostic output on printer port  
;	                    2. Add "Refresh going"-flag
;			3. Enable Amiga interrupts 
;
;		^--- CBM/BSW	3.0 release  
;
; 3.1   tb   2/26/87		1. Add sign on message in TOP8K module
;			2. Update version number
;			3. Add "fake RJ" routine
;						 
;		^--- CBM/BSW	3.1 release for AT-Emulator  2/27/87
;
; 3.2   tb   5/19/87		1. Move "Janus Initialisation" to earlier
;			   execution point
;			2. Change printer initialisation sequence
;			 
;		^--- CBM/BSW	3.2 release for AT-Emulator  5/26/87
;
; 3.3   tb   7/7/87		1. Turn drive motors off
;			 
;		^--- CBM/BSW	3.3 release for AT-Emulator 11/26/87
;
; 3.4   tb   8/17/88		-  implement Faraday BIOS 4.01 update
;			1. POD: Timer1 test removed
; 			2. POD: Init user font table pointer to NULL
;			3. POD: don't destroy DS during EGA init
;			4. POD: Set CMOS reg 33 bit 7 if RAM > 512kB
;			5. POD: Skip manufacturing test
;			6. POD: Skip INT 1f setup
;			7. POD: Enable Slave Interrupt decoder
;			8. POD: Enable keyb interrupt after keyb.test
;			9. POD: Skip memory warning during warm boot
;		         10. POD: Enable NMI after shutdown 4
;		         11. POD: Load DS after parity test
;		         12. POD: Set PowerUpError flag	
;		         13. POD: Enable redirected IRQ2 and RTC
;		         14. POD: Enable keyb, timer + disk int
;			        after keyb init
;		         15. POD: Get keyb ID and set keyb ID regs 
;			        for DOS3.3 compatibility
;		         16. POD: Turn on cascade int for 80287 test	
;		         17. POD: Skip Clock setup	
;		         18. POD: Set HD type default = 0	
;		         19. POD: Set floppy drive = 1 HD drive
;		         20. INT15: add function C0h
;		         21. INT15: add error handling in function 83h
;		         22. INT15: change function 87h
;		         23. INT15: change function 88h
;		         24. INT15: change function 89h
;		         25. INT17: don't change data, when printer busy	
;		         26. INT17: change printer strobe timing
;
;		          -  INT40: allow boot from 1.44MB floppy drive
;			-  CSETUP: implement BIOS Setup Utility
;			-  TOP8K: change BIOS header
;							
;		^--- CBM/BSW	3.4 release for AT-Emulator 12/01/88
;
; 3.5   tb   12/9/88		-  TOP8K,CSETUP,INT13: more HD tables
;			-  TOP8K: change copyright
;			1. POD: init mono AND color mem
;			2. POD: don't use printer at $3BCh
;			3. POD: change boot sequence
;			4. POD: report address error in extended mem
;
;		^--- CBM/BSW	3.5 release for AT-Emulator 12/12/88
; 3.6	BK  10/10/89		Removed check for warm boot in shutdown
;				code to fix protected mode for smaltalk
;	    10/20/89		Moved the setting of ref_rdy into the wait
;				loop for 8088Go to fix the Autoconfig prob
;
;------------------------------------------------------------------------------

page

.286
.xlist

halt	macro
	if	SYSTEM_ID EQ A_TEASE
	hlt
	endif
	if	SYSTEM_ID EQ BUS_AT
	mov	al,0c4h
	out	UPI+4,al
	hlt
	endif
	if 	System_ID eq Micro_At
	hlt
	endif
	endm

include	equates.inc
include	externals.inc

extrn	flag_1:byte
extrn	flag_2:byte
extrn	flag_3:byte
extrn	flag_4:byte
extrn	video_parm_tbl:byte
extrn	hf_num:byte
extrn	special:byte
extrn	vflag:byte

extrn	equip_flag:word
extrn	rom_segment:word
extrn	rom_address:word
extrn	memory_size:word
extrn	reset_key:word
extrn	key_buffer_start:word
extrn	key_buffer_end:word
extrn	parallel_table:word
extrn	serial_base:word
extrn	in_buffer:word
extrn	out_buffer:word
extrn	timer_upper:word
extrn	timer_lower:word
extrn	vir_mem_size:word
extrn	wd_config_tbl:word

extrn	read_switch:near
extrn	tvideo:near
extrn	display:near
extrn	video:near
extrn	move:near
extrn	beep:near
extrn	int_ptr1:near
extrn	chk_clk:near
extrn	end_interrupt:near
extrn	int_ptr2:near
extrn	ccheck:near
extrn	BOOT_MSG_1:near
extrn	unexpected_interrupt:near
extrn	parity_error:near
extrn	screen_print:near
extrn	time:near
extrn	keyboard:near
extrn	floppy_intr:near
extrn	video_start:near
extrn	config:near
extrn	ram_size:near
extrn	floppy_io:near
extrn	wd_drv:near
extrn	serial_io:near
extrn	intr_15:near
extrn	key:near
extrn	print:near
extrn	boot:near
extrn	clock:near
extrn	break:near
;*extrn	disk_parm:near
extrn	floppy_parms:near
extrn	detrm_drv_type:near
extrn	alarm:near
extrn	int71:near
extrn	math:near
extrn	wait_upi:near
extrn	serial_time:near
extrn	recal_opn:near
extrn	print_time:near
extrn	key_buffer:near
extrn	readkey:near
;*extrn	floppy_setup:near
extrn	wd_setup:near
extrn	start_clock:near
extrn	update:near
extrn	convert:near
extrn	shutdown_9:near
extrn	ver_msg:near
extrn	shut:near
extrn	convt_bin_dec:near
extrn	wd_intr:near
extrn	gate_a20:near
extrn   	sign_on:near

public	start
public	test_1
public	end_pod
public	shut_down
.list

zero	segment	at 00h
zero	ends

dataa	segment	at 40h
dataa	ends

;------------------------------------------------------------------------------
;
; Synchronize AT- and AMIGA- power up  - set "refresh ready" flag on janus
;				       - clear "wake up 80286" flag on janus 
;				       - wait for setting this flag from AMIGA 
; Update: 2/10/87 TB		       - return, if flag set or after timeout
;
;-----------------------------------------------------------------------------
;
janus	SEGMENT AT 0d000h	        ; The following are relative
					; to Segment D000h
	       	org   0
ref_rdy        	label byte		; lock byte of janus_base
	       	org   1
wu80286	       	label byte		; pad0 of janus_base
 
janus	ENDS



eproma	segment	byte public

	assume	cs:eproma,ds:dataa,es:nothing,ss:nothing

.xlist
begin	equ	$+1805h

checksum	dw	0		;value of checksum

	db	SYSTEM_ID		;system id code from equates.inc
	db	0

.list

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;COPYRIGHT MESSAGE FOR PROMS:
;
promsg	db 'CCooppyyrriigghhtt  11998888  CCoommmmooddoorree'
	db '  BBuussiinneessss  MMaacchhiinneess  '
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;Boot-up message, contains rev level.
;
 	db '      Commodore A2286 Bios  Rev. '
;
;**** CHANGE VERSION NUMBER HERE ****
;
	db '3.5 Release  ToBu'


 	org	0A6h

start:
;                                        
;******************************************************************************
;
;     TEST_1:	PROCESSOR TEST
;     		CHECKS 8088 FLAGS, ALU AND REGISTERS
;         
;******************************************************************************

TEST_1:
       
;TEST # TO LEDS       
	cli
        MOV     AL,1			;LED #1

        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL

;
;       VERIFY FLAGS CAN SET AND RESET
;

TA0:    MOV     AL,0FFH                ;
        ADD     AL,1                   ;CF,AF,ZF,PF SHOULD SET
                                       ;SF,OF SHOULD RESET
        JNP     TA1                    ;JMP TO HALT IF PF NOT SET
        JNZ     TA1                    ;JMP TO HALT IF ZF NOT SET
        JNC     TA1                    ;JMP TO HALT IF CF NOT SET
        JS      TA1                    ;JMP TO HALT IF SF SET    
        JO      TA1                    ;JMP TO HALT IF OF SET
        ADD     AL,80H                 ;SF SHOULD SET
                                       ;CF,AF,ZF,PF SHOULD NOT SET
        JNS     TA1                    ;JMP TO HALT IF NOT SET
        JC      TA1                    ;JMP TO HALT IF CF SET
        JZ      TA1                    ;JMP TO HALT IF ZF SET
        JP      TA1                    ;JMP TO HALT IF PF SET 
        ADD     AL,80H                 ;OF SHOULD SET
        JO      TA2                    ;HALT IF OF NOT SET
TA1:    halt                            ;HALT DUE TO FLAGS FAILURE

;
;       VERIFY THAT ALU DOES NOT PICK OR DROP BITS
;      

TA2:    XOR     AL,AL                  ;SET AL TO ZERO
        XOR     AH,AH                  ;SET AH TO ZERO
        OR      AL,AH                  
        JNZ     TA3                    ;JMP TO HLT IF `OR' INST PICKED BITS
        XOR     AL,AH
        ADD     AL,AH                  ;JMP TO HLT IF `XOR', `ADD' INST
        JNZ     TA3                    ;PICKED ANY BITS
        MOV     AL,0FFH                ;SET AH TO ALL ONES
        OR      AH,AL
         XOR     AH,AL
        ADD     AH,AL
        SUB     AL,AH                  
        JNZ     TA3                    ;JMP TO HLT IF `OR',`XOR',`ADD'
                                       ;DROPPED ANY BITS
        XOR     AL,AH
        AND     AL,AH
        ADD     AL,AH                  ;SEE IF CARRY PROPAGTES
        CMP     AL,0FEH
        JZ      TA4
TA3:    halt                            ;HALT DUE TO ALU/REG FAILURE
	
;
;	TURN OFF INTERRUPTS AND PARITY CHECKS
;
; 5/28/85 jwy v2.53
;	MOV	AL,0FFH
;	OUT	INT1+1,AL

TA4:	MOV	AL,0CH
	OUT	PIO,AL
	mov 	dx,dor			; turn drive motor off (V3.3)
	out	dx,al

;******************************************************************************
;
;     TEST_2:	SHUTDOWN
;
;     THIS ROUTINE CHECKS TO SEE IF A SHUTDOWN HAS BEEN ISSUED, AND IF SO
;     BRANCHES TO THE PROPER ROUTINE.  THE BRANCH INFORMATION IS STORED
;     IN THE CMOS MEMORY.  A SHUTDOWN IS EXECUTED TO EXIT 
;     PROTECTED MODE.                                                   
;
;******************************************************************************

TEST_2:

;TEST # TO LEDS

        MOV     AL,2			;LED #2
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL


;CHECK FOR KEYBOARD RESET

        MOV     AX,DATA
        MOV     DS,AX
;        CMP     RESET_KEY,1234H        ;KEYBOARD RESET ?
;        JNE     M0                     ;NO, JUMP
;        JMP     TEST_7			;yes, bypass initialise diagnostics

;CHECK FOR SHUTDOWN                   
 
M0:     IN     AL,UPI+4
        TEST   AL,04H                  ;SHUTDOWN ?
        JNZ     M3                      ;yes, JUMP
	jmp	M2


;GET THE SHUTDOWN BYTE AND CLEAR IT IN CMOS MEMORY               

M3:
	mov	dx,08fh			;initialize DMA page register
	mov	al,0
	out	dx,al			;

        MOV    SP,top_of_stack    
        MOV    AX,stack_seg
        MOV    SS,AX

        MOV    AL,80h + SHUTDOWN
        CALL   READ_CMOS               ;GET BRANCH INFORMATION
        MOV    BL,AH                   ;SAVE IT
        XOR    AH,AH      
        MOV    AL,80h + SHUTDOWN
        CALL   WRITE_CMOS              ;CLEAR SHUTDOWN BYTE

        CMP    BL,0BH                  ;ILLEGAL SHUTDOWN ?
        JB     shutdown_ok		;no
	jmp	shutdown_b		;yes,halt

;
;	check for shutdown_9. If true then skip reinitialization of
;	the interrupt controllers
;

shutdown_ok:

	cmp	bl,9
	je	test_3_a		;

;

	mov	al,0			;
	out	1+coprocessor_clr,al	;
	mov	al,11h			;send ICW1 to first 8259
	out	int1,al			;
	jmp	$+2			; small delay
	out	int2,al			;and to second 8259
	jmp	$+2			;wait
	mov	al,8			;setup ICW2 for first 8259
	out	int1+1,al		;
	mov	al,70h			;setup ICW2 for second 8259
	out	int2+1,al		;and send it
	jmp	$+2			;wait
	mov	al,4			;setup ICW3 for first 8258
	out	int1+1,al		;
	shr	al,1			;setup ICW3 for second 8259
	jmp	$+2
	out	int2+1,al		;
	shr	al,1			;setup ICW4 for first 8259
	out	int1+1,al		;
	jmp	$+2
	out	int2+1,al		;same ICW4 for second 8259
	mov	al,-1			;mask off all interrupts
	out	int1+1,al		;
 	jmp	$+2
	out	int2+1,al		;


;SETUP STACK

test_3_a:
        CLI
;JUMP TO APPROPRIATE ROUTINE

        XOR    BH,BH
        SHL    BX,1                    ;ADJUST FOR WORD
        STI
        JMP    WORD PTR CS:[BX + OFFSET M1]

;BRANCH TABLE

M1 	LABEL WORD    

        DW OFFSET SHUTDOWN_0           ;0     MANUFACTURING TEST LOOP
        DW OFFSET SHUTDOWN_1           ;1     
        DW OFFSET SHUTDOWN_2           ;2     OVER 1 MBYTE TEST
        DW OFFSET SHUTDOWN_3           ;3
        DW OFFSET SHUTDOWN_4           ;4
        DW OFFSET SHUTDOWN_5           ;5
        DW OFFSET SHUTDOWN_6           ;6     UNEXPECTED
        DW OFFSET SHUTDOWN_7           ;7     UNEXPECTED
        DW OFFSET SHUTDOWN_8           ;8     UNEXPECTED
        DW OFFSET SHUTDOWN_9           ;9     BLOCK MOVE  (INT 15H)
        DW OFFSET SHUTDOWN_A           ;A
;
shutdown_0:
	jmp	test_8			;continue from test 8

shutdown_1:
	mov	al,0f1h
	jmp	shutdown_x

shutdown_4:
; ToBu (10) 18.8.88 Ver 3.4
	mov	al,0dh		;enable NMI
	call	read_cmos		;enable parity
	in	al,61h		;
	and	al,11110011b	;set I/O check bit
	out	61h,al		;	
	int	19h
	
shutdown_5:
	in	al,upi
	mov	al,eoi
	out	int1,al
	jmp	shutdown_A

shutdown_6:
	mov	al,0f6h
	jmp	short shutdown_x

shutdown_7:
 	mov	al,0f7h
	jmp	short shutdown_x

shutdown_8:
	mov	al,0f8h
	jmp	short shutdown_x

shutdown_A:
	jmp	dword ptr ds:rom_address
shutdown_B:
	mov	al,0fbh

shutdown_x:
	cli				;clear interrupts
	mov	dx,diag_port
;	not	al
	out	dx,al			;light up LEDs
	mov	si,offset ill_shut_msg	;send message to screen
	call	display			;
	nop

M2:      NOP

;
;       READ/WRITE 8088 REGISTERS. PROPAGATE 55, AA PATTERN THRU REGS
;
        MOV     AX,0AAAAH              ;SETUP AA PATTERN
TA5:    MOV     BX,AX                  ;MOVE PATTERN THRU ALL REGS
        MOV     DS,BX
        MOV     CX,DS
        MOV     SS,CX                  
        MOV     DX,SS
        MOV     ES,DX
        MOV     SP,ES 
        MOV     BP,SP
        MOV     SI,BP
        MOV     DI,SI
        CMP     AX,DI                  ;DID PATTERN PROPAGATE THRU ALL REGS?
        JNE     TB2                    ;NO - JMP TO HALT
        XOR     AX,0FFFFH              ;YES- FLIP PATTERN AND REPEAT TEST   
        JNS     TA5

	mov	al,80h			; disable the NMI interrupt
	out	CMOS,al


;******************************************************************************
;
;     TEST_3:	EPROM CHECKSUM TEST
;     		VERIFY CONTENTS OF EPROM BY ADDING ALL BYTES AND CHECKING
;     		IF THE RESULT IS ZERO. CHECKSUM TO MAKE SUM ZERO IS AT FFFF.
;      
;******************************************************************************

TEST_3:

;TEST # TO LEDS

        MOV     AL,3
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL


;CHECK LOW ADDRESS 

        XOR     AX,AX                  ;SET AL TO ZERO
        MOV     SI,0		       ;GET EPROM START OFFSET
        MOV     CX,4000h	       ;SETUP LOOP CNT 
TB1:    ADD     AX,CS:[SI]             ;ACCUMULATIVE ADD
 	ADD     SI,2                   ;MOVE TO NEXT ADDRESS
        LOOP    TB1                    ;REPEAT FOR ALL ADDRESSES
        AND    AX,AX                   ;DOES IT ALL ADD UP TO ZERO?
        JZ     TB3                     ;YES, GO TO NEXT TEST
;	NOP				

; 5/28/85 jwy v2.53
TB2:	halt

TB3:    NOP

page

;******************************************************************************
;
;     TEST_4:	TEST DMA PAGE REGISTERS
;
;******************************************************************************

;TEST # TO LEDS

        MOV     AL,4
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL


;WRITE LOOP

        MOV     BL,1                   ;TEST PATTERN
TP0:    MOV     CX,0FH
        MOV     DX,DMA_PAGE_REG     
        MOV     AL,BL
TP1:    OUT     DX,AL                  ;WRITE PATTERN
        INC     DX
        ROL     AL,1
        LOOP    TP1

;READ AND COMPARE LOOP

        MOV     CX,0FH
        MOV     DX,DMA_PAGE_REG     
        MOV     BH,BL
 TP2:    IN      AL,DX                  ;READ PATTERN
        CMP     AL,BH                  ;ERROR ?
        JE      TP3                    ;NO, CONTINUE
        halt                            ;FATAL ERROR, HALT
TP3:    ROL     BH,1
        INC     DX
        LOOP    TP2

;REPEAT TEST

        SHL     BL,1
        JNZ     TP0                    ;REPEAT TEST

;CLEAR DMA PAGE REGISTERS

        MOV     CX,0FH
        XOR     AL,AL
        MOV     DX,DMA_PAGE_REG
TP4:    OUT     DX,AL                  ;CLEAR PAGE
        INC     DX        
        LOOP    TP4                    ;LOOP TILL ALL CLEAR

page

;******************************************************************************
;
;    TEST_5: TEST THE 8253 TIMER AND MAKE SURE IT COUNTS
;    
;******************************************************************************

TEST_5:

;TEST # TO LEDS

        MOV     AL,5
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL

        CLI                            ;INTERRUPTS OFF
        MOV     AL,0FFH                                 
        OUT     INT1+1,AL               ;MASK OFF INTERRUPTS
        IN      AL,PIO                 ;READ CONTROL PORT
        OR      AL,1

        OUT     PIO,AL               ;ENABLE TIMER 2

 ;CHECK TIMER 0

        MOV     AL,020H   
        OUT     TIMER+3,AL             ;SELECT TIMER 0 FOR MODE 0
        MOV     AL,0FFH   
        jmp	short $+2
        OUT     TIMER,AL               ;WRITE HIGH BYTE
        MOV     CX,0FF00H
TE1:    jmp	short $+2
         IN      AL,TIMER               ;READ TIMER 0
        CMP     AL,0                   ;CHECK FOR END OF COUNT
        JE      TE2
        LOOP    TE1                    ;WAIT FOR TIMER
        halt                            ;ERROR, TIMER 0 NOT COUNTING   
TE2:    CMP     CX,01F00H
        JA      TE3
        halt                            ;ERROR, TIMER 0 TOO FAST
TE3:    CMP     CX,0EFFFH    
        JB      TE8
        halt                            ;ERROR, TIMER 0 TOO SLOW

;CHECK TIMER 1
; ToBu (1) 18.8.88 Ver 3.4 - Test removed due FE3010B timer problem
	
;TEST TIMER 2 

TE8:    MOV     AL,0A0H
        jmp	short $+2
        OUT     TIMER+3,AL             ;SELECT TIME 2 FOR MODE 0
        MOV     AL,0FFH
        jmp	short $+2
        OUT     TIMER+2,AL             ;WRITE LOW BYTE
        MOV     CX,0FF00H    
TE9:    jmp	short $+2		;ECN 0047
	nop
        IN      AL,TIMER+2
        CMP     AL,0
        JE      TE10
        LOOP    TE9
        halt                            ;ERROR, TIMER 2 DID NOT COUNT
TE10:   CMP     CX,01F00H
        JA      TE11
        halt                            ;ERROR, TIMER 2 TOO FAST
TE11:   CMP     CX,0EFFFH
        JB      TE12
        halt                            ;ERROR, TIMER 2 TOO SLOW
 TE12:   MOV     AL,0B6H                      
        jmp	short $+2
        OUT     TIMER+3,AL             ;SETUP TIMER 2

page

;******************************************************************************
;
;     TEST_6:	START MEMORY REFRESH
;     		TEST REFRESH
;
;	     	TIMER AND DMA CHIPS ARE SETUP TO CAUSE REFRESH CYCLES
;     		TO OCCUR EVERY 15.1 MICROSECONDS.
;
;******************************************************************************

TEST_6:

;TEST # TO LEDS

        MOV     AL,6
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL

;	write to a dummy memory location before starting refresh
;		ECN 0047

	mov	ax,data
	mov	ds,ax
	mov	vir_mem_size,ax

	
        MOV     AL,54H                 ;SELECT TIM 1, LSB, MODE 2, BINARY
        OUT     TIMER+3,AL             ;WRITE TIM 1 MODE REG
        MOV     AL,REFRESH_DIV         ;GET REFRESH DIVISOR
        JMP     TO1   
TO1:    OUT     TIMER+1,AL             ;STORE DIVISOR INTO TIM 1 COUNTER REG
        MOV     CX,0FFFH
TO2:    IN      AL,PIO
        TEST    AL,10H                 ;IS REFRESH OCCURING
        JNZ     TO3                    ;YES, JUMP
        LOOP    TO2
        halt
TO3:    NOP			       
				       ;Send "RefreshReady" signal to Amiga
	assume  ds:janus	       ; (V3.0)
	push 	ds
	mov  	ax,janus  
	mov  	ds,ax		       ; setup segment register
; done by AMIGA
;	mov  	wu80286,0ffh	       ; clear "wake up 80286"
	mov  	ax,50		       ; timeout value
sleep: 	mov 	cx,0
	mov  	ref_rdy,0	       ; set "refresh ready"
sleep_deep:
        or   	wu80286,0
	loopnz	sleep_deep
	dec  	ax
	jnz  	sleep
	pop  	ds
	assume 	ds:dataa
	jmp  	TEST_7


page
;******************************************************************************
;
;     TEST_7:      TEST THE UPI (8042) Keyboard Processor
;
;******************************************************************************

TEST_7:

;TEST # TO LEDS

        MOV     AL,7
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL


;
;MAKE SURE THE INPUT AND OUTPUT BUFFERS ARE EMPTY
        XOR     CX,CX
TBB0:   IN      AL,UPI+4               ;GET STATUS
        TEST    AL,3                   ;IN AND OUT BUFFER EMPTY ?
        JZ      TBB1                   ;YES, JUMP
        LOOP    TBB0                   ;TRY AGAIN
        halt                            ;ERROR HALT
;ISSUE DIAGNOSTIC COMMAND                                      
TBB1:   MOV     AL,0AAH
        OUT     UPI+4,AL               ;ISSUE A DIAGNOSTIC COMMAND
;WAIT FOR THE RESULTS
        XOR     CX,CX
TBB2:   IN      AL,UPI+4               ;GET STATUS
        TEST    AL,1                   ;INPUT BUFFER FULL ?
        JNZ     TBB3                   ;YES, JUMP
        LOOP    TBB2
        halt                            ;ERROR HALT          
;CHECK THE RESULTS
TBB3:   IN      AL,UPI                 ;GET TEST RESULTS
        CMP     AL,55H                 ;RESULTS OK ?
        JE      TBB4                   ;YES, JUMP
        halt                            ;ERROR HALT               
TBB4:   NOP

        MOV     AX,DATA
        MOV     DS,AX
        CMP     RESET_KEY,1234H        ;KEYBOARD RESET ?
        JNE     TEST_8			;no, do the memory test
        JMP     TG5			;yes, bypass memory test


                                      
;******************************************************************************
;
;     TEST_8:	TEST FIRST 128K OF RAM               
;     		WRITES, READS AND VERIFIES 128K RAM BLOCKS      
;     		FIRST PASS WRITES ADDRESS INTO DATA (NO INVERT).
;     		SECOND PASS WRITES COMPLEMENT OF ADDRESS INTO DATA (INVERT).
;     		CLEARS MEMORY AFTER TEST
;
;******************************************************************************

TEST_8:

;TEST # TO LEDS

        MOV     AL,8
        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL

;
        XOR     AX,AX
        MOV     DS,AX                  ;SET DS,ES TO ZERO
        MOV     ES,AX
TG0:    CLD                            ;SETUP DIR FLAG FOR AUTOINCR
        XOR     BX,BX                  ;SETUP PATTERN INVERT MASK

;       WRITE LOOP

 TG1:    MOV     CX,8000H               ;SETUP COUNT TO 64K BYTES
        XOR     DI,DI                  ;SETUP ADDRESS
TG2:    MOV     AX,BX                  ;GET PATTERN INVERT MASK
        XOR     AX,DI                  ;GENERATE PATTERN TO BE WRITTEN
        STOSW                          ;WRITE PATTERN INTO MEMORY
        LOOP    TG2                    ;REPEAT 32K TIMES

;       READ AND COMPARE LOOP

        MOV     CX,8000H               ;SETUP LOOP CNT                         
        XOR     DI,DI                  ;SETUP ADDRESS
TG3:    MOV     AX,BX                  ;GET PATTERN INVERT MASK
        XOR     AX,DI                  ;GENERATE PATTERN TO BE READ
        SCASW                          ;DOES READ AND WRITE PATTERN MATCH?   
        JE      TG4                    ;YES, CONTINUE           
        halt
TG4:    LOOP    TG3

;REPEAT TEST

        XOR     BX,0FFFFH              ;FLIP INVERT MASK AND REPEAT TEST
        JNZ     TG1

;MOVE TO NEXT BLOCK

        MOV     AX,DS
        CMP     AH,10H                 ;DONE ?
        JE      TG5
        ADD     AX,1000H
        MOV     DS,AX                  ;(DS)=NEXT 64K BLOCK TO BE TESTED 
        MOV     ES,AX                  ;(ES)=NEXT 64K BLOCK TO BE TESTED
        JMP     TG0 

;CLEAR MEMORY

TG5:    mov	ax,data
	mov	ds,ax
	mov	dx,memory_size		;save memory size
	mov	ax,reset_key		;save reset key
	mov	bp,ax			;in bp
	mov	si,vir_mem_size

	cld				;
        XOR     AX,AX                  ;DATA = 0
        MOV     ES,AX                  ;START AT SEGMENT 0
TG6:    MOV     CX,8000H               ;32K WORD BLOCK
        XOR     DI,DI                  ;START AT 0
        REP     STOSW                  ;CLEAR MEMORY
        MOV     BX,ES
        CMP     BH,10h                  ;DONE ?
        JE      TG7                    ;YES, JUMP
        ADD     BX,1000H
        MOV     ES,BX
        JMP     TG6
 TG7:    cmp	bp,1234h
	jne	TG8
	mov	memory_size,dx		;restore memory size
	mov	reset_key,bp		;
	mov	vir_mem_size,si

TG8:
	NOP

page
                                      
;******************************************************************************
;
;     TEST_9:	SETUP VIDEO                         
;
;******************************************************************************

TEST_9:
        MOV     SP,top_of_stack		;setup stack pointer
        MOV     AX,stack_seg
        MOV     SS,AX			;setup SS register

	MOV	AL,9			;#
	call	MakeLed

;setup the necessary interrupt vector (int 2h, 10h, 11h & 1fh)

        MOV     DI,8
        MOV     AX,OFFSET UNEXPECTED_INTERRUPT  ;STORE NMI VECTOR
        CALL    MOVE
 
        MOV     DI,40H
        MOV     AX,OFFSET VIDEO        	; STORE VIDEO VECTOR
        CALL    MOVE

        MOV     DI,44H
        MOV     AX,OFFSET CONFIG        	; STORE CONFIG VECTOR
        CALL    MOVE
 
        MOV     DI,74H
        MOV     AX,OFFSET VIDEO_PARM_TBL
        CALL    MOVE                   	; STORE VIDEO CHARACTER VECTOR
 
; ToBu (2) 18.8.88 Ver 3.4
	mov	di,7ch		;  phd 10/21/87
	mov	ax,0		; default the vector at 0:7c
	call	move		; for user's font table 128 to 255

	in	al,upi+4		;get status port
	test	al,1			;is output buffer full
	jz	test_9_1		;no
	in	al,upi			;flush output buffer
test_9_1:
	cmp	reset_key,1234h		;see if ctrl+alt+del
	je	test_9_2		;yes, skip keyboard initialization

;to set system flag

	mov	al,60h			;write controller's command byte
	out	upi+4,al		;send out command
	mov	ah,2			;input buffer full bit
	call	wait_upi		;wait for command accecptd
	mov	al,5ch			;IBM PC compatibility mode (bit 6)
					;disable keyboard (bit 4)
					;inhibit override (bit 3)
					;set system flag (bit 2)
	out	upi,al			;write keyboard controller's command byte
test_9_2:
	cli
	call	setint_ctrl		; initialize interrupt controller
	mov	al,0ffh			;disable all interrupts in case??
	out	int1+1,al		;  rlm  3/21/85
	jmp	$+2
	out	int2+1,al		;  rlm  3/21/85

; ToBu (1) 10.12.88 Ver 3.5

	MOV     ax,2002h			; init color card
        	mov     byte ptr EQUIP_FLAG,ah	;SET VIDEO BITS
        	XOR     AH,AH
        	INT     10H                    	;INITILIZE VIDEO

	MOV     ax,3007h			; init mono card
        	mov     byte ptr EQUIP_FLAG,ah	;SET VIDEO BITS
        	XOR     AH,AH
        	INT     10H                    	;INITILIZE VIDEO


	CALL    READ_SWITCH            ;GET VIDEO TYPE
	TEST    AL,40H                 ;MONOCHROME ? 
        	JZ      test_9_4               ;NO, JUMP  

        	MOV     ax,3007h		;monochrome card
        	JMP     short test_9_5

test_9_4:
	MOV     ax,2002h		;color card
test_9_5:

        mov     byte ptr EQUIP_FLAG,ah  		;SET VIDEO BITS
        XOR     AH,AH
        INT     10H                    		;INITILIZE VIDEO

;check for enhanced video card (C000-C7FF)
 
	mov	al,00h			;initialize vflag
	mov	vflag,al		

        MOV     AX,DATA
        MOV     ES,AX   			;setup ES register
        MOV     AX,0C000H			;start segment address
test_9_7:
        MOV     DS,AX			;setup DS register
        XOR     DI,DI			;offset 0
        MOV     AX,0AA55H			;valid pattern

        CMP     [DI],AX                		;ENHANCED VIDEO PRESENT ?
        JNE     test_9_3               		;NO, JUMP
		
	push	ds			;save ptr to vid seg
	push	es 			;save ptr to data seg
	
        	and	byte ptr es:equip_flag,0cfh	;clear video bits
        	MOV     ES:ROM_ADDRESS,3H
        	MOV     ES:ROM_SEGMENT,DS
        	CALL    ES:DWORD PTR ES:ROM_ADDRESS	;service init routine
	pop	es			;restore ptr to data seg
	pop	ds			;restore ptr to vid seg
	inc	es:vflag		;increment count of valid video segs

; ToBu (3) 18.8.88 Ver 3.4
test_9_3:
	mov	ax,ds			;get current vid rom seg ptr
	add	ax,80h			;bump seg by 2k
	cmp	ax,0c800h			;checked c0000-c7fff yet?
	jb	test_9_7			;no, try again
	mov	al,es:vflag
	and	al,0ffh
	jz	test_9_6
	
;TEST FOR NO VIDEO

	call	tvideo			;check no video condition
test_9_6:
	nop			      	;print sign on message now
	mov	si,offset Sign_On
	call	display

page
;******************************************************************************
;
;     TEST_A:	TEST RAM FROM 128K TO 640K
;     		WRITES, READS AND VERIFIES 128K RAM BLOCKS      
;     		FIRST PASS WRITES ADDRESS INTO DATA (NO INVERT).
;     		SECOND PASS WRITES COMPLEMENT OF ADDRESS INTO DATA (INVERT).
;     		CONFIGURE FIRST 640K OF MEMORY
;     		CLEAR 128K TO 640K RAM
;
;******************************************************************************

TEST_A:
	MOV	AL,0Ah
	call	MakeLed

	call	set_data
	cmp	reset_key,1234h		;soft reset
	jne	TG19
	jmp	T27			;skip memory test
	
;DISPLAY MEMORY THAT HAS BEEN TESTED (FIRST 128K)

TG19:
        MOV     SI,OFFSET TEST_6B_MSG_1
        CALL    DISPLAY

;SETUP FOR TEST

        MOV     DL,2                   ;(DL)= 64K BLOCK BEING TESTED
        MOV     AX,2000H
        MOV     DS,AX                  ;START AT 128K     
        MOV     ES,AX

 TG20:   CLD                            ;SETUP DIR FLAG FOR AUTOINCR
        XOR     BX,BX                  ;SETUP PATTERN INVERT MASK

;       WRITE LOOP

TG21:   MOV     CX,8000H               ;SETUP COUNT TO 64K BYTES
        XOR     DI,DI                  ;SETUP ADDRESS
TG22:   MOV     AX,BX                  ;GET PATTERN INVERT MASK
        XOR     AX,DI                  ;GENERATE PATTERN TO BE WRITTEN
        STOSW                          ;WRITE PATTERN INTO MEMORY
        LOOP    TG22                   ;REPEAT 32K TIMES

;       READ AND COMPARE LOOP

        MOV     CX,8000H               ;SETUP LOOP CNT                         
        XOR     DI,DI                  ;SETUP ADDRESS
TG23:   MOV     AX,BX                  ;GET PATTERN INVERT MASK
        XOR     AX,DI                  ;GENERATE PATTERN TO BE READ
        SCASW                          ;DOES READ AND WRITE PATTERN MATCH?   
        JNE     TG25                   ;NO - EXIT               
        LOOP    TG23

;REPEAT TEST

        XOR     BX,0FFFFH              ;FLIP INVERT MASK AND REPEAT TEST
        JNZ     TG21

;DISPLAY BLOCK THAT HAS BEEN TESTED

        TEST    DL,1                   ;128K BOUNDRY ?
        JZ      TG24                   ;NO, SKIP DISPLAY
        INC     SI
        CALL    DISPLAY

;CHECK FOR TEST DONE
TG24:	INC	DL
        CMP     DL,0Ah                 ;640K TESTED
        JE      TG25                   ;YES, JUMP

;MOVE TO NEXT BLOCK

        MOV     AX,DS
        ADD     AX,1000H
        MOV     DS,AX                  ;(DS)=NEXT 64K BLOCK TO BE TESTED 
        MOV     ES,AX                  ;(ES)=NEXT 64K BLOCK TO BE TESTED
        JMP     TG20                   ;REPEAT TEST

;SEND CR, LF TO SCREEN

TG25:   MOV     SI,OFFSET TEST_6B_MSG_2
        CALL    DISPLAY

;SAVE MEMORY SIZE

        MOV     AX,DATA
         MOV     DS,AX
	and	dl,0feh
        MOV     AL,64 
        MUL     DL                     ;MEMORY SIZE IN K BYTES
        MOV     MEMORY_SIZE,AX         ;SAVE IT 

; ToBu (4) 18.8.88 Ver 3.4
;*******************************************************************
;	set bit 7 of byte 33 of CMOS RAM if memory is more
;	than 512K 	phd 10/21/87
;******************************************************************* 

	cmp	ax,200h
	ja	tx27			;jump if memory > 512K
	mov	al,cmos_inf_flag+80h
	call	read_cmos
	and	ah,3fh			;turn off bits 6 and 7
	dec	al
	call 	write_cmos
pt27:	jmp	short t27
tx27:
	mov	al,cmos_inf_flag+80h	;byte 33 of CMOS RAM
	call	read_cmos
	or	ah,80h			;set bit 7
	and	ah,0bfh			;turn off bit 6. this bit is used by setup utility
	dec	al
	call	write_cmos

;CLEAR RAM

T27:
        CLD
        MOV     AX,2000H
        MOV     ES,AX                  ;START AT 128K BOUNDRY
        XOR     AX,AX                  ;DATA = 0
TG26:   MOV     CX,8000H
        XOR     DI,DI
        REP     STOSW                  ;CLEAR MEMORY
        MOV     BX,ES
        ADD     BX,1000H               ;MOVE TO NEXT BLOCK
        MOV     ES,BX
        CMP     BH,0A0H                ;DONE ?
        JNE     TG26                   ;NO, JUMP



;**************************************************************************
;	Now go into virtual mode and test memory over 1 MByte
;	First check for soft reset. If true then skip this test
;**************************************************************************

	cmp	reset_key,1234h		
	je	test_b		
	in	al,cmos+1		;read a dummy from CMOS for new
				;clock/calender chip
	jmp	test_26		;go test memory over 1 meg

page
                                      
;******************************************************************************
;
;     TEST_B:	DMA CONTROLLER #1 REGISTER TEST
;     		WRITE  CURRENT ADDRESS AND WORD COUNT REGISTERS FOR EACH
;     		CHANNEL WITH A UNIQUE PATTERN. READ THE REGISTERS AND CHECK
;     		IF SAME AS WRITTEN PATTERN.
;
;		SETUPT THE DMA CONTROLLER
;
;******************************************************************************

TEST_B:

;TEST # TO LEDS

        MOV     AL,0BH
	call	MakeLed

;
         OUT     DMA1+0DH,AL            ;CLEAR DMA CHIP
        CALL    DELAY

;       WRITE REG LOOP

TD1:    MOV     BL,1                   ;BASE PATTERN IN BL
TD2:    MOV	CX,8		       ;SETUP LOOP CNT
        MOV     DX,DMA1                ;SETUP DMA PORT ADDR
        MOV     AL,BL                  ;SETUP BASE PATTERN
TD3:    CALL    DELAY
        OUT     DX,AL                  ;OUTPUT TO I/O REG (LSB)
        CALL    DELAY
        OUT     DX,AL                  ;OUTPUT TO I/O REG (MSB)
        INC     DX                     ;GO TO NEXT I/O REG
        ROL     AL,1                   ;ROTATE PATTERN
        LOOP    TD3                    ;GO WRITE NEXT REG

;       READ REG LOOP

        MOV     CX,8                   ;SETUP LOOP CNT
        MOV     DX,DMA1                ;SETUP DMA PORT ADDR
        MOV     BH,BL                  ;GET BASE PATTERN
TD4:    CALL    DELAY
        IN      AL,DX                  ;READ I/O REG (LSB)
        CMP     AL,BH                  ;PATTERN MATCH WHAT WAS WRITTEN?
        JE      TD6                    ;YES - GO AND READ I/O REG MSB
TD5:    
;	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_6C_MSG_1
        CALL    DISPLAY
        MOV     BL,0C0H
        CALL    BEEP
        halt                            ;FATAL ERROR, HALT             
TD6:    CALL    DELAY
        IN      AL,DX                  ;READ I/O REG (MSB)
        CMP     AL,BH                  ;PATTERN MATCH WHAT WAS WRITTEN?
        JNE     TD5                    ;NO - GO TO HALT
        INC     DX                     ;YES - GO TO NEXT REG
        ROL     BH,1                   ;ROTATE PATTERN
        LOOP    TD4
        SHL     BL,1                   ;SHIFT BASE PATTERN
        JNZ     TD2                    ;REPEAT 8 TIMES

;SETUP THE DMA CONTROLLER

        XOR     AL,AL
        CALL    DELAY
        OUT     DMA1+8,AL               ;COMMAND REGISTER
        MOV     CX,4
        MOV     AL,40H
TD7:    CALL    DELAY
        OUT     DMA1+0BH,AL            ;MODE REGISTER
        INC     AL
        LOOP    TD7
 
page
                                      
;******************************************************************************
;
;     TEST_C:	DMA CONTROLLER #2 REGISTER TEST
;     		WRITE  CURRENT ADDRESS AND WORD COUNT REGISTERS FOR EACH
;     		CHANNEL WITH A UNIQUE PATTERN. READ THE REGISTERS AND CHECK
;     		IF SAME AS WRITTEN PATTERN.
;
;******************************************************************************

TEST_C:

;TEST # TO LEDS
        MOV     AL,0CH
	call	MakeLed
;
        OUT     DMA2+1AH,AL            ;CLEAR DMA CHIP

;       WRITE REG LOOP

TW1:    MOV     BL,1                   ;BASE PATTERN IN BL
TW2:    MOV	CX,8		       ;SETUP LOOP CNT
        MOV     DX,DMA2                ;SETUP DMA PORT ADDR
        MOV     AL,BL                  ;SETUP BASE PATTERN
TW3:    CALL    DELAY
        OUT     DX,AL                  ;OUTPUT TO I/O REG (LSB)
        CALL    DELAY
        OUT     DX,AL                  ;OUTPUT TO I/O REG (MSB)
        ADD     DX,2                   ;GO TO NEXT I/O REG
        ROL     AL,1                   ;ROTATE PATTERN
        LOOP    TW3                    ;GO WRITE NEXT REG

;       READ REG LOOP

        MOV     CX,8                   ;SETUP LOOP CNT
        MOV     DX,DMA2                ;SETUP DMA PORT ADDR
        MOV     BH,BL                  ;GET BASE PATTERN
TW4:    CALL    DELAY
        IN      AL,DX                  ;READ I/O REG (LSB)
        CMP     AL,BH                  ;PATTERN MATCH WHAT WAS WRITTEN?
        JE      TW6                    ;YES - GO AND READ I/O REG MSB
TW5:    
        MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_7_MSG_1                                 
        CALL    DISPLAY
        halt
TW6:    CALL    DELAY
        IN      AL,DX                  ;READ I/O REG (MSB)
         CMP     AL,BH                  ;PATTERN MATCH WHAT WAS WRITTEN?
        JNE     TW5                    ;NO - GO TO HALT
        ADD     DX,2                   ;YES - GO TO NEXT REG
        ROL     BH,1                   ;ROTATE PATTERN
        LOOP    TW4
        SHL     BL,1                   ;SHIFT BASE PATTERN
        JNZ     TW2                    ;REPEAT 8 TIMES

;SETUP DMA #2 CONTROLLER

        XOR     AL,AL    
        CALL    DELAY 
        OUT     DMA2+10h,AL                ;SET COMMAND
        MOV     AL,0C0H
        CALL    DELAY
        OUT     DMA2+16H,AL            ;SET MODE FOR CHANNEL 0
        MOV     CX,3
        MOV     AL,41H
TW7:    CALL    DELAY
        OUT     DMA2+16H,AL             ;SET MODE FOR CHANNELS 1 - 3
        INC     AL
        LOOP    TW7
	xor	al,al
	call	delay
	out	dma2+14h,al		;enable cascade channel
	
page


;******************************************************************************
;
;     TEST_D:	INTERRUPT CONTROLLER #1 TEST                   
;     		READ,WRITE THE INTR MASK REG (IMR) WITH AA ,55 PATTERNS.  SETUP THE
;     		INTERRUPT CONTROLLER VERIFY THAT NO INTERRUPTS OCCUR IF THE IMR IS       
;     		SET TO FFH.  VERIFY THAT A TIMER INTERRUPT OCCURS.
;        
;******************************************************************************

TEST_D:

;TEST # TO LEDS
        MOV     AL,0DH
	call	MakeLed

	jmp	short tc0

;SETUP INTERRUPT CONTROLLER

setint_ctrl	proc	near
        CLI                            ;DISABLE INTERRUPTS
        MOV     AL,11H
        OUT     INT1,AL                ;ICW1                        
	out	int2,al
	call	delay
        MOV     AL,08H
        OUT     INT1+1,AL              ;ICW2              
 	mov	al,70h
	out	int2+1,al
	call	delay
        MOV     AL,04H                 
        OUT     INT1+1,AL              ;ICW3                 
	shr	al,1
	out	int2+1,al
	call	delay
	shr	al,1
        OUT     INT1+1,AL              ;ICW4
	out	int2+1,al		;
	ret
setint_ctrl	endp


;TEST IMR

TC0:	call	setint_ctrl		; initialize interrupt controller
        MOV     AH,0AAH                ;START WITH AA PATTERN
TC1:    MOV     AL,AH                   
        CALL    DELAY
        OUT     INT1+1,AL              ;WRITE IMR
        XOR     AL,AL                  ;CLEAR AL
        CALL    DELAY
        IN      AL,INT1+1              ;READ IMR
        XOR     AL,AH                  ;DOES WRITE PATTERN=READ PATTERN?
        JNZ     TC8                    ;NO, JUMP              
        XOR     AH,0FFH                ;FLIP PATTERN AND REPEAT TEST
        JNS     TC1 
;
        MOV     AL,0FFH                ;MASK OFF ALL INTERRUPTS
        CALL    DELAY
        OUT     INT1+01,AL

;       VERIFY THAT NO INTERRUPTS CAN OCCUR WHEN TOTALLY MASKED OFF.
;       INITIALIZE INT VECTOR AREA. IN THE EVENT AN INTERRUPT OCCURS,
;       IT WILL VECTOR TO THE TEMPORARY INTR SERVICE ROUTINE

        MOV     CX,8                   ;SETUP LOOP CNT
        MOV     DI,OFFSET INT_PTR1     ;GET START OF INT VEC AREA
        MOV     AX,OFFSET TC4          ;AX= TEMP INT SERVICE RTN ADDR
TC2:    CALL    MOVE                                           
        LOOP    TC2                    ;REPEAT 8 TIMES
        MOV     CX,0FFH                
        STI                            ;ENABLE INTERRUPTS
TC3:    LOOP    TC3                    ;WAIT FOR POSSIBLE INTERRUPT
        CLI                            ;INTERRUPTS OFF
        JMP     TC5

;       TEMPORARY INTR SERVICE ROUTINE

TC4     PROC    NEAR
        JMP     TC8                    ;ERROR, INTERRUPT OCCURED
TC4     ENDP

;VERIFY THAT A TIMER 0 INTERRUPT OCCURS

TC5:    MOV     DI,OFFSET INT_PTR1     ;LOAD ADDRESS OF VECTOR       
        MOV     AX,OFFSET TC7          ;VECTOR                               
         CALL    MOVE                   
        MOV     AL,036H
        OUT     TIMER+3,AL             ;SELECT TIMER 0, MODE 3
        XOR     AL,AL                                    
        CALL    DELAY
        OUT     TIMER,AL               ;LOAD LOWER BYTE
        CALL    DELAY
        OUT     TIMER,AL               ;LOAD UPPER BYTE
        MOV     AL,0FEH
        OUT     INT1+1,AL              ;MASK INTERRUPT 0 ON
        STI                            ;ENABLE INTERRUPTS
        MOV     CX,0FFH                 
TC6:    LOOP    TC6                    ;WAIT FOR TIMER INTERRUPT
        JMP     TC8

;INTERRUPT ROUTINE         

TC7 PROC NEAR
        CALL    END_INTERRUPT                               
        JMP     TC9
TC7 ENDP

;PROCESS ERROR

TC8:    MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI, OFFSET TEST_8_MSG_1  
        CALL    DISPLAY
        halt
TC9:    NOP


page


;******************************************************************************
;
;     TEST_E:	INTERRUPT CONTROLLER #2 TEST                   
;     		READ,WRITE THE INTR MASK REG (IMR) WITH AA ,55 PATTERNS.  SETUP THE
;     		INTERRUPT CONTROLLER VERIFY THAT NO INTERRUPTS OCCUR IF THE IMR IS       
;     		SET TO FFH.                                          
;        
;******************************************************************************

TEST_E:

;TEST # TO LEDS

	mov	al,0eh
	call	MakeLed

 ;TEST IMR

        MOV     AH,0AAH                ;START WITH AA PATTERN
TAA1:   MOV     AL,AH                  ;
        CALL    DELAY
        OUT     INT2+1,AL              ;WRITE IMR
        XOR     AL,AL                  ;CLEAR AL
        CALL    DELAY
        IN      AL,INT2+1              ;READ IMR
        XOR     AL,AH                  ;DOES WRITE PATTERN=READ PATTERN?
        JNZ     TAA5                   ;NO, JUMP              
        XOR     AH,0FFH                ;FLIP PATTERN AND REPEAT TEST
        JNS     TAA1
;
        MOV     AL,0FFH                ;MASK OFF ALL INTERRUPTS
        CALL    DELAY
        OUT     INT2+1,AL
        MOV     AL,0FBH                                  
        OUT     INT1+1,AL              ;MASTER MASK

;       VERIFY THAT NO INTERRUPTS CAN OCCUR WHEN TOTALLY MASKED OFF.
;       INITIALIZE INT VECTOR AREA. IN THE EVENT AN INTERRUPT OCCURS,
;       IT WILL VECTOR TO THE TEMPORARY INTR SERVICE ROUTINE

        MOV     CX,8                   ;SETUP LOOP CNT
        MOV     DI,OFFSET INT_PTR2     ;GET START OF INT VEC AREA
        MOV     AX,OFFSET TAA4         ;AX= TEMP INT SERVICE RTN ADDR
TAA2:   CALL    MOVE                                           
        LOOP    TAA2                   ;REPEAT 8 TIMES


;        XOR     AL,AL			;****ver 2.60 ****
;        OUT     INT2+1,AL              ;SLAVE MASK

        MOV     CX,0FFH                
        STI                            ;ENABLE INTERRUPTS
TAA3:   LOOP    TAA3                   ;WAIT FOR POSSIBLE INTERRUPT
        CLI                            ;INTERRUPTS OFF
        JMP     TAA6

;       TEMPORARY INTR SERVICE ROUTINE

TAA4    PROC    NEAR
        JMP     TAA5                   ;ERROR, INTERRUPT OCCURED
TAA4    ENDP

;PROCESS ERROR

TAA5:   MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI, OFFSET TEST_8A_MSG_1  
        CALL    DISPLAY
        halt
TAA6:   NOP


 page

;******************************************************************************
;
;     TEST_F:	TEST PIO
;
;******************************************************************************

TEST_F:

;TEST # TO LEDS 

        MOV     AL,0FH
	call	MakeLed

	mov	al,80h
	out	cmos,al			;turn off NMI

;
        MOV     AL,5H  
        OUT     PIO,AL
        CALL    DELAY
        IN      AL,PIO
        AND     AL,0FH
        CMP     AL,5H                  ;PIO ERROR ?
        JNE     TI0                    ;YES, JUMP 
        MOV     AL,0AH
        OUT     PIO,AL
        CALL    DELAY
        IN      AL,PIO
        AND     AL,0FH
        CMP     AL,0AH                 ;PIO ERROR ?
        JE      TI1                    ;NO, JUMP
TI0:    

	MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_8B_MSG_1
        CALL    DISPLAY
        halt
TI1:    NOP


page

;******************************************************************************
;                                                                             *
;     TEST_10:	TEST RAM PARITY                                                         *
;                                                                             *
;******************************************************************************

TEST_10:

 ;TEST # TO LEDS
        MOV     AL,010H
	call	MakeLed
;
        MOV     DI,8                   ;VECTOR ADDRESS  
        MOV     AX,OFFSET TU3          ;VECTOR
        CALL    MOVE                                                     
        IN      AL,PIO         
        AND     AL,03H  
        CALL    DELAY
        OUT     PIO,AL                 ;TURN ON PARITY

;TURN ON NMI                 

        XOR     AL,AL 
        CALL    READ_CMOS

;SETUP FOR WRITE

        MOV     AX,0800H
        MOV     DS,AX                  ;(DS)=800H
        XOR     SI,SI
        MOV     CX,255
        XOR     AX,AX

;WRITE MEMORY BLOCK

TU1:    MOV     [SI],AX
        INC     SI                     ;WRITE MEMORY BLOCK        
        ADD     AX,0101H
        LOOP    TU1

;SETUP FOR READ

        XOR     SI,SI
        MOV     CX,255

;READ MEMORY BLOCK    

TU2:    MOV     AX,[SI]                ;READ MEMORY BLOCK         
        INC     SI
        LOOP    TU2

;CHECK FOR ANY PARITY ERRORS

        IN      AL,PIO                     
        TEST    AL,0C0H                ;TEST FOR PARITY ERROR
        JZ      TU4                    ;JUMP IF NO PARITY ERROR

;INTERRUPT SERVICE ROUTINE

TU3     PROC    NEAR
        MOV     AL,80H
         OUT     NMI,AL                 ;OFF NMI
        MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_9_MSG_1
        CALL    DISPLAY
        halt                            ;HALT NMI ERROR    
TU3     ENDP

;TURN PARITY OFF

TU4:    MOV     AL,80H
        OUT     NMI,AL                 ;OFF PARITY ERROR
        IN      AL,PIO         
        OR      AL,0CH
        CALL    DELAY
        OUT     PIO,AL                 ;OFF PARITY

page

;******************************************************************************
;
;     TEST_11:	TEST CMOS CLOCK CALENDAR FOR                    
;     		1. BATTERY FAILURE
;     		2. CHECKSUM FAILURE
;
;******************************************************************************

TEST_11:

;TEST # TO LEDS

        MOV      AL,011H
	call	MakeLed

; ToBu (11) 18.8.88 Ver 3.4				
 	CALL	SET_DATA			; setup DS

;CLEAR STATUS

        MOV     AL,CMOS_STATUS_1+80H
        XOR     AH,AH                                      
        CALL    WRITE_CMOS             ;CLEAR STATUS

;CHECK BATTERY STATUS

        MOV     AL,BATTERY+80H
        CALL    READ_CMOS              ;GET BATTERY STATUS
        TEST    AH,80H                 ;BATTERY OK?
        JNZ     TEE0                   ;YES, JUMP
        MOV     AH,80H
        MOV     AL,CMOS_STATUS_1+80H
        CALL    WRITE_CMOS             ;INDICATE BAD BATTERY
        MOV     BL,0FFH
        CALL    BEEP                   ;BEEP SPEAKER
; 	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_9A_MSG_1
        CALL    DISPLAY                ;DISPLAY BATTERY ERROR

;CHECK FOR CHECKSUM ERROR

TEE0:   CALL    CCHECK                 ;DO A CMOS CHECKSUM
        JE      TEE2                   ;JUMP IF OK
        MOV     AL,CMOS_STATUS_1+80H
        CALL    READ_CMOS
        OR      AH,40H
        DEC     AL
        CALL    WRITE_CMOS             ;SET CHECKSUM ERROR

;CLEAR CMOS MEMORY IF IT ISN'T VALID

        MOV     AL,CMOS_STATUS_1+80H
        CALL    READ_CMOS
        TEST    AH,0C0H                ;CMOS VALID ?
        JZ      TEE2                   ;YES, SKIP CLEAR
        MOV     CX,CMOS_SIZE
        MOV     AL,CMOS_START+80H
        XOR     AH,AH
TEE1:   CALL    WRITE_CMOS             ;CLEAR LOCATION
        LOOP    TEE1                   ;CONTINUE TILL ALL CLEAR
; ToBu (12) 18.8.88 Ver 3.4
        OR	BYTE PTR [EQUIP_FLAG +1],01	;Indicate POST error
TEE2:   NOP

page

;******************************************************************************
;
;     TEST_12:	TEST FOR MANUFACTURING TEST MODE
;
;******************************************************************************

TEST_12:

;TEST # TO LEDS

        MOV     AL,12H
	call	MakeLed

;BEEP THE SPEAKER

        MOV     BX,0C0H                ;BEEP LENGTH
        CALL    BEEP

;TEST FOR MANUFACTURING TEST MODE

	call	set_data

; ToBu (5) 18.8.88 Ver 3.4
	if      LoopBackTest
;        	CALL    READ_SWITCH                                       
;        	TEST    AL,20H                 ;MANUFACTURING TEST MODE ?
;        	JNZ     TEST_13		       ;NO, JUMP                   
        	JMP     MFG_TEST                                  
	endif
page



;******************************************************************************
;                                                                              
;     TEST_13:	SETUP INTERRUPT CONTROLLER AND MOVE VECTORS TO RAM 
;                                                                              
;******************************************************************************

TEST_13:

	MOV	AL,13H
	call	MakeLed

;MOVE VECTORS FOR INTERRUPTS 0 TO 0F

        MOV     SI,OFFSET VECTOR_TABLE ;SOURCE ADDRESS=VECTOR_TABLE
        MOV     CX,10h
        XOR     DI,DI
TF0:    MOV     AX,CS:[SI]             ;GET VECTOR                 
        CALL    MOVE              
        ADD     SI,2
        LOOP    TF0

;
;	move vectors 11 through 1F
;
	add	di,4			;skip vector 10
	add	si,2

; ToBu (6) 18.8.88 Ver 3.4
	mov	cx,0eh 			; do 11 up to 1e
TF5:
	mov	ax,cs:[si]
	call	move			;
	add	si,2
	loop	TF5	
;
;	move vector 40h, 41h and 46h
;

	mov	di,100h
	mov	ax,cs:[si]
	call	move
	add	si,2
	mov	di,104h
	mov	ax,cs:[si]
	call	move
	add	si,2
	mov	di,118h
	mov	ax,cs:[si]
	call	move
	add	si,2

;MOVE VECTORS FOR INTERRUPTS 70 TO 77

        MOV     CX,8
        MOV     DI,01C0H
TF1:    
        mov	ax,cs:[si]
        CALL    MOVE        
        ADD     SI,2
        LOOP    TF1

;ENABLE MASTER INTERRUPTS            

        CLI

;       MOV     AL,0BAH
; ToBu (7) 18.8.88 Ver 3.4

        MOV     AL,0BEH		  ; ***  ver 2.60 ** 12/4/86  rlm

        OUT     INT1+1,AL              ;ENABLE TIMER,FLOPPY

;ENABLE SLAVE INTERRUPTS
; ToBu (13) 18.8.88 Ver 3.4
        MOV     AL,0FDH
;       MOV     AL,0FCH 		
        OUT     INT2+1,AL              ;ENABLE CALENDAR and redirected INT2
        JMP     TF2

VECTOR_TABLE LABEL WORD

        DW      OFFSET UNEXPECTED_INTERRUPT         ;00 DIVIDE BY ZERO
        DW      OFFSET UNEXPECTED_INTERRUPT         ;01 SINGLE STEP
        DW      OFFSET PARITY_ERROR                 ;02 NMI
        DW      OFFSET UNEXPECTED_INTERRUPT         ;03 BREAKPOINT
        DW      OFFSET UNEXPECTED_INTERRUPT         ;04 OVERFLOW
        DW      OFFSET SCREEN_PRINT                 ;05 PRINT SCREEN
        DW      OFFSET UNEXPECTED_INTERRUPT         ;06
        DW      OFFSET UNEXPECTED_INTERRUPT         ;07      
        DW      OFFSET TIME                         ;08 TIMER
        DW      OFFSET KEYBOARD                     ;09 KEYBOARD
        DW      OFFSET UNEXPECTED_INTERRUPT         ;0A
        DW      OFFSET UNEXPECTED_INTERRUPT         ;0B SERIAL 2
        DW      OFFSET UNEXPECTED_INTERRUPT         ;OC SERIAL 1
        DW      OFFSET UNEXPECTED_INTERRUPT         ;0D
        DW      OFFSET floppy_intr		 ;OE DISK
        DW      OFFSET UNEXPECTED_INTERRUPT         ;0F PRINTER
        DW      OFFSET VIDEO_START                  ;10 VIDEO
        DW      OFFSET CONFIG                       ;11 CONFIGURATION
        DW      OFFSET RAM_SIZE                     ;12 MEMORY SIZE
        DW      OFFSET wd_drv			 ;13 hard DISK
        DW      OFFSET SERIAL_IO                    ;14 SERIAL
        DW      OFFSET INTR_15                      ;15 CASSETTE
        DW      OFFSET KEY                          ;16 KEYBOARD
        DW      OFFSET PRINT                        ;17 PRINTER
        DW      OFFSET break			 ;18 CASSETTE BASIC (dummy)
        DW      OFFSET BOOT                         ;19 BOOTSTRAP
        DW      OFFSET CLOCK                        ;1A TIME OF DAY
        DW      OFFSET BREAK                        ;1B KEYBOARD BREAK
        DW      OFFSET BREAK                        ;1C TIMER USER INTERRUPT
        DW      OFFSET VIDEO_PARM_TBL               ;1D
        DW      OFFSET floppy_parms                 ;1E FLOPPY PARAMETERS
; ToBu (6) 18.8.88 Ver 3.4
;       dw	     0				 ;1f video
;
;	define interrupts 40h through 46h
;
 	dw	floppy_io			;int 40h diskette driver
	dw	wd_config_tbl		;int 41h parameter table for disk 0
	dw	wd_config_tbl		;int 46h parameter table for disk 1


        DW      OFFSET ALARM                        ;70 CMOS CLOCK CALENDAR
        DW      OFFSET INT71			    ;71
        DW      OFFSET UNEXPECTED_INTERRUPT         ;72
        DW      OFFSET UNEXPECTED_INTERRUPT         ;73
        DW      OFFSET UNEXPECTED_INTERRUPT         ;74             
        DW      OFFSET MATH                         ;75 COPROCESSOR
        DW      OFFSET wd_intr			    ; 76 hard disk
        DW      OFFSET UNEXPECTED_INTERRUPT         ;77

TF2:    ;CALL	TVIDEO

page

;******************************************************************************
;                                                                             
;     TEST_14:	KEYBOARD TEST                                                 
;                                                                             
;******************************************************************************

TEST_14:

;TEST # TO LEDS

	MOV	AL,14H
	call	MakeLed

;Cherry keyboard problem - V2.56

	cmp	reset_key,1234h	;see if soft reset
	je	test_14_3		;yes, don't enable the keyboard

	mov	ah,2		;wait for input buffer empty
	call	wait_upi
	mov	al,0aeh		;enable keyboard
	out	upi+4,al
	mov	ah,2		;wait for input buffer empty
	call	wait_upi
	mov	bh,4		;outer loop count
	mov	cx,0
test_14_1:
	mov	bl,6		;inner loop count
test_14_2:
	in	al,upi+4		;check output fbuffer full bit
	test	al,1
	jnz	test_14_3		;output buffer full
	loop	test_14_2
	dec	bl		;decrement inner loop count
	jnz	test_14_2
	dec	bh		;decrement outer loop count
	jnz	test_14_1
test_14_3:
	mov	ah,2		;wait for input buffer empty
	call	wait_upi
	mov	al,0adh		;disable the keyboard
	out	upi+4,al
	mov	ah,2		;wait for input buffer empty
	call	wait_upi

	in	al,upi+4		;get status
	and	al,1		;check output buffer full
	jz	test_14_4		;empty
	in	al,upi		;flush output buffer
test_14_4:
	CLI                           ;INTERRUPTS OFF
        	MOV     	DI,024H             ;VECTOR ADDRESS   
        	MOV     	AX,OFFSET TR2       ;VECTOR
        	CALL    	MOVE                                                        
        	IN      	AL,INT1+1
        	AND     	AL,0FDH
        	CALL    	DELAY
        	OUT     	INT1+1,AL           ;LOAD INTERRUPT MASK

;	set up upi

	mov	al,060h
	out	upi+4,al
	mov	ah,2
	call	wait_upi
	mov	al,45h
	out	upi,al

 ;CLEAR KEYBOARD

        MOV     AH,2
        CALL    WAIT_UPI               ;WAIT FOR input BUFFER
        MOV     AL,0FFH
        OUT     UPI,AL                 ;CLEAR KEYBOARD                   
        STI                            ;INTERRUPTS ON

;WAIT FOR KEYBOARD INTERRUPT

        XOR     CX,CX
TR1:    LOOP    TR1                    ;WAIT FOR INTERRUPT
        CLI
        JMP     TR51                   ;ERROR, NO INTERRUPT OCCURED

;KEYBOARD INTERRUPT ROUTINE

TR2:
;	mov	al,0BAh			;
; ToBu (7) 18.8.88 Ver 3.4
	mov	al,0beh			; **** ver 2.60 ** 12/4/86   rlm

	out	int1+1,al	   
	call	delay	   
	mov	al,eoi
	out	int1,al      

;CHECK CHARACTER                 

        	IN      	AL,UPI                 ;GET THE DATA
        	CMP     	AL,0FAH                ;KEYBOARD ERROR ? 
        	JNE      	TR52                  ;yes, JUMP             
	mov	bh,0ffh
TR8:
	xor	cx,cx
TR6:
	in	al,upi+4
	test	al,1
	jnz	TR7
	loop	TR6
	dec	bh
	jnz	TR8
	jmp	short TR53
TR7:
	in	al,upi
	cmp	al,0AAh
	je	TR4	
	cmp	al,'A'
	je	TR3	


;DISPLAY KEYBOARD ERROR                

	MOV     	SI,OFFSET TEST_13_MSG_50
	jmp     	tr5	
TR51:    
	MOV     	SI,OFFSET TEST_13_MSG_51
	jmp     	tr5	
TR52:    
	MOV     	SI,OFFSET TEST_13_MSG_52
	jmp	tr5
TR53:    
	MOV     	SI,OFFSET TEST_13_MSG_53

tr5:
        	CALL    	DISPLAY
        	JMP     	TR4


;DISPLAY "OK"

TR3:    
	MOV     SI, OFFSET TEST_13_MSG_2
        	CALL    DISPLAY

;RESTORE KEYBOARD VECTOR

TR4:    MOV     DI,024H
        MOV     AX,OFFSET KEYBOARD
        CALL    MOVE         

; ToBu (8) 18.8.88 Ver 3.4
	in	al,int1+1		;enable keyboard interrupt
	and	al,11111101B	;
	call	delay
	out	int1+1,al

page

;******************************************************************************
;                                                                             
;     TEST_15:	TEST AND CONFIGURE PARALLEL PORTS                             
;                                                                             
;******************************************************************************

TEST_15:
	MOV	AL,15H
	call	MakeLed

        MOV     SI,OFFSET TS3
        XOR     DI,DI

;TEST PARALLEL PORT

TS0:    MOV     DX,CS:[SI]             ;GET PARALLEL PORT ADDRESS
        CMP     DX,0                   ;ALL PARALLEL PORTS TESTED ?
        JE      TS4                    ;YES, JUMP
        MOV     AX,0AAAAH
        OUT     DX,AL                  ;WRITE PARALLEL PORT  
        MOV     CX,0FFH
TS1:    LOOP    TS1
	call	delay
        IN      AL,DX                  ;READ PARALLEL PORT  
        CMP     AL,AH                  ;PARALLEL PORT INSTALLED ?
        JNE     TS2                    ;NO, JUMP        
        MOV     PARALLEL_TABLE[DI],DX  ;ADDRESS OF PORT TO PARALLEL TABLE
        ADD     DI,2
TS2:    ADD     SI,2
        JMP     TS0

TS3 LABEL WORD
; ToBu (2) 12.12.88 Ver 3.5
	
        DW 378H,278H,0            ;PARALLEL PORT ADDRESSES
				       ; new order (V3.2)
;SET EQUIP FLAG

TS4:    MOV     AX,DI
        MOV     CL,13
        SHL     AX,CL                  ;MOVE # TO TOP OF REGISTER
        OR      EQUIP_FLAG,AX          ;CONFIGURE FOR PARALLEL

 ;SET TIMEOUT VALUES

        MOV     AX,DS                           
        MOV     ES,AX
        CLD
        MOV     DI,OFFSET PRINT_TIME 
        MOV     AX,1414H               ;TIMEOUT = 14h
        MOV     CX,2                   ;REPEAT COUNT = 2
        REP     STOSW                  ;Ser (V3.2)


;************************************************************************
;	CONFIGURE VIDEO BITS IN CMOS RAM
;************************************************************************

	MOV	BX,EQUIP_FLAG
	AND	BL,30H			;BL = VIDEO CONFIG
	MOV	AL,CMOS_EQUIP+80H
	CALL	READ_CMOS		;GET CMOS CONFIG
	AND	AH,0CFH			;
	OR	AH,BL
	DEC	AL
	CALL	WRITE_CMOS		;STORE CMOS EQUIP_FLAG
page
                                      
;******************************************************************************
;
;     TEST_16:	SERIAL (8250) CONFIGURATION            
;
;******************************************************************************

TEST_16:
	MOV	AL,16H
	call	MakeLed

        XOR     DI,DI
        MOV     DX,3FAH                             
        CALL    TH1                    ;TEST FOR COM1 
        MOV     DX,2FAH
        CALL    TH1                    ;TEST FOR COM2
        JMP     TH3                                

TH1 	PROC 	NEAR          

        IN      AL,DX                  ;READ SERIAL INTERRUPT ID
        TEST    AL,0F0H                ;UART PRESENT ?
        JNZ     TH2                    ;NO, JUMP
        SUB     DX,02H
        MOV     SERIAL_BASE[DI],DX
        ADD     DI,2
TH2:    RET
TH1 ENDP

 TH3:    MOV     CL,8
        SAL     DI,CL
        OR      EQUIP_FLAG,DI          ;SET # SERIAL CHANNELS

;SET TIMEOUT VALUE

        MOV     AX,DS                             
        MOV     ES,AX
        CLD
        MOV     CX,2                   ;REPEAT COUNT
        MOV     AX,0101H               ;TIMEOUT = 01
        MOV     DI,OFFSET SERIAL_TIME
        REP     STOSW                  ;SET TIMEOUT VALUE
page
;******************************************************************************
;                                                                             
;     TEST_17:	CONFIGURE MEMORY  < 640K                                 
;                                                                             
;******************************************************************************

TEST_17:

	MOV	AL,17H
	call	MakeLed

;TURN ON PARITY

        IN      AL,PIO  
        AND	AL,03H
	call	delay
        OUT     PIO,AL                 ;ENABLE PARITY

;CHECK TO SEE IF CMOS IS VALID. IF NOT, LOAD NEW CONFIGURATION   

        MOV     AL,CMOS_STATUS_1+80H
        CALL    READ_CMOS              ;GET STATUS
        TEST    AH,0C0H                ;CMOS VALID ?
        JZ      THH0                   ;YES, JUMP
        CALL    THH1                   ;WRITE NEW CONFIGURATION
        JMP     THH3

;CHECK NEW CONFIGURATION AGAINST OLD CONFIGURATION

THH0:   MOV     AL,LOW_RAM+80H
        CALL    READ_CMOS
        MOV     BL,AH
        CALL    READ_CMOS
        MOV     BH,AH                  ;AX = OLD MEMORY SIZE
        CMP     BX,MEMORY_SIZE         ;NEW = OLD ?
         JE      THH3                   ;YES, EXIT

;NEW DOES NOT EQUAL OLD. IF WE CAME FROM ALT/CNTL/DEL RELOAD SIZE FROM CMOS
;ELSE SAVE NEW AND SEND WARNING MESSAGE
	CMP	RESET_KEY,1234H		;FROM ALT/CNTL/DEL ?
	JNE	THH2			;..NO
	mov	memory_size,bx		;reload size, topview fix rj 7/7/86
	JMP	SHORT THH3
THH2:	CALL	THH1			;SAVE NEW SIZE IN CMOS
        MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_16_MSG_1              
        CALL    DISPLAY                ;DISPLAY WARNING MESSAGE
; ToBu (12) 18.8.88 Ver 3.4
        OR	BYTE PTR [EQUIP_FLAG +1],01	;Indicate POST error

        JMP     SHORT THH3             ;EXIT

THH1 	PROC NEAR                         ;STORE NEW MEMORY CONFIGURATION
        MOV     BX,MEMORY_SIZE
        MOV     AH,BL
        MOV     AL,LOW_RAM+80H
        CALL    WRITE_CMOS
        MOV     AH,BH
        CALL    WRITE_CMOS
        RET
THH1 ENDP

THH3:   NOP
page
;******************************************************************************
;
;     TEST_18:	CONFIGURE MEMORY OVER 1MB
;
;******************************************************************************

TEST_18:
	MOV	AL,18H
	call	MakeLed

; make sure that address line 20 is low

	mov	al,0ddh
	call	gate_a20


;TEST FOR CMOS VALID.  IF NOT, STORE NEW MEMORY SIZE

	mov	dx,vir_mem_size		;get vir_mem_size
        MOV     AL,CMOS_STATUS_1+80H
        CALL    READ_CMOS
        TEST    AH,0C0H                 ;VALID ?
        JNZ	shut1			;NO, LOAD NEW SIZE

;CHECK OLD SIZE VS NEW SIZE

        MOV     AL,high_ram+80H        ;low byte of expansion mem size
        CALL    READ_CMOS              ;GET LOW BYTE OF OLD SIZE
        MOV     BL,AH                  ;SAVE IT
        CALL    READ_CMOS      
        MOV     BH,AH                  ;BX = OLD SIZE
        CMP     BX,DX                  ;DOES IT MATCH ?
        JE	shut2			;YES, EXIT

; ToBu (9) 18.8.88 Ver 3.4

	cmp	reset_key,1234h
	jne	test_18_1		; skip warning if we are warm booted
	mov	vir_mem_size,bx
	jmp	short shut2

;SEND WARNING MESSAGE TO DISPLAY
TEST_18_1:
        MOV     BL,0C0H
        CALL    BEEP
;	mov	si,offset warning_msg
;	call	display
        MOV     SI,OFFSET TEST_16A_MSG_1
        CALL    DISPLAY
; ToBu (12) 18.8.88 Ver 3.4
        OR	BYTE PTR [EQUIP_FLAG +1],01	;Indicate POST error

;LOAD NEW SIZE

shut1:
	MOV     AL,high_ram+80H
	MOV     AH,DL
	CALL    WRITE_CMOS
	MOV     AH,DH
	CALL    WRITE_CMOS
	mov	al,17h+80H
	mov	ah,dl
	call	write_cmos
	mov	ah,dh
	call	write_cmos
shut2:
        NOP

page

;******************************************************************************
;                                                                             
;     TEST_19:	CONFIGURE KEYBOARD                                           
;                                                                             
;******************************************************************************

TEST_19:
	MOV	AL,19H
	call	MakeLed

;SETUP BUFFERS
	mov	reset_key,0
        MOV     KEY_BUFFER_START,OFFSET KEY_BUFFER
        MOV     KEY_BUFFER_END,OFFSET KEY_BUFFER + 20h
        MOV     IN_BUFFER,OFFSET KEY_BUFFER                        
        MOV     OUT_BUFFER,OFFSET KEY_BUFFER                        

;SET INTERRUPT MASK

        IN      AL,INT1+1                
; ToBu (14) 18.8.88 Ver 3.4
;       AND     AL,0FDH
        AND     AL,0bcH	        ;enable keyboard, timer, disk interrupt
        call    delay
        OUT     INT1+1,AL              ;TURN INTERRUPT ON


page
;******************************************************************************
;
;     TEST_FF:	INITILIZE JANUS INTERRUPTS  (V3.2)
;
;******************************************************************************

TEST_FF:

;TEST # TO LEDS

        MOV     AL,0ffH
	call	MakeLed

;-----------------------------------------------------------------------------
; Enable interrupts from AMIGA side
;
	 mov	 dx,parallel+1		; hidden JanIntEnable register (w/o)
	 xor	 al,al			; Bit6=0 enables interrupts
	 out	 dx,al


; Toggle keyboard enable line instad of R.J.'s "fake key"- routines 

	mov	 dx,pio
	in	 al,dx
	xor	 al,enbkb   
	out	 dx,al
	xor	 al,enbkb
	out	 dx,al


page

;CHECK KEY STATUS

	CALL	READ_SWITCH
        TEST    AL,80H                 ;KEY SWITCH ON ?
        JNZ     test_19_a                ;YES, JUMP       
        MOV     BL,0C0H
        CALL    BEEP
        MOV     SI,OFFSET TEST_17_MSG_1
        CALL    DISPLAY
TCC0:	CALL	READ_SWITCH
        TEST    AL,80H                 ;KEY SWITCH ON ?
        JZ      TCC0                   ;NO, TRY AGAIN 

; ToBu (15) 18.8.88 Ver 3.4  read keyboard ID and set some keyboard flags
test_19_a:
	xor	ax,ax
	mov	flag_1,al
	mov	flag_2,al
	mov	flag_3,al		;initialize keyboard flags
	mov	flag_4,al		;
	mov	ah,2			;wait for 8742
	call	wait_upi
	sti
	mov	al,read_kbd_id		;send command to keyboard
	out	upi,al			;send to keyboard
	or 	flag_4,0a0h		;set read ID in progress
	mov	al,25			;and numlok flags
	xor	cx,cx			;
wait_loop:	
	loop	wait_loop
	dec	al			;
	jnz	wait_loop		;
	cli
	and	flag_4,1fh		;reset above flags
page
;******************************************************************************
;									      
;     N E W  TEST_1A:	CONFIGURE FLOPPY DISK		      
;									      
;******************************************************************************

; ToBu (25) 18.8.88 Ver 3.4	Autoconfig supports 1.44MByte drives now

TEST_1A:
	mov	al,1ah
	mov	dx,diag_port
	not	al
	out	dx,al

;CALCULATE # OF FLOPPY DISK PRESENT

	XOR     AH,AH
	mov	dl,0			;set drive code
	INT     40h		    	;RESET THE DISK ADAPTER


; check for IBM compatible hard disk combo card

tpp8:
	call	set_data
	assume ds:dataa
	mov	data_rate,0	;assume no combo first

	mov	dx,03f1h	;look at diskette diag register
	in	al,dx		;
	cmp	al,50h		;check for dual data rate
	jz	combo		;combo board

;	mov	dx,3f7h
;	mov	al,2
;	out	dx,al
;	jmp	$+2
	
;	test some hard disk I/O ports

	mov	dx,01f4h	;cylinder read/write port
	mov	ax,0aaaah	;
	out	dx,al		;
	call	delay		;
	xor	al,al		;
	in	al,dx		;
	xor	al,ah		;input = output
;	jnz	no_combo	;
combo:
	mov	data_rate,01	;set dual combo flag
no_combo:
		
;
;	this routine will check if the drive type determined in the
;	machine match those specified in the CMOS memory. If not then
;	we assume that drives have been changed without modifying CMOS.
;	An error message is displayed on the screen 

	xor	ax,ax
	mov	word ptr drv_0_media,ax
	mov	word ptr drv_0_op,ax
	mov	word ptr motor_timout,ax	;set timeout value
	mov	word ptr recal_reqd,ax	;
	mov	dx,DOR			;
	mov	al,01ch			;turn on motor for drive A
	out	dx,al			;start motor
	xor	cx,cx
tpp8_1:	loop	tpp8_1			;wait for motor start
	mov	dl,0			;drive 0
	call	detrm_drv_type		;determine drive type
	mov	cl,al			;
	mov	al,flp_type+80h		;get diskette type
	call	read_cmos		;
	and	ah,0f0h			;first drive
	shr	ah,4			;
	cmp	cl,ah			;CMOS valid
	je	set_f1			;yes
	cmp	cl,2			;is it 80 trk drive
	jb	tpp8_x			;no,then error
	cmp	ah,3			;
	jb	tpp8_x			;
	jmp	short set_f1		;no change in configuration
tpp8_x:
	mov	si,offset drv0_config_msg	;print warning
	call	display			;
	mov	al,cmos_status_1+80h	;See if CMOS valid
	call	read_cmos			;display error msg and set
	and	ah,0c0h			; to default, if not valid
	jz	set_f1			
	push	cx
	mov	si,offset drv0_default_msg	;print update msg
	call	display			;
	pop	cx			;get drive determined back

; ToBu (19) 18.8.88 Ver 3.4			; set default, if CMOS is bad
	mov     	AH,20h			; set default = 96TPI drive
        	MOV     	AL,FLP_TYPE+80H
        	CALL    	WRITE_CMOS
 	mov	cl,2			; set 80track drive
	or	byte ptr [equip_flag +1],1	; set POST error
set_f1:	
;	mov	al,flp_type+80h	
;	call	read_cmos		
;	and	ah,0fh			;save second drive
;	shl	cl,4			;move 1st drive in bits 4-7
;	or	ah,cl			
;;	mov	ah,cl			;cl has first drive
;	mov	al,flp_type+80h		
;	call	write_cmos			

	mov	hf_cntrl,0
	or	cl,cl
	jz	tpp8_2
	cmp	cl,1
	jne	set_f1_2
	mov	drv_0_media,93h		;drive det, media det and rate_250
	mov	hf_cntrl,4		;drive determined
	jmp	short tpp8_2
set_f1_2:
	mov	drv_0_media,track_80
	mov	hf_cntrl,1

;	now check second drive
tpp8_2:
	mov	dl,1			;check second drive
	call	detrm_drv_type		;
	push	ax			;save drive determined type
	mov	al,flp_type+80h		;
	call	read_cmos		;
	and	ah,0fh			;
	pop	cx			;cl = drive type determined
	cmp	cl,ah			;ah = drive type in CMOS
	je	set_f2			;
	cmp	cl,2			;is it 80 trk drive
	jb	tpp8_y			;no,then error
	cmp	ah,3			;
	jb	tpp8_y			;
	jmp	short set_f2		;no change in configuration
tpp8_y:
	mov	al,cmos_status_1+80h	;See if CMOS valid
	call	read_cmos			;Don't display error msg 
	and	ah,0c0h			; if not valid
	jnz	set_f2			; (CMOS error msg 
	push	cx			; will be displayed)
;	mov	si,offset warning_msg
;	call	display
	mov	si,offset drv1_config_msg
	call	display
	pop	cx
	or	byte ptr [equip_flag +1],01
set_f2:
;	mov	al,flp_type+80h		;
;	call	read_cmos		;
;	and	ah,0f0h			;save drive 0
;	or	ah,cl			;add second drive
;	mov	al,flp_type+80h		;
;	call	write_cmos

	or	cl,cl
	jz	setup_end
	cmp	cl,1
	jne	set_f2_1
	mov	drv_1_media,93h		;drive det, media det and rate_250
	or	hf_cntrl,40h		;drive determined
	jmp	short setup_end
set_f2_1:
	mov	drv_1_media,track_80
	or	hf_cntrl,10h


;
;	now setup equipment flags in cmos and also in low
;	memory. From here on we assume that cmos is good and drives
;	are determined as defined in CMOS
;
setup_end:
	call	set_data			;make sure ds points to dataa
	mov	recal_reqd,00	
	mov	dsk_opn_status,0	
	mov	al,cmos_equip+80h		;read equip flag from cmos
	call	read_cmos		
	and	ah,3eh			;save all but diskette bits
	and	byte ptr equip_flag,3eh	;
	mov	bl,ah			;
	mov	cl,0			;# of drives present
	mov	al,flp_type+80h		;
	call	read_cmos			;get drive types
	mov	al,ah			;save it
	and	al,0f0h			;test for first drive
	jz	setup_4			;no first drive
	or	bl,1			;set APL diskette present bit
	or	byte ptr equip_flag,1	
setup_4:
	mov	al,ah			
	and	al,0fh			;
	jz	setup_5			;no second drive
	mov	al,bl			;
	and	al,01			;This way, CX = 0 if one drive
	or	cl,al			;  CX = 1 if two drives present
	or	bl,1
	or	byte ptr equip_flag,1
setup_5:
	shl	cl,6			;move into correct bits
	or	bl,cl			;
	or	byte ptr equip_flag,cl	;set new bits
	mov	ah,bl			;write new value in cmos
	mov	al,cmos_equip+80h	;
	call	write_cmos		;

;	Turn on floppy interrupts

	in	al,int1+1		;turn on floppy interrupt
	and	al,10111111b		;
	call	delay
	out	int1+1,al			;

;******************************************************************************
;
;     TEST_1B:	CONFIGURE THE HARD DISK
;
;******************************************************************************

TEST_1B:
	MOV	AL,01BH
	mov	dx,diag_port
	not	al
	out	dx,al			;

;TEST FOR VALID STATUS

	MOV     AL,CMOS_STATUS_1+80H
	CALL    READ_CMOS		;GET CMOS STATUS
	TEST    AH,0C0H			;CONFIGURATION VALID ?
	JNZ	TDD7			;cmos bad, skip hd setup

;	CHECK TO SEE IF CONFIGURATION IS CORRECT

TDD2:	MOV     AL,HDISK_TYPE+80H
	CALL    READ_CMOS	      	;AH = HARD DISK TYPE
	TEST    AH,0F0H			;DRIVE # 1 PRESENT ?
	JZ      TDD7		   	;NO, then set int 13h to floppy_io
	call	wd_setup		;initialize fixed disk
	jz	tdd8			;
tdd7:
	mov	ax,offset floppy_io	;floppy disk handler
	mov	di,4ch			;int 13h vector address
	call	move
TDD8:   NOP

page
   
;******************************************************************************
;                                                                             
;     TEST_1C:	CONFIGURE GAME CARD                                           
;                                                                             
;******************************************************************************

TEST_1C:
	MOV	AL,01CH
	call	MakeLed

        MOV     DX,GAME_PORT
        IN      AL,DX                  ;READ GAME CARD
        AND     AL,0FH                 ;TEST DATA = F
        JNZ     TT1
        OR      EQUIP_FLAG,01000H      ;CONFIGURE GAME CARD ON
TT1:    NOP



;******************************************************************************
;
;     TEST_1D:	CONFIGURE 80287
;
;******************************************************************************

TEST_1D:
	MOV	AL,01DH
	call	MakeLed

	xor	bl,bl			;assume no coprocessor
;fninit
	db	0dbh,0e3h		;initialize coprocessor
	xor	ah,ah			;zero ah register and memory byte
	mov	byte ptr serial_base+7,ah
;fnstcw	serial_base+6
	db	0d9h,3eh,6,0		;store coprocessor's control word in memory
	mov	ah,byte ptr serial_base+7
	cmp	ah,03h			;upper byte of control word will be 03 if
					;8087 or 80287 coprocessor is present
	jne	TX2			;jump if no coprocessor

	mov	bl,2			;coprocessor exists
	or	byte ptr equip_flag,bl	;set equip_flag

;ENABLE 80287 INTERRUPT

; ToBu (16) 18.8.88 Ver 3.4  turn on cascade
	in	al,int1+1		
	and	al,11111011b		;
	call	delay
	out	int1+1,al
        IN      AL,INT2+1
        AND     AL,0DFH
        CALL    DELAY
        OUT     INT2+1,AL

;CHECK TO SEE IF CMOS IS VALID 

TX2:    MOV     AL,CMOS_STATUS_1+80H
        CALL    READ_CMOS              ;GET CMOS STATUS    
        TEST    AH,0C0H                ;CMOS VALID ?    
        JZ      TX3                    ;NO, JUMP
        CALL    TX4                    ;STORE NEW CONFIGURATION
        JMP     TX5

;CHECK FOR CONFIGURATION CHANGE AND STORE NEW CONFIGURATION             

TX3:    MOV     AL,CMOS_EQUIP+80H
        CALL    READ_CMOS              ;GET CONFIGURATION FROM CMOS MEMORY
        MOV     BH,AH
        AND     BH,2                                            
        CALL    TX4                    ;STORE NEW CONFIGURATION
        XOR     BH,BL                  ;CONFIGURATION CHANGE
        JZ      TX5                    ;NO, JUMP
        MOV     BL,0C0H
        CALL    BEEP                   ;BEEP THE SPEAKER
;	mov	si,offset warning_msg
;	call	display
        MOV     SI, OFFSET TEST_20_MSG_1
        CALL    DISPLAY                ;SEND WARNING MESSAGE
        JMP     TX5

;SUBROUTINE TO LOAD CONFIGURATION

TX4 PROC NEAR
        MOV     AL,CMOS_EQUIP+80H
        CALL    READ_CMOS
        DEC     AL
        AND     AH,0FDH
        OR      AH,BL
        CALL    WRITE_CMOS
        RET
TX4 ENDP

TX5:    NOP

page

;******************************************************************************
 ;
;     TEST_1E:	CHECK CMOS CLOCK TO SEE IF IT IS WORKING AND IF IT HAS BEEN 
;		INITILIZED
;
;	       IF ITS OK, INITILIZE MEMORY VALUES
;     	       IF IT IS NOT WORKING, SET ERROR         
;     	       IF IT HAS NOT BEEN INITILIZED, SET NEW TIME AND START IT RUNNING
;
;******************************************************************************

TEST_1E:
	MOV	AL,01EH
	call	MakeLed

;CHECK FOR CMOS ERROR
;ENABLE NMI

        MOV     AL,CMOS_STATUS_1
        CALL    READ_CMOS
        TEST    AH,0C0H                	;BATTERY FAIL or checksum error?                   
        JNZ     TJ0                    	;YES, JUMP
        call    chk_clk		; check RTC is working
        jc	     tj0			; if RTC is not working
        JMP     TJ1			; skip

;GET THE MONTH       

TJ0:   
; ToBu (17) 18.8.88 Ver 3.4  turn on cascade
	
        CALL    START_CLOCK		;START THE CLOCK AND CHECK FOR UPDATE
TJ1:    CALL    UPDATE
        JC      TJ2                    ;ERROR, JUMP

;READ MINUTES

        MOV     AL,MINUTES
        CALL    READ_CMOS              ;GET MINUTES
        CALL    CONVERT                ;BCD TO HEX     
        MOV     AL,AH
        XOR     AH,AH
        CMP     AL,59                  ;LEGAL MINUTES ?
        JA      TJ2                    ;NO, JUMP
        PUSH    AX                     ;SAVE MINUTES

;READ SECONDS

        MOV     AL,SECONDS
        CALL    READ_CMOS              ;GET SECONDS
        CALL    CONVERT                ;BCD TO HEX
        MOV     AL,AH
        XOR     AH,AH
        CMP     AL,59                  ;LEGAL SECONDS ?
        JA      TJ2                    ;NO, JUMP
        PUSH    AX                     ;SAVE SECONDS

;READ HOURS   

        MOV     AL,HOURS   
        CALL    READ_CMOS              ;GET HOURS
        CALL    CONVERT                ;BCD TO HEX
        MOV     AL,AH
        XOR     AH,AH
        CMP     AL,23                  ;LEGAL HOURS ?  
        JA      TJ2                    ;NO, JUMP

;CONVERT TO TIMER VALUES AND SAVE

        MOV     TIMER_UPPER,AX         ;HOURS*7   
        POP     AX                     ;RECOVER SECONDS 
        MOV     BL,18
        MUL     BL                                          
        MOV     TIMER_LOWER,AX         ;SECONDS*18
        POP     BX                     ;RECOVER MINUTES
        MOV     AX,1092
        MUL     BX                     ;MINUTES * 1092
        ADD     TIMER_LOWER,AX
        MOV     AX,0
        ADC     TIMER_UPPER,AX
        JMP     TJ3

;CLOCK FAILED--SEND WARNING MESSAGE AND SET ERROR

TJ2:    
        MOV     SI,OFFSET TEST_20A_MSG
        CALL    DISPLAY                ;DISPLAY WARNING MESSAGE
        MOV     BL,0C0H
        CALL    BEEP                   ;BEEP THE SPEAKER
        MOV     AL,CMOS_STATUS_1
        CALL    READ_CMOS
        OR      AH,4                   ;SET CLOCK ERROR
        MOV     AL,CMOS_STATUS_1           
        CALL    WRITE_CMOS
; ToBu (12) 18.8.88 Ver 3.4
        OR	BYTE PTR [EQUIP_FLAG +1],01	;Indicate POST error

;TURN ON THE TIMER INTERRUPT

TJ3:    IN      AL,INT1+1              ;GET THE INTERRUPT MASK
        AND     AL,0FEH                ;ENABLE TIMER INTERRUPT
        CALL    DELAY
        OUT     INT1+1,AL              ;STORE MASK
        STI

page


;******************************************************************************
;
;     TEST_1F:	GENERATE NEW CMOS CHECKSUM AND SAVE IT IN CMOS RAM
;
;******************************************************************************

TEST_1F:
	MOV	AL,01FH
	call	MakeLed

        CALL    CCHECK                 ;GENERATE CHECKSUM
        MOV     AH,BH
        MOV     AL,CHECKSUM_ADR        ;CMOS CHECKSUM LOCATION
        CALL    WRITE_CMOS             ;WRITE HIGH BYTE
        MOV     AH,BL
        CALL    WRITE_CMOS             ;WRITE LOW BYTE

	mov	al,0eh			; diagnostic status byte
	call	read_cmos
	and	ah,3fh
	dec	al
	call	write_cmos

page

;******************************************************************************
;
;     TEST_21:	INITILIZE ROM DRIVERS (INCLUDING HARD DISK)
;
;******************************************************************************

TEST_21:

;TEST # TO LEDS

        MOV     AL,21H
	call	MakeLed

;SETUP SEGMENTS

        MOV     AX,0C800H
        MOV     DS,AX                                     

        MOV     AX,DATA
        MOV     ES,AX

;CHECK FOR ROM PRESENT

TFF0:   XOR     DI,DI
        MOV     AX,0AA55H
        CMP     [DI],AX                ;ROM PRESENT ?        
        JNE     TFF2                   ;NO, JUMP

;DETERMINE THE ROM SIZE

	xor	cx,cx			;64kb on-board ROM
	mov	ax,ds
	cmp	ax,0e000h		;see if on-board ROM
	je	TFF3			;yes, on-board ROM

	xor	ax,ax			;set ax to 0  rlm 2/26/85
        MOV     AL,[DI+2]              ;GET SIZE
        MOV     CX,512
        MUL     CX                     ;MULTIPLE SIZE BY 512
        MOV     CX,AX                  ;CX = SIZE

;DO CHECKSUM ON ROM    

TFF3:	XOR     BL,BL
TFF1:   ADD     BL,[DI]                ;GENERATE CHECKSUM
        INC     DI
        LOOP    TFF1
        CMP     BL,0                   ;CHECKSUM GOOD
        JNE     TFF2                   ;NO, JUMP

;INITILIZE ROM

        PUSH    DS
	push	es
        MOV     ES:ROM_ADDRESS,3H          
        MOV     ES:ROM_SEGMENT,DS      
        CALL    ES:DWORD PTR ES:ROM_ADDRESS  ;INITILIZE THE ROM       

;MOVE BOOTSTRAP VECTOR TO BASIC VECTOR

;	XOR     AX,AX
;	MOV     DS,AX                  ;DS = 0
;	MOV     SI,64H  
;	MOV     DI,60H
;	MOV     AX,[SI]                ;GET BOOTSTRAP VECTOR
;	MOV     [DI],AX                ;BASIC VECTOR = BOOTSTRAP
;	ADD     DI,2
;	ADD     SI,2
;	MOV     AX,[SI]                ;GET BOOTSTRAP SEGMENT
;	MOV     [DI],AX                ;BASIC SEGMENT = BOOTSTRAP
	pop	es			;
	POP     DS    

 ;MOVE TO NEXT ROM    

TFF2:   MOV     AX,DS
        ADD     AX,80H
        MOV     DS,AX                  ;MOVE TO NEXT ROM
        CMP     AX,0E000H              ;ALL ROM'S TESTED
        JBE     TFF0                   ;NO, JUMP

;RESTORE DATA SEGMENT

        MOV     AX,DATA
        MOV     DS,AX


page
;	|-------------------------------------------------------|
;	|							|
;	|	NOW IS THE TIME TO BOOT DOS			|
;	|							|
;	|-------------------------------------------------------|

	MOV	AL,0
	call	MakeLed

; ToBu (12) 18.8.88 Ver 3.4  check for error during power-on
 	TEST	BYTE PTR [EQUIP_FLAG +1],1
	JZ	GO_BOOT
	AND	BYTE PTR [EQUIP_FLAG +1],0FEH
	MOV	SI,OFFSET POST_F1_MSG
	CALL	DISPLAY
WAIT_F1:
	XOR	AH,AH
	INT	16H
	CMP	AH,3BH
	JNE	WAIT_F1
GO_BOOT:
	MOV	AL,0
	call	MakeLed
;	jmp	shutdown_4	;ISSUE BOOT REQUEST

; ToBu (10) 18.8.88 Ver 3.4
	in	al,61h		;
	and	al,11110011b	;set I/O check bit
	out	61h,al		;	
	int	19h
	
; ToBu (3) 12.12.88 Ver 3.5  change sequence to allow Janus Autoboot
 
	INT	19H		;ISSUE BOOT REQUEST

; if Janus Autoboot came back to here, an error happened during boot:
;  send error message to screen and wait for keyboard input to retry

	int	18h

	MOV     SI,OFFSET BOOT_MSG_1
        	CALL    DISPLAY
        	XOR     AX,AX
        	INT     16H                   ;WAIT FOR KEYBOARD INPUT
        	JMP     Go_Boot		;TRY TO BOOT AGAIN
;
MFG_TEST:
	if	LoopBackTest

;******************************************************************************
;                                                                             
;     TEST_22:	PARALLEL LOOPBACK TEST                            
;                                                                             
;******************************************************************************

TEST_22:

;TEST # TO LEDS

        MOV     AL,22H
	not	al
        OUT     DX,AL
;
        MOV     BL,0AAH                ;TEST PATTERN            

;TEST DATA PORT

TN0:    
	MOV	AL,BL			;GET TEST PATTERN
        MOV     DX,PARALLEL
        OUT     DX,AL                  ;WRITE DATA
        CALL    DELAY
        IN      AL,DX                  ;READ DATA          
	MOV	BH,AL			;
 	MOV	DX,DIAG_PORT		;
	MOV	AL,22H			;
	NOT	AL			;
	OUT	DX,AL			;TEST # TO LEDs
	CMP	BH,BL			;DATA ERROR ?
        JE      TN1                    ;NO, JUMP          
        halt                            ;ERROR, HALT         

;TEST CONTROL PORT 

TN1:    ADD     DX,2                   ;DX = CONTROL PORT
        MOV     BH,BL
        AND     BH,1FH                                    
        MOV     AL,BH                  ;GET DATA PATTERN
        OUT     DX,AL                  ;WRITE CONTROL PORT 
        CALL    DELAY
        IN      AL,DX                  ;READ CONTROL PORT
        AND     AL,1FH
        CMP     AL,BH                  ;CONTROL ERROR ?    
        JE      TN2                    ;NO, JUMP        
        halt                            ;ERROR HALT

;TEST STATUS PORT                

TN2:    DEC     DX                     ;DX = STATUS PORT
        IN      AL,DX                  ;READ STATUS
        AND     AL,0F8H
        AND     BL,0F8H
        XOR     BL,38H                 ;INVERT SOME BITS        
        CMP     AL,BL                  ;STATUS ERROR ?
        JE      TN3                    ;NO, JUMP          
        halt                            ;ERROR, HALT

;INVERT DATA AND DO IT AGAIN                              

TN3:    CMP     BH,0AH                 ;DONE ?          
        JNE     TN4                    ;YES, JUMP        
        MOV     BL,055H                ;NEW PATTERN
        JMP     TN0                    ;REPEAT TEST

;TEST PARALLEL INTERRUPT

TN4:    MOV     DI,03CH                ;VECTOR ADDRESS          
        MOV     AX,OFFSET TN7          ;VECTOR
        CALL    MOVE                                 
	MOV     DX,PARALLEL+2		;
        MOV     AL,07FH			;unmask int7 
        OUT     INT1+1,AL               ;ALLOW KEYB INT IN IMR 
        STI				;CPU INTERRUPTS ON
                         		;ecn#158 for FE2000/A
                                      	;add an int7 transition
        MOV     AL,010H			;
        OUT     DX,AL                   ;int 7 on 2000, off 2000a
	AAM				;delay 16 CYCLES
        MOV     AL,014H			;
        OUT     DX,AL                   ;int 7 off 2000, on 2000a
	AAM     			;delay 16 CYCLES
        MOV     AL,010H			;extra transition for FE2000A 
        OUT     DX,AL                   ;int 7 on 2000, off 2000a
					;end ecn#158
        halt                            ;WAIT FOR INTERRUPT

;INTERRUPT SERVICE ROUTINE

TN7     PROC    NEAR
        CALL    END_INTERRUPT
TN7 ENDP

page
                                      
;******************************************************************************
;
;     TEST_23:	SERIAL (8250) TEST
;     		LOOP BACK CONNECTOR TEST.  VERIFIES MODEM CONTROL REGISTER       
;     		LOOPS BACK TO MODEM STATUS REG. VERIFIES TRANSMITTED CHARACTER
;     		LOOPS BACK AS RECEIVED CHARACTER.
;
;******************************************************************************

TEST_23:

;TEST # TO LEDS

        MOV     DX,DIAG_PORT
        MOV     AL,23H
	not	al
        OUT     DX,AL

;SETUP SERIAL ROUTINE    

        MOV     AX,OFFSET SERIAL_IO
        MOV     DI,50H      
        CALL    MOVE                   ;SETUP SERIAL I/O PROGRAM

;TEST COM1	INITILIZE COM1              

        MOV     AX,COM1                                                       
        CALL    TK9                    ;INITILIZE COM1             

;CHECK MODEM CTRL REG BITS 0-3 LOOP BACK TO MODE STATUS REG BITS 4-7 FOR COM1

        MOV     DX,COM1+4
        CALL    TK10                   ;CKECK MODEM LOOP
        JE      TK1                    ;ERROR ?
        halt                            ;YES, HALT

;CHECK TRANSMITT AND RECEIVE FOR COM1      

TK1:    CALL    TK14                   ;TRANSMITT CHARACTER      
        JE      TK3                    ;ERROR ?                    
        halt                            ;YES, HALT

;INTERRUPT TEST FOR COM1

TK3:    MOV     DI,30H                 ;VECTOR ADDRESS
        MOV     AX,OFFSET TK4          ;INTERRUPT ROUTINE ADDRESS   
        CALL    MOVE            
         MOV     DX,COM1+4                        
        MOV     AH,0EFH                ;INTERRUPT MASK
        CALL    TK20                   ;CAUSE INTERRUPT    
        STI
        halt                            ;WAIT FOR INTERRUPT

;COM1 INTERRUPT ROUTINE

TK4 PROC NEAR
        MOV     DX,COM1+1 
        CALL    TK21                   ;DISABLE INTERRUPT                 
TK4 ENDP

;TEST COM2	INITILIZE COM2

        MOV     AX,COM2
        CALL    TK9                    ;INITILIZE COM2

;CHECK MODEM CTRL REG BITS 0-3 LOOP BACK TO STATUS REG BITS 4-7 FOR COM2

        MOV     DX,COM2+4
        CALL    TK10                   ;CHECK MODEM LOOP
        JE      TK5                    ;ERROR ?
        halt                            ;YES, HALT

;CHECK TRANSMITT AND RECEIVE COM2    

TK5:    CALL    TK14
        JE      TK7                    ;ERROR ?
        halt                            ;YES, HALT

;INTERRUPT TEST FOR COM2

TK7:    MOV     DI,2CH                 ;VECTOR ADDRESS
        MOV     AX,OFFSET TK8          ;INTERRUPT ROUTINE ADDRESS
        CALL    MOVE           
        MOV     DX,COM2+4
        MOV     AH,0F7H                ;INTERRUPT MASK
        CALL    TK20                   ;CAUSE INTERRUPT
        STI
        halt                            ;WAIT FOR INTERRUPT

;COM2 INTERRUPT ROUTINE

TK8 PROC NEAR
        MOV     DX,COM2+1
        CALL    TK21                   ;DISABLE INTERRUPT
        JMP     TK22
TK8 ENDP

;SUBROUTINES		SUBROUTINE TO INITILIZE COM PORT

TK9 PROC NEAR 
        XOR     DI,DI
        MOV     [DI],AX                ;MOVE COM ADDRESS TO TABLE
         XOR     DX,DX                  ;SELECT FIRST SERIAL PORT
        MOV     AX,00E3H               ;9600 BAUD,1 STOP,NO PARITY,8 BIT DATA
        INT     14H
        RET
TK9 ENDP

;SUBROUTINE TO CHECK LOOPBACK OF MODEM CONTROL BITS

TK10 PROC NEAR
        MOV     BL,0AH                 ;TEST PATTERN
TK11:   MOV     AL,BL                  ;GET BASE PATTERN
        AND     AL,03H                 ;USE ONLY 0-1
        OUT     DX,AL                  ;WRITE MCR
        ADD     DX,2                   ;POINT TO MSR
        CALL    DELAY
        IN      AL,DX                  ;READ MODE STATUS
        SUB     DX,2                   ;POINT TO MCR
        MOV     CL,4                    
        SHR     AL,CL                  ;LINE UP BITS
        CMP     AL,BL                  ;MATCH ?
        JNE     TK12                   ;NO, JUMP
        XOR     BL,0FH                 ;INVERT MASK
        CMP     BL,0Ah                 ;DONE ?
        JNE     TK11                   ;NO, REPEAT TEST
TK12:   RET
TK10 ENDP

;SUBROUTINE TO TRANSMITT AND RECEIVE CHARACTER

TK14 PROC NEAR
        MOV     AL,0AAH
	XOR	SI,SI
	MOV	DX,[SI]
	out	dx,al			;
	ADD	DX,5			;LSR
TK23:
	CALL	DELAY
	IN	AL,DX
	TEST	AL,1			;DATA READY ?
	JZ	TK23			;NO,TRY AGAIN
	SUB	DX,5
	IN	AL,DX			;GET DATA
	CMP	AL,0AAH			;DATA OK ?
        RET                                          
TK14 ENDP

;SUBROUTINE TO SETUP UART FOR INTERRUPT

TK20 PROC NEAR
        MOV     AL,8
        OUT     DX,AL                  ;ENABLE INTERRUPT
        MOV     AL,AH                  ;GET INTERRUPT MASK
        CALL    DELAY
        OUT     INT1+1,AL               ;LOAD MASK
        DEC     DX                     ;POINT TO LCR
        XOR     AL,AL 
        CALL    DELAY
        OUT     DX,AL                  ;DLAB = 0
        SUB     DX,2                   ;POINT TO INTERRUPT ENABLE REGISTER
         MOV     AL,0FH 
        CALL    DELAY
        OUT     DX,AL                  ;ENABLE INTERRUPT CONDITIONS
        RET
TK20 ENDP

;SUBROUTINE TO END INTERRUPT

TK21 PROC NEAR
        CALL    END_INTERRUPT
        XOR     AL,AL
        OUT     DX,AL                  ;DISABLE INTERRUPT CONDITIONS
        ADD     DX,3                   ;POINT TO MCR
        CALL    DELAY
        OUT     DX,AL                  ;DISABLE INTERRUPT
        RET
TK21 ENDP
;
TK22:   NOP

page

;******************************************************************************
;
;     TEST_24:	TEST CMOS CLOCK CHIP
;
;******************************************************************************

TEST_24:

;TEST # TO LEDS

        MOV     AL,24H
	call	MakeLed

;TEST BATTERY STATUS

        MOV     AL,BATTERY
        CALL    READ_CMOS
        TEST    AH,80H
        JNZ     TQ0 
        halt                            ;BAD BATERY STATUS, HALT

;START CLOCK                          

TQ0:    CALL    START_CLOCK
        MOV     AL,REGISTER_B
        CALL    READ_CMOS
        AND     AH,07FH
        DEC     AL
        CALL    WRITE_CMOS

 ;MAKE SURE THE CLOCK UPDATES

        CALL    UPDATE
        JNC     TQ8                    ;JUMP IF NO ERROR
        halt                            ;HALT, UPDATE STUCK
TQ8:	mov	bl,5
TQ1:    XOR     CX,CX
        MOV     AL,REGISTER_A
TQ2:    CALL    READ_CMOS
        TEST    AH,80H                 ;UPDATE ?
        JNZ     TQ3                    ;YES, JUMP
        DEC     AL                     ;AL = REGISTER_A
        LOOP    TQ2                    ;TRY AGAIN
	dec	bl
	jnz	TQ1			;10/10/1986 jwy, increment loop
        halt                            ;ERROR, CLOCK NOT COUNTING

;TEST CMOS MEMORY	WRITE CMOS MEMORY

TQ3:    XOR     BL,BL                  ;BASE PATTERN
TQ4:    MOV     AL,0EH                 ;STARTING CMOS ADDRESS
        MOV     CX,25H                 ;MEMORY SIZE
        MOV     AH,BL                   
TQ5:    CALL    WRITE_CMOS             ;WRITE PATTERN
        INC     AH
        LOOP    TQ5                    ;REPEAT UNTIL ALL MEMORY IS WRITTEN

;READ AND CHECK CMOS MEMORY

        MOV     BH,BL                  ;BASE PATERN
        MOV     AL,0EH                 ;CMOS STARTING ADDRESS
        MOV     CX,25H                 ;MEMORY SIZE
TQ6:    CALL    READ_CMOS              ;READ CMOS MEMORY
        CMP     AH,BH                  ;ERROR ?
        JE      TQ7                    ;NO, JUMP
        halt                            ;MEMORY ERROR, HALT
TQ7:    INC     BH                      
        LOOP    TQ6                    ;REPEAT UNTIL ALL MEMORY IS READ

;REPEAT MEMORY TEST

        XOR     BL,0FFH                ;COMPLEMENT THE PATTERN
        JNZ     TQ4
        

page

;******************************************************************************
;
;   TEST_25:	CAUSE A SHUTDOWN FOR MANUFACTURING TEST LOOP
;
;******************************************************************************

TEST_25:

;TEST # TO LEDS

        MOV     AL,25H
	call	MakeLed
;
        XOR     AH,AH          
        CALL    SHUT     
	call	shut_down		;do a shutdown and return to the
	halt				;beginning of diagnostics

        endif

page

;******************************************************************************
;
;	TEST_26:	TEST MEMORY OVER 1 MBYTE
;		CALL TO THIS ROUTINE IN MADE FROM TEST_9
;		FIRST CHECK IF WE DO HAVE MEMORY OVER 1 MBYTE BY CHCKING THE
;		VALUE OF MEMORY_SIZE VARIABLE
;
;******************************************************************************

;	define some equates used in this routine

pod_ss		equ	0		;stack for pod
pod_sp		equ	9000h
temp_stk_low	equ	0ffffh		;stack in virtual mode
temp_stk_hi	equ	0		;
gdt_loc		equ	0d000h		;location of GDT in ram
access_rights	equ	93h		;
pod_idt_loc	equ	0d100h		;
pod_idt_len	equ	256*8		;256 entries of 8 bytes each
trap_gate	equ	87h		;trap gate for exception interrupts
code_rights	equ	8

TEST_26:

	mov	al,26h	
	call	MakeLed

	mov	vir_mem_size,0		;initialize value to 0
	mov	ax,memory_size		;get real memory size
	cmp	ax,512			;is it 640 or above
	jae	test_26_a		;if equal or great, then do more testing
	jmp	test_26_rtn		;otherwise go to TEST_B
	
test_26_a:
	mov	si,offset test_26_msg_1		;print message on the screen
	call	display

;
;	setup return to shutdown_2 after completion of this test
;

 	mov	ah,2
	call	shut

;
;	now build the exception interrupt vector tables. Move the table
;	from ROM into Ram
;
	cld				;set direction
	mov	si,offset sys_idt_start		;start of table
	mov	cx,sys_idt_len		;# of words to move
	shr	cx,1
	mov	di,pod_idt_loc		;where it will move in RAm
	mov	ax,cs			;
	mov	ds,ax			;setup data register
	mov	ax,0
	mov	es,ax			;setup destination segment

	rep	movsw			;move

;
;	now move rest of the vectors 17 - 255
;
	push	bp
	mov	cx,256-17		;rest of the vectors
	mov	bp,offset null_idt	;

test_26_b:
	mov	si,bp
	push	cx
	mov	cx,4			;# of words to move
	
	rep	movsw			;move 4 words

	pop	cx			;
	loop	test_26_b		;repeat till all done


;
;	Now load the Interrupt descriptor register in CPU. First build
;	the descriptor.
;
	mov	bp,di			;save di register
	mov	ax,pod_idt_len		;get table length
	stosw
	mov	ax,pod_idt_loc		;get start
	stosw				;
	xor	ax,ax			;3/11/85
	stosw				;access rights

	db	26h,0fh,01,5eh,00	;hard code `LIDT [bp]' instruction



;
;	Now build Global Descriptor Table in RAM by copying it from ROM
 ;

test_26_c:
	mov	di,gdt_loc		;
	mov	si,offset start_gdt	;
	mov	cx,gdt_len		;# of words to move
	shr	cx,1
	rep	movsw			;move

;
;	Now load Global Descriptor register in CPU. First build descriptor
;	in RAM
;

	mov	bp,di
	mov	ax,gdt_len
	stosw
	mov	ax,gdt_loc
	stosw
	xor	ax,ax			;3/11/85
	stosw

	db	26h,0fh,01,56h,00	;hard code `LGDT [bp]' instruction

	mov	di,bp			;restore di
	pop	bp			;restore bp

;
;	Now switch CPU to virtual mode; first make sure that A20 line
;	is under CPU control
;

test_26_d:

	mov	al,0DFh			;
	call	gate_A20		;	2/25/85  rlm
	
	mov	ax,1			;load machine status word
	
	db	0fh,01,0f0h		;hard code `LMSW AX' instruction


;	Make a Far jump to empty cpu pre-fetch queue

	db	0eah
	dw	(offset test_26_e)
	dw	bios_cs_entry

test_26_e:
	
	mov	ax,gdt_entry
	mov	ds,ax
	mov	word ptr ds:(temp_ss_entry+2),0	;initialise stack segment
	mov	byte ptr ds:(temp_ss_entry+4),0
	mov	ax,temp_ss_entry
	mov	ss,ax
	mov	sp,-3

page

 
;
;	Now is the real start of TEST_26. First find out if we really
;	are in virtual mode. If true then start testing memory from 1MB
;	because lower 640k has already been tested.
;

test_26_f:

	db	0fh,01,0e0h		;get machine status word in ax
	test	ax,1			;
	jnz	test_26_g		;yes in virtual mode


;	Not in virtual mode. Error, do a shutdown and halt

	mov	ah,3			;do a shutdown 3
	call	shut
	mov	si,offset shut_msg_3
	call	display			;print message
	jmp	shut_down

shutdown_3:
	mov	dx,diag_port
	mov	al,0f3h	
	not	al			;light up LEDs
	out	dx,al			;
	nop				;delay
	halt				;stop CPU
	jmp	shutdown_3		;make sure it does that
						


test_26_g:

	mov	word ptr ds:(temp_es_entry+2),0	;set es register entry in GDT to point
	mov	byte ptr ds:(temp_es_entry+4),10h	;to 1 MB memory
	mov	dx,0

test_26_i:
	mov	ax,temp_es_entry		;point es to correct
	mov	es,ax			; GDT entry
	mov	bx,0			;set pattern map

test_26_k:
	mov	cx,8000h			;check 64k bytes at a time
	mov	di,0			;start from location 0

test_26_h:
	mov	ax,bx
	xor	ax,di			
	stosw				;write a pattern in memory
	loop	test_26_h		

 ;	now read and varify memory contents

	mov	cx,8000h
	mov	di,0

test_26_j:
	mov	ax,bx
	xor	ax,di
	scasw				;compare ax with es:di
	jne	test_26_z			;do not compare, end of memory
	loop	test_26_j			;repeat test 32k times

;	now invert the pattern and repeat test 1 more time
	
	xor	bx,0ffffh		
	jnz	test_26_k

;	now do next 64k bytes. But first zero out memory

	mov	di,0			
	mov	ax,0
	mov	cx,8000h
	rep	stosw

	add	dx,64			;keep track of # of kB tested
	inc	byte ptr ds:(temp_es_entry+4)
	cmp	byte ptr ds:(temp_es_entry+4),239	;are we at top
	jnz	test_26_i				;  of 16 Mbyte?
						;no -> go do more

;
;	End of memory test. Save value in low ram
;

test_26_z:
	push	ds
	mov	ax,dataa_entry		;
	mov	ds,ax			;point ds to local storage
	mov	vir_mem_size,dx		;save size in low memory
	pop	ds			;

;
;	Now test address lines 19 through 23. Write a pattern at location
;	5A5Ah then write it again at locations 1Mb, 2Mb ... 16 Mb and see 
;	if the pattern changed or not. If changed then error, shutdown the
;	system.
;

	mov	word ptr ds:(temp_es_entry+2),0
	mov	byte ptr ds:(temp_es_entry+4),0
	mov	dx,0a5a5h			;set pattern
	mov	ax,temp_es_entry
	mov	es,ax			;save it
	mov	di,5a5ah			;write a5a5 at loc 5a5a
	mov	es:[di],dx		;
	mov	bl,8
 	mov	cx,4

test_26_y:
	mov	byte ptr ds:(temp_es_entry+4),bl
	mov	ax,temp_es_entry
	mov	es,ax
	call	test_26_x
	shl	bl,1			;next segment
	loop	test_26_y
	jmp	shut_down			;go do a shutdown and return

test_26_x	proc	near
	mov	ax,0ffffh			;
	mov	es:[di],ax		;write a different pattern
	mov	byte ptr ds:(temp_es_entry+4),0
	mov	ax,temp_es_entry
	mov	es,ax			;read word in low memory
	mov	ax,es:[di]
	cmp	ax,dx			;are they same
	jz	test_26_x1		;yes,then we are ok

; ToBu (4) 12.12.88 Ver 3.5  report address error
 	push	ds			;we got problems
	mov	ax,dataa_entry		;point ds to local storage
	mov	ds,ax			
	mov	vir_mem_size,0		;adjust protected mem
	pop	ds			; size to zero

	mov	ah,3			;do a shutdown 3
	call	shut
	mov	si,offset shut_msg_4
	call	display			;print message
	jmp	shut_down

test_26_x1:
	ret
test_26_x	endp
	
	
;
;	come here in real mode. setup stack and data segments
;

shutdown_2:
	mov	sp,stack_seg		;
	mov	ss,sp
	mov	sp,top_of_stack
	mov	ax,data
	mov	ds,ax

;
;	display virtual memory size, first lower A20 line
;

test_26_rtn:
	mov	al,0DDh			;	rlm  2/25/85
	call	gate_A20		;
	
	mov	si,offset test_26_msg_2
	call	display
	mov	ax,vir_mem_size		;convert it for printing
	add	ax,memory_size			;
	call	convt_bin_dec		;convert binary to decimal and display
	mov	si,offset test_26_msg_3
	call	display
test_26_v:
	jmp	test_b			;return to caller



null_vector	label	word
 	mov	dx,diag_port
	mov	al,0ffh
	not	al
	out	dx,al			;put code in LEDs
	iret				;dummy return


MakeLed	proc	near

        MOV     DX,DIAG_PORT
;	not	al
        OUT     DX,AL
	ret
MakeLed	endp
page

;
;	Define service routines for exception interrupts. First put code
;	in the LEDs and then issue a halt
;

excep_int_00:
	mov	al,80h			;put code in LEDs
	jmp	short exception_int	;go service it

excep_int_01:
	mov	al,81h
	jmp	short exception_int

excep_int_02:
	mov	al,82h
	jmp	short exception_int

excep_int_03:
	mov	al,83h
	jmp	short exception_int

excep_int_04:
	mov	al,84h
	jmp	short exception_int

excep_int_05:
	mov	al,85h
	jmp	short exception_int

excep_int_06:
	mov	al,86h
	jmp	short exception_int

excep_int_07:
	mov	al,87h
	jmp	short exception_int

excep_int_08:
	mov	al,88h
	jmp	short exception_int

excep_int_09:
	mov	al,89h
	jmp	short exception_int

excep_int_10:
	mov	al,8ah
 	jmp	short exception_int

excep_int_11:
	mov	al,8bh
	jmp	short exception_int

excep_int_12:
	mov	al,8ch
	jmp	short exception_int

excep_int_13:
	mov	al,8dh
	jmp	short exception_int

excep_int_14:
	mov	al,8eh
	jmp	short exception_int

excep_int_15:
	mov	al,8fh
	jmp	short exception_int

excep_int_16:
	mov	al,90h

exception_int:
	mov	dx,diag_port
	out	dx,al
	not	al
	nop
	halt				;stop  cpu


page

;	define pre-initialized global descriptor table


START_GDT	label	word

dummy_entry	equ	0
	dw	0			;segment limit
	dw	0			;segment base address,low word
	db	0			;segment base address, high byte
	db	0			;access rights
	dw	0			;reserve word


gdt_entry	equ	dummy_entry+8		;entry of this table

	dw	gdt_len			;length of this table
	dw	gdt_loc			;location of this table in ram
	db	0			;high byte address
	db	access_rights		;access rights
 	dw	0			;reserved


idt_entry	equ	gdt_entry+8	;entry for exception interrupt vectors

	dw	pod_idt_len		;
	dw	pod_idt_loc		;
	db	0			;
	db	access_rights		;
	dw	0			;


dataa_entry	equ	idt_entry+8	;entry for system data areas

	dw	300h			;size of the areas
	dw	400h			;address of this area, low word
	db	0			;high byte
	db	access_rights		;
	dw	0			;reserved


bios_cs_entry	equ	dataa_entry+8	;entry for rom bios code

	dw	-1			;length is 64k
	dw	0			;
	db	0fh			;segment is F0000
	db	access_rights+code_rights		;
	dw	0			;


temp_es_entry	equ	bios_cs_entry+8	;temperory entry for es register

	dw	-1
	dw	0
	db	0
	db	access_rights
	dw	0


temp_cs_entry	equ	temp_es_entry+8	;temperory entry for cs register
	
	dw	-1
	dw	0
	db	0
	db	access_rights
	dw	0


temp_ds_entry	equ	temp_cs_entry+8

	dw	-1
	dw	0
	db	0
	db	access_rights
	dw	0
 

temp_ss_entry	equ	temp_ds_entry+8

	dw	-1
	dw	0
	db	0
	db	access_rights
	dw	0

END_GDT	label	word

gdt_len	equ	(end_gdt - start_gdt)	


page

;	
;	define Interrupt Descriptor Table (IDT) for POD
;

sys_idt_start	label	word

	dw	excep_int_00		;exception interrupt 0: divide error
	dw	bios_cs_entry		;located in ROM bios
	db	0			;
	db	trap_gate		;
	dw	0

	dw	excep_int_01		;exception interrupt 1:single step
	dw	bios_cs_entry		;
	db	0
	db	trap_gate		;
	dw	0			;

	dw	excep_int_02		;exception interrupt 2: NMI
	dw	bios_cs_entry
	db	0
	db	trap_gate
	dw	0

	dw	excep_int_03		;exception interrupt 3: Breakpoint
	dw	bios_cs_entry		;
	db	0
	db	trap_gate		;
	dw	0

	dw	excep_int_04		;exception interrupt 4: INTO detect
	dw	bios_cs_entry		;
	db	0
	db	trap_gate
	dw	0

	dw	excep_int_05		;exception interrupt 5: Bound
	dw	bios_cs_entry		;
 	db	0
	db	trap_gate		;
	dw	0

	dw	excep_int_06		;exception interrupt 6: Invalid opcode
	dw	bios_cs_entry
	db	0
	db	trap_gate
	dw	0

	dw	excep_int_07		;exception interrupt 7:
	dw	bios_cs_entry
	db	0
	db	trap_gate
	dw	0

	dw	excep_int_08		;exception interrupt 8: double exception
	dw	bios_cs_entry
	db	0
	db	trap_gate		;
	dw	0

	dw	excep_int_09		;exception interrupt 9:processor segment error
	dw	bios_cs_entry		;
	db	0			;
	db	trap_gate		;
	dw	0

	dw	excep_int_10		;exception interrupt 10:
	dw	bios_cs_entry		;
	db	0
	db	trap_gate		;
	dw	0			;

	dw	excep_int_11		;exception interrupt 11:segment not present
	dw	bios_cs_entry		;
	db	0
	db	trap_gate		;
	dw	0

	dw	excep_int_12		;exception interrupt 12: stack seg not present
	dw	bios_cs_entry		;
	db	0			;
	db	trap_gate		;
	dw	0			;

	dw	excep_int_13		;exception interrupt 13: general protection error
	dw	bios_cs_entry		;
	db	0
	db	trap_gate
	dw	0			;


	dw	excep_int_14		;exception interrrupt 14:
	dw	bios_cs_entry
 	db	0
	db	trap_gate
	dw	0

	dw	excep_int_15		;
	dw	bios_cs_entry
	db	0
	db	trap_gate
	dw	0

	dw	excep_int_16		;exception interrupt 16: processor extension error
	dw	bios_cs_entry		;
	db	0			;
	db	trap_gate		;
	dw	0			;

sys_idt_end	label	word

sys_idt_len	equ	(sys_idt_end - sys_idt_start)


;
;	define entry for exception interrupts from 16 till 256
;


null_idt	label 	word

	dw	null_vector
	dw	bios_cs_entry
	db	0
	db	86h			;
	dw	0


page

;
;	Shut_down:   Routine to send shutdown command to 8042 and halt
;		     the CPU. Return is made to location depending on
;		     shutdown code in cmos memory
;

shut_down:
;	mov	ah,1			;wait
;	call	wait_upi
	mov	al,0FEh			;shutdown command
	out	upi_stat_port,al	;
	cli
shut_down1:
	hlt
	jmp	shut_down1		;make sure it does


page

;******************************************************************************
;
;     			ERROR MESSAGES
;
;******************************************************************************

TEST_6B_MSG_1 LABEL BYTE     

	DB "Ram test  128K", -1
	DB  BS, BS, BS, BS            
	DB  "256K", -1

	DB  BS, BS, BS, BS             
	DB  "384K", -1

	DB  BS, BS, BS, BS             
	DB  "512K", -1

	DB  BS, BS, BS, BS            
	DB  "640K", -1

TEST_6B_MSG_2 LABEL BYTE

	DB  " OK",CR, LF, -1

TEST_6C_MSG_1  LABEL BYTE     

	DB "DMA 1 error ", CR, LF, -1

TEST_7_MSG_1   LABEL BYTE     

	DB "DMA 2 error ", CR, LF, -1

TEST_8_MSG_1   LABEL BYTE     

	DB "Interrupt controller 1 error", CR, LF, -1

TEST_8A_MSG_1  LABEL BYTE     

	DB "Interrupt controller 2 error", CR, LF, -1

TEST_8B_MSG_1 LABEL BYTE

	DB "PIO error", CR, LF, -1

TEST_9_MSG_1   LABEL BYTE     

	DB "Parity error", CR, LF, -1

TEST_9A_MSG_1  LABEL BYTE     

	DB "Battery failure", CR, LF, -1

TEST_13_MSG_50  LABEL BYTE     

 	DB "Keyboard error 50", CR, LF, -1

TEST_13_MSG_51  LABEL BYTE     

 	DB "Keyboard error 51", CR, LF, -1

TEST_13_MSG_52  LABEL BYTE     

 	DB "Keyboard error 52", CR, LF, -1

TEST_13_MSG_53  LABEL BYTE     

 	DB "Keyboard error 53", CR, LF, -1

TEST_13_MSG_2  LABEL BYTE     

	DB "ToBu is watching you ...", CR, LF, -1

TEST_16_MSG_1 LABEL BYTE
	DB "0-640K memory configuration error.  New configuration loaded."
	DB  CR,LF,-1

TEST_16A_MSG_1 LABEL BYTE
	DB "Over 1MB memory configuration error.  New configuration loaded."
	DB CR,LF,-1

TEST_17_MSG_1  LABEL BYTE     

	DB "Key switch is off.  Turn it on to continue.", CR, LF, -1

TEST_18_MSG_1 LABEL BYTE
	
	DB "Floppy configuration error." ; Default configuration loaded."
	DB  CR,LF, -1

drv0_config_msg	label	byte
	db	'Drive 0 configuration error',cr,lf,-1

drv0_default_msg	label	byte
	db	'Default configuration (1.2MByte) loaded',cr,lf,-1

drv1_config_msg	label	byte
	db	'Drive 1 configuration error',cr,lf,-1

TEST_20_MSG_1 LABEL BYTE
	DB "Coprocessor (80287) configuration error. "
	DB "New configuration loaded", CR, LF, -1


TEST_20A_MSG LABEL BYTE
	DB "Clock not initialized.", CR, LF, -1


test_26_msg_1	label	byte
	db	'Testing memory over 1 MB ',-1

test_26_msg_2	label	byte
	db	cr,lf,'Total Memory =',-1


test_26_msg_3	label	byte
	db	' kbytes ',cr,lf,-1


ill_shut_msg	label	byte
	db	CR,LF,'Illegal Shutdown Code in CMOS',-1

shut_msg_3	label	byte
	db	cr,lf,'Virtual Mode CPU error',-1

shut_msg_4	label	byte
	db	cr,lf,'Address error in extended memory',-1

POST_F1_MSG	LABEL BYTE
	DB	cr,lf,'Press <Ctrl-Alt-Esc> to run SETUP or press F1 key to continue',cr,lf,-1 



page


;************************** Please take note of this ******************
;
;	size of this module must be  1805H

	IF (BEGIN-$)
empty_space	db	BEGIN-$ dup (0)
	ENDIF

end_pod	LABEL BYTE

eproma	ends	
	end
 
