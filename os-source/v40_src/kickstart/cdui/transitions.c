
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/pointerclass.h>
#include <graphics/videocontrol.h>
#include <graphics/layers.h>
#include <graphics/sprite.h>
#include <cdtv/debox.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "cduibase.h"
#include "transitions.h"
#include "sound.h"
#include "cycler.h"
#include "utils.h"
#include "displayanim.h"


/*****************************************************************************/


#define SCR_X_OFFSET (0)
#define SCR_Y_OFFSET (0)

#define MAIN_CD_BITMAP_Y_OFFSET 108
#define MAIN_CD_BITMAP_X_OFFSET 380
#define MAIN_CD_SPRITE_Y_OFFSET 100
#define MAIN_CD_SPRITE_X_OFFSET 377
#define EXIT_CD_SPRITE_Y_OFFSET 10
#define EXIT_SCREEN_Y_OFFSET    44
#define EXIT_SCREEN_Y_DELTA     90


/*****************************************************************************/


struct ColorBatch16
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[16];
    UWORD           cb_EndMarker;
};

struct ColorBatch32
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[32];
    UWORD           cb_EndMarker;
};

struct ColorBatch256
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[256];
    UWORD           cb_EndMarker;
};


/*****************************************************************************/


/* debox magic data */
extern UBYTE * __far background;
extern UBYTE * __far cds;
extern UBYTE * __far cd;
extern UBYTE * __far amiga32_red;
extern UBYTE * __far palette0;
extern UBYTE * __far palette1;
extern UBYTE * __far palette2;
extern UBYTE * __far palette3;
extern UBYTE disc_in[];


/*****************************************************************************/


#undef SysBase

static VOID FlipCDIn(VOID)
{
ULONG            i;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;
LONG             x, y;
BOOL             aborting;
WORD             frameCount;
LONG             delta;
WORD             wait;
WORD             flipCount;

    SysBase  = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    i          = 0;
    x          = -200;
    y          = CDUIBase->cb_CDYOffset;
    aborting   = FALSE;
    frameCount = 80;
    delta      = 15;
    wait       = 60;
    flipCount  = 1;

    while (TRUE)
    {
        if (x == MAIN_CD_SPRITE_X_OFFSET)
        {
            if (wait)
                wait--;

            if (!wait)
                aborting = TRUE;
        }

        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i],CDUIBase->cb_Sprites[i+4],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1],CDUIBase->cb_Sprites[i+1+4],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2],CDUIBase->cb_Sprites[i+2+4],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+3],CDUIBase->cb_Sprites[i+3+4],NULL);

        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+4],x,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2+4],x+86,y);

        WaitTOF();

        if (!flipCount--)
        {
            flipCount = 1;

            i += 4;
            if (i >= 36)
                i = 0;
        }

        if (!frameCount--)
        {
            delta -= 5;
            frameCount = 11;
        }

        if (!i && aborting)
            break;

        if (x < MAIN_CD_SPRITE_X_OFFSET)
            x += delta;
        else
            x = MAIN_CD_SPRITE_X_OFFSET;
    }

    Wait(SIGBREAKF_CTRL_C);

    Forbid();
    Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
    CDUIBase->cb_FlipperTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


//#define ICOLOR(c) (*((SHORT *)0xdff180)=(c))
#define ICOLOR(c) ;

#undef SysBase

