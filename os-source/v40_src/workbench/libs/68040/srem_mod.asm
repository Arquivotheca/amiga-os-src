*
* $Id: srem_mod.asm,v 0.2 91/07/03 10:28:06 mks Exp $
*
* $Log:	srem_mod.asm,v $
* Revision 0.2  91/07/03  10:28:06  mks
* First pass at doing branch optimizations
* 
* Revision 0.1  91/06/26  08:59:57  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	srem_mod.sa 3.1 12/10/90
*
*      The entry point sMOD computes the floating point MOD of the
*      input values X and Y. The entry point sREM computes the floating
*      point (IEEE) REM of the input values X and Y.
*
*      INPUT
*      -----
*      Double-extended value Y is pointed to by address in register
*      A0. Double-extended value X is located in -12(A0). The values
*      of X and Y are both nonzero and finite; although either or both
*      of them can be denormalized. The special cases of zeros, NaNs,
*      and infinities are handled elsewhere.
*
*      OUTPUT
*      ------
*      FREM(X,Y) or FMOD(X,Y), depending on entry point.
*
*       ALGORITHM
*       ---------
*
*       Step 1.  Save and strip signs of X and Y: signX := sign(X),
*                signY := sign(Y), X := |X|, Y := |Y|,
*                signQ := signX EOR signY. Record whether MOD or REM
*                is requested.
*
*       Step 2.  Set L := expo(X)-expo(Y), k := 0, Q := 0.
*                If (L < 0) then
*                   R := X, go to Step 4.
*                else
*                   R := 2^(-L)X, j := L.
*                endif
*
*       Step 3.  Perform MOD(X,Y)
*            3.1 If R = Y, go to Step 9.
*            3.2 If R > Y, then { R := R - Y, Q := Q + 1}
*            3.3 If j = 0, go to Step 4.
*            3.4 k := k + 1, j := j - 1, Q := 2Q, R := 2R. Go to
*                Step 3.1.
*
*       Step 4.  At this point, R = X - QY = MOD(X,Y). Set
*                Last_Subtract := false (used in Step 7 below). If
*                MOD is requested, go to Step 6.
*
*       Step 5.  R = MOD(X,Y), but REM(X,Y) is requested.
*            5.1 If R < Y/2, then R = MOD(X,Y) = REM(X,Y). Go to
*                Step 6.
*            5.2 If R > Y/2, then { set Last_Subtract := true,
*                Q := Q + 1, Y := signY*Y }. Go to Step 6.
*            5.3 This is the tricky case of R = Y/2. If Q is odd,
*                then { Q := Q + 1, signX := -signX }.
*
*       Step 6.  R := signX*R.
*
*       Step 7.  If Last_Subtract = true, R := R - Y.
*
*       Step 8.  Return signQ, last 7 bits of Q, and R as required.
*
*       Step 9.  At this point, R = 2^(-j)*X - Q Y = Y. Thus,
*                X = 2^(j)*(Q+1)Y. set Q := 2^(j)*(Q+1),
*                R := 0. Return signQ, last 7 bits of Q, and R.
*
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

SREM_MOD    IDNT    2,1 Motorola 040 Floating Point Software Package

	section    8

	include	"fpsp.i"

Mod_Flag  equ	L_SCR3
SignY     equ	FP_SCR3+4
SignX     equ	FP_SCR3+8
SignQ     equ	FP_SCR3+12
Sc_Flag   equ	FP_SCR4

Y         equ	FP_SCR1
Y_Hi      equ	Y+4
Y_Lo      equ	Y+8

R         equ	FP_SCR2
R_Hi      equ	R+4
R_Lo      equ	R+8


Scale     DC.L	$00010000,$80000000,$00000000,$00000000

	xref	t_avoid_unsupp

        xdef        smod
smod:

   Move.L               #0,Mod_Flag(a6)
   BRA.B                Mod_Rem

        xdef        srem
srem:

   Move.L               #1,Mod_Flag(a6)

Mod_Rem:
*..Save sign of X and Y
   MoveM.L              D2-D7,-(A7)     ...save data registers
   Move.W               (A0),D3
   Move.W               D3,SignY(a6)
   AndI.L               #$00007FFF,D3   ...Y := |Y|

*
   Move.L               4(A0),D4
   Move.L               8(A0),D5        ...(D3,D4,D5) is |Y|

   Tst.L                D3
   BNE.B                Y_Normal

   Move.L               #$00003FFE,D3	...$3FFD + 1
   Tst.L                D4
   BNE.B                HiY_not0

HiY_0:
   Move.L               D5,D4
   CLR.L                D5
   SubI.L               #32,D3
   CLR.L                D6
   BFFFO                D4{0:32},D6
   LSL.L                D6,D4
   Sub.L                D6,D3           ...(D3,D4,D5) is normalized
*                                       ...with bias $7FFD
   BRA.B                Chk_X

