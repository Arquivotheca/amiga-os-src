
******* mathieeesingtrans.library/IEEESPSin ********************************
*
*   NAME
*	IEEESPSin -- compute the sine of a floating point number
*
*   SYNOPSIS
*	  x   = IEEESPSin(  y  );
*	d0		  d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute sine of y in IEEE single precision
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
*	IEEESPAsin(), IEEESPTan(), IEEESPCos()
**************************************************************************/
********************* SP Cordic Sine Routine ****************************
*** INPUTS:  D0 = T, an Angle in Radians, IEEESP Format.
***
*** OUTPUTS: D0 = SIN(T),  IEEESP Format.
***
*** NOTES:   The Range Reduction algorithm is from Cody and Waite's book
***          'Software Manual for the Elementary Functions'.  It chews
***          up the most time in this program (200 usec, 14.32 MHz 020). I
***          have not found a more efficient and accurate method.  I checked
***          the MathFFP algorithm and it is fast but it is not as accurate
***          as Cody and Waite's method!  So, dispite the loss in run time
***          I'm sticking to the methods used in this program.
***
***          I had to use the _ATanTable30 (30 elements).  I tried to get
***          to get around that but I just was not happy with the
***          _ATanTable29 (29 elements) results.  Anyway, now I have 3
***          Tables (30, 29, and 24 elements) for the Trig functions.
***
***          Because of the polynomial approximations this program
***          is specific to SIN(T).  COS(T), TAN(T), and SINCOS(T) must
***          be done separately.  The range reduction and CORDIC algorithms
***          will however be the same for SIN(T),COS(T),TAN(T) and
***          SINCOS(T) so with a BSR/JSR some code space could be saved
***          with little loss in performance. I have not done this
***          though.
***
*** BUGS:    None, except that program does not check for maximum angle
***          (TMax) beyond which the program has no accuracy whatsoever.
***
*** By:      Al Aburto ('ala' on BIX)
***          3016 Forrester Court
***          San Diego, Calif, 92123
***          (619) 553-1495 (work)
***          (619) 278-8521 (home)
***
*** Version: 29 Mar 1990  --- Fixed Bug in the sign when the angle (T)
***                           was in the range -2**(-04) through -PI/2.
***
*** Program Timing (Microseconds, 68020 with 32-bit RAM at 14.32 MHz).
*** I used the IEEEDPSin, and values published in Abramowitz and Segun's
*** book ('Handbook of Mathematical Functions') to check for accuracy. The
*** errors are shown in parenthesis.
***
***         T          IEEESPSin(T)           SWFP_SPSin(T)
***     (radians)        (usec)                  (usec)
***      0.00             331.8                    2.4
***      2**(-126)        430.0                    2.8
***      2**(-090)        392.1                    2.6
***      2**(-060)        488.6                    2.4
***      2**(-030)        482.0                    2.4
***      2**(-012)        483.8                    2.4
***      2**(-011)        485.8                  126.5
***      2**(-005)        493.3                  125.9
***      2**(-004)        493.3  (+2.0E-09)      133.0   (-1.7E-09)
***      0.125            493.5                  133.6
***      0.20             491.0  (-1.5E-09)      132.8   (-1.5E-09)
***      0.30             493.8  (+9.6E-09)      132.6   (+9.6E-09)
***      0.50             493.7  (-1.9E-08)      133.0   (+1.1E-08)
***      0.80             500.8  (+5.4E-08)      134.2   (-5.1E-09)
***      1.00             489.5  (-2.8E-08)      133.2   (-2.8E-08)
***      1.30             492.0  (-4.8E-08)      133.2   (+1.2E-08)
***      1.50             489.6  (+9.0E-09)      133.7   (+9.0E-09)
***      PI/2             353.1                  134.1
***      1.58             484.8  (-2.5E-08)      331.3   (-2.5E-08)
***      1.80             484.6  (+5.6E-08)      333.2   (-3.2E-09)
***      2.00             483.4  (+3.9E-08)      326.4   (-2.0E-08)
***      2.50             520.5  (+9.3E-08)      327.3   (+3.4E-08)
***      3.00             520.9  (+3.8E-08)      326.6   (-6.4E-09)
***      3.10             521.7  (+2.9E-07)      316.7   (+9.7E-08)
***      3.1415           521.2  (+2.2E-07)      208.0   (+3.8E-09)
***      3.14159          516.4  (-3.2E-08)      207.7   (-1.1E-07)
***      3.1415926        512.3  (+1.3E-07)      208.2   (+9.7E-08)
***      PI               370.6                   11.3
***      4.00             498.2  (+1.2E-07)      331.3   (-4.0E-09)
***      6.283185         510.9  (-6.7E-08)      211.4   (+5.2E-09)
***      PI*2             368.6                   11.6
***      9.00             513.0  (+5.8E-07)      329.1   (-2.1E-08)
***     11.00             485.8  (-1.8E-08)      329.4   (-1.8E-08)
***     12.50             521.9  (-2.7E-07)      330.1   (-4.6E-09)
***     15.50             515.3  (+4.9E-07)      331.9   (-2.5E-09)
***     15.707            517.2  (+1.4E-06)      333.8   (+2.2E-07)
***     15.7079           514.6  (+1.1E-06)      211.1   (-4.7E-08)
***     20.10             482.1  (-3.4E-07)      332.6   (+8.1E-08)
***     30.00             490.3  (-1.2E-07)      331.9   (-1.7E-09)
***     61.00             490.8  (+3.9E-07)      333.6   (-2.9E-08)
***     95.00             520.3  (-4.8E-06)      332.0   (-2.2E-08)
***     97.3893           508.6  (+5.6E-06)      212.7   (+2.5E-06)
***    125.00             517.8  (-3.3E-06)      332.7   (+5.1E-08)
***    125.6637           504.6  (-5.8E-06)      213.4   (-3.7E-06)
***    187.00             488.8  (-9.2E-07)      330.7   (-2.1E-08)
***    240.00             486.8  (-1.7E-06)      333.8   (+2.5E-08)
***    635.00             515.8  (-2.2E-05)      340.5   (-4.5E-09)
***    637.743            510.9  (+2.7E-05)      212.7   (+2.3E-05)
***   1255.00             493.0  (+5.3E-06)      332.8   (-2.6E-08)
***   5000.00             489.0  (-5.6E-05)      332.8   (+2.1E-08)
***   8000.00             488.3  (-1.8E-05)      336.0   (-5.2E-09)
***  10000.00             519.0  (+6.9E-04)      337.2   (+3.7E-08)
*************************************************************************
	include	"xref.i"

	xdef	MIIEEESPSin
	xdef	_rsin
