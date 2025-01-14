
		TTL	'F0LLS.ASM'



*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


*************************************************************************
*									*
*      F0 LOW LEVEL SYSTEM						*
*									*
*          ALL REGISTERS DESTROYED					*
*									*
*          REGISTER DEFINITIONS						*
*          A0 -                          A1 -				*
*          A2 -                          A3 -				*
*          A4 - ACCENT ARRAY             A5 - GLOBALS AREA		*
*          A6 - PEAK							*
*									*
*          D0 THRU D2 - SCRATCH						*
*          D3 - NUMBER OF AS             D4 - NUMBER OF SYLLABLES	*
*          D5 - POSITION OF FIRST AS     D6/D7 - SCRATCH		*
*									*
*									*
*    4/18/89  JK  --	Added F0 manual mode and F0enthusiasm for both	*
*			manual and natural modes.  This involves mods	*
*			to F0B3B, F0D, and F0F code.  See Note 1.	*
*									*
*    5/24/90  JK  --	F0BManual code needs to check F0NUMSYL		*
*			to insure at lease two syllables in utterance.  *
*									*
*    1/5/91   JK  --  	Put in test of F0NUMAS(A5) in F0G section.      *
*			This done to fix gurgling problem.  See log.	*
*			Note 2.						*
*									*
*    3/4/91   JK  --	In F0HDPEAK code, where clamping of Head Peak	*
*			is done.  Modified MOVE.B's to MOVE.W's.  This	*
*			was necessary to prevent pitch going wild.  	*
*			NUMCLS's was getting too large, causing the peak*
*			to become negative.  The MOVE.B's were leaving	*
*			crud in the upper nybble.	Note 3		*
*									*
*************************************************************************

          SECTION      speech

*          .NOMACROLIST

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


          XREF	    FEAT
          XDEF      F0LLS

*
*         ADDPCT macro definition.  Note that Manx assembler gets 
*	  easily confused, so this macro should only be changed with
*	  considerable care.  The actual problem is that the branch
*	  instructions get created with the wrong length offsets.  The
*	  symptom is that the linker hangs.
*
ADDPCT   MACRO    ADDPCT                ;ADD N% OF 0(Ax,Dx)-CONST TO 0(Ay,Dy)
*                                       ;    %1     %2  %3   %4       %5  %6
MACST\@  MOVEQ     #0,\7                ;  USING D REG SPECIFIED AS %7 AS SCRATCH
          MOVE.B    \2,\3,\7            ;GET 0(Ax,Dx)
*          IFNC       '\2','0(A6'        ;IF NOT USING PEAK ARRAY (ALWAYS A6)
	  IFEQ	    \4
          EXT.W     \7                  ;    SIGN EXTEND TO WORD
          ENDC
          IFNE      \4
          SUB.W     #\4,\7              ;IF NON-ZERO CONST, SUB FROM 0(Ax,Dx)
          ENDC
          MULS      \1,\7               ;MULTIPLY IN 128THS
          IFNE      MAC
          BPL.S     *+8
	  ENDC
	  IFNE      AMIGA
          BPL.S	    1$
	  ENDC
          SUB.W     #64,\7              ;ROUND IF NEGATIVE
	  IFNE	    MAC
          BRA.S     *+6
	  ENDC
	  IFNE	    AMIGA
	  BRA.S     2$
	  ENDC
1$        ADD.W     #64,\7              ;ROUND IF POSITIVE
2$        ASR.W     #7,\7               ;CONVERT TO PERCENTAGES
MACEND\@  ADD.B     \7,\5,\6            ;ADD TO 0(Ay,Dy)

          ENDM


ENTHUSFT  EQU	    5			;F0ENTHUSIASM IS IN 32NDS


F0LLS     MOVEQ     #0,D0               ;CLEAR ALL D REGS.  THIS IS NECESSARY
          MOVEQ     #0,D1               ;     BECAUSE THE MOVE.W'S BELOW WILL
          MOVEQ     #0,D2               ;     NOT CLEAR THE UPPER HALF OF REG
          MOVEQ     #0,D3
          MOVEQ     #0,D4
          MOVEQ     #0,D5
          MOVEQ     #0,D6
          MOVEQ     #0,D7

          MOVE.W    F0NUMAS(A5),D3      ;RESTORE SOME USEFUL COUNTS
          MOVE.W    F0NUMSYL(A5),D4     ;  D3 THROUGH D5 ARE GENERALLY
          MOVE.W    F01STAS(A5),D5      ;  PRESERVED THROUGH ALL SUBROUTINES
          MOVE.W    F0NUMCLS(A5),D6
          MOVE.W    F0NUMPHS(A5),D7
          CMP.W     #1,F0MODE(A5)
          BEQ.S     ROBOTF0
          TST.W     D3
          BEQ.S     F0DOF0E             ;IF NO AS'S, SKIP B, C, AND D RULES
          BSR       F0HDPEAK
          BSR       F0B
          BSR       F0C
          BSR       F0D
F0DOF0E   BSR       F0E
          BSR       F0F
          BSR       F0G
*          BRA.S     F0ADJ


*
*      ADJUST SYLLABLE ARRAY POINTERS FOR NEXT CLAUSE
*      (IF ANY).
*
ROBOTF0:					;Robotic stuff done in phonetf0
F0ADJ     LEA      F0START(A5),A0
          MOVEQ    #7,D0
F0SYLADJ  ADD.L    D4,(A0)+
          DBF      D0,F0SYLADJ
*
*        RETURN TO MAIN TO DEAL WITH NEXT CLAUSE (IF ANY)
*
          RTS

*-----------------------------------------------------------------------------
*
*         ASSIGN HEAD PEAK
*
*         On entry, D7 contains the number of words in the clause
*                   D6 contains the clause number
*
*-----------------------------------------------------------------------------

F0HDPEAK: 
	  MOVE.L   F0PEAK(A5),A6
          ADDQ.W   #4,D7                ;FAKING NUMBER OF PHRASES AS
          DIVU     #3,D7                ; # OF (WORDS+4)/3
          MOVEQ    #F012HZ,D0		;COMPUTE "12 - 2*#CLAUSES" HZ
          SUB.W    D6,D0                ;SUBTRACT NUMBER OF CLAUSES
	  IFEQ	   F0USE2HZ	      	;USING 2HZ STEPS?
          SUB.W	   D6,D0                ;NO, SUBTRACT ANOTHER "#CLAUSES" HZ
	  ENDC
          BPL.S    F0HDPK2              ;CLAMP AT 0
          MOVEQ    #0,D0
F0HDPK2   MULU     D7,D0                ;#PHR*(12-2*#CLAUSES)
          ADD.W    #F0HDBASE,D0         ;BASELINE + #PHR*(12-2*#CLAUSES)
          MOVE.W   D6,D1                ;
	  IFEQ	   F0USE2HZ		;2HZ STEPS? 
          LSL.W    #3,D1                ;NO, USE 8*#CLAUSES HZ
	  ELSE
	  LSL.W	   #2,D1		;YES, USE 4*#CLAUSES 2HZ STEPS
	  ENDC
          SUB.W    D1,D0                ;BASELINE - 8*#CLS + #PHR*(12-2*#CLS)

          CMP.W    #F0MINHPK,D0         ;Check against min head peak
          BGT.S    F0HDPK0		;Below minimum?
          MOVE.W   #F0MINHPK,D0         ;  yes, set to MIN peak (see Note 3)
	  BRA.S	   F0HDPK1		;       and store
