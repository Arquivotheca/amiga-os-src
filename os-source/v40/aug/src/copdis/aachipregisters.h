/*****************************************************************************
*
*	$Id: AAChipRegisters.h,v 39.2 92/11/20 17:31:50 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	AAChipRegisters.h,v $
 * Revision 39.2  92/11/20  17:31:50  Jim2
 * Corrected starting comment
 * 
*
******************************************************************************/
#ifndef REVSION
    #include "copdis_rev.h"

#endif
	{"bltddat","blitter destination early read (dummy address)",ParseNothing, 0},	/* 0x000 */
	{"dmaconr","DMA control (and blitter status) read",ParseNothing, 0},		/* 0x002 */
	{"vposr","read vert most signif. bit (and frame flop)",ParseNothing, 0},	/* 0x004 */
	{"vhposr","read vert and horiz. position of beam",ParseNothing, 0},		/* 0x006 */
	{"dskdatr","disk data early read (dummy address)",ParseNothing, 0},		/* 0x008 */
	{"joy0dat","joystick-mouse 0 data (vert,horiz)",ParseNothing, 0},		/* 0x00a */
	{"joy1dat","joystick-mouse 1 data (vert,horiz)",ParseNothing, 0},		/* 0x00c */
	{"clxdat","collision data (read and clear)",ParseNothing, 0},			/* 0x00e */
	{"adkconr","audio, disk control register read",ParseNothing, 0},		/* 0x010 */
	{"pot0dat","pot counter pair 0 data (vert,horiz)",ParseNothing, 0},		/* 0x012 */
	{"pot1dat","pot counter pair 1 data (vert,horiz)",ParseNothing, 0},		/* 0x014 */
	{"potinp","pot port data read",ParseNothing, 0},				/* 0x016 */
	{"serdatr","serial port data and status read",ParseNothing, 0},			/* 0x018 */
	{"dskbytr","disk data byte and status read",ParseNothing, 0},			/* 0x01a */
	{"intenar","interrupt enable bits read",ParseNothing, 0},			/* 0x01c */
	{"intreqr","interrupt request bits read",ParseNothing, 0},			/* 0x01e */

	{"dskpt","disk pointer (high 3 bits)",ParseNothing, ADDRESS},			/* 0x020 */
	{"dskpt","disk pointer (low 15 bits)",ParseNothing, ADDRESS},			/* 0x022 */
	{"dsklen","disk length",ParseNothing, 0},					/* 0x024 */
	{"dskdat","disk DMA data write",ParseNothing, 0},				/* 0x026 */

	{"refptr","refresh pointer",ParseNothing, 0},					/* 0x028 */
	{"vposw","write vert most signif. bit (and frame flop)",ParseNothing, 0},	/* 0x02a */
	{"vhposw","write vert and horiz position of beam",ParseNothing, 0},		/* 0x02c */
	{"copcon","copper control register (CDANG)",ParseNothing, 0},			/* 0x02e */
	{"serdat","serial port data and stop bits write",ParseNothing, 0},		/* 0x030 */
	{"serper","serial port period and control",ParseNothing, 0},			/* 0x032 */
	{"potgo","pot port data write and start",ParseNothing, 0},			/* 0x034 */
	{"joytest","write all four joystick-mouse counters",ParseNothing, 0},		/* 0x036 */
	{"strequ","strobe for horiz sync with vertical blank and EQU",ParseNothing, 0},	/* 0x038 */
	{"strvbl","strobe for horiz sync with vertical blank",ParseNothing, 0},		/* 0x03a */
	{"strhor","strobe for horizontal sync",ParseNothing, 0},			/* 0x03c */
	{"strlong","strobe for identification of long horiz. line",ParseNothing, 0},	/* 0x03e */

	{"bltcon0","blitter control register 0",ParseNothing, 0},			/* 0x040 */
	{"bltcon1","blitter control register 1",ParseNothing, 0},			/* 0x042 */
	{"bltafwm","blitter first word mask for source A",ParseNothing, 0},		/* 0x044 */
	{"bltalwm","blitter last word mask for source A",ParseNothing, 0},		/* 0x046 */
	{"bltcpt","blitter pointer to source C (high 3 bits)",ParseNothing, ADDRESS},	/* 0x048 */
	{"bltcpt","blitter pointer to source C (low 15 bits)",ParseNothing, ADDRESS},	/* 0x04a */
	{"bltbpt","blitter pointer to source B (high 3 bits)",ParseNothing, ADDRESS},	/* 0x04c */
	{"bltbpt","blitter pointer to source B (low 15 bits)",ParseNothing, ADDRESS},	/* 0x04e */
	{"bltapt","blitter pointer to source A (high 3 bits)",ParseNothing, ADDRESS},	/* 0x050 */
	{"bltapt","blitter pointer to source A (low 15 bits)",ParseNothing, ADDRESS},	/* 0x052 */
	{"bltdpt","blitter pointer to source D (high 3 bits)",ParseNothing, ADDRESS},	/* 0x054 */
	{"bltdpt","blitter pointer to source D (low 15 bits)",ParseNothing, ADDRESS},	/* 0x056 */
	{"bltsize","blitter start and size (window width,height)",ParseNothing, 0},	/* 0x058 */
	{"bltcon0l","blitter control 0 lower 8 bits (minterms)",ParseNothing, 0},	/* 0x05a */
	{"bltsizv","blitter V size",ParseNothing, 0},					/* 0x05c */
	{"bltsizh","blitter H size",ParseNothing, 0},					/* 0x05e */
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
	{"sprhdat","UHIRES stuff",ParseNothing, 0},					/* 0x078 */
	{"bplhdat","UHIRES stuff",ParseNothing, 0},					/* 0x07a */
	{"lisaid","Denise/Lisa chip rev",ParseNothing, 0},				/* 0x07c */
	{"dsksync","disk sync pattern for disk read",ParseNothing, 0},			/* 0x07e */

	{"cop1lc","copper first location (high 3 bits)",ParseNothing, ADDRESS},		/* 0x080 */
	{"cop1lc","copper first location (low 15 bits)",ParseNothing, ADDRESS},		/* 0x082 */
	{"cop2lc","copper second location (high 3 bits)",ParseNothing, ADDRESS},	/* 0x084 */
	{"cop2lc","copper second location (low 15 bits)",ParseNothing, ADDRESS},	/* 0x086 */
	{"copjmp1","restart copper at first location",DataStrobe, 0},			/* 0x088 */
	{"copjmp2","restart copper at second location",DataStrobe, 0},			/* 0x08a */
	{"copins","copper instruction fetch identify",ParseNothing, 0},			/* 0x08c */
	{"diwstrt","display window start (upper left vert-horiz position)",AADisplayStart, 0},	/* 0x08e */
	{"diwstop","display window stop (lower right vert-horiz position)",AADisplayStop, 0},	/* 0x090 */
	{"ddfstrt","display bit plane data fetch start (horiz. position)",AADataFetch, 0},	/* 0x092 */
	{"ddfstop","display bit plane data fetch stop (horiz. position)",AADataFetch, 0},	/* 0x094 */
	{"dmacon","DMA control write (clear or set)",ParseNothing, 0},			/* 0x096 */
	{"clxcon","collision control",ParseNothing, 0},					/* 0x098 */
	{"intena","interrupt enable bits",ParseNothing, 0},				/* 0x09a */
	{"intreq","interrupt request bits",ParseNothing, 0},				/* 0x09c */
	{"adkcon","audio, UART, disk control",ParseNothing, 0},				/* 0x09e */

	{"aud0lc","audio channel 0 location (high 3 bits)",ParseNothing, ADDRESS},	/* 0x0a0 */
	{"aud0lc","audio channel 0 location (low 15 bits)",ParseNothing, ADDRESS},	/* 0x0a2 */
	{"aud0len","audio channel 0 length",ParseNothing, 0},				/* 0x0a4 */
	{"aud0per","audio channel 0 period",ParseNothing, 0},				/* 0x0a6 */
	{"aud0vol","audio channel 0 volume",ParseNothing, 0},				/* 0x0a8 */
	{"aud0dat","audio channel 0 data",ParseNothing, 0},				/* 0x0aa */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0ac */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0ae */

	{"aud1lc","audio channel 1 location (high 3 bits)",ParseNothing, ADDRESS},	/* 0x0b0 */
	{"aud1lc","audio channel 1 location (low 15 bits)",ParseNothing, ADDRESS},	/* 0x0b2 */
	{"aud1len","audio channel 1 length",ParseNothing, 0},				/* 0x0b4 */
	{"aud1per","audio channel 1 period",ParseNothing, 0},				/* 0x0b6 */
	{"aud1vol","audio channel 1 volume",ParseNothing, 0},				/* 0x0b8 */
	{"aud1dat","audio channel 1 data",ParseNothing, 0},				/* 0x0ba */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0bc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0be */

	{"aud2lc","audio channel 2 location (high 3 bits)",ParseNothing, ADDRESS},	/* 0x0c0 */
	{"aud2lc","audio channel 2 location (low 15 bits)",ParseNothing, ADDRESS},	/* 0x0c2 */
	{"aud2len","audio channel 2 length",ParseNothing, 0},				/* 0x0c4 */
	{"aud2per","audio channel 2 period",ParseNothing, 0},				/* 0x0c6 */
	{"aud2vol","audio channel 2 volume",ParseNothing, 0},				/* 0x0c8 */
	{"aud2dat","audio channel 2 data",ParseNothing, 0},				/* 0x0ca */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0cc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0ce */

	{"aud3lc","audio channel 3 location (high 3 bits)",ParseNothing, ADDRESS},	/* 0x0d0 */
	{"aud3lc","audio channel 3 location (low 15 bits)",ParseNothing, ADDRESS},	/* 0x0d2 */
	{"aud3len","audio channel 3 length",ParseNothing, 0},				/* 0x0d4 */
	{"aud3per","audio channel 3 period",ParseNothing, 0},				/* 0x0d6 */
	{"aud3vol","audio channel 3 volume",ParseNothing, 0},				/* 0x0d8 */
	{"aud3dat","audio channel 3 data",ParseNothing, 0},				/* 0x0da */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0dc */
	{"unused","ERROR",ParseNothing, 0},						/* 0x0de */

	{"bpl1pt","blitter plane 1 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e0 */
	{"bpl1pt","blitter plane 1 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e2 */
	{"bpl2pt","blitter plane 2 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e4 */
	{"bpl2pt","blitter plane 2 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e6 */
	{"bpl3pt","blitter plane 3 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0e8 */
	{"bpl3pt","blitter plane 3 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0ea */
	{"bpl4pt","blitter plane 4 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0ec */
	{"bpl4pt","blitter plane 4 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0ee */
	{"bpl5pt","blitter plane 5 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f0 */
	{"bpl5pt","blitter plane 5 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f2 */
	{"bpl6pt","blitter plane 6 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f4 */
	{"bpl6pt","blitter plane 6 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f6 */
	{"bpl7pt","blitter plane 7 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0f8 */
	{"bpl7pt","blitter plane 7 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0fa */
	{"bpl8pt","blitter plane 8 pointer (high 3 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0fc */
	{"bpl8pt","blitter plane 8 pointer (low 15 bits)",ParseNothing, ADD_BITPLANE},	/* 0x0fe */


	{"bplcon0","bit plane control bits",AABPMiscControl, 0},			/* 0x100 */
	{"bplcon1","bit plane scroll values for PF1 & PF2",AAHScroll, 0},		/* 0x102 */
	{"bplcon2","bit plane priority control",AABPPriority, 0},			/* 0x104 */
	{"bplcon3","genloc, colour balketc.",AABPEnhance, 0},			       	/* 0x106 */

	{"bpl1mod","bit plane modulo for odd planes",AABPModulo, 0},			/* 0x108 */
	{"bpl2mod","bit plane modulo for even planes",AABPModulo, 0},			/* 0x10a */

	{"bplcon4","bitplane masks, sprintmasks",ParseNothing, 0},			/* 0x10c */
	{"clxcon2","upper bits of sprite collision",ParseNothing, 0},			/* 0x10e */

	{"bpl1dat","bit plane 1 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x110 */
	{"bpl2dat","bit plane 2 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x112 */
	{"bpl3dat","bit plane 3 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x114 */
	{"bpl4dat","bit plane 4 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x116 */
	{"bpl5dat","bit plane 5 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x118 */
	{"bpl6dat","bit plane 6 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x11a */
	{"bpl7dat","bit plane 7 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x11c */
	{"bpl8dat","bit plane 8 data (parallel-to-serial convert)",ParseNothing, 0},	/* 0x11e */

	{"spr0pt","sprite 0 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x120 */
	{"spr0pt","sprite 0 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x122 */
	{"spr1pt","sprite 1 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x124 */
	{"spr1pt","sprite 1 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x126 */
	{"spr2pt","sprite 2 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x128 */
	{"spr2pt","sprite 2 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x12a */
	{"spr3pt","sprite 3 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x12c */
	{"spr3pt","sprite 3 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x12e */
	{"spr4pt","sprite 4 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x130 */
	{"spr4pt","sprite 4 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x132 */
	{"spr5pt","sprite 5 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x134 */
	{"spr5pt","sprite 5 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x136 */
	{"spr6pt","sprite 6 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x138 */
	{"spr6pt","sprite 6 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x13a */
	{"spr7pt","sprite 7 pointer (high 3 bits)",ParseNothing, ADDRESS},		/* 0x13c */
	{"spr7pt","sprite 7 pointer (low 15 bits)",ParseNothing, ADDRESS},		/* 0x13e */

	{"spr0pos","sprite 0 vert-horiz start position",AASpriteStart, 0},		/* 0x140 */
	{"spr0ctl","sprite 0 vert stop position and control data",ParseNothing, 0},	/* 0x142 */
	{"spr0data","sprite 0 image data register A",ParseNothing, 0},			/* 0x144 */
	{"spr0datb","sprite 0 image data register B",ParseNothing, 0},			/* 0x146 */
	{"spr1pos","sprite 1 vert-horiz start position",AASpriteStart, 0},		/* 0x148 */
	{"spr1ctl","sprite 1 vert stop position and control data",ParseNothing, 0},	/* 0x14a */
	{"spr1data","sprite 1 image data register A",ParseNothing, 0},			/* 0x14c */
	{"spr1datb","sprite 1 image data register B",ParseNothing, 0},			/* 0x14e */
	{"spr2pos","sprite 2 vert-horiz start position",AASpriteStart, 0},		/* 0x150 */
	{"spr2ctl","sprite 2 vert stop position and control data",ParseNothing, 0},	/* 0x152 */
	{"spr2data","sprite 2 image data register A",ParseNothing, 0},			/* 0x154 */
	{"spr2datb","sprite 2 image data register B",ParseNothing, 0},			/* 0x156 */
	{"spr3pos","sprite 3 vert-horiz start position",AASpriteStart, 0},		/* 0x158 */
	{"spr3ctl","sprite 3 vert stop position and control data",ParseNothing, 0},	/* 0x15a */
	{"spr3data","sprite 3 image data register A",ParseNothing, 0},			/* 0x15c */
	{"spr3datb","sprite 3 image data register B",ParseNothing, 0},			/* 0x15e */
	{"spr4pos","sprite 4 vert-horiz start position",AASpriteStart, 0},		/* 0x160 */
	{"spr4ctl","sprite 4 vert stop position and control data",ParseNothing, 0},	/* 0x162 */
	{"spr4data","sprite 4 image data register A",ParseNothing, 0},			/* 0x164 */
	{"spr4datb","sprite 4 image data register B",ParseNothing, 0},			/* 0x166 */
	{"spr5pos","sprite 5 vert-horiz start position",AASpriteStart, 0},		/* 0x168 */
	{"spr5ctl","sprite 5 vert stop position and control data",ParseNothing, 0},	/* 0x16a */
	{"spr5data","sprite 5 image data register A",ParseNothing, 0},			/* 0x16c */
	{"spr5datb","sprite 5 image data register B",ParseNothing, 0},			/* 0x16e */
	{"spr6pos","sprite 6 vert-horiz start position",AASpriteStart, 0},		/* 0x170 */
	{"spr6ctl","sprite 6 vert stop position and control data",ParseNothing, 0},	/* 0x172 */
	{"spr6data","sprite 6 image data register A",ParseNothing, 0},			/* 0x174 */
	{"spr6datb","sprite 6 image data register B",ParseNothing, 0},			/* 0x176 */
	{"spr7pos","sprite 7 vert-horiz start position",AASpriteStart, 0},		/* 0x178 */
	{"spr7ctl","sprite 7 vert stop position and control data",ParseNothing, 0},	/* 0x17a */
	{"spr7data","sprite 7 image data register A",ParseNothing, 0},			/* 0x17c */
	{"spr7datb","sprite 7 image data register B",ParseNothing, 0},			/* 0x17e */

	{"color0","",ParseNothing, COLOR},						/* 0x180 */
	{"color1","",ParseNothing, COLOR},						/* 0x182 */
	{"color2","",ParseNothing, COLOR},						/* 0x184 */
	{"color3","",ParseNothing, COLOR},						/* 0x186 */
	{"color4","",ParseNothing, COLOR},						/* 0x188 */
	{"color5","",ParseNothing, COLOR},						/* 0x18a */
	{"color6","",ParseNothing, COLOR},						/* 0x18c */
	{"color7","",ParseNothing, COLOR},						/* 0x18e */
	{"color8","",ParseNothing, COLOR},						/* 0x190 */
	{"color9","",ParseNothing, COLOR},						/* 0x192 */
	{"color10","",ParseNothing, COLOR},						/* 0x194 */
	{"color11","",ParseNothing, COLOR},						/* 0x196 */
	{"color12","",ParseNothing, COLOR},						/* 0x198 */
	{"color13","",ParseNothing, COLOR},						/* 0x19a */
	{"color14","",ParseNothing, COLOR},						/* 0x19c */
	{"color15","",ParseNothing, COLOR},						/* 0x19e */
	{"color16","",ParseNothing, COLOR},						/* 0x1a0 */
	{"color17","",ParseNothing, COLOR},						/* 0x1a2 */
	{"color18","",ParseNothing, COLOR},						/* 0x1a4 */
	{"color19","",ParseNothing, COLOR},						/* 0x1a6 */
	{"color20","",ParseNothing, COLOR},						/* 0x1a8 */
	{"color21","",ParseNothing, COLOR},						/* 0x1aa */
	{"color22","",ParseNothing, COLOR},						/* 0x1ac */
	{"color23","",ParseNothing, COLOR},						/* 0x1ae */
	{"color24","",ParseNothing, COLOR},						/* 0x1b0 */
	{"color25","",ParseNothing, COLOR},						/* 0x1b2 */
	{"color26","",ParseNothing, COLOR},						/* 0x1b4 */
	{"color27","",ParseNothing, COLOR},						/* 0x1b6 */
	{"color28","",ParseNothing, COLOR},						/* 0x1b8 */
	{"color29","",ParseNothing, COLOR},						/* 0x1ba */
	{"color30","",ParseNothing, COLOR},						/* 0x1bc */
	{"color31","",ParseNothing, COLOR},						/* 0x1be */

	{"htotal","colour clocks per line",ParseNothing, 0},				/* 0x1c0 */
	{"hsstop","HSYNC stop",ParseNothing, 0},					/* 0x1c2 */
	{"hbstrt","HBLANK start",ParseNothing, 0},					/* 0x1c4 */
	{"hbstop","HBLANK stop",ParseNothing, 0},					/* 0x1c6 */
	{"vtotal","max rows",ParseNothing, 0},						/* 0x1c8 */
	{"vsstop","VSYNC stop",ParseNothing, 0},					/* 0x1ca */
	{"vbstrt","VBLANK start",ParseNothing, 0},					/* 0x1cc */
	{"vbstop","VBLANK stop",ParseNothing, 0},					/* 0x1ce */

	{"sprhstrt","UHIRES sprite v start",ParseNothing, 0},				/* 0x1d0 */
	{"sprhstop","UHIRES sprite v stop",ParseNothing, 0},				/* 0x1d2 */
	{"bplhstrt","UHIRES bitplane v start",ParseNothing, 0},				/* 0x1d4 */
	{"bplhstop","UHIRES bitplane v stop",ParseNothing, 0},				/* 0x1d6 */
	{"hhposw","UHIRES stuff",ParseNothing, 0},					/* 0x1d8 */
	{"hhposr","UHIRES stuff",ParseNothing, 0},					/* 0x1da */

	{"beamcon0","beam counter control",ParseNothing, 0},				/* 0x1dc */
	{"hsstrt","HSYNC start",ParseNothing, 0},					/* 0x1de */
	{"vsstrt","VSYNC start",ParseNothing, 0},					/* 0x1e0 */
	{"hcenter","hpos for vsync in interlace",ParseNothing, 0},			/* 0x1e2 */
	{"diwhigh","MSB and LSB for display window",AAXDisplay, 0},			/* 0x1e4 */

	{"bplhmod","UHIRES bitplane modulo",ParseNothing, 0},				/* 0x1e6 */
	{"sprhpt","UHIRES sprite pointer",ParseNothing, ADDRESS},			/* 0x1e8 */
	{"sprhpt","UHIRES sprite pointer",ParseNothing, ADDRESS},			/* 0x1ea */
	{"bplhpt","UHIRES bitplane pointer",ParseNothing, ADDRESS},			/* 0x1ec */
	{"bplhpt","UHIRES bitplane pointer",ParseNothing, ADDRESS},			/* 0x1ee */

	{"reserved","ERROR",ParseNothing, 0},						/* 0x1f0 */
	{"reserved","ERROR",ParseNothing, 0},						/* 0x1f2 */
	{"reserved","ERROR",ParseNothing, 0},						/* 0x1f4 */
	{"reserved","ERROR",ParseNothing, 0},						/* 0x1f6 */
	{"reserved","ERROR",ParseNothing, 0},						/* 0x1f8 */
	{"reserved","ERROR",ParseNothing, 0},						/* 0x1fa */

	{"fmode  ","fetch mode",AAFetch, 0},						/* 0x1fc */
	{"nop  ","space saver",ParseNothing, 0},					/* 0x1fe */
	{"UNKNOWN_OPCODE", "",ParseNothing, 0}						/* 0xFFF */


#define MAX_CHIP_REGISTER 0x1FE

#define REGISTER_MASK 0xFFF
