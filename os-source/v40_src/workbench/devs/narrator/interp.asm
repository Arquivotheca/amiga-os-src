
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

	TTL	'INTERP.ASM'

               SECTION     speech

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                       ;
*       INTERPOLATION  MODULE           ;
*                                       ;
* LINEARLY INTERPOLATES BETWEEN DATA    ;
* POINTS IN THE COEF BUFFER WHICH ARE   ;
* SITUATED AT INTERVALS OF 8 BYTES.     ;
* THEN SUBSTITUTES NASAL MURMURS.       ;
* ALSO MODIFIES F0 VALUES BASED ON      ;
* PHONEMICS.                            ;
*                                       ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
         INCLUDE   'featequ.i'
         INCLUDE   'pcequs.i'
         INCLUDE   'gloequs.i'
*
         XDEF     INTERP
         XREF     FEAT,F1T
*
INTERP   MOVEQ    #0,D0       ;CLEAR STUFF
         MOVEQ    #0,D1
         MOVEQ    #0,D2
         MOVEQ    #0,D3
         MOVEQ    #0,D4
         MOVEQ    #0,D5
         MOVEQ    #0,D6
         MOVEQ    #0,D7
*
* INTERPOLATE COEF BUFFER
*
         MOVE.L   COEFPTR(A5),A0   ;F1
         BSR      SEARCH
         MOVE.L   COEFPTR(A5),A0
         ADDQ.L   #1,A0
         BSR      SEARCH
         MOVE.L   COEFPTR(A5),A0
         ADDQ.L   #2,A0
         BSR      SEARCH
         MOVE.L   COEFPTR(A5),A0
         ADDQ.L   #7,A0
         BSR      SEARCH
*        BSR      INTERAMP
* SMOOTH FORMANT DATA
*        MOVE.L   COEFPTR(A5),A6
*        LEA      COEF,A6     ;F1
*        BSR      SMOOTH1
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6   ;F2
         ADDQ.L   #1,A6
         BSR      SMOOTH      ;SEVERE SMOOTHING OF F2
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6   ;F3
         ADDQ.L   #2,A6
         BSR      SMOOTH1
         BSR      PEF0        ;MODIFY F0 VALUES
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6   ;SMOOTH F0
         ADDQ.L   #7,A6
         BSR      SMOOTH
*
         BSR      FRICON
         BSR.S    INTERAMP
         BSR      LAMP
         BSR      NOSE
         TST.B    Mouth(A5)
         BEQ.S    INTEXIT
         BSR      SMOMO
INTEXIT  RTS
*
INTERAMP MOVE.L   COEFPTR(A5),A0
*        LEA      COEF,A0   ;AMP1
         ADDQ.L   #3,A0
         BSR      SEARCH1
         MOVE.L   COEFPTR(A5),A0
*         LEA      COEF,A0   ;AMP2
         ADDQ.L   #4,A0
         BSR      SEARCH1
         MOVE.L   COEFPTR(A5),A0
*         LEA      COEF,A0   ;AMP3
         ADDQ.L   #5,A0
         BSR      SEARCH1
         RTS
*
*      SUBROUTINE 'INTERP8'
*
* LINEARLY INTERPOLATES BETWEEN TWO VALUES
* SITUATED AT (A0) AND 0(A0,D1) PLACING
* INTERMEDIATE VALUES AT 8 BYTE INTERVALS.
* D1 MUST BE A MULTIPLE OF 8 > 8.
* ALL ARITHMETIC IS DONE WITH A 5 BIT FRACTION.
*
INTERP8  MOVEQ    #0,D0
         MOVE.L   D0,D6
         MOVE.B   0(A0,D1),D0 ;END PT TO D0
         MOVE.B   (A0),D6     ;START PT TO D6
         SUB.L    D6,D0       ;COMPUTE DELTA
         ASL.L    #5,D0       ;DELTA * 32
         LSR.W    #3,D1       ;D1=ARITH. OFFSET
         DIVS     D1,D0       ;D0=STEP * 32
         ADDQ.L   #8,A0
         ASL.W    #5,D6       ;START * 32
         SUBQ.W   #2,D1       ;FOR PROPER LOOP COUNT
