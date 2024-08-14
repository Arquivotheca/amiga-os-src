/*  :ts=8  bk=0
 * If you liked "Boing!", there's a better than even chance you'll like this.
 *
 * Leo L. Schwab		8608.6
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/sprite.h>

#define REV		0L
#define FRAMES		6
#define SPRHEIGHT	16
#define WORDSPERSPR	(2 * SPRHEIGHT + 4)
#define MAXX		(640-32)
#define MAXY		(200-16)
#define MAGIC1		33
#define MAGIC2		21
#define ever		(;;)

extern UWORD	ballmask[], ball0[];
extern void	*OpenLibrary(), *OpenWindow(), *AllocMem(), *GetMsg(),
		*ViewPortAddress();
extern long	GetSprite(), VBeamPos();


struct NewWindow windef = {
	0, 15, 300, 10,
	-1, -1,
	CLOSEWINDOW,
	WINDOWCLOSE | WINDOWDEPTH | WINDOWDRAG | ACTIVATE,
	NULL, NULL,
	(UBYTE *) "One moment....",
	NULL, NULL, 0, 0, 0, 0,
	WBENCHSCREEN
};
/*	Doing it this way is faster than saying  (i+offset) % 6  */
UBYTE		idx[] = { 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4 };

struct SimpleSprite	spr[FRAMES];
struct Window	*win;
struct ViewPort	*vp;
UWORD		*sprbuf;
UWORD		*sprites[FRAMES];
int		vx[FRAMES], vy[FRAMES];
void		*GfxBase, *IntuitionBase;


main ()
{
	int i, offset = 0, flag = 0;
	void *msg;

	openstuff ();
	setupsprites ();
	rnd ((short) -VBeamPos());	/*  Plant a seed  */

	for (i=0; i<FRAMES; i++) {
		if (GetSprite (&spr[i], (long) i+2) < 0)
			die ("Sprite allocation failed.");
		spr[i].x = 640 / 2;
		spr[i].y = 10;
		spr[i].height = SPRHEIGHT;
		ChangeSprite (vp, &spr[i], sprites[i]);
	}

	SetWindowTitles (win, "  \253\253<< Oing! >>\273\273  ", "Oing!");
	for ever {
		/*
		 * WaitTOF()'s presence way up here is significant.  I used
		 * have it down by the ChangeSprite() loop, but it flickered.
		 * When it's up here, I get just the right amount of delay
		 * after top-of-frame so that nothing glitches.
		 */
		WaitTOF ();
		if (msg = GetMsg (win -> UserPort)) {
			ReplyMsg (msg);
			closestuff ();
			return;
		}
		for (i=0; i<FRAMES; i++) {
			spr[i].x += vx[i];
			if (spr[i].x > MAXX) {
				/*  spr.x is unsigned, so we have to cheat  */
				spr[i].x = (spr[i].x & 0x8000) ? 0 : MAXX;
				vx[i] = -vx[i];
			}

			/* The shift by 2 is to slow vertical motion a bit */
			if ((spr[i].y += (vy[i] >> 2)) > MAXY) {
				spr[i].y = MAXY;
				vx[i] = rnd (MAGIC2) - MAGIC2 / 2;
				vy[i] = -rnd (MAGIC1) - 5;
			}
			vy[i]++;	/*  Gravity  */
		}

		for (i=0; i<FRAMES; i++) 
			ChangeSprite (vp, &spr[i], sprites[idx[i+offset]]);

		/*  Rotate balls every other loop  */
		if (flag = !flag)
			offset = (offset + 1) % 6;
	}
}

openstuff ()
{
	register int i;

	if (!(IntuitionBase = OpenLibrary ("intuition.library", REV)))
		die ("Intuition open failed.");

	if (!(GfxBase = OpenLibrary ("graphics.library", REV)))
		die ("Art shop closed.");

	if (!(win = OpenWindow (&windef)))
		die ("Window painted shut.");
	vp = ViewPortAddress (win);

	for (i=20; i<32; i += 4) {
		SetRGB4 (vp, i+1L, 15L, 0L, 0L);
		SetRGB4 (vp, i+3L, 15L, 14L, 13L);
	}
}

closestuff ()
{
	register int i;

	for (i=0; i<FRAMES; i++)
		if (spr[i].num)
			FreeSprite ((long) spr[i].num);

	if (sprbuf)
		FreeMem (sprbuf, 2L * WORDSPERSPR * FRAMES);
	if (win)
		CloseWindow (win);
	if (GfxBase)
		CloseLibrary (GfxBase);
	if (IntuitionBase)
		CloseLibrary (IntuitionBase);
}

die (str)
char *str;
{
	if (win) {
		SetWindowTitles (win, str, -1L);
		WaitPort (win -> UserPort);
	} else
		puts (str);
	closestuff ();
	exit (100);
}

/*
 * The following code segment was lifted (nearly) intact from the Alpha-9
 * 1.2 README disk.  Thanks to Jim Mackraz for the code, and also for
 * drawing all those ball sprites.
 */
setupsprites ()
{
	UWORD *cw;	/* current position in buffer of sprite images */
	UWORD *maskp;	/* current position in ballmask             */
	UWORD *ballp;	/* current position in ball0 data           */
	int frame, scan;

	if (!(sprbuf = AllocMem (2L * WORDSPERSPR * FRAMES, MEMF_CHIP)))
		die ("Can't allocate sprite buffer.");

	cw = sprbuf;	/* current position at top of buffer */
	ballp = ball0;	/* ... top of data to be interleaved */

	for (frame=0; frame<FRAMES; frame++) {
		maskp = ballmask;	/* one mask for all frames */
		sprites[frame] = cw;
		*cw++ = 0;		/* poscntl */
		*cw++ = 0;

		/* one word from ball0, one word from ballmask */
		for (scan=0; scan<SPRHEIGHT; scan++) {
			*cw++ = *maskp++;
			*cw++ = *ballp++;
		}
		*cw++ = 0;	/* termination */
		*cw++ = 0;
	}
}
