/*
	Diablo_C-150 driver.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	3	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	16	/* max # of color buffers */

extern UBYTE MargBuf[];

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color ptrs */
	static UWORD color_order[] =
		{0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13, 14, 15};
	static UWORD huns, tens, ones; /* used to program buffer size */
	static UWORD NumColorBufs; /* actual # of color buffers */
	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			 /* calc # of bytes of row data */
			RowSize = (x + 7) / 8;
			 /* size of each color buf */
			ColorSize = RowSize + NUMTOTALCMD;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
			}
			else {
				NumColorBufs = 4;
			}
			BufSize = ColorSize * MAXCOLORBUFS + NUMLFCMD;
			TotalBufSize = BufSize * 2;
			/*
				My color order		Black, Yellow, Magenta, Cyan.
				Diablo's color order	Black, Magenta, Yellow, Cyan.
			*/
			for (i=0; i<NumColorBufs; i++) {
				colors[color_order[i]] = ColorSize * i;
			}
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				dataoffset = NUMSTARTCMD;
				/* lmarg=.5 inch, rmarg=9 inches */
				err = (*(PD->pd_PWrite))
					("\033l5\r\033r90\r", 9);
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

				For each color buf, scan backwards through the
				buffer for the first non-zere data byte.
				Compact up to this point, then send the entire
				compacted buffer in one go.
			*/
			/* ptr to data */
			ptrstart = &PD->pd_PrintBuf[dataoffset];
			/* ptr to buf start */
			ptr2start = ptr2 = ptrstart - NUMSTARTCMD;
			x = 0; /* flag no transfer required yet */
			for (ct=0; ct<NumColorBufs;
				ct++, ptrstart += ColorSize) {
				i = RowSize;
				ptr = ptrstart + i - 1;
				while (i > 0 && *ptr == 0) {
					i--;
					ptr--;
				}
				if (i != 0) { /* if data */
					ptr = ptrstart - NUMSTARTCMD;
					*ptr++ = 27;
					*ptr++ = 'g';
					*ptr++ = ct + '0';	/* color */
					*ptr++ = (huns = i / 100) | '0';
					*ptr++ = (i - huns * 100) / 10 | '0';
					*ptr++ = i % 10 | '0';	/* width */
					*ptr = ',';	/* terminator */
					i += NUMTOTALCMD;
					/* if must transfer data */
					if (x != 0) {
						/* get src start */
						ptr = ptrstart - NUMSTARTCMD;
						do { /* transfer and update dest ptr */
							*ptr2++ = *ptr++;
						} while (--i);
					}
					else { /* no transfer required */
						ptr2 += i; /* update dest ptr */
					}
				}
				if (i != RowSize + NUMTOTALCMD) { /* if compacted or 0 */
					x = 1; /* flag that we need to transfer next time */
				}
			}
			*ptr2++ = 27;
			*ptr2++ = 'k';
			*ptr2++ = '1';		/* lf */
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
			ptr = &PD->pd_PrintBuf[dataoffset];
			i = BufSize - NUMTOTALCMD - NUMLFCMD;
			do {
				*ptr++ = 0;
			} while (--i);
			err = PDERR_NOERR; /* all ok */
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq struct.
				y	- 0.
			*/
			/* if user did not cancel print */
			if (ct != PDERR_CANCEL) {
				/* restore preferences pitch and margins */
				i = CalcMarg(PD->pd_Preferences.PrintLeftMargin
					, PD->pd_Preferences.PrintRightMargin);
				err = (*(PD->pd_PWrite))(MargBuf, i);
			}
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			err = PDERR_NOERR; /* all ok */
			break;

		case 5 : /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq struct.
				y	- 0.
			*/
			PED->ped_MaxColumns = PD->pd_Preferences.PaperSize ==
				W_TRACTOR ? 85 : 80;
	/* def is 80 chars (8.0 in.), W_TRACTOR is 85 chars (8.5 in.) */
			PED->ped_MaxXDots = (PED->ped_XDotsInch *
				PED->ped_MaxColumns) / 10;
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}