INTER81  ADD.W    D0,D6       ;ADD STEP
         MOVE.W   D6,D7       ;SAVE D6
         ASR.W    #5,D7       ;/32
         MOVE.B   D7,(A0)     ;PLACE INTERP VAL IN ARRAY
         ADDQ.L   #8,A0
         DBF      D1,INTER81
         RTS
*
*    SUBROUTINES 'SEARCH1' AND 'SEARCH'
*
* SCAN THE COEF BUFFER LOOKING FOR NON-CLEAR
* DATA POINTS.  THESE LOCATIONS ARE PASSED TO
* INTERP8.
*
SEARCH1  MOVEQ    #0,D1       ;ZERO INDEX
SCH4     ADDQ.W   #8,D1       ;LOOK AT NEXT PT
         MOVE.B   0(A0,D1),D6 ;VALUE TO SCR
         CMP.B    #$FF,D6     ;END?
         BNE      SCH3        ;NO, CONT.
         RTS
SCH3     CMP.B    #$FE,D6     ;CLEAR PT?
         BEQ      SCH4        ;YES, BRANCH
         CMP.W    #8,D1       ;OFFSET ONLY 1?
         BNE      SCH6        ;NO, BRANCH
         ADDQ.L   #8,A0       ;YES, DO NOT INTERP
         BRA      SEARCH1     ;NEXT PT.
SCH6     BSR      INTERP8
         BRA      SEARCH1
*
SEARCH   MOVEQ    #0,D1
SCH1     ADDQ.W   #8,D1
         TST.B    0(A0,D1)    ;CLEAR PT?
         BEQ      SCH1        ;YES, BRANCH
         CMP.B    #$FF,0(A0,D1) ;END?
         BNE      SCH2        ;NO, BRANCH
         RTS
SCH2     CMP.W    #8,D1       ;OFFSET ONLY 1?
         BNE      SCH5        ;NO, BRANCH
         ADDQ.L   #8,A0       ;YES, DO NOT INTERP
         BRA      SEARCH
SCH5     BSR      INTERP8
         BRA      SEARCH
*
* SUBROUTINE 'NOSE'
*
* REPLACES NASAL FRAMES WITH STRAIGHT NASAL COEFS.
* THE PARMS FOR M,N,NX & NH ARE STORED IN THE TARGS
* FOR UL, UM, UN AND UN+1 RESPECTIVELY.
*
*            REGISTER USAGE
*
* D0=DUR  D1=LOOP  D2=FEAT  D3=PHON  D4=LOOP2  D5=128
* A0=COEF  A1=PARMS  A2=PHON  A3=DUR  A4=FEAT
* A5 UNUSED  A6=PARMS+
*
NOSE     MOVE.L   COEFPTR(A5),A0
         LEA      F1T,A1
         LEA      PHON(A5),A2
         LEA      DUR(A5),A3
         LEA      FEAT,A4
         MOVEQ    #0,D5
         MOVE.B   #128,D5      ;PARM INCREMENT
NOSE3    MOVEQ    #0,D3
         MOVE.L   D3,D1
         MOVE.L   D3,D4
         MOVE.B   (A2)+,D3     ;GET PHON & INCR
         CMP.B    #$FF,D3      ;END?
         BNE      NOSE2        ;NO, BRANCH
         RTS
NOSE2    LSL.W    #2,D3        ;INDEX FOR FEAT
         MOVE.L   0(A4,D3),D2  ;GET FEATURE
         MOVEQ    #0,D0
         MOVE.B   (A3)+,D0     ;GET DUR & INCR
         AND.B    #$3F,D0      ;ISOLATE DURATION
         BTST     #NASALBIT,D2 ;NASAL?
         BNE      NOSE1        ;YES, BRANCH
         BTST     #VOWELBIT,D2 ;NO, VOWEL?
         BEQ      NOSE9        ;NO, BRANCH
         MOVEQ    #0,D3        ;YES, GET NEXT PHON
         MOVE.B   (A2),D3      ;
         LSL.W    #2,D3        ;INDEX FOR FEAT
         MOVE.L   0(A4,D3),D2  ;GET FEATURE
         BTST     #NASALBIT,D2 ;PHON+1 NASAL?
         BEQ      NOSE9        ;NO, BRANCH
