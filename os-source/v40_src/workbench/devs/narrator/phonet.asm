	TTL	'$Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/phonet.asm,v 1.3 90/08/20 10:57:33 andy Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
* $Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/phonet.asm,v 1.3 90/08/20 10:57:33 andy Exp $
*
* $Locker:  $
*
* $Log:	phonet.asm,v $
Revision 1.3  90/08/20  10:57:33  andy
new version from softvoice

Revision 1.2  90/04/03  12:07:25  andy
*** empty log message ***

Revision 1.1  90/04/02  16:40:00  andy
Initial revision

* Revision 32.1  86/01/22  00:24:21  sam
* placed under source control
* 
*
**********************************************************************
             SECTION    speech
_AbsExecBase EQU	4
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                        ;
*         MAIN MODULE 'PHONET'           ;
*                                        ;
* FILLS COEF BUFFER WITH TARGET VALUES,  ;
* COMPUTES BOUNDARY VALUES AND CREATES   ;
* TRANSITIONS FROM BOUNDARIES TO TARGETS ;
*                                        ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*


	INCLUDE 'assembly.i'
 	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'hardware/custom.i'

io_Size	EQU	IO_SIZE
         INCLUDE   'featequ.i'
         INCLUDE   'pcequs.i'
         INCLUDE   'gloequs.i'
	 INCLUDE   'devices/audio.i'
         INCLUDE   'private.i'
*
         EXTERN_SYS AllocMem

         XDEF     PHONET
         XREF     FEAT
         XREF     AMP1T,F1T,RANK,PCT,TEXT,TINT,IHNDUR,MINDUR,FEM1,MOUTHS
*	 XREF	  _AbsExecBase
*
PHONET   BSR      COMPRESS     ;REMOVE IGNORES
	 BSR      FILLDUR      ;FILL IN MISSING DURS
         BSR      COEFOVFL     ;LOOK FOR POSSIBLE COEF OVFL
         BSR      FILLCOEF     ;FILL COEF WITH TARGETS
         BSR.S    CLRCOEF      ;CLEAR F0 ELEMENTS
         BSR      BVAL         ;CALCULATE BOUNDARY VALUES
         BSR      TRANSIT      ;COMPUTE TRANSITIONS
         CLR.W    D0           ;INSURE OVFL NOT SET
         RTS
*
*      SUBROUTINE 'CLRCOEF'
*
* FILLS COEF WITH 0,0,0,FE,FE,FE,0,0...
* SEQUENCES.  THIS IS CONSIDERED A 'CLEAR'
* COEF BUFFER.
* THIS ROUTINE HAS BEEN MOD'ED TO CLEAR F0 ONLY.
*
CLRCOEF  MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6     ;PT TO 1ST F1 SPOT
         MOVE.B   #$FF,D1     ;FOR COMPARES
CLRC2    CMP.B    (A6),D1     ;END?
         BNE.S    CLRC1       ;NO, BRANCH
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6
         MOVE.B   #160,7(A6)  ;STARTING F0=75HZ
         RTS
CLRC1    CLR.B    7(A6)       ;CLEAR F0 ELEMENT
         ADDQ.L   #8,A6       ;INCR ADDR
         BRA      CLRC2
*
* SUBROUTINE 'FILLDUR'
*
* FILLS IN MISSING DURATIONS OF PRIME PHONS,
* ALSO PLACES FRIC RECORDING #'S OF PRIME
* ASPS IN PLACE OF THE 4 LSB STRESS NUMBER.
*
*         REGISTER USAGE
*
* D0=PHON  D1=FEATURE  D6=SCRATCH  D7=SCRATCH
* A0=PHON  A1=DUR  A2=STRESS A4=FEAT
*
FILLDUR  LEA      PHON(A5),A0
         LEA      DUR(A5),A1
         LEA      STRESS(A5),A2
         LEA      FEAT,A4
         MOVEQ    #0,D7        ;CLEAR SCRATCH
FILD0    MOVEQ    #0,D0        ;CLEAR PHON
         MOVE.B   (A0),D0      ;GET PHON
         CMP.B    #$FF,D0      ;LAST PHON?
         BNE.S    FILD9        ;NO, BRANCH
         RTS
