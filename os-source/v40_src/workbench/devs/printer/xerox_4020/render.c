/*
	Xerox-4020 driver.
	David Berezowski - October/87.
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
#define NUMLFCMD	9	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	16	/* max # of color buffers */

#define RLEMAX		136
#define RLENUMSTARTCMD	3	/* # of cmd bytes before binary data */
#define RLENUMENDCMD	1	/* # of cmd bytes after binary data */
#define RLENUMTOTALCMD 	(RLENUMSTARTCMD + RLENUMENDCMD)	/* total of above */
#define RLESAFETY	10	/* extra room for overwrites */

#define PMODE		15	/* index into StartBuf for print mode */
#define STARTLEN	16	/* length of start buffer */

extern UBYTE MargBuf[];

/* (30.5cm/ft) * (10mm/cm)					*/
#define MMPERFT		305

/* Margins: minus 1/2" width, minus 1" height (== US diffs)	*/
/*								*/
/* Left            Right           Top           Bottom		*/
/* 50/300 = 1/6    100/300 = 1/3   60/300 = 1/5  60/300 = 1/5	*/
/* 								*/

/* Half inches per ft */
#define HALFINCHPERFT 24

/* Inches per ft */
#define INCHPERFT 12

/* MM per 1/2 inch (round up [12.7->13]) */
#define HALFINCH ((MMPERFT + INCHPERFT)/HALFINCHPERFT)

/* MM per inch (round up) 25.4 */
#define INCH ((MMPERFT + INCHPERFT)/INCHPERFT)

/* Sparse table for paper sizes - offset constant */
#define PSIZE_OFFSET 16

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
		00-02	\033F0		set 10 cpi
		03-07	\033l05\r	set left margin to .5 inches
		09-12	\033r95\r	set right margin to 9.5 inches
		13-15	\033we		select standard (e) or
					enhanced (f) graphics mode.
	*/
	static UBYTE stdmode, StartBuf[STARTLEN] =
		"\033F0\033l05\r\033r95\r\033we";
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
			RLEBufSize = BufSize + RLESAFETY;
			TotalBufSize += RLEBufSize * 2;
			/*
				My color order:		B, Y, M, C
				Xerox's color order:	B, M, Y, C
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
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
				RLEBuf = &PD->pd_PrintBuf[BufSize * 2];
				rledataoffset = RLENUMSTARTCMD;
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
			rleptrstart = rleptr =
				&RLEBuf[rledataoffset - RLENUMSTARTCMD];
			/* ptr to data */
			ptrstart = &PD->pd_PrintBuf[dataoffset];
			for (ct=0; ct<NumColorBufs;
				ct++, ptrstart += ColorSize) {
				/* save start posn for this color */
				rleptrmark = rleptr;
				*rleptr++ = 27; /* rle start cmd */
				*rleptr++ = 'h';
				*rleptr++ = ct | '0'; /* color code */
				ptr = ptrstart; /* get ptr to bytes to rle */
				j = RowSize - 1; /* # of bytes left to rle */
				do {
					/* first do repeating bytes */
					/* get goal (repeating) byte */
					rledata = *ptr++;
					/* this many repetitions left to go*/
					i = RLEMAX - 1;
					/* while repeating and not too many
						and more to do */
					while (*ptr == rledata && i > 0 &&
						j > 0) {
						i--; /* one more rle byte */
						/* advance ptr to next byte */
						ptr++;
						/* one less byte to look at */
						j--;
					}
					/* calc repeating byte count */
					if ((rlecount = RLEMAX - i) == 1) {
						/* if only 1 then no repeat */
						rlecount = 0;
					}
					else {
						/* dont forget the goal byte */
						j--;
					}
					/* if there was repeat data */
					if (rlecount != 0) {
						/* save repeat count */
						*rleptr++ = rlecount;
						/* save repeat byte */
						*rleptr++ = rledata;
						/* get non-repeat goal byte */
						rledata = *ptr++;
					}
					/* now do non-repeating data */
					/* no non-repeating bytes yet */
					rlecount = 0;
					if (*ptr != rledata && j >= 0) {
						/* non-repeat data follows */
						*rleptr++ = 0x00;
					}
					/* while non-repeating and more to do */
					while (*ptr != rledata && j >= 0) {
						/* save byte */
						*rleptr++ = rledata;
						/* if byte same as terminator */
						if (rledata == 0xfe) {
							/* save byte (again) */
							*rleptr++ = rledata;
						}
						/* one more non-repeat byte */
						rlecount++;
						/* get goal byte */
						rledata = *ptr++;
						/* one less byte to look at */
						j--;
					}
					if (rlecount != 0) {
						/* end of non-repeating bytes */
						*rleptr++ = 0xfe;
					}
					if (j > 0) { /* if more data to do */
						/* set ptr back to start
							of repeat bytes */
						ptr--;
					}
					if (rleptr - rleptrstart > BufSize) {
						/* abort: too many rle bytes */
						break;
					}
				} while (j > 0); /* while more bytes to rle */

				/* if didnt abort && no non-repeating data */
				if (j < 1 && rlecount == 0) {
					/* check for trailing white space */
					/* line ends in trailing 0 */
					if (*(rleptr - 1) == 0x00) {
						/* ptr back to repeat count */
						rleptr -= 2;
					}
				}
				/* if line is just the cmd bytes */
				/* line null */
				if (rleptr - rleptrmark == RLENUMSTARTCMD) {
					/* reset ptr to start */
					rleptr = rleptrmark;
				}
				else {
					*rleptr++ = 0xff; /* end of rle line */
				}
			}
			i = rleptr - rleptrstart; /* calc size of rlebuf */
			/* if rle data is more send non-rle data */
			if (i > BufSize) {
				ptrstart = &PD->pd_PrintBuf[dataoffset -
					NUMSTARTCMD];
				ptr = ptrstart + BufSize - NUMLFCMD;
				/* if standard print mode and any black
					in this micro-line */
				if (stdmode && *(ptrstart + 2) < '4') {
					*ptr++ = 27;
					*ptr++ = 'k';
					*ptr++ = '0';	/* cr */
					*ptr++ = 27;
					*ptr++ = 'w';
					*ptr++ = 'B';	/* repeat black */
				}
				*ptr++ = 27;
				*ptr++ = 'k';
				*ptr++ = '1';		/* cr/lf */
				err = (*(PD->pd_PWrite))
					(ptrstart, ptr - ptrstart);
			}
			else { /* send rle data */
				/* if any black in this micro-line */
				if (rleptr - rleptrstart > 0 &&
					*(rleptrstart + 2) < '4') {
					*rleptr++ = 27;
					*rleptr++ = 'k';
					*rleptr++ = '0'; /* cr */
					*rleptr++ = 27;
					*rleptr++ = 'w';
					*rleptr++ = 'B'; /* repeat black */
				}
				*rleptr++ = 27;
				*rleptr++ = 'k';
				*rleptr++ = '1';	/* cr/lf */
				i = rleptr - rleptrstart; /* size of rlebuf */
				err = (*(PD->pd_PWrite))(rleptrstart, i);
			}
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NUMSTARTCMD ?
					BufSize : 0) + NUMSTARTCMD;
				rledataoffset = (rledataoffset ==
					RLENUMSTARTCMD ? RLEBufSize : 0) +
					RLENUMSTARTCMD;
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
				*ptr++ = 'g';
				*ptr++ = ct + '0';	/* color */
				*ptr++ = huns | '0';
				*ptr++ = tens | '0';
				*ptr++ = ones | '0';	/* printout width */
				*ptr = ',';		/* terminator */
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

			StartBuf[PMODE - 1] = 'w';
			if ((x & SPECIAL_DENSITYMASK) < SPECIAL_DENSITY2) {
				/* standard graphics mode */
				StartBuf[PMODE] = 'e';
				stdmode = 1;
			}
			else {
				/* enhanced graphics mode */
				StartBuf[PMODE] = 'f';
				stdmode = 0;
			}
