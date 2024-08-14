	TTL	'F0HLS.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


*************************************************************************
*      F0 HIGH LEVEL SYSTEM						*
*									*
*          ALL REGS DESTROYED						*
*									*
*          D3 - NUMBER OF AS               D4 - NUMBER OF SYLLABLES	*
*          D5 - POSITION OF FIRST AS       D6 - CLAUSE NUMBER		*
*									*
*									*
*   4/17/89 JK  --	Added test for manual F0 mode in code which	*
*			determines whether to set a stressed syllable	*
*			as an accented syllable.  Natural F0 requires	*
*			scaled stress number to be greater than 5,	*
*			Manual F0 mode makes syllable accented if it 	*
*			has any stress at all.  (Note 1 at F0CRAX2B)	*
*									*
*   1/5/91   JK  -- 	Added CLR.W	F0LASTAS(A5) to fix gurgling    *
*			problem.  See Log. 		Note 1		*
*									*
*   3/3/91   JK  --	Added CLR.W	F0NUMPHS(A5) to insure proper   *
*			count.	Note 2					*
*									*
*   3/4/91   JK  --	Added code to check for beginning of new 	*
*			sentence and if so, to clear F0NUMCLS.  Note 3	*
*									*
*************************************************************************

          SECTION    speech


	  INCLUDE   'assembly.i'
	  INCLUDE   'exec/types.i'
	  INCLUDE   'exec/nodes.i'
	  INCLUDE   'exec/lists.i'
	  INCLUDE   'exec/strings.i'
	  INCLUDE   'exec/initializers.i'
	  INCLUDE   'exec/memory.i'
	  INCLUDE   'exec/interrupts.i'
	  INCLUDE   'exec/ports.i'
	  INCLUDE   'exec/libraries.i'
	  INCLUDE   'exec/devices.i'
	  INCLUDE   'exec/io.i'
	  INCLUDE   'exec/tasks.i'
	  INCLUDE   'exec/resident.i'
	  INCLUDE   'hardware/custom.i'
	  INCLUDE   'hardware/dmabits.i'
	  INCLUDE   'exec/execbase.i'
	  INCLUDE   'exec/ables.i'
	  INCLUDE   'exec/errors.i'

          INCLUDE   'gloequs.i'
          INCLUDE   'featequ.i'
	  INCLUDE   'pcequs.i'
          INCLUDE   'f0equs.i'
  	  INCLUDE   'narrator.i'

          XREF       FEAT
          XDEF       F0HLS



F0HLS:

*	;------	If this is the beginning of a new sentence, we must clear
	;	F0NUMCLS.  We test the previous phon to see if it was either
	;	a '?' or a '.'.  No problem testing to the left of the first
	;	phon since F0INIT always clears F0NUMCLS.  (Note 3).

	MOVE.L    F0PHON(A5),A0       	;Get address of PHON array
	MOVE.B    -1(A0),D0		;Get previous phon
	CMP.B	  #PHCPE,D0		;Period?
	BEQ.S	  F0HLSNewSentence	;  Yes, this is a new sentence
	CMP.B	  #PHCQM,D0		;  No, was last phon a question mark?
	BNE.S	  F0HLSContinue		;  No, this is not a new sentence

F0HLSNewSentence:
	CLR.W	  F0NUMCLS(A5)		;New sentence, clear clause number


F0HLSContinue:
	ADDQ.W    #1,F0NUMCLS(A5)     	;INCREMENT COUNT OF CLAUSES
	CLR.W	  F0NUMPHS(A5)		;Clear number of phrases  (Note 2)

        BSR       F0CRACNT            	;CREATE ACCENT ARRAY
        BEQ.S     F0HLSRTN            	;CC  = 0 ==> END OF SENTENCE
