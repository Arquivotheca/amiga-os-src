
page 60,132
include	title.inc
subttl	INT 40H   Floppy Driver for AT

.xlist
include externals.inc
include equates.inc

public	floppy_setup
public	disk_parm
public	floppy_intr			;1/9/85
public	recal_opn			;1/9/85
public	floppy_io
.list

zero	segment	at 0h
zero	ends

DATAA	SEGMENT	AT 40H
DATAA	ENDS


EPROMA	segment	  byte public

ASSUME CS:EPROMA,DS:DATAA,ES:DATAA

;	Start of Main Code............


.xlist
start_of_int40	equ	$ + 835h
.list
page
;******************************************************************************
;
; 		Floppy_IO    INT 40H                      
;
;     THIS ROUTINE HANDLES ALL THE 5 1/4" DISKETTE OPERATIONS VIA
;     THE FLOPPY DISK CONTROLLER (FDC).
;               
;     INPUT TO ROUTINE :
;      AH = COMMAND TO PERFORM
;       
;       0 - RESET DISKETTE, SET RECALIBRATE REQUIRED ON ALL DRIVES
;       1 - READ PREVIOUS DISKETTE OPERATION STATUS
;       2 - READ SPECIFIED SECTORS INTO MEMORY
;       3 - WRITE THE SPECIFIED SECTORS FROM MEMORY
;       4 - VERIFY THE SPECIFIED SECTORS
;       5 - FORMAT THE SPECIFIED TRACK
;      15 - read disk type
;      16 - check disk change status line
;
;      AL = NUMBER OF SECTORS
;       
;      FOR READ,WRITE,VERIFY,FORMAT
;      BX = OFFSET OF BUFFER , ES = SEGMENT OF BUFFER
;      CH = TRACK NUMBER (0-39)
;      CL = SECTOR NUMBER (1-8)
;      DL = DRIVE NUMBER (0-3)
;      DH = HEAD NUMBER (0-1)
;      DSK_PARM_PTR  - POINTER TO DISK PARAMETER TABLE
;
;
;     OUTPUT FROM ROUTINE :
;      AH = OPERATION STATUS
;        
;       01 - BAD COMMAND PASSED TO ROUTINE
;       02 - ADDR MARK NOT FOUND ON TRACK
;       03 - WRITE ATTEMPTED ON WRITE PROTECTED DISK
;       04 - REQUESTED SECTOR NOT FOUND
;       08 - DATA OVERRUN
;       09 - ATTEMPTED TO CROSS 64K BOUNDARY
;       10 - CRC ERROR ON READING
;       20 - FDC FAILURE
;       40 - SEEK OPERATION ERROR
;       80 - FDC TIMEOUT ERROR 
;
;      AL = NUMBER OF SECTORS ACTUALLY TRANSFERRED
;           (NOT VALID IF TIMEOUT ERROR OCCURS)
;      CARRY FLAG = SUCESS/FAILURE INDICATION
;       0 - SUCESSFULL OPERATION
;       1 - FAILURE OCCURED ( SEE AH FOR FAILURE INFO)
; 
;          
;     OTHER REGS DESTROYED :
;      NONE
;
;     OTHER INFORMATION:
;     FORMAT OPERATION REQUIRES THAT THE BUFFER CONTAIN THE NECESSARY
;     ID FEILD INFORMATION FOR EACH SECTOR OF THE TRACK.
;       
;******************************************************************************

Floppy_IO	PROC    FAR            
	STI                             ;RENABLE INTERRUPTS
        PUSH    AX
	call	save_all		;save all registers
	cmp	ah,6			;check command
	jb	D3			;valid command
	cmp	ah,15h			;
	jb	disk_io_2		;bad command
D3:					;label moved          12/14/84    rlm
	cmp	dl,1			;check for valid drive code
	ja	disk_io_2
	jmp	D2			;go execute command
  
disk_io_2:
        MOV     DSK_OPN_STATUS,CMD_ERR 	;SET CMD ERROR IN STATUS
        XOR     AL,AL                  	;SET NUMBER OF SECTORS XFRD TO 0
        JMP     DSK_OPN_DONE

;	determine media type and reset disk change line here

Branch	proc	near			
	cmp	ah,1			;
	jbe	disk_io_1		;
	cmp	ah,15h			;
	jae	disk_io_1		;			
	call	read_Dsk_line		;go read disk change line
;
; ----- Rel. 3.3
;
;	pushf				;save flags
;	cmp	hf_cntrl,0		;test for controller card
;	jz	disk_io_6		;diskette adapter, ignore change line
;	popf				;get flags back
	jz	disk_io_7
	jmp	disk_io_8		;go handle disk change line

disk_io_7:
	call	get_state		;get current state
	cmp	al,0			;no information available
	jnz	disk_io_9		;
	mov	al,flp_dd_dd		;set default 360/360 state
	call	set_state		;go set state 
disk_io_9:
	cmp	al,default		;check state with power on default
	jne	disk_io_10		;
	mov	cx,[bp+bp_cx]		;get input value of cx
	cmp	ch,40			;check 1.2M diskette (80 tracks)
	jb	disk_io_10
	mov	al,2			;
	call	set_state		;go set state
	push	ax
	call	get_op_state		;get previous operation
	mov	bl,al			;put it in bl
	pop	ax			;get al back
	or	bl,bl			;
	jnz	disk_io_11		;
	mov	ah,al			;
	mov	al,default		;set default starting state
	call	set_op_state		;
	mov	al,ah			;
	jmp	short disk_io_11	;
disk_io_10:
	push	ax			;
	call	get_op_state		;
	or	al,al			;see if first time through or retry
	pop	ax			;
	jnz	disk_io_11		;retry
	call	set_op_state		;go set state for retry
disk_io_11:
	mov	bl,data_rate		;get data rate
	cmp	al,bl			;are they same
	je	disk_io_12		;yes,do not reset it
	mov	data_rate,al		;
	mov	cl,2			;get transfer rate in low bits
	rol	al,cl			;
	and	al,3			;get rid of unwanted bits
	mov	dx,fcr			;get floppy control register
	out	dx,al			;set transfer rate
disk_io_12:
	mov	ax,[bp+bp_ax]		;restore some registes
	mov	bx,[bp+bp_bx]		;
	mov	cx,[bp+bp_cx]		;
	mov	dx,[bp+bp_dx]		;
	jmp	short disk_io_1		;
disk_io_6:
	popf				;get rid of flags
disk_io_1:
        MOV     DH,AL                  	;SAVE NUMBER OF SECTORS IN DH
        AND     MOTOR_ON,7FH           	;CLEAR WRITE BIT
        MOV     AL,DSK_OPN_STATUS      	;SAVE DSK_OPN_STATUS TEMPORARILY IN AL
        MOV     DSK_OPN_STATUS,0       	;CLEAR STATUS
	push	ax			;
	cmp	ah,5			;check for valid command
	jbe	disk_io_16
	sub	ah,15h			;
	jc	disk_io_17		;
	add	ah,6
	jmp	short disk_io_16
disk_io_17:
	mov	dsk_opn_status,CMD_ERR	;return as bad command error
	pop	ax
	ret
;	jmp	disk_io_14		;pop ax at ths point
disk_io_16:
	shl	ah,1			;
	mov	al,0			;
	xchg	al,ah			;
	cmp	al,D1length		;
	jae	disk_io_17		;pcdos3.2 fix rj 7/7/86
	mov	si,offset D1		;get address of the table
	add	si,ax			;
	pop	ax			;
        jmp	WORD PTR CS:[si]	;jmp to appropriate routine
branch	endp				;

;
D2:	call	branch			;

DSK_OPN_DONE:
	push	ax			;save status of the command
	mov	ax,[bp+bp_ax]		;get input command back
	mov	dx,[bp+bp_dx]		;get drive code
;
; ----- Rel. 3.3
;
;	cmp	hf_cntrl,0		;check controller type
;	je	disk_io_3		;floppy adapter
	cmp	ah,2			;reset or read status command
	jb	disk_io_3		;
	cmp	ah,5			;
	ja	disk_io_3		;
	mov	al,dsk_opn_status	;any error during last operation
	or	al,al			;
	jz	disk_io_20
	cmp	al,6			;media change
	jne	retry_io_1		;
	call	read_dsk_line		;go read disk change status line
	jz	disk_io_4		;
	mov	dsk_opn_status,80h	;set time out status
	jmp	disk_io_4		;
