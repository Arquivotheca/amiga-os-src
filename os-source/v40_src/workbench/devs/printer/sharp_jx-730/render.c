/*
	Sharp_JX-730 driver.
	David Berezowski - Dec/89.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

/* these should ALWAYS be false (0) for a production release */
#define DEBUG0		0	/* case 0 debugging */
#define DEBUG1		0	/* case 1 debugging */
#define DEBUG2		0	/* case 2 debugging */
#define DEBUG3		0	/* case 3 debugging */
#define DEBUG4		0	/* case 4 debugging */
#define DEBUG5		0	/* case 5 debugging */

#define NOMEM		0	/* do not allocate any memory */
#define RLEDEBUG	0	/* enable debugging of rle code */

/* this should ALWAYS be true (1) for a production release */
#define RLE		1	/* enable RLE code */

#define NUMSTARTCMD	6	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	2	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	16	/* max # of color buffers */

#define RLEMAX		255	/* maximum rle count allowable */
#define RLENUMSTARTCMD	3	/* # of cmd bytes before binary data */
#define RLENUMENDCMD	2	/* # of cmd bytes after binary data */
#define RLENUMTOTALCMD 	(RLENUMSTARTCMD + RLENUMENDCMD)	/* total of above */
/*
    ESC J ct			- 3	; start of rle cmd (with color code)
    0x41 rlecount data		- 3	; rle line follows
    0x42 non-rlecount data	- 3	; non-rle line follows
    0x40 end_of_rle		- 1	; end of rle cmd
    ESC A			- 2	; cr/lf
				---
				 12
*/
#define RLESAFETY		 12	/* extra room for overwrites */

#define INIT_LEN	15	/* length of init (start) buffer */
#define END_LEN		(INIT_LEN-2)	/* length of init (end) buffer */
#define LMARG_POSN	4	/* index into init buf for left margin */
#define	RMARG_POSN	9	/* index into init buf for right margin */

/* (30.5cm/ft) * (10mm/cm)					*/
#define MMPERFT		305

/* Half inches per ft						*/

#define HALFINCHPERFT 24

/* Inches per ft						*/

#define INCHPERFT 12

/* Characters per ft at 10 CPI					*/

#define CHARSPERFT 120

/* MM per 1/2 inch 						*/

#define HALFINCH ((MMPERFT)/HALFINCHPERFT)

/* MM per inch							*/

#define INCH ((MMPERFT)/INCHPERFT)

/* Sparse table for paper sizes - offset constant		*/

