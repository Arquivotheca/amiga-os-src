*     TTL     FAST FLOATING POINT INTEGER TO FLOAT (FFPIFP)

************************************
* (C) COPYRIGHT 1980 MOTOROLA INC. *
************************************

**********************************************************
*               INTEGER TO FLOATING POINT                *
*                                                        *
*      INPUT: D0 = FIXED POINT INTEGER (2'S COMPLEMENT)  *
*      OUTPUT: D0 = FAST FLOATING POINT EQUIVALENT       *
*                                                        *
*      CONDITION CODES:                                  *
*                N - SET IF RESULT IS NEGATIVE           *
*                Z - SET IF RESULT IS ZERO               *
*                V - CLEARED                             *
*                C - UNDEFINED                           *
*                X - UNDEFINED                           *
*                                                        *
*      D1 IS DESTROYED                                   *
*                                                        *
*      INTEGERS OF GREATER THAN 24 BITS WILL BE ROUNDED  *
*      AND IMPRECISE.                                    *
*                                                        *
*      CODE SIZE: 56 BYTES      STACK WORK AREA: 0 BYTES *
*                                                        *
*      TIMINGS: (8MHZ NO WAIT STATES ASSUMED)            *
*         COMPOSITE AVERATE 31.75 MICROSECONDS           *
*            ARG = 0   4.25          MICROSECONDS        *
*            ARG > 0   13.75 - 47.50 MICROSECONDS        *
*            ARG < 0   15.50 - 50.25 MICROSECONDS        *
*                                                        *
**********************************************************
******* mathffp.library/SPFlt ******
*
*NAME
*	SPFlt -- Convert integer number to fast floating point.
*
*
*SYNOPSIS
*	fnum = SPFlt(inum)
*	D0	     D0
*
*	float SPFlt(inet inum);
*
*FUNCTION
*	Accepts an integer and returns the converted
*	floating point result of said number.
*
*INPUTS
*	inum 	- signed integer number
*
*RESULT
*	fnum 	- floating point number
*
*BUGS
*	None.
*
*SEE ALSO
*
******/
*FFPIFP IDNT    1,1  FFP INTEGER TO FLOAT
         PAGE
*     XREF    FFPCPYRT    COPYRIGHT STUB

      XDEF    FFPIFP      EXTERNAL NAME
	  XDEF	  MFSPFlt

FFPIFP:
MFSPFlt:
	     MOVEQ     #64+31,D1 SETUP HIGH END EXPONENT
         TST.L     D0        ? INTEGER A ZERO
         BEQ.S     ITORTN    RETURN SAME RESULT IF SO
         BPL.S     ITOPLS    BRANCH IF POSITIVE INTEGER
         MOVEQ     #-32,D1   SETUP NEGATIVE HIGH EXPONENT -#80+64+32
         NEG.L     D0        FIND POSITIVE VALUE
         BVS.S     ITORTI    BRANCH MAXIMUM NEGATIVE NUMBER
         SUBQ.B    #1,D1     ADJUST FOR EXTRA ZERO BIT
ITOPLS   CMP.L     #$00007FFF,D0 ? POSSIBLE 17 BITS ZERO
         BHI.S     ITOLP     BRANCH IF NOT
         SWAP.W    D0        QUICK SHIFT BY SWAP
         SUB.B     #16,D1    COUNT 16 BITS SHIFTED
ITOLP    ADD.L     D0,D0     SHIFT MANTISSA UP
         DBMI      D1,ITOLP  LOOP UNTIL NORMALIZED
         TST.B     D0        ? TEST FOR ROUND UP
         BPL.S     ITORTI    BRANCH NO ROUNDING NEEDED
         ADD.L     #$100,D0  ROUND UP
         BCC.S     ITORTI    BRANCH NO OVERFLOW
         ROXR.L    #1,D0     ADJUST DOWN ONE BIT
         ADDQ.B    #1,D1     REFLECT RIGHT SHIFT IN EXPONENT BIAS
ITORTI   MOVE.B    D1,D0     INSERT SIGN/EXPONENT
ITORTN   RTS                 RETURN

         END
