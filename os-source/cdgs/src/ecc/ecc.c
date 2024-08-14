/* ====================================================================== */
/*
 * $Id: ecc.c,v 1.2 93/02/10 09:36:18 havemose Exp Locker: havemose $
 *
 * Decoding of Cross Interleaved Reed Solomon codes as used in CDROM.
 * Created: Dec 1, 1992, Allan Havemose
 * See ecc.doc for technical documentation.
 */
/* ====================================================================== */

#include	<exec/types.h>
#include	"ecc.h"

/* --- LOCAL should be defined to 'static' when profiling is not needed */
#define	LOCAL	static

#define	INLINE	__inline

extern	UBYTE	Exp[],Log[];	// in ROM
extern	UBYTE	QIndex[];	// in ROM
extern	void	Print_QWord(UBYTE *Buffer, int Q_NO);
extern	void	Print_PWord(UBYTE *Buffer, int P_NO);

/* --- downcoded version of syndrome calculations --- */
extern 	void	__asm Calc_PSyns( register __a0 UBYTE *Buffer,
		    register __a1 UBYTE *Syn0,
		    register __a2 UBYTE *Syn1,
	    	    register __d0 WORD	Index);
extern 	void	__asm Calc_QSyns( register __a0 UBYTE *Buffer,
		    register __a1 UBYTE *Syn0,
		    register __a2 UBYTE *Syn1,
	    	    register __d0 WORD	Index);

/* ======================================================================= */
/* Function to gather statistics
 */
/* ======================================================================= */

#ifdef	RSTAT
extern	UWORD	RStat[];
extern	int	RStatNum;

LOCAL	void	AddStat(int index)
{
	RStat[RStatNum++]=(UWORD)index;
}
#endif

/* ======================================================================= */
/* Macros to calculate which P and Q code a certain index belongs to.
 * Used to update decoding caches during decoding
 */
/* ======================================================================= */

#define	Calc_PCodeNum(index) 	((index>>1)%43)
#define Calc_QCodeNum(index)	(QIndex[Index>>1])

/* ======================================================================= */
/* Calculate syndromes associated with P code.
/* P code have it's elements in the columns of the data matrix.
 */
/* ======================================================================= */

