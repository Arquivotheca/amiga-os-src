
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <graphics/sprite.h>
#include <cdtv/debox.h>
#include <libraries/cdgprefs.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/cdg_cr_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/cdg_cr_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "mpegplayerbase.h"
#include "utils.h"
#include "cdg.h"


/*****************************************************************************/


extern UBYTE * __far cdgicons;


/*****************************************************************************/


static UWORD *DumpSprPlane(struct BitMap *bm, long plane, long offset, UWORD *cptr)
{
UWORD *pp;
long   height, wpr;

    pp = (UWORD *)bm->Planes[plane] + offset;

    wpr  = bm->BytesPerRow >> 1;

    for (height = bm->Rows; height--;)
    {
        *cptr  = *pp;
        cptr  += 2;
        pp    += wpr;
    }

    return (cptr);
}


/*****************************************************************************/


static UWORD *FillSprCtl(int x, int y, int height, UWORD *cptr)
{
UBYTE *bptr;
int    vstop;

    bptr = (UBYTE *)cptr;

    *bptr++ = y;
    *bptr++ = x>>1;
    *bptr++ = (vstop = height + y);
    *bptr++ = (y & 0x100 ? 4:0) + (vstop & 0x100 ? 2:0) + (x & 1);

    return((UWORD *)bptr);
}


/*****************************************************************************/


VOID PrepareCDG(struct MPEGPlayerLib *MPEGPlayerBase)
{
struct CDGPrefs *cdgp;
UWORD           *Gcptr;
UWORD          **Gsptr;
LONG             Gi;
LONG             ssize;
struct BitMap   *bm;
struct BMInfo   *bmInfo;

    cdgp = MPEGPlayerBase->mp_CDGPrefs = CDGAllocPrefs();

    cdgp->cdgp_Control = CDGF_ALTSPRITES;

    GetBM(MPEGPlayerBase,&cdgicons,&bm,&bmInfo);

    ssize = (bm->Rows * 4 + 8) * (64 + 32);

    Gcptr = AllocVec(ssize, MEMF_CHIP | MEMF_CLEAR);
    MPEGPlayerBase->mp_CDGSprites = Gcptr;

    CopyMem(bmInfo->bmi_ColorMap, cdgp->cdgp_SpriteColors, 16);

    cdgp->cdgp_SpriteHeight = 30;
    Gsptr = cdgp->cdgp_ChannelSprites;

    Gi = 0;
    do
    {
        *Gsptr++ = Gcptr;
        Gcptr    = FillSprCtl(250 + (Gi & 1 ? 16:0), 160, bm->Rows, Gcptr);

        DumpSprPlane(bm, 1, Gi, &Gcptr[1]);

        Gcptr  = DumpSprPlane(bm, 0, Gi, Gcptr);
        Gcptr += 2;

        *Gsptr++   = Gcptr;
        Gcptr      = FillSprCtl(250+(Gi & 1 ? 16:0), 160, bm->Rows, Gcptr);
        Gcptr[-1] |= SPRITE_ATTACHED;
        Gcptr      = DumpSprPlane(bm, 2, Gi, Gcptr);
        Gcptr     += 2;
    }
    while (++Gi < 46);

    FreeBMInfo(bmInfo);
    FreeBitMap(bm);

    MPEGPlayerBase->mp_CDGChannel = 1;
    MPEGPlayerBase->mp_CDGState   = CDG_UNAVAILABLE;
}


/*****************************************************************************/


#if (CLEAN_EXIT)
VOID ShutdownCDG(struct MPEGPlayerLib *MPEGPlayerBase)
{
    CDGFreePrefs(MPEGPlayerBase->mp_CDGPrefs);
    FreeVec(MPEGPlayerBase->mp_CDGSprites);

    MPEGPlayerBase->mp_CDGPrefs = NULL;
    MPEGPlayerBase->mp_CDGSprites = NULL;
}
#endif