HiY_not0:
   CLR.L                D6
   BFFFO                D4{0:32},D6
   Sub.L                D6,D3
   LSL.L                D6,D4
   Move.L               D5,D7           ...a copy of D5
   LSL.L                D6,D5
   Neg.L                D6
   AddI.L               #32,D6
   LSR.L                D6,D7
   Or.L                 D7,D4           ...(D3,D4,D5) normalized
*                                       ...with bias $7FFD
   BRA.B                Chk_X

Y_Normal:
   AddI.L               #$00003FFE,D3   ...(D3,D4,D5) normalized
*                                       ...with bias $7FFD

Chk_X:
   Move.W               -12(A0),D0
   Move.W               D0,SignX(a6)
   Move.W               SignY(a6),D1
   EOr.L                D0,D1
   AndI.L               #$00008000,D1
   Move.W               D1,SignQ(a6)	...sign(Q) obtained
   AndI.L               #$00007FFF,D0
   Move.L               -8(A0),D1
   Move.L               -4(A0),D2       ...(D0,D1,D2) is |X|
   Tst.L                D0
   BNE.B                X_Normal
   Move.L               #$00003FFE,D0
   Tst.L                D1
   BNE.B                HiX_not0

HiX_0:
   Move.L               D2,D1
   CLR.L                D2
   SubI.L               #32,D0
   CLR.L                D6
   BFFFO                D1{0:32},D6
   LSL.L                D6,D1
   Sub.L                D6,D0           ...(D0,D1,D2) is normalized
*                                       ...with bias $7FFD
   BRA.B                Init

HiX_not0:
   CLR.L                D6
   BFFFO                D1{0:32},D6
   Sub.L                D6,D0
   LSL.L                D6,D1
   Move.L               D2,D7           ...a copy of D2
   LSL.L                D6,D2
   Neg.L                D6
   AddI.L               #32,D6
   LSR.L                D6,D7
   Or.L                 D7,D1           ...(D0,D1,D2) normalized
*                                       ...with bias $7FFD
   BRA.B                Init

X_Normal:
   AddI.L               #$00003FFE,D0   ...(D0,D1,D2) normalized
*                                       ...with bias $7FFD

Init:
*
   Move.L               D3,L_SCR1(a6)   ...save biased expo(Y)
   move.l		d0,L_SCR2(a6)	;save d0
   Sub.L                D3,D0           ...L := expo(X)-expo(Y)
*   Move.L               D0,L            ...D0 is j
   CLR.L                D6              ...D6 := carry <- 0
   CLR.L                D3              ...D3 is Q
   MoveA.L              #0,A1           ...A1 is k; j+k=L, Q=0

*..(Carry,D1,D2) is R
   Tst.L                D0
   BGE.B                Mod_Loop

*..expo(X) < expo(Y). Thus X = mod(X,Y)
*
   move.l		L_SCR2(a6),d0	;restore d0
   BRA.s                Get_Mod

*..At this point  R = 2^(-L)X; Q = 0; k = 0; and  k+j = L


Mod_Loop:
   Tst.L                D6              ...test carry bit
   BGT.B                R_GT_Y

*..At this point carry = 0, R = (D1,D2), Y = (D4,D5)
   Cmp.L                D4,D1           ...compare hi(R) and hi(Y)
   BNE.B                R_NE_Y
   Cmp.L                D5,D2           ...compare lo(R) and lo(Y)
   BNE.B                R_NE_Y

*..At this point, R = Y
   BRA.W                Rem_is_0

R_NE_Y:
*..use the borrow of the previous compare
   BCS.B                R_LT_Y          ...borrow is set iff R < Y

R_GT_Y:
*..If Carry is set, then Y < (Carry,D1,D2) < 2Y. Otherwise, Carry = 0
*..and Y < (D1,D2) < 2Y. Either way, perform R - Y
   Sub.L                D5,D2           ...lo(R) - lo(Y)
   SubX.L               D4,D1           ...hi(R) - hi(Y)
   CLR.L                D6              ...clear carry
   AddQ.L               #1,D3           ...Q := Q + 1

R_LT_Y:
*..At this point, Carry=0, R < Y. R = 2^(k-L)X - QY; k+j = L; j >= 0.
   Tst.L                D0              ...see if j = 0.
   BEQ.B                PostLoop

   Add.L                D3,D3           ...Q := 2Q
   Add.L                D2,D2           ...lo(R) = 2lo(R)
   RoXL.L               #1,D1           ...hi(R) = 2hi(R) + carry
   SCS                  D6              ...set Carry if 2(R) overflows
   AddQ.L               #1,A1           ...k := k+1
   SubQ.L               #1,D0           ...j := j - 1
*..At this point, R=(Carry,D1,D2) = 2^(k-L)X - QY, j+k=L, j >= 0, R < 2Y.

   BRA.B                Mod_Loop

PostLoop:
*..k = L, j = 0, Carry = 0, R = (D1,D2) = X - QY, R < Y.

*..normalize R.
   Move.L               L_SCR1(a6),D0           ...new biased expo of R
   Tst.L                D1
   BNE.B                HiR_not0

