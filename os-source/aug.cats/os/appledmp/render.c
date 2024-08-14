/*
	Apple ImagewriterII (Imagewriter compatible) driver.
	David Berezowski - May/87
	Dec 93 - CAS - change back graphics command to work with AppleDMP
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "devices/printer.h"
#include "devices/prtbase.h"
#include "devices/prtgfx.h"

#define MAXSTARTCMD	9	/* max # of cmd bytes before binary data */
#define NUMENDCMD	1	/* # of cmd bytes after binary data */
#define NUMLFCMD	5	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	4	/* max # of color buffers */

#define STARTLEN	9
#define ENDLEN		11
#define CPI		1
#define	LPI		3
#define LMARG		8

static ULONG TwoBufSize;
static UWORD RowSize, ColorSize, NumColorBufs, NumStartCmd, NumTotalCmd;
static UWORD colorcodes[MAXCOLORBUFS]; /* printer color codes */

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	/*
		00-01	\033N		cpi (dpi for gfx)
		02-03	\033>		uni-directional print
		04-08	\033L000	set left margin to 0
	*/
	static char StartCmd[STARTLEN+1] = "\033N\033>\033L000";
	/*
		00-01	\033N		cpi
		02-03	\033A		lpi
		04-05	\033<		bi-directional print
		06-10	\033Lnnn	set left margin to nnn
	*/
	static char EndCmd[ENDLEN+1] = "\033N\033A\033<\033Lnnn";

	UBYTE *CompactBuf();
	static ULONG BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color ptrs */
	UBYTE *ptr, *ptrstart;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct - pointer to IODRPReq structure.
				x - width of printed picture in pixels.
				y - height of printed picture in pixels.
			*/
			/* round up to nearest 8 bytes */
			RowSize = (x + 7) / 8 * 8;
			NumStartCmd = (PD->pd_Preferences.PrintShade ==
				SHADE_COLOR) ? MAXSTARTCMD : MAXSTARTCMD - 3;
			NumTotalCmd = NumStartCmd + NUMENDCMD;
			ColorSize = RowSize + NumTotalCmd;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
				colors[0] = ColorSize * 3; /* Black */
				colors[1] = ColorSize * 0; /* Yellow */
				colors[2] = ColorSize * 1; /* Magenta */
				colors[3] = ColorSize * 2; /* Cyan */
				colorcodes[0] = '1'; /* Yellow */
				colorcodes[1] = '2'; /* Magenta */
				colorcodes[2] = '3'; /* Cyan */
				colorcodes[3] = '0'; /* Black */
			}
			else { /* grey-scale or black&white */
				NumColorBufs = 1;
				colors[0] = ColorSize * 0; /* Black */
			}
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			if (PED->ped_YDotsInch == 144) {
				TwoBufSize = BufSize * 2;
				TotalBufSize = BufSize * 4;
			}
			else {
				TwoBufSize = BufSize * 1;
				TotalBufSize = BufSize * 2;
			}
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				dataoffset = NumStartCmd;
				err = (*(PD->pd_PWrite))(StartCmd, STARTLEN);
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1).
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors,
				BufSize);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).
			*/
			x = (PED->ped_YDotsInch == 144) ? 1 : y;
			ptrstart = &PD->pd_PrintBuf[dataoffset - NumStartCmd];
			ptr = CompactBuf(ptrstart + NumStartCmd, ptrstart,
				x, 1);
			if (PED->ped_YDotsInch == 144 && y > 1) {
				ptr = CompactBuf(&PD->pd_PrintBuf[
					dataoffset + BufSize], ptr, y - 1, 0);
			}
			err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NumStartCmd ?
					TwoBufSize : 0) + NumStartCmd;
			}
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
			ClearAndInit(&PD->pd_PrintBuf[dataoffset]);
			err = PDERR_NOERR;
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel the print */
			if (ct != PDERR_CANCEL) {
				/* restore pitch, spacing, and left margin */
				switch (PD->pd_Preferences.PrintPitch) {
					case PICA: /* 10 cpi */
						EndCmd[CPI] = 'N';
						break;
					case ELITE: /* 12 cpi */
						EndCmd[CPI] = 'E';
						break;
					default: /* 15 cpi (FINE) */
						EndCmd[CPI] = 'q';
						break;
				}
				switch (PD->pd_Preferences.PrintSpacing) {
					case SIX_LPI:
						EndCmd[LPI] = 'A';
						break;
					default: /* EIGHT_LPI */
						EndCmd[LPI] = 'B';
						break;
				}
				i = PD->pd_Preferences.PrintLeftMargin - 1;
				EndCmd[LMARG] = i / 100 + '0';
				EndCmd[LMARG + 1] = (i % 100) / 10 + '0';
				EndCmd[LMARG + 2] = (i % 10) + '0';
				err = (*(PD->pd_PWrite))(EndCmd, ENDLEN);
			}
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5 :  /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag.
				y	- 0.
			*/
			StartCmd[1] = SetDensity(x & SPECIAL_DENSITYMASK);
			err = PDERR_NOERR;
			break;
	}
	return(err);
}

