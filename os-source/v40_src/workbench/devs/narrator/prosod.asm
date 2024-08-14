
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************
	TTL	'prosod.asm'
              SECTION      speech

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                ;
*    MAIN MODULE 'PROSOD'        ;
*                                ;
* MODULE FETCHES AND MODIFIES    ;
* PHONEME DURATIONS BASED ON     ;
* CONTEXTUAL RULES               ;
*                                ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
*    REGISTER USAGE
*
* D0=PCT  D1=PCT1  D2=STRESS,DUR  D3=FEATURE  D4=SCRATCH
* D5=FEAT INDEX    D6=SCRATCH     D7=SCRATCH
*
* A0=PHON PTR  A1=DUR PTR  A2=PARMS+MINDUR  A3=STRESS PTR
* A4=FEATURE MATRIX  A5 VARS  A6=specific mindur
*
*
	far code
	far data

	INCLUDE	   'exec/types.i'
        INCLUDE    'featequ.i'
        INCLUDE    'gloequs.i'
        INCLUDE    'pcequs.i'
*
         XDEF    PROSOD
         XREF    FEAT,_Parms
*
PROSOD   MOVE.L     PHON(A5),A0     ;INIT POINTERS
         MOVE.L     DUR(A5),A1      ;PHON, DUR & STRESS WILL TRACK
         MOVE.L     STRESS(A5),A3
         LEA     FEAT,A4
         LEA     _Parms,A2	;base address of parm structures
	 add.l	 #OF_MINDUR,a2	;point to 1st mindur element
         ADDQ.L  #2,A0       ;SKIP THE INITIAL BLANKS
         ADDQ.L  #2,A1
         ADDQ.L  #2,A3
PROS     MOVEQ   #0,D2
         MOVE.B  (A3),D2     ;GET STRESS
         SWAP    D2          ;MOVE TO HI WORD
         MOVE.B  (A1),D2     ;GET DUR BYTE (DUR PORTION = 0)
         MOVE.W  #8192,D0    ;PCT=100% (8192) TO BEGIN
         MOVEQ   #0,D5
         MOVE.B  (A0),D5     ;PHON TO D5
         CMP.B   #$FF,D5     ;END OF PHON STR?
         BNE.S   PROSA       ;NO, CONTINUE
         RTS     	     ;EXIT PROSOD

PROSA    LSL.W   #2,D5		;INDEX TO FEAT
         MOVE.L  0(A4,D5),D3	;FEATURE TO D3
         BTST    #IGNOREBIT,D3	;IGNORE?
         BNE.S   PROSB		;NO,
PROSC    BTST    #PAUSEBIT,D3	;PAUSE?
         BEQ.S   PROS2		;NO, DO RULES
	 cmp.b	 #PHCQX,(a0)	;yes, but if QX do rules
	 beq.s	 PROS2
         MOVEQ   #24,D6		;PAUSE, SET DUR=200MS (24 FR)
	 cmp.b	 #PHCDA,(a0)
	 bne.s	 PROSC1
	 moveq	 #12,d6		;length of a dash
	 bra	 PROSOD2
PROSC1	 btst	 #TERMBIT,d3	;terminal? (. or ?)
	 beq.s	 PROSOD2	;no, branch
	 moveq	 #8,d6		;yes, set dur=80ms (10 frames)
         BRA     PROSOD2
PROSB    ADDQ.L  #1,A1		;YES, INCR PTR ADDR'S
         ADDQ.L  #1,A0
         ADDQ.L  #1,A3
         BRA.S   PROS		;BRANCH FOR NEXT PHON DATA
*
* RULE1 NOT USED - RULE2 CLAUSE FINAL LENGTHENING
* PCT1=11469 (140%) IF SEGMENT IS IN A CLAUSE FINAL SYLL.
*
PROS2    BTST    #6,D2       ;CLAUSE FINAL?
         BEQ.S   PROS3AA     ;NO, NEXT RULE
         MOVE.W  #11469,D1   ;YES, PCT1=140%
         BSR     PCTMOD      ;PCT=(PCT1*PCT)/32
*
* RULE3A  A PHRASE FINAL LIQUID OR NASAL IS LENGTHENED
* BY PCT1=11469 (140%)
*
* THIS RULE HAS BEEN MODIFIED TO WORD FINAL LIQUIDS & NASALS
*
PROS3AA  MOVE.L  D3,D6       ;FEATURE TO SCRATCH
         AND.L   #LIQUID+NASAL,D6  ;LIQUID OR NASAL?
         BEQ.S   PROS4A      ;NO, NEXT RULE
         CMP.B   #8,1(A0)    ;NEXT ELEMENT A NON-PHON
         BGT.S   PROS4A
