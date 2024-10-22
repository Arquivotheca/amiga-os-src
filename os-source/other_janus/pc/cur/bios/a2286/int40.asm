include	title.inc
subttl	                   INT 40H   Floppy Driver for AT

.xlist
include externals.inc
include equates.inc

public	disk_parm_tbl
public	floppy_intr			;1/9/85
public	recal_opn			;1/9/85
public	floppy_io
public	detrm_drv_type
extrn	fixed_wait:near
.list

zero	segment	at 0h
zero	ends

DATAA	SEGMENT	AT 40H
DATAA	ENDS


EPROMA	segment	  byte public

ASSUME CS:EPROMA,DS:DATAA,ES:DATAA

public aa_int40
aa_int40 label byte

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
;	8 - Read Drive parameters
;      15 - read disk type
;      16 - check disk change status line
;      17 - Set DASD type for Format
;      18 - Set Media Type for Format
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
	cmp	ah,18h			;
	ja	disk_io_2		;bad command
	cmp	ah,1			;v2.04 drive always valid for 
	jbe	valid_drv		;function 0 & 1 (W. Chessen)
	cmp	dl,1			;check for valid drive code
	ja	disk_io_2
valid_drv:
	jmp	D2			;go execute command
disk_io_2:
	mov	dsk_opn_status,cmd_err	;bad command error
	xor	al,al			;0 sectors Xfered
	mov	ah,cmd_err		;
	mov	dsk_opn_status,ah	;
	stc				;set carry flag
	jmp	disk_IO_15  		;operation complete

;	determine drive type and media type before executing
;	command. We assume that CMOS is good and drive type in
;	cmos is determined and verified at Boot-up
;

branch	proc	near
	cmp	ah,2			;check for reset or status
	jb	branch_6		;command	

;	convert state from old format to new format  (Xlat_New)

	call	get_state		;get current state of drive
	or	al,al			;is it zero
	jnz	branch_1		;no, leave it alone
	call	read_flp_type		;get floppy type from cmos
	jnz	branch_8		;0 means no drive present
	jmp	bad_cmd_rtn		;
branch_8:
	dec	ah			;check for 360kb drive
	jz	branch_3		;yes
	mov	al,track_80		;otherwise set for 80 track
	jmp	short branch_4		;
branch_3:
	mov	al,94h			;set 360kb drive /media
	jmp	short branch_4		;

;	modify low 3 bits of state from hf_cntrl

branch_1:
	mov	ah,hf_cntrl		;get current status
	call	get_state		;get current state
	and	al,0f8h			;save only necessary bits
	or	dl,dl			;drive 0
	jz	branch_5		;yes
	shr	ah,4			;
branch_5:
	and	ah,0fh			;keep bits for drive 0
	or	al,ah			;add new bits for drive 1
branch_4:	
	call	set_state		;
		
;	check for media change and disk change line
	cmp	byte ptr [bp+bp_ax+1],5	;check input function
	ja	branch_6		;no media change required
	call	read_dsk_line		;
	jz	branch_6		;
	jmp	disk_io_8		;go handle disk change line

;	go execute the command

branch_6:
	mov	ax,[bp+bp_ax]		;restore input registers
	mov	dx,[bp+bp_dx]		;
	mov	cx,[bp+bp_cx]		;
	cmp	ah,19h			;valid command
	jb	branch_7		;yes
	jmp	bad_cmd_rtn		;bad command
branch_7:
	mov	si,offset function_tbl	;get address of function table
	xor	bx,bx			;
	mov	bl,ah			;
	add	bl,bl			;word offset
	add	si,bx			;get correct address
	mov	bx,[bp+bp_bx]		;restore bx
	cmp	dsk_opn_status,0ffh	;is it retry
	je	branch_18		;yes
	cmp	ah,1		 	;v2.04  save status for dsk_status 
	jne	branch_18a		;  function (ah = 1)  (W. Chessen)
	mov	ah,dsk_opn_status
branch_18a:
	mov	dsk_opn_status,0	;set status to 0 otherwise
branch_18:
	jmp	cs:[si]			;go execute command
branch	endp				;

D2:
	call	branch			;

;	come here after execution of the code. check for retry 
;	attempts. Retries are done only in case of r/w/v commands
;

	push	ax			;save return parameters
	cmp	byte ptr [bp+bp_ax+1],2	;check input function
	jb	d2_30			;no xlat for reset or status
	call	xlat_old		;return to old format
d2_3:
	mov	ah,dsk_opn_status
	or	ah,ah			;no error
	JNZ	D2_3A
	JMP	dsk_opn_done		;yes, then done
D2_3A:	call	get_state		;get return
	test	al,media_det		;is media established
	JZ	D2_3B
	JMP	dsk_opn_done		;yes, then no retries
D2_3B:	cmp	ah,time_out		;timeout error
	jne	d2_4			;(W.Chessen)
	jmp	disk_io_14		;

d2_4:	cmp	dsk_opn_status,dma_bndry_err	;if dma error then no retirs
	jne	d2_5			;(W.Chessen)
	jmp	disk_io_14  		;yes, then no retries

d2_5:	cmp	dsk_opn_status,media_change	;if media change then no
	JNE	D2_6
	JMP	disk_io_14		;retries
D2_6:	cmp	byte ptr [bp+bp_ax+1],2	;check for function code
	JAE	D2_7
	JMP	disk_io_14  		;retries only for r/w/v
D2_7:	cmp	byte ptr [bp+bp_ax+1],5	;
	JBE	D2_8
	JMP	disk_io_14  		;
D2_8:	and	al,0c0h			;check rate
	mov	ah,data_rate		;
	and	ah,0ch			;
	shl	ah,4
	cmp	al,ah			;have we tried all rates
	je	disk_io_14  		;yes, then return
	shr	al,6			;rate in bits 0 and 1

;	here is algorthim to change to next rate (bits 6,7)
;		00 ===> 10    	500 to 250
;		10 ===> 01	250 to 300
;		01 ===> 00	300 to 500
;
	or	al,al			;is the rate 0
	jz	d2_1			;yes
	dec	al			;change rate to new
	jmp	short d2_2		;
;-----------------------------------------------------------------
;	changes made to  fix 360k in 1.2 m floppy error
;
d2_30:
	jmp	disk_io_14		;if function status or reset then
					;just exit
