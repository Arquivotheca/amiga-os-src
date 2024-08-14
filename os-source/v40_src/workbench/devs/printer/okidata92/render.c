/*
	Okidata-92 driver.
	David Berezowski - June/87
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	0	/* # of cmd bytes before binary data */
#define NUMENDCMD	2	/* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)	/* total of above */

#define STARTLEN	9

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, BufSize, OutSize, TotalBufSize;
	static UWORD dataoffset;
	static UBYTE *ptr, *ptr2, *ptr2start;
	/*
		00-05	\033%Cnnn	set left margin to n
		06-06	\034		12 cpi (required for 72x72 dpi gfx)
		07-07	\003		enter graphics mode
		08-08	\015		carriage return
	*/
	static UBYTE StartBuf[STARTLEN] = "\033%Cnnn\034\003\015";
	int i, err;

	switch(status) {
		case 0 : /* alloc memory for printer buffer */
			RowSize = x;
			BufSize = RowSize;
			OutSize = (BufSize * 2 + NUMTOTALCMD);
			TotalBufSize = (BufSize + OutSize) * 2;
			PD->pd_PrintBuf = AllocMem(TotalBufSize,
				MEMF_PUBLIC|MEMF_CLEAR);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY;
			}
			else {
				dataoffset = NUMSTARTCMD;
				/*
					This printer prints graphics within its
					text margins.  This code makes sure the
					printer is in 12 cpi and then sets the
					left and right margins to their minimum
					and maximum values (respectively).  A
					carriage return is sent so that the
					print head is at the leftmost position
					as this printer starts printing from
					the print head's position.
				*/
				StartBuf[0] = '\033';
				StartBuf[1] = '%';
				StartBuf[2] = 'C';
				StartBuf[3] = '0';
				StartBuf[4] = '0';
				StartBuf[5] = '1'; /* min left marg */
				StartBuf[6] = '\034'; /* 12 cpi */
				StartBuf[7] = '\015'; /* return */
				StartBuf[8] = '\003';/*  enter gfx mode */
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			break;

		case 1 :	/* put pixel in buffer */
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset]);
			err = PDERR_NOERR;
			break;

		case 2 : /* dump buffer to printer */
			ptr = &PD->pd_PrintBuf[dataoffset]; /* source */
			ptr2 = ptr2start = ptr + BufSize; /* dest */
			i = RowSize;
			do { /* transfer and double up on ETX ($03) */
				if ((*ptr2++ = *ptr++) == 3) {
					*ptr2++ = 3; /* repeat */
				}
			} while (--i);
			/*
				Scan backwards through the buffer for the first
				non-zero data byte.  Send up to this point only.
			*/
			i = RowSize;
			ptr2--;
			while (i > 0 && *ptr2 == 0) {
				i--;
				ptr2--;
			}
			ptr2++;
			*ptr2++ = 3;
			/* print data (if any) and advance 7/72 inch */
			*ptr2++ = 14;
			err = (*(PD->pd_PWrite))(ptr2start, ptr2 - ptr2start);
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NUMSTARTCMD ?
					BufSize + OutSize : 0) + NUMSTARTCMD;
			}
			break;

		case 3 : /* clear and init buffer */
			ptr = &PD->pd_PrintBuf[dataoffset];
			i = RowSize;
			do {
				*ptr++ = 0;
			} while (--i);
			err = PDERR_NOERR;
			break;

		case 4 : /* free the print buffer memory */
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel the print */
			if (ct != PDERR_CANCEL) {
				StartBuf[0] = '\003';
				StartBuf[1] = '\002'; /* exit gfx mode */
				/* restore preferences pitch */
				if (PD->pd_Preferences.PrintPitch == PICA) {
					StartBuf[2] = '\036';
					i = 100; /* 10 cpi */
				}
				else if (PD->pd_Preferences.PrintPitch == ELITE) {
					StartBuf[2] = '\034';
					i = 120; /* 12 cpi */
				}
				else { /* must be FINE */
					StartBuf[2] = '\035';
					i = 171; /* 17.1 cpi */
				}
				/* restore preferences margins */
			/* convert from 120ths/inch to chars @ current cpi */
				i = (PD->pd_Preferences.PrintLeftMargin - 1) *
					1200 / i + 1;
				StartBuf[3] = '\033';
				StartBuf[4] = '%';
				StartBuf[5] = 'C';
				StartBuf[6] = ((i % 1000) / 100) | '0';
				StartBuf[7] = ((i % 100) / 10) | '0';
				StartBuf[8] = (i % 10) | '0';
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5 : /* io_special flag call */
			err = PDERR_NOERR;
			break;
	}
	return(err);
}