LOCAL	UWORD	__asm   P_Check(
 	register __a0	UBYTE 	*Buffer,	// Data buffer
	register __d0	int 	P_NO,		// current P code
 	register __d1	UWORD	P_Stat,		// status of this and previous ecc's.
	register __a1	UBYTE	*PErr,		// PErr Cache
	register __a2	UBYTE	*QErr		// QErr Cache
	)
{
	WORD	Index;
	WORD	Pos;
	WORD	Row;
	UBYTE	Syn0,Syn1;	// syndromes
	UBYTE	elem;		// temporary vars used during syndrome calc

	// calculate syndrome for P-matrix

#ifdef	DOWNCODE
	//Syn0 = Syn1 = 0;	syndromes cleared in asm
	Index = 2*P_NO;
	Calc_PSyns( Buffer,&Syn0,&Syn1,Index);

	//printf("\nP_Check_Down: s0=%d, s1=%d, P_NO=%d, P_Stat=0x%x",(int)Syn0,(int)Syn1,P_NO,P_Stat);
#else
	Syn0 = Syn1 = 0;

	Index = 2*P_NO;
	for( Row = 0; Row <= 25; Row++ )
	{
		elem  = Buffer[Index];	// Index = 2* ( 43 * Row + P_NO ); at this point
		if (elem)
		{
		   Syn0 ^= elem;
		   Pos   = (Exp[elem]+ 25-Row);
		   if (Pos >= 255 ) Pos -= 255;
		   Syn1 ^= Log[Pos];
		}
		Index += 2*43;
	}
	//printf("\nP_Check_C   : s0=%d, s1=%d, P_NO=%d, P_Stat=0x%x",(int)Syn0,(int)Syn1,P_NO,P_Stat);
#endif

#ifdef	DEB2
	if (!( ( Syn0 == 0 ) && ( Syn1 == 0 )))
	  printf("\nP_Check: s0=%d, s1=%d, P_NO=%d, P_Stat=0x%x",(int)Syn0,(int)Syn1,P_NO,P_Stat);
#endif
	if( ( Syn0 == 0 ) && ( Syn1 == 0 ) )	// no error
	{
#ifdef	CACHE_CALC
		PErr[P_NO] = 0;			// Now OK
#endif
		return( P_Stat );
	}

	if( ( Syn0 == 0 ) || ( Syn1 == 0 ) )	// multi byte error
	{
		// do not need to modify PErr
		return( (UWORD)(P_Stat + P_ECC_PROB));
	}

	// should be able to correct this pattern

	Pos = Exp[Syn0] - Exp[Syn1] + 25;		// remember: code is backwards
	if (Pos >= 255) Pos -= 255;

	DEB(printf("\nP: fixing one error. P_NO=%d, pos=%d, val=%d",P_NO,Pos,(int)Syn0))

	if( ( Pos > 25 ) || ( Pos < 0 ) ) {
		DEB(printf(" ** OUTSIDE RANGE **"))
		// do not need to modify PErr
		return( (UWORD)(P_Stat + P_ECC_PROB));	// ups, cannot handle anyway
	}


	Index  = 2 * P_NO + 2*43 * Pos;

	Buffer[ Index ] ^= Syn0;

#ifdef	CACHE_CALC
	QErr[ Calc_QCodeNum(Index) ] = 1; // also check this Q-code . PErr is already 1
#endif

#ifdef	RSTAT
	AddStat(Index);
#endif

	return( (UWORD)(P_Stat + 1));			// corrected one byte
}
/* ================================================================================ */
/* Calculate syndromes associated with Q code.
/* Q code have it's elements in the "diagonals" of the data matrix.
/* ================================================================================ */
LOCAL	UWORD	__asm   Q_Check(
	register __a0	UBYTE 	*Buffer,
	register __d0	int 	Q_NO,
	register __d1	UWORD	Q_Stat,
	register __a1	UBYTE	*PErr,
	register __a2	UBYTE	*QErr
	)
{
	UWORD	Column;
	UWORD	Index;
	WORD	Pos;		// error pos and temp var during syndrome calc.
	UBYTE	Syn0,Syn1;	// syndromes
	UBYTE	elem;		// current element in received word.

	// calculate syndromes for Q-code


#ifdef	DOWNCODE
	Index = 2*43*Q_NO;
	Calc_QSyns( Buffer,&Syn0,&Syn1,Index);
	//printf("\nQ_Check_Down: s0=%d, s1=%d, P_NO=%d, Q_Stat=0x%x",(int)Syn0,(int)Syn1,Q_NO,Q_Stat);
#else
	Syn0 = Syn1 = 0;
	Index = 2*43*Q_NO;

	for( Column = 0; Column <= 42; Column++ )
	{
		if (Index >= (2*1118)) Index -= (2*1118);	// Index %= ( 2 * 1118 );
		elem = Buffer[Index];
		if (elem)
		{
		   Syn0 ^= elem;
		   Pos   = (Exp[elem]+ 44-Column);
		   if (Pos >= 255 ) Pos -= 255;
		   Syn1 ^= Log[Pos];
		}
		Index += 2*44;
	}
	//printf("\nQ_Check_C   : s0=%d, s1=%d, P_NO=%d, Q_Stat=0x%x",(int)Syn0,(int)Syn1,Q_NO,Q_Stat);
#endif
	// handle the special case of the parity check symbols
	elem = Buffer [2 * ( 43*26 + Q_NO )];
	if (elem)
	{
	   Syn0 ^= elem;
           Pos   = Exp[elem]+ 1;
	   if (Pos >= 255 ) Pos -= 255;
	   Syn1 ^= Log[Pos];
	}

	elem = Buffer[2 * ( 44*26 + Q_NO )];
	if (elem)
	{
	   Syn0 ^= elem;
	   Syn1 ^= elem;
	}
	//DEB(printf("\nNEW: Q_Check: s0=%d, s1=%d, Q_NO=%d, Q_Stat=0x%x",(int)Syn0,(int)Syn1,Q_NO,Q_Stat))

#ifdef	DEB2
	if(!( ( Syn0 == 0 ) && ( Syn1 == 0 ) ))
	  printf("\nQ_Check: s0=%d, s1=%d, Q_NO=%d, Q_Stat=0x%x",(int)Syn0,(int)Syn1,Q_NO,Q_Stat);
#endif
	if( ( Syn0 == 0 ) && ( Syn1 == 0 ) )	// no errors
	{
#ifdef	CACHE_CALC
		QErr[Q_NO] = 0;			// this column is now OK
#endif
		return( Q_Stat );
	}

	if( ( Syn0 == 0 ) || ( Syn1 == 0 ) )	// multi-byte error
	{
		return( (UWORD)(Q_Stat + Q_ECC_PROB));
	}

	// should be able to correct this pattern

	Pos = Exp[Syn0] - Exp[Syn1] + 44;		// remember: code is backwards
	if (Pos >= 255) Pos -= 255;

	DEB(printf("\nQ: fixing one error. Q_NO=%d, pos=%d, val=%d",Q_NO,Pos,(int)Syn0))

	if( ( Pos > 44 ) || ( Pos < 0 ) ) {
		DEB(printf(" ** OUTSIDE RANGE **"))
		return( (UWORD)(Q_Stat + Q_ECC_PROB));	/* multi-byte error */
	}

	if( Pos > 42 ) {
		Pos -= 43;
		Index = 2 * ( Q_NO + 43*26 );
		Index += 2 * 26 * Pos;
	}
	else {
		Index = 2 * 43 * Q_NO;
		Index += 2 * 44 * Pos;
		Index %= ( 2 * 1118 );
	}

	Buffer[ Index ] ^= (UBYTE)Syn0;

#ifdef	CACHE_CALC
	PErr[Calc_PCodeNum(Index)] = 1;	// check this P code.
#endif

#ifdef	RSTAT
	AddStat(Index);
#endif

	return( (UWORD)(Q_Stat + 1));				// corrected one byte
}

