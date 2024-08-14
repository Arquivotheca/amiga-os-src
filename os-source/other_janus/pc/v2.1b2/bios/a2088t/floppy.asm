        NAME	FLOPPY
	TITLE	Int. 13 /  Floppy Disk Driver
	PAGE	59,132
;******************************************************************************
;*    Copyright (c) 1987-1989 Phoenix Technologies Ltd.  This program contains	   *
;*    proprietary and confidential information.  All rights reserved except   *
;*    as may be permitted by prior written consent.			      *
;*									      *
;******************************************************************************


;***************************************************************************
;	Revision Information	$Revision:   1.5  $	
;				$Date:   26 Oct 1989 11:26:38  $
;***************************************************************************
;*									      *
;*	FLOPPY.ASM	- Int. 13   Floppy diskette driver		      *
;*									      *
;*	Date:		10/21/1987					      *
;*									      *
;*									      *
;*	Description : This module contains the Bios interface to the floppy   *
;*		    disk sub system. It is accessed via software int. 13H.    *
;*		    This module also contains the floppy disk hardware int.   *
;*		    service routine. This module contains support for the     *
;*		    following functions:				      *
;*									      *
;*	Name	    AH/function 	description			      *
;* ------------------------------------------------------------------	      *
;*	FD_RESET	00H	-  Reset diskette system		      *
;*	FD_STAT 	01H	-  Read status for previous operation	      *
;*	FD_READ 	02H	-  Read sector(s) into memory		      *
;*	FD_WRITE	03H	-  Write sector(s) from memory		      *
;*	FD_VERFY	04H	-  Verify sector(s)			      *
;*	FD_FORMT	05H	-  Format track 			      *
;*	null		06H	-  reserved				      *
 ;*	null		07H	-  reserved				      *
;*	FD_RDPAR	08H	-  Read drive parameters		      *
;*	null	     09H - 14H	-  reserved				      *
;*	FD_RDASD	15H	- Read DASD type			      *
;*	FD_CHNG 	16H	- Read diskette change status		      *
;*	FD_SDASD	17H	- Set DASD type 			      *
;*	FD_SFRMT	18H	- Set media type for format		      *
;*		    19H - FFH	- reserved				      *
;*									      *
;*									      *
;*	       --------- UTILITY ROUTINES ----------			      *
;*									      *
;*	CHEKRES 	--	Check results from FDC operation	      *
;*	CHK_CHG 	--	Check change line on drive		      *
;*	CHKTYP		--	Check media type and update FDMED	      *
;*	CLR_CARY	--	Clear carry bit on stack		      *
;*	CLR_CHG 	--	Clear change line			      *
;*	DORECAL 	--	Recalibrate drive (seek track 0)	      *
;*	DOSEEK		--	Seek to track				      *
;*	DPBADR		--	Get drive parameter table address	      *
;*	DSKMOT		--	Turn disk motor on			      *
;*	DSKRESET	--	Reset diskette subsystem		      *
;*	FDC_RDY 	--	Wait for floppy controller ready	      *
NNNEA89062302	=	0
       IFDEF	NNNEA89062302
;*	FDTYPIF 	--	Get drive type from CMOS, check CMOS	      *
       ENDIF    ;NNNEA89062302
;*	FDTYPE		--	Get drive type from CMOS, no CMOS check       *
;*	GETDINF 	--	Get drive info from 40:8F		      *
;*	INIT_DMA	--	Initialize DMA controller		      *
;*	MLTCMD		--	Write multiple commands to FDC		      *
;*	RESTYP		--	Reset media type to initial value (not really)*
;*	RSLTRD		--	Read and interpet FDC results		      *
;*	SDRVST		--	Sense drive status			      *
;*	SET_CARY	--	Set carry flag on stack 		      *
;*	SETRAT		--	Set transfer rate			      *
;*	SISSTAT 	--	Sense FDC interrupt status		      *
;*	WAITINT 	--	Wait for FDC interrupt			      *
;*	WRTCMD		--	Write a command to floppy controller	      *
;******************************************************************************
;		Local Macros
CLEAR_CARRY	MACRO
;
;	THIS MACROS WILL CLEAR THE CARRY BIT AND SET THE INTERRUPT BIT
;
	and	byte ptr STK_FL[bp], 11111110b	  ; clear C
	or	byte ptr STK_FL+1[bp], 00000010b	; set I
	endm
SET_CARRY	MACRO
;
;	THIS MACROS WILL SET THE CARRY BIT
;
	OR	WORD PTR STK_FL[BP],01		; SET CARRY BIT
	ENDM
	
;******************************************************************************
;*		INCLUDE FILES
;******************************************************************************
;
;.XLIST

INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE MACROS.INC		; ROM Macros file
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE CMSEQU.INC		; CMOS Equate file
INCLUDE	TEXTPC.INC		; displayed text messages			
.LIST


;******************************************************************************
;*		LOCAL EQUATES
;******************************************************************************


;SAFTNET	equ TRUE	; retry different rates within R/W/V (needed
;				;  because ReadIDs might work at wrong rate)
SUP720		equ FALSE	; explicit support for 720K diskette in 1.2M
				;  drive
SLEAZY		equ FALSE	; allow DSKTEST 80-track-drive-determination to
				;  work (test only - may mask other problems)
	; Setting only 1 of the next 2 true should prevent MSDOS 3.2 FORMAT.COM
	;  screwup when disk change line active:

NOFORFORMAT	equ TRUE	; don't change medium type if unknown on format
NOCLRFORMAT	equ FALSE	; don't clear known in CLR_CHG if it's format
	; Setting 1 of the next 2 true should allow boot of PCDOS 3.3 even 
	;  if CMOS lies about drive type (in most cases):
BESURE08	equ TRUE	; maintain flag indicating probable 1.44M drive
				;  to be checked by func 08
; Show Floppy messages?
INFO		equ	FALSE

JAMEOT		equ FALSE	; force EOT for R/W/V to some maxiumum:
BIGEOT		equ 24		;  EOT to use - why not?

FDC_REGR	EQU	X765+00
FDC_DAT 	EQU	FDC_REGR+1
CLRBP		EQU	DMACLFF
MODE03		EQU	DMAMD
CH2CNT		EQU	DMA2CNT
CH2ADR		EQU	DMA2ADR
CH2PG		EQU	DMAPG2
MASK03		EQU	DMARQST


;
FD_MAXF 	EQU	18H	; Maxinum function request
MAX_DRV 	EQU	2	;
;
;	    -- 765 FDC Command bytes equates --
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
WRITECMD	EQU	MT+MF+05h 	; WRITE DATA COMMAND
FRMTCMD 	EQU	MF+0Dh		; FORMAT TRACK COMMAND
READCMD 	EQU	MT+MF+SK+06h	; READ DATA COMMAND

IFDEF	NNNINT077
;NNNINT077 switch may be removed after sufficient test
;is performed to insure that this code does not produce problems.
;
VERFCMD		EQU	MT+MF+SK+16H	; VERIFY COMMAND
ENDIF	;NNNINT077

READID		EQU	MF+0AH		; READ ID COMMAND
FDC_DONE	EQU	80H 
;	     -- Status Register 0 bit equates --
;
IC		EQU	80h		; Invalid Command, command was never
					;   started
ATERM		EQU	40h		; Abnormal termination, command was not
					;   completed sucessfully
ATRDY		EQU	IC+ATERM	; Abnormal termination because FDD ready
					;   state changed during execution
NT		EQU	00h		; Normal termination
EC		EQU	10h		; Equipment check, set if fault in FDD
					;   or recalibrate didn't find track 0
NR		EQU	08h		; FDD Not Ready

;
SPFYCMD 	EQU	03h		; SPECIFY COMMAND
RECALCMD	EQU	07h		; RECALIBRATE COMMAND
SISCMD		EQU	08h		; SENSE INTERRUPT STATUS COMMAND
SEEKCMD 	EQU	0Fh		; SEEK COMMAND
SNSDRV		EQU	04h		; Sense drive status
;
;		  -- DISKETTE ERROR CODES --
;
NO_ERROR	EQU	0		; NO ERROR
FDSKTMO 	EQU	80H		; DEVICE TIMEOUT/DID NOT RESPOND
FDSKERR 	EQU	40H		; SEEK FAILURE
FDCTERR 	EQU	20H		; CONTROLLER FAILURE
ERR01		EQU	01H		; ILLEGAL FUNCTION
ERR02		EQU	02H		; ADDRESS MARK NOT FOUND
ERR03		EQU	03H		; WRITE PROTECT ERROR
ERR04		EQU	04H		; SECTOR NOT FOUND
ERR06		EQU	06h		; Media change (AT 1.2M controller only)
ERR08		EQU	08H		; DMA OVERRUN
ERR09		EQU	09H		; DMA ACCESS ACROSS 64K BOUNDARY
ERR10		EQU	10H		; BAD CRC ON DISK READ

;
; Stack access equates -- These values are the offsets relative to BP of the
;			  register values passed to this function
;
STK_BP		EQU	0	; Offset of original BP from BP
IFDEF	NNNINT077
;
STK_T1		EQU	2	; Offset of temporay storage area
				; Bit 0	: 82077 present if set
				; Bit 1 : 82077 FIFO enabled if set   
STK_SI		EQU	4	; Offset of original SI from BP
STK_DS		EQU	6	; Offset of original DS from BP
STK_DI		EQU	8	; Offset of original DI from BP
STK_ES		EQU	10	; Offset of original ES from BP
STK_DL		EQU	12	; Offset of original DL from BP
STK_DH		EQU	13	; Offset of original DH from BP
STK_CL		EQU	14	; Offset of original CL from BP
STK_CH		EQU	15	; Offset of original CH from BP
STK_BL		EQU	16	; Offset of original BL from BP
STK_BH		EQU	17	; Offset of original BH from BP
STK_AL		EQU	18	; Offset of original AL from BP
STK_AH		EQU	19	; Offset of original AH from BP
STK_IP		EQU	20	; Offset of original IP from BP
STK_CS		EQU	22	; Offset of original CS from BP
STK_FL		EQU	24	; Offset of original FLAGS from BP
ELSE		;NNNINT077
STK_SI		EQU	2	; Offset of original SI from BP
STK_DS		EQU	4	; Offset of original DS from BP
STK_DI		EQU	6	; Offset of original DI from BP
STK_ES		EQU	8	; Offset of original ES from BP
STK_DL		EQU	10	; Offset of original DL from BP
STK_DH		EQU	11	; Offset of original DH from BP
STK_CL		EQU	12	; Offset of original CL from BP
STK_CH		EQU	13	; Offset of original CH from BP
STK_BL		EQU	14	; Offset of original BL from BP
STK_BH		EQU	15	; Offset of original BH from BP
STK_AL		EQU	16	; Offset of original AL from BP
STK_AH		EQU	17	; Offset of original AH from BP
STK_IP		EQU	18	; Offset of original IP from BP
STK_CS		EQU	20	; Offset of original CS from BP
STK_FL		EQU	22	; Offset of original FLAGS from BP
ENDIF		;NNNINT077
;
MWAIT_R EQU	5		; absolute minimum wait for motor speedup 
				;  before read, in 8ths of seconds
MWAIT_W EQU	8		; absolute minimum wait for motor speedup 
				;  before write or format, in 8ths of seconds
MINWAIT EQU	MWAIT_W*182/80	; Min # of ticks since motor on for speedup wait
HST360	EQU	15		; SW-forced head settle time in msecs if 360K
HSTELS	EQU	20		; SW-forced head settle time in msecs otherwise

  IFDEF NNN_FIXUP
	.286
  ELSE  ;NNN_FIXUP

IF	AT
	.286P
ENDIF	;AT
  ENDIF ;NNN_FIXUP
;
;******************************************************************************
;
DGROUP	GROUP	ROMDAT
INTVEC	SEGMENT AT 0000h	; The following are relative to Segment 00h
;
	EXTRN	DPBPTR:DWORD	; 0:78 Pointer to disk parameter table
;
INTVEC	ENDS
;
;------------------------------------------------------------------------------
;
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
;
	EXTRN	DEVFLG:BYTE	; 40:10 Number of devices installed
	EXTRN	DRVSTAT:BYTE	; 40:3E Drive status
	EXTRN	FDMOTS:BYTE	; 40:3F Motor status
	EXTRN	FDTIMO:BYTE	; 40:40 Motor timeout count
	EXTRN	ERRSTAT:BYTE	; 40:41 Disk error status
	EXTRN	DSKST:BYTE	; 40:42 Disk controller status(7 bytes) 
	EXTRN	FDRATE:BYTE	; 40:8B Last Floppy Disk data rate selected
	EXTRN	HDCFLG:BYTE	; 40:8F Controller info
	EXTRN	FDMED:BYTE	; 40:90 Drive 0/1 Media state
	EXTRN	FDOPER:BYTE	; 40:92 Drive 0/1 Operation state
	EXTRN	FDTRCK:BYTE	; 40:94 Drive 0/1 Current track
	EXTRN	WTACTF:BYTE	; 40:A0 Wait active flag
	EXTRN	DLY_CNT:BYTE	; 40:A1 Delay count for 100 us.
ROMDAT	ENDS

;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
IFDEF  NNN_ECR_25
CODE   SEGMENT WORD PUBLIC 'CODE'
;        CODESTARTS
ELSE   ;NNN_ECR_25
CODE	SEGMENT BYTE PUBLIC 'CODE'
ENDIF  ;NNN_ECR_25
       ASSUME CS:CODE

       SYNC	FLOPPY_NEW

;
;------------------------- EXTERNAL DECLARATIONS -----------------------------
;
IF	CMOSCFG			; (vtb 6.3.90)
	EXTRN	CMREAD:NEAR
ELSE
CMREAD	proc	near
	mov	al,44h		; (wds)
	ret 			; dummy
CMREAD  endp		
ENDIF
	EXTRN	DLY100:NEAR
	EXTRN	SETERF:NEAR
;	EXTRN	FCLK10:WORD	; (vtb 6.3.90)
IFDEF	NNNCONSTRUCT
	EXTRN	OPT_CFG_TAB:WORD
	EXTRN	SPD_TAB:WORD
ENDIF	;NNNCONSTRUCT
;
SEG0H	DW	0000H		; Value for segment reg set to INTVECT
SEG40H	DW	0040H		; Value for segment reg set to ROMDAT

	PUBLIC	FCLK10		; (vtb 6.3.90)
FCLK10	DW	CLK10/2
;
;	Entry point - via software int. 13h - FD_DRVR
;
PUBLIC	FD_DRVR,DSKDSR,FD_SFRMT,FD_FORMT,FD_RDASD,FD_VERFY,FD_RDPAR
;******************************************************************************
;		    ------ FD_DRVR TABLES ------
;******************************************************************************
;
MED_TYP LABEL	BYTE		; Table of media type trial values
;
	DB	07H		; 1.44/1.44
	DB	87H		; 720/720
	DB	02H		; 1.2/1.2
	DB	61H		; 360/1.2
	DB	80H		; 360/360
	DB	0FFH		; EOT
;
STRT_TAB	LABEL	BYTE
	DB	07H		; If 0, drive not known by CMOS start at top
	DB	80H		; If 1, drive is a 360K
	DB	02H		; If 2, drive is a 1.2
	DB	87H		; If 3, drive is a 720K
	DB	07H		; If 4, drive is a 1.44
	DB	07H		; If 5 - 7 CMOS value invalid, start at top
	DB	07H
	DB	07H
;
;
O_MAXCYL	EQU 11
O_MAXSEC	EQU 4
O_DATRAT	EQU 12
;
;      ----------- DRIVE PARAMETER TABLES ------------
;
;***********************************

IFDEF	NNNCONSTRUCT
	PUBLIC	PAMSMED
       if 	SUP720
	DW	7*PAMSIZ	; # of bytes in floppy parameter table 
       else	;SUP720
	DW	6*PAMSIZ	; # of bytes in floppy parameter table 
       endif	;SUP720			
ENDIF	;NNNCONSTRUCT

PAMSMED LABEL BYTE
; medium-type 1 default params 360/360 :
	DB	0DFh,02h		;specify params
	DB	25H			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	9H			;max sector
	DB	2AH			;read/write gap length
	DB	0FFH			;sector size if <128
	DB	50H			;format gap length
	DB	0F6h			;format fill character
	DB	0FH			;head settle time
	DB	8H			;motor on delay in 1/8s of seconds
	DB	40-1			;max track
	DB	10000000b		;data rate
	DB	1			; CMOS type
PAMSIZ	EQU	$-PAMSMED
; medium-type 2 default params 360/1.2 :
	DB	0DFh,02h		;specify params
	DB	25H			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	09H			;max sector
	DB	2AH			;read/write gap length
	DB	0FFH			;sector size if <128
	DB	50H			;format gap length
	DB	0F6h			;format fill character
	DB	0FH			;head settle time
	DB	8H			;motor on delay in 1/8s of seconds
	DB	40-1			;max track
	DB	01000000b		;data rate 
	DB	2			; CMOS type
; medium-type 3 default params 1.2/1.2 :
	DB	0DFh,02h		;specify params
	DB	25H			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	0FH			;max sector
	DB	1BH			;read/write gap length
	DB	0FFH			;sector size if <128
	DB	54H			;format gap length
	DB	0F6h			;format fill character
	DB	0FH			;head settle time
	DB	8			;motor on delay in 1/8s of seconds
	DB	80-1			;max track
	DB	00000000b		;data rate 
	DB	2			; CMOS type
; medium-type 4 default params 720/720 :
	DB	0aFh,02h		;specify params
	DB	25H			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	09H			;max sector
	DB	2AH			;read/write gap length
	DB	0FFH			;sector size if <128
	DB	50H			;format gap length
	DB	0F6h			;format fill character
	DB	0FH			;head settle time
	DB	8			;motor on delay in 1/8s of seconds
	DB	80-1			;max track
	DB	10000000b		;data rate 
	DB	3			; CMOS type
; medium-type 5 default params 720/1.44 :
	DB	0aFh,02h		;specify params
	DB	25H			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	9			;max sector
	DB	2AH			;read/write gap length
	DB	0FFH			;sector size if <128
	DB	50H			;format gap length
	DB	0F6h			;format fill character
	DB	0FH			;head settle time
	DB	08			;motor on delay in 1/8s of seconds
	DB	80-1			;max track
	DB	10000000b		;data rate 
	DB	4			; CMOS type
; medium-type 6 default params 1.44/1.44:
	DB	0AFh,02h		;specify params
	DB	25H			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	12H			;max sector
	DB	1BH			;read/write gap length
	DB	0FFH			;sector size if <128
	DB	6CH			;format gap length
	DB	0F6h			;format fill character
	DB	0FH			;head settle time
	DB	8			;motor on delay in 1/8s of seconds
	DB	80-1			;max track
	DB	00000000b		;data rate 
	DB	4			; CMOS type
       if	SUP720                  ;Support 720 disk in 1.2 Meg drive
PAM720	LABEL BYTE
; 720K in 1.2M drive default params:
	DB	0DFh,02h		;specify params
	DB	37			;motor timeout count
	DB	02h			;log2(sector size/128)
	DB	9			;max sector
	DB	42			;read/write gap length
	DB	-1			;sector size if <128
	DB	80			;format gap length
	DB	0F6h			;format fill character
	DB	15			;head settle time
	DB	8			;motor on delay in 1/8s of seconds
	DB	80-1			;max track
	DB	01000000b		;data rate
	DB	2
       endif	;SUP720

