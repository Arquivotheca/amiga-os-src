
100 SUB"3dGENROT":P,X(),Y(),Z(),N1,N2,N3,T1
    'P = number of X,Y,Z triplets
    'X() = array containing X coords
    'Y() -= array containing Y coords
    'Z() = array containing Z coords
    'N1=DIRECTION COSINE OF rotation Axis w.r.t. X-direction
    'N2=direction cosine of rotation axis w.r.t. Y-direction
    'N3=direction cosine of rotation axis w.r.t. Z-direction 
    'T1=rotation angle in degrees

190 DIM U(100,4),V(100,4)	'100 position vectors
200 MAT U = ZER(P,4)		'Resize and zero matrix
    MAT V = ZER(P,4)

220 For I = 1 to P
    LET U(I,1) = X(I)
    LET U(I,2) = Y(I)
    LET U(I,3) = Z(I)
    LET U(I,4) = 1
    NEXT I

280 MAT T = ZER(4,4)		'Redim and zero matrix
    LET T2=T1/57.2957795	'Convert T1 to radians
    LET T(4,4) = 1		'Set up transformation matrix
    LET T(1,1) = N1*N1+(1-N1*N1)*COS(T2)
    LET T(1,2) = N1*N2*(1-COS(T2))+N3*SIN(T2)
    LET T(1,3) = N1*N3*(1-COS(T2))-N2*SIN(T2)

    LET T(2,1) = N1*N2*(1-COS(T2))-N3*SIN(T2)
    LET T(2,2) = N2*N2+(1-N2*N2)*COS(T2)
    LET T(2,3) = N2*N3*(1-COS(T2))+N1*SIN(T2)

    LET T(3,1) = N1*N3*(1-COS(T2))+N2*SIN(T2)
    LET T(3,2) = N2*N3*(1-COS(T2))-N1*SIN(T2)
    LET T(3,3) = N3*N3+(1-N3*N3)*COS(T2)

    MAT V = U*T
    FOR I = 1 to P
	LET X(I)=V(I,1)
	LET Y(I)=V(I,2)
	LET Z(I)=V(I,3)
    NEXT I

460 SUBEND