*VOWEL IS PRE-NASAL: RAISE F1 BY 100HZ AND LOWER ITS AMPLITUDE
*BY 3DB FOR THE LATTER HALF OF ITS DURATION.
         MOVE.B   D0,D1        ;YES, DUR TO D1
         LSR.B    #1,D1        ;DUR/2
         MOVE.B   D1,D4        ;SAVE DUR/2
         LSL.W    #3,D1        ;8 * (DUR/2)
         ADD.L    D1,A0        ;ADD TO COEF
         ADD.B    #4,(A0)      ;F1 + 50HZ AT CENTER FRAME
         ADDQ.L   #8,A0        ;PT TO NEXT FRAME
         SUB.B    D4,D0        ;REMAINDER OF FRAMES + 1
         SUBQ.B   #2,D0        ;LOOP COUNT
NOSE10   ADD.B    #9,(A0)      ;F1 + 100Hz
         MOVE.B   3(A0),D1     ;AMPL - 3db
         LSR.B    #2,D1
         SUB.B    D1,3(A0)
         ADDQ.L   #8,A0        ;PT TO NEXT FRAME
         DBF      D0,NOSE10    ;LOOP
         BRA      NOSE3        ;NEXT PHON
*
NOSE9    LSL.W    #3,D0        ;NO, MULT DUR BY 8
         ADD.L    D0,A0        ;ADD TO COEF
         BRA      NOSE3
NOSE1    LSR.W    #2,D3        ;RESTORE PHON
         CMP.B    #PHCM,D3     ;WHICH NASAL?
         BNE      NOSE4
         MOVE.B   #PHCUL,D3    ;ITS AN 'M'
         BRA      NOSEY
NOSE4    CMP.B    #PHCN,D3
         BNE      NOSE5
         MOVE.B   #PHCUM,D3    ;ITS AN 'N'
         BRA      NOSEY
NOSE5    CMP.B    #PHCNX,D3
         BNE      NOSE6
         MOVE.B   #PHCUN,D3    ;ITS AN 'NX
         BRA      NOSEY
NOSE6    MOVE.B   #PHCUN+1,D3  ;ITS AN 'NH'
NOSEY    MOVE.W   D0,D1        ;SET LOOP1=DUR-1
         SUBQ.W   #1,D1
NOSE8    MOVE.L   A1,A6        ;PARM BASE ADDR
         ADD.L    D3,A6        ;ADD PHON CODE
         MOVEQ    #5,D4        ;LOOP= 6 PARMS
NOSE7    MOVE.B   (A6),(A0)+   ;MOVE PARM TO COEF & INCR
         ADD.L    D5,A6        ;NEXT PARM
         DBF      D4,NOSE7
         ADD.L    #2,A0        ;PT TO NEXT FRAME
         DBF      D1,NOSE8
         BRA      NOSE3        ;RE-ENTER MAIN NOSE
*
*
* SUBROUTINE 'FRICON'
* SETS THE FIRST TWO, AND THE LAST TWO FRAMES
* OF AN UNVOICED FRICATIVE OR ASPIRANT TO -6DB
* OF ITS INHERENT VOLUME.
*
*         REGISTER USAGE
*
* D0=PHON  D1=FEAT  D2=DUR  D3=FRIC AMP  D4=$F0
* D5=$FF   D6=FRIC+ASPIR    D7=$3F
* A0=PHON  A1=DUR  A2=STRESS  A3 -  A4=FEAT  A5 -  A6=COEF
*
FRICON   LEA      PHON(A5),A0
         LEA      DUR(A5),A1
         LEA      STRESS(A5),A2   ;MAYBE FOR FUTURE USE
         LEA      FEAT,A4
         MOVE.L   COEFPTR(A5),A6
         MOVE.L   #FRIC+ASPIR,D6 ;TEMPLATES
         MOVEQ    #0,D4       ; CLEAR (USE BIT 31 AS NOT FIRST PHON FLAG)
         MOVE.B   #$F0,D4     ; FOR ANDS
         MOVE.B   #$FF,D5     ; & COMPARES
         MOVE.B   #$3F,D7
         BRA.S    FRO0A
