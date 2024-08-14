/*
	Okimate_20 driver.
	David Berezowski - Sept/87.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	5	/* # of cmd bytes before binary data */
#define NUMENDCMD	1	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	3	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	3	/* max # of color buffers */

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color ptrs */
	static UBYTE CRLF[] = "\033Jn";
	static UWORD NumColorBufs; /* actually # of color buffers */
	static UWORD NumSpcCmd; /* # of cmd bytes at very beginning */
	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x * 3;
			ColorSize = RowSize + NUMTOTALCMD;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
				NumSpcCmd = 2;
			}
			else {
				NumColorBufs = 1;
				NumSpcCmd = 0;
			}
			BufSize = NumSpcCmd + ColorSize * NumColorBufs +
				NUMLFCMD;
			TotalBufSize = BufSize * 2;
			for (i=0; i<NumColorBufs; i++) {
				colors[i] = ColorSize * i; /* YMC or B */
			}
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				dataoffset = NumSpcCmd + NUMSTARTCMD;
				if (PD->pd_Preferences.PrintShade ==
					SHADE_COLOR) {
					/* align ribbon (1st buffer) */
					*PD->pd_PrintBuf = 27;
					*(PD->pd_PrintBuf + 1) = 25;
					/* align ribbon (2nd buffer) */
					*(PD->pd_PrintBuf + BufSize) = 27;
					*(PD->pd_PrintBuf + BufSize + 1) = 25;
				}
				err = PDERR_NOERR; /* all ok */
			}
			break;

		case 1: /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (range 0 to Height - 1).
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).

				White-space strip.
			*/
			ptrstart = &PD->pd_PrintBuf[dataoffset];
			ptr2 = ptrstart - NUMSTARTCMD;
			ptr2start = ptr2 - NumSpcCmd;
			x = 0; /* flag no transfer required yet */
			for (ct=0; ct<NumColorBufs;
				ct++, ptrstart += ColorSize) {
				i = RowSize;
				ptr = ptrstart + i - 1;
				while (i > 0 && *ptr == 0) {
					i--;
					ptr--;
				}
				i = (i + 2) / 3;
				ptr = ptrstart - NUMSTARTCMD;
				*ptr++ = 27;
				*ptr++ = '%';
				*ptr++ ='O';		/* enter 24-dot mode */
				*ptr++ = i & 0xff;
				*ptr++ = i >> 8;	/* size */
				i *= 3;
				*(ptrstart + i) = 13; /* cr */
				i += NUMTOTALCMD;
				if (x != 0) { /* if must transfer data */
					/* get src start */
					ptr = ptrstart - NUMSTARTCMD;
					do { /* transfer and update dest ptr */
						*ptr2++ = *ptr++;
					} while (--i);
				}
				else { /* no transfer required */
					ptr2 += i; /* update dest ptr */
				}
				/* if compacted or 0 */
				if (i != RowSize + NUMTOTALCMD) {
					x = 1; /* we need to xfer next time */
				}
			}
			y += (y + 1) / 2; /* convert 144ths to 216ths */
			*ptr2++ = 0x1b;
			*ptr2++ = 'J';
			*ptr2++ = y;	/* y/216 lf */
			i = ptr2 - ptr2start; /* calc size */
			/* if completely empty */
			if (i == NumSpcCmd + NUMTOTALCMD * NumColorBufs +
				NUMLFCMD) {
				CRLF[2] = y;
				/* y/216 lf */
				err = (*(PD->pd_PWrite))(CRLF, NUMLFCMD);
			}
			else {
				err = (*(PD->pd_PWrite))(ptr2start, i);
			}
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NumSpcCmd +
					NUMSTARTCMD ? BufSize : 0) +
					NumSpcCmd + NUMSTARTCMD;
			}
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
			ptr = &PD->pd_PrintBuf[dataoffset];
			i = BufSize - NumSpcCmd - NUMTOTALCMD - NUMLFCMD;
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
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}
