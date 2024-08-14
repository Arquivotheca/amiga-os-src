/*** deskjet/render.c *************************************************
 *
 * render.c -- graphics rendering
 *
 *	$Id: render.c,v 1.4 91/07/01 13:03:50 darren Exp $
 *
 *	Copyright (c) 1987,1991 Commodore-Amiga, Inc.
 *	All Rights Reserved
 *
 **********************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <string.h>
#include <clib/exec_protos.h>

#define NUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)	/* total of above */

extern SetDensity();
/*
	0-4		\033&l0L	perf skip mode off
	5-11	\033*t075R	set raster graphics resolution (dpi)
	12-16	\033*b1M	set transmission mode (rle data)
	17-22   \033*r-1U	set # of color planes
	23-30   \033*r####S	set graphics width
	31-35	\033*r0A	start raster gfx
*/
#define START_LEN 36
#define END_LEN   11
#define SHING_LEN 10

#define RLE_ON 15
#define CMYPALETTE 21

char StartCmd[START_LEN] =    { 27, '&','l','0','L',
		     		27, '*','t','0','7','5','R',
		     		27, '*','b','0','M',
		     		27, '*','r','-','1','U',
		     		27, '*','r','1','4','4','0','S',
		     		27, '*','r','0','A'};

char EndGraphics[END_LEN] = {27, '*','r','b','C',27,'*','&',108,'1','L'};

char ShingDepl[SHING_LEN] = {27,'*','o',0,'Q',27,'*','o',1,'D'};

#define SHINGLING 3
#define DEPLETION 8

/* From paintjet driver */
#define NUMLFCMD	5	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	4	/* max # of color buffers */

#define RLEMAX		256
#define RLENUMSTARTCMD	9	/* # of cmd bytes before binary data */
#define RLENUMENDCMD	0	/* # of cmd bytes after binary data */
#define RLENUMTOTALCMD 	(RLENUMSTARTCMD + RLENUMENDCMD)	/* total of above */
#define RLENUMLFCMD	5	/* # of cmd bytes for linefeed */
				/* extra room required for overwrites */
#define RLESAFETY	(RLENUMTOTALCMD + RLENUMLFCMD + 2)
#define	RLE		0	/* last data was sent via rle */