F0HDPK0   CMP.W    #F0MAXHPK,D0         ;Check against max head peak
          BLT.S    F0HDPK1		;Above maximum?
          MOVE.W   #F0MAXHPK,D0         ;  yes, set to MAX peak (see Note 3)
F0HDPK1   MOVE.B   D0,0(A6,D5)          ;       and store

          RTS


*----------------------------------------------------------------------------
*
*         COMPUTE PEAKS FOR ACCENTED SYLLABLES
*
*         On entry, D0 contains HEAD PEAK calculated in F0HDPEAK
*
*----------------------------------------------------------------------------

F0B       MOVE.L    F0TUNELV(A5),A1
          MOVE.L    F0ACCENT(A5),A4
	  MOVE.W    F0NUMAS(A5),D3      ;GET NUMBER OF AS'S 
          MOVE.B    #F0110HZ,D1         ;DEFAULT TO TUNE A
          MOVE.B    -1(A1,D4),D2        ;GET TUNE/LEVEL OF LAST SYL
          AND.B     #F0TUNEMK,D2        ;ISOLATE TUNE
          CMP.B     #F0TUNEB,D2         ;CHECK TUNE
          BNE.S     F0B2AX0             ;TUNE A OR NO TUNE
          MOVE.B    #F0125HZ,D1         ;TUNE B

F0B2AX0   SUB.B     D1,D0               ;COMPUTE F0 ROOM FOR HEAD PEAK

