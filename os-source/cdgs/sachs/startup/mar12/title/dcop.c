/* copper.c -- copper list disassembly
*
 * This is an enhanced version of a Copper List Disassembler
 * Routine by Scott Evernden, 10/14/86, enhancements by
 * Jim Mackraz and I and I Computing.
 *
 * This program is in the public domain.
 *
 * When using Aztec C compiler, use "cc -z3000" because
 * of all the strings.
 */
/* magic z macro:  >%<<%<< */

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <graphics/sprite.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* needs to be updates real soon now	*/
static char *names[] = {
	"bltddat","dmaconr","vposr","vhposr","dskdatr",
	"joy0dat","joy1dat","clxdat","adkconr",
	"pot0dat","pot1dat","potinp","serdatr","dskbytr",
	"intenar","intreqr","dskpt","dskpt+2","dsklen","dskdat",
	"refptr","vposw","vhposw","copcon","serdat","serper",
	"potgo","joytest","strequ","strvbl","strhor","strlong",
	"bltcon0","bltcon1","bltafwm","bltalwm",
	"bltcpt","bltcpt+2","bltbpt","bltbpt+2",
	"bltapt","bltapt+2","bltdpt","bltdpt+2",
	"bltsize","pad2d[0]","pad2d[1]","pad2d[2]",
	"bltcmod","bltbmod","bltamod","bltdmod",
	"pad34[0]","pad34[1]","pad34[2]","pad34[3]",
	"bltcdat","bltbdat","bltadat",
	"pad3b[0]","pad3b[1]","pad3b[2]","pad3b[3]","dsksync",
	"cop1lc","cop1lc+2","cop2lc","cop2lc+2","copjmp1","copjmp2",
	"copins","diwstrt","diwstop","ddfstrt","ddfstop",
	"dmacon","clxcon","intena","intreq","adkcon",
	"aud[0].ac_ptr","aud[0].ac_ptr+2",
	"aud[0].ac_len","aud[0].ac_per","aud[0].ac_vol","aud[0].ac_dat",
	"aud[0].ac_pad[0]","aud[0].ac_pad[1]",
	"aud[1].ac_ptr","aud[1].ac_ptr+2",
	"aud[1].ac_len","aud[1].ac_per","aud[1].ac_vol","aud[1].ac_dat",
	"aud[1].ac_pad[0]","aud[1].ac_pad[1]",
	"aud[2].ac_ptr","aud[2].ac_ptr+2",
	"aud[2].ac_len","aud[2].ac_per","aud[2].ac_vol","aud[2].ac_dat",
	"aud[2].ac_pad[0]","aud[2].ac_pad[1]",
	"aud[3].ac_ptr","aud[3].ac_ptr+2",
	"aud[3].ac_len","aud[3].ac_per","aud[3].ac_vol","aud[3].ac_dat",
	"aud[3].ac_pad[0]","aud[3].ac_pad[1]",

	"bpl1pth","bpl1ptl","bpl2pth","bpl2ptl",
	"bpl3pth","bpl3ptl","bpl4pth","bpl4ptl",
	"bpl5pth","bpl5ptl","bpl6pth","bpl6ptl",

	"pad7c[0]","pad7c[1]","pad7c[2]","pad7c[3]",

	"bplcon0","bplcon1","bplcon2","pad83",
	"bpl1mod","bpl2mod","pad86[0]","pad86[1]",

	"bpl1dat","bpl2dat","bpl3dat","bpl4dat",
	"bpl5dat","bpl6dat","pad8e[0]","pad8e[1]",

	"spr0pth","spr0ptl","spr1pth","spr1ptl",
	"spr2pth","spr2ptl","spr3pth","spr3ptl",
	"spr4pth","spr4ptl","spr5pth","spr5ptl",
	"spr6pth","spr6ptl","spr7pth","spr7ptl",

	"spr0pos","spr0ctl","spr0data","spr0datb",
	"spr1pos","spr1ctl","spr1data","spr1datb",
	"spr2pos","spr2ctl","spr2data","spr2datb",
	"spr3pos","spr3ctl","spr3data","spr3datb",
	"spr4pos","spr4ctl","spr4data","spr4datb",
	"spr5pos","spr5ctl","spr5data","spr5datb",
	"spr6pos","spr6ctl","spr6data","spr6datb",
	"spr7pos","spr7ctl","spr7data","spr7datb",

	"color0","color1","color2","color3",
	"color4","color5","color6","color7",
	"color8","color9","color10","color11",
	"color12","color13","color14","color15",
	"color16","color17","color18","color19",
	"color20","color21","color22","color23",
	"color24","color25","color26","color27",
	"color28","color29","color30","color31"
};

#define MAXNAME (((sizeof (names))/sizeof (char *)) - 1)
char	noname[24];

static char *regname( indx )
{
	if (indx > MAXNAME)
	{
		sprintf( noname, "<<no name: %x>>", indx);
		return (noname);
	}
	else
	{
		 return (names[indx]);
	} 
}

extern struct GfxBase	*GfxBase;

/* command line switches */
int	all_bool = 0;		/* if true, do color tables, too */
int	intermed_bool = 0;	/* do view port intermed. copps	*/
int	header_bool = 0;	/* do hardware header list	*/
int	hardware_bool = 0;	/* default: hardware lists	*/


/* display intermediate copper list	*/
dumpCopList( str, cl )
char	*str;				/* name of this copper list	*/
struct CopList *cl;
{
	int count;
	struct CopIns	*ci;

