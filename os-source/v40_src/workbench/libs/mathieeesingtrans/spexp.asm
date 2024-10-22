
******* mathieeesingtrans.library/IEEESPExp ********************************
*
*   NAME
*	IEEESPExp -- compute the exponential of e
*
*   SYNOPSIS
*	  x   = IEEESPExp(  y  );
*	d0	          d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute e^y in IEEE single precision
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
*	IEEESPLog()
**************************************************************************/
********************* SP Cordic Exponential Routine *********************
*** INPUTS:  D0 = T, Argument in IEEESP Format.
***
*** OUTPUTS: D0 = Exp(T),  IEEESP Format.
***
***
*** BUGS:    Program does not handle the so-called 'denormalized' numbers
***          when T is less than -87.33654. The program in this case just
***          exits with Exp(T) = 0.0. 
*** 
*** NOTES:   The Range Reduction algorithm is from Cody and Waite's book
***          'Software Manual for the Elementary Functions'.  The range
***          reduction is accessed only when Abs(T) > 1.0. The range
***          reduction basically converts Exp(T) to 2**(N) * Exp(G) where
***          Abs(G) <= Log(2)/2 (= 0.3466).  The CORDIC is accessed
***          only when Abs(T) or Abs(G) is <= 1.0.  Of course Abs(G) is
***          always <= 1.0.  2**(N) where N is an integer is very easy to
***          compute but we need to use IEEESPFix to get 'N'.  When Abs(T)
***          is <= 1 the range reduction code is bypassed and we go directly
***          to the CORDIC.  When Abs(T) or Abs(G) is < 4.321289E-02 the
***          CORDIC losses accuracy and I revert to using various order
***          Maclaurin expansions of the Exp(T) to achieve 68882 accuracy
***          and overall high speed.
***
***
*** By:      Al Aburto ('ala' on BIX)
***          3016 Forrester Court
***          San Diego, Calif, 92123
***          (619) 553-1495 (work)
***          (619) 278-8521 (home)
***
*** Original Version:  06 Apr 1990
*** Revision:          08 Apr 1990
***                    Optimized the various thresholds used in the
***                    program to eliminate the 4'th order Exp() expansion
***                    employed in the version of 06 Apr 1990.  Progarm
***                    now uses a faster 3'rd order Maclaurin expansion
***                    of Exp(). Overall accuracy and speed exceeds that
***                    of the current IEEESPExp() library as indicated
***                    in the timing results below.
***                                                     
*** Program Timing (Microseconds(usec), 68020 with 32-bit RAM at 14.32 MHz).
*** The 68882 runs were at 14.32 MHz using the IEEESP Library. Runs also
*** done for negative values of all the numbers below and others with
*** similar accuracy.
***
***                                        *** Software Floating-Point ****
***           T            68882,14.32MHz   SWFP_SPExp(T)     IEEESPExp(T)
***                       (usec)           (usec)           (usec)
*** 512.0       $44000000  44.6 $7F800000    7.4 $7F800000   81.3 $7F7FC99E
*** 128.0       $43000000  44.2 $7f800000    7.4 $7F800000   81.0 $7F7FC99E
*** 100.0       $42C80000  36.0 $7F800000    7.1 $7F800000   83.2 $7F7FC99E
***  90.0       $42B40000  43.0 $7F800000    7.1 $7F800000   82.9 $7F7FC99E
***  88.72284   $42B17218  35.6 $7F800000    7.6 $7F800000   81.2 $7F7FC99E
***  88.72283   $42B17217  42.8 $7F7FFF84  243.5 $7F7FFF84  529.1 $7F7FFF4D
***  80.0       $42A00000  35.6 $792ABBCE  336.3 $792ABBCE  530.1 $792ABBD1
***  60.0       $42700000  43.6 $6ABCEDE5  336.8 $6ABCEDE5  529.8 $6ABCEDC6
***  50.0       $42480000  35.8 $638C881F  336.1 $638C881F  532.3 $638C8814
***  40.0       $42200000  43.8 $5C51106A  334.8 $5C51106A  529.2 $5C51106B
***  32.0       $42000000  35.8 $568FA1FE  338.6 $568FA1FE  529.9 $568FA1FA
***  20.0       $41A00000  42.8 $4DE75844  335.1 $4DE75844  529.3 $4DE75846
***  10.0       $41200000  35.8 $46AC14EE  337.4 $46AC14EE  531.9 $46AC14EF
***   5.5       $40B00000  43.0 $4374B122  335.6 $4374B123  529.0 $4374B120
***   5.3       $40A9999A  43.2 $4348563C  337.8 $4348563C  528.5 $4348563C
***   5.1       $40A33333  35.2 $4324059B  336.6 $4324059B  531.4 $4324059C
***   5.0       $40A00000  35.4 $431469C5  336.4 $431469C5  531.6 $431469C5
***   4.9       $409CCCCD  35.2 $43064A30  337.4 $43064A30  532.0 $43064A30
***   4.87      $409BD70A  35.4 $43025227  371.4 $43025227  531.6 $43025227
***   4.86      $409B851F  35.6 $43010633  372.6 $43010633  530.9 $43010633
***   4.853     $409B4BC7  35.0 $43001FCC  298.0 $43001FCC  531.6 $43001FCA
***   4.825     $409B47AE  35.0 $43000F65  294.0 $43000F65  532.7 $43000F64
***   4.8523    $409B460B  35.0 $430008D8  293.8 $430008D8  532.1 $430008D6
***   4.8522    $409B4539  35.0 $43000590  234.8 $43000590  533.0 $4300058E
***   4.8521    $409B4467  35.0 $43000248  234.7 $43000248  530.3 $43000246
***   4.85209   $409B4452  35.4 $430001F4  234.5 $430001F4  530.1 $430001F3
***   4.8520303 $409B43D5  35.2 $43000000  214.4 $43000000  505.9 $42FFFFFE
***   3.14      $4048F5C3  43.8 $41B8D4B9  336.3 $41B8D4B9  531.0 $41B8D4BB
***   2.5       $40200000  43.8 $4142EB7F  337.8 $4142EB7F  529.1 $4142EB7E
***   1.5709    $3FC91340  35.2 $4099F384  336.2 $4099F384  533.4 $4099F383
***   1.1       $3F8CCCCD  43.0 $40404442  337.2 $40404442  529.5 $40404441
***   1.0       $3F800000  34.2 $402DF854  141.5 $402DF854  532.0 $402DF854
***   0.9       $3F666666  34.5 $401D6A23  141.7 $401D6A23  532.2 $401D6A22
***   0.8       $3F4CCCCD  34.5 $400E6F43  140.8 $400E6F43  532.3 $400E6F42
***   0.7       $3F333333  34.5 $4000E153  142.0 $4000E153  533.4 $4000E153
***   0.5       $3F000000  42.0 $3FD3094C  141.6 $3FD3094C  508.6 $3FD3094B
***   0.4       $3ECCCCCD  41.6 $3FBEF41D  142.3 $3FBEF41D  509.4 $3FBEF41D
***   0.25      $3E800000  33.2 $3FA45AF2  142.1 $3FA45AF2  510.5 $3FA45AF1
***   0.125     $3E000000  33.0 $3F910B02  141.4 $3F910B02  510.4 $3F910B03
***   0.1       $3DCCCCCD  33.7 $3F8D763E  142.0 $3F8D763E  512.2 $3F8D763D
***   0.0625    $3D800000  33.1 $3F88415B  142.8 $3F88415B  510.7 $3F88415A
***             $3D310000  33.1 $3F85A70A  143.4 $3F85A70A  510.5 $3F85A70A
***             $3D30FFFF  33.2 $3F85A70A  163.4 $3F85A709  510.9 $3F85A70A
***  2**(-005)  $3D000000  32.9 $3F84102B  161.1 $3F84102B  511.2 $3F84102B
***  2**(-007)  $3C000000  32.9 $3F810101  161.6 $3F810101  510.9 $3F810101
***             $3BD10000  33.0 $3F80D1AB  163.2 $3F80D1AB  512.2 $3F80D1AB
***             $3BD0FFFF  33.6 $3F80D1AB   83.8 $3F80D1AB  512.3 $3F80D1AB
***  2**(-008)  $3B800000  33.3 $3F808040   81.8 $3F808040  511.0 $3F80803F
***  2**(-009)  $3B000000  33.4 $3F804010   81.8 $3F804010  511.4 $3F804010
***  2**(-011)  $3A000000  33.7 $3F801001   81.6 $3F801001  511.2 $3F801001
***  2**(-012)  $39800000  33.3 $3F800800   81.7 $3F800800  510.4 $3F800800
***             $39780000  33.3 $3F8007C0   83.2 $3F8007C0  512.7 $3F8007C0
***             $3977FFFF  33.4 $3F8007C0   24.5 $3F8007C0  512.9 $3F8007C0
***  2**(-013)  $39000000  33.8 $3F800400   24.5 $3F800400  510.8 $3F800400
***  2**(-018)  $36800000  33.7 $3F800020   24.2 $3F800020  510.8 $3F800020
***  2**(-024)  $33800000  33.4 $3F800001   24.5 $3F800001  510.7 $3F800000
***             $337FFFFF  33.7 $3F000000    3.1 $3F800000  512.7 $3F800000
***  2**(-026)  $32800000  33.2 $3F800000    3.6 $3F800000  511.3 $3F800000
***  2**(-032)  $2F800000  33.2 $3F800000    3.0 $3F800000  497.1 $3F7FFFFE
***  2**(-060)  $21800000  33.2 $3F800000    3.1 $3F800000  496.9 $3F7FFFFE
***  2**(-090)  $12800000  15.9 $3F800000    3.1 $3F800000  497.2 $3F7FFFFE
***  2**(-120)  $03800000  15.9 $3F800000    3.4 $3F800000  497.2 $3F7FFFFE
***  2**(-126)  $00800000  16.4 $3F800000    3.6 $3F800000  497.3 $3F7FFFFE
*** -2**(-126)  $80800000  22.4 $3F800000    3.1 $3F800000  541.0 $3F800000
*** -2**(-120)  $83800000  22.7 $3F800000    3.4 $3F800000  540.4 $3F800000
***************************************************************************
	include "xref.i"

	xdef	MIIEEESPExp
	xdef	_rexp
