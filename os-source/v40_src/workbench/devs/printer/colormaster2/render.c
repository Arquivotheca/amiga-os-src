/*
	CALCOMP ColorMaster2 driver (requires LOTS of memory).
	David Berezowski - Sept/87.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMENDCMD	0	/* # of cmd bytes after binary data */
#define MAXCOLORBUFS	3	/* max # of color buffers */
#define STARTLEN	4	/* # of start cmd bytes */
#define ORGLEN		7	/* # of origin cmd bytes */

#define DEBUG0	0
#define DEBUG1	0
#define DEBUG2	0
#define DEBUG3	0
#define DEBUG4	0
#define DEBUG5	0
#define DEBUG7	0

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static ULONG ColorSize, BufSize, TotalBufSize, dataoffset;
	static ULONG colors[MAXCOLORBUFS]; /* color ptrs */
	static UWORD DataSize, RowSize, NumDiscardCmd, YOrg;
	static UWORD NumColorBufs, NumFFCmd, Aspect, NumStartCmd, NumTotalCmd;
	static UWORD oldprintaspect, oldxdpi, oldydpi;
	static ULONG oldxmax, oldymax;
	/*
		00-00	\002	enter raster gfx mode
		01-01	\000	required padding after cmd byte
		02-02	\000	enter landscape/portrait raster gfx mode
		03-03	\000	required padding after cmd byte
	*/
	static UBYTE StartCmd[STARTLEN] = "\002\000\000\000";
	/*
		00-01	\033\117	set origin
		02-03	\000\000	y origin
		04-05	\000\000	x origin
		06-06	\000		required padding after cmd byte
	*/
	static UBYTE OrgCmd[ORGLEN] = "\033\117\000\000\000\000\000";
	UBYTE *ptr, *ptrstart, *ptr2, *ptr2start, *ptr3;
	int i, j, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
#if DEBUG0
			kprintf("0: ct=%lx, x=%ld, y=%ld, YOrg=%ld\n",
				ct, x, y, YOrg);
#endif DEBUG0
			StartCmd[2] = (Aspect == ASPECT_HORIZ) ? 0x50 : 0x4c;
			if (Aspect != ASPECT_HORIZ) {
				NumDiscardCmd = 0;
			}
			NumStartCmd = 4 + NumDiscardCmd;
			NumTotalCmd = NumStartCmd + NUMENDCMD;
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				NumColorBufs = MAXCOLORBUFS;
				NumFFCmd = 1;
			}
			else {
				NumColorBufs = 1;
				NumFFCmd = 0;
			}
			DataSize = (x + 7) / 8;
			RowSize = DataSize + NumTotalCmd;
			ColorSize = RowSize * y + NumFFCmd;
			BufSize = ColorSize * NumColorBufs;
			TotalBufSize = BufSize;
			for (i=0; i<NumColorBufs; i++) {
				colors[i] = ColorSize * i;
			}
#if DEBUG0
			kprintf("DataSize=%ld, RowSize=%ld, ColorSize=%ld\n",
				DataSize, RowSize, ColorSize);
			kprintf("BufSize=%ld, TotalBufSize=%ld, y=%ld\n",
				BufSize, TotalBufSize, y);
#endif DEBUG0
			PD->pd_PrintBuf = AllocMem(TotalBufSize,
				MEMF_PUBLIC|MEMF_CLEAR);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
		}
			else {
				dataoffset = NumStartCmd;
				if (YOrg == 0) { /* if first dump */
					PrinterReady();
					/* enter raster graphics mode */
					err = (*(PD->pd_PWrite))(StartCmd,
						STARTLEN);
					PrinterReady();
				}
				else { /* not first, set origin for succs. */
					/* set origin */
					OrgCmd[2] = YOrg / 256;
					OrgCmd[3] = YOrg & 255;
					PrinterReady();
					err = (*(PD->pd_PWrite))
						(OrgCmd, ORGLEN);
					PrinterReady();
				}
				YOrg += y;
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- color code.
				y	- row # (0 to Height - 1).
			*/
#if DEBUG1
			kprintf("1: ct=%lx, x=%ld, y=%ld, do=%ld, y*RS=%ld\n",
				ct, x, y, dataoffset, y * RowSize);
#endif DEBUG1
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], colors,
				RowSize);
			err = PDERR_NOERR; /* all ok */
			break;

		case 2 : /* Dump Buffer to Printer */
			/*
				ct	- 0.
				x	- 0.
				y	- # of rows sent (1 to NumRows).
			*/
