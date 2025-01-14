
******* mathieeesingtrans.library/IEEESPAtan ********************************
*
*   NAME
*	IEEESPAtan -- compute the arc tangent of number
*
*   SYNOPSIS
*	  x   = IEEESPAtan(  y  );
*	d0		   d0
*
*	single	x,y;
*
*   FUNCTION
*	Compute arctangent of y in IEEE single precision
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
**************************************************************************/
********************* SWFP CORDIC SP Atan(x) ****************************
*** INPUTS:  D0 = X,        IEEESP Format.
***
*** OUTPUTS: D0 = Atan(X),  IEEESP Format.
***
*** BUGS:    No known bugs.
***
*** BY:      Al Aburto ('ala' on BIX)
***          3016 Forrester Court
***          San Diego, Calif, 92123
***          (619) 553-1495 (work)
***          (619) 278-8521 (home)
***
*** INITIAL VERSION: 10 Nov 1989
*** REVISION:        12 Apr 1990
***                  Fixed bug in routines at S15005, S15007, and S15008
***                  so that routines satisfies Atan(x) = -Atan(-x).
***
*** NOTES:   I use two _ATanTables (25 and 30 elements). The 30 element
***          helps me preserve accuracy on the low end and the 25 element
***          helps me reach above x = 1.0 so I can fit fast polynomials
***          when x > 32 ---
***
***
***
*** Program Timing (Microseconds, 68020 with 32-bit RAM at 14.32 MHz).
*** The IEEESPAtan and SWFP_SPAtan results have the similar accuracy in
*** within +/- the least significant bit.
***
*** X (IEEESP)    IEEESPAtan(X)     SWFP_SPAtan(X)     FFPAtan(X)
*** $00000000        268.0                2.6             26.6
*** $00800000        346.0                2.6             26.3
*** $03800000        329.1                2.7             26.4
*** $12800000        329.0                2.3             26.3
*** $35800000        394.5                2.7             82.8
*** $38000000        394.2                2.4             79.8
*** $39FF0000        403.6                2.7            104.0
*** $3A000000        399.2              128.3             86.8
*** $3EC00000        401.3              162.1            112.6
*** $3ECCCCCD        599.1              155.3            107.6
*** $3F000000        603.4              119.1             81.4
*** $3FC00000        680.2              157.7            164.8
*** $40000000        677.6              144.4            158.4
*** $42800000        470.9               75.0            161.8
*** $44000000        471.5               72.7            163.6
*** $45FFFFFF        467.5               70.2            165.9
*** $4BFFFFFF        467.2               70.5            161.3
*** $4C000000        466.7                5.4            162.0
*** $4E800000        464.2                5.5             95.1
*** $53800000        464.6                5.3             94.8
*** $5D800000        464.6                5.0             94.8
*** $6C800000        399.5                5.2             94.8
*** $7E800000        419.0                5.4             94.5
*** $7F800000        308.8                5.4             94.5
*************************************************************************
	include "xref.i"
	xdef	MIIEEESPAtan

MIIEEESPAtan:
SWFP_SPAtan:
            MOVE.L   D0,D1
            SWAP     D1
            ANDI.W   #$7F80,D1
            CMPI.W   #$3980,D1         ;Is   ABS(X) <= $39FFFFFF?
            BHI.S    S15001            ;No:  ABS(X) >= $3A000000.

                                       ;Yes: ABS(X) <= $39FFFFFF.
            RTS                        ;Return D0 = Atan(X) = X

S15001:
            CMPI.W   #$3C80,D1         ;Is   ABS(X) <= $3CFFFFFF?
            BHI.S    S15003            ;No:  ABS(X) >= $3D000000.
            MOVEM.L  D2/A6,-(A7)       ;Yes:
            MOVE.L   D0,D1
            MOVE.L   D0,D2
            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPMul(A6) ;X*X
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;X*X*X
            MOVE.L   #$BEAAAAAB,D1
            JSR      _LVOIEEESPMul(A6) ;-X*X*X/3
            MOVE.L   D2,D1
            JSR      _LVOIEEESPAdd(A6) ;X-X*X*X/3
            MOVEM.L  (A7)+,D2/A6       ;D0 = Atan(X) = X-X*X*X/3
            RTS

