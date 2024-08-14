
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/pointerclass.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <libraries/lowlevel.h>
#include <devices/cd.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "utils.h"
#include "eventloop.h"
#include "cdg.h"
#include "mpegplayerbase.h"

/*	Debug switches */

#ifdef	DEBUG
extern void kprintf(char *,...);
#define	D(a)	kprintf a
#else
#define	D(a)
#endif



/*****************************************************************************/


extern struct Custom __far custom;
typedef void (*INTFUNC)();

/* debox magic data */
extern UBYTE * __far background;
extern UBYTE * __far icons;
extern UBYTE * __far arrows;

extern struct TextFont compact24TextFont;
extern struct TextFont compact31TextFont;

ULONG SystemControl( Tag firstTag, ... );

#define ON_BLITHOG  custom.dmacon = BITSET|DMAF_BLITHOG;
#define OFF_BLITHOG custom.dmacon = BITCLR|DMAF_BLITHOG;


/*****************************************************************************/


VOID __asm DiskChangeHandler(register __a1 struct MPEGPlayerLib *MPEGPlayerBase)
{
    Signal(MPEGPlayerBase->mp_MainTask,(1 << MPEGPlayerBase->mp_RemovalSigBit));
}


/*****************************************************************************/


static void GetFontStuff(struct MPEGPlayerLib *MPEGPlayerBase)
{
    MPEGPlayerBase->mp_Compact24 = compact24TextFont;
    MPEGPlayerBase->mp_Compact31 = compact31TextFont;

    ExtendFont(&MPEGPlayerBase->mp_Compact24,NULL);
    ExtendFont(&MPEGPlayerBase->mp_Compact31,NULL);
}


/*****************************************************************************/


#if (CLEAN_EXIT)
static void FreeFontStuff(struct MPEGPlayerLib *MPEGPlayerBase)
{
    StripFont(&MPEGPlayerBase->mp_Compact24);
    StripFont(&MPEGPlayerBase->mp_Compact31);
}
#endif


/*****************************************************************************/


static ULONG NOOP(void)
{
    return(0);
}


/*****************************************************************************/


