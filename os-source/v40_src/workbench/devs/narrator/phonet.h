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

struct PhonetParms
{
  WORD	 *Cumdur;		/* Pointer to current segment		*/
  WORD	 *Local; 		/* Pointer to start of local interp area*/
  UBYTE   Olddur;		/* Duration of previous segment (frames)*/
  UBYTE	  Segdur;		/* Duration of current segment (frames) */
  UBYTE	  Trantype;		/* Transition type			*/
  UWORD	  Mintime;		/* Max back prop of smoothing (frames)	*/
  WORD	  Target;		/* Final value at Cumdur+Segdur		*/
  WORD	  Oldval;		/* Value at frame before Cumdur		*/
  BYTE	  Tcf;			/* Forward smoothing time (in frames)	*/
  BYTE	  Tcb;			/* Backward smoothing time (in frames)	*/
  WORD	  Bper;			/* Boundry percentage			*/
  WORD	  Bvf;			/* Forward boundry target		*/
  WORD	  Bvb;			/* Backward boundry target		*/
};


#undef NULL
#define NULL (void *)0


#define	LAST_PC_IS(Feature)	(Last_PCF & (Feature))
#define	THIS_PC_IS(Feature)	(This_PCF & (Feature))
#define	NEXT_PC_IS(Feature)	(Next_PCF & (Feature))

#define LAST_PC_ISNOT(Feature)	(!(Last_PCF & (Feature)))
#define THIS_PC_ISNOT(Feature)	(!(This_PCF & (Feature)))
#define NEXT_PC_ISNOT(Feature)	(!(Next_PCF & (Feature)))


#define MIN(X,Y)	((X) < (Y)) ? (X) : (Y)
#define MAX(X,Y)	((X) > (Y)) ? (X) : (Y)
#define PCT(A,B)	((A)*(B)/100)


#define DISCON	0
#define	DISSMO	1
#define SMODIS	2
#define SETSMO	3


#define	VOWELMANNER		0
#define	STOPMANNER		1
#define FRICMANNER		2
#define SONMANNER		3
#define NUMMANNERCLASSES	4


#define NUMTCOBSTS		30		/* added DX and J*/
#define NUMBURDURS		15
#define NUMDIPHTRANS		21

#define MAXF0LEN		128		/* must match gloequs.i */

#define	SYLLSTART	(1 <<	7)   /* Definitions of This_bits etc. bits */	
#define	POLYSYLL	(1 <<	6)
#define SYLSTRESSED	(1 <<	5)
#define SEGSTRESSED	(1 <<	4)
#define WORDFINAL	(1 <<	3)
#define	PHRASEFINAL	(1 <<	2)


#ifdef PRINTDEBUGINFO
#define DBG(X)		(X)
#else
#define DBG(X)
#endif