FILD9    CMP.B    #PHCRX,D0    ;RX?
         BNE.S    FILD2        ;NO, BRANCH
         MOVEQ    #0,D6
         MOVE.B   (A1),D6      ;YES, GET DUR
         AND.B    #$3F,D6      ;ISOLATE
         MOVE.B   -1(A1),D7    ;GET DUR OF PREV VOWEL
         AND.B    #$3F,D7      ;ISOLATE
         ADD.W    D7,D6        ;ADD TOGETHER
         MOVE.W   D6,D7        ;COMPUTE 3/4 OF SUM
         LSR.W    #2,D7
         SUB.W    D7,D6
         LSR.W    #1,D6        ;/2, AND DISTRIBUTE
         AND.B    #$C0,(A1)    ;CLEAR DUR OF RX
         AND.B    #$C0,-1(A1)  ;CLEAR DUR OF VOWEL
         OR.B     D6,(A1)      ;REPLACE WITH
         OR.B     D6,-1(A1)    ; NEW DURATIONS
         BRA      FILD3        ;BUMP PTRS
FILD2    LSL.W    #2,D0        ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1  ;GET FEATURE
         LSR.W    #2,D0        ;RESTORE PHON #
         BTST     #DIPHBIT,D1  ;DIPHTHONG?
         BEQ.S    FILD1        ;NO, BRANCH
         MOVE.B   (A1),D6      ;YES, GET DURATION
         AND.B    #$3F,D6      ;ISOLATE DURATION
         LSR.B    #1,D6        ;/2
         OR.B     D6,1(A1)     ;PLACE IN DIPH'
         ADDQ.B   #1,D6        ;MAKE UP FOR LOST FRAME
         AND.B    #$C0,(A1)    ;CLEAR DUR OF DIPH
         OR.B     D6,(A1)      ;PLACE IN DIPH
         ADDQ.L   #2,A0        ;INCR
         ADDQ.L   #2,A1        ;ARRAY
         ADDQ.L   #2,A2        ;POINTERS
         BRA      FILD0
FILD1    BTST     #PRIMEBIT,D1 ;PRIME?
         BEQ      FILD3        ;NO, BRANCH
         MOVE.B   -1(A2),(A2)  ;MOVE STRESS INFO OVER
         AND.B    #$7F,(A2)    ;REMOVE SYLL START BIT
         LEA      IHNDUR,A3    ;A3 PTS TO DURATION PARMS
         ADD.L    D0,A3        ;ADD PHON #
         BTST     #4,(A2)      ;STRESSED SEGMENT?
         BNE.S    FILD4        ;YES, SKIP
         ADD.L    #128,A3      ;NO, USE MINDUR
FILD4    MOVE.B   (A3),(A1)+   ;MOVE IN DURATION & INCR
         ADDQ.L   #1,A0        ;INCR PHON PTR
         BTST     #ASPIRBIT,D1 ;ASPIR?
         BEQ.S    FILD5        ;NO, BRANCH
         MOVEQ    #0,D0        ;YES, GET PHON+1
         MOVE.B   (A0),D0
         LSL.W    #2,D0        ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1  ;GET FEATURE
         AND.B    #$F0,(A2)    ;CLEAR STRESS #
         BTST     #FRONTBIT,D1 ;FRONT?
         BEQ.S    FILD6        ;NO, BRANCH
         OR.B     #3,(A2)      ;/H
         BRA.S    FILD5
FILD6    BTST     #ROUNDBIT,D1 ;ROUND?
         BEQ.S    FILD7        ;NO, BRANCH
         OR.B     #6,(A2)      ;/R
         BRA.S    FILD5
FILD7    BTST     #BACKBIT,D1  ;BACK?
         BEQ.S    FILD8        ;NO, BRANCH
         OR.B     #5,(A2)      ;/B
         BRA      FILD5
FILD8    OR.B     #4,(A2)      ;/M (DEFAULT)
FILD5    ADDQ.L   #1,A2        ;INCR DUR
         BRA      FILD0
FILD3    ADDQ.L   #1,A0        ;INCR PTRS
         ADDQ.L   #1,A1
         ADDQ.L   #1,A2
         BRA      FILD0
*
* SUBROUTINE 'COEFOVFL'
*
* COMPUTES SUM OF THE DUR ARRAY AND EXITS
* WITH OVFL BIT SET IF THE SUM EXCEEDS THE
* RESERVED SPACE FOR THE COEF BUFFER
*
*             REGISTER USAGE
* D0=RUNNING SUM  D2=SCRATCH  D5=FF  D6=3F
* A1=DUR   A5=VARS
COEFOVFL LEA    DUR(A5),A1
        MOVE.B  #$FF,D5         ;FOR QUICK COMPARES
        MOVEQ   #$3F,D6         ;FOR QUICK ANDS
        MOVEQ   #0,D0           ;ZERO THE SUM
        MOVEQ   #0,D2           ;CLEAR SCRATCH REG
