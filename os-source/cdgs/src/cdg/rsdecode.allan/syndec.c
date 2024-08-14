#include	"ecc.h"
#include	<stdio.h>


#define	PRINT_OUTX
/* --- syndrome decoding of RS codes ---*/

void	Syn_Calc(GF Syn[], GF H[], GF rec[], int NRow, int NCol)
/*	Syn[] : syndrome (to be calculated)
	H[]   :	parity check matrix. H twodimensional H[NRow][NCol]
	rec   : received codeword
	NRow  : number of rows in H
	NCol  : number of columns in H = length of codeword

	NOTE:   since the definition of the Hp and Hq matrices are "backwards" the
		syndrome calculation invertes the positions in rec[].
*/
{
	int	i,j,HIndex;
	GF	n;

 	HIndex = 0;		/* j*NCol; */
	for(j=0;j<NRow;j++)
	 {
	 	Syn[j] = 0;
	 	for(i=0;i<NCol;i++)
	 	 {
	 	 	n = GF_Mul(H[HIndex],rec[NCol-i-1]);	/* WAS: rec[i] */
	 	 	Syn[j] = GF_Add(Syn[j],n);
	 	 	HIndex++;
	 	 }
	 }
#ifdef	PRINT_OUT
	printf("\n==============================================");
	printf("\nReceived:");
	PrintGFVector(rec,NCol);
	printf("\nSyndrome:");
	PrintGFVector(Syn,NRow);
#endif
}
/* --------------------------------------------------------------------------------- */

void 	SynPoly_Mul(GF *a,GF *x,GF *y)
/*
FUNCTION:
	Multiply two polynomials.
SYNOPSIS:
        GF *a : result of multiplication.
	GF *x : first factor.
	GF *y : second factor.
	int len: maximal degree of x and y.
	Parameters are like 'a = x*y'.
*/
{
   register int i,j;
   SynPoly_Clear2(a);	/* clear result: length = 2*LOCLEN */
   for(i=0;i<LOCLEN;i++)
    for(j=0;j<LOCLEN;j++)
     a[i+j] = GF_Add(a[i+j],GF_Mul(x[i],y[j]));
}
/* -------------------------------------------------------------------- */

void	ConstMul(GF *a,GF con)
/*
FUNCTION:
	Multipy a polynomial by a constant.
SYNOPSIS:
        GF *a: the polynomial. (source and destination).
	GF con: the constand.
	Parameters are like 'a(x) = a(x)*con.
*/
{
   register int i;
   for(i=0;i<LOCLEN;i++)
    a[i] = GF_Mul(a[i],con);
}  /*END: ConstMul() */
/* -------------------------------------------------------------------- */
UBYTE	SynPoly_Degree(GF *a)
{
	if (a[3])	return 3;
	if (a[2])	return 2;
	if (a[1])	return 1;
	if (a[0])	return 0;
}
/* -------------------------------------------------------------------- */

void DiffPolyn(GF *dif,GF *orig)
/*

FUNCTION:
	Differentiate a polynomial.
SYNOPSIS:
	GF *dif: the derived polynomial.
	GF *orig: the polynomial to differentiate.
	Parameters are like 'Dif = dF/dx '.
*/
{
  int i;
  for(i=0;i<LOCLEN-1;i++)			/* AH */
   dif[i] = GF_Mul(orig[i+1],(i+1)% 2);		/* p=2 */
} /*END: DiffPolyn() */
/* -------------------------------------------------------------------- */

GF EvalPolyn(GF *pp,GF elem,ULONG degr)
/*

FUNCTION:
	Evaluate a polynomial in a certain point.
SYNOPSIS:
        GF *pp: the polynomial.
	GF elem: the element in which to evaluate.
	ULONG degr: degree of polynomial.
	Return: polynomial(elem).
*/
{
	/* efficient evaluation of polynomials */
	if (degr == 0)
		return pp[0];
	if (degr == 1)
	 	return (GF)GF_Add( pp[0], GF_Mul(pp[1],elem));
	if (degr == 2)
	 	return (GF)GF_Add( pp[0],GF_Mul( GF_Add(pp[1],GF_Mul(pp[2],elem)),elem));
	/* degr == 3 */
	return (GF)GF_Add( pp[0],GF_Mul( GF_Add(pp[1],GF_Mul( GF_Add(pp[2],GF_Mul(pp[3],elem)),elem)),elem));

#if 0
   int i,j;
   GF tot,x;

   tot = pp[0];
   for(i=1;i<lastindex;i++)
    {
      x = 1;
      for(j=0;j<i;j++)
        x = GF_Mul(x,elem);	/*x indeholder nu elem^i*/
      x = GF_Mul(pp[i],x);		/*mult med konstant*/
      tot = GF_Add(x,tot);
    }
   return(tot);
#endif
}  /*END: EvalPolyn() */
/* -------------------------------------------------------------------- */

void	KillKoef(GF *pp,ULONG koef,ULONG stop)
/*

FUNCTION:
	Zero out the upper part of a polynomial.
SYNOPSIS:
	GF *pp: the polynomial.
	GF koef: coefficient to start
	GF stop: coefficient to end.
*/
{
   int i;
   for(i=koef;i<stop;i++)
    pp[i]=0;
} /*END: KillKoef() */
/* --------------------------------------------------------------------- */

GF	CalcDisp(int R, int Len, GF *Gamma, GF *SynVec)
{
  register int j;
  GF	   Disp;

  for(j=Disp=0;j<=Len;j++)
    Disp = GF_Add(Disp,GF_Mul(Gamma[j],SynVec[R-j-1]));
  return(Disp);
}
/* -------------------------------------------------------------------- */

