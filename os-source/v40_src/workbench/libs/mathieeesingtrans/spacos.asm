
******* mathieeesingtrans.library/IEEESPAcos ********************************
*
*   NAME
*	IEEESPAcos -- compute the arc cosine of a number
*
*   SYNOPSIS
*	  x   = IEEESPAcos(  y  );
*	d0	           d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute arc cosine of y in IEEE single precision
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
*	IEEESPCos(), IEEESPAtan(), IEEESPAsin()
**************************************************************************/
********************* SWFP SP Cordic Acos(X) ****************************
*** INPUTS:  D0 = X,        SP IEEE Format.
***
*** OUTPUT:  D0 = Acos(X),  SP IEEE Format.
***
*** BUGS:    No Known Bugs.
***
*** NOTES:   When X > 1.0 the routine just returns 0.0.  This
***          is the most appropriate value to return in this case.  
***
*** BY:      Al Aburto ('ala' on BIX)
***          3016 Forrester Court
***          San Diego, Calif, 92123
***          (619) 553-1495 (work)
***          (619) 278-8521 (home)
***
*** VERSION: 22 Nov 1989
***
*** Program Timing (Microseconds, using 68020 and 32-bit RAM at 14.32 MHz).
*** The SWFP_SPAcos is faster than the FFPAcos too.  Accuracy of the
*** SWFP_SPAcos is equal to that of the current IEEESPAcos routine.
***
*** X (IEEESP,Hex)     IEEESPAcos(X)          SWFP_SPAcos(X)
*** $00000000             54.4 usec               2.8 usec
*** $00800000             76.4                    2.8
*** $1A000000             77.4                    3.0
*** $1A800000            464.2                    3.0
*** $1F800000            488.8                    2.9
*** $20000000            678.1                    3.4
*** $337FFFFF            676.5                    3.1
*** $33800000            693.0                   28.4
*** $38000000            676.3                   28.1
*** $3A000000            700.4                   28.0
*** $3B7FFFFF            698.5                   28.1
*** $3B800000            697.7                  143.2
*** $3D000000            700.5                  143.2
*** $3D7FFFFF            699.8                  148.9
*** $3D800000            698.4                  272.7
*** $3E000000            699.3                  275.6
*** $3E800000            699.6                  269.8
*** $3EA00000            690.6                  260.7
*** $3EC00000            891.6                  268.1
*** $3EE00000            893.1                  280.2
*** $3F000000            897.9                  266.9
*** $3F200000            906.7                  254.5
*** $3F300000            904.4                  263.5
*** $3F500000            824.6                  275.5
*** $3F700000            623.8                  260.6
*** $3F7F0000            622.2                  282.4
*** $37FFFFFF            622.0                  230.9
*** $3F800000             93.3                    7.1

*** $3F800001 (& Greater) 93.0                    6.8
*************************************************************************
	include "xref.i"
	xref	MIIEEESPSqrt

	xdef	MIIEEESPAcos
MIIEEESPAcos:
;SWFP_SPAcos:
            MOVE.L   D0,D1
            SWAP     D1
            ANDI.W   #$7F80,D1
            CMPI.W   #$3300,D1         ;Is   ABS(X) <= $337FFFFF?
            BHI.S    S17001            ;No:  ABS(X) >= $33800000.

                                       ;Yes: ABS(X) <= $337FFFFF.
            MOVE.L   #$3FC90FDB,D0
            RTS                        ;D0 = IEEESPAcos(X) = PI/2
S17001:
            CMPI.W   #$3B00,D1         ;Is   ABS(X) <= $3B7FFFFF?
            BHI.S    S17002            ;No:  ABS(X) >= $3B800000.
            MOVEM.L  D2/A6,-(A7)       ;Yes: ABS(X) <= $3B7FFFFF.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D1             ;X
            MOVE.L   #$3FC90FDB,D0
            JSR      _LVOIEEESPSub(A6) ;PI/2 - X
            MOVEM.L  (A7)+,D2/A6
            RTS                        ;D0 = IEEESPAcos(x) = PI/2 - X
S17002:
            CMPI.W   #$3D00,D1         ;Is   ABS(X) <= $3D7FFFFF?
            BHI.S    S17003            ;No:  ABS(X) >= $3D800000.
            MOVEM.L  D2/A6,-(A7)       ;Yes: ABS(X) <= $3D7FFFFF.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D2             ;Save X
            MOVE.L   D0,D1
            JSR      _LVOIEEESPMul(A6) ;X*X
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;X*X*X
            MOVE.L   #$3E2AAAAB,D1
            JSR      _LVOIEEESPMul(A6) ;(1/6)*X*X*X
            MOVE.L   D2,D1
            JSR      _LVOIEEESPAdd(A6) ;X+(1/6)*X*X*X
            MOVE.L   D0,D1
            MOVE.L   #$3FC90FDB,D0
            JSR      _LVOIEEESPSub(A6) ;
            MOVEM.L  (A7)+,D2/A6
            RTS                        ;D0 = IEEESPAcos(X)
S17003:
            MOVE.L   D0,D1             ;Check X = +/- 1.0 Cases.
            BCLR.L   #$001F,D1
            CMPI.L   #$3F800000,D1
            BMI.S    S17005
            BTST.L   #$001F,D0
            BNE.S    S17004
            MOVEQ.L  #$00,D0
            RTS