disk_io_20:
	call	get_state		;
	test	al,10h			;see if media/drive is established
	jnz	disk_io_4		;yes
	add	al,13h			;set state as established
	call	set_state		;
disk_io_4:
	push	ax			;
	mov	al,0			;
	call	set_op_state		;
	pop	ax
disk_io_3:
        MOV     BX,2                   	;GET MOTOR WAIT PARAMETER
        CALL    GET_PARAMETER
        MOV     MOTOR_TIMOUT,AH         ;KEEP MOTOR RUNNING FOR ABOUT 1.4 SECS
	mov	ax,[bp+bp_ax]		;
	cmp	ah,15h			;
	jne	disk_io_14		;
	pop	ax			;
	xchg	al,ah			;
	clc				;
	jmp	short disk_io_15	;no error return
disk_io_14:
	pop	ax			;
        MOV     AH,DSK_OPN_STATUS      	;DISK STATUS INTO AH
        cmp	AH,1
        cmc
disk_io_15:
	mov	[bp+bp_ax],ax		;save ax on the stack
	call	restore_all		;restore all registers
	pop	ax			;and ax
        RET     2                      	;DISCARD FLAGS

retry_io:

retry_io_1:
	call	get_state		;get current state of the drive
	test	al,established		;is the state established
	jnz	disk_io_4		;yes, then true error, no retry
	and	al,7			;isolate state bits
	cmp	al,2			;
	jb	retry_io_2		;
	mov	al,-1			;set state to 0 
retry_io_2:
	inc	al			;try next state
	push	ax
	call	get_op_state		;get retry state
	mov	bl,al			;
	and	bl,7			;isolate states
	pop	ax			;
	cmp	al,bl			;all states tried
	je	retry_io_3		;yes,then set state and return error
	mov	ah,al			;save current state
	call	get_state		;
	and	al,0c0h			;isolate tranfer rate
	jnz	retry_io_4		;if not zero,skip
	mov	al,80h			;if zero then set rate to 250kb
	jmp	short retry_io_5	;
retry_io_4:
	shr	al,1			;decrement to next xrate
retry_io_5:
	and	al,0c0h			;get rid of unwanted bits
	cmp	ah,1			;double step rate required
	jne	retry_io_6		;no
	or	al,20h			;set double rate bit
retry_io_6:
	or	al,ah			;combine states
	call	set_state		;go set it
	pop	ax			;clean up stack for retry
	mov	ax,[bp+bp_ax]		;restore registers for retry
	mov	bx,[bp+bp_bx]		;
	mov	cx,[bp+bp_cx]		;
	mov	dx,[bp+bp_dx]		;
	jmp	D2			;and go retry
retry_io_3:
	mov	al,80h			;
	call	set_state		;
	jmp	disk_io_4

Floppy_IO	ENDP

disk_io_8:
	mov	al,default		;handle disk change line
	call	set_state		;
	call	dsk_reset		;go reset the NEC
	mov	dx,[bp+bp_dx]		;
	mov	ch,01			;seek to track 1
	push	dx
	call	seek_opn		;
	pop	dx			;
	mov	ch,0			;
	call	seek_opn		;
	mov	dsk_opn_status,6	;set status to media change
	ret				;

page


;******************************************************************************
;
; DSK_RESET                            
;     THIS ROUTINE HANDLES THE RESET COMMAND. THE FDC IS
;     IS ISSUED A HARD RESET.  A SPECIFY COMMAND  WITH
;     SRT,HUT,HLT,ND (STEP RATE TIME, HEAD UNLAOD TIME,
;     HEAD LOAD TIME, DMA MODE) PARAMETERS ARE SENT TO THE FDC.
;     THE PARAMETERS ARE OBTAINED FROM THE DISK PARAMETER BLOCK.
;               
;
;******************************************************************************
;       
DSK_RESET       PROC    NEAR           
        CLI                            ;DISABLE INTERRUPTS
        MOV     bl,MOTOR_ON            ;GET MOTOR ON STATUS
        MOV     CL,4                   ;SETUP SHIFT COUNT
	ror	bl,cl			;
	and	bl,0f3h			;
	mov	al,bl			;
        OR      AL,8                   ;SET "ENABLE INT & DMA " BIT
        MOV     DX,DOR                 ;SETUP PORT ADDR FOR DIGITAL OUTPUT REG
        OUT     DX,AL                  ;OUTPUT TO DIGITAL OUT REG(DOR)
        MOV     RECAL_REQD,0           ;SET RECAL REQUIRED ON ALL DRIVES
	mov	dsk_opn_status,0	;
	call	nop_proc		;delay
        OR      AL,4
        OUT     DX,AL                  ;TURN OFF RESET IN DOR
        STI                            ;REENABLE INTERRUPTS
        CALL    WAIT_INTR              ;WAIT FOR INTERRUPT
        JC      D10                    ;JMP IF ERROR
        CALL    ANALYZE_RESULT_1       ;ANALYZE RESULTS OF THE OPERATION
D10:    CMP     FDC_RESULTS,0C0H
        JZ      D11
        OR      DSK_OPN_STATUS,FDC_ERR ;SET FDC ERROR
        RET
D11:    MOV     AH,03H                  
        CALL    FDC_OUT                ;OUTPUT SPECIFY CMD TO FDC
        XOR     BX,BX                  ;POINT TO PARAMETER
        CALL    OUT_PARAMETER          ;OUTPUT SRT,HUT PARAMETERS TO FDC
        INC     BX                     ;POINT TO NEXT PARAMETER
        CALL    OUT_PARAMETER          ;OUTPUT HLD,ND PARAMETERS TO FDC
        RET
DSK_RESET       ENDP
;

                                        
;******************************************************************************
;
; DSK_STATUS        
;     THIS ROUTINE HANDLES THE STATUS COMMAND.
;     IT JUST RESTORES THE PREVIOUS DSK_OPN_STATUS.
;
;******************************************************************************

DSK_STATUS      PROC    NEAR
        MOV     DSK_OPN_STATUS,AL      ;RESTORE SAVED STATUS
        XOR     AL,AL
        RET
DSK_STATUS      ENDP                     
page

;******************************************************************************
;
; DSK_READ          
; DSK_WRITE
; DSK_VERIFY 	; DSK_FORMAT
;     THESE ROUTINES HANDLE THE READ/WRITE/VERIFY/FORMAT COMMANDS.
;     THESE ROUTINES BASICALLY FOLLOW THE SAME SEQUENCE OF EVENTS.
;     THE DMA CONTROLLER CHIP IS SETUP AND STARTED. THE DRIVE        
;     MOTOR IS TURNED ON IF NOT ALREADY ON.A SEEK OPERATION IS THEN
;     INITIATED TO POSITION THE HEAD AT THE APPROPRIATE TRACK.
;     THE COMMAND ALONG WITH PARAMETERS ARE SENT TO THE FDC.
;     THE ROUTINE THEN WAITS FOR END OF OPERATION INTERRUPT.
;     WHEN THE INTERRUPT OCCURS THE RESULTS OF THE OPERATION
;     ARE ANALYZED AND DSK_OPN STATUS UPDATED ACCORDINGLY.
;
;     INPUT TO ROUTINE :
;     AX = 
;     DL =
;
;     OUTPUT FROM ROUTINE :
;     AL =
;          
;     OTHER REGS DESTROYED :
;      
;
;******************************************************************************
;       
;       
DSK_READ        PROC    NEAR           
        MOV     AX,0E646H               ;PUT READ CMD FOR DMA IN AL   11/20/84
                                       ;PUT READ CMD FOR FDC IN AH
        JMP     SHORT DSK_OPN          ;GO TO COMMON SECTION
;
DSK_WRITE       PROC    NEAR           
        OR      MOTOR_ON,80H           ;SET WRITE BIT
        MOV     AX,0C54AH               ;PUT WRITE CMD FOR DMA IN AL  11/20/84
                                       ;PUT WRITE CMD FOR FDC IN AH
        JMP     SHORT DSK_OPN          ;GO TO COMMON SECTION