static VOID FlipCD(VOID)
{
LONG             i,j;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;
BOOL             aborting;
WORD             x;
WORD             frameCount;

    SysBase  = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    i        = 0;
    aborting = FALSE;

    CDUIBase->cb_Sprites[0]->es_SimpleSprite.x = MAIN_CD_SPRITE_X_OFFSET;
    CDUIBase->cb_Sprites[1]->es_SimpleSprite.x = MAIN_CD_SPRITE_X_OFFSET;
    CDUIBase->cb_Sprites[2]->es_SimpleSprite.x = MAIN_CD_SPRITE_X_OFFSET + 86;
    CDUIBase->cb_Sprites[3]->es_SimpleSprite.x = MAIN_CD_SPRITE_X_OFFSET + 86;

    while (TRUE)
    {
        if (SetSignal(0,0) & SIGBREAKF_CTRL_C)
            aborting = TRUE;

        if (SetSignal(0,SIGBREAKF_CTRL_D) & SIGBREAKF_CTRL_D)
        {
            for (j = 0; j < 36; j++)
                CDUIBase->cb_Sprites[j]->es_SimpleSprite.y = EXIT_CD_SPRITE_Y_OFFSET;

            Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
            Wait(SIGBREAKF_CTRL_D);
        }

        if (SetSignal(0,0) & SIGBREAKF_CTRL_E)
            break;

        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i],CDUIBase->cb_Sprites[i+4],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1],CDUIBase->cb_Sprites[i+1+4],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2],CDUIBase->cb_Sprites[i+2+4],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+3],CDUIBase->cb_Sprites[i+3+4],NULL);

        for (j = 0; j < 3; j++)
            WaitTOF();

        i += 4;
        if (i >= 36)
            i = 0;

        if (!i && aborting)
            break;
    }

    if (SetSignal(0,0) & SIGBREAKF_CTRL_E)
    {
        Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
        Wait(SIGBREAKF_CTRL_E);

        x = MAIN_CD_SPRITE_X_OFFSET;
        frameCount = 0;

        SetAPen(CDUIBase->cb_RastPort,0);

        // move CD towards the right, kicking out the 32...
        while (TRUE)
        {
            if (x >= MAIN_CD_SPRITE_X_OFFSET + 50)
                ScrollRaster(CDUIBase->cb_RastPort,-18,0,500,0,
                             CDUIBase->cb_Screen->Width - 1, 48);

            if (x >= MAIN_CD_SPRITE_X_OFFSET + 167)
                break;

            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+4],CDUIBase->cb_Sprites[i],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1+4],CDUIBase->cb_Sprites[i+1],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2+4],CDUIBase->cb_Sprites[i+2],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+3+4],CDUIBase->cb_Sprites[i+3],NULL);

            MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i],x,EXIT_CD_SPRITE_Y_OFFSET);
            MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2],x+86,EXIT_CD_SPRITE_Y_OFFSET);

            WaitTOF();

            if (!frameCount)
            {
                frameCount = 2;

                i += 4;
                if (i >= 36)
                    i = 0;
            }
            frameCount--;
            x += 7;
        }
        x -= 7;

        frameCount = 0;
        while (TRUE)
        {
            if (x >= -70)
            {
                RectFill(CDUIBase->cb_RastPort,
                         x+70,0,x+79,CDUIBase->cb_Screen->Height - 1);
            }

            if (x <= -100)
                break;

            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+4],CDUIBase->cb_Sprites[i],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1+4],CDUIBase->cb_Sprites[i+1],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2+4],CDUIBase->cb_Sprites[i+2],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+3+4],CDUIBase->cb_Sprites[i+3],NULL);

            MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i],x,EXIT_CD_SPRITE_Y_OFFSET);
            MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+2],x+86,EXIT_CD_SPRITE_Y_OFFSET);

            WaitTOF();

            if (!frameCount)
            {
                frameCount = 2;

                i -= 4;
                if (i < 0)
                    i = 32;
            }
            frameCount--;

            x -= 9;
        }
        SetRast(CDUIBase->cb_RastPort,0);
    }

    Wait(SIGBREAKF_CTRL_C);

    Forbid();
    Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
    CDUIBase->cb_FlipperTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


