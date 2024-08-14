
	TTL	'PHONOL.ASM'


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
*                   PHONOLOGICAL RULE INTERPRETER                            ;
*                                                                            ;
*                                                                            ;
*    A0 - ADDRESS OF RULE SET        A1 - ADDRESS OF PHON ARRAY              ;
*    A2 - STRESS ARRAY               A3 - DURATION ARRAY                     ;
*    A4 - FEATURE MATRICES           A5 - GLOBALS AREA                       ;
*    A6 - MUST CONTAIN ADDRESS OF RULE TABLE ON ENTRY                        ;
*                                                                            ;
*    D0 - INDEX INTO RULE SET        D1 - INDEX INTO PHON ARRAY              ;
*    D2 - PHONEME CODE               D3 - SCRATCH                            ;
*    D4 - SCRATCH                    D5 - SCRATCH                            ;
*    D6 - SCRATCH                    D7 - SCRATCH                            ;
*                                                                            ;
*                                                                            ;
* Note 1  Oct 10,1989  JK  --	Changed LEA's to MOVE.L's 		     ;
* Note 2  Oct 10,1989  JK  --	Commented out tests for overflow	     ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	  INCLUDE   'exec/types.i'
          INCLUDE   'featequ.i'
          INCLUDE   'gloequs.i'
	  INCLUDE   'pcequs.i'
          INCLUDE   'phonrules.i'


          XREF      FEAT
          XDEF      PHONOL


*-------- PHONOL Equs

PHORULLN  EQU          3      ;OFFSET TO RULE LENGTH
PHOOPTNS  EQU          3      ;OFFSET TO RULE OPTIONS
PHOKEYPC  EQU          0      ;OFFSET TO KEY (CENTER) PC
PHOLFTPC  EQU          1      ;OFFSET TO LEFT CONTEXT PC
PHORTPC   EQU          2      ;OFFSET TO RIGHT CONTEXT PC
PHOLFSPC  EQU          7      ;OPTION BIT FOR OPTIONAL-SPACE-ON-LEFT-OK
PHORTSPC  EQU          6      ;OPTION BIT FOR OPTIONAL-SPACE-ON-RIGHT-OK
PHORPLPC  EQU          4      ;OFFSET TO REPLACEMENT PC
PHOINSBF  EQU          5      ;OFFSET TO LEFT INSERTION PC
PHOINSAF  EQU          6      ;OFFSET TO RIGHT INSERTION PC
PHORULOF  EQU          7      ;OFFSET TO BEGINNING OF FEATURE BASED RULES
PHOLASTB  EQU          7      ;BIT SPECIFYING LAST FEATURE BASED RULE
PHOONOFF  EQU          5      ;BIT SPECIFYING WHETHER BIT TO BE TESTED SHOULD BE
*                              ;    ON OR OFF
PHOBIT    EQU          6      ;BIT SPECIFYING BIT TEST (ON) OR STRESS (OFF)
PHOCONT   EQU          5      ;BIT ON SPECIFIES THAT OTHER RULES SHOULD BE TRIED


*-------- Initialize

PHONOL    LEA       PHRULES1,A6		;GET ADDRESS OF PHON RULES
	  MOVE.L    PHON(A5),A1         ;GET ADDRESS OF PHON ARRAY	* Note 1
          MOVE.L    STRESS(A5),A2       ;GET ADDRESS OF STRESS ARRAY	* Note 1
          MOVE.L    DUR(A5),A3          ;GET ADDRESS OF DUR ARRAY	* Note 1
          LEA       FEAT,A4             ;GET ADDRESS OF FEATURE MATRICES

          MOVEQ     #-1,D0
          MOVEQ     #0,D1
          MOVEQ     #0,D2
          MOVEQ     #0,D3
          MOVEQ     #0,D4
          MOVEQ     #0,D7


*-------- Main phonological rules interpreter loop

