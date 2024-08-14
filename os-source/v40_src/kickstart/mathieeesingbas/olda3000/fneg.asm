*******************************************************************************
*
*	$Id: fneg.asm,v 37.1 91/02/07 16:28:32 mks Exp $
*
*******************************************************************************

******* mathieeesingbas.library/IEEESPNeg ***********************************
*
*   NAME
*	IEEESPNeg -- compute negative value of IEEE single precision number
*
*   SYNOPSIS
*	  x   = IEEESPNeg(  y  )
*	  d0		   d0
*
*	  float IEEESPNeg(float);
*
*   FUNCTION
*	Invert the sign of argument y and return it to caller.
*
*   INPUTS
*	y - IEEE single precision floating point value
*
*   RESULT
*	x - IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
******************************************************************************
	section mathieeesingbas

	xdef	MIIEEESPNeg
MIIEEESPNeg:
	BCHG	#31,D0	* invert sign bit
	RTS
*
	END
