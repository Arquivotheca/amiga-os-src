	TTL	'$Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/tag.asm,v 1.3 90/08/20 10:58:39 andy Exp $'
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
* $Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/tag.asm,v 1.3 90/08/20 10:58:39 andy Exp $
*
* $Locker:  $
*
* $Log:	tag.asm,v $
Revision 1.3  90/08/20  10:58:39  andy
new version from softvoice

Revision 1.2  90/04/03  12:10:41  andy
*** empty log message ***

Revision 1.1  90/04/02  16:40:27  andy
Initial revision

* Revision 32.1  86/01/22  00:24:57  sam
* placed under source control
* 
*
**********************************************************************
        SECTION    speech

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                       ;
*               'F0FILL'                ;
*                                       ;
* TAGS STRESS ARRAY WITH FLAGS THAT     ;
* INDICATE THE START AND END OF THE     ;
* SYLLABIC NUCLEUS AND THE LAST VOICED  ;
* SEGMENT OF THE SYLLABLE. PASS 2 FILLS ;
* THE F0 ELEMENTS OF COEF WITH THE END- ;
* POINTS OF THE F0 GESTURES AT POINTS   ;
* PROPORTIONAL TO THE SIZE OF THE MOVE- ;
* MENTS.                                ;
*                                       ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
*          REGISTER USAGE
*
* D0=PHON  D1=FEATURE  D2=STRESS  D3-D7 UNUSED
* A0=PHON  A1=STRESS  A3=TEMP  A4=FEAT  A2/A5-A6 UNUSED
*
         INCLUDE   'featequ.i'
         INCLUDE   'pcequs.i'
         INCLUDE   'gloequs.i'
	 INCLUDE   'f0equs.i'
*
         XDEF     TAG
         XREF     FEAT
*
TAG      BSR.S    TAGSTR
         BSR      FZFILL
         RTS
*
* CLEAR ALL STRESS #'S TO ZERO
*
TAGSTR   LEA      STRESS(A5),A1
         MOVE.B   #$FF,D0     ;FOR COMPARES
         MOVE.B   #$F0,D1     ;FOR AND'S
TAGCLR   CMP.B    (A1),D0     ;END?
         BEQ.S    TAGCLR1     ;YES, BRANCH
         AND.B    D1,(A1)+    ;NO, CLEAR STRESS #
         BRA      TAGCLR
*
* TAG APPROPRIATE SEGMENTS
* TAGBITS: 1=NUCLEUS START  2=NUCLEUS END
*          4=SEG AFTER LAST VOICED SEG
*
TAGCLR1  LEA      PHON(A5),A0
         LEA      STRESS(A5),A1
         LEA      FEAT,A4
TAG0     MOVEQ    #0,D0
         MOVE.B   (A0)+,D0    ;GET PHON
         CMP.B    #$FF,D0     ;END?
         BNE.S    TAG1        ;NO, BRANCH
         RTS                  ;YES, EXIT
TAG1     LSL.W    #2,D0       ;INDEX FOR FEAT
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         MOVE.B   (A1)+,D2    ;GET STRESS
TAG8     BTST     #7,D2       ;SYLL START?
         BEQ      TAG0        ;NO, BRANCH
TAG3     BTST     #VOWELBIT,D1 ;YES, VOWEL?
         BNE.S    TAG2        ;YES, BRANCH
         MOVEQ    #0,D0       ;NO, KEEP LOOKIN'
         MOVE.B   (A0)+,D0    ;GET NEXT PHON
         LSL.W    #2,D0
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         MOVE.B   (A1)+,D2    ;GET NEXT STRESS
         BTST     #7,D2       ;SYLL START?
         BNE      TAG9        ;YES, TAG LAST PHON
         BTST     #PAUSEBIT,D1 ;NO, PAUSE?              **** MODIFIED 11/28
         BNE      TAG9        ;YES, TAG LAST PHON
         BRA      TAG3        ;NO, BACK TO VOWEL CHECK
TAG2     OR.B     #1,-1(A1)   ;TAG VOWEL AS NUC START
         BTST     #DIPHBIT,D1 ;DIPHTHONG?
         BNE      TAG4        ;YES, BRANCH
         MOVE.B   (A0),D3     ;LOOK AT NEXT PHON
         CMP.B    #PHCLX,D3   ;LX?
         BEQ.S    TAG4        ;YES, TREAT LIKE DIPH
         CMP.B    #PHCRX,D3   ;NO, RX?
         BEQ.S    TAG4        ;YES, TREAT LIKE DIPH
         OR.B     #2,(A1)     ;NO, TAG END OF NUC
         MOVE.L   A1,A3       ;SAVE LOC
         BRA.S    TAG7        ;GET NEXT PHON
TAG4     OR.B     #2,1(A1)    ;TAG END AFTER DIPH'
         MOVE.L   A1,A3       ;SAVE LOC OF VOICED SEG + 1