_rexp:
	move.l	4(sp),d0
MIIEEESPExp:
SWFP_SPExp:
            MOVE.L   D0,D1             ;Test For Small values 'T'.
            BCLR.L   #$001F,D1         ;ABS(T)
            SWAP     D1
            CMPI.W   #$337F,D1         ;Is ABS(T) <= $337FFFFF?
            BHI.S    S18001            ;No ABS(T) >= $33800000
            MOVE.L   #$3F800000,D0     ;Yes.
            RTS                        ;D0 = Exp(T) = 1.0 .
S18001:
            CMPI.W   #$3977,D1         ;Is ABS(T) <= $3977FFFF?
            BHI.S    S18003            ;No ABS(T) >= $39780000
S18002:
            MOVE.L   A6,-(A7)          ;Yes.
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   #$3F800000,D1
            JSR      _LVOIEEESPAdd(A6) ;1.0 + T
            MOVEA.L  (A7)+,A6
            RTS                        ;D0 = Exp(T) = 1.0 + T
S18003:
            CMPI.W   #$3BD0,D1         ;Is ABS(T) <= $3BD0FFFF?
            BHI.S    S18004            ;No ABS(T) >= $3BD10000
                                       ;Yes.
            BRA      SUBEXP2
S18004:
            CMPI.W   #$3D30,D1         ;Is ABS(T) <= $3D30FFFF?
            BHI.S    S18005            ;No ABS(T) >= $3D310000
                                       ;Yes.
            BRA      SUBEXP3