	// kprintf("%s\n", str);

	while (cl)
	{
		ci = cl->CopIns;
		for (count = cl->Count; count; --count)
		{
			if (dumpCopIns( ci++ )) break;	/* it sez: "next list"	*/
		}

		cl = cl->Next;
	}
}

/* dump Intermediate Copper List instruction
 * return non-zero if it's time to branch to next list
 */
dumpCopIns( ci )
struct CopIns *ci;
{
	UWORD	dest;

	switch (ci->OpCode)
	{
	case COPPER_MOVE:
		dest = (ci->DESTADDR >> 1) &0xff;
		// kprintf("%lx\tCMOVE\t%04lx,%s\n", ci, ci->DESTDATA, regname(dest));
		break;
	case COPPER_WAIT:
		// kprintf("%lx\tCWAIT\t%04lx,%04lx\n", ci, ci->HWAITPOS, ci->VWAITPOS);
		break;
	case CPRNXTBUF:
		return (1);		/* is 'ci->nxtlist'	the same buffer as cl->Next? */
		
	default:
		// kprintf("unknown CopIns OpCodr: %lx\n", ci->OpCode);
	}
	return (0);
}

/* display User (intermediate) Copper List	*/
dumpUCopList( ucl )
struct UCopList *ucl;
{
	while (ucl)
	{
		// kprintf("UCopList block: %lx\n", ucl);
		dumpCopList("...", ucl->FirstCopList);
		ucl = ucl->Next;
	}
}

int skipped_color;

/*
 * display hardware copper list
 */
dumpcprlist( coplist, all)
struct cprlist *coplist;
int				all;		/* 'all' means "color table loading, too?" */
{
    UWORD	*p;		/* clicks off amiga addresses	*/
    int		count;
    UWORD	copinsbuff[2];

    if (!coplist) return;

    count = coplist->MaxCount;

    p = coplist->start;

    skipped_color = 0;		/* start him off on the right foot	*/

    while (count--)
    {
		if (copDisasm( p, p[0], p[1], all)) break;
		p += 2;
    }
}

/*
 * display copinit copper list header
 */
dumpcprheader( p )
UWORD	*p;		/* the actual instructions */
{
	while (! copDisasm(p, p[0], p[1], 1) )
	{
		p += 2;
	}
}

char	buff[256];
char	linebuff[256];

/* formatted disassembly of hardware copper instruction
 * return non-zero if you want to stop
 */
copDisasm( addr, op, w, all)
APTR	addr;
UWORD	op, w;
{
    UWORD vp, hp, vm, hm, da;
    int	skcolor;
	char	*name;

    int retval;

    skcolor = skipped_color;
    skipped_color = 0;

    linebuff[0] = '\0';

    /* build output, defer display	*/
    sprintf(buff, "%08lx  %04lx:%04lx\t", addr, op, w);
    strcat(linebuff,  buff);

    if (op & 1)		/* wait or skip	*/
    {
		vp = (op >> 8) & 0xFF;
		hp = (op >> 1) & 0x7F;
		vm = (w >> 8) & 0x7F;
		hm = (w >> 1) & 0x7F;

		sprintf(buff, "%s", w & 1 ? "SKIP" : "WAIT");
		strcat(linebuff,  buff);

		if ((w & 0x8000) == 0) printf("BF");

		sprintf(buff, "\t(%02lx,%02lx)", hp, vp);
		strcat(linebuff,  buff);

		if (hm != 0x7F || vm != 0x7F)
		{
		    sprintf(buff, " mask (%02lx,%02lx)", hm, vm);
		    strcat(linebuff,  buff);
		}

		/* comment	*/
		sprintf(buff, "\t\t; (x = %ld, y = %ld)", hp, vp);
		strcat(linebuff,  buff);

		// kprintf("%s\n", linebuff);	/* output	*/

		retval = !(w & 1) && vp >= 0xFF && hp >= 0x7F;
    }
    else			/* move	*/
    {
		retval = 0;
		da = (op >> 1) & 0xFF;

		name = regname( da );

		sprintf(buff, "MOVE\t%04lx,%s", w, name);
		strcat(linebuff,  buff);

		prComment( name, w);	/* append comment to linebuff	*/

		if (all || strncmp(name, "color", 5))
		{
		    printf("%s\n", linebuff);
		}
		else
		{
		    if (!skcolor)
		    {
			// kprintf("--- load color table ---\n");
		    }
		    skipped_color = 1;
		}
    }
    return retval;
}

/* give a little more info for the contents of some registers
 * kind o' hokey method, eh?
 */
prComment( regname, w)
char	*regname;
{
    int vp, hp;

    /* just in case */
    vp = (w >> 8) & 0xFF;
    hp = (w >> 0) & 0xFF;

    buff[0] = '\0';

    if (!strncmp(regname, "diwstrt", 7))
    {
		sprintf(buff, "\t; (left = %ld, top = %ld)", hp, vp);
    }
    else if (!strncmp(regname, "diwstop", 7))
    {
		sprintf(buff, "\t; (right = %d, bottom = %d)",
		    hp + 256, (!(vp & 0x80))?  vp+256 : vp);
    }
    else if (!strncmp(regname, "ddfstrt", 7))
    {
		sprintf(buff, "\t; pixel val = %ld", w << 1);
    }
    else if (!strncmp(regname, "ddfstop", 7))
    {
		sprintf(buff, "\t; pixel val = %ld", w << 1);
    }

    strcat(linebuff, buff);
}

