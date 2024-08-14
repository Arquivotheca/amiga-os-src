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


void PhonetC8()
{
extern	void	Phonet80();
extern	void	Phonet81();
extern	void	Phonet82();
extern	void	Phonet83();
extern	void	Phonet84();


    DBG(printf("Entering C.8 -- Obstruent rules\n"));

   /*
    *  Well, there don't seem to be any C.8 rules.  Guess we will just
    *  skip on to the C.8.1 rules
    */

    if (THIS_PC_IS(PLOSIVE) || THIS_PC_IS(FRICATIVE)) Phonet81();
    if THIS_PC_IS(FRICATIVE)			      Phonet82();
    if THIS_PC_IS(STOP)				      Phonet83();
    if THIS_PC_IS(NASAL)			      Phonet84();
	
}


void Phonet81()
{

    DBG(printf("rule 8.1.1: zarfed\n"));



    if (THIS_PC_IS(VELAR) && NEXT_PC_ISNOT(VOWEL))
      {
	Buramp -= 5;
	DBG(printf("rule c.8.1.5\n"));
      }


    if ((LAST_PC_ISNOT(VOICED) || LAST_PC_ISNOT(SONORANT))
	&& THIS_PC_IS(VOICED) && THIS_PC_IS(PLOSIVE))
      {
	AV.Target = 0;
	DBG(printf("rule 8.1.6 av.target = 0\n"));
      }

}


void Phonet82()
{

    if NEXT_PC_IS(SILENT)
      {
	AF.Target -= 4;
	DBG(printf("rule c.8.2.1:  new af.target=%ld\n",(LONG)AF.Target));
      }


/*	removed 8/16/88 af now adjusted in c10 rules
    if (LAST_PC_ISNOT(FRICATIVE) && LAST_PC_ISNOT(STOP) && NEXT_PC_IS(VOWEL))
      if (AF.Segdur < 10)
 	{
	  AF.Target += (10 - AF.Segdur)*AF.Target/AF.Segdur;
	  DBG(printf("rule c.8.2.2: short fric adj af.tar now=%d\n",AF.Target));
	}
*/


    if (LAST_PC_IS(VOWEL) && THIS_PC_IS(VOICED) && NEXT_PC_IS(VOWEL))
      {
	AV.Target += 3;
	DBG(printf("rule 8.2.3: av.target increase by 3 to %ld\n",AV.Target));
      }


    if (THIS_PC_IS(PALATAL) && NEXT_PC_IS(VOWEL) && NEXT_PC_IS(ROUND))
      {
	F2.Target -= FADJ(50);
	F3.Target -= FADJ(300);
	DBG(printf("rule c.8.2.4:\n   new f2.target=%ld  f3.target=%ld\n",
		(LONG)F2.Target, (LONG)F3.Target));
      }

}