;----------------------------------------------------------------
d2_1:
	mov	al,2			;
d2_2:
	shl	al,6			;put bits in appropriate place
	mov	ah,al			;save it
	call	get_state		;
	and	al,3fh			;get rid of old rate
	or	al,ah			;add new rate
	call	set_state		;
	pop	ax
	mov	ax,[bp+bp_ax]
	mov	dx,[bp+bp_dx]		;restore registers	
	mov	cx,[bp+bp_cx]
	mov	bx,[bp+bp_bx]
	mov	dsk_opn_status,0ffh	;set retry flag.
	OR	DL,DL
	JNZ	DRV_1
	AND	RECAL_REQD,0FEH
	JMP	d2
DRV_1:	AND	RECAL_REQD,0FDH
	jmp	d2			;
DSK_OPN_DONE:
	mov	ax,[bp+bp_ax]		;(W.Chessen)
	cmp	ah,8h
	jb	det_med
	cmp	ah,16h
	jbe	no_med_det
det_med:
	call	get_state		;
	or	al,media_det		;make sure media is established
	call	set_state		;
no_med_det:
        MOV     BX,2                   	;GET MOTOR WAIT PARAMETER
        CALL    GET_PARAMETER
        MOV     MOTOR_TIMOUT,AH         ;KEEP MOTOR RUNNING FOR ABOUT 1.4 SECS
	mov	ax,[bp+bp_ax]		;fn 15 returns code in AH
	cmp	ah,15h			;
	jne	disk_io_14		;
	pop	ax			;
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
Floppy_IO	ENDP

;
;	come here is function code in ah is invalid
;
bad_cmd_rtn:
	mov	dsk_opn_status,cmd_err
	mov	ah,cmd_err
	xor	al,al			;
	ret				;return to caller

disk_io_8:
check_media	proc	near		;
	call	get_state		;
	and	al, not media_det	;
	call	set_state		;
	mov	al,1			;
	add	al,dl			;get mask bits for motor on
	cli
	not	al
	and	motor_on,al		;
	sti
	call	start_motor		;start motor
	call	dsk_reset		;go reset the NEC
	mov	dx,[bp+bp_dx]		;
	mov	ch,01			;seek to track 1
	push	dx
	call	seek_opn		;
	pop	dx			;
	mov	ch,0			;
	call	seek_opn		;
	call	read_dsk_line		;check 1 more time
	jz	disk_io_9		;not active
	mov	dsk_opn_status,time_out	;no media, timeout
	mov	ah,time_out		;set error code
	xor	al,al			;
	stc				;set carry
	ret				;command complete
disk_io_9:
	mov	ah,media_change
	push	ax
	mov	ax,[bp+bp_ax]
	cmp	ah,18h			;v2.02 (W. Chessen)
	pop	ax
	je	chk_m_ret
	mov	dsk_opn_status,media_change
	stc
chk_m_ret:
	ret				;
check_media	endp			;



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
       
DSK_RESET       PROC    NEAR           
        CLI                            ;DISABLE INTERRUPTS
        MOV     bl,MOTOR_ON            ;GET MOTOR ON STATUS
	ror	bl,4			;
	and	bl,0f3h			;
	mov	al,bl			;
        OR      AL,8                   ;SET "ENABLE INT & DMA " BIT
        MOV     DX,DOR                 ;SETUP PORT ADDR FOR DIGITAL OUTPUT REG
        OUT     DX,AL                  ;OUTPUT TO DIGITAL OUT REG(DOR)
        MOV     RECAL_REQD,0           ;SET RECAL REQUIRED ON ALL DRIVES
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
D11:    MOV     AH,03                  
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
        MOV     DSK_OPN_STATUS,AH      ;RESTORE SAVED STATUS
	mov	al,byte ptr [bp+bp_ax]	;
        RET
DSK_STATUS      ENDP                     


;******************************************************************************
;
; DSK_READ          
; DSK_WRITE
; DSK_VERIFY 	
; DSK_FORMAT
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
;******************************************************************************
       
DSK_READ        PROC    NEAR           
	call	init_rate		;
	jc	dsk_opn_ret		;error return
        MOV     AX,0E646H               ;PUT READ CMD FOR DMA IN AL   11/20/84
	                               	;PUT READ CMD FOR FDC IN AH
	and	motor_on,07fh		;read operation
        JMP     SHORT DSK_OPN          	;GO TO COMMON SECTION
;
DSK_WRITE       PROC    NEAR           
 	call	init_rate
	jc	dsk_opn_ret
        OR      MOTOR_ON,80H           	;SET WRITE BIT
        MOV     AX,0C54AH              	;PUT WRITE CMD FOR DMA IN AL  11/20/84
                                       	;PUT WRITE CMD FOR FDC IN AH
        JMP     SHORT DSK_OPN          	;GO TO COMMON SECTION
;
DSK_VERIFY      PROC    NEAR                 
 	call	init_rate
	jc	dsk_opn_ret
        MOV     AX,0E642H               ;PUT VERF CMD FOR DMA IN AL   11/20/84
                                       	;PUT READ CMD FOR FDC IN AH
	and	motor_on,07fh		;read operation 
       	JMP     SHORT DSK_OPN          	;GO TO COMMON SECTION
;
DSK_FORMAT      PROC    NEAR           
	call	set_fmt_rate		;set xfer rate for format
	jc	dsk_opn_ret		;return if error
        OR      MOTOR_ON,80H           	;SET WRITE BIT
        MOV     AX,4D4AH               	;PUT WRITE CMD FOR DMA IN AL
                                      	;PUT FMT   CMD FOR FDC IN AH
	jmp	short dsk_opn

dsk_opn_ret:
	ret				;
;
DSK_OPN 	PROC    NEAR           	;THIS IS A COMMON SECTION.
                                       	;READ/WRITE/VERF/FORMAT ROUTINES
                                       	;JUMP HERE
	push	ax			;
	call	send_specify		;send specify command and data rate
	pop	ax			;
        CALL    START_DMA              	;SETUP&START DMA CONTROLLER
        JNC     D20                    	;CHECK FOR DMA BNDRY ERROR
        MOV     DSK_OPN_STATUS,DMA_BNDRY_ERR   ;SET DMA BNDRY ERROR
	mov	ah,dsk_opn_status	;
        XOR     AL,AL                  	;SET SECTORS XFRD TO 0
	stc				;set carry
        RET