F0B2B     DIVU      D3,D0               ;D0 = DROP (ALL THESE ARE BYTE QUANTITIES,
          MOVE.W    D0,D1               ;D1 = DROP   BUT DOING WORD MOVES TO CLEAR
          MOVE.W    D0,D2               ;D2 = DROP   UPPER BYTE IN WORD)
          CMP.W     #4,D3
          BLT.S     F0B2BX1             ;BRANCH IF LESS THAN 4 AS'S
          MULU      #F0PCT15,D1         ;
          LSR.W     #7,D1               ;D1 = 15% OF DROP
          MOVE.W    D1,D2               ;
          LSL.W     #1,D2               ;D2 = 30% OF DROP
          SUBQ.W    #3,D3               ;MODIFY # OF AS'S
          DIVU      D3,D2               ;D2=DROP/(#AS-3)
          NEG.W     D2
          ADD.W     D0,D2               ;D2=DROP-(30%/(#AS-3))
          ADD.W     D0,D1               ;D1=DROP+15%

F0B2BX1	  MOVE.B    D1,D7               ;INITIALIZE D7 TO LARGER DROP
          MOVE.B    0(A6,D5),D6         ;GET HEAD PEAK
          MOVE.W    D5,D0
          ADDQ.W    #1,D0               ;BEGIN AFTER 1ST AS
F0B2BX2   BTST      #F0SSBIT,0(A4,D0)   ;STRESSED?
          BEQ.S     F0B2BX3             ;NO, BRANCH
          CMP.W     F0LASTAS(A5),D0     ;LAST AS?
          BNE.S     F0B2BX4             ;    NO, USE SMALLER DROP
          MOVE.B    D1,D7               ;   YES, USE LARGER DROP
F0B2BX4   SUB.B     D7,D6               ;COMPUTE NEXT AS PEAK
          MOVE.B    D6,0(A6,D0)         ;STORE
          MOVE.B    D2,D7               ;DEFAULT TO SMALLER DROP
F0B2BX3   ADDQ.W    #1,D0
          CMP.W     D0,D4               ;LAST SYLLABLE?
          BGT.S     F0B2BX2             ;NO, CONTINUE


*-------- Level modiications

F0B3A     MOVE.W    F0LASTAS(A5),D1     ;GET LAST AS
          MOVE.W    D1,D2               ;SETUP D2 AND D1 TO POINT TO LAST AS

F0B3AX0   BTST      #F0SSBIT,0(A4,D1)   ;STRESSED SYLLABLE?
          BEQ.S     F0B3AXEND           ;
          MOVE.B    0(A1,D1),D7         ;GET TUNE/LEVEL ELEMENT
          AND.B     #F0LEVMK,D7         ;ISOLATE LEVEL ELEMENT
          SUBQ.B    #1,D7               ;ADJUST TO -1, 0, +1
          BEQ.S     F0B3AX2             ;IF ZERO, NO MODS
          BLT.S     F0B3A2              ;


*-------- Positive level

F0B3A1    CMP.W     F01STAS(A5),D1      ;ANY MORE AS'S?
          BLE.S     F0B3B               ;NO, GO DO ACCENT MODS
          MOVE.W    D1,D0               ;FIND AS ON LEFT (IF ANY)
          SUBQ.W    #1,D0               ;START TO LEFT OF D1
F0B3A1X1  BTST      #F0SSBIT,0(A4,D0)
          DBNE      D0,F0B3A1X1
          MOVE.B    0(A6,D1),D7         ;GET PEAK(I)
          SUB.B     0(A6,D0),D7         ;PEAK(I) - PEAK(I-1)
          EXT.W     D7
          TST.W     D7
          BPL.S     F0B3AX1
          NEG.W     D7                  ;INSURE POSITIVE DIFFERENCE
          BRA.S     F0B3AX1


*-------- Negative level

F0B3A2    MOVE.B    0(A6,D1),D7         ;GET PEAK(I)
          SUB.B     0(A6,D2),D7         ;PEAK(I) - PEAK(I+1)
          EXT.W     D7                  ;EXTEND TO WORD
          TST.W     D7                  ;
          BMI.S     F0B3AX1             ;
          NEG.W     D7                  ;INSURE NEGATIVE DIFFERENCE

F0B3AX1   MULS      #F0PCT40,D7         ;
          LSR.L     #7,D7               ;COMPUTE 40% OF DIFFERENCE
          ADD.B     D7,0(A6,D1)         ;MODIFY PEAK(I)

F0B3AX2   MOVE.W    D1,D2               ;MOVE D2 ONE AS TO LEFT
F0B3AXEND DBF       D1,F0B3AX0          ;LOOP


*-------- Accent modification to AS peaks

F0B3B	  MOVEQ	    #F0PCT10,D7			;INIT D7 TO 10% OF LOCAL ROOM
	  MOVE.L    USERIORB(A5),A1		;GET IORB POINTER
	  BTST	    #NDB_NEWIORB,NDI_FLAGS(A1)	;IS THIS THE EXTENDED IORB?
	  BEQ.S	    F0B3BX0		  	;   NO, USE 10% OF LOCAL ROOM
	  CMP.W	    #MANUALF0,F0MODE(A5)	;   YES, IN MANUAL MODE?
	  BEQ.S	    F0BMANUAL			;      YES, BRANCH
*	  MOVE.B    NDI_F0ENTHUSIASM(A1),D7	;      NO, DO NATURAL MODE
*	  MULU	    #F0PCT10,D7			;          WITH D7=ENTHUS*10%
*	  ASR.L	    #ENTHUSFT,D7		;	   (ENTHUS IS IN 32NDS)

F0B3BX0	  MOVE.W    F0LASTAS(A5),D0     	;GET POSITION OF LAST AS

F0B3BX1   MOVEQ     #0,D1
          MOVEQ     #0,D2
	  MOVEQ	    #0,D3
          MOVE.B    0(A4,D0),D1         ;GET ACCENT BYTE
          BTST      #F0SSBIT,D1         ;STRESSED SYLLABLE?
          BEQ.S     F0B3BX2
          AND.B     #F0ACNTNO,D1        ;YES, ISOLATE ACCENT NUMBER
          SUBQ.W    #8,D1               ;SUBTRACT 8
          MOVE.B    0(A6,D0),D2         ;GET PEAK
          SUB.W     #F0BOR,D2           ;D2 = LOCAL ROOM
          MULU      D7,D2         	;10% IF OLD IORB, 10%*ENTHUS IF NEW IORB
          LSR.W     #7,D2               ;D2 = 10% OF LOCAL ROOM (OR 10%*ENTHUS)
          MULS      D2,D1               ;D1 = (ACCT-8)*10% OF LOCAL ROOM
	  MOVE.B    0(A6,D0),D3		;GET PEAK 		* Note 1
	  ADD.L     D1,D3		;ADD IN PEAK MOD	* Note 1
     	  CMP.L	    #F0BOR,D3		;CLAMP AT BOR		* Note 1
	  BGE.S	    F0B3BX3					* Note 1
	  MOVE.B    #F0BOR,D3					* Note 1
	  BRA.S	    F0B3BX4		;STORE
F0B3BX3	  CMP.L	    #F0MAXHZ,D3		;CLAMP AT 400 HZ	* Note 1
	  BLE.S	    F0B3BX4					* Note 1
	  MOVE.B    #F0MAXHZ,D3					* Note 1
F0B3BX4   MOVE.B    D3,0(A6,D0)         ;ADD TO PEAK		* Note 1
F0B3BX2   DBF       D0,F0B3BX1

	  BRA.S	    F0BRTS


F0BMANUAL MOVE.W    F0NUMSYL(A5),D0		;NUMBER OF SYLLABLES
	  SUBQ.W    #1,D0			;ADJ FOR DBF AND PEAK ADJ VALUE
	  BEQ.S	    F0BRTS			;Only 1 syl, see note 4
	  MOVEQ	    #0,D3			;INDEX INTO PEAK ARRAY
	  MOVEQ	    #0,D6			;HEAD PEAK
	  MOVE.B    0(A6,D5),D6			;GET HEAD PEAK
	  MOVE.L    D6,D1			;COMPUTE HEAD PEAK ROOM       
	  SUB.L	    #F0BOR,D1			;
	  MOVE.L    D1,D7			;SAVE FOR MANUAL PEAK ADJ
    	  DIVU	    D0,D1			;DECLINATION LINE ADJUSTMENT
	  MULS	    #F0PCT10,D7			;USE 10% OF HEAD PEAK ROOM FOR
	  ASR.L	    #7,D7		       	;   MANUAL PEAK ADJUSTMENT
	  
F0BMANX0  MOVE.B    0(A4,D3),D2         	;GET ACCENT BYTE
          AND.L     #F0ACNTNO,D2            	;ISOLATE ACC # & CLEAR FOR MULT
*	  MULS	    D7,D2			;MOD = 10% OF HEAD PEAK ROOM*ACC
	  MULS	    #F05HZ,D2			;MOD = ACCENT*5Hz
	  ADD.L	    D6,D2			;PEAK + MOD

	  CMP.L	    #F0MAXHZ,D2			;CLAMP AT 400 HZ
	  BLT.S	    F0BMANX1
	  MOVE.B    #F0MAXHZ,D2
F0BMANX1  MOVE.B    D2,0(A6,D3)			;STORE IN PEAK ARRAY
	  SUB.B	    D1,D6			;D6 = NEW LOCAL PEAK
	  ADDQ.W    #1,D3			;BUMP INDEX
	  DBF       D0,F0BMANX0			;LOOP THRU SYLLABLES

F0BRTS	  RTS


*-----------------------------------------------------------------------------
*
*         ASSIGN RISES AND FALLS FOR ACCENTED SYLLABLES
*
*-----------------------------------------------------------------------------

F0C       MOVE.L    F0CRBRK(A5),A1      ;BREAK ARRAY
          MOVE.L    F0FALL(A5),A2       ;FALL ARRAY
          MOVE.L    F0RISE(A5),A3       ;RISE ARRAY
          MOVE.W    F0LASTAS(A5),D0
F0CX1     BTST      #F0SSBIT,0(A4,D0)   ;STRESSED SYLLABLE?
          BEQ       F0CXEND


*-------- COMPUTE RISE ACCENTS ------------------------------------------------

F0C1A     MOVEQ     #0,D1
          MOVEQ     #0,D2
          MOVE.B    0(A6,D0),D1         ;GET PEAK
          SUB.W     #F0BOR,D1           ;D1 = LOCAL ROOM
          MOVE.B    0(A1,D0),D2         ;D2 = BREAK/CR ELEMENT
          AND.B     #F0BRMASK,D2        ;ISOLATE BREAK NUMBER
          BTST      #3,D2               ;TEST FOR NEGATIVE BREAK
          BEQ.S     F0C1AX0             ;
          OR.W      #$FFF0,D2           ;IF NEGATIVE NYBLE, SIGN EXTEND TO WORD
F0C1AX0
          MOVE.W    D2,D7               ;BREAK NUMBER
          MULS      #F0PCT20,D7         ;20% OF BREAK NUMBER (DONT SHIFT YET)
          ADD.L     #128,D7             ;1 + 20% OF BREAK NUMBER (IN 128THS)
          MULS      D1,D7               ;ROOM*(1 + 20% OF BREAK)
          LSR.L     #7,D7               ;ADJUST FOR PERCENTAGE
          MULS      #F0PCT40,D7         ;
          LSR.L     #7,D7               ;40% OF 120%*ROOM*BREAK NUMBER
          MOVE.B    D7,0(A3,D0)         ;NEW RISE


*-------- COMPUTE FALL ACCENTS ---------------------------------------------

F0C2A     MOVE.W    D2,D7               ;BREAK NUMBER
          SUBQ.W    #1,D7               ;
          MULS      D1,D7               ;ROOM*(BREAK-1)
          MULS      #F0PCT20,D7         ;
          LSR.L     #7,D7               ;20% OF ROOM*(BREAK-1)
          NEG.B     D7                  ;INVERT SIGN
          BPL.S     F0C2AX0             ;CLAMP D7 TO INSURE POSITIVE VALUE
          MOVEQ     #0,D7
F0C2AX0   MOVE.B    D7,0(A2,D0)         ;NEW FALL


*-------- MODIFY ACCENTS WITHIN P-UNITS -------------------------------------

F0C3      BTST      #F0INPUNB,0(A4,D0)  ;IN P-UNIT?
          BEQ.S     F0CXEND             ;NO, SKIP SECTION C3 MODS
          TST.B     D2                  ;CHECK FOR ZERO BREAK
          BNE.S     F0CXEND             ;NON-ZERO BREAK, SKIP C3 MODS

          ADDPCT    #-F0PCT30,0(A3,D0),0,0(A3,D0),D2        ;REDUCE RISE BY 30%
          ADDPCT    #-F0PCT30,0(A2,D0),0,0(A2,D0),D2        ;REDUCE FALL BY 30%


F0CXEND   DBF       D0,F0CX1


*-------- ASSIGN RISE FOR HEAD SYLLABLE -------------------------------------

F0C1C     MOVE.B    0(A6,D5),0(A3,D5)   ;MOVE PEAK OF 1ST AS TO RISE
          SUB.B     #F0BOR,0(A3,D5)     ;RISE = PEAK - BOR

          RTS


*-----------------------------------------------------------------------------
*
*         PHONETIC MODIFICATIONS
*
*-----------------------------------------------------------------------------

F0D	  MOVE.L    F0CRBRK(A5),A0      ;CR ARRAY
          MOVE.L    F0TUNELV(A5),A1     ;TUNE/LEVEL ARRAY

          CMP.W	    #MANUALF0,F0MODE(A5)	;IF MANUAL F0 MODE,	* Note 1
	  BEQ	    F0D2			;   SKIP PHON MODS	* Note 1

          MOVE.W    F0LASTAS(A5),D1     ;POSTION OF LAST AS
          MOVE.W    D1,D0               ;COPY TO "PREVIOUS AS POINTER"
          BRA       F0DXEND             ;START LOOP

F0DX0     BTST      #F0SSBIT,0(A4,D0)   ;STRESSED SYLLABLE?
          BEQ       F0DXEND             ;NO, CONTINUE LOOKING

          MOVE.W    D1,D2               ;COMPUTE NUMBER OF US'S BETWEEN AS'S
          SUB.W     D0,D2
          SUBQ.W    #2,D2
          BEQ       F0D1X0              ;1 US BETWEEN AS'S, NO MODS
          BPL       F0D1B               ;MORE THAN 1 US BETWEEN AS'S


*-------- IF 2 AS'S ARE ADJACENT -----------------------------------------------

F0D1A     ADDPCT    #-F0PCT40,0(A3,D0),0,0(A3,D0),D2        ;REDUCE 1ST RISE 40%

          ADDPCT    #-F0PCT40,0(A3,D1),0,0(A3,D1),D2        ;REDUCE 2ND RISE 40%

          ADDPCT    #-F0PCT20,0(A6,D0),F0BOR,0(A6,D0),D2    ;REDUCE 1ST PEAK 20%LR

          ADDPCT    #F0PCT20,0(A6,D1),F0BOR,0(A6,D1),D2     ;RAISE 2ND PEAK 20%LR


*         INCREASE FALL ACCENT OF FIRST OR RISE ACCENT
*                 OF SECOND SO THAT END POINTS MEET

          MOVE.B    0(A6,D0),D2   ;GET PEAK OF 1ST AS
          SUB.B     0(A2,D0),D2   ;PEAK(1ST) - FALL(1ST)
          SUB.B     0(A6,D1),D2   ;PEAK(1ST) - FALL(1ST) - PEAK(2ND)
          ADD.B     0(A3,D1),D2   ;PEAK(1ST) - FALL(1ST) - (PEAK(2ND) - RISE(2ND))
          BPL.S     F0D1AX1       ;
          SUB.B     D2,0(A3,D1)   ;PEAK-RISE > PEAK-FALL, INCREASE RISE OF 2ND AS
          BRA       F0D1X0

F0D1AX1   ADD.B     D2,0(A2,D0)   ;PEAK-RISE < PEAK-FALL, INCREASE FALL OF 1ST AS
          BRA       F0D1X0


*-------- ADJUST RISE ACCENTS IF AS'S ARE NOT ADJACENT -------------------------

F0D1B     MOVE.W    D2,-(SP)            ;SAVE COUNT OF INTERVENING US'S
          MOVE.W    #F0PCT15,D7         ;1 EXTRA US (TWO INTERVENING US'S)
          SUBQ.W    #1,D2
          BEQ.S     F0D1BX1
          ADD.W     #F0PCT10,D7         ;2 EXTRA US'S
          SUBQ.W    #1,D2
          BEQ.S     F0D1BX1
          ADD.W     #F0PCT5,D7          ;3 OR MORE EXTRA US'S

F0D1BX1   ADDPCT    D7,0(A3,D0),0,0(A3,D0),D2     ;INCREASE RISE OF 1ST AS

          ADDPCT    D7,0(A3,D1),0,0(A3,D1),D2     ;INCREASE RISE OF 2ND AS



*-------- ADJUST PEAKS IF AS'S ARE NOT ADJACENT -------------------------------

F0D1C     MOVE.W    (SP),D2             ;RESTORE COUNT OF INTERVENING US'S
*                                        ;NOTE THAT WE LEAVE D2 ON STACK FOR
*                                        ;LATER USE
          MOVE.W    #-F0PCT15,D7        ;2 US'S
          SUBQ.W    #1,D2
          BEQ.S     F0D1CX1
          SUB.W     #F0PCT10,D7         ;3 OR MORE US'S

F0D1CX1   ADDPCT    D7,0(A6,D1),F0BOR,0(A6,D1),D2           ;DECREASE 2ND PEAK


F0D1D     MOVE.W    (SP),D2             ;RESTORE COUNT, KEEP ON STACK
          MOVE.W    #F0PCT10,D7         ;2 US'S
          SUBQ.W    #1,D2
          BEQ.S     F0D1DX1
          ADD.W     #F0PCT5,D7          ;3 OR MORE US'S

F0D1DX1   ADDPCT    D7,0(A6,D0),F0BOR,0(A6,D0),D2           ;INCREASE 1ST PEAK


*-------- 3 AS'S IN SUCCESSION ------------------------------------------------

F0D1E     NOP                           ;THIS MOD NOT IMPLEMENTED


*-------- MOD TO RISE OF AS NOT IN NOR AT END OF P-UNIT -----------------------

F0D1F     MOVE.W    (SP)+,D2            ;GET # OF US'S AND FIX STACK
          SWAP      D2                  ;SAVE IN UPPER HALF
          BTST      #F0INPUNB,0(A4,D1)  ;OUTSIDE P-UNIT?
          BEQ.S     F0D1FX0             ;YES, CHECK IF 3+ US'S
          MOVE.B    0(A0,D1),D2         ;GET CR/BREAK ELEMENT
          AND.B     #F0BRMASK,D2        ;ISOLATE BREAK ELEMENT
          BEQ.S     F0D1X0              ;NO BREAK, SKIP D1F MOD
          BTST      #3,D2               ;
          BNE.S     F0D1X0              ;-IVE BREAK, SKIP D1F MOD

*         2ND AS OUTSIDE OR AT BEGINNING OF P-UNIT

F0D1FX0   SWAP      D2                  ;RESTORE # OF INTERVENING US'S
          CMP.W     #2,D2
          BLT.S     F0D1X0              ;LESS THAN 3 INTERVENING US'S
          MOVE.B    0(A6,D1),D2         ;GET PEAK
          SUB.B     #F0BOR-F05HZ,D2     ;PEAK-(BOR-5)
          MOVE.B    D2,0(A3,D1)         ;STORE AS RISE

F0D1X0    MOVE.W    D0,D1               ;UPDATE CURRENT AS POSITION
F0DXEND   DBF       D0,F0DX0


*-------- COMPUTE ACCENTS OF TERMINALS ---------------------------------------

F0D2      MOVE.W    F0LASTAS(A5),D0               ;COUNT OF SYLLABLES
F0D2X0    BTST      #F0SSBIT,0(A4,D0)             ;STRESSED?
          BEQ.S     F0D2XEND                      ;NO
*
F0D2A     BTST      #F0WORDFB,0(A4,D0)            ;WORD FINAL
          BEQ.S     F0D2XEND                      ;NO, NEXT SYLLABLE
          MOVE.B    0(A1,D0),D1                   ;GET TUNE ARRAY
          AND.B     #F0TUNEMK,D1                  ;ISOLATE TUNE
          CMP.B     #F0TUNEB,D1                   ;TUNE B?
          BEQ.S     F0D2B                         ;YES
          CMP.B     #F0TUNEA,D1                   ;TUNE A?
          BNE.S     F0D2XEND                      ;NO TUNE

*-------- TUNE A, USE F0TERM AS ENDING POINT --------------------------------

F0D2A1    MOVEQ     #0,D7

          IFNE      MAC
          MOVE.B    Ticks+3,D7                    ;GET LOW BYTE OF CLOCK
          AND.B     #$0F,D7                       ;USE LOW NYBBLE
          ENDC

          ADD.B     0(A6,D0),D7                   ;GET PEAK
          SUB.B     #F0TERM,D7                    ;PEAK-F0 TERM
          MOVE.B    D7,0(A2,D0)                   ;SET FALL
          BRA.S     F0D2XEND                      ;CONTINUE

*-------- TUNE B, USE 120% OF PREVIOUS PEAK AS ENDING POINT ------------------
*                 AND TRANSFER 80% OF FALL TO BE ADDITIONAL RISE

F0D2B     ADDPCT    #F0PCT80,0(A2,D0),0,0(A3,D0),D1         ;80% FALL TO RISE
*
F0D2A2    MOVEQ     #0,D7                         ;CLEAR FOR COMPARE
          MOVE.W    D0,D2
          MOVE.B    0(A6,D2),D7                   ;GET PEAK
          BRA.S     F0D2A2X1
F0D2A2X0  MOVE.B    0(A6,D2),D1                   ;GET PEAK
          CMP.W     D1,D7                         ;COMPARE PEAKS (UNSIGNED VALUES)
          BGT.S     F0D2A2X1                      ;NOT BIGGER
          MOVE.B    D1,D7                         ;NEW MAX
F0D2A2X1  DBF       D2,F0D2A2X0                   ;LOOP

          MULU      #F0PCT20+F0PCT100,D7          ;GET 120%
          LSR.W     #7,D7                         ;COMPUTE PERCENTAGE
          MOVE.B    0(A6,D0),D1                   ;GET PEAK WITH TUNE
          SUB.B     D7,D1                         ;CURRENT PEAK - 120% OF PREV HI PEAK
          MOVE.B    D1,0(A2,D0)                   ;SET FALL
*
F0D2XEND  DBF       D0,F0D2X0

          RTS

*-----------------------------------------------------------------------------
*
*         ASSIGN CONTINUATION RISES
*
*-----------------------------------------------------------------------------
F0E       MOVE.L    F0FALL(A5),A2       ;
          MOVE.L    F0CRBRK(A5),A0      ;

F0E1      NOP                           ;THIS RULE IS DONE IN "F0FILL"


F0E2      MOVEQ     #0,D1
          MOVE.W    D4,D0               ;COUNT OF SYLLABLES
          BRA.S     F0E2X2              ;START LOOP
*
F0E2X0    MOVE.B    0(A0,D0),D1         ;GET CR/BREAK BYTE
          AND.B     #F0CRMASK,D1        ;ISOLATE CR TYPE AND NUMBER
          BEQ.S     F0E2X2              ;NO CR
          MOVE.W    #-F0PCT80,D2        ;ASSUME MONOTONIC, DECREASE FALL BY 80%
          BTST      #F0CRTYPE,D1        ;FALL+RISE OR MONOTONIC
          BEQ.S     F0E2X1              ;BIT OFF ==> MONOTONIC
          MOVE.W    #F0PCT30,D2         ;FALL+RISE CR, INCREASE FALL BY 30%
F0E2X1    ADDPCT    D2,0(A2,D0),0,0(A2,D0),D1

F0E2X2    DBF       D0,F0E2X0

          RTS


*-----------------------------------------------------------------------------
*
*         F0 PATTERNS ON THE UNSTRESSED SYLLABLES
*
*-----------------------------------------------------------------------------
*F0F	  CMP.W	    #MANUALF0,F0MODE(A5)	;MANUAL F0 MODE?	* Note 1
*	  BNE.S	    F0FX0			;  No, continue.	* Note 1
*	  RTS					;  Yes, skip this code	* Note 1

F0F       MOVE.L    F0END(A5),A0        ;DEFINE A REGS
          MOVE.L    F0START(A5),A1
          MOVE.L    F0FALL(A5),A2
          MOVE.L    F0RISE(A5),A3
          MOVE.L    F0ACCENT(A5),A4
          MOVE.L    F0PEAK(A5),A6       ;

          MOVE.W    F0LASTAS(A5),D0     ;
F0F2      BTST      #F0SSBIT,0(A4,D0)   ;STRESSED?
          BEQ.S     F0F2X1              ;NO
          MOVE.B    0(A6,D0),D1         ;GET PEAK
          MOVE.B    D1,D2               ;COPY
          SUB.B     0(A3,D0),D1         ;PEAK-RISE
          MOVE.B    D1,0(A1,D0)         ;MOVE TO START
          SUB.B     0(A2,D0),D2         ;PEAK-FALL
          MOVE.B    D2,0(A0,D0)         ;MOVE TO END
F0F2X1    DBF       D0,F0F2
*
*
*
F0F3
          MOVE.W    D5,D1               ;POSITION OF 1ST AS
          MOVE.W    F0NUMAS(A5),D3      ;GET NUMBER OF AS'S
F0F3X1    MOVE.W    D1,D0               ;UPDATE POSITION OF "PREVIOUS" AS
          MOVEQ     #0,D6               ;CLEAR FOR USE IN MULTIPLIES
          SUBQ.W    #1,D3               ;DECREMENT COUNT OF AS'S
          BLE       F0F4B1              ;ONLY 1 AS (OR NO MORE AS'S)


*         COMPUTE STARTS, PEAKS, AND ENDS OF US'S

          MOVE.L    F0ACCENT(A5),A4
F0F3X2    ADDQ.W    #1,D1
          BTST      #F0SSBIT,0(A4,D1)   ;STRESSED SYLLABLE?
          BEQ.S     F0F3X2              ;NO, KEEP LOOKING
          MOVE.W    D1,D2
          SUB.W     D0,D2
          SUBQ.W    #1,D2               ;COMPUTE NUMBER OF INTERVENING US'S
          BEQ.S     F0F3X1              ;ADJACENT AS'S, SKIP
          MOVE.B    0(A0,D0),D6         ;END OF 1ST AS
          SUB.B     0(A1,D1),D6         ;END OF 1ST AS - START OF 2ND AS
          BPL.S     F0F4                ;+IVE DIFFERENCE IS OK

*         ADJUST ENDPOINTS OF AS'S TO PREVENT RISING F0 IN US'S

          NEG.B     D6                  ;ABS(DIFFERENCE)
          LSR.B     #1,D6               ;ABS(DIFFERNECE)/2
          SUB.B     D6,0(A1,D1)         ;SUBTRACT FROM START
          ADD.B     D6,0(A3,D1)         ;  AND ADD TO RISE
          ADD.B     D6,0(A0,D0)         ;ADD TO END
          SUB.B     D6,0(A2,D0)         ;  AND SUB FROM FALL
          MOVEQ     #0,D6               ;SET DELTA TO ZERO
*
F0F4      MOVE.L    F0CRBRK(A5),A0      ;GET CR/BREAK ARRAY (TEMPORARILY)
          MOVE.B    0(A0,D0),D7         ;GET CR/BREAK ELEMENT
          MOVE.L    F0END(A5),A0        ;RESTORE END ARRAY POINTER
          AND.B     #F0BRMASK,D7        ;ISOLATE BR NUMBER
          CMP.B     #8,D7               ;NEGATIVE BREAK?
          BGE.S     F0F4OPNT            ;YES, AT END OF P-UNIT
          BTST      #F0INPUNB,0(A4,D0)  ;INSIDE P-UNIT, BUT NOT AT END?
          BEQ.S     F0F4OPNT            ;NO, JUMP TO OUTSIDE P-UNIT CODE

*         US'S INSIDE P-UNIT EXHIBIT A LINEAR DROP

F0F4IPNT  DIVU      D2,D6               ;COMPUTE DELTA
          BRA.S     F0F4X2              ;START LOOP
F0F4X1    ADDQ.W    #1,D0
          MOVE.B    -1(A0,D0),0(A1,D0)  ;END(I) -> ST(I+1)
          MOVE.B    0(A1,D0),0(A0,D0)   ;ST(I+1) - DELTA ->
          SUB.B     D6,0(A0,D0)         ;  END(I+1)
          MOVE.B    0(A0,D0),0(A6,D0)   ;END(I+1) + 6 ->
          ADDQ.B    #F05HZ,0(A6,D0)     ;  PEAK(I+1)	
F0F4X2    DBF       D2,F0F4X1
          BRA       F0F3X1

*         US'S OUTSIDE P-UNIT EXHIBIT AN EXPONENTIAL DROP TO TOP OF BOR

F0F4OPNT
          CMP.W     #1,D2
          BEQ.S     F0F4I1              ;ONE INTERVENING US, USE PREV CALCD DROP
          CMP.W     #2,D2
          BEQ.S     F0F4I2              ;TWO INTERVENING US'S, USE PREV CALCD DROP

*         THREE OR MORE INTERVENING US'S.  SET DROP TO END-(BOR-6)

F0F4I3    MOVE.B    0(A0,D0),D6         ;END OF FIRST AS
          SUB.B     #F0BOR-F05HZ,D6     ;END - (BOR - 5)
          SWAP      D2                  ;SAVE COUNT OF INTERVENING US'S IN UPPER D2
          MOVE.W    #3,D2               ;SET NUMBER OF US'S TO 3
          LEA       F0PNPCT3,A4         ;GET ADDR OF PERCENTAGES SEQUENCE
          BSR       F0INTERP            ;INTERPOLATE FIRST 3 INTERMEDIATE US VALUES
          MOVE.L    F0ACCENT(A5),A4     ;RESTORE ACCENT ARRAY
          SWAP      D2                  ;RESTORE COUNT OF INTERVENING US'S
          SUBQ.W    #3,D2               ;# US'S - 3
          BRA.S     F0F4I3X3            ;START LOOP
F0F4I3X2  MOVE.B    0(A0,D0),D6         ;GET END OF PREVIOUS US
          ADDQ.W    #1,D0               ;BUMP INDEX COUNTER
          MOVE.B    D6,0(A1,D0)         ;END -> START
          MOVE.B    D6,0(A0,D0)         ;START -> END
          MOVE.B    D6,0(A6,D0)         ;END -> PEAK
          MOVE.B    0(A4,D0),D6         ;GET ACCENT BYTE
          AND.B     #F0ACNTNO,D6        ;ISOLATE ACCENT NUMBER
	  IFEQ	    F0USE2HZ		;IF NOT USING 2 HZ STEPS,
          LSL.B     #1,D6               ;  MULTIPLY ACCENT NUMBER BY 2
	  ENDC
          ADD.B     D6,0(A6,D0)         ;ADD TO PEAK
F0F4I3X3  DBF       D2,F0F4I3X2
          BRA       F0F3X1


*         ONE INTERVENING US

F0F4I1
          LEA       F0PNPCT1,A4         ;GET ADDRESS OF PERCENTAGES SEQUENCE
          BSR       F0INTERP            ;COMPUTE INTERMEDIATE US VALUES
          BRA       F0F3X1              ;CONTINUE


*         TWO INTERVENING US'S

F0F4I2
          LEA       F0PNPCT2,A4         ;GET ADDRESS OF PERCENTAGES SEQUENCE
          BSR       F0INTERP            ;COMPUTE INTERMEDIATE US VALUES
          BRA       F0F3X1
*
*
*
F0F4B1    MOVE.L    F0ACCENT(A5),A4
          MOVE.B    #F0BOR,D6           ;SET D6 TO BOR
          MOVE.W    D5,D0               ;GET POS OF 1ST AS
          BRA.S     F0F4B1X1
F0F4B1X0  MOVE.B    0(A4,D0),D1         ;GET ACCENT
          AND.B     #F0ACNTNO,D1        ;ISOLATE ACCENT
	  IFEQ	    F0USE2HZ		;USING 2 HZ STEPS?
          LSL.B     #1,D1               ;NO, MULTIPLY ACCENT BY 2
	  ENDC
          MOVE.B    D6,0(A6,D0)         ;PEAK=BOR
          ADD.B     D1,0(A6,D0)         ;       + 2*ACCENT
          MOVE.B    D6,0(A1,D0)         ;START = BOR
          MOVE.B    D6,0(A0,D0)         ;END   = BOR
F0F4B1X1  DBF       D0,F0F4B1X0


F0F5      MOVEQ     #0,D7
          MOVE.L    F0TUNELV(A5),A3     ;ADDR OF TUNE ARRAY
          MOVE.W    D4,D0
          SUBQ.W    #1,D0               ;POS OF LAST SYL
          BTST      #F0SSBIT,0(A4,D0)   ;UNSTRESSED?
          BNE       F0F5XEND            ;NO, SKIP F0F5 RULES
          MOVE.B    0(A3,D0),D2         ;GET TUNE/LEVEL
          AND.B     #F0TUNEMK,D2        ;ISOLATE TUNE FOR LATER
*
F0F5A     MOVE.W    D0,D1
          BRA.S     F0F5AX1
F0F5AX0   BTST      #F0SSBIT,0(A4,D0)   ;STRESSED SYL?
          BNE.S     F0F5AX2             ;FOUND
F0F5AX1   DBF       D0,F0F5AX0          ;CONTINUE UNTIL STR SYL
          MOVEQ     #0,D0               ;NO PREVIOUS AS, USE 0
*
F0F5AX2   SUB.W     D0,D1               ;NUMBER OF UNSTR SYL
          BEQ.S     F0F5AX5             ;ONLY 1 SYL, JUST SET ENDPT
          MOVEQ     #0,D7

          IFNE       MAC
          MOVE.B    Ticks+3,D7          ;GET CLOCK COUNT
          AND.B     #$0F,D7             ;LOW NYBBLE
          ENDC

          ADD.B     0(A0,D0),D7         ;GET END OF STR SYL
          SUB.W     #F0BOR,D7           ;TUNE B OR NO TUNE, USE F0 BOR
          CMP.B     #F0TUNEA,D2         ;TUNE A?
          BNE.S     F0F5AX6             ;NO, USE F0 BOR
          SUB.W     #F0TERM-F0BOR,D7    ;YES, USE F0 TERM
F0F5AX6   DIVU      D1,D7               ;DIFFERENTIAL
          BRA.S     F0F5AX4             ;START LOOP
F0F5AX3   ADDQ.W    #1,D0               ;1ST US AFTER LAST AS
          MOVE.B    -1(A0,D0),0(A1,D0)  ;END(I-1) -> START(I)
          MOVE.B    0(A1,D0),0(A0,D0)   ;START(I) -> END(I)
          SUB.B     D7,0(A0,D0)         ;END(I)-DIFF -> END(I)
          MOVE.B    0(A0,D0),0(A6,D0)   ;END(I) -> PEAK(I)
          ADDQ.B    #F05HZ,0(A6,D0)     ;ADD 5HZ PERTURBATION
F0F5AX4   DBF       D1,F0F5AX3          ;LOOP
F0F5AX5   CMP.B     #F0TUNEB,D2         ;SEE IF TUNE B
          BEQ.S     F0F5B               ;IF SO, USE RULE F0F5B
          MOVE.B    #F0TERM,0(A0,D0)    ;SET LAST US END PT TO F0 TERM
          CMP.B     #F0TUNEA,D2         ;TUNE A?
          BEQ.S     F0F5XEND            ;YES, USE F0 TERM
          MOVE.B    #F0BOR,0(A0,D0)     ;NO TUNE, USE F0 BOR
          BRA.S     F0F5XEND
*
*
*
F0F5B     MOVEQ     #0,D7               ;CLEAR D7
          MOVEQ     #0,D1               ;CLEAR D1
          MOVE.W    D4,D0               ;GET COUNT OF SYLS
          BRA.S     F0F5BX1             ;START LOOP
*
F0F5BX0   MOVE.B    0(A6,D0),D1         ;GET SYL PEAK
          CMP.W     D1,D7               ;CHECK AGAINST OLD PEAK
          BGT.S     F0F5BX1             ;NOT GREATER
          MOVE.B    D1,D7               ;SAVE NEW MAX PEAK
F0F5BX1   DBF       D0,F0F5BX0          ;LOOP FOR ENTIRE CLAUSE
*
          MULU      #F0PCT20+F0PCT100,D7
          LSR.W     #7,D7                         ;COMPUTE 120% OF MAX PEAK
          MOVE.B    D7,-1(A0,D4)                  ;SAVE AS END POINT
          ADDQ.W    #F05HZ,D7                     ;END + 5HZ 
          MOVE.B    D7,-1(A6,D4)                  ;SAVE AS PEAK
          CLR.B     -1(A2,D4)                     ;SET FALL TO 0
          CMP.W     #1,D4                         ;ONLY 1 SYL?
          BLE.S     F0F5XEND                      ;YES, RETURN
          MOVE.B    -2(A0,D4),-1(A1,D4)           ;NO, GET PREV END PT TO START

F0F5XEND  RTS


*----------------------------------------------------------------------------
*
*         SEPARATION INTO RISE/JUMP AND DROP/FALL
*
*----------------------------------------------------------------------------
F0G	  NOP

F0G3	  TST.W	    F0NUMAS(A5)		;ANY ACCENTED SYLLABLES?         Note 2
	  BEQ	    F0GRTS		;   NO, SKIP THIS SECTION	 Note 2

	  MOVE.L    F0PHON(A5),A0       ;GET ADDRESSES OF PHON,
          MOVE.L    F0DUR(A5),A1        ;   DUR, AND
          MOVE.L    F0STRESS(A5),A2     ;   STRESS
          SUBQ.L    #1,A0               ;ADJUST TO BEFORE BOUND
          SUBQ.L    #1,A1
          SUBQ.L    #1,A2
          MOVEQ     #0,D2               ;CLEAR D2
          MOVE.W    D4,D0               ;COUNT OF SYLLABLES
          BRA       F0GXEND             ;START LOOP
*
F0G3X0:
	  SUBQ.L    #1,A0               ;MOVE PHON POINTER BACK
          SUBQ.L    #1,A1               ;MOVE DUR POINTER BACK
          TST.B     -(A2)               ;MOVE STRESS AND CHECK
          BPL.S     F0G3X0              ;  FOR START OF SYLLABLE
          BTST      #F0SSBIT,0(A4,D0)   ;START OF SYL FOUND, ACCENTED SYL?
          BEQ       F0GXEND             ;NO
*
          MOVEQ     #0,D1               ;CLEAR D1
          MOVE.B    (A0),D1             ;GET PHONEME CODE
          LSL.W     #2,D1               ;MAKE F.M. INDEX
          LEA       FEAT,A3             ;GET ADDR OF FEATURE MATRICES
          MOVE.L    0(A3,D1),D1         ;GET PHONEME FEATURES
          BTST      #CONSBIT,D1         ;INITIAL CONSONENT?
          BEQ       F0G4                ;NO, SKIP G3 RULES
          BTST      #VOICEDBIT,D1       ;VOICED?
          BEQ.S     F0G3A               ;NO

*-------- VOICED CONSONENT BEGINS AS
*             USE 20% JUMP AND 80% RISE

F0G3B     MOVE.W    #F0PCT20,D1
          MOVE.L    F0RISE(A5),A3       ;GET ADDR OF RISE ARRAY (FOR  F0G3AX0)
          BRA.S     F0G3AX0

*-------- UNVOICED CONSONENT BEGINS AS
*            INCREASE PEAK BY 20%,
*            INCREASE ACCENTS BY 20%
*            USE 80% JUMP AND 20% RISE

F0G3A     MOVE.L    F0PEAK(A5),A6                           ;GET PEAK ARRAY
          ADDPCT    #F0PCT20,0(A6,D0),F0BOR,0(A6,D0),D1     ;BUMP PEAK 20% OF LR
          MOVE.L    F0FALL(A5),A3                           ;GET FALL ARRAY
          ADD.B     D1,0(A3,D0)                             ;BUMP FALL ACCENT
          MOVE.L    F0RISE(A5),A3                           ;GET RISE ARRAY
          ADD.B     D1,0(A3,D0)                             ;BUMP RISE ACCENT

          MOVE.W    #F0PCT80,D1                             ;USE 80% JUMP
*
F0G3AX0   MOVE.L    F0START(A5),A6                          ;GET START ARRAY
          ADDPCT    D1,0(A3,D0),0,0(A6,D0),D2               ;INCREASE START
          SUB.B     D2,0(A3,D0)                             ;DECREASE RISE
*
F0G4      LEA       FEAT,A3                       ;GET ADDR OF FEATURE MATRICES
          BTST      #F0WORDFB,0(A4,D0)            ;WORD FINAL?
          BNE.S     F0G4AX                        ;YES, SKIP G4 RULES
          BTST      #F0SSBIT,1(A4,D0)             ;NEXT SYL US?
          BNE.S     F0G4AX                        ;NO, SKIP G4 RULES
*
*         FIND STRESSED VOWEL IN ACCENTED SYLLABLE
*
          MOVEQ     #0,D1
F0G4X0    MOVE.B    0(A2,D1),D2                   ;GET STRESS BYTE
          ADDQ.W    #1,D1                         ;BUMP INDEX
          AND.B     #F0ACNTNO,D2                  ;ISOLATE ACCENT NUMBER
          BEQ.S     F0G4X0                        ;NOT STRESSED VOWEL
*
*         SEARCH FORWARD UNTIL EITHER AN UNVOICED CONSONENT
*         IS FOUND - AND APPLY RULE 4A, OR THE UNSTRESSED
*         VOWEL IN THE FOLLOWING SYLLABLE IS FOUND - AND APPLY
*         RULE 4B
*
          MOVEQ     #0,D2                         ;UNVOICED CONS FLAG
F0G4X1    MOVEQ     #0,D7                         ;CLEAR
          MOVE.B    0(A0,D1),D7                   ;GET PHONEME
          CMP.B     #PHCQ,D7                      ;GLOTTAL?
          BEQ.S     F0G4B                         ;YES, USE RULE G4B
          LSL.W     #2,D7                         ;CONVERT TO F.M. INDEX
          MOVE.L    0(A3,D7),D7                   ;GET PHONEME FEATURES
          BTST      #VOWELBIT,D7                  ;VOWEL FOUND?
          BNE.S     F0G4B                         ;YES, USE RULE G4B
          BTST      #VOICEDBIT,D7                 ;VOICED CONSONENT?
          BEQ.S     F0G4A                         ;NO, USE RULE G4A
*
F0G4X2    ADDQ.W    #1,D1                         ;BUMP INDEX
          BRA.S     F0G4X1                        ;  AND CONTINUE

*-------- USE STANDARD FALL/DROP RATIO OF 50/50

F0G4AX    MOVE.W    D4,D7                         ;GET NUMBER OF SYLS
          SUBQ.W    #1,D7                         ;GET POSITION OF LAST SYL
          CMP.W     D0,D7                         ;IS AS THE LAST SYLLABLE?
          BEQ.S     F0GXEND                       ;YES, NO DROP ON TERM
*                                                  ;OTHERWISE,
          MOVE.W    #F0PCT50,D7                   ;   USE 50% OF FALL AS DROP
          BRA.S     F0G4BX0                       ;   AND JUMP TO COMMON CODE

*-------- FOUND A VOICED CONSONENT BETWEEN ACCENTED AND UNACCENTED VOWELS

F0G4A     MOVE.W    #F0PCT66,D7                   ;USE 66% OF FALL AS DROP
          BRA.S     F0G4BX0                       ;AND JUMP TO COMMON CODE

*-------- DID NOT FIND VOICED CONS BETWEEN ACCENTED AND UNACCENTED VOWELS

F0G4B     MOVE.W    #F0PCT20,D7                   ;USE 20% OF FALL AS DROP


F0G4BX0   MOVE.L    F0FALL(A5),A3                 ;GET ADDRESS OF FALL ARRAY
          MOVE.L    F0END(A5),A6                  ;GET ADDRESS OF END ARRAY
          ADDPCT    D7,0(A3,D0),0,0(A6,D0),D2     ;INCREASE END POINT
          SUB.B     D2,0(A3,D0)                   ;REDUCE FALL AMOUNT

F0GXEND   DBF       D0,F0G3X0                     ;LOOP

F0GRTS    RTS


*
*-----------------------------------------------------------------------------;
*                                                                             ;
*                           SUBROUTINE F0INTERP                               ;
*                                                                             ;
*                                                                             ;
*             COMPUTES INTERMEDIATE US START, PEAK, AND END VALUES            ;
*                                                                             ;
*             A0 - END ARRAY                    A1 - START ARRAY              ;
*             A4 - ADDRESS OF MULT TABLE        A6 - PEAK ARRAY               ;
*                                                                             ;
*             D0 - INDEX INTO START, PEAK, AND END ARRAYS                     ;
*             D2 - NUMBER OF INTERVENING US'S                                 ;
*             D6 - DROP FROM END OF FIRST "AS" TO TOP OF BOR (BOR-5HZ)        ;
*                                                                             ;
*-----------------------------------------------------------------------------;

F0INTERP  BRA.S     F0INTRP1            ;START LOOP
F0INTRP0  MOVE.B    0(A0,D0),1(A1,D0)   ;END(I) -> ST(I+1)
          MULU      (A4)+,D6
          LSR.W     #7,D6               ;COMPUTE PERCENTAGE OF DROP
          ADDQ.W    #1,D0               ;BUMP INDEX COUNTER
          MOVE.B    0(A1,D0),0(A0,D0)   ;ST(I+1) -> END(I+1)
          SUB.B     D6,0(A0,D0)         ;ST(I+1) - % OF DROP -> END(I+1)
          MOVE.B    0(A0,D0),0(A6,D0)   ;END(I+1) -> PEAK(I+1)
          ADDQ.B    #F05HZ,0(A6,D0)     ;PEAK = PEAK + 5HZ
F0INTRP1  DBF       D2,F0INTRP0         ;LOOP
          RTS



*--------- PERCENTAGES SEQUENCE FOR THREE US'S OUTSIDE P-UNIT ------------------

F0PNPCT3  DC.W      F0PCT45             ;DROP OF 1ST US = 45%
          DC.W      F0PCT78             ;DROP OF 2ND US = 35% (78% OF 45%)
          DC.W      F0PCT57             ;DROP OF 3RD US = 20% (57% OF 35%)


*--------- PERCENTAGES SEQUENCE FOR TWO US'S OUTSIDE P-UNIT --------------------

F0PNPCT2   DC.W     F0PCT60             ;DROP OF 1ST US = 60%
           DC.W     F0PCT66             ;DROP OF 2ND US = 40% (66.6% OF 60%)


*--------- PERCENTAGES SEQUENCE FOR ONE US OUTSIDE P-UNIT ----------------------

F0PNPCT1  DC.W      F0PCT100            ;DROP OF ONLY US = 100%


          END 
