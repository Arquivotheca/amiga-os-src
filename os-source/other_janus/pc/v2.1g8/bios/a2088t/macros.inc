	IFNDEF LISTINC
.XLIST
.XCREF
	ENDIF
;	NAME	MACROS
;	TITLE	Macros for the ROM
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
;------------------------------------------------------------------------------
;   The SYNC macro fills space between the current IP and address passed as an
; argument.  Because of many bugs (or possibly design limitations) in the MASM
; assembler, this code works, but just barely.
;
;   The argument passed (due to MASM) must be an absolute #, so you can pass a
; constant or an EQUATE that has a number type.
;
;   The space is filled with the value FILBYT.	This may be useful for either
; faster PROM programming (use a 0FFh) or as an instruction trap for programs
; that go wild, etc.
;
;   If you place an initial SYNC at the beginning of the program, the argument
; passed to subsequent SYNCs will be relatively referenced to the initial SYNC
; value.
;
;   If the current code is past the SYNC point, an error message will be output
; with the # of bytes it was exceeded by.  However, the code is correctly
; assembled and can be run.  It will just not be located at those absolute
; locations relative to the beginning of that module.
;   Also all subsequent overruns of syncs will tell you by how much from the
; last sync, not from the beginning of the module
;
;   If the SYNC macros are not in order, an error message will be output.  If
; you need to go backwards (e.g. to put the code before the data) then use the
; RESYNC macro.  With the error, the SYNC is ignored.
;
SYNC	MACRO	OP1		; Do indirect for non-AT
	SYNCA	<OP1>		;   else do it
	ENDM
;
SYNCA	MACRO	NXTADR			;; Move location counter to NXTADR,
	IFDEF	LSTSYC
	  IF1
	    IF	NXTADR LT LSTSYC
		MSG	<<The sync address>>,NXTADR,<<is out of order>>
	    ENDIF
	    IF	(NXTADR-LSTSYC) LT (($-FSTPC)-(LSTPC-FSTPC))
MSGA	(($-FSTPC)-(LSTPC-FSTPC)+LSTSYC),<<	>>,(($-FSTPC)-(LSTPC-FSTPC)-(NXTADR-LSTSYC)),<<bytes too long ****>>
	    ELSE
MSGA	(($-FSTPC)+FSTSYC),<<** >>,((NXTADR-LSTSYC)-(($-FSTPC)-(LSTPC-FSTPC))),<<	-- Free space>>
	    ENDIF
	  ENDIF
	ENDIF
;
	IFNDEF	FSTPC
	  LSTPC 	=	$
	  ORG		0		; Move PC back to 0
	  FSTPC 	=	$	;   and create a label there
	  ORG		(LSTPC-FSTPC)	; Restore the PC
;
	  %OUT	.      | SIZE  |
	  %OUT	ADDRESS|IN HEX |
	  %OUT	-------+-------
	  PASS1 =	TRUE	; Pass 1 is true
	  IFB	<NXTADR>
	    FSTSYC	=	($-FSTPC) ; If not defined, default to current PC
	  ELSE
	    FSTSYC	=	NXTADR	; address of the 1st sync address
	  ENDIF
	  LSTSYC	=	FSTSYC	; This was our last sync address
	ENDIF
;
	IF2
	  IF	PASS1			; Is this the 1st SYNC of PASS 2
	    LSTPC	=	$	; Save current PC
	    ORG 	0		; Move PC back to 0
	    FSTPC	=	$	;   and create a label there
	    ORG 	(LSTPC-FSTPC)	; Restore the PC
	  IFB	<NXTADR>
	    FSTSYC	=	($-FSTPC) ; If not defined, default to current PC
	  ELSE
	    FSTSYC	=	NXTADR	; address of the 1st sync address
	  ENDIF
	    LSTSYC	=	FSTSYC	; Yes, set last sync to 1st sync
	    PASS1	=	FALSE	; We're now not in PASS 1
	  ENDIF
	ENDIF
;
	IF	(NXTADR-LSTSYC) GT (($-FSTPC)-(LSTPC-FSTPC))
	    DB		((NXTADR-LSTSYC)-(($-FSTPC)-(LSTPC-FSTPC))) DUP (FILBYT)
					; Fill with 0FFh bytes for greater
					;    reliability and faster programming
					;    or maybe with a trapped
					;    instruction value, etc.
		ORG	(LSTPC-FSTPC)+(NXTADR-LSTSYC)	; Origin for NXTADR-LSTSYC
	ENDIF
