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
#include "narrator.h"

#define AVBIAS		   0		/* AV bias control		    */
#define AFBIAS		   0		/* duh				    */
#define AHBIAS		   3		/* duh				    */
#define LOCAL_BFR_SIZE	 128
#define	MIN_DBAMP	  23		/* Minimum amplitude for audibility */

/* Flags byte in coef buffer 6(An) */
#define	FF_PHONSTART	0X01
#define	FF_WORDSTART	0X02
#define FF_SYLSTART	0X04
#define FF_UNUSED	0X08
#define FF_NASAL_OR_VB	0X10
#define	FF_ASP		0X20
#define FF_FRIC		0X40
#define	FF_VOICED	0X80

#define NCOEFBYTES	8		/* Number of bytes in coef frame */
#define OF_F0		7		/* Offset to F0 element		 */

struct	PhonetParms  F0;
struct	PhonetParms  F1;
struct	PhonetParms  F2;
struct	PhonetParms  F3;
struct	PhonetParms  A1;
struct	PhonetParms  A2;
struct	PhonetParms  A3;
struct	PhonetParms  AV;
struct	PhonetParms  AF;
struct	PhonetParms  AH;
struct	PhonetParms  MH;
struct	PhonetParms  MW;

#define NTRACKS 12			/* Number of Phonet tracks	    */


UBYTE	Old_PC;				/* Phon to the left of Last Phon    */
ULONG	Old_PCF;			/* Features of phon 2 to the left   */

UBYTE	Last_PC;			/* Previous segment's phoneme code  */
ULONG	Last_PCF;			/* Features of previous phoneme	    */
UBYTE	Last_PCS;			/* Stress of previous phoneme	    */
UBYTE	Last_dur;			/* Duration of previous phoneme	    */
UBYTE	Last_bits;			/* Syll & seg bits for F0           */

UBYTE	This_PC;			/* This segment's phoneme code	    */
ULONG	This_PCF;			/* Features of current phoneme	    */
UBYTE	This_PCS;			/* Stress of current phoneme	    */
UBYTE	This_dur;			/* Duration of current phoneme	    */
UBYTE	This_bits;			/* Syll & seg bits for F0           */ 

UBYTE	Next_PC;			/* Next segment's phoneme code	    */
ULONG	Next_PCF;			/* Features of next phoneme         */
UBYTE	Next_PCS;			/* Stress of next phoneme	    */
UBYTE	Next_dur;			/* Duration of next phoneme   	    */
UBYTE	Next_bits;			/* Syll & seg bits for F0           */

BOOL syllstart,termassign;		/* Flags for F0			    */
WORD LastCR, ThisCR;			/* Continuation rises		    */
WORD F0delta, F0start, F0peak, F0end;	/* F0 values per syllable	    */

WORD Manner, PrevManner;
WORD Burstdur, Buramp, Aspamp, Aspdux, Aspam1, Aspdur;

UBYTE BperTable[NUMMANNERCLASSES][NUMMANNERCLASSES] = 
  {
    50, 35, 50, 75,
    65, 50, 50, 65,
    50, 50, 50, 75,
    25, 35, 25, 50
  };

UBYTE Tcobst[NUMTCOBSTS][2] =
  {
    {PC_B,   6},  {PC_CH, 10},  {PC_D,   8},  {PC_DH,  8},  {PC_DX, 4},
    {PC_UL,  8},  {PC_UM,  6},  {PC_UN,  8},  {PC_F,   6},  {PC_J, 10},
    {PC_G,  10},  {PC_GX, 10},  {PC_HH,  8},  {PC_HX,  8},
    {PC_KX, 10},  {PC_K,  10},  {PC_M,   6},  {PC_NX, 10},
    {PC_N,   8},  {PC_P,   8},  {PC_Q,   8},  {PC_SH, 10},
    {PC_QX , 8},  {PC_S,   8},  {PC_TH,  8},  {PC_TQ,  8},
    {PC_T,   8},  {PC_V,   6},  {PC_ZH, 10},  {PC_Z,   8}
  };


