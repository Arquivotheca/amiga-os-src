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

/*
		Extern declarations for Phonet sub modules
*/

extern	struct	PhonetParms  F0;
extern	struct	PhonetParms  F1;
extern	struct	PhonetParms  F2;
extern	struct	PhonetParms  F3;
extern	struct	PhonetParms  A1;
extern	struct	PhonetParms  A2;
extern	struct	PhonetParms  A3;
extern	struct	PhonetParms  AV;
extern	struct	PhonetParms  AF;
extern	struct	PhonetParms  AH;
extern	struct	PhonetParms  MH;
extern	struct	PhonetParms  MW;

extern	UBYTE	Old_PC;			/* Phone left of Last_PC	    */
extern	ULONG	Old_PCF;		/* Features of said phone 	    */

extern	UBYTE	Last_PC;		/* Previous segment's phoneme code  */
extern	ULONG	Last_PCF;		/* Features of previous phoneme	    */
extern	UBYTE	Last_PCS;		/* Stress of previous phoneme	    */
extern	UBYTE	Last_dur;		/* Duration of previous phoneme	    */
extern	UBYTE	Last_bits;		/* Bits for F0			    */

extern	UBYTE	This_PC;		/* This segment's phoneme code	    */
extern	ULONG	This_PCF;		/* Features of current phoneme	    */
extern	UBYTE	This_PCS;		/* Stress of current phoneme	    */
extern	UBYTE	This_dur;		/* Duration of current phoneme	    */
extern	UBYTE	This_bits;

extern	UBYTE	Next_PC;		/* Next segment's phoneme code	    */
extern	ULONG	Next_PCF;		/* Features of next phoneme         */
extern	UBYTE	Next_PCS;		/* Stress of next phoneme	    */
extern	UBYTE	Next_dur;		/* Duration of next phoneme   	    */
extern	UBYTE	Next_bits;

extern	WORD Manner, PrevManner;
extern	WORD Burstdur, Buramp, Aspamp, Aspdux, Aspam1, Aspdur;

extern WORD LastF0posn, ThisF0posn;	/* F0 position assignments	    */
extern WORD LastCR, ThisCR;		/* Continuation rises		    */
extern WORD F0delta, F0start, F0peak, F0end;  /* F0 values per syllable	    */

extern	UBYTE BperTable[NUMMANNERCLASSES][NUMMANNERCLASSES];
extern  UBYTE Tcobst[NUMTCOBSTS][2];
extern  UBYTE Burdur[NUMBURDURS][2];
extern	UBYTE Diphtran[NUMDIPHTRANS][3];

/*extern	BYTE  AVbfr[]; */

extern	WORD	*LocalF1;		/* Local bfr pointers	*/
extern	WORD	*LocalF2;
extern	WORD	*LocalF3;
extern	WORD	*LocalA1;
extern	WORD 	*LocalA2;
extern	WORD	*LocalA3;
extern	WORD 	*LocalAV;
extern	WORD 	*LocalAF;
extern	WORD 	*LocalAH;
extern	WORD 	*LocalF0;


extern 	struct PhonemeParms	Parms[];
