/*
	Tektronix 4693D driver.
	Bill Koester - December / 87
	( with a little help from his friend :^) )
	( sung to the tune of the popular Beatles song)
*/

#include <exec/types.h>
#include <exec/memory.h>
#include "../intuition.h"
#include <devices/printer.h>
#include <devices/prtbase.h>
#include "TEK4693D.h"

#define NUMSTARTCMD     0   /* # of cmd bytes before binary data */
#define NUMENDCMD       1   /* # of cmd bytes after binary data */
#define NUMTOTALCMD     (NUMSTARTCMD + NUMENDCMD)   /* total of above */
#define COPIES          0   /* use printer default */

#define CASE0           0     /* init */
#define CASE0_VARS      0
#define CASE0_PREAMBLE  0

#define CASE1           0     /* transfer-render */
#define CASE2           0     /* dump */
#define CASE2_BUFFER    0
#define CASE3           0     /* clear */
#define CASE4           0     /* close down */
#define CASE5           0     /* pre master init */
#define CASE7		0

#define NORMAL    2
#define SIDEWAYS  4

Render(ct, x, y, status)
long ct, x, y, status;
{
   extern void *AllocMem(), FreeMem();
   extern struct PrinterData *PD;
   extern struct PrinterExtendedData *PED;

   static LONG pixel_width, aspect, mode;
   static UWORD RowSize, ColorSize, BufSize, TotalBufSize, dataoffset;
   static UWORD oldprintaspect, oldprintshade, YOrg, StripPrint;
   static UWORD oldxdpi, oldydpi;
   static ULONG oldxmax, oldymax;
   static unsigned char preamble[255];
   LONG host_vert,host_horz,screen_vert,screen_horz;
   int err, i, j, ysave;
   UBYTE *ptr;

   switch(status)
   {
	case 0 : /* Master Initialization */
	/*
		ct   - pointer to IODRPReq structure.
		x   - width of printed picture in pixels.
		y   - height of printed picture in pixels.
	*/
#if CASE0
kprintf("Master Init Case 0: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif
	RowSize = x << 1;
	ColorSize = RowSize + NUMTOTALCMD; /* size of each color buf */
	BufSize = ColorSize; /* size of 1 of the double buffer buffers */
	TotalBufSize = BufSize * 2; /* total amount of mem required */
	PD->pd_PrintBuf = (UBYTE *)AllocMem(TotalBufSize, MEMF_PUBLIC);
#if CASE0_VARS
kprintf("Rowsize=%ld, ColorSize=%ld, BufSize=%ld, TotalBufSize=%ld\n",
RowSize,ColorSize,BufSize,TotalBufSize);
kprintf("Buffer at %lx\n",PD->pd_PrintBuf);
#endif
	ysave = y; /* save strip height */
	err = PDERR_NOERR; /* assume no error */
	if (PD->pd_PrintBuf == NULL) {
		err = PDERR_BUFFERMEMORY; /* could not get the memory */
	}
	else if (YOrg == 0) { /* if first strip, send pre-amble */
		/* if strip printing, kludge height of dump */
		if (StripPrint) {
#if CASE0
kprintf("0: strip printing, y was %ld, kludging to %ld, RESERVE\n",
y, PED->ped_MaxYDots);
#endif
			/* set to max (for 8.5 x 11 inch paper) */
			y = PED->ped_MaxYDots;
			/* send RESERVE command, this prevents the printer */
			/* from timing out on strip prints */
			PrinterReady();
			if ((err = (*(PD->pd_PWrite))("\005", 1)) !=
				PDERR_NOERR) {
				break;
			}
			/* force 1:1 pixel ratio */
			host_horz = 1;
			host_vert = 1;
			screen_horz = 1;
			screen_vert = 1;
		}
		else { /* not strip printing */
			if (((struct IODRPReq *)ct)->io_Modes & HIRES ) {
				host_horz = 640;
			}
			else {
				host_horz = 320;
			}
			if (((struct IODRPReq *)ct)->io_Modes & LACE ) {
				host_vert = 400;
			}
			else {
				host_vert = 200;
			}
			screen_horz = 26;
			screen_vert = 19;
		}
		pixel_width = (host_vert*screen_horz*32) /
			(host_horz * screen_vert);

		Make_Preamble(&preamble[0],x,y,DATA_STREAMING,COPIES,
			PIXEL_REC,pixel_width,ARBITRARY_4,ORDER_1,
			SIZE_INTERP,mode,COLOR_DEF,MANIP_DEF,BW_DEF,
			(aspect==NORMAL) ? ORIENT_TOP : ORIENT_LAND,
			COPIER_NULL);

#if undef
PrinterReady();
err = (*(PD->pd_PWrite))(&preamble[0],(int)(preamble[1]&63)+2);
#else
		for (i=0; i < ((preamble[1] & 63) +2 ); i++) {
			PrinterReady();
			err = (*(PD->pd_PWrite))(&preamble[i],1);
			if (err != PDERR_NOERR) {
				break;
			}
		}
#endif undef

#if CASE0_PREAMBLE
kprintf("Write error = %ld\n",err);
kprintf("Preamble at %lx, Length = %ld\n",&preamble[0],(int)((preamble[1]&63)+2));
for(i=0;i<(preamble[1]&63)+2;i++)
kprintf("%lx ",(int)preamble[i]);
kprintf("\n");
#endif
	}
	YOrg += ysave;
	dataoffset = NUMSTARTCMD;
	break;

      case 1: /* Scale, Dither and Render */
         /*
            ct   - pointer to PrtInfo structure.
            x   - 0.
            y   - row # (0 to Height - 1).
         */
#if CASE1
kprintf("Scale Dither and Render Case 1: ct=%lx, x=%ld, y=%ld\n", ct, x, y);
#endif
         Transfer(ct, y, &PD->pd_PrintBuf[dataoffset]);
         err = PDERR_NOERR; /* all ok */
         break;

      case 2 : /* Dump Buffer to Printer */
         /*
            ct  - 0.
            x   - 0.
            y   - # of rows sent (1 to NumRows).
         */
#if CASE2
kprintf("Dump Buffer Case 2: %lx, %ld, %ld, %ld\n", ct,x,y,dataoffset);
#endif

	PrinterReady();
         err = (*(PD->pd_PWrite))
            (&PD->pd_PrintBuf[dataoffset - NUMSTARTCMD], BufSize);

#if CASE2_BUFFER
   y=0;
   for(x=(dataoffset-NUMSTARTCMD);x<(dataoffset-NUMSTARTCMD+BufSize);x++)
   {
      kprintf("%lx ",(int)PD->pd_PrintBuf[x]);
      y++; if(y>25) { kprintf("\n"); y=0; }
   }
   kprintf("\n");
#endif
         if (err == PDERR_NOERR) {
		dataoffset = (dataoffset == NUMSTARTCMD ? BufSize : 0) +
			NUMSTARTCMD;
         }
         break;

      case 3 : /* Clear and Init Buffer */
         /*
            ct  - 0.
            x   - 0.
            y   - 0.
         */
#if CASE3
kprintf("Clear and Init Buffer Case 3: %lx, %ld, %ld\n",ct,x,y);
#endif
/*
         ptr = &PD->pd_PrintBuf[dataoffset];
         i = RowSize >> 1;
         do {
            *(ptr++) = 0x00;
            *(ptr++) = 0x00;
         } while (--i);
*/

         /* we will always write the entire buffer so why clear? */
         PD->pd_PrintBuf[dataoffset+(BufSize-1)] = 0x02;
         err = PDERR_NOERR; /* all ok */
         break;

      case 4 : /* Close Down */
         /*
            ct   - error code.
            x   - io_Special flag from IODRPReq structure.
            y   - 0.
         */
#if CASE4
kprintf("Close Down Case 4: ct=%lx, x=%ld, y=%ld, YOrg=%ld\n",ct,x,y,YOrg);
#endif
	err = PDERR_NOERR; /* assume all ok */
	if (ct != PDERR_CANCEL) { /* if user did not cancel the print */
		/* if do not want to unload paper */
		if (x & SPECIAL_NOFORMFEED) {
			/* do nothing (for now) */
		}
		else if (StripPrint) {
			ptr = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
			i = BufSize - NUMTOTALCMD;
			do {
				*ptr++ = 0xff;
			} while (--i);
			*ptr = 0x02;
			ptr = &PD->pd_PrintBuf[dataoffset - NUMSTARTCMD];
			i = PED->ped_MaxYDots - YOrg;
#if CASE4
kprintf("sending %ld NULL lines...", i);
#endif
			while (i-- && err == PDERR_NOERR) {
				PrinterReady();
				err = (*(PD->pd_PWrite))(ptr, BufSize);
			}
#if CASE4
kprintf("ok\n");
#endif
			if (err == PDERR_NOERR) {
#if CASE4
kprintf("sending EOT and ABORT\n");
#endif
				PrinterReady();
				/* EOT / ABORT */
				err = (*(PD->pd_PWrite))("\001\003", 2);
			}
			YOrg = 0;
		}
		else { /* eject paper */
			YOrg = 0;
#if CASE4
kprintf("4: sending EOT cmd\n");
#endif
			PrinterReady();
			/* send EOT (End Of Transmission) */
			err = (*(PD->pd_PWrite))("\001", 1);
		}
	}
#if CASE4
kprintf("4: watiing for both buffers to empty\n");
#endif
	(*(PD->pd_PBothReady))();  /* wait for both buffers to empty */
	if (PD->pd_PrintBuf != NULL) {
#if CASE4
kprintf("4: freeing %ld bytes at %06lx\n", TotalBufSize, PD->pd_PrintBuf);
#endif
		FreeMem(PD->pd_PrintBuf, TotalBufSize); /* free memory */
	}
#if CASE4
kprintf("4: break\n");
#endif
	break;

      case 5 : /* Pre-Master Initialization */
         /*
            ct   - 0 or pointer to IODRPReq structure.
            x   - io_Special flag from IODRPReq structure.
            y   - 0.
         */

	 /* make the printer device think that we are doing a 1:1 dump */
	 /* we DO NOT need to save/restore this info */
         PD->pd_Preferences.PrintMaxWidth  = 1;
         PD->pd_Preferences.PrintMaxHeight = 1;
         PD->pd_Preferences.PrintFlags &= ~DIMENSIONS_MASK;
         PD->pd_Preferences.PrintFlags |= MULTIPLY_DIMENSIONS;

	if (ct != 0) { /* if not case 5 open */
		if (YOrg == 0) { /* if first time thru */
			/* set flag to whether or not we are strip printing */
			StripPrint = x & SPECIAL_NOFORMFEED;
		}
		/* save original values (for later restoration in case 4) */
		oldprintaspect = PD->pd_Preferences.PrintAspect;
		oldprintshade = PD->pd_Preferences.PrintShade;
		oldxmax = PED->ped_MaxXDots;
		oldymax = PED->ped_MaxYDots;
		oldxdpi = PED->ped_XDotsInch;
		oldydpi = PED->ped_YDotsInch;

		if (PD->pd_Preferences.PrintAspect & ASPECT_VERT) {
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
			   case 7 to undo what we ve done
			   here to keep things in sync.
			*/
			aspect = SIDEWAYS;
			/* swap x and y stuff */
			PED->ped_MaxXDots = oldymax;
			PED->ped_MaxYDots = oldxmax;
			PED->ped_XDotsInch = oldydpi;
			PED->ped_YDotsInch = oldxdpi;
		}
		else {
			aspect = NORMAL;
		}
		PD->pd_Preferences.PrintAspect &= ~ASPECT_VERT;

		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
			/* set mode grey force preferences color */
			mode = RENDER_GREY;
			PD->pd_Preferences.PrintShade = SHADE_COLOR;
		}
		else if (PD->pd_Preferences.PrintShade == SHADE_BW) {
			/* mode = BW preferences is cool */
			mode = RENDER_BW;
		}
		else {
			/* mode = color preferences is cool */
			if (PD->pd_Preferences.PaperType == FANFOLD) {
				/* printing on white paper (single black) */
				mode = RENDER_FC1;
			}
			else {
				/* printer on transparency (extra black) */
				mode = RENDER_FC2;
			}
		}
	}
	else { /* case 5 open */
		YOrg = 0;
	}
	SetDensity(x & SPECIAL_DENSITYMASK);
	err = PDERR_NOERR; /* all ok */
#if CASE5
kprintf("Case 5: ct=%lx, x=%ld, y=%ld, YOrg=%ld, StripPrint=%ld\n",
ct,x,y,YOrg,StripPrint);
#endif
	break;

	/*
	This is a NEW case that was added after the offical
	V1.3 release of the printer device and this driver.
	Although this driver will work with the official
	V1.3 printer device (version 35.562); it may not
	print full size vertical pictures correctly since
	it does not contain a case 7 call.  The printer
	device version 35.564 (and beyond) contains a
	case 7 call.
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