/* =============================================================================== */

LOCAL	ULONG	__asm Do_ECC(
	register __a0	UBYTE *StartAddr,
	register __d0	int Try_Max
)
{
	UWORD		Try;		/* Loop count */
	UWORD		P_NO;		/* Column index */
	UWORD		Q_NO;		/* Row index */
	UWORD		P_Stat;		/* Column check flags */
	UWORD		Q_Stat;		/* Row check flags */
	UBYTE		PErr[44];	// PErr[43],
	UBYTE		QErr[28];	// QErr[27]


#ifdef	CACHE_CALC
	ULONG		*lptr;		/* pointer to ULONG */

	lptr = (ULONG*)PErr;
	for(Try=0;Try<11;Try++)		// 44 bytes in PErr
	  lptr[Try] = 0xFFFFFFFF;

	lptr = (ULONG*)QErr;		// 28 bytes in QErr
	for(Try=0;Try<7;Try++)
	  lptr[Try] = 0xFFFFFFFF;
#endif

	for( Try = 1; Try <= Try_Max; Try++ )
	{
		Q_Stat = P_Stat = 0;
		DEB(printf("\nIteration no %d ",Try))
		//DEB(Print_PQErr(PErr,QErr))
		for( P_NO = 0; P_NO <= 42; P_NO++ )
		{
#ifdef	CACHE_CALC
		   if (PErr[P_NO])
#endif
		   {
			P_Stat = P_Check( StartAddr,P_NO, P_Stat,PErr,QErr);
		   }
		}

		for( Q_NO = 0; Q_NO <= 25; Q_NO++ )
		{
		   //printf("\nQErr[%d]=%d",(int)Q_NO,(int)QErr[Q_NO]);
#ifdef	CACHE_CALC
		   if (QErr[Q_NO])
#endif
		   {
			Q_Stat = Q_Check( StartAddr,Q_NO, Q_Stat,PErr,QErr);
		   }
		}

		DEB(printf("\nP_Stat=0x%x, Q_Stat=0x%x",P_Stat,Q_Stat))

		if( ((P_Stat&0xFF) | (Q_Stat&0xFF))  == 0 )
		{
			break;			// got it !
		}
	}
	return (ULONG)((P_Stat | (Q_Stat<<16)));
}
/* =============================================================================== */
/* Main entry point to CD ecc algorithm.
 *
 */
/* =============================================================================== */

int	__asm Decode_CDECC(register __a0 UBYTE	*Buffer)
{
	int	StartAddr;
	int 	Odd_Stat,Even_Stat;

	/* --- Even bytes --- */
	StartAddr = (ULONG)Buffer+12;
	Even_Stat = Do_ECC((UBYTE*)StartAddr,TRY_MAX);

#ifdef	DEBA
	P_Stat = Even_Stat&0xFFFF,
	Q_Stat = (Even_Stat>>16)&0xFFFF;

	printf("\nEven: Status = 0x%x. \n\tP_Stat= 0x%x, Q_Stat = 0x%x ",Even_Stat, P_Stat,Q_Stat);
#endif

	/* --- Odd bytes --- */
	StartAddr++;
	Odd_Stat = Do_ECC((UBYTE*)StartAddr,TRY_MAX);

#ifdef	DEBA
	P_Stat = Odd_Stat&0xFFFF,
	Q_Stat = (Odd_Stat>>16)&0xFFFF;

	printf("\nOdd: Status = 0x%x. \n\tP_Stat= 0x%x, Q_Stat = 0x%x ",Odd_Stat, P_Stat,Q_Stat);
#endif

	return (Even_Stat | Odd_Stat);
}
