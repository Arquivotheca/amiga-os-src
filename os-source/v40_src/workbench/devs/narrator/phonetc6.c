/*
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************
*/

#include <exec/types.h>
#include <functions.h>
#include "parms.h"
#include "phonet.h"
#include "pc.h"
#include "phonetsubs.h"


void PhonetC6()
{
LONG	delta, incr, i;
extern	void	Phonet60();
extern	void	Phonet61();
extern	void	Phonet62();



    DBG(printf("Entering C.6 -- Sonorant rules\n"));


    Phonet60();

    if (LAST_PC_ISNOT(VOICED) && LAST_PC_ISNOT(GLOTTAL) && LAST_PC_ISNOT(SILENT)
	&& (Last_PC != PC_HH)) Phonet61();

    if THIS_PC_ISNOT(VOWEL) Phonet62();


   /*
    *	Delay onset of voicing after /D/, /G/, and /GX/.  (3/28/91)
    *	Changed	the way AV is done.  Was copied from previous time,
    *	now just set to 0.
    */

    switch(Last_PC)
    {
	case PC_D:
	case PC_GX:
	case PC_G:
	    AV.Cumdur++;    AV.Segdur--;    AV.Mintime++;
	    F1.Cumdur++;    F1.Segdur--;    F1.Mintime++; 
	    F2.Cumdur++;    F2.Segdur--;    F2.Mintime++; 
	    F3.Cumdur++;    F3.Segdur--;    F3.Mintime++; 
	    A1.Cumdur++;    A1.Segdur--;    A1.Mintime++; 
	    A2.Cumdur++;    A2.Segdur--;    A2.Mintime++; 
	    A3.Cumdur++;    A3.Segdur--;    A3.Mintime++; 

	    *(F1.Cumdur - 1) = *(F1.Cumdur - 2);
	    *(F2.Cumdur - 1) = *(F2.Cumdur - 2);
	    *(F3.Cumdur - 1) = *(F3.Cumdur - 2);
	    *(A1.Cumdur - 1) = *(A1.Cumdur - 2);
	    *(A2.Cumdur - 1) = *(A2.Cumdur - 2);
	    *(A3.Cumdur - 1) = *(A3.Cumdur - 2);
	    *(AV.Cumdur - 1) = 0;

	    break;

	default:
	    break;
    }

}  /* end PhonetC6() */


void Phonet60()
{

   /*
    *	C.6.0.1 and C.6.0.2  -- Applies to f4 -- ignored.
    */


   /*
    *	C.6.0.3a and b
    */

    if (THIS_PC_IS(ROUND) && (Last_PC == PC_Y))
      {
	F1.Tcf = 13;
	F2.Tcf = 13;
	F3.Tcf = 13;
	A1.Tcf = 13;
	A2.Tcf = 13;
	A3.Tcf = 13;
	DBG(printf("C.6.0.3a\n"));
      }
    else if LAST_PC_IS(LIQGLIDE)
      {
	F1.Tcf = 9;
	F2.Tcf = 9;
	A1.Tcf = 9;
	A2.Tcf = 9;
	DBG(printf("c.6.0.3b\n"));
      }


    if (LAST_PC_IS(LIQGLIDE) && LAST_PC_IS(RETRO))
      {
	F3.Tcf = 10;
	A3.Tcf = 10;
	DBG(printf("c.6.04\n"));
      }


    if (LAST_PC_IS(WGLIDE) || LAST_PC_IS(YGLIDE))
      {
	F1.Bper = 25;
	F2.Bper = 25;
	F3.Bper = 25;
	A1.Bper = 25;
	A2.Bper = 25;
	A3.Bper = 25;
	DBG(printf("c.6.05\n"));
      }


    if (THIS_PC_IS(ROUND) && LAST_PC_IS(PALATAL))
      {
	F2.Tcf += 5;
	A2.Tcf += 5;
	DBG(printf("c.6.06\n"));
      }

}  /* end Phonet60() */