D20:    PUSH    AX                     	;SAVE CMD
        CALL    SEEK_OPN               	;SEEK TO CORRECT TRACK
        POP     AX                     	;RESTORE COMMAND
        JNC     D26                    	;no error
	jmp	D25			;jmp if seek error
D26:    MOV     BL,AH                  	;COMMAND TO BL
        MOV     DI,OFFSET D25          	;SETUP RETURN ADDR FOR FDC_OUT ROUTINE
        PUSH    DI
        CALL    FDC_OUT                	;OUTPUT THE COMMAND TO FDC
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
	mov	ah,cs:[si+gap_len]
	call	fdc_out			;go output gap length
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
	mov	bx,[bp+bp_bx]		;make sure register are good
	mov	es,[bp+bp_es]		;for data transfer
	mov	dh,byte ptr [bp+bp_ax]	;get # of sectors to read
	cli				;
        OUT     DMA1+11,AL             ;OUTPUT MODE BYTE
	jmp	short $+2
	jmp	short $+2
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
	jmp	short $+2
	jmp	short $+2
        AND     BL,0F0H                ;GET RID OF PAGE ADDR FROM BX
        OR      BL,CH                  ;MERGE SAVED NIBBLE
        PUSH    BX                     ;BX NOW CONTAINS DMA ADDR -- SAVE IT
        OUT     DMA1+12,AL             ;INITIALIZE FIRST/LAST FF
	jmp	short $+2
	jmp	short $+2
        MOV     AL,BL                                        
        OUT     DMA1+4,AL              ;OUTPUT DMA ADDR (LSB)
	jmp	short $+2
	jmp	short $+2
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
	jmp	short $+2
	jmp	short $+2
        MOV     AL,BH
        OUT     DMA1+5,AL              ;OUTPUT DMA WORD COUNT (MSB)
	sti				; 
        POP     AX                     ;GET SAVED ADDRESS
        ADD     AX,BX                  ;CARRY WILL SET IF 64K BNDRY CROSS
        MOV     AL,2
        OUT     DMA1+10,AL             ;CLEAR CH2 MASK BIT         
	jmp	short $+2
        POP     CX                     ;RESTORE REGS
        POP     AX                     
        RET
START_DMA       ENDP



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
;     IN CX
;
;******************************************************************************
       
WAIT_MSEC       PROC     NEAR          
        PUSH    CX                     ;SAVE REGS
	mov	cx,66			;66 times through wait rtn
	call	fixed_wait		;
	pop	cx			;
        RET
WAIT_MSEC       ENDP

;
;    WAIT 1/8 SECOND
;

WAIT_125MSEC    PROC    NEAR           
	push	bx
	mov	bx,125			;
wait_125_loop:
        CALL    WAIT_MSEC              ;WAIT 125 MILLISECONDS
	dec	bl			;
	jnz	wait_125_loop		;
	pop	bx			;
        RET
WAIT_125MSEC    ENDP

                                        
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
        MOV     AL,1                   	;
	add	al,dl			;drive mask to check
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
	test	data_rate,dual		;check for dual adapter
	jz	seek_1			;single board
	call	get_state		;get current drive state
	test	al,dbl_step		;double step rate required
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
	test	al,dbl_step		;did we modify original track #
	jz	seek_4			;no, then skip
	shr	ch,1			;yes, get back original track #
seek_4:
	PUSHF                          ;SAVE FLAGS
        MOV     BX,9                   ;GET HEAD SETTLE PARAMETER
        CALL    GET_PARAMETER
	push	bx			;
	mov	bl,ah			;head wait time in bl
seek_5:
        CALL    WAIT_MSEC              ;WAIT FOR HEAD SETTLE
	dec	bl			;
	jnz	seek_5			;
	pop	bx			;
	POPF                           ;RESTORE FLAGS
D52:	RET

seek_3:
	call	get_state		;
	test	al,20h			;test for double step rate
	jz	D52			;not require
	shr	ch,1			;required, adjust step rate
	ret				;

SEEK_OPN        ENDP                    

                                        
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
	push	cx			;Do a seek to outer track first
	push	dx			;  in case we somehow get into a
	mov	ch,0ch			;  track below track 0 (V2.01  W. Chessen) 
	mov	dl,1
	call	d50 
	pop	dx  
	pop	cx  

        MOV     AH,07H
        CALL    FDC_OUT                ;OUTPUT RECAL CMD TO FDC
        MOV     AH,DL
        CALL    FDC_OUT                ;OUTPUT DRIVE NUMBER TO FDC
        CALL    WAIT_INTR              ;WAIT FOR INTERRUPT
        JC      D55                    ;SKIP RESULT ANALYSIS IF ERROR
        CALL    ANALYZE_RESULT_1       ;ANALYZE RESULTS OF RECAL OPERATION
D55:    RET                            
RECAL_OPN       ENDP


                                        
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
	and	al,rate_mask		;isolate rate
	jz	get_parm1		;no state
	cmp	al,rate_250		;check for state 3
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

FDC_OUT         PROC    NEAR           
        PUSH    CX                     ;SAVE REGS
        PUSH    DX
	push	bx			;  11/26/84
	mov	bl,2			;go through the loop twice
        SUB     CX,CX                  ;SETUP TIMEOUT COUNT
        MOV     DX,FDC                 ;SETUP PORT ADDRESS
D60:    IN      AL,DX                  ;GET FDC MAIN STATUS
	jmp	short $+2
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
	jmp	short $+2
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
	push	bx			;
	clc				;
	mov	ah,090h			;call int 15
	mov	al,1			;
	int 	15h			;
	jc	D77			;
	mov	bx,2000			;2 sec waitloop
d75:
	mov	cx,70			;setup1msec loop
	call	fixed_wait		;processor independent wait
	test	recal_reqd,intr_flag	;check interrupt every1 msec
	jnz	d76			;
	dec	bx			;
	jnz	d75			;
D77:    OR      DSK_OPN_STATUS,TIME_OUT ;YES - SET TIMEOUT ERROR IN DISK STAT
	stc				;set carry flag
D76:    
	pop	bx			;
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
;        STI                            ;RENABLE INTERRUPTS
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
	or	dsk_opn_status,seek_err
	stc				;set carry
D80:	ret				;
D81:	clc				;
	ret				;

