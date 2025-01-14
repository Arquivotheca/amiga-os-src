	TTL	'PHONETINTERP.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


 	INCLUDE 'exec/types.i'
	INCLUDE	"phonet.i"


;void PhonetInterp(DebugString, Track)
;	char		*DebugString;
;struct	PhonetParms	*Track;


	PUBLIC	_PhonetInterp

_PhonetInterp:
	LINK	A5,#0				;C Style
	MOVEM.L	D2-D3/A2,-(SP)			;Save some registers


;{

	MOVE.L	12(A5),A0			;Get pointer to Track structure


;    switch(trantype)
	MOVE.L	#0,D0
	MOVE.B	PP_TRANTYPE(A0),D0
	BRA	.87



;      {
;	case DISCON:
.89
;	  break;
	BRA	PhonetReturn	
;
;
;	cases SMODIS: and SETSMO:
.90

;	  if ((tcb = MIN(tcb, (BYTE)mintime)) > 0)
;	    {
	MOVEQ	#0,D0				;Clear
	MOVE.B	PP_TCB(A0),D0			;Get Tcb (in D0)
	CMP.B	PP_MINTIME+1(A0),D0		;Less than Mintime?
	BLE.S	1$				;  Yes, use Tcb
	MOVE.W	PP_MINTIME(A0),D0		;  No, set Tcb to Mintime
1$	TST.B	D0				;Tcb > 0?
	BLE.S	SMODISSkip			;  No, skip this code



;	      start  = *(cumdur - tcb) << 5;

	MOVE.L	PP_CUMDUR(A0),A1		;Get Cumdur (in A1)
	MOVE.L	A1,A2				;Copy
	SUB.L	D0,A2				;Cumdur - Tcb
	SUB.L	D0,A2				;   (Cumdur is words)
	MOVE.W	(A2),D1				;Get *(Cumdur - Tcb)
	EXT.L	D1				;Extend sign bit
	ASL.L	#5,D1				;Compute starting value



;	      incr   = ((bvb << 5) - start)/tcb;

	MOVE.W	PP_BVB(A0),D2			;Get Bvb
	EXT.L	D2				;Extend sign bit
	ASL.L	#5,D2				;Shift 5 places
	SUB.L	D1,D2				;(Bvb << 5) - start
	DIVS	D0,D2				;Compute increment
	EXT.L	D2				;Sign extend


;	      cumdur = cumdur - (LONG)(tcb-1);

	SUB.L	D0,A1				;Cumdur - Tcb
	SUB.L	D0,A1
	ADDQ.L	#2,A1				;Cumdur - (Tcb - 1)


;	      while (--tcb)

	SUBQ.W	#1,D0
	BRA.S	SMODISDBF

SMODISLoop:
;	      	  start     += incr;
	ADD.L	D2,D1				;Increment start by incr


;	      	  *cumdur++  = start >> 5;

	MOVE.L	D1,D3				;Copy to scratch reg
	ASR.L	#5,D3				;Shift back down
	MOVE.W	D3,(A1)+			;Store in Cumdur (and incr ptr)

;	     	}
SMODISDBF:
	DBF	D0,SMODISLoop


;	  If trantype is SETSMO, fall through to DISSMO code to do right
;	  half of transition.  Otherwise, just break.

SMODISSkip:
	CMP.B	#SETSMO,PP_TRANTYPE(A0)		;Is transition type SETSMO?
	BNE	PhonetReturn			;  No, skip DISSMO code
						;  Yes, do DISSMO code


;	case DISSMO:
.96

;	  if ((tcf = MIN((BYTE)(segdur-1), tcf)) > 0)
;	    {
	MOVE.L	#0,D0				;Clear
	MOVE.B	PP_SEGDUR(A0),D0		;Get Segdur
	BEQ.S	DISSMOSkip			;If = 0, skip this code
	SUBQ.L	#1,D0
	CMP.B	PP_TCF(A0),D0			;Segdur-1 < Tcf?
	BLT.S	1$				;   Yes, set Tcf = Segdur-1
	MOVE.B	PP_TCF(A0),D0			;   No, use Tcf
1$	TST.B	D0				;Is Tcf > 0?
	BLE.S	DISSMOSkip			;  No, skip this code



;	      incr   = ((*(cumdur+tcf) - bvf) << 5)/tcf;

	MOVE.L	PP_CUMDUR(A0),A1		;Get Cumdur (in A1)
	MOVE.L	A1,A2				;Copy
	ADD.L	D0,A2				;Cumdur + tcf
	ADD.L	D0,A2
	MOVE.W	(A2),D1
	SUB.W	PP_BVF(A0),D1			;*(Cumdur+Tcf) - Bvf
	EXT.L	D1				;Sign extend
	ASL.L	#5,D1				;Shift 5 positions
	DIVS	D0,D1				;Divide by Tcf
	EXT.L	D1				;Sign extend


;	      start  = bvf << 5;

	MOVE.W	PP_BVF(A0),D2			;Get Bvf
	EXT.L	D2				;Sign extend
	ASL.L	#5,D2				;Compute start value



;	      while (tcf--)

	SUBQ.W	#1,D0				;Adjust for DBF

DISSMOLoop:
;	      {
;	      	*cumdur++ = start >> 5;

	MOVE.L	D2,D3				;Copy start value
	ASR.L	#5,D3				;Shift back
	MOVE.W	D3,(A1)+			;Store in Cumdur (and incr ptr)


;	    	start    += incr;

	ADD.L	D1,D2				;Incr start value by incr


DISSMODBF:
	DBF	D0,DISSMOLoop
;	      }
	BRA.S	DISSMOBREAK


DISSMOSkip:
;	  else *cumdur++ = bvf;

	MOVE.L	PP_CUMDUR(A0),A1		;Get Cumdur
	MOVE.W	PP_BVF(A0),(A1)+


DISSMOBREAK:
;	  break

	BRA.S	PhonetReturn



;	  }			/* END OF SWITCH(TRANTYPE) */
.115
	dc.w	.89-.116-2			;DISCON
	dc.w	.96-.116-2			;DISSMO
	dc.w	.90-.116-2			;SMODIS
	dc.w	.90-.116-2			;SETSMO (.90 was .103)
.87
	cmp.l	#4,d0
	bcc	PhonetReturn
	asl.l	#1,d0
	move.w	.115(pc,d0.w),d0
.116
	jmp	(pc,d0.w)


PhonetReturn:
;}


.117
	MOVEM.L	(SP)+,D2-D3/A2
	UNLK	A5
	RTS

	END