HiR_0:
   Move.L               D2,D1
   CLR.L                D2
   SubI.L               #32,D0
   CLR.L                D6
   BFFFO                D1{0:32},D6
   LSL.L                D6,D1
   Sub.L                D6,D0           ...(D0,D1,D2) is normalized
*                                       ...with bias $7FFD
   BRA.B                Get_Mod

HiR_not0:
   CLR.L                D6
   BFFFO                D1{0:32},D6
   BMI.B                Get_Mod         ...already normalized
   Sub.L                D6,D0
   LSL.L                D6,D1
   Move.L               D2,D7           ...a copy of D2
   LSL.L                D6,D2
   Neg.L                D6
   AddI.L               #32,D6
   LSR.L                D6,D7
   Or.L                 D7,D1           ...(D0,D1,D2) normalized

*
Get_Mod:
   CmpI.L		#$000041FE,D0
   BGE.B		No_Scale
Do_Scale:
   Move.W		D0,R(a6)
   clr.w		R+2(a6)
   Move.L		D1,R_Hi(a6)
   Move.L		D2,R_Lo(a6)
   Move.L		L_SCR1(a6),D6
   Move.W		D6,Y(a6)
   clr.w		Y+2(a6)
   Move.L		D4,Y_Hi(a6)
   Move.L		D5,Y_Lo(a6)
   FMove.X		R(a6),fp0		...no exception
   Move.L		#1,Sc_Flag(a6)
   BRA.B		ModOrRem
No_Scale:
   Move.L		D1,R_Hi(a6)
   Move.L		D2,R_Lo(a6)
   SubI.L		#$3FFE,D0
   Move.W		D0,R(a6)
   clr.w		R+2(a6)
   Move.L		L_SCR1(a6),D6
   SubI.L		#$3FFE,D6
   Move.L		D6,L_SCR1(a6)
   FMove.X		R(a6),fp0
   Move.W		D6,Y(a6)
   Move.L		D4,Y_Hi(a6)
   Move.L		D5,Y_Lo(a6)
   Move.L		#0,Sc_Flag(a6)

*


ModOrRem:
   Move.L               Mod_Flag(a6),D6
   BEQ.B                Fix_Sign

   Move.L               L_SCR1(a6),D6           ...new biased expo(Y)
   SubQ.L               #1,D6           ...biased expo(Y/2)
   Cmp.L                D6,D0
   BLT.B                Fix_Sign
   BGT.B                Last_Sub

   Cmp.L                D4,D1
   BNE.B                Not_EQ
   Cmp.L                D5,D2
   BNE.B                Not_EQ
   BRA.s                Tie_Case

Not_EQ:
   BCS.B                Fix_Sign

Last_Sub:
*
   FSub.X		Y(a6),fp0		...no exceptions
   AddQ.L               #1,D3           ...Q := Q + 1

*

Fix_Sign:
*..Get sign of X
   Move.W               SignX(a6),D6
   BGE.B		Get_Q
   FNeg.X		fp0

*..Get Q
*
Get_Q:
   clr.l		d6
   Move.W               SignQ(a6),D6        ...D6 is sign(Q)
   Move.L               #8,D7
   LSR.L                D7,D6
   AndI.L               #$0000007F,D3   ...7 bits of Q
   Or.L                 D6,D3           ...sign and bits of Q
   Swap                 D3
   FMove.L              fpsr,D6
   AndI.L               #$FF00FFFF,D6
   Or.L                 D3,D6
   FMove.L              D6,fpsr         ...put Q in fpsr

*
Restore:
   MoveM.L              (A7)+,D2-D7
   FMove.L              USER_FPCR(a6),fpcr
   Move.L               Sc_Flag(a6),D0
   BEQ.B                Finish
   FMul.X		Scale(pc),fp0	...may cause underflow
   bra			t_avoid_unsupp	;check for denorm as a
*					;result of the scaling

Finish:
	fmove.x		fp0,fp0		;capture exceptions & round
	rts

Rem_is_0:
*..R = 2^(-j)X - Q Y = Y, thus R = 0 and quotient = 2^j (Q+1)
   AddQ.L               #1,D3
   CmpI.L               #8,D0           ...D0 is j
   BGE.B                Q_Big

   LSL.L                D0,D3
   BRA.B                Set_R_0

Q_Big:
   CLR.L                D3

Set_R_0:
   FMove.S		#:00000000,fp0
   Move.L		#0,Sc_Flag(a6)
   BRA.s                Fix_Sign

Tie_Case:
*..Check parity of Q
   Move.L               D3,D6
   AndI.L               #$00000001,D6
   Tst.L                D6
   BEq.s                Fix_Sign	...Q is even

*..Q is odd, Q := Q + 1, signX := -signX
   AddQ.L               #1,D3
   Move.W               SignX(a6),D6
   EOrI.L               #$00008000,D6
   Move.W               D6,SignX(a6)
   BRA.W                Fix_Sign

   End
