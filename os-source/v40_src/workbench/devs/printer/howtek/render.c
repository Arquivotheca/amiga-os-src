/*
	Howtek_Pixelmaster driver.
	David Berezowski - April/88.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/prtbase.h"
#include "../printer/printer.h"

#define NUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	0	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	3	/* max # of color buffers */

#define DEBUG0	0
#define DEBUG1	0
#define DEBUG2	0
#define DEBUG3	0
#define DEBUG4	0
#define DEBUG5	0

#define START_LEN	17	/* length of start_buf */

extern SetDensity();
/*
	0-4	\033&l0L	perf skip mode off
	5-11	\033*t000R	set raster graphics resolution (dpi)
	12-16	\033*r0A	start raster graphics
*/
char StartCmd[START_LEN] = "\033&l0L\033*t000R\033*r0A";

static UWORD NumColorBufs;

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color indexes */
	static UWORD huns, tens, ones; /* used to program buffer size */
	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start, val;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
#if DEBUG0
			kprintf("0: ct=%ld, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG0
			RowSize = (x + 7) / 8;
			ColorSize = RowSize + NUMTOTALCMD;
			NumColorBufs = (PD->pd_Preferences.PrintShade ==
				SHADE_COLOR) ? MAXCOLORBUFS : 1;
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			TotalBufSize = BufSize * 2;
#if DEBUG0
			kprintf("0: RS=%ld, CS=%ld, BS=%ld, TBS=%ld\n",
				RowSize, ColorSize, BufSize, TotalBufSize);
#endif DEBUG0
			for (i=0; i<NumColorBufs; i++) {
				colors[i] = ColorSize * i;
			}
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				ptr = PD->pd_PrintBuf;
				dataoffset = NUMSTARTCMD;
			/* perf skip mode off, set dpi, start raster gfx */
				err = (*(PD->pd_PWrite))(StartCmd, START_LEN);
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

				White-space strip.  Scan backwards through
				the buffer for the first non-zero data byte.
				Send up to this point only.
			*/
			val = (NumColorBufs == MAXCOLORBUFS) ? ~0 : 0;
#if DEBUG2
			kprintf("2: ct=%ld, x=%ld, y=%ld, val=%ld, %lx\n",
				ct, x, y, val, val);
#endif DEBUG2
			ptrstart = &PD->pd_PrintBuf[dataoffset];
			ptr2start = ptr2 = ptrstart - NUMSTARTCMD;
			x = 0; /* flag no transfer required yet */
			for (ct=0; ct<NumColorBufs;
				ct++, ptrstart += ColorSize) {
				i = RowSize;
				/*
					White-space stripping is temporarely
					disabled (for color dumps) as the
					current firmware of the printer can't
					handle variable sized bit planes.
				*/
				if (NumColorBufs == 1) { /* if not color */
				ptr = ptrstart + i - 1;
				while (i > 0 && *ptr == val) {
					i--;
					ptr--;
				}
				}
				ptr = ptrstart - NUMSTARTCMD;
				*ptr++ = 27;
				*ptr++ = '*';
				*ptr++ = 'b'; /* xfer raster gfx */
				*ptr++ = (huns = i / 100) | '0';
				*ptr++ = (i - huns * 100) / 10 | '0';
				*ptr++ = i % 10 | '0'; /* width */
				*ptr = (ct == NumColorBufs - 1) ?
					'W' : 'V'; /* terminator */
				i += NUMTOTALCMD;
				if (x != 0) { /* if must transfer data */
					ptr = ptrstart - NUMSTARTCMD;
					do { /* xfer and update dest ptr */
						*ptr2++ = *ptr++;
					} while (--i);
				}
				else { /* no transfer required */
					ptr2 += i; /* update dest ptr */
				}
				/* if compacted or 0 */
				if (i != RowSize + NUMTOTALCMD) {
					x = 1; /* we must xfer next time */
				}
			}
			err = (*(PD->pd_PWrite))(ptr2start, ptr2 - ptr2start);
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NUMSTARTCMD ?
					BufSize : 0) + NUMSTARTCMD;
			}
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
#if DEBUG3
			kprintf("3: ct=%ld, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG3
			ClearAndInit(&PD->pd_PrintBuf[dataoffset], BufSize);
			err = PDERR_NOERR; /* all ok */
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
#if DEBUG4
			kprintf("4: ct=%ld, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG4
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel the print */
			if (ct != PDERR_CANCEL) {
				/* end raster graphics, perf skip mode on */
				if ((err = (*(PD->pd_PWrite))
				("\033*rB\033&l1L", 9)) == PDERR_NOERR) {
					/* if want to unload paper */
					if (!(x & SPECIAL_NOFORMFEED)) {
						err = (*(PD->pd_PWrite))
						("\014", 1); /* eject paper */
					}
				}
			}
			/*
			flag that there is no alpha data waiting that needs
			a formfeed (since we just did one)
			*/
			PED->ped_PrintMode = 0;
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
#if DEBUG5
			kprintf("5: ct=%ld, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG5
			/* select density */
			SetDensity(x & SPECIAL_DENSITYMASK);
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