UBYTE *CompactBuf(ptrstart, ptr2start, y, flag)
UBYTE *ptrstart, *ptr2start;
long y;
int flag;
{
	static int x;
	UBYTE *ptr, *ptr2;
	long ct;
	int i,e;

	ptr2 = ptr2start; /* where to put the compacted data */
	if (flag) {
		x = 0; /* flag no transfer required yet */
	}
	for (ct=0; ct<NumColorBufs; ct++, ptrstart += ColorSize) {
		i = RowSize;
		ptr = ptrstart + i - 1;
		while (i > 0 && *ptr == 0) {
			i--;
			ptr--;
		}
		if (i != 0) { /* if data */
			i = (i + 7) / 8; /* calc size in bytes */
			ptr = ptrstart - NumStartCmd;
			if (NumStartCmd == MAXSTARTCMD) { /* if color */
				*ptr++ = 27;
				*ptr++ = 'K';
				*ptr++ = colorcodes[ct];	/* color */
			}
			*ptr++ = 27;
			/* CAS for AppleDMP - concert back to pixels now,
			 * pass 'G' (not 'g') and
			 * pixel count as 4 (not 3) digits
			 */
			i *= 8; /* convert back to pixels */
			*ptr++ = 'G';
			*ptr++ = ((i % 10000) / 1000) | '0';
			*ptr++ = ((i % 1000) / 100) | '0';
			*ptr++ = ((i % 100) / 10) | '0';
			*ptr   = (i % 10 ) | '0';	/* printout width */
			/* used to convert back to pixels here */
			*(ptrstart + i) = 13;		/* <cr> */
			i += NumTotalCmd;
			if (x != 0) { /* if must transfer data */
				/* get src start */
				ptr = ptrstart - NumStartCmd;
				do { /* transfer and update dest ptr */
					*ptr2++ = *ptr++;
				} while (--i);
			}
			else { /* no transfer required */
				ptr2 += i; /* update dest ptr */
			}
		}
		if (i != RowSize + NumTotalCmd) { /* if compacted or 0 */
			x = 1; /* flag that we need to transfer next time */
		}
	}
	if (PED->ped_YDotsInch == 72) {
		y *= 2; /* convert from 72nds to 144ths */
	}
	*ptr2++ = 27;
	*ptr2++ = 'T';
	*ptr2++ = (y / 10) | '0';
	*ptr2++ = (y % 10) | '0'; /* set spacing to y/144 */
	*ptr2++ = 10; /* lf */
	return(ptr2);
}

ClearAndInit(ptr)
UBYTE *ptr;
{
	ULONG *lptr, i, j;

	/*
		Note : Since 'NumTotalCmd + NUMLFCMD' is > 3 bytes if is safe
		to do the following to speed things up.
	*/
	i = TwoBufSize - NumTotalCmd - NUMLFCMD;
	j = (ULONG)ptr;
	if (!(j & 1)) { /* if on a word boundary, clear by longs */
		i = (i + 3) / 4;
		lptr = (ULONG *)ptr;
		do {
			*lptr++ = 0;
		} while (--i);
	}
	else { /* clear by bytes */
		do {
			*ptr++ = 0;
		} while (--i);
	}
}
