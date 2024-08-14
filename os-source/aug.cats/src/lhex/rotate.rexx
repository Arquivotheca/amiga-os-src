/*  P = number of X,Y,Z triplets
    X() = array containing X coords
    Y() = array containing Y coords
    Z() = array containing Z coords
    N1=direction cosine of rotation Axis w.r.t. X-direction
    N2=direction cosine of rotation axis w.r.t. Y-direction
    N3=direction cosine of rotation axis w.r.t. Z-direction 
    T1=rotation angle in degrees
*/


say hi

P = 10
T1 = 45
N1 = 1
N2 = 2
N3 = 1
X. = 0
Y. = 0
Z. = 0
rotate()

exit

rotate: procedure expose P X. Y. Z. N1 N2 N3 T1 V.


if show("l", "rexxmathlib.library") = 0 then do
   check = addlib('rexxsupport.library', 0, -30, 0)
   check = addlib('rexxmathlib.library',  0, -30, 0)
end

    U. = 0
    V. = 0

    Do I = 1 to P
    	U.I.1 = X.I
    	U.I.2 = Y.I
    	U.I.3 = Z.I
    	U.I.4 = 1
    End

    T2=T1/57.2957795		/* Convert T1 to radians */
    T. = 1			/* Set up transformation matrix */

    T.1.1 = N1*N1+(1-N1*N1)*COS(T2)
    T.1.2 = N1*N2*(1-COS(T2))+N3*SIN(T2)
    T.1.3 = N1*N3*(1-COS(T2))-N2*SIN(T2)

    T.2.1 = N1*N2*(1-COS(T2))-N3*SIN(T2)
    T.2.2 = N2*N2+(1-N2*N2)*COS(T2)
    T.2.3 = N2*N3*(1-COS(T2))+N1*SIN(T2)

    T.3.1 = N1*N3*(1-COS(T2))+N2*SIN(T2)
    T.3.2 = N2*N3*(1-COS(T2))-N1*SIN(T2)
    T.3.3 = N3*N3+(1-N3*N3)*COS(T2)

    /* Multiply U and T to get V matrix */

    Do I = 1 to P
    	Do J = 1 to 4
	    Do K = 1 to 4
	    	V.I.J = V.I.J + U.I.J * T.K.J
	    End
	End
    End

    Do I = 1 to P
	X.I = V.I.1
	Y.I = V.I.2
	Z.I = V.I.3
    End

    return 0