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

#define NUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)	/* total of above */

extern SetDensity();
/*
	0-4		\033&l0L	perf skip mode off
	5-11	\033*t075R	set raster graphics resolution (dpi)
	12-16	\033*r0A	start raster graphics
*/
char StartCmd[17] = "\033&l0L\033*t075R\033*r0A";

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, BufSize, TotalBufSize, dataoffset;
	static UWORD huns, tens, ones; /* used to program buffer size */
	UBYTE *ptr, *ptrstart;
	int i, err;

	/* set to NOERR for cases which do not set err */

	err=PDERR_NOERR;

	switch(status) {
		case 0 : /* Master Initialization */
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
				/* perf skip mode off, set dpi, start raster graphics */
				err = (*(PD->pd_PWrite))(StartCmd, 17);
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- NA
				y	- row.
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset]);
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
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- NA.
				x	- NA.
				y	- NA.
			*/
			ptr = &PD->pd_PrintBuf[dataoffset];
			i = RowSize;
			do {
				*ptr++ = 0;
			} while (--i);
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
				if ((err = (*(PD->pd_PWrite))("\033*rB\033&l1L", 9)) ==
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
			SetDensity(x & SPECIAL_DENSITYMASK); /* select density */
			break;
	}
	return(err);
}