ANALYZE_RESULT_1    ENDP


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
;	mov	al,dl			;
;	rol	al,4			;move drive bits in correct 
;	and	motor_on,0cfh		;place
;	or	motor_on,al		;
;	mov	al,01			;
;	add	al,dl			;drive mask to check
;	cli
;	test	al,motor_on		;
;	jnz	L1			;motor already on, jump
;      	mov	motor_timout,0ffh	;set large timeout
;	or	motor_on,al		;insert drive code
;L1:	sti				;		
;	mov	bl,motor_on		;
;	ror	bl,4
;	and	bl,0f3h			;
;	or	bl,0ch			;setup dma and non reset bit
;	mov	al,bl			;
;	mov	dx,3f2h			;get DOR register
;	out	dx,al			;select drive
;	add	dl,5			;get DIR
;	call	nop_proc		;some delay
;	in	al,dx			;get status
;	and	al,80h			;check for bit 7
;---------------------------------------------------------------------
;	The above code has been commented out as it does not wait
;	for the  motor to come to speed if the motor is turned on
;	start_motor routine iis used to start the motor.
;	mods made on 10-14-1987
;--------------------------------------------------------------------
	call	start_motor		;enable drive and motor
	test	data_rate,dual		;
	jnz	ll1			;
	xor	ax,ax			;set the zero flag
	jmp	ll2			;
;-------------------------------------------------------------
ll1:
	mov	dx,3f7h			;
	in	al,dx			;get status
	and	al,80h			;check for bit 7
ll2:

	pop	dx			;
	pop	cx			;restore registers
	pop	bx			;
	pop	ax			;
	ret				;
Read_dsk_line	endp




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

;
;	get current floppy drive type from CMOS.
;	input	dl = drive number
;	output  ah = 0 means no drive present, Z flag set
;		   = drive type from CMOS
;	reg AX destroyed
;
read_flp_type	proc	near
	mov	al,flp_type
	call	read_cmos		;
	or	dl,dl			;drive number 0
	jnz	flp_type_1		;
	shr	ah,4			;
flp_type_1:
	and	ah,0fh			;
	ret				;
read_flp_type	endp			;


;
;	Read_disk_parms: This routines reads current disk parameters
;	and returns to the user.
;	Input Registers:	AH = 8
;				DL = drive #
;	Output Registers:	CL = max sector count (bits 0-5)
;				CH = max cylinder count
;				BL = Valid cmos drive type 1-4
;				DL = max # of drives installed
;				DH = max head number
;				ES:DI = address of drive parm tbl
;

read_disk_parms	proc	near
	push	dx
	mov	al,flp_type		;read cmos
	call	read_cmos		;
	mov	al,ah			;save it in al
	xor	dx,dx			;
	and	al,0f0h			;check for first drive
	jz	parms_1			;0 means drive A not present
	inc	dl			;
parms_1:
	mov	al,ah			;check 2nd drive
	and	al,0fh			;
	jz	parms_2			;2nd drive not present
	inc	dl
parms_2:
	mov	[bp+bp_dx],dl		;save # of drives installed in dl
	pop	dx			;get drive code back
	or	dl,dl			;is it drive 0
	jnz	parms_3			;no drive 1
	shr	ah,4			;
parms_3:
	and	ah,0fh			;isolate drive code
	or	ah,ah			;valid drive type
	jz	parms_err		;no drive type, error
	cmp	ah,max_drv_type		;check for valid type
	ja	parms_err		;error
	mov	[bp+bp_bx],ah		;save cmos drive type in bl
	mov	al,ah			;save drive type
	mov	ah,0
	dec	al			;
	shl	ax,4			;multiply by 4
	mov	bx,offset disk_parm_tbl	;get parameter table address
	add	bx,ax			;
	mov	[bp+bp_di],bx		;save offset in DI
	mov	[bp+bp_es],cs		;save segment address in ES
	mov	byte ptr [bp+bp_dx+1],1		;save max # of heads in dh
	mov	al,cs:[bx+sec_trk]	;get sectors per track
	mov	ah,cs:[bx+max_trk]	;get maximum tracks
	mov	[bp+bp_cx],ax		;save it for return in CX
	mov	dsk_opn_status,0	;set return status to 0
	clc				;
	ret				;return
parms_err:
	xor	ax,ax			;set values to 0
	mov	[bp+bp_bx],ax
	mov	[bp+bp_dx],ax
	mov	[bp+bp_di],ax
	mov	dsk_opn_status,cmd_err	;
	mov	ah,cmd_err		;bad command
	stc				;set carry
	ret				;
read_disk_parms	endp			;



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
	call	read_flp_type		;get floppy type from cmos
	jz	type_err		;
	cmp	ah,2			;is it above 3
	ja	type_err		;
	clc				;clear carry
	ret				;
type_err:
	xor	ah,ah			;error
	stc				;set carry
	ret				;
read_dsk_type	endp
	





;
;	Change_Disk:	This routine returns the state of the disk
;			change line.
;	Input Parameters:	AH = 16, DL = drive code
;	Output Parameters	AH = 0 disk change line not active
;				AH = 6 disk change line active

Change_Disk	proc	near
	test	data_rate,dual		;test for combo board
	jnz	change_disk_1		;
	mov	dsk_opn_status,00	;no combo board
	mov	ah,0			;
	clc				;
	ret				;
change_disk_1:
	call	get_state		;get current state
	or	al,al			;current state = 0
	jnz	change_disk_2		;no
change_disk_4:
	mov	dsk_opn_status,time_out	;timeout error
	mov	ah,time_out		;
	stc				;
	ret				;
change_disk_2:
	call	read_flp_type		;get floppy type from cmos
	jz	change_disk_4		;error if type 0
	cmp	ah,1			;type 1
	ja	change_disk_5		;
change_disk_6:
	mov	dsk_opn_status,media_change
	stc				;set carry
	mov	ah,media_change		;
	ret				;
change_disk_5:
	call	read_dsk_line		;
	jnz	change_disk_6		;
	xor	ah,ah			;
	clc				;
	ret				;
change_disk	endp


;
;Set_type: This routine is used to set DASD type for diskette format
;Input Registers:	AH = 17h
;			AL = 0	not used
;			   = 1 	diskette 320/360 in 320/360 drive
;			   = 2  diskette 320/360 in 1.2M drive
;			   = 3  diskette 1.2M in 1.2M drive
;			   = 4  diskette 720k in 720k drive
;			DL = 0 or 1 drive code
;Output Registers:	AH = return status
;			CY = 0 no error
;			   = 1 error
;