COEFOV2 MOVE.B  (A1)+,D2        ;GET DUR & INCR
        CMP.B   D5,D2           ;END?
        BEQ.S   COEFOV1         ;YES, BRANCH
        AND.B   D6,D2           ;NO, ISOLATE DURATION
        ADD.W   D2,D0           ;ADD DURATION TO SUM
        BRA     COEFOV2         ;NEXT DUR ELEMENT
COEFOV1 MOVE.L  D0,MOTHSIZE(A5) ;SAVE #of MOUTHS
	LSL.L   #3,D0           ;MULTIPLY SUM * 8
        ADDQ.L  #8,D0           ;ADD 1 FRAME (FF'S)

*------- GET COEF BUFFER AND SAVE ADDRESS IN GLOBALS AREA -----------
*        AMIGA VERSION ONLY!!!! 

         MOVE.L  D0,COEFSIZE(A5)   ;SAVE SIZE FOR FreeMem
	 MOVEQ   #0,D1	
	 MOVE.L  _AbsExecBase,A6 
         CALLSYS AllocMem 
         TST.L   D0                ;OK?
         BNE.S   COEFOV3           ;YES, BRANCH
COEFOV5  MOVE.W  #2,CCR            ;NO, SET OVFL
         ADDQ.L  #4,SP             ;POP TO RTN TO MAINLINE
         RTS

COEFOV3  MOVE.L  D0,COEFPTR(A5)    ;SAVE BUFFER POINTER
	 TST.B   Mouth(A5)         ;MAKE MOUTHS?
	 BEQ.S   COEFOV4           ;NO, BRANCH
	 MOVE.L  MOTHSIZE(A5),D0   ;YES, ALLOCATE MEMORY
	 MOVEQ   #0,D1             ;    FOR MOUTH BUFFER
	 CALLSYS AllocMem
	 TST.L   D0
	 BEQ     COEFOV5
         MOVE.L  D0,MOTHPTR(A5)
COEFOV4  RTS
*
* SUBROUTINE 'FILLCOEF'
*
* FILLS COEF BUFFER WITH PHONEME TARGET VALUES
*
*           REGISTER USAGE
*
* D0=PHON  D1=FEATURE  D2=DUR  D3=AMP1,F1
* D4=AMP2,F2  D5=AMP3,F3  D6=FRIC  D7=SCRATCH
*
* A0=PHON  A1=DUR  A2=STRESS  A3=F1T or FEM1T
* A4=MOUTHS or FEAT   A5=VARS  A6=COEF
*
VOLINC   EQU       2          ;VOLUME INCR 2DB FOR
*                             ;  STRESSED PHONS
FILLCOEF LEA      PHON(A5),A0
         LEA      DUR(A5),A1
         LEA      STRESS(A5),A2
	 MOVE.L   MOTHPTR(A5),A4
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6
FILL0    MOVEQ    #0,D0
         MOVE.B   (A0)+,D0     ;GET PHON & INCR
         CMP.B    #$FF,D0      ;DONE?
         BNE.S    FILL1        ;NO, BRANCH
         MOVEQ    #7,D6        ;YES, FILL LAST
FILL2    MOVE.B   #$FF,(A6)+   ;  FRAME WITH $FF'S
         DBF      D6,FILL2     ;LOOP, AND
         RTS                   ;  EXIT
FILL1    MOVEQ    #0,D2
         MOVE.B   (A1)+,D2     ;GET AND ISOLATE
         AND.B    #$3F,D2      ;  DURATION & INCR
         LSL.W    #2,D0        ;INDEX FOR FEAT
         MOVE.L   A4,D7        ;SAVE MOUTHS
	 LEA      FEAT,A4      ;POINT TO FEATURES
         MOVE.L   0(A4,D0),D1  ;GET FEATURE
         MOVE.L   D7,A4        ;RESTORE MOUTHS
         MOVEQ    #0,D3        ;CLEAR REGS
         MOVE.L   D3,D4
         MOVE.L   D3,D5
         LSR.W    #2,D0        ;RESTORE PHON #
         LEA      AMP1T,A3     ;PT TO AMPLITUDES
         ADD.L    D0,A3        ;ADD PHON
         MOVE.B   (A3),D3      ;GET AMP1T
         ADD.L    #128,A3
         MOVE.B   (A3),D4      ;GET AMP2T
         ADD.L    #128,A3
         MOVE.B   (A3),D5      ;GET AMP3T