*        BVS.S     F0HLSRTN            	;CC  = OVFL ==> OVERFLOW
        BSR       F0CRCRBR            	;CREATE CR/BREAK ARRAY
        BSR       F0LEVEL             	;CREATE LEVEL
        BSR       F0TUNE              	;CREATE TUNE
        BSR       F0SETCR             	;MODIFY CR/BREAK ARRAY AT CLAUSE BNDRIES
        MOVE.W    #$0,CCR             	;SET CC TO NON-ZERO

*         SAVE FOR USE IN LLS

F0HLSRTN  RTS                           ;RETURN CODE  = 0 IF END OF SENTENCE
*                                        ;            OVFL IF OVERFLOW
*                                        ;            +IVE IF SOMETHING TO SAY

*----------------------------------------------------------------------------
*
*         CREATE ACCENT ARRAY.
*
*----------------------------------------------------------------------------

F0CRACNT  MOVE.L    F0DUR(A5),A2        ;GET ADDRESS OF DUR ARRAY
          MOVE.L    F0STRESS(A5),A1     ;GET ADDRESS OF STRESS ARRAY
          MOVE.L    F0PHON(A5),A0       ;GET ADDRESS OF PHON ARRAY
          MOVE.L    F0ACCENT(A5),A4     ;GET ADDRESS OF ACCENT ARRAY
          LEA       FEAT,A3             ;GET ADDRESS OF FEATURE MATRICES

          MOVEQ     #0,D0
          MOVEQ     #0,D1
          MOVEQ     #0,D4
          MOVEQ     #-1,D5
          CLR.W     F0NUMAS(A5)         ;CLEAR ANY PREVIOUSLY STORED VALUE
          CLR.W     F0LASTAS(A5)				; Note 1

F0CRAX0   MOVE.B    (A0)+,D0            ;GET PHONEME CODE
          MOVE.B    (A1)+,D1            ;GET STRESS ELEMENT
          MOVE.B    (A2)+,D2            ;GET DUR ELEMENT
          MOVEQ     #0,D6               ;CLEAR ATTRIBUTES BITS


*-------- Check for end of input, bound, and word break

          CMP.B     #$FF,D0             ;END OF INPUT STRING?
          BEQ       F0CRAXEND           ;YES
          MOVE.W    D0,D3
          LSL.W     #2,D3               ;CONVERT TO FM INDEX
          MOVE.L    0(A3,D3),D3         ;GET FEATURES
          BTST      #WORDBRKBIT,D3      ;WORD BREAK
          BEQ.S     F0NOTWB             ;NO
          ADD.W     #1,F0NUMPHS(A5)     ;YES, INCREASE NUMBER OF WORDS
          OR.B      #F0WORDF,-1(A4,D4)  ;     SET WORD FINAL BIT IN PREV SYL
          BTST      #BOUNDBIT,D3        ;BOUND?
          BNE       F0CRAXEND           ;YES
          BRA.S     F0CRAX0             ;NO, KEEP ON GOING


*-------- Not at a word break, find start of syllable

F0NOTWB   TST.B     D1                  ;CHECK HIGH BIT OF STRESS ELEMENT
          BPL.S     F0CRAX0             ;NOT START OF SYLLABLE


*-------- Found beginning of syllable, determine accent and attributes

*          BTST      #F0WORDFB,-1(A4,D4) ;IS PREV SYLLABLE WORD FINAL?
*          BEQ.S     F0NOTWF             ;NO
F0NOTWF   BTST      #F0SSBIT,D1         ;STRESSED SYLLABLE?
          BEQ.S     F0CRAX4             ;BRANCH IF NO


*-------- Syllable has stress number, find stress number on the next vowel

F0CRAX2A  AND.B     #F0ACNTNO,D1        ;ISOLATE ACCENT NUMBER
          BNE.S     F0CRAX2B            ;FOUND STRESSED VOWEL, OTHERWISE...
          ADDQ.L    #1,A2               ;   BUMP DUR ARRAY
          ADDQ.L    #1,A0               ;   BUMP PHON ARRAY
          MOVE.B    (A1)+,D1            ;   GET NEXT STRESS
          BRA.S     F0CRAX2A            ;   AND KEEP LOOKING


