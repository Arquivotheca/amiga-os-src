		TTL	'F0ENTHUS.ASM'



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
*									*
*          REGISTER DEFINITIONS						*
*          A0 - START ARRAY              A1 - USERIORB & PEAK ARRAY	*
*          A2 - END ARRAY                A3 -				*
*          A4 - 	                 A5 - GLOBALS AREA		*
*          A6 - 							*
*									*
*          D0 - START VALUE	         D1 - PEAK VALUE 		*
*          D2 - END VALUE		 D7 - ENTHUSIASM FACTOR		*
*									*
*									*
*									*
*************************************************************************
          SECTION      speech


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
          INCLUDE   'f0equs.i'
	  INCLUDE   'narrator.i'


          XDEF      F0ENTHUS


ENTHSHFT  EQU	    5				;F0ENTHUSIASM IS IN 32NDS
ENTHUNTY  EQU	    (1<<ENTHSHFT)		;UNITY F0 ENTHUSIASM (32)



F0ENTHUS  MOVE.L  USERIORB(A5),A1		;GET USERIORB
	  BTST	  #NDB_NEWIORB,NDI_FLAGS(A1)	;IS THIS A NEW IORB?
	  BEQ.S	  JustReturn			;  NO, JUST RETURN
*	  CMP.W	  #NATURALF0,F0MODE(A5)		;IS THIS NATURAL F0 MODE?
*	  BNE.S	  JustReturn			;  NO, JUST RETURN

	  MOVEQ	  #0,D7				;CLEAR
	  MOVE.B  NDI_F0ENTHUSIASM(A1),D7	;GET ENTHUSIASM FACTOR
	  CMP.W	  #ENTHUNTY,D7			;IS IT UNITY?
	  BEQ.S	  JustReturn 			;  YES, JUST RETURN


*	  ;------ DO THE THING.

Setup     MOVE.L  START(A5),A0
	  MOVE.L  PEAK(A5),A1
          MOVE.L  FEND(A5),A2
	   
ScaleLoop MOVEQ	  #0,D0  			;CLEAR D REGS FOR MATH
	  MOVEQ	  #0,D1
	  MOVEQ	  #0,D2
	  MOVE.B  (A0)+,D0			;GET F0 START
	  MOVE.B  (A1)+,D1			;       PEAK
	  MOVE.B  (A2)+,D2			;       END
	  BEQ.S	  JustReturn			;0 ==> END OF F0 ARRAYS


ScalePeak CMP.W	  #F0BOR,D2			;CLAMP END PT AT F0BOR, ELSE AT
	  BGE.S	  3$				;  PERIOD, ENTHUS FACTOR CAUSES
	  MOVE.W  #F0BOR,D2			;  EXCESSIVE PEAK MOVEMENT
3$	  ADD.W	  D2,D0				;FIND CENTER POINT OF LINE
	  ASR.W	  #1,D0				;  CONNECTING START TO END PTS
    	  SUB.W	  D0,D1				;HEIGHT OF PEAK ABOVE CENTER PT
	  MULS	  D7,D1				;MULTIPLY BY FRACTIONAL
	  ASR.L	  #ENTHSHFT,D1			;  ENTHUSIASM VALUE
	  ADD.W	  D0,D1
	  CMP.W	  #F0BOR,D1			;INSURE F0BOR <= PEAK <= 400 HZ
	  BGE.S	  1$
	  MOVE.B  #F0BOR,D1	
	  BRA.S	  2$
1$	  CMP.W	  #F0MAXHZ,D1	
	  BLE.S	  2$
	  MOVE.B  #F0MAXHZ,D1	
2$        MOVE.B  D1,-1(A1)			;STORE NEW PEAK VALUE
	  BRA.S	  ScaleLoop			;GO BACK FOR MORE


JustReturn:
	  RTS

          END 