* ADD 2DB TO NON-ZERO AMPL STRESSED SEGMENTS
         BTST     #4,(A2)      ;STRESSED?
         BEQ.S    FILL5        ;NO, BRANCH
         TST.B    D3           ;ZERO?
         BEQ.S    FILL5A       ;YES, BRANCH
         ADD.B    #VOLINC,D3   ;NO, ADD 2DB
         CMP.B    #31,D3       ;CLIPPED?
         BLE.S    FILL5A       ;NO, BRANCH
         MOVEQ    #31,D3       ;YES, FORCE 31DB
FILL5A   TST.B    D4
         BEQ.S    FILL5B
         ADD.B    #VOLINC,D4
FILL5B   TST.B    D5
         BEQ.S    FILL5
         ADD.B    #VOLINC,D5
FILL5    ADD.L    #5*128,A3
         MOVE.B   (A3),D6      ;GET FRIC
         SWAP     D3           ;POSITION AMPS
         SWAP     D4           ;IN HI WORD
         SWAP     D5
         MOVEQ    #0,D7
         BTST     #PRIMEBIT,D1 ;PRIME?
         BEQ.S    FILL3        ;NO, BRANCH
         BTST     #ASPIRBIT,D1    ;ASPIR?
         BEQ.S    FILL3        ;NO, BRANCH
         MOVE.B   (A2),D7      ;YES, GET FRIC #
         AND.B    #7,D7        ;ISOLATE
         LSL.B    #4,D7        ;POSITION

FILL3    LEA      F1T,A3       ;LOAD PTR TO MALE FORMANT TARGETS
         TST.W    SEX(A5)      ;CHECK FOR FEMALE
         BEQ.S    FILL6M       ;NO, KEEP MALE FORMANTS
         LEA      FEM1,A3      ;YES, USE FEMALE FORMANTS

* IF PHON IS A PERIOD OR QUESTION, USE THE FORMANTS OF THE PREVIOUS PHON
FILL6M   CMP.B    #PHCPE,D0    ;A PERIOD?
         BEQ.S    FILL6P       ;YES, BRANCH
         CMP.B    #PHCQM,D0    ;NO, QUESTION?
         BNE.S    FILL6        ;NO, BRANCH
FILL6P   MOVE.W   D0,-(SP)     ;SAVE PHON,
         MOVE.B   -2(A0),D0    ;GET PREVIOUS PHON & USE ITS FTARGS
         ADD.L    D0,A3        ;ADD PHON
         MOVE.W   (SP)+,D0     ;RESTORE ORIGINAL PHON 
         BRA.S    FILL6PA

FILL6    ADD.L    D0,A3        ;ADD PHON
FILL6PA  MOVE.B   (A3),D3      ;GET F1T
         ADD.L    #128,A3
         MOVE.B   (A3),D4      ;GET F2T
         ADD.L    #128,A3
         MOVE.B   (A3),D5      ;GET F3T
         OR.B     D7,D6        ;OR IN ASP # (IF ANY)
	 TST.B    Mouth(A5)    ;MAKING MOUTHS?
	 BEQ.S    FILL6A       ;NO, BRANCH
	 MOVE.L   A6,-(SP)     ;SAVE COEF
	 LEA      MOUTHS,A6
	 MOVE.B   0(A6,D0),D1  ;GET A MOUTH SHAPE
	 MOVE.L   (SP)+,A6     ;RESTORE COEF
* SUBTRACT 6DB OF NOISE FROM UNSTRESSED FRICATIVE
FILL6A   TST.B    D6           ;FRIC?
         BEQ.S    FILLSF       ;NO, BRANCH
         BTST     #4,(A2)      ;YES, STRESSED?
         BNE.S    FILLSF       ;YES, BRANCH
         MOVE.B   D6,D0        ;SAVE FRIC BYTE
         AND.B    #$F0,D0      ;  AND REMOVE AMPLITUDE
         AND.B    #$0F,D6      ;KEEP AMPLITUDE ONLY
         LSR.B    #1,D6        ;  AND SUBTRACT 6DB
         OR.B     D0,D6        ;OR BACK TOGETHER
