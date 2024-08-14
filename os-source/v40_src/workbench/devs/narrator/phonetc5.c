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


void PhonetC5()
{
LONG	delta, incr, i;
BYTE	temp;


    DBG(printf("Entering C.5 -- General rules\n"));


   /*
    *  (C.5) General rules.
    */

    F0.Bper = 75;
    DBG(printf("rule c.5.1  set F0 bper to 75 (all contexts)\n"));


    if LAST_PC_IS(PLOSIVE)
      {
	AF.Mintime = 0;
	DBG(printf("rule c.5.2 set AF mintime to AF cumdur\n"));
      }


    if LAST_PC_ISNOT(VOICED)
      {
	F0.Trantype = DISSMO;
	AV.Trantype = DISSMO;
	DBG(printf("rule c.5.3.1 set trantype f0 and av to dissmo\n"));

/*	F0.Mintime = 0; */
      }


/*
    if (THIS_PC_IS(SILENT) && LAST_PC_ISNOT(SILENT))
      {
	Aspdux = 3;
	Aspam1 = Aspamp;
	DBG(printf("rule c.5.4  do a breathy offset into a pause\n"));
      }
*/


/*
    if (LAST_PC_IS(SONORANT) && THIS_PC_ISNOT(VOICED))
      {
	Aspdux = 1;
	DBG(printf("rule c.5.5 \n"));
      }
*/


    if (LAST_PC_IS(SONORANT) && THIS_PC_ISNOT(VOICED) && THIS_PC_IS(FRICATIVE))
      {
	AF.Cumdur  -= 2;
	AF.Segdur  += 2;
	AF.Mintime -= 2;
     	DBG(printf("rule c.5.6  start frication early\n"));
      }


/*
    if (LAST_PC_IS(SONORANT) && THIS_PC_ISNOT(VOICED) && THIS_PC_IS(STOP))
      {
	Aspam1 -= 6;
     	DBG(printf("rule c.5.7  \n"));
      }
*/


/*  
    if (Aspdux != 0)
      {
	AH.Cumdur -= 8*(Aspdux+3);
	incr	   = ((Aspam1 - *AH.Cumdur) << 5)/3;
	delta	   = *AH.Cumdur << 5;
	for (i=0; i<3; ++i)
	  {
	    AH.Cumdur  += 8;
	    *AH.Cumdur  = delta >> 5;
	    delta      += incr;
	  }
	DBG(printf("rule 5.8:  no used\n"));
      }
*/


    /*  Moving rule 5.9 after 5.13.b */



/*			F0 handled by phonetf0
    if (THIS_PC_ISNOT(VOICED) && LAST_PC_ISNOT(VOICED))
      {
	F0.Trantype = DISCON;
	DBG(printf("Rule C.5.10\n"));
      }


    if (THIS_PC_ISNOT(VOICED) && LAST_PC_IS(VOICED))
      {
	F0.Trantype = SMODIS;
	DBG(printf("Rule C.5.11\n"));
      }
*/


    if (THIS_PC_ISNOT(STOP) && LAST_PC_IS(STOP))
      {
	for (i=0; i<NUMTCOBSTS; ++i) if (Tcobst[i][0] == Last_PC) break;
	temp   = Tcobst[i][1];
	F1.Tcf = temp;
	F2.Tcf = temp;
	F3.Tcf = temp;
	A1.Tcf = temp;
	A2.Tcf = temp;
	A3.Tcf = temp;
	DBG(printf("Rule C.5.12  PC=%lx  f1..f3,a1..a3 Tcf=%ld  index=%ld\n",
		(LONG)Tcobst[i][0], (LONG)Tcobst[i][1], i));
      }


    if (THIS_PC_ISNOT(STOP) && THIS_PC_IS(VOICED))
      if LAST_PC_IS(STOP)				/* RULE MODIFIED */
        {
	  AV.Trantype = LAST_PC_IS(NASAL) ? DISCON : DISSMO;
	  AF.Trantype = DISSMO;
	  AH.Trantype = SETSMO;
	  F1.Trantype = DISSMO;
	  F2.Trantype = DISSMO;
	  F3.Trantype = DISSMO;
	  A1.Trantype = DISSMO;
	  A2.Trantype = DISSMO;
	  A3.Trantype = DISSMO;
	  DBG(printf("Rule C.5.13.a \n"));
        }



    if THIS_PC_ISNOT(VOICED)
      {
	AV.Trantype = SMODIS;
	DBG(printf("Rule c.5.9\n"));
      }



    if (LAST_PC_IS(FRICATIVE) && THIS_PC_IS(SONORANT))
      {
	AF.Trantype = SMODIS;
	DBG(printf("Rule c.5.14: New rule.. af.trantype now smodis\n"));
      }

}
