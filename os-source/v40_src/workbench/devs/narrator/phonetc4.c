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

#define A1BIAS	-3
#define A2BIAS	0
#define A3BIAS	0

void PhonetC4()
{
UBYTE	PC;
UWORD	temp;

   /*
    *  (C.4)  Initialization.
    *
    */

   /*
    *  (C.4.1)  Set manner class.
    */

    PrevManner = Manner;

    if THIS_PC_IS(VOWEL) Manner = VOWELMANNER;
      else if THIS_PC_IS(STOP) Manner = STOPMANNER;
	else if THIS_PC_IS(FRICATIVE) Manner = FRICMANNER;
	  else Manner = SONMANNER;


   /*
    *  (C.4.2)  Set freq and amp targets from Tables C.1 and C.2.  
    *           If this seg is HH or HX, use targets from next segment.
    */

    PC = ((This_PC == PC_HH) || (This_PC == PC_HX)) ? Next_PC : This_PC;


    F1.Target = Parms[PC].f1; 
    F2.Target = Parms[PC].f2; 
    F3.Target = Parms[PC].f3; 
    
    A1.Target = Parms[PC].a1 + A1BIAS;
    A2.Target = Parms[PC].a2 + A2BIAS; 
    A3.Target = Parms[PC].a3 + A3BIAS; 

    AV.Target = Parms[This_PC].av;

    if((This_bits & PHRASEFINAL) && !This_PCS && THIS_PC_IS(VOICED))
    {
	AV.Target -= 2;
	DBG(printf("ad hoc: Unstressed phrase final voiced seg AV down 2db\n"));
    }


    AF.Target = 0;
    if THIS_PC_IS(FRICATIVE) 
      {
	switch (This_PC)
          {
	    case PC_S :     AF.Target = 50;	break;
	    case PC_SH:     AF.Target = 51;	break;
	    case PC_F :     AF.Target = 43;	break;
	    case PC_TH:     AF.Target = 41;	break;
	    case PC_V :     AF.Target = 45;	break;
	    case PC_DH:     AF.Target = 42;	break;
	    case PC_Z :     AF.Target = 53;	break;
	    case PC_ZH:     AF.Target = 48;	break;
	  }
	if (!This_PCS)
	{
	    AF.Target -= 3;		/* Ad hoc rule added 8/16/88 */
	    if(This_bits & PHRASEFINAL)  AF.Target -= 3;
	}
      }


    switch (This_PC)
      {
	case PC_HH:   AH.Target = 52;	break;
	case PC_HX:   AH.Target = 52;	break;
	case PC_WH:   AH.Target = 52;	break;
	default:      AH.Target =  0;	break;
      }

    MH.Target = Parms[This_PC].height;
    MW.Target = Parms[This_PC].width;



    F0.Segdur = This_dur;
    F1.Segdur = This_dur;
    F2.Segdur = This_dur;
    F3.Segdur = This_dur;
    A1.Segdur = This_dur;
    A2.Segdur = This_dur;
    A3.Segdur = This_dur;
    AF.Segdur = This_dur;
    AV.Segdur = This_dur;
    AH.Segdur = This_dur;
    MH.Segdur = This_dur;
    MW.Segdur = This_dur;


    /* Set Tcf from table C.3. */

    AV.Tcf = 3;
    AF.Tcf = 5;		/* was 4  */
    AH.Tcf = 3;		/* was 4  */
    F0.Tcf = 14;	/* was 12 */
    F1.Tcf = 8;		/* These all should be 10, but I just can't
    F2.Tcf = 8;		/*  bring myself to do it		    */
    F3.Tcf = 8;
    A1.Tcf = 8;
    A2.Tcf = 8;
    A3.Tcf = 8;
    MH.Tcf = 8;
    MW.Tcf = 8;
			
    Buramp = 0;
    if (THIS_PC_IS(PLOSIVE) || (This_PC == PC_DX)) switch (This_PC)
      {
	case  PC_B :	Buramp = 44;	break;
	case  PC_D :	Buramp = 46;	break;
	case  PC_G :	Buramp = 43;	break;
	case  PC_GX:	Buramp = 43;	break;
	case  PC_DX:	Buramp = 40;	break;
	case  PC_P :	Buramp = 42;	break;
	case  PC_T :	Buramp = 44;	break;
	case  PC_K :	Buramp = 46;	break;	/* was 44 */
	case  PC_KX:	Buramp = 46;	break;	/* was 44 */
	case  PC_J :	Buramp = 44;	break;
	case  PC_CH:	Buramp = 41;	break;
      }


    Aspamp   = 51;
    Aspdux   = 0;

    AV.Trantype = SETSMO;
    AF.Trantype = SETSMO;
    AH.Trantype = SETSMO;
    F0.Trantype = SETSMO;
    F1.Trantype = SETSMO;
    F2.Trantype = SETSMO;
    F3.Trantype = SETSMO;
    A1.Trantype = SETSMO;
    A2.Trantype = SETSMO;
    A3.Trantype = SETSMO;
    MH.Trantype = SETSMO;
    MW.Trantype = SETSMO;

    temp     = BperTable[PrevManner][Manner];
    AV.Bper  = temp;
    AF.Bper  = temp;
    AH.Bper  = temp;
    F0.Bper  = temp;
    F1.Bper  = temp;
    F2.Bper  = temp;
    F3.Bper  = temp;
    A1.Bper  = temp;
    A2.Bper  = temp;
    A3.Bper  = temp;
    MH.Bper  = temp;
    MW.Bper  = temp;
}
