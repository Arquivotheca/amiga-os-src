/* copdis - standalone copper list disassembler
 *
 * by Karl Lehenbauer, April 1989
 *
 * This code is released to the public domain for any legal use.
 *
 * This is free software.  No warranties are expressed or implied.
 *
 *
 * There should be a text file, copdis.doc, that describes the function
 * of the program in greater detail.  If you don't have it, you've been
 * ripped off!
 *
 * A quick summary: to disassemble a copper list, call copdis and pass it
 * the address of the start of the copper list.  Note that this is not
 * the address of the cprlst or CopList data structure, but the actual
 * address of the first copper instruction.
 *
 * Some examples:
 *
 *  GfxBase's copinit list, this may not be right
 *  copdis(GfxBase->copinit);	
 *
 *	Top Viewport's display instructions for the "long frame"
 *  copdis(GfxBase->ActiView->ViewPort->DspIns->CopLStart);
 *
 *	Top Viewport's display instructions for the "short frame"
 *  copdis(GfxBase->ActiView->ViewPort->DspIns->CopSStart);
 *
 *	Currently active LOF copper list
 *  copdis(GfxBase->ActiView->LOFCprList->start);
 *
 *  Currently active SHF copper list:
 *  copdis(GfxBase->ActiView->SHFCprList->start);
 *
 */

#include <stdio.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>

struct copregstruct
{
	char *regname;
	char *longname;
	UWORD regid;
	UWORD flags;
};

struct copinstruction 
{
	UWORD ir1;
	UWORD ir2;
};

#define COLOR 1
#define ADDRESS 2

static struct copregstruct copregisters[] =
{
	{"bltddat","blitter destination early read (dummy address)",0,0},
	{"dmaconr","DMA control (and blitter status) read",0x2,0},
	{"vposr","read vert most signif. bit (and frame flop)",0x4,0},
	{"vhposr","read vert and horiz. position of beam",0x6,0},
	{"dskdatr","disk data early read (dummy address)",0x8,0},
	{"joy0dat","joystick-mouse 0 data (vert,horiz)",0xa,0},
	{"joy1dat","joystick-mouse 1 data (vert,horiz)",0xc,0},
	{"clxdat","collision data (read and clear)",0xe,0},
	{"adkconr","audio, disk control register read",0x10,0},
	{"pot0dat","pot counter pair 0 data (vert,horiz)",0x12,0},
	{"pot1dat","pot counter pair 1 data (vert,horiz)",0x14,0},
	{"potgor","pot port data read",0x16,0},
	{"serdatr","serial port data and status read",0x18,0},
	{"dskbytr","disk data byte and status read",0x1a,0},
	{"intenar","interrupt enable bits read",0x1c,0},
	{"intreqr","interrupt request bits read",0x1e,0},

	{"dskpth","disk pointer (high 3 bits)",0x20,ADDRESS},
	{"dskptl","disk pointer (low 15 bits)",0x22,ADDRESS},
	{"dsklen","disk length",0x24,0},
	{"dskdat","disk DMA data write",0x26,0},

	{"refptr","refresh pointer",0x28,ADDRESS},
	{"vposw","write vert most signif. bit (and frame flop)",0x2a,0},
	{"vhposw","write vert and horiz position of beam",0x2c,0},
	{"copcon","copper control register (CDANG)",0x2e,0},
	{"serdat","serial port data and stop bits write",0x30,0},
	{"serper","serial port period and control",0x32,0},
	{"potgo","pot port data write and start",0x34,0},
	{"joytest","write all four joystick-mouse counters",0x36,0},
	{"strequ","strobe for horiz sync with vertical blank and EQU",0x38,0},
	{"strvbl","strobe for horiz sync with vertical blank",0x3a,0},
	{"strhor","strobe for horizontal sync",0x3c,0},
	{"strlong","strobe for identification of long horiz. line",0x3e,0},