static VOID MakeCDs(struct CDUILib *CDUIBase, WORD x)
{
struct BitMap      *tmpBM;
struct BitMap      *cdsBM;
struct BMInfo      *cdsBMInfo;
UWORD               i,j;

    GetBM(CDUIBase,&cds,&cdsBM,&cdsBMInfo);

    tmpBM = AllocBitMap(64,45,4,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    for (i = 0; i < 18; i++)
    {
        BltBitMap(cdsBM,(i % 6) * 43, (i / 6) * 44,
                  tmpBM,0,0,
                  43,44,0xc0,0xff,NULL);

        WaitBlit();

        for (j = 0; j < 2; j++)
        {
            CDUIBase->cb_Sprites[i*2+j] = AllocSpriteData(tmpBM,
                                                          SPRITEA_Width,    43,
                                                          SPRITEA_Attached, j & 1,
                                                          TAG_DONE);
        }
    }
    FreeBitMap(tmpBM);
    FreeBM(CDUIBase,cdsBM,cdsBMInfo);

    for (i = 0; i < 4; i++)
    {
        GetExtSprite(CDUIBase->cb_Sprites[i],GSTAG_SPRITE_NUM,i+1,TAG_DONE);
        CDUIBase->cb_Sprites[i]->es_SimpleSprite.num = i;
    }

    /* miror the first four sprites after the four last one, makes other code
     * much simpler, trust me
     */
    for (i = 36; i < 40; i++)
        CDUIBase->cb_Sprites[i] = CDUIBase->cb_Sprites[i - 36];

    RemakeDisplay();

    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[0],x,MAIN_CD_SPRITE_Y_OFFSET);
    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[1],x,MAIN_CD_SPRITE_Y_OFFSET);
    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[2],x + 86,MAIN_CD_SPRITE_Y_OFFSET);
    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[3],x + 86,MAIN_CD_SPRITE_Y_OFFSET);
}


/*****************************************************************************/


static VOID NukeCDs(struct CDUILib *CDUIBase)
{
UWORD i;

    for (i = 1; i < 5; i++)
        FreeSprite(i);

    for (i = 0; i < 36; i++)
        FreeSpriteData(CDUIBase->cb_Sprites[i]);
}


/*****************************************************************************/


static const cmsVCTags[] =
{
    VC_IntermediateCLUpdate, FALSE,
    VTAG_SPODD_BASE_SET,     80,
    VTAG_SPEVEN_BASE_SET,    80,
    VTAG_SPRITERESN_SET,     SPRITERESN_140NS,
    TAG_DONE
};

static struct Screen *CreateMainScreen(struct CDUILib *CDUIBase, Tag tag, ...)
{
    return (OpenScreenTags(NULL,
                           SA_VideoControl, cmsVCTags,
                           TAG_MORE,        (struct TagItem *)&tag));
}


/*****************************************************************************/


static const cbsVCTags[] =
{
    VC_IntermediateCLUpdate, FALSE,
    VTAG_SPODD_BASE_SET,     16,
    VTAG_SPEVEN_BASE_SET,    16,
    VTAG_SPRITERESN_SET,     SPRITERESN_140NS,
    TAG_DONE
};

static struct Screen *CreateBootingScreen(struct CDUILib *CDUIBase,
                                          struct BitMap *bm,
                                          struct BMInfo *bmInfo,
                                          APTR colors)
{
struct Rectangle  rect;
struct Screen    *sp;

    QueryOverscan(HIRESLACE_KEY,&rect,OSCAN_MAX);
    rect.MinX += 8;
    rect.MinY += EXIT_SCREEN_Y_DELTA;
    rect.MaxY  = rect.MinY + bmInfo->bmi_Height - 1;

    sp = OpenScreenTags(NULL,
                        SA_Width,           716,
                        SA_Height,          bmInfo->bmi_Height,
                        SA_Depth,           bmInfo->bmi_Depth,
                        SA_DisplayID,       HIRESLACE_KEY,
                        SA_BitMap,          bm,
                        SA_Draggable,       FALSE,
                        SA_Exclusive,       TRUE,
                        SA_Behind,          TRUE,
                        SA_Quiet,           TRUE,
                        SA_ShowTitle,       FALSE,
                        SA_DClip,           &rect,
                        SA_VideoControl,    cbsVCTags,
                        SA_Colors32,        colors,
                        SA_BackFill,        LAYERS_NOBACKFILL,
                        TAG_DONE);

    return(sp);
}


/*****************************************************************************/