* FILL IN COEF WITH TARGETS
FILLSF   SUBQ.W   #1,D2        ;LOOP COUNT = DUR - 1
FILL4    MOVE.B   D3,(A6)+     ;MOVE
         MOVE.B   D4,(A6)+     ; IN
         MOVE.B   D5,(A6)+     ; FORMANTS
         SWAP     D3           ;POSITION AMPLITUDES
         SWAP     D4           ; IN
         SWAP     D5           ; LOW WORDS
         MOVE.B   D3,(A6)+     ;MOVE
         MOVE.B   D4,(A6)+     ; IN
         MOVE.B   D5,(A6)+     ; AMPLITUDES
         SWAP     D3           ;RESTORE
         SWAP     D4           ; FORMANT
         SWAP     D5           ; POSITIONS
         MOVE.B   D6,(A6)+     ;MOVE IN FRIC
         ADDQ.L   #1,A6        ;SKIP OVER F0
         TST.B    Mouth(A5)    ;IF Mouth=TRUE
         BEQ.S    FILL4A       ;  THEN
         MOVE.B   D1,(A4)+     ;  MOVE MOUTH SHAPE INTO BUFFER
FILL4A   DBF      D2,FILL4     ;NEXT FRAME
         ADDQ.L   #1,A2        ;INCR STRESS PTR
         BRA      FILL0        ;NEXT PHON
*
* SUBROUTINE 'BVAL'
*
* COMPUTES BOUNDARY VALUES AT START OF EACH PHON
* SKIPS CALCULATION IF EITHER FORMANT AT BOUNDARY = 0
*
*              REGISTER USAGE
*
* D0=PHON  D1=FEAT & PCT.W  D2=RANK(P)  D3=RANK(P+1)
* D4=TARGET.W  D5=LOCUS.W  D6=LOOP  D7 SCRATCH
* A0=PHON  A1=DUR  A2=RANK  A3=PCT  A4=FEAT
* A5 UNSUSED  A6=COEF
*
BVAL     LEA      PHON(A5),A0
         LEA      DUR(A5),A1
         LEA      RANK,A2
         LEA      PCT,A3
         LEA      FEAT,A4
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6
BV0      MOVEQ    #0,D0
         MOVEQ    #0,D7        ;CLEAR SCRATCH
         MOVE.B   (A1)+,D7     ;GET DUR TO SCRATCH & INCR
         AND.B    #$3F,D7      ;ISOLATE DURATION
         LSL.W    #3,D7        ;*8
         ADD.L    D7,A6        ;COEF AT BOUNDARY F1
         MOVE.B   1(A0),D0     ;GET PHON+1
         CMP.B    #$FF,D0      ;END?
         BNE.S    BV1          ;NO, BRANCH
         RTS
BV1      LSL.W    #2,D0        ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1  ;GET FEATURE
         BTST     #PRIMEBIT,D1 ;PRIME?
         BEQ.S    BV9          ;NO, BRANCH
         AND.L    #PLOS+PLOSA+ASPIR,D1 ;THESE THINGS?
         BEQ.S    BV9          ;NO, BRANCH
         ADDQ.L   #1,A0        ;SKIP THIS PHON
         BRA      BV0
BV9      LSR.W    #2,D0        ;RESTORE PHON+1
         SWAP     D0           ;SAVE PHON+1 IN HI WORD
         MOVE.B   (A0),D0      ;GET PHON
         MOVE.B   0(A2,D0),D2  ;GET RANK(P)
         SWAP     D0           ;GET PHON+1
         MOVE.B   0(A2,D0),D3  ;GET RANK(P+1)
         MOVEQ    #0,D1        ;CLR PCT
         CMP.B    D2,D3        ;COMPARE RANKS
         BGE.S    BV2          ;BRANCH, RANK(P+1) >= RANK(P)
         SWAP     D0           ;RANK(P+1) < RANK(P)
BV2      MOVE.B   0(A3,D0),D1  ;GET PCT(RANKING P)
         MOVEQ    #5,D6        ;PARM COUNT
BV5      MOVEQ    #0,D4        ;CLR TARGET
         MOVEQ    #0,D5        ;CLR LOCUS
         CMP.B    D2,D3        ;COMPARE RANKS
         BGE.S    BV3
         MOVE.B   (A6),D4      ;TARGET = COEF(P+1)
         MOVE.B   -8(A6),D5    ;LOCUS = COEF(P)
         BRA.S    BV4