	{"bltcon0","blitter control register 0",0x40,0},
	{"bltcon1","blitter control register 1",0x42,0},
	{"bltafwm","blitter first word mask for source A",0x44,0},
	{"bltalwm","blitter last word mask for source A",0x46,0},
	{"bltcpth","blitter pointer to source C (high 3 bits)",0x48,ADDRESS},
	{"bltcptl","blitter pointer to source C (low 15 bits)",0x4a,ADDRESS},
	{"bltbpth","blitter pointer to source B (high 3 bits)",0x4c,ADDRESS},
	{"bltbptl","blitter pointer to source B (low 15 bits)",0x4e,ADDRESS},
	{"bltapth","blitter pointer to source A (high 3 bits)",0x50,ADDRESS},
	{"bltaptl","blitter pointer to source A (low 15 bits)",0x52,ADDRESS},
	{"bltdpth","blitter pointer to source D (high 3 bits)",0x54,ADDRESS},
	{"bltdptl","blitter pointer to source D (low 15 bits)",0x56,ADDRESS},
	{"bltsize","blitter start and size (window width, height)",0x58,0},
	{"bltcon0l","blitter control 0 lower 8 bits (minterms)",0x5a,0},
	{"bltsizv","blitter V size",0x5c,0},
	{"bltsizh","blitter H size",0x5e,0},
	{"bltcmod","blitter modulo for source C",0x60,0},
	{"bltbmod","blitter modulo for source B",0x62,0},
	{"bltamod","blitter modulo for source A",0x64,0},
	{"bltdmod","blitter modulo for source D",0x66,0},
	{"unused","ERROR",0x68,0},
	{"unused","ERROR",0x6a,0},
	{"unused","ERROR",0x6c,0},
	{"unused","ERROR",0x6e,0},
	{"bltcdat","blitter source C data",0x70,0},
	{"bltbdat","blitter source B data",0x72,0},
	{"bltadat","blitter source A data",0x74,0},
	{"unused","ERROR",0x76,0},
	{"sprhdat","UHIRES stuff",0x78,0},
	{"bplhdat","UHIRES stuff",0x7a,ADDRESS},
	{"lisaid","Denise/Lisa chip rev",0x7c,0},
	{"dsksync","disk sync pattern for disk read",0x7e,0},

	{"cop1lch","copper first location (high 3 bits)",0x80,ADDRESS},
	{"cop1lcl","copper first location (low 15 bits)",0x82,ADDRESS},
	{"cop2lch","copper second location (high 3 bits)",0x84,ADDRESS},
	{"cop2lcl","copper second location (low 15 bits)",0x86,ADDRESS},
	{"copjmp1","restart copper at first location",0x88,ADDRESS},
	{"copjmp2","restart copper at second location",0x8a,ADDRESS},
	{"copins","copper instruction fetch identify",0x8c,0},
	{"diwstrt","display window start (upper left vert-horiz position)",0x8e,0},
	{"diwstop","display window stop (lower right vert-horiz position)",0x90,0},
	{"ddfstrt","display bit plane data fetch start (horiz. position)",0x92,0},
	{"ddfstop","display bit plane data fetch stop (horiz. position)",0x94,0},
	{"dmacon","DMA control write (clear or set)",0x96,0},
	{"clxcon","collision control",0x98,0},
	{"intena","interrupt enable bits",0x9a,0},
	{"intreq","interrupt request bits",0x9c,0},
	{"adkcon","audio, disk, UART control",0x9e,0},

	{"aud0lch","audio channel 0 location (high 3 bits)",0xa0,ADDRESS},
	{"aud0lcl","audio channel 0 location (low 15 bits)",0xa2,ADDRESS},
	{"aud0len","audio channel 0 length",0xa4,0},
	{"aud0per","audio channel 0 period",0xa6,0},
	{"aud0vol","audio channel 0 volume",0xa8,0},
	{"aud0dat","audio channel 0 data",0xaa,0},
	{"unused","ERROR",0xac,0},
	{"unused","ERROR",0xae,0},

	{"aud1lch","audio channel 1 location (high 3 bits)",0xb0,ADDRESS},
	{"aud1lcl","audio channel 1 location (low 15 bits)",0xb2,ADDRESS},
	{"aud1len","audio channel 1 length",0xb4,0},
	{"aud1per","audio channel 1 period",0xb6,0},
	{"aud1vol","audio channel 1 volume",0xb8,0},
	{"aud1dat","audio channel 1 data",0xba,0},
	{"unused","ERROR",0xbc,0},
	{"unused","ERROR",0xbe,0},

