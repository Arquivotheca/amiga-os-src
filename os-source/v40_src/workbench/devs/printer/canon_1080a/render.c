/*
	Render routine for Canon_PJ-1080A driver.
	David Berezowski - March/88
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	3	/* # of cmd bytes before binary data */
#define NUMCOLORBUFS	3	/* # of color buffers */

Render(ct, x, y, status)
ULONG ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
	static UWORD colors[NUMCOLORBUFS]; /* color ptrs */
	UBYTE *ptr1, *ptr2, *ptr3;
	int i, j, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = (x + 7) / 8;
			ColorSize = RowSize;
			BufSize = ColorSize * NUMCOLORBUFS + NUMSTARTCMD;
			TotalBufSize = BufSize * 2;
			for (i=0; i<NUMCOLORBUFS; i++) {
				colors[i] = ColorSize * i;
			}
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* whoops, no mem */
			}
			else {
				dataoffset = NUMSTARTCMD;
				err = PDERR_NOERR; /* all ok */
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- NA
				y	- row.
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- NA.
				x	- NA.
				y	- # of rows sent (1 to NumRows - 1)

				White-space strip.  Scan backwards through the
				buffer for the first non-zero data byte.  Send
				up to this point only.
			*/
			i = ColorSize;
			ptr1 = &PD->pd_PrintBuf[dataoffset + i - 1];
			ptr2 = ptr1 + ColorSize;
			ptr3 = ptr2 + ColorSize;
			while ((i > 0) && (*ptr1-- == 0xff) &&
				(*ptr2-- == 0xff) && (*ptr3-- == 0xff)) {
				i--;
			}
			if (i == 0) { /* if no data to send advance y dots */
				PD->pd_PrintBuf[dataoffset - 3] = 27;
				PD->pd_PrintBuf[dataoffset - 2] = 'e';
				PD->pd_PrintBuf[dataoffset - 1] = y;
				err = (*(PD->pd_PWrite))
					(&(PD->pd_PrintBuf[dataoffset -
					NUMSTARTCMD]), 3);
			}
			else {
				PD->pd_PrintBuf[dataoffset - 3] = 27;
				PD->pd_PrintBuf[dataoffset - 2] = 'X';
				PD->pd_PrintBuf[dataoffset - 1] = i;
				if (i < ColorSize) {
					ptr1 += 2;
					ptr2 =  &PD->pd_PrintBuf[dataoffset +
						ColorSize];
					ptr3 = ptr2 + ColorSize;
					j = i;
					do { /* transfer green stuff */
						*ptr1++ = *ptr2++;
					} while (--j);
					j = i;
					do { /* transfer blue stuff */
						*ptr1++ = *ptr3++;
					} while (--j);
				}
				err = (*(PD->pd_PWrite))
					(&(PD->pd_PrintBuf[dataoffset -
					NUMSTARTCMD]), i * 3 + NUMSTARTCMD);
			}
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NUMSTARTCMD ?
				BufSize : 0) + NUMSTARTCMD;
			}
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- NA.
				x	- NA.
				y	- NA.
			*/
			ptr1 = &PD->pd_PrintBuf[dataoffset];
			i = BufSize - NUMSTARTCMD;
			do {
				*ptr1++ = 0xff; /* set to white */
			} while (--i);
			err = PDERR_NOERR; /* all ok */
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag
				y	- NA.
			*/
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) { /* free mem */
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			err = PDERR_NOERR; /* all ok */
			break;

		case 5 :  /* Pre-Master Initialization */
			/*
				ct	- NA.
				x	- io_Special flag
				y	- NA.
			*/
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}