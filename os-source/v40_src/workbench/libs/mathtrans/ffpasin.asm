*        TTL       FAST FLOATING POINT ARCSINE (FFPASIN)
*****************************************
* (C) COPYRIGHT 1985 BY COMMODORE-AMIGA *
*  rewritten by Dale Luck
*****************************************

*************************************************
*             FFPASIN / FFPACOS                 *
*    FAST FLOATING POINT ARCSINE / ARCCOSINE   	*
*                                               *
*  INPUT:   D7 - INPUT ARGUMENT                 *
*                                               *
*  OUTPUT:  D7 - ARCSINE/ARCCOSINE RADIAN RESULT*
*                                               *
*     ALL OTHER REGISTERS TOTALLY TRANSPARENT   *
*                                               *
*************************************************
*
*	new code from cody & waite
*
*#define DOASINE	0
*#define DOACOSINE	1
*asin(x)	return(qsincos(x,DOASINE));
*acos(x) return(qsincos(x,DOACOSINE));
*
*float doit(y,g)
*float y,g;
*{
*	/*  return(y + y*R(g)) */
*	/* R(g) = g*P(g)/Q(g) */
*	/* P(g) = p2*g+p1 */
*	/* Q(g) = (g+q1)*g+q0 */
*
*	float R,Q,P;
*	Q = (g+q1)*g + q0;
*	P = p2*g + p1;
*	R = g * P / Q;
*	return ( y + y*R);
*}
*
*qsincos(x,flag)
*float x;
*int flag;
*{
*	float y,result;
*	int i;
*	y = FABS(x);
*	if (y > 0.5)
*	{	/* reduce argument */
*		if (y > 1.0)	return (error);
*		i = 1 - flag;
*		g =  ((0.5-y)+0.5)/2.0;/*(1-y)/2;*/
*		y = -2*sqrt(g);
*		result = doit(y,g);
*	}
*	else
*	{
*		i = flag;
*		if (y>eps)
*		{
*			g = y*y;
*			result = doit(y,g);
*		}
*		else
*		{
*			result = y;
*		}
*	}
*	if (flag == DOSINE)
*	{
*		result = (a[i] + result) + a[i];
*		if (x < 0)	result = -result;
*	}
*	else	/* DOING COSINE */
*	{
*		if (x < 0)	result = (a[i]-result)+a[i];
*		else		result = (b[i]+result)+b[i];
*	}
*	return(result);
*}

******* mathtrans.library/SPAsin ************************
*
*NAME	
* 
*	SPAsin - obtain the arcsine of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPAsin(fnum1);
*                       d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number representing the sine
*	of an angle and returns the value of said angle in
*	radians
*
*INPUTS
* 
*	fnum1 - Motorola fast floating point number
*
*RESULT
* 
*	fnum2 - Motorola fast floating point number
* 
*BUGS
* 
*	None
*
*SEE ALSO
* 
*	SPCos
******

	section mathtrans

	xref	_LVOSPAdd,_LVOSPMul,_LVOSPSub,_LVOSPDiv
	xref	_LVOSPAbs,_LVOSPCmp,_LVOSPTst,_LVOSPNeg
	xref	_MathBase
	xref	_GetCC
	xref	MFSPSqrt

	xdef	FFPASIN,FFPACOS


fp0	equ		$00000000		* FFP 0.0
fphalf	equ		$80000040		* FFP 0.5
fp1	equ		$80000041		* FFP 1.0
fp2	equ		$80000042		* FFP 2.0
fp3	equ		$C0000042		* FFP 3.0
fp28	equ		$E0000045		* FFP 28.0
fp34	equ		$88000046		* FFP 34.0
fppid2	equ		$C90FDB41		* FFP 3.1415926535897 / 2.0

minusfp2	equ	$800000C2		* -2.0

PIOVER2 equ		fppid2
PIOVER4 equ		$C90FDB40		* FFP PI/4

eps	equ		0
asub0	equ		fp0	* not really used
asub1	equ		PIOVER4
bsub0	equ	PIOVER2
bsub1	equ	asub1

q1	equ	-1316156477		* -5.54846723
q0	equ	-1286541245		* 5.60363004
p2	equ	-2128583232		* -0.504400557
p1	equ	-283743424		* 0.933935835

overflow:
	move.l	d0,d7
	tst.b	d7
	jsr	_GetCC
	bset	#1,d0
	bclr	#3,d0
	bra	qsincosreturn

doit:
*	inputs d5 = y,d3 = g
*	outputs d0
*{
	move.l	d6,-(sp)
*	Q = (g+q1)*g + q0;		* Q = d6 */
		move.l	d3,d0
		move.l	#q1,d1
		jsr	_LVOSPAdd(a5)
		move.l	d3,d1
		jsr	_LVOSPMul(a5)
		move.l	#q0,d1
		jsr	_LVOSPAdd(a5)
		move.l	d0,d6

*	P = p2*g + p1;
		move.l	d3,d0
		move.l	#p2,d1
		jsr	_LVOSPMul(a5)
		move.l	#p1,d1
		jsr	_LVOSPAdd(a5)
*		leave P in d0.l

*	R = g * P / Q;
		move.l	d3,d1
		jsr	_LVOSPMul(a5)
		move.l	d6,d1
		jsr	_LVOSPDiv(a5)
*		leave result in d0.l

*	return ( y + y*R);
		move.l	d5,d1
		jsr	_LVOSPMul(a5)
		move.l	d5,d1
		jsr	_LVOSPAdd(a5)
*	result is in d0.l

	move.l	(sp)+,d6
	rts
*}

