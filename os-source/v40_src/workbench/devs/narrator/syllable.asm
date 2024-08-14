
	TTL	'SYLLABLE.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


          SECTION      speech


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                                                            ;
*                            SYLLABLE MARKER                                 ;
*                                                                            ;
*                                                                            ;
*    A0 - ADDRESS OF PHON ARRAY      A1 - ADDRESS OF STRESS ARRAY            ;
*    A2 - ADDRESS OF DURATION ARRAY  A3 - ADDRESS OF FEATURE MATRICES        ;
*    A4 -                            A5 - GLOBALS AREA                       ;
*    A6 -                            A7 - STACK POINTER                      ;
*                                                                            ;
*    D0 - INDEX INTO PHON ARRAY      D1 - PHONEME CODE                       ;
*    D2 - START OF SYL POSITION      D3 - PHONEME FEATURE                    ;
*    D4 - POSITION OF FIRST VOWEL    D5 - SCRATCH                            ;
*         IN SYLLABLE                                                        ;
*    D6 - FEATURES TO BE 'OR'D' IN   D7 -                                    ;
*         TO STRESS AND DUR ARRAYS                                           ;
*                                                                            ;
*
*    CHANGE RECORD:
*
*	1/??/89  JK --	QX now has PAUSE bit set in featmat.  This causes
*			SYLLABLE to mark the blank after the QX that
*			PARSE inserts at the beginning of the input string
*			as "start of syllable", causing mis-alignment of 
*			syllables.  Put in test at SYLBOUND to insure blank
*			is not marked.  (Note 1)
*			
*
*	4/17/89  JK --	Phrase final used to be set only if syllable was 
*			followed by some punctuation mark.  Now we also
*			set it (from vowel to end of syl) if the syllable
*			immediately precedes a P-UNIT terminator.  (Note 2)
*
* 
*	6/25/89  JK --  Starting syllable processing after 1st QX in PHON
*			array.  Moves start of syllable and start of word
*			to first "real" phoneme. (Note 3) removed.
*
*	10/10/89 JK --	Changed LEA's to MOVE.L's for PHON, STRESS, and DUR
*			pointer initialization to reflect change from fixed
*			buffers to allocated memory.
*
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	  INCLUDE     'exec/types.i'
          INCLUDE     'featequ.i'
          INCLUDE     'pcequs.i'
          INCLUDE     'gloequs.i'

          XREF        FEAT
          XDEF        SYLLABLE


*-------- Syllable bits contained in the STRESS array

SYLSTART  EQU       $80       ;START OF SYLLABLE
SYLPOLY   EQU       $40       ;POLY-SYLLABLIC
SYLSTRD   EQU       $20       ;STRESSED PHONEME
SYLSGMT   EQU       $10       ;STRESSED SEGMENT


*-------- Syllable bits contained in the DUR array

SYLWORDF  EQU       $80       ;WORD FINAL
SYLPHRSF  EQU       $40       ;PHRASE FINAL
SYLPUNTI  EQU	    $20	      ;P-UNIT INITIATOR
SYLPUNTT  EQU	    $10	      ;P-UNIT TERMINATOR


*-------- Initialize

SYLLABLE  MOVE.L    PHON(A5),A0         ;GET ADDRESS OF PHON ARRAY
          MOVE.L    STRESS(A5),A1       ;GET ADDRESS OF STRESS ARRAY
          MOVE.L    DUR(A5),A2          ;GET ADDRESS OD DUR ARRAY
          LEA       FEAT,A3             ;GET ADDRESS OF FEATURE MATRICES

          MOVEQ     #2,D0               ;START PHON AFTER 1ST "QX" (Note 3)
          MOVEQ     #0,D1
          MOVEQ     #3,D2		;DUR LEADS PHON BY 1	   (Note 3)
          MOVEQ     #0,D3
          MOVEQ     #0,D4
          MOVEQ     #0,D5
          MOVEQ     #0,D6


*-------- Syllable marking loop

