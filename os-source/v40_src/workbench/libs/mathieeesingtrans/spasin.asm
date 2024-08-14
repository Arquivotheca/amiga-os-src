
******* mathieeesingtrans.library/IEEESPAsin ********************************
*
*   NAME
*	IEEESPAsin -- compute the arcsine of a number
*
*   SYNOPSIS
*	  x   = IEEESPAsin(  y  );
*	d0	           d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute the arc sine of y in IEEE single precision
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
*	IEEESPSin(), IEEESPAtan(), IEEESPAcos()
**************************************************************************/
********************* SWFP SP Cordic Asin(X) ****************************
*** INPUTS:  D0 = X,        SP IEEE Format.
***
*** OUTPUT:  D0 = Asin(X),  SP IEEE Format.
***
*** BUGS:    None (that I know of at least).
***
*** NOTES:   When ABS(X) > 1.0 the routine just returns PI/2.  I felt
***          that this was the most appropriate value to return in these
***          cases.  
***
*** BY:      Al Aburto ('ala' on BIX)
***          3016 Forrester Court
***          San Diego, Calif, 92123
***          (619) 553-1495 (work)
***          (619) 278-8521 (home)
***
*** VERSION: 10 Nov 1989
***
*** Program Timing (Microseconds, using 68020 and 32-bit RAM at 14.32 MHz).
*** The SWFP_SPAsin is faster than the FFPAsin too.  Accuracy of the
*** SWFP_SPAsin is equal to that of the current IEEESPAsin routine.
***
*** X (IEEESP,Hex)     IEEESPAsin(X)          SWFP_SPAsin(X)
*** $00000000             32.0 usec               4.9 usec
*** $00800000             32.1                    4.1
*** $12800000             31.6                    4.6
*** $26800000             31.9                    4.6
*** $27000000            602.4                    4.9
*** $39800000            613.2                    4.6
*** $39FFFFFF            619.5                    4.6
*** $3A000000            618.0                  117.0
*** $3C000000            619.0                  118.6
*** $3C7FFFFF            625.6                  124.4
*** $3CFF0000            631.2                  121.6
*** $3D000000            618.2                  171.4
*** $3DFFFFFF            622.6                  176.8
*** $3E000000            620.0                  264.4
*** $3E400000            626.4                  262.6
*** $3E7F0000            623.4                  268.8
*** $3EB00000            627.7                  255.0
*** $3EC00000            822.2                  271.4
*** $3EDF0000            825.4                  274.6
*** $3F200000            825.5                  282.0
*** $3F400000            896.6                  276.1
*** $3F500000            902.0                  267.9
*** $3F6C0000            893.6                  273.6
*** $3F6F0000            700.4                  273.8
*** $3F7F0000            698.2                  271.8
*** $3F7FFF00            697.8                  245.6
*** $37FFFFFF            696.3                  250.6
*** $3F800000             91.4                    7.8

*** $3F800001(and Greater)94.4                    8.2
*************************************************************************
	include "xref.i"
	xdef MIIEEESPAsin
	xref MIIEEESPSqrt
MIIEEESPAsin:
;SWFP_SPAsin:
            MOVE.L   D0,D1
            SWAP     D1
            ANDI.W   #$7F80,D1
            CMPI.W   #$3980,D1         ;Is   ABS(X) <= $39FFFFFF?
            BHI.S    S16001            ;No:  ABS(X) >= $3A000000.

                                       ;Yes: ABS(X) <= $39FFFFFF.
            RTS                        ;Return D0 = Asin(X) = X
S16001:
            CMPI.W   #$3C80,D1         ;Is   ABS(X) <= $3CFFFFFF?
            BHI.S    S16002            ;No:  ABS(X) >= $3D000000.
            MOVEM.L  D2/A6,-(A7)       ;Yes: ABS(X) <= $3CFFFFFF.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D2
            MOVE.L   D0,D1
            JSR      _LVOIEEESPMul(A6) ;X*X
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;X*X*X
            MOVE.L   #$3E2AAAAB,D1
            JSR      _LVOIEEESPMul(A6) ;X*X*X*(1/6)
            MOVE.L   D2,D1
            JSR      _LVOIEEESPAdd(A6) ;X+X*X*X*(1/6)
            MOVEM.L  (A7)+,D2/A6
            RTS                        ;D0 = IEEESPAsin(x)
