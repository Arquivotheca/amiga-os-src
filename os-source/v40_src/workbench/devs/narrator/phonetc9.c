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


BOOL	MannerSwap;


void PhonetC9()
{
UBYTE	Temp_PC;			/* Temp storage of PC		*/
ULONG	Temp_PCF;			/* Temp PC features		*/
UBYTE	Temp_PCS;			/* Temp PC stress		*/

UBYTE	Prev_PC;			/* Save last PC			*/
ULONG	Prev_PCF;			/*      last PC features	*/
UBYTE	Prev_PCS;			/* 	last PC stress		*/

UBYTE	Curr_PC;			/* Save current PC		*/
ULONG	Curr_PCF;			/*	current PC features	*/
UBYTE	Curr_PCS;			/* 	current PC stress	*/

WORD	temp;

extern	void	Phonet91();
extern	void	Phonet92();


    DBG(printf("Entering C.9 -- Adjustments\n"));


    MannerSwap = FALSE;
    if (Manner > PrevManner)
      {
	Prev_PC  = Last_PC;			/* Save previous PC	   */
	Curr_PC  = This_PC;			/* Save current PC	   */
	Temp_PC  = This_PC;			/* Swap curr and prev PC's */
	This_PC  = Last_PC;
	Last_PC  = Temp_PC;

	Prev_PCF = Last_PCF;			/* Save previous PCF	   */
	Curr_PCF = This_PCF;			/* Save current PCF        */
	Temp_PCF = This_PCF;			/* Swap cur and prev PCF's */
	This_PCF = Last_PCF;
	Last_PCF = Temp_PCF;

	Prev_PCS = Last_PCS;			/* Save previous PCS	   */
	Curr_PCS = This_PCS;			/* Save current PCS   	   */
	Temp_PCS = This_PCS;			/* Swap cur and prev PCS's */
	This_PCS = Last_PCS;
	Last_PCS = Temp_PCS;

	temp = F1.Target;    F1.Target = F1.Oldval;    F1.Oldval = temp;
	temp = F2.Target;    F2.Target = F2.Oldval;    F2.Oldval = temp;
	temp = F3.Target;    F3.Target = F3.Oldval;    F3.Oldval = temp;
	temp = A1.Target;    A1.Target = A1.Oldval;    A1.Oldval = temp;
	temp = A2.Target;    A2.Target = A2.Oldval;    A2.Oldval = temp;
	temp = A3.Target;    A3.Target = A3.Oldval;    A3.Oldval = temp;

	MannerSwap = TRUE;
	DBG(printf("rule c.9.1 swapping current and prev segments\n"));
      }


    if THIS_PC_IS(RETRO)
      {
	F2.Bper = 75;
	F3.Bper = 75;
	A2.Bper = 75;
	A3.Bper = 75;
	DBG(printf("rule 9.2: bper[f2..a3] <-- 75 / [+retro]\n"));
	DBG(printf("                                [  --- ]\n\n"));
      }


    if (LAST_PC_IS(PALATAL) && LAST_PC_ISNOT(SONORANT))
      {
	F2.Bper  = 20;
	A2.Bper  = 20;

	F2.Tcf  += 3;
	A2.Tcf  += 3;
	DBG(printf("rule 9.3: Bper[f2,a2]<--20, Tcf^30/ [+pal,-son] --- \n"));
	DBG(printf("                                    [    S    ]\n\n"));
      }


    if (LAST_PC_IS(STOP) && THIS_PC_ISNOT(STOP) && LAST_PC_ISNOT(AFFRICATE)
	&& (Last_PC != PC_DX))   Phonet91();


    if (MannerSwap)
      {
	Last_PC  = Prev_PC;			/* Restore swapped segments  */
	Last_PCF = Prev_PCF;
	Last_PCS = Prev_PCS;

	This_PC  = Curr_PC;
	This_PCF = Curr_PCF;
	This_PCS = Curr_PCS;

	temp = F1.Target;    F1.Target = F1.Oldval;    F1.Oldval = temp;
	temp = F2.Target;    F2.Target = F2.Oldval;    F2.Oldval = temp;
	temp = F3.Target;    F3.Target = F3.Oldval;    F3.Oldval = temp;
	temp = A1.Target;    A1.Target = A1.Oldval;    A1.Oldval = temp;
	temp = A2.Target;    A2.Target = A2.Oldval;    A2.Oldval = temp;
	temp = A3.Target;    A3.Target = A3.Oldval;    A3.Oldval = temp;


   /*
    *	Complement percentages to account for target swapping
    */

	F1.Bper = 100 - F1.Bper;
	F2.Bper = 100 - F2.Bper;
	F3.Bper = 100 - F3.Bper;
	A1.Bper = 100 - A1.Bper;
	A2.Bper = 100 - A2.Bper;
	A3.Bper = 100 - A3.Bper;

	DBG(printf("restoring swapped segments at end of C.9 rules\n"));
      }


    if (LAST_PC_IS(RGLIDE) && THIS_PC_IS(STOP) && THIS_PC_IS(VELAR))
      {
	F2.Oldval = FADJ(1400);
	F3.Oldval = FADJ(1600);
	DBG(printf("ad hoc: rglide after velar stop,F2,3.Oldval =1400,1600\n"));
      }


    Phonet92();

}