void Phonet83()
{
LONG	oldAF, incr, i;
WORD	tempdur, temp;


    if LAST_PC_ISNOT(STOP)
      {
	AV.Trantype = SMODIS;
	AF.Trantype = SMODIS;
	AH.Trantype = SMODIS;
	F1.Trantype = SMODIS;
	A1.Trantype = SMODIS;
	F2.Trantype = SETSMO;
	F3.Trantype = SETSMO;
	A2.Trantype = SETSMO;
	A3.Trantype = SETSMO;
	DBG(printf("rule 8.3.1:\n  av,af,ah,f1,a1 trantypes now smodis\n"));
	DBG(printf("             f2,f3,a2,a3 trantypes now setsmo\n"));
      }

    if LAST_PC_IS(HIGH)
      {
	F1.Tcf = 2;
	F2.Tcf = 1;
	F3.Tcf = 1;
	A1.Tcf = 2;
	A2.Tcf = 1;
	A3.Tcf = 1;
	DBG(printf("rule 8.3.0.2"));
      }
    else
      {
	for (i = 0;  i < NUMTCOBSTS;  ++i) if (Tcobst[i][0] == This_PC) break;
	temp   = Tcobst[i][1];
	F1.Tcf = (temp + 2) >> 1;
	F2.Tcf = temp;
	F3.Tcf = temp;
	A1.Tcf = (temp + 2) >> 1;
	A2.Tcf = temp;
	A3.Tcf = temp;
	DBG(printf("rule 8.3.3\n  f1,a1 tcf=%d   f2,f3,a2,a3 tcf=%d\n",
		(WORD)F1.Tcf, (WORD)F2.Tcf));
      }


   /*
    *	C.8.3.1 rules
    */

    if (THIS_PC_IS(AFFRICATE) || (This_PC == PC_DX) ||
	(THIS_PC_IS(PLOSIVE) && 
	(NEXT_PC_ISNOT(SILENT) || NEXT_PC_IS(NASAL))))
/*	(NEXT_PC_ISNOT(STOP) && NEXT_PC_ISNOT(SILENT) || NEXT_PC_IS(NASAL)))) */
      {
	for (i = 0; i < NUMBURDURS; ++i) if (This_PC == Burdur[i][0]) break;
	Burstdur = Burdur[i][1];
	DBG(printf("rule c.8.3.1.1:\n  burstdur=%ld\n", (LONG)Burstdur));



	if ((This_PC == PC_T) && NEXT_PC_IS(RETRO))
	  {
	    Burstdur += 1;
	    DBG(printf("rule c.8.3.1.2:\n  burstdur=%ld\n", (LONG)Burstdur));
	  }


	if (THIS_PC_IS(AFFRICATE) && This_PCS && Next_PC != PC_RR)
	  {
	    if (This_PC == PC_J) Burstdur = 8;
	    else Burstdur = 12;
	    DBG(printf("rule c.8.3.1.2.5 stressed j/ch burstdur=70/100msec\n"));
	  }


	if((Last_PC == PC_S) && (Last_bits & SYLLSTART))
	  {
	    Burstdur = 2;
	    DBG(printf("ad hoc: /S/ precedes plos in same syll: Burstdur=2\n"));
	  }


	Burstdur = MIN(Burstdur, AF.Segdur);
	DBG(printf("rule c.8.3.1.3:\n  burstdur=%ld\n", (LONG)Burstdur));


	Burstdur = MAX(Burstdur, 1);
        DBG(printf("rule c.8.3.1.4:\n  burstdur=%ld\n", (LONG)Burstdur));



	if (*(AF.Cumdur-1) != 0)
	  {
	    *--AF.Cumdur -= 20;				/* shoud dbl check */
	    oldAF       = *(AF.Cumdur - 3) << 5;
	    incr        = ((*AF.Cumdur << 5) - oldAF)/3;
	    AF.Cumdur  -= 2;
	    for (i=0; i<3; ++i)
	      {
	    	oldAF	     += incr;
	    	*AF.Cumdur++  = oldAF >> 5;
	      }
	    DBG(printf("rule c.8.3.1.5:    smoothed back 30 msec in af\n"));
	  }



	tempdur = AF.Segdur - Burstdur;
	AF.Mintime = tempdur;
	DBG(i = tempdur);
	while (tempdur--) *AF.Cumdur++  = 0;
	AF.Segdur = Burstdur;
	DBG(printf("rule c.8.3.1.6:   drew af seg of 0 for dur=%ld\n",
		(LONG)(i)));



	if THIS_PC_ISNOT(VOICED)
	  {
	    Buramp += 6;
	    DBG(printf("rule c.8.3.1.7:   new Buramp=%ld\n",(LONG)Buramp));
	  }


	if NEXT_PC_IS(STOP)
	  {
	    Buramp -= 3;
	    DBG(printf("c.8.3.1.7.5 ad hoc: nxt pc stop,buramp-3=%d\n",Buramp));
	  }


	if (LAST_PC_IS(NASAL) && !This_PCS)
	  {
	    Buramp -= 6;
/*	    if(This_bits & PHRASEFINAL)  Buramp -= 3; */
	    DBG(printf("rule c.8.3.1.8:   new Buramp=%ld\n",(LONG)Buramp));
	  }


	
	if (LAST_PC_ISNOT(NASAL) && !This_PCS)
	  {
	    Buramp -= 3;
/*	    if(This_bits & PHRASEFINAL)  Buramp -= 3; */
	    DBG(printf("rule c.8.3.1.9:   new Buramp=%ld\n",(LONG)Buramp));
	  }



	if ((Next_PC == PC_AXP) || NEXT_PC_IS(FRICATIVE))
	  {
	    Buramp -= 3;
	    DBG(printf("rule c.8.3.1.10:   new Buramp=%ld\n",(LONG)Buramp));
	  }



	if((Last_PC == PC_S) && (Last_bits & SYLLSTART))
	  {
	    Buramp -= 5;
	    DBG(printf("ad hoc: /S/ precedes plos in same syll: Buramp -=5\n"));
	  }



	AF.Target = Buramp;
	DBG(printf("rule c.8.3.1.11:  AF target = %ld\n",Buramp));



	AF.Trantype = DISCON;
	DBG(printf("rule c.8.3.1.12:  af trantype now discon\n"));


      }					/* end of c.8.3.1 rules */

}



void Phonet84()
{

    if THIS_PC_ISNOT(SYLLABIC)
      {
	AF.Trantype = SMODIS;
	AH.Trantype = SMODIS;
	F1.Trantype = SMODIS;
	F2.Trantype = SMODIS;
	F3.Trantype = SMODIS;
	A1.Trantype = SMODIS;
	A2.Trantype = SMODIS;
	A3.Trantype = SMODIS;
	DBG(printf("rule c.8.4.2:    af..a3 trantypes now smodis\n"));



	AV.Trantype = DISCON;
	DBG(printf("rule c.8.4.4:    av trantype now discon\n"));
      }



    if LAST_PC_ISNOT(VOICED)
      {
	AV.Trantype = DISSMO;
	DBG(printf("rule c.8.4.5:    av trantype now dissmo\n"));
      }


}
    