	{"aud2lch","audio channel 2 location (high 3 bits)",0xc0,ADDRESS},
	{"aud2lcl","audio channel 2 location (low 15 bits)",0xc2,ADDRESS},
	{"aud2len","audio channel 2 length",0xc4,0},
	{"aud2per","audio channel 2 period",0xc6,0},
	{"aud2vol","audio channel 2 volume",0xc8,0},
	{"aud2dat","audio channel 2 data",0xca,0},
	{"unused","ERROR",0xcc,0},
	{"unused","ERROR",0xce,0},

	{"aud3lch","audio channel 3 location (high 3 bits)",0xd0,ADDRESS},
	{"aud3lcl","audio channel 3 location (low 15 bits)",0xd2,ADDRESS},
	{"aud3len","audio channel 3 length",0xd4,0},
	{"aud3per","audio channel 3 period",0xd6,0},
	{"aud3vol","audio channel 3 volume",0xd8,0},
	{"aud3dat","audio channel 3 data",0xda,0},
	{"unused","ERROR",0xdc,0},
	{"unused","ERROR",0xde,0},

	{"bpl1pth","blitter plane 1 pointer (high 3 bits)",0xe0,ADDRESS},
	{"bpl1ptl","blitter plane 1 pointer (low 15 bits)",0xe2,ADDRESS},
	{"bpl2pth","blitter plane 2 pointer (high 3 bits)",0xe4,ADDRESS},
	{"bpl2ptl","blitter plane 2 pointer (low 15 bits)",0xe6,ADDRESS},
	{"bpl3pth","blitter plane 3 pointer (high 3 bits)",0xe8,ADDRESS},
	{"bpl3ptl","blitter plane 3 pointer (low 15 bits)",0xea,ADDRESS},
	{"bpl4pth","blitter plane 4 pointer (high 3 bits)",0xec,ADDRESS},
	{"bpl4ptl","blitter plane 4 pointer (low 15 bits)",0xee,ADDRESS},
	{"bpl5pth","blitter plane 5 pointer (high 3 bits)",0xf0,ADDRESS},
	{"bpl5ptl","blitter plane 5 pointer (low 15 bits)",0xf2,ADDRESS},
	{"bpl6pth","blitter plane 6 pointer (high 3 bits)",0xf4,ADDRESS},
	{"bpl6ptl","blitter plane 6 pointer (low 15 bits)",0xf6,ADDRESS},
	{"bpl7pth","blitter plane 7 pointer (high 3 bits)",0xf8,ADDRESS},
	{"bpl7ptl","blitter plane 7 pointer (low 15 bits)",0xfa,ADDRESS},
	{"bpl8pth","blitter plane 8 pointer (high 3 bits)",0xfc,ADDRESS},
	{"bpl8ptl","blitter plane 8 pointer (low 15 bits)",0xfe,ADDRESS},


	{"bplcon0","bit plane control bits",0x100,0},
	{"bplcon1","bit plane scroll values for PF1 & PF2",0x102,0},
	{"bplcon2","bit plane priority control",0x104,0},
	{"bplcon3","genloc, colour bank etc.",0x106,0},

	{"bpl1mod","bit plane modulo for odd planes",0x108,0},
	{"bpl2mod","bit plane modulo for even planes",0x10a,0},

	{"bplcon4","bitplane masks, sprite masks",0x10c,0},
	{"clxcon2","upper bits of sprite collision",0x10e,0},
	
	{"bpl1dat","bit plane 1 data (parallel-to-serial convert)",0x110,0},
	{"bpl2dat","bit plane 2 data (parallel-to-serial convert)",0x112,0},
	{"bpl3dat","bit plane 3 data (parallel-to-serial convert)",0x114,0},
	{"bpl4dat","bit plane 4 data (parallel-to-serial convert)",0x116,0},
	{"bpl5dat","bit plane 5 data (parallel-to-serial convert)",0x118,0},
	{"bpl6dat","bit plane 6 data (parallel-to-serial convert)",0x11a,0},
	{"bpl7dat","bit plane 7 data (parallel-to-serial convert)",0x11c,0},
	{"bpl8dat","bit plane 8 data (parallel-to-serial convert)",0x11e,0},

