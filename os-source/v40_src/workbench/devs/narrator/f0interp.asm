
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

	TTL	'F0INTERP.ASM'
               SECTION     speech

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                       ;
*       INTERPOLATION  MODULE           ;
*                                       ;
* LINEARLY INTERPOLATES BETWEEN DATA    ;
* POINTS IN THE COEF BUFFER WHICH ARE   ;
* SITUATED AT INTERVALS OF 8 BYTES.     ;
* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	 INCLUDE   'exec/types.i'	
         INCLUDE   'featequ.i'
         INCLUDE   'pcequs.i'
         INCLUDE   'gloequs.i'
*
         XDEF     F0INTERP

*
F0INTERP:
         MOVEQ    #0,D0       ;CLEAR STUFF
         MOVEQ    #0,D1
         MOVEQ    #0,D2
         MOVEQ    #0,D3
         MOVEQ    #0,D4
         MOVEQ    #0,D5
         MOVEQ    #0,D6
         MOVEQ    #0,D7
*
* INTERPOLATE F0 in COEF BUFFER
*
         MOVE.L   COEFPTR(A5),A0
         ADDQ.L   #7,A0			;point to first F0 entry
         BSR      SEARCH
         MOVE.L   COEFPTR(A5),A6
         ADDQ.L   #7,A6
         BSR      SMOOTH
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
         MOVEQ    #0,D6
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
*    SUBROUTINE 'SEARCH'
*
* SCAN THE COEF BUFFER LOOKING FOR NON-CLEAR
* DATA POINTS.  THESE LOCATIONS ARE PASSED TO
* INTERP8.
*
SEARCH   MOVEQ    #0,D1
SCH1     ADDQ.W   #8,D1
         TST.B    0(A0,D1)    ;CLEAR PT?
         BEQ      SCH1        ;YES, BRANCH
         CMP.B    #$FF,0(A0,D1) ;NO, END?
         BNE.S    SCH2        ;NO, BRANCH
         RTS
SCH2     CMP.W    #8,D1       ;OFFSET ONLY 1?
         BNE.S    SCH5        ;NO, BRANCH
         ADDQ.L   #8,A0       ;YES, DO NOT INTERP
         BRA      SEARCH
SCH5     BSR      INTERP8
         BRA      SEARCH

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

	END