PHOMAIN   MOVE.L    A6,A0               ;GET ADDRESS OF RULES TABLE
          ADDQ.W    #1,D1               ;BUMP INDEX INTO RULE SET
PHONXTST  MOVE.B    0(A1,D1),D2         ;GET/RESTORE PHONEME CODE
          CMP.B     #$FF,D2             ;END OF STRING?
          BEQ       PHODONE             ;YES, EXIT

          MOVEQ     #0,D0               ;CLEAR INDEX INTO RULE SET


*-------- Test for match of key (center) portion of rule

          MOVE.B    PHOKEYPC(A0),D3     ;GET KEY PC
          CMP.B     #$FF,D3             ;X'FF' MATCHES ALL
          BEQ.S     PHOMATCH
          CMP.B     D2,D3               ;DOES PHONEME CODE MATCH RULE?
          BEQ.S     PHOMATCH            ;YES


*-------- Get next rule

PHONXRUL  MOVE.B    PHORULLN(A0),D4     ;GET RULE SET LENGTH/OPTIONS
          AND.B     #$0F,D4             ;ISOLATE RULE SET LENGTH
          BEQ.S     PHOMAIN             ;HAVE GONE THRU ALL RULE SETS
          ADD.L     D4,A0               ;UPDATE A0 TO NEXT RULE SET
          BRA.S     PHONXTST            ;BEGIN TESTING WITH NEW RULE SET


*-------- Matched key portion, check left context

PHOMATCH  MOVE.B    PHOLFTPC(A0),D3     ;GET LEFT CONTEXT PC RULE
          CMP.B     #$FF,D3             ;
          BEQ.S     PHOMATLC            ;X'FF' MATCHES ALL
          CMP.B     -1(A1,D1),D3        ;DOES LEFT CONTEXT PC MATCH INPUT?
          BNE.S     PHONXRUL            ;NO, TRY NEXT RULE SET


*-------- Matched left context, check right context

PHOMATLC  MOVE.B    PHORTPC(A0),D3      ;GET RIGHT CONTEXT PC RULE
          CMP.B     #$FF,D3             ;X'FF' MATCHES ALL
          BEQ.S     PHOMATRT
          CMP.B     #$FF,1(A1,D1)       ;END OF PHON ARRAY?
          BEQ.S     PHONXRUL            ;YES. CANNOT POSSIBLY MATCH RIGHT CONTEXT
          CMP.B     1(A1,D1),D3         ;RIGHT CONTEXT MATCH PC?
          BNE.S     PHONXRUL            ;NO, TRY NEXT RULE SET


*-------- All phoneme code rules have applied, check feature based rules

*-------- Check center context rules

PHOMATRT  MOVE.B    0(A2,D1),D4         ;GET STRESS CODE (ALREADY HAVE PC IN D2)
          BSR       PHODORLS            ;CHECK KEY CONTEXT RULES
          BNE.S     PHONXRUL            ;A RULE DID NOT APPLY, TRY NEXT RULE SET


*-------- Center context rules applied, check left context

          MOVE.B    -1(A2,D1),D4                  ;GET STRESS FROM LEFT CONTEXT
          MOVE.B    -1(A1,D1),D2                  ;GET P.C. FROM LEFT CONTEXT
          BNE.S     PHODOLFT                      ;NOT SPACE, CHECK LEFT CONTEXT RULES
          BTST      #PHOLFSPC,PHOOPTNS(A0)        ;CHECK IF OPTIONAL SPACE ON LEFT OK
          BEQ.S     PHODOLFT                      ;NO, TRY RULE WITH SPACE
          MOVE.B    -2(A2,D1),D4                  ;GET STRESS RESIDING BEFORE SPACE
          MOVE.B    -2(A1,D1),D2                  ;GET P.C. RESIDING BEFORE SPACE
PHODOLFT  BSR       PHODORLS                      ;CHECK LEFT CONTEXT RULES
          BNE.S     PHONXRUL                      ;SOME RULE OF THE LEFT CONTEXT
