*       TTL       FAST FLOATING POINT FLOAT TO INTEGER (FFPFPI)

**************************************
* (C) COPYRIGHT 1980 BY MOTOROLA INC.*
**************************************

**********************************************************
*            FAST FLOATING POINT TO INTEGER              *
*                                                        *
*      INPUT:  D0 = FAST FLOATING POINT NUMBER           *
*      OUTPUT: D0 = FIXED POINT INTEGER (2'S COMPLEMENT) *
*                                                        *
*  CONDITION CODES:                                      *
*             N - SET IF RESULT IS NEGATIVE              *
*             Z - SET IF RESULT IS ZERO                  *
*             V - SET IF OVERFLOW OCCURRED               *
*             C - UNDEFINED                              *
*             X - UNDEFINED                              *
*                                                        *
*  REGISTER D1 IS DESTROYED                              *
*                                                        *
*  INTEGERS OF OVER 24 BIT PRECISION WILL BE IMPRECISE   *
*                                                        *
*  NOTE: MAXIMUM SIZE INTEGER RETURNED IF OVERFLOW       *
*                                                        *
*   CODE SIZE: 78 BYTES        STACK WORK AREA: 0 BYTES  *
*                                                        *
*      TIMINGS:  (8 MHZ NO WAIT STATES ASSUMED)          *
*           COMPOSITE AVERAGE 15.00 MICROSECONDS         *
*            ARG = 0   4.75 MICROSECONDS                 *
*            ARG # 0   10.50 - 18.25 MICROSECONDS        *
*                                                        *
**********************************************************

******* mathffp.library/SPFix ******
*
*NAME
*	SPFix -- Convert fast floating point number to integer.
*
*SYNOPSIS
*	inum = SPFix(fnum)
*	D0	     D0
*
*	int SPFix(float fnum);
*
*FUNCTION
*	Accepts a floating point number and returns the truncated
*	integer portion of said number.
*
*INPUTS
*
*	fnum 	- floating point number.
*
*RESULT
*
*	inum 	- signed integer number.
*
*BUGS
*	None.
*
*SEE ALSO
*
*******/

         PAGE
*FFPFPI  IDNT      1,1  FFP FLOAT TO INTEGER

        XDEF      FFPFPI     ENTRY POINT
		XDEF	  MFSPFix

*       XREF      FFPCPYRT   COPYRIGHT NOTICE


FFPFPI:
MFSPFix:
	    MOVE.B    D0,D1      SAVE SIGN/EXPONENT              4
        BMI.S     FPIMI      BRANCH IF MINUS VALUE           8/10
        BEQ.S     FPIRTN     RETURN IF ZERO                  8/10
        CLR.B     D0         CLEAR FOR SHIFT                 4
        SUB.B     #65,D1     EXPONENT-1 TO BINARY            8
        BMI.S     FPIRT0     RETURN ZERO FOR FRACTION        8/10
        SUB.B     #31,D1     ? OVERFLOW                      8
        BPL.S     FPIOVP     BRANCH IF TOO LARGE             8/10
        NEG.B     D1         ADJUST FOR SHIFT                4
        LSR.L     D1,D0      FINALIZE INTEGER                8-70
FPIRTN  RTS                  RETURN                          16

* POSITIVE OVERFLOW
FPIOVP  MOVEQ     #-1,D0     LOAD ALL ONES
        LSR.L     #1,D0      PUT ZERO IN AS SIGN
*       ORI       #$02,CCR   SET OVERFLOW BIT ON
        DC.L      $003C0002  ****SICK ASSEMBLER****
        RTS                  RETURN

* FRACTION ONLY RETURNS ZERO
FPIRT0  MOVEQ     #0,D0      LOAD ZERO
        RTS                  RETURN

* INPUT IS A MINUS INTEGER
FPIMI   CLR.B     D0         CLEAR FOR CLEAN SHIFT               4
        SUB.B     #$80+65,D1 EXPONENT-1 TO BINARY AND STRIP SIGN 8
        BMI.S     FPIRT0     RETURN ZERO FOR FRACTION            8/1
        SUB.B     #31,D1     ? OVERFLOW                          8
        BPL.S     FPICHM     BRANCH POSSIBLE MINUS OVERFLOW      8/10
        NEG.B     D1         ADJUST FOR SHIFT COUNT              4
        LSR.L     D1,D0      SHIFT TO PROPER MAGNITUDE           8-0
        NEG.L     D0         TO MINUS NOW                        6
        RTS                  RETURN                              16

* CHECK FOR MAXIMUM MINUS NUMBER OR MINUS OVERFLOW
FPICHM  BNE.S     FPIOVM     BRANCH MINUS OVERFLOW
        NEG.L     D0         ATTEMPT CONVERT TO NEGATIVE
        TST.L     D0         CLEAR OVERFLOW BIT
        BMI.S     FPIRTN     RETURN IF MAXIMUM NEGATIVE INTEGER
FPIOVM  MOVEQ     #0,D0      CLEAR D0
        BSET.L    #31,D0     SET HIGH BIT ON FOR MAXIMUM NEGATIVE
*       ORI       #$02,CCR   SET OVERFLOW BIT ON
        DC.L      $003C0002  ****SICK ASSEMBLER****
        RTS                  RETURN

        END