TAG7     MOVEQ    #0,D0       ;FIND LAST VOICED SEG OF SYLL
         MOVE.B   (A0)+,D0    ;GET NEXT PHON
         LSL.W    #2,D0
         MOVE.L   0(A4,D0),D1 ;GET FEATURE
         MOVE.B   (A1)+,D2    ;GET NEXT STRESS
TAG10    BTST     #7,D2       ;SYLL START?
         BNE.S    TAG6        ;YES, BRANCH
         BTST     #PAUSEBIT,D1  ;NO, PAUSE?             *** MODIFIED 11/28
         BNE.S    TAG6        ;YES, BRANCH
         BTST     #VOICEDBIT,D1  ;NEITHER, VOICED?
         BEQ      TAG7        ;NO, NEXT PHON
         MOVE.L   A1,A3       ;YES, SAVE LOC
         BRA      TAG7        ;GET NEXT PHON
TAG6     OR.B     #4,(A3)     ;TAG LAST VOICED SEG + 1
         BRA      TAG8        ;NEXT SYLL
*
* NO VOWEL FOUND IN SYLLABLE,
* USE LAST PHON AS NUCLEUS
* THIS IS NOT YET HANDLED PROPERLY !!
*
TAG9     OR.B     #1,-2(A1)   ;TAG NUC START
         OR.B     #2,-1(A1)   ;TAG NUC END
         OR.B     #4,-1(A1)   ;TAG LAST VOICED SEG
         BRA      TAG8
*
* SUBROUTINE 'FZFILL'
*
* FILLS F0 ELEMENTS WITH ENDPOINTS OF
* RISES, PEAKS, FALLS AND CONT. RISES
* CONTAINED IN THE SYLLABLE ARRAYS.
*
*          REGISTER USAGE
*
* D0=DUR  D1=COEF INDEX  D2=SCRATCH  D3=START PT
* D4=PEAK  D5=END PT  D6=CR  D7=SCRATCH
* A0=UNUSED  A1=DUR  A2=STRESS  A3=SYLL  A4=UNUSED
* A5=VARS  A6=COEF
*
FZFILL   LEA      START(A5),A3
         LEA      STRESS(A5),A2
         LEA      DUR(A5),A1
         MOVE.L   COEFPTR(A5),A6
*
* CONVERT BASELINE PITCH TO SAMPLES/GLOT PULSE
*
         MOVE.L   #1221000,D0
	 MOVE.L   #F0BOR*11100,D0 ;GRAND CONSTANT (1221000)
         DIVU     FREQ(A5),D0
         AND.L    #$FFFF,D0     ;CLEAR REMAINDER
         MOVE.L   D0,KHZ12(A5)  ;MOVE TO VARSPACE
         CMP.W    #1,F0MODE(A5) ;ROBOTIC MODE?
         BEQ      TAGROBOT      ;YES, DO ROBOTIC STUFF
FZF0     MOVEQ    #0,D0         ;NO, CLEAH EVAHBODY
         MOVEQ    #0,D1
         MOVEQ    #0,D2
         MOVEQ    #0,D3
         MOVEQ    #0,D4
         MOVEQ    #0,D5
         MOVEQ    #0,D6
         MOVEQ    #0,D7
         MOVE.B   (A1)+,D0    ;GET DUR
         CMP.B    #$FF,D0     ;END?
         BNE.S    FZF01       ;NO, BRANCH
         MOVE.W   #256,D1     ;YES, DISPL TO END ARRAY
         MOVEQ    #0,D5
         MOVE.B   -1(A3,D1),D5 ;GET LAST END VALUE
         MOVE.L   KHZ12(A5),D7 ;COMPUTE END HZ
         DIVU     D5,D7
         MOVE.B   D7,-1(A6)   ;PLACE AS LAST F0 ELEMENT
         RTS                  ;EXIT TO MAINLINE
FZF01    AND.B    #$3F,D0     ;ISOLATE DUR
         BTST     #0,(A2)+    ;NUC START?
         BNE.S    FZF1        ;YES, BRANCH
         LSL.W    #3,D0       ;NO, DUR * 8
         ADD.L    D0,A6       ;ADD TO COEF
         BRA      FZF0
FZF1     MOVE.B   (A3),D3     ;GET START PT
         MOVE.L   #128,D7
         ADD.L    D7,A3
         MOVE.B   (A3),D4     ;GET PEAK
         ADD.L    D7,A3
         MOVE.B   (A3),D5     ;GET END PT
         MOVE.B   D5,D6       ;DUPL END PT
         SWAP     D5          ; IN HI WORD
         MOVE.B   D6,D5       ; FOR USE LATER
         ADD.L    D7,A3
         MOVE.B   (A3)+,D6    ;GET CR & INCR
         SUB.L    #384,A3     ;RESET SYLL PTR
         MOVE.L   KHZ12(A5),D7
         DIVU     D3,D7       ;COMPUTE START HZ
         MOVE.B   D7,7(A6)    ;PLACE IN COEF
         AND.B    #$70,D6     ;ISOLATE CR#
         LSR.B    #1,D6       ;CR# * 8HZ
         MOVEQ    #0,D1
         BRA.S    FZF3A