static UWORD NumColorBufs; /* actually # of color buffers */
/**/

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD ColorSize, RowSize, BufSize, TotalBufSize, dataoffset;
	static UWORD thous, huns, tens, ones; /* used to program buffer size */
	UBYTE *ptr, *ptrstart;
	int i, err;
	/* from paintjet driver */
	static UWORD colors[MAXCOLORBUFS]; /* color indexes */
	static UBYTE Mode; /* print mode, rle or non-rle */

	static UWORD RLEBufSize, rledataoffset;
	static UBYTE *RLEBuf;
	UBYTE *rleptrstart, *rleptr, *rleptrmark, rledata;
	int rlecount, j;
	int gocolor = 0;

	/* set to NOERR for cases which do not set err */

	err=PDERR_NOERR;

	if (PD->pd_Preferences.PrintShade == SHADE_COLOR)
		gocolor = 1;

	switch(status) {
		case 0 : /* Master Initialization */
			/* Set Shingling and Depletion according to density:
			 *
			 * 1-3: No depeletion, no shingling
			 *   4: 25% depletion, no shingling
			 *   5: 25% depletion, 50% shingling
			 *   6: 25% depletion, 25% shingling
			 *   7: No depletion, 25% shingling
			 *
			 * Case 7 is ment for transparancies.
			 *
			 * Depletion is ignored in B&W mode
			 */
			switch (PD->pd_Preferences.PrintDensity) {
				case 4:	/* standard */
					ShingDepl[DEPLETION] = '2';
					ShingDepl[SHINGLING] = '0';
					break;
				case 5:	/* NORMAL */
					ShingDepl[DEPLETION] = '2';
					ShingDepl[SHINGLING] = '1';
					break;
				case 6:	/* BEST */
					ShingDepl[DEPLETION] = '2';
					ShingDepl[SHINGLING] = '2';
					break;
				case 7: /* Transparancies BEST */
					ShingDepl[DEPLETION] = '1';
					ShingDepl[SHINGLING] = '2';
					break;
				default: /* Densities 1 - 4 */
					ShingDepl[DEPLETION] = '1';
					ShingDepl[SHINGLING] = '0';
					break;
				}

			err = (*(PD->pd_PWrite))(ShingDepl, SHING_LEN);

			if (gocolor == 0) {

				/*
					ct	- pointer to IODRPReq structure (use with caution!).
					x	- width of printed picture in pixels.
					y	- height of printed picture in pixels.
				*/
				RowSize = (x + 7) / 8;

				BufSize = RowSize + NUMTOTALCMD;
				TotalBufSize = BufSize * 2;
				PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
				if (PD->pd_PrintBuf == NULL) {
					err = PDERR_BUFFERMEMORY; /* couldn't get the memory */
				}
				else {
					ptr = PD->pd_PrintBuf;

					*ptr++ = 27;
					*ptr++ = '*';
					*ptr++ = 'b';			/* transfer raster graphics */
					*ptr++ = huns | '0';
					*ptr++ = tens | '0';
					*ptr++ = ones | '0';	/* printout width */
					*ptr = 'W';				/* terminator */

					ptr = &PD->pd_PrintBuf[BufSize];
					*ptr++ = 27;
					*ptr++ = '*';
					*ptr++ = 'b';			/* transfer raster graphics */
					*ptr++ = huns | '0';
					*ptr++ = tens | '0';
					*ptr++ = ones | '0';	/* printout width */
					*ptr = 'W';				/* terminator */

					dataoffset = NUMSTARTCMD;

					thous = x / 1000;
					huns = (x % 1000) / 100;
					tens = (x % 100) / 10;
					ones = x % 10;
					StartCmd[26] = thous + '0';
					StartCmd[27] = huns + '0';
					StartCmd[28] = tens + '0';
					StartCmd[29] = ones + '0';

					huns = RowSize / 100;
					tens = (RowSize % 100) / 10;
					ones = RowSize % 10;

					err = (*(PD->pd_PWrite))(StartCmd, START_LEN);


				}
			} else {
				RowSize = (x + 7) / 8;
				ColorSize = RowSize + NUMTOTALCMD;
				NumColorBufs = (PD->pd_Preferences.PrintShade ==
					SHADE_COLOR) ? MAXCOLORBUFS : 1;
				BufSize = ColorSize * NumColorBufs + NUMLFCMD;
				TotalBufSize = BufSize * 2;
				RLEBufSize = (RowSize + RLENUMTOTALCMD) * NumColorBufs + (RLENUMLFCMD + RLESAFETY);
				TotalBufSize += RLEBufSize * 2;
				for (i=0; i<NumColorBufs; i++) {
					colors[i] = ColorSize * i;
				}
				PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
				if (PD->pd_PrintBuf == NULL) {
					err = PDERR_BUFFERMEMORY; /* no mem */
				}
				else {
					dataoffset = NUMSTARTCMD;
					RLEBuf = &PD->pd_PrintBuf[BufSize * 2];
					rledataoffset = RLENUMSTARTCMD;
					thous = x / 1000;
					huns = (x % 1000) / 100;
					tens = (x % 100) / 10;
					ones = x % 10;
					StartCmd[26] = thous + '0';
					StartCmd[27] = huns + '0';
					StartCmd[28] = tens + '0';
					StartCmd[29] = ones + '0';
					huns = RowSize / 100;
					tens = (RowSize % 100) / 10;
					ones = RowSize % 10;
					StartCmd[CMYPALETTE] = NumColorBufs + '0';
					err = (*(PD->pd_PWrite))(StartCmd, START_LEN);

				}
			}

			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- NA
				y	- row.
			*/

/*
			if (gocolor == 0)
				Transfer(ct, y, &PD->pd_PrintBuf[dataoffset]);
			else
				CTransfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors);
*/

			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors);

			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- NA.
				x	- NA.
				y	- # of rows sent (1 to PED->ped_NumRows).

				White-space strip.  Scan backwards through the buffer for
				the first non-zero data byte.  Send up to this point only.
			*/
			if (gocolor == 0) {
				i = RowSize;
				ptrstart = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
				ptr = ptrstart + NUMSTARTCMD + i - 1;
				while (i > 0 && *ptr == 0) {
					i--;
					ptr--;
				}
				ptr = ptrstart + 3; /* get ptr to density info */
				*ptr++ = (huns = i / 100) | '0';
				*ptr++ = (i - huns * 100) / 10 | '0';
				*ptr = i % 10 | '0'; /* set printout width */
				err = (*(PD->pd_PWrite))(ptrstart, i + NUMTOTALCMD);
				if (err == PDERR_NOERR) {
					dataoffset =
						(dataoffset == NUMSTARTCMD ? BufSize : 0) + NUMSTARTCMD;
				}
			} else {
				rleptrstart = rleptr = &RLEBuf[rledataoffset - RLENUMSTARTCMD];
				ptrstart = &PD->pd_PrintBuf[dataoffset]; /* ptr to data */
				for (ct=0; ct<NumColorBufs; ct++, ptrstart += ColorSize) {
					Mode = RLE; /* default value */
					rleptrmark = rleptr; /* save start posn for this color */
					rleptr += RLENUMSTARTCMD; /* ptr to data */
					x = 0; /* clear # of data bytes */
					ptr = ptrstart; /* get ptr to bytes to rle */
					j = RowSize; /* # of bytes left to rle */
					do {
						rledata = *ptr++; /* get goal byte */
						j--;              /* and subtract it */
						i = RLEMAX - 1; /* this many repetitions left to go*/
						/* while repeating and not too many and more to do */
						while (*ptr == rledata && i > 0 && j > 0) {
							i--; /* one more rle byte */
							ptr++; /* advance ptr to next byte */
							j--; /* one less byte to look at */
						}
						/* calc repeating count */
						rlecount = RLEMAX - i - 1;
						*rleptr++ = rlecount; /* save repeat count */
						*rleptr++ = rledata; /* save repeat byte */
						x += 2; /* two more data bytes */
						if (x >= RowSize) {
							/* use uncompressed data if rle too big */
							x = RowSize;
							memcpy(rleptrmark + RLENUMSTARTCMD, ptrstart, x);
							rleptr = rleptrmark + RLENUMSTARTCMD + x;
							Mode = !RLE;
							break;
						}
					} while (j > 0); /* while more bytes to rle */
					*rleptrmark++ = 27;
					*rleptrmark++ = '*';
					*rleptrmark++ = 'b';
					*rleptrmark++ = (Mode == RLE) ? '1' : '0';
					*rleptrmark++ = 'm';
					*rleptrmark++ = (x / 100) | '0';
					*rleptrmark++ = ((x % 100) / 10) | '0';
					*rleptrmark++ = (x % 10) | '0';
					*rleptrmark = 'V'; /* set # of data bytes to follow */
				}
				*rleptr++ = 27;
				*rleptr++ = '*';
				*rleptr++ = 'b';
				*rleptr++ = '0';
				*rleptr++ = 'W'; /* last plane, no data (ie. lf) */

				/* send data to printer */
				err = (*(PD->pd_PWrite))(rleptrstart, rleptr - rleptrstart);

				if (err == PDERR_NOERR) { /* if no error during last write */
					dataoffset =
						(dataoffset == NUMSTARTCMD ? BufSize : 0) + NUMSTARTCMD;
					rledataoffset =
						(rledataoffset == RLENUMSTARTCMD ? RLEBufSize : 0) +
							RLENUMSTARTCMD;
				}

			}
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- NA.
				x	- NA.
				y	- NA.
			*/
			if (gocolor == 0) {
				ptr = &PD->pd_PrintBuf[dataoffset];
				i = RowSize;
				do {
					*ptr++ = 0;
				} while (--i);
			} else {
				ClearAndInit(&PD->pd_PrintBuf[dataoffset], BufSize);
				for (ct=0; ct<NumColorBufs; ct++) {
					ptr = &PD->pd_PrintBuf[dataoffset -
						NUMSTARTCMD + ct * ColorSize];
					*ptr++ = 27;
					*ptr++ = '*';
					*ptr++ = 'b';
					*ptr++ = huns | '0';
					*ptr++ = tens | '0';
					*ptr++ = ones | '0';
					*ptr = 'V';
				}
			}
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq structure.
				y	- NA.
			*/

			err = PDERR_NOERR; /* assume all ok */
			if (ct != PDERR_CANCEL) { /* if user did not cancel the print */
				/* end raster graphics, perf skip mode on */
				if ((err = (*(PD->pd_PWrite))(EndGraphics, END_LEN)) ==
					PDERR_NOERR) {
					/* if want to unload paper */
					if (!(x & SPECIAL_NOFORMFEED)) {
						err = (*(PD->pd_PWrite))("\014", 1); /* eject paper */
					}
				}
			}
			/*
				flag that there is no alpha data waiting that needs
				a formfeed (since we just did one)
			*/
			PED->ped_PrintMode = 0;
			(*(PD->pd_PBothReady))(); /* wait for both buffers to empty */
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5 : /* Pre-Master Initialization */
			/*
				ct	- NA.
				x	- io_Special flag from IODRPReq structure.
				y	- NA.
			*/
			if (gocolor) {
				StartCmd[RLE_ON] = '1';
			} else {
				StartCmd[RLE_ON] = '0';
				StartCmd[CMYPALETTE] = '1';
			}

			SetDensity(x & SPECIAL_DENSITYMASK); /* select density */

			break;
	}
	return(err);
}

ClearAndInit(ptr, size)
BYTE *ptr;
UWORD size;
{
	long *lptr, i, j, val;

	/*
		Note: Since 'NUMTOTAL + NUMLFCMD' is > 3 bytes it is safe
		to do the following to speed things up.
	*/
	i = size - NUMTOTALCMD - NUMLFCMD;
	j = (ULONG)ptr;

	val = 0;

	if (!(j & 1)) { /* if on a word boundary, clear by longs */
		i = (i + 3) / 4;
		lptr = (ULONG *)ptr;
		do {
			*lptr++ = val;
		} while (--i);
	}
	else { /* clear bytes */
		do {
			*ptr++ = val;
		} while (--i);
	}
	return(0);
}