FRO0     BSET     #31,D4      ;SET FIRST PHONEME FLAG
FRO0A    MOVEQ    #0,D0       ;CLEAR PHON
         MOVEQ    #0,D2       ;CLEAR DUR
         MOVE.B   (A0)+,D0    ;GET PHON & INCR
         CMP.B    D5,D0       ;END?
         BNE.S    FRO1        ;NO, BRANCH
         RTS                  ;YES, EXIT
FRO1     MOVE.B   (A1)+,D2    ;GET DURATION
         AND.B    D7,D2       ;ISOLATE DURATION
         CMP.B    #PHCPE,D0   ;PERIOD?
         BEQ.S    FRO1B       ;YES, BRANCH
         CMP.B    #PHCQM,D0   ;QUESTION MARK?
         BNE.S    FRO1A       ;NO, BRANCH
*MAKE FORMANTS OF PERIOD OR '?' EQUAL THAT OF PREV FRAME
FRO1B    MOVE.L   A6,A3       ;YES, COEF TO A3
         SUBQ.L   #8,A3       ;PT TO PREV FRAME (PHON)
         MOVE.B   D2,D1       ;DUR TO D1
         EXT.W    D1          ;MAKE SUITABLE
         SUBQ.W   #1,D1       ; AS LOOP COUNTER
FRO1C    MOVE.B   (A3),(A6)   ;MOVE F1
         MOVE.B   1(A3),1(A6) ; F2
         MOVE.B   2(A3),2(A6) ; F3
         ADDQ.L   #8,A6       ;NEXT FRAME OF PERIOD (OR ?)
         DBF      D1,FRO1C
         BRA      FRO0        ;NEXT PHON (WILL BE THE END)
*
FRO1A    LSL.W    #2,D0       ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         BTST     #VOICEDBIT,D1 ;VOICED?
         BNE      FRO2        ;YES, BRANCH
         AND.L    D6,D1       ;NO, FRIC OR ASPIR?
         BEQ      FRO2        ;NO, BRANCH
         CMP.B    #2,D2       ;DUR > 2 FRAMES
         BLE      FRO2        ;NO, BRANCH
* DECAY OF VOICING BEFORE A FRIC OR ASPIR
         BTST     #31,D4      ;CHECK NOT FIRST PHONEME FLAG
         BEQ.S    FRO3        ;BRANCH IF FIRST PHONEME
         MOVEQ    #0,D0       ;YES, CLEAR PHON
         MOVE.B   -2(A0),D0   ;GET PRECEDING PHON
         LSL.W    #2,D0       ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         BTST     #SONORBIT,D1 ;SONORANT?
         BEQ      FRO3        ;NO, BRANCH
         CLR.B    -5(A6)      ;AMPL'S OF LAST FRAME=0
         CLR.W    -4(A6)
         MOVE.B   #$FE,-13(A6) ;CLEAR 2ND TO LAST FRAME
         MOVE.W   #$FEFE,-12(A6)
*
FRO3     MOVE.B   6(A6),D3    ;GET FF
         AND.B    #$F,D3      ;ISOLATE AMPL
         LSR.B    #1,D3       ;DOWN 6DB
         MOVE.B   D3,D1
         ADDQ.B   #1,D1       ;ROUND
         LSR.B    #1,D1       ;DOWN 12DB
         AND.B    D4,6(A6)    ;ERASE AMPLS OF 1ST
         AND.B    D4,14(A6)   ;  TWO FRAMES
         OR.B     D1,6(A6)    ;REPLACE WITH NEW AMPLS
         OR.B     D3,14(A6)
         LSL.W    #3,D2
         ADD.L    D2,A6       ;PT TO NEXT PHON
         AND.B    D4,-2(A6)   ;ERASE AMPLS OF LAST
         AND.B    D4,-10(A6)  ;  TWO FRAMES
         OR.B     D1,-2(A6)   ;REPLACE WITH NEW AMPLS
         OR.B     D3,-10(A6)
         MOVEQ    #0,D0
