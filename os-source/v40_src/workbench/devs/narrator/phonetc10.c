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


void PhonetC10(iorb)
struct narrator_rb *iorb;
{

WORD temp;
UWORD utemp;

extern void PhonetC101();


    /* Obstruent tests with segment reversal */
    if (THIS_PC_ISNOT(SONORANT) || THIS_PC_IS(NASAL|GLOTTAL) ||
	This_PC == PC_UL)  PhonetC101(Last_PC, This_PC, FALSE);

    else if (LAST_PC_ISNOT(SONORANT) || LAST_PC_IS(NASAL|GLOTTAL) ||
	     Last_PC == PC_UL)  PhonetC101(This_PC, Last_PC, TRUE);


   /*
    *  (C.10 Modifications)
    */


    if (!This_PCS)
      {
	AV.Target -= 2;
	DBG(printf("rule 10.01: PC not stressed reduce AV targ by 2 now =%d\n",
		AV.Target));
      }


    if (THIS_PC_IS(PLOSIVE) && THIS_PC_IS(VOICED) &&
	(LAST_PC_ISNOT(VOICED) || LAST_PC_ISNOT(SONORANT)))
      {
	AV.Target = 0;
	DBG(printf("c10: voiced plos preceded by non-sonor, AV.Target=0\n"));
      }


    /*  C.10.1  -- Target[f1..f3] <- avg(Target,Oldval,Nextar)/[+DX] */

    if (This_PC == PC_DX)
      {
	F1.Target = (F1.Target + F1.Oldval + Parms[Next_PC].f1) / 3;
	utemp = ((UWORD)F2.Target<<1)+(UWORD)F2.Oldval+(UWORD)Parms[Next_PC].f2;
	F2.Target = utemp >> 2; 
	F3.Target = (F3.Target + F3.Oldval + Parms[Next_PC].f3) / 3;
	A1.Target = (A1.Target + A1.Oldval + Parms[Next_PC].a1) / 3;
	temp = ((WORD)A2.Target<<1)+(WORD)A2.Oldval+(WORD)Parms[Next_PC].a2;
	A2.Target = temp >> 2;
	A3.Target = (A3.Target + A3.Oldval + Parms[Next_PC].a3) / 3;
	DBG(printf("C.10.1   F2[DX] is now double weighted\n"));
      }
 

/*** SPECIAL CASE SPECIFIC CODE ***/


    if (THIS_PC_IS(DENTAL) && LAST_PC_IS(VOWEL))
    {
	F3.Bper = 100;
	A3.Bper = 100;
    }

    if (LAST_PC_IS(DENTAL) && THIS_PC_IS(VOWEL))
    {
	F3.Bper = 0;
	A3.Bper = 0;
    }

/*** END OF SPECIAL CASE SPECIFIC CODE *** Insert more above this line */


    /*  C.10.2  -- F2 transitions involving f2back segments are shorter */ 

    if LAST_PC_IS(F2BACK)
      {
	F2.Tcf -= 2;
	A2.Tcf -= 2;
	DBG(printf("rule 10.2 f2 transitions shorter in f2back segments\n"));
      }


    /*  C.10.3 -- Minimum formant separation of f2..f4 is 200 Hz */

    if ((F2.Target - F1.Target) < FADJ(200)) F2.Target = F1.Target + FADJ(200);
    if ((F3.Target - F2.Target) < FADJ(200)) F3.Target = F2.Target + FADJ(200);


    /*  C.10.4 -- Set boundary values and Tcb */

/*  F0.Tcb = F0.Tcf;		   F0.Tcb set in phonetF0()	*/
    F1.Tcb = F1.Tcf;	 	/* C.10.4.1 Assign Tcb's	*/
    F2.Tcb = F2.Tcf;
    F3.Tcb = F3.Tcf;
    A1.Tcb = A1.Tcf;
    A2.Tcb = A2.Tcf;
    A3.Tcb = A3.Tcf;
    AF.Tcb = AF.Tcf;
    AH.Tcb = AH.Tcf;
    AV.Tcb = AV.Tcf;
    MH.Tcb = MH.Tcf;
    MW.Tcb = MW.Tcf;

						
/* #define BVALF(X)  X.Bvf = (X.Bper*X.Target + (100 - X.Bper)*X.Oldval)/100 */

/* Simplified boundary value formula */
#define BVALF(X)     X.Bvf = X.Oldval + (X.Bper * (X.Target - X.Oldval)) / 100