*                                                 ;A RULE DID NOT APPLY, TRY NEXT RULE

*-------- Left context rules applied, check right context

          MOVE.B    1(A2,D1),D4                   ;GET STRESS FROM RIGHT CONTEXT
          MOVE.B    1(A1,D1),D2                   ;GET P.C. FROM RIGHT CONTEXT
          BNE.S     PHODORT                       ;NOT SPACE, CHECK RIGHT CONTEXT
          BTST      #PHORTSPC,PHOOPTNS(A0)        ;OPTIONAL SPACE ON RIGHT OK?
          BEQ.S     PHODORT                       ;NO, TRY RULE WITH SPACE
          MOVE.B    2(A2,D1),D4                   ;GET STRESS FROM P.C. AFTER SPACE
          MOVE.B    2(A1,D1),D2                   ;GET P.C. RESIDING AFTER SPACE
PHODORT   CMP.B     #$FF,D2                       ;END OF PHON ARRAY?
          BNE.S     PHODORT1                      ;  NO, TRY TO APPLY RULE
          CLR.B     D2                            ;  YES, ZERO OUT PC.
*                                                  ;       THE ONLY RULE THAT WILL
*                                                  ;       APPLY IS 'MATCH ALL'
*                                                  ;       RULE.  (KLUDGE)
PHODORT1  BSR       PHODORLS                      ;CHECK RIGHT CONTEXT RULES
          BNE       PHONXRUL                      ;A RULE DID NOT APPLY, TRY NEXT RULE


*-------- All rules have applied, do the appropriate thing

*         Replacement PC

PHOAPPLY  MOVE.B    PHORPLPC(A0),D3     ;GET REPLACEMENT PC
          CMP.B     #$FF,D3             ;X'FF' IMPLIES NO REPLACE
          BEQ.S     PHOAPLY0            ;SKIP REPLACE
          MOVE.B    D3,0(A1,D1)         ;DO REPLACEMENT


*         Left insertions

PHOAPLY0  MOVE.B    PHOINSBF(A0),D3     ;GET PHONEME TO PRE-INSERT
          CMP.B     #$FF,D3             ;
          BEQ.S     PHOAPLY1            ;X'FF' IMPLIES NO INSERT
          SUBQ.W    #1,D1               ;ADJUST INDEX FOR PRE-INSERTION
          BSR       PHOINSRT            ;DO INSERT
*          BVC.S     PHOAPLYA
*          RTS                           ;IF OVERFLOW, RETRUN
PHOAPLYA  ADDQ.W    #1,D1               ;ADJUST INDEX TO AFTER KEY PHONEME


*         Right insertion

PHOAPLY1  MOVE.B    PHOINSAF(A0),D3     ;GET PHONEME TO POST-INSERT
          CMP.B     #$FF,D3             ;
          BEQ.S     PHOAPLY2            ;X'FF' IMPLIES NO INSERT
          BSR       PHOINSRT            ;DO INSERT
*          BVC.S     PHOAPLY2
*          RTS                           ;IF OVERFLOW, RETURN


*-------- Check to see if we should continue with rules or start at beginning

PHOAPLY2  BTST      #PHOCONT,PHOOPTNS(A0)         ;
          BNE       PHONXRUL                      ;CONTINUE WITH NEXT RULE
          BRA       PHOMAIN                       ;START AT BEGINNING


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                                                            ;
*                                                                            ;
*           SUBROUTINE TO CHECK FEATURE AND STRESS BASED RULES               ;
*                                                                            ;
*           ON ENTRY:  D2 CONTAINS PC       D4 CONTAINS STRESS BYTE          ;
*                                                                            ;
*                                                                            ;
*                                                                            ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


PHODORLS  LSL.W     #2,D2                         ;CONVERT PC INTO FM INDEX
          MOVE.L    0(A4,D2),D3                   ;FEATURE MATRIX
          LSR.W     #2,D2                         ;RESTORE PC IN D2