* ONSET OF VOICING
         MOVE.B   (A0),D0     ;GET NEXT PHON
         CMP.B    D5,D0       ;END OF INPUT?    **** ADDED 11/27
         BEQ      FRO0        ;YES, BRANCH UP   ****       "
         LSL.W    #2,D0       ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         BTST     #SONORBIT,D1  ;SONORANT?
         BEQ      FRO0        ;NO, BRANCH UP
         CLR.B    3(A6)       ;YES, SET AMPL OF FRAME 1
         CLR.W    4(A6)       ; TO ZERO
         MOVE.B   #$FE,11(A6) ;CLEAR NEXT 2 FRAMES
         MOVE.W   #$FEFE,12(A6)
         MOVE.B   #$FE,19(A6)
         MOVE.W   #$FEFE,20(A6)
         BRA      FRO0        ;AGAIN
FRO2     LSL.W    #3,D2       ;PT TO NEXT PHON
         ADD.L    D2,A6
         BRA      FRO0
*
* SUBROUTINE 'LAMP'
*
* CONVERTS AMPLITUDE ARRAYS FROM DB TO LINEAR
*
         DC.B     0,0,0,0,0,0,0,0  ;ROOM FOR SUBTRACTS
LINT     DC.B     0,1,1,1,1,2,2,2,3,3,3,3,4 ;DB TO LINEAR
         DC.B     4,5,5,6,7,7,8,9,10,11,13  ; CONVERSION
         DC.B     14,16,18,20,23,25,28,31   ; TABLE
LAMP     LEA      LINT,A1
         MOVE.L   COEFPTR(A5),A0
         ADDQ.L   #3,A0
         MOVE.B   #$FE,D4     ;FOR COMPARES
         MOVE.B   #$FF,D3
         MOVEQ    #0,D0
LAMP2    MOVE.B   (A0),D0     ;GET AMPL
         CMP.B    D0,D3       ;END?
         BEQ.S    LAMP1       ;YES, BRANCH
         CMP.B    D0,D4       ;CLEAR BYTE?
         BNE.S    LAMP3       ;NO, BRANCH
         ADDQ.L   #8,A0       ;YES, SKIP THIS FRAME
         BRA.S    LAMP2

LAMP3    MOVE.B   0(A1,D0),(A0)+  ;REPLACE FROM TABLE
         MOVE.B   (A0),D0
         MOVE.B   0(A1,D0),(A0)+
         MOVE.B   (A0),D0
         MOVE.B   0(A1,D0),(A0)
         ADDQ.L   #6,A0
         BRA.S    LAMP2
LAMP1    RTS
*
* SUBROUTINE 'SMOOTH'
*
* 7-POINT DATA AVERAGER, WEIGHTING = 1112111
*
*         REGISTER USAGE
*
* D0=SUM  D1=LOOP  D2=8*LOOP D3=SCRATCH
* D4 -    D5 -     D6=$FF    D7 -
* A0-A5 -          A6=COEF+DISP
*
SMOOTH   MOVE.B   #$FF,D6     ;FOR COMPARES
         MOVEQ    #0,D2       ;CLEAR STUFF
         MOVEQ    #0,D3
SMO0     MOVEQ    #0,D0
         MOVEQ    #6,D1       ;LOOP COUNT
SMO1     MOVE.B   D1,D2
         LSL.B    #3,D2       ;8*LOOP FOR INDEX
         MOVE.B   0(A6,D2),D3 ;GET ELEMENT
         CMP.B    D3,D6       ;END?
         BNE      SMO2        ;NO, BRANCH
         RTS                  ;YES, EXIT
SMO2     ADD.W    D3,D0       ;ADD TO SUM
         DBF      D1,SMO1     ;GET NEXT ELEMENT
         MOVE.B   24(A6),D3   ;GET 3RD ELEMENT AGAIN
         ADD.W    D3,D0       ;DOUBLE ITS WEIGHT
         LSR.W    #3,D0       ;SUM / 8
         MOVE.B   D0,24(A6)   ;PUT BACK
         ADDQ.L   #8,A6       ;NEXT FRAME
         BRA      SMO0
*
* SUBROUTINE 'SMOOTH1'
* SAME AS ABOVE EXCEPT WITH WEIGHTING = 1226221
*
SMOOTH1  MOVE.B   #$FF,D6     ;FOR COMPARES
         MOVEQ    #0,D2       ;CLEAR STUFF
         MOVEQ    #0,D3