;
	LSTSYC	=	NXTADR
	LSTPC	=	$	; DEFINING NEW LSTSYC AND LSTPC FOR NEXT TIME
;
	PRGSIZA <>

	ENDM
;------------------------------------------------------------------------------
;   The RESYNC macro fills space between the current IP and address passed as an
; argument.  Because of many bugs (or possibly design limitations) in the MASM
; assembler, this code works, but just barely.
;
;   The argument passed (due to MASM) must be an absolute #, so you can pass a
; constant or an EQUATE that has a number type.
;
;   The space is not filled as with the SYNC macro.
;
;   If the current RESYNC point is before the 1st SYNC point an error message
; will be output
;
RESYNC	MACRO	NXTADR			;; Move location counter to NXTADR
;
	  IFNDEF	FSTSYC		; If not defined, then assume 0
OUT1	<***** There must be an initial SYNC macro to use a RESYNC macro>
	  ENDIF
	  OUT1
;
	  IF	NXTADR LT FSTSYC
	    MSG 	<<The sync address>>,NXTADR,<<is before the 1st address>>
	  ELSE
	    ORG 	(NXTADR-FSTSYC)
	  ENDIF
;
	  LSTSYC	=	NXTADR		; Defining the values for next time
	  LSTPC 	=	$
;
	  PRGSIZA	<>
;
	ENDM
;------------------------------------------------------------------------------
;   This Macro prints out size of code on terminal from the last PRGSIZ
; and the origin from the last PRGSIZ
;
;	Call	PRGSIZ		; Some versions of MASM don't like this form
;		PRGSIZ <>	;   or this form, because they don't do the
;				;   IFNB <FLAG> evaluation correctly
;		PRGSIZ <string>
;
PRGSIZ	MACRO	OP1		; Do indirect for non-AT
	PRGSIZA <OP1>		;   else do it
	ENDM
;
PRGSIZA MACRO	FLAG
	  IF1
	    IFNDEF	FSTSYC			; If not defined, then assume 0
	      FSTSYC	=	0
	    ENDIF
	    IFNDEF	LSTADD			; If not defined, then assume 0
	      LSTADD	=	0
	    ENDIF
;
;   IFNB function fails on some versions of MASM so we use IFB, which also fails
; sometimes.  Nothing distastourous occurs except the displayed output is messed
; up.
;
	    IFB 	<FLAG>
	    ELSE
	      MSGA	FSTSYC+LSTADD,<<	>>,$-LSTADD,<<	-- &FLAG >>
	    ENDIF
	      LSTADD	=	$
	  ENDIF
	ENDM
;------------------------------------------------------------------------------
; Macro to print out the SYNC macros address current PC value on terminal 
;
;	Call	TOTAL
;
TOTAL	MACRO
	  IF1
	    IFNDEF	FSTSYC			; If not defined, then assume 0
			FSTSYC	=	0
	    ENDIF
;
	    %OUT ----------------
	    MSGA	FSTSYC+$,<<	>>,$,<< -- Total Size of file>>
	  ENDIF
	ENDM
;------------------------------------------------------------------------------
; This macro prints out a message with numbers printed in base 16
;
MSGA	MACRO	W,X,Y,Z
	  .RADIX	16
	  MSG1A 	%(&W),X,%(&Y),Z
	  .RADIX	10
	ENDM
;------------------------------------------------------------------------------
; This macro prints out a message
;
MSG1A	MACRO	A,B,C,D
		%OUT A B C D
	ENDM
;------------------------------------------------------------------------------
; This macro prints out a message with numbers printed in base 16
;
MSG	MACRO	X,Y,Z
	  .RADIX	16
	  MSG1		X,%(&Y),Z
	  .RADIX	10
	ENDM
;------------------------------------------------------------------------------
; This macro prints out a message
;
MSG1	MACRO	A,B,C
	  %OUT A B C
	ENDM
;------------------------------------------------------------------------------
; OUT to console only during pass 1
;
OUT1	MACRO	X
	  IF1
	    %OUT	X
	  ENDIF
	ENDM

;------------------------------------------------------------------------------
; Macros to delay for I/O devices
WAFORIO	MACRO				; forces a wait of at least 1 usec
	JMP	$+2
	WAFORIOIF
	ENDM