FZF3     MOVEQ    #0,D0       ;FIND NUC END PT
         MOVE.B   (A1)+,D0    ;GET DUR
         AND.B    #$3F,D0     ;ISOLATE
         BTST     #1,(A2)+    ;NUC END?
         BNE.S    FZF2        ;YES, BRANCH
FZF3A    LSL.W    #3,D0       ;DUR * 8
         ADD.W    D0,D1       ;BUMP NUC LENGTH
         BRA      FZF3        ;NEXT PHON
FZF2     MOVE.W   D1,D7       ;SAVE NUC LENGTH
         SWAP     D1          ; IN HI WORD
         MOVE.W   D7,D1       ; FOR USE LATER
         TST.B    D6          ;A CR?
         BEQ.S    FZF2A       ;NO, BRANCH
* IF A CR, PUT END PT 1/2 WAY THRU NUCLEUS
* INSTEAD OF AT END
         LSR.W    #3,D1       ;CONVERT LEN TO FRAMES
         MOVE.W   D1,D7       ;LEN TO SCRATCH
         LSR.W    #1,D7       ;LEN / 2
         SUB.W    D7,D1       ;D1 = 1/2 LEN
         LSL.W    #3,D1       ;CONVERT BACK TO ELEMENTS
FZF2A    MOVE.L   KHZ12(A5),D7
         DIVU     D5,D7       ;COMPUTE END HZ
         MOVE.B   D7,-1(A6,D1) ;PLACE IN COEF
* COMPUTE POSITION OF PEAK IN TIME
         MOVEQ    #0,D7
         MOVE.B   D4,D7       ;PEAK TO SCRATCH
         SUB.W    D3,D7       ;SUB START
         BPL.S    FZF4        ;ABS VALUE
         NEG.W    D7
FZF4     MOVE.W   D7,D2
         LSL.W    #5,D7       ;DIFF * 32
         SUB.W    D4,D5       ;END - PEAK
         BPL.S    FZF5        ;ABS VALUE
         NEG.W    D5
FZF5     ADD.W    D5,D2       ;D2=TOTAL MOVEMENT (HZ)
         BEQ.S    FZF5A       ;IF ZERO SKIP PEAK PLACEMENT
         DIVU     D2,D7       ;PEAK POSITION RATIO
         MULU     D1,D7       ;*DUR
         LSR.W    #8,D7       ;NORMALIZE, D7=FRAME OF PEAK
         LSL.W    #3,D7       ;D7=ELEMENT PEAK POS
         MOVE.L   KHZ12(A5),D2
         DIVU     D4,D2       ;COMPUTE PEAK HZ
         MOVE.B   D2,-1(A6,D7)  ;PLACE IN COEF
FZF5A    SUBQ.L   #1,A1       ;PT TO NUC END 'CAUSE
         SUBQ.L   #1,A2       ; IT MIGHT BE A NUC START
         CLR.W    D1          ;CLEAR LO BEFORE SWAPPING
         SWAP     D1          ;GET NUC LENGTH BACK
         ADD.L    D1,A6       ;PT COEF AT NUC END
         TST.B    D6          ;A CR?
         BEQ      FZF0        ;NO, NEXT SYLLABLE
FZF7     MOVEQ    #0,D0       ;YES, GET DUR
         MOVE.B   (A1)+,D0
         AND.B    #$3F,D0     ;ISOLATE
         BTST     #2,(A2)+    ;LAST VOICED?
         BNE.S    FZF6        ;YES, BRANCH
         LSL.W    #3,D0       ;NO, DUR * 8
         ADD.L    D0,A6       ;ADD TO COEF
         BRA      FZF7        ;NEXT PHON
FZF6     SWAP     D5          ;GET END VALUE BACK
         ADD.W    D6,D5       ;ADD CONT. RISE
         MOVE.L   KHZ12(A5),D7   ;COMPUTE CR
         DIVU     D5,D7
         MOVE.B   D7,-1(A6)   ;PLACE IN COEF
         SUBQ.L   #1,A1       ;PT BACK AT WHERE
         SUBQ.L   #1,A2       ; COEF IS POINTING
         BRA      FZF0        ;NEXT SYLLABLE
*
* FILLS PITCH VALUES IN COEF BUFFER WITH MONOTONE
*
TAGROBOT MOVE.L   COEFPTR(A5),A6 ;PT TO COEF BUFFER
         MOVE.L   KHZ12(A5),D7   ;COMPUTE PITCH PERIOD
         DIVU     #F0BOR,D7      ;NORMALIZE FOR CORRECT PITCH
         ADDQ.L   #7,A6          ;PT TO PITCH ENTRIES
TAGROB1  CMP.B    #$FF,(A6)      ;END OF BUFFER?
         BEQ.S    TAGROB2        ;YES, BRANCH
         MOVE.B   D7,(A6)        ;MOVE IN PITCH VALUE
         ADDQ.L   #8,A6          ;PT TO NEXT FRAME
         BRA      TAGROB1
TAGROB2  RTS

         END 
