/*
	EpsonXOld (EX/FX/JX/LX/MX/RX) driver.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

#define NUMSTARTCMD	4	/* # of cmd bytes before binary data */
#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	5	/* # of cmd bytes for linefeed */

#define LPI		2
#define STARTLEN	3

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, BufSize, TotalBufSize;
	static UWORD dataoffset, dpi_code;
	/*
		00-00	\015		carriage return
		01-02	\0332		set lpi
	*/
	static UBYTE StartBuf[STARTLEN] = "\015\0332";
	UBYTE *ptr, *ptrstart;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x;
			BufSize = RowSize + NUMTOTALCMD + NUMLFCMD;
			TotalBufSize = BufSize * 2;
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY;
			}
			else {
				dataoffset = NUMSTARTCMD;
				/*
					A carriage return is sent so that the
					print head is at the leftmost position
					as this printer starts printing from
					the print head's position.
				*/
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1).
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset]);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).

				Scan backwards through the buffer for the
				first non-zero data byte, send only up to
				this point only.
			*/
			i = RowSize;
			ptrstart = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
			ptr = ptrstart + i + NUMSTARTCMD - 1;
			while (i > 0 && *ptr == 0) {
				i--;
				ptr--;
			}
			if (i != 0) { /* if data */
				ptr++; /* advance past last valid data */
				*ptrstart = 27;
				*(ptrstart + 1) = dpi_code; /* density */
				*(ptrstart + 2) = i & 0xff;
				*(ptrstart + 3) = i >> 8; /* size */
			}
			else { /* no data */
				ptr = ptrstart; /* reset ptr to buf start */
			}
			*ptr++ = 13; /* cr */
			*ptr++ = 27;
			*ptr++ = 'A';
			*ptr++ = y;
			*ptr++ = 10; /* y/72 lf */
			err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
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
			err = PDERR_NOERR;
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq struct.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel print */
			if (ct != PDERR_CANCEL) {
				/* restore preferences lpi */
				StartBuf[LPI] = (PD->pd_Preferences.
					PrintSpacing == SIX_LPI) ? '2' : '0';
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5 :  /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq struct.
				y	- 0.
			*/
			dpi_code = SetDensity(x & SPECIAL_DENSITYMASK);
			err = PDERR_NOERR;
			break;
	}
	return(err);
}
