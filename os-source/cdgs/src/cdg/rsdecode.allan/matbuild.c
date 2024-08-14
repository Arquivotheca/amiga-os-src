#include	"ecc.h"

GF	Hp[P_RANK][P_LEN],Hq[Q_RANK][Q_LEN];

/* --- Build parity check matrices for P and Q codes --- */

void	Hp_Build(void)
{
	int	i,j;
	GF	a;

	for(i=0;i< P_RANK;i++)
	{
	  a = Log[i];		/* j is the exponent */
	  Hp[i][0] = 1;
	  for(j=1;j<P_LEN;j++)
	  	Hp[i][j] = GF_Mul(Hp[i][j-1],a);
	}
	PrintGFMatrixRaw(Hp,P_RANK,P_LEN);
}

#if 0
void	Hq_Build(void)
{
	int	i,j;
	GF	a;

	for(i=0;i< Q_RANK;i++)
	{
	  a = Log[i];		/* j is the exponent */
	  Hq[i][0] = 1;
	  for(j=1;j<Q_LEN;j++)
	  	Hq[i][j] = GF_Mul(Hq[i][j-1],a);
	}
/*	PrintGFMatrix(Hq,Q_RANK,Q_LEN);*/
}
#endif

void	BuildParityMatrices()
{
	Hp_Build();
/* 	Hq_Build(); */
}

main()
{
	BuildParityMatrices();
}