;
;---------------------------- ENTRY POINT -------------------------------------
;
DSKDSR:
FD_DRVR:

;
;------------------------------ SAVE ALL REGS ---------------------------------
;
	PUSH	AX			; Save all registers used
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	ES
	PUSH	DI
	PUSH	DS
	PUSH	SI
IFDEF	NNNINT077
	PUSH	AX			; temporary storage area
ENDIF	;NNNINT077
	PUSH	BP		
	MOV	DS,SEG40H		; Set DS to ROMDAT
      ASSUME DS:ROMDAT
	MOV	BP,SP			; Set BP to current stack location
IFDEF	NNNCONSTRUCT
	TEST	OPT_CFG_TAB.CFG_OPT2, MASK TRBO		; if TURBO
	JZ	NO_TURB1				;   jump if not
	TEST	SPD_TAB.SPD_OPT1, MASK DSPD 		; if DISKSPD
	JZ	NO_TURB1				;   jump if not

	EXTRN	GET_SPEED:NEAR
	EXTRN	SET_SPEED:NEAR

	PUSH	AX			; save ax
	PUSH	DS			; save current data segment
	CALL	GET_SPEED		; get current speed in al - ah preserved
	CMP	AL,SPD_TAB._DISKSPDV	; If already at the desired speed
	JE	DONT_SET		;   don't need to call SET_SPEED
	PUSH	AX			; save curr. speed
	MOV	AL,SPD_TAB._DISKSPDV
	CALL	SET_SPEED
	POP	AX			; restore curr. speed
DONT_SET:
	XCHG	AX,[BP-2]		; restore AX & save curr. speed on stack
	POP	DS			; restore data segment
					; curr. speed now at T.O.S.
NO_TURB1:
 
ELSE	;NNNCONSTRUCT
       IF	TURBO
       IF	DISKSPD
	EXTRN	GET_SPEED:NEAR
	EXTRN	SET_SPEED:NEAR

	PUSH	AX			; save ax
	PUSH	DS			; save current data segment
	CALL	GET_SPEED		; get current speed in al - ah preserved
	CMP	AL,DISKSPDV		; If already at the desired speed
	JE	DONT_SET		;   don't need to call SET_SPEED
	PUSH	AX			; save curr. speed
	MOV	AL,DISKSPDV
	CALL	SET_SPEED
	POP	AX			; restore curr. speed
DONT_SET:
	XCHG	AX,[BP-2]		; restore AX & save curr. speed on stack
	POP	DS			; restore data segment
					; curr. speed now at T.O.S.

       ENDIF	;DISKSPD
       ENDIF	;TURBO
ENDIF	;NNNCONSTRUCT
;
;------------------------ CHECK FUNCTION RANGE ---------------------------------
;
  IFDEF  NNN_FIXUP
	STI
  ENDIF ;NNN_FIXUP
	CMP	AH,FD_MAXF		; Check function range
	JBE	GO_FUNCT
	JMP	NO_FUNCT		; If out of range return error info
GO_FUNCT:
;
;----------------------------- GO TO FUNCTION ------------------------------------
;
	SHL	AH,1			; Set to offset within table	
	MOV	DI,OFFSET F_TABLE 	; Get table offset
	XCHG	AL,AH
	XOR	AH,AH
	ADD	DI,AX			; Add in offset 
	XCHG	AH,AL
	SHR	AH,1			; Restore AH
	JMP	WORD PTR CS:[DI]
;
;------------------------------------------------------------------------------
;	FD_DRVR  exit point
;------------------------------------------------------------------------------
;
FD_EXIT:CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	MOV	AH,ES:[SI+2]		; GET DISK TIMEOUT COUNT
	CMP	FDTIMO,0FFh-MINWAIT
	JNA	EXSPUK
	ADD	AH,FDTIMO
	SUB	AH,0FFh-MINWAIT
EXSPUK:	MOV	FDTIMO,AH
FD_EXITF:
	AND	DRVSTAT,07FH		; MAKE SURE INTERRUPT BIT IS RESET

IFDEF	NNNCONSTRUCT
	TEST	OPT_CFG_TAB.CFG_OPT2, MASK TRBO		; if TURBO
	JZ	ALRDY_SET				;   jump if not
	TEST	SPD_TAB.SPD_OPT1, MASK DSPD		; if DISKSPD
	JZ	ALRDY_SET				;   jump if not
	POP	BX			; GET PREVIOUS SPEED FROM STACK
	CMP	BL,SPD_TAB._DISKSPDV	; If same as previous speed
	JE	ALRDY_SET		;   don't need to call SET_SPEED
	XCHG	AX,BX
	PUSH	DS			; save current data segment
	CALL	SET_SPEED		; restore old speed
	POP	DS			; restore data segment
	XCHG	BX,AX			; restore ax for exit
ALRDY_SET:
ELSE	;NNNCONSTRUCT
       IF	TURBO
       IF	DISKSPD
	POP	BX			; GET PREVIOUS SPEED FROM STACK
	CMP	BL,DISKSPDV		; If same as previous speed
	JE	ALRDY_SET		;   don't need to call SET_SPEED
	XCHG	AX,BX
	PUSH	DS			; save current data segment
	CALL	SET_SPEED		; restore old speed
	POP	DS			; restore data segment
	XCHG	BX,AX			; restore ax for exit
ALRDY_SET:
       ENDIF	;DISKSPD
       ENDIF	;TURBO
ENDIF	;NNNCONSTRUCT
;
;------------------------- RESTORE ALL REGISTERS ------------------------------
	POP	BP
IFDEF	NNNINT077
	POP	AX		;temp variables
ENDIF	;NNNINT077
	POP	SI
	POP	DS
	POP	DI
	POP	ES
	POP	DX
	POP	CX
	POP	BX
	POP	AX
	IRET					       
;
;------------------------------------------------------------------------------
;	F_TABLE  -- Function call table
;------------------------------------------------------------------------------
;
F_TABLE DW	OFFSET	FD_RESET	; AH = 00h - Reset disk system
	DW	OFFSET	FD_STAT 	; AH = 01H - Read disk status
	DW	OFFSET	FD_READ 	; AH = 02H - Read sector(s)
	DW	OFFSET	FD_WRITE	; AH = 03H - Write sector(s)
	DW	OFFSET	FD_VERFY	; AH = 04H - Verify sector(s)
	DW	OFFSET	FD_FORMT	; AH = 05H - Format track
	DW	OFFSET	NO_FUNCT	; AH = 06H - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 07H - Non supported function
	DW	OFFSET	FD_RDPAR	; AH = 08H - Read drive parameters
	DW	OFFSET	NO_FUNCT	; AH = 09H - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 0AH - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 0BH - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 0CH - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 0DH - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 0EH - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 0FH - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 10H - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 11H - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 12H - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 13H - Non supported function
	DW	OFFSET	NO_FUNCT	; AH = 14H - Non supported function
	DW	OFFSET	FD_RDASD	; AH = 15H - Read DASD type
	DW	OFFSET	FD_CHNG 	; AH = 16H - Read disk change status
	DW	OFFSET	FD_SDASD	; AH = 17H - Set DASD type
	DW	OFFSET	FD_SFRMT	; AH = 18H - Set type for format
;
;------------------------------------------------------------------------------
;	NO_FUNCT - illegal function requests handler
;
;	INPUT: nothing
;
;	OUTPUT: carry flag set on stack
;------------------------------------------------------------------------------
;
NO_FUNCT:
	MOV	BYTE PTR STK_AH[BP],01h
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXITF 
;
;------------------------------------------------------------------------------
;	FD_RESET (00H) - Reset diskette system
;
;	INPUT:	AH = 00
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;
;	OUTPUT: Carry set - Status non 0
;		AH value on stack = status :
;		00H - no error
;		01H - invalid parameters
;		02H - address mark not found
;		03H - write protect error
;		04H - sector not found
;		06H - change line set
;		08H - DMA overrun
;		09H - DMA boundary error
;		0CH - Media type error
;		10H - CRC error
;		20H - controller error
;		40H - seek error
;		80H - drive not ready
;
;	Status byte 40:41 = status = AH
;------------------------------------------------------------------------------
;
FD_RESET:
;
	CALL	DSKRESET		; Reset disk system
	JNZ	RESET_1
	MOV	ERRSTAT,NO_ERROR	; Clear error status byte
  IFDEF  NNNCE89070602
  ELSE  ;NNNCE89070602
	MOV	BYTE PTR STK_AL[BP],0	; Zero number of sectors
  ENDIF ;NNNCE89070602
	MOV	BYTE PTR STK_AH[BP],0	; Zero status
  IFDEF NNNMACRO
	CLEAR_CARRY			; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY		; Clear carry flag
  ENDIF ;NNNMACRO
	JMP	FD_EXITF		; Exit function 
RESET_1:MOV	BYTE PTR STK_AH[BP],AH	; Save AH on stack
  IFDEF NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE	;NNNMACRO
	CALL	SET_CARY		; Set error
  ENDIF ;NNNMACRO
	JMP	FD_EXITF		; Return
;
;------------------------------------------------------------------------------
;	FD_STAT (01H) - Read status of previous operation
;
;	INPUT:	AH = 01H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;
;	OUTPUT: Carry set - Status non 0
;		AH value on stack = status,  refer to FD_RESET for status 
;------------------------------------------------------------------------------
;
FD_STAT:
;
	mov	ah, ERRSTAT		; Get current error status
	MOV	STK_AH[bp], ah		; Put it on stack for return
  IFDEF NNNCO89030721
  ELSE  ;NNNCO89030721
	XOR	AL,AL			; Clear new status
	MOV	ERRSTAT,AL
  ENDIF ;NNNCO89030721
	and	byte ptr STK_FL[bp], not 1b	; clear returned C
	add	ah, 0FFh		; set C if old ERRSTAT > 0
	adc	byte ptr STK_FL[bp], 0	; set returned C if old ERRSTAT > 0
	JMP	FD_EXITF
;
;------------------------------------------------------------------------------
;	FD_READ (02H) - Read sector(s) from diskette
;
;	INPUT:	AH = 02H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;		DH = Head number (0 - 1 )
;		CH = Track number
;		CL = Sector number
;		AL = number of sectors
;	     ES:BX = address of buffer
;
;	OUTPUT: Carry set - Status non 0
;		AL = number of sectors transfered
;		AH = status : refer to FD_RESET for status values
;	Status byte 40:41 = status = AH
;------------------------------------------------------------------------------
;
FD_READ:
;	
;	FD_READ, FD_WRITE and FD_VERFY	all use common code under FD_verfy
;
;------------------------------------------------------------------------------
;	FD_WRITE (03H) - Write sector(s) to diskette
;
;	INPUT:	AH = 03H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;		DH = Head number (0 - 1 )
;		CH = Track number
;		CL = Sector number
;		AL = number of sectors
;	     ES:BX = address of buffer
;
;	OUTPUT: Carry set - Status non 0
;		AL = number of sectors transfered
;		AH = status : refer to FD_RESET for status values
;	Status byte 40:41 = status = AH
;------------------------------------------------------------------------------
;
FD_WRITE:
;
;	FD_READ, FD_WRITE and FD_VERFY	all use common code under FD_verfy
;
;------------------------------------------------------------------------------
;	FD_VERFY (04H) - Verify sector(s) from diskette
;
;	INPUT:	AH = 04H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;		DH = Head number (0 - 1 )
;		CH = Track number
;		CL = Sector number
;		AL = number of sectors
;	     ES:BX = address of buffer
;
;	OUTPUT: Carry set - Status non 0
;		AL = number of sectors verified
;		AH = status : refer to FD_RESET for status values
;	Status byte 40:41 = status = AH
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_VERFY,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_VERFY   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
;	FD_READ, FD_WRITE and FD_VERFY	all use common code under FD_verfy
;
      ASSUME DS:ROMDAT,ES:NOTHING
	CMP	DL,MAX_DRV	; Check for valid drive select
	JB	RWV_10
  IFDEF  NNN_FIXUP
	MOV	ERRSTAT,ERR01
	MOV	AL,STK_AL[BP]	; LOAD CURRENT AL
	JMP	RWV_110
  ELSE  ;NNN_FIXUP
	JMP	RWV_130
  ENDIF ;NNN_FIXUP
;
A0	DB	0      
;
;
RWV_10:	CALL	DSKMOT			; Select disk and start motor
IFDEF	NNNINT077
	CALL	IS_I077_INSTD		; sense 82077
ENDIF	;NNNINT077
	MOV	ERRSTAT,NO_ERROR	; init error code
RWV_15:	CALL	CHK_CHG 		; check change line
	JC	RWV_30			; If change line active, exit
	CALL	INIT_DMA		; initialize DMA
	JC	RWV_30			; If DMA boundary error, exit
;*			    if drive known 40-track, set medium known 360K
	mov	bl, STK_DL[bp]
	mov	bh, 0
	call	GETDINF
	cmp	al, 0100b
	jne	RWV_20
	mov	byte ptr FDMED[bx], 10010000b+3
RWV_20:	mov	al, FDMED[bx]	
	test	al, 00010000b
	jz	RWV_35
;*				seek
	MOV	CH,STK_CH[BP]		; get requested cylinder #
       if	SUP720