S15003:
            CMPI.W   #$4C00,D1         ;Is   ABS(X) >= $4C000000?
            BMI.S    S15005            ;No:  ABS(X) <= $4BFFFFFF.

            MOVE.L   #$3FC90FDB,D1     ;Yes: ABS(X) >= $4C000000.
	    tst.l    d0
	    bge.s    S15004
            BCHG.L   #$001F,D1
S15004:
            MOVE.L   D1,D0             ;Return D0 = Atan(X) = PI/2
            RTS

S15005:
            CMPI.W   #$4380,D1         ;Is   ABS(X) >= $43800000?
            BMI.S    S15007            ;No:  ABS(X) <= $437FFFFF.

            MOVE.L   A6,-(A7)          ;Yes: ABS(X) >= $43800000.
            MOVE.L   D0,D1
            MOVE.L   #$BF800000,D0
            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPDiv(A6) ;D0 = - 1 / X

	    tst.l	d0
	    bmi.s	T15001
            MOVE.L   #$BFC90FDB,D1
            BRA.S    T15002
T15001:
            MOVE.L   #$3FC90FDB,D1
T15002:
            JSR      _LVOIEEESPAdd(A6) ;D0 = Atan(X) = PI/2 - 1/X
            MOVEA.L  (A7)+,A6
            RTS

S15007:
            CMPI.W   #$4300,D1         ;Is   ABS(X) >= $43000000?
            BMI.S    S15008            ;No:  ABS(X) <= $43FFFFFF.

            MOVE.L   A6,-(A7)          ;Yes: ABS(X) >= $43000000.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D1
            MOVE.L   #$BF7FFDBA,D0
            JSR      _LVOIEEESPDiv(A6)


	    tst.l    d0
	    bmi.s    U15001
            MOVE.L   #$BFC90FDA,D1
            BRA.S    U15002
U15001:
            MOVE.L   #$3FC90FDA,D1
U15002:
            JSR      _LVOIEEESPAdd(A6)
                                       ;D0 = Atan(X) = A+B/X
            MOVEA.L  (A7)+,A6
            RTS

S15008:
            CMPI.W   #$4280,D1         ;Is   ABS(X) >= $42800000?
            BMI.S    S15009            ;No:  ABS(X) <= $427FFFFF.

            MOVE.L   A6,-(A7)          ;Yes: ABS(X) >= $42800000.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D1
            MOVE.L   #$BF7FF6BC,D0
            JSR      _LVOIEEESPDiv(A6)

	    tst.l    d0
	    bmi.s    U15001
            MOVE.L   #$BFC90FD2,D1
            BRA.S    V15002
V15001:
            MOVE.L   #$3FC90FD2,D1
V15002:
            JSR      _LVOIEEESPAdd(A6)
                                       ;D0 = Atan(X) = A+B/X
            MOVEA.L  (A7)+,A6
            RTS

S15009:
            MOVEM.L  D4-D6/A2/A6,-(A7) ;Handle Remaining Cases.
                                       ;**** START CORDIC CODE ****
                                       ;D0 = X
	    move.l	d0,-(sp)	; save sign on stack
;           MOVE.L   D0,D7
            BCLR.L   #$001F,D0         ;X = ABS(X)

            LEA.L    _ATanCoef24,A2
            CMPI.W   #$3F80,D1
            BHI.S    S15012
            LEA.L    _ATanCoef29,A2    ;A2 points to S0 FP Scale Factor.
S15012:
            ADD.L    (A2)+,D0          ;D0 = (FLOAT)(S0 * X)
            MOVE.L   (A2)+,D6          ;Number Of Loops - 1

            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPFix(A6) ;X = D0 =  (LONG)(S0 * X)

	move.l	d2,a0		; save in address registers
	move.l	d3,a1

            MOVE.L   (A2)+,D1          ;Y = D1 = S0
                                       ;A2 Now --> Atan[2**(-i)] Table.
            MOVEQ.L  #$00,D2           ;Z = D2 = 0

            MOVEQ.L  #$00,D5           ;Shift Counter (i = 0).

                                       ;Start the i = 0 Case.
            MOVE.L   D0,D3             ;XS = X

            SUB.L    D1,D0             ;X  = X - YS
            BMI.S    S15018

            ADD.L    D3,D1             ;Y  = Y + XS
            ADD.L    (A2),D2           ;Z  = Z + S0 * Atan(2**(0))
            MOVE.L   D0,D3             ;XS = X
