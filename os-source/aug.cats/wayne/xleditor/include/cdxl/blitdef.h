/************

	BlitDef.h

	W.D.L 920509

*************/

#ifndef	BLITDEF_H	// [
#define	BLITDEF_H

typedef	struct	blitter_def {
	UWORD	bltcon0;
	UWORD	bltcon1;
	UWORD	bltcpt;
	UWORD	bltbpt;
	UWORD	bltapt;
	UWORD	bltdpt;
	UWORD	bltafwm;
	UWORD	bltalwm;
	UWORD	bltcmod;
	UWORD	bltbmod;
	UWORD	bltamod;
	UWORD	bltdmod;
	UWORD	bltcdat;
	UWORD	bltbdat;
	UWORD	bltadat;
	UWORD	bltsize;
	ULONG	srceoff;
	ULONG	destoff;
	UWORD	TotalClip;

} BLITDEF;

#define	BDEF_CONTIGIOUS	0x00000001

#endif				// ]