void Phonet61()
{
LONG	i, delta, incr;


    Aspdur = 5;
    Aspam1 = Aspamp;
    DBG(printf("c.6.1.1 and c.6.1.2\n"));

       

    if (!Last_PCS)
      {
	Aspdur  = 2;
	Aspam1 -= 3;
	DBG(printf("c.6.1.3\n"));
      }


    if THIS_PC_ISNOT(VOWEL)
      if (THIS_PC_ISNOT(LATERAL) && LAST_PC_ISNOT(LABIAL))
 	{
	  Aspdur = 7;
	  DBG(printf("c.6.1.4a\n"));
        }
      else
	{
	  Aspdur = 6;
	  DBG(printf("c.6.1.4b\n"));
	}


    if ((Old_PCF & VOICED) && !This_PCS)
      {
	Aspdur -= 1;
	DBG(printf("c.6.1.5\n"));
       }


    if ((This_PC == PC_AXP) || (This_PC == PC_QX))
      {
	Aspdur = 8;
	Aspam1 -= 11;
	if LAST_PC_ISNOT(VOICED)   AV.Target = 0;
	DBG(printf("6.1.6\n"));
      }


    if LAST_PC_IS(PLOSIVE)
      {
	Burstdur   >>= 1;
	AV.Cumdur   -= Burstdur;
	AH.Cumdur   -= Burstdur;
	AV.Segdur   += Burstdur;
	AH.Segdur   += Burstdur;
	AV.Mintime  -= Burstdur;
	AH.Mintime  -= Burstdur;
	Aspdur      += Burstdur;
 	DBG(printf("c.6.1.7 adjusting av,ah cumdur,segdur by %ld\n",Burstdur));
      }


    if ((Old_PC == PC_S) && !(Last_bits & SYLLSTART) && LAST_PC_IS(PLOSIVE))
      {
	Aspdur = 0;
    	DBG(printf("c.6.1.7.5  /S/ precedes plos in same syll, Aspdur = 0\n"));
      }


    if LAST_PC_IS(FRICATIVE)
      if THIS_PC_IS(VOWEL)
	{
	  Aspdur = 1;
	  DBG(printf("c.6.1.8\n"));
	}
      else
	{
  	  Aspdur = MIN(AH.Segdur >> 1, 2);	/* was just segdur/2 in book */
	  DBG(printf("c.6.1.9\n"));
	}



    Aspdur  = MIN(MIN(Aspdur,AH.Segdur),AV.Segdur);
    DBG(printf("c.6.1.10 aspdur now %ld\n",Aspdur));



    if (THIS_PC_IS(RETRO) && LAST_PC_IS(ALVEOLAR))
      {
	Aspam1 += 5;
	DBG(printf("c.6.1.11\n"));		/* Changed 3/15/91 was 9 */
      }



    if (Aspdur != 0)
      {
	AH.Segdur  -= Aspdur;			/* Adjust segdurs */
	AV.Segdur  -= Aspdur;
	AH.Mintime  = Aspdur;
	AV.Mintime  = Aspdur;
	while (Aspdur--)
          {
	    *AH.Cumdur++  = Aspam1;
	    *AV.Cumdur++  = 0;
          }
	DBG(printf("c.6.1.12a/b ah.segdur=%d  av.segdur=%d\n",
		AH.Segdur,AV.Segdur));
      }


}