BV3      MOVE.B   -8(A6),D4    ;TARGET = COEF(P)
         MOVE.B   (A6),D5      ;LOCUS = COEF(P+1)
BV4      CMP.B    #3,D6        ;FORMANT PARM?
         BLT.S    BV6          ;NO, BRANCH
         TST.B    D4           ;YES, EITHER ZERO?
         BEQ.S    BV7          ;YES, SKIP BV CALC
         TST.B    D5
         BEQ.S    BV7
* CALCULATE  BV = T + PCT * (L - T)
BV6      SUB.W    D4,D5        ;L=L-T
         MULS     D1,D5        ;*PCT
         ASR.L    #5,D5        ;JUSTIFY /32
         ADD.W    D4,D5        ;+T
         MOVE.B   D5,(A6)+     ;BVAL INTO COEF
BV8      DBF      D6,BV5       ;NEXT PARM
         ADDQ.L   #1,A0        ;NEXT PHON
         SUBQ.L   #6,A6        ;PT TO START OF CURRENT FRAME
         BRA      BV0
BV7      ADDQ.L   #1,A6        ;SKIP THIS PARM (F(N)=0)
         BRA      BV8
*
* SUBROUTINE 'TRANS'
* COMPUTES LOCATIONS OF FRAMES OF TRANSITION
* AND CLEARS THEM TO 00,00,00,FE,FE,FE,-,- PATTERN
*
*              REGISTER USAGE
*
* D0=DUR(P)  D1=LOOP  D2=T1  D3=T2  D4=RANK(P-1) & TRIAL & GAP
* D5=RANK(P)  D6=RANK(P+1)  D7=SCRATCH
* A0=PHON  A1=DUR  A2=RANK  A3=TEXT  A4=TINT & FEAT  A5 -  A6=COEF
*
TRANSIT  LEA      PHON(A5),A0      ;START AT 2ND PHON
         ADDQ.L   #1,A0
         LEA      DUR(A5),A1
         LEA      RANK,A2
         LEA      TEXT,A3
*        LEA      TINT,A4      ;TAKEN CARE OF IN LINE
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6
         MOVEQ    #0,D0        ;CLEAR STUFF
         MOVE.L   D0,D1
         MOVE.L   D0,D2
         MOVE.L   D0,D3
         MOVE.L   D0,D4
         MOVE.L   D0,D7
         MOVE.B   (A1)+,D0     ;GET DUR(P-1)
         AND.B    #$3F,D0      ;ISOLATE
TR97     LSL.W    #3,D0        ;*8
         ADD.L    D0,A6        ;ADD TO COEF (1ST BOUNDARY)
TRANS0   MOVEQ    #0,D0        ;CLR PHON & T1 AMPL FLAG
         MOVEQ    #0,D5        ;CLR T2 AMPL FLAG
         MOVEQ    #0,D7        ;CLEAR SCRATCH
         MOVE.B   -1(A0),D7    ;GET P-1
         MOVE.B   0(A2,D7),D4  ;GET R(P-1)  (R=RANK)
         MOVE.B   (A0),D7      ;GET P
         CMP.B    #$FF,D7      ;END?
         BNE.S    TR1          ;NO, BRANCH
         RTS                   ;NO, EXIT
TR1      MOVE.B   0(A2,D7),D5  ;GET R(P)
         MOVE.B   (A1)+,D0     ;GET DUR(P) & INCR
         AND.B    #$3F,D0      ;ISOLATE
         LSL.W    #2,D7        ;INDEX FOR FEAT
         LEA      FEAT,A4
         MOVE.L   0(A4,D7),D1  ;GET FEATURE
         BTST     #PLOSBIT,D1  ;A PLOSIVE?
         BNE.S    TR96         ;YES, BRANCH
         BTST     #PLOSABIT,D1 ;NO, PLOSIVE ASP?
         BEQ.S    TR98         ;NO, BRANCH
TR96     ADDQ.L   #1,A0        ;YES, INCR PHON
         BRA      TR97         ;MOVE COEF PTR & CONTINUE