;*				(if known 360K-in-1.2 & current cyl = 0 & 
;*				 sought cyl > 0, pretend it's not known)
	and	al, 11110000b
	cmp	al, 01110000b
	jne	RWV_25
	cmp	FDTRCK[bx], 0
	jne	RWV_25
	or	ch, ch
	jnz	RWV_35
RWV_25:
       endif    ;SUP720
	CALL	DOSEEK			; seek to it
	JC	RWV_30			; if seek fails, exit
	call	HEDSETL			; WAIT FOR HEAD SETTLE
	call	SPINUP
	jmp	short RWV_40

RWV_30:	MOV	AL,0			; Zero number of sectors
	JMP	RWV_100			; and bail out
RWV_35:
;*				determine medium
	call	CHKTYP			; get drive type
	JC	RWV_30			; if can't even read IDs
	MOV	CH,STK_CH[BP]		; get requested cylinder #
	CALL	DOSEEK			; seek to it
	JC	RWV_30			; if seek fails, exit
	call	HEDSETL			; WAIT FOR HEAD SETTLE
;*			    restore xfr rate in case DOSEEK zapped it
RWV_40:	CALL	SETRAT
;*			    pump commands into FDC
	CMP	BYTE PTR STK_AH[BP],03h ; function = 3 (write)?
	MOV	AL,WRITECMD
	JE	RWV_55
IFDEF	NNNINT077
;
	TEST	WORD PTR STK_T1[BP],1
	JZ	RWV_50
	CMP	BYTE PTR STK_AH[BP],04h ; function = 4 (verify)?
	MOV	AL,VERFCMD
	JE	RWV_55
RWV_50:
ENDIF	;NNNINT077
	MOV	AL,READCMD		; if not use READCMD to read or verify
RWV_55:	CALL	WRTCMD			; write command to FDC
	JC	RWV_30
	CALL	DRIHEDSEL		; write drive/head select to FDC
	MOV	AL,STK_CH[BP]
	CALL	WRTCMDA 		; write track # (C)
	MOV	AL,STK_DH[BP]		; restore head #
	CALL	WRTCMDA 		; write head # (H)
	MOV	AL,STK_CL[BP]
	CALL	WRTCMDA 		; write start sector # (R)
	JC	RWV_30
	MOV	BX,3			; BX=beginning offset in DPB

       if	JAMEOT          	;force EOT for R/W/V to some maxiumum
	MOV	CX,1			; copy only 1 byte from DPB to FDC
	CALL	MLTCMD			; write bytes/sec (N)
	mov	al, BIGEOT
	call	WRTCMDA 		; write sec/trk (EOT)

       else     ;JAMEOT         	; do not force EOT to some maximum
	CALL	MLTCMD2 		; write bytes/sec (N) & sec/trk (EOT)
       endif    ;JAMEOT


	JC	RWV_30
	MOV	BL,STK_DL[BP]		; Create index to status byte
	XOR	BH,BH
	MOV	BL,FDMED[BX]		;   No, get the status byte
	TEST	BL,11000000b
	MOV	AL,27			; Set value for read gap if high data rate
	JZ	RWV_60	 		; If high data rate set gap 
	MOV	AL,42			; Else set gap for lower rates
RWV_60:	CALL	WRTCMD			; Write Read/Write Gap length (GPL)
	JC	RWV_30
	MOV	BX,6			; Offset for the data length byte
	MOV	CX,1			; will copy just 1 byte from DPB to FDC
	CALL	MLTCMD			; copy it
	JC	RWV_30			; If FDC error, exit
	CALL	WAITINT 		; wait for FDC interrupt
	JNC	RWV_65			; BR IF WE GOT THE INTERRUPT
	MOV	AH,ERRSTAT		; Save error stat during reset
	PUSH	AX
	CALL	DSKRESET		; If timeout reset disk if no interrupt
	POP	AX
	MOV	ERRSTAT,AH		; Restore error stat
	MOV	STK_AH[BP],AH		; Put error on stack for return
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
  IFDEF NNNCE89061711
	JMP	short RWV_100			; EXIT
   ELSE ;NNNCE89061711
	JMP	RWV_70			; exit
  ENDIF ;NNNCE89061711
RWV_65:	CALL	CHEKRES			; read and check results - C set if fail
;*			    if OK, set medium known; if retryable error, retry
	call	AFTERTRY		; returns Z to request retry
	jnz	RWV_70			; else major failure or success
	jmp	RWV_15
RWV_70:
   IFDEF  NNNCE89061712
	CMP	ERRSTAT,0		; CHECK IF ERROR
	JZ	RWV_72			; BR IF NO
	MOV	AL,0			; SET TO 
	JMP	short RWV_100			; BR IF YES
RWV_72:
   ENDIF  ;NNNCE89061712
	mov	bl, STK_DL[bp]
	mov	bh, 0
	mov	byte ptr FDOPER[bx], 0	; clear first-rate-tried marker
	MOV	CL,STK_CL[BP]		; CL= start sector
;
RWV_75: 	;This exit - attempt to calculate # of sectors xferred:
        sti

        mov     ch, stk_ch[bp]  	; ch=track number
        mov     dh, dskst+3     	; dh=track number that controller is at
        cmp     dh, ch          	; if not equal, one track is read
        je      RWV_85
        call    dpbadr          	; get disk parameter table address
                                	;    into ES:DI
        mov     ch, es:[si+4]   	; ch = # sector/track
        mov     dh, stk_dh[bp]  	; dh=head number
        test    dh, 1           	; if head is started at side 1
        jnz     RWV_80			; al = total # of sectors on both side - start sector + 1
        add     ch, ch          	; ch=total # sector on both side
RWV_80:
        ; al = #of sector - start sector + 1
        sub     ch, cl
        add     ch, 1
        mov     al, ch
        jmp     short RWV_100

RWV_85:	mov     dh, stk_dh[bp]  	; dh=head number
        mov     dl, dskst+4     	; get head address, either 0 or 1
        mov     al, dskst+5     	; get last sector xferred + 1
        cmp     dh, dl          	; if head stays on the same side after
        jne     RWV_90	        	;    the command
        sub     al, cl          	; subtract start sector to get #
                                	;  of sectors xferred.
        jmp     short RWV_95
RWV_90: call    dpbadr          	; get disk parameter table address
                                	;  into ES:DI
        push    cx
        mov     ch, es:[si+4]   ; ch = # sector/track
       ; AL = (# of sector/track-start sector+1) + sector to be read (R)-1
        sub     ch, cl              
        add     al, ch              
        pop     cx

RWV_95: mov     dl, dskst+1     	; read ST1
        test    dl, 80h         	; if controller tries to read sector
                                	;  beyond the final sector of a track
        jz      RWV_100
        add     al, 1           	; 1 is add to al because in this case

;
RWV_100:				;This exit - turn everything off - AL already set
	AND	DRVSTAT,07FH		; MAKE SURE INTERRUPT BIT IS RESET
RWV_110:				; This exit - load AH & C from ERRSTAT
	MOV	AH,ERRSTAT
	CMP	A0,AH			; set carry if non-zero
RWV_120:				; This exit - AX and C ready to go
	MOV	ERRSTAT,AH		; Set error status byte
	mov	STK_AL[bp], ax		; Load AL too!	We went to a lot of 
	PUSHF
	POP	STK_FL[BP]		;place flags where IRET will get
	JMP	FD_EXIT

  IFNDEF   NNN_FIXUP
RWV_130:MOV	ERRSTAT,ERR01		; INVALID OPERATION CODE
   IFDEF  NNNCE89061702
	MOV	AL,STK_AL[BP]		; LOAD PASSED AL VALUE
   ENDIF  ;NNNCE89061702
	JMP	RWV_110

    ENDIF  ;NNN_FIXUP
FD_VERFY	ENDP
;
;
;------------------------------------------------------------------------------
;	FD_FORMT (05H) - Format track(s) on diskette
;
;	INPUT:	AH = 05H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;		DH = Head number (0 - 1 )
;		CH = Track number
;		CL = Sector number
;		AL = number of sectors
;	     ES:BX = address of buffer containing address fields for track:
;		     byte 0 - track number
;		     byte 1 - head number
;		     byte 2 - sector number
;		     byte 3 - # of bytes per sector :
;			       00 - 128 bytes per sector
;			       01 - 256 bytes per sector
;			       02 - 512 bytes per sector
;			       03 - 1024 bytes per sector
;
;	OUTPUT: Carry set - Status non 0
;		AH = status : refer to FD_RESET for status values
;	Status byte 40:41 = status = AH
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_FORMT,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_FORMT   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
      ASSUME DS:ROMDAT

;
	CMP	DL,MAX_DRV	; Check for valid drive 
	JB	FMT_10		; BR IF LEGAL DRIVE
FMT_01: MOV     ERRSTAT,ERR01   ; SET ERROR CODE FOR INVALID COMMAND
	JMP	FMT_50	 	; If invalid drive, exit
   IFNDEF   NNN_FIXUP
FMT_05:	JMP	FMT_40
   ENDIF  ;NNN_FIXUP
;
;		       --- CHECK IF MEDIA KNOWN ---
;
FMT_10:
	MOV	BL,STK_DL[BP]	; Get drive number
	XOR	BH,BH		; Creat index to FDMED array
;
       if	not NOFORFORMAT ; change medium type if known on format
	TEST	FDMED[BX],10H	; If media known
	JNZ	FMT_20		;   Do format
;
;		  --- MEDIA NOT KNOWN, CHECK CMOS FOR TYPE ---
	CALL	FDTYPIF 	; Get cmos drive info
	JBE	FMT_01		; CMOS BAD - REPORT ERROR
				;      exit with error (Z=1 or C=1)
	cmp	al, 4
	ja	FMT_01		; if drive type invalid, exit w/ error
	MOV	DI,OFFSET CS:STRT_TAB	; Get index to media byte value
	XOR	AH,AH
	ADD	DI,AX						       
	MOV	AL,CS:[DI]
	MOV	BYTE PTR FDMED[BX],AL
       endif    ;not NOFORFORMAT 
;
;		       --- MEDIA SET, DO FORMAT ---
;

FMT_20:	MOV	ERRSTAT,NO_ERROR; init error code
	CALL	DSKMOT		; start motor
IFDEF	NNNINT077
	CALL	IS_I077_INSTD		; sense 82077
ENDIF	;NNNINT077
	CALL	CHK_CHG 	; check change line
  IFDEF NNNCE89042810
	JC	FMT_50		; WE GOT AN ERROR, LETS REPORT IT
  ELSE ;NNNCE89042810  
	JC	FMT_01		; If change line or timeout, exit
  ENDIF ;NNNCE89042810  
;
	CALL	INIT_DMA	; initialize DMA
  IFDEF NNNCE89061705
	JC	FMT_50		; GOT AN ERROR - REPORT IT
  ELSE ;NNNCE89061705
	JC	FMT_01		; If DMA boundary error, exit
  ENDIF ;NNNCE89061705

	MOV	CH,STK_CH[BP]	; get requested cylinder #
	CALL	DOSEEK		; seek to it
  IFDEF NNNCE89061704
	JC	FMT_50		; GOT AN ERROR - REPORT IT
  ELSE ;NNNCE89061704
	JC	FMT_01		; If DMA boundary error, exit
  ENDIF ;NNNCE89061704

	push	dx
	CALL	SETRAT		; Doseek forces lower data rate
	pop	dx		; restore flag used by HEDSETL
	call	HEDSETL			; WAIT FOR HEAD SETTLE
	call	SPINUP		; wait for spinup if necessary
	MOV	AL,FRMTCMD
	CALL	WRTCMD		; load command in FDC
	JC	FMT_50
	CALL	DRIHEDSEL	; load drive/head select in FDC
	JC	FMT_50
	MOV	BX,3		; BX= offset in DPB
	CALL	MLTCMD2 	; write bytes/sec & sec/trk
	JC	FMT_50
	MOV	BX,7		; BX= offset in DPB
	CALL	MLTCMD2 	; write gap len & filler byte
	JC	FMT_50		
	CALL	WAITINT 	; wait for FDC interrupt
	JNC	FMT_30
	MOV	AH,ERRSTAT	; Save error status during reset
	PUSH	AX
	CALL	DSKRESET	; If timeout reset disk if no interrupt
	POP	AX
	MOV	ERRSTAT,AH	; Restore error stat
	JMP	short FMT_50	 	; exit
;
;	       --- FORMAT COMPLETE, CHECK RESULTS ---
;
FMT_30:	CALL	CHEKRES 	; read and check results - C set if fail
	JC	FMT_50
;
;		     --- FORMAT OK EXIT ---
;
FMT_40:	MOV	CL,1		; CL is first sector
	MOV	STK_CL[BP],CL	; Set sector number
	MOV	BYTE PTR STK_AH[BP],0	; Zero AH for exit
  IFDEF NNNMACRO
	CLEAR_CARRY		; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY	; Clear carry flag
  ENDIF ;NNNMACRO
	JMP	FD_EXIT 		; And exit
;
;		   --- FORMAT ERROR HANDLER ---
;
FMT_50:	
	MOV	AH,ERRSTAT		; Get error code
	MOV	STK_AH[BP],AH		; Set in AH for return
  IFDEF  NNNMACRO
	SET_CARRY		; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY	; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXIT 		; Exit

FD_FORMT	ENDP
;
;
;------------------------------------------------------------------------------
;	FD_RDPAR (08H) - Read drive parameters
;
;	INPUT:	AH = 08H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;
;	OUTPUT: Carry set - Status non 0
;		AL = 0
;		AH = status : refer to FD_RESET for status values
;		BL = bits 4 - 7 =0, 
;		     bits 0 - 3 = valid drive type
;			  = 01H - 360Kb. , 5.25 in. , 40 track
;			  = 02H - 1.2Mb. , 5.25 in. , 80 track
;			  = 03H - 720Kb. , 3.5 in. , 80 track
;			  = 04H - 1.44Mb. , 3.5 in. , 80 track
;		BH = 00
;		CL = bits 6,7 = number of tracks (high 2 bits of 10 bit number)
;		     bits 0 - 5 = number of sectors per track
;		CH = number of tracks( low 8 bits)
;		DL = number of diskette drives
;		DH = number of heads
;	     ES:DI = pointer to drive parameter table	   
;	Status byte 40:41 = status = AH
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_RDPAR,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_RDPAR   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
;	       --- CHECK DRIVE REQUEST NUMBER ---
;
	MOV	ERRSTAT,NO_ERROR	; Clear ERRSTAT
  IFDEF NNNMACRO
	CLEAR_CARRY			; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY		; Clear carry flag
  ENDIF ;NNNMACRO
	OR	DL,DL
	JS	RDP_10
	CMP	DL,MAX_DRV	
	JAE	RDP_20			;if DL out of range exit w/ regs zeroed
;
;	  --- GET DRIVE INFO ---
;
	CALL	GET_TYPE
	OR	DI,DI			; Drive info known
	JZ	RDP_20
	MOV	CL,CS:[DI+4]		; Get number of sectors
	MOV	CH,CS:[DI+11]		; Get number of tracks
	MOV	BH,0
	MOV	BL,CS:[DI+0DH]		; Get cmos type
;
;	       --- GET NUMBER OF DRIVES ---
;
	MOV	DL,BYTE PTR DEVFLG	; Get DEVFLG
	AND	DL,0C0H 		; Keep just drive bits
	MOV	DL,1			; Set number to 1
	JZ	RDP_05			; Keep 1 if only 1
	INC	DL			; Else set to 2
RDP_05:	MOV	DH,1			; Set head to 1
	MOV	AX,CS			; Set segment to AX for exit
	JMP	short RDP_30
;
;    --- EXIT POINT IF REQUESTED DRIVE NUMBER OUT OF RANGE ---
;
RDP_10:
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	MOV	AH,01  
	MOV	STK_AH[BP],AH		; Set error status to a 01
	OR	STK_FL[BP],200h 	; Set interrupt flag
	MOV	ERRSTAT,AH
	jmp	FD_EXITF		; return without zapping other regs
;
;	 --- EXIT POINT IF NO DRIVES INDICATED BY DEVFLG ---
RDP_20:	mov	dh, byte ptr DEVFLG	; Get DEVFLG
	test	dh, 00000001b		; boot diskette available
	mov	dl, 0
	jz	RDP_25			; no - 0 drives
	inc	dx			; else try 1 drive
	test	dh, 11000000b		; Keep just drive bits
	jz	RDP_25			; Keep 1 if only 1
	inc	dl			; Else set to 2
RDP_25:	XOR	DH,DH
	XOR	BX,BX			; zero rest of the registers
	XOR	CX,CX
	XOR	DI,DI
	XOR	AX,AX			; ES-to-be
;
;		       ---   EXIT   ---
;
RDP_30:	MOV	WORD PTR STK_CL[BP],CX	; Set register values back on the stack
	MOV	WORD PTR STK_DL[BP],DX
	MOV	WORD PTR STK_BL[BP],BX
	MOV	STK_DI[BP],DI
	MOV	STK_ES[BP],AX
	XOR	AX,AX				       
	MOV	AH,ERRSTAT
	MOV	WORD PTR STK_AL[BP],AX
	OR	STK_FL[BP],200h 	; Set interrupt flag
	JMP	FD_EXITF

FD_RDPAR	ENDP
;
;
;------------------------------------------------------------------------------
;	FD_RDASD (15H) - Read DASD type
;
;	INPUT:	AH = 15H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;
;	OUTPUT: Carry set - Status non 0
;		AH = status =
;		     00H - drive not present
;		     01H - diskette , no change line present
;		     02H - diskette , change line present
;		     03H - reserved
;
;	Status byte 40:41 = status = AH
;	
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_RDASD,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_RDASD   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
;	    --- CHECK FOR VALID DRIVE NUMBER ---
;
	CMP	DL,MAX_DRV		; If drive invalid,
   IFDEF NNNCE89061703
 	JB	RDA_05			; BR IF DRIVE OK
   ELSE  ;NNNCE89061703
	JNA	RDA_05	 		;   exit w/ AH=01
   ENDIF ;NNNCE89061703
;	
;	   ---- INVALID DRIVE NUMBER, SET ERROR TO 01 AND EXIT ---
;
	MOV	AH,01H			; Set error code
	MOV	ERRSTAT,AH
	MOV	STK_AH[BP],AH
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXITF		; EXIT
;
;	   --- DRIVE NUMBER OK, CHECK IF DRIVE PRESENT ---
;
RDA_05:	MOV	ERRSTAT,NO_ERROR	; Clear error status byte
	MOV	BL,DL			; Create index to status byte
	XOR	BH,BH			; Clear upper byte of index
	MOV	AH,FDMED[BX]		; Get the status byte
	TEST	AH,0FFH 		; Drive present?      
	JZ	RDA_10

RDA_06:	CALL	GETDINF 		; Get drive info from HDCFLG
	TEST	AL,01			; Does drive support change line??
	MOV	AH,1			; Set exit values

  IFDEF NNNCE89061701

	JZ	RDA_20			; If no chang line leave at 01
	INC	AH			; Else set to 02
	JMP	SHORT RDA_20		; LET'S GET OUT
;
;	Determined Drive is not Present
;	Make one attempt see if drive is really
;	there, if not then exit. If there program
;	will set up FDMED for the selected drive.
;
RDA_10:	MOV	BYTE PTR STK_AH[BP],2	; set for read
	CALL	DSKTEST			; cause disk test to enter
	MOV	BYTE PTR STK_AH[BP],15H	; restore orginal
	MOV	BL,DL			; Create index to status byte
	XOR	BH,BH			; Clear upper byte of index
	MOV	AH,FDMED[BX]		; Get the status byte
	TEST	AH,0FFH 		; Drive present?      
	JNZ	RDA_06			; CONTINUE IN ROUTINE
;					; OTHERWISE DRIVE NOT PRESENT	
RDA_20:	MOV	STK_AH[BP],AH		; Set values on stack for return
    ELSE   ;NNNCE89061701

	JZ	RDA_10			; If no chang line leave at 01
	INC	AH			; Else set to 02
RDA_10: MOV	STK_AH[BP],AH		; SET VALUE ON STACK
    ENDIF  ;NNNCE89061701
  IFDEF NNNMACRO
	CLEAR_CARRY			; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY		; Clear carry flag
  ENDIF ;NNNMACRO
	JMP	FD_EXITF			; Exit
FD_RDASD	ENDP
;
;------------------------------------------------------------------------------
;	FD_CHNG (16H) - Read disk change line status
;
;	INPUT:	AH = 16H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;
;	OUTPUT: Carry set - Status non 0
;		AH = status =
;		     00H - diskette change line not active
;		     01H - invalid diskette parameter 
;		     06H - diskette change line active
;		     80H - diskette drive not ready
;
;	Status byte 40:41 = status = AH
;	
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_CHNG,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_CHNG   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
;	    --- CHECK FOR VALID DRIVE NUMBER ---
;
IFDEF	REV_A_FIX
    IF	INFO
	call	wrtinl		; wds
	DB	'FD_CHNG: ',0
    ENDIF
ENDIF
	CMP	DL,MAX_DRV		; If not valid drive request
   IFDEF NNNCE89061703
	JB	CHNG_05			; BR IF DRIVE OK
   ELSE  ;NNNCE89061703
	JNA	CHNG_05			
   ENDIF  ;NNNCE89061703
;
	MOV	AH,ERR01		; Set error code to 01
	MOV	ERRSTAT,AH
	MOV	STK_AH[BP],AH
;
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXITF		; and exit
;	
;	     --- DRIVE NUMBER OK, CHECK IF DRIVE EXISTS ---
;
CHNG_05:
      IFDEF    NNNEA89061413
       IFDEF	NNNEA89062302
	CALL	FDTYPIF 		; Get drive type from CMOS (10h)
; Both ZF and CF are errors coming out of FDTYPIF
	JNBE	CHNG_07 		; If drive doesn't exist
       ELSE	;NNNEA89062302
        CALL    FDTYPE                  ; Get drive type from CMOS
        JNZ     CHNG_07                 ; If drive doesn't exist
       ENDIF	;NNNEA89062302
        MOV     AH,FDSKTMO              ; Set error code to 80
        MOV     ERRSTAT,AH
        MOV     STK_AH[BP],AH
;
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE	;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
        JMP     FD_EXITF                ; and exit
CHNG_07:
      ENDIF    ;NNNEA89061413
;
;            --- DRIVE NUMBER OK, CHECK IF 40 TRACK DRIVE ---
; 
        MOV	BL,STK_DL[BP]		; Create index to status byte
	XOR	BH,BH
	CALL	GETDINF 		; If 40-track drive (no change line)
	TEST	AL,0001b		;      
	JNZ	CHNG_10		      
	JMP	short CHNG_40			;	 Go return 06
;							 
;      --- DRIVE IS 80 TRACK THEREFORE IT SUPPORTS CHANGE LINE ---
;
CHNG_10: 				; Else
;
IFNDEF	REV_A_FIX	; original stuff
	CALL	DSKMOT			; Select drive by turning motor on
;
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
	IN	AL,DX
	TEST	AL,80h			; Is the diskette change line active?
	MOV	ERRSTAT,NO_ERROR	; Clear error code
	MOV	AH,00
	JZ	CHNG_50	 		;   No
ELSE	; have to fake diskchange (wds)
	mov	al,FDMOTS
	test	dl,1
	jz	check_drive0
; checking drive 1
	test	al,2h
	jnz	CHNG_50
	jmp	skipover_drive0
; checking drive 0
check_drive0:
	test	al,1h
	jnz	CHNG_50
skipover_drive0:
    IF	INFO
	call	wrtinl	
	DB	'Motor is off, changed',CR,LF,0
    ENDIF
ENDIF	; REV_A_FIX
	MOV	AL,FDMED[BX]		; Clear media known flag
	AND	AL,07			; If 3.5 " drive
	CMP	AL,07
	JNZ	CHNG_20
	AND	BYTE PTR FDMED[BX],0EFH ; Just clear media known bit
	JMP	short CHNG_40
CHNG_20:MOV	AL,FDMED[BX]		; Else clear media known adjust
	AND	AL,11101111b		; Media byte
	INC	AL
	TEST	AL,00000100b
	JZ	CHNG_30			; low bits were 3, 4, 5, or 6
	SUB	AL,3
CHNG_30:DEC	AL
	MOV	FDMED[BX],AL
;
;      ---  CHANGE LINE ACTIVE, SET UP VALUES FOR RETURN ---
;			    
CHNG_40:MOV	ERRSTAT,ERR06		; Set Media change
	MOV	BYTE PTR STK_AH[BP],ERR06	
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXIT 		; Exit
;
;		--- NO CHANGE EXIT ---
;
CHNG_50:
  IFDEF NNNMACRO
	CLEAR_CARRY			; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY		; Clear carry flag
  ENDIF ;NNNMACRO
	MOV	STK_AH[BP],AH
	JMP	FD_EXIT 		; use this exit if DSKMOT called

FD_CHNG 	ENDP
;
;
;------------------------------------------------------------------------------
;	FD_SDASD (17H) - Set DASD type
;
;	INPUT:	AH = 17H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;		AL = 00H - invalid request
;		     01H - diskette 360Kb in 360Kb drive
;		     02H - diskette 360Kb in 1.2Mb drive
;		     03H - diskette 1.2Mb in 1.2Mb drive
;		     04H - diskette 720Kb in 720Kb drive
;
;	OUTPUT: Carry set - Status non 0
;		AH = status : refer to FD_RESET for status values
;
;	Status byte 40:41 = status = AH
;	
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_SDASD,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_SDASD   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	xor	di, di			; clear DSKMOT-called flag
	MOV	ERRSTAT,NO_ERROR	; Clear error code
	MOV	BL,STK_DL[BP]		; Get drive #
	XOR	BH,BH			; Make it into a byte pointer
  IFDEF NNNEN89030722
	CMP	BL,MAX_DRV		; CHECK IF VALID DRIVE
	JAE	SDA_40			; BR IF INVALID
   ENDIF ;NNNEN89030722
	
	MOV	AL,STK_AL[BP]		; Get request type
	DEC	AL			; If AL = 1, assume a 360K drive
	MOV	AH,93h			;   Yes, set media type
       IFDEF	NNNEA89070704
        jz	SDA_80                  ; set media type  
       ELSE	;NNNEA89070704
	JNZ	SDA_10
	CALL	GETDINF 		; Get drive info
	TEST	AL,1			; Change line is not supported om 360
	JZ	SDA_80

        cmp     byte ptr stk_dl[bp], 0
        je      SDA_05
        and     hdcflg, 0efh
        jmp     short SDA_80
SDA_05: and     hdcflg, 0feh            ; reset change line
        jmp     short SDA_80                  ; set media type  
       ENDIF	;NNNEA89070704

SDA_10:	DEC	AL			; If AL = 2, it's 360K in 1.2M drive
	MOV	AH,74h			;   Yes, set media type
    IFNDEF  NNN_FIXUP
	JNZ	SDA_20
     ENDIF ;NNN_FIXUP
	JZ	SDA_60			;   and go check change line
;
SDA_20:	DEC	AL			; If AL = 3, it's 1.2M in 1.2M drive
	MOV	AH,15h			;   Yes, set media type
   IFNDEF  NNN_FIXUP
	JNZ	SDA_30
   ENDIF ;NNN_FIXUP
	JZ	SDA_60			;   and go check change line
;
SDA_30:	DEC	AL			; If AL = 4, it's 720K in 720K drive
	MOV	AH,97h			;   Yes, set media type
   IFNDEF  NNN_FIXUP
	JNZ	SDA_40
   ENDIF ;NNN_FIXUP
	JZ	SDA_60
SDA_40:
;
;		 --- INVALID TYPE REQUEST ---
;
	MOV	AH,01			; Set error for invalid drive type req
;
SDA_50:	MOV	ERRSTAT,AH
	MOV	STK_AH[BP],AH
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	jmp	short SDA_90

       IFDEF	NNNEA89062001
SDA_60:
	PUSH	AX
	CALL	DSKMOT
	CALL	CHK_CHG			; if change line active
	POP	AX
       ELSE	; NNNEA89062001
SDA_60: call    dskmot
        call    chk_chg                 ; if change line active
       ENDIF	; NNNEA89062001	
        jnc     SDA_70                  ; returned with ah =06h
        mov     ah, errstat
        jmp     SDA_50
SDA_70:	CALL	GETDINF 		; Get drive info
	TEST	AL,1			; Change line is not supported om 360
	jz	SDA_80
	PUSH	AX
	call	DSKMOT
	inc	di			; set DSKMOT-called flag
	CALL	CLR_CHG 		; Clear change line, maybe set ERRSTAT
	POP	AX
	JNC	SDA_80			; Jump if it could be cleared
	MOV	AH,ERRSTAT
	JMP	SDA_50
SDA_80:	MOV	BL,STK_DL[BP]		; Get drive #
	XOR	BH,BH			; Make it into a byte pointer
	MOV	FDMED[BX],AH		;   Yes, set media type
        mov     ah, 0
        mov     stk_ah[bp], ah
  IFDEF NNNMACRO
	CLEAR_CARRY			; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY		; Clear carry flag
  ENDIF ;NNNMACRO
SDA_90:	or	di, di			; test DSKMOT-called flag
	JNZ	SDA_100
	JMP	FD_EXITF		; exit without restoring FDTIMO

SDA_100:JMP	FD_EXIT 		; exit restoring FDTIMO
FD_SDASD	ENDP
;	       
;------------------------------------------------------------------------------
;	FD_SFRMT (18H) - Set type for format
;
;	INPUT:	AH = 18H
;		DL = Drive number (0 - 3), bit 7=0 for floppy
;		CL = bits 6,7 = number of tracks (high 2 bits of 10 bit number)
;		     bits 0 - 5 = number of sectors per track
;		CH = number of tracks( low 8 bits)
;
;	OUTPUT: Carry set - Status non 0
;		AH = status : refer to FD_RESET for status values
;	     ES:DI = pointer to 11 byte parameter table, unchanged if AH non 0
;	Status byte 40:41 = status = AH
;	
;------------------------------------------------------------------------------
;

IFDEF   NNN_ECR_25
        SUBROUT         FD_SFRMT,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FD_SFRMT   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	IF	DIAG	; (wds)
	call	wrtinl
	DB	'Set Drive type for format',CR,LF,0
	ENDIF

	MOV	ERRSTAT,NO_ERROR	; Clear error code
;
;	  --- CHECK FOR VALID DRIVE REQUEST ---
;
	CMP	DL,MAX_DRV		; If drive request invalid
   IFDEF  NNNCE89061703
	JB	SFR_10
   ELSE  ;NNNCE89061703
	JNA	SFR_10
   ENDIF ;NNNCE89061703
	MOV	ERRSTAT,ERR01		; Set error stat
	MOV	BYTE PTR STK_AH[BP],ERR01   
;
SFR_05:
  IFDEF  NNNMACRO
	SET_CARRY			; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY		; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXITF				    
;
SFR_10:
SFR_20:
       IFDEF	NNNEA89062302
	CALL	FDTYPIF 		; Get drive type from CMOS (10h)
; CF coming out of FDTYPIF indicates bad CMOS
	JC	SFR_30			; drive might exist; try to find out
; ZF coming out of FDTYPIF indicates no drive present
       ELSE	;NNNEA89062302
        call    fdtype                  ; get floppy disk type from CMOS (10h)
        cmp     al, 0                   ; if no drive is present
       ENDIF	;NNNEA89062302
        jne     SFR_30	                ;  return ah = 0 w/ carry clear
        mov     ah, 0
        mov     stk_ah[bp], ah
        mov     errstat, ah
  IFDEF NNNMACRO
	CLEAR_CARRY
  ELSE ;NNNMACRO
        call    clr_cary
  ENDIF ;NNNMACRO
        jmp     fd_exitf

SFR_30: 
       IFNDEF	NNNEA89070704
	cmp     al, 1                   ; if drive type in cmos is 360
        jne     SFR_40
        mov     al, hdcflg
        cmp     byte ptr stk_dl[bp], 0
        je      SFR_35
        and     al, 0efh
        mov     hdcflg, al
        jmp     short SFR_40
SFR_35: and     al, 0feh                ; reset change line
        mov     hdcflg, al
SFR_40:	
       ENDIF	;NNNEA89070704 NDEF
	CALL	GET_TYPE		; Get type info
	or	di, di			; Drive info known
	jz	SFR_70
	MOV	CX,WORD PTR STK_CL[BP]	; Restore CX
  IFDEF  NNNEN88121443
SFR_45:	CMP	CH,CS:O_MAXCYL[DI]	; See if it matches callers info
	JNE	SFR_50	 		; If not try another
	CMP	CL,CS:O_MAXSEC[DI]
	JE	SFR_55
SFR_50:	SUB	DI,PAMSIZ		; CHECK NEXT TABLE
	DEC	BL			; SEE IF LAST ON CHECKED
	JNZ	SFR_45			; TRY THAT ONE
	JMP	short SFR_70			; REPORT THE ERROR
  ELSE  ;NNNEN88121443
SFR_45:	CMP	CH,CS:O_MAXCYL[DI]	; See if it matches callers info
	JNE	SFR_50	 		; If not try another
	CMP	CL,CS:O_MAXSEC[DI]
	JE	SFR_55
SFR_50:	SUB	DI,PAMSIZ
	CMP	CH,CS:O_MAXCYL[DI]	; See if it matches callers info
	JNE	SFR_70			; If not try another
	CMP	CL,CS:O_MAXSEC[DI]
	JNE	SFR_70
   ENDIF ;NNNEN88121443
SFR_55:
;
	MOV	AH,CS:O_DATRAT[DI]	; Get data rate info
;
	CMP	DI,OFFSET PAMSMED	; If we think it is a 360/360
	JE	SFR_60
	CMP	CH,40			; Make sure tracks = 40
	JNB	SFR_60
;
	OR	AH,00100000b		; Set double step
SFR_60:	mov	bh, 0
	mov	bl, byte ptr STK_DL[bp]
	CALL	RESTYP			; Store medium flags 
	XOR	AH,AH			; Clear error indicator
	MOV	ERRSTAT,AH
	MOV	STK_AH[BP],AH
  IFDEF NNNMACRO
	CLEAR_CARRY			; CLEAR CARRY BIT
  ELSE	;NNNMACRO
	CALL	CLR_CARY		; Clear carry flag
  ENDIF ;NNNMACRO
;
	MOV	STK_DI[BP],DI		; Set ES:DI to point to parameters
	MOV	STK_ES[BP],CS
SFR_65:	JMP	FD_EXITF
;*			     endif

SFR_70:
     IF SUP720	; Support for 720 disk in 1.2 meg drive
; CHECK IF IT IS A REQUEST FOR 720 IN A 1.2

	CALL	FDTYPIF 		; Check CMOS should be a 1.2
	JC	SFR_75			; If CMOS bad, drive not configured
	JZ	SFR_75			; 
	CMP	AL,2			; Or not a 1.2 drive
	JNZ	SFR_75			; then exit with invalid request
;
	MOV	DI,OFFSET PAM720	; Else check if values match 720 in 1.2
	CMP	CH,CS:O_MAXCYL[DI]	; See if it matches callers info
	JNE	SFR_75			; If not then error exit
	CMP	CL,CS:O_MAXSEC[DI]
	JNE	SFR_75
	JMP	SFR_55
     ENDIF      ;SUP720

SFR_75:	MOV	AX,0C00h
	MOV	STK_AH[BP],AH
	MOV	ERRSTAT,AH
  IFDEF  NNNMACRO
	SET_CARRY		; SET CARRY BIT
  ELSE  ;NNNMACRO
	CALL	SET_CARY	; Set the carry flag on stack
  ENDIF ;NNNMACRO
	JMP	FD_EXITF

       	RET
FD_SFRMT	ENDP


;
;******************************************************************************
;*			UTILITY ROUTINES
;******************************************************************************
;
;------------------------------------------------------------------------------
;	CHEKRES - Read and checks results from FDC
;
;	INPUT:
;
;	OUTPUT:   Carry set if error, 
;		  AL = number of sectors transfered
;------------------------------------------------------------------------------
;
;
SETERRT DB	ERR04		; SECTOR NOT FOUND (msb)
	DB	FDCTERR 	; CONTROLLER ERROR
	DB	ERR10		; CRC ERROR
	DB	ERR08		; DMA OVERRUN
	DB	FDCTERR 	; CONTROLLER
	DB	ERR04		; SECTOR NOT FOUND
	DB	ERR03		; WRITE PROTECTED
	DB	ERR02		; MISSING ADDRESS MARK (lsb)
;
IFDEF   NNN_ECR_25
        SUBROUT         CHEKRES,NON_GLOBAL
ELSE    ;NNN_ECR_25
        CHEKRES   PROC    NEAR
ENDIF   ;NNN_ECR_25
;			       
	CALL	RSLTRD7 	; Read command results, 7 bytes
	JC	CHK_35	 	; Exit if can't read results
	AND	AH,ATERM+IC	; Normal completion ?
	JZ	CHK_35	 	;  Yes, operation complete, no errors
				;  else fall thru to decode error
;
;	    ----- INTERPET STATUS RETURNED BY FDC -----
;
CHK_05: CMP	AH,ATERM	; Unless it's abnormal termination, 
	JNE	CHK_25		;  go call it FDC failure
				;  continue only if status 0 is 01xxxxxxb
	MOV	AH,BYTE PTR [DSKST+1]	; Fetch FDC result ST1
	AND	AH,10110111B	; Mask out unwanted bits
;
	MOV	BX,8		; BX = # of bits to look at
CHK_10:	DEC	BX		; Decrement BX
	SHR	AH,1		; Get next bit and fill MSB with zero
IFDEF	NNNINT077
	JC	CHK_15		; Is error bit set?
ENDIF	;NNNINT077
	JNZ	CHK_10		; No, keep looking until we zero reg
IFDEF	NNNINT077
	JMP	CHK_25		
ENDIF	;NNNINT077
;
CHK_15:
IFDEF	NNNINT077
;
;if 82077 is detected and its FIFO is enabled
;
	TEST	WORD PTR STK_T1[BP],02
	JZ	CHK_20
;
;"No Data" error is only meaningful in a read operation.
	CMP	BYTE PTR STK_AH[BP],02
	JE	CHK_20
	CMP	BL,5
	JE	CHK_35
CHK_20:
ENDIF	;NNNINT077
	MOV	AL,[BX+SETERRT] ; GET ERROR MASK
	JMP	short CHK_30
CHK_25:	MOV	AL,FDCTERR	; Call it controller failure
CHK_30:	mov	ERRSTAT,AL	; include it in ERRSTAT
	STC
CHK_35:	RET			; return if no error found
CHEKRES ENDP

;------------------------------------------------------------------------------
;	CHK_CHG - Check change line on drive
;
;	INPUT:
;	
;	OUTPUT: C set if change line error
;------------------------------------------------------------------------------
IFDEF   NNN_ECR_25
         SUBROUT         CHK_CHG,NON_GLOBAL
ELSE    ;NNN_ECR_25
        CHK_CHG   PROC    NEAR
ENDIF   ;NNN_ECR_25
;				   
	MOV	BL,STK_DL[BP]	; Get offset to FDMED, FDOP arrays
	XOR	BH,BH
;					
;	       --- CHECK IF CHANGE LINE SUPPORTED ---
;
	CALL	GETDINF 	; Get info from HDCFLG
	TEST	AL,1		; See if change line supported
	CLC			;	
	JZ	CHG_55	 	; Return with carry clear, disk changed
;
;				;    valid
	MOV	DX,FDRTCP	; AT Floppy disk rate control port	
	IN	AL,DX		; Read rate control port
	OR	AL,AL		; Is the diskette change line active ?
	JNS	CHG_55	 	; Exit if change line not active
    IF	INFO
	call	wrtinl		; (wds)
	DB	'Disk changed',CR,LF,0	; (wds) informational 
    ENDIF
if	NOCLRFORMAT     ;don't clear known in CLR_CHG if it's format
	cmp	STK_AH[bp], 05h
	je	CHG_40
	cmp	STK_AH[bp], 05h+10000000b
	je	CHG_40
       endif    ;NOCLRFORMAT

	MOV	AL,FDMED[BX]	; Get media type byte
	AND	AL,07H		; If 3.5" just clear known bit
	CMP	AL,07
	JNZ	CHG_10
	MOV	AL,FDMED[BX]
	AND	AL,0EFH
	JMP	short CHG_30
CHG_10:
;
;		 ---- CLEAR MEDIA KNOWN BIT ----
;
	MOV	AL,FDMED[BX]	; Get media type byte
	AND	AL,11101111b	; Clear known bit
;
;	     ---- SET MEDIA/DRIVE COMBINATION BITS ----
;
	INC	AL		; Increment FDMED to check known type
	TEST	AL,00000100b	; Bit 2 will be set if media was known
	JZ	CHG_20		; Media was known, set it to trying
	SUB	AL,3
CHG_20:	DEC	AL		; Restore orig type
CHG_30:	MOV	FDMED[BX],AL	; Set media byte
;
;	       ---- TRY TO CLEAR CHANGE LINE ----
;
CHG_40:	MOV	AH, 6		; Default: Media change line error
	CALL	CLR_CHG 	; Clear change line, set timeout and
				;   media change as appropriate
CHG_50:	STC			; RETURN WITH CARRY SET
CHG_55:	RET

CHK_CHG ENDP
;

;------------------------------------------------------------------------------
;	CHKTYP	- Check media type - Called prior to R/W operation to verify
;				     & update, if necessary, FDMED.  Returns 
;				     C set if no remaining valid FDMEDs to try.
;
;	INPUT:	FDMED, ERRSTAT (if non-0, it's retry), BX= drive
;
;	OUTPUT: updated FDMED, ERRSTAT = C = 0 if any more valid 
;			rate / step combos; FDMED restored, FDOP = 00,
;			ERRSTAT and C set if no more
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         CHKTYP,NON_GLOBAL
ELSE    ;NNN_ECR_25
        CHKTYP   PROC    NEAR
ENDIF   ;NNN_ECR_25
;*			    (if non-0, seek to physical cylinder 2)
;*			    else seek to cylinder 0
	push	cx
	mov	ch, byte ptr STK_CH[bp]
	or	ch,ch
	jz	CHKT_05
	mov	ch, 2
CHKT_05:call	SEEK_25 	;don't double track, but recal if necessary
	pop	cx
	call	HEDSETL		; WAIT FOR HEAD SETTLE TIME
	call	SPINUP		; WAIT FOR SPIN UP TIME

     if	SUP720       ;Support 720 disk in 1.2 M drive
;*if known 360K-in-1.2M, skip rate setting
	test	FDMED[bx],00010000b; the only way CHKTYP called if known
	jnz	CHKT_25
     endif    ;SUP720
;*			    if error (not 1st time), goto next prov rate
	cmp	ERRSTAT,NO_ERROR
	jne	CHKT_45


;*			    if drive type known to be 3 or 4,
	call	FDTYPIF
	jc	CHKT_10
	cmp	al, 3
	jnae	CHKT_10

;*				first provisional rate = slow
	mov	ax, (10000000b+7)*100h+10000000b

	jmp     short CHKT_15
CHKT_10:
;*				first provisional rate = medium
	mov	ax, (01100000b+1)*100h+01000000b
				; try double-step when trying medium
CHKT_15:


;*			    for each provisional rate,
	mov	FDOPER[bx], al  ; set end marker
CHKT_20:mov	FDMED[bx], ah
CHKT_25:call	SETRAT		; SET PROVISIONAL RATE
	MOV	AL,READID	; Issue read id command
	CALL	WRTCMD		
	JC	CHKT_60		; If error exit
	CALL	DRIHEDSEL	; Send head select command
	JC	CHKT_60		; If error exit
	CALL	WAITINT 	; Wait for interrupt
	JC	CHKT_60		; Error if timeout
	PUSH	BX
	call	CHEKRES 	; Get results & load ERRSTAT
	POP	BX
;*				if non-data error, give up
;*				if successful,
	jnc	CHKT_30	; C returned by CHKRES
	cmp	ERRSTAT,ERR02	; no AM - data error
	je	CHKT_40
	cmp	ERRSTAT,ERR10	; bad CRC in ID field - data err
	je	CHKT_40
	jmp	short CHKT_60		; ERRSTAT must be already loaded
CHKT_30:	cmp	byte ptr DSKST+3,2
	jne	CHKT_35
;*					clear double-step
	and	byte ptr FDMED[bx], not(00100000b)
CHKT_35:jmp	short CHKT_55

CHKT_40:
;*			     next provisional rate
;*			    return error
CHKT_45:mov	al, byte ptr FDMED[bx]
	and	al, 11000000b	; look at rates only
	mov	ah, 10000000b+0 ;if fast try slow
	jz	CHKT_50
	cmp	al, 01000000b
	mov	ah, 00000000b+2 ;if medium try fast
	je	CHKT_50
	mov	ah, 01100000b+1 ;if slow try medium
CHKT_50:mov	al, ah
	and	al, 11000000b	; look at rates only
	cmp	al, FDOPER[bx] 	; is it first tried?
	je	CHKT_60		; yes, we tried all - exit 
				;  w/ ERRSTAT already loaded
	jmp	CHKT_20

CHKT_55:AND	ERRSTAT,NO_ERROR; Clear ERRSTAT
	JMP	short CHKT_65

;		 ----- CHKTYP EXIT POINT -----
CHKT_60:mov	byte ptr FDOPER[bx], 0
	push	word ptr ERRSTAT
	CALL	DSKRESET
	pop	word ptr ERRSTAT; Restore error status
	STC			; Set error indication
CHKT_65:RET
CHKTYP	ENDP


;------------------------------------------------------------------------------
AFTERTRY	proc near	;called after diskette operation
;		Passed:  ERRSTAT, FDMED[drive]
;		Returns:  Z set to request retry if medium not known
;			  and error occurred.
;*			    prepare offset into FDMED, FDOP arrays
	mov	bl, STK_DL[bp]
	mov	bh, 0				;BX is index
;*			    if medium known, return without retry request
	test	FDMED[bx], 00010000b

       if	BESURE08                ;maintain flag indicating probable 1.44M drive
		        		;  to be checked by func 08
	jz	AFTER_05
	jmp	short AFTER_10		;possibly update drive info
AFTER_05:
       else     ;BESURE08               
	jnz	AFTER_70
       endif    ;BESURE08
;*			    if data error, return with retry request
	mov	al, ERRSTAT
	cmp	al, 2
	je	AFTER_70
	cmp	al, 4
	je	AFTER_70
	cmp	al, 10h
	je	AFTER_70
;*			    if other error, return without retry request
	cmp	al, 0
	jne	AFTER_70
;*			    force medium known & restore compatible low bits
	mov	ah, FDMED[bx]
	and	ah, 11100000b
	call	RESTYP			; medium in both AH and FDMED
AFTER_10:
;*			    get drive info
	call	GETDINF
	mov	cl, al
;*			    if rate = med or fast,
	test	byte ptr FDMED[bx], 10000000b
	jnz	AFTER_20		; also clears Z if jump
;*				say it's known multi-rate drive
	or	cl, 0110b
;*			     endif
AFTER_20:
       if	BESURE08                ;maintain flag indicating probable 1.44M drive
        				;  to be checked by func 08

	mov	al, STK_CL[bp]		;passed start sector
	add	al, STK_AL[bp]		;passed sector count
	cmp	al, 15+1
       IFDEF	NNNEA89061412
	ja	AFTER_40	 	; set flag saying probable 1.44M drive
       ELSE	;NNNEA89061412
	jna	AFTER_30
;*		
	or	cl, 1000b
;*			     else if we've read or written past sector 9,
	jmp	AFTER_60
AFTER_30:
       ENDIF	;NNNEA89061412
	cmp	al, 9+1
	jna	AFTER_60
;*				if CMOS says it's known drive type 1 or 3,
	call	FDTYPIF
       IFDEF	NNNEA89061412
; On these AT compatible machines, using 40:8F bits 3 and 7 to mean "we think
; drive and diskette are both 1.44M" is a better strategy than using it as a
; "drive is bigger than CMOS thinks" flag.  A 1.2M drive will be recognized
; despite a 360k or invalid CMOS setting because there is a change line and 
; the data rate is 300KBS or 500KBS.
;
; The problem with the "drive is bigger than CMOS thinks" flag is that a 1.2M 
; drive will be treated as a 1.44M if the CMOS indicates 360k or is invalid.
; 1.2M is a better guess because these are AT compatible machines.
	jbe	AFTER_60	; don't assume drive is bigger than 1.2M
       ELSE	;NNNEA89061412
	jbe	AFTER_40	; assume drive is bigger than CMOS thinks
	cmp	al, 1
	je	AFTER_40
       ENDIF	;NNNEA89061412
	cmp	al, 3
	jne	AFTER_60
AFTER_40:
;*				    set drive-bigger-than-CMOS-thinks flag
;*				    force to 1.44M drive
	or	cl, 1000b
AFTER_60:
       endif	;BESURE08

;*			    store updated drive info
	mov	al, cl
	call	STODINF
;*			    return without retry request
	or	al, 1				;clear Z
;*			    return w/ flag
AFTER_70:
	ret
AFTERTRY	endp


;  IFDEF NNNMACRO
;  ELSE  ;NNNMACRO
;------------------------------------------------------------------------------
;	CLR_CARY  -  Clear carry flag
;
;	INPUT: nothing
; 
;	OUTPUT: Carry flag bit cleared in stack location
;
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         CLR_CARY,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  CLR_CARY
        CLR_CARY   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	and	byte ptr STK_FL[bp], 11111110b	  ; clear C
	or	byte ptr STK_FL+1[bp], 00000010b	; set I

	RET
;
CLR_CARY	ENDP
;  ENDIF  ;NNNMACRO
;
;------------------------------------------------------------------------------
;	CLR_CHG - Test and Clear Media Change line
;
;	INPUT:	DL = Drive #
;
;	OUTPUT: ERRSTAT = timeout or media change error code if appropriate
;		Carry flag set if timeout
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         CLR_CHG,NON_GLOBAL
ELSE    ;NNN_ECR_25
        CLR_CHG   PROC    NEAR
ENDIF   ;NNN_ECR_25
					;   media change as appropriate
;*			    if change line active,
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
	IN	AL,DX
	OR	AL,AL			; Is the diskette change line active ?
					;   (clear carry in case of return)
	JNS	CLR_25			;   No

;*				(prepare index to arrays)
	MOV	BL,STK_DL[BP]		; Create index to medium type byte
	XOR	BH,BH
;*				    force 1 msec between drive select & steps
	PUSH	CX
	MOV	CX, 10	    ;@10 changed
CLR_05:	CALL	DLY100			; Delay 100 us per loop
	LOOP	CLR_05		;
;*				    seek to physical track 4
	MOV	CH,4
	CALL	SEEK_35
;
;      --- DELAY 6 MS BEFOR CHECKING CHANGE LINE ---
	PUSH	CX
	MOV	CX, 30	     ;@@
CLR_10:	CALL	DLY100
	LOOP	CLR_10
	POP	CX
;*				    recal
	CALL	DORECAL
;*				if change line cleared,
;*				    error code = 6
	POP	CX
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
	IN	AL,DX
	OR	AL,AL			; Is the diskette change line active ?
	MOV	AL,ERR06		;   Else set Media change
					;   (only on AT 1.2M controller)
	JNS	CLR_20			;   No

;*				    error code = 80
	MOV	AL,FDSKTMO		; Set device timeout error code
	STC				; Indicate timeout with carry flag
CLR_20:	MOV	ERRSTAT,AL		; Set error code
CLR_25:	RET

CLR_CHG ENDP
;
;------------------------------------------------------------------------------
;	DOSEEK	- Seek to track CH
;
;	INPUT:	  CH= track to seek,
;		  STK_DL[BP]= drive #
;		  BX = offset to FDMED
;	OUTPUT:   Carry, ERRSTAT; DX=0 if C=0 and seek not done
;			    if medium type flags say double step,
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
IFDEF   NNN_ECR_25
        SUBROUT         DOSEEK,NON_GLOBAL
ELSE    ;NNN_ECR_25
        DOSEEK   PROC    NEAR
ENDIF   ;NNN_ECR_25
       ASSUME ES:NOTHING
			; passed CH= track to seek,
			;  STK_DL[BP]= drive #
			; returns C, ERRSTAT; DX=0 if C=0 and seek not done
;*			    if medium type flags say double step,
	MOV	BL,STK_DL[BP]
	XOR	BH,BH			; BX= drive #
 	PUSH	DX
	MOV	AL,FDMED[BX]		; Get media type
	ROL	AL,1			; Put rate bits in LSBs
	ROL	AL,1
	AND	AL,11111b		; Save media type and rate bits
	CMP	AL,11100b		; Check media type
	JAE	SEEK_05			; Jump if 3.5 media
	MOV	AL,2 			;  Else force slow data rate
SEEK_05:AND	AL,11b			; Keep only rate bits					       
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
	OUT	DX,AL			; Set data rate
	ROR	AL,1			; Restore rate bits to MSBs
	ROR	AL,1
	OR 	FDRATE,80H		; Set rate bit
	POP	DX
	TEST	FDMED[BX],20h		; is media type being double-stepped?
	JZ	SEEK_10
	SHL	CH,1			; if so, double selected track
SEEK_10:
  IFDEF NNNSEEK_UPDATE
	
					;  known track in FDTRCK[drive]
;*			    if current track = sought track, return w/ no error
	XOR	DX,DX			; DX = 0 NO SEEK
	MOV	BL,STK_DL[BP]
	XOR	BH,BH			; BX= drive #
	CMP	FDTRCK[BX],CH
	JNZ	SEEK_11
	JMP	SEEK_65 		; C cleared if equal

SEEK_11:
  ENDIF  ;NNNSEEK_UPDATE						     

;------------------------------------------------------------------------------
;*			    if drive just now selected (motor just now started)
	CMP	FDTIMO,0FEH
	JNAE	SEEK_20
;*				force 4 msec between drive select & steps
	PUSH	CX
	MOV	CX, 40			; FOR 40 M SEC DELAY
SEEK_15:CALL	DLY100
	LOOP	SEEK_15			; EXTERNAL LOOP
	POP	CX    
SEEK_20:
;------------------------------------------------------------------------------
SEEK_25:
; 		passed CH= track to seek (*2 if double-stepping),
;  		STK_DL[BP]= drive #
; 		returns C, ERRSTAT; DX=0 if C=0 and seek not done
;	    set flag to indicate no real seek
	XOR	DX,DX			; DX=0 means no seek
					; set to port by SISSTAT, etc.
;*			    if current track unknown,
	MOV	AL,STK_DL[BP]		; get drive #
	INC	AL			; AL is 1 for drive 0, 2 for drive 1
	AND	AL,DRVSTAT		; recal needed here?
	JNZ	SEEK_30
	PUSH	CX
	CALL	DORECAL 		; RECALIBRATE DRIVE
	POP	CX
	JC	SEEK_65 		; return if recal failed
					; BX= drive # still
;------------------------------------------------------------------------------
SEEK_30:
  IFDEF NNNSEEK_UPDATE
  ELSE  ;NNNSEEK_UPDATE
					;  known track in FDTRCK[drive]
;*			    if current track = sought track, return w/ no error
	MOV	BL,STK_DL[BP]
	XOR	BH,BH			; BX= drive #
	CMP	FDTRCK[BX],CH
	JE	SEEK_65 		; C cleared if equal
  ENDIF  ;NNNSEEK_UPDATE						     
;------------------------------------------------------------------------------
SEEK_35:
			; passed CH= track to seek (*2 if double-stepping),
			;  operation forced no matter what
			; returns C, ERRSTAT, DX non-0
	test	FDMOTS, 10000000b	; WARNING - incompat use of this bit!
	mov	al, 00b 				; fastest rate
  IFDEF  NNN_FAST_SEEK
	mov	bl, STK_DL[bp]
	mov	bh, 0
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
        MOV	AL,0
	MOV	CL,FDMED[BX]		; LOAD TYPE
	AND	CL,3			; MASK LOWER BITS
	CMP	CL,3			; CHECK IF 360 IN 360
	JNZ	SEEK_36
	MOV	AL,2			; SET FOR SLOW RATE
SEEK_36:
	OUT	DX,AL
  ELSE  ;NNN_FAST_SEEK
	jnz	SEEK_40
;*			    force slow transfer rate (affects step rate)
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
	mov	al, 10b 		;slow rate
	out	dx, al
	or	FDRATE, 10000000b	;not used but good for debug
SEEK_40:
    ENDIF  ;NNN_FAST_SEEK	
 

;*			    load commands to seek selected track
	MOV	AL,SEEKCMD
	CALL	WRTCMD			; load FDC w/ seek command
	JC	SEEK_65
	CALL	DRIHEDSEL		; load FDC w/ drive/head select
	JC	SEEK_65
	MOV	AL,CH
	MOV	BL,STK_DL[BP]
	XOR	BH,BH			; BX= index to arrays
	MOV	FDTRCK[BX],CH		; load current track #
	CALL	WRTCMD			; load FDC w/ selected track
	JC	SEEK_65

;------------------------------------------------------------------------------
SEEK_45: 		;seek/recal common code
;*			    wait for interrupt, get status
SEEK_50:
	CALL	WAITINT
	JNC	SEEK_55
	PUSH	WORD PTR ERRSTAT
	CALL	DSKRESET
	POP	WORD PTR ERRSTAT
	stc
	JMP	short SEEK_65 		;exit if timeout
SEEK_55:PUSH	CX
	CALL	DLY100			; DELAY 100 MICRO - SECONDS
	CALL	SISSTAT 		;check FDC status
	POP	CX
	JC	SEEK_65 		;exit if error
; These 4 lines useful when unterminated cables to C and D drives cause
; spurious "ready" line transitions.  This shouldn't happen w/ AT.
	MOV	AL,AH
	AND	AL,11b			; isolate drive select bits
	CMP	AL,STK_DL[BP]		; is it the drive in question?
	JNE	SEEK_50			; no, wait for the right int
	TEST	AH,ATERM+IC		; NORMAL TERMINATION ?
	JZ	SEEK_65			;  Yes, then operation is OK,
					;   return w/ C clear
	mov	ERRSTAT,FDSKERR 	;  No, force seek error,
SEEK_60:STC				;   return w/ C set
SEEK_65:RET
	
;
;------------------------------------------------------------------------------
DORECAL:
			; slow transfer rate
			; recalibrate
			; returns C, ERRSTAT if fail; else BX= drive, DRVSTAT
			;  & FDTRCK updated
;*			    load commands to recal
;------------------------------------------------------------------------------
;*    force slow transfer rate (affects step rate) for Mitsubitsi 360 drive

	MOV	AL,RECALCMD
	CALL	WRTCMD			; load FDC w/ recal command
	JC	SEEK_65
	CALL	DRIHEDSEL		; load FDC w/ drive/head select
	JC	SEEK_65
	CALL	SEEK_45			; common seek/recal code
	JNC	RECAL_05		; exit happy if no error
					; else not yet successful - see why
	CMP	ERRSTAT,FDSKERR 	; if it wasn't seek error,
	JNE	SEEK_60			;  another reason than recal pooped out
	AND	AH,IC+ATERM+EC		; else, AH valid,
	CMP	AH,ATERM+EC		; was it abnormal term because track
					;  0 not reached?
	JNE	SEEK_60			; no, some other reason
					; else recal pooped out - retry:
	MOV	ERRSTAT,NO_ERROR	; first restore ERRSTAT
	MOV	AL,RECALCMD
	CALL	WRTCMD			; load FDC w/ recal command
	JC	SEEK_65
	CALL	DRIHEDSEL		; load FDC w/ drive/head select
	JC	SEEK_65
	CALL	SEEK_45			; common seek/recal code
	JC	SEEK_65 		; exit if error
RECAL_05:
	TEST	AH,EC			; track 0 reached? (C still 0)
	JZ	RECAL_10 		; exit happy if so
	mov	ERRSTAT,FDSKERR 	; if not, report error
	STC
	RET				; exit reporting failure
RECAL_10:
	MOV	AL,STK_DL[BP]		; get drive #
	INC	AL			; AL is 1 for drive 0, 2 for drive 1
	OR	DRVSTAT,AL		; set "known track" bit, clear C
	MOV	BL,STK_DL[BP]
	XOR	BH,BH			; BX= index to arrays
	MOV	BYTE PTR FDTRCK[BX],0	; load current track #
	RET
DOSEEK	ENDP
;
;
;
;------------------------------------------------------------------------------
;	DPBADR	- Get address of disk parameter block into ES:DI
;
;	INPUT:	- nothing
;
;	OUTPUT	- ES:DI points to disk parameter block
;
;	REGISTERS USED: ES,DI
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         DPBADR,NON_GLOBAL
ELSE    ;NNN_ECR_25
        DPBADR   PROC    NEAR
ENDIF   ;NNN_ECR_25
;			get address of disk parameter block
;			returns:  ES:DI= address of DPB
	MOV	ES,CS:SEG0H			; ES=0
	ASSUME	ES:INTVEC
	LES	SI,ES:DPBPTR		; GET DISK PARAMETER BLOCK
	ASSUME	ES:NOTHING
	RET
DPBADR	ENDP
;
;
;------------------------------------------------------------------------------
;	DRIHEDSEL - Set drive head select
DRIHEDSEL:		;load drive/unit select param in FDC
	MOV	AL,STK_DH[BP]		; AL= passed DH (head select)
  IFDEF   NNN_FIXUP
        SHL	AL,2
  ELSE   ;NNN_FIXUP
	SHL	AL,1
	SHL	AL,1			; AL= head select * 4
   ENDIF ;NNN_FIXUP
	OR	AL,STK_DL[BP]		; AL= this param for FDC
	CLC
					;!! fall through to WRTCMD
	JMP	WRTCMD

;
;------------------------------------------------------------------------------
;	DSKMOT	- Turn motor drive on
;		
;	INPUT:	 DL contains drive number
;
;	OUTPUT:  FDTIMO= maximum unsigned motor timeout value,
;------------------------------------------------------------------------------
;     
IFDEF   NNN_ECR_25
        SUBROUT         DSKMOT,NON_GLOBAL
ELSE    ;NNN_ECR_25
        DSKMOT   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	CLI				; Disable interrupts
	MOV	CL,STK_DL[BP]		; CL = Drive #
	MOV	DH,1
	SHL	DH,CL			; DH = Motor on bit
	MOV	CH,FDMOTS		; Get motor status byte in CH

	MOV	AL,0FFh 		; Make a large motor timeout value
	TEST	CH,DH			; Motor already on?
	JZ	DSKMO2			;  No force wait
;
;		   --- MOTOR ON,FROM LAST OPERATION ---
;
;*			  FDTIMO = FF - MINWAIT
;*			  FDTIMO += (MINWAIT-time_motor_on), if that's > 0
; currently, FDTIMO = timeout_count+(MINWAIT-time_motor_on) 
;  if time_motor_on < MINWAIT, else FDTIMO <= timeout_count
	CALL	DPBADR
	MOV	AH, ES:2[SI]		; motor timeout count
	CMP	FDTIMO, AH		; if above, motor not on long enough - 
					;  FDTIMO += FF-MINWAIT-timeout_count
					; if below or =, force FDTIMO = FF-MINWAIT
	MOV	AL, 0FFh-MINWAIT
	JBE	DSKMO1
	ADD	AL, FDTIMO		; AL = old_FDTIMO+FF-MINWAIT
	SUB	AL, AH			; AL = old_FDTIMO+FF-MINWAIT-timeout_count
DSKMO1:
  IFDEF NNNMOTOR_ON
	MOV	FDTIMO,AL		; SAVE TIME COUNTER
	RET				; RETURN TO CALLER
  ENDIF ;NNNMOTOR_ON
DSKMO2:

	MOV	FDTIMO,AL
	MOV	CH, DH
	MOV	AL,CH			; Copy it to AL
   IFDEF NNN_FIXUP
	SHL	AL,4
	mov	dh,stk_dl[bp]
	SHL	DH,4

   ELSE ;NNN_FIXUP
	MOV	CL,4
	SHL	AL,CL			; Convert to motor-on bit
	mov	dh,stk_dl[bp]
	SHL	DH,CL
   ENDIF  ;NNN_FIXUP
	OR	CH,DH			; Set selected bit
	OR	AL,0CH			; Include Reset & DMA mode bits
	OR	AL,STK_DL[BP]			; Or in drive #
	PUSH	DX
	MOV	DX,FDCDOR		; Output to FDC Digital Output register
	OUT	DX,AL			;   to turn on diskette motor
	POP	DX
	MOV	FDMOTS,CH		;   and place back status
DSKM05:	STI
	RET
;		     

DSKMOT	ENDP
;
;------------------------------------------------------------------------------
;	DSKRESET  -  Reset diskette system
; 
;	INPUT:	DL = drive number
;
;	OUTPUT: Carry set if error
;		AH = ERRSTAT = error code if error , else 0
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         DSKRESET,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  DSKRESET
        DSKRESET   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	PUSH	CX
;
;------------------------ DISABLE INTS ----------------------------------------
;
	CLI
	XOR	AL,AL
;
;--------------------- CLEAR DRVSTAT AND ERRSTAT ------------------------------
;
	MOV	DRVSTAT,AL		; Clear recalibration bits for drives
	MOV	ERRSTAT,AL		; Clear ERRSTAT
;
;---------------- GET MOTOR STATUS FOR DISKETTE CONTROLLER RESET --------------;
;
	MOV	AL,FDMOTS		; Get motor status
;	MOV	AH,AL			; Save motor status for drive select test
   IFDEF  NNN_FIXUP
	ROL	AL,4
   ELSE  ;NNN_FIXUP
	MOV	CL,4			; Shift to upper nibble
	ROL	AL,CL			; AL = Select bits for drive 2
   ENDIF  ;NNN_FIXUP
;
	AND	AL,39H			; Mask off all reserved bits
	or	al, 1000b		; Don't let INT/DMA line go off and on;
					;  it can cause IRQ glitch.  stan 4/89
;
;-------------------------- ISSUE RESET COMMAND -------------------------------
;
	MOV	DX,FDCDOR		; Set address of digital output reg
	OUT	DX,AL			; Issue reset command
;
;------------------------ HOLD RESET FOR 3.5 us --------------------------------
;
	MOV	CX,CS:FCLK10		; for 5 usecs -- CLK10/2*8
   IFDEF  NNN_FIXUP
	SHL	CX,3
    ELSE  ;NNN_FIXUP
	SHL	CX,1			; CX=FCLK10*8
	SHL	CX,1			; Delay for at least 5 usecs.
	SHL	CX,1
   ENDIF ;NNN_FIXUP
DSKR_05:				; Count = CLK10/ 3 * ( # cycles in LOOP)
	LOOP	DSKR_05			; Delay for at least 3.3 microseconds
;
	OR	AL,1100b		; Turn off FDC reset
	OUT	DX,AL			;  
	STI				; Enable ints
;
;		  ---- GET RESET RESULT INTERRUPT ----
;
	CALL	WAITINT 		; Wait for interrupt 
;
;			---- CHECK RESULT BYTES ----
;
	CALL	FDC_RDY
	MOV	CX,4			; Expect 4 status bytes
DSKR_10:PUSH	CX
	CALL	SISSTAT
	POP	CX
	LOOP	DSKR_10			; Read all 4
	
;
DSKR_20:
IFDEF	NNNINT077
;
;if 82077 is installed then enable FIFO and set threshold
;
	CALL	IS_I077_INSTD
	TEST	WORD PTR STK_T1[BP],1  ;NO 82077 installed ?
	JZ	DSKR_30
	MOV	AH,CNFG_2
	CALL	CONFIGURE
	JC	DSKR_40
DSKR_30:
ENDIF	;NNNINT077
	MOV	AL,SPFYCMD		; Issue specify command
	CALL	WRTCMD			;   to FDC
	JC	DSKR_40			; If error , exit with carry
	XOR	BX,BX			; Set offset from Disk Para Block
	CALL	MLTCMD2 		; Write 2 commands
;*			    clear flag saying don't need to slow seeks
	AND	FDMOTS, NOT 10000000B	; WARNING - incompat use of this bit!
;*			    if step rate is plenty slow,
	cmp	byte ptr es:[si], 0AFh	; if this or slower,
	jnbe	DSKR_35			; rate OK
;*				set flag saying don't need to slow seeks
	or	FDMOTS, 10000000b	; WARNING - incompat use of this bit!
DSKR_35:XOR	AH,AH
DSKR_40:POP	CX
	OR	AH,AH			; Set zero flag on AH
	RET
DSKR_45:mov	ERRSTAT,FDCTERR 	; Set error status
	MOV	AH,ERRSTAT
	JMP	DSKR_40
DSKRESET	ENDP



;------------------------------------------------------------------------------
HEDSETL proc	near	;conditional wait for head settle
;	INPUT:	DX non-0 if head settle wait needed
;	OUTPUT: ES:DI pts to DPB
;------------------------------------------------------------------------------
;
;	    --- CHECK IF HEAD SETTLE WAIT NEEDED ---
;
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	OR	DX,DX			; If head settle wait complete
	JZ	HD_S_20			;   Go on
	MOV	AL,ES:[SI+9]		; GET HEAD SETTLE TIME
	OR	AL,AL			; Zero Settle time ?
	JNZ	HD_S_5			;   No, then leave alone
	MOV	AL,HSTELS		;   Yes, set head settle time to 20 ms
HD_S_5:	MOV	AH,10			; Multiply by 10
	MUL	AH
	MOV	CX,AX 
HD_S_10:CALL	DLY100			; Delay 100 us per loop
	LOOP	HD_S_10			; wait for head to settle
HD_S_20:ret

HEDSETL endp


;------------------------------------------------------------------------------
SPINUP	proc	near	;conditional wait for disk spin-up
;	INPUT:	STK_AH[BP] bit 7 = motor wait needed, ES:SI pts to DPB
;	OUTPUT: 
;------------------------------------------------------------------------------
	CALL	DPBADR
	MOV	AH,ES:[SI+0AH]
	TEST	BYTE PTR STK_AH[BP],1B
	MOV	AL,MWAIT_R
	JZ	SPU_05
	MOV	AL,MWAIT_W
SPU_05:	CMP	AH,AL
	JAE	SPU_10
	MOV	AH,AL
SPU_10:
					; Convert wait time in 1/8 sec to timer
;					; Ticks
	MOV	AL,AH			;
   IFDEF  NNN_FIXUP
	SHR	AL,3
    ELSE   ;NNN_FIXUP
	SHR	AL,1			; Multiply by 2.25 to convert to timer 
	SHR	AL,1			; ticks
	SHL	AH,1
    ENDIF ;NNN_FIXUP
	ADD	AH,AL
	MOV	AL,0FFH 		; Check if wait ok
	SUB	AL,AH
	CMP	AL,FDTIMO
	JAE	SPU_40
;
;		  --- NOTIFY OS OF WAIT ---
;
	PUSH	AX
	MOV	AX,90h*256+0FDh 	; Tell operating system about wait
	INT	15h
	POP	AX
	JC	SPU_40			; If carry set, then motor is up to
					;   speed, else we have to wait for it
;
;*			    get wait time in usec
;*			     (multiply count in AL by 1000000/8)
	PUSH	AX
	CALL	DPBADR			; Get pointer to DPB
	MOV	AL,ES:[SI+0AH]		; Get motor wait time from DPB
	MOV	AH,125
	MUL	AH
   IFDEF NNNFAST_SPINUP
	MOV	DX,100			; CAUSES ABOUT 75 M-SEC DELAY
   ELSE ;NNNFAST_SPINUP
	MOV	DX,1000			; CAUSES ABOUT 1 SECOND DELAY
   ENDIF ;NNNFAST_SPINUP
	
	MUL	DX			; DX.AX = wait time in usec
	MOV	CX,DX
	MOV	DX,AX			; CX.DX = wait time in usec

	MOV	AH,86h
	INT	15h			; OS WAIT
;*			    if wait happened, skip our own waiting
	POP	AX
	JNC	SPU_40
	MOV	CX,5000 		; wait at worst 500 msec 
SPU_30:	CALL	DLY100
	CMP	AL,FDTIMO		; If MINWAIT has elapsed
	JAE	SPU_40
	LOOP	SPU_30
;
SPU_40:	MOV	FDTIMO,AL	; MAKE MOTOR TIMEOUT AS LARGE AS POSSIBLE
	RET

SPINUP	endp


;------------------------------------------------------------------------------
;	FDC_RDY - Wait for floppy disk controller ready
;
;	INPUT:	nothing
;
;	OUTPUT: carry set if timeout
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FDC_RDY,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  FDC_RDY
        FDC_RDY   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	PUSH	DX			; Save DX
	PUSH	CX			; Save CX
	MOV	DX,FDC_REGR		; Set address to FDC main status reg
;
;--------------- Guarantee 12 us between command and ready check -----------
;
	XOR	CH,CH			; Zero CH for loop
	MOV	CL,DLY_CNT		; Get count for 100 us
  IFDEF  NNN_FIXUP
	SHR	CL,3
  ELSE   ;NNN_FIXUP
	SHR	CL,1			; 100/2 = 50
	SHR	CL,1			; 50/2 = 25
	SHR	CL,1			; 25/2 = 12.5		     
   ENDIF  ;NNN_FIXUP
	ADD	CL,2			; Be safe
FDC_05:	LOOP	FDC_05			; Kill 12 us
;
;----------------------- Check status for ready ------------------------
;
	XOR	CX,CX			; Set outside loop to max
FDC_10:
       WAFORIO
	IN	AL,DX			; Read FDC status
	TEST	AL,FDC_DONE		; Command terminated, is FDC ready ?
	LOOPZ	FDC_10			;   No, spin
	JNZ	FDC_20			;   Yes, and carry is clear
	mov	ERRSTAT,FDSKTMO 	; SET TIMEOUT ERROR
	STC				; Indicate error
FDC_20:	POP	CX			; Restore regs
	POP	DX
	RET

FDC_RDY ENDP
;
;------------------------------------------------------------------------------
;	FDTYPIF - Get floppy disk type from CMOS if CMOS OK
;			Passed STK_DL[BP] = drive #
;			Returns C set if CMOS no good, else
;				AL= drive type, Z= 1 if no drive or type 
;				unknown, top of AH non-0 if another drive
;-----------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         FDTYPIF,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  FDTYPIF
        FDTYPIF   PROC    NEAR
ENDIF   ;NNN_ECR_25


IF	CMOSCFG				; (vtb 6.3.90) skip cmos access

	MOV	AL,CMSTAT		; CMOS Diagnostic status
	CALL	CMREAD			; Read it from the CMOS memory
	STI
       IFDEF	NNNEA89062302
; We want to set CY when either of the two high bits are set.  All values
; greater than or equal to 40h indicate error, all those less than 40h are okay.
; The CMC instruction will reverse the result of the CMP to give us CY == error.
	CMP	AL,40h			; CMOS chip lost power or bad csum?
       ELSE	;NNNEA89062302
	CMP	AL,11000000b		; CMOS chip lost power or bad csum?
       ENDIF	;NNNEA89062302
	CMC
	JC	FDTY_10 		; yes
ENDIF	;CMOSCFG
       IFNDEF	NNNEA89062302
					; else fall through to FDTYPE
FDTYPIF ENDP
;------------------------------------------------------------------------------
IFDEF   NNN_ECR_25
        SUBROUT         FDTYPE,NON_GLOBAL
ELSE    ;NNN_ECR_25
        FDTYPE   PROC    NEAR
ENDIF   ;NNN_ECR_25
;			Passed STK_DL[BP] = drive #
;			Returns AL= drive type, Z= 1 if no drive or type 
;				unknown, top of AH non-0 if another drive
;------------------------------------------------------------------------------
       ENDIF    ;NNNEA89062302
IF	CMOSCFG				; (vtb 6.3.90) skip cmos access
	MOV	AL,CMFDSK		; CMOS Floppy disk types, Drive A upper
					;   nibble, Drive B lower nibble
	CALL	CMREAD			; Read it from the CMOS memory
	STI

ELSE
	IF	DIAG
	call	wrtinl
	DB	'Fake CMOS read',CR,LF,0
	ENDIF
	mov	al,hdcflg
	or	al,al
	jz	startvalue
	IF	DIAG
	call	wrtinl
	DB	'Getting current parameters',CR,LF,0
	ENDIF
; Drives have already been determined by dsktest. So rely on data
; in hdcflg to make cmos look correct and consistent with drives.
	mov	al,hdcflg
	cmp	BYTE PTR STK_DL[BP],0	; Is it drive 0 ?
	je	fdtype_0
	shr	al,1
	shr	al,1
	shr	al,1
	shr	al,1
	and	al,0fh
fdtype_0:
;	test	al,04h		; Is there a drive at all?
;	jz	no_drive
	and	al,01h		; is change line supported
	jnz	no360k		; not a 360k drive, reckon its 1.44 M
	IF	INFO
	call	wrtinl
	DB	'Pretend 360K drive',CR,LF,0
	ENDIF
	mov	ax,1h
	jmp	done_cmos
no360k:
	mov	ax,4h
	jmp	done_cmos
no_drive:
startvalue:
	mov	ax,4h		; let them try 1.44 M
done_cmos:
	or	al,al
	sti
	ret
ENDIF	;CMOSCFG

       IFDEF	NNNEA89062302
FDTYPIF ENDP
       ELSE	;NNNEA89062302
FDTYPE	ENDP
       ENDIF	;NNNEA89062302
;
;
;------------------------------------------------------------------------------
;     GETDINF - Get drive info from HDCFLG
;		Input:	drive info in STK_DL[BP] half of 40:8F
;		Output: low 3 bits of AL = drive info
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         GETDINF,NON_GLOBAL
ELSE    ;NNN_ECR_25
        GETDINF   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	MOV	AL,HDCFLG		; Get HDCFLG
	CMP	BYTE PTR STK_DL[BP],0
	JE	GDINEIF1		; If drive 1 
   IFDEF  NNN_FIXUP
	SHR	AL,4
   ELSE  ;NNN_FIXUP
	PUSH	CX   
	MOV	CL,4			; Shift upper nibble down to lower
	SHR	AL,CL
	POP	CX
   ENDIF  ;NNN_FIXUP
GDINEIF1:
	AND	AL,1111b		; Return info for drive requested only
	RET
GETDINF ENDP
;
;------------------------------------------------------------------------------ 
;	GET_TYPE  -  Get pointer to BIOS drive parameter table 
;
;	INPUT:	  -  STK_DL[BP] = drive number
;
;	OUTPUT:   -  DI contains pointer to drive info table as determined by
;		     FDMED,HDCFLG, and CMOS.
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         GET_TYPE,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  GET_TYPE
        GET_TYPE   PROC    NEAR
ENDIF   ;NNN_ECR_25
;*			    get drive #
	xor	bh, bh
	mov	bl, STK_DL[bp]
;*			    if drive type known,
	call	FDTYPIF
	jc	G_TYP_30
	jz	G_TYP_30
	cmp	al, 4
	ja	G_TYP_30
;*				get index to max dimensions for drive
	je	G_TYP_05		; xlat drive type 4 to table index 5
	dec	al
	jz	G_TYP_10		; xlat drive type 1 to table index 0
G_TYP_05:
	inc	al			; xlat 2 to 2 and 3 to 3
G_TYP_10:
       if	BESURE08                ;maintain flag indicating probable 
	        			; 1.44 drive to be checked by func 08
        
;*				if we say drive type 1 but drive is 80-track,
	cmp	al, 0
	jne	G_TYP_20
	push	ax
	call	GETDINF
	and	al, 0101b
	cmp	al, 0101b
	pop	ax
	jne	G_TYP_20
       IFDEF	NNNEA89061412
; If drive is 80 track, it could be 720k, 1.2M or 1.44M, but we arbitrarily 
; guess 1.2M because these are AT compatible machines.
	mov	al, 2		    ; force to 1.2M drive
       ELSE	;NNNEA89061412
;*				    force to 1.44M drive
	mov	al, 5
       ENDIF	;NNNEA89061412
;*				 endif
G_TYP_20:
;;*				if drive type 2 or 3 but we think it's 1.44M,
	push	ax
	call	GETDINF
	test	al, 1000b
	pop	ax
	jz	G_TYP_25
;*				    force to 1.44M drive
	mov	al, 5
;*				 endif
G_TYP_25:
       endif	;BESURE08


   ;*				go return dimensions
	jmp	short G_TYP_40		; translate index to offset
;*			     endif
G_TYP_30:
;*			    if medium known,
	mov	ah, FDMED[bx]
	test	ah, 00010000b		; If media known
	jz	G_TYP_35
;*				if medium rate, return drive 2 dimensions
	test	ah, 01000000b
	mov	al, 2
	jnz	G_TYP_40
       IFDEF	NNNEA89061412
; If drive is 80 track, it could be 720k, 1.2M or 1.44M, but we arbitrarily 
; guess 1.2M because these are AT compatible machines.
	mov	al, 2		    ; force to 1.2M drive
       ELSE	;NNNEA89061412
;*				if low rate, return drive 4 dimensions
;*				if high rate, return drive 4 dimensions
	mov	al, 5
       ENDIF	;NNNEA89061412
	jmp	short G_TYP_40
;*			     endif
G_TYP_35:
;*			    return flag saying not found
	xor	di, di
	jmp	short G_TYP_45
;	 --- GET POINTER INDICATED BY AL INTO DI ---
G_TYP_40:
   IFDEF NNNEN88121403
	PUSH	AX			; SAVE REGSITER
	MOV	DI,OFFSET CS:PAMSMED	; Get base pointer to media tables
	MOV	AH,BYTE PTR PAMSIZ
	MUL	AH			; Get into table
	ADD	DI,AX
	POP	AX			; RESTORE AX
   ELSE  ;NNNEN88121403
	MOV	DI,OFFSET CS:PAMSMED	; Get base pointer to media tables
	MOV	AH,BYTE PTR PAMSIZ
	MUL	AH			; Get into table
	ADD	DI,AX
   ENDIF ;NNNEN88121403	
	RET
G_TYP_45:
	RET
GET_TYPE  ENDP
;
;------------------------------------------------------------------------------
;	INIT_DMA  -  Initialize DMA controller for floppy operation
;
;	INPUT:	  -  
;
;	OUTPUT:   - carry set if error, ERRSTAT set
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         INIT_DMA,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  INIT_DMA
        INIT_DMA   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
IFDEF	NNNINT077
;
; if INTEL 82077 is installed AND FIFO is enabled then DMA channel should 
; be programmed to operate in DEMAND mode.
; 
	TEST	WORD PTR STK_T1[BP],2	;82077 installed and FIFO enabled
	JZ	NO_FIFO
;
;if verify function then leave DMA alone.
;
	CMP	byte ptr STK_AH[BP],4
	JNE	ST_DMA
	JMP	DMA_30
ST_DMA:
;------------------------- DISABLE INTS -------------------------------------
;
	CLI				; Disable ints while initializing 
;
;-------------------- CLEAR DMA FIRST LAST FLIP FLOP ----------------------
;
	OUT	CLRBP,AL		; Clear DMA first last flip flop
;
; set DMA mode
;
	MOV	AH,STK_AH[BP]		; Get DSR func code from stack 
	SHR	AH,1			; is it odd (03=write, 05=format)?
	MOV	AL,0Ah			; DMA mode for write or format
	JC	INITDMA1 		; yes - go ahead
	SHR	AH,1			; is it function 02 (read)?
	MOV	AL,06h		; DMA mode for read
	JC	INITDMA1 		; yes - go ahead
	JMP	DMA_30
INITDMA1:
	MOV	DX,MODE03		; 
	OUT	DX,AL			; Write mode command
;
	JMP	SET_ADR
NO_FIFO:				;FIFO is disabled or NO 82077
ENDIF	;NNNINT077
;------------------------- DISABLE INTS -------------------------------------
;
	CLI				; Disable ints while initializing 
;
;-------------------- CLEAR DMA FIRST LAST FLIP FLOP ----------------------
;
	OUT	CLRBP,AL		; Clear DMA first last flip flop
;
;-------------------------- SET DMA MODE -------------------------------
;
	MOV	AH,STK_AH[BP]		; Get DSR func code from stack 
	SHR	AH,1			; is it odd (03=write, 05=format)?
	MOV	AL,4Ah			; DMA mode for write or format
	JC	DMA_20			; yes - go ahead
	SHR	AH,1			; is it function 02 (read)?
	MOV	AL,46h			; DMA mode for read
	JC	DMA_20	 		; yes - go ahead
	MOV	AL,42h			; DMA mode for verify
DMA_20:	MOV	DX,MODE03		; 
	OUT	DX,AL			; Write mode command
IFDEF	NNNINT077
;
SET_ADR:
ENDIF	;NNNINT077

;
;----------------- CONVERT ES:BX PAIR TO 20 BIT ADDRESS ---------------------
;
	MOV	DX,STK_ES[BP]		; Get buffer segment from stack
;
	MOV	CL,4			; Set rotate count to convert 
	ROL	DX,CL			;  segment into absolute
	MOV	CH,DL			; Put copy in CH
	AND	CH,00Fh 		; Get 64 k section in CH
	AND	DL,0F0h 		; CH.DX = 16 * passed ES
	ADD	DX,STK_BL[BP]		; DX = Bits 0-15 of transfer address
	ADC	CH,0			; Add in carry (if any)
;					; CH.DX = total 20-bit address
	MOV	BX,DX			; Put copy of starting address in BX
 
;
;------------------------ CALCULATE BYTE COUNT ---------------------------
;
; Should this calculation check the function so that the DMA calculation is 
; correct ??????
;
;!! Warning - this calculation is for read and writes.	The DMA byte count it
; generates for format operations is way too much - a minimum of 512 bytes for
; 512-byte sectors, enough for 128 sectors.  However, it doesn't hurt to have
; too large a DMA count for writes and formats.  Boundary errors returned from
; verifys and most formats are not really errors but are returned for compatibility.
;------------------------------------------------------------------------------
;
	MOV	AL,STK_AL[BP]		; AL= of sectors
	XOR	AH,AH			; CLEAR TOP BYTE
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	MOV	CL,ES:[SI+3]		; Get the value used to compute
					;   Bytes/sector = 2^([SI+3]) * 128
					;	maximum value = 8
	SHL	AX,CL			; AX = (bytes to read)/128
	CMP	AX,512			; If > 64k/128,
	JA	DMA_35			;  product > 64k - go report error
	MOV	CL,7
	SHL	AX,CL			; Multiply by 128
	DEC	AX			; adjust count for DMA chip
	ADD	DX,AX			; Check if crossing 64K boundary
	JC	DMA_35
	MOV	DX,CLRBP		; Clear 1st/last Flip Flop
	OUT	DX,AL
;
;--------------------------- SET BYTE COUNT -------------------------------------
;
	MOV	DX,CH2CNT		; Get address of DMA byte count to DX
	WAFORIO 			; Delay for chip I/O recovery time
	OUT	DX,AL			; Set DMA byte count, low byte
	XCHG	AL,AH			; Get high byte of length
	WAFORIO 			; Delay for chip I/O recovery time
	OUT	DX,AL			; Set DMA byte count, high byte
;		     
;-------------------------- SET ADDRESS BITS 0 - F -----------------------------
;
	MOV	DX,CH2ADR		; Get address of DMA address reg to DX
	XCHG	AX,BX			; Get address in BX to AX for output
	WAFORIO 			; Delay for chip I/O recovery time
	OUT	DX,AL			; Set address, low byte
	XCHG	AL,AH			; Get bits 8 - F to AL for output
	WAFORIO 			; Delay for chip I/O recovery time
	OUT	DX,AL			; Set address, high byte
	STI				; Enable ints now that the DMAC is set
;			
;------------------------ SET ADDRESS BITS 10 - 13 ----------------------------
;
	XCHG	AL,CH			; Get 64 k section address
	MOV	DX,CH2PG		; Get address of DMA page reg
	OUT	DX,AL			; Set page address bits
;
;------------------------ SET DMA REQUEST FOR FLOPPY --------------------------
;
	MOV	DX,MASK03		; Get adderss of DMA request reg to DX
	MOV	AL,DMARST+DMACH2	; Set channel 2 Select
	WAFORIOIF			; Delay for chip I/O recovery time
	OUT	DX,AL			; Issue request for channel 2
	CLC				; Clear error indicator
;
;------------------------- INIT_DMA EXIT POINT --------------------------------
;
DMA_30:	STI
	RET
;
;------------------------- DMA INIT ERROR HANDLER -----------------------------
;
DMA_35:	MOV	ERRSTAT,ERR09		; SET DMA BOUNDARY ERROR
	STC				; SET CARRY TO INDICATE ERROR	*A
	JMP	DMA_30

INIT_DMA	ENDP
;
;
;------------------------------------------------------------------------------
;	MLTCMD - Write multiple commands to FDC
;
;	INPUT:	BX = starting table offset
;		CX = number of bytes to write (MLTCMD entry)
;	OUTPUT: ES:SI = Disk Parameter Block
;		ERRSTAT bit 7 = 1 on timeout
;		ERRSTAT bit 5 = 1 on controller error
;		Carry Flag set on timeout or controller error, else not set
;		CX = 2 (if MLTCMD2 entry)
;------------------------------------------------------------------------------
IFDEF   NNN_ECR_25
        SUBROUT         MLTCMD2,NON_GLOBAL
ELSE    ;NNN_ECR_25
        MLTCMD2   PROC    NEAR
ENDIF   ;NNN_ECR_25
	MOV	CX,2			; Output 2 commands
MLTCMD:
	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
MULT5:
	MOV	AL,ES:[SI+BX]		; GET CMD DATA
	CALL	WRTCMDA 		; WRITE IT
	JC	MULTRTN 		; JUMP IF ERROR 
	INC	BX			; POINT TO NEXT BYTE
	LOOP	MULT5			; REPEAT UNTIL DONE
	CLC				; BE SURE TO RETURN NO ERROR
MULTRTN:
	RET
MLTCMD2 ENDP


;------------------------------------------------------------------------------
;	RESTYP	-	;force FDMED[BX] known & restore compatible low bits
			; and clear FDOP[BX]
			;entry - AH = ONLY top 3 bits of FDMED[BX]
;------------------------------------------------------------------------------
IFDEF   NNN_ECR_25
        SUBROUT         RESTYP,NON_GLOBAL
ELSE    ;NNN_ECR_25
        RESTYP   PROC    NEAR
ENDIF   ;NNN_ECR_25
;*			    force known, low bits = 3, 4, or 5
	add	ah, 00010011b
	test	ah, 10000000b
	jnz	RTYPOK1
	inc	ah
	test	ah, 01000000b
	jnz	RTYPOK1
	inc	ah
RTYPOK1:
	MOV	FDMED[BX],AH
;*			    if drive type > 2,
	CALL	FDTYPIF
	JC	RTYPOK2 		;if CMOS invalid
	CMP	AL,2
	JNA	RTYPOK2 		;if no drive or type 1 or 2
;*				force low bits of medium to 111
	OR	FDMED[BX],00000111b
;*			     endif
RTYPOK2:
	RET
RESTYP	ENDP


;------------------------------------------------------------------------------
;	
;	RSLTRD - Read result bytes from FDC
;
;	INPUT:	DS = Rom data area segment
;		CX = NUMBER OF RESULT BYTES TO READ
;
;	OUTPUT: AL = FDC status
;		AH = FDC ST0, only valid if no carry
;		ERRSTAT bit 7 = 1, C set on timeout
;		ERRSTAT bit 5 = 1, C set if can't flush FDC results
;		C set if less results available than expected
;		DSKST = FDC ST0, valid only if no carry
;		DX non-zero
;
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         RSLTRD7,NON_GLOBAL
ELSE    ;NNN_ECR_25
        RSLTRD7   PROC    NEAR
ENDIF   ;NNN_ECR_25
      ASSUME DS:ROMDAT
	MOV	CX,7
RSLTRD: ;	PROC	NEAR
;
      ASSUME DS:ROMDAT
;
	MOV	BX,OFFSET DSKST 	; Set address of status buffer
RSLTR0:	CALL	FDC_RDY 		; Check for ready
	JC	RSLTEXIT		; If ready timeout, exit 
	AND	AL,CB+DIO		; Check for CB and DIO ?? 
	CMP	AL,CB+DIO		; Both should be set.
	JNE	RSLTERR 		; If not, we're done reading results
	MOV	DX,FDC_DAT		; Set address of FDC data reg to DX
	IN	AL,DX			; Read result
	MOV	[BX],AL 		; Store result
	INC	BX			; Bump pointer 
	LOOP	RSLTR0			; Loop until all bytes are read
	CLC				; Clear error indicator
	JMP	SHORT RSLTEXIT		; Skip over error handler
;
;---------------------- RESULT READ ERROR HANDLER -----------------------------
;
RSLTERR:
	mov	ERRSTAT,FDCTERR 	; Set controller error code
	STC				; Set carry for error
RSLTEXIT:
	MOV	AH,DSKST		; Return FDC ST0 in AH,
	RET
;
RSLTRD7 ENDP

;
;------------------------------------------------------------------------------
;	SDRVST	- Sense drive status
;
;	INPUT:
;
;	OUTPUT: returns C=1, ERRSTAT if error; else C=0 & AH= FDC's ST0,
;		 Z=0 if track 0 reached
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         SDRVST,NON_GLOBAL
ELSE    ;NNN_ECR_25
        SDRVST   PROC    NEAR
ENDIF   ;NNN_ECR_25
		PUSH	AX			; Save regs
		PUSH	CX
		MOV	AL,SNSDRV		; Get sense drive status command
		CALL	WRTCMD			; Write command to FDC
		JC	SDSEXIT 		; If error writing command, exit
		CALL	DRIHEDSEL		; Write drive select
		MOV	CX,1			; # of bytes to read for status
		CALL	SISST0			; Read status of FDC op
		TEST	AH,10h			; Is the track 0 flag set ?
SDSEXIT:	POP	CX
		POP	AX
		RET
;
SDRVST	ENDP
;  IFDEF  NNNMACRO
;  ELSE  ;NNNMACRO

;
;------------------------------------------------------------------------------
;	SET_CARY  -  Set carry flag
;
;	INPUT: nothing
; 
;	OUTPUT: Carry flag bit set in stack location
;
;------------------------------------------------------------------------------
;    
IFDEF   NNN_ECR_25
        SUBROUT         SET_CARY,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  SET_CARY
        SET_CARY   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	OR	WORD PTR STK_FL[BP],01H ; Set carry flag bit
	RET
;
SET_CARY	ENDP
;  ENDIF  ;NNNMACRO
;
;
;------------------------------------------------------------------------------
;	SETRAT	-  Set transfer rate
;
;	INPUT:	BX = drive #
;
;	OUTPUT: rate control port updated
;______________________________________________________________________________
;
IFDEF   NNN_ECR_25
        SUBROUT         SETRAT,NON_GLOBAL
ELSE    ;NNN_ECR_25
        SETRAT   PROC    NEAR
ENDIF   ;NNN_ECR_25

;
	PUSH	AX
	MOV	AL,FDMED[BX]
	AND	AL,0C0h 		; Extract rate bits from status byte
;

									
;
	AND	FDRATE,03FH		; Clear rate bits
	OR	FDRATE,AL		; Set new rate
	ROL	AL,1			;   and shift to lower two bits
	ROL	AL,1
	MOV	DX,FDRTCP		; AT Floppy disk rate control port	
	OUT	DX,AL			; Output new disk rate
;
	CLC
	POP	AX
	RET
SETRAT	ENDP

;
;------------------------------------------------------------------------------
;	SISSTAT - Sense interrupt status
;
;	INPUT: nothing
;
;	OUTPUT: If error - carry set, ERRSTAT set , AH = FDC stat
;		Else - carry clear AL = present cylinder
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         SISSTAT,NON_GLOBAL
ELSE    ;NNN_ECR_25
        SISSTAT   PROC    NEAR
ENDIF   ;NNN_ECR_25
;		  

	MOV	AL,SISCMD		; Set sense interrupt status command
	CALL	WRTCMD			; Issue command
	JC	SISEXIT 		; Jump if error (can't write command);
	MOV	CX,2			; # of bytes to read for status
SISST0:	MOV	SI,OFFSET DSKST
	PUSH	[SI]			; Save top 2 bytes from FDC status list
	PUSH	BX			; RSLTRD corrupts BX
	CALL	RSLTRD			; READ RESULT	    
	POP	BX
	JC	SISERR			; JUMP IF ERROR (CAN'T READ RESULTS)
	MOV	AX,[SI] 		; GET RETURNED STATUS BYTES (2)
	XCHG	AH,AL
					; Say no error with carry flag cleared
SISERR:	POP	[SI]			; Return original FDC status bytes
SISEXIT:RET

SISSTAT 	ENDP
;
;------------------------------------------------------------------------------
IFDEF   NNN_ECR_25
        SUBROUT         STODINF,NON_GLOBAL
ELSE    ;NNN_ECR_25
        STODINF   PROC    NEAR
ENDIF   ;NNN_ECR_25
;			Input:	BX = drive #, AL = drive info
;			Output: drive info in [BX] half of 40:8F
;				CL = ?
	MOV	CL,11110000b
	OR	BL,BL
	JZ	SDIEIF1
	MOV	CL,4
	SHL	AL,CL
	MOV	CL,00001111b
SDIEIF1:
	AND	HDCFLG,CL
	OR	HDCFLG,AL
	RET
STODINF ENDP

;
;------------------------------------------------------------------------------
;	WAITINT - Wait for FDC interrupt	
;
;	OUTPUT: Carry Flag set if timeout error, else not
;		ERRSTAT bit 7 = 1 if timeout
;------------------------------------------------------------------------------
;
IFDEF   NNN_ECR_25
        SUBROUT         WAITINT,NON_GLOBAL
ELSE    ;NNN_ECR_25
        WAITINT   PROC    NEAR
ENDIF   ;NNN_ECR_25
      ASSUME DS:ROMDAT
	PUSH	AX
	PUSH	CX
	PUSH	DX
	CLI
	IN	AL,PIC0MSK		; Read current mask
	WAFORIO
	AND	AL,0BFH 		; Enable floppy interrupt
	OUT	PIC0MSK,AL
	MOV	AX,90h*256+01h		; Suspend task
	INT	15h
	STI
;
IFDEF	NNNINT077
	;some controllers that have FIFO (82077) require longer delay
	MOV	CX,26000		; count for 2.6 s delay
ELSE	;NNNINT077
	MOV	CX,13000		; Count for 1.3 s delay
ENDIF	;NNNINT077
   IFDEF  NNN_PASS_WAIT
	JMP	SHORT WAITLI1		; LOOP PAST WAIT FIRST
WAITLI:	CALL	DLY100			; Delay 100 us per loop
WAITLI1:TEST	DRVSTAT,80H ;BIT7	; Did disk interrupt occur ?
   ELSE ;NNN_PASS_WAIT

WAITLI:	CALL	DLY100			; Delay 100 us per loop
	TEST	DRVSTAT,80H ;BIT7	; Did disk interrupt occur ?
   ENDIF ;NNN_PASS_WAIT
	LOOPZ	WAITLI
	JNZ	WAIT2			;   Yes
WAIT1:	MOV	ERRSTAT,FDSKTMO 	; error code (overwrite any DSKRES error)
	STC				; OPERATION TIMED OUT
	JMP	short WAITEX
WAIT2:					; interrupt occurred
	AND	DRVSTAT,07FH		; RESET INTERRUPT OCCURRED BIT and clear
	CLC
WAITEX:	POP	DX
	POP	CX
	POP	AX
	RET
WAITINT ENDP

;
;------------------------------------------------------------------------------
;	WRTCMD - Write command to FDC
;
;	INPUT: AL = command
;	       DS = 40H
;
;	OUTPUT: AH,DX trashed, carry and ERRSTAT set if error
;------------------------------------------------------------------------------
WRTCMDA:
	JC	WRTERR
;
IFDEF   NNN_ECR_25
        SUBROUT         WRTCMD,NON_GLOBAL
ELSE    ;NNN_ECR_25
        WRTCMD   PROC    NEAR
ENDIF   ;NNN_ECR_25
	MOV	AH,AL			; Save command
	CALL	FDC_RDY 		; Check for FDC ready
	JC	WRTERR			; If ready timeout, exit
	TEST	AL,DIO			; Ready for command? (also clears C)
	JNE	WRT15			; If not ready, exit via error
	MOV	DX,FDC_DAT		; point to command/result port
	MOV	AL,AH			; Get command byte
	OUT	DX,AL			; Issue command
	RET				; Clean exit
;
;-------------------------- WRTCMD ERROR EXIT ---------------------------------
;
WRT15:
	PUSH	BX
	CALL	RSLTRD7 		; Empty the 7 result bytes
	POP	BX
	mov	ERRSTAT,FDCTERR 	; Say it's a controller failure
	STC				; SET THE CARRY FLAG
WRTERR:	RET
WRTCMD	ENDP
;

;
;******************************************************************************
;*	FD_ISR	-  Interrupt 0EH service routine - FDC interrupt	      *
;******************************************************************************
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_ISR,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  FD_ISR
        FD_ISR   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	PUSH	AX			; Save regs
	PUSH	DX
	PUSH	DS
	MOV	DS,SEG40H		; Set DS to the ROM data segment
      ASSUME DS:ROMDAT
	MOV	AL,20H
	OUT	20H,AL
	OR	DRVSTAT,80H		; Set interrupt bit
	MOV	AX,9101h		; Resume task function for disk
	INT	15h
BAIL_OUT:				; 
	POP	DS			; 
	ASSUME	DS:NOTHING
	POP	DX
	POP	AX
	IRET
FD_ISR	ENDP
;
;******************************************************************************
;	FDINIT	 - Floppy diskette system initialization code
;
;	INPUT:	Nothing
;
;	OUTPUT: FDMED,FDMOTS,ERRSTAT,DRVSTAT,HDCFLG, AND FDRATE all initialized
;
;******************************************************************************
;
IFDEF   NNN_ECR_25
        SUBROUT         FD_INIT,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  FD_INIT
        FD_INIT   PROC    NEAR
ENDIF   ;NNN_ECR_25
;
	PUSH	AX		; Save all registers used
	PUSH	BX		; This is done to allow use of 
	PUSH	CX		;   the driver subroutines
	PUSH	DX
	PUSH	ES
	PUSH	DI
	PUSH	DS
	PUSH	SI
IFDEF	NNNINT077
	PUSH	AX			; temporary storage area
ENDIF	;NNNINT077
	PUSH	BP		
	MOV	BP,SP		; Set BP to current stack location
;
;		--- ZERO ALL FLOPPY VARIABLES ---
;
	MOV	DS,SEG40H	 
	MOV	ES,SEG40H
      ASSUME DS:ROMDAT,ES:ROMDAT
;

	XOR	AX,AX		
	MOV	CX,11		 ; Zero DRVSTAT,FDMOTS,FDTIMO,ERRSTAT,and
	MOV	DI,OFFSET DRVSTAT ; DSKST
      REP STOSB 		;
;
	MOV	DI,OFFSET HDCFLG
	MOV	CX,5
      REP STOSB 		; Zero HDCFLG,FDMED,FDOPER,and FDTRCK
;
	MOV	WTACTF,AL	; Zero wait flag
	MOV	FDRATE,0C0H	; Set FDRATE to invalid
;
;
;	  --- RESET DISK SYSTEM ---
;				   
IF	DIAG
	CALL	WRTINL			; Write diag message to CRT
	TM_DDT3 			; diag message		       
ENDIF	;DIAG

	CALL	DSKRESET
	JNC	RSTINITOK
      extrn	wrtinl:near
	CALL	WRTINL			; Display error message
	TM_DDF1 			; "RESET failed" message		       
;	CALL	SETERF			; set error flag


RSTINITOK:	

;
;	      --- TRY TO DETERMINE DRIVE 0 TYPE ---
;
	MOV	BYTE PTR STK_DL[BP],0	; Set drive number
	MOV	BYTE PTR STK_AH[BP],02	; Set function to read
	MOV	BYTE PTR STK_DH[BP],0	; Set head number
	CALL	DSKTEST
	JNC	TRY_1
;
TRY_1:
	MOV	BYTE PTR STK_DL[BP],1	; Set drive number to 1
	CALL	DSKTEST
	JNC	INIT_DONE
;		 
INIT_DONE:

	CALL	DPBADR			; Get Disk Parameter Block in ES:SI
	MOV	AH,ES:[SI+2]		; GET DISK TIMEOUT COUNT

	CMP	FDTIMO,0FFh-MINWAIT
	JNA	TEXSPUK
	ADD	AH,FDTIMO
	SUB	AH,0FFh-MINWAIT
TEXSPUK:
;@
	MOV	FDTIMO,AH 
;
;-------------------- CHECK FLOPPY CONFIG DATA VS. ACTUAL -------------------	;gh
;
;
					     
	mov	byte ptr STK_DL[bp], 0	; Set up for drive 0
	MOV	SI,OFFSET FDMED 	; set SI to drive type found by init 
	CALL	CKFDCFG 		; check config vs. actual
	JC	S_BADCFG		; bad config if carry set
;

	mov	byte ptr STK_DL[bp], 1	; Set up for drive 1
	INC	SI			; set SI to drive B type found
	CALL	CKFDCFG 		; check config
	JC	S_BADCFG
        JMP	SHORT FDCFEND		; no config error if no carry
;
S_BADCFG:

IFDEF	NNNCONSTRUCT
	TEST	OPT_CFG_TAB.CFG_OPT2, MASK NFLOP	; if NOFLOP
	JZ	DO_F1
	TEST	OPT_CFG_TAB.CFG_OPT2, MASK CMCFG	; if CMOSCFG
	JZ	NOCMCFG
	MOV	AL,CMCFGEN+NMIOFF	; Read cmos to see if supress
	CALL	CMREAD						     
	TEST	AL,FLPEN
	JNZ	DO_F1
NOCMCFG:
	JMP     short FDCFEND
ELSE	;NNNCONSTRUCT
      IF	NOFLOP                  ; no floppy installed
      IF	CMOSCFG
	MOV	AL,CMCFGEN+NMIOFF	; Read cmos to see if supress
	CALL	CMREAD						     
	TEST	AL,FLPEN
	JNZ	DO_F1
	JMP	FDCFEND
      ELSE	; CMOSCFG
	JMP	FDCFEND
      ENDIF	; CMOSCFG
      ENDIF	; NOFLOP
ENDIF	;NNNCONSTRUCT

DO_F1:	   
	MOV	AL,CMSTAT+NMIOFF	; set up for cmos write
	CALL	CMREAD
	TEST	AL,00100000B		; if bad config,
	JNZ	FDCFEND 		;  bit already set and "Run SETUP" 
					;  announcement already made
	OR	AL,00100000B		; set bad config bit
	MOV	AH,CMSTAT+NMIOFF	; set up for cmos write
;      EXTRN CMWRT:NEAR
;	push	ax
;	CALL	CMWRT
;	pop	ax
	test	al, 11000000b		; if bad csum or lost power,
	jnz	FDCFEND 		; "Run SETUP" announcement already made
	CALL	WRTINL
	TM_INVC
	CALL	SETERF			; set error flag
FDCFEND:

	POP	BP
IFDEF	NNNINT077
	POP	AX			; temporary storage area
ENDIF	;NNNINT077
	POP	SI
	POP	DS
	POP	DI
	POP	ES
	POP	DX
	POP	CX
	POP	BX
	POP	AX


	RET
FD_INIT	ENDP
;
;____________________________________________________________________________
;
IFDEF   NNN_ECR_25
        SUBROUT         CKFDCFG,NON_GLOBAL
ELSE    ;NNN_ECR_25
        CKFDCFG   PROC    NEAR
ENDIF   ;NNN_ECR_25

;
	call	fdtypif

	jc	badfcfg 		; If cmos bad exit
;										 
; Some tape backup devices are installed onto the WDAT Floppy Disk Controller	
; as another floppy drive.  However, these devices will cause POST to generate	
; an error (regardless of what drive type in CMOS they are set for, even if	
; they are set for 'Not Installed'.)  They will 'seek' to some location, or	
; otherwise respond to FDINIT as a 'broken' or misinitialized CMOS for the	
; drive type.  The fix below will allow these devices to be installed as	
; drive B: without error.							
;										
	IF	CMOSBDR								
	CMP	BYTE PTR STK_DL[BP], 1	; If checking drive 1,		    
	JNZ	CKFDCFG_DRIVE0						    
	CMP	AL, 0			; If CMOS says no drive, (regardless
	JNE	CKFDCFG_DRIVE0		;  of what's really there), it's OK.
	MOV	BYTE PTR [SI],0H	; Set drive type on stack to none.  
	JMP	short FDCOK			; Exit				    
CKFDCFG_DRIVE0:								    
	ENDIF   ;CMOSBDR						      
;									      
	CMP	BYTE PTR [SI],0H	; If no drive present
	JNE	TRYDRV			;    try drive
;					; else
	CMP	AL,0			; if cmos says no drive
	JE	FDCOK			;    good
	JMP	short BADFCFG 		; Else bad
;
TRYDRV:
	CMP	BYTE PTR [SI],93H	; if drive is not 40 track
	JNE	TRY80			;     check others
;					; else
	CMP	AL,1			;     check for configed for 40
	JNE	BADFCFG 		;     if not 40 then bad
	JMP	short FDCOK			;	 else ok
;
TRY80:
  
	CMP	AL,2			; check if config for other
	JB	BADFCFG 		;	if not other then bad
;					;	else ok
FDCOK:	
	CLC				; clear carry
	JMP	short ENDCKCFG		; and exit
BADFCFG:
	STC				; set carry
ENDCKCFG:
	RET
CKFDCFG ENDP

;
;------------------------------------------------------------------------------
;	DSKTEST -- Test and determine drive type
;								      
;INPUT:	SS:BP Points to stack frame for disk driver   
;		STK_DL[BP] = drive number		   
;
;	OUTPUT: FDMED[drive #], HDCFLG initialized
;		Carry set if no drive
;------------------------------------------------------------------------------
;
       PUBLIC    DSKTEST
IFDEF   NNN_ECR_25
        SUBROUT         DSKTEST,GLOBAL
ELSE    ;NNN_ECR_25
        PUBLIC  DSKTEST
        DSKTEST   PROC    NEAR
ENDIF   ;NNN_ECR_25

IF	DIAG
	CALL	WRTINL			; Write diag message to CRT
	TM_DDT4 			; diag message		       
ENDIF	;DIAG
;
	mov	al, SPFYCMD
	call	WRTCMD

	call	fdtypif
	jc	NOT_KNOWN			;if CMOS bad then use default
	cmp	al,1				;if 360K then
	jne	NOT_KNOWN
	mov	al, 0DFh			;use different step rate
	jmp	short SEND_SPFY
NOT_KNOWN:
	mov	al, 0AFh			;specify slow step rate
SEND_SPFY:
	call	WRTCMDA
	mov	al, 02h
	call	WRTCMDA

	MOV	BL,STK_DL[BP]	; Set BX to drive number
	XOR	BH,BH
;	MOV	FDMED[BX],87H	; Force slow data rate during drive test
;	CALL	SETRAT		; (seeks will)
	MOV	FDMED[BX],0
;
	CALL	DSKMOT		; Turn motor on
	CALL	DORECAL 	; Attempt a recal 
	JNC	_RECALD 	; If ok we are at track 0
	CALL	DORECAL 	; Else try it one more time
	JC	CHK_MSG 	; If not there now, then say no drive
_RECALD: 
	JMP	RECALD
CHK_MSG:
IF	DIAG
	CALL	WRTINL			; Write diag message to CRT
	TM_DDT5 			; diag message		       
ENDIF	;DIAG
;
;	   --- NO DRIVE, CHECK IF MESSAGE DESIRED ---
;						     
IFDEF	NNNCONSTRUCT
	TEST	OPT_CFG_TAB.CFG_OPT2, MASK NFLOP	; if NOFLOP
	JZ	DO_MSG
	TEST	OPT_CFG_TAB.CFG_OPT2, MASK CMCFG	; if CMOSCFG
	JZ	NOCMCFG2
	MOV	AL,CMCFGEN+NMIOFF	; Read cmos to see if supress
	CALL	CMREAD						     
	TEST	AL,FLPEN
	JNZ	DO_MSG
NOCMCFG2:
	JMP	BYE_BYE
ELSE	;NNNCONSTRUCT
      IF	NOFLOP          ; No floppy installed
      IF	CMOSCFG
	MOV	AL,CMCFGEN+NMIOFF	; Read cmos to see if supress
	CALL	CMREAD						     
	TEST	AL,FLPEN
	JNZ	DO_MSG
	JMP	BYE_BYE
      ELSE	; CMOSCFG
	JMP	BYE_BYE
      ENDIF	; CMOSCFG
      ENDIF	; NOFLOP
ENDIF	;NNNCONSTRUCT

DO_MSG:
	CALL	FDTYPIF 	; Check cmos for drive type
	JC	BDMM
	JNZ	BDMM
	JMP	BYE_BYE 		; If none configured




BDMM:
;
	test	byte ptr STK_DL[bp], 1
	jnz	dr1er

	CALL	WRTINL			; Display error message
	TM_DDF2 			; "seek failed" message 	       
	JMP	TSEEKER
dr1er:
	CALL	WRTINL			; Display error message
	TM_DDF3 			; "seek failed" message 	       

	JMP	TSEEKER

;
;    --- DRIVE HAS BEEN RECALIBRATED TO TRACK 0 ---
;
RECALD:
IF	DIAG
	CALL	WRTINL			; Write diag message to CRT
	TM_DDT6 			; diag message		       
ENDIF	;DIAG

	MOV	CH,50		; Set track number to 50
	CALL	DOSEEK		; Seek to track 50 (40 track drives go to 41)
IFDEF	REV_A_FIX
	CALL	SDRVST		; Check for track 0 signal
	JZ	Normal_drive
    IF	INFO
	call	wrtinl
	DB	'Seek 50 fail',CR,LF,0
    ENDIF
	jmp	IS_MUX_DRIVE		; If track 0 then muxed drive
Normal_drive:
    IF	INFO
	call	wrtinl
	DB	'Seems real drive',CR,LF,0
    ENDIF
ENDIF
	mov	cx, 180 	; Allow screwy TEAC drives time to settle down
rclld2: 			;  before change in direction
	call	dly100
	loop	rclld2

	MOV	CH,8		; Seek back to track 8 (40 track drive should
;				; just about reach 0)
RECALLP:
	CALL	DOSEEK		
	PUSH	CX
	MOV	CX, 48
RECLP1:
	CALL	DLY100
	LOOP	RECLP1
	POP	CX
	CALL	SDRVST		; Check for track 0 signal
	JNZ	IS_40		; If track 0 then 40 track drive
	DEC	CH		; Decrement track count


        if	SLEAZY	;get this test to work
	cmp	ch, 2
       endif    ;sleazy

	JNZ	RECALLP 	; If track number is zero, should be 80 track
;
       if	not SLEAZY
	CALL	DOSEEK		; This seek should reach track 0 on an 80
	PUSH	CX
	MOV	CX, 48
RECLP2:
	CALL	DLY100
	LOOP	RECLP2
	POP	CX

	CALL	SDRVST		; track drive. If not, error
	JZ	TSEEKER
       endif    ;NOT SLEAZY
				; known 80-cyl drive
	CALL	FDTYPIF 	; Get CMOS type
	JC	FORC_144
	AND	AL,07H		;
	CMP	AL,1b
	JNA	FORC_144	; Invalid configuration if CMOS says 40 track
	MOV	DI,OFFSET CS:STRT_TAB ; Get pointer to media type table
	XOR	AH,AH
	ADD	DI,AX		; Get media type from table
	MOV	AL,CS:[DI]
	MOV	FDMED[BX],AL
	MOV	AL,111b 	; Set change line present

	CALL	STODINF
	JMP	short DKTEX
;
TSEEKER:
	CALL	FDTYPIF 	; Check cmos for drive type
       IFDEF	NNNEA89062302
	JC	CNFG_ERR
       ENDIF	;NNNEA89062302
	JZ	DKTEX		; If none configured
;
	JMP	short CNFG_ERR
FORC_144:
	MOV	FDMED[BX],07H	; Force media to 1.44

	MOV	AL,011b
	CALL	STODINF
	JMP	short CNFG_ERR
;
IS_40:
IF	DIAG
	CALL	WRTINL			; Write diag message to CRT
	TM_DDT7 			; diag message		       
ENDIF	;DIAG
	
	MOV	FDMED[BX],93H	; Set FDMED to try 360/360   
	MOV	AL,04		; Set val for HDCFLG
	CALL	STODINF 	; Set HDCFLG
	CALL	FDTYPIF
	JC	CNFG_ERR
	CMP	AL,1		; CMOS should indicate 360
	JNZ	CNFG_ERR
DKTEX:
	CALL	DORECAL 	; Attempt a recal 
	JC	CNFG_ERR		; If ok we are at track 0
BYE_BYE:
	mov	al, SPFYCMD			;set specify back to original
	call	WRTCMD				;value
	mov	al, 0AFh			;specify slow step rate
	call	WRTCMDA
	mov	al, 02h
	call	WRTCMDA

	CLC
	RET
IFDEF	REV_A_FIX
; Claims to be a valid 1.44 Meg drive
IS_MUX_DRIVE:
    IF	INFO
	call	wrtinl
	DB	'Is muxed drive',CR,LF,0
    ENDIF
	mov	al,07h
	MOV	FDMED[BX],AL
	MOV	AL,11b 	; Set change line present

	CALL	STODINF
	JMP	short DKTEX
ENDIF
CNFG_ERR:
	mov	al, SPFYCMD			;set specify back to
	call	WRTCMD				;original value
	mov	al, 0AFh			;specify slow step rate
	call	WRTCMDA
	mov	al, 02h
	call	WRTCMDA

	STC
	RET
DSKTEST ENDP
IFDEF	NNNINT077
;
FIFOENB	EQU	TRUE	;TRUE if fifo feature should be enabled
DUMPREG	EQU	0EH	;dumpreg command byte
CNFG_0	EQU	13H	;configure command byte 0
CNFG_1	EQU	0	;configure command byte 1
IF	FIFOENB
CNFG_2	EQU	07	;configure command byte 2,EIS=EFIFO=POLL=0,FIFOTHR=7
ELSE	;FIFOENB
CNFG_2	EQU	021H	;configure command byte 2,EIS=POLL=0,EFIFO=1,FIFOTHR=1
ENDIF	;FIFOENB
CNFG_3	EQU	0	;configure command byte 3
;----------------------------------------------------------------------------
IS_I077_INSTD	PROC	NEAR
;
;description:
;	This routine dynamically determines if a 82077 FDC is installed.
;if 82077 is installed and the FIFO feature is enabled, it clears the CY
;else it sets the CY.
;
;input: none
;output:  STK_T1	B0: 1 if 82077 installed
;			B1: 1 if FIFO enabled
	PUSHA
	PUSH	DS
;
	MOV	DS,SEG40H
      ASSUME DS:ROMDAT
; save errstat
	PUSH	WORD PTR ERRSTAT
; reset 82077 stat bits
	AND	WORD PTR STK_T1[BP], NOT 3

	MOV	AL,DUMPREG
	CALL	WRTCMD
	JC	DONE
;
;expecting 10 result bytes. If error then know that there is no 82077.
;
;read the 1st 7 result bytes
;
	CALL	RSLTRD7
	JC	DONE
;
;read the last 3 result bytes
;
	MOV	CX,3
	CALL	RSLTRD
	JC	DONE
;
	OR	WORD PTR STK_T1[BP],1
	MOV	SI,OFFSET DSKST
	TEST	BYTE PTR[SI+1],20H		;EFIFO BIT
	JNZ	DONE
	OR	WORD PTR STK_T1[BP],2
DONE:   
	POP	WORD PTR ERRSTAT
	POP	DS
	POPA
	RET

IS_I077_INSTD	ENDP
;
;----------------------------------------------------------------------------
;
CONFIGURE	PROC	NEAR
;
;description:
;	This routine issues the configure command to enable the FIFO in
;82077 FDC. If FDC error then CY is set, else it CY is cleared.
;
;input: AH	;BYTE 2 OF CONFIG CMD
;output: CY set if FDC error
;           else cleared
;
 	PUSH	AX
;issue configure command
; 
	MOV	AL,CNFG_0
	CALL	WRTCMD
	JC	ERR_FDC
	MOV	AL,CNFG_1
	CALL	WRTCMD
	JC	ERR_FDC
	POP	AX
	PUSH	AX
	MOV	AL,AH
	CALL	WRTCMD
	JC	ERR_FDC
	MOV	AL,CNFG_3
	CALL	WRTCMD
ERR_FDC:
	POP	AX
	RET
CONFIGURE	ENDP
;----------------------------------------------------------------------------
ENDIF	;NNNINT077
   PUBLIC	ENDCODE
ENDCODE:
;
;------------------------------- THE END -------------------------------------
;
	SYNC	BASIC_INT		; creates ORG at $C003 in BIOS segment

	TOTAL

CODE	ENDS
	END