	{"spr0pth","sprite 0 pointer (high 3 bits)",0x120,ADDRESS},
	{"spr0ptl","sprite 0 pointer (low 15 bits)",0x122,ADDRESS},
	{"spr1pth","sprite 1 pointer (high 3 bits)",0x124,ADDRESS},
	{"spr1ptl","sprite 1 pointer (low 15 bits)",0x126,ADDRESS},
	{"spr2pth","sprite 2 pointer (high 3 bits)",0x128,ADDRESS},
	{"spr2ptl","sprite 2 pointer (low 15 bits)",0x12a,ADDRESS},
	{"spr3pth","sprite 3 pointer (high 3 bits)",0x12c,ADDRESS},
	{"spr3ptl","sprite 3 pointer (low 15 bits)",0x12e,ADDRESS},
	{"spr4pth","sprite 4 pointer (high 3 bits)",0x130,ADDRESS},
	{"spr4ptl","sprite 4 pointer (low 15 bits)",0x132,ADDRESS},
	{"spr5pth","sprite 5 pointer (high 3 bits)",0x134,ADDRESS},
	{"spr5ptl","sprite 5 pointer (low 15 bits)",0x136,ADDRESS},
	{"spr6pth","sprite 6 pointer (high 3 bits)",0x138,ADDRESS},
	{"spr6ptl","sprite 6 pointer (low 15 bits)",0x13a,ADDRESS},
	{"spr7pth","sprite 7 pointer (high 3 bits)",0x13c,ADDRESS},
	{"spr7ptl","sprite 7 pointer (low 15 bits)",0x13e,ADDRESS},

	{"spr0pos","sprite 0 vert-horiz start position",0x140,0},
	{"spr0ctl","sprite 0 vert stop position and control data",0x142,0},
	{"spr0data","sprite 0 image data register A",0x144,0},
	{"spr0datb","sprite 0 image data register B",0x146,0},
	{"spr1pos","sprite 1 vert-horiz start position",0x148,0},
	{"spr1ctl","sprite 1 vert stop position and control data",0x14a,0},
	{"spr1data","sprite 1 image data register A",0x14c,0},
	{"spr1datb","sprite 1 image data register B",0x14e,0},
	{"spr2pos","sprite 2 vert-horiz start position",0x150,0},
	{"spr2ctl","sprite 2 vert stop position and control data",0x152,0},
	{"spr2data","sprite 2 image data register A",0x154,0},
	{"spr2datb","sprite 2 image data register B",0x156,0},
	{"spr3pos","sprite 3 vert-horiz start position",0x158,0},
	{"spr3ctl","sprite 3 vert stop position and control data",0x15a,0},
	{"spr3data","sprite 3 image data register A",0x15c,0},
	{"spr3datb","sprite 3 image data register B",0x15e,0},
	{"spr4pos","sprite 4 vert-horiz start position",0x160,0},
	{"spr4ctl","sprite 4 vert stop position and control data",0x162,0},
	{"spr4data","sprite 4 image data register A",0x164,0},
	{"spr4datb","sprite 4 image data register B",0x166,0},
	{"spr5pos","sprite 5 vert-horiz start position",0x168,0},
	{"spr5ctl","sprite 5 vert stop position and control data",0x16a,0},
	{"spr5data","sprite 5 image data register A",0x16c,0},
	{"spr5datb","sprite 5 image data register B",0x16e,0},
	{"spr6pos","sprite 6 vert-horiz start position",0x170,0},
	{"spr6ctl","sprite 6 vert stop position and control data",0x172,0},
	{"spr6data","sprite 6 image data register A",0x174,0},
	{"spr6datb","sprite 6 image data register B",0x176,0},
	{"spr7pos","sprite 7 vert-horiz start position",0x178,0},
	{"spr7ctl","sprite 7 vert stop position and control data",0x17a,0},
	{"spr7data","sprite 7 image data register A",0x17c,0},
	{"spr7datb","sprite 7 image data register B",0x17e,0},