WAFORIOIF MACRO				;don't wait if CLK10 "normal" - 
					; forces a wait of 1.5 +/- 1.5 usec
	if	CLK10 GE 150
	DW	((CLK10-50)/100) DUP (00EBh)	;"JMP $+2" repeated, usually 0x
	endif   ;CLK10 GE 150
	ENDM

;------------------------------------------------------------------------------
; Macro for 80286 CPU to create a Descriptor table entry
;
;	Call DSCRTR	LIMIT,BASE,ACCESS
;
;DESCR	MACRO	LIMIT,BASE,ACCESS
;	DW	LIMIT			; Limit of segment offset
;	DW	BASE AND 0FFFFh 	; Bits 15 ->  0 of segment physical base
;	DB	(BASE SHR 16) AND 00FFh ; Bits 23 -> 15 of segment physical base
;	DB	ACCESS			; Access rights
;	DW	0			; Reserved, must be 0 for i386
;	ENDM
;------------------------------------------------------------------------------
; Macro for 80286 CPU to create an Interrupt Descriptor table entry
;
;	Call DESCRI	CODADD,SEGSEL,ACCESS
;
DESCRI	MACRO	CODADD,SEGSEL,ACCESS
	DW	OFFSET CODADD		; Interrupt code offset
	DW	SEGSEL			; Interrupt code segment selector
	DB	0			; Unused
	DB	ACCESS			; present, privilege, & task/int flags
	DW	0			; Reserved, must be 0 for i386
	ENDM
;------------------------------------------------------------------------------
	IF	true	;false if Microsoft ever makes a real 3.0 assembler
;------------------------------------------------------------------------------
; Macro to create instruction PUSHA
;
;	Call PUSHA
;
PUSHA	MACRO
	DB	60h			; Push All instruction for 286
	ENDM
;------------------------------------------------------------------------------
; Macro to create instruction PUSHA
;
;	Call POPA
;
POPA	MACRO
	DB	61h			; Pop All instruction for 286
	ENDM
;------------------------------------------------------------------------------
; Macro to create overide CS:
;
;	Call SEGCS
;
SEGCS	MACRO
	DB	2Eh			; Segment overide
	ENDM
;------------------------------------------------------------------------------
; Macro to create overide DS:
;
;	Call SEGDS
;
SEGDS	MACRO
	DB	3Eh			; Segment overide
	ENDM
;------------------------------------------------------------------------------
; Macro to create overide ES:
;
;	Call SEGES
;
SEGES	MACRO
	DB	26h			; Segment overide
	ENDM
;------------------------------------------------------------------------------
; Macro to create overide SS:
;
;	Call SEGSS
;
SEGSS	MACRO
	DB	36h			; Segment overide
	ENDM
;------------------------------------------------------------------------------
; Macro to create a LMSW instruction
;
;	Call LMSW	OP1 OP2 OP3 OP4 OP5 OP6
;
LMSW	MACRO	OP1,OP2,OP3,OP4,OP5,OP6
	LOCAL	A,B
	DB	0Fh			; Initial code value
A:
	PUSH	OP1 OP2 OP3 OP4 OP5 OP6
B:
	ORG	A
	DB	01h			; Value to insert instead of OPCODE
	ORG	B
	ENDM
;------------------------------------------------------------------------------
; Macro to create a LGDT instruction
;
;	Call LGDT	OP1 OP2 OP3 OP4 OP5 OP6
;
LGDT	MACRO	OP1,OP2,OP3,OP4,OP5,OP6
	LOCAL	A,B
	DB	0Fh			; Initial code value
A:
	MOV	SS,OP1 OP2 OP3 OP4 OP5 OP6
B:
	ORG	A
	DB	01h			; Value to insert instead of OPCODE
	ORG	B
	ENDM
;------------------------------------------------------------------------------
; Macro to create a LIDT instruction
;
;	Call LIDT	OP1 OP2 OP3 OP4 OP5 OP6
;
LIDT	MACRO	OP1,OP2,OP3,OP4,OP5,OP6
	LOCAL	A,B
	DB	0Fh			; Initial code value
A:
	MOV	DS,OP1 OP2 OP3 OP4 OP5 OP6
B:
	ORG	A
	DB	01h			; Value to insert instead of OPCODE
	ORG	B
	ENDM