#if DEBUG2
			kprintf("2: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG2
			ptr3 = &PD->pd_PrintBuf[dataoffset];
			ptr2start = ptr2 = ptr3 - NumStartCmd;
			x = 0; /* flag no transfer required yet */
			for (ct=0; ct<NumColorBufs; ct++, ptr3 += ColorSize) {
				ptrstart = ptr3;
				for (j=0; j<y; j++, ptrstart += RowSize) {
					i = DataSize;
					ptr = ptrstart + i - 1;
					while (i > 0 && *ptr == 0) {
						i--;
						ptr--;
					}
					if (i != 0) { /* if data */
						ptr = ptrstart - NumStartCmd;
						*ptr++ = 27;
						*ptr++ = 'K';
						*ptr++ = (i + NumDiscardCmd)
							>> 8;
						*ptr = (i + NumDiscardCmd)
							& 0xff;
						i += NumTotalCmd;
						/* if must transfer data */
						if (x != 0) {
							/* get src start */
							ptr = ptrstart -
								NumStartCmd;
							/* xfer and update...*/
							do { /* dest ptr */
								*ptr2++ =
								*ptr++;
							} while (--i);
						}
						/* no transfer required */
						else {
							/* update dest ptr */
							ptr2 += i;
						}
					}
					else { /* blank line, no data */
						*ptr2++ = 10; /* lf */
					}
					/* if compacted or 0 */
					if (i != DataSize + NumTotalCmd) {
						/* we need to xfer next time */
						x = 1;
					}
				}
				if (NumFFCmd && ct < NumColorBufs - 1) {
					/* advance to next color panel */
					*ptr2++ = 12;
				}
			}
#if DEBUG2
			kprintf("2: sending %ld bytes to printer\n",
				ptr2 - ptr2start);
#endif DEBUG2
			/*
				Under V1.3 the parallel device generates
				continuous interrupts and locks the machine
				for the duration of the write.  That is why
				I've broken the write up into n byte blocks.
			*/
			i = ptr2 - ptr2start;
			do {
				j = i < 1000 ? i : 1000;
				PrinterReady();
				err = (*(PD->pd_PWrite))(ptr2start, j);
				ptr2start += j;
				i -= j;
			} while (err == PDERR_NOERR && i > 0);
			break;

		case 3 : /* Clear and Init Buffer */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
#if DEBUG3
			kprintf("3: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG3
			/* the clear is handled by 'MEMF_CLEAR' in case '0' */
			err = PDERR_NOERR; /* all ok */
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel the print*/
			if (ct != PDERR_CANCEL) {
				/* if do not want to unload paper */
				if (x & SPECIAL_NOFORMFEED) {
					/* if color */
					if (NumColorBufs == MAXCOLORBUFS) {
						PrinterReady();
						/* Advance Color Panel */
						err = (*(PD->pd_PWrite))
							("\014\000", 2);
						PrinterReady();
					}
				}
				else { /* eject paper */
					YOrg = 0;
					PrinterReady();
					/* End of Transmission */
					err = (*(PD->pd_PWrite))("\004\000",
						2);
					PrinterReady();
				}
			}
			(*(PD->pd_PBothReady))();
			if (PD->pd_PrintBuf != NULL) {
				FreeMem(PD->pd_PrintBuf, TotalBufSize);
			}
#if DEBUG4
			kprintf("4: ct=%lx, x=%ld, y=%ld, YOrg=%ld\n",
				ct, x, y, YOrg);
#endif DEBUG4
			break;

		case 5 :  /* Pre-Master Initialization */
			/*
				ct	- 0 or pointer to IODRPReq structure.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
			if (ct != 0) { /* if not case 5 open */
				/* save original value */
				oldprintaspect =
					PD->pd_Preferences.PrintAspect;
				oldxmax = PED->ped_MaxXDots;
				oldymax = PED->ped_MaxYDots;
				oldxdpi = PED->ped_XDotsInch;
				oldydpi = PED->ped_YDotsInch;

				if ((Aspect = PD->pd_Preferences.PrintAspect)
					== ASPECT_VERT) { /* if sideways */
					/*
					   This is a neat trick we play.
					   Horizontal printing is MUCH
					   faster than vertical printing
					   since the printer device uses
					   the blitter to read an entire
					   row of pixel data.  This printer
					   has the ability to take a
					   horizontally oriented picture
					   and print it vertically.  So
					   we fool the printer device into
					   thinking that we are doing a
					   horizontal printout by temporarily
					   changing the PrintAspect to
					   ASPECT_HORIZ.  We then tell the
					   printer to print the image
					   vertically.  We MUST put code in
					   case 7 to undo what we've done
					   here to keep things in sync.
					*/
					PD->pd_Preferences.PrintAspect =
						ASPECT_HORIZ; /* force horz */
					/* swap x and y stuff */
					PED->ped_MaxXDots = oldymax;
					PED->ped_MaxYDots = oldxmax;
					PED->ped_XDotsInch = oldydpi;
					PED->ped_YDotsInch = oldxdpi;
				}
			}
			else { /* case 5 open */
				YOrg = 0;
				if (PD->pd_Preferences.PaperSize ==
					W_TRACTOR) {
					/* CalComp_ColorView-5912 */
					/* 11 x 17 inch paper (B/A3 size) */
					PED->ped_MaxXDots = 2048;
					PED->ped_MaxYDots = 3200;
					NumDiscardCmd = 0;
				}
				else {
					/* 8.5 x 11 inch paper (A/A4 size) */
					PED->ped_MaxXDots = 1600;
					PED->ped_MaxYDots = 2000;
					NumDiscardCmd = 8;
				}
			}
#if DEBUG5
			kprintf("5: ct=%lx, x=%ld, y=%ld, YOrg=%ld\n",
				ct, x, y, YOrg);
#endif DEBUG5
			PED->ped_NumRows = PED->ped_MaxYDots;
			err = PDERR_NOERR; /* all ok */
			break;

		/*
			This is a NEW case that was added after the offical
			V1.3 release of the printer device and this driver.
			Although this driver will work with the official
			V1.3 printer device (version 35.562); it may not
			print full size vertical pictures correctly since
			it does not contain a 'case 7' call.  The printer
			device version 35.564 (and beyond) contains a
			'case 7' call.
		*/
		case 7 : /* Restore any values we may have temp. changed */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
#if DEBUG7
			kprintf("7: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG7
			/* restore what we may have altered in case 5 */
			PD->pd_Preferences.PrintAspect = oldprintaspect;
			PED->ped_MaxXDots = oldxmax;
			PED->ped_MaxYDots = oldymax;
			PED->ped_XDotsInch = oldxdpi;
			PED->ped_YDotsInch = oldydpi;
			err = PDERR_NOERR; /* all ok */
			break;
	}
	return(err);
}
