	TTL	'PHONET.ASM'

*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

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
	 INCLUDE   'narrator.i'
         INCLUDE   'private.i'

         EXTERN_SYS AllocMem

         XDEF     COMPCOEF
         XREF     FEAT

COMPCOEF:
         BSR      COMPRESS     	;REMOVE IGNORES
         BSR      COEFOVFL     	;LOOK FOR POSSIBLE COEF OVFL
			       	;        CAUTION!!!!
			      	;COEFOVFL SETS CONDITION CODES WHICH
				;NARRATOR.ASM LOOKS AT.  DO NOT BLINDLY
				;ADD CODE AFTER THE BSR TO COEFOVFL THAT
				;MIGHT CHANGE CC SETTINGS. (YUCK!)
         RTS

*********************************************************
*
* SUBROUTINE 'COMPRESS'
*
* DELETES 'IGNORE' TYPE PHONS FROM PHON STRESS & DUR
* ARRAYS AND COMPRESSES THE SPACES OUT.
*
*********************************************************
COMPRESS MOVE.L     PHON(A5),A0     ;PHON READ
         MOVE.L     DUR(A5),A1      ;DUR READ
         MOVE.L     STRESS(A5),A2   ;STRESS READ
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

*************************************************
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
*
*************************************************
COEFOVFL:
        MOVE.L  DUR(A5),A1
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



*	  ;------- Get COEF and, possibly, MOUTHS buffer(s).  Return 
	  ;	   error if no memory available, else return 0 (in D3).

	  MOVE.L    D0,D2			;Save COEFSIZE for later
	  CLR.L     COEFSIZE(A5)		;Initialize COEFSIZE
	  CLR.L     COEFPTR(A5)			;Initialize COEFPTR
	  MOVE.L    #MEMF_CLEAR,D1		;Get any old memory
	  MOVE.L    _AbsExecBase,A6 
          CALLSYS   AllocMem 
          TST.L     D0                		;Did we get the memory?
          BNE.S     COEFOV3           		;   Yes, branch
	  MOVEQ	    #ND_NoMem,D3		;   No, mark as error
          RTS					;	 and return

COEFOV3   MOVE.L    D0,COEFPTR(A5)    		;Save coef pointer
	  MOVE.L    D2,COEFSIZE(A5)		;Save coef size


	  MOVE.L    MOTHSIZE(A5),D2 		;Get mouths array size
	  CLR.L	    MOTHSIZE(A5)		;Initialize mouth buffer pointer
	  CLR.L	    MOTHPTR(A5)			;Initialize mouth buffer length
	  TST.B     Mouth(A5)         		;Are mouth shapes requested?
	  BEQ.S     COEFOV4           		;No, branch
	  MOVE.L    D2,D0			;Yes, get length of buffer
	  MOVEQ     #0,D1			;Any old memory will do
	  CALLSYS   AllocMem			;Try to get it
	  TST.L     D0				;Did we get it?
	  BNE.S	    COEFOV5			;   Yes, branch
	  MOVEQ	    #ND_NoMem,D3		;   No, mark as error
	  RTS					;       and return

COEFOV5   MOVE.L    D0,MOTHPTR(A5)		;Save mouth shapes buffer
	  MOVE.L    D2,MOTHSIZE(A5)		;Save mouth shapes length

COEFOV4	  MOVEQ	    #0,D3			;Set good return code

	  RTS					;Return

          END