;------------------------------------------------------------------------------
; Macro to create a FNSTCW instruction
;
;	Call FNSTCW	OP1 OP2 OP3 OP4 OP5 OP6
;
FNSTCW	MACRO	OP1,OP2,OP3,OP4,OP5,OP6
	LOCAL	A,B
A:
	IDIV	OP1 OP2 OP3 OP4 OP5 OP6
B:
	ORG	A
	DB	0D9h			; Value to insert instead of OPCODE
	ORG	B
	ENDM
;------------------------------------------------------------------------------
; Macro to create a FNSTSW instruction
;
;	Call FNSTSW	OP1 OP2 OP3 OP4 OP5 OP6
;
FNSTSW	MACRO	OP1,OP2,OP3,OP4,OP5,OP6
	LOCAL	A,B
A:
	IDIV	OP1 OP2 OP3 OP4 OP5 OP6
B:
	ORG	A
	DB	0DDh			; Value to insert instead of OPCODE
	ORG	B
	ENDM
;------------------------------------------------------------------------------
; Macro to create a FNINIT instruction
;
;	Call FNINIT
;
FNINIT	MACRO
	DB	90h,0DBh,0E3h		; Initial code value
	ENDM
;------------------------------------------------------------------------------
	endif
;
;------------------------------------------------------------------------------
; Following are MACROS for included at various places of the code modules.
; These are done because the same code may be desired at one of several
; different locations depending if it's an 8k ROM (PC-XT), 16k ROM (PC-XT) or
; an AT ROM
;------------------------------------------------------------------------------
;
PNXMAC4 MACRO			; Include DS40H, BOOTCODE,
;------------------------------------------------------------------------------
;
; ***** Warning: the MASM assembler decides that the segment for DS40H is CS:
; even though we have an assume statement and a public statement.  It seems
; that the 1st use of DS40H assigns it to the DS group, and when it is finally
; defined later, it ignores its CS segment.  This does not seem to be a problem
; when externally defined but only in this module.  Also in MASM 1.25 and 1.27
; it thinks its a byte pointer when loading DS, even though it should overide
; this.  However to keep all MASMs happy, place a WORD PTR in front of forward
; it references to DS40H
;
	PUBLIC	DS40H
DS40H	DW	0040H			; Segment of ROM data
	PRGSIZ	<Segment of ROM data constant>	; Display program size
;
;------------------------------------------------------------------------------
	PUBLIC	BOOTCODE
BOOTCODE	LABEL	DWORD			; boot up address
	dw	7C00H
	dw	0000H
	PRGSIZ	<Boot code load address>	; Display program size
	ENDM


PNXMAC5 MACRO			; Include COMXITA, COMEXIT
;------------------------------------------------------------------------------
;
; Restore Stack and return to caller
;
;	Input:	Registers PUSHed on stack
;	Output: Registers restored
;
;------------------------------------------------------------------------------
	PUBLIC	COMXITA
	PUBLIC	COMEXIT
COMXITA PROC	NEAR			; Common DSR return code with A register
	POP	AX
COMEXIT:				; Common DSR return code
	POP	ES
	ASSUME	ES:NOTHING
	POP	DS
	ASSUME	DS:NOTHING
	POP	BP
	POP	DI
	POP	SI
	POP	DX
	POP	CX
	POP	BX
	IRET
COMXITA ENDP
	PRGSIZ	<Common DSR Exit>	; Display program size
	ENDM
;------------------------------------------------------------------------------


PNXMAC6 MACRO			; Include HEXMSG, CRLFMSG
;------------------------------------------------------------------------------
; Trailing String for Hex messages
;------------------------------------------------------------------------------
	PUBLIC	HEXMSG
	PUBLIC	CRLFMSG
HEXMSG	DB	'h'
;------------------------------------------------------------------------------
; String for Carriage return, Line feed
;------------------------------------------------------------------------------
CRLFMSG DB	CR,LF,0
	PRGSIZ	<CR,LF message> 	; Display program size
	ENDM


PNXMAC7 MACRO			; Boot fail message
;------------------------------------------------------------------------------
; Boot fail message
;------------------------------------------------------------------------------
	PUBLIC	BTMSG0
BTMSG0	DB	CR,'Boot disk failure.  Type key to retry',0
	PRGSIZ	<Boot fail message>	; Display program size
	ENDM
;
.CREF
.LIST