SMO10    MOVEQ    #0,D0
         MOVEQ    #6,D1       ;LOOP COUNT
SMO11    MOVE.B   D1,D2
         LSL.B    #3,D2       ;8*LOOP FOR INDEX
         MOVEQ    #0,D3
         MOVE.B   0(A6,D2),D3 ;GET ELEMENT
         CMP.B    D3,D6       ;END?
         BNE      SMO12       ;NO, BRANCH
         RTS                  ;YES, EXIT
SMO12    LSL.W    #1,D3       ;DOUBLE IT
         ADD.W    D3,D0       ;ADD TO SUM
         DBF      D1,SMO11
         MOVEQ    #0,D3
         MOVE.B   (A6),D3     ;GET 1ST ELEMENT
         SUB.W    D3,D0       ;SUB FROM SUM
         MOVE.B   48(A6),D3   ;GET 7TH
         SUB.W    D3,D0       ;SUB FROM SUM
         MOVE.B   24(A6),D3   ;GET 4TH
         LSL.W    #2,D3       ;*4
         ADD.W    D3,D0       ;ADD TO SUM
         LSR.W    #4,D0       ;SUM / 16
         MOVE.B   D0,24(A6)   ;PUT BACK IN 4TH
         ADDQ.L   #8,A6       ;NEXT FRAME
         BRA      SMO10
*
* SUBROUTINE 'PEF0'
*
* MODIFIES F0 VALUES BASED ON THE TYPE OF PHON
*
*               REGISTER USAGE
*
* D0=PHON  D1=FEATURE  D2=DURATION  D3=DELTA  D4=SCRATCH  D5-7 --
* A0=PHON  A1 --  A2=DUR  A3 --  A4=FEAT  A5 --  A6=COEF
*
PEF0     LEA      PHON(A5),A0
         LEA      DUR(A5),A2
         LEA      FEAT,A4
         MOVE.L   COEFPTR(A5),A6
*         LEA      COEF,A6
PEF2     MOVEQ    #0,D2       ;CLEAR DURATION
         MOVEQ    #0,D0       ;CLEAR PHON
         MOVE.B   (A0)+,D0    ;GET PHON
         CMP.B    #$FF,D0     ;END?
         BNE      PEF1        ;NO, BRANCH
         RTS                  ;YES, RETURN
PEF1     MOVE.B   (A2)+,D2    ;GET DURATION
         AND.B    #$3F,D2     ;ISOLATE DURATION
         CMP.B    #PHCB,D0    ;'B'?
         BEQ      PEFVPLO     ;YES, BRANCH
         CMP.B    #PHCD,D0    ;NO, 'D'?
         BEQ      PEFVPLO     ;YES, BRANCH
         CMP.B    #PHCQ,D0    ;NO, GLOTTAL STOP?
         BEQ      PEFGLOT     ;YES, BRANCH
         LSL.W    #2,D0       ;NO, INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         BTST     #PLOSABIT,D1  ;PLOSA?
         BNE      PEFPLO        ;YES, BRANCH
         BTST     #NASALBIT,D1  ;NO, NASAL?
         BNE      PEFNAS        ;YES, BRANCH
         BTST     #FRICBIT,D1   ;NO, FRIC?
         BNE      PEFRIC        ;YES, BRANCH
         AND.L    #VOWEL+LIQUID+GLIDE,D1  ;NO, THESE THINGS?
         BNE      PEFVOW      ;YES, BRANCH
         LSL.W    #3,D2       ;NO, SKIP THIS PHON
         ADD.L    D2,A6       ;PT TO NEXT PHON
         BRA      PEF2        ;NEXT PHON
PEFVPLO  MOVEQ    #10,D3      ;DELTA = -8HZ
PEF3     SUBQ     #1,D2       ;LOOP COUNT =  DUR-1
PEF3A    ADD.B    D3,7(A6)    ;ADD DELTA TO F0 ENTRY
         ADDQ.L   #8,A6       ;PT TO NEXT FRAME
         DBF      D2,PEF3A
         BRA      PEF2        ;NEXT PHON
PEFPLO   MOVEQ    #-6,D3      ;DELTA = +5HZ
         BRA      PEF3
