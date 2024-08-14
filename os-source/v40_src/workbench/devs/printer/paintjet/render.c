/*
	HP_PaintJet driver.
	David Berezowski - August/87
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD) /* total of above */
#define NUMLFCMD	5	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	3	/* max # of color buffers */

#define RLEMAX		256
#define RLENUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define RLENUMENDCMD	0	/* # of cmd bytes after binary data */
#define RLENUMTOTALCMD 	(RLENUMSTARTCMD + RLENUMENDCMD)	/* total of above */
#define RLENUMLFCMD	5	/* # of cmd bytes for linefeed */
				/* extra room required for overwrites */
#define RLESAFETY	(RLENUMTOTALCMD + RLENUMLFCMD + 2)
#define	RLE		0	/* last data was sent via rle */

#define START_LEN	30	/* length of start_buf */
#define END_LEN		5	/* length of end_buf */

static UWORD NumColorBufs; /* actually # of color buffers */

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();
	extern struct PrinterData *PD;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color indexes */
	static UWORD thous, huns, tens, ones; /* used to program buf size */
	static UBYTE Mode; /* print mode, rle or non-rle */
	/*
		\033*b1M	00-04	- set transmission mode (rle data)
		\033*t180R	05-11	- set 180 dpi mode
		\033*r1440S	12-19	- set width to 1440 dots (the max)
		\033*r3U	20-24	- set # of color planes
		\033*r0A	25-29	- start raster gfx
	*/
	static UBYTE start_buf[START_LEN] =
		"\033*b1M\033*t180R\033*r1440S\033*r3U\033*r0A";
	/*
		\033*r0B	00-04	end raster gfx
	*/
	static UBYTE end_buf[END_LEN] = "\033*r0B";
	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start;
	int i, err;

	static UWORD RLEBufSize, rledataoffset;
	static UBYTE *RLEBuf;
	UBYTE *rleptrstart, *rleptr, *rleptrmark, rledata;
	int rlesize, rlecount, j;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = (x + 7) / 8;
			ColorSize = RowSize + NUMTOTALCMD;
			NumColorBufs = (PD->pd_Preferences.PrintShade ==
				SHADE_COLOR) ? MAXCOLORBUFS : 1;
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			TotalBufSize = BufSize * 2;
			RLEBufSize = BufSize + RLESAFETY;
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
				start_buf[15] = thous + '0';
				start_buf[16] = huns + '0';
				start_buf[17] = tens + '0';
				start_buf[18] = ones + '0';
				start_buf[23] = NumColorBufs + '0';
				huns = RowSize / 100;
				tens = (RowSize % 100) / 10;
				ones = RowSize % 10;
				Mode = RLE; /* start with RLE mode on */
				err = (*(PD->pd_PWrite))
					(start_buf, START_LEN);
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1).
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).
			*/
			/* Run-Length Encode (rle) the data */
			/* get ptr to rle buffer */
			rleptrstart = rleptr = &RLEBuf[rledataoffset - RLENUMSTARTCMD];
			ptrstart = &PD->pd_PrintBuf[dataoffset]; /* ptr to data */
			for (ct=0; ct<NumColorBufs; ct++, ptrstart += ColorSize) {
				rleptrmark = rleptr; /* save start posn for this color */
				rleptr += RLENUMSTARTCMD; /* ptr to data */
				x = 0; /* clear # of data bytes */
				ptr = ptrstart; /* get ptr to bytes to rle */
				j = RowSize; /* # of bytes left to rle */
				do {
					rledata = *ptr++; /* get goal byte */
					i = RLEMAX - 1; /* this many repetitions left to go*/
					/* while repeating and not too many and more to do */
					while (*ptr == rledata && i > 0 && j > 0) {
						i--; /* one more rle byte */
						ptr++; /* advance ptr to next byte */
						j--; /* one less byte to look at */
					}
					j--; /* dont forget the first byte */
					/* calc repeating count */
					rlecount = RLEMAX - i - 1;
					*rleptr++ = rlecount; /* save repeat count */
					*rleptr++ = rledata; /* save repeat byte */
					x += 2; /* two more data bytes */
					if (rleptr - rleptrstart >= BufSize) {
						/* abort if rle too big */
						goto rleover;
					}
				} while (j > 0); /* while more bytes to rle */
				*rleptrmark++ = 27;
				*rleptrmark++ = '*';
				*rleptrmark++ = 'b';
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
rleover:
			i = rleptr - rleptrstart; /* calc size of rlebuf */
			err = PDERR_NOERR; /* clear error code */
			if (i < BufSize) { /* if rle data is less send it */

				if (Mode != RLE) { /* if printer is not expecting rle data */
					/* tell it that we are sending rle data */
					err = (*(PD->pd_PWrite))("\033*b1M",5);
					Mode = RLE;
				}
				if (err == PDERR_NOERR) { /* if no error during last write */
					/* send rle data to printer */
					err = (*(PD->pd_PWrite))(rleptrstart, i);
				}
			}
			else { /* send non-rle data */
				ptrstart = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
				ptr = ptrstart + BufSize - NUMLFCMD;
				*ptr++ = 27;
				*ptr++ = '*';
				*ptr++ = 'b';
				*ptr++ = '0';
				*ptr++ = 'W'; /* last plane, no data (ie. lf) */
				if (Mode == RLE) { /* if printer is expecting rle data */
					/* tell it that we are not sending rle data */
					err = (*(PD->pd_PWrite))("\033*b0M",5);
					Mode = !RLE;
				}
				if (err == PDERR_NOERR) { /* if no error during last write */
					/* send non-rle data to printer */
					err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
				}
			}
			if (err == PDERR_NOERR) { /* if no error during last write */
				dataoffset =
					(dataoffset == NUMSTARTCMD ? BufSize : 0) + NUMSTARTCMD;
				rledataoffset = 
					(rledataoffset == RLENUMSTARTCMD ? RLEBufSize : 0) +
						RLENUMSTARTCMD;
			}
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
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
			err = PDERR_NOERR; /* all ok */
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel the print */
			if (ct != PDERR_CANCEL) {
				/* end raster gfx */
				err = (*(PD->pd_PWrite))(end_buf, END_LEN);
			}
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5 : /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			err = PDERR_NOERR; /* all ok */
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
	val = (NumColorBufs == MAXCOLORBUFS) ? ~0 : 0;
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
}