PROS3A   MOVE.W  #11469,D1   ;YES, PCT1=140%
         BSR     PCTMOD
*
PROS4A   BTST    #VOWELBIT,D3  ;VOWEL?
         BEQ     PROS6       ;NO, SKIP RULES 3,4,5,7
*
* RULE3 - NON-PHRASE FINAL SHORTENING
* SYLLABIC SEGMENTS ARE SHORTENED (PCT1=60%)
* IF NOT IN A PHRASE FINAL SYLLABLE
*
	BTST	#6,D2	;PHRASE FINAL?
	BNE	PROS4	;YES, BRANCH
	MOVE.W	#4915,D1 ;PCT1=60%
	BSR	PCTMOD	
*
* RULE4  NON-WORD FINAL SHORTENING
* PCT1=6963 (85%) IF NOT IN A WORD FINAL SYLLABLE
*
PROS4    BTST    #7,D2       ;WORD FINAL?
         BNE.S   PROS5       ;YES, NEXT RULE
         MOVE.W  #6963,D1    ;NO, PCT1=85%
         BSR     PCTMOD
*
* RULE5  POLYSYLLABIC SHORTENING
* PCT1=6554 (80%) IF IN A POLYSYLLABIC WORD
*
PROS5    BTST    #22,D2      ;POLYSYL?
         BEQ.S   PROS7       ;NO, NEXT RULE
         MOVE.W  #6554,D1    ;YES, PCT1=80%
         BSR     PCTMOD
*
* RULE7  UNSTRESSED SHORTENING
* PCT1=5734 (70%) IF UNSTRESSED VOWEL
* ALSO MINDUR=MINDUR/2 WHEN DUR IS COMPUTED
*
PROS7    BTST    #20,D2      ;STRESSED?
         BNE.S   PROS9       ;YES, NEXT RULE
         MOVE.W  #5734,D1    ;NO, PCT1=70%
         BSR     PCTMOD
*
* RULE9  POSTVOCALIC CONTEXT OF VOWELS
*
PROS9    MOVEQ   #0,D6       ;GET FEAT OF PHON+1
         MOVE.B  1(A0),D6    ;PHON+1 TO SCRATCH
         LSL.W   #2,D6       ;INDEX TO FEAT
         MOVE.L  0(A4,D6),D7 ;FEAT TO SCRATCH