    BVALF(F0);				/* C.10.4.3 Assign Bvf's	*/
    BVALF(F1);
    BVALF(F2);
    BVALF(F3);
    BVALF(A1);
    BVALF(A2);
    BVALF(A3);
    BVALF(MH);
    BVALF(MW);
/*
    BVALF(AF);				taken care of later
    BVALF(AH);
    BVALF(AV);
*/

    F0.Bvb = F0.Bvf;			/* C.10.4.3 Assign Bvb's	*/
    F1.Bvb = F1.Bvf;
    F2.Bvb = F2.Bvf;
    F3.Bvb = F3.Bvf;
    A1.Bvb = A1.Bvf;
    A2.Bvb = A2.Bvf;
    A3.Bvb = A3.Bvf;
    MH.Bvb = MH.Bvf;
    MW.Bvb = MW.Bvf;


    /*  C.10.5  --  Discontinuous formant jump in a lateral release */

    if LAST_PC_IS(LATERAL)
      {
	F1.Bvf += FADJ(50);
	F2.Bvf += FADJ(50);

	F1.Bvb -= FADJ(50);
	F2.Bvb -= FADJ(50);

	if THIS_PC_IS(VOWEL)  AV.Trantype = DISSMO;
	DBG(printf(" C.10.5\n"));
      }



   /*
    *	C.10.7 -- Special treatment for amplitude parms.
    *	 	  NOTE:  AH.Bvf is set to AH.Target to provide for
    *		         backward only smoothing in the given context.
    */

    AV.Bvf = (AV.Target + AV.Oldval)/2;
    AF.Bvf = (AF.Target + AF.Oldval)/2;
    if (THIS_PC_IS(SONORANT) && LAST_PC_IS(FRICATIVE) && (Aspdur > 0))
	 AH.Bvf = AH.Target;
    else AH.Bvf = (AH.Target + AH.Oldval)/2;
    AV.Bvb = AV.Bvf;
    AF.Bvb = AF.Bvf;
    AH.Bvb = AH.Bvf;
    DBG(printf("C.10.7\n"));



    /*  C.10.8 -- Vowel amplitude offset more gradual */

    if LAST_PC_IS(VOWEL)
      {
	AV.Tcb += 1;				/* was set to 3 */
	DBG(printf("C.10.8\n"));
      }


    /*  C.10.9  --  Voicing offset in stop or fric more gradual, in onset too */
    /*  This now acts as the general offset rule.  New specific rules follow  */


    if (THIS_PC_ISNOT(VOICED) && THIS_PC_ISNOT(SONORANT) && LAST_PC_IS(VOICED))
      {
	AV.Bvb -= 6;
	AV.Bvf  = AV.Bvb;
	DBG(printf("C.10.9.1\n"));
      }

/*
 *  New rules regarding AV   8/10/88
 *  These rules cover the release of plosives voiced & unvoiced
 *  and unvoiced fricatives
 */

    if (THIS_PC_IS(VOICED) && THIS_PC_IS(SONORANT))
    {
	if (LAST_PC_IS(PLOSIVE) && LAST_PC_IS(VOICED))
	{
	    AV.Bvf = AV.Target - 10;
	    AV.Tcf = 1;		/* release of voiced stop has rapid onset */
	}
	else if ( LAST_PC_IS(PLOSIVE) ||
		 (LAST_PC_IS(FRICATIVE) && LAST_PC_ISNOT(VOICED)) )
		        AV.Bvf = AV.Target - 6;
    }


    /* AV offset rule for flap */

    if(Last_PC == PC_DX)
    {
	AV.Tcf = 1;
	AV.Tcb = 1;
    }


    /* AV offset rules */
    
