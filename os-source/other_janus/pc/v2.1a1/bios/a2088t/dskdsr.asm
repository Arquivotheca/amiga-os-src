	NAME	DSKDSR
	TITLE	ROM DISKETTE DSR
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; ROM DISKETTE DEVICE & INTERRUPT SERVICE ROUTINES
;	THIS MODULE CONTAINS THE DEVICE SERVICE ROUTINE AND
;	INTERRUPT SERVICE ROUTINES
;	ACCESS TO THE DISKETTE DSR IS VIA S/W INTERRUPT 13H;
;	ACCESS TO THE INTERRUPT SERVICE ROUTINE IS VIA S/W
;	INTERRUPT 0EH.	FUNCTIONS SUPPORTED:
;
;		00H - RESET DISK SUB-SYSTEM
;		01H - READ DISK STATUS
;		02H - READ SECTOR(S) INTO MEMORY
;		03H - WRITE SECTOR(S)
;		04H - VERIFY SECTOR(S)
;		05H - FORMAT TRACK
;	Fixed disk
;		06h - (PC,XT) Format and set bad sector flags	(not implemented)
;		07h - (PC,XT) Format drive			(not implemented)
;		08h -	      Get current drive parameters	(not implemented)
;		09h -	      Initialize drive pair parameters	(not implemented)
;		0Ah -	      Read Long (include 4 ECC bytes)	(not implemented)
;		0Bh -	      Write long (include 4 ECC bytes)	(not implemented)
;		0Ch -	      Seek				(not implemented)
;		0Dh -	      Alternate disk reset		(not implemented)
;		0Eh - (PC,XT) Read sector buffer		(not implemented)
;		0Fh - (PC,XT) Write sector buffer		(not implemented)
;		10h -	      Test drive ready			(not implemented)
;		11h -	      Recalibrate			(not implemented)
;		12h - (PC,XT) Controller RAM diagnostic 	(not implemented)
;		13h - (PC,XT) Drive diagnostic			(not implemented)
;		14h -	      Controller internal diagnostic	(not implemented)
;	Fixed or floppy disk
;		15h - (AT)    Read DASD type
;	disk
;		16h - (AT)    Read disk change line
;		17h - (AT)    Set DASD type for format
;
;	WRITTEN: 10/19/83
;	REVISED: mm/dd/yy ** (initials)
;		 02/27/84 *A (DLK)
;		  - FIX CMD RANGE RETURN
;		  - RETURN DISK STATUS IN BOTH AL & AH
;		  - PUT 0 IN AL ON RESET ERROR RETURN
;		  - AL=0 ON READ/WRITE/VERIFY EXIT
;		  - FIX DMA BOUNDARY ERROR EXIT
;		  - UNMASK INTERRUPTS WHILE IN MOTOR SPEED WAIT LOOP
;		  - MAKE GETTING DPB ADDR A SUBROUTINE
;		 03/16/84 *B (DLK)
;		  - REWORK DISK RESET LOGIC
;		 03/21/84 *C (DLK)
;		  - FIX DISK RESET LOGIC (FORMAT FAILED)
;	CODE REDUCTION: mm/dd/yy *n (initials)
;		 04/05/84 *4 (DLK)
;		  - FOURTH PASS
;		 04/06/84 *5 (DLK)
;		  - FIFTH PASS
;		 6/85 (stan) added WAITHLT false option to not depend on timer
;			(some applications mess with it)
;		 7/85 (stan) changed error-reporting - report seek error only
;			if seek error; some code reduction
;		 8/85 (stan) no call to SETERR after seek fail; continue wait
;			for right interrupt after spurious int in DOSKSB
;		 9/85 (stan) RSLTRD fixed to wait for RQM in MSR between reads
;------------------------------------------------------------------------------
;  Code Adaption to Commodore PC - 11/25/85 Dieter Preiss Commodore BSW (DP):
;
;		 - Added support of 96 TPI diskette drives:
;		   Check drive number given in .DL against number
;		   of drives in DEVFLG.
;
;		      If DIP Switches specify 1..3 drives:
;
;			 Use bit 20H of .DL (Drive #) to step a
;			 96 TPI in single step mode (Drive Sel #4 active)
;			 If this bit is clear, use a 96TPI drive in compatible
;			 double step mode.
;			 Each Recalibrate is performed in 96 TPI mode, auto-
;			 matically ignored by 48TPI drives.
;
;		      If DIP switches specify 4 drives:
;
;			 Return Error if .DL is higher than 3; no 96 TPI
;			 diskette drives supported.
;
;		  - Corrected most of the documentation errors
;
;		  - Added some wait time during FDC reset
;
;		  - Deselect all drives on FDC reset, but leave motors on
;
;		  - Corrected Motor start time to reflect multiples of
;		    1/8 (.128 here) seconds instead of .246 seconds
;
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<DSKDSR - ROM Floppy Disk DSR and ISR code>
;
BIT0		EQU	01H
BIT1		EQU	02H
BIT2		EQU	04H
BIT3		EQU	08H
BIT4		EQU	10H
BIT5		EQU	20H
BIT6		EQU	40H
BIT7		EQU	80H
;
T80FLG		EQU	20H    ; Set if accessing a 96TPI drive in 96TPI mode
;
;
;------------------------------------------------------------------------------
;
; 765 Floppy Disk Controller (FDC) equates
;
FDCSR		EQU	X765+00h	; FDC MAIN STATUS REGISTER
FDCDR		EQU	FDCSR+01h	; FDC DATA REGISTER
;
;------------------------------------------------------------------------------
;
; 765 FDC Command bytes equates
;
MT		EQU	80h		; Multi-Track
MF		EQU	40h		; MFM mode if 1, else FM
SK		EQU	20h		; Skip Deleted Data Address Marks
;
HD		EQU	04h		; Head #
US		EQU	03h		; Unit select 0 - 3
;
RQM		EQU	80h		; Request for Master
DIO		EQU	40h		; Data Input/output
EXM		EQU	20h		; Execution mode, used only in non-DMA
					;   mode
CB		EQU	10h		; Command Busy
;------------------------------------------------------------------------------
;
;	Status Register 0 bit equates
;
IC		EQU	80h		; Invalid Command, command was never
					;   started
ATERM		EQU	40h		; Abnormal termination, command was not
					;   completed sucessfully
ATRDY		EQU	IC+ATERM	; Abnormal termination because FDD ready
					;   state changed during execution
NT		EQU	00h		; Normal termination
NR		EQU	08h		; FDD Not Ready
;------------------------------------------------------------------------------
;
SPFYCMD 	EQU	03h		; SPECIFY COMMAND
RECALCMD	EQU	07h		; RECALIBRATE COMMAND
SISCMD		EQU	08h		; SENSE INTERRUPT STATUS COMMAND
SEEKCMD 	EQU	0Fh		; SEEK COMMAND
;
WRITECMD	EQU	MT+MF+05h	; WRITE DATA COMMAND
FRMTCMD 	EQU	MF+0Dh		; FORMAT TRACK COMMAND
READCMD 	EQU	MT+MF+SK+06h	; READ DATA COMMAND
;
;------------------------------------------------------------------------------
;
; DISKETTE ERROR CODES
;
FDSKTMO 	EQU	80H		; DEVICE TIMEOUT/DID NOT RESPOND
FDSKERR 	EQU	40H		; SEEK FAILURE
FDCTERR 	EQU	20H		; CONTROLLER FAILURE
;
ERR01		EQU	01H		; ILLEGAL FUNCTION
ERR02		EQU	02H		; ADDRESS MARK NOT FOUND
ERR03		EQU	03H		; WRITE PROTECT ERROR
ERR04		EQU	04H		; SECTOR NOT FOUND
ERR06		EQU	06h		; Media change (AT 1.2M controller only)
ERR08		EQU	08H		; DMA OVERRUN
ERR09		EQU	09H		; DMA ACCESS ACROSS 64K BOUNDARY
ERR10		EQU	10H		; BAD CRC ON DISK READ
;
;------------------------------------------------------------------------------
;
DGROUP	GROUP	ROMDAT
INTVEC	SEGMENT AT 0000h	; The following are relative to Segment 00h
	EXTRN	DPBPTR:DWORD		; Disk Parameter Block pointer
INTVEC	ENDS
;
;------------------------------------------------------------------------------
;
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	DRVSTAT:BYTE		; DRIVE STATUS
	EXTRN	FDMOTS:BYTE		; MOTOR STATUS
	EXTRN	FDTIMO:BYTE		; DISK MOTOR TIMEOUT VALUE
	EXTRN	ERRSTAT:BYTE		; DISK STATUS
	EXTRN	DSKST:BYTE		; FLOPPY DISK CONTROLLER STATUS
	EXTRN	DEVFLG:WORD		; Device Configuration (*DP)
;
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CGROUP,DS:DGROUP,ES:NOTHING
;
	EXTRN	SAVREGS:NEAR		; SAVE REGISTERS		*6
	EXTRN	COMEXIT:NEAR		; COMMON DSR EXIT ROUTINE	*6
	EXTRN	RNGCHK:NEAR		; CHECK OPCODE RANGE		*4
	EXTRN	MS_DELAY:NEAR
;
	EXTRN	SETDS40:NEAR		; Set DS to ROM data segment (0040h)
;
; DISKDSR ENTRY POINT VIA S/W INT 13H
;
;	AH = OPCODE (00H - 05H)
;
	SYNC	DSKDSRA
;
IF	ENHFDD

	EXTRN   DSKDSR:NEAR
DSKDSRC:
        JMP     DSKDSR
        PRGSIZA <Floppy Disk DSR jump>          ; Display program size

ELSE	;ENHFDD

	PUBLIC	DSKDSR
DSKDSR	PROC	FAR
	CALL	SAVREGS 		; SAVE CALLER'S REGISTERS       *6
	MOV	DI,OFFSET XFEREND-2	; DI=LENGTH OF CMD JUMP TABLE	*4
	CALL	RNGCHK			; GO CHECK OPCODE		*4
	JC	EXITX			; JUMP ON RANGE ERROR		*4
	JMP	WORD PTR CS:[DI+OFFSET CMDXFER]  ; JUMP TO PROCESSOR
;
CMDXFER DW	OFFSET DSKRST		; RESET DISK SUB-SYSTEM
	DW	OFFSET DSKSTAT		; READ DISK STATUS
	DW	OFFSET SECREAD		; READ SECTOR(S) INTO MEMORY
	DW	OFFSET SECWRT		; WRITE SECTOR(S)
	DW	OFFSET SECVRFY		; VERIFY SECTOR(S)
	DW	OFFSET TRKFMT		; FORMAT TRACK
;
XFEREND EQU	$-CMDXFER
;
EXITX:
	MOV	ERRSTAT,ERR01		; INVALID OPERATION CODE
	JMP	EXIT
;
;------------------------------------------------------------------------------
;
; RESET DISK SUBSYSTEM (AH=00H)
;
;   OUTPUT:	AH = DISK STATUS
;
DSKRST:
	MOV	ERRSTAT,0		; INITIALIZE DISK STATUS BYTE
;
	AND	DRVSTAT,070H		; RESET RECALIBRATION BIT FOR ALL DRIVES
	MOV	AL,FDMOTS		; GET MOTOR STATUS		*B
	MOV	CL,4			; SHIFT TO UPPER NIBBLE 	*C
	SHL	AL,CL			;    and zero lower nibble
	OR	AL,08h			; Enable FDC INTS & turn off motors
					; AL = Select bits for drive 3
	AND	AL,39H			; Mask off all reserved bits
	MOV	DX,FDCDOR		; DX=I/O ADDR OF DIGITAL OUTPUT REGISTER
	OUT	DX,AL			; ISSUE FDC RESET

	MOV	CX,10
DSKRS1: LOOP	DSKRS1			; Allow reset time for FDC

	OR	AL,04H			; TURN OFF FDC RESET
	OUT	DX,AL			;  *
	STI				; ENABLE INTERRUPTS
;
	CALL	WAITINT 		; WAIT FOR INTERRUPT (IGNORE ERROR)
	CALL	SISSTAT 		; READ INTERRUPT STATUS
	JC	RESETERR		; JUMP IF ERROR
;
	XOR	AH,IC+ATERM		; Mask out all the other bits
	TEST	AH,ATRDY		; Abnormal termination because FDD ready
					;   state changed during execution
;
	JZ	RESET20 		; If reversed bits are 0, ignore other
					;   errors for now
	OR	ERRSTAT,FDCTERR 	;  * YES, CONTROLLER FAILURE
	JMP	SHORT RESETERR
;
RESET20:
	MOV	AL,SPFYCMD		; AL=SPECIFY COMMAND
	CALL	WRTCMD			; ISSUE SPECIFY COMMAND TO FDC
	JC	RESETERR		; JUMP IF ERROR
;
	XOR	BX,BX			; BX=BEGINNING OFFSET IN DPB	*5
	CALL	MLTCMD2 		; WRITE SPECIFY CMDS 2 commands
RESETERR:
;
	JMP	SHORT RWVCOMP		; AL = 0 means no sectors read
;
; READ SECTOR(S) (AH=02H)
; WRITE SECTOR(S) (AH=03H)
; VERIFY SECTOR(S) (AH=04H)
;
;   INPUT:	DL = DRIVE NUMBER (0-3)
;		DH = HEAD NUMBER (0-1)
;		CH = TRACK NUMBER (0-39)
;		CL = SECTOR NUMBER (1-8)
;		AL = NUMBER OF SECTORS TO READ, WRITE OR VERIFY (1-8)
;		ES:BX = BUFFER ADDRESS
;
;   OUTPUT:	AH = DISK STATUS
;		AL = 0
;
SECWRT:
	CALL	INITDMW 		; GO INITIALIZE DMA
	MOV	AX,80h*256+WRITECMD	; SET UP WRITE COMMAND and flag to say
					;   wait for motor speed
;
	JMP	SHORT WRTCOD		; GO TO COMMON READ/WRITE/VERIFY CODE
SECREAD:
	MOV	AH,46H			; SET UP READ DMA COMMAND
	JMP	SHORT RWVCODE		; GO TO COMMON READ/VERIFY CODE
SECVRFY:
	MOV	AH,42H			; SET UP VERIFY DMA COMMAND
RWVCODE:
	CALL	INITDMA 		; GO INITIALIZE DMA
	MOV	AX,00h*256+READCMD	; SET UP READ COMMAND FOR A VERIFY
					;   and flag to not wait for motor speed
;
; ISSUE READ, WRITE OR VERIFY SECTOR(S) COMMAND SEQUENCE (CMD IN AL)
;
WRTCOD:
	CALL	SELUNH			; Select unit in DL and head in DH
;
	JC	RWVCOM			; JUMP IF ERROR (CAN'T WRITE CMD)
	MOV	AL,CH
	CALL	WRTCMD			; WRITE TRACK # (C)
	JC	RWVCOM			; JUMP IF ERROR (CAN'T WRITE CMD)
	MOV	AL,DH			; RESTORE HEAD #
	CALL	WRTCMD			; WRITE HEAD # (H)
	JC	RWVCOM			; JUMP IF ERROR (CAN'T WRITE CMD)
	MOV	AL,CL
	CALL	WRTCMD			; WRITE CURRENT SECTOR # (R)
	MOV	BX,3			; BX=BEGINNING OFFSET IN DPB	*5
;
	MOV	CX,4			; CX=BYTE TO OUTPUT		*5
					; WRITE BYTES/SEC, SEC/TRK	*5
;					;  R/W GAP LEN & DATA LEN BYTE	*5
	CALL	FDCFIN
;
RWVCOMP:
	XOR	AL,AL			; RETURN AL=0			*A
RWVCOM:
	JMP	SHORT EXIT		; Finish up in status routine
;
; FORMAT TRACK (AH=05H)
;
;   INPUT:	DL = DRIVE NUMBER (0-3)
;		DH = HEAD NUMBER (0-1)
;		CH = TRACK NUMBER (0-39)
;		AL = # of sectors to format to see if we have a DMA boundary
;			error
;		ES:BX = BUFFER ADDRESS 4-BYTE TRACK INFO FIELDS
;		(C,H,R,N):
;		  C = TRACK NUMBER
;		  H = HEAD NUMBER
;		  R = SECTOR NUMBER
;		  N = BYTES/SECTOR (00=128,01=256,10=512,11=1024)
;
;   OUTPUT:	AH = DISK STATUS
;
TRKFMT:
	CALL	INITDMW 		; GO INITIALIZE DMA
	MOV	AX,80h*256+FRMTCMD	; SET UP flag to wait for motor speed
					;   and set Format track command in AL
;
	CALL	SELUNH			; Select unit in DL and head in DH
;
	JC	EXIT			; JUMP IF ERROR (CAN'T WRITE CMD)
;
	MOV	BX,3			; BX=BEGINNING OFFSET IN DPB	*5
	CALL	MLTCMD2 		; WRITE BYTES/SEC & SEC/TRK	*5
	MOV	BX,7			; BX=BEGINNING OFFSET IN DPB	*5
					; WRITE GAP LEN & FILLER BYTE	*5
	CALL	FDCFIN
;
; COMMON EXIT
;
EXIT:
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	MOV	BL,ES:[SI+2]		; GET DISK TIMEOUT COUNT
	MOV	FDTIMO,BL		; RESET TIMEOUT COUNT
	STI
	AND	DRVSTAT,NOT BIT7	; MAKE SURE INTERRUPT BIT IS RESET
;
	MOV	AH,ERRSTAT		; RETURN ERROR CODE IN AH
	OR	AH,AH
	JZ	EXIT1
EXIT2:
	STC
EXIT1:
	MOV	BP,SP			; Get stack pointer
	PUSHF				; Move flags to
	POP	[BP+20] 		;  place on stack holding them
	JMP	COMEXIT 		; COMMON DSR EXIT ROUTINE	*6
;
; READ DISK STATUS (AH=01H)
;
;   OUTPUT:	AH & AL = DISK STATUS
;
DSKSTAT:
	MOV	AL,ERRSTAT		; Get disk status in AL too
	JMP	SHORT EXIT
;
DSKDSR	ENDP
	PRGSIZ	<Disk DSR>
ENDIF 	;ENHFDD
;******************************************************************************
;
; Subroutine to select unit and head #
;
;	Input:	DS = Rom data area segment
;		DL = DRIVE NUMBER (0-3) + T80FLG
;		DH = HEAD NUMBER (0-1)
;		CH = TRACK NUMBER (0-39)
;		CL = SECTOR NUMBER
;		AL = command to pass to WRTCMD after DOSEEK
;	Output: ERRSTAT = bits 4 -> 0 set to either illegal function error
;		Carry Flag set on drive # too big,
;			on timeout or controller error, else not set
;		AL = ?
;		AH = ?
;		ERRSTAT bit 7 = 1 on timeout
;		ERRSTAT bit 5 = 1 on controller error
;		ES:SI = Address of DPB
;		FDTIMO = ?
;		FDMOTS = ?
;		BX = ?
;
	PUBLIC	SELUNH
SELUNH	PROC	NEAR
	JC	SELUN1			; EXIT DUE TO DMA BOUNDARY ERR	*A
;
; Changed (DP) -----------------------------------------------------------
;
	MOV	BH,BYTE PTR DEVFLG	; How many Drives installed ?
	ROL	BH,1			; Bit 6,7 -> 0,1
	ROL	BH,1
	AND	BH,3			; isolate bits
	MOV	BL,DL			; get a copy
	CMP	BH,3			; four drives installed ?
	JE	SELU10			; yes if eq
	AND	BL,NOT T80FLG		; else ignore T80FLG for now
SELU10: CMP	BH,BL			; Drive # in range ?

	MOV	ERRSTAT,ERR01		; Set illegal function error
	JC	SELUN1			; Drive is not valid, so exit

	CALL	DOSEEK			; SELECT DRIVE & SEEK TO SPECIFIED TRACK					; BH=3 if no 96TPI support

	JC	SELUN1			; JUMP ON ERROR

	AND	DL,NOT T80FLG		; Not longer required

	CALL	WRTCMD			; ISSUE COMMAND (AL still OK ?)
	JC	SELUN1			; JUMP IF ERROR (CAN'T WRITE CMD)
;
; Write second byte of command sequence (Head & Unit #)
;
WRTCMT: 
	MOV	AL,DH			; Build HD+US bits to follow CMD
	SHL	AL,1			; MAKE AL=HEAD # SHIFTED LEFT 2 BITS
	SHL	AL,1			;  *
	OR	AL,DL			; INCLUDE DRIVE NUMBER
	AND	AL,7			; fall thru to 
					;   WRITE UNIT SELECT & HEAD #
;
;******************************************************************************
;
; SUBROUTINE TO WRITE A COMMAND TO THE FDC
;
;	Input:	DS = Rom data area segment
;		AL = Command byte
;	Output: AL = ?
;		AH = ?
;		ERRSTAT bit 7 = 1 on timeout
;		ERRSTAT bit 5 = 1 on controller error
;		Carry Flag set on timeout or controller error, else not set
;
WRTCMD:
	PUSH	DX
	MOV	AH,AL			; SAVE COMMAND
	CALL	FDCST
	JC	WRTERR
	TEST	AL,DIO			; Q: DIRECTION FLAG TO FDC
	JNE	WRT15			;   No
;
; The carry flag is cleared here to return OK
;
	INC	DX			; DX = FDC Data register
	MOV	AL,AH			; RECALL CMD BYTE
	OUT	DX,AL			; WRITE THE COMMAND
	POP	DX
SELUN1:
	RET
WRT15:
	CALL	RSLTRD7 		; Empty the command result of 7 bytes
	OR	ERRSTAT,FDCTERR 	; Say it's a controller failure
	STC				; SET THE CARRY FLAG
WRTERR:
	POP	DX
	RET
;
SELUNH	ENDP
	PRGSIZ	<Write FDC command and Select Unit and head>
;******************************************************************************
;
; Format Command mode continue & finish
;
FDCFIN	PROC	NEAR
	JC	FDCFI0			; JUMP IF ERROR
;
	CALL	MULTCMD 		; WRITE GAP LEN & FILLER BYTE	*5
	JC	FDCFI0			; JUMP IF ERROR 		*5
;
	CALL	WAITINT 		; Wait for operation completed
	JC	FDCFI0			; JUMP IF ERROR (TIMEOUT)
;
	CALL	RSLTRD7 		; READ COMMAND RESULT 7 bytes
	JC	FDCFI0			; JUMP IF ERROR (CAN'T READ RESULTS)
;
	AND	AH,ATERM+IC		; Normal completion ?
	JZ	FDCFI0			;   Yes, disk operation complete
	MOV	AL,FDCTERR
	CMP	AH,ATERM		; abnormal termination ?
	JNE	SETER2			;   no, call it an FDC error
					;   else fall through to identify error
;******************************************************************************
;
; SUBROUTINE TO INTERPRET STATUS BITS RETURNED BY FDC
;
;	Input:	DS = Rom data area segment
;	Output: ERRSTAT = FDC ERROR CODE
;		AL = ? (0 or error code)
;		BX = ? (0 - 7)
;
SETERR:
	MOV	AL,BYTE PTR DSKST+1	; Fetch ST1
;
	AND	AL,10110111B		; MASK OFF UNWANTED BITS
	MOV	BX,8			; BL=NUMBER OF BITS TO LOOK AT
SETER0:
	DEC	BX
	SHR	AL,1			; Get next bit and fill MSB with zero
	JC	SETER1			; Is error bit set ?
	JNZ	SETER0			; No, keep looking until we zero the
					;   register
FDCFI0:
	RET				; return IF NO ERROR FOUND
;
SETERRT DB	ERR04			; SECTOR NOT FOUND (msb)
	DB	FDCTERR
	DB	ERR10			; CRC ERROR
	DB	ERR08			; DMA OVERRUN
	DB	FDCTERR
	DB	ERR04			; SECTOR NOT FOUND
	DB	ERR03			; WRITE PROTECTED
	DB	ERR02			; MISSING ADDRESS MARK (lsb)
;
SETER1:
	MOV	AL,[BX+SETERRT] 	; GET ERROR MASK
SETER2:
	OR	ERRSTAT,AL		; INCLUDE IT IN ERRSTAT
	RET
;
FDCFIN	ENDP
	PRGSIZ	<Determine disk error bits and Finish Disk operation code>
;******************************************************************************
;
; Read FDC Status
;
;	Input:	DS = Rom data area segment
;	Output: AL = status
;		DX = offset FDCSR
;		ERRSTAT bit 7 = 1 on timeout
;		Carry Flag set on timeout
;
; Notes: The RQM bit can change at no faster than 12 microsecond intervals.
;	 This means that the CPU need only check it at this rate.  If we had
;	 a faster timer tic, we would do it with a HLT.  But to minimize
;	 latency time to a ready condition, we do it with a software loop
;
FDCST	PROC	NEAR
	PUSH	CX
	MOV	DX,FDCSR		; DX=I/O ADDR OF FDC MAIN STATUS REG
	XOR	CX,CX
FDCST0:
	IN	AL,DX			; READ FDC STATUS
	TEST	AL,RQM			; Request for Master, is FDC ready ?
	JNZ	FDCST1			;   Yes, and clear carry
	LOOP	FDCST0			;   No, spin
	OR	ERRSTAT,FDSKTMO 	; SET TIMEOUT ERROR
	STC				; Indicate error
FDCST1:
	POP	CX
	RET
FDCST	ENDP
	PRGSIZ	<Get status from FDC>
;******************************************************************************
;
; SUBROUTINE TO SELECT DRIVE AND SEEK TO SPECIFIED TRACK with head load and 
;	motor-spin up
;
;	Input:	DS = Rom data area segment
;		DL = DRIVE NUMBER (0-3)
;		DH = HEAD NUMBER (0-1)
;		CH = TRACK NUMBER (0-39)
;		BH = Number of drives (no 96TPI support if =3)
;
;   OUTPUT:	C FLAG = 0 IF SEEK IS SUCCESSFUL
;		C FLAG = 1 IF SEEK FAILED
;		ERRSTAT bit 7 = 1 on timeout
;		ES:SI = Address of DPB
;		FDTIMO = ?
;		FDMOTS = ?
;		BX = ?
;
	PUBLIC	DOSEEK
DOSEEK	PROC	NEAR
	PUSH	AX
	PUSH	CX
;
	CLI				; MASK INTERRUPTS
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	MOV	ERRSTAT,0		; Drive is valid, so set ERRSTAT
					;   back to 0
;
; FIRST TURN ON MOTOR AND SELECT DRIVE
;
	MOV	AX,110H
	MOV	CL,DL			; CL = Drive #
	AND	CL,3			; Leave drive # bits
	SHL	AX,CL			; AL = Motor on bit (4..7)
					; AH = select bit (0..3)
	MOV	BL,AH			; Save a copy
	OR	AL,0CH			; INCLUDE RESET & INTERRUPT BITS
	OR	AL,CL
;
; Check whether RECALIBRATE is necessary:
;
; If drive bit (0..3) in DRVSTAT is zero, this drive needs
; a recalibrate.
; If this drive shall seek to track 00, also do a recalibrate.
;
; If RECALIBRATE is required and we support 96TPI drives,
; always set 96TPI mode.
;

DOSE30: AND	AH,DRVSTAT		; Seek to track 00 ?
	JZ	DOSE05			; Yes if Z
	SUB	AH,AH
	OR	AH,CH			; Track 00 requestded ?
	JZ	DOSE05			; Yes if Z
;
; No Recalibrate required - AH not zero
;
	TEST	DL,T80FLG		; 96TPI mode requested ?
	JZ	DOSE10			; No if Z

DOSE05: CMP	BH,3			; 96TPI mode valid ?
	JE	DOSE10			; No special seek if 4 drives
	OR	AL,80H			; Else force 96TPI mode

DOSE10: PUSH	DX
	MOV	DX,FDCDOR		; DX=I/O ADDRESS OF DIGITAL OUTPUT REG
	OUT	DX,AL			; TURN ON DISKETTE MOTOR
	POP	DX

	AND	DL,NOT T80FLG		; Must be clear from now

	SUB	BH,BH			; Assume motor on
	TEST	FDMOTS,BL		; Was Motor already on ?
	JNZ	DOSK0			; Yes if NZ
;
	AND	FDMOTS,0F0H		; Clear all motor on bits and set this
					;   drive motor on bit
	OR	FDMOTS,BL		; Set selected bit to on	
	MOV	BH,ES:[SI+10]		; Motor start time for later use
;
DOSK0:
	MOV	FDTIMO,0FFh		; Make a large motor timeout value
	STI				; ENABLE INTERRUPTS
;
; NOW DO THE SEEK
;
	OR	AH,AH			; Recalibrate required ?
	JNZ	DOSK2			; Yes if Z
;
; 2 recalibrate attempts
;
	CALL	RECALIBRATE		; Try another time if track was > 77
	JC	SEEKERR

	OR	DRVSTAT,BL		; Everything is ok so say RECAL not
	OR	CH,CH			; Target track = 00?
	JZ	DOSK2A			; Done in this case

DOSK2:	MOV	AL,SEEKCMD		;
	CALL	WRTCMD			; ISSUE SEEK COMMAND
	JC	SEEKERR 		; JUMP IF ERROR (CAN'T WRITE CMD)
	CALL	WRTCMT			; WRITE UNIT SELECT & HEAD #
	JC	SEEKERR 		; JUMP IF ERROR (CAN'T WRITE CMD)
	MOV	AL,CH			; Write new track # in AL
	CALL	DOSKSB
;
	JC	SEEKERR 		; JUMP IF ERROR
;
DOSK2A:
	MOV	AH,BH			; Motor start time
	SUB	AL,AL
	SHR	AX,1			; times 128 ms
	JNZ	DOSK2B			; Wait if this time not zero
	ADD	AL,ES:[SI+9]		; plus head settling time   
	JZ	SELDONE 		; skip if also zero
DOSK2B:
	CALL	MS_DELAY

SELDONE:
	AND	FDTIMO,00h		; MAKE MOTOR TIMEOUT AS LARGE AS POSSIBLE
					; and clear C
SEEKERR:
	POP	CX
	POP	AX
	RET
DOSEEK	ENDP
	PRGSIZ	<Select disk and seek with head load>
;------------------------------------------------------------------------------
;
; SUBROUTINE TO GET ADDRESS OF DISK PARAMETER BLOCK			*A
;
;	Input:	None
;	Output: ES = SEGMENT ADDR OF DPB
;		SI = SEGMENT OFFSET
;
DPBADR	PROC	NEAR						       ;*A
	XOR	SI,SI			; SET EXTRA SEGMENT TO 0	*A
	MOV	ES,SI			; ES=0				*A
	ASSUME	ES:INTVEC
	LES	SI,ES:DPBPTR		; GET DISK PARAMETER BLOCK	*A
	ASSUME	ES:NOTHING
	RET							       ;*A
DPBADR	ENDP							       ;*A
	PRGSIZ	<Get address of DPB>
;******************************************************************************
;
; SUBROUTINE TO REQUEST SENSE INTERRUPT STATUS
;
;	Input:	DS = Rom data area segment
;	Output: AL = ? if carry set
;		AH = ? if carry set
;		AL = PRESENT CYLINDER NUMBER if no carry
;		AH = ST0 BYTE (FROM FDC) if no carry
;		ERRSTAT bit 7 = 1 on timeout
;		ERRSTAT bit 5 = 1 on controller error
;		Carry Flag set on timeout or controller error, else not set
;		DX = offset FDCSR
;		DSKST = FDC ST0, valid only if no carry
;
SISSTAT PROC	NEAR
	PUSH	CX
	PUSH	SI
	MOV	SI,OFFSET DSKST
	PUSH	[SI]			; Save top 2 bytes from FDC status list
;
	MOV	AL,SISCMD		; AL=SENSE INTERRUPT STATUS CMD
	CALL	WRTCMD			; ISSUE INTERRUPT STATUS CMD
	JC	SISERR			; JUMP IF ERROR (CAN'T WRITE CMD)
;
	MOV	CX,2
	CALL	RSLTRD			; READ RESULT
	JC	SISERR			; JUMP IF ERROR (CAN'T READ RESULTS)
;
	MOV	AX,[SI] 		; GET RETURNED STATUS BYTES (2)
	XCHG	AH,AL
	POP	[SI]			; Return original FDC status bytes
					; Say no error with carry flag cleared
	JMP	SHORT SISEXIT
SISERR:
	POP	SI			; RESTORE STACK
					; The carry flag is set on error
SISEXIT:
	POP	SI
	POP	CX
	RET
SISSTAT ENDP
	PRGSIZ	<Sense disk interrupt status>
;******************************************************************************
;
; SUBROUTINE TO WRITE MULTIPLE CMDS TO FDC				*5
;									*5
;	Input:	DS = Rom data area segment
;		BX = STARTING TABLE OFFSET
;		CX = NUMBER OF BYTES TO WRITE
;	Output: AL = ?
;		AH = ?
;		BX = ?
;		ES:SI = Disk Parameter Block
;		ERRSTAT bit 7 = 1 on timeout
;		ERRSTAT bit 5 = 1 on controller error
;		Carry Flag set on timeout or controller error, else not set
;									*5
MLTCMD2 PROC	NEAR						       ;*5
	MOV	CX,2			; Output 2 commands
MULTCMD:
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	PUSH	CX						       ;*5
MULT5:								       ;*5
	MOV	AL,ES:[SI+BX]		; GET CMD DATA			*5
	CALL	WRTCMD			; WRITE IT			*5
	JC	MULTRTN 		; JUMP IF ERROR 		*5
	INC	BX			; POINT TO NEXT BYTE		*5
	LOOP	MULT5			; REPEAT UNTIL DONE		*5
	CLC				; BE SURE TO RETURN NO ERROR	*5
MULTRTN:							       ;*5
	POP	CX						       ;*5
	RET							       ;*5
MLTCMD2 ENDP							       ;*5
	PRGSIZ	<Disk write multiple commands>	; Display program size
;******************************************************************************
;
; SUBROUTINE TO INITIALIZE DMA
;
;	Input:	DS = Rom data area segment
;		AH = COMMAND BYTE TO DMA
;		AL = NUMBER OF SECTORS TO READ, WRITE OR VERIFY (1-8)
;		ES:BX = TRANSFER ADDRESS
;	OUTPUT: ES:SI = Address of DPB
;		Carry Flag set if error, else not
;
INITDMW PROC	NEAR
	MOV	AH,4AH			; SET UP WRITE DMA COMMAND
INITDMA:
	PUSH	CX
	PUSH	DX
	PUSH	AX
	PUSH	BX
;
INITD0:
	XCHG	AL,AH			; Move mode command to register
					;   and # of sectors to AH
	MOV	DX,DMAMD		; 
	OUT	DX,AL			; WRITE TO MODE REGISTER
;
; CONVERT ES:BX PAIR TO 20 BIT ADDRESS
;
	MOV	DX,ES			; CALCULATE LOWER 16 BITS OF ADDRESS
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
;
	MOV	CL,4			;  *
	ROL	DX,CL			;  *
	MOV	CH,DL			; Put copy in CH
	AND	CH,00Fh 		; Get 64 k section in CH
	AND	DL,0F0h 		; Get paragraph section of 64 k section
					;   in DX
	ADD	DX,BX			; DX = Bits 0-15 of transfer address
	ADC	CH,0			; Add in carry (if any)
;
	MOV	AL,AH			; Get back # of sectors
	XOR	AH,AH			; CLEAR TOP BYTE
	MOV	CL,ES:[SI+3]		; Get the value used to compute
					;   Bytes/sector = 2^([SI+3]) * 128
					;	maximum value = 8
	SHL	AX,CL			; AX = (bytes to read)/128
	CMP	AX,512
	JA	DMAERR
	XCHG	AH,AL			; Multiply by 128 by using trick
	ROR	AX,1			;  *
	AND	AL,0FEh 		; Get rid of lowest bit in case
					;   its exactly 512
;
	DEC	AX			; ADJUST COUNT FOR DMA CHIP'S SAKE
;
	MOV	BX,DX			; Put copy of starting address in BX
	ADD	DX,AX			; Q: RESULT IN DMA ACROSS 64K BOUNDARY
	JC	DMAERR			;  * YES, IT'S AN ERROR
;
	CLI				; MASK INTERRUPTS
;
	MOV	DX,DMACLFF		; Clear 1st/last Flip Flop
	OUT	DX,AL			; 
;
	MOV	DX,DMA2CNT		; DX=I/O ADDR OF DMA CURRENT WORD REG
	OUT	DX,AL			; OUTPUT TRANSFER LENGTH, LOW BYTE 1ST
;
	XCHG	AL,AH			; Delay for chip I/O recovery time
	OUT	DX,AL			; HIGH BYTE LAST
;
	MOV	DX,DMA2ADR		; DX=I/O ADDR OF DMA CURRENT ADDR REG
	XCHG	AX,BX			; Get address in BL to DMA to/from
					;   Delay for chip I/O recovery time
	OUT	DX,AL			; OUTPUT TRANSFER ADDRESS, LOW BYTE 1ST
	XCHG	AL,AH			; Delay for chip I/O recovery time
	OUT	DX,AL			; HIGH BYTE LAST
;
	XCHG	AL,CH			; Get 64 k section address
					; Delay for chip I/O recovery time
;
	MOV	DX,DMAPG2		; DX=I/O ADDR OF DMA PAGE REGISTER
	OUT	DX,AL			; INITIALIZE DMA PAGE REGISTER
;
	STI				; ENABLE INTERRUPTS
	MOV	DX,DMARQST		; DX=I/O ADDR OF DMA REQUEST REGISTER
	MOV	AL,DMARST+DMACH2	; Channel 2 Select
	OUT	DX,AL			; REQUEST DMA CHANNEL 2
	CLC				; CLEAR CARRY TO SHOW NO ERROR	*A
DMAEXIT:
	POP	BX
	POP	AX
	POP	DX
	POP	CX
	RET
DMAERR:
	MOV	ERRSTAT,ERR09		; SET DMA BOUNDARY ERROR
DMAER0:
	STC				; SET CARRY TO INDICATE ERROR	*A
	JMP	SHORT DMAEXIT
INITDMW ENDP
	PRGSIZ	<Initialize Disk DMA>
;
; Recalibrate Drive in .DL
;

RECALIBRATE    PROC NEAR
	CALL	RECAL1			; 1st attempt
	JNC	DOSKS0			; OK if NC

RECAL1: MOV	AL,RECALCMD		; 2nd attempt
	CALL	WRTCMD
	JC	DOSKS0
	MOV	AL,DL
RECALIBRATE    ENDP			; Fall into DOSKSB

;
;******************************************************************************
;
; SUBROUTINE TO WRITE LAST PARAM OF SEEK/RECAL AND COMPLETE SEEK/RECAL
;
;	Input:	DS = Rom data area segment
;		AL = last command byte of seek/recal
;		DL = drive #
;	Output: AH = FDC status if carry not set
;		AH = ? if carry set
;		AL = ? 
;		ERRSTAT bit 7 = 1 on timeout
;		ERRSTAT bit 5 = 1 on controller error
;		Carry Flag set on timeout or controller error, else not set
;		FDTIMO set to large value
;
DOSKSB	PROC	NEAR
	CALL	WRTCMD			; WRITE 
	JC	DOSKS0			; JUMP IF ERROR (CAN'T WRITE CMD)
DOSKSTRY:
	CALL	WAITINT 		; WAIT FOR operation COMPLETE
	JC	DOSKS0			; JUMP IF ERROR (TIMEOUT)
;
	CALL	SISSTAT 		; CHECK IF ERROR
	JC	DOSKS0			; JUMP IF ERROR (CAN'T READ OR WRITE)
;
	MOV	AL,AH			;				stan
	AND	AL,11b			; isolate drive select bits	stan
	CMP	AL,DL			; is it the drive in question?	stan
	JNE	DOSKSTRY		; no, wait for the right int	stan
	TEST	AH,ATRDY		; NORMAL TERMINATION ?
	JZ	DOSKS0			;  Yes, then operation is OK,
					;   return w/ C clear
	OR	ERRSTAT,FDSKERR 	;  No, force seek error,
	STC				;   return w/ C set
DOSKS0:
	RET
DOSKSB	ENDP
	PRGSIZ	<Write command to FDC and wait for complete>
;******************************************************************************
;
; DISKETTE INTERRUPT SERVICE ROUTINE
;
	SYNC	DSKISRA
;
IF	ENHFDD
        
	EXTRN   FD_ISR:NEAR
DSKISRC:
        JMP     FD_ISR
	PRGSIZA <Floppy Disk ISR jump>          ; Display program size

ELSE	;ENHFDD

	PUBLIC	DSKISR
DSKISR	PROC	FAR
	PUSH	AX
	PUSH	DS
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	OR	DRVSTAT,BIT7		; SET INTERRUPT HAPPENED BIT
	MOV	AL,EOINSP		; AL=END OF INTERRUPT BIT
	OUT	PIC0,AL 		; ACKNOWLEDGE 8259 INTERRUPT
;
	POP	DS
;
;
	POP	AX
	IRET
DSKISR	ENDP
	PRGSIZ	<Disk ISR>
ENDIF	;ENHFDD

;******************************************************************************
;	
; SUBROUTINE TO WAIT FOR FDC INTERRUPT
;
;	Input:	DS = Rom data area segment
;	OUTPUT: Carry Flag set if timeout error, else not
;		ERRSTAT bit 7 = 1 if timeout
;		FDTIMO set to large value
;
WAITINT PROC	NEAR
	MOV	FDTIMO,0FFh		; Make a large motor timeout value
;
;
	IF	WAITHLT
; loop is 4*(39*(2**16)+21)/4.8= 2.13 seconds
WAIT0:
	TEST	DRVSTAT,BIT7		; Did disk interrupt occur ?
	JNZ	WAIT2			;   Yes
	HLT				; Wait for disk or timer interrupt
	CMP	FDTIMO,(0FFh-(213*182/1000))
	JNC	WAIT0			; Loop until timeout
	ELSE	;WAITHLT
	PUSH	CX
	MOV	CX,CLK10/6		;CLK10 would be about 12 sec
WAITLO: 				; outer loop
	PUSH	CX
	XOR	CX,CX			; set inner loop count to 64K
WAITLI: 				; inner loop
	TEST	DRVSTAT,BIT7		; Did disk interrupt occur ?
	LOOPZ	WAITLI
	POP	CX
	LOOPZ	WAITLO
	POP	CX
	JNZ	WAIT2			;   Yes
	ENDIF	;WAITHLT ELSE
;
WAIT1:	OR	ERRSTAT,FDSKTMO 	; SET TIMEOUT ERROR
	STC				; OPERATION TIMED OUT
	RET
;
WAIT2:
	AND	DRVSTAT,NOT BIT7	; RESET INTERRUPT OCCURRED BIT
					; Carry is cleared indicating interrupt
					;   occured
	RET
WAITINT ENDP
	PRGSIZ	<Wait for FDC interrupt>

;******************************************************************************
;	
; SUBROUTINE TO READ RESULT BYTE(S) FROM THE FDC
;
;	Input:	DS = Rom data area segment
;		CX = NUMBER OF RESULT BYTES TO READ
;	Output: AL = FDC status
;		AH = FDC ST0, only valid if no carry
;		CX = ?
;		ERRSTAT bit 7 = 1, C set on timeout
;		ERRSTAT bit 5 = 1, C set if can't flush FDC results
;		C set if less results available than expected
;		DSKST = FDC ST0, valid only if no carry
;
RSLTRD7 PROC	NEAR
	ASSUME	DS:ROMDAT
	MOV	CX,7
RSLTRD:
	PUSH	DX
	PUSH	BX
	MOV	BX,OFFSET DSKST
RSLTR0:
	CALL	FDCST
	JC	RSLTR3
	AND	AL,CB+DIO		; these bits
	CMP	AL,CB+DIO		; should both be on.
	JNE	RSLTR2			;  if not, we're done reading results
;
	INC	DX			; DX = FDC Data register
	IN	AL,DX			; READ RESULT
	MOV	[BX],AL 		; STORE RESULT
	INC	BX			;
;
; 80 "8088 clocks" between last IN & FDCST's IN - if this is less than the
; 12 usec before the main status reg is guaranteed valid,
	IF	80 LT CLK10*12/10
	MUL	BX			; kill even more time - DX not needed
	ENDIF
	LOOP	RSLTR0			; JUMP UNTIL ALL BYTES HAVE BEEN READ
;
; Verify command complete needn't be here - next FDC op will report any error
; Waiting here will impact performance.
	if	false
	MUL	BX			; kill even more time - DX not needed
	CALL	FDCST			; We're done -
	JC	RSLTR2A
	TEST	AL,CB			;    does FDC agree?
	JZ	RSLTR3			;   Yes, go exit happy
	else	;false
	CLC
	JMP	SHORT RSLTR3
	endif	;false else
RSLTR2A:
	OR	ERRSTAT,FDCTERR 	; CONTROLLER FAILURE
RSLTR2:
	STC				; SET THE CARRY FLAG
RSLTR3:
	MOV	AH,DSKST		; Return FDC ST0 in AH,
					;   only valid if no carry
	POP	BX
	POP	DX
	RET
;
RSLTRD7 ENDP
	PRGSIZ	<Read result byte from disk>
;
;******************************************************************************
;
	SYNC	DSKPRMA
	PUBLIC	DSKPRM
DSKPRM:

IF	ENHFDD
	DB      13*16+15        ; Upper nibble - Track Step time as follows;
                                ;   (16-((step rate in ms)/2)) mod 16
                                ; Lower nibble - Head unload time in 32 ms
                                ;   increments
        DB      1*2+0           ; Upper 7 bits - Head Load Time in 4 ms
                                ;   increments
                                ; Lower bit - non-DMA Mode if 0
        DB      37              ; Motor Off Wait time in clock tics 54.9 ms
        DB      2               ; Log2 (Bytes/sector/128)
        DB      18              ; Last sector # on track for quad density
        DB      1Bh             ; Gap length between sectors
        DB      0FFH            ; Data length, when sector length not specified 
        DB      54h             ; Gap length for formatting                     
        DB      0F6h            ; Format fill byte                              
        DB      15              ; Head settle time in milliseconds              
        DB      8               ; Motor start time in .125 sec increments       
        	
ELSE	
	DB	0DFH		; Upper nibble - Track Step time as follows;
				;   (16-((step rate in ms)/2)) mod 16
				; Lower nibble - Head unload time in 32 ms
				;   increments
	DB	50		; Upper 7 bits - Head Load Time in 4 ms
				;   increments
				; Lower bit - non-DMA Mode if 0
	DB	40		; Motor Off Wait time in clock tics 54.9 ms
	DB	2		; Log2 (Bytes/sector/128)
	DB	8		; Last sector # on track
	DB	2AH		; Gap length between sectors
	DB	0FFH		; Data length, when sector length not specified
	DB	50H		; Gap length for formatting
	DB	0F6H		; Format fill byte
	DB	15		; Head settle time in milliseconds
	DB	5		; Motor start time in .128
				;   sec increments
ENDIF
	PRGSIZ	<Disk parameters>	; Display program size
;
	SYNC	PRTDSRA
	TOTAL
;
CODE	ENDS
	END
