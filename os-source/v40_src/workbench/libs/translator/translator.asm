**
**	$Id: translator.asm,v 36.5 90/05/30 17:43:20 kodiak Exp $
**
**	translator library code
**
**	(C) Copyright 1986,1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION translator


******* translator.library/Translate *******************************************
*
*    NAME
*	Translate -- Convert an English string into narrator device phonemes.
*
*    SYNOPSIS
*	rtnCode = Translate(inString, inLength, outBuffer, outLength)
*	D0		    A0	      D0	A1	   D1
*
*	LONG Translate( STRPTR inString, LONG inLength, STRPTR outBuffer,
*	    LONG outlen );
*
*    FUNCTION
*	The translate function converts an English string into
*	a string of phonetic codes suitable as input to the
*	narrator device.  
*
*    INPUTS
*	inString - pointer to English string
*	inLength - length of English string
*	outBuffer - a char array which will hold the phonetic codes
*	outLength - the length of the output array
*
*    RESULTS
*	rtnCode - zero if no error has occured.
*	    The only error that can occur is overflowing the outBuffer.
*	    If Translate() determines that an overflow will occur, it
*	    will stop the translation at a word boundary before the
*	    overflow happens.  If this occurs, rtnCode will be a
*	    negative number whose absolute value indicates where in
*	    inString Translate() stopped.  The user can then use the
*	    offset -rtnCode from the beginning of inString in a
*	    subsequent Translate() call to continue the translation.
*
*    SEE ALSO
*	narrator.device/CMD_WRITE
*
********************************************************************************
*
*   IDEAS FOR ENHANCEMENT
*	required rules, optional exception table
*
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*								    ;
*		   ENGLISH TO ARPABET TRANSLATOR		    ;
*								    ;
*	  CONVERTS ASCII ENGLISH STRING TO ASCII PHONETIC	    ;
*	  REPRESENTATION.  USES AN EXPANDED NRL RULE SYSTEM.	    ;
*								    ;
*	  D0 - SCRATCH REGISTER		D1 - LETTER/SCRATCH	    ;
*	  D2 - SIZE OF HANDLE		D3 - WORD LENGTH	    ;
*	  D4 - LETTER FEATURE		D5 - SCRATCH		    ;
*	  D6 - VARIED			D7 - DIRECTION TO FETCH	    ;
*								    ;
*	  A0 - ENGLISH INPUT		A1 - PHONETIC OUTPUT	    ;
*	  A2 - BEG OF CNTR OF RULE	A3 - END OF CNTR OF RULE    ;
*	  A4 - FEATURE MATRIX		A5 - PTR TO WORD BUFFER	    ;
*	  A6 - RULE BASE		A7 - STACK		    ;
*								    ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	XDEF	TLTranslate

*-------- FRAME OFFSETS (FROM A5)

EXCPSHAN  EQU	    0		;OFFSET TO EXCEPTIONS HANDLE
RULESHAN  EQU	    4		;OFFSET TO RULES HANDLE
STOUTOFF  EQU	    8		;OFFSET TO START OF OUTPUT
STINPOFF  EQU	    12		;OFFSET TO START OF INPUT
OUTPTR	  EQU	    16		;OFFSET TO ACCENTING RULE LAST POS
ACCFLAG	  EQU	    20		;OFFSET TO ACCENTING RULE FLAG
WORDBOFF  EQU	    22		;OFFSET TO START OF WORD BUFFER
INLENOFF  EQU	    130		;OFFSET TO INPUT LENGTH
OUTHDOFF  EQU	    182		;OFFSET TO OUTPUT HANDLE

SYMCNT	  EQU	    12		;NUMBER OF SPECIAL SYMBOLS [not including ' ']
MAXLTRS	  EQU	    100		;MAX NUMBER OF LETTERS IN A WORD


SYMBOL	  EQU	    $1
DIGIT	  EQU	    $2
UAFF	  EQU	    $4
VOICED	  EQU	    $8
SIBILANT  EQU	    $10
CONS	  EQU	    $20
VOWEL	  EQU	    $40
LETTER	  EQU	    $80
FRONT	  EQU	    $100
IGNORE	  EQU	    $200
KEYWORD	  EQU	    $400
WORDBRK	  EQU	    $800

SYMBOLBIT  EQU	      0
DIGITBIT   EQU	      1
UAFFBIT	   EQU	      2
VOICEDBIT  EQU	      3
SIBILBIT   EQU	      4
CONSBIT	   EQU	      5
VOWELBIT   EQU	      6
LETTERBIT  EQU	      7
FRONTBIT   EQU	      8
IGNOREBIT  EQU	      9
KEYWORDBIT EQU	      10
WORDBRKBIT EQU	      11



*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*		   INITIALIZATION CODE			  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


TLTranslate:
	  MOVEM.L   A2-A6/D2-D7,-(SP)	;SAVE REGISTERS

	  MOVE.L    D1,D2
	  SUBQ.L    #5,D2		;REDUCE MAX OUTPUT TO INSURE NO OVERFLOW

*-------- ;Create stack frame

	  MOVE.L    D0,-(SP)		;SAVE LENGTH OF INPUT IN FRAME
	  
	  MOVE.L    #'	  ',D0		;CLEAR WORD BUFFER (108 BYTES)
	  MOVEQ	    #26,D1