    if (LAST_PC_IS(VOICED) && LAST_PC_IS(SONORANT))
    {
	if THIS_PC_IS(FRICATIVE)
	{
	    AV.Trantype = SETSMO;
	    if THIS_PC_ISNOT(VOICED)	/* unvoiced fricative */
	    {
		AV.Bvf = AV.Oldval - 14;
		AV.Tcb = 5;
	    }
	    else  AV.Tcb = 8;		/* voiced fricative */
	}
	else if THIS_PC_IS(PLOSIVE)
	{
	    AV.Tcb = 4;
	    if THIS_PC_IS(VOICED)  AV.Bvf = AV.Oldval - 18; /* voiced plosive */
	    else
	    {
		AV.Tcb = 1;		/* unvoiced plosive */
		AV.Bvf = AV.Oldval - 12;
		AV.Trantype = SMODIS;
	    }
	}
	else if((This_PC == PC_PE) || (This_PC == PC_QM))  /* sentence ending */
	{
	    AV.Tcb = MIN(Last_dur >> 1, 8);  /* min of 50% of sonor's dur & 8 */
	    AV.Bvf = 20;  /* for a more gradual fade (20 is below audibility) */
	}
	else if(This_PC == PC_DX)	/* AV onset rule for flap */
	{
	    AV.Tcf = 1;
	    AV.Tcb = 1;
	}
    }		/* end AV offset rule */
	
    AV.Bvb = AV.Bvf;	  




    /*  C.10.10  --  Glottal segments have no inherent "articulartory" targs */

    if (LAST_PC_IS(GLOTTAL) || LAST_PC_IS(SILENT))
      {
	F1.Bvf = F1.Bvb = F1.Oldval = F1.Target;
	F2.Bvf = F2.Bvb = F2.Oldval = F2.Target;
	F3.Bvf = F3.Bvb = F3.Oldval = F3.Target;
	A1.Bvf = A1.Bvb = A1.Oldval = A1.Target;
	A2.Bvf = A2.Bvb = A2.Oldval = A2.Target;
	A3.Bvf = A3.Bvb = A3.Oldval = A3.Target;
	DBG(printf("C.10.10 leading glot segs have no inherent artic targs\n"));
      }


    if THIS_PC_IS(SILENT)  /* [. ? , - QX] inherit the Last_PC artic targs */
      {
	F1.Target = F1.Bvb = F1.Bvf = F1.Oldval;
	F2.Target = F2.Bvb = F2.Bvf = F2.Oldval;
	F3.Target = F3.Bvb = F3.Bvf = F3.Oldval;
	A1.Target = A1.Bvb = A1.Bvf = A1.Oldval;
	A2.Target = A2.Bvb = A2.Bvf = A2.Oldval;
	A3.Target = A3.Bvb = A3.Bvf = A3.Oldval;
      }



    /*  C.10.11 - Stops have abrupt fricative offset */

    if LAST_PC_IS(PLOSIVE)
      {
	AF.Bvf = AF.Target;
	DBG(printf("C.10.11\n"));
      }

/*					don't remember why
    if LAST_PC_IS(AFFRICATE)
      {
	AF.Trantype = SETSMO;
	AF.Bvb = AF.Bvf;
	DBG(printf("C.10.111: LastPC affric closure, AF.Trantype = SETSMO\n"));
      }
*/

    if LAST_PC_IS(FRICATIVE)
      {
	if THIS_PC_IS(STOP) AF.Tcb = 2;
	else if THIS_PC_IS(SILENT) AF.Tcb = 9;

	AF.Tcb = MAX(MIN((WORD)(Last_dur)-5, AF.Tcb), 0);
	DBG(printf("rule c.10.12 (ad hoc) adjust AF.Tcb, now=%d\n",AF.Tcb));


	if THIS_PC_ISNOT(FRICATIVE)
	  {
	    AF.Bvf = AF.Bvb = AF.Oldval - 20;
	    DBG(printf("rule c.10.13 (ad hoc) adj af bvf/b, now=%d\n",AF.Bvf));
	  }

      }	



    if (LAST_PC_ISNOT(FRICATIVE) && THIS_PC_IS(FRICATIVE))
      {
	AF.Bvf = AF.Bvb = AF.Target - 15;
	DBG(printf("rule c.10.14 (ad hoc) adjust af.bvb/f, now =%d\n",AF.Bvf));
      }



/***** ULTRA-SPECIAL VERY SPECIFIC CASE HANDLING CODE *****/