_rsin:	move.l	4(sp),d0
MIIEEESPSin:
;SWFP_SPSin:
            MOVE.L   D0,D1             ;Test For FP Small Angles 'T'.
            SWAP     D1                ;Less than or equal to Thresholds.
            ANDI.W   #$7F80,D1
            CMPI.W   #$3980,D1         ;Is ABS(T) <= $39FFFFFF?
            BHI.S    S19001            ;No ABS(T) >= $3A000000
            RTS                        ;D0 = SIN(T) = T.
S19001:
                                       ;Another Small Angle Approximation.
            CMPI.W   #$3D00,D1         ;Is ABS(T) <= $3D7FFFFF?
            BHI.S    S19003            ;No ABS(T) >= $3D800000
S19002:
            MOVEM.L  D2/A6,-(A7)
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D2
            MOVE.L   D0,D1
            JSR      _LVOIEEESPMul(A6) ;T*T
            MOVE.L   D2,D1
            JSR      _LVOIEEESPMul(A6) ;T*T*T
            MOVE.L   #$3E2AAAAB,D1     ;A = 1/6
            JSR      _LVOIEEESPMul(A6) ;A*T*T*T
            MOVE.L   D0,D1
            MOVE.L   D2,D0
            JSR      _LVOIEEESPSub(A6) ;T-A*T*T*T
            MOVEM.L  (A7)+,D2/A6
            RTS                        ;D0 = SIN(T) = T-A*T*T*T
