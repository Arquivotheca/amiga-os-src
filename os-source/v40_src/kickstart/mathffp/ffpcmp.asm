*      TTL     FAST FLOATING POINT CMP/TST (FFPCMP/FFPTST)
***************************************
* (C) COPYRIGHT 1981 BY MOTOROLA INC. *
***************************************

************************************************************
*                      FFPCMP                              *
*              FAST FLOATING POINT COMPARE                 *
*                                                          *
*  INPUT:  D1 - FAST FLOATING POINT ARGUMENT (SOURCE)      *
*          D0 - FAST FLOATING POINT ARGUMENT (DESTINATION) *
*                                                          *
*  OUTPUT: CONDITION CODE REFLECTING THE FOLLOWING         *
*          BRANCHES FOR THE RESULT OF COMPARING            *
*          THE DESTINATION MINUS THE SOURCE:               *
*                                                          *
*                  GT - DESTINATION GREATER                *
*                  GE - DESTINATION GREATER OR EQUAL       *
*                  EQ - DESTINATION EQUAL                  *
*                  NE - DESTINATION NOT EQUAL              *
*                  LT - DESTINATION LESS THAN              *
*                  LE - DESTINATION LESS THAN OR EQUAL     *
*                                                          *
*      CONDITION CODES:                                    *
*              N - CLEARED                                 *
*              Z - SET IF RESULT IS ZERO                   *
*              V - CLEARED                                 *
*              C - UNDEFINED                               *
*              X - UNDEFINED                               *
*                                                          *
*               ALL REGISTERS TRANSPARENT                  *
*                                                          *
************************************************************
******* mathffp.library/SPCmp ******
*
*NAME
*	SPCmp -- Compares two floating point numbers.
*
*SYNOPSIS
*	result = SPCmp(fnum1, fnum2)
*	D0               D0     D1
*
*	int SPCmp(float fnum1, float fnum2);
*
*FUNCTION
*
*	Accepts two floating point numbers and returns the condition
*	codes set to indicate the result of said comparison.  Additionally,
*	the integer functional result is returned to indicate the result
*	of said comparison.
*
*INPUTS
*	fnum1 	- floating point number.
*	fnum2 	- floating point number.
*
*RESULT
*	Condition codes set to reflect the following branches:
*
*		GT - fnum2 >  fnum1
*		GE - fnum2 >= fnum1
*		EQ - fnum2 =  fnum1
*		NE - fnum2 != fnum1
*		LT - fnum2 <  fnum1
*		LE - fnum2 <= fnum1
*
*	Integer functional result as:
*
*		+1 => fnum1 > fnum2
*		-1 => fnum1 < fnum2
*		 0 => fnum1 = fnum2
*
*BUGS
*	None.
*
*SEE ALSO
*
******/
         PAGE
*FFPCMP IDNT    1,3  FFP CMP/TST


		 XREF	   _MathBase,_AbsExecBase,_LVOGetCC

         XDEF      FFPCMP    FAST FLOATING POINT COMPARE
		 XDEF	   MFSPCmp

* comment out next line for 68000
M68010	equ 1

***********************
* COMPARE ENTRY POINT *
***********************
FFPCMP:
MFSPCmp:
	     TST.B     D1        ? FIRST NEGATIVE
         BPL.S     FFPCP     NO FIRST IS POSITIVE
         TST.B     D0        ? SECOND NEGATIVE
         BPL.S     FFPCP     NO, ONE IS POSITIVE
* IF BOTH NEGATIVE THEN COMPARE MUST BE DONE BACKWADS
         CMP.B     D0,D1     COMPARE SIGN AND EXPONENT ONLY FIRST
         BNE.S     FFPCRTN   RETURN IF THAT IS SUFFICOENT
         CMP.L     D0,D1     COMPARE REVERSE ORDER IF BOTH NEGATIVE
		 BRA.S	   set_d0	 SET D0 TO -1, 0 OR +1 and EXIT

FFPCP    CMP.B     D1,D0     COMPARE SIGN AND EXPONENT ONLY FIRST
         BNE.S     FFPCRTN   RETURN IF THAT IS SUFFICIENT
         CMP.L     D1,D0     NO, COMPARE FULL LONGWORDS THEN