static void CreateWindow(struct CDUILib *CDUIBase, BOOL flipper)
{
struct Window *oldWindow;

    // hold off the CD flipper while we change things...
    if (flipper)
        SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_D);

    CDUIBase->cb_RastPort = &CDUIBase->cb_Screen->RastPort;
    CDUIBase->cb_BitMap   = CDUIBase->cb_RastPort->BitMap;
    CDUIBase->cb_ViewPort = &CDUIBase->cb_Screen->ViewPort;

    // CD flipper can resume
    if (flipper)
    {
        ScreenToFront(CDUIBase->cb_Screen);
        Signal(CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_D);
    }

    oldWindow = CDUIBase->cb_Window;

    CDUIBase->cb_Window = OpenWindowTags(NULL,
                                 WA_CustomScreen,  CDUIBase->cb_Screen,
                                 WA_Borderless,    TRUE,
                                 WA_Backdrop,      TRUE,
                                 WA_Activate,      TRUE,
                                 WA_RMBTrap,       TRUE,
                                 WA_Pointer,       CDUIBase->cb_Pointer,
                                 WA_RptQueue,      1,
                                 WA_NoCareRefresh, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 WA_BackFill,      LAYERS_NOBACKFILL,
                                 TAG_DONE);

    if (oldWindow)
        CloseWindow(oldWindow);
}


/*****************************************************************************/


static void DestroyWindow(struct CDUILib *CDUIBase)
{
    CloseWindow(CDUIBase->cb_Window);
    CDUIBase->cb_Window = NULL;
}


/*****************************************************************************/


void CloseMainScreen(struct CDUILib *CDUIBase)
{
    DestroyWindow(CDUIBase);
    IBASE_NUKEPOINTER(IntuitionBase) = (STRPTR)FALSE;
    CloseScreenQuiet(CDUIBase,CDUIBase->cb_Screen);
    FreeBM(CDUIBase,CDUIBase->cb_BackgroundBM,NULL);
}


/*****************************************************************************/


