/*****************************************************************************
*
*	$Id: AAAChipRegisters.h,v 41.0 92/11/23 10:58:58 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	AAAChipRegisters.h,v $
 * Revision 41.0  92/11/23  10:58:58  Jim2
 * Initial release.
 * 
*
******************************************************************************/
#ifndef REVSION
    #include "copdisAAA_rev.h"
#endif

	{"bltddat","blitter destination early read (dummy address)",ParseNothing, READREG},	/* 0x000 */
	{"dmaconr","DMA control (and blitter status) read",ParseNothing, READREG},	/* 0x002 */
	{"vposr","read vert most signif. bit (and frame flop)",ParseNothing, READREG},	/* 0x004 */
	{"vhposr","read vert and horiz. position of beam",ParseNothing, READREG},	/* 0x006 */
	{"dskdatr","disk data early read (dummy address)",ParseNothing, READREG},	/* 0x008 */
	{"joy0dat","joystick-mouse 0 data (vert,horiz)",ParseNothing, READREG},		/* 0x00a */
	{"joy1dat","joystick-mouse 1 data (vert,horiz)",ParseNothing, READREG},		/* 0x00c */
	{"clxdat","collision data (read and clear)",ParseNothing, READREG},		/* 0x00e */
	{"adkconr","audio, disk control register read",AAAADUControl, READREG},		/* 0x010 */
	{"pot0dat","pot counter pair 0 data (vert,horiz)",ParseNothing, READREG},	/* 0x012 */
	{"pot1dat","pot counter pair 1 data (vert,horiz)",ParseNothing, READREG},	/* 0x014 */
	{"potinp","pot port data read",ParseNothing, READREG},				/* 0x016 */
	{"serdatr","serial port data and status read",ParseNothing, READREG},		/* 0x018 */
	{"dskbytr","disk data byte and status read",ParseNothing, READREG},		/* 0x01a */
	{"intenar","interrupt enable bits read",ParseNothing, READREG},			/* 0x01c */
	{"intreqr","interrupt request bits read",ParseNothing, READREG},		/* 0x01e */

	{"dskpt","disk pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x020 */
	{"dskpt","disk pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x022 */
	{"dsklen","disk length",ParseNothing, 0},					/* 0x024 */
	{"dskdat","disk DMA data write",ParseNothing, 0},				/* 0x026 */

	{"refptr","refresh pointer",ParseNothing, 0},					/* 0x028 */
	{"vposw","write vert most signif. bit (and frame flop)",AAABeamHigh, 0},	/* 0x02a */
	{"vhposw","write vert and horiz position of beam",AAABeamLow, 0},		/* 0x02c */
	{"copcon","copper control register (CDANG)",AAACopperControl, 0},		/* 0x02e */
	{"serdat","serial port data and stop bits write",ParseNothing, 0},		/* 0x030 */
	{"serper","serial port period and control",ParseNothing, 0},			/* 0x032 */
	{"potgo","pot port data write and start",ParseNothing, 0},			/* 0x034 */
	{"joytest","write all four joystick-mouse counters",ParseNothing, 0},		/* 0x036 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x038 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x03a */
	{"unused","ERROR",ParseNothing, 0},						/* 0x03c */
	{"unused","ERROR",ParseNothing, 0},						/* 0x03e */

	{"bltcon0","blitter control register 0",AAABlitterControl0, 0},			/* 0x040 */
	{"bltcon1","blitter control register 1",AAABlitterControl1, 0},			/* 0x042 */
	{"bltafwm","blitter first word mask for source A",ParseNothing, 0},		/* 0x044 */
	{"bltalwm","blitter last word mask for source A",ParseNothing, 0},		/* 0x046 */
	{"bltcpt","blitter pointer to source C (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x048 */
	{"bltcpt","blitter pointer to source C (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x04a */
	{"bltbpt","blitter pointer to source B (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x04c */
	{"bltbpt","blitter pointer to source B (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x04e */
	{"bltapt","blitter pointer to source A (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x050 */
	{"bltapt","blitter pointer to source A (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x052 */
	{"bltdpt","blitter pointer to source D (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x054 */
	{"bltdpt","blitter pointer to source D (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x056 */
	{"bltsize","blitter start and size (window width,height)",AAABlitterSizeSm, 0},	/* 0x058 */
	{"bltcon0l","blitter control 0 lower 8 bits (minterms)",AAABlitterControl0Short, 0},	/* 0x05a */
	{"bltsizv","blitter V size",AAABlitterSizeMedV, 0},				/* 0x05c */
	{"bltsizh","blitter H size",AAABlitterSizeMedH, 0},				/* 0x05e */
	{"bltcmod","blitter modulo for source C",ParseNothing, 0},			/* 0x060 */
	{"bltbmod","blitter modulo for source B",ParseNothing, 0},			/* 0x062 */
	{"bltamod","blitter modulo for source A",ParseNothing, 0},			/* 0x064 */
	{"bltdmod","blitter modulo for source D",ParseNothing, 0},			/* 0x066 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x068 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x06a */
	{"unused","ERROR",ParseNothing, 0},						/* 0x06c */
	{"unused","ERROR",ParseNothing, 0},						/* 0x06e */
	{"bltcdat","blitter source C data",ParseNothing, 0},				/* 0x070 */
	{"bltbdat","blitter source B data",ParseNothing, 0},				/* 0x072 */
	{"bltadat","blitter source A data",ParseNothing, 0},				/* 0x074 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x076 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x078 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x07a */
	{"monicaid","Monica chip rev",ParseNothing, READREG},				/* 0x07c */
	{"dsksync","disk sync pattern for disk read",ParseNothing, 0},			/* 0x07e */

	{"cop1lc","copper first location (high 3 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x080 */
	{"cop1lc","copper first location (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x082 */
	{"cop2lc","copper second location (high 3 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x084 */
	{"cop2lc","copper second location (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x086 */
	{"copjmp1","restart copper at first location",DataStrobe, 0},			/* 0x088 */
	{"copjmp2","restart copper at second location",DataStrobe, 0},			/* 0x08a */
	{"copins","copper instruction fetch identify",ParseNothing, 0},			/* 0x08c */
	{"diwstrt","display window start (upper left vert-horiz position)",AAADisplayStart, 0},	/* 0x08e */
	{"diwstop","display window stop (lower right vert-horiz position)",AAADisplayStop, 0},	/* 0x090 */
	{"ddfstrt","display bit plane data fetch start (horiz. position)",AAADataFetch, 0},	/* 0x092 */
	{"ddfstop","display bit plane data fetch stop (horiz. position)",AAADataFetch, 0},	/* 0x094 */
	{"dmacon","DMA control write (clear or set)",AAADMA16, 0},			/* 0x096 */
	{"clxcon","collision control",AAACollision, 0},					/* 0x098 */
	{"intena","interrupt enable bits",AAAInterrupt, 0},				/* 0x09a */
	{"intreq","interrupt request bits",AAAInterrupt, 0},				/* 0x09c */
	{"adkcon","audio, UART, disk control",AAAADUControl, 0},			/* 0x09e */

	{"aud0lc","audio channel 0 location (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0a0 */
	{"aud0lc","audio channel 0 location (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0a2 */
	{"aud0len","audio channel 0 length", AAAAudioLength, 0},			/* 0x0a4 */
	{"aud0per","audio channel 0 period",AAAAudioCoursePeriod, 0},			/* 0x0a6 */
	{"aud0vol","audio channel 0 volume",AAAAudioVolumeUnsigned, 0},			/* 0x0a8 */
	{"aud0dat","audio channel 0 data",ParseNothing, 0},				/* 0x0aa */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0ac */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0ae */

	{"aud1lc","audio channel 1 location (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0b0 */
	{"aud1lc","audio channel 1 location (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0b2 */
	{"aud1len","audio channel 1 length", AAAAudioLength16, 0},			/* 0x0b4 */
	{"aud1per","audio channel 1 period",AAAAudioCoursePeriod, 0},			/* 0x0b6 */
	{"aud1vol","audio channel 1 volume",AAAAudioVolumeUnsigned, 0},			/* 0x0b8 */
	{"aud1dat","audio channel 1 data",ParseNothing, 0},				/* 0x0ba */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0bc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0be */

	{"aud2lc","audio channel 2 location (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0c0 */
	{"aud2lc","audio channel 2 location (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0c2 */
	{"aud2len","audio channel 2 length",AAAAudioLength16, 0},			/* 0x0c4 */
	{"aud2per","audio channel 2 period",AAAAudioCoursePeriod, 0},			/* 0x0c6 */
	{"aud2vol","audio channel 2 volume",AAAAudioVolumeUnsigned, 0},			/* 0x0c8 */
	{"aud2dat","audio channel 2 data",ParseNothing, 0},				/* 0x0ca */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0cc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0ce */

	{"aud3lc","audio channel 3 location (high 5 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0d0 */
	{"aud3lc","audio channel 3 location (low 15 bits)",ParseNothing, ADD_UNKNOWN},	/* 0x0d2 */
	{"aud3len","audio channel 3 length",AAAAudioLength16, 0},			/* 0x0d4 */
	{"aud3per","audio channel 3 period",AAAAudioCoursePeriod, 0},			/* 0x0d6 */
	{"aud3vol","audio channel 3 volume",AAAAudioVolumeUnsigned, 0},			/* 0x0d8 */
	{"aud3dat","audio channel 3 data",ParseNothing, 0},				/* 0x0da */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0dc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0de */

	{"bpl1pt","blitter plane 1 pointer (high 5 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e0 */
	{"bpl1pt","blitter plane 1 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e2 */
	{"bpl2pt","blitter plane 2 pointer (high 5 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e4 */
	{"bpl2pt","blitter plane 2 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e6 */
	{"bpl3pt","blitter plane 3 pointer (high 5 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e8 */
	{"bpl3pt","blitter plane 3 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0ea */
	{"bpl4pt","blitter plane 4 pointer (high 5 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0ec */
	{"bpl4pt","blitter plane 4 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0ee */
	{"bpl5pt","blitter plane 5 pointer (high 5 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f0 */
	{"bpl5pt","blitter plane 5 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f2 */
	{"bpl6pt","blitter plane 6 pointer (high 5 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f4 */
	{"bpl6pt","blitter plane 6 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f6 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0f8 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0fa */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0fc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0fe */


	{"bplcon0","bit plane control bits",AAABPMiscControl, 0},			/* 0x100 */
	{"bplcon1","bit plane scroll values for PF1 & PF2",AAAHScroll, 0},		/* 0x102 */
	{"bplcon2","bit plane priority control",AAABPPriority, 0},			/* 0x104 */
	{"bplcon3","bit plane enhanced",AAABPEnhance, 0},			       	/* 0x106 */

	{"bpl1mod","bit plane modulo for odd planes",AAABPModulo, 0},			/* 0x108 */
	{"bpl2mod","bit plane modulo for even planes",AAABPModulo, 0},			/* 0x10a */

	{"bplcon4","bit plane enhanced",AAABPSpriteMagic, 0},				/* 0x10c */
	{"unused","ERROR",ParseNothing, 0},						/* 0x10e */

	{"bpl1dat","bit plane 1 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x110 */
	{"bpl2dat","bit plane 2 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x112 */
	{"bpl3dat","bit plane 3 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x114 */
	{"bpl4dat","bit plane 4 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x116 */
	{"bpl5dat","bit plane 5 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x118 */
	{"bpl6dat","bit plane 6 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x11a */
	{"unused","ERROR",ParseNothing, 0},						/* 0x11c */
	{"unused","ERROR",ParseNothing, 0},						/* 0x11e */

	{"spr0pt","sprite 0 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x120 */
	{"spr0pt","sprite 0 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x122 */
	{"spr1pt","sprite 1 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x124 */
	{"spr1pt","sprite 1 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x126 */
	{"spr2pt","sprite 2 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x128 */
	{"spr2pt","sprite 2 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x12a */
	{"spr3pt","sprite 3 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x12c */
	{"spr3pt","sprite 3 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x12e */
	{"spr4pt","sprite 4 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x130 */
	{"spr4pt","sprite 4 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x132 */
	{"spr5pt","sprite 5 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x134 */
	{"spr5pt","sprite 5 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x136 */
	{"spr6pt","sprite 6 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x138 */
	{"spr6pt","sprite 6 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x13a */
	{"spr7pt","sprite 7 pointer (high 5 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x13c */
	{"spr7pt","sprite 7 pointer (low 15 bits)",ParseNothing, ADD_UNKNOWN},		/* 0x13e */

	{"spr0pos","sprite 0 vert-horiz start position",AAASpriteStart, 0},		/* 0x140 */
	{"spr0ctl","sprite 0 vert stop position and control data",AAASpriteControl, 0},	/* 0x142 */
	{"spr0data","sprite 0 image data register A",ParseNothing, 0},			/* 0x144 */
	{"spr0datb","sprite 0 image data register B",ParseNothing, 0},			/* 0x146 */
	{"spr1pos","sprite 1 vert-horiz start position",AAASpriteStart, 0},		/* 0x148 */
	{"spr1ctl","sprite 1 vert stop position and control data",AAASpriteControl, 0},	/* 0x14a */
	{"spr1data","sprite 1 image data register A",ParseNothing, 0},			/* 0x14c */
	{"spr1datb","sprite 1 image data register B",ParseNothing, 0},			/* 0x14e */
	{"spr2pos","sprite 2 vert-horiz start position",AAASpriteStart, 0},		/* 0x150 */
	{"spr2ctl","sprite 2 vert stop position and control data",AAASpriteControl, 0},	/* 0x152 */
	{"spr2data","sprite 2 image data register A",ParseNothing, 0},			/* 0x154 */
	{"spr2datb","sprite 2 image data register B",ParseNothing, 0},			/* 0x156 */
	{"spr3pos","sprite 3 vert-horiz start position",AAASpriteStart, 0},		/* 0x158 */
	{"spr3ctl","sprite 3 vert stop position and control data",AAASpriteControl, 0},	/* 0x15a */
	{"spr3data","sprite 3 image data register A",ParseNothing, 0},			/* 0x15c */
	{"spr3datb","sprite 3 image data register B",ParseNothing, 0},			/* 0x15e */
	{"spr4pos","sprite 4 vert-horiz start position",AAASpriteStart, 0},		/* 0x160 */
	{"spr4ctl","sprite 4 vert stop position and control data",AAASpriteControl, 0},	/* 0x162 */
	{"spr4data","sprite 4 image data register A",ParseNothing, 0},			/* 0x164 */
	{"spr4datb","sprite 4 image data register B",ParseNothing, 0},			/* 0x166 */
	{"spr5pos","sprite 5 vert-horiz start position",AAASpriteStart, 0},		/* 0x168 */
	{"spr5ctl","sprite 5 vert stop position and control data",AAASpriteControl, 0},	/* 0x16a */
	{"spr5data","sprite 5 image data register A",ParseNothing, 0},			/* 0x16c */
	{"spr5datb","sprite 5 image data register B",ParseNothing, 0},			/* 0x16e */
	{"spr6pos","sprite 6 vert-horiz start position",AAASpriteStart, 0},		/* 0x170 */
	{"spr6ctl","sprite 6 vert stop position and control data",AAASpriteControl, 0},	/* 0x172 */
	{"spr6data","sprite 6 image data register A",ParseNothing, 0},			/* 0x174 */
	{"spr6datb","sprite 6 image data register B",ParseNothing, 0},			/* 0x176 */
	{"spr7pos","sprite 7 vert-horiz start position",AAASpriteStart, 0},		/* 0x178 */
	{"spr7ctl","sprite 7 vert stop position and control data",AAASpriteControl, 0},	/* 0x17a */
	{"spr7data","sprite 7 image data register A",ParseNothing, 0},			/* 0x17c */
	{"spr7datb","sprite 7 image data register B",ParseNothing, 0},			/* 0x17e */

	{"color0","",AAAColor16, COLOR},						/* 0x180 */
	{"color1","",AAAColor16, COLOR},						/* 0x182 */
	{"color2","",AAAColor16, COLOR},						/* 0x184 */
	{"color3","",AAAColor16, COLOR},						/* 0x186 */
	{"color4","",AAAColor16, COLOR},						/* 0x188 */
	{"color5","",AAAColor16, COLOR},						/* 0x18a */
	{"color6","",AAAColor16, COLOR},						/* 0x18c */
	{"color7","",AAAColor16, COLOR},						/* 0x18e */
	{"color8","",AAAColor16, COLOR},						/* 0x190 */
	{"color9","",AAAColor16, COLOR},						/* 0x192 */
	{"color10","",AAAColor16, COLOR},						/* 0x194 */
	{"color11","",AAAColor16, COLOR},						/* 0x196 */
	{"color12","",AAAColor16, COLOR},						/* 0x198 */
	{"color13","",AAAColor16, COLOR},						/* 0x19a */
	{"color14","",AAAColor16, COLOR},						/* 0x19c */
	{"color15","",AAAColor16, COLOR},						/* 0x19e */
	{"color16","",AAAColor16, COLOR},						/* 0x1a0 */
	{"color17","",AAAColor16, COLOR},						/* 0x1a2 */
	{"color18","",AAAColor16, COLOR},						/* 0x1a4 */
	{"color19","",AAAColor16, COLOR},						/* 0x1a6 */
	{"color20","",AAAColor16, COLOR},						/* 0x1a8 */
	{"color21","",AAAColor16, COLOR},						/* 0x1aa */
	{"color22","",AAAColor16, COLOR},						/* 0x1ac */
	{"color23","",AAAColor16, COLOR},						/* 0x1ae */
	{"color24","",AAAColor16, COLOR},						/* 0x1b0 */
	{"color25","",AAAColor16, COLOR},						/* 0x1b2 */
	{"color26","",AAAColor16, COLOR},						/* 0x1b4 */
	{"color27","",AAAColor16, COLOR},						/* 0x1b6 */
	{"color28","",AAAColor16, COLOR},						/* 0x1b8 */
	{"color29","",AAAColor16, COLOR},						/* 0x1ba */
	{"color30","",AAAColor16, COLOR},						/* 0x1bc */
	{"color31","",AAAColor16, COLOR},						/* 0x1be */

	{"htotal","colour clocks per line",LowResColorClk, 0},				/* 0x1c0 */
	{"hsstop","HSYNC stop",LowResColorClk, 0},					/* 0x1c2 */
	{"hbstrt","HBLANK start",LowResColorClk, 0},					/* 0x1c4 */
	{"hbstop","HBLANK stop",LowResColorClk, 0},					/* 0x1c6 */
	{"vtotal","max rows",LineCount, 0},						/* 0x1c8 */
	{"vsstop","VSYNC stop",LineCount, 0},						/* 0x1ca */
	{"vbstrt","VBLANK start",LineCount, 0},						/* 0x1cc */
	{"vbstop","VBLANK stop",LineCount, 0},						/* 0x1ce */

	{"unused","ERROR",ParseNothing, 0},						/* 0x1d0 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1d2 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1d4 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1d6 */
	{"hhposw","dual mode highres H beam",LowResColorClk, 0},			/* 0x1d8 */
	{"hhposr","dual mode highres H bran",LowResColorClk, 0},			/* 0x1da */

	{"beamcon0","beam counter control",AAABeamCounter, 0},				/* 0x1dc */
	{"hsstrt","HSYNC start",LowResColorClk, 0},					/* 0x1de */
	{"vsstrt","VSYNC start", LineCount, 0},						/* 0x1e0 */
	{"hcenter","hpos for vsync in interlace",ParseNothing, 0},			/* 0x1e2 */
	{"diwhigh","MSB and LSB for display window",AAAXDisplay, 0},			/* 0x1e4 */

	{"unused","ERROR",ParseNothing, 0},						/* 0x1e6 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1e8 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1ea */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1ec */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1ee */

	{"aud4dat","audio channel 4 data",ParseNothing, 0},				/* 0x1f0 */
	{"aud5dat","audio channel 5 data",ParseNothing, 0},				/* 0x1f2 */
	{"aud6dat","audio channel 6 data",ParseNothing, 0},				/* 0x1f4 */
	{"aud7dat","audio channel 7 data",ParseNothing, 0},				/* 0x1f6 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1f8 */
	{"unused","ERROR",ParseNothing, 0},						/* 0x1fa */

	{"unused","ERROR",ParseNothing, 0},						/* 0x1fc */
	{"nop  ","space saver",ParseNothing, 0},					/* 0x1fe */

	{"aduleftr", "Read Left Audio Channel", ParseNothing, READREG},			/* 0x200 */
	{"bhposrx", "Extended Read Beam(hori, vert)", ParseNothing, READREG},		/* 0x204 */
	{"dskconrx", "Disk DMA Data Read", ParseNothing, READREG},			/* 0x208 */
	{"dmaconrx", "Extended DMA Control Read", ParseNothing, READREG},		/* 0x20C */
	{"adkconrx", "Extended Disk, Control read", ParseNothing, READREG},		/* 0x210 */
	{"dskbytrx", "Disk DATA word and status read", ParseNothing, READREG},		/* 0x214 */
	{"intreqrx", "Interrupt Request bits read", ParseNothing, READREG},		/* 0x218 */
	{"intenarx", "Interrupt Enable bits read", ParseNothing, READREG},		/* 0x21C */

	{"dskpllp", "Disk Phase Lock Loop Phase Adjust", ParseNothing, LONG_REG},	/* 0x220 */
	{"dskpllf", "Disk Phase Lock Loop Frequency Adjust", ParseNothing, LONG_REG},   /* 0x224 */

	{"dskdatx", "Disk DMA Data Write", ParseNothing, LONG_REG},			/* 0x228 */
	{"vhposwx", "Extended Write Beam(hori, vert)", AAABeamPos, LONG_REG},		/* 0x22C */

	{"dskpllr", "Disk Phase Lock Loop status read", ParseNothing, READREG},		/* 0x230 */
	{"dskptx", "Extended Disk Pointer (A[23:00])", ParseNothing, LONG_REG},		/* 0x234 */

	{"bltllenx", "Blitter line length read", ParseNothing, READREG},		/* 0x238 */
	{"bltpatrnr", "Blitter line draw pattern read", ParseNothing, READREG},		/* 0x23C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x240 */
	{"bltpmskx", "Plane Mask", ParseNothing, LONG_REG},				/* 0x244 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x248 */
	{"bltasrcptx", "Bit Map A Pointer", ParseNothing, ADD32_UNKNOWN},		/* 0x24C */
	{"bltbsrcptx", "Bit Map B Pointer", ParseNothing, ADD32_UNKNOWN},		/* 0x250 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x254 */
	{"bltdsrcptx", "Bit Map D Pointer", ParseNothing, ADD32_UNKNOWN},		/* 0x258 */
	{"bltasrcwdx", "Bit Map A Width", AAABitmapWidth, LONG_REG},			/* 0x25C */
	{"bltbsrcwdx", "Bit Map B Width", AAABitmapWidth, LONG_REG},			/* 0x260 */
	{"bltdsrcwdx", "Bit Map D Width", AAABitmapWidth, LONG_REG},			/* 0x264 */
	{"bltsizex", "Blit Size", AAABlitterSizeLar, LONG_REG},				/* 0x268 */
	{"bltaorgx", "Blit A Origin", AAABlitterPosition, ADD32_UNKNOWN},		/* 0x26C */
	{"bltborgx", "Blit B Origin", AAABlitterPosition, ADD32_UNKNOWN},		/* 0x270 */
	{"bltdorgx", "Blit D Origin", AAABlitterPosition, ADD32_UNKNOWN},		/* 0x274 */
	{"bltfuncx", "Blit Function and Start Register", AAABlitterFunction, LONG_REG},	/* 0x278 */

	{"dsksyncx", "Longword disk sync pattern for disk read", ParseNothing, LONG_REG},	/* 0x27C */

	{"cop1lc", "Extended coprocessor first location", ParseNothing, ADD32_UNKNOWN},	/* 0x280 */
	{"cop2lc", "Extended coprocessor second location", ParseNothing, ADD32_UNKNOWN},/* 0x284 */

	{"unused", "ERROR", ParseNothing, 0}, 						/* 0x288 */

	{"adkconx", "Extended Disk, Control write", AAADiskControl, LONG_REG},		/* 0x28C */
	{"dmaconx", "Extended DMA control write", AAADMA32, LONG_REG},			/* 0x290 */
	{"intenax", "Interrupt Enable bits (clear or set)", AAAInterruptXtend, LONG_REG},	/* 0x294 */
	{"clxconx", "Extended Collision Control", AAACollisionXtend, LONG_REG},		/* 0x298 */
	{"intreqx", "Interrupt Request bits (clear or set)", AAAInterruptXtend, LONG_REG},	/* 0x29C */

	{"aud0lcx", "Extended Audio channel 0 backup pointer", ParseNothing, ADD32_UNKNOWN},	/* 0x2A0 */
	{"aud0lenx", "Extended Audio channel 0 length", AAAAudioLength, LONG_REG},	/* 0x2A4 */
	{"aud0perx", "Extended Audio channel 0 period", AAAAudioPeriod, LONG_REG},	/* 0x2A8 */
	{"aud0volx", "Extended Audio channel 0 volume", AAAAudioVolumeSigned, LONG_REG},/* 0x2AC */
	{"aud0datx", "Extended Audio channel 0 data", ParseNothing, LONG_REG},		/* 0x2B0 */
	{"aud0ctl", "Audio channel 0 control bits", AAAAudioControl, LONG_REG},		/* 0x2B4 */

	{"aud1lcx", "Extended Audio channel 1 backup pointer", ParseNothing, ADD32_UNKNOWN},	/* 0x2B8 */
	{"aud1lenx", "Extended Audio channel 1 length", AAAAudioLength, LONG_REG},	/* 0x2BC */
	{"aud1perx", "Extended Audio channel 1 period", AAAAudioPeriod, LONG_REG},	/* 0x2C0 */
	{"aud1volx", "Extended Audio channel 1 volume", AAAAudioVolumeSigned, LONG_REG},/* 0x2C4 */
	{"aud1datx", "Extended Audio channel 1 data", ParseNothing, LONG_REG},		/* 0x2C8 */
	{"aud1ctl", "Audio channel 1 control bits", AAAAudioControl, LONG_REG},		/* 0x2CC */

	{"aud2lcx", "Extended Audio channel 2 backup pointer", ParseNothing, ADD32_UNKNOWN},	/* 0x2D0 */
	{"aud2lenx", "Extended Audio channel 2 length", AAAAudioLength, LONG_REG},	/* 0x2D4 */
	{"aud2perx", "Extended Audio channel 2 period", AAAAudioPeriod, LONG_REG},	/* 0x2D8 */
	{"aud2volx", "Extended Audio channel 2 volume", AAAAudioVolumeSigned, LONG_REG},/* 0x2DC */
	{"aud2datx", "Extended Audio channel 2 data", ParseNothing, LONG_REG},		/* 0x2E0 */
	{"aud2ctl", "Audio channel 2 control bits", AAAAudioControl, LONG_REG},		/* 0x2E4 */

	{"aud3lcx", "Extended Audio channel 3 backup pointer", ParseNothing, ADD32_UNKNOWN},	/* 0x2E8 */
	{"aud3lenx", "Extended Audio channel 3 length", AAAAudioLength, LONG_REG},	/* 0x2EC */
	{"aud3perx", "Extended Audio channel 3 period", AAAAudioPeriod, LONG_REG},	/* 0x2F0 */
	{"aud3volx", "Extended Audio channel 3 volume", AAAAudioVolumeSigned, LONG_REG},/* 0x2F4 */
	{"aud3datx", "Extended Audio channel 3 data", ParseNothing, LONG_REG},		/* 0x2F8 */
	{"aud3ctl", "Audio channel 3 control bits", AAAAudioControl, LONG_REG},		/* 0x2FC */

	{"bpl1datx", "Extended Bitplane data register 1", ParseNothing, LONG_REG},	/* 0x300 */
	{"bpl2datx", "Extended Bitplane data register 2", ParseNothing, LONG_REG},	/* 0x304 */
	{"bpl3datx", "Extended Bitplane data register 3", ParseNothing, LONG_REG},	/* 0x308 */
	{"bpl4datx", "Extended Bitplane data register 4", ParseNothing, LONG_REG},	/* 0x30C */
	{"bpl5datx", "Extended Bitplane data register 5", ParseNothing, LONG_REG},	/* 0x310 */
	{"bpl6datx", "Extended Bitplane data register 6", ParseNothing, LONG_REG},	/* 0x314 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x318 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x31C */

	{"spr0ptx", "Extended SPRITE 0 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x320 */
	{"spr1ptx", "Extended SPRITE 1 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x324 */
	{"spr2ptx", "Extended SPRITE 2 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x328 */
	{"spr3ptx", "Extended SPRITE 3 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x32C */
	{"spr4ptx", "Extended SPRITE 4 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x330 */
	{"spr5ptx", "Extended SPRITE 5 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x334 */
	{"spr6ptx", "Extended SPRITE 6 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x338 */
	{"spr7ptx", "Extended SPRITE 7 pointer register", ParseNothing, ADD32_UNKNOWN},	/* 0x33C */

	{"spr0datax", "Extended Sprite 0 image data registerA", ParseNothing, LONG_REG},/* 0x340 */
	{"spr0datbx", "Extended Sprite 0 image data registerB", ParseNothing, LONG_REG},/* 0x344 */
	{"spr1datax", "Extended Sprite 1 image data registerA", ParseNothing, LONG_REG},/* 0x348 */
	{"spr1datbx", "Extended Sprite 1 image data registerB", ParseNothing, LONG_REG},/* 0x34C */
	{"spr2datax", "Extended Sprite 2 image data registerA", ParseNothing, LONG_REG},/* 0x350 */
	{"spr2datbx", "Extended Sprite 2 image data registerB", ParseNothing, LONG_REG},/* 0x354 */
	{"spr3datax", "Extended Sprite 3 image data registerA", ParseNothing, LONG_REG},/* 0x358 */
	{"spr3datbx", "Extended Sprite 3 image data registerB", ParseNothing, LONG_REG},/* 0x35C */
	{"spr4datax", "Extended Sprite 4 image data registerA", ParseNothing, LONG_REG},/* 0x360 */
	{"spr4datbx", "Extended Sprite 4 image data registerB", ParseNothing, LONG_REG},/* 0x364 */
	{"spr5datax", "Extended Sprite 5 image data registerA", ParseNothing, LONG_REG},/* 0x368 */
	{"spr5datbx", "Extended Sprite 5 image data registerA", ParseNothing, LONG_REG},/* 0x36C */
	{"spr6datax", "Extended Sprite 6 image data registerB", ParseNothing, LONG_REG},/* 0x370 */
	{"spr6datbx", "Extended Sprite 6 image data registerA", ParseNothing, LONG_REG},/* 0x374 */
	{"spr7datax", "Extended Sprite 7 image data registerA", ParseNothing, LONG_REG},/* 0x378 */
	{"spr7datbx", "Extended Sprite 7 image data registerB", ParseNothing, LONG_REG},/* 0x37C */

	{"spr0posx", "Extended Sprite 0 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x380 */
	{"spr0ctlx", "Extended Sprite 0 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x384 */
	{"spr1posx", "Extended Sprite 1 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x388 */
	{"spr1ctlx", "Extended Sprite 1 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x38C */
	{"spr2posx", "Extended Sprite 2 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x390 */
	{"spr2ctlx", "Extended Sprite 2 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x394 */
	{"spr3posx", "Extended Sprite 3 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x398 */
	{"spr3ctlx", "Extended Sprite 3 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x39C */
	{"spr4posx", "Extended Sprite 4 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x3A0 */
	{"spr4ctlx", "Extended Sprite 4 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x3A4 */
	{"spr5posx", "Extended Sprite 5 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x3A8 */
	{"spr5ctlx", "Extended Sprite 5 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x3AC */
	{"spr6posx", "Extended Sprite 6 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x3B0 */
	{"spr6ctlx", "Extended Sprite 6 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x3B4 */
	{"spr7posx", "Extended Sprite 7 Vertical Start/Stop", AAASpriteStartXtend, LONG_REG},	/* 0x3B8 */
	{"spr7ctlx", "Extended Sprite 7 position and control", AAASpriteControlXtend, LONG_REG},	/* 0x3BC */

	{"htotalx", "Extended horiz line count", HighResColorClk, LONG_REG},		/* 0x3C0 */

	{"diwstrtx", "Extended Display Window Start", AAADisplayNew, LONG_REG},		/* 0x3C4 */
	{"diwstopx", "Extended Display Window Stop", AAADisplayNew, LONG_REG},		/* 0x3C8 */

	{"hbstopx", "Extended horiz blank stop", HighResColorClk, LONG_REG},		/* 0x3CC */
	{"hbstrtx", "Extended horiz blank start", HighResColorClk, LONG_REG},		/* 0x3D0 */
	{"hcenterx", "Extended horiz center", HighResColorClk, LONG_REG},		/* 0x3D4 */

	{"hsstopx", "Extended horiz sync stop", HighResColorClk, LONG_REG},		/* 0x3D8 */
	{"hsstrtx", "Extended horiz sync start", HighResColorClk, LONG_REG},		/* 0x3DC */

	{"bpl1ptx", "Extended Bitplane 1 pointer", ParseNothing, ADD32_BITPLANE},	/* 0x3E0 */
	{"bpl2ptx", "Extended Bitplane 2 pointer", ParseNothing, ADD32_BITPLANE},	/* 0x3E4 */
	{"bpl3ptx", "Extended Bitplane 3 pointer", ParseNothing, ADD32_BITPLANE},	/* 0x3E8 */
	{"bpl4ptx", "Extended Bitplane 4 pointer", ParseNothing, ADD32_BITPLANE},	/* 0x3EC */
	{"bpl5ptx", "Extended Bitplane 5 pointer", ParseNothing, ADD32_BITPLANE},	/* 0x3E0 */
	{"bpl6ptx", "Extended Bitplane 6 pointer", ParseNothing, ADD32_BITPLANE},	/* 0x3F4 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x3F8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x3FC */

	{"ramatrn", "RAM Bank Attributes Read", ParseNothing, READREG},			/* 0x400 */
	{"cointrer", "Coprocessor interrupt request/enable read", ParseNothing, READREG},	/* 0x404 */
	{"pritest", "Read Processor and Blitter priority control", ParseNothing, READREG},	/* 0x408 */
	{"bltromd", "Blitter ROM data", ParseNothing, READREG},				/* 0x40C */
	{"audtst0r", "Read audio cycle addr, ctrl state, and data", ParseNothing, READREG},	/* 0x410 */
	{"audtst1r", "Read audio cycle control and test control", ParseNothing, READREG},	/* 0x414 */
	{"audrightr", "Read right audio channel", ParseNothing, READREG},		/* 0x418 */
	{"serdatr2n", "Serial Port 2 Data and Status", ParseNothing, READREG},		/* 0x41C */
	{"audtst0w", "Write audio cycle addr, ctrl state, and data", ParseNothing, LONG_REG},	/* 0x420 */
	{"generalr", "General purpose register read", ParseNothing, READREG},		/* 0x424 */
	{"vpposrn", "Read Primary vertical position of beam", ParseNothing, READREG},	/* 0x428 */
	{"blitenrn", "Blitter Enable read", ParseNothing, READREG},			/* 0x42C */
	{"priset", "Write Processor and Blitter priority control", ParseNothing, LONG_REG},	/* 0x430 */
	{"bliten", "Blitter Enable", AAABlitterEnable, LONG_REG},				/* 0x434 */
	{"audtst1w", "Write audio cycle control and test control", ParseNothing, LONG_REG},	/* 0x438 */

	{"bltlclpax", "Blitter Clip Rectangle, Source A", AAABlitterPosition, LONG_REG},/* 0x43C */
	{"bltlclpbx", "Blitter Clip Rectangle, Source B", AAABlitterPosition, LONG_REG},/* 0x440 */
	{"bltlcolorx", "Blitter Line Color", ParseNothing, LONG_REG},			/* 0x444 */
	{"bltlpatrnx", "Blitter Line Pattern", ParseNothing, LONG_REG},			/* 0x448 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x44C */

	{"bltlpmskx", "Blitter Line Plane Mask", ParseNothing, LONG_REG},		/* 0x450 */
	{"bltlmapptx", "Blitter Line Source Pointer", ParseNothing, ADD32_UNKNOWN},	/* 0x454 */
	{"bltlmapwdx", "Blitter Line Width", AAABitmapWidth, LONG_REG},			/* 0x458 */
	{"bltlend1x", "Blitter Line End Point 1", AAABlitterPosition, LONG_REG},	/* 0x45C */
	{"bltlend2x", "Blitter Line End Point 2", AAABlitterPosition, LONG_REG},	/* 0x460 */
	{"bltlfuncx", "Blitter Line Function and Start", AAABlitterFunctionLine, LONG_REG},	/* 0x464 */
	{"bltadatx", "Blitter Source A Data", ParseNothing, ADD32_UNKNOWN},		/* 0x468 */
	{"bltbdatx", "Blitter Source B Data", ParseNothing, ADD32_UNKNOWN},		/* 0x46C */
	{"bltcdatx", "Blitter Source C Data", ParseNothing, ADD32_UNKNOWN},		/* 0x470 */
	{"bltddatx", "Blitter Source D Data", ParseNothing, ADD32_UNKNOWN},		/* 0x474 */
	{"bltedatx", "Blitter Source E Data", ParseNothing, ADD32_UNKNOWN},		/* 0x478 */

	{"ramatwx", "RAM Bank Attributes write", ParseNothing, LONG_REG},		/* 0x47C */
	{"blttst", "Blitter ROM test", ParseNothing, LONG_REG},				/* 0x480 */
	{"serdat2n", "Serial Port 2 Data and stop bits write", ParseNothing, LONG_REG},	/* 0x484 */
	{"monitorid", "External Hardware Monitor ID", ParseNothing, READREG},		/* 0x488 */
	{"serpern", "Serial Port 2 Perion and control", ParseNothing, LONG_REG},	/* 0x48C */

	{"bploffr", "Bitplane and Sprite LUT Address Offset read", ParseNothing, READREG},	/* 0x490 */
	{"clxconoen", "Chunky Pixel collision control; enable", AAACollisionCHUNKYEnable, LONG_REG},	/* 0x494 */
	{"clxconodn", "Chunky Pixel collision control; match", AAACollisionCHUNKYMatch, LONG_REG},	/* 0x498 */
	{"montest", "MONICA test", MonicaTest, LONG_REG},				/* 0x49C */

	{"aud4lc", "Audio channel 4 location", ParseNothing, ADD32_UNKNOWN},		/* 0x4A0 */
	{"aud4len", "Audio channel 4 length", AAAAudioLength, LONG_REG},		/* 0x4A4 */
	{"aud4per", "Audio channel 4 period", AAAAudioPeriod, LONG_REG},		/* 0x4A8 */
	{"aud4vol", "Audio channel 4 volume", AAAAudioVolumeSigned, LONG_REG},		/* 0x4AC */
	{"aud4dat", "Audio channel 4 data", ParseNothing, LONG_REG},			/* 0x4B0 */
	{"aud4ctl", "Audio channel 4 control bits", AAAAudioControl, LONG_REG},		/* 0x4B4 */

	{"aud5lc", "Audio channel 5 location", ParseNothing, ADD32_UNKNOWN},		/* 0x4B8 */
	{"aud5len", "Audio channel 5 length", AAAAudioLength, LONG_REG},		/* 0x4BC */
	{"aud5per", "Audio channel 5 period", AAAAudioPeriod, LONG_REG},		/* 0x4C0 */
	{"aud5vol", "Audio channel 5 volume", AAAAudioVolumeSigned, LONG_REG},		/* 0x4C4 */
	{"aud5dat", "Audio channel 5 data", ParseNothing, LONG_REG},			/* 0x4C8 */
	{"aud5ctl", "Audio channel 5 control bits", AAAAudioControl, LONG_REG},		/* 0x4CC */

	{"aud6lc", "Audio channel 6 location", ParseNothing, ADD32_UNKNOWN},		/* 0x4D0 */
	{"aud6len", "Audio channel 6 length", AAAAudioLength, LONG_REG},		/* 0x4D4 */
	{"aud6per", "Audio channel 6 period", AAAAudioPeriod, LONG_REG},		/* 0x4D8 */
	{"aud6vol", "Audio channel 6 volume", AAAAudioVolumeSigned, LONG_REG},		/* 0x4DC */
	{"aud6dat", "Audio channel 6 data", ParseNothing, LONG_REG},			/* 0x4E0 */
	{"aud6ctl", "Audio channel 6 control bits", AAAAudioControl, LONG_REG},		/* 0x4E4 */

	{"aud7lc", "Audio channel 7 location", ParseNothing, ADD32_UNKNOWN},		/* 0x4E8 */
	{"aud7len", "Audio channel 7 length", AAAAudioLength, LONG_REG},		/* 0x4EC */
	{"aud7per", "Audio channel 7 period", AAAAudioPeriod, LONG_REG},		/* 0x4F0 */
	{"aud7vol", "Audio channel 7 volume", AAAAudioVolumeSigned, LONG_REG},		/* 0x4F4 */
	{"aud7dat", "Audio channel 7 data", ParseNothing, LONG_REG},			/* 0x4F8 */
	{"aud7ctl", "Audio channel 7 control bits", AAAAudioControl, LONG_REG},		/* 0x4FC */

	{"graphics", "Graphics fetch cycle", ParseNothing, LONG_REG},			/* 0x500 */
	{"bploffn", "Bitplane and sprite offset", AAALUTOffset, LONG_REG},		/* 0x504 */

	{"bpl7datn", "New Bit plane 7 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x508 */
	{"bpl8datn", "New Bit plane 8 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x50C */
	{"bpl9datn", "New Bit plane 9 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x510 */
	{"bpl10datn", "New Bit plane 10 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x514 */
	{"bpl11datn", "New Bit plane 11 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x518 */
	{"bpl12datn", "New Bit plane 12 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x51C */
	{"bpl13datn", "New Bit plane 13 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x520 */
	{"bpl14datn", "New Bit plane 14 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x524 */
	{"bpl15datn", "New Bit plane 15 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x528 */
	{"bpl16datn", "New Bit plane 16 data(Parallel to serial)", ParseNothing, LONG_REG},	/* 0x52C */
	{"bplovrdat", "Overlay Bitplane data(Parallel to serial)", ParseNothing, ADD32_UNKNOWN},	/* 0x530 */

	{"copwait", "Coprocessor wait interrupt routine start", ParseNothing, ADD32_UNKNOWN},	/* 0x534 */
	{"copjmp3", "Coprocessor restart at third location", DataStrobe, 0},		/* 0x538 */
	{"copjmp4", "Coprocessor restart at fourth location", DataStrobe, 0},		/* 0x53C */
	{"cop3lc", "Coprocessor third jump location", ParseNothing, ADD32_UNKNOWN},	/* 0x540 */
	{"cop4lc", "Coprocessor fourth jump location", ParseNothing, ADD32_UNKNOWN},	/* 0x544 */
	{"copbltlc", "Blitter Finished interrupt start", ParseNothing, ADD32_UNKNOWN},	/* 0x548 */
	{"copret", "Coprocessor return from interupt", DataStrobe, 0},			/* 0x54C */
	{"cointrew", "Coprocessor interrupt request/enable write", ParseNothing, LONG_REG},	/* 0x550 */
	{"copretc", "Coprocessor return from blitter interrupt", DataStrobe, 0},	/* 0x554 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x558 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x55C */

	{"generalw", "General purpose, write", ParseNothing, LONG_REG},			/* 0x560 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x564 */

	{"brststrt", "Burst start", HighResColorClk, LONG_REG},				/* 0x568 */
	{"brststop", "Burst stop", HighResColorClk, LONG_REG},				/* 0x56C */
	{"equ1stop", "First equalization pulse stop position", HighResColorClk, LONG_REG},	/* 0x570 */
	{"equ2stop", "Second equalization pulse stop position", HighResColorClk, LONG_REG},	/* 0x574 */
	{"ser1stop", "First serration pulse stop position", HighResColorClk, LONG_REG},	/* 0x578 */
	{"ser2stop", "Second serration pulse stop position", HighResColorClk, LONG_REG},/* 0x57C */
	{"vequstop", "Verical equalization stop line", HighWordLineCount, LONG_REG},	/* 0x580 */
	{"clcntr", "Composite Line Counter", HighResColorClk, LONG_REG},		/* 0x584 */
	{"mlsync", "Mid Line Sync.", HighResColorClk, LONG_REG},			/* 0x588 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x58C */

	{"dskcrci", "Initial CRC value", ParseNothing, LONG_REG},			/* 0x590 */
	{"dskgap", "Disk gap timer", ParseNothing, LONG_REG},				/* 0x594 */
	{"dkslenn", "New mode length", ParseNothing, LONG_REG},				/* 0x598 */
	{"dkshdpt", "New disk DMA header pointer", ParseNothing, LONG_REG},		/* 0x59C */
	{"dksbkpt", "New disk DMA backup pointer", ParseNothing, ADD32_UNKNOWN},	/* 0x5A0 */
	{"dkstst", "Disk test", ParseNothing, LONG_REG},				/* 0x5A4 */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x5A8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5AC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5B0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5B4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5B8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5BC */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x5C0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5C4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5C8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5CC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5D0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x5D4 */

	{"bpl7pt", "New bit plane 7 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5D8 */
	{"bpl8pt", "New bit plane 8 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5DC */
	{"bpl9pt", "New bit plane 9 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5E0 */
	{"bpl10pt", "New bit plane 10 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5E4 */
	{"bpl11pt", "New bit plane 11 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5E8 */
	{"bpl12pt", "New bit plane 12 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5EC */
	{"bpl13pt", "New bit plane 13 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5F0 */
	{"bpl14pt", "New bit plane 14 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5F4 */
	{"bpl15pt", "New bit plane 15 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5F8 */
	{"bpl16pt", "New bit plane 16 pointer", ParseNothing, ADD32_BITPLANE},		/* 0x5FC */

	{"unused", "ERROR", ParseNothing, 0},						/* 0x600 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x604 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x608 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x60C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x610 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x614 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x618 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x61C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x620 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x624 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x628 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x62C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x630 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x634 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x638 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x63C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x640 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x644 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x648 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x64C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x650 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x654 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x658 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x65C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x660 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x664 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x668 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x66C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x670 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x674 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x678 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x67C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x680 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x684 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x688 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x68C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x690 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x694 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x698 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x69C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6A0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6A4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6A8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6AC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6B0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6B4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6B8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6BC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6C0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6C4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6C8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6CC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6D0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6D4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6D8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6DC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6E0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6E4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6E8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6EC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6F0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6F4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6F8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x6FC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x700 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x704 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x708 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x70C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x710 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x714 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x718 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x71C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x720 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x724 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x728 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x72C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x730 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x734 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x738 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x73C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x740 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x744 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x748 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x74C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x750 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x754 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x758 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x75C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x760 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x764 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x768 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x76C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x770 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x774 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x778 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x77C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x780 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x784 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x788 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x78C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x790 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x794 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x798 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x79C */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7A0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7A4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7A8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7AC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7B0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7B4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7B8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7BC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7C0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7C4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7C8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7CC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7D0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7D4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7D8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7DC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7E0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7E4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7E8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7EC */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7F0 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7F4 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7F8 */
	{"unused", "ERROR", ParseNothing, 0},						/* 0x7FC */

	{"color000rx", "", ParseNothing, READREG},					/* 0x800 */
	{"color001rx", "", ParseNothing, READREG},					/* 0x804 */
	{"color002rx", "", ParseNothing, READREG},					/* 0x808 */
	{"color003rx", "", ParseNothing, READREG},					/* 0x80C */
	{"color004rx", "", ParseNothing, READREG},					/* 0x810 */
	{"color005rx", "", ParseNothing, READREG},					/* 0x814 */
	{"color006rx", "", ParseNothing, READREG},					/* 0x818 */
	{"color007rx", "", ParseNothing, READREG},					/* 0x81C */
	{"color008rx", "", ParseNothing, READREG},					/* 0x820 */
	{"color009rx", "", ParseNothing, READREG},					/* 0x824 */
	{"color010rx", "", ParseNothing, READREG},					/* 0x828 */
	{"color011rx", "", ParseNothing, READREG},					/* 0x82C */
	{"color012rx", "", ParseNothing, READREG},					/* 0x830 */
	{"color013rx", "", ParseNothing, READREG},					/* 0x834 */
	{"color014rx", "", ParseNothing, READREG},					/* 0x838 */
	{"color015rx", "", ParseNothing, READREG},					/* 0x83C */
	{"color016rx", "", ParseNothing, READREG},					/* 0x840 */
	{"color017rx", "", ParseNothing, READREG},					/* 0x844 */
	{"color018rx", "", ParseNothing, READREG},					/* 0x848 */
	{"color019rx", "", ParseNothing, READREG},					/* 0x84C */
	{"color020rx", "", ParseNothing, READREG},					/* 0x850 */
	{"color021rx", "", ParseNothing, READREG},					/* 0x854 */
	{"color022rx", "", ParseNothing, READREG},					/* 0x858 */
	{"color023rx", "", ParseNothing, READREG},					/* 0x85C */
	{"color024rx", "", ParseNothing, READREG},					/* 0x860 */
	{"color025rx", "", ParseNothing, READREG},					/* 0x864 */
	{"color026rx", "", ParseNothing, READREG},					/* 0x868 */
	{"color027rx", "", ParseNothing, READREG},					/* 0x86C */
	{"color028rx", "", ParseNothing, READREG},					/* 0x870 */
	{"color029rx", "", ParseNothing, READREG},					/* 0x874 */
	{"color020rx", "", ParseNothing, READREG},					/* 0x878 */
	{"color031rx", "", ParseNothing, READREG},					/* 0x87C */
	{"color032rx", "", ParseNothing, READREG},					/* 0x880 */
	{"color033rx", "", ParseNothing, READREG},					/* 0x884 */
	{"color034rx", "", ParseNothing, READREG},					/* 0x888 */
	{"color035rx", "", ParseNothing, READREG},					/* 0x88C */
	{"color036rx", "", ParseNothing, READREG},					/* 0x890 */
	{"color037rx", "", ParseNothing, READREG},					/* 0x894 */
	{"color038rx", "", ParseNothing, READREG},					/* 0x898 */
	{"color039rx", "", ParseNothing, READREG},					/* 0x89C */
	{"color040rx", "", ParseNothing, READREG},					/* 0x8A0 */
	{"color041rx", "", ParseNothing, READREG},					/* 0x8A4 */
	{"color042rx", "", ParseNothing, READREG},					/* 0x8A8 */
	{"color043rx", "", ParseNothing, READREG},					/* 0x8AC */
	{"color044rx", "", ParseNothing, READREG},					/* 0x8B0 */
	{"color045rx", "", ParseNothing, READREG},					/* 0x8B4 */
	{"color046rx", "", ParseNothing, READREG},					/* 0x8B8 */
	{"color047rx", "", ParseNothing, READREG},					/* 0x8BC */
	{"color048rx", "", ParseNothing, READREG},					/* 0x8C0 */
	{"color049rx", "", ParseNothing, READREG},					/* 0x8C4 */
	{"color050rx", "", ParseNothing, READREG},					/* 0x8C8 */
	{"color051rx", "", ParseNothing, READREG},					/* 0x8CC */
	{"color052rx", "", ParseNothing, READREG},					/* 0x8D0 */
	{"color053rx", "", ParseNothing, READREG},					/* 0x8D4 */
	{"color054rx", "", ParseNothing, READREG},					/* 0x8D8 */
	{"color055rx", "", ParseNothing, READREG},					/* 0x8DC */
	{"color056rx", "", ParseNothing, READREG},					/* 0x8E0 */
	{"color057rx", "", ParseNothing, READREG},					/* 0x8E4 */
	{"color058rx", "", ParseNothing, READREG},					/* 0x8E8 */
	{"color059rx", "", ParseNothing, READREG},					/* 0x8EC */
	{"color060rx", "", ParseNothing, READREG},					/* 0x8F0 */
	{"color061rx", "", ParseNothing, READREG},					/* 0x8F4 */
	{"color062rx", "", ParseNothing, READREG},					/* 0x8F8 */
	{"color063rx", "", ParseNothing, READREG},					/* 0x8FC */
	{"color064rx", "", ParseNothing, READREG},					/* 0x900 */
	{"color065rx", "", ParseNothing, READREG},					/* 0x904 */
	{"color066rx", "", ParseNothing, READREG},					/* 0x908 */
	{"color067rx", "", ParseNothing, READREG},					/* 0x90C */
	{"color068rx", "", ParseNothing, READREG},					/* 0x910 */
	{"color069rx", "", ParseNothing, READREG},					/* 0x914 */
	{"color060rx", "", ParseNothing, READREG},					/* 0x918 */
	{"color071rx", "", ParseNothing, READREG},					/* 0x91C */
	{"color072rx", "", ParseNothing, READREG},					/* 0x920 */
	{"color073rx", "", ParseNothing, READREG},					/* 0x924 */
	{"color074rx", "", ParseNothing, READREG},					/* 0x928 */
	{"color075rx", "", ParseNothing, READREG},					/* 0x92C */
	{"color076rx", "", ParseNothing, READREG},					/* 0x930 */
	{"color077rx", "", ParseNothing, READREG},					/* 0x934 */
	{"color078rx", "", ParseNothing, READREG},					/* 0x938 */
	{"color079rx", "", ParseNothing, READREG},					/* 0x93C */
	{"color080rx", "", ParseNothing, READREG},					/* 0x940 */
	{"color081rx", "", ParseNothing, READREG},					/* 0x944 */
	{"color082rx", "", ParseNothing, READREG},					/* 0x948 */
	{"color083rx", "", ParseNothing, READREG},					/* 0x94C */
	{"color084rx", "", ParseNothing, READREG},					/* 0x950 */
	{"color085rx", "", ParseNothing, READREG},					/* 0x954 */
	{"color086rx", "", ParseNothing, READREG},					/* 0x958 */
	{"color087rx", "", ParseNothing, READREG},					/* 0x95C */
	{"color088rx", "", ParseNothing, READREG},					/* 0x960 */
	{"color089rx", "", ParseNothing, READREG},					/* 0x964 */
	{"color090rx", "", ParseNothing, READREG},					/* 0x968 */
	{"color091rx", "", ParseNothing, READREG},					/* 0x96C */
	{"color092rx", "", ParseNothing, READREG},					/* 0x970 */
	{"color093rx", "", ParseNothing, READREG},					/* 0x974 */
	{"color094rx", "", ParseNothing, READREG},					/* 0x978 */
	{"color095rx", "", ParseNothing, READREG},					/* 0x97C */
	{"color096rx", "", ParseNothing, READREG},					/* 0x980 */
	{"color097rx", "", ParseNothing, READREG},					/* 0x984 */
	{"color098rx", "", ParseNothing, READREG},					/* 0x988 */
	{"color099rx", "", ParseNothing, READREG},					/* 0x98C */
	{"color100rx", "", ParseNothing, READREG},					/* 0x990 */
	{"color101rx", "", ParseNothing, READREG},					/* 0x994 */
	{"color102rx", "", ParseNothing, READREG},					/* 0x998 */
	{"color103rx", "", ParseNothing, READREG},					/* 0x99C */
	{"color104rx", "", ParseNothing, READREG},					/* 0x9A0 */
	{"color105rx", "", ParseNothing, READREG},					/* 0x9A4 */
	{"color106rx", "", ParseNothing, READREG},					/* 0x9A8 */
	{"color107rx", "", ParseNothing, READREG},					/* 0x9AC */
	{"color108rx", "", ParseNothing, READREG},					/* 0x9B0 */
	{"color109rx", "", ParseNothing, READREG},					/* 0x9B4 */
	{"color110rx", "", ParseNothing, READREG},					/* 0x9B8 */
	{"color111rx", "", ParseNothing, READREG},					/* 0x9BC */
	{"color112rx", "", ParseNothing, READREG},					/* 0x9C0 */
	{"color113rx", "", ParseNothing, READREG},					/* 0x9C4 */
	{"color114rx", "", ParseNothing, READREG},					/* 0x9C8 */
	{"color115rx", "", ParseNothing, READREG},					/* 0x9CC */
	{"color116rx", "", ParseNothing, READREG},					/* 0x9D0 */
	{"color117rx", "", ParseNothing, READREG},					/* 0x9D4 */
	{"color118rx", "", ParseNothing, READREG},					/* 0x9D8 */
	{"color119rx", "", ParseNothing, READREG},					/* 0x9DC */
	{"color110rx", "", ParseNothing, READREG},					/* 0x9E0 */
	{"color121rx", "", ParseNothing, READREG},					/* 0x9E4 */
	{"color122rx", "", ParseNothing, READREG},					/* 0x9E8 */
	{"color123rx", "", ParseNothing, READREG},					/* 0x9EC */
	{"color124rx", "", ParseNothing, READREG},					/* 0x9F0 */
	{"color125rx", "", ParseNothing, READREG},					/* 0x9F4 */
	{"color126rx", "", ParseNothing, READREG},					/* 0x9F8 */
	{"color127rx", "", ParseNothing, READREG},					/* 0x9FC */
	{"color128rx", "", ParseNothing, READREG},					/* 0xA00 */
	{"color129rx", "", ParseNothing, READREG},					/* 0xA04 */
	{"color130rx", "", ParseNothing, READREG},					/* 0xA08 */
	{"color131rx", "", ParseNothing, READREG},					/* 0xA0C */
	{"color132rx", "", ParseNothing, READREG},					/* 0xA10 */
	{"color133rx", "", ParseNothing, READREG},					/* 0xA14 */
	{"color134rx", "", ParseNothing, READREG},					/* 0xA18 */
	{"color135rx", "", ParseNothing, READREG},					/* 0xA1C */
	{"color136rx", "", ParseNothing, READREG},					/* 0xA20 */
	{"color137rx", "", ParseNothing, READREG},					/* 0xA24 */
	{"color138rx", "", ParseNothing, READREG},					/* 0xA28 */
	{"color139rx", "", ParseNothing, READREG},					/* 0xA2C */
	{"color140rx", "", ParseNothing, READREG},					/* 0xA30 */
	{"color141rx", "", ParseNothing, READREG},					/* 0xA34 */
	{"color142rx", "", ParseNothing, READREG},					/* 0xA38 */
	{"color143rx", "", ParseNothing, READREG},					/* 0xA3C */
	{"color144rx", "", ParseNothing, READREG},					/* 0xA40 */
	{"color145rx", "", ParseNothing, READREG},					/* 0xA44 */
	{"color146rx", "", ParseNothing, READREG},					/* 0xA48 */
	{"color147rx", "", ParseNothing, READREG},					/* 0xA4C */
	{"color148rx", "", ParseNothing, READREG},					/* 0xA50 */
	{"color149rx", "", ParseNothing, READREG},					/* 0xA54 */
	{"color150rx", "", ParseNothing, READREG},					/* 0xA58 */
	{"color151rx", "", ParseNothing, READREG},					/* 0xA5C */
	{"color152rx", "", ParseNothing, READREG},					/* 0xA60 */
	{"color153rx", "", ParseNothing, READREG},					/* 0xA64 */
	{"color154rx", "", ParseNothing, READREG},					/* 0xA68 */
	{"color155rx", "", ParseNothing, READREG},					/* 0xA6C */
	{"color156rx", "", ParseNothing, READREG},					/* 0xA70 */
	{"color157rx", "", ParseNothing, READREG},					/* 0xA74 */
	{"color158rx", "", ParseNothing, READREG},					/* 0xA78 */
	{"color159rx", "", ParseNothing, READREG},					/* 0xA7C */
	{"color160rx", "", ParseNothing, READREG},					/* 0xA80 */
	{"color161rx", "", ParseNothing, READREG},					/* 0xA84 */
	{"color162rx", "", ParseNothing, READREG},					/* 0xA88 */
	{"color163rx", "", ParseNothing, READREG},					/* 0xA8C */
	{"color164rx", "", ParseNothing, READREG},					/* 0xA90 */
	{"color165rx", "", ParseNothing, READREG},					/* 0xA94 */
	{"color166rx", "", ParseNothing, READREG},					/* 0xA98 */
	{"color167rx", "", ParseNothing, READREG},					/* 0xA9C */
	{"color168rx", "", ParseNothing, READREG},					/* 0xAA0 */
	{"color169rx", "", ParseNothing, READREG},					/* 0xAA4 */
	{"color170rx", "", ParseNothing, READREG},					/* 0xAA8 */
	{"color171rx", "", ParseNothing, READREG},					/* 0xAAC */
	{"color172rx", "", ParseNothing, READREG},					/* 0xAB0 */
	{"color173rx", "", ParseNothing, READREG},					/* 0xAB4 */
	{"color174rx", "", ParseNothing, READREG},					/* 0xAB8 */
	{"color175rx", "", ParseNothing, READREG},					/* 0xABC */
	{"color176rx", "", ParseNothing, READREG},					/* 0xAC0 */
	{"color177rx", "", ParseNothing, READREG},					/* 0xAC4 */
	{"color178rx", "", ParseNothing, READREG},					/* 0xAC8 */
	{"color179rx", "", ParseNothing, READREG},					/* 0xACC */
	{"color180rx", "", ParseNothing, READREG},					/* 0xAD0 */
	{"color181rx", "", ParseNothing, READREG},					/* 0xAD4 */
	{"color182rx", "", ParseNothing, READREG},					/* 0xAD8 */
	{"color183rx", "", ParseNothing, READREG},					/* 0xADC */
	{"color184rx", "", ParseNothing, READREG},					/* 0xAE0 */
	{"color185rx", "", ParseNothing, READREG},					/* 0xAE4 */
	{"color186rx", "", ParseNothing, READREG},					/* 0xAE8 */
	{"color187rx", "", ParseNothing, READREG},					/* 0xAEC */
	{"color188rx", "", ParseNothing, READREG},					/* 0xAF0 */
	{"color189rx", "", ParseNothing, READREG},					/* 0xAF4 */
	{"color190rx", "", ParseNothing, READREG},					/* 0xAF8 */
	{"color191rx", "", ParseNothing, READREG},					/* 0xAFC */
	{"color192rx", "", ParseNothing, READREG},					/* 0xB00 */
	{"color193rx", "", ParseNothing, READREG},					/* 0xB04 */
	{"color194rx", "", ParseNothing, READREG},					/* 0xB08 */
	{"color195rx", "", ParseNothing, READREG},					/* 0xB0C */
	{"color196rx", "", ParseNothing, READREG},					/* 0xB10 */
	{"color197rx", "", ParseNothing, READREG},					/* 0xB14 */
	{"color198rx", "", ParseNothing, READREG},					/* 0xB18 */
	{"color199rx", "", ParseNothing, READREG},					/* 0xB1C */
	{"color290rx", "", ParseNothing, READREG},					/* 0xB20 */
	{"color201rx", "", ParseNothing, READREG},					/* 0xB24 */
	{"color202rx", "", ParseNothing, READREG},					/* 0xB28 */
	{"color203rx", "", ParseNothing, READREG},					/* 0xB2C */
	{"color204rx", "", ParseNothing, READREG},					/* 0xB30 */
	{"color205rx", "", ParseNothing, READREG},					/* 0xB34 */
	{"color206rx", "", ParseNothing, READREG},					/* 0xB38 */
	{"color207rx", "", ParseNothing, READREG},					/* 0xB3C */
	{"color208rx", "", ParseNothing, READREG},					/* 0xB40 */
	{"color209rx", "", ParseNothing, READREG},					/* 0xB44 */
	{"color210rx", "", ParseNothing, READREG},					/* 0xB48 */
	{"color211rx", "", ParseNothing, READREG},					/* 0xB4C */
	{"color212rx", "", ParseNothing, READREG},					/* 0xB50 */
	{"color213rx", "", ParseNothing, READREG},					/* 0xB54 */
	{"color214rx", "", ParseNothing, READREG},					/* 0xB58 */
	{"color215rx", "", ParseNothing, READREG},					/* 0xB5C */
	{"color216rx", "", ParseNothing, READREG},					/* 0xB60 */
	{"color217rx", "", ParseNothing, READREG},					/* 0xB64 */
	{"color218rx", "", ParseNothing, READREG},					/* 0xB68 */
	{"color219rx", "", ParseNothing, READREG},					/* 0xB6C */
	{"color220rx", "", ParseNothing, READREG},					/* 0xB70 */
	{"color221rx", "", ParseNothing, READREG},					/* 0xB74 */
	{"color222rx", "", ParseNothing, READREG},					/* 0xB78 */
	{"color223rx", "", ParseNothing, READREG},					/* 0xB7C */
	{"color224rx", "", ParseNothing, READREG},					/* 0xB80 */
	{"color225rx", "", ParseNothing, READREG},					/* 0xB84 */
	{"color226rx", "", ParseNothing, READREG},					/* 0xB88 */
	{"color227rx", "", ParseNothing, READREG},					/* 0xB8C */
	{"color228rx", "", ParseNothing, READREG},					/* 0xB90 */
	{"color229rx", "", ParseNothing, READREG},					/* 0xB94 */
	{"color230rx", "", ParseNothing, READREG},					/* 0xB98 */
	{"color231rx", "", ParseNothing, READREG},					/* 0xB9C */
	{"color232rx", "", ParseNothing, READREG},					/* 0xBA0 */
	{"color233rx", "", ParseNothing, READREG},					/* 0xBA4 */
	{"color234rx", "", ParseNothing, READREG},					/* 0xBA8 */
	{"color235rx", "", ParseNothing, READREG},					/* 0xBAC */
	{"color236rx", "", ParseNothing, READREG},					/* 0xBB0 */
	{"color237rx", "", ParseNothing, READREG},					/* 0xBB4 */
	{"color238rx", "", ParseNothing, READREG},					/* 0xBB8 */
	{"color239rx", "", ParseNothing, READREG},					/* 0xBBC */
	{"color240rx", "", ParseNothing, READREG},					/* 0xBC0 */
	{"color241rx", "", ParseNothing, READREG},					/* 0xBC4 */
	{"color242rx", "", ParseNothing, READREG},					/* 0xBC8 */
	{"color243rx", "", ParseNothing, READREG},					/* 0xBCC */
	{"color244rx", "", ParseNothing, READREG},					/* 0xBD0 */
	{"color245rx", "", ParseNothing, READREG},					/* 0xBD4 */
	{"color246rx", "", ParseNothing, READREG},					/* 0xBD8 */
	{"color247rx", "", ParseNothing, READREG},					/* 0xBDC */
	{"color248rx", "", ParseNothing, READREG},					/* 0xBE0 */
	{"color249rx", "", ParseNothing, READREG},					/* 0xBE4 */
	{"color250rx", "", ParseNothing, READREG},					/* 0xBE8 */
	{"color251rx", "", ParseNothing, READREG},					/* 0xBEC */
	{"color252rx", "", ParseNothing, READREG},					/* 0xBF0 */
	{"color253rx", "", ParseNothing, READREG},					/* 0xBF4 */
	{"color254rx", "", ParseNothing, READREG},					/* 0xBF8 */
	{"color255rx", "", ParseNothing, READREG},					/* 0xBFC */

	{"color000wx", "", AAAColor32, COLOR32},					/* 0xC00 */
	{"color001wx", "", AAAColor32, COLOR32},					/* 0xC04 */
	{"color002wx", "", AAAColor32, COLOR32},					/* 0xC08 */
	{"color003wx", "", AAAColor32, COLOR32},					/* 0xC0C */
	{"color004wx", "", AAAColor32, COLOR32},					/* 0xC10 */
	{"color005wx", "", AAAColor32, COLOR32},					/* 0xC14 */
	{"color006wx", "", AAAColor32, COLOR32},					/* 0xC18 */
	{"color007wx", "", AAAColor32, COLOR32},					/* 0xC1C */
	{"color008wx", "", AAAColor32, COLOR32},					/* 0xC20 */
	{"color009wx", "", AAAColor32, COLOR32},					/* 0xC24 */
	{"color010wx", "", AAAColor32, COLOR32},					/* 0xC28 */
	{"color011wx", "", AAAColor32, COLOR32},					/* 0xC2C */
	{"color012wx", "", AAAColor32, COLOR32},					/* 0xC30 */
	{"color013wx", "", AAAColor32, COLOR32},					/* 0xC34 */
	{"color014wx", "", AAAColor32, COLOR32},					/* 0xC38 */
	{"color015wx", "", AAAColor32, COLOR32},					/* 0xC3C */
	{"color016wx", "", AAAColor32, COLOR32},					/* 0xC40 */
	{"color017wx", "", AAAColor32, COLOR32},					/* 0xC44 */
	{"color018wx", "", AAAColor32, COLOR32},					/* 0xC48 */
	{"color019wx", "", AAAColor32, COLOR32},					/* 0xC4C */
	{"color020wx", "", AAAColor32, COLOR32},					/* 0xC50 */
	{"color021wx", "", AAAColor32, COLOR32},					/* 0xC54 */
	{"color022wx", "", AAAColor32, COLOR32},					/* 0xC58 */
	{"color023wx", "", AAAColor32, COLOR32},					/* 0xC5C */
	{"color024wx", "", AAAColor32, COLOR32},					/* 0xC60 */
	{"color025wx", "", AAAColor32, COLOR32},					/* 0xC64 */
	{"color026wx", "", AAAColor32, COLOR32},					/* 0xC68 */
	{"color027wx", "", AAAColor32, COLOR32},					/* 0xC6C */
	{"color028wx", "", AAAColor32, COLOR32},					/* 0xC70 */
	{"color029wx", "", AAAColor32, COLOR32},					/* 0xC74 */
	{"color020wx", "", AAAColor32, COLOR32},					/* 0xC78 */
	{"color031wx", "", AAAColor32, COLOR32},					/* 0xC7C */
	{"color032wx", "", AAAColor32, COLOR32},					/* 0xC80 */
	{"color033wx", "", AAAColor32, COLOR32},					/* 0xC84 */
	{"color034wx", "", AAAColor32, COLOR32},					/* 0xC88 */
	{"color035wx", "", AAAColor32, COLOR32},					/* 0xC8C */
	{"color036wx", "", AAAColor32, COLOR32},					/* 0xC90 */
	{"color037wx", "", AAAColor32, COLOR32},					/* 0xC94 */
	{"color038wx", "", AAAColor32, COLOR32},					/* 0xC98 */
	{"color039wx", "", AAAColor32, COLOR32},					/* 0xC9C */
	{"color040wx", "", AAAColor32, COLOR32},					/* 0xCA0 */
	{"color041wx", "", AAAColor32, COLOR32},					/* 0xCA4 */
	{"color042wx", "", AAAColor32, COLOR32},					/* 0xCA8 */
	{"color043wx", "", AAAColor32, COLOR32},					/* 0xCAC */
	{"color044wx", "", AAAColor32, COLOR32},					/* 0xCB0 */
	{"color045wx", "", AAAColor32, COLOR32},					/* 0xCB4 */
	{"color046wx", "", AAAColor32, COLOR32},					/* 0xCB8 */
	{"color047wx", "", AAAColor32, COLOR32},					/* 0xCBC */
	{"color048wx", "", AAAColor32, COLOR32},					/* 0xCC0 */
	{"color049wx", "", AAAColor32, COLOR32},					/* 0xCC4 */
	{"color050wx", "", AAAColor32, COLOR32},					/* 0xCC8 */
	{"color051wx", "", AAAColor32, COLOR32},					/* 0xCCC */
	{"color052wx", "", AAAColor32, COLOR32},					/* 0xCD0 */
	{"color053wx", "", AAAColor32, COLOR32},					/* 0xCD4 */
	{"color054wx", "", AAAColor32, COLOR32},					/* 0xCD8 */
	{"color055wx", "", AAAColor32, COLOR32},					/* 0xCDC */
	{"color056wx", "", AAAColor32, COLOR32},					/* 0xCE0 */
	{"color057wx", "", AAAColor32, COLOR32},					/* 0xCE4 */
	{"color058wx", "", AAAColor32, COLOR32},					/* 0xCE8 */
	{"color059wx", "", AAAColor32, COLOR32},					/* 0xCEC */
	{"color060wx", "", AAAColor32, COLOR32},					/* 0xCF0 */
	{"color061wx", "", AAAColor32, COLOR32},					/* 0xCF4 */
	{"color062wx", "", AAAColor32, COLOR32},					/* 0xCF8 */
	{"color063wx", "", AAAColor32, COLOR32},					/* 0xCFC */
	{"color064wx", "", AAAColor32, COLOR32},					/* 0xD00 */
	{"color065wx", "", AAAColor32, COLOR32},					/* 0xD04 */
	{"color066wx", "", AAAColor32, COLOR32},					/* 0xD08 */
	{"color067wx", "", AAAColor32, COLOR32},					/* 0xD0C */
	{"color068wx", "", AAAColor32, COLOR32},					/* 0xD10 */
	{"color069wx", "", AAAColor32, COLOR32},					/* 0xD14 */
	{"color060wx", "", AAAColor32, COLOR32},					/* 0xD18 */
	{"color071wx", "", AAAColor32, COLOR32},					/* 0xD1C */
	{"color072wx", "", AAAColor32, COLOR32},					/* 0xD20 */
	{"color073wx", "", AAAColor32, COLOR32},					/* 0xD24 */
	{"color074wx", "", AAAColor32, COLOR32},					/* 0xD28 */
	{"color075wx", "", AAAColor32, COLOR32},					/* 0xD2C */
	{"color076wx", "", AAAColor32, COLOR32},					/* 0xD30 */
	{"color077wx", "", AAAColor32, COLOR32},					/* 0xD34 */
	{"color078wx", "", AAAColor32, COLOR32},					/* 0xD38 */
	{"color079wx", "", AAAColor32, COLOR32},					/* 0xD3C */
	{"color080wx", "", AAAColor32, COLOR32},					/* 0xD40 */
	{"color081wx", "", AAAColor32, COLOR32},					/* 0xD44 */
	{"color082wx", "", AAAColor32, COLOR32},					/* 0xD48 */
	{"color083wx", "", AAAColor32, COLOR32},					/* 0xD4C */
	{"color084wx", "", AAAColor32, COLOR32},					/* 0xD50 */
	{"color085wx", "", AAAColor32, COLOR32},					/* 0xD54 */
	{"color086wx", "", AAAColor32, COLOR32},					/* 0xD58 */
	{"color087wx", "", AAAColor32, COLOR32},					/* 0xD5C */
	{"color088wx", "", AAAColor32, COLOR32},					/* 0xD60 */
	{"color089wx", "", AAAColor32, COLOR32},					/* 0xD64 */
	{"color090wx", "", AAAColor32, COLOR32},					/* 0xD68 */
	{"color091wx", "", AAAColor32, COLOR32},					/* 0xD6C */
	{"color092wx", "", AAAColor32, COLOR32},					/* 0xD70 */
	{"color093wx", "", AAAColor32, COLOR32},					/* 0xD74 */
	{"color094wx", "", AAAColor32, COLOR32},					/* 0xD78 */
	{"color095wx", "", AAAColor32, COLOR32},					/* 0xD7C */
	{"color096wx", "", AAAColor32, COLOR32},					/* 0xD80 */
	{"color097wx", "", AAAColor32, COLOR32},					/* 0xD84 */
	{"color098wx", "", AAAColor32, COLOR32},					/* 0xD88 */
	{"color099wx", "", AAAColor32, COLOR32},					/* 0xD8C */
	{"color100wx", "", AAAColor32, COLOR32},					/* 0xD90 */
	{"color101wx", "", AAAColor32, COLOR32},					/* 0xD94 */
	{"color102wx", "", AAAColor32, COLOR32},					/* 0xD98 */
	{"color103wx", "", AAAColor32, COLOR32},					/* 0xD9C */
	{"color104wx", "", AAAColor32, COLOR32},					/* 0xDA0 */
	{"color105wx", "", AAAColor32, COLOR32},					/* 0xDA4 */
	{"color106wx", "", AAAColor32, COLOR32},					/* 0xDA8 */
	{"color107wx", "", AAAColor32, COLOR32},					/* 0xDAC */
	{"color108wx", "", AAAColor32, COLOR32},					/* 0xDB0 */
	{"color109wx", "", AAAColor32, COLOR32},					/* 0xDB4 */
	{"color110wx", "", AAAColor32, COLOR32},					/* 0xDB8 */
	{"color111wx", "", AAAColor32, COLOR32},					/* 0xDBC */
	{"color112wx", "", AAAColor32, COLOR32},					/* 0xDC0 */
	{"color113wx", "", AAAColor32, COLOR32},					/* 0xDC4 */
	{"color114wx", "", AAAColor32, COLOR32},					/* 0xDC8 */
	{"color115wx", "", AAAColor32, COLOR32},					/* 0xDCC */
	{"color116wx", "", AAAColor32, COLOR32},					/* 0xDD0 */
	{"color117wx", "", AAAColor32, COLOR32},					/* 0xDD4 */
	{"color118wx", "", AAAColor32, COLOR32},					/* 0xDD8 */
	{"color119wx", "", AAAColor32, COLOR32},					/* 0xDDC */
	{"color110wx", "", AAAColor32, COLOR32},					/* 0xDE0 */
	{"color121wx", "", AAAColor32, COLOR32},					/* 0xDE4 */
	{"color122wx", "", AAAColor32, COLOR32},					/* 0xDE8 */
	{"color123wx", "", AAAColor32, COLOR32},					/* 0xDEC */
	{"color124wx", "", AAAColor32, COLOR32},					/* 0xDF0 */
	{"color125wx", "", AAAColor32, COLOR32},					/* 0xDF4 */
	{"color126wx", "", AAAColor32, COLOR32},					/* 0xDF8 */
	{"color127wx", "", AAAColor32, COLOR32},					/* 0xDFC */
	{"color128wx", "", AAAColor32, COLOR32},					/* 0xE00 */
	{"color129wx", "", AAAColor32, COLOR32},					/* 0xE04 */
	{"color130wx", "", AAAColor32, COLOR32},					/* 0xE08 */
	{"color131wx", "", AAAColor32, COLOR32},					/* 0xE0C */
	{"color132wx", "", AAAColor32, COLOR32},					/* 0xE10 */
	{"color133wx", "", AAAColor32, COLOR32},					/* 0xE14 */
	{"color134wx", "", AAAColor32, COLOR32},					/* 0xE18 */
	{"color135wx", "", AAAColor32, COLOR32},					/* 0xE1C */
	{"color136wx", "", AAAColor32, COLOR32},					/* 0xE20 */
	{"color137wx", "", AAAColor32, COLOR32},					/* 0xE24 */
	{"color138wx", "", AAAColor32, COLOR32},					/* 0xE28 */
	{"color139wx", "", AAAColor32, COLOR32},					/* 0xE2C */
	{"color140wx", "", AAAColor32, COLOR32},					/* 0xE30 */
	{"color141wx", "", AAAColor32, COLOR32},					/* 0xE34 */
	{"color142wx", "", AAAColor32, COLOR32},					/* 0xE38 */
	{"color143wx", "", AAAColor32, COLOR32},					/* 0xE3C */
	{"color144wx", "", AAAColor32, COLOR32},					/* 0xE40 */
	{"color145wx", "", AAAColor32, COLOR32},					/* 0xE44 */
	{"color146wx", "", AAAColor32, COLOR32},					/* 0xE48 */
	{"color147wx", "", AAAColor32, COLOR32},					/* 0xE4C */
	{"color148wx", "", AAAColor32, COLOR32},					/* 0xE50 */
	{"color149wx", "", AAAColor32, COLOR32},					/* 0xE54 */
	{"color150wx", "", AAAColor32, COLOR32},					/* 0xE58 */
	{"color151wx", "", AAAColor32, COLOR32},					/* 0xE5C */
	{"color152wx", "", AAAColor32, COLOR32},					/* 0xE60 */
	{"color153wx", "", AAAColor32, COLOR32},					/* 0xE64 */
	{"color154wx", "", AAAColor32, COLOR32},					/* 0xE68 */
	{"color155wx", "", AAAColor32, COLOR32},					/* 0xE6C */
	{"color156wx", "", AAAColor32, COLOR32},					/* 0xE70 */
	{"color157wx", "", AAAColor32, COLOR32},					/* 0xE74 */
	{"color158wx", "", AAAColor32, COLOR32},					/* 0xE78 */
	{"color159wx", "", AAAColor32, COLOR32},					/* 0xE7C */
	{"color160wx", "", AAAColor32, COLOR32},					/* 0xE80 */
	{"color161wx", "", AAAColor32, COLOR32},					/* 0xE84 */
	{"color162wx", "", AAAColor32, COLOR32},					/* 0xE88 */
	{"color163wx", "", AAAColor32, COLOR32},					/* 0xE8C */
	{"color164wx", "", AAAColor32, COLOR32},					/* 0xE90 */
	{"color165wx", "", AAAColor32, COLOR32},					/* 0xE94 */
	{"color166wx", "", AAAColor32, COLOR32},					/* 0xE98 */
	{"color167wx", "", AAAColor32, COLOR32},					/* 0xE9C */
	{"color168wx", "", AAAColor32, COLOR32},					/* 0xEA0 */
	{"color169wx", "", AAAColor32, COLOR32},					/* 0xEA4 */
	{"color170wx", "", AAAColor32, COLOR32},					/* 0xEA8 */
	{"color171wx", "", AAAColor32, COLOR32},					/* 0xEAC */
	{"color172wx", "", AAAColor32, COLOR32},					/* 0xEB0 */
	{"color173wx", "", AAAColor32, COLOR32},					/* 0xEB4 */
	{"color174wx", "", AAAColor32, COLOR32},					/* 0xEB8 */
	{"color175wx", "", AAAColor32, COLOR32},					/* 0xEBC */
	{"color176wx", "", AAAColor32, COLOR32},					/* 0xEC0 */
	{"color177wx", "", AAAColor32, COLOR32},					/* 0xEC4 */
	{"color178wx", "", AAAColor32, COLOR32},					/* 0xEC8 */
	{"color179wx", "", AAAColor32, COLOR32},					/* 0xECC */
	{"color180wx", "", AAAColor32, COLOR32},					/* 0xED0 */
	{"color181wx", "", AAAColor32, COLOR32},					/* 0xED4 */
	{"color182wx", "", AAAColor32, COLOR32},					/* 0xED8 */
	{"color183wx", "", AAAColor32, COLOR32},					/* 0xEDC */
	{"color184wx", "", AAAColor32, COLOR32},					/* 0xEE0 */
	{"color185wx", "", AAAColor32, COLOR32},					/* 0xEE4 */
	{"color186wx", "", AAAColor32, COLOR32},					/* 0xEE8 */
	{"color187wx", "", AAAColor32, COLOR32},					/* 0xEEC */
	{"color188wx", "", AAAColor32, COLOR32},					/* 0xEF0 */
	{"color189wx", "", AAAColor32, COLOR32},					/* 0xEF4 */
	{"color190wx", "", AAAColor32, COLOR32},					/* 0xEF8 */
	{"color191wx", "", AAAColor32, COLOR32},					/* 0xEFC */
	{"color192wx", "", AAAColor32, COLOR32},					/* 0xF00 */
	{"color193wx", "", AAAColor32, COLOR32},					/* 0xF04 */
	{"color194wx", "", AAAColor32, COLOR32},					/* 0xF08 */
	{"color195wx", "", AAAColor32, COLOR32},					/* 0xF0C */
	{"color196wx", "", AAAColor32, COLOR32},					/* 0xF10 */
	{"color197wx", "", AAAColor32, COLOR32},					/* 0xF14 */
	{"color198wx", "", AAAColor32, COLOR32},					/* 0xF18 */
	{"color199wx", "", AAAColor32, COLOR32},					/* 0xF1C */
	{"color290wx", "", AAAColor32, COLOR32},					/* 0xF20 */
	{"color201wx", "", AAAColor32, COLOR32},					/* 0xF24 */
	{"color202wx", "", AAAColor32, COLOR32},					/* 0xF28 */
	{"color203wx", "", AAAColor32, COLOR32},					/* 0xF2C */
	{"color204wx", "", AAAColor32, COLOR32},					/* 0xF30 */
	{"color205wx", "", AAAColor32, COLOR32},					/* 0xF34 */
	{"color206wx", "", AAAColor32, COLOR32},					/* 0xF38 */
	{"color207wx", "", AAAColor32, COLOR32},					/* 0xF3C */
	{"color208wx", "", AAAColor32, COLOR32},					/* 0xF40 */
	{"color209wx", "", AAAColor32, COLOR32},					/* 0xF44 */
	{"color210wx", "", AAAColor32, COLOR32},					/* 0xF48 */
	{"color211wx", "", AAAColor32, COLOR32},					/* 0xF4C */
	{"color212wx", "", AAAColor32, COLOR32},					/* 0xF50 */
	{"color213wx", "", AAAColor32, COLOR32},					/* 0xF54 */
	{"color214wx", "", AAAColor32, COLOR32},					/* 0xF58 */
	{"color215wx", "", AAAColor32, COLOR32},					/* 0xF5C */
	{"color216wx", "", AAAColor32, COLOR32},					/* 0xF60 */
	{"color217wx", "", AAAColor32, COLOR32},					/* 0xF64 */
	{"color218wx", "", AAAColor32, COLOR32},					/* 0xF68 */
	{"color219wx", "", AAAColor32, COLOR32},					/* 0xF6C */
	{"color220wx", "", AAAColor32, COLOR32},					/* 0xF70 */
	{"color221wx", "", AAAColor32, COLOR32},					/* 0xF74 */
	{"color222wx", "", AAAColor32, COLOR32},					/* 0xF78 */
	{"color223wx", "", AAAColor32, COLOR32},					/* 0xF7C */
	{"color224wx", "", AAAColor32, COLOR32},					/* 0xF80 */
	{"color225wx", "", AAAColor32, COLOR32},					/* 0xF84 */
	{"color226wx", "", AAAColor32, COLOR32},					/* 0xF88 */
	{"color227wx", "", AAAColor32, COLOR32},					/* 0xF8C */
	{"color228wx", "", AAAColor32, COLOR32},					/* 0xF90 */
	{"color229wx", "", AAAColor32, COLOR32},					/* 0xF94 */
	{"color230wx", "", AAAColor32, COLOR32},					/* 0xF98 */
	{"color231wx", "", AAAColor32, COLOR32},					/* 0xF9C */
	{"color232wx", "", AAAColor32, COLOR32},					/* 0xFA0 */
	{"color233wx", "", AAAColor32, COLOR32},					/* 0xFA4 */
	{"color234wx", "", AAAColor32, COLOR32},					/* 0xFA8 */
	{"color235wx", "", AAAColor32, COLOR32},					/* 0xFAC */
	{"color236wx", "", AAAColor32, COLOR32},					/* 0xFB0 */
	{"color237wx", "", AAAColor32, COLOR32},					/* 0xFB4 */
	{"color238wx", "", AAAColor32, COLOR32},					/* 0xFB8 */
	{"color239wx", "", AAAColor32, COLOR32},					/* 0xFBC */
	{"color240wx", "", AAAColor32, COLOR32},					/* 0xFC0 */
	{"color241wx", "", AAAColor32, COLOR32},					/* 0xFC4 */
	{"color242wx", "", AAAColor32, COLOR32},					/* 0xFC8 */
	{"color243wx", "", AAAColor32, COLOR32},					/* 0xFCC */
	{"color244wx", "", AAAColor32, COLOR32},					/* 0xFD0 */
	{"color245wx", "", AAAColor32, COLOR32},					/* 0xFD4 */
	{"color246wx", "", AAAColor32, COLOR32},					/* 0xFD8 */
	{"color247wx", "", AAAColor32, COLOR32},					/* 0xFDC */
	{"color248wx", "", AAAColor32, COLOR32},					/* 0xFE0 */
	{"color249wx", "", AAAColor32, COLOR32},					/* 0xFE4 */
	{"color250wx", "", AAAColor32, COLOR32},					/* 0xFE8 */
	{"color251wx", "", AAAColor32, COLOR32},					/* 0xFEC */
	{"color252wx", "", AAAColor32, COLOR32},					/* 0xFF0 */
	{"color253wx", "", AAAColor32, COLOR32},					/* 0xFF4 */
	{"color254wx", "", AAAColor32, COLOR32},					/* 0xFF8 */
	{"color255wx", "", AAAColor32, COLOR32},					/* 0xFFC */

	{"UNKNOWN_OPCODE", "",ParseNothing, 0}						/* 0xFFF */


#define MAX_CHIP_REGISTER 0xFFE

#define REGISTER_MASK 0xFFF
