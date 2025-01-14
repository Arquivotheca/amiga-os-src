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


#define  DIGRAPH(A,B)	(UWORD)(((ULONG)(A) << 8) | (ULONG)(B))
#define  FADJ(A)	(UWORD)(((LONG)A+8L)/16L)

#define	 NONE		0X00000000
#define	 AFFRICATE	0X00000001
#define	 ALVEOLAR	0X00000002
#define	 ASPSEG		0X00000004
#define	 DENTAL		0X00000008
#define	 DIPHTHONG	0X00000010
#define	 F2BACK		0X00000020
#define	 FRICATIVE	0X00000040
#define	 FRONT		0X00000080
#define	 GLOTTAL	0X00000100
#define	 HIGH		0X00000200
#define	 LABIAL		0X00000400
#define	 LATERAL	0X00000800
#define	 LAX		0X00001000
#define	 LIQGLIDE	0X00002000
#define	 LOW		0X00004000
#define	 NASAL		0X00008000
#define	 PALATAL	0X00010000
#define	 PALVEL		0X00020000
#define	 PLOSIVE	0X00040000
#define	 RETRO		0X00080000
#define	 RGLIDE		0X00100000
#define	 ROUND		0X00200000
#define	 SCHWA		0X00400000
#define	 SONORANT	0X00800000
#define	 STOP		0X01000000
#define	 SYLLABIC	0X02000000
#define	 VELAR		0X04000000
#define	 VOICED		0X08000000
#define	 VOWEL		0X10000000
#define	 WGLIDE		0X20000000
#define	 YGLIDE		0X40000000
#define  SILENT		0x80000000


/* Aspirant ID's for plosive aspirant transition table */

#define ASP_IY	6
#define ASP_IH	7
#define ASP_EH	8
#define ASP_AE	9
#define ASP_AA	10
#define ASP_AO	11
#define ASP_AH	12
#define ASP_OW	13
#define ASP_UH	14
#define ASP_UW	15
#define ASP_ER	16
#define ASP_AXP	17

/* Fricative/aspirant ID numbers:
1 = /S/		10 = /AA/
2 = /K/		11 = /AO/
3 = /SH/	12 = /AH/
4 = /KX/	13 = /OW/
5 = /F,TH/	14 = /UH/
6 = /IY/	15 = /UW/
7 = /IH/	16 = /ER/
8 = /EH/	17 = /AXP/
9 = /AE/	*/


struct PhonemeParms
  {
    UWORD    digraph;			/* Two character ASCII symbol	*/
    ULONG    features;			/* Feature array		*/
    UBYTE    f1, f2, f3;		/* Male formants		*/
    BYTE     a1, a2, a3;		/* Formant amplitudes		*/
    UBYTE    mindur, inhdur;		/* Minimum and inherent durs	*/
    BYTE     av;			/* Amplitude of voicing		*/
    UBYTE    fricID;			/* Fricative/Aspirant ID	*/
    UBYTE    height, width;		/* Mouth opening height, width	*/
  };