*-------- Rescale stress number to lie between 1 and 13.  If syllable
*         is multi-syllabic, add 2.  If new stress is less than 5, do
*         not mark syllable as accented.  Changed to check if in manual 
*	  F0 mode.  If so, mark any stressed syllable as accented.

F0CRAX2B  MULU      #F0STRSCL,D1	  ;RESCALE STRESS TO LIE BETWEEN
          LSR.W     #7,D1		  ;    1 AND 13
          BTST      #F0POLY,-1(A1)        ;CHECK FOR MULT-SYLLABIC
          BEQ.S     F0CRAX2C              ;IF MULT-SYLLABIC,
          ADDQ.W    #2,D1                 ;    RAISE ACCENT BY TWO
F0CRAX2C  CMP.W	    #MANUALF0,F0MODE(A5)  ;IF MANUAL F0 MODE, MARK ALL	 *Note 1
	  BEQ.S	    F0CRAX2D		  ;    STRESSED SYLS AS ACCENTED *Note 1
	  CMP.B     #4,D1                 ;OTHERWISE, IF NEW STRESS < 5, TREAT
          BLE.S     F0CRAX4		  ;     SYL AS UNACCENTED


*-------- New stress is greater than 5, mark syllable as accented

F0CRAX2D  ADDQ.W    #1,F0NUMAS(A5)      ;INCREMENT NUMBER OF ACCENTED SYLLABLES
          MOVE.W    D4,F0LASTAS(A5)     ;UPDATE POS OF LAST 'AS' IN UTTERANCE
          OR.B      #F0STRSYL,D6        ;SET STRESSED SYL BIT IN ACCENT
F0CRAX3   TST.B     D5                  ;IF D5 IS POSITIVE, WE'VE ALREADY
          BPL.S     F0CRAX4		;   FOUND POSITION OF FIRST AS.
          MOVE.W    D4,D5               ;   OTHERWISE, SAVE POSITION OF FIRST AS


*-------- Combine accent and attribute bits

F0CRAX4   AND.B     #F0ACNTNO,D1        ;ISOLATE ACCENT NUMBER
          OR.B      D6,D1               ;COMBINE BITS AND ACCENT
          MOVE.B    D1,0(A4,D4)         ;SAVE IN ACCENT ARRAY


*-------- Increment count of syllables and check for overflow

          ADDQ.W    #1,D4		;INCREMENT NUM OF SYLS IN CLAUSE
	  BRA.S	    F0CRAX0		;CONTINUE FILLING ACCENT ARRAY



*-------- Fininshed, set some final bits and go away

F0CRAXEND OR.B      #F0CLAUSF,-1(A4,D4)           ;SET CLAUSE FINAL


*-------- If no accented syllables, set D5 to count of syllables.
*         This is done so rule F4B1 will correctly compute the peaks
*         for all unstressed syllables in clause initial positions.

          MOVE.W    F0NUMAS(A5),D3      ;GET NUMBER OF ACCENTED SYLS
          BNE.S     F0SOMEAS            ;AT LEAST ONE
          MOVE.W    D4,D5               ;NONE, SET D5 TO COUNT OF SYLS


*-------- Store vars for later use, and return

F0SOMEAS  MOVE.W    D5,F01STAS(A5)      ;SAVE POSITION OF FIRST AS
	  MOVE.W    D4,F0NUMSYL(A5)	;Save number of syls in this clause

          RTS


*----------------------------------------------------------------------------
*
*                          CREATE CR/BREAK ARRAY
*
*               DESTROYS AND RECREATES D4 (NUMBER OF SYLLABLES)
*               UPDATES POSITION OF PHON, STRESS, AND DUR ARRAYS
*
*-----------------------------------------------------------------------------

