#include "ecc.h"
#include <stdio.h>
#include <string.h>

/* ================================================================================ */
void	GF_PrintElem(GF a)
{
	int i;
	for(i=5;i>=0;i--)
	 (a & (1<<i)) ?	  printf("1") : printf("0");
}
/* ================================================================================ */
/* Print matrices */
/* ================================================================================ */

UBYTE *alpha_print(GF x)
{
  UBYTE c[10],d[10];

  if (x == 0)
   strcpy(c,"0");
  else if (x == 1)
   strcpy(c,"1");
  else
   {
     strcpy(c,"a^");
     stci_d(d,Exp[x]);
     strcat(c,d);
   }
  /*printf("TEST: %s",c);*/
  return c;
}
/* -------------------------------------------------------------------- */

void	PrintGFMatrix(GF *matr, int NRow, int NCol)
{
	int i,j;
	for(j=0;j<NRow;j++)
	 {
	    printf("\n");
	    for(i=0;i<NCol;i++)
	     {
	     	printf("%d, ",(int)Exp[matr[j*NCol + i]]);
	     }
	 }
}

void	PrintGFMatrixRaw(GF *matr, int NRow, int NCol)
{
	int i,j;
	for(j=0;j<NRow;j++)
	 {
	    printf("\n");
	    for(i=0;i<NCol;i++)
	     {
	     	printf("%d, ",(int)matr[j*NCol + i]);
	     }
	 }
}
/* -------------------------------------------------------------------- */

void	PrintGFVector(GF *vec, int N)
{
	int j;

        printf("\n");
	for(j=0;j<N;j++)
	 {
	   printf("%d ",(int)Exp[vec[j]]);
	 }
}
/* -------------------------------------------------------------------- */

void PrintPolyn(GF *pp,int maxgrad)
{
   int i;
   for(i=0;i<maxgrad;i++)
    printf("%s ",alpha_print(pp[i]));
   printf("\n");
} /*END: PrintPolyn() */
/* -------------------------------------------------------------------- */