* OPEN SYLLABLE, STRESSED, WORD FINAL -- PCT1=120%  (mod'd to stress not nec)
*						    (mod'd to 130%)
         BTST    #IGNOREBIT,D7  ;NOT A PHON?
         BNE.S   PROS9AA      ;YES, BRANCH
         BTST    #PAUSEBIT,D7 ;PAUSE?
         BEQ.S   PROS9A       ;NO, NEXT
PROS9AA:
*        BTST    #4,(A3)     ;PHON STRESSED?
*        BEQ.S   PROS9A       ;NO, NEXT
         MOVE.W  #10650,D1    ;YES, PCT1=130%
*        MOVE.W  #9830,D1     ;YES, PCT1=120%
         BSR     PCTMOD
         BRA     PROS9X      ;NEXT RULE
* BEFORE A VOICED FRIC -- PCT1=160%
* Much too much too much  PCT1=130%   (this mod removed)
PROS9A   BTST    #FRICBIT,D7 ;FRIC?
         BEQ.S   PROS9B      ;NO, NEXT
         BTST    #VOICEDBIT,D7  ;YES, VOICED?
         BEQ     PROS6A      ;NO, NEXT
         MOVE.W  #13107,D1   ;YES, PCT1=160%
*        MOVE.W  #10650,D1   ;YES, PCT1=130%
         BSR     PCTMOD
         BRA     PROS9X
* BEFORE A VOICED PLOSIVE -- PCT1=120%
PROS9B   BTST    #PLOSBIT,D7 ;VOICED PLOS?
         BEQ.S   PROS9C      ;NO, NEXT
         MOVE.W  #9830,D1    ;YES, PCT1=120%
         BSR     PCTMOD
         BRA.S   PROS9X
* BEFORE AN UNSTRESSED NASAL -- PCT1=85%
PROS9C   BTST    #NASALBIT,D7  ;NASAL?
         BEQ.S   PROS9D      ;NO, NEXT
         BTST    #4,1(A3)    ;YES, UNSTRESSED?
         BNE     PROS6A      ;NO, NEXT
         MOVE.W  #6963,D1    ;YES, PCT1=85%
         BSR     PCTMOD
         BRA.S   PROS9X
* BEFORE A VOICELESS PLOSIVE -- PCT1=70%
* Mod'd to PCT1=60%
PROS9D   BTST    #PLOSABIT,D7  ;PLOSA?
         BEQ.S   PROS6       ;NO, NEXT RULE
*        MOVE.W  #5734,D1    ;YES, PCT1=70%
         MOVE.W  #4915,D1    ;YES, PCT1=60%
         BSR     PCTMOD
*
* IF NON-PHRASE FINAL, CHANGE PCT1 TO BE 70 + 30 * PCT1
*
* Modified to capture more of these effects in non-phrase final positions
* PCT1 = 30 + 70 * PCT1
*
PROS9X	BTST	#6,D2	;CLAUSE FINAL?
	BNE	PROS10	;YES, BRANCH
*	MOVE.W	#2458,D1 ;NO, PCT1=30%
	MOVE.W	#5734,D1 ;NO, PCT1=70%
	BSR	PCTMOD
*	ADD.W	#5734,D0 ;AND ADD 70%
	ADD.W	#2458,D0 ;AND ADD 30%
	BRA	PROS10	
*
* RULE6  NON-INITIAL CONSONANT SHORTENING
* PCT1=6963 (85%) IF CONS. NOT WORD INITIAL
*
PROS6    BTST    #CONSBIT,D3 ;CONS?
         BEQ.S   PROS6A      ;NO, NEXT RULE
         CMP.B   #8,-1(A0)   ;YES, WORD INITIAL?
         BLS.S   PROS13      ;NO, PREV PHON NON-BLANK
         MOVE.W  #6963,D1    ;PREV PHON = BLANK
         BSR     PCTMOD
*
* RULE13   SPECIAL /S/ [plos] CLUSTER
* PCT1=14336 (175%) IF /S/ PRECEDES PLOSIVE
*
PROS13	cmp.b	#PHCS,(a0)	;/S/ ?
	bne.s	PROS14		;no, next rule
	btst	#PLOSBIT,d7	;plosive on right?
	bne.s	PROS13A		;yes, branch
	btst	#PLOSABIT,d7	;plosive aspirant on right?
	beq.s	PROS14		;no, next rule
PROS13A	move.w	#14336,d1	;yes, PCT1=175
	bsr	PCTMOD
	bra.s	PROS6A
*
* RULE14   SPECIAL [S] /plos/ CLUSTER
* PCT1=4915 (60%) IF PLOSIVE OR PLOSIVE ASPIRANT FOLLOWS /S/		
*
PROS14	cmp.b	#PHCS,-1(a0)	;/S/ to the left?
	bne.s	PROS6A		;no, next rule
	btst	#PLOSABIT,d3	;plosa?
	bne.s	PROS14A		;yes, branch
	btst	#PLOSBIT,d3	;plos?
	beq.s	PROS6A		;no, next rule
PROS14A	move.w	#4915,d1	;yes, PCT1=60
	bsr	PCTMOD
	bra.s	PROS7D		;skip liqgli testing
*
* RULE7C   UNSTRESSED PREVOCALIC LIQUID OR GLIDE
* PCT1=819 (10%)  mod'd to 30%
*
PROS6A   BTST    #20,D2      ;STRESSED?
         BNE.S   PROS10      ;YES, NEXT RULE
         MOVE.L  D3,D4       ;FEATURE TO SCRATCH
         AND.L   #LIQUID+GLIDE,D4  ;LIQUID OR GLIDE?
         BEQ.S   PROS7D      ;NO, NEXT RULE
         MOVEQ   #0,D4       ;CLEAR SCRATCH
         MOVE.B  1(A0),D4    ;GET PHON+1
         LSL.W   #2,D4       ;INDEX FOR FEAT
         BTST    #VOWELBIT,3(A4,D4)  ;PHON+1=VOWEL?
         BEQ.S   PROS10      ;NO, NEXT RULE
         MOVE.W  #2458,D1    ;YES, PCT1=30% (was 819 10%)
         BSR     PCTMOD
	 bra.s	 PROS10	     ;don't do additional unstressed shortening
*
* RULE7D  Unstressed shortening
* Other unstressed consonants are shortened by PCT1=70%
* This rule was wrongly omitted until 9/11/88
*
PROS7D	btst	#CONSBIT,d3	;consonant?
	beq.s	PROS10		;no, branch
	move.w	#5734,d1	;yes, PCT1=70%
	bsr	PCTMOD
*
* RULE10  SHORTENING IN CLUSTERS
* CLUSTERS IGNORE INTERVENING BLANKS, BUT NOT PUNCTUATION
* D6=PHON-1 FEATURE       D7=PHON+1 FEATURE
*
PROS10   MOVEQ   #0,D5
         MOVE.B  -1(A0),D5   ;PHON-1 CODE TO D5
         BNE.S   PROS10K     ;NOT A BLANK
         MOVE.B  -2(A0),D5   ;BLANK, GET NEXT PHON
PROS10K  LSL.W   #2,D5       ;INDEX FOR FEAT
         MOVE.L  0(A4,D5),D6 ;FEAT(PHON-1) TO D6
         MOVEQ   #0,D5
         MOVE.B  1(A0),D5    ;PHON+1 CODE TO D5
         BNE.S   PROS10L     ;NOT A BLANK
         MOVE.B  2(A0),D5    ;BLANK, GET NEXT PHON
PROS10L  LSL.W   #2,D5       ;INDEX FOR FEAT
         MOVE.L  0(A4,D5),D7 ;FEAT(PHON+1) TO D7
* VOWEL FOLLOWED BY A VOWEL -- PCT1=120%
         BTST    #VOWELBIT,D3  ;PHON=VOWEL?
         BEQ.S   PROS10C     ;NO, SKIP
         BTST    #VOWELBIT,D7  ;YES, PHON+1=VOWEL?
         BEQ.S   PROS10B     ;NO, NEXT
         MOVE.W  #9830,D1    ;YES, PCT1=120%
         BSR     PCTMOD
* VOWEL PRECEDED BY A VOWEL -- PCT1=70%
PROS10B  BTST    #VOWELBIT,D6  ;PHON-1=VOWEL?
         BEQ.S   PROSOD1     ;NO, SKIP
         MOVE.W  #5734,D1    ;YES, PCT1=70%
         BSR     PCTMOD
         BRA.S   PROSOD1
* CONS. SURROUNDED BY CONS'S -- PCT1=50%
PROS10C  BTST    #CONSBIT,D6 ;PHON-1=CONS?
         BEQ.S   PROS10E     ;NO, NEXT
         BTST    #CONSBIT,D7 ;YES, PHON+1=CONS?
         BEQ.S   PROS10DD    ;NO, NEXT
         MOVE.W  #4096,D1    ;YES, PCT=50%
         BSR     PCTMOD
         BRA.S   PROSOD1     ;RULES COMPLETE
* CONS. PRECEDED BY A CONS. -- PCT1=70%
PROS10D  BTST    #CONSBIT,D6 ;PHON-1=CONS?
         BEQ.S   PROS10E     ;NO, NEXT
PROS10DD MOVE.W  #5734,D1    ;YES, PCT1=70%
         BSR     PCTMOD
         BRA.S   PROSOD1
* CONS. FOLLOWED BY A CONS. -- PCT1=70%
PROS10E  BTST    #CONSBIT,D7 ;PHON+1=CONS?
         BNE.S   PROS10DD    ;YES, BRANCH BACK
*
* END OF RULES SECTION, APPLY DURATION FORMULA:
* DUR = ((INHDUR - MINDUR) * PCT) / 8192 + MINDUR
* IF SEGMENT IS UNSTRESSED THEN MINDUR = MINDUR / 2
*
PROSOD1  MOVEQ   #0,D5       ;3 CLEARS
	 moveq	 #0,d6
	 moveq	 #0,d7
         MOVE.B  (A0),D5     ;PHON TO SCRATCH
	 mulu	 #PARMSIZE,d5
	 move.l	 a2,a6		;parms+mindur  base address
	 add.l	 d5,a6		;specific mindur entry
	 move.b	 (a6),d7	;get mindur
	 move.b	 1(a6),d6	;get inhdur

         BTST    #20,D2      ;STRESSED SEGMENT?
         BNE.S   PROSOD3     ;YES, BRANCH
*         BTST    #LIQUIDBIT,D3 ;LIQUID?
*         BNE.S   PROSOD3     ;YES, DO NOT COMPRESS
*         BTST    #GLIDEBIT,D3  ;NO, GLIDE?
*         BNE.S   PROSOD3     ;YES, DO NOT COMPRESS
	 addq.w	 #1,d7	     ;round for divide
         LSR.W   #1,D7       ;NO, DIVIDE MINDUR BY 2
PROSOD3  SUB.W   D7,D6       ;D6 = IHNDUR - MINDUR
         MULU    D0,D6       ;MULTIPLY BY PCT (1/8192'S)
	 asl.l	 #3,d6		;justify product
	 add.l	 #$8000,d6	;round
	 swap	 d6
         ADD.W   D7,D6       ;ADD MINDUR
*
* RULE11: ADDITIVE LENGTHENING DUE TO PLOSIVE ASPIRATION.
* A STRESSED SONORANT PRECEDED BY A VOICELESS PLOSIVE IS
* LENGTHENED BY 25 msec.   (CHANGED TO STRESSED VOWEL)
*			   (Changed back to sonorant 8/88)
*			   (Stress requirement removed 8/88)
*
         BTST    #SONORBIT,D3  ;SONORANT?
         BEQ.S   PROSOD12    ;NO, SKIP
*        BTST    #20,D2      ;YES, STRESSED?
*        BEQ.S   PROSOD4     ;NO, SKIP
         MOVEQ   #0,D5
         MOVE.B  -1(A0),D5   ;PHON-1 TO SCRATCH
         LSL.W   #2,D5       ;INDEX FOR FEAT
         BTST    #3,2(A4,D5)  ;FEAT(PHON-1)=PLOSA?
         BEQ.S   PROSOD4     ;NO, SKIP
         ADD.W   #25,D6      ;YES, ADD 25msec
*
* RULE 12: Additive lengthening for affricates.
* MINDUR, INHDUR determine only the duration of closure.
* For CH: unstressed add 60, stressed with no following R add 100
*      J: unstressed add 50, stressed with no following R add 65
*
PROSOD12:
*	bra	PROSOD4		;skip affricate code

	btst	#AFFRICBIT,d3	;affricate?
	beq.s	PROSOD4		;no, branch
	move.b	(a0),d5		;get phon code
	move.b	1(a0),d1	;get phon+1 code
	moveq	#60,d0		;assume unstressed CH add 60
	btst	#20,d2		;stressed?
	beq.s	PROS12A		;no, branch
	cmp.b	#PHCRR,d1	;yes, does an R follow?
	beq.s	PROS12A		;yes, branch
	moveq	#100,d0		;no, add 100 instead
PROS12A	cmp.b	#PHCJ,d5	;J?
	bne.s	PROS12B		;no, branch
	moveq	#50,d0		;yes, assume unstressed J add 50
	btst	#20,d2		;stressed?
	beq.s	PROS12B		;no, branch
	cmp.b	#PHCRR,d1	;yes, does an R follow?
	beq.s	PROS12B		;yes, branch
	moveq	#65,d0		;no, add 65 instead
PROS12B	add.w	d0,d6		;do the add

*
* Convert from milliseconds to frames (8msec per frame)
*
PROSOD4	and.l	#$ffff,d6	;clear upper word
*	addq.w	#4,d6		;pre-round for division
*	divu	#9,d6		;divide msecs by 9 to get frames
	 lsr.w	 #3,d6	;divide msecs by 8 to get frames (should be 8.333)
         CMP.B   #$3F,D6     ;TOO BIG?
         BLE.S   PROSOD2     ;NO, BRANCH
         MOVE.B  #$3F,D6     ;YES, CLAMP DURATION AT $3F
PROSOD2  AND.B   #$C0,(A1)   ;CLEAR DURATION OF GARBAGE
         OR.B    D6,(A1)     ;PUT DURATION IN ARRAY
         BRA     PROSB       ;BRANCH TO PTR INCR'S
*
* SUBROUTINE PCTMOD - PERFORMS CALCULATION: PCT = (PCT * PCT1) / 8192
*       WITH ROUNDING.  PCT'S ARE ACTUALLY IN 1/8192'S
*
PCTMOD   MULU    D1,D0       ;MULTIPLY BY # OF 1/8192'S
	 asl.l	 #3,d0		;justify
	 add.l	 #$8000,d0	;round
	 swap	 d0
         RTS

         END 