F0CRCRBR  MOVE.L    F0DUR(A5),A2        ;GET ADDRESS OF DUR ARRAY
          MOVE.L    F0STRESS(A5),A1     ;GET ADDRESS OF STRESS ARRAY
          MOVE.L    F0PHON(A5),A0       ;GET ADDRESS OF PHON ARRAY
          MOVE.L    F0CRBRK(A5),A6      ;GET ADDRESS OF CR/BRK ARRAY
          MOVE.L    F0ACCENT(A5),A4     ;GET ADDRESS OF ACCENT ARRAY
          LEA       FEAT,A3             ;GET ADDRESS OF FEATURE MATRICES

          MOVEQ     #0,D0
          MOVEQ     #0,D4

F0CRB00   MOVE.B    (A0)+,D0            ;GET PHONEME CODE
          MOVE.B    (A1)+,D1            ;GET STRESS ELEMENT
          MOVE.B    (A2)+,D2            ;GET DUR ELEMENT

          CMP.B     #$FF,D0             ;END OF INPUT STRING?
          BEQ.S     F0CRXFF             ;YES

          CMP.B     #PHCDA,D0           ;DASH?
          BNE.S     F0CRB01             ;NO, BRANCH AROUND
          OR.B      #$90,-1(A6,D4)      ;YES, CR=F+R CR#=1
          BRA.S     F0CRB02

F0CRB01   MOVE.W    D0,D3
          LSL.W     #2,D3               ;CONVERT TO FM INDEX
          MOVE.L    0(A3,D3),D3         ;GET PHONEME FEATURES
          BTST      #BOUNDBIT,D3        ;CHECK FOR BOUND
          BNE.S     F0CRBEND            ;BRANCH IF BOUND


*-------- Time to actually create break array

F0CRB02   TST.B     D1
          BPL.S     F0CRB03
          ADDQ.W    #1,D4

F0CRB03   BTST      #F0PUNITI,D2        ;BEGINNING OF P-UNIT?
          BEQ.S     F0NOTPUI            ;NO
          MOVE.B    #$02,-1(A6,D4)      ;YES, SET BREAK OF 2
          BRA.S     F0CRB00

F0NOTPUI  BTST      #F0PUNITT,D2        ;END OF P-UNIT?
          BEQ.S     F0CRB00             ;NO
          MOVE.B    -1(A6,D4),D1        ;YES, GET CR/BREAK ELEMENT
          AND.B     #$0F,D1             ;  ISOLATE BREAK, ASSIGN BRK OF -2
          LSL.B     #4,D1               ;  IF NO HIGHER BREAK IS ALREADY
          ASR.B     #4,D1               ;  ASSIGNED
          BEQ.S     F0NOTPI1
          CMP.B     #-2,D1
          BGT.S     F0CRB00
F0NOTPI1  OR.B      #$0E,-1(A6,D4)      ;SET BREAK AND PRESERVE ANY CR
          BRA.S     F0CRB00


*-------- Have reached end of input string as delimited by X'FF'.
*         Want F0PHON, F0STRESS, and F0DUR to point to the X'FF', not
*         to the char after.  Necessary when a DASH terminates the clause.

F0CRXFF   SUBQ.L    #1,A0               ;BACKUP PHON
          SUBQ.L    #1,A1               ;BACKUP STRESS
          SUBQ.L    #1,A2               ;BACKUP DUR


*-------- F0PHON, F0STRESS, and F0DUR now point one past the char
*         with BOUND attribute (or DASH).  Save the updated pointers

F0CRBEND  MOVE.L    A0,F0PHON(A5)       ;SAVE UPDATED PHON POSITION
          MOVE.L    A1,F0STRESS(A5)     ;SAVE UPDATED STRESS POSITION
          MOVE.L    A2,F0DUR(A5)        ;SAVE UPDATED DUR POSITION