PHODORLC  MOVE.B    PHORULOF(A0,D0),D5            ;GET RULE

*-------- There are two types of tests; one to determine if the stress of
*         phoneme in question matches the rule, the other checks for
*         specific features to be present or absent.

          BTST      #PHOBIT,D5          ;FEATURE TEST?
          BNE.S     PHOBITST            ;YES
          MOVE.B    D4,D3               ;NO, MOVE STRESS TO D3


*-------- Combined stress and feature tests

PHOBITST  BTST      #PHOONOFF,D5        ;TEST FOR BIT ON OR OFF
          BEQ.S     PHOBITOF            ;BIT IS SUPPOSED TO BE OFF
          NOT.L     D3                  ;INVERT FEATURE MATRIX
PHOBITOF  BTST      D5,D3               ;BIT TEST CORRECTLY SETS RC
          BNE.S     PHODONE             ;TEST FAILED, RETURN WITH NON-ZERO RC


*-------- Try next feature/stress rule

          ADDQ.W    #1,D0               ;BUMP INDEX INTO RULE SET
          EOR.B     #$FF,D5             ;FLIP RULE (KLUDGE FOR RC)
          BTST      #PHOLASTB,D5         ;LAST RULE IN RULE SET?
          BNE.S     PHODORLS            ;NO, TEST NEXT RULE

PHODONE   RTS


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                                                            ;
*         PHONOL INSERT ROUTINE.  RIGHT NOW DOES NOT HANDLE OVERFLOWS        ;
*                                                                            ;
*         A1 - PHON ARRAY              A2 - STRESS ARRAY                     ;
*         A5 - DURATION ARRAY          A3 - TEMP                             ;
*         A4 -                         A5 -                                  ;
*         A6 -                         A7 - STACK POINTER                    ;
*                                                                            ;
*         D1 - INDEX INTO ARRAYS       D7 - LENGTH OF ARRAYS/MOVE AMOUNT     ;
*         D3 - PHONEME CODE TO INSERT                                        ;
*                                                                            ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


PHOINSRT  MOVEM.L   A0-A3,-(SP)         ;SAVE SOME REGS

          MOVEQ     #0,D7
          MOVE.W    PHONLEN(A5),D7      ;GET LENGTH OR ARRAYS

*          CMP.W     #MAXPHONL,D7        ;OVERFLOW?		*** Note 2
*          BLT.S     PHOINAT1            ;NO
*          MOVE.W    #$02,CCR            ;YES, SET OVERFLOW
*          BRA.S     PHOINSRN            ;     AND RETURN

PHOINAT1  ADDQ.W    #1,PHONLEN(A5)      ;BUMP LENGTH OF ARRAYS
          ADD.L     D7,A1               ;POINT TO END OF PHON ARRAY
          ADD.L     D7,A2               ;                STRESS ARRAY
          ADD.L     D7,A3               ;                DURATION ARRAY
          SUB.W     D1,D7               ;COMPUTE LENGHT OF PORTION TO MOVE
          SUBQ.W    #2,D7               ;ADJUST PROPERLY FOR DBF
*                                        ;   REMEMBER TWO ZEROS AT BEGINNING
PHOINSLP  MOVE.B    -(A1),1(A1)         ;MOVE PHON ARRAY
          MOVE.B    -(A2),1(A2)         ;MOVE STRESS ARRAY
          MOVE.B    -(A3),1(A3)         ;MOVE DURATION ARRAY
          DBF       D7,PHOINSLP         ;LOOP FOR COUNT

          MOVE.B    D3,(A1)             ;INSERT PHONEME CODE
          CLR.B     (A2)                ;CLEAR STRESS ARRAY
          CLR.B     (A3)                ;CLEAR DURATION ARRAY
          ADDQ.W    #1,D1               ;ADJUST INDEX INTO PHON ARRAY

PHOINSRN  MOVEM.L   (SP)+,A0-A3         ;RESTORE REGISTERS

          RTS

          END 