PEFNAS   MOVEQ    #-6,D3      ;DELTA = +5HZ
         BRA      PEF3
PEFRIC   MOVEQ    #-6,D3     ;DELTA = +5HZ
         BRA      PEF3
PEFGLOT  MOVE.B   #230,D3     ;SUBSTITUTE F0 = 48HZ
         SUBQ     #1,D2       ;   AT A GLOTTAL STOP
PEF4     MOVE.B   D3,7(A6)
         ADDQ.L   #8,A6
         DBF      D2,PEF4
         BRA      PEF2        ;NEXT PHON
PEFVOW   SUBQ     #1,D2       ;LOOP COUNT
PEF5     MOVE.B   (A6),D3     ;GET F1
         SUB.B    #43,D3      ;F1-43   (F1-500HZ)
         ASR.B    #2,D3       ;(F1-43)/4
*         MOVE.B   D3,D4       ;SAVE
*         ASR.B    #1,D3       ;(F1-43)/8
*         ADD.B    D4,D3       ;(1/4 + 1/8) * (F1 - 43)
         ADD.B    D3,7(A6)    ;MODIFY F0 ENTRY
         ADDQ.L   #8,A6       ;PT TO NEXT FRAME
         DBF      D2,PEF5
         BRA      PEF2
*
* SUBROUTINE 'SMOMO'
*
* SMOOTHS PACKED MOUTH ARRAY
* WEIGHTING IS 1112111
*
*
*  Smooth lower nibble
*
SMOMO	MOVE.L	MOTHPTR(A5),A0	;pt to mouth buffer
	MOVE.L  MOTHSIZE(A5),D1 ;number of bytes in above
	SUBQ.W	#5,D1		;compensate for loop count
	MOVEQ	#$0F,D4		;mask
	MOVEQ	#-16,D5		;mask
SMOMO3	MOVEQ	#0,D2		;clear sum
	MOVEQ	#2,D0		;for 1st 3 elements
SMOMO1	MOVE.B	(A0)+,D6	;get value
	AND.B	D4,D6		;isolate lower nibble
	ADD.B	D6,D2		;add to running sum
	DBF	D0,SMOMO1	;next element (inner)
	MOVE.B	(A0)+,D6	;add next element double weight
	AND.B	D4,D6
	ADD.B	D6,D2
	ADD.B	D6,D2
	MOVEQ	#2,D0		;for last 3
SMOMO2	MOVE.B	(A0)+,D6
	AND.B	D4,D6
	ADD.B	D6,D2
	DBF	D0,SMOMO2	;next element (inner)
	SUBQ.L	#6,A0		;pt to 2nd element for next time
	LSR.B	#3,D2		;divide sum by 8
	AND.B	D5,2(A0)	;clear lower nibble
	OR.B	D2,2(A0)	;replace with smoothed value
	DBF	D1,SMOMO3	;next element (outer)
*
* Smooth upper nibble
*
	MOVE.L	MOTHPTR(A5),A0
	MOVE.L	MOTHSIZE(A5),D1
	SUBQ.W	#5,D1
	MOVEQ	#$0F,D5		;mask
	MOVEQ	#-16,D3		;mask
	MOVEQ	#4,D4		;shift count
SMOMO6	MOVEQ	#0,D2		;clear sum
	MOVEQ	#2,D0		;for 1st 3
SMOMO4	MOVE.B	(A0)+,D6	;get value
	LSR.B	D4,D6		;shift down
	ADD.B	D6,D2		;add to running sum
	DBF	D0,SMOMO4	;next element (inner)
	MOVE.B	(A0)+,D6	;add next element double weight
	LSR.B	D4,D6
	ADD.B	D6,D2
	ADD.B	D6,D2
	MOVEQ	#2,D0		;for last 3
SMOMO5	MOVE.B	(A0)+,D6
	LSR.B	D4,D6
	ADD.B	D6,D2
	DBF	D0,SMOMO5
	SUBQ.L	#6,A0
	ADD.B	D2,D2		;justify sum/8 to upper nibble
	AND.B	D3,D2		;isolate upper nibble
	AND.B	D5,2(A0)
	OR.B	D2,2(A0)
	DBF	D1,SMOMO6	;next element (outer)

	RTS
	END
