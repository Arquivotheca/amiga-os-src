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
#include "narrator.h"

WORD	F1Diptar;
WORD	F2Diptar;
WORD	F3Diptar;
WORD	A1Diptar;
WORD	A2Diptar;
WORD	A3Diptar;

void PhonetC7(iorb, centphon, centpct)
struct narrator_rb *iorb;
ULONG centphon, centpct;
{
	LONG	delta, incr, i;
extern	void	Phonet70();
extern	void	Phonet71();



    DBG(printf("Entering C.7 -- Vowel rules\n"));


   /*
    *  	Setup temp diphthong target values.  Use formant and ampltude 
    *	values from the "This_PC+1" entry of the Parms array.  Note that
    *	This_PC+1 is not equal to Next_PC because Next_PC points to the
    *	phone after the "diphthong prime" entry.
    */

    if THIS_PC_IS(DIPHTHONG)
      {
	F1Diptar = Parms[This_PC+1].f1;
	F2Diptar = Parms[This_PC+1].f2;
	F3Diptar = Parms[This_PC+1].f3;
	A1Diptar = Parms[This_PC+1].a1;
	A2Diptar = Parms[This_PC+1].a2;
	A3Diptar = Parms[This_PC+1].a3;
	DBG(printf("initializing temp f1..f3  a1..a3 diptars\n"));
      }

			     Phonet70(iorb, centphon, centpct);
    if THIS_PC_IS(DIPHTHONG) Phonet71(iorb);


}