Set_type	proc	near
	cmp	al,4			;valid type
	ja	set_type_err		;error return
	push	ax			;
	call	check_media		;check for change in media
	jnc	set_type_3		;no error
	cmp	ah,time_out		;is it time out error
	je	set_type_4		;
	mov	dsk_opn_status,0	;set error code to 0
set_type_3:
	pop	ax			;
	push	bx			;
	mov	bx,offset set_type_state	;
	xor	ah,ah			;
	add	bx,ax			;point to correct byte for drive
	mov	ah,byte ptr cs:[bx]	;get state byte
	mov	bl,al			;save drive type
	call	get_state		;
	and	al,0fh			;
	cmp	bl,4			;is it type 4
	jz	set_type_1		;yes
set_type_2:
	or	al,ah			;set new state
	call	set_state		;
	pop	bx			;
	ret				;	
set_type_1:
	test	al,drive_det		;is drive determined
	jz	set_type_2		;no same as type 1
	test	al,2			;multiple format capacity
	jz	set_type_2		;no
	mov	ah,rate_300+media_det	;
	jmp	set_type_2		;
set_type_err:
	mov	ah,cmd_err		;command error
	mov	dsk_opn_status,ah	;
	stc				;set carry
	ret				;
set_type_4:
	pop	ax			;
	mov	ah,dsk_opn_status	;return timeout error
	stc				;
set_type	endp

set_type_state	label	byte
	db	0			;dummy entry
	db	rate_250+media_det	;360/360 for al = 1
	db	rate_300+dbl_step+media_det	;360 in 1.2M  for al = 2
	db	rate_500+media_det	;1.2 in 1.2M  for al =3
	db	rate_250+media_det	;initial value for al = 4



;
;	Set_Disk_Media:	This call is used to set media type and
;	data xfer rate for floppy before a format operation is
;	attempted.
;	Input Registers:  AH = 18h
;			  CL = max sectors per tracks (5 - 0)
;			  CH = max # of tracks
;			  DL = drive # 0 or 1 value checked
;	
;	Output	Registers: ES:DI = Pointer to drive parameter tbl for
;			   requested drive type
;			   AH = 0, CY = 0 valid drive
;			   AH = 1, CY = 1 function not available
;					older versions of BIOS
;			   AH = 0c,CY = 1 Drive type not supported
;

Set_disk_media	proc	near
	call	read_dsk_line			;v2.02 (W. Chessen)
	jz	go_set
	pusha
	call	check_media
	cmp	ah,80h
	popa
	jne	go_set
	ret

go_set:
	push	bx			;save BX
	push	dx
	call	read_flp_type		;read floppy type from cmos
	or	ah,ah			;is it 0
	jz	media_2			;yes, drive not present
	cmp	ah,max_drv_type		;check range
	ja	media_2			;invalid cmos
	mov	al,ah			;
	mov	dl,al			;save drive type
	xor	ah,ah			;
	dec	al			;
	shl	ax,4			;multiple type by 16
	mov	bx,offset disk_parm_tbl	;get table address
	add	bx,ax			;point to right table
	
;	check values to make sure parameters match

	mov	cx,[bp+bp_cx]		;get original cx in case it was destroyed
	cmp	cl,cs:[bx+sec_trk]	;sector/track does it match
	jne	media_3			;no try next media
	cmp	ch,cs:[bx+max_trk]	;compare max tracks
	je	media_4			;match

;	if we do not find the match in first 4 table then we look
;	at one of the two remaining tables. If drive type is 2 i.e
;	1.2M drive, and media is not 1.2Mb (table 2) then we look
;	into 1.2 M drive / 360Kb media (table 5)
;	similarly if drive type is 4 i.e 1.44Mb drive and media type
;	is not 1.44Mb (table 4) then we look into 1.44Mb drive/720 Kb
;	media. (table 6)

media_3:
	mov	bx,offset type_5_tbl	;
	cmp	dl,2			;is it true
	je	media_5			;yes BX = add of tbl_5
	add	bx,16			;no set bx = add of tbl_6
media_5:
	cmp	cl,cs:[bx+sec_trk]	;sec/trk match
	jne	media_err		;no drive not supported
	cmp	ch,cs:[bx+max_trk]	;max track match
	jne	media_err		;

;	check data xfer rate and set state

media_4:
	mov	al,cs:[bx+xfer_rate]	;get xfer rate
	cmp	al,rate_300		;check for double step
	jne	media_6			;
	or	al,dbl_step		;360 media/1.2Mb drive
media_6:
	or	al,media_det		;
	mov	ah,al			;
	pop	dx			;
	call	get_state		;get current state
	and	al,not dbl_step+rate_mask+media_det
	or	al,ah			;
	call	set_state		;
	mov	[bp+bp_di],bx		;save offset
	mov	[bp+bp_es],cs		;save segment
	mov	dsk_opn_status,0	;
	mov	ah,0			;
	pop	bx			;stack clean
	clc				;clear carry
	ret

media_2:
	mov	dsk_opn_status,cmd_err
media_7:
	mov	ah,dsk_opn_status	;bad command
	pop	dx
	pop	bx			;stack clean
	stc				;set carry
	ret

media_err:
	mov	dsk_opn_status,0ch	;media not supported
	jmp	short media_7
set_disk_media	endp



;	
;		Determine floppy Drive type                
;	Input Dl = drive number 0 or 1             
;	Output al = 1 drive has 40 trk capability  
;		  = 2 drive has 80 trk capadility  
;	          
;	Procedure: We first seek to track 48 which is valid track for
;		   1.2M drive and dead stop for 360kb. Then we seek 
;		   slowly back to track 0.
;

track_00	equ	10h

detrm_drv_type	proc	near
	call	start_motor
	call	recal_opn		;start motor and issue recal
;	jc	detrm_5			;recal error
	jnc	do_seek			;Go do seek if no error 
	mov	dsk_opn_status,0	;  Otherwise clear status and
	call	recal_opn		;  try again  (V2.01  W. Chessen)
	jc	detrm_5			;Recal error
do_seek:
	mov	ch,48			;seek to track 48
	call	seek_opn		;
	jc	detrm_5			;
	mov	cx,16			;seek loop