TR98     LEA      TINT,A4      ;RESTORE A4
         MOVEQ    #0,D7        ;CLEAR SHIFTED PHON
         MOVE.B   1(A0),D7     ;GET P+1
         CMP.B    #$FF,D7      ;END?
         BNE.S    TR98A        ;NO, BRANCH
         BSET     #30,D0       ;YES, SET END OF UTTERANCE FLAG
TR98A    MOVE.B   0(A2,D7),D6  ;GET R(P+1)
         MOVEQ    #0,D7        ;CLEAR SCRATCH
         CMP.B    D5,D4        ;CMP R(P) & R(P-1)
         BLE.S    TR2
         MOVE.B   -1(A0),D7    ;R(P-1) > R(P)
         MOVE.B   0(A3,D7),D2  ;T1=TEXT(P-1)
         BRA.S    TR3
TR2      MOVE.B   (A0),D7      ;R(P-1) <= R(P)
         MOVE.B   0(A4,D7),D2  ;T1=TINT(P)
TR3      CMP.B    D5,D6        ;CMP R(P) & R(P+1)
         BGE.S    TR4
         MOVE.B   (A0),D7      ;R(P) > R(P+1)
         MOVE.B   0(A4,D7),D3  ;T2=TINT(P)
         BRA.S    TR9A
TR4      MOVE.B   1(A0),D7     ;R(P) <= R(P+1)
         MOVE.B   0(A3,D7),D3  ;T2=TEXT(P+1)
* CHECK FOR END OF UTTERANCE
TR9A     BTST     #30,D0       ;CHECK END OF UTTERANCE FLAG
         BEQ.S    TR9          ;NOT END, BRANCH
         MOVEQ    #0,D3        ;END, SET T2=0
* TEST FOR PLOSIVE VOWEL / VOWEL PLOSIVE COMBINATIONS
* IN SUCH, THE AMPLITUDES TO NOT TRANSITION
TR9      BTST     #SONORBIT,D1 ;(P) A SONORANT?
         BEQ.S    TR5          ;NO, BRANCH
         MOVEQ    #0,D7
         LEA      FEAT,A4
         MOVE.B   -1(A0),D7    ;YES, GET P-1
         LSL.W    #2,D7        ;INDEX FOR FEAT
         MOVE.L   0(A4,D7),D6  ;GET FEAT(P-1)
         MOVE.L   D6,D7        ;SAVE FEATURE
         AND.L    #PLOS+PLOSA,D6  ;NAUGHTY?
         BEQ.S    TR10         ;NO, BRANCH
         BSET     #31,D0       ;YES, SET T1 AMPL FLAG
         BRA.S    TR11         ; AND TEST FOR LIQUID+GLIDE
* TEST FOR PLOS + FRIC & ~VOICED / LIQUID+GLIDE
* TRANSITIONS SHOULD BE SET TO 2 AT THESE BOUNDARIES
TR10     BTST     #FRICBIT,D7  ;P-1 A FRIC
         BEQ.S    TR6          ;NO, BRANCH
         BTST     #VOICEDBIT,D7  ;YES, VOICED?
         BNE.S    TR6          ;YES, BRANCH
TR11     BTST     #LIQUIDBIT,D1  ;NO, (P) A LIQUID?
         BNE.S    TR8          ;YES, BRANCH
         BTST     #GLIDEBIT,D1 ;NO, GLIDE?
         BEQ.S    TR6          ;NO, BRANCH
TR8      MOVEQ    #2,D2        ;YES, FORCE T1=2
TR6      MOVEQ    #0,D7
         MOVE.B   1(A0),D7     ;GET P+1
         LSL.W    #2,D7
         MOVE.L   0(A4,D7),D6  ;GET FEAT(P+1)
         AND.L    #PLOS+PLOSA,D6  ;STUFF?
         BEQ.S    TR5          ;NO, BRANCH
         BSET     #31,D5       ;YES, SET T2 AMPL FLAG
*
TR5      SUBQ.W   #1,D0        ;DUR=DUR-1 (LOOP COUNT)
         SUBQ.B   #1,D2        ;T1=T1-1 TO ACCOUNT FOR BVAL POS
         BPL.S    TR7          ;NOT -, BRANCH
         MOVEQ    #0,D2        ;WENT -, FORCE ZERO
TR7      MOVE.W   #$FEFE,D6    ;CLEARING PATTERN
         MOVEQ    #2,D4        ;TRIAL=3
