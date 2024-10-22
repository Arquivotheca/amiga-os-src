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
#include "parms.h"
#include "pc.h"

#define MW(x)	((x * 1000) / 3667)		/* Mouth width macro	*/

struct PhonemeParms Parms[] = {

{DIGRAPH(' ',0),		 		/* DIGRAPH (' ')  ID=00	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

{DIGRAPH('.',0),				/* DIGRAPH ('.')  ID=01	*/
	SILENT|SONORANT,			/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2400),	/* F1, F2, F3		*/
	-5, -20, -30,				/* A1, A2, A3		*/
	60, 60,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	17,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/
 
{DIGRAPH('?',0),				/* DIGRAPH ('?')  ID=02	*/
	SILENT|SONORANT,			/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2400),	/* F1, F2, F3		*/
	-5, -20, -30,				/* A1, A2, A3		*/
	60, 60,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	17,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

{DIGRAPH(',',0),	 			/* DIGRAPH (',')  ID=03	*/
	GLOTTAL|SILENT|SONORANT,		/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2400),	/* F1, F2, F3		*/
	-5, -20, -30,				/* A1, A2, A3		*/
	200, 200,				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	17,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

{DIGRAPH('-',0), 				/* DIGRAPH ('-')  ID=04	*/
	GLOTTAL|SILENT|SONORANT,		/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2400),	/* F1, F2, F3		*/
	-5, -20, -30,				/* A1, A2, A3		*/
	200, 200,				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	17,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

{DIGRAPH('(',0), 				/* DIGRAPH ('(')  ID=05	*/
	NONE,					/* FEATURES		*/
	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(')',0), 				/* DIGRAPH (')')  ID=06	*/
	NONE,					/* FEATURES		*/
	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (UNUSED) 07 	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (UNUSED) 08	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('I','Y'),				/* DIGRAPH (IY)	ID=09	*/
	DIPHTHONG|F2BACK|FRONT|			/* FEATURES		*/
	HIGH|SONORANT|SYLLABIC|
	VOICED|VOWEL|YGLIDE,
  	FADJ(300), FADJ(2200), FADJ(2700),	/* F1, F2, F3		*/
	-4, -14, -12,				/* A1, A2, A3		*/
	100, 185,	/* was 60, 155 */	/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	6,					/* FRIC/ASP ID  	*/
	9, MW(55)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (IY') ID=10	*/
	NONE,					/* FEATURES		*/
  	FADJ(350), FADJ(2070), FADJ(2700),	/* F1, F2, F3		*/
	-4, -17, -12,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	9, MW(55)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('I','H'),				/* DIGRAPH (IH)	 ID=11	*/
	DIPHTHONG|FRONT|			/* FEATURES		*/
	HIGH|LAX|SONORANT|SYLLABIC|
	VOICED|VOWEL,
  	FADJ(400), FADJ(1920), FADJ(2400),	/* F1, F2, F3		*/
	-3, -14, -11,				/* A1, A2, A3		*/
	40, 135,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	7,					/* FRIC/ASP ID  	*/
	9, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (IH') ID=12	*/
	NONE,					/* FEATURES		*/
  	FADJ(470), FADJ(1800), FADJ(2400),	/* F1, F2, F3		*/
	-3, -14, -11,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	9, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('E','H'),				/* DIGRAPH (EH)	 ID=13	*/
	DIPHTHONG|FRONT|			/* FEATURES		*/
	LAX|SONORANT|SYLLABIC|
	VOICED|VOWEL,
  	FADJ(550), FADJ(1700), FADJ(2530),	/* F1, F2, F3		*/
	-3, -12, -18,				/* A1, A2, A3		*/
	70, 150,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	8,					/* FRIC/ASP ID  	*/
	10, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (EH') ID=14	*/
	NONE,					/* FEATURES		*/
  	FADJ(620), FADJ(1550), FADJ(2530),	/* F1, F2, F3		*/
	-3, -14, -18,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	10, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','E'),				/* DIGRAPH (AE)	 ID=15	*/
	DIPHTHONG|FRONT|			/* FEATURES		*/
	LAX|LOW|SONORANT|SYLLABIC|
	VOICED|VOWEL,
  	FADJ(620), FADJ(1630), FADJ(2325),	/* F1, F2, F3		*/
	-5, -10, -15,				/* A1, A2, A3		*/
	80, 230,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	9,					/* FRIC/ASP ID  	*/
	15, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (AE') ID=16	*/
	NONE,					/* FEATURES		*/
  	FADJ(640), FADJ(1500), FADJ(2400),	/* F1, F2, F3		*/
	-5, -10, -16,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	15, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','A'),				/* DIGRAPH (AA)	 ID=17	*/
	LOW|SONORANT|SYLLABIC|VOICED|VOWEL,	/* FEATURES		*/
  	FADJ(720), FADJ(1250), FADJ(2600),	/* F1, F2, F3		*/
	-2, -10, -21,				/* A1, A2, A3		*/
	100, 240,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	10,					/* FRIC/ASP ID  	*/
	15, MW(47)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','H'),				/* DIGRAPH (AH)	 ID=18	*/
	SONORANT|SYLLABIC|VOICED|VOWEL,		/* FEATURES		*/
  	FADJ(600), FADJ(1270), FADJ(2550),	/* F1, F2, F3		*/
	-3, -10, -14,				/* A1, A2, A3		*/
	60, 140,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	12,					/* FRIC/ASP ID  	*/
	8, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','O'),				/* DIGRAPH (AO)	 ID=19	*/
	DIPHTHONG|LAX|LOW|ROUND|		/* FEATURES		*/
	SONORANT|SYLLABIC|
	VOICED|VOWEL,
  	FADJ(550), FADJ(1050), FADJ(2400),	/* F1, F2, F3		*/
	-7, -13, -26,				/* A1, A2, A3		*/
	100, 240,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	11,					/* FRIC/ASP ID  	*/
	13, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (AO') ID=20	*/
	NONE,					/* FEATURES		*/
  	FADJ(630), FADJ(1040), FADJ(2600),	/* F1, F2, F3		*/
	-7, -14, -26,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	13, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('U','H'),				/* DIGRAPH (UH)	 ID=21	*/
	DIPHTHONG|HIGH|LAX|ROUND|		/* FEATURES		*/
	SONORANT|SYLLABIC|VOICED|VOWEL,
  	FADJ(500), FADJ(1100), FADJ(2500),	/* F1, F2, F3		*/
	-3, -14, -24,				/* A1, A2, A3		*/
	60, 160,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	14,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (UH') ID=22	*/
	NONE,					/* FEATURES		*/
  	FADJ(500), FADJ(1180), FADJ(2390),	/* F1, F2, F3		*/
	-3, -14, -20,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','X'),				/* DIGRAPH (AX)	 ID=23	*/
	LAX|SCHWA|SONORANT|SYLLABIC|		/* FEATURES		*/
	VOICED|VOWEL,
  	FADJ(550), FADJ(1260), FADJ(2470),	/* F1, F2, F3		*/
	-2, -8, -11,				/* A1, A2, A3		*/
	60, 110,				/* MINDUR, INHDUR	*/
	57,	/* was 60 */			/* AV			*/
	12,					/* FRIC/ASP ID  	*/
	8, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('I','X'),				/* DIGRAPH (IX)	 ID=24	*/
	FRONT|HIGH|LAX|SCHWA|			/* FEATURES		*/
	SONORANT|SYLLABIC|VOICED|VOWEL,
  	FADJ(420), FADJ(1680), FADJ(2520),	/* F1, F2, F3		*/
	0, -13, -11,				/* A1, A2, A3		*/
	60, 110,				/* MINDUR, INHDUR	*/
	57,	/* was 60 */			/* AV			*/
	7,					/* FRIC/ASP ID  	*/
	9, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('E','R'),				/* DIGRAPH (ER)	 ID=25	*/
	DIPHTHONG|RETRO|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL,
  	FADJ(450), FADJ(1200), FADJ(1400),	/* F1, F2, F3		*/
	-4, -9, -16,				/* A1, A2, A3		*/
	80, 180,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	16,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (ER') ID=26	*/
	NONE,					/* FEATURES		*/
  	FADJ(420), FADJ(1310), FADJ(1540),	/* F1, F2, F3		*/
	-5, -10, -17,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('Q','X'),				/* DIGRAPH (QX)	 ID=27	*/
	GLOTTAL|SILENT|SONORANT,		/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2400),	/* F1, F2, F3		*/
	-2, -17, -19,				/* A1, A2, A3		*/
	60, 120,				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	12,					/* FRIC/ASP ID  	*/
	0, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (AXP) ID=28	*/
	LAX|SONORANT|VOICED,			/* FEATURES		*/
  	FADJ(430), FADJ(1500), FADJ(2500),	/* F1, F2, F3		*/
	-7, -14, -18,				/* A1, A2, A3		*/
	70, 70,					/* MINDUR, INHDUR	*/
	47,					/* AV			*/
	12,					/* FRIC/ASP ID  	*/
	8, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('R','X'),				/* DIGRAPH (RX)	 ID=29	*/
	LIQGLIDE|RETRO|SONORANT|VOICED,		/* FEATURES		*/
  	FADJ(460), FADJ(1260), FADJ(1560),	/* F1, F2, F3		*/
	0, -6, -13,				/* A1, A2, A3		*/
	70, 80, 				/* MINDUR, INHDUR	*/
	57,					/* AV			*/
	16,					/* FRIC/ASP ID  	*/
	10, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('L','X'),				/* DIGRAPH (LX)	 ID=30	*/
	LATERAL|LIQGLIDE|SONORANT|VOICED,	/* FEATURES		*/
  	FADJ(550), FADJ(900), FADJ(2450),	/* F1, F2, F3		*/
	0, -9, -18,				/* A1, A2, A3		*/
	70, 90, 				/* MINDUR, INHDUR	*/
	57,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	8, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('E','Y'),				/* DIGRAPH (EY)	ID=31	*/
	DIPHTHONG|FRONT|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL|YGLIDE,
  	FADJ(510), FADJ(1680), FADJ(2600),	/* F1, F2, F3		*/
	-3, -15, -19,				/* A1, A2, A3		*/
	100, 190,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	8,					/* FRIC/ASP ID  	*/
	10, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (EY') ID=32	*/
	NONE,					/* FEATURES		*/
	FADJ(350), FADJ(2300), FADJ(2700),	/* F1, F2, F3		*/
	-3, -14, -9,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	0,					/* FRIC/ASP ID 		*/
	9, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','Y'),				/* DIGRAPH (AY)  ID=33	*/
	DIPHTHONG|LOW|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL|YGLIDE,
  	FADJ(660), FADJ(1200), FADJ(2700),	/* F1, F2, F3		*/
	-7, -12, -15,				/* A1, A2, A3		*/
	150, 250,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	10,					/* FRIC/ASP ID  	*/
	15, MW(47)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (AY') ID=34	*/
	NONE,					/* FEATURES		*/
	FADJ(470), FADJ(2000), FADJ(2550),	/* F1, F2, F3		*/
	-5, -13, -11,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	9, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('O','Y'),				/* DIGRAPH (OY)  ID=35	*/
	DIPHTHONG|ROUND|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL|YGLIDE,
  	FADJ(520), FADJ(800), FADJ(2400),	/* F1, F2, F3		*/
	-3, -12, -26,				/* A1, A2, A3		*/
	150, 255,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	8, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (OY')  ID=36	*/
	NONE,					/* FEATURES		*/
	FADJ(440), FADJ(2030), FADJ(2550),	/* F1, F2, F3		*/
	-3, -15, -12,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	9, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('A','W'),				/* DIGRAPH (AW)	 ID=37	*/
	DIPHTHONG|LOW|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL|WGLIDE,
  	FADJ(640), FADJ(1500), FADJ(2500),	/* F1, F2, F3		*/
	-3, -11, -15,				/* A1, A2, A3		*/
	100, 255,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	10,					/* FRIC/ASP ID  	*/
	13, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (AW')  ID=38	*/
	NONE,					/* FEATURES		*/
	FADJ(500), FADJ(850), FADJ(2060),	/* F1, F2, F3		*/
	-3, -14, -29,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	6, MW(29)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('O','W'),				/* DIGRAPH (OW)	 ID=39	*/
	DIPHTHONG|ROUND|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL|WGLIDE,
  	FADJ(500), FADJ(1180), FADJ(2350),	/* F1, F2, F3		*/
	-3, -14, -20,				/* A1, A2, A3		*/
	80, 220,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	5, MW(26)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (OW')  ID=40	*/
	NONE,					/* FEATURES		*/
	FADJ(440), FADJ(780), FADJ(2400),	/* F1, F2, F3		*/
	-4, -12, -29,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	2, MW(18)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('U','W'),				/* DIGRAPH (UW)	 ID=41	*/
	DIPHTHONG|HIGH|ROUND|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED|VOWEL|WGLIDE,
  	FADJ(400), FADJ(1250), FADJ(2300),	/* F1, F2, F3		*/
	-4, -21, -21,				/* A1, A2, A3		*/
	70, 210,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	3, MW(18)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (UW')  ID=42	*/
	NONE,					/* FEATURES		*/
	FADJ(300), FADJ(850), FADJ(2250),	/* F1, F2, F3		*/
	-9, -35, -48,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	2, MW(14)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (IXR) ID=43	*/
	DIPHTHONG|FRONT|HIGH|RGLIDE|SONORANT|	/* FEATURES		*/
	SYLLABIC|VOICED|VOWEL,
  	FADJ(320), FADJ(1900), FADJ(2900),	/* F1, F2, F3		*/
	-3, -13, -10,				/* A1, A2, A3		*/
	100, 230,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	6,					/* FRIC/ASP ID  	*/
	9, MW(52)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (IXR') ID=44	*/
	NONE,					/* FEATURES		*/
	FADJ(420), FADJ(1550), FADJ(1750),	/* F1, F2, F3		*/
	-3, -10, -15,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	6,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (EXR) ID=45	*/
	DIPHTHONG|FRONT|RGLIDE|SONORANT|	/* FEATURES		*/
	SYLLABIC|VOICED|VOWEL,
  	FADJ(500), FADJ(1750), FADJ(2400),	/* F1, F2, F3		*/
	-2, -16, -20,				/* A1, A2, A3		*/
	130, 255,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	8,					/* FRIC/ASP ID  	*/
	10, MW(50)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (EXR') ID=46	*/
	NONE,					/* FEATURES		*/
	FADJ(470), FADJ(1570), FADJ(1850),	/* F1, F2, F3		*/
	-3, -12, -18,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	8,					/* FRIC/ASP ID  	*/
	10, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (AXR) ID=47	*/
	DIPHTHONG|LOW|RGLIDE|SONORANT|		/* FEATURES		*/
	SYLLABIC|VOICED|VOWEL,
  	FADJ(680), FADJ(1170), FADJ(2380),	/* F1, F2, F3		*/
	-2, -11, -22,				/* A1, A2, A3		*/
	120, 255,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	10,					/* FRIC/ASP ID  	*/
	15, MW(47)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (AXR') ID=48	*/
	NONE,					/* FEATURES		*/
	FADJ(600), FADJ(1350), FADJ(1850),	/* F1, F2, F3		*/
	-5, -13, -21,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	10,					/* FRIC/ASP ID  	*/
	10, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (OXR) ID=49	*/
	DIPHTHONG|RGLIDE|ROUND|SONORANT|	/* FEATURES		*/
	SYLLABIC|VOICED|VOWEL,
  	FADJ(550), FADJ(820), FADJ(2200),	/* F1, F2, F3		*/
	-3, -10, -27,				/* A1, A2, A3		*/
	130, 240,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	7, MW(25)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (OXR') ID=50	*/
	NONE,					/* FEATURES		*/
	FADJ(490), FADJ(1300), FADJ(1500),	/* F1, F2, F3		*/
	-3, -15, -23,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	10, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0, 0),				/* DIGRAPH (UXR) ID=51	*/
	DIPHTHONG|HIGH|RGLIDE|SONORANT|		/* FEATURES		*/
	SYLLABIC|VOICED|VOWEL,
  	FADJ(360), FADJ(800), FADJ(2000),	/* F1, F2, F3		*/
	-3, -13, -37,				/* A1, A2, A3		*/
	110, 230,				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (UXR') ID=52	*/
	NONE,					/* FEATURES		*/
	FADJ(390), FADJ(1150), FADJ(1500),	/* F1, F2, F3		*/
	-3, -10, -20,				/* A1, A2, A3		*/
	0, 0,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	7, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('W','H'),				/* DIGRAPH (WH)	 ID=53	*/
	ASPSEG|HIGH|LABIAL|LIQGLIDE|		/* FEATURES		*/
	ROUND|SONORANT|VOICED,
  	FADJ(330), FADJ(600), FADJ(2100),	/* F1, F2, F3		*/
	-6, -10, -40,				/* A1, A2, A3		*/
	60, 70, 				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	2, MW(14)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('R',0),				/* DIGRAPH (R)	 ID=54	*/
	LIQGLIDE|RETRO|SONORANT|VOICED,		/* FEATURES		*/
  	FADJ(300), FADJ(900), FADJ(1580),	/* F1, F2, F3		*/
	-12, -23, -30,				/* A1, A2, A3		*/
	30, 80, 				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	16,					/* FRIC/ASP ID  	*/
	4, MW(20)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('L',0),				/* DIGRAPH (L)	 ID=55	*/
	LATERAL|LIQGLIDE|SONORANT|VOICED,	/* FEATURES		*/
  	FADJ(400), FADJ(870), FADJ(2350),	/* F1, F2, F3		*/
	-2, -19, -15,				/* A1, A2, A3		*/
	55, 80,		/* was 40, 80 */	/* MINDUR, INHDUR	*/
	54,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	7, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('W',0),				/* DIGRAPH (W)	ID=56	*/
	HIGH|LABIAL|LIQGLIDE|ROUND|		/* FEATURES		*/
	SONORANT|VOICED,		
  	FADJ(280), FADJ(600), FADJ(2170),	/* F1, F2, F3		*/
	-13, -31, -55,				/* A1, A2, A3		*/
	60, 80,					/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	15,					/* FRIC/ASP ID  	*/
	2, MW(7)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('Y',0),				/* DIGRAPH (Y)	ID=57	*/
	F2BACK|FRONT|HIGH|LIQGLIDE|PALATAL|	/* FEATURES		*/
	SONORANT|VOICED,
  	FADJ(230), FADJ(2300), FADJ(3200),	/* F1, F2, F3		*/
	-11, -33, -22,				/* A1, A2, A3		*/
	50, 90,		/* was 40, 80 */	/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	6,					/* FRIC/ASP ID  	*/
	8, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('Y','U'),				/* DIGRAPH (YU)	ID=58	*/
	DIPHTHONG|F2BACK|FRONT|HIGH|ROUND|	/* FEATURES		*/
	SONORANT|VOICED|VOWEL|WGLIDE,
  	FADJ(290), FADJ(1900), FADJ(2600),	/* F1, F2, F3		*/
	-2, -17, -12,				/* A1, A2, A3		*/
	150, 230, 				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	6,					/* FRIC/ASP ID  	*/
	9, MW(52)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (YU') ID=59	*/
	NONE,					/* FEATURES		*/
  	FADJ(330), FADJ(1200), FADJ(2100),	/* F1, F2, F3		*/
	-2, -19, -26,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	60,					/* AV			*/
	6,					/* FRIC/ASP ID  	*/
	3, MW(18)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('M',0),				/* DIGRAPH (M)	ID=60	*/
	LABIAL|NASAL|SONORANT|STOP|VOICED,	/* FEATURES		*/
  	FADJ(480), FADJ(1050), FADJ(2100),	/* F1, F2, F3		*/
	-9, -22, -27,				/* A1, A2, A3		*/
	60, 90, 				/* MINDUR, INHDUR	*/
	55,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('N',0),				/* DIGRAPH (N)	ID=61	*/
	ALVEOLAR|NASAL|SONORANT|STOP|VOICED,	/* FEATURES		*/
  	FADJ(480), FADJ(1600), FADJ(2700),	/* F1, F2, F3		*/
	-9, -29, -21,				/* A1, A2, A3		*/
	50, 80, 				/* MINDUR, INHDUR	*/
	55,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	6, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('N','X'),				/* DIGRAPH (NX)	 ID=62	*/
	NASAL|VELAR|SONORANT|STOP|VOICED,	/* FEATURES		*/
  	FADJ(480), FADJ(1600), FADJ(2050),	/* F1, F2, F3		*/
	-12, -21, -20,				/* A1, A2, A3		*/
	60, 95, 				/* MINDUR, INHDUR	*/
	55,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	6, MW(42)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('D','X'),				/* DIGRAPH (DX)	 ID=63	*/
	ALVEOLAR|STOP|VOICED,			/* FEATURES		*/
  	FADJ(200), FADJ(1600), FADJ(2700),	/* F1, F2, F3		*/
	-2, -25, -17,				/* A1, A2, A3		*/
	40, 40,		/* was 20, 20 */	/* MINDUR, INHDUR	*/
	38,		/* was 50     */	/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	7, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('Q',0),				/* DIGRAPH (Q)	 ID=64	*/
	GLOTTAL|STOP|VOICED,			/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2450),	/* F1, F2, F3		*/
	-5, -21, -26,				/* A1, A2, A3		*/
	55, 55,		/* was 30, 50 */	/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	7, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('S',0),				/* DIGRAPH (S)	ID=65	*/
	ALVEOLAR|FRICATIVE,			/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2700),	/* F1, F2, F3		*/
	-6, -22, -22,				/* A1, A2, A3		*/
	60, 105,				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	1,					/* FRIC/ASP ID  	*/
	5, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('S','H'),				/* DIGRAPH (SH)	 ID=66	*/
	FRICATIVE|PALATAL,			/* FEATURES		*/
  	FADJ(400), FADJ(1650), FADJ(2400),	/* F1, F2, F3		*/
	-6, -20, -29,				/* A1, A2, A3		*/
	80, 105,				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	3,					/* FRIC/ASP ID  	*/
	6, MW(25)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('F',0),				/* DIGRAPH (F)	 ID=67	*/
	FRICATIVE|LABIAL,			/* FEATURES		*/
  	FADJ(340), FADJ(1100), FADJ(2080),	/* F1, F2, F3		*/
	-6, -23, -36,				/* A1, A2, A3		*/
	80, 100,				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	5,					/* FRIC/ASP ID  	*/
	2, MW(20)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('T','H'),				/* DIGRAPH (TH)	 ID=68	*/
	DENTAL|FRICATIVE,			/* FEATURES		*/
  	FADJ(400), FADJ(1400), FADJ(2700),	/* F1, F2, F3		*/
	-2, -18, -12,				/* A1, A2, A3		*/
	60, 90, 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	5,					/* FRIC/ASP ID  	*/
	8, MW(34)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('Z',0),				/* DIGRAPH (Z)	 ID=69	*/
	ALVEOLAR|FRICATIVE|VOICED,		/* FEATURES		*/
  	FADJ(300), FADJ(1600), FADJ(2700),	/* F1, F2, F3		*/
	-1, -19, -17,				/* A1, A2, A3		*/
	40, 100,	/* was 75 */		/* MINDUR, INHDUR	*/
	44,					/* AV			*/
	1,					/* FRIC/ASP ID  	*/
	5, MW(35)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('Z','H'),				/* DIGRAPH (ZH)	  ID=70	*/
	FRICATIVE|PALATAL|VOICED,		/* FEATURES		*/
  	FADJ(300), FADJ(1650), FADJ(2400),	/* F1, F2, F3		*/
	-6, -26, -32,				/* A1, A2, A3		*/
	40, 70, 				/* MINDUR, INHDUR	*/
	42,					/* AV			*/
	3,					/* FRIC/ASP ID  	*/
	6, MW(25)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('V',0),				/* DIGRAPH (V)	ID=71	*/
	FRICATIVE|LABIAL|VOICED,		/* FEATURES		*/
  	FADJ(300), FADJ(1130), FADJ(2100),	/* F1, F2, F3		*/
	0, -20, -32,				/* A1, A2, A3		*/
	40, 60,					/* MINDUR, INHDUR	*/
	40,					/* AV			*/
	5,					/* FRIC/ASP ID  	*/
	2, MW(20)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('D','H'),				/* DIGRAPH (DH)	 ID=72	*/
	DENTAL|FRICATIVE|VOICED,		/* FEATURES		*/
  	FADJ(300), FADJ(1200), FADJ(2700),	/* F1, F2, F3		*/
	-2, -18, -12,				/* A1, A2, A3		*/
	50, 60,      /* was 30, 50 */		/* MINDUR, INHDUR	*/
	48,					/* AV			*/
	5,					/* FRIC/ASP ID  	*/
	8, MW(34)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('C','H'),				/* DIGRAPH (CH)	 ID=73	*/
	AFFRICATE|PALATAL|PLOSIVE|STOP,		/* FEATURES		*/
  	FADJ(300), FADJ(1700), FADJ(2400),	/* F1, F2, F3		*/
	-8, -24, -32,				/* A1, A2, A3		*/
	50, 70,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	3,					/* FRIC/ASP ID  	*/
	6, MW(25)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('J',0),				/* DIGRAPH (J)	 ID=74	*/
	AFFRICATE|PALATAL|PLOSIVE|STOP|VOICED,	/* FEATURES		*/
  	FADJ(200), FADJ(1700), FADJ(2400),	/* F1, F2, F3		*/
	-4, -23, -30,				/* A1, A2, A3		*/
	50, 70, 				/* MINDUR, INHDUR	*/
	54,					/* AV			*/
	3,					/* FRIC/ASP ID  	*/
	6, MW(25)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('/','H'),				/* DIGRAPH (/H)	  ID=75	*/
	ASPSEG|GLOTTAL|SONORANT,		/* FEATURES		*/
  	FADJ(450), FADJ(1450), FADJ(2450),	/* F1, F2, F3		*/
	-13, -28, -34,				/* A1, A2, A3		*/
	20, 80,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	12,					/* FRIC/ASP ID  	*/
	9, MW(44)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (HX)  ID=76	*/
	ASPSEG|GLOTTAL|SONORANT|VOICED,		/* FEATURES		*/
  	FADJ(450), FADJ(1450), FADJ(2450),	/* F1, F2, F3		*/
	-11, -23, -29,				/* A1, A2, A3		*/
	25, 70,					/* MINDUR, INHDUR	*/
	44,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	6, MW(37)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('B',0),				/* DIGRAPH (B)	ID=77	*/
	LABIAL|PLOSIVE|STOP|VOICED,		/* FEATURES		*/
  	FADJ(200), FADJ(900), FADJ(2100),	/* F1, F2, F3		*/
	-1, -18, -24,	/* was -6,-26,-41 */	/* A1, A2, A3		*/
	60, 85, 				/* MINDUR, INHDUR	*/
	54,					/* AV			*/
	5,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('D',0),				/* DIGRAPH (D)	 ID=78	*/
	ALVEOLAR|PLOSIVE|STOP|VOICED,		/* FEATURES		*/
  	FADJ(200), FADJ(1800), FADJ(2700),	/* F1, F2, F3		*/
	-1, -22, -14,				/* A1, A2, A3		*/
	50, 75, 				/* MINDUR, INHDUR	*/
	54,					/* AV			*/
	1,					/* FRIC/ASP ID  	*/
	6, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('G',0),  /* VELAR is new */		/* DIGRAPH (G)	 ID=79	*/
	PALVEL|PLOSIVE|STOP|VOICED|VELAR,	/* FEATURES		*/
  	FADJ(200), FADJ(1950), FADJ(2800),	/* F1, F2, F3		*/
	-2, -21, -14,				/* A1, A2, A3		*/
	40, 80, 				/* MINDUR, INHDUR	*/
	54,					/* AV			*/
	2,					/* FRIC/ASP ID  	*/
	9, MW(44)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('G','X'),				/* DIGRAPH (GX)	 ID=80	*/
	PLOSIVE|STOP|VELAR|VOICED,		/* FEATURES		*/
  	FADJ(250), FADJ(1600), FADJ(1900),	/* F1, F2, F3		*/
	-2, -16, -20,				/* A1, A2, A3		*/
	60, 80, 				/* MINDUR, INHDUR	*/
	54,					/* AV			*/
	4,					/* FRIC/ASP ID  	*/
	5, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('P',0),				/* DIGRAPH (P)	 ID=81	*/
	LABIAL|PLOSIVE|STOP,			/* FEATURES		*/
  	FADJ(300), FADJ(900), FADJ(2100),	/* F1, F2, F3		*/
	-8, -19, -35,				/* A1, A2, A3		*/
	50, 90, 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	5,					/* FRIC/ASP ID  	*/
	0, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('T',0),				/* DIGRAPH (T)	 ID=82	*/
	ALVEOLAR|PLOSIVE|STOP, 			/* FEATURES		*/
  	FADJ(300), FADJ(1800), FADJ(2700),	/* F1, F2, F3		*/
	-8, -32, -26,				/* A1, A2, A3		*/
	50, 75, 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	1,					/* FRIC/ASP ID  	*/
	6, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(0,0),					/* DIGRAPH (TQ)  ID=83	*/
	ALVEOLAR|PLOSIVE|STOP|VOICED,		/* FEATURES		*/
  	FADJ(200), FADJ(1600), FADJ(2700),	/* F1, F2, F3		*/
	-7, -33, -30,				/* A1, A2, A3		*/
	50, 75,					/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	6, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('K',0),     /* VELAR is new */	/* DIGRAPH (K)	 ID=84	*/
	PALVEL|PLOSIVE|STOP|VELAR,		/* FEATURES		*/
  	FADJ(300), FADJ(2250), FADJ(2700),	/* F1, F2, F3		*/
	-8, -20, -14,				/* A1, A2, A3		*/
	60, 80, 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	2,					/* FRIC/ASP ID  	*/
	9, MW(44)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('K','X'),				/* DIGRAPH (KX)	 ID=85	*/
	PLOSIVE|STOP|VELAR,			/* FEATURES		*/
  	FADJ(350), FADJ(1600), FADJ(1900),	/* F1, F2, F3		*/
	-8, -27, -29,				/* A1, A2, A3		*/
	60, 80, 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	4,					/* FRIC/ASP ID  	*/
	5, MW(30)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('U','L'),				/* DIGRAPH (UL)	  ID=86	*/
	LATERAL|LIQGLIDE|SONORANT|SYLLABIC|	/* FEATURES		*/
	VOICED,
  	FADJ(420), FADJ(900), FADJ(2400),	/* F1, F2, F3		*/
	-2, -19, -15,				/* A1, A2, A3		*/
	110, 255, 				/* MINDUR, INHDUR	*/
	57,					/* AV			*/
	13,					/* FRIC/ASP ID  	*/
	8, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('U','M'),				/* DIGRAPH (UM)	 ID=87	*/
	LABIAL|NASAL|SONORANT|STOP|SYLLABIC|	/* FEATURES		*/
	VOICED,
  	FADJ(200), FADJ(900), FADJ(2100),	/* F1, F2, F3		*/
	-5, -22, -41,				/* A1, A2, A3		*/
	110, 170, 				/* MINDUR, INHDUR	*/
	52,					/* AV			*/
	17,					/* FRIC/ASP ID  	*/
	4, MW(45)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('U','N'),				/* DIGRAPH (UN)	 ID=88	*/
	ALVEOLAR|NASAL|SONORANT|STOP|SYLLABIC|	/* FEATURES		*/
	VOICED,
  	FADJ(350), FADJ(1600), FADJ(2700),	/* F1, F2, F3		*/
	-9, -29, -21,				/* A1, A2, A3		*/
	100, 170, 				/* MINDUR, INHDUR	*/
	55,					/* AV			*/
	17,					/* FRIC/ASP ID  	*/
	6, MW(40)},				/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('I','L'),				/* DIGRAPH (IL)	 ID=89	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('I','M'),				/* DIGRAPH (IM)	 ID=90	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('I','N'),				/* DIGRAPH (IN)	ID=91	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('O','H'),				/* DIGRAPH (OH)	ID=92	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('/','C'),				/* DIGRAPH (/C)	ID=93	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('0',0),				/* DIGRAPH (0)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3 		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('1',0),				/* DIGRAPH (1)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('2',0),				/* DIGRAPH (2)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('3',0),				/* DIGRAPH (3)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('4',0),				/* DIGRAPH (4)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('5',0),				/* DIGRAPH (5)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('6',0),				/* DIGRAPH (6)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('7',0),				/* DIGRAPH (7)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('8',0),				/* DIGRAPH (8)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH('9',0),				/* DIGRAPH (9)		*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0},					/* MOUTH HEIGHT, WIDTH	*/

 {DIGRAPH(' ',0),				/* DIGRAPH (' ')	*/
	NONE,					/* FEATURES		*/
  	0, 0, 0,				/* F1, F2, F3		*/
	0, 0, 0,				/* A1, A2, A3		*/
	0, 0,	 				/* MINDUR, INHDUR	*/
	0,					/* AV			*/
	0,					/* FRIC/ASP ID  	*/
	0, 0}					/* MOUTH HEIGHT, WIDTH	*/

  };