detrm_1:
	push	cx			;
	mov	ch,cl			;ch = track to seek
	call	seek_opn		;
	jc	detrm_err		;seek error
	mov	ax,offset detrm_err	;save return address
	push	ax			;on stack
	mov	ah,4			;sence drive status after every
	call	fdc_out			;seek to make sure we are at
	mov	ah,dl			;track 0
	call	fdc_out			;
	call	analyze_result_2	;get results
	test	fdc_results,track_00	;are we home
	pop	ax			;
	pop	cx			;
	jnz	detrm_2			;yes
	dec	cl			;go to next track
	cmp	cl,0ffh			;no track 0 found
	je	detrm_5			;error code, no drive found
	jmp	short detrm_1
detrm_2:
	mov	al,1			;
	or	cl,cl			;
	jnz	detrm_3			;must be 40 track
	inc	al			;set drive to 1.2M
detrm_3:
	clc				;
	ret				;
detrm_5:
	push	cx			;keep stack in order
detrm_err:
	pop	cx			;seek error
	mov	al,0			;
	call	set_state		;
	stc				;set carry flag
	ret				;
detrm_drv_type	endp			;



;
;	Start_Motor:	Start specified motor and wait for motor
;	startup time. If the motor is already on, then return. 
;	Otherwise call INT 15 fn 90 to tell BIOS that processor is
;	about to wait for a device. Wait time is 2 secs. A carry from
;	INT 15 suggest INT 15 performed minimum wait. 
;
;	Input	dl = drive code
;

Start_motor	proc	near
	push	dx			;
	push	ax			;
	mov	dh,dl			;save drive code
	shl	dl,4			;push drive code in right place
	cli				;no interrupt allowed while
	mov	motor_timout,0ffh	;set max timeout value
	mov	ah,motor_on		;checking motor status
	mov	al,dh			;
	inc	al			;
	and	ah,00110000b		;mask needed bits
	cmp	dl,ah			;motor already on
	jne	motor_1			;yes, no need to restart
	test	al,motor_on		;motor also selected
	jnz	motor_rtn			;yes, then donot wait for motor
motor_1:
	and	motor_on,0cch		;enable new drive
	or	motor_on,al		;set motor-on bits
	or	motor_on,dl		;set drive select bits
	mov	al,motor_on		;
	rol	al,4			;
	and	al,0f3h			;
	or	al,0ch			;reset and dma enable
	sti				;enable interrupts
	mov	dx,dor			;DOR register
	out	dx,al			;
motor_3:
	mov	ax,090fdh		;call INT 15
	int	15h			;
	jc	motor_rtn		;wait done

;
;	wait for motor start
;
motor_4:
	push	bx			;
	mov	bl,8			;8 units of wait time
motor_5:
	call	wait_125msec		;
	dec	bl			;
	jnz	motor_5			;
	pop	bx			;
motor_rtn:
	clc				;
	pop	ax
	pop	dx			;
	ret
start_motor	endp



;
;	Xlat_Old: This routine converts the new format of drive
;	state into old format for compatibility. This routine is
;	complement of Xlat_new routine. Xlat_new is called before
;	code branches to execute the function call and Xlat_old is
;	called before returning to caller.
;

Xlat_old	proc	near
	cmp	byte ptr [bp+bp_ax+1],8	;This is the kludge for function 8h
	je	got_drive		;  -- the drive num on stack is changed
	mov	dx,[bp+bp_dx]		;get drive code (if func 8, DL still 
got_drive:				;  has drive number)
	call	read_flp_type
	cmp	ah,max_drv_type		;is it valid drive
	ja	xlat_err		;no then error
	call	get_state		;get current state
	and	al,7			;get rid of unnessary bits
	mov	dh,hf_cntrl		;get previous status
	or	dl,dl			;
	jnz	xlat_2			;
	and	hf_cntrl,0f0h		;save status for drive B
	or	hf_cntrl,al		;add status for drive A
	jmp	short xlat_3		;
xlat_2:
	shl	al,4			;move current status of B
	and	hf_cntrl,0fh		;save status of A
	or	hf_cntrl,al		;add status of B

;	convert new format of drive state into old format

xlat_3:
	call	get_state		;get current state
	mov	bl,al			;save it
	and	bl,0c0h			;save only data_rate bits
	test 	ah,1			;drive type to be either 1 or 3
	jz	xlat_4			;jmp if  drive type is 2 or 4			
	cmp	bl,rate_250		;360kb in 360 drive
	jne	xlat_5			;check for 360kb in 1.2M drive
	mov	ah,0			;set state
	test	al,media_det		;is media determine
	jz	xlat_6			;no
	add	ah,3			;set determined state
	jmp	short xlat_6		;
xlat_5:
	cmp	bl,rate_300		;rate is 300
	jnz	xlat_7			;no, then unknown drive type
	test	al,dbl_step		;40trks media in 80 trk drive
	jz	xlat_7			;no, then unknow media
	mov	ah,1			;set 360 into 1.2M
	test	al,media_det		;is media determined
	jz	xlat_6			;no go set state
	add	ah,3			;
	jmp	short xlat_6		;

;	unknown media or drive
xlat_7:
	mov	ah,7			;
	jmp	short xlat_6		;

;	come here if drive type is either 1.2M or 1.44M

xlat_4:
	cmp	bl,rate_500		;rate must be 500
	jne	xlat_5			;otherwise unknown
	mov	ah,2			;
	test	al,media_det		;is media determined
	jz	xlat_6			;no
	add	ah,3			;

;	ah = low 3 bits of state to be replaced

xlat_6:
	and	al,0f0h			;get rid of other bits
	or	al,ah			;add new bits
	call	set_state		;go set state
	jmp	short xlat_rtn		;return

xlat_err:
	mov	al,0			;
	call	set_state		;
xlat_rtn:	
	ret				;
xlat_old	endp			;


;	Init_rate:	This routine sets up data xfer rates both
;	start rate and end rate. These parameters are used to check for
;	retry attemps. This routine is called by read,write,verify and
;	format function calls. If the routine does not find a valid 
;	drive code in the CMOS, it sets a carry, zero outs the state
;	and returns with error code of time out.
;	On exit SI will contain the address of drive/media parameter
;	table.
;
Init_rate 	proc	near
	push	ax			;
	call	read_flp_type
	or	ah,ah			;is cmos 0
	je	init_err		;yes, then error
	cmp	ah,max_drv_type		;is there invalid type
	ja	init_err		;yes
	push	ax			;save drive code
	cmp	dsk_opn_status,0ffh	;is it a retry
