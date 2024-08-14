
/* includes */
#include <exec/devices.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <devices/audio.h>
#include <prefs/sound.h>
#include <graphics/layers.h>
#include <intuition/gadgetclass.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <prefs/wbpattern.h>
#include <dos.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/graphics_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/graphics_pragmas.h>

/* application includes */
#include "backdrop.h"
#include "iprefs.h"
#include "pread.h"
#include "resetwb.h"


/*****************************************************************************/


extern struct ExecBase *SysBase;
extern struct Library  *IntuitionBase;
extern struct Library  *LayersBase;
extern struct Library  *GfxBase;
extern struct Library  *DataTypesBase;
extern struct Task     *iprefsTask;
extern Object          *soundObj;


/*****************************************************************************/


struct BackdropLock
{
    struct SignalSemaphore  bl_Semaphore;
    struct BitMap          *bl_BitMap;
    UWORD		    bl_Width;
    UWORD		    bl_Height;
};

struct BackdropLock    bl;
struct Hook            render;
APTR                   oldOpenScreen;
APTR                   oldCloseWorkBench;
struct IPatternBitMap *backdrops[3];
BOOL                   newScreenBackdrop;

ULONG __stdargs DoMethodA (Object *obj, Msg message);


/*****************************************************************************/


static VOID FlashWindow(VOID)
{
struct Screen *wb;
struct Window *wp;
BOOL           gotBackdrop;

    if (wb = LockWB())
    {
        gotBackdrop = FALSE;

        wp = wb->FirstWindow;
        while (wp)
        {
            if ((wp->Flags & WFLG_BACKDROP) && (wp->Flags & WFLG_WBENCHWINDOW))
            {
                gotBackdrop = TRUE;
            }

            wp = wp->NextWindow;
        }

        if (!gotBackdrop)
        {
            if (wp = OpenWindowTags(NULL,WA_Flags,     WFLG_BACKDROP | WFLG_SIMPLE_REFRESH | WFLG_BORDERLESS,
                                         WA_PubScreen, wb,
                                         WA_Height,    wb->Height - wb->BarHeight,
                                         TAG_DONE))
            {
                CloseWindow(wp);
            }
        }

        UnlockPubScreen(NULL,wb);
    }
}


/*****************************************************************************/


BOOL AttemptOpenDT(VOID)
{
    if (!DataTypesBase)
    {
        if (!(DataTypesBase = OpenLibrary("datatypes.library",0)))
        {
            UserProblems(MSG_IP_ERROR_NO_DATATYPES_LIB,NULL);
            return(FALSE);
        }
    }

    return(TRUE);
}


/*****************************************************************************/


VOID AttemptCloseDT(VOID)
{
UWORD i;

    if (!soundObj)
    {
        i = 0;
        while (i < 3)
        {
            if (backdrops[i] && (!backdrops[i]->pbm_BitMap || backdrops[i]->pbm_Object))
                return;

            i++;
        }

        CloseLibrary(DataTypesBase);
        DataTypesBase = NULL;
    }
}


/*****************************************************************************/


VOID IWBConfig(ULONG type, APTR data)
{
struct Library *WorkbenchBase;

    if (WorkbenchBase = OpenLibrary("workbench.library",39))
    {
        WBConfig(type,(ULONG)data);
        CloseLibrary(WorkbenchBase);
    }
}


/*****************************************************************************/


VOID BackdropConfig(UWORD backdropType, struct IPatternBitMap *pbm)
{
struct IPatternBitMap *old;
BOOL                   flashWindow = FALSE;

    ObtainSemaphore(&bl);

    if (backdropType < WBP_SCREEN)
    {
        if (pbm && pbm->pbm_BitMap)
        {
            IWBConfig(1 + backdropType,pbm);
        }
        else
        {
            IWBConfig(1 + backdropType,NULL);
        }
    }
    else
    {
        bl.bl_BitMap = NULL;
        if (pbm)
        {
            bl.bl_BitMap = pbm->pbm_BitMap;
            bl.bl_Width  = pbm->pbm_Width;
            bl.bl_Height = pbm->pbm_Height;
        }
        newScreenBackdrop = TRUE;
    }

    old = backdrops[backdropType];
    backdrops[backdropType] = pbm;

    if (old)
    {
        if (old->pbm_Object)
        {
            DisposeDTObject(old->pbm_Object);

            flashWindow = (backdropType == WBP_SCREEN);
        }
        else if (old->pbm_BitMap)
        {
            WaitBlit();
            old->pbm_BitMap->Depth = 3;
            FreeBitMap(old->pbm_BitMap);
        }

        FreeVec(old);
    }

    AttemptCloseDT();

    ReleaseSemaphore(&bl);

    if (flashWindow)
        FlashWindow();

    Signal(iprefsTask,SIGBREAKF_CTRL_E);
}