;
DSK_VERIFY      PROC    NEAR                 
        MOV     AX,0E642H               ;PUT VERF CMD FOR DMA IN AL   11/20/84
                                       ;PUT READ CMD FOR FDC IN AH
        JMP     SHORT DSK_OPN          ;GO TO COMMON SECTION
;
DSK_FORMAT      PROC    NEAR           
        OR      MOTOR_ON,80H           ;SET WRITE BIT
        MOV     AX,4D4AH               ;PUT WRITE CMD FOR DMA IN AL
                                       ;PUT FMT   CMD FOR FDC IN AH
;
DSK_OPN 	PROC    NEAR           ;THIS IS A COMMON SECTION.
                                       	;READ/WRITE/VERF/FORMAT ROUTINES
                                       ;JUMP HERE
        CALL    START_DMA              ;SETUP&START DMA CONTROLLER
        JNC     D20                    ;CHECK FOR DMA BNDRY ERROR
        MOV     DSK_OPN_STATUS,DMA_BNDRY_ERR   ;SET DMA BNDRY ERROR
        XOR     AL,AL                  ;SET SECTORS XFRD TO 0
        RET
D20:    PUSH    AX                     ;SAVE CMD
    	push	cx			;save input parameters 11/26/84  rlm
        MOV     AL,1                   ;DECODE CURRENT DRIVE NUMBER
        CALL    DECODE
        CLI                            ;HOLD OFF INTERRUPTS WHEN LOOKING
                                       ;AT MOTOR COUNT
        TEST    MOTOR_ON,AL            ;IS CURRENT MOTOR ON ?
        JZ      D30                    ;NO - TURN ON MOTOR

	CMP	MOTOR_TIMOUT,0E0H	;MOTOR SPIN UP?
	JB	D22			;YES

D30:	MOV     MOTOR_TIMOUT,0FFH       ;INIT MOTOR COUNT

	OR      MOTOR_ON,AL  	       ;TURN ON CURRENT MOTOR
	and	motor_on,0cfh		;
	call	encode_1
	or	motor_on,al		;
	mov	al,dl			;drive code in al
	sti
	mov	bl,motor_on		;get motor on status
	mov	cl,4			;
	ror	bl,cl			;
	and	bl,0f3h			;
	or	bl,0ch			;set reset and interrupt enable
	mov	al,bl			;
        PUSH    DX
        MOV     DX,DOR
        OUT     DX,AL                  ;OUTPUT TO DOR
        POP     DX
;       TEST    MOTOR_ON,80H           ;IS IT WRITE OP?
;       JZ      D22                    ;NO - SKIP WAIT LOOP 

;	wait for motor on
	
	clc				;clear carry
	mov	al,0fdh			;load type code for diskette motor
	mov	ah,90h			;code, ah = function code for
	int	15h			;Int 15
	jc	d22			;

        MOV     BX,10                  ;YES - WAIT THE TIME SPECIFIED
        CALL    GET_PARAMETER          ;BY THE MOTOR WAIT PARAMETER
	OR	AH,AH
	JZ	D22

D21:    CALL    WAIT_125MSEC           ;WAIT 1/8 OF A SECOND(125 MILLISECONDS)
        DEC     AH                     ;MORE WAIT?
        JNZ     D21                    ;YES - GO BACK TO WAIT LOOP

D22:    STI                            ;REENABLE INTERRUPTS
	pop	cx			;  11/26/84   rlm
        CALL    SEEK_OPN               ;SEEK TO CORRECT TRACK
        POP     AX                     ;RESTORE COMMAND
        JNC     D26                    ;no error
	jmp	D25			;jmp if seek error
D26:    MOV     BL,AH                  ;COMMAND TO BL
        MOV     DI,OFFSET D25          ;SETUP RETURN ADDR FOR FDC_OUT ROUTINE
        PUSH    DI
        CALL    FDC_OUT                ;OUTPUT THE COMMAND TO FDC
        MOV     AH,[BP+bp_dx+1]       	;GET CURRENT HEAD NUMBER
        AND     AH,1
        JZ      D22A
        MOV     AH,4
D22A:   OR      AH,DL                  ;PUT DRIVE NUMBER IN BITS 0,1
        CALL    FDC_OUT                ;OUTPUT HEAD
        CMP     BL,04DH                ;IF CMD IS FORMAT THEN HANDLE DIFFERENT
        JNE     D23                    ;JMP IF NOT FMT CMD

;	FMT CMD; needs special handling

        MOV     BX,3                   ;GET BYTES/SEC PARAMETER
        CALL    OUT_PARAMETER          ;OUTPUT TO FDC
        INC     BX                     ;POINT TO SEC/TRACK PARAMETER
        CALL    OUT_PARAMETER          ;OUTPUT TO FDC
        MOV     BX,7                   ;POINT TO GAP LENGTH
        CALL    OUT_PARAMETER          ;OUTPUT TO FDC
        INC     BX                     ;POINT TO FILLER BYTE
        CALL    OUT_PARAMETER          ;OUTPUT TO FDC
        JMP     D24
D23:                                   ;READ/WRITE/VERIFY CMD
        MOV     AH,CH                  ;GET CYCLINDER NUMBER
        CALL    FDC_OUT                ;OUTPUT TO FDC
        MOV     AH,[BP+bp_dx+1]      	;GET HEAD NUMBER
        CALL    FDC_OUT                ;OUTPUT TO FDC
        MOV     AH,CL                  ;GET SECTOR NUMBER
        CALL    FDC_OUT                ;OUTPUT TO FDC
        MOV     BX,3                   ;OUTPUT BYTES/SEC
D23A:   CALL    OUT_PARAMETER          ;TO FDC
        INC     BX                     
	call	out_parameter		;eot parameter
	push	dx
	mov	dx,[bp+bp_dx]		;get input parameters
	call	get_state		;
	pop	dx			;
	test	al,established		;is the states established
	jz	D23B			;no
	and	al,7			;yes
	sub	al,3			;
D23B:	and	al,7			;get only last three bits
	or	al,al			;is the state 0
	jnz	D23C			;no
	mov	ah,2ah			;get gap for 360 media in 360 drive
	jmp	short D23E		;
D23C:
	dec	al			;check for 360 media in 1.2M drive
	jnz	D23D			;nop
	mov	ah,23h			;yes, load gap for this config
	jmp	short D23E		;
D23D:	mov	ah,1bh			;move gap for 1.2M media in 1.2M drive
D23E:	call	fdc_out			;go output gap length
	mov	bx,6			;get dtl parameter
	call	out_parameter
D24:    POP     DI                     ;GET RID OF DUMMY RETURN ADDR
        CALL    WAIT_INTR              ;WAIT FOR END OF OPERATION INTERRUPT
        JC      D25                    ;SKIP ANALYSIS IF INT ERROR
        CALL    ANALYZE_RESULT_2       ;ANALYZE FDC RESULTS
        RET
D25:    CALL    GET_FDC_RESULTS        ;READ RESULTS TO CLEAN UP FDC
        XOR     AL,AL                  ;NUMBER OF SECTORS XFRD IS 0
        RET
DSK_OPN         ENDP
DSK_FORMAT      ENDP
DSK_VERIFY      ENDP
DSK_WRITE       ENDP
DSK_READ        ENDP

page
;                                        
;******************************************************************************
;
; START DMA         
;     THIS ROUTINE SETS UP AND STARTS THE DMA CONTROLLER.
;     THE DMA IS INITIALIZED WITH THE MODE BYTE, ADDRESS
;     AND WORD COUNT.                         
;
;     
;     INPUT TO ROUTINE :
;     AL = MODE BYTE FOR DMA CONTROLLER
;     DH = NUMBER OF SECTORS
;     ES:BX = DMA ADDRESS
;       
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = O  SUCESSFUL
;                 1  FAILURE - ATTEMPTED TO CROSS 64K BOUNDARY
;
;     OTHER REGS DESTROYED :
;     BX
;
;******************************************************************************