;	je	init_not_needed		;yes, then donot initialize rates
	jne	jeff
	jmp	init_not_needed
jeff:	call	get_state		;
	test	al,media_det		;is media determined
	jnz	init_7			;yes,then do not reset it
	and	data_rate,0f3h		;prepare to store end rate
	dec	ah			;check for type 1
	jnz	init_2			;360 drive
	mov	al,rate_250		;start rate
	mov	ah,rate_250		;end rate
	jmp	short init_4		;
init_err:
	mov	dsk_opn_status,time_out
	mov	al,0
	call	set_state		;
	pop	ax			;
	stc				;set carry
	ret				;
init_2:
	mov	al,rate_300		;start rate
	mov	ah,rate_250		;end rate
	test	data_rate,dual		;combo card?
	jnz	init_4			;yes
	mov	al,rate_250		;no, set start rate to 250
init_4:
	ror	ah,4			;
	or	data_rate,ah		;
	mov	ah,al			;save end rate
	call	get_state		;
	and	al,03fh			;
	or	al,ah			;put start rate in state
	call	set_state		;

;
;	now pick up parameters table for specified drive type and 
;	make sure that xfer rate define in the table matches with
;	one in the drive state.
;
init_7:
	pop	ax
	push	ax			;get drive type
	mov	al,ah			;
	dec	al			;
	xor	ah,ah			;
	shl	al,4			;multiply by 16
	mov	si,offset disk_parm_tbl	;get table address
	add	si,ax			;point to right table
	mov	ah,byte ptr cs:[si+xfer_rate]	;get data rate 
	call	get_state		;
	and	al,rate_mask		;get rate from state
	cmp	al,ah			;are they equal
	je	init_6			;yes

;
;	if the rates are not equal then we pick one of the two table
;	i.e. type_5_tbl which specifies that drive is 1.2m and media
;	is 360kb or type_6_tbl which specifies that drive is 1.44m
;	and media is 720kb.
;
	pop	ax			;get drive type
	mov	si,offset type_5_tbl	;assume 1.2M drive
	cmp	ah,2			;is it true
	je	init_5			;yes
	add	si,16			;no, then get type_6_tbl
init_5:
	clc				;
	pop	ax
	ret
init_6:
	pop	ax
	pop	ax			;make sure stack is cleaned up
	clc				;
	ret				;		

;	come here if we are doing a retry. IN that case we do not 
;	need to initialize first and last rates

init_not_needed:
	mov	dsk_opn_status,00	;reset status to 0
	jmp	short init_7		;
init_rate	endp


;
;	This routine sends specify command to FDC and after that it
;	sends a data rate to port 3f7. On entry cs:si point to drive
;	media table. If the command is a format command, then specify
;	parameters are used from the table pointed to by location 
;	00:78, 00:7B.
;
send_specify	proc	near
	push	es
	push	ax
	mov	ax,cs			;setup
	mov	es,ax			;
	cmp	byte ptr [bp+bp_ax+1],5	;is input command format
	jne	specify_1		;no, then we are ok
	xor	ax,ax			;yes
	mov	es,ax			;
	assume	es:zero
	les	si,es:dsk_parm_ptr	;get address of parameter table
	
specify_1:
	mov	ax,offset specify_err		;put error address on stack
	push	ax			;
	mov	ah,3			;send specify command
	call	fdc_out			;
	mov	ah,byte ptr es:[si+spec_1]	;get first specify byte
	call	fdc_out			;
	mov	ah,byte ptr es:[si+spec_2]	;get second specify byte
	call	fdc_out			;
	pop	ax			;get error address of the stack

;	now send data rate to the controller (send_rate) and also
;	set flag for double step in drive state (setup_dbl)
;

	push	dx			;
	mov	dx,[bp+bp_dx]		;get drive code
	call	get_state		;
	mov	ah,al			;
	and	al,rate_mask		;use only rate bits
	shr	al,6			;put it in low bits
	mov	dx,3f7h			;fdc port address
	out	dx,al			;
	and	data_rate,3fh		;
	shl	al,6			;put it in bits 6,7
	or	data_rate,al		;put new rate
	and	ah,not dbl_step		;test for double step
	cmp	al,rate_300		;is it double step
	jne	specify_2		;
	or	ah,dbl_step		;set double step bit
specify_2:
	mov	al,ah			;set new state
	mov	dx,[bp+bp_dx]		;
	call	set_state		;
	pop	dx			;
	pop	ax
	assume	es:nothing
	pop	es
	ret				;
specify_err:
	pop	ax			;
	pop	es			;restore stack
	stc				;set carry
	ret			;
send_specify	endp			;


;
;	Set_fmt_rate:	This routine sets up xfer rate for format
;	function. DL register contains the drive code.
;	

Set_fmt_rate	proc	near
	push	ax			;
	push	bx			;
	call	get_state		;get current drive state
	TEST	AL,MEDIA_DET	;SKIP IF MEDIA DETERMINED
	JNZ	RATE_RTN
	and	al,0fh			;get rid of rate bits
	mov	bl,al			;save it
	call	read_flp_type		;get floppy type from cmos
	or	ah,ah			;is it 0
	jz	rate_err		;yes, then drive present
	test	ah,1			;check for type 1 or 3
	jnz	fmt_360			;format 360 drive
	mov	al,bl			;get state
	or	al,10h			;rate_500 + media detr
	jmp	short rate_rtn		;return
fmt_360:
	mov	al,bl			;
	or	al,90h			;set rate_250 + media det
	cmp	ah,3			;check for 720 drive
	jne	rate_rtn		;it is 360 drive
	test	al,6			;test for drive detm
	jz	rate_rtn		;
	and	al,0fh			;get rid of rate
	or	al,50h			;add rate_300
rate_rtn:
	call	set_state		;go set state
	pop	bx
	pop	ax			;
	clc				;
	ret				;
rate_err:
	pop	bx			;
	pop	ax			;
	mov	ax,time_out		;no drive present
	mov	dsk_opn_status,time_out
	stc				;
	ret				;
set_fmt_rate	endp