TRA7     MOVE.B   D2,D7        ;T1 TO SCRATCH
         ADD.B    D3,D7        ;ADD T2
         CMP.B    D0,D7        ;T1+T2 >= DUR?
         BLT.S    TRANS        ;NO, BRANCH
         SUBQ.B   #1,D2        ;YES, T1=T1-1
         BMI.S    TRALL        ;GO TO 'CLEAR ALL'
         SUBQ.B   #1,D3        ;T2=T2-1
         BMI.S    TRALL
         SUBQ.B   #1,D4
         BEQ.S    TRALL
         BRA      TRA7
TRANS    MOVE.B   D0,D4        ;GAP=DUR-T1-T2
         SUB.B    D2,D4
         SUB.B    D3,D4        ;D4=GAP
         MOVEQ    #0,D1
         MOVE.B   D2,D1        ;LOOP=T1
         BRA.S    TRA1
TRA2     CLR.W    (A6)         ;CLEAR ELEMENTS
         CLR.B    2(A6)
         BTST     #31,D0       ;TEST T1 AMPL FLAG
         BNE.S    TRA1         ;SET, NO AMPL TRANS
         MOVE.B   D6,3(A6)     ;NOT SET, CLEAR AMPLS
         MOVE.W   D6,4(A6)
TRA1     ADDQ.L   #8,A6        ;NEXT FRAME
         DBF      D1,TRA2
         MOVEQ    #0,D1
         MOVE.B   D4,D1        ;LOOP=GAP
         BRA.S    TRA3
TRA4     ADDQ.L   #8,A6        ;SKIP FRAME
TRA3     DBF      D1,TRA4
         MOVEQ    #0,D1
         MOVE.B   D3,D1        ;LOOP=T2
         BRA.S    TRA5
TRA6     CLR.W    (A6)         ;CLEAR ELEMENTS
         CLR.B    2(A6)
         BTST     #31,D5       ;TEST T2 AMPL FLAG
         BNE.S    TRA5A        ;SET, NO AMPL TRANS
         MOVE.B   D6,3(A6)
         MOVE.W   D6,4(A6)
TRA5A    ADDQ.L   #8,A6        ;NEXT FRAME
TRA5     DBF      D1,TRA6
         ADDQ.L   #1,A0        ;INCR PHON
         BRA      TRANS0
TRALL    MOVEQ    #0,D1        ;CLEAR ALL FRAMES
         MOVE.B   D0,D1        ;LOOP=DUR
         MOVEQ    #0,D4        ;GAP=0
         MOVEQ    #0,D3        ;T2=0
         BRA      TRA1

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
* SUBROUTINE 'COMPRESS'
*
* DELETES 'IGNORE' TYPE PHONS FROM PHON STRESS & DUR
* ARRAYS AND COMPRESSES THE SPACES OUT.
*
COMPRESS LEA     PHON(A5),A0     ;PHON READ
         LEA     DUR(A5),A1      ;DUR READ
         LEA     STRESS(A5),A2   ;STRESS READ
         LEA     FEAT,A3
         MOVE.L  A5,-(SP)    ;PRESERVE A5
         MOVE.L  A0,A4       ;PHON WRITE
         MOVE.L  A1,A5       ;DUR WRITE
         MOVE.L  A2,A6       ;STRESS WRITE
         MOVE.B  #$FF,D4     ;USE INSTEAD OF IMMED'S
COMPR0   MOVEQ   #0,D0
         MOVE.B  (A0)+,D0    ;GET PHON
         MOVE.B  D0,D5       ;SAVE FOR LATER
         CMP.B   D4,D0       ;LAST PHON?
         BEQ.S   COMPR1      ;YES, EXIT
         MOVE.B  (A1)+,D1    ;GET DUR
         MOVE.B  (A2)+,D2    ;GET STRESS
         LSL.W   #2,D0       ;INDEX FOR FEATURE
         MOVE.L  0(A3,D0),D3 ;GET FEATURE
         BTST    #IGNOREBIT,D3  ;IGNORE?
         BNE     COMPR0      ;YES, DON'T USE
         MOVE.B  D5,(A4)+    ;NO, MOVE DATA BACK
         MOVE.B  D1,(A5)+
         MOVE.B  D2,(A6)+
         BRA     COMPR0      ;NEXT PHON
COMPR1   MOVE.B  D4,(A4)
         MOVE.B  D4,(A5)
         MOVE.B  D4,(A6)
         MOVE.L  (SP)+,A5    ;RESTORE A5
         RTS

         END