CLEARWDBF MOVE.L    D0,-(SP)
	  DBF	    D1,CLEARWDBF

	  CLR.W	    -(SP)		;ACCENTING CODE - ACCFLAG	  
	  MOVE.L    A1,-(SP)		;ACCENTING CODE - LAST POS PTR

	  MOVE.L    A0,-(SP)		;START OF INPUT
	  MOVE.L    A1,-(SP)		;SAVE START OF OUTPUT
	  CLR.L	    -(SP)		;SPACE FOR RULES HANDLE	  
	  CLR.L	    -(SP)		;SPACE FOR EXCEPTIONS HANDLE	  

	  MOVE.L    SP,A5		;SAVE FRAME POINTER

	  MOVE.L    #0,EXCPSHAN(A5)	;    NO EXCPS ARE TO BE USED !!!

	  MOVEQ	    #0,D0		;CLEAR SOME REGISTERS (DON'T CLEAR D2!!)
	  MOVEQ	    #0,D1
	  MOVEQ	    #0,D3
	  MOVEQ	    #0,D4
	  MOVEQ	    #0,D5
	  MOVEQ	    #0,D6
	  MOVEQ	    #0,D7

	  LEA	    WORDBOFF+2(A5),A2	;POINTER TO BEGINNING OF WORD BUFFER
	  LEA	    FEATURES,A4		;POINTER TO LETTER FEATURES

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	  MAIN LOOP.					  ;
*	  1)  CHECK TO SEE IF A '#' WAS INSERTED,	  ;
*	      INDICATING THAT THE INPUT STRING HAS BEEN	  ;
*	      ENTIRELY PROCESSED,			  ;
*	  2)  IF NOT END OF WORD, CONTINUE PROCESSING	  ;
*	      WITH NEXT LETTER/COMBINATION,		  ;
*	  3)  IF END OF WORD, CALL PRE-PROCESSOR FOR	  ;
*	      NEXT WORD.				  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

WORDC	  CMPI.B    #'#',-1(A1)
	  BEQ.S	    RETURN		;IF "#" INSERTED, END OF STRING
	  CMPI.B    #' ',-1(A2)		;IF BLANK PROCESSED, GET NEXT WORD
	  BNE.S	    WORDC1

NEXTWORD:
	  BSR	    PREPROC		;GET NEXT WORD FROM ENGLISH
	  MOVE.W    D3,-(SP)		;SAVE RETURN CODE    
	  BSR	    ACCENTER		;ACCENT THE PREVIOUS WORD
	  MOVE.W    (SP)+,D3		;RESTORE RETURN CODE
	  BMI.S	    FIXUP		;OVERFLOW MAX # LTRS IN WORD
	  BEQ.S	    RETURN		;NOTHING LEFT, GO AWAY
	  LEA	    WORDBOFF+2(A5),A2	;POINT TO BEGINNING OF STRING
	  BRA.S	    WORDC0

ERROR	  MOVE.W    D0,D1		;SET BAD RETURN CODE
	  BRA.S	    FIXUP		;COMMON RETURN CODE

RETURN	  MOVEQ	    #0,D1
	  BSR	    ACCENTER

	  MOVE.B    #0,-1(A1)		;Change # to a NULL


FIXUP:

	  MOVE.L    A1,D0		;GET CURRENT OUTPUT POS
	  SUB.L	    STOUTOFF(A5),D0	;COMPUTE LENGTH OF OUTPUT

	  ADD.L	    #134,SP		;REMOVE FRAME FROM STACK
	  MOVEM.L   (SP)+,A2-A6/D2-D7	;RESTORE REGISTERS

	  MOVE.W    D1,D0
	  EXT.L	    D0

	  RTS				;RETURN TO PASCAL

WORDC0	  MOVE.L    EXCPSHAN(A5),A6	;DO EXCPS RULES
	  CMP.L	    #0,A6		;NIL?
	  BEQ.S	    WORDC1		;YES, SKIP EXCPS
	  MOVE.L    (A6),A6		;DEREFERENCE TO POINTER
	  BSR.S	    CINDEX		;FIND RULE GROUP
	  BSR.S	    CHECK		;DO EXCP CHECKING
	  BEQ.S	    WORDC1		;NO EXCP RULE APPLIED, DO BASIC RULES
	  BMI	    ERROR		;REPLACE OVFL, COULD NOT GET MORE MEM
	  BRA.S	    WORDC		;EXCP RULE APPLIED, GET NEXT WORD

WORDC1
;KODIAK	  MOVE.L    RULESHAN(A5),A6	;DO BASIC RULES
;KODIAK	  MOVE.L    (A6),A6		;DEREFERENCE TO POINTER
	  LEA	    RULEBASE,A6
	  BSR.S	    CINDEX		;FIND RULE GROUP
	  BSR.S	    CHECK		;DO EXCP CHECKING
	  TST.W	    D7
	  BMI	    ERROR		;REPLACE OVFL, COULD NOT GET MORE MEM
	  BRA.S	    WORDC		;EXCP RULE APPLIED, GET NEXT WORD

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	       DO RULE CHECKING				  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CHECK	BSR.S	CHKCENTR		;CHECK CENTER CONTEXT
	BNE.S	NEXTRULE		;UNSUCCESSFUL
	MOVE.L	A2,-(SP)		;SAVE PTR TO BEG OF CENTER
	MOVE.L	A3,-(SP)		;SAVE PTR TO END OF CENTER
	BSR.S	CHKLEFT			;CHECK LEFT CONTEXT
	BNE.S	LNEXTRULE		;UNSUCCESSFUL
	BSR	CHKRIGHT		;CHECK RIGHT CONTEXT
	BNE.S	LNEXTRULE		;UNSUCCESSFUL
	BSR	REPLACE			;DO REPLACEMENT
	MOVE.L	(SP)+,A2		;MAKE END OF CENTER, BEG OF CENTER
	ADDQ.L	#4,SP			;REMOVE OLD BEG OF CENTER
	RTS


