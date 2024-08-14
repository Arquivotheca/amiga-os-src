
#include "exec/types.h"
#include "exec/io.h"
#include "exec/memory.h"

#include <graphics/sprite.h>

#include "cdtv/cdtv.h"
#include <cdtv/cdtvprefs.h>
#include <cdtv/debox.h>
#include <cdtv/cdgprefs.h>

#include "/playerprefs.h"
#include "/playerprefsbase.h"
#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"

#include <pragmas/exec_old_pragmas.h>
#include "cdtv/cdg_cr_pragmas.h"
#include <cdtv/debox_pragmas.h>

#include <clib/exec_protos.h>
#include <cdtv/debox_protos.h>

/********************* External Structures/Variables ***********************/

extern struct Library         *CDGBase;
extern struct CDTVPrefs        CDTVPrefs;
extern struct Library         *GfxBase;
extern struct DeBoxBase       *DeBoxBase;
extern struct PlayerPrefsBase *PlayerPrefsBase;
extern WORD                    ViewCenterX, ViewCenterY;

/******************************* Functions *********************************/

int  CDG_Open(struct CDGPrefs *cdp);
void CDG_Close();

/************************* Structures/Variables ****************************/

struct MsgPort  *mp;
struct IOStdReq *ior;
UWORD           *SpriteMem;
struct BitMap   *sbm;
struct BMInfo   *sbmi;

/************************* Setup CDG information ***************************/

int CDG_Open(struct CDGPrefs *cdp) {

BOOL fail;

    fail = TRUE;
    SpriteMem = NULL;

    if(CDGBase = OpenLibrary("cdg.library",0)) {

        memset(cdp, 0, sizeof(struct CDGPrefs));

        cdp->cdgp_Control = CDGF_ALTSPRITES;

        cdp->cdgp_DisplayX = CDTVPrefs.DisplayX - ViewCenterX;
        cdp->cdgp_DisplayY = CDTVPrefs.DisplayY - ViewCenterY;

        if (mp = CreateMsgPort())
            if (ior = CreateIORequest(mp,sizeof(struct IOStdReq)))
                if (!OpenDevice("cd.device", 0, ior, 0)) fail = FALSE;
        }

    if (fail) {

        CDG_Close();
        return(FALSE);
        }

    return(TRUE);
    }



/*********************** Close Down CDG information ************************/

void CDG_Close() {

    if (SpriteMem) {

        FreeVec(SpriteMem);
        SpriteMem = NULL;
        }

    if (ior) {

        if (ior->io_Device) CloseDevice(ior);
        DeleteIORequest(ior);
        }

    if (mp) DeleteMsgPort(mp);

    if (CDGBase) CloseLibrary(CDGBase);
    }




/*======================== Get Plane Information ==========================*/

UWORD *dumpsprplane(struct BitMap *bm, int plane, int offset, UWORD *cptr) {

int    height, wpr;
UWORD *pp;
        
    pp   = (UWORD *)bm->Planes[plane];
    pp  += offset;
    wpr  = bm->BytesPerRow >> 1;

    for (height=bm->Rows; height--;) {

        *cptr  = *pp;
        cptr  += 2;
        pp    += wpr;
        }

    return(cptr);
    }



/*======================== Get Sprite Information =========================*/

UWORD *fillsprctl(int x, int y, int height, UWORD *cptr) {

UBYTE *bptr;
int    vstop;
        
    bptr = (UBYTE *)cptr;

    *bptr++ = y;
    *bptr++ = x>>1;
    *bptr++ = (vstop = height + y);
    *bptr++ = (y & 0x100 ? 4:0) + (vstop & 0x100 ? 2:0) + (x & 1);

    return((UWORD *)bptr);
    }




/************************** Unpack CD+G Sprites ****************************/

int UnpackSprites(struct CDGPrefs *cdgp) {

extern UBYTE __far cdgnumb[];

UWORD   *Gcptr;
UWORD  **Gsptr;
int      Gi;
int      ssize, i;

    sbmi = NULL;
    sbm  = NULL;

    if (!GrabBm(NULL, cdgnumb, &sbmi, &sbm, NULL)) return(0);                                       /* unpack the bitmaps */
    
    ssize = (sbm->Rows * 4 + 8) * (64 + 32);

    if (!(Gcptr = AllocVec(ssize, MEMF_CHIP | MEMF_CLEAR))) return(0);                              /* free the bitmaps as well */

    if (!(SpriteMem = Gcptr)) return(0);

    CopyMem(sbmi->bmi_ColorMap, cdgp->cdgp_SpriteColors, 16);

    cdgp->cdgp_SpriteHeight = 30;
    Gsptr = cdgp->cdgp_ChannelSprites;

    Gi = 0; do {

        if (!sbm) return(0);                                                                        /* sprite unpacking failed */

        *Gsptr++ = Gcptr;
        Gcptr    = fillsprctl(250 + (Gi & 1 ? 16:0), 160, sbm->Rows, Gcptr);

        dumpsprplane(sbm, 1, Gi, &Gcptr[1]);

        Gcptr  = dumpsprplane(sbm, 0, Gi, Gcptr);
        Gcptr += 2;
    
        *Gsptr++   = Gcptr;
        Gcptr      = fillsprctl(250+(Gi & 1 ? 16:0), 160, sbm->Rows, Gcptr);
        Gcptr[-1] |= SPRITE_ATTACHED;
        Gcptr      = dumpsprplane(sbm, 2, Gi, Gcptr);
        Gcptr     += 2;
        } while (++Gi < 46);

    for (i=0; i<4; i++) {                                                                           /* zap out the ntrack and ptrack sprites (it looks better to reuse the ff and rew sprites */

        cdgp->cdgp_NTrackSprite[i] = cdgp->cdgp_FFSprite[i];
        cdgp->cdgp_PTrackSprite[i] = cdgp->cdgp_RWSprite[i];
        }

    if (sbm) {

        FreeGrabBm(&sbmi, &sbm, NULL);
        sbm = NULL;
        }

    return(1);
    }





