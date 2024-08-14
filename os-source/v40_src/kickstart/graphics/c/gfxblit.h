/******************************************************************************
*
*	$Id: gfxblit.h,v 39.0 91/08/21 17:18:55 chrisg Exp $
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