#define PSIZE_OFFSET 16


Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	ULONG *ulongptr;
	static UWORD RowSize, ColorSize, BufSize;
	static UWORD colors[MAXCOLORBUFS]; /* color indexes */
	static UWORD color_order[] =
		{0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13, 14, 15};


	static int PaperXSizes[] = {
	/* Paper sizes in inches x 10		*/

		80,			/* US_LETTER */
		80,			/* US_LEGAL  */
		80,			/* N_TRACTOR */
		136,			/* W_TRACTOR */
		80,			/* CUSTOM    */

	/* Euro Ax sizes follow						*/
	/* Paper size in millimeters - 1/2 inch 			*/
	/* Subtract 1/2 inch to be consistent with US size behavior	*/
		841 - HALFINCH,		/* A0	*/
		594 - HALFINCH,		/* A1	*/
		420 - HALFINCH,		/* A2	*/
		297 - HALFINCH,		/* A3	*/
		210 - HALFINCH,		/* A4	*/
		148 - HALFINCH,		/* A5	*/
		105 - HALFINCH,		/* A6	*/
		74  - HALFINCH,		/* A7	*/
		52  - HALFINCH,		/* A8	*/

		};

	/* Calculate max dots based on paper size selection */

	UWORD psize;
        ULONG maxwidth;
	ULONG maxcols;


	static UWORD huns, tens, ones; /* used to program buffer size */
	static UWORD NumColorBufs; /* actually # of color buffers */
	/*
	    00-01    \033N	- enable group N (normal) commands
	    02-06    \033L001	- left margin to 1 (left edge)
	    07-11    \033/999	- right margin to 999 (right edge)
	    12-12    \015	- carriage-return
	    13-14    \033G	- enable group G (graphics) commands
	*/
	static UBYTE init_buf[INIT_LEN] = "\033N\033L001\033/999\015\033G";

	static UBYTE *Buf1 = NULL, *Buf2 = NULL, *BufPtr;
	static UBYTE *RLEBuf1 = NULL, *RLEBuf2 = NULL, *RLEBufPtr;

	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start;
	int i, err;

	static UWORD RLEBufSize;
	UBYTE *rleptrstart, *rleptr, *rleptrmark, *rleptrend, rledata;
	int rlesize, rlecount, j;

    switch(status) {
	case 0 : /* Master Initialization */
	    /*
		ct	- pointer to IODRPReq structure.
		x	- width of printed picture in pixels.
		y	- height of printed picture in pixels.
	    */
#if DEBUG0
	    kprintf("case 0: ioreq=$%lx, width=%ld, height=%ld, PBuf=$%lx\n",
		ct, x, y, PD->pd_PrintBuf);
#endif DEBUG0
#if NOMEM
	    err = PDERR_NOERR;
	    break;
#endif NOMEM
	    /* calc # of bytes of row data */
	    RowSize = (x + 7) / 8;
	    /* size of each color buf */
	    ColorSize = RowSize + NUMTOTALCMD;
	    huns = RowSize / 100;
	    tens = (RowSize - huns * 100) / 10;
	    ones = RowSize % 10;
	    if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
		NumColorBufs = MAXCOLORBUFS;
	    }
	    else {
		NumColorBufs = 4;
	    }
	    BufSize = ((((ColorSize * NumColorBufs + NUMLFCMD)/4)+1)*4);
	    RLEBufSize = BufSize + RLESAFETY;
	    /*
		My color order:			B, Y, M, C
		Sharp's color order:		B, M, Y, C
	    */
	    for (i=0; i<NumColorBufs; i++) {
		colors[color_order[i]] = ColorSize * i;
	    }
	    err = PDERR_BUFFERMEMORY; /* assume no mem */
	    if (Buf1 = AllocMem(BufSize, NULL)) {
#if DEBUG0
		kprintf("0: allocated %ld bytes @$%lx for Buf1\n",
		    BufSize, Buf1);
#endif DEBUG0
		if (Buf2 = AllocMem(BufSize, NULL)) {
#if DEBUG0
		    kprintf("0: allocated %ld bytes @$%lx for Buf2\n",
			BufSize, Buf2);
#endif DEBUG0
		    if (RLEBuf1 = AllocMem(RLEBufSize, NULL)) {
#if DEBUG0
			kprintf("0: allocated %ld bytes @$%lx for RLEBuf1\n",
			    RLEBufSize, RLEBuf1);
#endif DEBUG0
			if (RLEBuf2 = AllocMem(RLEBufSize, NULL)) {
#if DEBUG0
			    kprintf("0: allocated %ld bytes @$%lx for RLEBuf2\n", RLEBufSize, RLEBuf2);
#endif DEBUG0
			    PD->pd_PrintBuf = Buf1;
			    BufPtr = Buf1;
			    RLEBufPtr = RLEBuf1;
			    /*
			    This printer prints graphics within its
			    text margins.  This code sets the
			    left and right margins to their minimum
			    and maximum values (respectively).  A
			    carriage return is sent so that the
			    print head is at the leftmost position
			    as this printer starts printing from
			    the position of the print head.
			    */
			    /* clear margins and select gfx mode */
			    init_buf[LMARG_POSN] = '0';
			    init_buf[LMARG_POSN+1] = '0';
			    init_buf[LMARG_POSN+2] = '1'; /* min lmarg */
			    init_buf[RMARG_POSN] = '9';
			    init_buf[RMARG_POSN+1] = '9';
			    init_buf[RMARG_POSN+2] = '9'; /* max rmarg */
			    err = (*(PD->pd_PWrite))(init_buf, INIT_LEN);
			}
		    }
		}
	    }
	    break;

	case 1 : /* Scale, Dither and Render */
	    /*
		ct	- pointer to PrtInfo structure.
		x	- 0.
		y	- row # (0 to Height - 1).
	    */
#if DEBUG1
	    kprintf("case 1: PrtInfo=$%lx, x=%ld, row=%ld\n",
		ct, x, y);