*FFPCRTN BRA.S	   set_d0	 SET D0 TO -1, 0 OR +1 and EXIT
FFPCRTN:
set_d0:
*******************************************************************************
		ifnd	M68010
		move.w	sr,d0					* NOT 68010 COMPATIBLE, USE CODE BELOW!
		and.w	#$FF,d0
		endc

		ifd		M68010
		movem.l	a6,-(sp)				* Save A6 for GetCC call
		movea.l	_AbsExecBase,a6
		jsr		_LVOGetCC(a6)			* Go get CCR in D0
		move.l	(sp)+,a6				* Restore A6 upon completion
		endc
*******************************************************************************

		move.w	d0,d1					* Put CCR into D1 so D0 can be returned
		moveq.l	#0,d0					* Assume operands equal until proven otherwise
		move.w	d1,CCR					* Restore CCR to determine D0 value
		blt.s	set_m1
		bgt.s	set_p1
		bra.s	set_ex
set_p1:
		subq.l	#1,d0					* Source > destination, return d0 = -1
		bra.s	set_ex
set_m1:
		addq.l	#1,d0					* Source < destination, return d0 = +1
set_ex:
		move.w	d1,CCR					* Restore CCR before exiting
		rts								* Go back to local caller




         PAGE
************************************************************
*                     FFPTST                               *
*           FAST FLOATING POINT TEST                       *
*                                                          *
*  INPUT:  D1 - FAST FLOATING POINT ARGUMENT               *
*                                                          *
*  OUTPUT: CONDITION CODES SET FOR THE FOLLOWING BRANCHES: *
*                                                          *
*                  EQ - ARGUMENT EQUALS ZERO               *
*                  NE - ARGUMENT NOT EQUAL ZERO            *
*                  PL - ARGUMENT IS POSITIVE OR ZERO       *
*                  MI - ARGUMENT IS NEGATIVE               *
*                                                          *
*      CONDITION CODES:                                    *
*              N - SET IF RESULT IS NEGATIVE               *
*              Z - SET IF RESULT IS ZERO                   *
*              V - CLEARED                                 *
*              C - UNDEFINED                               *
*              X - UNDEFINED                               *
*                                                          *
*               ALL REGISTERS TRANSPARENT                  *
*                                                          *
************************************************************
         PAGE
         XDEF      FFPTST    FAST FLOATING POINT TEST
		 XDEF	   MFSPTst
******* mathffp.library/SPTst ******
*
*NAME
*	SPTst - Compares a float against zero (0.0).
*
*SYNOPSIS
*	result = SPTst(fnum)
*	D0	       D1
*
*	int SPTst(float fnum);
*
*FUNCTION
*	Accepts a floating point number and returns the condition
*	codes set to indicate the result of a comparison against
*	the value of zero (0.0).  Additionally, the integer functional
*	result is returned.
*
*INPUTS
*	fnum 	- floating point number.
*
*RESULT
*	Condition codes set to reflect the following branches:
*
*		EQ - fnum =  0.0
*		NE - fnum != 0.0
*		PL - fnum >= 0.0
*		MI - fnum <  0.0
*
*	Integer functional result as:
*
*		+1 => fnum > 0.0
*		-1 => fnum < 0.0
*		 0 => fnum = 0.0
*
*BUGS
*	None.
*
*SEE ALSO
*
******/

********************
* TEST ENTRY POINT *
********************
FFPTST:
MFSPTst:
	     TST.B     D1        TEST CONDITION CODE, SET D1 AND EXIT
********************************************************************************
		ifnd	M68010
		move.w	sr,d0					* NOT 68010 COMPATIBLE, USE CODE BELOW!!
		and.w	#$FF,d0
		endc

		ifd		M68010
		movem.l	a6,-(sp)				* Save A6 for GetCC call
		movea.l	_AbsExecBase,a6
		jsr		_LVOGetCC(a6)			* Go get CCR in D0
		move.l	(sp)+,a6				* Restore A6 upon completion
		endc
********************************************************************************

		move.w	d0,d1					* Put CCR into D1 so D0 can be returned
		moveq.l	#0,d0					* Assume operands equal until proven otherwise
		move.w	d1,CCR					* Restore CCR to determine D0 value
		blt.s	set_m1_t
		bgt.s	set_p1_t
		bra.s	set_ex_t
set_m1_t:
		subq.l	#1,d0					* Source < destination, return d0 = -1
		bra.s	set_ex_t
set_p1_t:
		addq.l	#1,d0					* Source > destination, return d0 = +1
set_ex_t:
		move.w	d1,CCR					* Restore CCR before exiting
		rts								* Go back to local caller




		end