void Phonet91()
{
    F1.Bper = 50;
    A1.Bper = 50;

    F2.Bper = 0;
    A2.Bper = 0;

    F3.Bper = 0;
    A3.Bper = 0;

    DBG(printf("rule 9.1.1: Bper[f1,a1] <-- 50\n\n"));
    DBG(printf("rule 9.1.2: Bper[f2,a2] <--  0\n\n"));
    DBG(printf("rule 9.1.3: Bper[f3,a3] <--  0\n\n"));



    if LAST_PC_IS(NASAL)
      {
	F1.Bper = 0;
	A1.Bper = 0;
	DBG(printf("rule 9.1.4: Bper[f1,a1] <-- 0 / [+nasal] --- \n"));
	DBG(printf("                                [   S  ]\n\n"));
      }


    if (LAST_PC_IS(LABIAL) && THIS_PC_ISNOT(GLOTTAL))
      {
	if THIS_PC_IS(FRONT)
	  {
	    F2.Bper = 65;
	    F3.Bper = 20;
	    A2.Bper = 65;
	    A3.Bper = 20;
	    DBG(printf("rule 9.1.5:\n"));
	  }
        else
          {
	    F2.Bper = 20;
	    F3.Bper = 70;
	    A2.Bper = 20;
	    A3.Bper = 70;
 	    DBG(printf("rule 9.1.6\n"));
	  }

        if THIS_PC_IS(RETRO)
	  {
	    F3.Oldval = FADJ(1750);
	    F3.Bper   = 20;
	    A3.Bper   = 20;
	    DBG(printf("rule 9.1.7\n"));
	  }

      }



    if (LAST_PC_IS(ALVEOLAR) && THIS_PC_IS(LATERAL))
      {
	F2.Oldval = FADJ(1050);
	DBG(printf("rule 9.1.8\n"));
      }



    if ((Last_PC == PC_N) && THIS_PC_ISNOT(LATERAL))
    {      
	if(THIS_PC_IS(ROUND) && MannerSwap)	/* eg /UW N/ */
	{
	    F2.Bper = 50;
	    A2.Bper = 50;
	}	
	else F2.Oldval = FADJ(1600);		/* was 1400 */
	DBG(printf("rule 9.1.9\n"));
    }	



    if (LAST_PC_IS(ALVEOLAR) && (Last_PC != PC_N) && (Last_PC != PC_DX) &&
	THIS_PC_ISNOT(LATERAL))
      {
	F2.Oldval = FADJ(1700);
	A2.Oldval = Parms[Last_PC].a2;
	if THIS_PC_IS(FRONT)  F2.Oldval += FADJ(150);
	DBG(printf("rule 9.1.10\n"));
      }



    if (LAST_PC_IS(ALVEOLAR) && THIS_PC_IS(RETRO))
      {
	F3.Oldval = FADJ(2300);
	DBG(printf("rule 9.1.11\n"));
      }



    if (LAST_PC_IS(ALVEOLAR) && THIS_PC_ISNOT(RETRO))
      {
	F3.Oldval = FADJ(2620);
	DBG(printf("rule 9.1.12\n"));
      }



    if LAST_PC_IS(VELAR)
      {
	F2.Oldval = F2.Target + F1.Target - FADJ(300);
	DBG(printf("rule 9.1.13\n"));
      }



    if LAST_PC_IS(VELAR)
      if THIS_PC_IS(LABIAL)
	{
	  F3.Oldval = F2.Oldval + FADJ(800);
	  DBG(printf("rule 9.1.14\n"));
        }
      else
        {
	  F3.Oldval = F2.Oldval + FADJ(400);
          DBG(printf("rule 9.1.15\n"));
        }



/*
    if (LAST_PC_IS(VELAR) && THIS_PC_IS(FRONT))
      {
	F2.Oldval = Parms[PC_K].f2;
	A2.Oldval = Parms[PC_K].a2;
	DBG(printf("rule 9.1.16 use KP targets \n"));
      }


    if LAST_PC_IS(VELAR)
      {
	if (!MannerSwap)
	  {
	    F3.Oldval = F3.Target;
	    A3.Oldval = A3.Target;
	  }
	DBG(printf("rule 9.1.17 set F3,A3 boundary for velars at target\n"));
      }	
*/

/* Boundary is 50% going into unvoiced velar stop

    if(MannerSwap && LAST_PC_IS(VELAR) && LAST_PC_ISNOT(VOICED))
      {
	F2.Oldval = (Oldval_f2 + F2.Oldval) >> 1;
	F3.Oldval = (Oldval_f3 + F3.Oldval) >> 1;
	A2.Oldval = (Oldval_a2 + A2.Oldval) >> 1;
	A3.Oldval = (Oldval_a3 + A3.Oldval) >> 1;
      }
*/

    if (Last_PC == PC_NX)
      {
	F2.Oldval = (F2.Oldval + F2.Target) >> 1;
	F3.Oldval = (F3.Oldval + F3.Target) >> 1;
	A2.Oldval = (A2.Oldval + A2.Target) >> 1;
	A3.Oldval = (A3.Oldval + A3.Target) >> 1;
	DBG(printf("rule 9.1.17\n"));
      }



    if (LAST_PC_IS(VELAR) && ((This_PC == PC_IY) || (This_PC == PC_IH)))
      {
	F2.Oldval += FADJ(250);
	F3.Oldval += FADJ(50);
	DBG(printf("rule 9.1.18: F2.Oldval up 250, F3.Oldval up 50 \n"));
      }

}