int ErrorPositions(GF *pp,int degr, GF *ErrorPos)
{
   int i,elem, ErrorNum;

   ErrorNum=0;

   /* NOTE: there must be some way to not check whole field. Still confused :-( */

   for(elem=0; elem < GF64_SIZE ; elem++)
    {
      if (EvalPolyn(pp,elem,degr)== 0)
       {
          ErrorPos[ErrorNum++]=GF_Inv(elem);
       }
    }

#ifdef PRINT_OUT
   /* print out the result */
   printf("\nError Positions:");
   for(i=0;i<ErrorNum;i++)
    {
       printf("%d",(int)ErrorPos[i]);
       printf("[=%s] ",alpha_print(ErrorPos[i]));
    }
   printf("ErrorNum = %d \n",ErrorNum);
#endif
   return ErrorNum;
}  /*END: RootsInPolyn() */
/* -------------------------------------------------------------------- */

void ErrorValues(GF *Rec, GF *Gamma, GF *SynVec, GF *ErrorPos, int ErrorNum)
{
   GF   Omega[2*LOCLEN],GamDif[LOCLEN];
   GF	ErrorVal[ERRNUM];
   GF 	ep,n,i;

   SynPoly_Mul(Omega,SynVec,Gamma);
   KillKoef(Omega,LOCLEN,2*LOCLEN);	/*egl Omega = Omega mod x^2t */
   DiffPolyn(GamDif,Gamma);

#ifdef	PRINT_OUT
   printf("Om: ");
   PrintPolyn(Omega,15);
   printf("GA': ");
   PrintPolyn(GamDif,10);
#endif

   for(i=0;i<ErrorNum;i++)
    {
      ep = GF_Inv(ErrorPos[i]);
      ErrorVal[i] = EvalPolyn(Omega,ep,LOCLEN-1);
      n = EvalPolyn(GamDif,ep,LOCLEN-2);	/* degree drops 1 after differentiation */
      n = GF_Mul(n,ep);
      n = GF_Inv(n);
      ErrorVal[i]=GF_Mul(ErrorVal[i],n);
    }
#ifdef PRINT_OUT
   printf("\nError values: ");
   for(i=0;i<ErrorNum;i++)
    {
      printf("%s ",alpha_print(ErrorVal[i]));
      printf("[=%d], ",(int)ErrorVal[i]);
    }
#endif

   /* NOTE: we now need to account for the fact that the error positions are calculated
      backwards. Current implementation assumes P code */
   for(i=0;i<ErrorNum;i++)
    {
    	ep = Exp[ErrorPos[i]];
    	if (ep != 63)
    	 {
    	   if (ep < 24)				/* check for outside vector */
    	     Rec[P_LEN-1-ep] ^= ErrorVal[i];
    	 }
    	else
    	  Rec[P_LEN-1]  ^= ErrorVal[i];
    }
#ifdef	PRINT_OUT
   printf("\nError Positions:");
   for(i=0;i<ErrorNum;i++)
    printf("%d ",(int)Exp[ErrorPos[i]]);
   printf("\nCorrected vector");
   PrintGFVector(Rec,P_LEN);
#endif
} /*END: ErrorValues() */
/* --------------------------------------------------------------------- */

void DecodeRS(GF *Rec)
{
   GF	T[LOCLEN];				/* tmp var for polynomial math*/
   GF	Gamma[LOCLEN],B[LOCLEN],SynVec[LOCLEN];
   GF	ErrorPos[ERRNUM];
   GF	StopNum,R,Len,Disp,DegGam;
   int	ErrorNum;

   SynPoly_Clear(Gamma);
   SynPoly_Clear(B);
   SynPoly_Clear(T);
   Gamma[0]=B[0]=1;
   R = Len = 0;
   /* --- specific to P-code. --- */
   StopNum = 4;					/* StopNum = 2 for Q code */
   Syn_Calc(SynVec,(GF *)Hp,Rec,P_RANK,P_LEN);	/* Syn_Calc(SynVec,Hq,Rec,Q_RANK,Q_LEN); */

   do
   {
     R++;
     if (Disp = CalcDisp(R,Len,Gamma,SynVec))
      {
        SynPoly_Copy(T,B);
	SynPoly_MulX(T);
	ConstMul(T,Disp);
	SynPoly_Add(T,Gamma);
	if (2*Len <= R-1 )
 	 {
	   SynPoly_Copy(B,Gamma);
	   ConstMul(B,GF_Inv(Disp));
	   SynPoly_Copy(Gamma,T);
	   Len=R-Len;
	 }
	else
	 {
	   SynPoly_Copy(Gamma,T);
           SynPoly_MulX(B);
	 }
      }
     else
      {
        SynPoly_MulX(B);
      }
   } while (R != StopNum);

   DegGam = SynPoly_Degree(Gamma);

#ifdef	PRINT_OUT
   printf("\nGamma: %d %d %d %d",(int)Gamma[0],(int)Gamma[1],(int)Gamma[2],(int)Gamma[3]);
   printf("\nLocator degree= %d, L=%d",(int)DegGam,(int)Len);
   printf("\nGamma: ");
   PrintPolyn(Gamma,DegGam+1);
#endif
   if (DegGam == Len )
    {
      if (ErrorNum = ErrorPositions(Gamma,DegGam,ErrorPos))/* find positions */
          ErrorValues(Rec,Gamma,SynVec,ErrorPos,ErrorNum);  /*find error values */
    }
   else
    {
          printf("\nTo many errors");
    }
}/*END: DecodeRS() */
/* --------------------------------------------------------------------- */