void NothingToMain(struct CDUILib *CDUIBase)
{
struct Rectangle      rect;
struct Screen        *oldScreen;
struct BitMap        *bm;
struct BMInfo        *bmInfo;
struct BMInfo        *backgroundBMInfo;
struct ColorBatch256 *colors256;
struct ColorBatch16  *colors16;
struct ColorBatch32   black;
ULONG                 i;
struct BitMap        *paletteBM;
struct BMInfo        *paletteBMInfo;

    /* Open shallow screen
     * Start fanfare
     * Move disc into view
     * Open expensive screen
     * Close low-cost screen
     * Fade in aurora
     * Start CD flipping
     * Move CD in from left to its target position
     * Render CD bitmap
     * Stop CD flipping
     */

    IBASE_NUKEPOINTER(IntuitionBase) = (STRPTR)TRUE;

    black.cb_NumColors  = 32;
    black.cb_FirstColor = 0;
    black.cb_EndMarker  = 0;
    for (i = 0; i < 32; i++)
    {
        black.cb_Colors[i].Red   = 0;
        black.cb_Colors[i].Green = 0;
        black.cb_Colors[i].Blue  = 0;
    }

    /* Activate the audio device */
    InitResident (FindResident ("audio.device"), NULL);

    GetBM(CDUIBase,&palette3,&paletteBM,&paletteBMInfo);
    CDUIBase->cb_LowCostColors = (struct ColorBatch32 *)GetBMInfoRGB32(paletteBMInfo,32,0);
    FreeBM(CDUIBase,paletteBM,paletteBMInfo);
    GetBM(CDUIBase,&amiga32_red,&CDUIBase->cb_LowCostBM,&CDUIBase->cb_LowCostBMInfo);
    CDUIBase->cb_LowCostScreen = CreateBootingScreen(CDUIBase,CDUIBase->cb_LowCostBM,CDUIBase->cb_LowCostBMInfo,&black);

    colors256 = AllocVec(sizeof(struct ColorBatch256), MEMF_ANY);

    // decompress main background
    GetBM(CDUIBase,&background,&CDUIBase->cb_BackgroundBM,&backgroundBMInfo);
    CDUIBase->cb_BitMapColors = (struct ColorBatch256 *)GetBMInfoRGB32(backgroundBMInfo,256,0);

    *colors256 = *((struct ColorBatch256 *)CDUIBase->cb_BitMapColors);
    /* The colors used for the Amiga logo's border need to come up black.
     * However, we need the color values in the picture file itself,
     * because we'll fade them in from black.  So let's nuke the colors
     * in the copy of the color-table we'll use to open the screen.
     */
    for ( i = CYCLE_AMIGABORDER_FIRST; i <= CYCLE_AMIGABORDER_LAST; i++ )
    {
        colors256->cb_Colors[i].Red   = 0;
        colors256->cb_Colors[i].Green = 0;
        colors256->cb_Colors[i].Blue  = 0;
    }

    // decompress animation's colors
    bmInfo   = DecompBMInfo(NULL, NULL, &palette0);
    colors16 = (struct ColorBatch16 *)GetBMInfoRGB32(bmInfo,16,0);

    // decompress the cycling aurora colors
    CDUIBase->cb_AuroraBM[0]     = DecompBMInfo(NULL, NULL, &palette1);
    CDUIBase->cb_AuroraColors[0] = GetBMInfoRGB32(CDUIBase->cb_AuroraBM[0],256,0);
    CDUIBase->cb_AuroraBM[1]     = DecompBMInfo(NULL, NULL, &palette2);
    CDUIBase->cb_AuroraColors[1] = GetBMInfoRGB32(CDUIBase->cb_AuroraBM[1],256,0);

    // activate sound sub-task
    CDUIBase->cb_FanfareTask = CreateTask("Fanfare",21,FanfarePlayer,1024);
    SyncSignal(CDUIBase,CDUIBase->cb_FanfareTask,SIGBREAKF_CTRL_D);

    QueryOverscan(HIRESLACE_KEY,&rect,OSCAN_MAX);
    rect.MinX += 8;

    CDUIBase->cb_Screen = CreateMainScreen(CDUIBase,
                             SA_Width,      716,
                             SA_Height,     565,
                             SA_Depth,      4,
                             SA_DisplayID,  HIRESLACE_KEY,
                             SA_Draggable,  FALSE,
                             SA_Exclusive,  TRUE,
                             SA_Quiet,      TRUE,
                             SA_ShowTitle,  FALSE,
                             SA_DClip,      &rect,
                             SA_Colors32,   &black,
                             SA_BackFill,   LAYERS_NOBACKFILL,
                             TAG_DONE);

    CreateWindow(CDUIBase,FALSE);

    oldScreen = CDUIBase->cb_Screen;

    CDUIBase->cb_Screen = CreateMainScreen(CDUIBase,
                             SA_Width,           716,
                             SA_Height,          565,
                             SA_Depth,           8,
                             SA_DisplayID,       HIRESLACE_KEY,
                             SA_BitMap,          CDUIBase->cb_BackgroundBM,
                             SA_Draggable,       FALSE,
                             SA_Exclusive,       TRUE,
                             SA_Quiet,           TRUE,
                             SA_Behind,          TRUE,
                             SA_ShowTitle,       FALSE,
                             SA_DClip,           &rect,
                             SA_Colors32,        colors256,
                             SA_BackFill,        LAYERS_NOBACKFILL,
                             TAG_DONE);

    DisplayAnim(oldScreen,disc_in,CDUIBase,(APTR)colors16);

    FreeBMInfoRGB32((ULONG *)colors16);
    FreeBMInfo(bmInfo);

    CreateWindow(CDUIBase,FALSE);

    FreeBMInfo(backgroundBMInfo);

    ScreenToFront(CDUIBase->cb_Screen);
    CloseScreenQuiet(CDUIBase,oldScreen);

    GetBM(CDUIBase,&cd,&bm,&bmInfo);
    MakeCDs(CDUIBase, -170);

    CDUIBase->cb_BgSaveBM = AllocBitMap(168,70,8,BMF_INTERLEAVED,NULL);
    BltBitMap(CDUIBase->cb_BitMap,MAIN_CD_BITMAP_X_OFFSET,MAIN_CD_BITMAP_Y_OFFSET,
              CDUIBase->cb_BgSaveBM,0,0,
              168,70,0xc0,0xff,NULL);

    CDUIBase->cb_CyclerTask = CreateTask("Cycler",21,ColorCycler,8192);

    CDUIBase->cb_CDYOffset = MAIN_CD_SPRITE_Y_OFFSET;
    CDUIBase->cb_FlipperTask = CreateTask("Flipper",22,FlipCDIn,4096);
    SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_C);

    BltBitMap(bm,0,0,
              CDUIBase->cb_BitMap,MAIN_CD_BITMAP_X_OFFSET,MAIN_CD_BITMAP_Y_OFFSET,
              168,70,0xc0,0xff,NULL);

    NukeCDs(CDUIBase);
    FreeBM(CDUIBase,bm,bmInfo);
    FreeVec(colors256);
}


