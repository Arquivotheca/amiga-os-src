/* copper.c -- copper list disassembly */
/* $Id: copper.c,v 1.2 91/04/24 20:51:28 peter Exp $	*/

#include "exec/types.h"
#include "exec/libraries.h"

#include "graphics/gfxbase.h"
#include "graphics/view.h"
#include "graphics/copper.h"

#include "symbols.h"
#include "special.h"

#include <stdio.h>

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
	"bltsize","bltcon0l","bltsizv","bltsizh",
	"bltcmod","bltbmod","bltamod","bltdmod",
	"pad34[0]","pad34[1]","pad34[2]","pad34[3]",
	"bltcdat","bltbdat","bltadat",
	"pad3b[0]","pad3b[1]","pad3b[2]","deniseid","dsksync",
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
	"bpl7pth","bpl7ptl","bpl8pth","bpl8ptl",

	"bplcon0","bplcon1","bplcon2","pad83",
	"bpl1mod","bpl2mod","pad86[0]","pad86[1]",

	"bpl1dat","bpl2dat","bpl3dat","bpl4dat",
	"bpl5dat","bpl6dat","bpl7dat","bpl8dat",

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
	"color28","color29","color30","color31",

	/* new ones, all */
	"htotal","hsstop","hbstrt","hbstop",
	"vtotal","vsstop","vbstrt","vbstop",
	"sprhstrt","sprhstop","bplhstrt","bplhstop",
	"hhposw","hhposr","beamcon0","hsstrt",
	"vsstrt","hcenter","diwhigh",
};



#define NUMNAMES	(sizeof (names) / sizeof (char *))

char	noname[69];

char *
regname( reg )
{
    if (reg < NUMNAMES) return (names[reg]);
    else 
    {
	sprintf( noname, "<<no name: %x>>", reg);
	return (noname);
    }
}



dumpCops()
{
    doDumpCops(0);	/* no color table	*/
}

dumpAllCops()
{
    doDumpCops(1);	/* color table, too	*/
}

dumpCopHeader()
{
    doDumpCops(2);	/* color table, too	*/
}


/*
 *	0 - LOF/SHF: skip color table
 *	1 - LOF/SFH: include color table
 *	2 - GfxBase->copinit
 */
doDumpCops( listtype )
{
    APTR gbase;			/* amiga address of graphics base */
    struct GfxBase GBase;	/* sun buffer for base */
    struct View 	view;

    printf("\n");

    if (gbase =  FindBase ("graphics.library"))
    {
	/* fill sun bugger with data from amiga */
	GetBlock(gbase, &GBase, sizeof GBase);

	if ( listtype  == 2 )
	{
	    ShowAddress( "copinit", GBase.copinit );
	    dumpCprList( GBase.copinit, listtype );
	}
	else
	{
	    ShowAddress ("ActiView", GBase.ActiView);
	    GetBlock(GBase.ActiView, &view, sizeof view);

	    printf("LOF:\n");
	    dumpCprList( view.LOFCprList, listtype );

	    printf("SHF:\n");
	    dumpCprList( view.SHFCprList, listtype );
	}
    }
    else
    {
	printf("No graphics.library?\n");
    }
}

int skipped_color;

/*
 * 'type' means:
 *	0 - struct cprlist, skip colors
 *	1 - struct cprlist, include colors
 *	2 - pointer to actual copper instructions
 */
dumpCprList( list, type )
struct cprlist *list;
{
    UWORD	*p;		/* clicks off amiga addresses	*/
    int		count;
    struct	cprlist coplist;
    UWORD	copinsbuff[2];
    int		all;

    if (!list) return;


    /* rely on inifinite WAIT to terminate	*/
    if ( type == 2 )
    {
	count = 9999;
	p = (UWORD *) list;
	all = 1;
    }
    else
    {
	GetBlock( list, &coplist, sizeof coplist);
	count =  coplist.MaxCount;
	printf("copper count: %d\n", count);
	p = coplist.start;
	all = type;
    }


    skipped_color = 0;		/* start him off on the right foot	*/

    while (count--)
    {
	GetBlock( p, copinsbuff, 4);
	if (copDisasm( p, copinsbuff[0], copinsbuff[1], all))
	{
	    break;
	}
	p += 2;
    }
}

char	buff[256];
char	linebuff[256];

copDisasm( addr, op, w, all)
APTR	addr;
UWORD	op, w;
{
    UWORD vp, hp, vm, hm, da;
    int	skcolor;

    int r;

    skcolor = skipped_color;
    skipped_color = 0;

    linebuff[0] = '\0';

    /* build output, defer display	*/
    sprintf(buff, "%08lx\t", addr);
    strcat(linebuff,  buff);

    if (op & 1)
    {
	vp = (op >> 8) & 0xFF;
	hp = (op >> 1) & 0x7F;
	vm = (w >> 8) & 0x7F;
	hm = (w >> 1) & 0x7F;

	sprintf(buff, "%s", w & 1 ? "SKIP" : "WAIT");
	strcat(linebuff,  buff);

	if ((w & 0x8000) == 0) printf("BF");

	sprintf(buff, "\t(%02x,%02x)", hp, vp);
	strcat(linebuff,  buff);

	if (hm != 0x7F || vm != 0x7F)
	{
	    sprintf(buff, " mask (%02x,%02x)", hm, vm);
	    strcat(linebuff,  buff);
	}

	/* comment	*/
	sprintf(buff, "\t\t; (x = %d, y = %d)", hp, vp);
	strcat(linebuff,  buff);

	printf("%s\n", linebuff);	/* output	*/

	r = !(w & 1) && vp >= 0xFF && hp >= 0x7F;
    }
    else
    {
	r = 0;
	da = (op >> 1) & 0xFF;

	sprintf(buff, "MOVE\t%04x,%s", w, regname(da));
	strcat(linebuff,  buff);

	prComment( regname(da), w);	/* append comment to linebuff	*/

	if (all || strncmp(regname(da), "color", 5))
	{
	    printf("%s\n", linebuff);
	}
	else
	{
	    if (!skcolor)
	    {
		printf("--- load color table ---\n");
	    }
	    skipped_color = 1;
	}

    }
    return r;
}

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
	sprintf(buff, "\t; (x = %d, y = %d)", hp, vp);
    }
    else if (!strncmp(regname, "diwstop", 7))
    {
	sprintf(buff, "\t; (x = %d, y = %d)",
	    hp + 256, (!(vp & 0x80))?  vp+256 : vp);
    }
    else if (!strncmp(regname, "ddfstrt", 7))
    {
	sprintf(buff, "\t; pixel val = %d", w << 1);
    }
    else if (!strncmp(regname, "ddfstop", 7))
    {
	sprintf(buff, "\t; pixel val = %d", w << 1);
    }

    strcat(linebuff, buff);
}