void Phonet70(iorb, centphon, centpct)
struct narrator_rb *iorb;
ULONG centphon, centpct;
{


    if ((This_PC == PC_AXP) && (Old_PCF & SONORANT) && 
	((Old_PCF & NASAL) ? 0 : 1))
      {
	F2.Target = (F2.Target + Parms[Old_PC].f2) >> 1;
	A2.Target = (A2.Target + Parms[Old_PC].a2) >> 1;
	DBG(printf("rule 7.01: new f2,a2 targs = %d  %d\n",
		F2.Target,A2.Target));
      }



    if (THIS_PC_IS(FRONT) && THIS_PC_IS(LAX) && NEXT_PC_IS(VELAR))
      {
	F2Diptar = F2.Target;
	A2Diptar = A2.Target;
	DBG(printf("rule 7.02\n"));
      }


    if ((THIS_PC_IS(FRONT) || THIS_PC_IS(YGLIDE)) && NEXT_PC_IS(LATERAL))
      {
	if(Next_PC == PC_LX)
	  {
	    F2Diptar = (F2Diptar + Parms[PC_LX].f2) >> 1;
	    A2Diptar = (A2Diptar + Parms[PC_LX].a2) >> 1;
	  }
	else  F2Diptar -= FADJ(300);
	DBG(printf("rule 7.03\n"));

	if THIS_PC_ISNOT(DIPHTHONG)
	  {
	    if(Next_PC == PC_LX)
	      {
	        F2.Target = (F2.Target + (UWORD)Parms[PC_LX].f2) >> 1;
	        A2.Target = (A2.Target +  (WORD)Parms[PC_LX].a2) >> 1;
	      }
	    else  F2.Target -= FADJ(300);
	    DBG(printf("rule 7.04 \n"));
	  }
      }



    if ((This_PC == PC_YU) && NEXT_PC_IS(ALVEOLAR) && NEXT_PC_IS(PLOSIVE))
      {
	F2Diptar += FADJ(200);
	DBG(printf("rule 7.05\n"));
      }



    if (LAST_PC_IS(F2BACK) && THIS_PC_IS(ROUND))
      {
	F2.Tcf += 5;
	A2.Tcf += 5;
	DBG(printf("rule 7.06\n"));
      }



    if (THIS_PC_IS(YGLIDE) && NEXT_PC_IS(ALVEOLAR))
      {
	F2Diptar -= FADJ(150);
	DBG(printf("rule 7.07\n"));
      }


/*	rule 7.08 not included 	*/


    if THIS_PC_IS(SCHWA)
      {
	F3.Target = (F3.Oldval + F3.Target + (UWORD)Parms[Next_PC].f3)/3;
	A3.Target = (A3.Oldval + A3.Target +  (WORD)Parms[Next_PC].a3)/3;
	DBG(printf("rule 7.09 \n"));
      }


    if THIS_PC_IS(F2BACK)
      {
	F2.Tcf -= 2;
	A2.Tcf -= 2;
	DBG(printf("rule 7.010\n"));
      }


    if NEXT_PC_IS(NASAL)
      if THIS_PC_IS(DIPHTHONG)
	{
	  F1Diptar  = (F1Diptar + FADJ(500)) >> 1;
	  A1Diptar -= 4;
	  DBG(printf("rule 7.011a vowel nasalztn F1Diptar=%ld  A1Diptar=%ld\n",
	         (LONG)F1Diptar, (LONG)A1Diptar));
	}
      else
	{
	  F1.Target  = (F1.Target + FADJ(500)) >> 1;
	  A1.Target -= 4;
	  DBG(printf("rule  7.011b (vowel nasalztn) F1targ=%ld  A1targ=%ld\n",
		(LONG)F1.Target, (LONG)A1.Target));
	}


    if LAST_PC_IS(LATERAL)
      {
	A1.Bper = 75;
	A2.Bper = 75;
	A3.Bper = 75;
	DBG(printf("rule 7.012  Lateral precedes vowel: Bper[a1..a3] = 75"));
      }


/* == Centralization ==
   Shifts f1..a3 and diphthong targets towards a certain user specified
   phoneme by a fixed user specified percentage
*/
	if((iorb->flags & NDF_NEWIORB) && (centpct > 0))
	{
	  F1.Target += PCT((WORD)centpct, ((WORD)Parms[centphon].f1-F1.Target));
	  F2.Target += PCT((WORD)centpct, ((WORD)Parms[centphon].f2-F2.Target));
	  F3.Target += PCT((WORD)centpct, ((WORD)Parms[centphon].f3-F3.Target));
	  A1.Target += PCT((WORD)centpct, ((WORD)Parms[centphon].a1-A1.Target));
	  A2.Target += PCT((WORD)centpct, ((WORD)Parms[centphon].a2-A2.Target));
	  A3.Target += PCT((WORD)centpct, ((WORD)Parms[centphon].a3-A3.Target));
	  F1Diptar  += PCT((WORD)centpct, ((WORD)Parms[centphon].f1-F1Diptar));
	  F2Diptar  += PCT((WORD)centpct, ((WORD)Parms[centphon].f2-F2Diptar));
	  F3Diptar  += PCT((WORD)centpct, ((WORD)Parms[centphon].f3-F3Diptar));
	  A1Diptar  += PCT((WORD)centpct, ((WORD)Parms[centphon].a1-A1Diptar));
	  A2Diptar  += PCT((WORD)centpct, ((WORD)Parms[centphon].a2-A2Diptar));
	  A3Diptar  += PCT((WORD)centpct, ((WORD)Parms[centphon].a3-A3Diptar));
	}
} /* end Phonet70() */


void Phonet71(iorb)

struct narrator_rb *iorb;
{
	LONG	i;
	UWORD	Tcdiph, Tdmid;
	UWORD	F1Tcenter, F2Tcenter, F3Tcenter;
	UWORD	F1Tcdips,  F2Tcdips,  F3Tcdips;
	UWORD	A1Tcenter, A2Tcenter, A3Tcenter;
	UWORD	A1Tcdips,  A2Tcdips,  A3Tcdips;
	UWORD	artic;				/* local articulation coef */
extern	void	DrawDiphTrans();



