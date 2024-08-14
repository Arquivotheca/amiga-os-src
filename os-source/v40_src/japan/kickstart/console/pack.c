#include	"exec/types.h"
#include	"conmap.h"

#define PDEBUG	0	/* pack debug   */
#define PCDEBUG 0	/* pack debug - character info */
#define UDEBUG	0	/* unpack debug */

void packSorted(struct ConsoleMap *,ULONG,ULONG,ULONG,ULONG,ULONG);
unpackSortedLine(struct ConsoleMap *,UWORD,UWORD *,UWORD *,UWORD *,UWORD *,UWORD *);

void packSorted(cm, displayXL, displayYL, cpX, cpY, lastAttr)
struct ConsoleMap *cm;
ULONG displayXL, displayYL;
ULONG cpX, cpY;
ULONG lastAttr;
{
    UBYTE character, *dispC, *bufC, *ssC, *sdC;
    WORD attr, *dispA, *bufA, *ssA, *sdA, implicitNL;
    short limitX, limitCPX, limitY, sx, sy, x, y;

    UWORD saveAttr;
    saveAttr=(lastAttr & CMAM_BGPEN);

    limitY = (displayYL > cpY) ? displayYL : cpY;

#if PDEBUG
	kprintf("packSorted routine\n");

	kprintf("limitY=%ld displayYL=%ld cpY=%ld\n",
		(long)limitY,(long)displayYL,(long)cpY);
#endif

    for (y = 0; y <= limitY; y++) {

#if PDEBUG
	kprintf("y=%ld\n",(long)y);
#endif

	/* find destination */
	bufC = (UBYTE *) (cm->cm_BufferStart +
		(cm->cm_BufferWidth * cm->cm_BufferYL) + cm->cm_BufferXL);
	bufA = (WORD *) (((LONG) bufC) << 1);
	bufC += cm->cm_AttrToChar;

#if PDEBUG
	kprintf("   bufA=%lx  bufC=%lx\n",(long)bufA,(long)bufC);
#endif

	if (y <= displayYL) {
	    /* find source */
	    dispC = (UBYTE *) (cm->cm_DisplayStart + (cm->cm_DisplayWidth * y));
	    dispA = (WORD *) (((LONG) dispC) << 1);
	    dispC += cm->cm_AttrToChar;
	    implicitNL = *dispA & CMAF_IMPLICITNL;

#if PDEBUG
	kprintf("      dispA=%lx dispC=%lx implicitNL=%lx\n",
		(long)dispA,(long)dispC,(long)implicitNL);
#endif

	    /* ensure no overlap with destination */
	    if (((long) bufC) + cm->cm_BufferWidth > ((long) dispC)) {

#if PDEBUG
	kprintf("         Slow scroll required\n");
#endif

		/* scroll buffer (painfully) */
		sdC = (UBYTE *) cm->cm_BufferStart;
		sdA = (WORD *) (((LONG) sdC) << 1);
		sdC += cm->cm_AttrToChar;
		ssA = sdA + cm->cm_BufferWidth;
		ssC = sdC + cm->cm_BufferWidth;
		for (sy = cm->cm_BufferYL; sy > 0; sy--) {
		    for (sx = cm->cm_BufferWidth; sx > 0; sx--) {
			*sdA++ = *ssA++;
			*sdC++ = *ssC++;
		    }
		}
		bufA -= cm->cm_BufferWidth;
		bufC -= cm->cm_BufferWidth;
		cm->cm_BufferYL--;
	    }

	    /* find source width */
	    if (y == displayYL)
		limitX = displayXL;
	    else
		limitX = cm->cm_DisplayWidth;
	}
	else {
	    limitX = 0;
	    implicitNL = 0;
	}

#if PDEBUG
	kprintf("   limitX=%ld implicitNL=%lx\n",
		(long)limitX,(long)implicitNL);

	kprintf("   y=%ld displayYL=%ld BufferYL=%ld BufferXL=%ld\n",
		(long)y,(long)displayYL,
		(long)cm->cm_BufferYL,(long)cm->cm_BufferXL);

#endif

	/* check for empty source or explicit newline */
	if (((y > displayYL) || (!implicitNL)) &&
		/* but not empty buffer */
		((cm->cm_BufferYL != 0) || (cm->cm_BufferXL != 0))) {
	    /* clear the rest of this line in the buffer */

#if PDEBUG
	kprintf("      Clear the rest of line in buffer\n");
#endif

	    for (x = cm->cm_BufferXL; x < cm->cm_BufferWidth; x++) {
		*bufA++ = 0;
		*bufC++;
	    }
	    cm->cm_BufferXL = 0;
	    cm->cm_BufferYL++;
	}

	/* calculate limit with CP in mind */
	limitCPX = limitX;
	if ((y == cpY) && (limitX <= cpX))
	    limitCPX = cpX + 1;

#if PDEBUG
	kprintf("   limitCPX=%ld\n",(long)limitCPX);
#endif

	/* copy source to destination */
	for (x = 0; x < limitCPX; x++) {
	    if (cm->cm_BufferXL == cm->cm_BufferWidth) {
#if PDEBUG
	kprintf("      Buffer line wrapped\n");
#endif
		/* buffer line wrap has occurred */
		implicitNL = CMAF_IMPLICITNL;
		cm->cm_BufferXL = 0;
		cm->cm_BufferYL++;
	    }
	    if (x < limitX) {
		attr = *dispA++;
		character = *dispC++;
	    }
	    else
		attr = 0;
	    if (attr >= 0) {
		if ((y != cpY) || (x > cpX))
		    /* no more rendered on this line */
		    break;
		attr = lastAttr;
		character = ' ';
	    }
	    else
		lastAttr = attr;

#if PCDEBUG
	kprintf("   character=%ld attr=%ld lastAttr=%ld\n",
		(long)character,(long)attr,(long)lastAttr);

	kprintf("   y=%ld cpY=%ld x=%ld cpX=%ld\n",
		(long)y,(long)cpY,(long)x,(long)cpX);
#endif

	    if ((y == cpY) && (x == cpX))
	    {
		*bufA++ = (attr & ~(CMAF_HIGHLIGHT | CMAF_SELECTED))
			| implicitNL | CMAF_CURSOR;

		/* special case - cursor at col 0, last line of display 
		   such as a NEWLINE(s) but no text printed yet.

		   - if we
		   don't do this the previous attr can have the IMPLICITNL bit
		   which will be bogus when unpacking the map.

		   Also some code to fix a special case when the colors of the
		   last character gets carried forward as bogus.


		*/

		if(y==limitY)
		{
			if(x==(limitCPX - 1))	/* outside of text - explicit NL */
			{
				if(x == limitX)
				{
					*(bufA - 1) &= ~(CMAM_FGPEN|CMAM_BGPEN);
					*(bufA - 1) |= (saveAttr | (saveAttr >> CMAS_BGPEN));

					if(x==0)
					{
						*(bufA - 1) &= ~CMAF_IMPLICITNL;
					}
				}
			}
		}
	    }
	    else
		*bufA++ = (attr & ~(CMAF_HIGHLIGHT | CMAF_SELECTED))
			| implicitNL;
	    *bufC++ = character;
	    cm->cm_BufferXL++;
	    implicitNL = CMAF_IMPLICITNL;
	}
    }
}