*------ Find next rule in ruleset
LNEXTRULE	MOVE.L	(SP)+,A3	;REMOVE END OF CENTER
		MOVE.L	(SP)+,A2	;REMOVE BEG OF CENTER
NEXTRULE	MOVE.B	(A6)+,D7	;GET NEXT CHAR FROM RULE
		CMP.B	#'\',D7		;END OF RULE?
		BEQ.S	CHECK		;YES
		CMP.B	#'`',D7		;OTHER END OF RULE?
		BEQ.S	CHECK		;YES
		BRA.S	NEXTRULE	;NOT END OF RULE, KEEP LOOKING

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	       COMPUTE INDEX INTO RULESET		  ;
*	A2 - Pointer to next char in word		  ;
*	A6 - Pointer to EXCP/RULE set			  ;
*	A4 - Features					  ;
*							  ;
*	RETURNS:					  ;
*	D1 - SCRATCH					  ;
*	D4 - Features					  ;
*	A6 - Pointer to letter group of rules		  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CINDEX	MOVEQ	#0,D1		;CLEAR SCRATCH REG
	MOVE.B	(A2),D1		;GET CHAR FROM WORD BUFFER
	MOVE.W	D1,D4		;COPY
	LSL.W	#1,D4		;SHIFT FOR FEAT INDEX
	MOVE.W	0(A4,D4),D4	;GET FEATURES
	BTST	#LETTERBIT,D4	;LETTER?
	BNE.S	CLETTER		;YES

CSYMBOL MOVE.B	#'Z'+1,D1	;SET RULE PTR TO 'Z'+1 RULESET
	BTST	#DIGITBIT,D4	;DIGIT?
	BNE.S	CLETTER		;YES, USE 'Z'+1 RULES
CDIGIT	ADDQ.B	#1,D1		;NO, USE 'Z'+2 RULES

CLETTER SUB.B	#'A',D1		;CONVERT TO 'A OFFSET'
	LSL.W	#2,D1		;ADJUST FOR RULE INDEXING
	MOVE.L	0(A6,D1),A3	;GET OFFSET LETTER RULE GROUP
	ADD.L	A3,A6		;CREATE PTR TO RULE GROUP
	RTS

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	       CHECK CENTER PORTION OF RULE		  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CHKCENTR  MOVEQ	    #0,D6		;CLEAR INDEX INTO RULE
	  MOVEQ	    #-1,D7		;SET TEMP REG
	  MOVE.L    A2,A3

*	  FIND '[', BEGINS CENTER PORTION OF RULE

CLTR1	  MOVE.B    0(A6,D6),D5		;GET BYTE FROM RULE
	  ADDQ.W    #1,D6		;BUMP RULE INDEX
	  CMP.B	    #'[',D5		;IS RULE CHAR='['?
	  BNE.S	    CLTR1		;NO, KEEP LOOKING
	  ADD.W	    D6,D7		;D7 POINTS TO '['
	  SWAP	    D6			;D6 = POS OF CENTER,0
	  MOVE.W    D7,D6		;D6 = POS OF CENTER, POS OF '['
	  SWAP	    D6			;D6 = POS OF '[', POS OF CENTER

*	  CHECK CENTER PORTION.	 ONLY LETTER MATCHING IS ALLOWED.
*	  CONTINUE CHECKING UNTIL ']' (RIGHT CONTEXT DELIMITER)
*	  IS FOUND.

CLTR2	  MOVE.B    0(A6,D6),D5		;GET RULE CHAR
	  ADDQ.W    #1,D6		;BUMP RULE INDEX
	  CMP.B	    #']',D5		;END OF CENTER CONTEXT?
	  BEQ.S	    CENTRTN		;YES, CHECK LEFT CONTEXT
	  CMP.B	    (A3)+,D5		;RULE CHAR MATCH ENG CHAR?
	  BEQ.S	    CLTR2		;YES, CONTINUE WITH NEXT RULE CHAR

CENTRTN	  RTS

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	     CHECK 'LEFT' PORTION OF RULE		  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CHKLEFT	  SWAP	    D6			;POS OF RIGHT CONTEXT, POS OF '['

CHKLEFT1  SUBQ.W    #1,D6		;MOVE LEFT ONE CHAR
	  BMI.S	    CHKLEFTR		;DONE, RETURN WITH GOOD RC
	  MOVE.B    0(A6,D6),D5		;GET RULE CHAR
	  MOVE.W    D5,D4
	  LSL.B	    #1,D4		;FEATURE MATRIX INDEX
	  MOVE.W    0(A4,D4),D4		;GET FEATURES
	  BTST	    #KEYWORDBIT,D4	;SPECIAL SYMBOL?
	  BNE.S	    LSYMBOL		;YES
	  CMP.B	    -(A2),D5		;NO, COMPARE WITH LEFT CHAR
	  BEQ.S	    CHKLEFT1		;IF MATCH, CONTINUE
	  RTS
CHKLEFTR  MOVE.W    #$04,CCR		;SET SUCCESSFUL RC
	  RTS

LSYMBOL	  MOVEQ	    #1,D7		;SET LEFT MOVING FETCH
	  BSR	    SYMHDLR		;GO DO SYMBOL CHECKING
	  BNE.S	    CHKLEFT1		;RULE WAS SUCCESSFUL
	  MOVE.W    #$00,CCR		;UNSUCCESSFUL RC
	  RTS



*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	     CHECK 'RIGHT' PORTION OF RULE		  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CHKRIGHT  MOVE.L    4(SP),A2		;GET RIGHT POINTER INTO A2
	  SWAP	    D6			;RESTORE RIGHT CONTEXT POINTER
