**
**	$Id: cdecc.asm,v 1.1 93/02/11 13:37:16 jerryh Exp Locker: jerryh $
**
**	cd.device Reed-Solomon Error Correction Code
**
**	Downcoded (based on code provided by Allan Havemose)
**
**	See C code for documents, and more details
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

		INCLUDE	"exec/types.i"

** Exports
		XDEF	_Decode_CDECC
		XDEF	PerformECC
		XDEF	_Exp
		XDEF    _Log
		XDEF	_QIndex

** Equates

TRY_MAX		EQU	5

ECC_OK			EQU	0
ECC_ERROR		EQU	1		;unable to correct all errors
ECC_HEADER_ERROR	EQU	2		;uncorrectable error in CDROM header!

P_ECC_PROB		EQU	$100		;P-ECC not able to correct data */
Q_ECC_PROB		EQU	$100		;Q-ECC not able to correct data */

		SECTION	cdecc

********************************************************************************
*                                                                              *
*   PerformECC - Perform error correction on CD-ROM data                       *
*                                                                              *
*        in:                                                                   *
*                        a0 = pointer to 4k buffer                             *
*                                                                              *
*               ior equr a4 = pointer to IORequest  (you should not need)      *
*               hb  equr a5 = hardware base pointer (you should not need)      *
*               db  equr a6 = pointer to global data structure                 *
*                                                                              *
*       out:                                                                   *
*             D0.l      = 0: sector is now valid, else still invalid           *
*             Z flag    = status of d0.l                                       *
*                                                                              *
*     NOTES: d2-d7/a2-a6 should be preserved                                   *
*                                                                              *
********************************************************************************
_Decode_CDECC:
PerformECC:
		movem.l	d2-d7/a2-a6,-(sp)	;yes, I'll be using all of these!

	;-- do even bytes first

		lea	12(a0),a6
		bsr.s	Do_ECC
		move.l	d0,-(sp)		;cache result

	;-- do odd bytes

		addq.l	#1,a6	
		bsr.s	Do_ECC
		or.l	(sp)+,d0		;return(Even_Stat | Odd_Stat
						;Z flag set
		movem.l	(sp)+,d2-d7/a2-a6
		rts



*************************************************************************
*                                                                        
*   DoECC - Check odd/even P&Q                                           
*                                                                        
*        in:                                                             
*                        a6 = pointer to buffer                          
*                                                                        
*     NOTES:
*                                                                        
*
*	A0		Exp[0]	(static)
*	A1		Log[0] or QIndex[0]
*	A2		QErr array pointer (static)
*	A3		PErr array pointer (incrementing)
*	A4		QErr array pointer (incrementing)
*	A5		Pointer to Locales and PErr array (static)
*	A6		Pointer to Buffer
*
*	D5		PSTAT
*	D6		QSTAT
*
*	D0,D1,D2,D3,D4,D7	temp stuff
*
*************************************************************************

PSTAT			EQUR	D5
QSTAT			EQUR	D6

PERR_ARRAY_SIZE		EQU	44
QERR_ARRAY_SIZE		EQU	28

P_NO_LOOP		EQU	42		;loop 43x
Q_NO_LOOP		EQU	25		;loop 26x

** Structures

    STRUCTURE Do_ECCLocals,0
	STRUCT	PErr,PERR_ARRAY_SIZE		;PError array
	STRUCT	QErr,QERR_ARRAY_SIZE		;QError array
	UWORD	TryMax				;loop 5x
	UWORD	P_NO				;loop 43x
	UWORD	Q_NO				;loop 26x
	ALIGNLONG
	LABEL	Do_ECCLocals_SIZEOF


Do_ECC:
		lea	-Do_ECCLocals_SIZEOF(sp),sp
		move.l	sp,a5

	;-- initialize P&Q error arrays (fill with 1's)

	IFNE	((QErr-PErr)-PERR_ARRAY_SIZE)
	FAIL	"QErr array does not follow PErr; recode!"
	ENDC

	IFNE	PErr
	FAIL	"PErr not first in structure; recode!"
	ENDC

	IFGT	((P_NO_LOOP*2)-255)
	FAIL	"P Loop byte size test invalid; recode!"
	ENDC


*#ifdef	CACHE_CALC
*	ULONG		*lptr;		/* pointer to ULONG */
*
*	lptr = (ULONG*)PErr;
*	for(Try=0;Try<11;Try++)		// 44 bytes in PErr
*	  lptr[Try] = 0xFFFFFFFF;
*
*	lptr = (ULONG*)QErr;		// 28 bytes in QErr
*	for(Try=0;Try<7;Try++)
*	  lptr[Try] = 0xFFFFFFFF;
*#endif
*
 
		moveq	#-1,d1

		moveq	#(((PERR_ARRAY_SIZE+QERR_ARRAY_SIZE)/4)-1),d0			;11*4 longwords

		move.l	a5,a0		;set PErr & QErr
ecc_initpqarrays:
		move.l	d1,(a0)+
		dbf	d0,ecc_initpqarrays

		lea	QErr(a5),a2		;quick lookup for QErr 
		lea	_Exp(pc),a0
		lea	_Log(pc),a1		;pre-loaded

*	for( Try = 1; Try <= Try_Max; Try++ )
*	{

		move.w	#TRY_MAX,TryMax(a5)	;loop 5x

*		Q_Stat = P_Stat = 0;
Do_ECCLoop:
		moveq	#00,PSTAT
		moveq	#00,QSTAT

*		for( P_NO = 0; P_NO <= 42; P_NO++ )
*		{

		clr.w	P_NO(a5)		;P_NO = 0;

		move.l	a5,a3			;PErr[0]

*#ifdef	CACHE_CALC
*		   if (PErr[P_NO])
*#endif
*		   {
check_Ploop:

		tst.b	(a3)+
		beq	check_Ploope

*			P_Stat = P_Check( StartAddr,P_NO, P_Stat,PErr,QErr);

	;-- so we now have
	;
	;-- A3 = PErr[P_NO+1] (P_Check just needs to get at -1(a3)
	;
	;-- A2 = QErr[0]
	;-- A5 = PErr[0]
	;-- A6 = StartAddr
	;
	; Do P_Check() inline to avoid many stack push-pulls


*	Syn0 = Syn1 = 0;
*
*	Index = 2*P_NO;
*	for( Row = 0; Row <= 25; Row++ )
*	{
*		elem  = Buffer[Index];	// Index = 2* ( 43 * Row + P_NO ); at this point
*		if (elem)
*		{
*		   Syn0 ^= elem;
*		   Pos   = (Exp[elem]+ 25-Row);
*		   if (Pos >= 255 ) Pos -= 255;
*		   Syn1 ^= Log[Pos];
*		}
*		Index += 2*43;
*	}

		moveq	#Q_NO_LOOP,d7		;loop

		moveq	#00,d3			; Syn1<<16 & Syn0
		move.w	#255,d2			;max Pos

		move.w	P_NO(a5),d0
	;; we already increment P_NO * 2
	;;	add.w	d0,d0			;Index = P_NO*2

		moveq	#00,d1			;elem (clear upper 24 bits)
pcheck_rows:
		move.b	0(a6,d0.w),d1		;elem = Buffer[Index]
		beq.s	pcheck_1		;if(elem)

		eor.b	d1,d3			;Syn0 ^= elem
		moveq	#00,d4			; Pos (pre clear)
		move.b	0(a0,d1.w),d4		;Pos = Exp[elem]...
		add.w	d7,d4			;+(25-Row) 25,24,23...

		cmp.w	d2,d4			;Pos < Max?
		blt.s	ppos_range1

		sub.w	d2,d4			;Pos -= 255;

ppos_range1:
		swap	d3
		move.b	0(a1,d4.w),d1		;byte 1 already cleared
		eor.b	d1,d3			;Syn1 ^= Log[Pos]
		swap	d3

pcheck_1:
		add.w	#(2*43),d0		;Index += 2*43 (next row)
		dbf	d7,pcheck_rows


*		   }
*		}
*

*	if( ( Syn0 == 0 ) && ( Syn1 == 0 ) )	// no error
*	{
*#ifdef	CACHE_CALC
*		PErr[P_NO] = 0;			// Now OK
*#endif
*		return( P_Stat );
*	}
*

		tst.l	d3			;Syn0 & Syn1 all in one
		beq.s	noP_error		;assumed average case


*	if( ( Syn0 == 0 ) || ( Syn1 == 0 ) )	// multi byte error
*	{
*		// do not need to modify PErr
*		return( (UWORD)(P_Stat + P_ECC_PROB));
*	}
*
		tst.w	d3			;Syn0
		beq.s	multiP_error
		swap	d3
		tst.w	d3			;Syn1
		beq.s	multiP_error


*	// should be able to correct this pattern
*
*	Pos = Exp[Syn0] - Exp[Syn1] + 25;		// remember: code is backwards
*	if (Pos >= 255) Pos -= 255;

		moveq	#00,d4
	;	moveq	#00,d1			;alread clear
		move.b	0(a0,d3.w),d1		;Exp[Syn1]
		swap	d3
		move.b	0(a0,d3.w),d4		;Exp[Syn0]
 		sub.w	d1,d4
 		add.w	#25,d4
 
 		cmp.w	d2,d4
 		blt.s	ppos_range2		;signed compare
 
		sub.w	d2,d4

ppos_range2:		

*	if( ( Pos > 25 ) || ( Pos < 0 ) ) {
*		// do not need to modify PErr
*		return( (UWORD)(P_Stat + P_ECC_PROB));	// ups, cannot handle anyway
*	}

		tst.w	d4
		bmi.s	multiP_error
		cmp.w	#25,d4
		bgt.s	multiP_error

*	Index  = 2 * P_NO + 2*43 * Pos;
*

		move.w	d4,d0
		mulu.w	#(2*43),d0
		add.w	P_NO(a5),d0		;+ (P_NO*2)

*	Buffer[ Index ] ^= Syn0;
*
		eor.b	d3,0(a6,d0.w)

*#ifdef	CACHE_CALC
*	QErr[ Calc_QCodeNum(Index) ] = 1; // also check this Q-code . PErr is already 1
*#endif
		moveq	#01,d1
		lea	_QIndex(pc),a1
		lsr.w	d1,d0			;Index>>1
	;	moveq	#00,d4			;already <=25 and >=0
		move.b	0(a1,d0.w),d4		;upper 24 bits already clear in D4
		move.b	d1,0(a2,d4.w)		;QErr[n] = 1

		lea	_Log(pc),a1		;restore

		addq.w	#1,PSTAT		;return(P_STAT+1)
		bra.s	check_Ploope

multiP_error:
		add.w	#P_ECC_PROB,PSTAT
		bra.s	check_Ploope

noP_error:
		clr.b	-1(a3)			;PErr[P_NO] = 0;

check_Ploope:
		addq.w	#2,P_NO(a5)
		cmp.w	#(P_NO_LOOP*2),P_NO(a5)
		bls	check_Ploop


*		for( Q_NO = 0; Q_NO <= 25; Q_NO++ )
*		{

		clr.w	Q_NO(a5)		;P_NO = 0;
		move.l	a2,a4			;QErr[0]


*#ifdef	CACHE_CALC
*		   if (QErr[Q_NO])
*#endif
*		   {

check_Qloop:
		tst.b	(a4)+
		beq	check_Qloope


*			Q_Stat = Q_Check( StartAddr,Q_NO, Q_Stat,PErr,QErr);


	;-- so we now have
	;
	;-- A4 = QErr[Q_NO+1]
	;
	;-- A2 = QErr[0]
	;-- A5 = PErr[0]
	;-- A6 = StartAddr
	;
	; Do Q_Check() inline to avoid many stack push-pulls

*	Syn0 = Syn1 = 0;
*	Index = 2*43*Q_NO;
*
		movem.l	PSTAT/QSTAT,-(sp)
		move.w	#(2*1118),d5		;constants
		moveq	#(2*44),d6		;constants

		moveq	#00,d3			; Syn1<<16 & Syn0
		move.w	#255,d2			;max Pos

		move.w	Q_NO(a5),d0
		mulu.w	#86,d0			;Index = 2*43*Q_NO

		moveq	#P_NO_LOOP,d7		;Column

		moveq	#00,d1			;pre clear upper 24 bits

*	for( Column = 0; Column <= 42; Column++ )
*	{
*		if (Index >= (2*1118)) Index -= (2*1118);	// Index %= ( 2 * 1118 );
qcolumn_loop:
		cmp.w	d5,d0
		bcs.s	qpos_range1		;blt unsigned

		sub.w	d5,d0
qpos_range1:

*		elem = Buffer[Index];

		move.b	0(a6,d0.w),d1

*		if (elem)
*		{

		beq.s	qelem0_a

*		   Syn0 ^= elem;
*		   Pos   = (Exp[elem]+ 44-Column);
*		   if (Pos >= 255 ) Pos -= 255;
*		   Syn1 ^= Log[Pos];
*		}

		eor.b	d1,d3			;Syn0 ^= elem;
		moveq	#00,d4
		move.b	0(a0,d1.w),d4		;Pos = Exp[elem]..
		add.w	d7,d4
		addq.w	#2,d4			;44, 43, 42, 41...

		cmp.w	d2,d4
		bcs.s	qpos_range2
		sub.w	d2,d4
qpos_range2:

		swap	d3
		move.b	0(a1,d4.w),d1
		eor.b	d1,d3			;Syn1 ^= elem
		swap	d3

qelem0_a:
*		Index += 2*44;

		add.w	d6,d0
		dbf	d7,qcolumn_loop
		movem.l	(sp)+,PSTAT/QSTAT	;restore

*	}
*	// handle the special case of the parity check symbols
*	elem = Buffer [2 * ( 43*26 + Q_NO )];

		move.w	Q_NO(a5),d0
		add.w	#(43*26),d0
		add.w	d0,d0
		move.b	0(a6,d0.w),d1

*	if (elem)
*	{
		beq.s	qelem0_b

*	   Syn0 ^= elem;
*          Pos   = Exp[elem]+ 1;
*	   if (Pos >= 255 ) Pos -= 255;
*	   Syn1 ^= Log[Pos];

		eor.b	d1,d3
		moveq	#00,d4
		move.b	0(a0,d1.w),d4
		addq.w	#1,d4
		cmp.w	d2,d4
		bcs.s	qpos_range3
		sub.w	d2,d4
qpos_range3:

		swap	d3
		move.b	0(a1,d4.w),d1
		eor.b	d1,d3
		swap	d3

*	}

qelem0_b:

*	elem = Buffer[2 * ( 44*26 + Q_NO )];

		move.w	Q_NO(a5),d0
		add.w	#(44*26),d0
		add.w	d0,d0
		move.b	0(a6,d0.w),d1

*	if (elem)
*	{
		beq.s	qelem0_c

*	   Syn0 ^= elem;
*	   Syn1 ^= elem;

		eor.b	d1,d3			;Syn0 ^= elem
		swap	d3
		eor.b	d1,d3			;Syn1 ^= elem
		swap	d3
*	}

qelem0_c:

*	if( ( Syn0 == 0 ) && ( Syn1 == 0 ) )	// no errors
*	{

		tst.l	d3
		beq.s	noQ_Error

*#ifdef	CACHE_CALC
*		QErr[Q_NO] = 0;			// this column is now OK
*#endif
*		return( Q_Stat );
*	}
*	if( ( Syn0 == 0 ) || ( Syn1 == 0 ) )	// multi-byte error
*	{
		tst.w	d3
		beq.s	multiQ_Error
		swap	d3
		tst.w	d3
		beq.s	multiQ_Error

*		return( (UWORD)(Q_Stat + Q_ECC_PROB));
*	}
*	// should be able to correct this pattern
*
*	Pos = Exp[Syn0] - Exp[Syn1] + 44;		// remember: code is backwards
*	if (Pos >= 255) Pos -= 255;

		moveq	#00,d4
		moveq	#00,d1
		move.b	0(a0,d3.w),d1		;Exp[Syn1]
		swap	d3
		move.b	0(a0,d3.w),d4		;Exp[Syn0]
 		sub.w	d1,d4
 		add.w	#44,d4
 
 		cmp.w	d2,d4
 		blt.s	qpos_range4		;signed compare
 
		sub.w	d2,d4

qpos_range4:		

*	if( ( Pos > 44 ) || ( Pos < 0 ) ) {
*		DEB(printf(" ** OUTSIDE RANGE **"))
*		return( (UWORD)(Q_Stat + Q_ECC_PROB));	/* multi-byte error */
*	}

		tst.w	d4
		bmi.s	multiQ_Error
		cmp.w	#44,d4
		bgt.s	multiQ_Error

*	if( Pos > 42 ) {
*		Pos -= 43;
*		Index = 2 * ( Q_NO + 43*26 );
*		Index += 2 * 26 * Pos;
*	}
*	else {
*		Index = 2 * 43 * Q_NO;
*		Index += 2 * 44 * Pos;
*		Index %= ( 2 * 1118 );
*	}

		move.w	Q_NO(a5),d0

		cmp.w	#42,d4
		bhi.s	qposgt42

		mulu	#(2*43),d0		;sets upper 16 bits
		mulu	#(2*44),d4
		add.w	d4,d0
		divu.w	#(2*1118),d0
		swap	d0
		bra.s	qpos_newindex

qposgt42:
		sub.w	#43,d4
		add.w	#(43*26),d0
		add.w	d0,d0
		mulu	#(2*26),d4
		add.w	d4,d0

qpos_newindex:

*	Buffer[ Index ] ^= (UBYTE)Syn0;
		eor.b	d3,0(a6,d0.w)

*#ifdef	CACHE_CALC
*	PErr[Calc_PCodeNum(Index)] = 1;	// check this P code.
*#endif

	;	moveq	#00,d1
	;	move.w	d0,d1
	;	asr.l	#1,d1
	;	divsl.l	#43,d0:d1		;is all this really needed?

		moveq	#01,d1
		ext.l	d0			;clear upper 16 bits
		lsr.l	d1,d0			;Index>>1
		divu.w	#43,d0			;mod 43
		swap	d0

		move.b	d1,0(a5,d0.w)		;PErr[n] = 1

		addq.w	#1,QSTAT		;return(Q_STAT+1)
		bra.s	check_Qloope

multiQ_Error:
		add.w	#Q_ECC_PROB,QSTAT
		bra.s	check_Qloope

noQ_Error:
		clr.b	-1(a4)

check_Qloope:
		addq.w	#1,Q_NO(a5)
		cmp.w	#Q_NO_LOOP,Q_NO(a5)
		bls	check_Qloop		

*		}
*
*		DEB(printf("\nP_Stat=0x%x, Q_Stat=0x%x",P_Stat,Q_Stat))
*
*		if( ((P_Stat&0xFF) | (Q_Stat&0xFF))  == 0 )
*		{
*			break;			// got it !
*		}

	;;;	move.w	PSTAT,d1
	;;;	or.w	QSTAT,d1
	;;;	and.w	#$00FF,d1

*
* Darren - revised, break only if both are 0, else retry again
* (relatively cheap in actual use)
*
*		if(P_Stat | Q_Stat)  == 0 )
*		{
*			break;			// got it !
*		}

		move.l	PSTAT,d1
		or.l	QSTAT,d1
		beq.s	eccloopdone

		subq.w	#1,TryMax(a5)
		bne	Do_ECCLoop


*	}
*	return (ULONG)((P_Stat | (Q_Stat<<16)));
*

eccloopdone:
		move.w	QSTAT,d0
		swap	d0
		move.w	PSTAT,d0
				
	;-- Return result in D0

		lea	Do_ECCLocals_SIZEOF(sp),sp
		rts


*************************************************************************
* Optimization notes -- Darren
*
* 1.) Recode P/QErr initialization loop to use a fast DBF based fill,
*     and fill both P&QErr arrays at the same time.
*
* 2.) P_Stat, and Q_Stat kept in registers (faster than passing them
*     in to P_Check()/Q_Check(), and passing them back as returns
*
* 3.) P_Check() and Q_Check() are inline now; this avoids pushing/pulling
*     many long words on/off the stack in the main DoECC() loop.
*
* 4.) Register optimizations:
*
*	P_Stat, and Q_Stat in registers
*	A0 - points to Exp[] table (used frequently)
*	A2 - points to Log[] table (or temporarily to QIndex[] table)
*	A2 - points to PErr[] array
*	A3 - auto-incrementing pointer, indexes PErr[]
*	A4 - auto-incrementing pointer, indexes QErr[]
*	A5 - pointer to locals, and static pointer to PErr[]
*
* 5.) P_Check() & Q_Check loops are optimized via use of decrementing
*     DBF loops (rather than incrementing loops).  Constants are
*     pre-loaded in registers.  
*
* 6.) D3 is used to hold both Syn0, and Syn1 (lacking one more register,
*     this is still faster to use SWAP than moving data on/off temp
*     stack space.  As a side benefit, both Syn0 & Syn1 can be
*     checked for 0 (case of no errors) with a single TST.L
*
* 7.) D1 is generally used as the 'elem' (element) value, and
*     most often is used as a BYTE size register.. the main trick
*     here is avoiding lots of MOVEQ #00, or EXT's because of careful
*     use to avoid trashing the upper 24 bits when this register is
*     needed as a WORD size index register.
*
* 8.) D4 is most often used as 'Pos' (Position), and is treated
*     as a WORD wide register.
*
* 9.) P_NO is incremented by 2 rather than 1 during the main loop,
*     which avoids a step during the P_Check() code which calcs
*     Index as P_NO*2.
*
* 10.) Use of mulu.w, and divu.w - overflow conditions not possible.
*
*************************************************************************