*-------- Reposition break numbers to accented syllables within P-UNITS

          MOVEQ     #0,D7
          MOVE.W    D4,D0               ;GET LENGTH OF CLAUSE
          BRA.S     F0BRKDBF            ;START LOOP
F0BRKX0   OR.B      D7,0(A4,D0)         ;SET IN-PUNIT BIT IN ACCENT ARRAY
          MOVE.W    D0,D2
          CMP.B     #$02,0(A6,D0)
          BEQ.S     F0BRKX1             ;P-UNIT INITIATOR
          CMP.B     #$0E,0(A6,D0)
          BNE.S     F0BRKDBF            ;NOT P-UNIT TERMINATOR
          MOVEQ     #-1,D1              ;SET BACKWARD MOTION
          MOVE.B    #F0INPUN,D7         ;SET IN PUNIT BIT
          OR.B      D7,0(A4,D0)         ;SET IN ACCENT ARRAY
          BRA.S     F0BRKCMN            ;JUMP TO COMMON CODE
F0BRKX1   MOVEQ     #1,D1               ;SET FORWARD MOTION
          MOVEQ     #0,D7               ;TURN OFF IN-PUNIT BIT
F0BRKCMN  BTST      #F0SSBIT,0(A4,D2)   ;STRESSED?
          BNE.S     F0BRKGOT            ;YES, MOVE BREAK NUMBER
          ADD.W     D1,D2               ;NO, MOVE TO NEXT/PREV SYL
          BMI.S     F0BRKIGN            ;AT BEG OF CLAUSE, SKIP
          CMP.W     D2,D4               ;
          BGE.S     F0BRKCMN            ;NOT AT END OF CLAUSE, CONTINUE
F0BRKIGN  CLR.B     0(A6,D0)            ;CLEAR BREAK NUMBER
          BRA.S     F0BRKDBF            ;CONTINUE LOOPING
F0BRKGOT  MOVE.B    0(A6,D0),0(A6,D2)   ;MOVE BREAK NUMBER TO AS
          CMP.W     D0,D2
          BEQ.S     F0BRKDBF            ;DO NOT CLEAR IF ORIGINALLY ON AS
          CLR.B     0(A6,D0)            ;CLEAR ORIGINAL BREAK NUMBER
F0BRKDBF  DBF       D0,F0BRKX0
          RTS


*----------------------------------------------------------------------------
*
*         INITIALIZE LEVEL.  IN THIS VERSION OF THE HLS, ALL LEVELS
*         WILL BE SET TO ONE, IMPLYING NO LEVEL MODS ARE TO BE DONE
*
*----------------------------------------------------------------------------

F0LEVEL   MOVE.L    F0TUNELV(A5),A1 ;GET TUNE/LEVEL ARRAY
          MOVE.W    D4,D0           ;GET COUNT OF SYLLABLES
          BRA.S     F0LEVEL1        ;START LOOP
F0LEVEL0  OR.B      #1,0(A1,D0)     ;SET LEVEL=1 (NO LEVEL), PRESERVING ANY TUNE
F0LEVEL1  DBF       D0,F0LEVEL0     ;LOOP

          RTS

*----------------------------------------------------------------------------
*
*         MARK TUNE ON LAST WORD OF CLAUSE
*
*----------------------------------------------------------------------------

F0TUNE    MOVE.L    F0TUNELV(A5),A1   ;GET TUNE/LEVEL ARRAY POINTER
          MOVE.L    F0PHON(A5),A0     ;GET UPDATED PHON ARRAY POINTER
          MOVE.L    F0ACCENT(A5),A4   ;GET ACCENT ARRAY POINTER
          MOVEQ     #0,D1             ;DEFAULT IS NO TUNE
          MOVE.B    -1(A0),D0         ;GET LAST PHONEME CODE
          CMP.B     #PHCPE,D0         ;IS IT A "." ?
          BNE.S     F0TUNEX0          ;NO
          MOVE.B    #F0TUNEA,D1       ;YES, USE TUNE A