*					;   IN LOWER HALF OF D6
CHKR1	  MOVE.B    0(A6,D6),D5		;GET RULE CHAR
	  ADDQ.W    #1,D6
	  CMP.B	    #'=',D5		;END OF RIGHT CONTEXT?
	  BEQ.S	    CHKRRTN		;YES, DO REPLACEMENT
	  MOVE.W    D5,D4
	  LSL.B	    #1,D4
	  MOVE.W    0(A4,D4),D4		;GET FEATURES
	  BTST	    #KEYWORDBIT,D4	;SPECIAL SYMBOL?
	  BNE.S	    RSYMBOL		;YES, TRY SYMBOL
	  CMP.B	    (A2)+,D5		;NO, COMPARE WITH RULE CHAR
	  BEQ.S	    CHKR1		;IF MATCH, CONTINUE CHECKING
CHKRRTN	  RTS

RSYMBOL	  MOVEQ	    #-1,D7		;SET RIGHT FETCHES
	  BSR	    SYMHDLR		;GO DO SYMBOL CHECKING
	  BNE.S	    CHKR1		;RULE WAS SUCCESSFUL
	  MOVE.W    #$00,CCR		;SET RC (BAD)
	  RTS


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	EVERYTHING IS JUST GREAT, DO REPLACEMENT	  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

REPLACE:
*	  FIND LENGTH OF REPLACEMENT TEXT

	  MOVE.W    D6,D7		;CURRENT POS
	  SUBQ.W    #1,D7		;BACKUP IN PREPARATION
REPL1	  ADDQ.W    #1,D7		;MOVE FORWARD ONE CHAR
	  MOVE.B    0(A6,D7),D0		;GET RULE CHAR
	  CMP.B	    #'\',D0		;AT END?
	  BEQ.S	    REPL1X		;YES
	  CMP.B	    #'`',D0		;AT END?
	  BNE.S	    REPL1		;NO, CONTINUE LOOKING
REPL1X	  SUB.W	    D6,D7		;COMPUTE LENGTH OF REPL TEXT
	  BNE.S	    REPL2		;
	  RTS				;ZERO REPLACEMENT LENGTH

*	  INSURE THAT REPLACEMENT TEXT WILL FIT IN OUTPUT

REPL2	  MOVE.L    A1,D0		;GET CURRENT POS
	  SUB.L	    STOUTOFF(A5),D0	;COMPUTE NUMBER OF BYTES IN OUTPUT
	  ADD.W	    D7,D0		;NEW LENGTH
	  CMP.W	    D2,D0		;BIGGER THAN BLOCK SIZE?
	  BGE.S	    OVERFLOW		;YES, DO OVERFLOW PROCESSING

*	  NEW TEXT FITS IN OUTPUT

DOREPLCE  AND.L	    #$0000FFFF,D6	;CLEAR UPPER HALF OF D6
	  ADD.L	    D6,A6		;POINT A6 TO BEG OF REPL TEXT
	  MOVE.W    D7,-(SP)		;SAVE LENGTH OF REPLACEMENT
	  BRA.S	    REPBLOOP		;BEGIN LOOP
REPLOOP	  MOVE.B    (A6)+,(A1)+		;MOVE CHARS
REPBLOOP  DBF	    D7,REPLOOP		;LOOP FOR LENGTH OF REPL TEXT
	  MOVE.W    (SP)+,D7		;RESTORE REPLACEMENT LENGTH
* If last rule did not output a letter, don't flip accent flag
	MOVE.L	D0,-(SP)
	MOVEQ	#0,D0
	MOVE.B	-1(A1),D0	;get last char of output
	ADD.W	D0,D0
	MOVE.W	0(A4,D0),D0
	BTST	#LETTERBIT,D0	;a letter?
	BEQ.S	REPLOOP1	;no, don't flip flag
* If last rule was an exception, clear the accent flag
	BSET	#0,ACCFLAG(A5)
	CMP.B	#'`',(A6)	;from an exception?
	BNE.S	REPLOOP1	;no, branch
	BCLR	#0,ACCFLAG(A5)	;yes, clear accent flag
REPLOOP1 MOVE.L (SP)+,D0
	RTS
*

OVERFLOW:
	  MOVEQ	    #0,D0
	  MOVE.B    -1(A1),D0		;WAS LAST CHAR INSERTED A WORD BREAK?	
	  LSL.W	    #1,D0
	  MOVE.W    0(A4,D0),D0
	  BTST	    #WORDBRKBIT,D0
	  BNE.S	    OVERFL1		;YES
OVERFL2	  MOVEQ	    #0,D0
	  MOVE.B    -(A1),D0
	  LSL.W	    #1,D0
	  MOVE.W    0(A4,D0),D0
	  BTST	    #WORDBRKBIT,D0
	  BEQ.S	    OVERFL2
	  LEA	    1(A1),A1
	  
OVERFL1	  MOVE.B    #0,(A1)+		;PUT IN A NULL
*	   MOVE.B    #0,(A1)		;NULL TERMINATE STRING
	  SUB.L	    D3,A0		;ADJUST ENGLISH STRING POINTER
	  MOVE.L    A0,D0
	  SUB.L	    STINPOFF(A5),D0
	  NEG.L	    D0	  
	  MOVE.L    D0,D1
	  MOVE.L    D0,D7
	  RTS


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	    GIVEN A SPECIAL SYMBOL IN D5, FIND AND	  ;
*	    EXECUTE THE APPROPRIATE ROUTINE		  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SYMHDLR	  LEA	    SYMLIST,A3		;LIST OF SPECIAL SYMBOLS
	  MOVE.W    #SYMCNT-1,D0
