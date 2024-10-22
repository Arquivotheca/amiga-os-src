
#include <exec/types.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "playanim.h"
#include "cduibase.h"

void __asm ShowILBM(register __a0 struct BitMapHeader *bmhd, register __a1 struct BitMap *bm, register __a2 UBYTE *buff);

void PlayAnim(FILETYPE InFile, struct Frame1Head *f1h,
              struct BitMap *bmbuff[], struct ViewPort *vp,
              UWORD *table, UBYTE *inbuff, ULONG FirstRS, ULONG FirstDS,
              struct CDUILib *CDUIBase, ULONG *ColourTable);

void DisplayAnim(struct Screen *screen, UBYTE * srcbuff, struct CDUILib *CDUIBase, APTR ColourTable)
{
    struct BitMap *bmbuff[2];
    UWORD *table;
    UBYTE *rbuff, *InFile;
    UBYTE *from = srcbuff;
    ULONG *secptr, *bytesptr;
    ULONG ReadSectors;
    ULONG DLTASize;
    struct Frame1Head f1h;
    struct BitMapHeader bmhd;
    UWORD ncolours;
    int r;

    f1h = (*(struct Frame1Head *)srcbuff);
    ncolours = f1h.f1_Colours;

    /* Now, we have a screen and we know the maximum buffer size we need.
     * So, read in the first frame, and treat it as a raw ILBM, once we skip over
     * all the Frame1Header info.
     */

    /* Set up our two buffers */
    bmbuff[0] = screen->RastPort.BitMap;
    bmbuff[1] = AllocBitMap(f1h.f1_ScreenWidth, f1h.f1_ScreenHeight, bmbuff[0]->Depth, BMF_DISPLAYABLE, bmbuff[0]);

    /* create the offset table for the DLTA code */
    table = AllocVec((f1h.f1_ImageHeight << 1), 0);
    {
	UWORD c = 0;
	UWORD *tmp = table;
	for (r = 0; r < f1h.f1_ImageHeight; r++)
	{
		*tmp++ = c;
		c += screen->RastPort.BitMap->BytesPerRow;
	}
    }

    InFile = OPEN(from, MODE_READ, (SECTORSIZE * f1h.f1_MaxSectorSize * 2));
    rbuff = srcbuff;
    READ(InFile, rbuff, (f1h.f1_Sectors * SECTORSIZE));
    {
	UBYTE *image = (rbuff + DATASIZE);

	/* set up the bmhd with the values ShowILBM() needs */
	bmhd.nplanes = f1h.f1_ImageDepth;
	bmhd.w = f1h.f1_ImageWidth;
	bmhd.h = f1h.f1_ImageHeight;

        ShowILBM(&bmhd, bmbuff[0], image);

	/* Copy the image into the other buffer */
	BltBitMap(bmbuff[0], 0, 0, bmbuff[1], 0, 0, f1h.f1_ScreenWidth, f1h.f1_ScreenHeight, 0xc0, -1, NULL);
    }

    /* get the ReadSectors and DLTASize here, because reading the Audio data trashes the rbuff buffer */
    bytesptr = (ULONG *)(rbuff + DATASIZE + f1h.f1_ByteSize);
    secptr = (bytesptr + 1);
    ReadSectors = *secptr;
    DLTASize = *bytesptr;

    /* Now play the anim until the end! */
    PlayAnim(InFile, &f1h, bmbuff, &screen->ViewPort, table, rbuff, ReadSectors, DLTASize, CDUIBase, ColourTable);

    /* Free it all up. */
    CLOSEFILE(InFile);
    FreeVec(table);
    WaitBlit();    /* just in case... */
    FreeBitMap(bmbuff[1]);
}
