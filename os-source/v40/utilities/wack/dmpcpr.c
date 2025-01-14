/* $Id: dmpcpr.c,v 1.2 91/04/24 20:52:08 peter Exp $	*/
/*
 * Copper List Disassembler Routine - Scott Evernden 10/14/86
 *
 * I use this routine to help in understanding how the copper is
 * manipulated by gels, audio, display, me, etc.
 *
 * dumpCprList(fil, lis)
 * FILE *fil;
 * struct cprlist *lis;
 *
 *	Dump the contents of the harware copper instruction list
 *	pointed to by `lis' onto file `fil'.  If fil is NULL, stdout
 *	is used.  If lis is NULL, this routine return immediately.
 *	`lis' is usually view->LOFCprList (and/or view->SHFCprList for
 *	interlaced frames).
 *
 *	This routine will continue disassembly until either
 *		1) A WAIT instruction is encountered for pos (0x7F, 0xFF)
 *		2) lis->maxCount instructions have been decoded.
 *
 *	The copper has only 3 opcodes: WAIT, MOVE, and SKIP.  This
 *	routine will also list instructions MOVEBF and SKIPBF, if the
 *	BFD (blitter-finished-disable) bit is reset.
 *
 *	See the "Hardware Manual" for more information on the copper.
 *
 * When using Aztec C compiler, use "cc -z3000 dmpcpr.c" because
 * of all the text strings in here.
 *
 */

#include <stdio.h>
#include "exec/types.h"
#include "graphics/copper.h"

/*
 * These names are derived from `hardware/custom.h'.  Some of
 * these register can't be written to by the copper, but I've
 * included them all, since you might find this list handy for
 * other purposes.
 */
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
	"bplpt[0]","bplpt[0]+2","bplpt[1]","bplpt[1]+2",
	"bplpt[2]","bplpt[2]+2","bplpt[3]","bplpt[3]+2",
	"bplpt[4]","bplpt[4]+2","bplpt[5]","bplpt[5]+2",
	"pad7c[0]","pad7c[1]","pad7c[2]","pad7c[3]",
	"bplcon0","bplcon1","bplcon2","pad83",
	"bpl1mod","bpl2mod","pad86[0]","pad86[1]",
	"bpldat[0]","bpldat[1]","bpldat[2]",
	"bpldat[3]","bpldat[4]","bpldat[5]","pad8e[0]","pad8e[1]",
	"sprpt[0]","sprpt[0]+2","sprpt[1]","sprpt[1]+2",
	"sprpt[2]","sprpt[2]+2","sprpt[3]","sprpt[3]+2",
	"sprpt[4]","sprpt[4]+2","sprpt[5]","sprpt[5]+2",
	"sprpt[6]","sprpt[6]+2","sprpt[7]","sprpt[7]+2",
	"spr[0].pos","spr[0].ctl","spr[0].dataa","spr[0].datab",
	"spr[1].pos","spr[1].ctl","spr[1].dataa","spr[1].datab",
	"spr[2].pos","spr[2].ctl","spr[2].dataa","spr[2].datab",
	"spr[3].pos","spr[3].ctl","spr[3].dataa","spr[3].datab",
	"spr[4].pos","spr[4].ctl","spr[4].dataa","spr[4].datab",
	"spr[5].pos","spr[5].ctl","spr[5].dataa","spr[5].datab",
	"spr[6].pos","spr[6].ctl","spr[6].dataa","spr[6].datab",
	"spr[7].pos","spr[7].ctl","spr[7].dataa","spr[7].datab",
	"color[0]","color[1]","color[2]","color[3]",
	"color[4]","color[5]","color[6]","color[7]",
	"color[8]","color[9]","color[10]","color[11]",
	"color[12]","color[13]","color[14]","color[15]",
	"color[16]","color[17]","color[18]","color[19]",
	"color[20]","color[21]","color[22]","color[23]",
	"color[24]","color[25]","color[26]","color[27]",
	"color[28]","color[29]","color[30]","color[31]"
};

/***********************************************************************/

dumpCprList(fil, lis)
FILE *fil;
struct cprlist *lis;
{
	UWORD *p, op, w;
	SHORT cnt;

	if (!lis)
		return;

	if (!fil)
		fil = stdout;

	cnt = lis->MaxCount;
	p = lis->start;
	while (cnt--) {
		op = *p++;
		w = *p++;
		if (disasm(fil, p - 2, op, w))
			break;
	}
}

static int disasm(fil, p, op, w)
FILE *fil;
UWORD *p, op, w;
{
	UWORD vp, hp, vm, hm, da;
	int r;

	fprintf(fil, "%08lx\t", p);
	if (op & 1) {
		vp = (op >> 8) & 0xFF;
		hp = (op >> 1) & 0x7F;
		vm = (w >> 8) & 0x7F;
		hm = (w >> 1) & 0x7F;
		fprintf(fil, "%s", w & 1 ? "SKIP" : "WAIT");
		if ((w & 0x8000) == 0)
			printf("BF");
		fprintf(fil, "\t(%02x,%02x)", hp, vp);
		if (hm != 0x7F || vm != 0x7F)
			fprintf(fil, "; mask (%02x,%02x)", hm, vm);
		r = !(w & 1) && vp >= 0xFF && hp >= 0x7F;
	}
	else {
		r = 0;
		da = (op >> 1) & 0xFF;
		fprintf(fil, "MOVE\t%04x,%s", w, names[da]);
	}
	fprintf(fil, "\n");
	return r;
}