    if(((Last_PC == PC_W) || (Last_PC == PC_Y)) && THIS_PC_IS(VOWEL))
    {
	F1.Tcb = 4;   F2.Tcb = 4;   /* F3.Tcb = 4; */
	A1.Tcb = 4;   A2.Tcb = 4;   /* A3.Tcb = 4; */
	F1.Tcf = 6;   F2.Tcf = 6;   /* F3.Tcf = 6; */
	A1.Tcf = 6;   A2.Tcf = 6;   /* A3.Tcf = 6; */
	DBG(printf("c.10.15 (ad hoc): F1-2,A1-2 tcb = 4  F1-2,A1-2 tcf = 6\n"));
    }

    if (((This_PC == PC_W) || (This_PC == PC_Y)) && LAST_PC_IS(VOWEL))
    {
	F1.Tcb = 6;   F2.Tcb = 6;   /* F3.Tcb = 6; */
	A1.Tcb = 6;   A2.Tcb = 6;   /* A3.Tcb = 6; */
	F1.Tcf = 4;   F2.Tcf = 4;   /* F3.Tcf = 4; */
	A1.Tcf = 4;   A2.Tcf = 4;   /* A3.Tcf = 4; */
	DBG(printf("c.10.16 (ad hoc): F1-2,A1-2 tcb = 6  F1-2,A1-2 tcf = 4\n"));
    }


/*  Offset rule for vowel/stop combinations   (ad hoc 2/4/91) */
    if (THIS_PC_IS(STOP) && LAST_PC_IS(VOWEL))
    {
	F1.Tcb = 2;
	AV.Tcb = 2;
	DBG(printf("c.10.16 (ad hoc) PC STOP, last PC VOWEL: F1,AV tcb set to 2\n"));
    }


    if ((iorb->flags & NDF_NEWIORB) && (iorb->articulate == 0))
      {
	F1.Trantype = DISCON;
	F2.Trantype = DISCON;
	F3.Trantype = DISCON;
	A1.Trantype = DISCON;
	A2.Trantype = DISCON;
	A3.Trantype = DISCON;
	AV.Trantype = DISCON;
	AF.Trantype = DISCON;
	AH.Trantype = DISCON;
      }
    MH.Trantype = F1.Trantype;		/* Match mouth timing to F1 */
    MW.Trantype = F1.Trantype;

}  /* end PhonetC10() */


/*
 *   Subroutine PhonetC101()
 *   Assigns Tcf's to obstruents
 */

void PhonetC101(lastseg,thisseg,short_f1) /** thisseg = obstruent on entry */
BOOL  short_f1;
UBYTE lastseg,thisseg;
{

WORD i, temp;

   /*
    *	The following was transplanted (plus mods) from phonetc8.
    */
    DBG(printf("PhonetC101:  assigning TCF to obstruent:"));
    DBG(printf("  last seg=%c%c", Parms[lastseg].digraph >> 8,
		Parms[lastseg].digraph & 0xFF));
    DBG(printf("    this seg=%c%c\n", Parms[thisseg].digraph >> 8,
		Parms[thisseg].digraph & 0xFF));


/*
    if (Parms[lastseg].features & HIGH)
      {
	F1.Tcf = 3;
	F2.Tcf = 2;
	F3.Tcf = 2;
	A1.Tcf = 3;
	A2.Tcf = 2;
	A3.Tcf = 2;
	DBG(printf("rule 8.3.2:\n  f1,a1 tcf now 2   f2,f3,a2,a3 tcf now 1\n"));
      }
    else
*/
      {
	for (i = 0;  i < NUMTCOBSTS;  ++i) if (thisseg == Tcobst[i][0]) break;
	temp = Tcobst[i][1];
	if (short_f1 && LAST_PC_IS(PLOSIVE))
	     F1.Tcf = (temp + 2) >> 1;
	else F1.Tcf = temp;
	F2.Tcf = temp;
	F3.Tcf = temp;
	A1.Tcf = F1.Tcf;
	A2.Tcf = temp;
	A3.Tcf = temp;
	DBG(printf("rule 8.3.3: f1,a1 tcf=%d   f2-a3 tcf=%d\n",F1.Tcf,F2.Tcf));
      }


}  /* end PhonetC101() */