* ================================================================================= */
*
* $Id: cdecc.asm,v 1.1 93/02/11 13:37:16 jerryh Exp Locker: jerryh $
*
* ROM'mable tables to speed up RS decoding. 2 tables with 256 bytes
* and 1 table with 1170 bytes.
* Created: Nov 31, 1992, Allan Havemose
*/
* ================================================================================= */

_Exp:
	dc.b 255,0,1,25,2,50,26,198,3,223
	dc.b 51,238,27,104,199,75,4,100,224,14
	dc.b 52,141,239,129,28,193,105,248,200,8
	dc.b 76,113,5,138,101,47,225,36,15,33
	dc.b 53,147,142,218,240,18,130,69,29,181
	dc.b 194,125,106,39,249,185,201,154,9,120
	dc.b 77,228,114,166,6,191,139,98,102,221
	dc.b 48,253,226,152,37,179,16,145,34,136
	dc.b 54,208,148,206,143,150,219,189,241,210
	dc.b 19,92,131,56,70,64,30,66,182,163
	dc.b 195,72,126,110,107,58,40,84,250,133
	dc.b 186,61,202,94,155,159,10,21,121,43
	dc.b 78,212,229,172,115,243,167,87,7,112
	dc.b 192,247,140,128,99,13,103,74,222,237
	dc.b 49,197,254,24,227,165,153,119,38,184
	dc.b 180,124,17,68,146,217,35,32,137,46
	dc.b 55,63,209,91,149,188,207,205,144,135
	dc.b 151,178,220,252,190,97,242,86,211,171
	dc.b 20,42,93,158,132,60,57,83,71,109
	dc.b 65,162,31,45,67,216,183,123,164,118
	dc.b 196,23,73,236,127,12,111,246,108,161
	dc.b 59,82,41,157,85,170,251,96,134,177
	dc.b 187,204,62,90,203,89,95,176,156,169
	dc.b 160,81,11,245,22,235,122,117,44,215
	dc.b 79,174,213,233,230,231,173,232,116,214
	dc.b 244,234,168,80,88,175