START_DMA       PROC    NEAR
        PUSH    AX                     ;SAVE REGS
        PUSH    CX
	cli				;
        OUT     DMA1+11,AL             ;OUTPUT MODE BYTE
        MOV     CL,4                   ;SETUP SHIFT COUNT
        MOV     CH,BL                  ;SAVE LOW NIBBLE OF BX
        AND     CH,0FH                 ;ISOLATE BITS 0-3
        SHR     BX,CL                  ;SHIFT OFFSET 4 BITS 
        MOV     AX,ES                  ;GET SEGMENT
        ADD     BX,AX                  ;ADD SEGMET TO OFFSET
        ROL     BX,CL                  ;PAGE ADDR IS NOW IN 0-3 OF BX 
        MOV     AL,BL                  ;
        AND     AL,0FH                 ;ISOLATE PAGE ADDR
        OUT     DMA_PAGE,AL            ;OUPUT TO DMA PAGE REG
        AND     BL,0F0H                ;GET RID OF PAGE ADDR FROM BX
        OR      BL,CH                  ;MERGE SAVED NIBBLE
        PUSH    BX                     ;BX NOW CONTAINS DMA ADDR -- SAVE IT
        OUT     DMA1+12,AL             ;INITIALIZE FIRST/LAST FF
        MOV     AL,BL                                        
        OUT     DMA1+4,AL              ;OUTPUT DMA ADDR (LSB)
        MOV     AL,BH
        OUT     DMA1+4,AL              ;OUTPUT DMA ADDR (MSB)
;
        MOV     BX,3                   ;POINT TO BYTES/SECTOR PARAMETER
        CALL    GET_PARAMETER          ;GET IT
        MOV     CL,AH                  ;USE IT FOR SHIFT COUNT LATER
        MOV     BH,DH                  ;GET NUMBER OF SECTORS
        XOR     BL,BL
        SHR     BX,1                   ;BX NOW CONTAINS NUMBER OF
                                       ;SECTORS MULTIPLIED BY 128
        SHL     BX,CL                  ;ADJUST DMA COUNT BASED ON PARAMETER
        DEC     BX                     ;ADJUST FOR DMA
        MOV     AL,BL                  
        OUT     DMA1+5,AL              ;OUTPUT DMA WORD COUNT (LSB)
        MOV     AL,BH
        OUT     DMA1+5,AL              ;OUTPUT DMA WORD COUNT (MSB)
	sti				; 
        POP     AX                     ;GET SAVED ADDRESS
        ADD     AX,BX                  ;CARRY WILL SET IF 64K BNDRY CROSS
        MOV     AL,2
        OUT     DMA1+10,AL             ;CLEAR CH2 MASK BIT         
        POP     CX                     ;RESTORE REGS
        POP     AX                     
        RET
START_DMA       ENDP

page

;******************************************************************************
;
; DECODE            
;     THIS ROUTINE DECODES THE NUMBER IN DL (DRIVE NUMBER) AND
;     RETURNS THE DECODED VALUE IN AL.
;               
;     INPUT TO ROUTINE :
;     DL = VALUE TO BE DECODED        
;
;     OUTPUT FROM ROUTINE :
;     AL = DECODED VALUE
;          
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************

DECODE  PROC    NEAR                                  
        PUSH    CX                     ;SAVE REGS
        MOV     CL,DL                  ;USE DL AS SHIFT AMOUNT
        SAL     AL,CL                  ;DO THE SHIFT
        POP     CX                     ;RESTORE REGS
        RET
DECODE  ENDP

;******************************************************************************
;
; ENCODE            
;     THIS ROUTINE ENCODES THE VALUE IN BITS 0:3 OF AL
;     AND RETURNS THE ENCODED VALUE IN AL
;               
;     INPUT TO ROUTINE :
;     AL = VALUE TO BE ENCODED
;
;     OUTPUT FROM ROUTINE :
;     AL = ENCODED VALUE
;          
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************
       
ENCODE  PROC    NEAR
        TEST    AL,01H                 ;TEST BIT 0
        JNZ     D41
        TEST    AL,02H                 ;TEST BIT 1
        JNZ     D41
        TEST    AL,04H                 ;TEST BIT 2
        JNZ     D40
        TEST    AL,08H                 ;TEST BIT 3
        JZ      D42                    
        SUB     AL,03H
D40:    DEC     AL
D41:    DEC     AL
D42:    RET
ENCODE  ENDP        
                                        
;******************************************************************************
;
; WAIT_MSEC         
;     THIS ROUTINE WAITS THE NUMBER OF MILLISECONDS SPECIFIED
;     IN AH.                                                       
;               
;     INPUT TO ROUTINE :
;     AH = NUMBER OF MILLISECONDS TO BE WAITED
;
;     OUTPUT FROM ROUTINE :
;     NONE 
;          
;     OTHER REGS DESTROYED :
;     AH
;
;******************************************************************************
       
WAIT_MSEC       PROC     NEAR          
        PUSH    CX                     ;SAVE REGS
        OR      AH,AH                  ;CHECK FOR 0
        JZ      D48                    ;SKIP WAIT LOOP IF 0
D45:    MOV     CX,550                 ;SETUP 1 MSEC LOOP COUNT  
D46:    LOOP    D46                    ;WAIT 1 MSEC
        DEC     AH                     ;DECREMENT MSEC COUNT
        JNZ     D45                    ;KEEP DOING UNTIL TOTAL TIME EXPIRES
D48:    POP     CX                     ;RESTORE REGS
        RET
WAIT_MSEC       ENDP
;
;    WAIT 1/8 SECOND
;
WAIT_125MSEC    PROC    NEAR           
        PUSH    AX                     ;SAVE REGS
        MOV     AH,125                 ;SETUP 125 MILLISECOND COUNT
        CALL    WAIT_MSEC              ;WAIT 125 MILLISECONDS
        POP     AX                      
        RET
WAIT_125MSEC    ENDP
page
                                        
;******************************************************************************
;
; SEEK_OPN          
;     THIS ROUTINE POSITIONS THE HEAD ON THE SPECIFIED DRIVE    
;     TO THE SPECIFIED TRACK. IF THIS IS THE FIRST SEEK OPERATION
;     AFTER DRIVE RESET WAS ISSUED, THE DRIVE IS RECALIBRATED BEFORE 
;     THE SEEK.
;               
;     INPUT TO ROUTINE :
;     DL = DRIVE NUMBER TO WHICH TO SEEK ON
;     CH = TRACK NUMBER TO WHICH TO SEEK TO
;
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = 0   SEEK SUCESSFULL
;     CARRY FLG = 1   SEEK FAILED 
;	              DSK_OPN_STATUS UPDATED
;
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************

SEEK_OPN	PROC    NEAR
        MOV     AL,1                   
        CALL    DECODE                 	;DECODE DRIVE NUMBER
        TEST    AL,RECAL_REQD          	;IS  RECALIBRATE REQUIRED?
        JNZ     D50                    	;NO - GO DO THE SEEK
        OR      RECAL_REQD,AL          	;YES -SET NO RECAL REQD BIT
        CALL    RECAL_OPN              	;DO THE RECAL OPERATION
        JNC     seek_2			;GO DO SEEK IF NO ERROR
	mov	dsk_opn_status,0	;issue recal to 1.2M drives
	call	recal_opn		;
	jnc	seek_2			;no error
	ret				;ERROR - RETURN

seek_2:
	mov	al,0			;
	call	set_drv_cyl		;go set current cyl #

D50:    
;
; ----- Rel. 3.3
;
;	cmp	hf_cntrl,1		;check for dual adapter
;	jne	seek_1			;
	call	get_state		;get current drive state
	test	al,20h			;double step rate required
	jz	seek_1			;no, 1.2M drive
	shl	ch,1			;double the # of steps to take
seek_1:
	call	get_drv_cyl		;are we at right track #
	cmp	al,ch			;
	je	seek_3			;yes,we are done
	mov	al,ch			;no,then set current position
	call	set_drv_cyl		;
	MOV     AH,0FH                 ;                 
        CALL    FDC_OUT                ;OUTPUT SEEK CMD TO FDC
        MOV     AH,DL
        CALL    FDC_OUT                ;OUTPUT DRIVE NUMBER TO FDC
        MOV     AH,CH
        CALL    FDC_OUT                ;OUTPUT TRACK NUMBER TO FDC
        CALL    WAIT_INTR              ;WAIT FOR INTERRUPT TO OCCUR
        JC      D51                    ;SKIP RESULT ANALYSIS IF ERROR  
        CALL    ANALYZE_RESULT_1       ;ANALYZE RESULTS OF OPERATION