SYMHDLR1  CMP.B	    (A3)+,D5
	  DBEQ	    D0,SYMHDLR1
	  NEG.W	    D0
	  ADD.W	    #SYMCNT-1,D0
	  LSL.W	    #2,D0		;FOUND OFFSET TO SPECIAL SYMBOL
	  LEA	    SYMBASE,A3		;GET BASE OF SYMBOL ROUTINES

	  MOVE.L    0(A3,D0),A3		;GET SPECIFIC SYMBOL ROUTINE

	  JSR	    (A3)		;GO DO IT
	  RTS				;AND RETURN



*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*      FETCHES A CHARACTER FROM THE 'WORD' BUFFER	  ;
*      INTO D1 AND ASSOCIATED FEATURES INTO D4.		  ;
*      ON ENTRY D7 CONTAINS A +IVE VALUE IF THE CHAR	  ;
*      IS TO BE FETCHED FROM THE LEFT AND A -IVE VALUE	  ;
*      IF THE FETCHING IS TO BE DONE FROM THE RIGHT	  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

FETCH	  TST.W	    D7			;CHECK D7
	  BMI.S	    FETCH1		;-IVE, LOOK TO THE RIGHT
	  MOVE.B    -(A2),D1		;GET CHAR ON LEFT
	  BRA.S	    FETCH2		;GO GET FEATURES
FETCH1	  MOVE.B    (A2)+,D1		;GET CHAR ON RIGHT
FETCH2	  MOVEQ	    #0,D4
	  MOVE.B    D1,D4
	  LSL.B	    #1,D4
	  MOVE.W    0(A4,D4),D4		;GET FEATURES
	  RTS



*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*      THE FOLLOWING SUBROUTINES HANDLE CHECKS FOR	  ;
*      THE VARIOUS SPECIAL SYMBOLS THAT COULD APPEAR	  ;
*      IN A RULE.  ALL SET CC = 0 IF THE RULE DOES NOT	  ;
*      APPLY, AND INSURE CC != 0 IF RULE DOES APPLY.	  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


*
*	  CHECK FOR A SINGLE VOWEL
*

SVOWEL	  BSR.S	    FETCH		;GET CHAR (AND FEATURES) FROM ENG
	  BTST	    #VOWELBIT,D4	;DO TEST FOR VOWEL
	  RTS


*
*	  CHECK FOR ONE OR MORE CONSONANT
*

OMCONS	  BSR.S	    FETCH		;GET NEXT CHAR
	  BTST	    #CONSBIT,D4
	  BEQ.S	    OMCONS1		;NOT A CONSONANT, RULE FAILS
OMCONS0	  BSR.S	    FETCH		;AT LEAST ONE CONS, KEEP LOOKING
	  BTST	    #CONSBIT,D4		;
	  BNE.S	    OMCONS0		;GET EM ALL
	  MOVE.W    #$08,CCR		;SET N BIT, CLEAR Z BIT, GOOD RTN
OMCONS1	  RTS
*
*	  CHECK FOR A SINGLE VOICED CONSONANT
*

SVCONS	  BSR.S	    FETCH		;GET CHAR AND FEATURES
	  BTST	    #VOICEDBIT,D4	;VOICED?
	  BEQ.S	    SVCONS1		;NO, RETURN
	  BTST	    #CONSBIT,D4		;CONSONANT?
SVCONS1	  RTS				;RETURN, RC IS IMPLICITLY SET


*
*	  CHECK FOR A SINGLE CONSONANT FOLLOWED BY AN 'I' OR 'E'
*

SCONSIE	  BSR.S	    FETCH		;GET CHAR AND FEATURES
	  BTST	    #CONSBIT,D4		;CONSONANT?
	  BEQ.S	    SCONSIE2		;NO, RETURN
	  BSR.S	    FETCH		;GET NEXT CHAR AND FEATURES
	  CMP.B	    #'I',D1		;IS IT AN 'I'?
	  BEQ.S	    SCONSIE1		;YES
	  CMP.B	    #'E',D1		;IS IT AN 'E'?
	  BEQ.S	    SCONSIE1		;YES
	  MOVE.W    #$04,CCR		;NO, BAD RETURN, SET Z BIT
	  RTS				;   AND RETURN
SCONSIE1  MOVE.W    #$08,CCR		;SET N BIT, GOOD RETURN
SCONSIE2  RTS


*
*	  CHECK FOR ONE OF THE FOLLOWING SUFFIXES:
*	  E, ES, ED, ER, ING, ELY, ERS OR INGS.
*

SUFFIXES  BSR.S	    FETCH
	  CMP.B	    #'E',D1		;IS IT AN E?
	  BNE.S	    SUFFI		;NO, TRY AN I ENDING SUFFIX
	  BSR.S	    FETCH
	  BTST	    #LETTERBIT,D4
	  BEQ.S	    SUFFOK
	  CMP.B	    #'S',D1
	  BEQ.S	    SUFFCK		;ES
	  CMP.B	    #'D',D1
	  BEQ.S	    SUFFCK		;ED
	  CMP.B	    #'R',D1
*	  BEQ.S	    SUFFCK		;ER		!!!sam 10/26/85!!!
	  BEQ.S	    SUFFS		;ER		!!!sam 10/26/85!!!
	  CMP.B	    #'L',D1
	  BNE.S	    SUFFNO
	  BSR	    FETCH
	  CMP.B	    #'Y',D1
	  BEQ.S	    SUFFCK		;ELY
	  BRA.S	    SUFFNO
SUFFI	  BSR	    FETCH
	  CMP.B	    #'N',D1
	  BNE.S	    SUFFNO
	  BSR	    FETCH
	  CMP.B	    #'G',D1
	  BNE.S	    SUFFNO		;ING
