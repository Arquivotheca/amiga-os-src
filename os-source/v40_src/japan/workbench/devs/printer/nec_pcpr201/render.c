/*
 *  render.c
 *
 */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <devices/prtgfx.h>

#include <clib/exec_protos.h>

/**********    debug macros     **********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
#define bug kprintf
#if MYDEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

#define NUMSTARTCMD  6                       /* # of cmd bytes before binary data */
#define NUMENDCMD    0                       /* # of cmd bytes after binary data (cr) */
#define NUMTOTALCMD  (NUMSTARTCMD+NUMENDCMD) /* total of above */
#define NUMLFCMD     6                       /* # of cmd bytes for linefeed (cr+lf) */
#define MAXCOLORBUFS 4                       /* max # of color buffers */

#define STARTLEN     20
#define ENDLEN       4

#define LMARG        5
#define RMARG        10

extern void SetDensity(ULONG);
extern void Transfer(long, UWORD, UBYTE *, UWORD *);
extern int numberString(UBYTE, int, char *);
void Swap(UBYTE *, long);

int Render(long ct, long x, long y, long status)
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD RowSize, ColorSize, BufSize, TotalBufSize;
	static UWORD dataoffset;
	static UWORD colors[MAXCOLORBUFS]; /* color ptrs */
	static UWORD colorcodes[MAXCOLORBUFS]; /* printer color codes */
	static UWORD NumColorBufs; /* actually number of color buffers req. */
	static UWORD thous, huns, tens;	/* used to program buffer size */
	/*
		00-00	\012		line feed
		01-02	\033H       HD pika
		03-07   \033L000	left margin
		08-12	\033/000	right margin
		13-14	\034A       Kanji width:3/20, size:10.5 point
		15-18	\033T18		18/120 inchi line feed
		19-19	\015		cr
	*/
	static char StartCmd[] = "\012\033H\033L000\033/000\034A\033T18\015";
//	static char DrawCmd[]  = "\033J0000";
	/*
		00-01	\033A		1/6 inchi line feed
		02-02	\015		cr
		03-03	\012		line feed
	*/
	static char EndCmd[]   = "\033A\015\012";

	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start, **dummy;

	int i, err;

    switch(status)
	{
	    case 0:  /* Master Initialization */
D(bug("status:0/x=%ld,y=%ld,",x,y));
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x * 3;
			ColorSize = RowSize + NUMTOTALCMD;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR)
			{
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
			else	/* grey-scale or black&white */
			{
				NumColorBufs = 1;
				colors[0] = ColorSize * 0; /* Black */
				colorcodes[0] = 0; /* Black */
			}
			BufSize = ColorSize * NumColorBufs + NUMLFCMD;
			TotalBufSize = BufSize * 2;

			D(bug("BufSize=%ld,TotalBufSize=%ld\n",BufSize,TotalBufSize));

			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL)
			{
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else
			{
				dataoffset = NUMSTARTCMD;
				numberString(PD->pd_Preferences.PrintLeftMargin, LMARG, StartCmd);
				numberString(PD->pd_Preferences.PrintRightMargin, RMARG, StartCmd);
				err = (*(PD->pd_PWrite))(StartCmd, STARTLEN);
			}
			break;

	    case 1: /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1).
			*/

//D(bug("status:1/y=%02ld,dataoffset=%ld,",y,dataoffset));
#if 0
			{
				struct PrtInfo *PInfo;
				PInfo = (struct PrtInfo *)ct;
				D(bug("pi=%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx,",
					PInfo->pi_ScaleX[0],PInfo->pi_ScaleX[1],
					PInfo->pi_ScaleX[2],PInfo->pi_ScaleX[3],
					PInfo->pi_ScaleX[4],PInfo->pi_ScaleX[5],
					PInfo->pi_ScaleX[6],PInfo->pi_ScaleX[7]));
			}
#endif

			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors);
#if 0
			D(bug("PBuf=%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx\n",
				PD->pd_PrintBuf[dataoffset+0],PD->pd_PrintBuf[dataoffset+1],
				PD->pd_PrintBuf[dataoffset+2],PD->pd_PrintBuf[dataoffset+3],
				PD->pd_PrintBuf[dataoffset+4],PD->pd_PrintBuf[dataoffset+5],
				PD->pd_PrintBuf[dataoffset+6],PD->pd_PrintBuf[dataoffset+7]));
#endif
			err = PDERR_NOERR; /* all ok */
			break;

	    case 2: /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).
			*/
