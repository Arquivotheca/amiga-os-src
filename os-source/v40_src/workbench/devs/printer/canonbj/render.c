/**********************************************************/
/* Copyright   �1991 Wolf-Juergen Faust                   */
/* Am Dorfgarten 10                                       */
/* W-6000 Frankfurt 50   Canon BJ 10 driver               */
/* Germany                                                */
/* FIDO: 2:243/43.5 (Wolf Faust)                          */
/* UUCP: cbmvax.commodore.com!cbmehq!cbmger!venus!wfaust  */
/* Tel: +(49) 69 5486556                                  */
/*                                                        */
/* This File is NOT PART OF THE DISTRIBUTION package !!!  */
/*          DO NOT SPREAD THIS SOURCE FILE !!!            */
/*                                                        */
/* Versions required:                                     */
/* SAS/C 5.10a                                            */
/**********************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <devices/prtgfx.h>
#include <clib/all_protos.h>

#define	DB(x)	;

#define NUMSTARTCMD	6       /* # of cmd bytes before binary data */
#define NUMLFCMD	13        /* # of cmd bytes for linefeed + cr  */

#ifdef TURBO
int __stdargs RenderT(long,long,long,long);
int __stdargs RenderT(ct, x, y, status)
#else
int __stdargs Render(long,long,long,long);
int __stdargs Render(ct, x, y, status)
#endif
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	/* Don't change Transfer declaration !!!!!!!!! */
	extern void __regargs Transfer(struct PrtInfo *,UWORD, UBYTE *);
	extern char __regargs SetDensity(ULONG);
	extern void *  __regargs MemCopy(void *,void *,LONG);
	extern void __regargs MemClr(void *,LONG);

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;
	extern UBYTE charsize;

	static UWORD RowSize, BufSize, TotalBufSize;
	static UWORD dataoffset, dpi_code, white, mult;
	static UBYTE StartBuf[22];

	UBYTE  *ptr, *ptrstart, *ptr2, *ptr2start, flag;
	int  i, err;
	UWORD n, g, tmp;

	switch(status) {
	    case 0:  /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
			RowSize = x * ((PED->ped_YDotsInch == 360) ? 6 : 3);
			BufSize = RowSize + NUMLFCMD + NUMSTARTCMD;
			TotalBufSize = BufSize * 2;
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				dataoffset = NUMSTARTCMD;
				StartBuf[0]  = 0x12;	/* condensed fine off & set Pica mode */
				StartBuf[1]  = 0x1b;	/* esc */
				StartBuf[2]  = 'X';	  /* set left/right margin */
				StartBuf[3]  = 0x01;	/* left margin */
				StartBuf[4]  = (PD->pd_Preferences.PaperSize == W_TRACTOR) ? 136 : 80;	/* right margin */
				StartBuf[5]  = 0x1b;  /* esc */
				StartBuf[6]  = 'U';   /* unidirectional */
				StartBuf[7]  = 0x01;  /* on  */
				StartBuf[8]  = 0x1b;  /* ESC */
				StartBuf[9]  = 95;    /* overscore */
				StartBuf[10] = 0;     /* off */
				StartBuf[11] = 0x1b;  /* ESC */
				StartBuf[12] = '-';   /* underscore */
				StartBuf[13] = 0;     /* off */
				StartBuf[14] = 0x0d;  /* cr  */
				err = (*(PD->pd_PWrite))(StartBuf,15);
			}
			break;

	    case 1: /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- 0.
				y	- row # (0 to Height - 1).
			*/
			Transfer((struct PrtInfo *)ct, y, &PD->pd_PrintBuf[dataoffset]);
			err = PDERR_NOERR; /* all ok */
			break;

	    case 2: /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).
			*/

			/* white-space strip */
			ptr = ptrstart = &PD->pd_PrintBuf[dataoffset];
			ptr2start = ptr2 = ptrstart - NUMSTARTCMD;
			flag = n = g = 0;
			for(i=0; i<RowSize; i++)
			{
				if(!flag)
				{
					if (!(*ptr))
					{
						n++;
					} else
					{
						if (n<white)
						{
							g += n;
							g++;
						} else
						{
							g = (i % white)+1;
							tmp = (n / white) * mult;
							*ptr2++ = 27;  /* advance head horiz. tmp/120 inch */
							*ptr2++ = 100; /* d */
							*ptr2++ = tmp & 0xff;
							*ptr2++ = tmp >> 8;
						}
						n = 0;
						flag = 1;
					}
				} else
				{
					if (*ptr)
					{
						g++;
						if (n)
						{
							g += n;
							n = 0;
						}
					} else
					{
						n++;
						if (n>=white)
						{
							if(!( (g+n)%white ))
							{
								tmp = (g+n-white);
								*ptr2++ = 27;             /* high resolution graphics command */
								*ptr2++ = 91;             /* [ */
								*ptr2++ = 103;            /* g */
								*ptr2++ = (tmp+1) & 0xff; /* size */
								*ptr2++ = (tmp+1) >> 8;   /* size */
								*ptr2++ = dpi_code;       /* resolution mode */
								ptr2 = MemCopy(ptr2,ptr-g-n+1,tmp);
								flag = g = 0;
								n = white;
							}
						}
					}
				}
				ptr++;
			}
			if (flag)
			{
				tmp = ( (g+((PED->ped_YDotsInch == 360) ? 5 : 2)) / ((PED->ped_YDotsInch == 360) ? 6 : 3)) *  ((PED->ped_YDotsInch == 360) ? 6 : 3);    /* rows */
				*ptr2++ = 27;             /* ESC [ g n1 n2 m d1...dn high resolution graphics command */
				*ptr2++ = 91;             /* [ */
				*ptr2++ = 103;            /* g */
				*ptr2++ = (tmp+1) & 0xff; /* size */
				*ptr2++ = (tmp+1) >> 8;   /* size */
				*ptr2++ = dpi_code;       /* resolution mode */
				ptr2 = MemCopy(ptr2,ptr-g-n,tmp);
				DB (kprintf ("high-res : ESC [ g %ld %ld %ld\n", (LONG)((tmp+1)&0xff), (LONG)((tmp+1)>>8), (LONG)dpi_code));
			}
			*ptr2++ = 13; /* cr */
			*ptr2++ = 27;
			*ptr2++ = 91;
			*ptr2++ = 92;
			*ptr2++ = 04;
			*ptr2++ = 0;
			*ptr2++ = 0;
			*ptr2++ = 0;
			*ptr2++ = ((PED->ped_YDotsInch == 360) ? 0x68 : 0xb4);
			*ptr2++ = ((PED->ped_YDotsInch == 360) ? 1 : 0 );
			*ptr2++ = 27;  /* WARNING with BJ10 in Mode 1: there is no perforation skip! */
			*ptr2++ = 'J';
			*ptr2++ = y;
			err = (*(PD->pd_PWrite))(ptr2start, ptr2 - ptr2start);
			if (err == PDERR_NOERR)
			{
				dataoffset = ((dataoffset == NUMSTARTCMD) ? BufSize : 0) + NUMSTARTCMD;
			}
			break;

	    case 3: /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
			MemClr(&PD->pd_PrintBuf[dataoffset],BufSize - NUMSTARTCMD - NUMLFCMD);
			err = PDERR_NOERR; /* all ok */
			break;

	    case 4: /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel print */
			if (ct != PDERR_CANCEL) {
				StartBuf[0]  = 0x1b;   /* bidirectional */
				StartBuf[1]  = 'U';
				StartBuf[2]  = 0x00;
				StartBuf[3]  = 27;     /* set n/216 linefeed (default) */
				StartBuf[4]  = '[';
				StartBuf[5]  = 92;
				StartBuf[6]  = 04;
				StartBuf[7]  = 0;
				StartBuf[8]  = 0;
				StartBuf[9]  = 0;
				StartBuf[10]  = 0xd8;
				StartBuf[11] = 0x00;
				StartBuf[12] = 0x1b;   /* margins */
				StartBuf[13] = (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) ? '0' : '2';
				i=14;
				charsize = 0;
				/* restore preferences pitch and margins */
				if (PD->pd_Preferences.PrintPitch == ELITE) {
					StartBuf[i++] = 0x1b;      /* 12 cpi */
					StartBuf[i++] = 58;        /* ESC : = 12 cpi */
					charsize = 8;
				}
				else if (PD->pd_Preferences.PrintPitch == FINE) {
					StartBuf[i++] = 0x0f;      /* on */
					charsize = 16;
				}
				StartBuf[i++] = 0x1b;
				StartBuf[i++] = 'X';
				StartBuf[i++] = PD->pd_Preferences.PrintLeftMargin;
				StartBuf[i++] = PD->pd_Preferences.PrintRightMargin;
				StartBuf[i++] = 0x0d;        /* make changes activ with cr */

				if (((err = (*(PD->pd_PWrite))(StartBuf, i)) == PDERR_NOERR) && (PD->pd_Preferences.PaperType == SINGLE))
				{
					if (!(x & SPECIAL_NOFORMFEED))
					{
						/* eject paper */
						err = (*(PD->pd_PWrite)) ("\014", 1);
					}
				}

			}
			/* 	flag that there is no alpha data waiting that	*/
			/* 	needs a formfeed (since we just did one)	*/
			if (PD->pd_Preferences.PaperType == SINGLE) PED->ped_PrintMode = 0;
			/* wait for both buffers to empty */
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
			break;

		case 5:   /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			dpi_code = SetDensity(x & SPECIAL_DENSITYMASK);
			err = PDERR_NOERR; /* all ok */
			/* calc min white bytes needed for n/120 inch command*/
			switch(dpi_code)
			{
				case 11:
					white = 18;
					mult = 4;
					break;
				case 12:
					white = 18;
					mult = 2;
					break;
				case 13:
					white = 12;
					mult = 2;
					break;
				case 14:
					white = 18;
					mult = 2;
					break;
				case 16:
					white = 18;
					mult = 1;
					break;
				case 15:
					white = 1152; /* = 96/120 inch */
					mult = 96;
					break;
			}
			break;
	}
	return(err);
}