F0TUNEX0  CMP.B     #PHCQM,D0         ;IS IT A "?" ?
          BNE.S     F0TUNEX1          ;NO
          MOVE.B    #F0TUNEB,D1       ;YES, USE TUNE B
F0TUNEX1  MOVE.W    D4,D0             ;GET COUNT OF SYLLABLES
          SUBQ.W    #1,D0             ;ADJUST FOR DBF


*-------- Mark syllable with tune, moving backwards until either the end
*         of the previous word or the beginning of the clause is found.

F0TUNEX2  OR.B      D1,0(A1,D0)        ;SET TUNE, PRESERVE LEVEL IF PRESENT
          BTST      #F0WORDFB,0(A4,D0) ;TEST FOR WORD FINAL
F0TUNEX3  DBNE      D0,F0TUNEX2        ;LOOP IF NOT WF UNITL BEGINNING OF CLAUSE

          RTS


*----------------------------------------------------------------------------
*
*         CREATE CR AND BREAKS AT CLAUSE BOUNDRIES:  CR=2, BREAK=3
*         IF TUNE A:  CR=FALL+RISE  (TUNE A OR NO TUNE)
*         IF TUNE B:  CR=MONOTONIC
*
*         FORMAT OF CR/BREAK ENTRY:  BIT 7=CR TYPE
*                                    BITS 6-4=CR NUMBER
*                                    BITS 3-0=BREAK NUMBER
*
*----------------------------------------------------------------------------

F0SETCR   MOVE.L    F0CRBRK(A5),A2      ;GET CR/BREAK ARRAY POINTER
          MOVE.L    F0TUNELV(A5),A1     ;GET TUNE/LEVEL ARRAY POINTER
          MOVE.L    F0PHON(A5),A0       ;GET UPDATED PHON ARRAY POINTER
          MOVE.L    F0ACCENT(A5),A4     ;GET ACCENT ARRAY POINTER
          CMP.B     #PHCPE,-1(A0)       ;PERIOD?
          BEQ.S     F0SETX0             ;YES, SKIP CR/BREAK
          CMP.B     #PHCQM,-1(A0)       ;NO, QUESTION?
          BEQ.S     F0SETX0             ;YES, SKIP CR/BREAK
*         MOVE.B    #$A0,D0             ;CR=F+R, CR#=2   (OLD)
          MOVE.B    #$B0,D0             ;CR=F+R, CR#=3 (CHANGED)
          CMP.B     #F0TUNEB,-1(A1,D4)  ;CHECK FOR TUNE
          BNE.S     F0SETCR1
*         MOVE.B    #$20,D0             ;CR=MONO, CR#=2   (OLD)
          MOVE.B    #$30,D0             ;CR=MONO, CR#=3 (CHANGED)
F0SETCR1  OR.B      D0,-1(A2,D4)        ;SET CR, PRESERVE ANY BREAK PRESENT


*-------- Create break at clause boundries

F0SETBRK  TST.W     F0NUMAS(A5)         ;ANY AS'S?
          BEQ.S     F0SETX0             ;NO, SKIP THIS CODE

          MOVE.W    D4,D0
          SUBQ.W    #1,D0               ;ADJUST COUNT FOR DBF AND POSITION
F0SETBR0  BTST      #F0SSBIT,0(A4,D0)   ;AS?
          DBNE      D0,F0SETBR0         ;NO, CONTINUE LOOPING
          AND.B     #$F0,0(A2,D0)       ;  PRESERVE CR, CLEAR BREAK
*         OR.B      #$03,0(A2,D0)       ;  SET BREAK=3     (OLD)
          OR.B      #$04,0(A2,D0)       ;  SET BREAK=4   (CHANGED)

F0SETX0   RTS

          END 