//D(bug("status:2/y=%ld,",y));
			/* white-space strip */
			ptrstart = &PD->pd_PrintBuf[dataoffset];
			ptr2start = ptr2 = ptrstart - NUMSTARTCMD;
			x = 0; /* flag no transfer required yet */

			for (ct=0; ct<NumColorBufs;
				ct++, ptrstart += ColorSize)
			{
				i = RowSize;
				ptr = ptrstart + i - 1;
//D(bug("ptr=%ld,",ptr));
				while (i > 0 && *ptr == 0)
				{
					i--;
					ptr--;
				}
//D(bug("datasize=%ld\n",i));
				if (i != 0)	/* if data */
				{
					/* convert to # of pixels */
					i = (i + 2) / 3;
					ptr = ptrstart - NUMSTARTCMD;

					/* set drawing info data */
					*ptr++ = 0x1b;
					*ptr++ = 'J';
					*ptr++ = (thous = i / 1000) | '0';
					*ptr++ = (huns = (i - thous * 1000) / 100) | '0';
					*ptr++ = (tens = (i - thous * 1000 - huns * 100) / 10) | '0';
					*ptr++ = (i - thous * 1000 - huns * 100 - tens * 10) | '0';

					i *= 3; /* back to # of bytes used */
					*(ptrstart + i) = 13; /* cr */
					i += NUMTOTALCMD;
					/* if must transfer data */
					if (x != 0)
					{
						/* get src start */
						ptr = ptrstart - NUMSTARTCMD;
                                                /* otherwise Lattice looses
                                                   track of the pointer.... */
                                                dummy = &ptr;
						/* xfer and update dest ptr */
						do
						{
							*ptr2++ = *ptr++;
						} while (--i);
					}
					else	/* no transfer required */
					{
						/* update dest ptr */
						ptr2 += i;
					}
				}
				/* if compacted or 0 */
				if (i != RowSize + NUMTOTALCMD)
				{
					/* we need to transfer next time */
					x = 1;
				}
			}
			*ptr2++ = 13; /* cr */
			*ptr2++ = 0x1b;	/* set lf size */
			*ptr2++ = 'T';
			y = (y + 3)/ 3 * 2;	/* y/180 lf = ((y+3(?))/3*2)/120 lf */
			*ptr2++ = (tens = y / 10) | '0';
			*ptr2++ = (y - tens * 10) | '0';
			*ptr2++ = 10;	/* lf */

			Swap(ptr2start + NUMSTARTCMD, i - NUMSTARTCMD - 1);

			err = (*(PD->pd_PWrite))(ptr2start, ptr2 - ptr2start);

/*
			D(bug("totalsize=%ld ,",ptr2 - ptr2start));
			D(bug("PBuf=%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx\n",
				ptr2start[0],ptr2start[1],ptr2start[2],ptr2start[3],
				ptr2start[4],ptr2start[5],ptr2start[6],ptr2start[7]));
*/
			if (err == PDERR_NOERR)
			{
				dataoffset = (dataoffset == NUMSTARTCMD ?
					BufSize : 0) + NUMSTARTCMD;
			}
			break;

	    case 3: /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
//D(bug("status:3\n"));
			ptr = &PD->pd_PrintBuf[dataoffset];
			i = BufSize - NUMTOTALCMD - NUMLFCMD;
			do
			{
				*ptr++ = 0;
			} while (--i);
			err = PDERR_NOERR; /* all ok */
			break;

	    case 4: /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			if (ct != PDERR_CANCEL)
			{
				/* end raster graphics */
				if ((err = (*(PD->pd_PWrite))(EndCmd, ENDLEN)) == PDERR_NOERR)
				{
D(bug("cr and linefeed..."));
					/* if want to unload paper */
					if (!(x & SPECIAL_NOFORMFEED))
					{
D(bug("eject..."));
						/* eject paper */
						err = (*(PD->pd_PWrite))("\014", 1);
					}
				}
			}
			/*
				flag that there is no alpha data waiting that
				needs a formfeed (since we just did one)
			*/
			PED->ped_PrintMode = 0;
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL)
			{
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5:   /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/

			/* select density */
			SetDensity(x & SPECIAL_DENSITYMASK);
D(bug("status:5/XDotsInch=%ld,YDotsInch=%ld,MaxXDots = %ld\n",PED->ped_XDotsInch,PED->ped_YDotsInch,PED->ped_MaxXDots));
			break;
	}
	return(err);
}

void Swap(UBYTE *ptr, long count)
{
//	UBYTE *psave = ptr;
    UBYTE c;
    int i;

	for (i = 0; i < count; i++, *ptr++)
	{
		c = *ptr;
		*ptr = 0;
		if (c & 0x80) *ptr |= 0x01;
		if (c & 0x40) *ptr |= 0x02;
		if (c & 0x20) *ptr |= 0x04;
		if (c & 0x10) *ptr |= 0x08;
		if (c & 0x08) *ptr |= 0x10;
		if (c & 0x04) *ptr |= 0x20;
		if (c & 0x02) *ptr |= 0x40;
		if (c & 0x01) *ptr |= 0x80;
	}
//	ptr = psave;
}