SUFFS	  BSR	    FETCH				!!!sam 10/26/85!!!
	  BTST	    #LETTERBIT,D4			!!!sam 10/26/85!!!
	  BEQ.S	    SUFFOK				!!!sam 10/26/85!!!
	  CMP.B	    #'S',D1				!!!sam 10/26/85!!!
	  BNE.S	    SUFFNO		;INGS OR ERS	!!!sam 10/26/85!!!
SUFFCK	  BSR	    FETCH
	  BTST	    #LETTERBIT,D4
	  BEQ.S	    SUFFOK
SUFFNO	  MOVE.W    #$04,CCR		;BAD RETURN
	  RTS
SUFFOK	  MOVE.W    #$08,CCR		;GOOD RETURN
	  RTS


*
*	  CHECK FOR A SIBILANT
*

SIBIL	  BSR	    FETCH
	  BTST	    #SIBILBIT,D4
	  RTS


*
*	  CHECK FOR A CONSONANT WITH A U AFFECTATION
*

CUAFF	  BSR	    FETCH
	  BTST	    #UAFFBIT,D4
	  RTS


*
*	  CHECK FOR A SINGLE CONSONANT
*

SCONS	  BSR	    FETCH
	  BTST	    #CONSBIT,D4
	  RTS


*
*	  CHECK FOR A FRONT VOWEL
*

FVOWEL	  BSR	    FETCH
	  BTST	    #FRONTBIT,D4
	  RTS


*
*	  CHECK FOR ZERO OR MORE CONSONANT
*

ZMCONS	  BSR	    FETCH
	  BTST	    #CONSBIT,D4
	  BNE.S	    ZMCONS
	  ADD.L	    D7,A2		;FIXUP A2
	  MOVE.W    #$08,CCR		;GOOD RC
	  RTS


*
*	  CHECK FOR A SINGLE NUMERAL (DIGIT)
*

SNUM	  BSR	    FETCH
	  BTST	    #DIGITBIT,D4
	  RTS


*
*	  CHECK FOR ZERO OR MORE NUMERALS (DIGITS)
*

ZMNUMS	  BSR	    FETCH
	  BTST	    #DIGITBIT,D4
	  BNE.S	    ZMNUMS
	  ADD.L	    D7,A2		;FIXUP A2
	  MOVE.W    #$08,CCR		;GOOD RETURN
	  RTS


*
*	  CHECK FOR A NON LETTER
*

NONLTR	  BSR	    FETCH
	  BTST	    #LETTERBIT,D4	;LETTER BIT ON?
	  BNE.S	    NONLTR1		;YES, RULE FAILS
	  MOVE.W    #$08,CCR		;RULE CHECKS, SET GOOD RETURN
	  RTS
NONLTR1	  MOVE.W    #$04,CCR		;RULE FAILS, SET BAD RETURN
	  RTS




*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*		      PREPROCESSOR			  ;
*							  ;
*      CURRENTLY THE PREPROCESSOR ISOLATES WORDS,	  ;
*      CHECKS TO SEE THAT THE CHARACTERS ARE VALID	  ;
*      ASCII CODES, AND CONVERTS LOWER CASE TO UPPER	  ;
*      CASE.  THE LENGTH OF THE WORD IS RETURNED IN	  ;
*      D3.						  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PREPROC	  LEA	    WORDBOFF(A5),A2	;POINTER TO WORD BUFFER
	  MOVE.B    0(A2,D3),(A2)+	;GET LAST CHAR OF PREVIOUS WORD
	  MOVE.B    #' ',(A2)+		;MOVE IN A BLANK
	  MOVEQ	    #0,D0		;CLEAR SOME REGS
	  MOVEQ	    #0,D1
	  MOVEQ	    #0,D3
	  TST.L	    INLENOFF(A5)
	  BLE.S	    PREATEND
PREMAIN	  MOVE.B    D1,D0
	  MOVE.B    (A0)+,D1		;GET CHAR FROM ENGLISH STRING

	  BNE.S	    PRENOTNUL		;   IF NEXT CHAR IS A NULL, PROCESS
	  MOVE.L    #1,INLENOFF(A5)	;   THIS WORD, THEN TERMINATE INPUT

PRENOTNUL CMP.B	    #' ',D1		;CHECK AGAINST ASCII BLANK
	  BGE.S	    PREAT0		;GREATER OR EQUAL, USE CHAR
	  MOVE.B    #' ',D1		;NOT AN ASCII CHAR, SUBST A BLANK
PREAT0	  CMP.B	    #$7F,D1		;CHECK AGAINST DEL CHAR
	  BNE.S	    PREAT00		;NOT A DEL
	  MOVE.B    #' ',D1		;NOT AN ASCII CHAR, SUBST A BLANK
PREAT00	  CMP.B	    #'a',D1		;CHECK FOR LOWER CASE
	  BLT.S	    PRENOTLC
	  CMP.B	    #'z',D1
	  BGT.S	    PRENOTLC
	  SUB.B	    #'a'-'A',D1		;IF LOWER CASE, CONVERT TO UC

PRENOTLC  CMP.B	    #']',D1		;RT BRACKET (RULE DELIM) MUST BE
	  BNE.S	    PREAT1		;   REMOVED TO PREVENT DEATH
	  MOVE.B    #' ',D1		;   INSERT A BLANK IN ITS PLACE