BOOL ASM CDPlayerLVO(REG(a6) struct MPEGPlayerLib *MPEGPlayerBase)
{
UWORD             i;
struct Rectangle  textRect;
LONG              excess;
ULONG             bits;
struct MsgPort   *removalPort;
struct IOStdReq  *removalReq;
struct Interrupt  removalInt;
struct MsgPort   *timerPort;
WORD              height;
struct TagItem    vtags[2];
APTR              res;
struct BitMap    *pointerBM;

    bits = SetChipRev(SETCHIPREV_AA);
    if (!(bits & GFXF_AA_LISA))
        return(FALSE);                  /* we need AA */

    MPEGPlayerBase->mp_MainTask = FindTask(NULL);

    pointerBM = AllocBitMap(64,1,2,BMF_CLEAR,NULL);
    MPEGPlayerBase->mp_Pointer = NewObject(NULL,"pointerclass",POINTERA_BitMap, pointerBM, TAG_DONE),

    // install a cd.device disk change handler. This handler signals us
    // whenever the disk is removed or inserted
    MPEGPlayerBase->mp_RemovalSigBit = AllocSignal(-1);
    removalPort                = CreateMsgPort();
    removalReq                 = CreateIORequest(removalPort,sizeof(struct IOStdReq));
    removalInt.is_Code         = (INTFUNC)DiskChangeHandler;
    removalInt.is_Data         = MPEGPlayerBase;
    removalInt.is_Node.ln_Pri  = 0;
    removalInt.is_Node.ln_Type = NT_INTERRUPT;
    OpenDevice("cd.device",0,removalReq,0);
    removalReq->io_Command = CD_ADDCHANGEINT;
    removalReq->io_Data    = &removalInt;
    removalReq->io_Length  = sizeof(removalInt);
    SendIO(removalReq);

    timerPort                      = CreateMsgPort();
    MPEGPlayerBase->mp_TimerReq    = CreateIORequest(timerPort,sizeof(struct timerequest));
    MPEGPlayerBase->mp_TimerSigBit = timerPort->mp_SigBit;
    OpenDevice("timer.device",UNIT_MICROHZ,MPEGPlayerBase->mp_TimerReq,0);

    MPEGPlayerBase->mp_IOPort   = CreateMsgPort();

    // prepare for MPEG IO...
    MPEGPlayerBase->mp_MPEGReq  = (struct IOMPEGReq *)CreateIORequest(MPEGPlayerBase->mp_IOPort,sizeof(struct IOMPEGReq));
    MPEGPlayerBase->mp_MPEGReq2 = (struct IOMPEGReq *)CreateIORequest(MPEGPlayerBase->mp_IOPort,sizeof(struct IOMPEGReq));

    if (OpenDevice("cd32mpeg.device",0,MPEGPlayerBase->mp_MPEGReq,0))
    {
        if (res = FindResident("cd32mpeg.device"))
        {
            InitResident(res,NULL);
            OpenDevice("cd32mpeg.device",0,MPEGPlayerBase->mp_MPEGReq,0);
        }
    }
    MPEGPlayerBase->mp_MPEGReq2->iomr_Req.io_Device = MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Device;
    MPEGPlayerBase->mp_MPEGReq2->iomr_Req.io_Unit   = MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Unit;

    // prepare for audio IO...
    MPEGPlayerBase->mp_AudioReq  = CreateIORequest(MPEGPlayerBase->mp_IOPort,sizeof(struct IOStdReq));
    MPEGPlayerBase->mp_AudioReq2 = CreateIORequest(MPEGPlayerBase->mp_IOPort,sizeof(struct IOStdReq));
    OpenDevice("cd.device",0,MPEGPlayerBase->mp_AudioReq,0);
    OpenDevice("cd.device",0,MPEGPlayerBase->mp_AudioReq2,0);

    // provide initially black color table
    MPEGPlayerBase->mp_BgColors.cp_NumColors  = NUM_COLORS;
    MPEGPlayerBase->mp_BgColors.cp_FirstColor = 0;
    MPEGPlayerBase->mp_BgColors.cp_EndMarker  = 0;
    for (i = 0; i < NUM_COLORS; i++)
    {
        MPEGPlayerBase->mp_BgColors.cp_Colors[i].Red   = 0;
        MPEGPlayerBase->mp_BgColors.cp_Colors[i].Green = 0;
        MPEGPlayerBase->mp_BgColors.cp_Colors[i].Blue  = 0;
    }

    // get background
    GetBM(MPEGPlayerBase,&background,&MPEGPlayerBase->mp_BgBitMap,&MPEGPlayerBase->mp_BgBMInfo);

    // Add an extra plane to the bitmap. BltBitMap() will treat the NULL plane
    // pointer as a completely clear bit-plane.
    // Note that this assumes GetBM() uses BMF_MINPLANES!
    MPEGPlayerBase->mp_BgBitMap->Planes[MPEGPlayerBase->mp_BgBitMap->Depth++] = NULL;

    // allocate the two dbuf bitmaps
    MPEGPlayerBase->mp_DBBitMap[0] = AllocBitMap(MPEGPlayerBase->mp_BgBMInfo->bmi_Width,MPEGPlayerBase->mp_BgBMInfo->bmi_Height, NUM_PLANES, BMF_DISPLAYABLE | BMF_INTERLEAVED, NULL);
    MPEGPlayerBase->mp_DBBitMap[1] = AllocBitMap(MPEGPlayerBase->mp_BgBMInfo->bmi_Width,MPEGPlayerBase->mp_BgBMInfo->bmi_Height, NUM_PLANES, BMF_DISPLAYABLE | BMF_INTERLEAVED, NULL);

    // copy the background bitmap into the two dbuf bitmaps
    BltBitMap(MPEGPlayerBase->mp_BgBitMap,0,0,
              MPEGPlayerBase->mp_DBBitMap[0],0,0,
              MPEGPlayerBase->mp_BgBMInfo->bmi_Width,MPEGPlayerBase->mp_BgBMInfo->bmi_Height,
              0xc0,0xff,NULL);
    BltBitMap(MPEGPlayerBase->mp_BgBitMap,0,0,
              MPEGPlayerBase->mp_DBBitMap[1],0,0,
              MPEGPlayerBase->mp_BgBMInfo->bmi_Width,MPEGPlayerBase->mp_BgBMInfo->bmi_Height,
              0xc0,0xff,NULL);

    // get more debox stuff we need
    GetBM(MPEGPlayerBase,&icons,&MPEGPlayerBase->mp_IconBitMap,&MPEGPlayerBase->mp_IconBMInfo);
    GetBM(MPEGPlayerBase,&arrows,&MPEGPlayerBase->mp_ArrowBitMap,&MPEGPlayerBase->mp_ArrowBMInfo);

    PrepareCDG(MPEGPlayerBase);

    SystemControl(SCON_AddCreateKeys,0,
                  SCON_AddCreateKeys,1,
                  SCON_AddCreateKeys,2,
                  SCON_AddCreateKeys,3,
                  TAG_DONE);

    /* get the overscan values for this display id */
    QueryOverscan(HIRESLACE_KEY,&textRect,OSCAN_TEXT);

    D(("Overscan Text MinX %ld MinY %ld MaxX %ld MaxY %ld\n",(ULONG)textRect.MinX,(ULONG)textRect.MinY,(ULONG)textRect.MaxX,(ULONG)textRect.MaxY));

    D(("pic width %ld height %ld\n",(ULONG)MPEGPlayerBase->mp_BgBMInfo->bmi_Width,(ULONG)MPEGPlayerBase->mp_BgBMInfo->bmi_Height));

    /* adjust things so our display is horizontally centered relative to
     * text oscan
     */
    excess         = MPEGPlayerBase->mp_BgBMInfo->bmi_Width - (textRect.MaxX - textRect.MinX + 1);

    excess -= 4;
    textRect.MinX -= excess / 2;
    textRect.MaxX += (excess - (excess / 2));
    textRect.MinX += 8;
    textRect.MaxX += 8;

    /* adjust things so our display is vertically centered relative to
     * text oscan
     */
    if (PAL & GfxBase->DisplayFlags)
        height = MPEGPlayerBase->mp_BgBMInfo->bmi_Height;
    else
        height = 468;

    excess         = height - (textRect.MaxY - textRect.MinY + 1);
    textRect.MinY -= excess / 2;
    textRect.MaxY += (excess - (excess / 2));

    D(("Adjusted Overscan MinX %ld MinY %ld MaxX %ld MaxY %ld\n",(ULONG)textRect.MinX,(ULONG)textRect.MinY,(ULONG)textRect.MaxX,(ULONG)textRect.MaxY));

#if (!CLEAN_EXIT)
	/* I have removed this NUKEPOINTER call since it effectively
	 * tells Intuition not to call any sprite related functions.
	 * That means setting the windows sprite specifically to a
	 * blank image is ignored.  Fortunately this screen is black
	 * when its opened, so the sprite won't show before we get around
	 * to modifying the colors in the event loop

    IBASE_NUKEPOINTER(IntuitionBase) = (STRPTR)TRUE;


	 */


    /* Patching ScreenDepth() to fail prevents Amiga-M/N from having
     * any effect.  This is required to support CD+G.  We could
     * alternately apply an input-handler to catch those keys.
     * An input handler is easier to remove, but the SetFunction()
     * is easier to apply.  Since we exit through reboot, this
     * seems good enough.
     */

    MPEGPlayerBase->mp_ScreenDepth = (DEPTHFUNC)SetFunction(IntuitionBase, -786, NOOP);
#endif

    vtags[0].ti_Tag  = VC_IntermediateCLUpdate;
    vtags[0].ti_Data = FALSE;
    vtags[1].ti_Tag  = TAG_DONE;

    MPEGPlayerBase->mp_Screen = OpenScreenTags(NULL,
                                 SA_Depth,           NUM_PLANES,
                                 SA_DisplayID,       HIRESLACE_KEY,
                                 SA_Interleaved,     TRUE,
                                 SA_Draggable,       FALSE,
                                 SA_Quiet,           TRUE,
                                 SA_ShowTitle,       FALSE,
                                 SA_Colors32,        &MPEGPlayerBase->mp_BgColors,
                                 SA_DClip,           &textRect,
                                 SA_BackFill,        LAYERS_NOBACKFILL,
                                 SA_BitMap,          MPEGPlayerBase->mp_DBBitMap[0],
                                 SA_MinimizeISG,     TRUE,
                                 SA_Exclusive,       TRUE,
                                 SA_VideoControl,    vtags,
                                 TAG_DONE);

    if (!(PAL & GfxBase->DisplayFlags))
    {
        MPEGPlayerBase->mp_Screen->ViewPort.RasInfo->RyOffset += 26;
        ScrollVPort(&MPEGPlayerBase->mp_Screen->ViewPort);
    }

    GetFontStuff(MPEGPlayerBase);

    MPEGPlayerBase->mp_DBBuffers[0] = AllocScreenBuffer(MPEGPlayerBase->mp_Screen, MPEGPlayerBase->mp_DBBitMap[0], 0);
    MPEGPlayerBase->mp_DBBuffers[1] = AllocScreenBuffer(MPEGPlayerBase->mp_Screen, MPEGPlayerBase->mp_DBBitMap[1], 0);

    MPEGPlayerBase->mp_DBNotify = CreateMsgPort();
    MPEGPlayerBase->mp_DBBuffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = MPEGPlayerBase->mp_DBNotify;
    MPEGPlayerBase->mp_DBBuffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = MPEGPlayerBase->mp_DBNotify;

    InitRastPort(&MPEGPlayerBase->mp_DBRastPort[0]);
    InitRastPort(&MPEGPlayerBase->mp_DBRastPort[1]);
    MPEGPlayerBase->mp_DBRastPort[0].BitMap = MPEGPlayerBase->mp_DBBuffers[0]->sb_BitMap;
    MPEGPlayerBase->mp_DBRastPort[1].BitMap = MPEGPlayerBase->mp_DBBuffers[1]->sb_BitMap;
    MPEGPlayerBase->mp_DBCurrentBuffer      = 1;

#if (DOUBLE_BUFFER)
    MPEGPlayerBase->mp_RenderRP             = &MPEGPlayerBase->mp_DBRastPort[1];
#else
    MPEGPlayerBase->mp_RenderRP             = &MPEGPlayerBase->mp_DBRastPort[0];
#endif

    MPEGPlayerBase->mp_Window = OpenWindowTags(NULL,
                                 WA_CustomScreen,  MPEGPlayerBase->mp_Screen,
                                 WA_Borderless,    TRUE,
                                 WA_Backdrop,      TRUE,
                                 WA_RMBTrap,       TRUE,
                                 WA_Activate,      TRUE,
                                 WA_IDCMP,         IDCMP_RAWKEY | IDCMP_INTUITICKS,
                                 WA_RptQueue,      1,
                                 WA_NoCareRefresh, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 WA_BackFill,      LAYERS_NOBACKFILL,
                                 WA_Pointer,       MPEGPlayerBase->mp_Pointer,
                                 TAG_DONE);

    EventLoop(MPEGPlayerBase);

#if (CLEAN_EXIT)
    CloseWindow(MPEGPlayerBase->mp_Window);
    FreeScreenBuffer(MPEGPlayerBase->mp_Screen,MPEGPlayerBase->mp_DBBuffers[0]);
    FreeScreenBuffer(MPEGPlayerBase->mp_Screen,MPEGPlayerBase->mp_DBBuffers[1]);
    DeleteMsgPort(MPEGPlayerBase->mp_DBNotify);
    CloseScreenQuiet(MPEGPlayerBase,MPEGPlayerBase->mp_Screen);
    FreeBitMap(MPEGPlayerBase->mp_DBBitMap[0]);
    FreeBitMap(MPEGPlayerBase->mp_DBBitMap[1]);
    FreeBMInfo(MPEGPlayerBase->mp_BgBMInfo);
    FreeBitMap(MPEGPlayerBase->mp_IconBitMap);
    FreeBMInfo(MPEGPlayerBase->mp_IconBMInfo);
    FreeBitMap(MPEGPlayerBase->mp_ArrowBitMap);
    FreeBMInfo(MPEGPlayerBase->mp_ArrowBMInfo);
    FreeFontStuff(MPEGPlayerBase);

    MPEGPlayerBase->mp_BgBitMap->Depth--;
    FreeBitMap(MPEGPlayerBase->mp_BgBitMap);

    ShutdownCDG(MPEGPlayerBase);

    removalReq->io_Command = CD_REMCHANGEINT;
    DoIO(removalReq);
    CloseDevice(removalReq);
    DeleteIORequest(removalReq);
    DeleteMsgPort(removalPort);

    CloseDevice(MPEGPlayerBase->mp_MPEGReq);
    DeleteIORequest(MPEGPlayerBase->mp_MPEGReq);
    DeleteIORequest(MPEGPlayerBase->mp_MPEGReq2);

    CloseDevice(MPEGPlayerBase->mp_AudioReq);
    CloseDevice(MPEGPlayerBase->mp_AudioReq2);
    DeleteIORequest(MPEGPlayerBase->mp_AudioReq);
    DeleteIORequest(MPEGPlayerBase->mp_AudioReq2);

    DeleteMsgPort(MPEGPlayerBase->mp_IOPort);

    FreeSignal(MPEGPlayerBase->mp_RemovalSigBit);

    CloseDevice(MPEGPlayerBase->mp_TimerReq);
    DeleteIORequest(MPEGPlayerBase->mp_TimerReq);
    DeleteMsgPort(timerPort);

    DisposeObject(MPEGPlayerBase->mp_Pointer);
    FreeBitMap(pointerBM);

    SystemControl(SCON_RemCreateKeys,0,
                  SCON_RemCreateKeys,1,
                  SCON_RemCreateKeys,2,
                  SCON_RemCreateKeys,3,
                  TAG_DONE);

#else

    ColdReboot();

#endif
}
