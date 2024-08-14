
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeedoubbas.library/IEEEDPAbs ***********************************
*
*   NAME
*	IEEEDPAbs -- compute absolute value of IEEE double precision argument
*
*   SYNOPSIS
*	  x   = IEEEDPAbs(  y  );
*	d0/d1		  d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Take the absolute value of argument y and return it to caller.
*
*   INPUTS
*	y -- IEEE double precision floating point value
*
*   RESULT
*	x -- IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
******************************************************************************
	section mathieeedoubbas

	xdef  MIIEEEDPAbs
MIIEEEDPAbs:
	BCLR	#31,D0		* Make positive.
	RTS

	END
