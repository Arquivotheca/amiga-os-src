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

extern UBYTE *f0start;
extern UBYTE *f0peak;
extern UBYTE *f0end;
extern UBYTE *f0cr;
extern BOOL  syllstart;
extern BOOL  termassign;

#define F0LEFT(x)	*(coef + 7) = (x)
#define F0MIDDLE(x)	*(coef + 7 + ((this_dur >> 1) << 3)) = (x)
#define F0RIGHT(x)	*(coef + 7 + ((this_dur -  1) << 3)) = (x)
#define F0NEXTMIDDLE(x)	*(coef+7+((this_dur<<3)+((next_dur>>1)<<3))) = (x)

void PhonetF0(coef)
UBYTE *coef;

{
BOOL phrasefinal, lastvoiced;
WORD this_dur, next_dur;

this_dur = (WORD)This_dur;
next_dur = (WORD)Next_dur;
phrasefinal = (This_bits & PHRASEFINAL) ? TRUE : FALSE;
lastvoiced = (NEXT_PC_IS(SILENT) || NEXT_PC_ISNOT(VOICED) ||
	     (NEXT_PC_IS(STOP) && NEXT_PC_ISNOT(NASAL)))   ? TRUE : FALSE;


if(This_bits & SYLLSTART)
{
    if (LAST_PC_IS(GLOTTAL) && THIS_PC_IS(VOICED))
    {
	F0start = 90 >> 1;			/* 2 Hz steps 		*/
	f0start++;
    }
    else  F0start = (WORD)*f0start++;
    F0peak  = (WORD)*f0peak++;
    F0end   = (WORD)*f0end++;
    ThisCR  = ((WORD)*f0cr++ & 0x0070) >> 2;	/* isolated CR# * 8 (in 2 Hz)*/
    syllstart = TRUE;
    termassign = FALSE;
}

/* if term was assigned, just continue with f0end */
if(termassign)
{
    if(ThisCR)  F0RIGHT(F0end + ThisCR);
    else	F0RIGHT(F0end);
    return();
}


if(syllstart)
{
    termassign = FALSE;
    if THIS_PC_IS(VOICED)
    {
	syllstart = FALSE;
	if THIS_PC_IS(SYLLABIC)  F0LEFT(F0start);
	else if LAST_PC_IS(GLOTTAL)
	{
	    F0LEFT(F0start);
	    F0MIDDLE((F0start + F0peak) >> 1);
	}
	else if(phrasefinal && NEXT_PC_IS(SYLLABIC))  F0MIDDLE(F0start);
	else F0RIGHT(F0start);
    }
}

/* Peak assignments */

if THIS_PC_IS(SYLLABIC)
{
    if(LAST_PC_IS(PLOSIVE) && LAST_PC_ISNOT(VOICED))  F0LEFT(F0peak);
    else if(This_PCS)
    {					/* stressed peak */
	if(phrasefinal)  F0LEFT(F0peak);
	else if(Next_bits & SYLLSTART)  F0MIDDLE(F0peak);
	else F0RIGHT(F0peak);
    }
    else
    {					/* unstressed peak */
	if(phrasefinal)  F0LEFT(F0peak);
	else		 F0MIDDLE(F0peak);
    }
    if(!phrasefinal && !ThisCR)
    {					/* assign fall */
	if(Next_bits & SYLLSTART)  F0RIGHT(F0end);
	else			   F0NEXTMIDDLE(F0end);
    }
}

if(phrasefinal && lastvoiced && !ThisCR)
{   F0RIGHT(F0end);				/* assign term */
    termassign = TRUE;
    return();
}

if(phrasefinal && ThisCR)			/* assign continuation rises */
{
    if THIS_PC_IS(SYLLABIC)  F0MIDDLE(F0end);
    if(lastvoiced)
    {
	F0RIGHT(F0end + ThisCR);
	termassign = TRUE;
    }
    return();
}

}  /* end phonetf0() */
