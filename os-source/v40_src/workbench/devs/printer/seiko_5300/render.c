/*
	Seiko_5300 driver.
	David Berezowski - June/88.
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define STARTLEN	51	/* # of start cmd bytes */
#define MCPOSN		5	/* posn of machine control data */
#define HSPOSN		23	/* posn of horizontal size data */
#define VSPOSN		34	/* posn of vertical size data */
#define COLORPOSN	43	/* posn of color format */

#define DEBUG0	0
#define DEBUG1	0
#define DEBUG2	0
#define DEBUG3	0
#define DEBUG4	0
#define DEBUG5	0
#define DEBUG6	0

Render(ct, x, y, status)
long ct, x, y, status;
{
	extern void *AllocMem(), FreeMem();

	extern struct PrinterData *PD;

	static UWORD DataSize, RowSize, BufSize, TotalBufSize, dataoffset;
	static UWORD pass;
	static UBYTE CBuf[3] = "\034Y:";
	static UBYTE CCodes[3] = "YMC";
	/*
		00-00	\001	SOH (Start Of Header)
		01-05	MC1.3	Machine Control
		06-09	,PRM	prime engine
		10-14	,MAG1	magnification to 1
		15-19	,TRN1	transmission mode 1
		20-26	,HS0000	horizontal size (width)
		27-30	,HO0	horizontal offset
		31-37	,VS0000	vertical size (height)
		38-41	,VO0	vertical offset
		42-45	,xxx	color format
		46-49	,SB1	byte structure
		50	\002	STX (Start of Text)
	*/
	static UBYTE StartCmd[STARTLEN] =
	"\001MC1.3,PRM,MAG1,TRN1,HS0000,HO0,VS0000,VO0,xxx,SB1\002";
	UBYTE *ptr, *ptrstart;
	int i, err;

	switch(status) {
		case 0 : /* Master Initialization */
			/*
				ct	- pointer to IODRPReq structure.
				x	- width of printed picture in pixels.
				y	- height of printed picture in pixels.
			*/
#if DEBUG0
			kprintf("0: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG0
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				StartCmd[COLORPOSN] = 'Y';
				StartCmd[COLORPOSN + 1] = 'M';
				StartCmd[COLORPOSN + 2] = 'C';
			}
			else {
				StartCmd[COLORPOSN] = 'M';
				StartCmd[COLORPOSN + 1] = 'N';
				StartCmd[COLORPOSN + 2] = 'O';
			}
			DataSize = (x + 7) / 8;
			RowSize = DataSize;
			BufSize = RowSize;
			TotalBufSize = BufSize * 2;
#if DEBUG0
			kprintf("DataSize=%ld, RowSize=%ld, \n",
				DataSize, RowSize);
			kprintf("BufSize=%ld, TotalBufSize=%ld, y=%ld\n",
				BufSize, TotalBufSize, y);
#endif DEBUG0
			PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
			if (PD->pd_PrintBuf == NULL) {
				err = PDERR_BUFFERMEMORY; /* no mem */
			}
			else {
				dataoffset = 0;
				x = (x + 7) / 8 * 8;
				StartCmd[HSPOSN] = (x / 1000) + '0';
				StartCmd[HSPOSN + 1] = (x % 1000) / 100 + '0';
				StartCmd[HSPOSN + 2] = (x % 100) / 10 + '0';
				StartCmd[HSPOSN + 3] = (x % 10) + '0';
				StartCmd[VSPOSN] = (y / 1000) + '0';
				StartCmd[VSPOSN + 1] = (y % 1000) / 100 + '0';
				StartCmd[VSPOSN + 2] = (y % 100) / 10 + '0';
				StartCmd[VSPOSN + 3] = (y % 10) + '0';
				PrinterReady();
				if ((err = (*(PD->pd_PWrite))(StartCmd, STARTLEN)) == PDERR_NOERR) {
					PrinterReady();
					CBuf[1] = CCodes[pass++];
					err = (*(PD->pd_PWrite))(CBuf, 3);
				}
			}
			break;

		case 1 : /* Scale, Dither and Render */
			/*
				ct	- pointer to PrtInfo structure.
				x	- color code.
				y	- row # (0 to Height - 1).
			*/
#if DEBUG1
			kprintf("1: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG1
			Transfer(ct, y, &PD->pd_PrintBuf[dataoffset], x);
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
			PrinterReady();
			err = (*(PD->pd_PWrite))
				(&PD->pd_PrintBuf[dataoffset], BufSize);
			if (err == PDERR_NOERR) {
				dataoffset = (dataoffset == 0 ? BufSize : 0);
			}
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
			ptr = &PD->pd_PrintBuf[dataoffset];
			i = RowSize;
			do {
				*ptr++ = 0;
			} while (--i);
			err = PDERR_NOERR; /* all ok */
			break;

		case 4 : /* Close Down */
			/*
				ct	- error code.
				x	- io_Special flag from IODRPReq.
				y	- 0.
			*/
#if DEBUG4
			kprintf("4: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG4
			err = PDERR_NOERR; /* assume all ok */
			/* if user did not cancel the print*/
			if (ct != PDERR_CANCEL) {
				PrinterReady();
				/* ETX (End Of Text) */
				err = (*(PD->pd_PWrite))("\003", 1);
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
#if DEBUG5
			kprintf("5: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG5
			StartCmd[MCPOSN] =
				SetDensity(x & SPECIAL_DENSITYMASK);
			pass = 0;
			err = PDERR_NOERR; /* all ok */
			break;

		case 6 : /* Switch to Next Color */
			/*
				ct	- 0.
				x	- 0.
				y	- 0.
			*/
#if DEBUG6
			kprintf("6: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif DEBUG6
			if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
				CBuf[1] = CCodes[pass++];
				PrinterReady();
				err = (*(PD->pd_PWrite))(CBuf, 3);
			}
			err = PDERR_NOERR; /* all ok*/
			break;
	}
	return(err);
}