	{"color0","",0x180,COLOR},
	{"color1","",0x182,COLOR},
	{"color2","",0x184,COLOR},
	{"color3","",0x186,COLOR},
	{"color4","",0x188,COLOR},
	{"color5","",0x18a,COLOR},
	{"color6","",0x18c,COLOR},
	{"color7","",0x18e,COLOR},
	{"color8","",0x190,COLOR},
	{"color9","",0x192,COLOR},
	{"color10","",0x194,COLOR},
	{"color11","",0x196,COLOR},
	{"color12","",0x198,COLOR},
	{"color13","",0x19a,COLOR},
	{"color14","",0x19c,COLOR},
	{"color15","",0x19e,COLOR},
	{"color16","",0x1a0,COLOR},
	{"color17","",0x1a2,COLOR},
	{"color18","",0x1a4,COLOR},
	{"color19","",0x1a6,COLOR},
	{"color20","",0x1a8,COLOR},
	{"color21","",0x1aa,COLOR},
	{"color22","",0x1ac,COLOR},
	{"color23","",0x1ae,COLOR},
	{"color24","",0x1b0,COLOR},
	{"color25","",0x1b2,COLOR},
	{"color26","",0x1b4,COLOR},
	{"color27","",0x1b6,COLOR},
	{"color28","",0x1b8,COLOR},
	{"color29","",0x1ba,COLOR},
	{"color30","",0x1bc,COLOR},
	{"color31","",0x1be,COLOR},

	{"htotal","colour clocks per line",0x1c0,0},
	{"hsstop","HSYNC stop",0x1c2,0},
	{"hbstrt","HBLANK start",0x1c4,0},
	{"hbstop","HBLANK stop",0x1c6,0},
	{"vtotal","max rows",0x1c8,0},
	{"vsstop","VSYNC stop",0x1ca,0},
	{"vbstrt","VBLANK start",0x1cc,0},
	{"vbstop","VBLANK stop",0x1ce,0},

	{"sprhstrt","UHIRES sprite v start",0x1d0,0},
	{"sprhstop","UHIRES sprite v stop", 0x1d2,0},
	{"bplhstrt","UHIRES bitplane v start",0x1d4},
	{"bplhstop","UHIRES bitplane v stop",0x1d6},
	{"hhposw","UHIRES stuff",0x1d8,0},
	{"hhposr","UHIRES stuff",0x1da,0},

	{"beamcon0","beam counter control",0x1dc,0},
	{"hsstrt","HSYNC start",0x1de,0},
	{"vsstrt","VSYNC start",0x1e0,0},
	{"hcenter","hpos for vsync in interlace",0x1e2,0},
	{"diwhigh","MSB and LSB for display window",0x1e4,0},

	{"bplhmod","UHIRES bitplane modulo",0x1e6,0},
	{"sprhpth","UHIRES sprite pointer",0x1e8,ADDRESS},
	{"sprhptl","UHIRES sprite pointer",0x1ea,ADDRESS},
	{"bplhpth","UHIRES bitplane pointer",0x1ec,ADDRESS},
	{"bplhptl","UHIRES bitplane pointer",0x1ee,ADDRESS},

	{"reserved","ERROR",0x1f0,0},
	{"reserved","ERROR",0x1f2,0},
	{"reserved","ERROR",0x1f4,0},
	{"reserved","ERROR",0x1f6,0},
	{"reserved","ERROR",0x1f8,0},
	{"reserved","ERROR",0x1fa,0},

	{"fmode  ","fetch mode",0x1fc,0},
	{"nop  ","space saver",0x1fe,0},

	{"","",-1,0}
};

struct copregstruct *find_copper_register_info(reg)
int reg;
{
	struct copregstruct *regp;

	regp = &copregisters[reg >> 1];
	return(regp);

#ifdef UNDEF
	for (regp = &copregisters[0]; regp->regid != -1; regp++)
	{
		if (regp->regid == reg)
			return(regp);
	}
	return(0);
#endif
}