#endif DEBUG1
#if NOMEM
	    err = PDERR_NOERR;
	    break;
#endif NOMEM
	    Transfer(ct, y, BufPtr + NUMSTARTCMD, colors);
	    err = PDERR_NOERR; /* all ok */
	    break;

	case 2 : /* Dump Buffer to Printer */
	    /*
		ct	- 0.
		x	- 0.
		y	- # of rows sent (1 to NumRows).
	    */
#if DEBUG2
	    kprintf("case 2: ct=%ld, x=%ld, rows sent=%ld\n",
		ct, x, y);
#endif DEBUG2
#if NOMEM
	    err = PDERR_NOERR;
	    break;
#endif NOMEM
#if RLE
	    /* Run-Length Encode (rle) the data */
	    rleptrstart = rleptr = RLEBufPtr;
	    rleptrend = rleptrstart + RLEBufSize - RLESAFETY - 1;
	    /* ptr to data */
	    ptrstart = BufPtr + NUMSTARTCMD;
	    for (ct=0; ct<NumColorBufs;	ct++, ptrstart += ColorSize) {
		/* save start posn for this color */
		rleptrmark = rleptr;
#if RLEDEBUG
		CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr1");
#endif RLEDEBUG
		*rleptr++ = 27; /* rle start cmd */
#if RLEDEBUG
		CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr2");
#endif RLEDEBUG
		*rleptr++ = 'J';
#if RLEDEBUG
		CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr3");
#endif RLEDEBUG
		*rleptr++ = ct | '0'; /* color code */
		ptr = ptrstart; /* get ptr to bytes to rle */
		j = RowSize - 1; /* # of bytes left to rle */
		do {
		    /* first do repeating bytes */
		    /* get goal (repeating) byte */
		    rledata = *ptr++;
		    /* this many repetitions left to go*/
		    i = RLEMAX - 1;
		    /* while repeating and not too many and more to do */
		    while (*ptr == rledata && i > 0 && j > 0) {
			i--; /* one more rle byte */
			/* advance ptr to next byte */
			ptr++;
			/* one less byte to look at */
			j--;
		    }
		    /* calc repeating byte count */
		    if ((rlecount = RLEMAX - i) == 1) {
			/* if only 1 then no repeat */
			rlecount = 0;
		    }
		    else {
			/* dont forget the goal byte */
			j--;
		    }
		    /* if there was repeat data */
		    if (rlecount != 0) {
#if RLEDEBUG
			CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr4");
#endif RLEDEBUG
			/* repeating data follows */
			*rleptr++ = 0x41;
#if RLEDEBUG
			CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr5");
#endif RLEDEBUG
			/* save repeat count */
			*rleptr++ = rlecount;
#if RLEDEBUG
			CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr6");
#endif RLEDEBUG
			/* save repeat byte */
			*rleptr++ = rledata;
			/* get non-repeat goal byte */
			rledata = *ptr++;
		    }
		    /* now do non-repeating data */
		    /* no non-repeating bytes yet */
		    rlecount = 0;
		    if (*ptr != rledata && j >= 0) {
#if RLEDEBUG
			CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr7");
#endif RLEDEBUG
			/* non-repeat data follows */
			*rleptr++ = 0x42;
			/* repeat count (filled in later) */
			rleptr++;
			/* this many non-repetitions
			   left to go */
			i = RLEMAX - 1;
		    }
		    /* while non-repeating and not too many and more to do */
		    while (*ptr != rledata && i > 0 && j >= 0) {
#if RLEDEBUG
			CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr8");
#endif RLEDEBUG
			/* save byte */
			*rleptr++ = rledata;
			/* one more non-repeat byte */
			rlecount++;
			/* get goal byte */
			rledata = *ptr++;
			/* one less byte to look at */
			j--;
			/* one more non-rle byte */
			i--;
			/* abort if danger of overrun */
			if (rleptr >= rleptrend) goto rletoobig;
		    }
		    if (rlecount != 0) {
#if RLEDEBUG
			CheckRange(rleptr - rlecount - 1, RLEBufPtr,
			    RLEBufSize, "rleptr - rlecount - 1");
#endif RLEDEBUG
			*(rleptr - rlecount - 1) = rlecount;
		    }
		    if (j > 0) { /* if more data to do */
			/* set ptr back to start of repeat bytes */
			ptr--;
		    }
		    /* abort if danger of overrun */
		    if (rleptr >= rleptrend) goto rletoobig;
		} while (j > 0); /* while more bytes to rle */

		/* if didnt abort && no non-repeating data */
		if (j < 1 && rlecount == 0) {
		    /* check for trailing white space */
		    /* line ends in trailing 0 */
		    if (*(rleptr - 1) == 0x00) {
			/* ptr back to delimiter code (0x41) */
			rleptr -= 3;
		    }
		}
		/* if line is just the cmd bytes */
		/* line null */
		if (rleptr - rleptrmark == RLENUMSTARTCMD) {
		    /* reset ptr to start */
		    rleptr = rleptrmark;
		}
		else {
#if RLEDEBUG
		    CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr9");
#endif RLEDEBUG
		    *rleptr++ = 0x40; /* end of rle cmd */
		}
	    }
	    i = rleptr - rleptrstart; /* calc size of rlebuf */
	    /* if rle data is more than non-rle data, send non-rle data */
	    if (i > BufSize) {
rletoobig:
		ptrstart = BufPtr;
		ptr = ptrstart + BufSize - NUMLFCMD;
#if RLEDEBUG
		CheckRange(ptr, BufPtr, BufSize, "ptr");
#endif RLEDEBUG
		*ptr++ = 27;
#if RLEDEBUG
		CheckRange(ptr, BufPtr, BufSize, "ptr");
#endif RLEDEBUG
		*ptr++ = 'A';		/* cr/lf */
		err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
	    }
	    else { /* send rle data */
#if RLEDEBUG
		CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr10");
#endif RLEDEBUG
		*rleptr++ = 27;
#if RLEDEBUG
		CheckRange(rleptr, RLEBufPtr, RLEBufSize, "rleptr11");
#endif RLEDEBUG
		*rleptr++ = 'A';	/* cr/lf */
		i = rleptr - rleptrstart; /* size of rlebuf */
		err = (*(PD->pd_PWrite))(rleptrstart, i);
	    }