S19003:
            MOVEM.L  D2-D7/A6,-(A7)    ;Cases for ABS(T) >= $3D800000

                                       ;**** START RANGE REDUCTION CODE ****
            MOVE.L   D0,D7             ;To Hold SIGN of 'T'.
            BCLR.L   #$001F,D0         ;Form ABS(T).
S19101:
            CMPI.L   #$3FC90FDB,D0     ;ABS(T) - PI/2 <= 0 ?
            BLS      S19106            ;Yes --- Skip Range Reduction Code.
                                       ;No  --- Do Range Reduction.
            MOVE.L   D0,D1
            ANDI.L   #$000FFFFF,D1
            CMPI.L   #$00090FDB,D1
            BNE.S    S19102
            MOVEQ.L  #$00,D0
            MOVEM.L  (A7)+,D2-D7/A6
            RTS

S19102:
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D2             ;D2 = ABS(T)
            MOVE.L   #$3EA2F983,D1     ;(1/PI)
            JSR      _LVOIEEESPMul(A6) ;Q  = ABS(T) * (1/PI)
            MOVE.L   #$3F000000,D1     ;0.5
            JSR      _LVOIEEESPAdd(A6) ;Q+0.5
            JSR      _LVOIEEESPFloor(A6)
            MOVE.L   D0,D3             ;D3 = TN = FINT(Q). Round to Nearest.

            JSR      _LVOIEEESPFix(A6) ;N = FIX(TN)
            BTST.L   #$0000,D0         ;is N EVEN ?
            BEQ.S    S19103            ;Yes --- No SIGN Change then.
            BCHG.L   #$001F,D7         ;No  --- Handle COS(N*PI)=(-1)**N
                                       ;        where N is ODD then.
S19103:
            MOVE.L   D3,D0             ;TN
            MOVE.L   #$C0490000,D1     ;-C1
            JSR      _LVOIEEESPMul(A6) ;-TN*C1
            MOVE.L   D2,D1             ;ABS(T)
            JSR      _LVOIEEESPAdd(A6) ;ABS(T)-TN*C1
            MOVE.L   D0,D4

            MOVE.L   D3,D0             ;TN
            MOVE.L   #$BA7DAA22,D1     ;-C2
            JSR      _LVOIEEESPMul(A6) ;-TN*C2
            MOVE.L   D4,D1             ;(ABS(T)-TN*C1)
            JSR      _LVOIEEESPAdd(A6) ;(ABS(T)-TN*C1)-TN*C2

            BTST.L   #$001F,D7
            BEQ.S    S19104
            BCHG.L   #$001F,D0
S19104:
            MOVE.L   D0,D1             ;Test For FP Small Angles 'T'.
            SWAP     D1                ;Less than or equal to Thresholds.
            ANDI.W   #$7F80,D1
            CMPI.W   #$3980,D1         ;Is ABS(T) <= $39FFFFFF?
            BHI.S    S19105            ;No ABS(T) >= $3A000000
            MOVEM.L  (A7)+,D2-D7/A6
            RTS                        ;D0 = SIN(T) = T.
S19105:
                                       ;Another Small Angle Approximation.
            CMPI.W   #$3D00,D1         ;Is ABS(T) <= $3D7FFFFF?
            BHI.S    S19107            ;No ABS(T) >= $3D800000
            MOVEM.L  (A7)+,D2-D7/A6
            BRA      S19002

S19106:
            MOVE.L   D7,D0
                                       ;**** END   RANGE REDUCTION CODE ****
S19107:
                                       ;**** START CORDIC CODE ****
            ADDI.L   #$0F000000,D0     ;D0 = V(0) = S0 * T, S0 = 2**(30)
            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPFix(A6) ;Convert To Hex Long Word.

            MOVEQ.L  #$00,D3           ;D3 = i  = 0

            MOVE.L   #$26DD3B6A,D1     ;D1 = X(0) = S0 * P0, S0 = 2**(30)
            MOVE.L   D1,D4             ;D4 = XS

            MOVEQ.L  #$00,D2           ;D2 = Y(0) = 0
            MOVE.L   D2,D5             ;D5 = YS

            MOVEQ.L  #$1C,D6           ;Do M = 29 Loops (29 = #$1D).
                                       ;The DBF needs counter set to one
                                       ;less than actual Number of loops.
                                       ;We also capture the 30'th iteration
                                       ;without doing a whole extra loop.

            LEA.L    _ATanTable30,A0   ;A[i] = S0 * ATAN[2**(-i)]