S16002:
            CMPI.W   #$3D80,D1         ;Is   ABS(X) <= $3DFFFFFF?
            BHI.S    S16003            ;No:  ABS(X) >= $3E000000.
            MOVEM.L  D2/D3/A6,-(A7)    ;Yes: ABS(X) <= $3DFFFFFF.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D2             ;Save X
            MOVE.L   D0,D1
            JSR      _LVOIEEESPMul(A6) ;X*X
            MOVE.L   D0,D3             ;Save X*X
            MOVE.L   #$3D99999A,D1
            JSR      _LVOIEEESPMul(A6) ;X*X*(3/40)
            MOVE.L   #$3E2AAAAB,D1
            JSR      _LVOIEEESPAdd(A6) ;(1/6)+X*X*(3/40)
            MOVE.L   D3,D1
            JSR      _LVOIEEESPMul(A6) ;X*X*((1/6)+X*X*(3/40))
            MOVE.L   #$3F800000,D1
            JSR      _LVOIEEESPAdd(A6) ;1+X*X*((1/6)+X*X*(3/40))
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;X*(1+X*X((1/6)+X*X*(3/40)))
            MOVEM.L  (A7)+,D2/D3/A6
            RTS                        ;D0 = IEEESPAsin(X)
S16003:
            MOVE.L   D0,D1             ;Check X = 1.0 Case.
            BCLR.L   #$1F,D1		; error in transmission? #$...F
            CMPI.L   #$3F800000,D1
            BMI.S    S16005
            BTST.L   #$001F,D0
            BNE.S    S16004
            MOVE.L   #$3FC90FDB,D0
            RTS

S16004:     MOVE.L   #$BFC90FDB,D0
            RTS

S16005:
S16009:
            MOVEM.L  D2-D7/A2/A6,-(A7) ;Handle Remaining Cases.
                                       ;**** START CORDIC CODE ****
                                       ;D0 = X, and D1 = ABS(X).

            LEA.L    _ATanCoef29,A2    ;A2 points to S0 FP Scale Factor.

            MOVE.L   D0,D7             ;Save X
            MOVE.L   D1,D0             ;X =  ABS(X)
            MOVE.L   D0,D2             ;Save X
            ADD.L    (A2)+,D0          ;D0 = (FLOAT)(S0 * X)
            MOVE.L   (A2)+,D6          ;Number Of Loops - 1

            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPFix(A6) ;X = D0 =  (LONG)(S0 * X)
            MOVE.L   D0,D3

            MOVE.L   D2,D0
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;X*X
            MOVE.L   D0,D1
            MOVE.L   #$3F800000,D0
            JSR      _LVOIEEESPSub(A6) ;1 - X*X
;           JSR      SWFP_SPSqrt       ;NOTE: ---  I'm using SWFP_SPSqrt.
            BSR      MIIEEESPSqrt       ;NOTE: ---  I'm using SWFP_SPSqrt.

            ADD.L    -$08(A2),D0       ;S0 * SQRT( 1- X*X )
            JSR      _LVOIEEESPFix(A6)
            MOVE.L   D0,D1
            MOVE.L   D3,D0             ;S0 * X

            ADDQ.L   #$04,A2           ;A2 Now --> Atan[2**(-i)] Table.
            MOVEQ.L  #$00,D2           ;Z = D2 = 0

            MOVEQ.L  #$00,D5           ;Shift Counter (i = 0).

                                       ;Start the i = 0 Case.
            MOVE.L   D0,D3             ;XS = X

            SUB.L    D1,D0             ;X  = X - YS
            TST.L    D0
            BMI.S    S16018

            ADD.L    D3,D1             ;Y  = Y + XS
            ADD.L    (A2),D2           ;Z  = Z + S0 * Atan(2**(0))
S16014:
S16016:     MOVE.L   D0,D3             ;XS = X
            MOVE.L   D1,D4             ;YS = Y

            ASR.L    D5,D4             ;YS * 2**(-i)
            SUB.L    D4,D0             ;X = X - YS * 2**(-i)
            TST.L    D0                ;X < 0 ?
            BMI.S    S16018            ;Branch On Yes.

            MOVE.L   D3,D4             ;XS
            ASR.L    D5,D4             ;XS * 2**(-i)
            ADD.L    D4,D1             ;Y = Y + XS * 2**(-i)
            ADD.L    (A2),D2           ;Z = Z + S0 * Atan(2**(-i))
            BRA.S    S16016            ;Do It Again.
S16018:
            MOVE.L   D3,D0             ;X = XS
            ADDQ.L   #$01,D5           ;INC Shift Counter.
            ADDQ.L   #$04,A2           ;INC Table Pointer.
            DBF      D6,S16014         ;Loop until D6 = -1 .

            MOVE.L   D2,D0
                                       ;D0=Z=(LONG)(S0 * IEEESPAsin(X))
            JSR      _LVOIEEESPFlt(A6) ;Convert to IEEESP Format.
            ADDQ.L   #$04,A2           ;Points To End Of Table.
            SUB.L    (A2),D0           ;Remove Scale Factor ( Z/S0 )
                                       ;**** END   CORDIC CODE ****
            BTST.L   #$001F,D7
            BEQ.S    S16020
            BCHG.L   #$001F,D0
S16020:
            MOVEM.L  (A7)+,D2-D7/A2/A6 ;Return D0 = IEEESPAsin(X)
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

**************** End of SWFP_SPAsin *************************************

	end