#if 1
			{
				/* Paper sizes in inches */
				static int PaperXSizes[] = {
					80,			/* US_LETTER 8.0 x 10.0 */
					80,			/* US_LEGAL  8.0 x 13.0 */
					80,			/* N_TRACTOR == LETTER */
					90,			/* W_TRACTOR == 9.0 x  10.0 */
					80,			/* CUSTOM    == LETTER */
				/* Euro Ax sizes follow		*/
				/* Paper size in millimeters - 1/2 inch */
					841 - HALFINCH,		/* A0	*/
					594 - HALFINCH,		/* A1	*/
					420 - HALFINCH,		/* A2	*/
					297 - HALFINCH,		/* A3	*/
					210 - HALFINCH,		/* A4	*/
					148 - HALFINCH,		/* A5	*/
					105 - HALFINCH,		/* A6	*/
					74  - HALFINCH,		/* A7	*/
					52  - HALFINCH,		/* A8	*/
					};

				/* Calculate max dots based on paper size selection */
				UWORD psize;
			        ULONG maxwidth;

				psize = PD->pd_Preferences.PaperSize;
				maxwidth =  (ULONG)PaperXSizes[psize/PSIZE_OFFSET];
				if(psize <= CUSTOM)
				{
					PED->ped_MaxColumns = maxwidth;
					PED->ped_MaxXDots = maxwidth * PED->ped_XDotsInch / 10;
				}
				else
				{
					PED->ped_MaxColumns = maxwidth * 100 / 254;
					PED->ped_MaxXDots = (maxwidth * INCHPERFT * PED->ped_XDotsInch) / MMPERFT;
				}
			}
#else
			PED->ped_MaxColumns = PD->pd_Preferences.PaperSize == W_TRACTOR ? 90 : 80;
			/* def is 80 chars (8.0 in.), W_TRACTOR is 90 chars (9.0 in.) */
			PED->ped_MaxXDots = (PED->ped_XDotsInch * PED->ped_MaxColumns) / 10;
#endif

			/*
			The manual says that the printer has 1088 dots BUT I
			could never get more than 1080 out of it.  This kludge
			is here as '121 * 90 / 10 = 1089' which is > 1080.
			*/
			if (PED->ped_MaxXDots > 1080) {
	 			PED->ped_MaxXDots = 1080;
			}
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}