D51:    
	call	get_state		;
	test	al,20h			;did we modify original track #
	jz	seek_4			;no, then skip
	shr	ch,1			;yes, get back original track #
seek_4:
	PUSHF                          ;SAVE FLAGS
        MOV     BX,9                   ;GET HEAD SETTLE PARAMETER
        CALL    GET_PARAMETER
        CALL    WAIT_MSEC              ;WAIT FOR HEAD SETTLE
        POPF                           ;RESTORE FLAGS
D52:	RET

seek_3:
	call	get_state		;
	test	al,20h			;test for double step rate
	jz	D52			;not require
	shr	ch,1			;required, adjust step rate
	ret				;

SEEK_OPN        ENDP                    
page
                                        
;******************************************************************************
;
; RECAL_OPN         
;     THIS ROUTINE RECALIBRATES THE SPECIFIED DRIVE.
;               
;     INPUT TO ROUTINE :
;     DL = DRIVE NUMBER TO BE RECALIBRATED 
;
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = 0   RECALIBRATE SUCESSFULL
;     CARRY FLG = 1   RECALIBRATE FAILED
;                     DSK_OPN_STATUS UPDATED
;          
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************
;       
RECAL_OPN       PROC    NEAR           
        MOV     AH,07H
        CALL    FDC_OUT                ;OUTPUT RECAL CMD TO FDC
        MOV     AH,DL
        CALL    FDC_OUT                ;OUTPUT DRIVE NUMBER TO FDC
        CALL    WAIT_INTR              ;WAIT FOR INTERRUPT
        JC      D55                    ;SKIP RESULT ANALYSIS IF ERROR
        CALL    ANALYZE_RESULT_1       ;ANALYZE RESULTS OF RECAL OPERATION
D55:    RET                            
RECAL_OPN       ENDP
;                                        
;******************************************************************************
;
; GET PARAMETER        
;     THIS ROUTINE GETS THE DISK PARAMETERS AND RETURNS THE PARAMETER	
;     IN AH. THE PARAMETERS ARE STORED IN A TABLE POINTED BY DISK_POINTER.
;     THE INDEX TO THE TABLE IS SUPPLIED BY THE CALLING ROUTINE.
;               
;     INPUT TO ROUTINE :
;     BX = INDEX TO TABLE CONTAINING PARAMETERS
;
;     OUTPUT FROM ROUTINE :
;     AH = PARAMETER FROM TABLE
;          
;     OTHER REGS DESTROYED :
;     AL
;
;******************************************************************************
;       
GET_PARAMETER   PROC    NEAR
        ASSUME  DS:ZERO                    
        PUSH    DS                     	;SAVE REG
	push	si			;
        XOR     AX,AX	                ;SET DS TO ABSOLUTE SEG 0
        MOV     DS,AX
        LDS     SI,DSK_PARM_PTR        	;GET TABLE POINTER
        MOV     AH,[SI+BX]             	;GET THE PARAMETER
	pop	si
	pop	ds			;11/21/84
	assume ds:dataa
	cmp	bl,10			;check for startup delay parameter
	jne	get_parm3		;nop
	mov	ah,8			;institute a minimum delay
	ret				;and return
get_parm3:
	cmp	bl,9			;is this a head settle time para
	jne	get_parm1		;no,return
	push	dx			;save dx register
	mov	dx,[bp+bp_dx]		;
	call	get_state		;
	pop	dx			;
	or	ah,ah			;is the HST already specified
	jnz	get_parm1		;yes, then leave it alone
	mov	ah,20			;get HST for 330 drive
	and	al,7			;isolate state
	jz	get_parm1		;no state
	cmp	al,3			;check for state 3
	jz	get_parm1		;
	mov	ah,15			;get HST for 1.2M drive
get_parm1:
        RET
GET_PARAMETER   ENDP
;
;       GET PARM AND OUTPUT TO FDC
;
OUT_PARAMETER   PROC    NEAR
        CALL    GET_PARAMETER          ;GET PARAMETER
        JMP     FDC_OUT                ;GO TO OUTPUT BYTE
OUT_PARAMETER   ENDP
;                                        
;******************************************************************************
;
; FDC_OUT           
;     THIS ROUTINE SENDS A BYTE TO THE FLOPPY DISK CONTROLLER(FDC)
;     AFTER CHECKING IF THE FDC IS READY TO ACCEPT DATA.
;     THE FDC IS READY TO ACCEPT DATA WHEN THE DIRECTION BIT, READY
;     BIT IN THE MAIN STATUS REG ARE 0,1 RESPECTIVELY.
;     A TIMEOUT ERROR IS RECOGNIZED IF THE "READY TO ACCEPT DATA"
;     CONDITION DOES NOT OCCUR WITHIN A REASONABLE TIME.                     
;                                                                      
;     INPUT TO ROUTINE :
;     AH = BYTE TO BE OUTPUTTED TO FDC
;
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = 0   BYTE OUTPUT SUCESSFULLY
;     CARRY FLG = 1   TIMEOUT ERROR  - DSK_OPN_STATUS UPDATED 
;                     (RETURN IS MADE TO ONE LEVEL HIGHER THAN
;                      THE CALLER)     
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************
;
FDC_OUT         PROC    NEAR           
        PUSH    CX                     ;SAVE REGS
        PUSH    DX
	push	bx			;  11/26/84
	mov	bl,2			;go through the loop twice
        SUB     CX,CX                  ;SETUP TIMEOUT COUNT
        MOV     DX,FDC                 ;SETUP PORT ADDRESS
D60:    IN      AL,DX                  ;GET FDC MAIN STATUS
        AND     AL,0C0H                ;MASK BITS 0-5
        CMP     AL,80H                 ;IS DIRECTION=0, READY=1 ?
        JE      D61                    ;YES - GO OUTPUT BYTE
        LOOP    D60                    ;NO - KEEP CHECKING
                                       ;TIMEOUT ERROR
	xor	cx,cx			;one more time
	dec	bl			;
	jnz	d60			;   11/26/84
        OR      DSK_OPN_STATUS,TIME_OUT  ;SET TIMEOUT BIT IN DISK STATUS
        STC                            ;SET CARRY TO INDICATE ERROR
	pop	bx			;   11/26/84
        POP     DX
        POP     CX                     ;RESTORE REGS
        POP     AX                     ;DISCARD RETURN ADDR OF CALLING RTN
        RET                            ;RETURN ONE LEVEL HIGHER THAN CALLER
D61:    INC     DX
        MOV     AL,AH
        OUT     DX,AL                  ;OUTPUT BYTE TO FDC
        CLC                            ;CLEAR CARRY
	pop	bx			;  11/26/84
        POP     DX
        POP     CX
        RET
FDC_OUT	ENDP
;                                        
;******************************************************************************
;
; FDC_IN            
;     THIS ROUTINE READS A BYTE FROM THE FLOPPY DISK CONTROLLER(FDC)
;     AFTER CHECKING IF THE FDC IS READY TO SEND DATA.  
;     THE FDC IS READY TO SEND DATA WHEN THE DIRECTION BIT, READY
;     BIT IN THE MAIN STATUS REG ARE 1,1 RESRECTIVELY.
;     A TIMEOUT ERROR IS RECOGNIZED IF THE "READY TO SEND DATA"
;     CONDITION DOES NOT OCCUR WITHIN A REASONABLE TIME.             
;     A FDC ERROR IS RECOGNIZED IF DIRECTION BIT IS BAD.
;
;     INPUT TO ROUTINE :
;     NONE                             
;
;     OUTPUT FROM ROUTINE :
;     AL = BYTE READ FROM THE FDC
;     CARRY FLG = 0   BYTE INPUT SUCESSFULLY
;     CARRY FLG = 1   TIMEOUT ERROR  - DSK_OPN_STATUS UPDATED 
;                                      
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************
FDC_IN          PROC    NEAR           
        PUSH    CX                     ;SAVE REGS
        PUSH    DX
        SUB     CX,CX                  	;SETUP TIMEOUT COUNT
        MOV     DX,FDC