PREAT1	  ADDQ.W    #1,D3		;BUMP COUNT OF LETTERS IN WORD
	  CMP.W	    #MAXLTRS,D3		;OVERFLOW?
	  BGT.S	    PREERROR
	  MOVE.B    D1,(A2)+		;MOVE CHAR TO WORD
	  CMP.B	    #' ',D0		;WAS LAST CHAR A BLANK
	  BEQ.S	    PREWRDND		;YES, RETURN
	  SUBQ.L    #1,INLENOFF(A5)	;REDUCE COUNT OF REMAINING LTRS
	  BEQ.S	    PREGONE		;IF ZERO, INSERT '#' AND GO AWAY
	  CMP.B	    #'#',D1		;HAVE WE REACHED END OF INPUT STRING
	  BNE.S	    PREMAIN		;NO, CONTINUE
	  CMP.B	    #'#',D0		;MAYBE, LOOK FURTHER
	  BNE.S	    PREMAIN		;NO, CONTINUE
	  CMP.B	    #2,D3		;IF ONLY 2
	  BNE.S	    PRETEST
PREATEND:
	  MOVE.B    #'#',(A1)+
	  MOVEQ	    #0,D3
	  RTS

PRETEST	  SUBQ.L    #1,A0		;YES, BACK UP TO '##'S IN ENGLISH
	  MOVE.B    #' ',-2(A2)		;ADD A BLANK AFTER LAST WORD
	  SUBQ.W    #1,D3		;FIXUP COUNT OF LETTERS

PREWRDND  SUBQ.L    #1,A0		;A0 --> 1ST LETTER OF NEXT WORD
	  SUBQ.W    #1,D3		;REDUCE LENGTH OF WORD
	  RTS

PREERROR  MOVEQ	    #-3,D1		;SET FOR BAD RETURN
	  RTS

PREGONE	  MOVE.B    #' ',(A2)+
	  MOVE.B    #' ',(A2)+
	  RTS


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*								    ;
*		      ACCENTING POSTPROCESSER			    ;	
*   Accent the first syllable of the previous word if not accented  ; 
*   already and if last replace did not come from an exception.	    ;
*								    ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ACCENTER: 
	BTST	#0,ACCFLAG(A5)	;should we accent?
	BEQ.S	SQUEAK1		;no, bypass
	MOVEM.L D0/D1/A0/A2/A3,-(SP) ;save regs (never touch A1)
	MOVE.L	A1,D0		;compute total len of replacements
	SUB.L	OUTPTR(A5),D0
	CMP.W	#3,D0		;at least 3 chars?
	BLT.S	SQUEAK		;no, get out
	MOVE.L	OUTPTR(A5),A0	;yes, look for a number
SCANUM	MOVEQ	#0,D0
	MOVE.B	(A0)+,D0	;get a char
	ADD.W	D0,D0		;shift for feat index
	MOVE.W	0(A4,D0),D0	;get feature
	BTST	#DIGITBIT,D0	;a number?
	BNE.S	SQUEAK		;yes, get out
	CMP.L	A0,A1		;scan whole word?
	BGT.S	SCANUM		;no, keep scanning
	MOVE.L	OUTPTR(A5),A0	;yes, find first vowel (if any)
	LEA	VOWTBL,A2	;get vowel table address
FINDVOW1 MOVE.B (A0)+,D0	;get 1st char
	LSL.W	#8,D0		;shift to hi byte
	MOVE.B	(A0),D0		;get 2nd char
	MOVEQ	#NUMVOW-1,D1
	MOVE.L	A2,A3		;point to vowel table
FINDVOW CMP.W	(A3)+,D0	;a vowel?
	BEQ.S	ITSAVOW		;yes, branch
	DBF	D1,FINDVOW	;no, keep looking
	CMP.L	A0,A1		;searched whole word?
	BGT.S	FINDVOW1	;no, get next letter pair

* Ran out of letters - no vowel in word.

	BRA.S	SQUEAK		;get out

* First vowel located - insert an accent number

ITSAVOW ADDQ.L	#1,A0		;point to insertion spot
	MOVE.L	A1,A2		;point to word end
INSRT1	MOVE.B	-(A2),1(A2)	;move right portion over one
	CMP.L	A2,A0
	BNE.S	INSRT1
	MOVE.B	#'4',(A0)	;put in number
	ADDQ.L	#1,A1		;increment output pointer
*
SQUEAK	MOVEM.L (SP)+,D0/D1/A0/A2/A3
SQUEAK1 MOVE.L	A1,OUTPTR(A5)	;save new starting point
*
SQUEAK2 RTS

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	  SPECIAL SYMBOL ROUTINES JUMP TABLE		  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


SYMBASE	  DC.L	    SVOWEL		;POUND
	  DC.L	    OMCONS		;STAR
	  DC.L	    SVCONS		;PERIOD
	  DC.L	    SCONSIE		;DOLLAR
	  DC.L	    SUFFIXES		;PERCENT
	  DC.L	    SIBIL		;AMPERSAND
	  DC.L	    CUAFF		;AT SIGN
	  DC.L	    SCONS		;CARET
	  DC.L	    FVOWEL		;PLUS
	  DC.L	    ZMCONS		;COLON
	  DC.L	    SNUM		;QUESTION
	  DC.L	    ZMNUMS		;UNDERBAR
	  DC.L	    NONLTR		;BLANK


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*		 FEATURE MATRIX				  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