UBYTE Burdur[NUMBURDURS][2] =
  {
    {PC_B,  1},  {PC_CH, 7},  {PC_D,  1},  {PC_DX, 1},  {PC_G,  1},
    {PC_GX, 1},  {PC_J,  6},  {PC_KX, 2},  {PC_K,  2},  {PC_M,  0},
    {PC_NX, 0},  {PC_N,  0},  {PC_P,  1},  {PC_TQ, 0},  {PC_T,  2}
  };


UBYTE Diphtran[NUMDIPHTRANS][3] =
  {
    {PC_AA,   0, 80},  {PC_AE,  10, 75},  {PC_AH,   0,  0},
    {PC_AO,  11, 80},  {PC_AW,  12, 70},  {PC_AXR, 13, 40},
    {PC_AY,  10, 55},  {PC_EH,   6, 70},  {PC_ER,  18, 50},
    {PC_EXR, 10, 65},  {PC_EY,  14, 55},  {PC_IH,   9, 65},
    {PC_IXR, 10, 50},  {PC_IY,  20, 45},  {PC_OW,  12, 50},
    {PC_OXR, 11, 60},  {PC_OY,  10, 60},  {PC_UH,   9, 65},
    {PC_UW,  14, 55},  {PC_UXR, 15, 50},  {PC_YU,  10, 45}
  };


UBYTE PlosAspID[][6] =
  {      /*    P       T       K       KX      CH    */
    {PC_IY, ASP_EH, ASP_IH, ASP_IY, ASP_IH, ASP_IH},
    {PC_IXR,ASP_IH, ASP_IH, ASP_IY, ASP_IH, ASP_IH},
    {PC_Y,  ASP_IH, ASP_IH, ASP_IY, ASP_IH, ASP_IH},
    {PC_YU, ASP_IH, ASP_IH, ASP_IY, ASP_IH, ASP_IH},
    {PC_IH, ASP_AXP,ASP_IH, ASP_IH, ASP_IH, ASP_IH},
    {PC_IX, ASP_IH, ASP_IH, ASP_IH, ASP_IH, ASP_IH},
    {PC_EXR,ASP_AXP,ASP_EH, ASP_IH, ASP_IH, ASP_EH}, /* k,kx was eh */
    {PC_EY, ASP_AXP,ASP_EH, ASP_IH, ASP_IH, ASP_EH}, /* k,kx was eh */
    {PC_EH, ASP_AXP,ASP_EH, ASP_IH, ASP_IH, ASP_EH}, /* k,kx was eh */
    {PC_AE, ASP_AXP,ASP_EH, ASP_IH, ASP_EH, ASP_EH},
    {PC_AXP,ASP_AXP,ASP_AXP,ASP_IH, ASP_AXP,ASP_AXP},
    {PC_AW, ASP_AH, ASP_AE, ASP_IH, ASP_AH, ASP_AH},
    {PC_UH, ASP_UH, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_UW, ASP_UW, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_UXR,ASP_UW, ASP_UW, ASP_IH, ASP_UW ,ASP_UW},
    {PC_OW, ASP_AH, ASP_AH, ASP_IH, ASP_AH, ASP_AH},
    {PC_AH, ASP_AH, ASP_AH, ASP_IH, ASP_AH, ASP_AH},
    {PC_AX, ASP_AH, ASP_AH, ASP_IH, ASP_AH, ASP_AH},
    {PC_AA, ASP_AH, ASP_AXP,ASP_IH, ASP_AXP,ASP_AH},
    {PC_AXR,ASP_UH, ASP_AXP,ASP_IH, ASP_AXP,ASP_AH}, /* p was ah */
    {PC_AY, ASP_AH, ASP_AXP,ASP_IH, ASP_AXP,ASP_AH},
    {PC_AO, ASP_UH, ASP_AXP,ASP_IH, ASP_AXP,ASP_AH},
    {PC_ER, ASP_ER, ASP_ER, ASP_ER, ASP_ER, ASP_ER},
    {PC_RR, ASP_ER, ASP_ER, ASP_ER, ASP_ER, ASP_ER},
    {PC_OY, ASP_OW, ASP_AH, ASP_IH, ASP_OW, ASP_AH},
    {PC_OXR,ASP_OW, ASP_AH, ASP_IH, ASP_OW, ASP_AH},
    {PC_L,  ASP_UW, ASP_OW, ASP_IH, ASP_AXP,ASP_UH}, /* p was uw */
    {PC_UL, ASP_OW, ASP_OW, ASP_IH, ASP_AXP,ASP_UH},
    {PC_W,  ASP_UW, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_WH, ASP_UW, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_M,  ASP_UW, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_N,  ASP_UW, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_NX, ASP_UW, ASP_AXP,ASP_IH, ASP_AXP,ASP_UH},
    {PC_HH, ASP_AXP,ASP_UH, ASP_IH, ASP_AXP,ASP_UH},
    {PC_HX, ASP_AXP,ASP_UH, ASP_IH, ASP_AXP,ASP_UH},
    {PC_UM, ASP_AXP,ASP_UH, ASP_IH, ASP_AXP,ASP_UH},
    {PC_UN, ASP_AXP,ASP_UH, ASP_IH, ASP_AXP,ASP_UH}
  };


