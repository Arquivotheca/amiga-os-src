/*
	Okidata_292I driver.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMSTARTCMD	7	/* # of cmd bytes before binary data */
#define NUMENDCMD	1	/* # of cmd bytes after binary data */
#define NUMTOTALCMD 	(NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	5	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	4	/* max # of color buffers */

#define STARTLEN	11
#define LMARG		2
#define RMARG		3
#define DIREC		7

static ULONG TwoBufSize;
static UWORD RowSize, ColorSize, NumColorBufs, dpi_code, spacing;
static UWORD colorcodes[MAXCOLORBUFS];

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	UBYTE *CompactBuf();
	static ULONG BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS];
	/*
		00-03	\033Xlr	set left and right margin
		04-04	\r	carriage return
		05-07	\033U1	uni-directional mode
		08-10	\033#B	16-pin graphics
	*/
	static UBYTE StartBuf[STARTLEN] = "\033Xlr\r\033U1\033#B";
	UBYTE *ptr, *ptrstart;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x * 2;
			ColorSize = RowSize + NUMTOTALCMD;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
				colors[0] = ColorSize * 3; /* Black */
				colors[1] = ColorSize * 0; /* Yellow */
				colors[2] = ColorSize * 1; /* Magenta */
				colors[3] = ColorSize * 2; /* Cyan */
				colorcodes[0] = 4; /* Yellow */
				colorcodes[1] = 1; /* Magenta */
				colorcodes[2] = 2; /* Cyan */
				colorcodes[3] = 0; /* Black */
			}
			else { /* grey-scale or black&white */
				NumColorBufs = 1;
				colors[0] = ColorSize * 0; /* Black */
				colorcodes[0] = 0; /* Black */
			}
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			if (PED->ped_YDotsInch == 288) {
				TwoBufSize = BufSize * 2;
				TotalBufSize = BufSize * 4;
			}
			else {
				TwoBufSize = BufSize * 1;
				TotalBufSize = BufSize * 2;
			}
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY;
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
					reduce wavy vertical lines.
				*/
				StartBuf[LMARG] = 1; /* min left margin */
				StartBuf[RMARG] = 255; /* max right margin */
				StartBuf[DIREC] = '1'; /* uni-direct mode */
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
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
			x = (PED->ped_YDotsInch == 288) ? 1 : y;
			ptrstart = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
			ptr = CompactBuf(ptrstart + NUMSTARTCMD, ptrstart,
				x, 1);
			if (PED->ped_YDotsInch == 288 && y > 1) {
				ptr = CompactBuf(&PD->pd_PrintBuf[
					dataoffset + BufSize], ptr, y - 1, 0);
			}
			err = (*(PD->pd_PWrite))(ptrstart, ptr - ptrstart);
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == NUMSTARTCMD ?
					TwoBufSize : 0) + NUMSTARTCMD;
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
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel print */
			if (ct != PDERR_CANCEL) {
				/* restore preferences margins */
				StartBuf[LMARG] =
					PD->pd_Preferences.PrintLeftMargin;
				StartBuf[RMARG] =
					PD->pd_Preferences.PrintRightMargin;
				StartBuf[DIREC] = '0'; /* bi-directional */
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5 :  /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			/* kludge for sloppy tractor mechanism */
			spacing = PD->pd_Preferences.PaperType == SINGLE ?
				1 : 0;
			dpi_code = SetDensity(x & SPECIAL_DENSITYMASK);
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
			i = (i + 1) / 2; /* convert to # of pixels */
			ptr = ptrstart - NUMSTARTCMD;
			*ptr++ = 27;
			*ptr++ = 'r';
			*ptr++ = colorcodes[ct];	/* color */
			*ptr++ = 27;
			*ptr++ = dpi_code;		/* density */
			*ptr++ = i & 0xff;
			*ptr++ = i >> 8;		/* size */
			i *= 2;
			*(ptrstart + i) = 13;		/* <cr> */
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
		}
		if (i != RowSize + NUMTOTALCMD) { /* if compacted or 0 */
			x = 1; /* flag that we need to transfer next time */
		}
	}
	if (PED->ped_YDotsInch == 144) {
		y *= 2; /* convert from 288ths to 144ths */
		y -= spacing; /* kludge for sloppy tractor mechanism */
	}
	*ptr2++ = 13; /* cr */
	*ptr2++ = 27;
	*ptr2++ = '%';
	*ptr2++ = '4';
	*ptr2++ = y; /* y/288 lf */
	return(ptr2);
}


ClearAndInit(ptr)
UBYTE *ptr;
{
	ULONG *lptr, i, j;

	/*
		Note : Since 'NUMTOTALCMD + NUMLFCMD' is > 3 bytes if is safe
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
	else { /* clear by bytes */
		do {
			*ptr++ = 0;
		} while (--i);
	}
}