void Phonet92()
{


    if (LAST_PC_IS(ALVEOLAR) && LAST_PC_IS(STOP) && (This_PC == PC_W))
      {
	F2.Oldval = FADJ(1200);
	F3.Oldval = FADJ(2050);
	DBG(printf("rule 9.2.1\n"));
      }


    if (LAST_PC_IS(RGLIDE) || LAST_PC_IS(RETRO))
      if THIS_PC_IS(ALVEOLAR)
        {
	  F2.Target = FADJ(1850);
	  F3.Target = FADJ(2200);
  	  DBG(printf("rule 9.2.2\n"));
        }
      else if THIS_PC_IS(VELAR)
	{
	  F2.Target = FADJ(1700);
	  F3.Target = FADJ(1900);
	  DBG(printf("rule 9.2.3\n"));
	}



    if (LAST_PC_IS(VELAR) && THIS_PC_IS(RETRO))
      {
	F2.Oldval = FADJ(1100);
	F3.Oldval = FADJ(1500);
	DBG(printf("rule 9.2.4\n"));
      }



    if (LAST_PC_IS(ALVEOLAR) && THIS_PC_IS(RETRO))
      {
	F3.Oldval = FADJ(2100);
	DBG(printf("rule 9.2.5\n"));
      }



    if (LAST_PC_IS(LABIAL) && THIS_PC_IS(RETRO))
      {
	F3.Oldval = FADJ(1700);
	DBG(printf("rule 9.2.6\n"));
      }



    if THIS_PC_IS(VELAR)
      if LAST_PC_IS(WGLIDE)
        {
	  F2.Target = FADJ(900);
	  F3.Target = FADJ(2000);
	  DBG(printf("rule 9.2.7a\n"));
        }
/*
      else if (LAST_PC_ISNOT(RGLIDE) && LAST_PC_ISNOT(RETRO))
      	{
	  F2.Target = FADJ(1900);
	  DBG(printf("rule 9.2.7b\n"));
        }
*/


    if (LAST_PC_IS(ALVEOLAR) && LAST_PC_IS(STOP) && THIS_PC_IS(F2BACK))
      {
	F2.Oldval = FADJ(1850);
	DBG(printf("rule 9.2.8\n"));
      }

}

