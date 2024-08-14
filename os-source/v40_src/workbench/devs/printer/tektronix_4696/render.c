/*
	Tektronix_4696 driver.
	David Berezowski - May/88.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	6	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	2	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	16	/* max # of color buffers */

#define PMODE_START	32	/* index into start_buf for print mode */
#define START_LEN	33	/* length of start buffer */

extern UBYTE MargBuf[];

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color indexes */
	static UWORD color_order[] =
		{0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13, 14, 15};
	static UWORD huns, tens, ones; /* used to program buffer size */
	static UWORD NumColorBufs; /* actually # of color buffers */
	/*
		00-01	\033H	- cmd header
		02-03	10	- # of tab stops (max is 10)
		04-05	10	- left margin (from left edge, min is 10)
		06-25	08	- intertab spacing of 8 chars (10 times)
		26-27	04	- right margin (from right edge, min is 04)
		28-28	\015	- carriage-return
		29-32	\033#$1	- select graphics mode.
	*/
	static UBYTE start_buf[START_LEN] =
		"\033H10100808080808080808080804\015\033#$1";
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
			huns = RowSize / 100;
			tens = (RowSize - huns * 100) / 10;
			ones = RowSize % 10;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
			}
			else {
				NumColorBufs = 4;
			}
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			TotalBufSize = BufSize * 2;
			/*
				My color order:			B, Y, M, C
				Tektronix's color order:	B, M, Y, C
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
				/*
					This printer prints graphics within its
					text margins.  This code sets the
					left and right margins to their minimum
					and maximum values (respectively).  A
					carriage return is sent so that the
					print head is at the leftmost position
					as this printer starts printing from
					the print head's position.				*/
				/* clear margins and select gfx mode */
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
			ptrstart = &PD->pd_PrintBuf[dataoffset];
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
					*ptr++ = 'I';
					*ptr++ = ct + '0'; /* color */
					*ptr++ = (huns = i / 100) | '0';
					*ptr++ = (i - huns * 100) / 10 | '0';
					*ptr++ = i % 10 | '0'; /* width */
					i += NUMTOTALCMD;
					if (x != 0) { /* if must transfer data */
						ptr = ptrstart - NUMSTARTCMD; /* get src start */
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
			*ptr2++ = 'A'; /* lf */
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
			for (ct=0; ct<NumColorBufs; ct++) {
				ptr = &PD->pd_PrintBuf[dataoffset -
					NUMSTARTCMD + ct * ColorSize];
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
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			x &= SPECIAL_DENSITYMASK;
			if (x < SPECIAL_DENSITY2) {
				start_buf[PMODE_START] = '1';
			}
			else if (x < SPECIAL_DENSITY3) {
				start_buf[PMODE_START] = '2';
			}
			else {
				start_buf[PMODE_START] = '3';
			}
			PED->ped_MaxColumns = PD->pd_Preferences.PaperSize ==
				W_TRACTOR ? 85 : 80;
	/* default is 80 chars (8.0 in.), W_TRACTOR is 85 chars (8.5 in.) */
			if ((PED->ped_MaxXDots = (PED->ped_XDotsInch *
				PED->ped_MaxColumns) / 10) > 1024) {
				PED->ped_MaxXDots = 1024;
			}
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}