SYLNEXT   ADDQ.W    #1,D0               ;BUMP INDEX INTO INPUT ARRAYS
          MOVEQ     #0,D1               ;CLEAR PHONEME CODE REGISTER
          MOVE.B    0(A0,D0),D1         ;GET PHONEME CODE
          BEQ       SYLBOUND            ;SPACE
          CMP.B     #$FF,D1             ;
          BEQ       SYLDONE             ;END OF INPUT STRING
*         CMP.B     #PHCQ,D1            ;
*         BEQ       SYLGLOTL            ;GLOTTAL STOP
          LSL.W     #2,D1               ;CONVERT PC INTO FM INDEX
          MOVE.L    0(A3,D1),D3         ;GET PHONEME FEATURES
          BTST      #PAUSEBIT,D3        ;
          BNE       SYLBOUND            ;BOUND
          BTST      #VOWELBIT,D3        ;
          BEQ.S     SYLNEXT             ;NOTHING INTERESTING, KEEP LOOKING


*-------- Found a vowel (treat LX and RX as non-vowels)

SYLVOWEL  LSR.W     #2,D1               ;RESTORE PHONEME CODE
          CMP.B     #PHCLX,D1           ;CHECK FOR LX
          BEQ.S     SYLNEXT             ;      ....IGNORE
          CMP.B     #PHCRX,D1           ;CHECK FOR RX
          BEQ.S     SYLNEXT             ;      ....IGNORE


*-------- Not an LX or RX, treat as a real vowel

          TST.W     D4
          BNE.S     SYLVOWL2            ;THIS IS THE SECOND VOWEL IN A WORD
          MOVE.W    D0,D4               ;FIRST OR ONLY VOWEL, MARK IT
          BRA.S     SYLNEXT             ;  AND CONTINUE


*-------- Found two vowels in a word.  Find syllable break.
*         Compute the number of segments to mark.

SYLVOWL2  MOVE.W    D0,D5               ;D5=CURRENT POSITION (POS)
          SUB.W     D4,D5               ;   POS-POSITION OF PREV VOWEL (VL)
          SUBQ.W    #1,D5               ;   POS-VL-1
          LSR.W     #1,D5               ;   (POS-VL-1)/2
          ADD.W     D4,D5               ;   (POS-VL-1)/2 + VL
          SUB.W     D2,D5               ;   (POS-VL-1)/2 + VL - POS OF 1ST
*                                       ;                       SEGMENT (P1S)


*-------- Setup syllable features.  Note that if D4=0 (no previous vowel)
*         then pick up byte from the beginning of the STRESS array.  This
*         will always be zero, insuring an unstressed vowel.

SYLVSET   MOVE.B    0(A1,D4),D6         ;GET STRESS BYTE
          AND.B     #SYLSGMT,D6         ;ISOLATE STRESSED SEGMENT BIT
          LSL.W     #1,D6               ;MOVE TO STRESSED SYLLABLE POSITION
          OR.B      #SYLPOLY,D6         ;SET POLYSYLLABIC BIT


*-------- Get phoneme to be marked as start of syllable.
*         LX and RX are never marked.

          MOVE.B    0(A0,D2),D4         ;GET PHON TO BE MARKED
          CMP.B     #PHCLX,D4           ;LX?
          BEQ.S     SYLUGX              ;YES, MARK NEXT PHON
          CMP.B     #PHCRX,D4           ;NO, RX?
          BNE.S     SYLIGX              ;YES  , MARK NEXT PHON
SYLUGX    OR.B      D6,0(A1,D2)         ;TRANSFER POLYSYLLABIC BIT, IF SET
          ADDQ.W    #1,D2               ;BUMP START OF SYL POINTER
          SUBQ.W    #1,D5               ;DECREASE NUMBER OF SYLS TO MARK
SYLIGX    OR.B      #SYLSTART,0(A1,D2)  ;MARK START OF SYLLABLE


*-------- Fill in syllable bits in STRESS array (DUR array not changed)