/*****************************************************************************/


void NothingToBooting(struct CDUILib *CDUIBase)
{
struct BMInfo       *bmInfo;
struct ColorBatch32 *colors;
struct BMInfo       *paletteBMInfo;
struct BitMap       *paletteBM;

    /* Open low-cost screen
     * Create CD
     * Start flipping CD
     * Move CD in from left to its target position
     * Keep flipping CD
     */

    IBASE_NUKEPOINTER(IntuitionBase) = (STRPTR)TRUE;

    GetBM(CDUIBase,&palette3,&paletteBM,&paletteBMInfo);
    colors = (struct ColorBatch32 *)GetBMInfoRGB32(paletteBMInfo,32,0);

    GetBM(CDUIBase,&amiga32_red,&CDUIBase->cb_BackgroundBM,&bmInfo);
    CDUIBase->cb_Screen = CreateBootingScreen(CDUIBase,CDUIBase->cb_BackgroundBM,bmInfo,colors);
    ScreenToFront(CDUIBase->cb_Screen);

    CreateWindow(CDUIBase,FALSE);

    MakeCDs(CDUIBase,-170);

    FreeBMInfo(bmInfo);
    FreeBMInfoRGB32((ULONG *)colors);
    FreeBM(CDUIBase,paletteBM,paletteBMInfo);

    CDUIBase->cb_CDYOffset = EXIT_CD_SPRITE_Y_OFFSET;
    CDUIBase->cb_FlipperTask = CreateTask("Flipper",22,FlipCDIn,4096);
    SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_C);
    CDUIBase->cb_FlipperTask = CreateTask("Flipper",22,FlipCD,4096);
}


/*****************************************************************************/


void DoorClosedToMain(struct CDUILib *CDUIBase)
{
struct BitMap *bm;
struct BMInfo *bmInfo;

    /* stop flipping CD, restoring bitmap version */

    GetBM(CDUIBase,&cd,&bm,&bmInfo);

    SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_C);

    BltBitMap(bm,0,0,
              CDUIBase->cb_BitMap,MAIN_CD_BITMAP_X_OFFSET,MAIN_CD_BITMAP_Y_OFFSET,
              168,70,0xc0,0xff,NULL);
    WaitBlit();

    NukeCDs(CDUIBase);
    FreeBM(CDUIBase,bm,bmInfo);
}


/*****************************************************************************/


void DoorClosedToBooting(struct CDUILib *CDUIBase)
{
struct Screen *oldScreen;

    /* Fly disc off the screen
     * Fade out stars and aurora
     * Open low-cost screen
     * Close expensive screen
     */

    oldScreen = CDUIBase->cb_Screen;

    CDUIBase->cb_Screen = CDUIBase->cb_LowCostScreen;
    LoadRGB32(&CDUIBase->cb_Screen->ViewPort,CDUIBase->cb_LowCostColors);
    FreeBMInfoRGB32((ULONG *)CDUIBase->cb_LowCostColors);
    FreeBMInfo(CDUIBase->cb_LowCostBMInfo);

    // CTRL_D is a request to fade out leaving an image which matches
    // the low-cost screen
    SyncSignal(CDUIBase,CDUIBase->cb_CyclerTask,SIGBREAKF_CTRL_D);

    FreeBMInfoRGB32(CDUIBase->cb_BitMapColors);
    FreeBMInfoRGB32(CDUIBase->cb_AuroraColors[1]);
    FreeBMInfo(CDUIBase->cb_AuroraBM[1]);
    FreeBMInfoRGB32(CDUIBase->cb_AuroraColors[0]);
    FreeBMInfo(CDUIBase->cb_AuroraBM[0]);
    FreeBitMap(CDUIBase->cb_BgSaveBM);

    CreateWindow(CDUIBase,TRUE);

    CloseScreenQuiet(CDUIBase,oldScreen);
    FreeBM(CDUIBase,CDUIBase->cb_BackgroundBM,NULL);

    CDUIBase->cb_BackgroundBM = CDUIBase->cb_LowCostBM;
}