D65:    IN      AL,DX                  ;GET FDC MAIN STATUS
        AND     AL,0C0H                ;MASK BITS 0-5
        CMP     AL,0C0H                ;IS DIRECTION=1, READY=1 ?
        JE      D66                    ;YES - GO INPUT BYTE
        LOOP    D65                    ;NO - KEEP CHECKING
                                       ;ERROR - FIND OUT WHICH ONE
        POP     DX                     ;RESTORE REGS
        POP     CX
        TEST    AL,80H                 ;IS READY BIT ON
        JNZ     D65A                   ;YES - GO SET FDC ERR
        OR      DSK_OPN_STATUS,TIME_OUT  ;NO - SET TIMEOUT BIT IN DISK STATUS
        STC                            ;SET CARRY TO INDICATE ERROR
        RET
D65A:   OR      DSK_OPN_STATUS,FDC_ERR ;FDC FAILURE
        STC                            ;SET CARRY TO INDICATE ERROR
        RET                            ;TIMEOUT ERROR
D66:    INC     DX
        IN      AL,DX                  ;INPUT BYTE FROM FDC
        CLC                            ;CLEAR CARRY
        POP     DX
        POP     CX
        RET
FDC_IN  ENDP
;                                        
;******************************************************************************
;
; GET_FDC_RESULTS   
;     THIS ROUTINE READS RESULTS FROM THE FDC AFTER AN INTERRUPT.
;     THE RESULTS ARE STORED IN MEMORY STARING AT LOCATION FDC_RESULTS.
;     A TIMEOUT ERROR IS RECOGNIZED IF FDC IS UANABLE TO GIVE STATUS
;     WITHIN A REASONABLE TIME.
;     INPUT TO ROUTINE :
;     NONE
;
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = 0   RESULTS SUCESSFULLY TRANSFERED TO MEMORY
;     CARRY FLG = 1   TIMEOUT ERROR  - DSK_OPN STATUS UPDATED
;
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************
;       
GET_FDC_RESULTS PROC    NEAR           
        PUSH    ES                     ;SAVE REGS
        PUSH    BX                                 
        PUSH    CX
        PUSH    DI
        PUSH    DX
        MOV     DX,DS
        MOV     ES,DX                  ;SETUP ES FOR STRING OPERATION
        MOV     DX,FDC                 ;SETUP PORT ADDRESS
        MOV     DI,OFFSET FDC_RESULTS  ;GET START ADDR OF RESULT BLOCK
        MOV     BH,7                   ;SETUP MAX# OF RESULT BYTES
        CLD                            ;SET DIRECTION FLG TO AUTOINC
D70:    CALL    FDC_IN                 ;GET BYTE FROM FDC
        JC      D72                    ;RETURN IF ERROR
        STOSB                          ;STORE BYTE IN RESULT BLOCK
        MOV     CX,20
D71:    LOOP    D71                    ;WAIT TIME
	IN      AL,DX                  ;GET STATUS BYTE
        TEST    AL,10H                 ;IS FDC STILL BUSY?
        JZ      D72                    ;NO - WE ARE DONE
        DEC     BH                     ;YES - MORE TO READ
        JNZ     D70                    
        OR      DSK_OPN_STATUS,FDC_ERR ;FDC GAVE MORE THAN MAX BYTES
        STC                            ;SET CARRY TO INDICATE ERROR
D72:    POP     DX
        POP     DI                     ;RESTORE REGS
        POP     CX
        POP     BX
        POP     ES
        RET
GET_FDC_RESULTS ENDP                     
  ;                                        
;******************************************************************************
;
; WAIT_INTR                            
;     THIS ROUTINE WAITS FOR AN INTERRUPT FROM THE FDC. A TIMEOUT
;     ERROR IS RECOGNIZED IF AN INTERRUPT IS NOT RECEIVED WITHIN
;     A REASONABLE TIME.
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = 0   INTERRUPT OCCURED
;     CARRY FLG = 1   TIMEOUT ERROR (NO INTERRUPT)  
;                     DSK_OPN_STATUS UPDATED
;
;******************************************************************************
       
WAIT_INTR       PROC    NEAR           
        PUSH    CX                     	;SAVE REGS
	push	ax			;
	clc				;
	mov	ah,090h			;call int 15
	mov	al,1			;
	int 	15h			;
	jc	D77			;
        MOV     AL,6                   	;SETUP FOR TWO SECOND TIMEOUT WAIT
        SUB     CX,CX
D75:    TEST    RECAL_REQD,INTR_FLAG   	;DID INTERRUPT OCCUR?
        JNZ     D76                    	;YES - RESTORE REGS AND RETURN
        LOOP    D75                    	;NO - KEEP CHECKING FOR INTERRUPTS
        DEC     AL                     	;TWO SECONDS OVER ?          
        JNZ     D75                    	;NO - KEEP CHECKING FOR INTERRUPTS
D77:    OR      DSK_OPN_STATUS,TIME_OUT ;YES - SET TIMEOUT ERROR IN DISK STAT
	stc				;set carry flag
D76:    
	pop	ax
	POP     CX                     	;RESTORE REGS
	pushf
	AND     RECAL_REQD,NOT INTR_FLAG ;RESET INTR FLAG AND CARRY FLAG
        popf

        RET                            
WAIT_INTR ENDP
                                        
;******************************************************************************
;
; Floppy_Intr
;     THIS ROUTINE HANDLES THE INTERRUPT FROM THE FDC. THE INTR
;     FLAG IN THE RECAL_REQD BYTE IS SET.
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     INTR FLAG SET IN THE RECAL_REQD BYTE IN MEMORY
;
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************
                                        
floppy_intr	PROC    FAR
        STI                            ;RENABLE INTERRUPTS
        PUSH    AX                     ;SAVE REGS
        PUSH    DS
        MOV     AX,DATA                ;SETUP DS TO POINT TO DATA SEG
        MOV     DS,AX
        OR      RECAL_REQD,INTR_FLAG   ;SET INTR FLAG
        MOV     AL,EOI                 ;SIGNAL EOI TO INTR CONTROLLER
        OUT     INT1,AL			;
	mov	al,01			;
	mov	ah,91h			;
	int	15h			;
        POP     DS                     ;RESTORE REGS
        POP     AX
        IRET
floppy_intr	ENDP
;
;******************************************************************************
;
; ANALYZE_RESULT_1 
;     THIS ROUTINE IS NORMALLY CALLED AFTER AN INTERRUPT DUE TO
;     A SEEK, RECALIBRATE OR RESET TO THE FDC. FOLLOWING AN INTERRUPT
;     THE FDC MUST BE RELEIVED OF STATUS BYTES (RESULTS). THIS ROUTINE 
;     SENDS THE SENSE INTERRUPT COMMAND TO THE FDC, READS AND ANALYZES
;     THE RESULTS AND UPDATES THE DSK_OPN_STATUS ACCORDINGLY.
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     CARRY FLG = 0   RESULTS ARE GOOD       
;     CARRY FLG = 1   FAILURE OCCURED IN EITHER READING THE RESULTS
;                     OR IN RESULTS ITSELF. DSK_OPN_STATUS UPDATED.
;
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************
;
ANALYZE_RESULT_1   PROC    NEAR        
        MOV     AH,08H                 ;SEND SENSE INTR CMD TO FDC
        CALL    FDC_OUT
        CALL    GET_FDC_RESULTS        ;GET RESULTS FROM FDC
        JC      D80                    ;JMP IF ERROR
        MOV     AH,FDC_RESULTS         ;GET ST0  
        AND     AH,60H                
	cmp	ah,60h			;
	jne	D81			;
	stc				;set carry
	or	dsk_opn_status,seek_err
D80:	ret				;
D81:	clc				;
	ret				;

ANALYZE_RESULT_1    ENDP
;       
;                                        
;******************************************************************************
;
; ANALYZE_RESULT_2  
;     THIS ROUTINE IS NORMALLY CALLED AFTER AN INTERRUPT DUE
;     TO A READ/WRITE/VERFIFY/FORMAT OPERATION. FOLLOWING AN INTERRUPT
;     THE FDC MUST BE RELEIVED OF 7 BYTES OF RESULTS. THIS ROUTINE
;     READS AND ANALYSES THE RESULTS, AND UPDATES THE DSK_OPN_STATUS
;     ACCORDINGLY.ALSO THE ACTUAL NUMBER OF SECTORS TRANSFERRED
;     ARE CALCULATED.
;
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     AL = ACTUAL NUMBER OF SECTORS TRANSFERRED
;          
;     OTHER REGS DESTROYED :
;     AX,BX 
;
;******************************************************************************
;       
ANALYZE_RESULT_2    PROC   NEAR
        CALL    GET_FDC_RESULTS        ;GET RESULTS FROM FDC
        JNC     D85                    ;JMP IF NO ERROR
        RET                            ;ERROR - RETURN