S19004:     TST.L    D0                ;D0 = S0 * T
            BMI.S    S19005

            SUB.L    D5,D1             ;X(i) = X(i) - YS
            ADD.L    D4,D2             ;Y(i) = Y(i) + XS
            SUB.L    (A0)+,D0          ;V(i) = V(i) - A[i]

            ADDQ.L   #$01,D3           ;i = i + 1
            MOVE.L   D1,D4             ;XS = X(i)
            ASR.L    D3,D4             ;XS = XS*2**(-i)
            MOVE.L   D2,D5             ;YS = Y(i)
            ASR.L    D3,D5             ;YS = YS*2**(-i)

            DBF      D6,S19004         ;Loop Done?
                                       ;Yes, Get last correction terms.
            SUB.L    D5,D1             ;X(i) = X(i) - YS
            ADD.L    D4,D2             ;Y(i) = Y(i) + XS
            SUB.L    (A0)+,D0          ;V(i) = V(i) - A[i]

            BRA.S    S19006
            NOP
S19005:
            ADD.L    D5,D1             ;X(i) = X(i) + YS
            SUB.L    D4,D2             ;Y(i) = Y(i) - XS
            ADD.L    (A0)+,D0          ;V(i) = V(i) + A[i]

            ADDQ.L   #$01,D3           ;i = i + 1
            MOVE.L   D1,D4             ;XS = X(i)
            ASR.L    D3,D4             ;XS = XS*2**(-i)
            MOVE.L   D2,D5             ;YS = Y(i)
            ASR.L    D3,D5             ;YS = YS*2**(-i)

            DBF      D6,S19004         ;Loop Done?
                                       ;Yes, Get last correction terms.
            ADD.L    D5,D1             ;X(i) = X(i) + YS
            SUB.L    D4,D2             ;Y(i) = Y(i) - XS
            ADD.L    (A0)+,D0          ;V(i) = V(i) + A[i]


S19006:     MOVE.L   D2,D0             ;D2 = Y(M) = S0 * SIN(T)

            JSR      _LVOIEEESPFlt(A6) ;Convert to IEEESP Format.
            SUBI.L   #$0F000000,D0     ;Remove Scale Factor ( X(M)/S0 )
                                       ;**** END   CORDIC CODE ****
S19007:
            MOVEM.L  (A7)+,D2-D7/A6    ;Return D0 = SIN(T), IEEESP Format.
            RTS

****************** S0 * Atan[2**(-i)] Tables ****************************
            CNOP  0,4
            DC.L  $00000000
                                  ;Long Word Align This!!  It can cost
                                  ;about 10 usec if NOT Long Word Aligned.

_ATanTable30:                     ;S0 * Atan[2**(-i)] Table.
            DC.L     $3243F6A9    ;i = 0
            DC.L     $1DAC6705
            DC.L     $0FADBAFD
            DC.L     $07F56EA7
            DC.L     $03FEAB77
            DC.L     $01FFD55C    ;i = 5
            DC.L     $00FFFAAB
            DC.L     $007FFF55
            DC.L     $003FFFEB
            DC.L     $001FFFFD
            DC.L     $00100000    ;i = 10
            DC.L     $00080000
            DC.L     $00040000
            DC.L     $00020000
            DC.L     $00010000
            DC.L     $00008000    ;i = 15
            DC.L     $00004000
            DC.L     $00002000
            DC.L     $00001000
            DC.L     $00000800
            DC.L     $00000400    ;i = 20
            DC.L     $00000200
            DC.L     $00000100
            DC.L     $00000080
            DC.L     $00000040    ;i = 24
            DC.L     $00000020    ;i = 25
            DC.L     $00000010    ;i = 26
            DC.L     $00000008    ;i = 27
            DC.L     $00000004    ;i = 28
            DC.L     $00000002    ;i = 29
            DC.L     $00000001    ;i = 30
            DC.L     $00000000
                                  ;End Of Table.


	end
