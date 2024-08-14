#include <exec/types.h>
#include <graphics/rastport.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "prtgfx.h"
#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0	0
#define DEBUG2	0

/*
	Read an array of color index values.
*/
MyReadPixelArray(y, PInfo, buf)
UWORD y;
struct PrtInfo *PInfo;
WORD *buf;
{
	struct RastPort *rp;
	WORD *bufsav, x, width, width2;

#if DEBUG0
	kprintf("MRPA: y=%ld, buf=%lx, flags=%lx\n", y, buf, PInfo->pi_flags);
#endif /* DEBUG0 */

	bufsav = buf; /* save ptr to buffer start */

	if (PInfo->pi_flags & PRT_INVERT) { /* inverted, use ReadPixel */
		rp = PInfo->pi_rp;
		x = PInfo->pi_xstart;
		width = PInfo->pi_width;
		do {
			/* get color lookup code (if pixel obscured) */
			if ((*buf = ReadPixel(rp, y, x--)) < 0) {
				*buf = 0; /* make it the background color */
			}
			buf++;
		} while (--width);
	}
	else { /* not inverted BUT we may not be able to use ReadPixelLine */
		/* cant use ReadPixelLine, must call ReadPixel */
		if (PInfo->pi_flags & PRT_NORPL) {
			rp = PInfo->pi_rp;
			x = PInfo->pi_xstart;
			width = PInfo->pi_width;
			do {
				/* get color lookup code (if obscured) */
				if ((*buf = ReadPixel(rp, x++, y)) < 0) {
					*buf = 0; /* make it the background color */
				}
				buf++;
			} while (--width);
		}
		else { /* use the super duper read pixel line routine */
			rp = PInfo->pi_rp;
			x = PInfo->pi_xstart;
			width2 = PInfo->pi_width;
			do {
				width = width2 < MAXBLITSIZE ? width2 : MAXBLITSIZE;
#if DEBUG0
	kprintf(
		"rp=%lx, x=%ld, y=%ld, width=%ld, buf=%lx, temprp=%lx, flags=%lx\n",
		rp, x, y, width, buf, PInfo->pi_temprp, PInfo->pi_flags & PRT_NOBLIT);
#endif /* DEBUG0 */
				ReadPixelLine(rp, x, y, width, buf, PInfo->pi_temprp,
					PInfo->pi_flags & PRT_NOBLIT);
				width2 -= MAXBLITSIZE;
				x += MAXBLITSIZE;
				buf += MAXBLITSIZE;
			} while (width2 > 0);
#if DEBUG2
			rp = PInfo->pi_rp;
			x = PInfo->pi_xstart;
			width = PInfo->pi_width;
			buf = bufsav;
			do {
				if (*buf != ReadPixel(rp, x, y)) {
					kprintf("ERROR in RPL: x=%ld, *buf=%ld, RP=%ld\n",
						x, *buf, ReadPixel(rp, x, y));
					/* display source plane ptrs */
					width = rp->BitMap->Depth - 1;
					kprintf("Source Plane Ptrs :\n");
					do {
						kprintf("Planes[%02ld]=%08lx\n",
							width, rp->BitMap->Planes[width]);
					} while (--width >= 0);
					/* dump ReadPixel info */
					x = PInfo->pi_xstart;
					width = PInfo->pi_width;
					kprintf("ReadPixel :\n");
					do {
						kprintf("%04ld ", ReadPixel(rp, x++, y));
					} while (--width);
					/* dump RPL info */
					width = PInfo->pi_width;
					buf = bufsav;
					kprintf("\nReadPixelLine :\n");
					do {
						kprintf("%04ld ", *buf++);
					} while (--width);
					kprintf("\n");
					/* call Wack */
					Debug();
					break;
				}
				else {
					buf++;
					x++;
				}
			} while (--width);
#endif /* DEBUG2 */
		}
	}
}