FFPASIN:
*	assume d7 = input arg
*	assume d1 = trash register
	clr.l	d1
	bra.s	qsincos


******* mathtrans.library/SPAcos ************************
*
*NAME	
* 
*	SPAcos - obtain the arccosine of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPAcos(fnum1);
*                       d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number representing the cosine
*	of an angle and returns the value of said angle in
*	radians
*
*INPUTS
* 
*	fnum1 - Motorola fast floating point number
*
*RESULT
* 
*	fnum2 - Motorola fast floating point number
* 
*BUGS
* 
*	None
*
*SEE ALSO
* 
*	SPSin
******

FFPACOS:
*	assume d7 = input arg
*	assume d1 = trash register
	moveq	#1,d1
*	bra.s	qsincos	* just fall into it

qsincos:
*	inputs d7 = argument
*	d6 = sin/cos control 0/1
*	local variables:
*		y = d5, x = d7, flag = d6
*		i = d4
*		g = d3
*	result	  = d2
	movem.l	d2-d6/a5,-(sp)
	move.l	d1,d6		* transfer to saved register
*	y = abs(x)
	move.l	_MathBase,a5
	move.l	d7,d0		* put in arg register
	jsr	_LVOSPAbs(a5)
	move.l	d0,d5

*	if y > 0.5 then
	move.l	#fphalf,d1
	jsr	_LVOSPCmp(a5)
	blt	lessthanhalf

*	
*	{	/* reduce argument */
*		if (y > 1.0)	return (error);
		move.l	d5,d0
		move.l	#fp1,d1
		jsr	_LVOSPCmp(a5)
		bgt	overflow

*		i = 1 - flag;
		moveq	#1,d4
		sub.l	d6,d4

*		g =  ((0.5-y)+0.5)/2.0;/*(1-y)/2;*/
		move.l	#fphalf,d0
		move.l	d5,d1
		jsr	_LVOSPSub(a5)
		move.l	#fphalf,d1
		jsr	_LVOSPAdd(a5)
		move.l	#fp2,d1
		jsr	_LVOSPDiv(a5)
		move.l	d0,d3

*		y = -2*sqrt(g);
		bsr	MFSPSqrt
		move.l	#minusfp2,d1
		jsr	_LVOSPMul(a5)
		move.l	d0,d5
		
		bsr doit
		move.l	d0,d2
		bra	secondpart
*	}

lessthanhalf:
*	{
*		i = flag;
		move.l	d6,d4

*		if (y>eps)
		move.l	d5,d0
		move.l	#eps,d1
		jsr	_LVOSPCmp(a5)
		ble	ygrtr

*		{
*			g = y*y;
			move.l	d5,d0
			move.l	d5,d1
			jsr	_LVOSPMul(a5)
			move.l	d0,d3

*			result = doit(y,g);
			move.l	d0,d2
			bsr	doit
			move.l	d0,d2
			bra	secondpart
*		}
*		else
ygrtr:
*		{
*			result = y;
			move.l	d5,d2
*		}
*	}


secondpart:
*	if (flag == DOSINE)
	tst.l	d6
	bne	docosine
*	{
*		result = (a[i] + result) + a[i];
*		a little optimization here
		tst.l	d4	* if (i == 0) then a[i] = 0.0
		beq	skipthis
			move.l	d2,d0	*	d0 gets result
			move.l	#asub1,d1
			jsr	_LVOSPAdd(a5)
			move.l	#asub1,d1
			jsr	_LVOSPAdd(a5)
			move.l	d0,d2	* put back in result
skipthis:

*		if (x < 0)	result = -result;
		move.l	d7,d1
		jsr	_LVOSPTst(a5)
		bpl	ispositive
			move.l	d2,d0
			jsr	_LVOSPNeg(a5)
			move.l	d0,d2
ispositive:
*	}
	bra	done

*	else	/* DOING COSINE */
docosine:
*	{
*		if (x < 0)	result = (a[i]-result)+a[i];
		move.l	d7,d1
		jsr	_LVOSPTst(a5)
		bmi	isnegative
			tst.l	d4
			beq	skipthis2
				move.l	#asub1,d0
				move.l	d2,d1
				jsr	_LVOSPSub(a5)
				move.l	#asub1,d1
				jsr	_LVOSPAdd(a5)
				move.l	d0,d2	* put back in result
			bra	done
skipthis2:
*			just negate it
				move.l	d2,d0
				jsr	_LVOSPNeg(a5)
				move.l	d0,d2
			bra	done
isnegative:

*		else		result = (b[i]+result)+b[i];
			move.l	d2,d0
			tst.l	d4
			bne	useone
				move.l	#bsub0,d2
			bra.s useonedone
useone:
				move.l	#bsub1,d2
useonedone:
			move.l	d2,d1
			jsr	_LVOSPAdd(a5)
			move.l	d2,d1
			jsr	_LVOSPAdd(a5)
			move.l	d0,d2
*	}
*	return(result);
done:
	move.l	d2,d7	* put result in d7
	tst.b	d7
	jsr	_GetCC
	andi.b	#$F5,d0
qsincosreturn:
	move.w	d0,CCR
	movem.l	(sp)+,d2-d6/a5	* restore registers
	rts

	end
