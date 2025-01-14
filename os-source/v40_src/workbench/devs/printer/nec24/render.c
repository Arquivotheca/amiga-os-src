/*** nec24/render.c ***************************************************
 *
 * render.c -- graphics rendering
 *
 *	$Id: render.c,v 1.6 91/07/08 10:46:10 darren Exp $
 *
 *	Copyright (c) 1988,1991 Commodore-Amiga, Inc.
 *	All Rights Reserved
 *
 **********************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <devices/printer.h>
#include <devices/prtbase.h>
#include <devices/prtgfx.h>

#include <intuition/preferences.h>

/*
	Nec 24-pin (P5, P6, P7, P9, P2200) driver.
	David Berezowski - March/88.
*/

#define DEBUG0	0

#define NUMSTARTCMD	8	/* # of cmd bytes before binary data */
#define NUMENDCMD	1	/* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)	/* total of above */
#define NUMLFCMD	5	/* # of cmd bytes for linefeed */
#define MAXCOLORBUFS	4	/* max # of color buffers */

#define STARTLEN	21
#define PITCH		1
#define CONDENSED	2
#define LMARG		8
#define RMARG		11
#define DIREC		15
#define LPI		20

static ULONG TwoBufSize;
static UWORD RowSize, ColorSize, NumColorBufs, dpi_code;
static UWORD colorcodes[MAXCOLORBUFS];

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	UBYTE *CompactBuf();
	static ULONG BufSize, TotalBufSize, dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color ptrs */
	/*
		00-01	\003P		set pitch (10 or 12 cpi)
		02-02	\022		set condensed fine (on or off)
		03-05	\033W\000	enlarge off
		06-08	\033ln		set left margin to n
		09-11	\033Qn		set right margin to n
		12-12	\015		carriage return
		13-15	\033U1		set uni-directional mode
		16-18	\033r\000	black text
		19-20	\0330		8 lpi spacing
	*/
	static UBYTE StartBuf[STARTLEN] =
		"\033P\022\033W\000\033ln\033Qn\015\033U1\033r\000\0330";
	UBYTE *ptr, *ptrstart;
	int i, err;

    switch(status) {
	    case 0:  /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x * 3;
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
			if (PED->ped_YDotsInch == 360) {
				TwoBufSize = BufSize * 2;
				TotalBufSize = BufSize * 4;
			}
			else {
				TwoBufSize = BufSize * 1;
				TotalBufSize = BufSize * 2;
			}
#if DEBUG0
			kprintf("RS=%ld, CS=%ld, BS=%ld, 2BS=%ld, TBS=%ld\n",
				RowSize, ColorSize, BufSize,
				TwoBufSize, TotalBufSize);
#endif DEBUG0
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				dataoffset = NUMSTARTCMD;
				/*
					This printer prints graphics within its
					text margins.  This code makes sure the
					printer is in 10 cpi and then sets the
					left and right margins to their minimum
					and maximum values (respectively).  A
					carriage return is sent so that the
					print head is at the leftmost position
					as this printer starts printing from
					the print head's position.  The printer
					is put into unidirectional mode to
					reduce wavy vertical lines.
				*/
				StartBuf[PITCH] = 'P'; /* 10 cpi */
				StartBuf[CONDENSED] = '\022'; /* off */
				/* left margin of 1 */
				StartBuf[LMARG] = 0;
				/* right margin of 80 or 136 (or other Ax size */
				StartBuf[RMARG] = PED->ped_MaxColumns;
				/* uni-directional mode */
				StartBuf[DIREC] = '1';
				err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
			}
			break;

	    case 1: /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1)
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
			*/
			x = (PED->ped_YDotsInch == 360) ? 1 : y;
			ptrstart = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
			ptr = CompactBuf(ptrstart + NUMSTARTCMD, ptrstart,
				x, 1);
			if (PED->ped_YDotsInch == 360 && y > 1) {
				ptr = CompactBuf(&PD->pd_PrintBuf[
					dataoffset + BufSize], ptr, y - 1, 0);
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
				/* restore preferences pitch and margins */
				if (PD->pd_Preferences.PrintPitch == ELITE) {
					StartBuf[PITCH] = 'M'; /* 12 cpi */
				}
				else if (PD->pd_Preferences.PrintPitch == FINE) {
					StartBuf[CONDENSED] = '\017'; /* on */
				}
				StartBuf[LMARG] =
					PD->pd_Preferences.PrintLeftMargin - 1;
				StartBuf[RMARG] =
					PD->pd_Preferences.PrintRightMargin;
				StartBuf[DIREC] = '0'; /* bi-directional */
				StartBuf[LPI] = (PD->pd_Preferences.
					PrintSpacing == EIGHT_LPI) ? '0' : '2';
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
				ct	- 0.
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
			i = (i + 2) / 3; /* convert to # of pixels */
			ptr = ptrstart - NUMSTARTCMD;
			*ptr++ = 27;
			*ptr++ = 'r';
			*ptr++ = colorcodes[ct];	/* color */
			*ptr++ = 27;
			*ptr++ = '*';
			*ptr++ = dpi_code;			/* density */
			*ptr++ = i & 0xff;
			*ptr++ = i / 256;			/* size */
			i *= 3; /* convert back to # of bytes used */
			*(ptrstart + i) = 13; /* cr */
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
	*ptr2++ = 13; /* cr */
	if (PED->ped_YDotsInch == 360) {
		*ptr2++ = 28;
		*ptr2++ = '3';
		*ptr2++ = y; /* set to y/360 lf */
		*ptr2++ = 10; /* lf */
	}
	else {
		*ptr2++ = 27;
		*ptr2++ = '3';
		*ptr2++ = y; /* set to y/180 lf */
		*ptr2++ = 10; /* lf */
	}
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
