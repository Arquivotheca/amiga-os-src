/******************************************************************************
*
*	$Id: gfxblit.h,v 42.0 93/06/16 11:17:46 chrisg Exp $
*
******************************************************************************/

struct gfxbltnd
{
	short	bltsize;
	short	fillbytes;
	UWORD	fwmask,lwmask;
	short	bltcon1;
	short	bltmdb,bltmda,bltmdc;
	/*short	bltmdc,bltmdb,bltmda;*/
	char	*bltpta;
	short	*bltptb;
	short	jstop;	/* can be used for other things */
	char	filln;
	char	j;
	UWORD	bltcon0;
	short	lminy,lmaxy,bpw;
	long  bltptr;
	short	fudge;
	short	rx;
	/* additions for new chips */
	short	blitrows;
	short	blitwords;
};