D85:    LEA     BX,FDC_RESULTS         ;GET RESULTS BLK POINTER
        MOV     AL,[BX]                ;GET ST0
        AND     AL,0C0H                ;CHECK FOR NORMAL TERMINATION
        JZ      D88                    ;JMP IF NORNAL TERMINATION
        CMP     AL,40H                 ;ABNORMAL TERMINATION?
        JNZ     D86                    ;NO - GO SET FDC ERROR
                                       ;YES - ANALYZE ABNORMAL CONDITIONS
        MOV     AL,[BX+1]              ;GET ST1
        OR      AL,AL                  ;IS THERE AT LEAST ONE ERR BIT SET ?
        JZ      D86                    ;NO - GO SET FDC ERR
        MOV     AH,REC_NOT_FOUND
        TEST    AL,80H                 ;CHECK RECORD NOT FOUND (END OF CYL)
        JNZ     D87
        MOV     AH,CRC_ERR              
        TEST    AL,20H                 ;CHECK CRC DATA ERROR
        JNZ     D87
        MOV     AH,DMA_OVERRUN
        TEST    AL,10H                 ;CHECK DMA OVERRUN
        JNZ     D87
        MOV     AH,REC_NOT_FOUND
        TEST    AL,04H                 ;CHECK RECORD NOT FOUND (SECTOR)
        JNZ     D87                    
        MOV     AH,WR_PROT_ERR         
        TEST    AL,02H                 ;CHECK WRITE PROTECT ERROR
        JNZ     D87
        MOV     AH,ADDR_MARK_ERR       ;MUST BE MISSING ADDR MARK ERR
        JMP     D87
D86:    MOV     AH,FDC_ERR
D87:    OR      DSK_OPN_STATUS,AH
D88:    CALL    SECTOR_XFRD            ;CALCULATE SECTORS TRANSFERRED
	xor	ah,ah			;no error reported
        RET
ANALYZE_RESULT_2    ENDP
;******************************************************************************
;
; SECTOR_XFRD       
;     THIS ROUTINE CALCULATES ACTUAL NUMBER OF SECTORS      
;     TRANSFERRED.           
;
;     INPUT TO ROUTINE :
;     CH = CYCLINDER NUMBER
;     CL = STARTING SECTOR NUMBER
;
;     OUTPUT FROM ROUTINE :
;     AL = ACTUAL NUMBER OF SECTORS TRANSFERRED
;          
;     OTHER REGS DESTROYED :
;     AX,BX 
;
;******************************************************************************
;
SECTOR_XFRD     PROC    NEAR
        MOV     AL,FDC_RESULTS+3       ;GET ENDING CYL NUMBER
        CMP     AL,CH                  ;ARE START AND END CYL THE SAME
        MOV     AL,FDC_RESULTS+5       ;GET ENDING SECTOR NUMBER
        JZ      D90                    ;IF START CYL= END CYL, SKIP ADJUST
        MOV     BX,4                   
        CALL    GET_PARAMETER          ;GET EOT
        INC     AH                     
        SUB     AH,CL
        MOV     AL,AH
        RET
D90:    SUB     AL,CL
        RET
SECTOR_XFRD	ENDP
page


;	floppy_setup:
;	This routine is called at at reset or bootup time to configure
;	what type of drives are attached to the system. This routine is
;	called only in case of hard disk/floppy combo card
;

floppy_setup	proc	near
	push	ax
	call	save_all
	xor	bx,bx
	mov	ax,bx

;
;	initialize some floppy parameters in low memory
;

	mov	word ptr drv_0_media,ax
	mov	word ptr drv_0_op,ax
	mov	word ptr recal_reqd,ax
	mov	motor_timout,al
	mov	data_rate,al

;
;	now read cmos to find out what kind of floppy drives we have
;	in the system
;

	mov	al,10h+80h		;get diskette type from cmos
	call	read_cmos		;
	mov	cl,ah			;save it cl
	and	ah,0f0h			;check for first drive
	jz	try_second		;no first try next 
	mov	al,61h			;assume 1.2M drive/360 diskette
	cmp	ah,20h			;is it true
	je	setup_1			;yes
	mov	al,93h			;no, then 360 drive/diskette
setup_1:
	mov	drv_0_media[bx],al

try_second:
	inc	bx			;next drive
	and	cl,0fh			;try next drive
	jz	setup_2			;not present, return
	mov	al,61h			;assume 1.2M drive/360 diskette
	cmp	cl,2			;is it true
	je	setup_3			;yes
	mov	al,93h
setup_3:
	mov	drv_0_media[bx],al

setup_2:
	call	restore_all		;restore registers
	pop	ax			;
	ret

floppy_setup	endp

page

;
;Read_dsk_line:	This routine reads the status of floppy disk changed
;		line. 
;		On input dl = drive code to be tested
;		On output Z flag = 0 disk change line inactive
;				 = 1 disk change line active
;		

Read_dsk_line	proc	near
	push	ax			;
	push	bx			; save registers
	push	cx			;
	push	dx			;
	call	encode_1		;get drive bits
	and	motor_on,0cfh		;
	or	motor_on,al		;
	mov	al,01			;
	call	decode			;
	cli
	test	al,motor_on		;
	jnz	L1			;motor already on, jump
      	mov	motor_timout,0ffh	;set large timeout
	or	motor_on,al		;insert drive code
L1:	sti				;		
	mov	cl,4			;
	mov	bl,motor_on		;
	ror	bl,cl
	and	bl,0f3h			;
	or	bl,0ch			;setup dma and non reset bit
	mov	al,bl			;
	mov	dx,3f2h			;get DOR register
	out	dx,al			;select drive
	add	dl,5			;get DIR
	call	nop_proc		;some delay
	in	al,dx			;get status
	and	al,80h			;check for bit 7
	pop	dx			;
	pop	cx			;restore registers
	pop	bx			;
	pop	ax			;
	ret				;
Read_dsk_line	endp

encode_1	proc	near
	xor	ax,ax
	mov	al,dl			;get drive code
	mov	cl,4			;# of bits to shift
	rol	al,cl			;do it
	ret				;
encode_1	endp

	
page
;
;Read_dsk_type:	Routine to either establish the type of media/drive
;		for Format command or return the information to
;		the user
;	On Input:	Ah = 15, Dl = drive code
;	On Output:	Ah = 00 drive not present
;			   = 01 320/360 drive
;			   = 02 1.2M drive
;			   = 03 fixed disk	
;			Cy = 0 no error , = 1 error

Read_dsk_type	proc	near
;
; ----- Rel. 3.3
;
;	cmp	hf_cntrl,0		;check for diskette card
;	je	type_3			;
	call	get_state		;get drive state
	test	al,established		;see if the state is known
	jnz	type_2			;media established,go check it
	cmp	al,0			;no disk available
	jne	type_4			;no,check it
type_6:
	mov	al,0			;return zero value
	ret				;
type_2:	
	and	al,7			;lets check current state
	cmp	al,3			;is it 360 media/360 drive
	je	type_7			;yes,then return code of 1
	mov	al,2			;otherwise it is 1.2m drive
	ret				;return code of 2
type_7:
	mov	al,1			;return code of 1
	ret

type_4:
	and	al,7			;isolate state bits
	jz	type_3			;0 means go read cmos
	mov	al,2			;otherwise set to 1.2M drive
	ret			
type_3:
	call	chk_cmos_good		;check if cmos is good
	jnz	type_6			;cmos bad
	mov	al,cmos_diskette_type	;get diagnostic port
	call	read_cmos		;go read it
	cmp	dl,0			;drive code 0
	jnz	type_8			;no
	mov	cl,4
	rol	ah,cl			;	11/20/84   rlm
