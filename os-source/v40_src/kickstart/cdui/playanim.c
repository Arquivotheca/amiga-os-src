/* Will play an animation of the RAW Anim8 format.
 * The raw file is created with the MakeRaw program, with sectorsize set to
 * 256 bytes.
 */

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <graphics/gfx.h>
#include <graphics/view.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>

#include "cduibase.h"
#include "playanim.h"

void __asm PlayDLTA(register __a2 struct BitMap *bm, register __a1 UBYTE *buff, register __a3 WORD *table, register __d4 UWORD ImageWidth, register __d5 UWORD ImageDepth);

/* Hard code the number of vblanks to count before showing the next frame for
 * a frame rate of FRAMESPERSEC. (*2 for lace)
 */
#define FRAMESPERSEC 30
#define FRAMECOUNT ((ispal) ? (100 / FRAMESPERSEC) : (120 / FRAMESPERSEC))

void PlayAnim(FILETYPE InFile, struct Frame1Head *f1h,
              struct BitMap *bmbuff[], struct ViewPort *vp,
              UWORD *table, UBYTE *inbuff, ULONG FirstRS, ULONG FirstDS,
              struct CDUILib *CDUIBase, ULONG *ColourTable)
{
    /* This is the main loop that plays the animation.
     *
     * It uses the V39 ChangeVPBitMap() functions, and whilst waiting for the
     * signal to double-buffer, it handles all the input and audio events.
     */

    #define WAIT_TO_WRITE while(!GetMsg(ports)) Wait(1l << (ports->mp_SigBit))

    struct DBufInfo *dbuf;
    struct MsgPort *ports;
    ULONG *secptr, *bytesptr;
    ULONG ReadSectors;
    ULONG DLTASize;
    ULONG VBLast;
    BOOL SafeToWrite = TRUE;
    WORD sbuffno = 1;
    UWORD imagewidth = (((f1h->f1_ImageWidth + 15) >> 4) << 1);
    BOOL ispal = ((GetVPModeID(vp) & MONITOR_ID_MASK) == PAL_MONITOR_ID);

    /* Set up the DBuff stuff */
    if ((dbuf = AllocDBufInfo(vp)) == NULL)
    {
	return;
    }

    if (ports = CreateMsgPort())
    {
	/* A message port for ChangeVPBitMap() */
	dbuf->dbi_SafeMessage.mn_ReplyPort = ports;

	ReadSectors = FirstRS;
	DLTASize = FirstDS;

	WaitBlit();	/* from the BltBitMap() call in main() */
        /* Now it is safe to load the colours */
	LoadRGB32(vp, ColourTable);

	/**************************************/
	/*                                    */
	/* Here is the main animation loop... */
	/*                                    */
	/**************************************/

	VBLast = (GfxBase->VBCounter - FRAMECOUNT - 1);
	while (ReadSectors)
	{
		READ(InFile, inbuff, (SECTORSIZE * ReadSectors));

		bytesptr = (ULONG *)(inbuff + DLTASize);
		secptr = (bytesptr + 1);
		ReadSectors = *secptr;
		DLTASize = *bytesptr;

		if (!SafeToWrite)
			WAIT_TO_WRITE;

		/* The last frame is a repeat of the first, so if ReadSectors is now
		 * 0, this must be the last frame.
		 */
                PlayDLTA(bmbuff[sbuffno], inbuff, table, imagewidth, f1h->f1_ImageDepth);

                while ((GfxBase->VBCounter - VBLast) < FRAMECOUNT)
                {
                        WaitTOF();
                }
                ChangeVPBitMap(vp, bmbuff[sbuffno], dbuf);
                VBLast = GfxBase->VBCounter;
                SafeToWrite = FALSE;
                sbuffno ^= 1;
}

	/**************************************/
	/*                                    */
	/*           That was it!!            */
	/*                                    */
	/**************************************/

	if (!SafeToWrite)
	{
		WAIT_TO_WRITE;
	}
	if (sbuffno == 0)
	{
		/* we are showing the spare buffer */
		BltBitMap(bmbuff[1], 0, 0, bmbuff[0], 0, 0, f1h->f1_ScreenWidth, f1h->f1_ScreenHeight, 0xc0, -1, NULL);
		WaitBlit();
		ChangeVPBitMap(vp, bmbuff[0], dbuf);
		WAIT_TO_WRITE;
	}

	DeleteMsgPort(ports);
    }

    FreeDBufInfo(dbuf);
}