    for (i = 0; i < NUMDIPHTRANS; ++i) if (Diphtran[i][0] == This_PC) break;
    Tcdiph = Diphtran[i][1];
    Tdmid  = Diphtran[i][2];
    DBG(printf("rule 7.1.1 Tcdiph=%ld Tdmid=%ld\n", (LONG)Tcdiph, (LONG)Tdmid));


    F1Tcenter = (Tdmid*F1.Segdur)/100;		/* Tdmid is a percentage */
    F2Tcenter = F1Tcenter;
    F3Tcenter = F1Tcenter;
    A1Tcenter = F1Tcenter;
    A2Tcenter = F1Tcenter;
    A3Tcenter = F1Tcenter;

    DBG(printf("rule 7.1.2:  Tcenter=%d \n",F1Tcenter));


/*  Tcdips   = Tcdiph * .5 * (1 + Segdur[f1] / 225ms)		*/
/*    F1Tcdips = (Tcdiph * (64 + (F1.Segdur << 6) / 27)) >> 7; */
    F1Tcdips = (Tcdiph * (F1.Segdur << 6) / 27) >> 7;
    F2Tcdips = F1Tcdips;
    F3Tcdips = F1Tcdips;
    A1Tcdips = F1Tcdips;
    A2Tcdips = F1Tcdips;
    A3Tcdips = F1Tcdips;

    DBG(printf("rule 7.1.3:  Tcdips = %d\n",F1Tcdips));


    if ((Last_PC == PC_HH) && THIS_PC_ISNOT(LAX))
      {
	F1Tcenter = (21*F1Tcenter) >> 5;
	F2Tcenter = F1Tcenter;
	F3Tcenter = F1Tcenter;
	A1Tcenter = F1Tcenter;
	A2Tcenter = F1Tcenter;
	A3Tcenter = F1Tcenter;
	DBG(printf("rule 7.1.4:  Tcenter = %d\n",F1Tcenter));
      }


    F1.Bvf = (F1.Target + F1Diptar) >> 1;
    F2.Bvf = (F2.Target + F2Diptar) >> 1;
    F3.Bvf = (F3.Target + F3Diptar) >> 1;
    A1.Bvf = (A1.Target + A1Diptar) >> 1;
    A2.Bvf = (A2.Target + A2Diptar) >> 1;
    A3.Bvf = (A3.Target + A3Diptar) >> 1;
    DBG(printf("rule 7.1.5:  f1-f3 Bvf = %d   %d   %d\n",F1.Bvf,F2.Bvf,F3.Bvf));
    DBG(printf("             a1-a3 Bvf = %d   %d   %d\n",A1.Bvf,A2.Bvf,A3.Bvf));


    if (This_PC == PC_OY)
      {
	F1.Bvf += FADJ(70);
	DBG(printf("rule 7.1.6:  f1 bvf = %d\n",F1.Bvf));
      }


    if ((This_PC == PC_OY) || (This_PC == PC_AY))
      {
	F2.Bvf += FADJ(200);
	F3.Bvf -= FADJ(200);
	DBG(printf("rule 7.1.7 and 7.1.8: F2 bvf = %d  F3 bvf = %d\n",
		F2.Bvf, F3.Bvf));
      }


    if (This_PC == PC_YU)
      {
	F3Tcenter = (10*F3Tcenter) >> 5;
	A3Tcenter = (10*A3Tcenter) >> 5;
 	DBG(printf("rule 7.1.9:  A3,F3 Tcenter = %d   %d\n",F3Tcenter,A3Tcenter));
      }
/*
    else if ((This_PC == PC_EXR) || (This_PC == PC_IXR))
      {
	F3Tcenter = (19*F3Tcenter) >> 5;
	A3Tcenter = (19*A3Tcenter) >> 5;
	DBG(printf("rule 7.1.10:  A3,F3 Tcenter = %d  %d\n",F3Tcenter,A3Tcenter));
      }
*/