_Log:
	dc.b 1,2,4,8,16,32,64,128,29,58
	dc.b 116,232,205,135,19,38,76,152,45,90
	dc.b 180,117,234,201,143,3,6,12,24,48
	dc.b 96,192,157,39,78,156,37,74,148,53
	dc.b 106,212,181,119,238,193,159,35,70,140
	dc.b 5,10,20,40,80,160,93,186,105,210
	dc.b 185,111,222,161,95,190,97,194,153,47
	dc.b 94,188,101,202,137,15,30,60,120,240
	dc.b 253,231,211,187,107,214,177,127,254,225
	dc.b 223,163,91,182,113,226,217,175,67,134
	dc.b 17,34,68,136,13,26,52,104,208,189
	dc.b 103,206,129,31,62,124,248,237,199,147
	dc.b 59,118,236,197,151,51,102,204,133,23
	dc.b 46,92,184,109,218,169,79,158,33,66
	dc.b 132,21,42,84,168,77,154,41,82,164
	dc.b 85,170,73,146,57,114,228,213,183,115
	dc.b 230,209,191,99,198,145,63,126,252,229
	dc.b 215,179,123,246,241,255,227,219,171,75
	dc.b 150,49,98,196,149,55,110,220,165,87
	dc.b 174,65,130,25,50,100,200,141,7,14
	dc.b 28,56,112,224,221,167,83,166,81,162
	dc.b 89,178,121,242,249,239,195,155,43,86
	dc.b 172,69,138,9,18,36,72,144,61,122
	dc.b 244,245,247,243,251,235,203,139,11,22
	dc.b 44,88,176,125,250,233,207,131,27,54
	dc.b 108,216,173,71,142,0