/*****************************************************************************/


void DoorClosedToClosing(struct CDUILib *CDUIBase)
{
    /* fade out */

    // CTRL_C is a request to fade the cycler out completely
    SyncSignal(CDUIBase,CDUIBase->cb_CyclerTask,SIGBREAKF_CTRL_C);

    FreeBMInfoRGB32(CDUIBase->cb_BitMapColors);
    FreeBMInfoRGB32(CDUIBase->cb_AuroraColors[1]);
    FreeBMInfo(CDUIBase->cb_AuroraBM[1]);
    FreeBMInfoRGB32(CDUIBase->cb_AuroraColors[0]);
    FreeBMInfo(CDUIBase->cb_AuroraBM[0]);
    FreeBitMap(CDUIBase->cb_BgSaveBM);

    SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_C);

    NukeCDs(CDUIBase);

    CloseMainScreen(CDUIBase);

    if (CDUIBase->cb_FanfareTask)
        SyncSignal(CDUIBase,CDUIBase->cb_FanfareTask,SIGBREAKF_CTRL_C);
}


/*****************************************************************************/


void MainToDoorClosed(struct CDUILib *CDUIBase)
{
    MakeCDs(CDUIBase,MAIN_CD_SPRITE_X_OFFSET);

    BltBitMap(CDUIBase->cb_BgSaveBM,0,0,
              CDUIBase->cb_BitMap,MAIN_CD_BITMAP_X_OFFSET,MAIN_CD_BITMAP_Y_OFFSET,
              168,70,0xc0,0xff,NULL);
    WaitBlit();

    CDUIBase->cb_FlipperTask = CreateTask("Flipper",22,FlipCD,4096);
}


/*****************************************************************************/


void MainToClosing(struct CDUILib *CDUIBase)
{
    /* fade out, and shut down */

    // CTRL_C is a request to fade the cycler out completely
    SyncSignal(CDUIBase,CDUIBase->cb_CyclerTask,SIGBREAKF_CTRL_C);

    FreeBMInfoRGB32(CDUIBase->cb_BitMapColors);
    FreeBMInfoRGB32(CDUIBase->cb_AuroraColors[1]);
    FreeBMInfo(CDUIBase->cb_AuroraBM[1]);
    FreeBMInfoRGB32(CDUIBase->cb_AuroraColors[0]);
    FreeBMInfo(CDUIBase->cb_AuroraBM[0]);
    FreeBitMap(CDUIBase->cb_BgSaveBM);

    CloseMainScreen(CDUIBase);

    if (CDUIBase->cb_FanfareTask)
        SyncSignal(CDUIBase,CDUIBase->cb_FanfareTask,SIGBREAKF_CTRL_C);
}


/*****************************************************************************/


void BootingToClosing(struct CDUILib *CDUIBase)
{
    /* Move flipping CD towards the right, erasing 32
     * Reverse flip direction on CD
     * Move flipping CD towards the left, erasing Amiga
     * Clear to black
     * Close screen
     */

    SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_E);
    SyncSignal(CDUIBase,CDUIBase->cb_FlipperTask,SIGBREAKF_CTRL_C);

    NukeCDs(CDUIBase);

    CloseMainScreen(CDUIBase);

    if (CDUIBase->cb_FanfareTask)
        SyncSignal(CDUIBase,CDUIBase->cb_FanfareTask,SIGBREAKF_CTRL_C);
}