FEATURES  DC.W	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	    ;CONTROL
	  DC.W	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	    ;	CODES
	  DC.W	    SYMBOL+KEYWORD+WORDBRK		    ;BLANK
	  DC.W	    SYMBOL+WORDBRK		;!
	  DC.W	    SYMBOL		;'
	  DC.W	    SYMBOL+KEYWORD+WORDBRK	;#
	  DC.W	    SYMBOL+KEYWORD	;$
	  DC.W	    SYMBOL+KEYWORD	;%
	  DC.W	    SYMBOL+KEYWORD	;&
	  DC.W	    SYMBOL		;'
	  DC.W	    SYMBOL+WORDBRK		;(
	  DC.W	    SYMBOL+WORDBRK		;)
	  DC.W	    SYMBOL+KEYWORD	;*
	  DC.W	    SYMBOL+KEYWORD	;+
	  DC.W	    SYMBOL+WORDBRK		;,
	  DC.W	    SYMBOL+WORDBRK		;-
	  DC.W	    SYMBOL+KEYWORD+WORDBRK	;.
	  DC.W	    SYMBOL		;/
	  DC.W	    DIGIT		;0
	  DC.W	    DIGIT		;1
	  DC.W	    DIGIT		;2
	  DC.W	    DIGIT		;3
	  DC.W	    DIGIT		;4
	  DC.W	    DIGIT		;5
	  DC.W	    DIGIT		;6
	  DC.W	    DIGIT		;7
	  DC.W	    DIGIT		;8
	  DC.W	    DIGIT		;9
	  DC.W	    SYMBOL+KEYWORD	;:
	  DC.W	    SYMBOL		;;
	  DC.W	    SYMBOL		;<
	  DC.W	    SYMBOL		;=
	  DC.W	    SYMBOL		;>
	  DC.W	    SYMBOL+KEYWORD+WORDBRK	;?
	  DC.W	    SYMBOL+KEYWORD	;@
	  DC.W	    LETTER+VOWEL		  ;A
	  DC.W	    LETTER+CONS+VOICED		  ;B
	  DC.W	    LETTER+CONS+SIBILANT	  ;C
	  DC.W	    LETTER+CONS+VOICED+UAFF	  ;D
	  DC.W	    LETTER+VOWEL+FRONT		  ;E
	  DC.W	    LETTER+CONS			  ;F
	  DC.W	    LETTER+CONS+VOICED+SIBILANT	  ;G
	  DC.W	    LETTER+CONS			  ;H
	  DC.W	    LETTER+VOWEL+FRONT		  ;I
	  DC.W	    LETTER+CONS+VOICED+SIBILANT+UAFF  ;J
	  DC.W	    LETTER+CONS			  ;K
	  DC.W	    LETTER+CONS+VOICED+UAFF	  ;L
	  DC.W	    LETTER+CONS+VOICED		  ;M
	  DC.W	    LETTER+CONS+VOICED+UAFF	  ;N
	  DC.W	    LETTER+VOWEL		  ;O
	  DC.W	    LETTER+CONS			  ;P
	  DC.W	    LETTER+CONS			  ;Q
	  DC.W	    LETTER+CONS+VOICED+UAFF	  ;R
	  DC.W	    LETTER+CONS+SIBILANT+UAFF	  ;S
	  DC.W	    LETTER+CONS+UAFF		  ;T
	  DC.W	    LETTER+VOWEL		  ;U
	  DC.W	    LETTER+CONS+VOICED		  ;V
	  DC.W	    LETTER+CONS+VOICED		  ;W
	  DC.W	    LETTER+CONS+SIBILANT	  ;X
	  DC.W	    LETTER+VOWEL+FRONT		  ;Y
	  DC.W	    LETTER+CONS+VOICED+SIBILANT+UAFF  ;Z
	  DC.W	    SYMBOL		;[
	  DC.W	    SYMBOL		;\
	  DC.W	    SYMBOL		;]
	  DC.W	    SYMBOL+KEYWORD	;^
	  DC.W	    SYMBOL+KEYWORD	;_
	  DC.W	    SYMBOL		;`
	  DC.W	    0,0,0,0,0,0,0,0,0,0,0,0,0	  ;LOWER CASE
	  DC.W	    0,0,0,0,0,0,0,0,0,0,0,0,0	  ;LETTERS
	  DC.W	    SYMBOL		;{
	  DC.W	    SYMBOL		;|
	  DC.W	    SYMBOL		;}
	  DC.W	    SYMBOL		;~
	  DC.W	    0			;DEL



*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							  ;
*							  ;
*	SPECIAL SYMBOLS USED IN LETTER TO SOUND RULES	  ;
*							  ;
*							  ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SYMLIST	  DC.B	    '#'	      ;1 OR MORE VOWELS
	  DC.B	    '*'	      ;1 OR MORE CONSONANTS
	  DC.B	    '.'	      ;A VOICED CONSONANT
	  DC.B	    '$'	      ;SINGLE CONSONANT FOLLOWED BY 'I' OR 'E'
	  DC.B	    '%'	      ;SUFFIX (E, ES, ED, ER, ING, ELY)
	  DC.B	    '&'	      ;A SIBILANT
	  DC.B	    '@'	      ;A CONSONANT FOLLOWED BY A LONG U
	  DC.B	    '^'	      ;A SINGLE CONSONANT
	  DC.B	    '+'	      ;A FRONT VOWEL (I, E, Y)
	  DC.B	    ':'	      ;0 OR MORE CONSONANTS
	  DC.B	    '?'	      ;A SINGLE DIGIT
	  DC.B	    '_'	      ;0 OR MORE DIGITS
	  DC.B	    ' '	      ;NON-LETTER [never actually compared in dbeq loop]

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*							 ;
*	 LIST OF VOWELS FOR ACCENTING ROUTINE		 ;
*							 ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

NUMVOW	EQU	15
VOWTBL	DC.W	'IH'
	DC.W	'EH'
	DC.W	'AA'
	DC.W	'AE'
	DC.W	'IY'
	DC.W	'AO'
	DC.W	'AH'
	DC.W	'ER'
	DC.W	'OH'
	DC.W	'EY'
	DC.W	'AY'
	DC.W	'OY'
	DC.W	'AW'
	DC.W	'OW'
	DC.W	'UW'


	INCLUDE 'ltrrules.i'

************************


	  END 