/*****************************************************************************/


VOID RemoveDTBackdrops(VOID)
{
UWORD i;

    geta4();     /* since we're called from CloseWorkBench() */

    ObtainSemaphore(&bl);

    i = 0;
    while (i < 3)
    {
        if (backdrops[i] && backdrops[i]->pbm_Object)
        {
            if (i < WBP_SCREEN)
            {
                IWBConfig(1 + i,NULL);
            }
            else
            {
                bl.bl_BitMap = NULL;
            }

            DisposeDTObject(backdrops[i]->pbm_Object);
            backdrops[i]->pbm_Object = NULL;
            backdrops[i]->pbm_BitMap = NULL;
        }

        i++;
    }

    AttemptCloseDT();

    newScreenBackdrop = TRUE;

    ReleaseSemaphore(&bl);
}


/*****************************************************************************/


VOID UpdateDTBackdrops(VOID)
{
struct Screen       *wb;
struct gpLayout      gpl;
struct BitMapHeader *bmhd;
struct BitMap       *bm;
UWORD                i;
BOOL                 flashWindow;
Object              *obj;

    if (wb = LockWB())
    {
        flashWindow       = newScreenBackdrop;
        newScreenBackdrop = FALSE;

        InstallLayerInfoHook(&wb->LayerInfo,LAYERS_BACKFILL);

        i = 0;
        while (i < 3)
        {
            if (backdrops[i] && !backdrops[i]->pbm_BitMap && backdrops[i]->pbm_Name)
            {
                ObtainSemaphore(&bl);

                if (AttemptOpenDT())
                {
                    if (obj = NewDTObject(&backdrops[i]->pbm_Name,
                                          DTA_SourceType,        DTST_FILE,
                                          PDTA_Remap,            backdrops[i]->pbm_DoRemap,
                                          PDTA_Screen,           wb,
                                          PDTA_FreeSourceBitMap, TRUE,
                                          TAG_DONE))
                    {
                        gpl.MethodID    = DTM_PROCLAYOUT;
                        gpl.gpl_GInfo   = NULL;
                        gpl.gpl_Initial = 1;

                        SetTaskPri(iprefsTask,-1);

                        if (DoMethodA(obj,&gpl))
                        {
                            GetDTAttrs(obj,PDTA_BitMapHeader, &bmhd,
                                           PDTA_BitMap,       &bm,
                                           TAG_DONE);

                            if (bm)
                            {
                                backdrops[i]->pbm_Width  = bmhd->bmh_Width;
                                backdrops[i]->pbm_Height = bmhd->bmh_Height;
                                backdrops[i]->pbm_Object = obj;
                                backdrops[i]->pbm_BitMap = bm;

                                if (i < WBP_SCREEN)
                                {
                                    IWBConfig(i+1,backdrops[i]);
                                }
                                else
                                {
                                    bl.bl_BitMap = bm;
                                    bl.bl_Width  = bmhd->bmh_Width;
                                    bl.bl_Height = bmhd->bmh_Height;

                                    flashWindow = TRUE;
                                }
                            }
                            else
                            {
                                DisposeDTObject(obj);
                            }
                        }
                        else
                        {
                            DisposeDTObject(obj);
                        }

                        SetTaskPri(iprefsTask,0);
                    }
                    else
                    {
                        UserProblems(MSG_IP_ERROR_NO_PICTURE,&backdrops[i]->pbm_Name);
                    }
                }
                ReleaseSemaphore(&bl);
            }
            i++;
        }

        AttemptCloseDT();

        InstallLayerInfoHook(&wb->LayerInfo,&render);

        if (flashWindow)
            FlashWindow();

        UnlockPubScreen(NULL,wb);
    }
}