_QIndex:			;45*26 entries
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,1,0,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,2,1,0,25
	dc.b 24,23,22,21,20,19,18,17,16,15
	dc.b 14,13,12,11,10,9,8,7,6,5
	dc.b 4,3,2,1,0,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 18,17,16,15,14,13,12,11,10,9
	dc.b 8,7,6,5,4,3,2,1,0,25
	dc.b 24,23,22,21,20,19,18,17,16,15
	dc.b 14,13,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 18,17,16,15,14,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,6,5
	dc.b 4,3,2,1,0,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,7,6,5,4,3,2,1,0,25
	dc.b 24,23,22,21,20,19,18,17,16,15
	dc.b 14,13,12,11,10,9,8,7,6,5
	dc.b 4,3,2,1,0,25,24,23,22,21
	dc.b 20,19,18,17,8,7,6,5,4,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 18,17,16,15,14,13,12,11,10,9
	dc.b 8,7,6,5,4,3,2,1,0,25
	dc.b 24,23,22,21,20,19,18,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,11,10,9,8,7,6,5
	dc.b 4,3,2,1,0,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,12,11,10,9
	dc.b 8,7,6,5,4,3,2,1,0,25
	dc.b 24,23,22,21,20,19,18,17,16,15
	dc.b 14,13,12,11,10,9,8,7,6,5
	dc.b 4,3,2,1,0,25,24,23,22,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 18,17,16,15,14,13,12,11,10,9
	dc.b 8,7,6,5,4,3,2,1,0,25
	dc.b 24,23,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 2,1,0,25,24,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,16,15
	dc.b 14,13,12,11,10,9,8,7,6,5
	dc.b 4,3,2,1,0,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,17,16,15,14,13,12,11,10,9
	dc.b 8,7,6,5,4,3,2,1,0,25
	dc.b 24,23,22,21,20,19,18,17,16,15
	dc.b 14,13,12,11,10,9,8,7,6,5
	dc.b 4,3,2,1,18,17,16,15,14,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 18,17,16,15,14,13,12,11,10,9
	dc.b 8,7,6,5,4,3,2,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,21,20,19,18,17,16,15
	dc.b 14,13,12,11,10,9,8,7,6,5
	dc.b 4,3,2,1,0,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,22,21,20,19
	dc.b 18,17,16,15,14,13,12,11,10,9
	dc.b 8,7,6,5,4,3,2,1,0,25
	dc.b 24,23,22,21,20,19,18,17,16,15
	dc.b 14,13,12,11,10,9,8,7,6,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,9,8,7,6,5,4,3
	dc.b 2,1,0,25,24,23,22,21,20,19
	dc.b 18,17,16,15,14,13,12,11,10,9
	dc.b 8,7,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,8,7
	dc.b 6,5,4,3,2,1,0,25,24,23
	dc.b 22,21,20,19,18,17,16,15,14,13
	dc.b 12,11,10,9,8,25,24,23,22,21
	dc.b 20,19,18,17,16,15,14,13,12,11
	dc.b 10,9,8,7,6,5,4,3,2,1
	dc.b 0,25,24,23,22,21,20,19,18,17
	dc.b 16,15,14,13,12,11,10,9,0,1
	dc.b 2,3,4,5,6,7,8,9,10,11
	dc.b 12,13,14,15,16,17,18,19,20,21
	dc.b 22,23,24,25,0,1,2,3,4,5
	dc.b 6,7,8,9,10,11,12,13,14,15
	dc.b 16,17,18,19,20,21,22,23,24,25

		END