    F1.Bvb = F1.Bvf;
    F2.Bvb = F2.Bvf;
    F3.Bvb = F3.Bvf;
    A1.Bvb = A1.Bvf;
    A2.Bvb = A2.Bvf;
    A3.Bvb = A3.Bvf;
    DBG(printf("rule 7.1.11:   Bvb's set to Bvf's\n"));



   /*
    *	Articulation of diphthongs.
    *   Smoothing adjusted according to user specified percentage
    */

    if((iorb->flags & NDF_NEWIORB) && (iorb->articulate != 100))
    {
	artic = (UWORD)iorb->articulate;
	F1Tcdips = PCT(artic, F1Tcdips);
	F2Tcdips = PCT(artic, F2Tcdips);
	F3Tcdips = PCT(artic, F3Tcdips);
	A1Tcdips = PCT(artic, A1Tcdips);
	A2Tcdips = PCT(artic, A2Tcdips);
	A3Tcdips = PCT(artic, A3Tcdips);
	DBG(printf("Adjusting Tcdips for articulation, artic=%d\n",artic));
    }


    DrawDiphTrans("F1", &F1, F1Tcenter, F1Diptar, F1Tcdips);
    DrawDiphTrans("F2", &F2, F2Tcenter, F2Diptar, F2Tcdips);
    DrawDiphTrans("F3", &F3, F3Tcenter, F3Diptar, F3Tcdips);
    DrawDiphTrans("A1", &A1, A1Tcenter, A1Diptar, A1Tcdips);
    DrawDiphTrans("A2", &A2, A2Tcenter, A2Diptar, A2Tcdips);
    DrawDiphTrans("A3", &A3, A3Tcenter, A3Diptar, A3Tcdips);
    
    DBG(printf("rule 7.1.12:  draw diphthong tracks\n"));

}


void DrawDiphTrans(DebugString, Track, Tcenter, Diptar, Tcdips)
	char		*DebugString;
struct 	PhonetParms	*Track;
	WORD		 Diptar;
	UWORD		 Tcdips;
	UWORD		 Tcenter;
{
extern	void	 PhonetInterp();
	WORD	*cumdur;
	WORD	 target;
	WORD	 oldval;
	UWORD	 mid;
	WORD	 bvb;
	WORD	 bvf;
	UBYTE	 segdur;
	UBYTE	 olddur;
	BYTE	 tcb;
	BYTE	 tcf;
	WORD	 start, incr;
	WORD	 i,j;



    DBG(printf("interpolating diphthong track %s\n",DebugString));
    DBG(printf("    target = %d   diptar = %d\n",Track->Target,Diptar));
    
    cumdur = Track->Cumdur;
    target = Track->Target;
    oldval = Track->Oldval;
    segdur = Track->Segdur;
    olddur = Track->Olddur;
    bvb    = Track->Bvb;
    bvf	   = Track->Bvf;

    for (mid = 0; mid < Tcenter;         mid++) *cumdur++ = target;
    for (mid = 0; mid < segdur-Tcenter;  mid++) *cumdur++ = Diptar;
    cumdur -= segdur - Tcenter;

    if ((tcb = MIN(Tcdips, Tcenter-1)) > 0)
      {
 	start  = *(cumdur - (tcb + 1)) << 5;
	incr   = ((bvb << 5) - start)/tcb;
	cumdur = cumdur - (LONG)tcb;
	while (tcb--)
	  {
	    start     += incr;
	    *cumdur++  = (start >= 0) ?  ( start + 16) >> 5 :
	  			       -((-start + 16) >> 5);
      	  }
      }


    if ((tcf = MIN(Tcdips, segdur-Tcenter-1)) > 0)
      {
	incr   = ((*(cumdur+tcf) - *(cumdur-1)) << 5)/tcf;   /*cumdur+tcf-1 */
	start  = (*(cumdur-1) << 5) + incr;
	while (tcf--)
	  {
	    *cumdur++  = (start >= 0) ?   ( start + 16) >> 5 :
			 		-((-start + 16) >> 5);
	    start    += incr;
	  }
      }
}