type_8:
	and	ah,0fh			;get drive specs in low nibble
	cmp	ah,2			;valid specs
	jae	type_6			;invalid, say so
	ret				;valid,return
Read_Dsk_type	endp

page

;	get current state of the drive... drive code in dl

get_state	proc	near
	mov	al,drv_0_media		;assume dl = 0
	or	dl,dl			;true
	jz	get_state_1		;yes
	mov	al,drv_1_media		;no get state for drive 1
get_state_1:	ret
get_state	endp


;	set current state of the drive.. drive code in dl, state in al

set_state	proc	near
	or	dl,dl			;check drive code
	jnz	set_state_1		;drive code is 1
	mov	drv_0_media,al		;set state for drive 0
	ret				;
set_state_1:
	mov	drv_1_media,al		;set state for drive 1
	ret
set_state	endp


;	Get operation start state for drive... drive code in dl
;	return state in al

get_op_state	proc	near
	mov	al,drv_0_op		;assume we get it for drive 0
	or	dl,dl			;true
	jz	get_op_state1		;yes,return
	mov	al,drv_1_op		;no,get it for drive 1
get_op_state1:
	ret				;
get_op_state	endp


;	Set operation start state for drive... dl = drive code,
;	al = state

set_op_state	proc	near
	or	dl,dl			;drive code = 0
	jz	set_op_state1		;yes
	mov	drv_1_op,al		;set it for drive 1
	ret				;
set_op_state1:
	mov	drv_0_op,al		;set it for drive 0
	ret				;
set_op_state	endp			;


;
;	Set current drive cylinder.... dl = drive code
;	al = cylinder #			3/11/85

set_drv_cyl	proc	near
	or	dl,dl			;is it drive 0
	jnz	set_drv_cyl1		;no. then set for drive 1
	mov	drv_0_cyl,al		; drive 0
	ret				;
set_drv_cyl1:
	mov	drv_1_cyl,al		;no then set it
	ret				;
set_drv_cyl	endp


;
;	Get current drive cylinder #.... dl = drive code
;	al on return will have cyl #		3/11/85

get_drv_cyl	proc	near
	mov	al,drv_0_cyl		;assume drive 0
	or	dl,dl			;is it true
	jz	get_drv_cyl1		;yes, then return
	mov	al,drv_1_cyl		;no get for drive 1
get_drv_cyl1:
	ret				;return
get_drv_cyl	endp

page
	
;
;	Change_Disk:	This routine returns the state of the disk
;			change line.
;	Input Parameters:	AH = 16, DL = drive code
;	Output Parameters	AH = 0 disk change line not active
;				AH = 6 disk change line active

Change_Disk	proc	near
;
; ----- Rel. 3.3
;
;	cmp	hf_cntrl,0		;check for type of controller
;	jz	change_disk2		;just floppy adapter
	call	get_state		;get state of current drive
	mov	ah,al			;save it
	and	al,7			;mask everything but state
	cmp	al,3			;check for 360/360 established
	jne	change_disk1		;disk available
change_disk4:
	mov	dsk_opn_status,6	;indicate media removed from drive
	ret				;return to caller
change_disk1:
;	cmp	al,3			;check for 360/360 established
	ja	change_disk3		;high density drive
;	mov	al,ah			;
	cmp	ah,0			;any disk available
	jne	change_disk4		;
change_disk8:
	or	motor_timout,80h	;set time out bit
	ret				;
change_disk3:
	call	Read_dsk_line		;
	jnz	change_disk4		;
	ret				;	

change_disk2:
	call	chk_cmos_good		;see if battery is good
	jnz	change_disk8		;no good
	mov	al,cmos_diskette_type	;read diskette type
	call	read_cmos		;
	cmp	dl,0			;
	jnz	change_disk9		;
	mov	cl,4
	rol	ah,cl			;	11/20/84   rlm
change_disk9:
	and	ah,0fh			;check for drive present
	jnz	change_disk4		;
	or	motor_timout,80h	;set time out
	ret				;
change_disk	endp

page

;
;Set_type: This routine is used to set DASD type for diskette format
;Input Registers:	AH = 17h
;			AL = 0	not used
;			   = 1 	diskette 320/360 in 320/360 drive
;			   = 2  diskette 320/360 in 1.2M drive
;			   = 3  diskette 1.2M in 1.2M drive
;			DL = 0 or 1 drive code
;Output Registers:	AH = return status
;			CY = 0 no error
;			   = 1 error
;
Set_type	proc	near
	mov	ax,[bp+bp_ax]		;get input parameter
;
; ----- Rel. 3.3
;
;	cmp	hf_cntrl,0		;check controller type
;	jz	set_type_rtn		;return if not dual board
	cmp	al,3			;check value in al
	ja	set_type5		;bad command
	cmp	al,1			;is it 360/320 in 360/320 drive
	jne	set_type1		;no check for 1.2M
	mov	al,93h			;set state for 360/360
set_type4:
	call	set_state		;go set state
	ret				;
set_type1:
	push	ax			;
	call	Read_dsk_line		;
	jnz	set_type2		;line active
set_type6:
	pop	ax			;get back input parameter
	cmp	al,2			;is it 360 media in 1.2M drive
	je	set_type3		;
	cmp	al,3			;is it 1.2M media in 1.2M drive
	jne	set_type5		;no bad command
	mov	al,15h			;1.2M media in 1.2M drive est
	jmp	short set_type4		;
set_type3:
	mov	al,74h			;320/360 media in 1.2M drive est
	jmp	short set_type4		;
set_type2:
	mov	dx,[bp+bp_dx]		;get input parameters
	mov	ch,01			;
	mov	dsk_opn_status,6	;set media change flag
	push	dx
	call	seek_opn		;
	mov	ch,0			;
	pop	dx			;
	push	dx			;
	call	seek_opn		;
	pop	dx			;
	call	Read_dsk_line		;
	jz	set_type6		;
	pop	ax
	mov	motor_timout,80h	;indicate timeout
	mov	al,default		;power on default value
	jmp	short set_type4		;

set_type5:
	mov	dsk_opn_status,1	;bad command
set_type_rtn:	ret			;and return
set_type	endp


page


D1      LABEL   WORD		        ;TABLE OF OFFSETS TO DISK ROUTINES
        DW      OFFSET DSK_RESET	; ah = 0
        DW      OFFSET DSK_STATUS	; ah = 1
        DW      OFFSET DSK_READ		; ah = 2
        DW      OFFSET DSK_WRITE	; ah = 3
        DW      OFFSET DSK_VERIFY	; ah = 4
        DW      OFFSET DSK_FORMAT	; ah = 5
	dw	offset read_dsk_type	; ah = 15h
	dw	offset Change_Disk	; ah = 16h 
	dw	offset Set_Type		; ah = 17h 
D1LENGTH EQU    $-D1
;
;       DISK  PARAMETER TABLE
;       DATA VARIABLE DSK_PARM_PTR CONTAINS A POINTER TO
;       TO THIS TABLE.
;
DISK_PARM       LABEL    BYTE
        DB      0CFH                   	;1ST SPECIFY BYTE   rlm 02/20/85
                                       	;STEPRATE TIME=4 MS,HD UNLOAD TIME=240MS
        DB      02H                    	;2ND SPECIFY BYTE
                                       	;HD LOAD TIME=2 MS, DMA MODE
        DB      MOTOR_WAIT             	;MOTOR WAIT TIME
        DB      02H                    	;512 BYTES/SECTOR
        DB      0FH                    	;FINAL SECTOR NUMBER=8(EOT)
        DB      1BH                    	;GAP LENGTH (GPL)
        DB      0FFH                   	;DATA LENGTH(DTL)
        DB      054H                   	;FORMAT GAP LENGTH (GAP 3)
        DB      0F6H                   	;FORMAT FILLER BYTE
        DB      25                     	;HEAD SETTLE TIME= 25 MSEC
        DB      8                    	;MOTOR START TIME EXPRESSED IN 1/8 SECS


page

;******************Please Note this ***************************************
;
;	The size of this file must be 835h
;

empty_space	db	147 dup (0)

end_of_INT40	label	byte

	if	(end_of_INT40 - start_of_int40)

;	error

	endif

EPROMA	ENDS
	end