function_tbl	LABEL   WORD		;TABLE OF OFFSETS TO DISK ROUTINES
        DW      OFFSET DSK_RESET	; ah = 0
        DW      OFFSET DSK_STATUS	; ah = 1
        DW      OFFSET DSK_READ		; ah = 2
        DW      OFFSET DSK_WRITE	; ah = 3
        DW      OFFSET DSK_VERIFY	; ah = 4
        DW      OFFSET DSK_FORMAT	; ah = 5
	dw	bad_cmd_rtn		; ah = 6
	dw	bad_cmd_rtn		; ah = 7
	dw	read_disk_parms		; ah = 8
	dw	bad_cmd_rtn		; ah = 9
	dw	bad_cmd_rtn		; ah = 0A
	dw	bad_cmd_rtn		; ah = 0b
	dw	bad_cmd_rtn		; ah = 0c
	dw	bad_cmd_rtn		; ah = 0d
	dw	bad_cmd_rtn		; ah = 0e
	dw	bad_cmd_rtn		; ah = 0f
	dw	bad_cmd_rtn		; ah = 10
	dw	bad_cmd_rtn		; ah = 11
	dw	bad_cmd_rtn		; ah = 12
	dw	bad_cmd_rtn		; ah = 13
	dw	bad_cmd_rtn		; ah = 14
	dw	offset read_dsk_type	; ah = 15h
	dw	offset Change_Disk	; ah = 16h 
	dw	offset Set_Type		; ah = 17h
	dw	set_disk_media		; ah = 18h 


;	Define Disk parameter table for various disk/media combination

disk_parm_tbl	label	byte

rate_250	equ	80h
rate_300	equ	40h
rate_500	equ	00
dual		equ	1
media_change	equ	6
rate_mask	equ	0c0h
max_drv_type	equ	4
media_det	equ	10h		;media determine bit in state
dbl_step	equ	20h		;double stepping required
drive_det	equ	4		;drive determined

;	define offsets in parameter table

spec_1		equ	0		;first specify byte
spec_2		equ	1		;second specify byte
M_Wait		equ	2		;Motor Wait time
Bytes_sec	equ	3		;bytes per sector
Sec_trk		equ	4		;sectors per track
gap_len		equ	5		;gap length
dtl_byte	equ	6		;DTL byte
gap_frmt	equ	7		;gap length for format
fil_byte	equ	8		;fill byte
HD_time		equ	9		;head settle time
Mot_srt		equ	10		;motor start time
max_trk		equ	11		;maximum track #
xfer_rate	equ	12		;data xfer rate
tbl_size	equ	16		;table size


;	Type 1:	360 Kb media in 360Kb drive

type_1_tbl	label	byte
	
	db	0dfh			;1st specify byte 
	db	2			;2nd specify byte
	db	25h			;Motor wait time
	db	2			;Bytes per sector (512)
	db	9			;EOT # of sectors per track
	db	2ah			;gap length
	db	0ffh			;DTL
	db	50h			;Gap length for Format
	db	0F6h			;fill byte for format
	db	0fh			;head settle time in millisecs
	db	8			;motor start time (1/8)secs
	db	39			;last track number
	db	rate_250		;data xfer rate
	db	0,0,0			;make table 16 bytes long

;	type 2:	1.2 Mb media in 1.2 Mb drive

type_2_tbl	label	byte
	
	db	0dfh			;1st specify byte
	db	2			;2nd specify byte
	db	25h			;motor wait time
	db	2			;sector size in byte (512)
	db	0fh			;EOT sectors per track
	db	1bh			;gap length
	db	0ffh			;DTL
	db	84			;gap length for format
	db	0f6h			;fill byte for format
	db	0fh			;head settle time in millisec
	db	8			;motor start time (1/8 sec)
	db	79			;last track number
	db	rate_500		;xfer rate
	db	0,0,0			;make table 16 bytes long

;	type 3:	720 Kb media in 720 Kb drive

type_3_tbl	label	byte
	
	db	0dfh			;1st specify byte
	db	2			;2nd specify byte
	db	25h			;motor wait time
	db	2			;sector size in byte (512)
	db	09			;EOT sectors per track
	db	2ah			;gap length
	db	0ffh			;DTL
	db	80			;gap length for format
	db	0f6h			;fill byte for format
	db	0fh			;head settle time in millisec
	db	8			;motor start time (1/8 sec)
	db	79			;last track number
	db	rate_250		;xfer rate
	db	0,0,0			;make table 16 bytes long

;	type 4:	1.44Mb media in 1.44 Mb drive

type_4_tbl	label	byte
	
	db	0afh			;1st specify byte
	db	2			;2nd specify byte
	db	25h			;motor wait time
	db	2			;sector size in byte (512)
	db	18			;EOT sectors per track
	db	1bh			;gap length
	db	0ffh			;DTL
	db	6ch		;gap length for format
	db	0f6h			;fill byte for format
	db	0fh			;head settle time in millisec
	db	8			;motor start time (1/8 sec)
	db	79			;last track number
	db	rate_500		;xfer rate
	db	0,0,0			;make table 16 bytes long

;	type 5: 360Kb media in 1.2Mb drive

type_5_tbl	label	byte
	
	db	0dfh			;1st specify byte
	db	2			;2nd specify byte
	db	25h			;motor wait time
	db	2			;sector size in byte (512)
	db	09			;EOT sectors per track
	db	2ah			;gap length
	db	0ffh			;DTL
	db	80 			;gap length for format
	db	0f6h			;fill byte for format
	db	0fh			;head settle time in millisec
	db	8			;motor start time (1/8 sec)
	db	39			;last track number
	db	rate_300		;xfer rate
	db	0,0,0			;make table 16 bytes long

;	type 6: 720Kb media in 1.44Mb drive

type_6_tbl	label 	byte
	
	db	0dfh			;1st specify byte
	db	2			;2nd specify byte
	db	25h			;motor wait time
	db	2			;sector size in byte (512)
	db	09			;EOT sectors per track
	db	2ah			;gap length
	db	0ffh			;DTL
	db	80			;gap length for format
	db	0f6h			;fill byte for format
	db	0fh			;head settle time in millisec
	db	8			;motor start time (1/8 sec)
	db	79			;last track number
	db	rate_250		;xfer rate
	db	0,0,0			;make table 16 bytes long

eproma	ends
	end

