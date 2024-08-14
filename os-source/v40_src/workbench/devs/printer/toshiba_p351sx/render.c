/*
	Render routine for Toshiba_P351SX driver.
	David Berezowski - April/88.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

#define NUMSTARTCMD	8	/* # of cmd bytes before binary data */
#define NUMENDCMD	1	/* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	9	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	4	/* max # of color buffers */

#define STARTLEN	20
#define	LPI		4
#define DIREC		6
#define	LMARG		9
#define	RMARG		15

static ULONG TwoBufSize;
static UWORD RowSize, ColorSize, NumColorBufs, dpi_code;
static UWORD colorcodes[MAXCOLORBUFS]; /* printer color codes */

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;
	extern UBYTE HexTable[];

	UBYTE *CompactBuf();
	static ULONG BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color ptrs */
	/*
		00-01	\033b		select black ribbon
		02-04	\033\036\007	select 8 lpi
		05-06	\033<		select bi-directional mode
		07-10	\033CLL		move to position 'LL'
		11-12	\0339		set left margin
		13-16	\033CRR		move to position 'RR'
		17-18	\0330		set right margin
		19-19	\r		carriage return
	*/
	static UBYTE StartBuf[STARTLEN] =
		"\033b\033\036\007\033<\033CLL\0339\033CRR\0330\r";
	UBYTE *ptr, *ptrstart;
	int i, err;

	switch(status) {
		case 0:  /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x * 4;
			/* size of each color buf */
			ColorSize = RowSize + NUMTOTALCMD;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
				colors[0] = ColorSize * 3; /* Black */
				colors[1] = ColorSize * 0; /* Yellow */
				colors[2] = ColorSize * 1; /* Magenta */
				colors[3] = ColorSize * 2; /* Cyan */
				colorcodes[0] = 'y'; /* Yellow */
				colorcodes[1] = 'm'; /* Magenta */
				colorcodes[2] = 'c'; /* Cyan */
				colorcodes[3] = 'b'; /* Black */
			}
			else { /* grey-scale or black&white */
				NumColorBufs = 1;
				colors[0] = ColorSize * 0; /* Black */
				colorcodes[0] = 'b'; /* Black */
			}
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			if (PED->ped_YDotsInch == 360) {
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
				dataoffset = NUMSTARTCMD;
				/*
					This printer prints graphics within its
					text margins.  This code sets the
					left and right margins to their minimum
					and maximum values (respectively).  A
					carriage return is sent so that the
					print head is at the leftmost position
					as this printer starts printing from
					the print head's position.  The printer
					is put into unidirectional mode to
					reduce wavy vertical lines.  The set
					black ribbon cmd assures that the
					text will be black after the dump.  The
					set lpi cmd assures that the printer
					will return to the proper text line
					spacing after the dump.
				*/
				StartBuf[DIREC] = '>'; /* uni-direc */
				i = 0; /* minimum left margin */
				StartBuf[LMARG] = HexTable[i / 10];
				StartBuf[LMARG + 1] = HexTable[i % 10];
				i = 159; /* maximum right margin */
				StartBuf[RMARG] = HexTable[i / 10];
				StartBuf[RMARG + 1] = HexTable[i % 10];
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			break;

		case 1: /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1).
			*/
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors,
				BufSize);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2: /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).

				For each color buf, scan backwards through
				the buffer for the first non-zere data byte.
				Compact up to this point, then send the entire
				compacted buffer in one go.
			*/
			x = (PED->ped_YDotsInch == 360) ? 1 : y * 2 / 3;
			ptrstart = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
			ptr = CompactBuf(ptrstart + NUMSTARTCMD, ptrstart,
				x, 1);
			if (PED->ped_YDotsInch == 360 && y > 1) {
				ptr = CompactBuf(&PD->pd_PrintBuf[
					dataoffset + BufSize], ptr,
					(y * 2 + 3) / 6 - 1, 0);
			}
			err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NUMSTARTCMD ?
					TwoBufSize : 0) + NUMSTARTCMD;
			}
			break;

		case 3: /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
			ClearAndInit(&PD->pd_PrintBuf[dataoffset]);
			err = PDERR_NOERR; /* all ok */
			break;

		case 4: /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel print */
			if (ct != PDERR_CANCEL) {
				StartBuf[LPI] = (PD->pd_Preferences.
					PrintSpacing == EIGHT_LPI) ? 7 : 9;
				StartBuf[DIREC] = '<'; /* bi-direc */
				i = PD->pd_Preferences.PrintLeftMargin - 1;
				StartBuf[LMARG] = HexTable[i / 10];
				StartBuf[LMARG + 1] = HexTable[i % 10];
				i = PD->pd_Preferences.PrintRightMargin - 1;
				if (i > 159) {
					i = 159;
				}
				StartBuf[RMARG] = HexTable[i / 10];
				StartBuf[RMARG + 1] = HexTable[i % 10];
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5:   /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag.
				y	- 0.
			*/
			dpi_code = SetDensity(x & SPECIAL_DENSITYMASK);
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}

/*
	For each color buf, scan backwards through the buffer
	for the first non-zere data byte.  Compact up to this
	point.  Return this last point.
*/
UBYTE *CompactBuf(ptrstart, ptr2start, y, flag)
UBYTE *ptrstart, *ptr2start;
long y;
int flag;
{
	static int x;
	UBYTE *ptr, *ptr2;
	long ct;
	int i;

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
			/* convert to # of pixels */
			i = (i + 3) / 4;
			ptr = ptrstart - NUMSTARTCMD;
			*ptr++ = 27;
			*ptr++ = colorcodes[ct]; /* color */
			*ptr++ = 27;
			*ptr++ = dpi_code; /* density */
			*ptr++ = (i / 1000) | '0';
			*ptr++ = ((i % 1000) / 100) | '0';
			*ptr++ = ((i % 100) / 10) | '0';
			*ptr = (i % 10 ) | '0'; /* width */
			i *= 4; /* convert to # of bytes */
			*(ptrstart + i) = 13; /* cr */
			i += NUMTOTALCMD;
			if (x != 0) { /* if must transfer */
				/* get src start */
				ptr = ptrstart - NUMSTARTCMD;
				/* xfer and update dest ptr */
				do {
					*ptr2++ = *ptr++;
				} while (--i);
			}
			else { /* no transfer required */
				ptr2 += i; /* update dest ptr */
			}
		}
		/* if compacted or 0 */
		if (i != RowSize + NUMTOTALCMD) {
			/* we need to transfer next time */
			x = 1;
		}
	}
	*ptr2++ = 13; /* cr */
	*ptr2++ = 27;
	*ptr2++ = '~';
	*ptr2++ = 'V';
	*ptr2++ = '0';
	*ptr2++ = '3';
	*ptr2++ = y / 256 + '@';
	*ptr2++ = (y % 256) / 16 + '@';
	*ptr2++ = (y % 16) + '@';
	return(ptr2);
}

ClearAndInit(ptr)
UBYTE *ptr;
{
	ULONG *lptr, i, j;

	/*
		Note: Since 'NUMTOTAL + NUMLFCMD' is > 3 bytes it is safe
		to do the following to speed things up.
	*/
	i = TwoBufSize - NUMTOTALCMD - NUMLFCMD;
	j = (ULONG)ptr;
	if (!(j & 1)) { /* if on a word boundary, clear by longs */
		i = (i + 3) / 4;
		lptr = (ULONG *)ptr;
		do {
			*lptr++ = 0;
		} while (--i);
	}
	else { /* clear bytes */
		do {
			*ptr++ = 0;
		} while (--i);
	}
}