#else
	    ptrstart = BufPtr;
	    ptr = ptrstart + BufSize - NUMLFCMD;
	    *ptr++ = 27;
	    *ptr++ = 'A';		/* cr/lf */
	    err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
#endif RLE
	    if (err == PDERR_NOERR) {
		BufPtr = (BufPtr == Buf1) ? Buf2 : Buf1;
		RLEBufPtr = (RLEBufPtr == RLEBuf1) ? RLEBuf2 : RLEBuf1;
	    }
	    break;

	case 3 : /* Clear and Init Buffer */
	    /*
		ct	- 0.
		x	- 0.
		y	- 0.
	    */
#if DEBUG3
	    kprintf("case 3: ct=%ld, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG3
#if NOMEM
	    err = PDERR_NOERR;
	    break;
#endif NOMEM
	    ulongptr = (ULONG *)BufPtr;
	    i = (BufSize + 3) / 4;
	    do {
		*ulongptr++ = 0;
	    } while (--i);
	    for (ct = 0, ptr = BufPtr, i = ColorSize - NUMSTARTCMD;
		ct < NumColorBufs; ct++, ptr += i) {
		*ptr++ = 27;
		*ptr++ = 'I';
		*ptr++ = ct + '0';	/* color */
		*ptr++ = huns | '0';
		*ptr++ = tens | '0';
		*ptr++ = ones | '0';	/* printout width */
	    }
	    err = PDERR_NOERR; /* all ok */
	    break;

	case 4 : /* Close Down */
	    /*
		ct	- error code.
		x	- io_Special flag from IODRPReq.
		y	- 0.
	    */
#if DEBUG4
	    kprintf("case 4: err=%ld, special=$%lx, y=%ld\n", ct, x, y);
#endif DEBUG4
#if NOMEM
	    err = PDERR_NOERR;
	    break;
#endif NOMEM
	    /* if user did not cancel print */
	    if (ct != PDERR_CANCEL) {
		/* restore preferences pitch and margins */
		i = PD->pd_Preferences.PrintLeftMargin;
		init_buf[LMARG_POSN] = (i / 100) | '0';
		init_buf[LMARG_POSN+1] = ((i / 10) % 10) | '0';
		init_buf[LMARG_POSN+2] = (i % 10) | '0';
		if ((i = PD->pd_Preferences.PrintRightMargin) < 999) i++;
		init_buf[RMARG_POSN] = (i / 100) | '0';
		init_buf[RMARG_POSN+1] = ((i / 10) % 10) | '0';
		init_buf[RMARG_POSN+2] = (i % 10) | '0';
		err = (*(PD->pd_PWrite))(init_buf, END_LEN);
	    }
	    else {
		err = PDERR_NOERR; /* all ok */
	    }
	    /* wait for both buffers to empty */
	    (*(PD->pd_PBothReady))();
	    /* free all mem used */
	    if (RLEBuf2) {
#if DEBUG4
		kprintf("4: freeing %ld bytes @$%lx for RLEBuf2\n",
			RLEBufSize, RLEBuf2);
#endif DEBUG4
		FreeMem(RLEBuf2, RLEBufSize);
	    }
	    if (RLEBuf1) {
#if DEBUG4
		kprintf("4: freeing %ld bytes @$%lx for RLEBuf1\n",
		    RLEBufSize, RLEBuf1);
#endif DEBUG4
		FreeMem(RLEBuf1, RLEBufSize);
	    }
	    if (Buf2) {
#if DEBUG4
		kprintf("4: freeing %ld bytes @$%lx for Buf2\n",
		    BufSize, Buf2);
#endif DEBUG4
		FreeMem(Buf2, BufSize);
	    }
	    if (Buf1) {
#if DEBUG4
		kprintf("4: freeing %ld bytes @$%lx for Buf1\n",
		    BufSize, Buf1);
#endif DEBUG4
		FreeMem(Buf1, BufSize);
	    }
	    break;

	case 5 : /* Pre-Master Initialization */
	    /*
		ct	- 0 or pointer to IODRPReq structure.
		x	- io_Special flag from IODRPReq.
		y	- 0.
	    */
#if DEBUG5
	    kprintf("case 5: req=$%lx, special=$%lx, y=%ld\n", ct, x, y);
#endif DEBUG5

		/* reference array (constant steps of 16) */
		/* Any new in-between sizes will be rounded down */

		psize = PD->pd_Preferences.PaperSize;

		maxwidth =  (ULONG)PaperXSizes[psize/PSIZE_OFFSET];

#if DEBUG5
	kprintf("psize = %ld  maxwidth = %ld\n",(long)psize,(long)maxwidth);
#endif DEBUG5

	if(psize <= CUSTOM)
	{
		PED->ped_MaxColumns = maxwidth;
		PED->ped_MaxXDots = (PED->ped_XDotsInch * maxwidth) / 10;
	}
	else
	{
		PED->ped_MaxXDots =
			(maxwidth * INCHPERFT *	PED->ped_XDotsInch)/MMPERFT;

		maxcols = ((maxwidth * CHARSPERFT)+(MMPERFT - 1))/MMPERFT;

		/* A0 sizes can overflow UBYTE MaxColumns.		*/

		PED->ped_MaxColumns = maxcols <= 136 ? maxcols : 136;
	}
		
#if DEBUG5
	kprintf("ped_MaxXDots = %ld ped_MaxColumns = %ld\n",
		(long)PED->ped_MaxXDots, (long)PED->ped_MaxColumns);
#endif DEBUG5

	    err = PDERR_NOERR; /* all ok */
	    break;
	}
	return(err);
}

#if RLEDEBUG
CheckRange(ptr, start, size, name)
UBYTE *ptr;
UBYTE *start;
ULONG size;
char *name;
{
    if ((ptr < start) || (ptr > (start + size - 1))) {
	kprintf("\nCR: ERROR for '%s'\n", name);
	kprintf("\tptr=$%lx, start=$%lx, size=$%lx, end=$%lx\n",
	    ptr, start, size, start + size - 1);
	kprintf("\tptr=%ld, start=%ld, size=%ld, end=%ld\n",
	    ptr, start, size, start + size - 1);
	Debug(0L);
    }
}
#endif RLEDEBUG