S18005:
            CMPI.L   #$C2AEAC4F,D0     ;D0 <= $C2AEAC4F ?
            BLS.S    S18006
                                       ;Denormalized numbers should be
                                       ;handled with a routine here.
            MOVEQ.L  #$00,D0
            RTS
S18006:
            CMPI.L   #$42B17217,D0     ;D0 <= $42B17217 ?
            BLE.S    S18007
            MOVE.L   #$7F800000,D0     ;Just like the 68882.
            RTS
S18007:
            MOVEM.L  D2-D7/A6,-(A7)    ;$3D300000 <= T <  $42B17317, or
                                       ;$C2AEAC4F <  T <= $BD300000.

S18101:                                ;**** START RANGE REDUCTION CODE ****
            MOVE.L   D0,D7             ;Save T
            BCLR.L   #$001F,D0         ;Form ABS(T).

            CMPI.L   #$3F800000,D0     ;ABS(T) <= 1.0 ?
            BLS      S18108            ;Yes --- Skip Range Reduction Code.
                                       ;No  --- Do Range Reduction Then.
S18102:
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D7,D0             ;D0 = T
            MOVE.L   #$3FB8AA3B,D1     ;(1/ln(2))
            JSR      _LVOIEEESPMul(A6) ;Q  = T * (1/ln(2)
            MOVE.L   #$3F000000,D1     ;0.5
            JSR      _LVOIEEESPAdd(A6) ;Q + 0.5
            JSR      _LVOIEEESPFloor(A6)
            MOVE.L   D0,D3             ;D3 = XN = FINTRZ(Q + 0.5).
S18103:
            MOVE.L   #$BF318000,D1     ;-C1
            JSR      _LVOIEEESPMul(A6) ;-XN * C1
            MOVE.L   D7,D1             ;T
            JSR      _LVOIEEESPAdd(A6) ;T - XN * C1
            MOVE.L   D0,D4

            MOVE.L   D3,D0             ;XN
            MOVE.L   #$395E8083,D1     ;-C2
            JSR      _LVOIEEESPMul(A6) ;-XN * C2
            MOVE.L   D4,D1             ;T - XN * C1
            JSR      _LVOIEEESPAdd(A6) ;D0 = G = (T - XN * C1) - XN * C2

                                       ;**** END   RANGE REDUCTION CODE ****

                                       ;**** TEST FOR SMALL VALUES OF G ****
            MOVE.L   D3,-(A7)          ;Save XN

            MOVE.L   D0,D1             ;G
            BCLR.L   #$001F,D1
            SWAP     D1
            CMPI.W   #$337F,D1         ;Is ABS(G) <= $337FFFFF?
            BHI.S    S18104            ;No ABS(G) >= $33800000
            MOVE.L   #$3F800000,D0     ;Yes.
            BRA      S18011            ;D0 = Exp(G) = 1.0 .
S18104:
            CMPI.W   #$3977,D1         ;Is ABS(G) <= $3977FFFF?
            BHI.S    S18106            ;No ABS(G) >= $39780000
S18105:                                ;Yes.
            MOVE.L   #$3F800000,D1
            JSR      _LVOIEEESPAdd(A6) ;1.0 + G
            BRA      S18011            ;D0 = Exp(G) = 1.0 + G
S18106:
            CMPI.W   #$3BD0,D1         ;Is ABS(G) <= $3BD0FFFF?
            BHI.S    S18107            ;No ABS(G) >= $3BD10000
                                       ;Yes.
            BSR      SUBEXP2
            BRA      S18011
S18107:
            CMPI.W   #$3D30,D1         ;Is ABS(G) <= $3D30FFFF?
            BHI.S    S18109            ;No ABS(G) >= $3D310000 --- Do CORDIC
                                       ;Yes.
            BSR      SUBEXP3
            BRA      S18011

S18108:                                ;No Range Reduction Performed.
            MOVEQ.L  #$00,D3           ;Set XN = 0.0
            MOVE.L   D7,D0             ;Restore D0 = T = G (here).

            MOVE.L   D3,-(A7)          ;Save XN

S18109:
                                       ;**** START CORDIC CODE ****

            ADDI.L   #$0E800000,D0     ;D0 = V(0) = S0 * G, S0 = 2**(29)
            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPFix(A6) ;Convert To Hex Long Word.

            MOVEQ.L  #$01,D3           ;D3 = i  = 1

            MOVE.L   #$26907A20,D1     ;D1 = X(0) = S0 * P0, S0 = 2**(29)
                                       ;P0 = 1.205136358

            MOVE.L   D1,D4             ;D4 = XS
            ASR.L    D3,D4

            MOVEQ.L  #$00,D2           ;D2 = Y(0) = 0
            MOVE.L   D2,D5             ;D5 = YS

            MOVEQ.L  #$1B,D6           ;Do M-1 = 28 Loops (28 = #$1C).

            LEA.L    _ATanhTable29,A0  ;A[i] = S0 * Atanh[2**(-i)]

S18008:     TST.L    D0                ;D0 = S0 * T
            BMI.S    S18009

            SUB.L    D5,D1             ;X(i) = X(i) - YS
            SUB.L    D4,D2             ;Y(i) = Y(i) - XS
            SUB.L    (A0)+,D0          ;V(i) = V(i) - A[i]

            ADDQ.L   #$01,D3           ;i = i + 1
            MOVE.L   D1,D4             ;XS = X(i)
            ASR.L    D3,D4             ;XS = XS*2**(-i)
            MOVE.L   D2,D5             ;YS = Y(i)
            ASR.L    D3,D5             ;YS = YS*2**(-i)

            DBF      D6,S18008         ;Loop Done?
                                       ;Yes, Get last correction terms.
            SUB.L    D5,D1             ;X(i) = X(i) - YS
            SUB.L    D4,D2             ;Y(i) = Y(i) - XS
            SUB.L    (A0)+,D0          ;V(i) = V(i) - A[i]

            BRA.S    S18010
S18009:
            ADD.L    D5,D1             ;X(i) = X(i) + YS
            ADD.L    D4,D2             ;Y(i) = Y(i) + XS
            ADD.L    (A0)+,D0          ;V(i) = V(i) + A[i]

            ADDQ.L   #$01,D3           ;i = i + 1
            MOVE.L   D1,D4             ;XS = X(i)
            ASR.L    D3,D4             ;XS = XS*2**(-i)
            MOVE.L   D2,D5             ;YS = Y(i)
            ASR.L    D3,D5             ;YS = YS*2**(-i)

            DBF      D6,S18008         ;Loop Done?
                                       ;Yes, Get last correction terms.
            ADD.L    D5,D1             ;X(i) = X(i) + YS
            ADD.L    D4,D2             ;Y(i) = Y(i) + XS
            ADD.L    (A0)+,D0          ;V(i) = V(i) + A[i]
S18010:
                                       ;D1 = X(M) =  S0 * Cosh(G)
                                       ;D2 = Y(M) = -S0 * Sinh(G)
            MOVE.L   D1,D0
            SUB.L    D2,D0             ;D0 = S0 * Exp(G)
            JSR      _LVOIEEESPFlt(A6) ;Convert to IEEESP Format.
            SUBI.L   #$0E800000,D0     ;Remove Scale Factor
                                       ;**** END CORDIC CODE ****

S18011:                                ;**** FORM EXP(T) ********
            MOVE.L   D0,D3             ;D3 = Exp(G)

            MOVEQ.L  #$00,D4
            MOVE.L   (A7)+,D0          ;D0 = XN (A Floating-Point Integer)
            BEQ.S    S18012            ;Skip if XN = 0!

            JSR      _LVOIEEESPFix(A6) ;D0 = (LONG)XN
            LSL.L    #$07,D0
            SWAP     D0
            CLR.W    D0                ;Need this when XN < 0.0 !
            MOVE.L   D0,D4             ;2**(XN)
S18012:
            MOVE.L   D3,D0             ;D0 = Exp(G)
            ADD.L    D4,D0             ;D0 = Exp(T) = Exp(G) * 2**(XN)
S18013:
            MOVEM.L  (A7)+,D2-D7/A6    ;Return D0 = Exp(T), IEEESP Format.
            RTS

******************** Second Order Exp(U) Approximation ******************
SUBEXP2:    MOVEM.L  D2/A6,-(A7)
            MOVE.L   D0,D2             ;U
            MOVE.L   D0,D1
            MOVEA.L  _MathIeeeSingBasBase,A6
            JSR      _LVOIEEESPMul(A6) ;U * U
            SUBI.L   #$00800000,D0     ;U * U / 2
            MOVE.L   D2,D1
            JSR      _LVOIEEESPAdd(A6) ;U + U * U / 2
            MOVE.L   #$3F800000,D1
            JSR      _LVOIEEESPAdd(A6) ;V = 1.0 + U + U * U / 2
            MOVEM.L  (A7)+,D2/A6
            RTS                        ;D0 = V = Exp(U)


******************** Third  Order Exp(U) Approximation ******************
SUBEXP3:    MOVEM.L  D2/D3/A6,-(A7)
            MOVEA.L  _MathIeeeSingBasBase,A6
            MOVE.L   D0,D2             ;U
            MOVE.L   D0,D1             ;U
            JSR      _LVOIEEESPMul(A6) ;U**2
            MOVE.L   D0,D3             ;U**2
            MOVE.L   D2,D1             ;U
            JSR      _LVOIEEESPMul(A6) ;U**3
            MOVE.L   #$3E2AAAAB,D1     ;C = 1/6
            JSR      _LVOIEEESPMul(A6) ;C * U**3
            SUBI.L   #$00800000,D3     ;B * U**2, B = 1/2
            MOVE.L   D3,D1             ;B * U**2
            JSR      _LVOIEEESPAdd(A6) ;B * U**2 + C * U**3
            MOVE.L   D2,D1             ;U
            JSR      _LVOIEEESPAdd(A6) ;U + B * U**2 + C * U**3
            MOVE.L   #$3F800000,D1     ;1
            JSR      _LVOIEEESPAdd(A6) ;V = 1 + U + B * U**2 + C * U**3
            MOVEM.L  (A7)+,D2/D3/A6
            RTS                        ;D0 = V = Exp(U)

****************** S0 * Atanh[2**(-i)] Table ****************************
            CNOP  0,4
            DC.L  $00000000
                                  ;Long Word Align This!!  It can cost
                                  ;about 10 usec if NOT Long Word Aligned.

                                  ;M = 29 and S0 = 2**M
_ATanhTable29:                    ;Q(i) = S0 * ArcTanh(2**(-i))
            DC.L     $1193EA7B    ;i = 01
            DC.L     $082C577D
            DC.L     $04056247
            DC.L     $0200AB11    ;i = 04
            DC.L     $01001559
            DC.L     $008002AB
            DC.L     $00400055
            DC.L     $0020000B
            DC.L     $00100001    ;i = 09
            DC.L     $00080000
            DC.L     $00040000
            DC.L     $00020000
            DC.L     $00010000
            DC.L     $00008000    ;i = 14
            DC.L     $00004000
            DC.L     $00002000
            DC.L     $00001000
            DC.L     $00000800
            DC.L     $00000400    ;i = 19
            DC.L     $00000200
            DC.L     $00000100
            DC.L     $00000080
            DC.L     $00000040    ;i = 23
            DC.L     $00000020    ;i = 24
            DC.L     $00000010    ;i = 25
            DC.L     $00000008    ;i = 26
            DC.L     $00000004    ;i = 27
            DC.L     $00000002    ;i = 28
            DC.L     $00000001    ;i = 29
            DC.L     $00000000
                                  ;End Of Table.
*********** End of SWFP_SPExp.asm ***************************************

	end