SYLVLOOP  OR.B        D6,0(A1,D2)      ;SET APPROPRIATE BITS
          ADDQ.W      #1,D2
          DBF         D5,SYLVLOOP      ;DO LOOP

          MOVE.W      D0,D4            ;UPDATE POSITION ON LAST VOWEL
          BRA         SYLNEXT


*-------- Found word boundary, possibly end of phrase

SYLBOUND  MOVE.B    0(A0,D2),D5         ;GET PHON TO BE MARKED	
	  BEQ.S	    SYLBLANK		;DO NOT MARK BLANK  		* Note 1
          CMP.B     #PHCLX,D5           ;LX?
          BEQ.S     SYLUGX2             ;YES, MARK NEXT PHON
          CMP.B     #PHCRX,D5           ;NO, RX?
          BNE.S     SYLIGX2             ;YES, MARK NEXT PHON
SYLUGX2   ADDQ.W    #1,D2               ;BUMP START OF SYL POINTER
SYLIGX2   OR.B      #SYLSTART,0(A1,D2)  ;MARK START OF SYLLABLE
SYLBLANK  MOVEQ     #0,D5
          TST.W     D4
          BEQ.S     SYLBSKIP            ;IF D4=0, SKIP WORD FINAL MARKING
          OR.B      #SYLWORDF,0(A2,D4)  ;MARK VOWEL AS WORD FINAL
          MOVE.B    0(A1,D4),D5         ;GET STRESS OF LAST VOWEL
          AND.B     #SYLSGMT,D5         ;ISOLATE STRESSED SEGMENT BIT
          LSL.W     #1,D5               ;MOVE TO STRESSED SYLLABLE POSITION
          BCLR      #5,D6               ;RESET STR SYL BIT, KEEP POLY SYL
          OR.B      D5,D6               ;COMBINE STR AND POLY BITS
          MOVE.W    D0,D5               ;D5=POS
          SUB.W     D2,D5               ;  =POS - P1S
          SUBQ.W    #1,D5               ;  =POS - P1S - 1
SYLBLP1   OR.B      D6,0(A1,D2)         ;SET SYL BITS IN STRESS ARRAY
          ADDQ.W    #1,D2
          DBF       D5,SYLBLP1


*-------- If bound (as opposed to space), mark phrase final bits.
*	  Also done if we've encountered a P-UNIT terminator (right paren).

          BTST      #PAUSEBIT,D3        ;PAUSE BIT ON?
          BNE.S     SYLPHFIN            ;DO PHRASE FINAL CODE      	* Note 2
	  BTST	    #4,-1(A2,D2)	;P-UNIT TERMINATOR?	       	* Note 2
	  BEQ.S	    SYLBSKIP		;NO, SKIP PHRASE FINAL CODE    	* Note 2

SYLPHFIN  MOVE.W    D4,D2               ;RESET D2 TO LAST VOWEL POSITION
          MOVE.W    D0,D5               ;D5=POS
          SUB.W     D4,D5               ;  =POS - VL
          SUBQ.W    #1,D5               ;  =POS - VL - 1

SYLBLP2   BSET      #6,0(A2,D2)         ;SET PHRASE FINAL BIT IN DUR ARRAY
          ADDQ.W    #1,D2
          DBF       D5,SYLBLP2


*-------- Setup for continuing syllable search

SYLBSKIP  MOVE.W    D0,D2               ;!!!sam 10/16/85!!!
          ADDQ.W    #1,D2
          MOVEQ     #0,D4               ;RESET LAST VOWEL POSITION
          MOVE.W    D4,D6               ;CLEAR POLYSYLLABIC BIT
          BRA       SYLNEXT             ;CONTINUE WITH NEXT PHONEME


*-------- Handle glottal stops

SYLGLOTL  MOVE.W    D0,D5               ;COMPUTE AMOUNT TO MARK
          SUB.W     D2,D5
          SUBQ.W    #1,D5
          BRA       SYLVSET             ;GO DO WORK

*-------- Return

SYLDONE   RTS                           ;RETURN

          END 