WORD 	 LocalBfr[NTRACKS*LOCAL_BFR_SIZE];	/* Local buffer		*/

WORD	*LocalF1;				/* Local buffer ptrs	*/
WORD	*LocalF2;
WORD	*LocalF3;
WORD	*LocalA1;
WORD 	*LocalA2;
WORD	*LocalA3;
WORD 	*LocalAV;
WORD 	*LocalAF;
WORD 	*LocalAH;
WORD 	*LocalF0;
WORD 	*LocalMH;
WORD 	*LocalMW;



WORD	*TempF1, *TempF2, *TempF3;
WORD	*TempA1, *TempA2, *TempA3;
WORD	*TempAV, *TempAF, *TempAH;
WORD    *TempF0, *TempMH, *TempMW;

UBYTE	*f0start, *f0peak, *f0end, *f0cr;	/* These pointers are global. */


void Phonet(phon, stress, dur, coef, F0START, F0PEAK, F0END, F0CR, mouths,
	    iorb, centphon, centpct)
	  UBYTE	  *phon, *stress, *dur;
register  UBYTE	  *coef;
	  UBYTE	  *F0START, *F0PEAK, *F0END, *F0CR;
	  UBYTE	  *mouths;
struct	  narrator_rb	*iorb;
	  ULONG	   centphon, centpct;
{
extern 	struct PhonemeParms	Parms[];
extern	void			PhonetInterp();
extern	void			FillBuffer();
extern	int			PrintParms();
extern	void			AdjustTrack();
extern	void		   	PhonetC4();
extern	void		   	PhonetC5();
extern	void		   	PhonetC6();
extern	void		        PhonetC7();
extern	void		   	PhonetC8();
extern	void		   	PhonetC9();
extern	void		   	PhonetC10();
extern	void			PhonetF0();
	UBYTE 			firstdur;
	WORD			i,j,k,index;
	WORD			half_dur;
	WORD			artic;		/* local articulation coef */
	ULONG			flags;
	UBYTE			fricid;
	UBYTE			aspid;
	UBYTE			nasalid;
	BOOL			nasal_f;
	BOOL			sylnasal_f;
	BOOL			voiced_plosive_f;
	BOOL			use_left_fricid;
	BOOL			wordfinal;
	BOOL			mouthflag;



   /*  
    *  The lowercase array names are for the GLOBAL pointers. The uppercase
    *  names are for the args passed to Phonet from main.
    */

    f0start = F0START;
    f0peak  = F0PEAK;
    f0end   = F0END;
    f0cr    = F0CR;


    termassign = syllstart = FALSE;

   /*
    *	Initialization.
    *	   Set Local to beginning of local interp area.
    *	   Initialize first frame with QX targets.
    */

    LocalF1 = &LocalBfr[0*LOCAL_BFR_SIZE];	/* Initialize local bfr ptrs */
    LocalF2 = &LocalBfr[1*LOCAL_BFR_SIZE];
    LocalF3 = &LocalBfr[2*LOCAL_BFR_SIZE];
    LocalA1 = &LocalBfr[3*LOCAL_BFR_SIZE];
    LocalA2 = &LocalBfr[4*LOCAL_BFR_SIZE];
    LocalA3 = &LocalBfr[5*LOCAL_BFR_SIZE];
    LocalAV = &LocalBfr[6*LOCAL_BFR_SIZE];
    LocalAF = &LocalBfr[7*LOCAL_BFR_SIZE];
    LocalAH = &LocalBfr[8*LOCAL_BFR_SIZE];
    LocalF0 = &LocalBfr[9*LOCAL_BFR_SIZE];
    LocalMH = &LocalBfr[10*LOCAL_BFR_SIZE];
    LocalMW = &LocalBfr[11*LOCAL_BFR_SIZE];

    F1.Local  = F1.Cumdur  = LocalF1;
    F2.Local  = F2.Cumdur  = LocalF2;
    F3.Local  = F3.Cumdur  = LocalF3;
    A1.Local  = A1.Cumdur  = LocalA1;
    A2.Local  = A2.Cumdur  = LocalA2;
    A3.Local  = A3.Cumdur  = LocalA3;
    AV.Local  = AV.Cumdur  = LocalAV;
    AF.Local  = AF.Cumdur  = LocalAF;
    AH.Local  = AH.Cumdur  = LocalAH;
    F0.Local  = F0.Cumdur  = LocalF0;
    MH.Local  = MH.Cumdur  = LocalMH;
    MW.Local  = MW.Cumdur  = LocalMW;

    firstdur  = *dur & 0X3F;			/* Assign old durations	*/
    F1.Olddur = firstdur;
    F2.Olddur = firstdur;
    F3.Olddur = firstdur;
    A1.Olddur = firstdur;
    A2.Olddur = firstdur;
    A3.Olddur = firstdur;
    AV.Olddur = firstdur;
    AF.Olddur = firstdur;
    AH.Olddur = firstdur;
    F0.Olddur = firstdur;
    MH.Olddur = firstdur;
    MW.Olddur = firstdur;

    while (firstdur--)
      {
	*F1.Cumdur++ = Parms[PC_QX].f1;
	*F2.Cumdur++ = Parms[PC_QX].f2;
	*F3.Cumdur++ = Parms[PC_QX].f3;
	*A1.Cumdur++ = Parms[PC_QX].a1;
	*A2.Cumdur++ = Parms[PC_QX].a2;
	*A3.Cumdur++ = Parms[PC_QX].a3;
	*AV.Cumdur++ = 0;
	*AF.Cumdur++ = 0;
	*AH.Cumdur++ = 0;
	*F0.Cumdur++ = 110 >> 1;
	*MH.Cumdur++ = Parms[PC_M].height;
	*MW.Cumdur++ = Parms[PC_M].width;
	*(coef + NCOEFBYTES*firstdur + OF_F0) = 110 >> 1;
      }

#ifdef DEBUG
    firstdur = LOCAL_BFR_SIZE - (*dur & 0X3F);
    TempF1 = F1.Cumdur;
    TempF2 = F2.Cumdur;
    TempF3 = F3.Cumdur;
    TempA1 = A1.Cumdur;
    TempA2 = A2.Cumdur;
    TempA3 = A3.Cumdur;
    TempAV = AV.Cumdur;
    TempAF = AF.Cumdur;
    TempAH = AH.Cumdur;
    TempF0 = F0.Cumdur;
    TempMH = MH.Cumdur;
    TempMW = MW.Cumdur;
	  
    while (firstdur--)
      {
	*TempF1++ = -99;
	*TempF2++ = -99;
	*TempF3++ = -99;
	*TempA1++ = -99;
	*TempA2++ = -99;
	*TempA3++ = -99;
	*TempAV++ = -99;
	*TempAF++ = -99;
	*TempAH++ = -99;
	*TempF0++ = -99;
	*TempMH++ = -99;
	*TempMW++ = -99;
      }
#endif


    F1.Oldval = *(F1.Cumdur - 1);		/* Assign Oldvals	*/
    F2.Oldval = *(F2.Cumdur - 1);
    F3.Oldval = *(F3.Cumdur - 1);
    A1.Oldval = *(A1.Cumdur - 1);
    A2.Oldval = *(A2.Cumdur - 1);
    A3.Oldval = *(A3.Cumdur - 1);
    AV.Oldval = *(AV.Cumdur - 1);
    AF.Oldval = *(AF.Cumdur - 1);
    AH.Oldval = *(AH.Cumdur - 1);
    F0.Oldval = *(F0.Cumdur - 1);
    MH.Oldval = *(MH.Cumdur - 1);
    MW.Oldval = *(MW.Cumdur - 1);

    F0.Target = 110 >> 1;


    DBG(printf("coef=%lx    dur=%lx after first qx \n",coef,dur));


    Last_PC  = PC_QX;
    Last_PCF = Parms[Last_PC].features;
    Last_bits= 0;

    This_PC  = *phon++;
    This_PCF = Parms[This_PC].features;
    This_PCS = *stress   & 0X10;
    This_bits= *stress++ & 0xf0;
    This_bits |= (*dur & 0xc0) >> 4;
    This_dur = *dur++ & 0X3F;

    Next_PC  = *phon++;
    Next_PCF = Parms[Next_PC].features;
    Next_PCS = *stress   & 0X10;
    Next_bits= *stress++ & 0xf0;
    Next_bits |= (*dur & 0xc0) >> 4;
    Next_dur = *dur++ & 0X3F;

    ThisCR = LastCR = 0;			/* Init F0 variables    */
    F0start = F0peak = F0end = 110;
    F0delta = 0;

    Manner = SONMANNER;				/* If QX is not vowel	*/

    wordfinal = TRUE;
    
    mouthflag = iorb->mouths ? TRUE : FALSE;


   /*
    *	Process until end of phoneme string.
    */

    while (*phon != 0XFF)
      {
	Old_PC   = Last_PC;
	Old_PCF  = Last_PCF;
	 
	Last_PC	 = This_PC;
	Last_PCF = This_PCF;
	Last_PCS = This_PCS;
	Last_dur = This_dur;
	Last_bits= This_bits;

	This_PC  = Next_PC;
	This_PCF = Next_PCF;
	This_PCS = Next_PCS;
	This_dur = Next_dur;
	This_bits= Next_bits;

	Next_PC  = *phon++;
 	Next_PCF = Parms[Next_PC].features;
	Next_PCS = *stress & 0X10;
	Next_bits= *stress++ & 0xf0;
	Next_bits |= (*dur & 0xc0) >> 4;
	Next_dur = *dur++ & 0X3F;



	DBG(printf("working on phon=%c%c\n", Parms[This_PC].digraph >> 8,
		Parms[This_PC].digraph & 0xFF));

	DBG(PrintParms(Last_PC));
	DBG(PrintParms(This_PC));
	DBG(PrintParms(Next_PC));



   /*
    *  The Meat
    */

	F1.Mintime = F1.Olddur;
	F2.Mintime = F2.Olddur;
	F3.Mintime = F3.Olddur;
	A1.Mintime = A1.Olddur;
	A2.Mintime = A2.Olddur;
	A3.Mintime = A3.Olddur;
	AV.Mintime = AV.Olddur;
	AF.Mintime = AF.Olddur;
	AH.Mintime = AH.Olddur;
	F0.Mintime = F0.Olddur;
	MH.Mintime = MH.Olddur;
	MW.Mintime = MW.Olddur;

	PhonetC4();  
	PhonetC5();
	if (THIS_PC_IS(SONORANT) && THIS_PC_ISNOT(NASAL)) PhonetC6();
	if THIS_PC_IS(VOWEL) PhonetC7(iorb, centphon, centpct);
	if (THIS_PC_ISNOT(SONORANT) || THIS_PC_IS(NASAL)) PhonetC8();
	PhonetC9();
	PhonetC10(iorb);


	DBG(PrintPParms("F1", &F1));
	DBG(PrintPParms("F2", &F2));
	DBG(PrintPParms("F3", &F3));

        DBG(printf("\n\nFormant tracks before fillbuffer and interp\n"));
	DBG(PrintTrack("F1", &F1));
        DBG(PrintTrack("F2", &F2));

	if THIS_PC_ISNOT(DIPHTHONG)
	  {
	    FillBuffer(&F1);
	    FillBuffer(&F2);
	    FillBuffer(&F3);
	    FillBuffer(&A1);
	    FillBuffer(&A2);
	    FillBuffer(&A3);
	  }
	FillBuffer(&AV);
	FillBuffer(&AF);
	FillBuffer(&AH);
	FillBuffer(&MH);
	FillBuffer(&MW);
/*	FillBuffer(&F0); */



   /*
    *	Articulation.
    *	    Modify Tcb and Tcf according to user specified percentage.
    *       Default = 100.
    *

	if((iorb->flags & NDF_NEWIORB) && (iorb->articulate != DEFARTIC))
	{
	    artic = iorb->articulate;
	    F1.Tcf = PCT(artic, F1.Tcf);
	    F2.Tcf = PCT(artic, F2.Tcf);
	    F3.Tcf = PCT(artic, F3.Tcf);
	    A1.Tcf = PCT(artic, A1.Tcf);
	    A2.Tcf = PCT(artic, A2.Tcf);
	    A3.Tcf = PCT(artic, A3.Tcf);
	    F1.Tcb = PCT(artic, F1.Tcb);
	    F2.Tcb = PCT(artic, F2.Tcb);
	    F3.Tcb = PCT(artic, F3.Tcb);
	    A1.Tcb = PCT(artic, A1.Tcb);
	    A2.Tcb = PCT(artic, A2.Tcb);
	    A3.Tcb = PCT(artic, A3.Tcb);
	    MH.Tcb = PCT(artic, MH.Tcb);
	    MW.Tcb = PCT(artic, MW.Tcb);
	}



   /*
    *  Interpolate each parameter in its own local buffer.
    */

	PhonetInterp("F1", &F1);	/* strings are for debugging only */
	PhonetInterp("F2", &F2);
	PhonetInterp("F3", &F3);
	PhonetInterp("A1", &A1);
	PhonetInterp("A2", &A2);
	PhonetInterp("A3", &A3);
	PhonetInterp("AV", &AV);
	PhonetInterp("AF", &AF);
	PhonetInterp("AH", &AH);
	PhonetInterp("MH", &MH);
	PhonetInterp("MW", &MW);
/*	PhonetInterp("F0", &F0); */

        DBG(printf("formant tracks after interpolation\n"));
        DBG(PrintTrack("F1", &F1));
        DBG(PrintTrack("F2", &F2));

	DBG(printf("\n\n\n\n\n\n"));



   /*
    *	Install appropriate arrays into the coef buffer.
    *	    
    */

	LocalF1 = &LocalBfr[0*LOCAL_BFR_SIZE];	      /* Reset local bfr ptrs */
	LocalF2 = &LocalBfr[1*LOCAL_BFR_SIZE];
	LocalF3 = &LocalBfr[2*LOCAL_BFR_SIZE];
	LocalA1 = &LocalBfr[3*LOCAL_BFR_SIZE];
 	LocalA2 = &LocalBfr[4*LOCAL_BFR_SIZE];
	LocalA3 = &LocalBfr[5*LOCAL_BFR_SIZE];
 	LocalAV = &LocalBfr[6*LOCAL_BFR_SIZE];
 	LocalAF = &LocalBfr[7*LOCAL_BFR_SIZE];
 	LocalAH = &LocalBfr[8*LOCAL_BFR_SIZE];
 	LocalF0 = &LocalBfr[9*LOCAL_BFR_SIZE];
 	LocalMH = &LocalBfr[10*LOCAL_BFR_SIZE];
 	LocalMW = &LocalBfr[11*LOCAL_BFR_SIZE];

	if (LAST_PC_IS(SONORANT) && (Last_PC != PC_HH) && (Last_PC != PC_HX))
	     aspid = Parms[Last_PC].fricID;
	else aspid = Parms[This_PC].fricID;

	if (!(Old_PCF & VOICED) && (Old_PCF & PLOSIVE) && LAST_PC_IS(SONORANT))
	{
	    switch(Old_PC)
	    {
		case PC_P:	index = 1;	break;    
		case PC_T:	index = 2;	break;    
		case PC_K:	index = 3;	break;    
		case PC_KX:	index = 4;	break;    
		case PC_CH:	index = 5;
	    }
	    for(i=0; i<37; i++)  if(PlosAspID[i][0] == Last_PC) break;
	    aspid = PlosAspID[i][index];
	}
    
	nasal_f = LAST_PC_IS(NASAL) ? FF_NASAL_OR_VB : 0;

	if (LAST_PC_IS(VOICED) && LAST_PC_IS(PLOSIVE))
	  {
	    if(!(Old_PCF & VOICED) || !(Old_PCF & SONORANT))
		 voiced_plosive_f = 0;
	    else voiced_plosive_f = FF_NASAL_OR_VB;
	  }
	else voiced_plosive_f = 0;

	sylnasal_f	 = (!(Old_PCF & GLOTTAL) &&
			    LAST_PC_IS(NASAL)    && 
			    LAST_PC_IS(SYLLABIC)) ? TRUE : FALSE;
			    
	nasalid = 0;
	if LAST_PC_IS(NASAL) switch(Last_PC)
	  {
	    case PC_M:	    nasalid = 1;	break;
	    case PC_UM:	    nasalid = 1;	break;
	    case PC_N:	    nasalid = 2;	break;
	    case PC_UN:	    nasalid = 2;	break;
	    case PC_NX:	    nasalid = 3;	break;
	  }


	use_left_fricid = TRUE;

	half_dur = MIN(Last_dur >> 1, 5);
	for (i = 0;  i < Last_dur; ++i)
          {
	    if (i >= half_dur) use_left_fricid = FALSE;

	    if LAST_PC_IS(FRICATIVE | PLOSIVE) fricid = Parms[Last_PC].fricID;
	    else if (use_left_fricid) fricid = Parms[Old_PC].fricID;
	    else if LAST_PC_IS(PLOSIVE) fricid = Parms[Last_PC].fricID;
	    else fricid = Parms[This_PC].fricID;

	    if(iorb->flags & NDF_NEWIORB)
	    {
		*LocalAV += iorb->AVbias + AVBIAS;
		*LocalAH += iorb->AVbias + AHBIAS;
		*LocalAF += iorb->AFbias + AFBIAS;
	    }
	    else
	    {
		*LocalAV += AVBIAS;
		*LocalAH += AHBIAS;
		*LocalAF += AFBIAS;
	    }

/*	    flags = (sylnasal_f && (i < 4)) ? 0 : nasal_f;  */
	    flags = nasal_f;
	    if (*LocalAV + *LocalA1 >  MIN_DBAMP) flags |= FF_VOICED;
	    if (*LocalAF            >  MIN_DBAMP) flags |= FF_FRIC;
/*	    if (*LocalAF            <= MIN_DBAMP) flags |= voiced_plosive_f; */
	    flags |= voiced_plosive_f;
	    if (*LocalAH            >  MIN_DBAMP) flags |= FF_ASP;
	    

	    *coef++ = ((flags & nasal_f) | voiced_plosive_f) ? 
					   nasalid  : *LocalF1;
	    *coef++ = (flags  & FF_FRIC) ? fricid   : *LocalF2;
	    *coef++ = (flags  & FF_ASP ) ? aspid    : *LocalF3;
	    *coef++ = *LocalAV + *LocalA1++;
	    *coef++ = (flags  & FF_FRIC) ? *LocalAF : *LocalAV + *LocalA2;
	    *coef++ = (flags  & FF_ASP ) ? *LocalAH : *LocalAV + *LocalA3;
	    *coef++ = flags;
	    coef++;				/* Bump past F0		 */ 

	    if (mouthflag) *mouths++ = (*LocalMH++ << 4) + *LocalMW++;

	    LocalAH++;				/* Adjust local bfr ptrs */
	    LocalAF++;				/* Note that LocalA1,F0  */
	    LocalAV++;				/* and LocalMH,MW are	 */
	    LocalF1++;				/* incremented above 	 */
	    LocalF2++;
	    LocalF3++;
	    LocalA2++;
	    LocalA3++;

	  }

	PhonetF0(coef);



   /*
    *	Set start of phon, syllable, and word bits in coef buffer.
    */

	flags = FF_PHONSTART;			/* Always start of phon	*/

	if (Last_bits & 0X80) {
	    flags |= FF_SYLSTART;		/* Add in start of syl	*/
	    if (wordfinal) {
	        flags |= FF_WORDSTART;		/* Add in start of word	*/
	        wordfinal   = FALSE;
	    }
	}
	wordfinal = (wordfinal || (Last_bits & 0X08));

	*(coef - Last_dur*8 + 6) |= flags;	/* Store in coef buffer	*/



   /*
    *	Adjust phonet track pointers.
    */

	AdjustTrack(&F1);
	AdjustTrack(&F2);
	AdjustTrack(&F3);
	AdjustTrack(&A1);
	AdjustTrack(&A2);
	AdjustTrack(&A3);
	AdjustTrack(&AV);
	AdjustTrack(&AF);
	AdjustTrack(&AH);
/*	AdjustTrack(&F0); */
	AdjustTrack(&MH);
	AdjustTrack(&MW);

      }					/* End of phoneme input */


    for (i = 0; i < 8; ++i)  *coef++ = 0xff;	/* Last frame fill */

}