void Phonet62()
{
WORD	temp;

    if (This_PC == PC_L)
      {
	AV.Target -= 3;
	DBG(printf("C.6.2.1\n"));
      }


    if ((This_PC == PC_L) && NEXT_PC_IS(SONORANT) && NEXT_PC_ISNOT(NASAL))
      {
	F2.Target = (29*F2.Target + 3*Parms[Next_PC].f2) >> 5;
	A2.Target = (29*A2.Target + 3*Parms[Next_PC].a2) >> 5;
	DBG(printf("c.6.2.2\n"));
      }



    if (This_PC == PC_RR)
      {
	F3.Target = F2.Target + FADJ(250);
	DBG(printf("c.6.2.3\n"));

	if LAST_PC_IS(LABIAL | DENTAL | PALATAL)
	  {
	    F3.Bper = 90;
	    A3.Bper = 90;
	    DBG(printf("c.6.2.3.1 R before lab|dent|palat set f3.bper=90\n"));
	  }
      }



    if ((This_PC == PC_RR) && NEXT_PC_IS(SONORANT) && NEXT_PC_ISNOT(NASAL))
      {
	F2.Target = (24*F2.Target + 8*Parms[Next_PC].f2) >> 5;
	A2.Target = (24*A2.Target + 8*Parms[Next_PC].a2) >> 5;
	DBG(printf("c.6.2.4\n"));
      }


    if ((This_PC == PC_L) && (NEXT_PC_ISNOT(SONORANT) || NEXT_PC_IS(NASAL)))
      {
	F2.Target = (29*F2.Target + 3*FADJ(1450)) >> 5;
	DBG(printf("c.6.2.5\n"));
      }


    if ((This_PC == PC_RR) && (NEXT_PC_ISNOT(SONORANT) || NEXT_PC_IS(NASAL)))
      {
	F2.Target = (24*F2.Target + 8*Parms[Next_PC].f2) >> 5;
	A2.Target = (24*A2.Target + 8*Parms[Next_PC].a2) >> 5;
	DBG(printf("c.6.2.6\n"));
      }


    if (LAST_PC_ISNOT(NASAL) && LAST_PC_IS(SONORANT) && THIS_PC_IS(LATERAL) &&
	(This_PC != PC_LX))
      {
	F1.Bper = 90;
	F2.Bper = 90;
	F3.Bper = 90;
	A1.Bper = 90;
	A2.Bper = 90;
	A3.Bper = 90;
 	DBG(printf("c.6.2.7\n"));
      }


    if (This_PC == PC_LX)
      {
	F1.Bper = 25;	A1.Bper = 25;
	F2.Bper = 50;	A2.Bper = 50;
	F3.Bper = 50;	A3.Bper = 50;
	DBG(printf("c.6.2.7.5  LX Bper rule, F1=25%, F2,3=50%\n"));
      } 


    if THIS_PC_ISNOT(LIQGLIDE)
      {
	F1.Tcf = 9;
	F2.Tcf = 9;
	F3.Tcf = 9;
	A1.Tcf = 9;
	A2.Tcf = 9;
	A3.Tcf = 9;
	DBG(printf("c.6.2.8.a\n"));
      }
    else if LAST_PC_IS(LIQGLIDE)
      {
	F1.Tcf = 5;
	F2.Tcf = 5;
	F3.Tcf = 5;
	A1.Tcf = 5;
	A2.Tcf = 5;
	A3.Tcf = 5;
	DBG(printf("c.6.2.8.b\n"));
      }
    else
      {
	F1.Tcf = 7;
	F2.Tcf = 7;
	F3.Tcf = 7;
	A1.Tcf = 7;
	A2.Tcf = 7;
	A3.Tcf = 7;
	DBG(printf("c.6.2.8.c\n"));
      }


    if THIS_PC_IS(RETRO)
      {
	F3.Tcf = 9;
	A3.Tcf = 9;
	DBG(printf("c.6.2.9\n"));
      }


    if THIS_PC_IS(LIQGLIDE)
      {
	if LAST_PC_IS(VOWEL)
	  {
	    temp   = This_dur >> 1;
	    F1.Tcf = temp;
	    F2.Tcf = temp;
	    F3.Tcf = temp;
	    A1.Tcf = temp;
	    A2.Tcf = temp;
	    A3.Tcf = temp;
	    DBG(printf("c.6.2.10 this pc liqgli,last pc voicd,tcf=segdur/2\n"));
	  }
      }


    if (THIS_PC_IS(LATERAL) && LAST_PC_IS(STOP) && LAST_PC_IS(LABIAL))
      {
	    F1.Tcf = 1;
	    F2.Tcf = 1;
	    F3.Tcf = 1;
	    A1.Tcf = 1;
	    A2.Tcf = 1;
	    A3.Tcf = 1;
	    DBG(printf("c.6.2.11 labial stop before lateral f1-a3 tcf=1\n"));
      }	


}  /* end phonet62() */