S17004:
            MOVE.L   #$40490FDB,D0
            RTS
S17005:
S17009:
            MOVEM.L  D2-D7/A2/A6,-(A7) ;Handle Remaining Cases.
                                       ;**** START CORDIC CODE ****
                                       ;D0 = Y, and D1 = ABS(Y).

            LEA.L    _ATanCoef29,A2    ;A2 points to S0 FP Scale Factor.

            MOVE.L   D0,D7             ;Save Y
            MOVE.L   D1,D0             ;Y =  ABS(Y)
            MOVE.L   D0,D2             ;Save Y
            ADD.L    (A2)+,D0          ;D0 = (FLOAT)(S0 * Y)
            MOVE.L   (A2)+,D6          ;Number Of Loops - 1

            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPFix(A6) ;Y = D0 =  (LONG)(S0 * Y)
            MOVE.L   D0,D3

            MOVE.L   D2,D0
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;Y*Y
            MOVE.L   D0,D1
            MOVE.L   #$3F800000,D0
            JSR      _LVOIEEESPSub(A6) ;1 - Y*Y
            BSR      MIIEEESPSqrt       ;NOTE: ---  I'm using SWFP_SPSqrt.

            ADD.L    -$08(A2),D0       ;S0 * SQRT( 1- Y*Y )
            JSR      _LVOIEEESPFix(A6) ;X

            MOVE.L   D3,D1             ;S0 * Y

            ADDQ.L   #$04,A2           ;A2 Now --> Atan[2**(-i)] Table.
            MOVEQ.L  #$00,D2           ;Z = D2 = 0

            MOVEQ.L  #$00,D5           ;Shift Counter (i = 0).

                                       ;Start the i = 0 Case.
            MOVE.L   D0,D3             ;XS = X

            SUB.L    D1,D0             ;X  = X - YS
            TST.L    D0
            BMI.S    S17018

            ADD.L    D3,D1             ;Y  = Y + XS
            ADD.L    (A2),D2           ;Z  = Z + S0 * Atan(2**(0))
S17014:
S17016:     MOVE.L   D0,D3             ;XS = X
            MOVE.L   D1,D4             ;YS = Y

            ASR.L    D5,D4             ;YS * 2**(-i)
            SUB.L    D4,D0             ;X = X - YS * 2**(-i)
            TST.L    D0                ;X < 0 ?
            BMI.S    S17018            ;Branch On Yes.

            MOVE.L   D3,D4             ;XS
            ASR.L    D5,D4             ;XS * 2**(-i)
            ADD.L    D4,D1             ;Y = Y + XS * 2**(-i)
            ADD.L    (A2),D2           ;Z = Z + S0 * Atan(2**(-i))
            BRA.S    S17016            ;Do It Again.
S17018:
            MOVE.L   D3,D0             ;X = XS
            ADDQ.L   #$01,D5           ;INC Shift Counter.
            ADDQ.L   #$04,A2           ;INC Table Pointer.
            DBF      D6,S17014         ;Loop until D6 = -1 .

            MOVE.L   D2,D0
                                       ;D0=Z=(LONG)(S0 * IEEESPAcos(X))
            JSR      _LVOIEEESPFlt(A6) ;Convert to IEEESP Format.
            ADDQ.L   #$04,A2           ;Points To End Of Table.
            SUB.L    (A2),D0           ;Remove Scale Factor ( Z/S0 )
                                       ;**** END   CORDIC CODE ****
            BTST.L   #$001F,D7
            BEQ.S    S17020
            MOVE.L   D0,D1
            MOVE.L   #$40490FDB,D0
            JSR      _LVOIEEESPSub(A6)
S17020:
            MOVEM.L  (A7)+,D2-D7/A2/A6 ;Return D0 = IEEESPAcos(X)
            RTS

                                  ;Long Word Align This!!  It can cost
                                  ;about 10 usec if NOT Long Word Aligned.
_ATanCoef29:
            DC.L     $0E800000    ;S0 FP Scale Factor.
            DC.L     $0000001C    ;Number Of Loops - 1
            DC.L     $20000000    ;S0 = 2**(29)

                                  ;S0 * Atan[(2**(-i)] Table.
            DC.L     $1921FB60    ;i = 0
            DC.L     $0ED63390
            DC.L     $07D6DD80
            DC.L     $03FAB754
            DC.L     $01FF55BC
            DC.L     $00FFEAAE    ;i = 5
            DC.L     $007FFD55
            DC.L     $003FFFAB
            DC.L     $001FFFF5
            DC.L     $000FFFFF
            DC.L     $00080000    ;i = 10
            DC.L     $00040000
            DC.L     $00020000
            DC.L     $00010000
            DC.L     $00008000
            DC.L     $00004000    ;i = 15
            DC.L     $00002000
            DC.L     $00001000
            DC.L     $00000800
            DC.L     $00000400
            DC.L     $00000200    ;i = 20
            DC.L     $00000100
            DC.L     $00000080
            DC.L     $00000040
            DC.L     $00000020    ;i = 24
            DC.L     $00000010    ;i = 25
            DC.L     $00000008    ;i = 26
            DC.L     $00000004    ;i = 27
            DC.L     $00000002    ;i = 28
            DC.L     $00000001    ;i = 29
            DC.L     $0E800000    ;S0 FP Scale factor Must Be At
                                  ;End Of Table.

***************** End of SWFP_SPAcos.asm ********************************


	end