int
unpackSortedLine(cm, y, displayXL, displayYL, cursorMask, cpX, cpY)
struct ConsoleMap *cm;
UWORD y;
UWORD *displayXL, *displayYL;
UWORD *cursorMask;
UWORD *cpX, *cpY;
{
    UBYTE *dispC, *bufC;
    WORD attr, *dispA, *bufA;
    short sizeX, x, i;
    long bufOffset;

#if UDEBUG
	kprintf("unpackSortedLine routine\n");
	kprintf("BufferYL=%ld BufferXL=%ld\n",
		(long)cm->cm_BufferYL,(long)cm->cm_BufferXL);
#endif

    /* test for degenerate case */
    if ((cm->cm_BufferYL == 0) && (cm->cm_BufferXL == 0))
	return(1);


    /* find source */
    bufC = (UBYTE *) (cm->cm_BufferStart +
	    (cm->cm_BufferWidth * cm->cm_BufferYL));
    bufA = (WORD *) (((LONG) bufC) << 1);
    bufC += cm->cm_AttrToChar;
    bufOffset = cm->cm_BufferXL;

    i = cm->cm_BufferYL;

#if UDEBUG
	kprintf("   bufA=%lx bufC=%lx bufOffset=%lx i=%ld\n",
		(long)bufA,(long)bufC,(long)bufOffset,(long)i);
#endif

    while ((i > 0) && ((*bufA & (CMAF_RENDERED | CMAF_IMPLICITNL)) ==
	    (CMAF_RENDERED | CMAF_IMPLICITNL))) {
	/* roll back to non-implicit beginning of buffer */
	bufOffset += cm->cm_BufferWidth;
	bufA -= cm->cm_BufferWidth;
	bufC -= cm->cm_BufferWidth;
	i--;
    }

    /* recover source end to be used for this display line */
    if (bufOffset != 0) {
	sizeX = bufOffset % cm->cm_DisplayWidth;
	if (sizeX == 0)
	    sizeX = cm->cm_DisplayWidth;
#if UDEBUG
	kprintf("bufOffset ! 0  sizeX=%ld\n",(long)sizeX);
#endif

    }
    else
	sizeX = 0;
    bufA += bufOffset;
    bufC += bufOffset;

    /* find destination end */
    dispC = (UBYTE *) (cm->cm_DisplayStart + (cm->cm_DisplayWidth * y) + 
	    cm->cm_DisplayWidth);
    dispA = (WORD *) (((LONG) dispC) << 1);
    dispC += cm->cm_AttrToChar;

#if UDEBUG
	kprintf("   sizeX=%ld bufA=%lx bufC=%lx dispA=%lx dispC=%lx\n",
		(long)sizeX,(long)bufA,(long)bufC,(long)dispA,(long)dispC);

#endif

    /* clear unused destination end */
    for (x = cm->cm_DisplayWidth; x > sizeX; x--) {
	*--dispA = 0;
	--dispC;
    }

    if (y == *displayYL)
	*displayXL = x;

    /* copy from buffer */
    for (; x > 0; x--) {
	attr = *--bufA;
	if (attr & *cursorMask) {
	    *cpX = x - 1;
	    *cpY = y;
	    attr &= ~CMAF_CURSOR;
	    *cursorMask = 0;
	    /* bufA[1] = attr; */
	}
	*--dispA = attr;
	*--dispC = *--bufC;
    }

#if UDEBUG
	kprintf("   BufferXL=%ld BufferYL=%ld sizeX=%ld\n",
		(long)cm->cm_BufferXL,(long)cm->cm_BufferYL,(long)sizeX);
#endif

    /* find new BufferX */
    cm->cm_BufferXL -= sizeX;
    if (cm->cm_BufferXL == 0) {
	if (cm->cm_BufferYL != 0) {
	    /* look for cm_BufferX start on prior line */
	    cm->cm_BufferXL = cm->cm_BufferWidth;
	    while ((((WORD) cm->cm_BufferXL) > 0) && (*--bufA >= 0))
		cm->cm_BufferXL--;
	    cm->cm_BufferYL -= 1;
	}
    }
    else if (((WORD) cm->cm_BufferXL) < 0) {
	/* this can only happen when grabbing buffer data thru implicit NL */
	cm->cm_BufferXL += cm->cm_BufferWidth;
	cm->cm_BufferYL -= 1;
    }

#if UDEBUG
	kprintf("   BufferXL=%ld BufferYL=%ld\n",
		(long)cm->cm_BufferXL,(long)cm->cm_BufferYL);
#endif
    return(0);
}