S15014:     MOVE.L   D1,D4             ;YS = Y

            ASR.L    D5,D4             ;YS * 2**(-i)
            SUB.L    D4,D0             ;X = X - YS * 2**(-i)
            BMI.S    S15018            ;Branch On Yes.

looper:
            ASR.L    D5,D3             ;XS * 2**(-i)
            ADD.L    D3,D1             ;Y = Y + XS * 2**(-i)
            ADD.L    (A2),D2             ;Z = Z + S0*Atan(2**(-i))
	move.l	d0,d3
	move.l	d1,d4
	asr.l	d5,d4
	sub.l	d4,d0
	bge.s	looper
S15018:
            MOVE.L   D3,D0             ;X = XS
            ADDQ.W   #$01,D5           ;INC Shift Counter.
            ADDQ.L   #$04,A2           ;INC Table Pointer.
            DBF      D6,S15014         ;Loop until D6 = -1 .

            MOVE.L   D2,D0
	move.l	a0,d2		; restore these registers now
	move.l	a1,d3
                                       ;D0=Z=(LONG)(S0 * IEEESPAtan(X))
            JSR      _LVOIEEESPFlt(A6) ;Convert to IEEESP Format.
            ADDQ.L   #$04,A2           ;Points To End Of Table.
            SUB.L    (A2),D0           ;Remove Scale Factor ( Z/S0 )
                                       ;**** END   CORDIC CODE ****
            tst.l	(sp)+		; retrieve sign from stack
            bge.S    S15020
            BCHG.L   #$001F,D0
S15020:
            MOVEM.L  (A7)+,D4-D6/A2/A6 ;Return D0 = IEEESPAtan(X)
            RTS

********************* Atan CORDIC Coefficients **************************
            CNOP  0,4
            DC.L  $00000000

_ATanCoef24:                      ;24 because I start counting at zero.
            DC.L     $0C000000    ;S0 FP Scale Factor.
            DC.L     $00000017    ;Number Of Loops - 1
            DC.L     $01000000    ;S0 = 2**(24)

                                  ;Table Of S0 * Atan[2**(-i)] Values.
            DC.L     $00C90FDB    ;i = 0
            DC.L     $0076B19C
            DC.L     $003EB6EC
            DC.L     $001FD5BB
            DC.L     $000FFAAE
            DC.L     $0007FF55    ;i = 5
            DC.L     $0003FFEB
            DC.L     $0001FFFD
            DC.L     $00010000
            DC.L     $00008000
            DC.L     $00004000    ;i = 10
            DC.L     $00002000
            DC.L     $00001000
            DC.L     $00000800
            DC.L     $00000400
            DC.L     $00000200    ;i = 15
            DC.L     $00000100
            DC.L     $00000080
            DC.L     $00000040
            DC.L     $00000020
            DC.L     $00000010    ;i = 20
            DC.L     $00000008
            DC.L     $00000004
            DC.L     $00000002
            DC.L     $00000001    ;i = 24
            DC.L     $0C000000    ;S0 FP Scale factor Must be at
                                  ;End of Table.

_ATanCoef29:
            DC.L     $0E800000    ;S0 FP Scale Factor.
            DC.L     $0000001C    ;Number Of Loops - 1
            DC.L     $20000000    ;S0 = 2**(29)

_ATanTable29:                     ;S0 * Atan[2**(-i)] Table.
            DC.L     $1921FB54    ;i = 0
            DC.L     $0ED63382
            DC.L     $07D6DD7E
            DC.L     $03FAB753
            DC.L     $01FF55BB
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

********************** Say Good Night Carol! ****************************

	end