disassemble_copper_instruction(ip, comm, data, nocolours)
struct copinstruction *ip;
BOOL comm;
BOOL data;
BOOL nocolours;
{
	short vp, hp, ve, he, bfd;
	int registerid;
	struct copregstruct *regp;
	char *regtext, *longtext;

	/* printf("instruction %x, %x\n",ip->ir1,ip->ir2); */


	/* true if it's a branch or skip instruction */
	if (ip->ir1 & 1)
	{
		printf("%08lx", (data ? NULL : ip));
		/* branch or skip instruction */

		/* calculate vertical and horizontal position */
		vp = (ip->ir1 >> 8) & 0xff;
		hp = (ip->ir1 >> 1) & 0x7f;

		/* calculate vertical and horizontal position compare bits
		 * and blitter-finished-disable bit */
		ve = (ip->ir2 >> 8) & 0x7f;
		he = (ip->ir2 >> 1) & 0x7f;
		bfd = (ip->ir2 & 0x8000);

		/* print the instruction type, check bit 0 of second word */
		if (ip->ir2 & 1)
		{
			/* it's a skip */
			printf("\tSKIP");
		}
		else
		{
			/* it's a wait */
			printf("\tWAIT");
		}

		printf("\t(%x,%x)", ip->ir1, ip->ir2);

		/* reverse sense of blitter finish wait disable to show blitter 
		 * finish wait -- that way we occasionally say "blitter" instead
		 * of almost always saying "no blitter"
		 */
		if (!bfd)
			printf("\tbf,");
		else
			printf("\t");

		/* print vertical and horizontal position */
		printf("vpos=%x,hpos=%x",vp,hp);
/*		printf(" instruction %x, %x\n",ip->ir1,ip->ir2);*/

		/* if enable masks are not basically "all", print them */
		if ((ve != 0x7f) || (he != 0x7f))
		{
			printf(",vcomp=%d,hcomp=%d",ve,he);
		}
		printf("\n");
	}
	else
	{
		/* it's a move instruction */

		/* calculate register ID and look it up */
		registerid = ip->ir1 & 0x1ff;
		regp = find_copper_register_info(registerid);

		/* if we found the register in our table, print the text
		 * else whine about something we don't understand */
		if (regp)
		{
			regtext = regp->regname;
			longtext = regp->longname;
		}
		else
		{
			regtext = "UNKNOWN_OPCODE";
			longtext = "";
		}

		if (((data) && (regp->flags & ADDRESS)) ||
		    ((nocolours) && (regp->flags & COLOR)))
		{
//			printf("\tMOVE\t%4x,%s",NULL,regtext);
//			printf("\t(%x,%x)", NULL, NULL);
		}
		else
		{
			printf("%08lx", (data ? NULL : ip));
			printf("\tMOVE\t%4x,%s",ip->ir2,regtext);
			printf("\t(%x,%x)", ip->ir1, ip->ir2);
		}

		if ((comm) && (*longtext != '\0'))
			printf("\t;%s",longtext);

		if (!(nocolours && (regp->flags & COLOR)))
		{
			printf("\n");
		}
	}
}

extern struct GfxBase *GfxBase;

/* have a go at disassembling a copper list */
copdis(coplist, comm, data, nocolours)
struct copinstruction *coplist;
BOOL comm;
BOOL data;
BOOL nocolours;
{

	/* check and refuse null list pointers */
	if (coplist == (struct copinstruction *)NULL)
	{
		fprintf(stderr,"copdis: copper list pointer is null\n");
		return;
	}

	/* while we don't have an instruction of 0xffff 0xfffe, which is
	 * a de facto END PROGRAM instruction, print instructions.
	 *
	 * Use of do-while is in order that the END instruction is also
	 * disassembled.
	 *
	 * This should really have a sanity check (like no more than a few
	 * hundred instructions, and/or a length -- heck, it's in the cprlst
	 * and CopList structures anyway), but it doesn't, or at least not
	 * yet
	 */
//	ObtainSemaphore(GfxBase->ActiViewCprSemaphore);
	do
	{
		disassemble_copper_instruction(coplist++, comm, data, nocolours);
	}
	while ((coplist->ir1 != 0xffff) || (coplist->ir2 != 0xfffe));

	/* and disassemble the WAIT 255 instruction */
	disassemble_copper_instruction(coplist, comm, data, nocolours);
//	ReleaseSemaphore(GfxBase->ActiViewCprSemaphore);

}