#ifdef PRINTDEBUGINFO

int PrintParms(pcode)
int pcode;
{
   printf("    features = %lx\n", Parms[pcode].features);
   printf("    f1=%ld  f2=%ld  f3=%ld\n", (LONG)Parms[pcode].f1,
					    (LONG)Parms[pcode].f2,
					    (LONG)Parms[pcode].f3);
   printf("    a1=%ld  a2=%ld  a3=%ld\n", (LONG)Parms[pcode].a1,
					    (LONG)Parms[pcode].a2,
					    (LONG)Parms[pcode].a3);
   printf("    mindur=%ld   inhdur=%ld\n", (LONG)Parms[pcode].mindur,
   					     (LONG)Parms[pcode].inhdur);
}


int PrintPParms(TrackName, Track)
	char		*TrackName;
struct	PhonetParms	*Track;
{

    printf("\n\nFinal phonet parms values for %s\n",TrackName);

    printf("Cumdur=%lx    Local=%lx    Olddur=%ld    Segdur=%ld\n",
	Track->Cumdur, Track->Local, (LONG)Track->Olddur, (LONG)Track->Segdur);

    printf("Trantype=%ld   Mintime=%ld   Target=%ld   Oldval=%ld\n",
	(LONG)Track->Trantype, (LONG)Track->Mintime, (LONG)Track->Target,
	(LONG)Track->Oldval);

    printf("Tcf=%ld   Tcb=%ld   Bper=%ld   Bvf=%ld   Bvb=%ld\n",
	(LONG)Track->Tcf, (LONG)Track->Tcb, (LONG)Track->Bper, 
	(LONG)Track->Bvf, (LONG)Track->Bvb);
}



int PrintTrack(DebugString, Track)
	char		*DebugString;
struct	PhonetParms	*Track;
{
int	 i,j;
WORD	*local;

	local = Track->Local;

	printf("%s\n", DebugString);

	for (i=0; i<8; ++i) {
	    for (j=0; j<16; ++j) printf("%3d ",*local++);
	    printf("\n");
	}
}

#endif
