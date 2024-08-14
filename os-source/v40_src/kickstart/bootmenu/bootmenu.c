
/* Define the symbol NO_DIAGNOSTIC to compile a version of this file that
 * doesn't include the "syscheck" code
 *
 * Define the symbol CDGS to compile a version of this file specifically
 * for the AmigaCD game system.
 *
 * Define the symbol TEST_VERSION to enable cleanup code, which helps when
 * debugging this puppy
 *
 * Memory allocations in bootmenu aren't checked for success. If they would
 * fail, you would not be able to do much with your computer...
 */

#ifdef MACHINE_A600
#define NO_DIAGNOSTIC
#endif

#ifdef MACHINE_CDGS
#define CDGS
#endif


/*****************************************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <internal/librarytags.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/expansionbase.h>
#include <graphics/text.h>
#include <graphics/rastport.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <graphics/gfxbase.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <string.h>
#include <dos.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#define EXEC_PRIVATE_PRAGMAS
#define GRAPHICS_PRIVATE_PRAGMAS

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/expansion_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>


/*****************************************************************************/


/* The forgotten few */
APTR TaggedOpenLibrary(ULONG libtag);
ULONG SetChipRev(ULONG want);
BOOL SetDefaultMonitor(UWORD mon);
ULONG ReadGayle(void);


/*****************************************************************************/


VOID __asm UnlinkRomTag(register __a0 struct Resident *kt);
#define BOOLEAN UBYTE   /* byte boolean */


/*****************************************************************************/


/* for our magic config recovery stuff */
struct KickTag
{
    struct Resident *kt_TagAddr;
    APTR             kt_NextArray;
};


/*****************************************************************************/


/* One of these is allocated for every device found on expansion's MountList */
struct BMNode
{
    struct Node bn_Link;
    char        bn_Name[32];        /* where the name is stored  */
    char        bn_Info[65];        /* info string, for listview */
    BOOLEAN     bn_NeverEnable;     /* disabled on the RDB       */
};

/* make use of two otherwise wasted fields in the Node structure */
#define bn_Enabled     bn_Link.ln_Pri        /* enabled/disabled by user */
#define bn_TempEnabled bn_Link.ln_Type

/* offset from the start of the BMNode structure to the bn_Name field */
#define BNNAMEOFFSET 14


/*****************************************************************************/


/* values for BMConfig.bc_Mode */
#define BMMODE_NTSC 0
#define BMMODE_PAL  1

/* values for BMConfig.bc_Chip */
#define BMCHIP_A    0
#define BMCHIP_ECS  1
#define BMCHIP_BEST 2


/*****************************************************************************/


struct BMConfig
{
    UBYTE           bc_Pad[8];   /* 8 bytes that get trashed by a memory header */

    /* all the stuff needed for the magic "find myself after reboot" code */

    struct Resident bc_RomTag;
    struct KickTag  bc_KickTag;
    struct MemList  bc_KickMem;
    struct MemEntry bc_DevicesMem;

    /* the actual config info */

    struct MinList  bc_Devices;         /* list of BMNodes                    */
    struct BMNode  *bc_BootDevice;      /* device to boot from                */
    ULONG           bc_Mode;            /* requested display mode             */
    ULONG           bc_Chip;            /* requested chip rev                 */
    ULONG           bc_DisableCaches;   /* do we want to turn off the caches? */
    BOOLEAN         bc_StartupSequence; /* do we want the S-S ?               */
    BOOLEAN         bc_SysCheckNode;    /* this tag created by SysCheck       */
};

/* offset from the start of the BMConfig structure to the embedded ROMTag */
#define ROMTAGOFFSET 8


/*****************************************************************************/


#define AC_ORIGINAL 0
#define AC_HR_AGNUS 1
#define AC_ECS      2
#define AC_AA       3

struct GlobalData
{
    struct ExecBase      *gd_SysLib;
    struct ExpansionBase *gd_ExpansionLib;
    struct Library       *gd_GadToolsLib;
    struct Library       *gd_IntuitionLib;
    struct GfxBase       *gd_GfxLib;
    struct Library       *gd_UtilityLib;

    struct Screen        *gd_Screen;
    struct Window        *gd_Window;
    APTR                  gd_VisualInfo;
    struct Gadget        *gd_Gadgets;
    struct Gadget        *gd_LastAdded;
    struct NewGadget     *gd_NewGadget;

    struct Gadget        *gd_DisplayTypeGad;
    struct Gadget        *gd_ChipTypeGad;
    struct Gadget        *gd_StartupGad;
    struct Gadget        *gd_DevicesGad;
    struct Gadget        *gd_DisableCaches;

    STRPTR                gd_ChipTypes[4];
    STRPTR                gd_DisplayTypes[3];

    BOOL                  gd_NTSC;
    UBYTE                 gd_AvailChips;

    struct BMConfig      *gd_Config;
    struct MinList        gd_BootDevices;     /* list of Nodes           */
    struct Node          *gd_BootDev;         /* currently selected node */
};


/*****************************************************************************/


#define ASM           __stdargs __asm
#define REG(x)	      register __ ## x

#define SysBase       gd->gd_SysLib
#define IntuitionBase gd->gd_IntuitionLib
#define GfxBase       gd->gd_GfxLib
#define GadToolsBase  gd->gd_GadToolsLib
#define ExpansionBase gd->gd_ExpansionLib
#define UtilityBase   gd->gd_UtilityLib
#define screen        gd->gd_Screen
#define window        gd->gd_Window
#define visualInfo    gd->gd_VisualInfo
#define gadgets       gd->gd_Gadgets

kprintf(STRPTR,...);

#define MONITOR_PART(id)	((id) & MONITOR_ID_MASK)
#define MODE_PART(id)           ((id) & ~(MONITOR_ID_MASK))

extern struct Custom custom;


/*****************************************************************************/


static const struct TextAttr topazAttr =
{
    "topaz.font",
     8,
     FS_NORMAL,
     FPF_ROMFONT
};


/*****************************************************************************/


#define WIDTH  640
#define HEIGHT 200


/*****************************************************************************/


#define MSG_DIAG_TITLE         "Expansion Board Diagnostic"
#define MSG_DIAG_HDR           "Board Number    Manufacturer      Product         Status"
#define MSG_CONTINUE           "Continue"
#define MSG_WORKING            " Working"
#define MSG_DEFECTIVE          "Defective"

#define MSG_GFX_TITLE          "Display Options"
#define MSG_GFX_DISP_TYPE      "Display Type"
#define MSG_GFX_NTSC           "NTSC"
#define MSG_GFX_PAL            "PAL"
#define MSG_GFX_CHIP_TYPE      "Chip Type"
#define MSG_GFX_A              "Original"
#define MSG_GFX_ECS            "Enhanced"
#define MSG_GFX_BEST           "Best Available"
#define MSG_GFX_USE            "Use"
#define MSG_GFX_CANCEL         "Cancel"

#define MSG_BOOT_TITLE         "Boot Options"
#define MSG_BOOT_USE           "Use"
#define MSG_BOOT_CANCEL        "Cancel"
#define MSG_BOOT_DEVICES       "Select Boot Device"
#define MSG_BOOT_ACTIVEDEVICES "Control Active Devices"
#define MSG_BOOT_ENABLED       "Enabled "
#define MSG_BOOT_DISABLED      "Disabled"
#define MSG_BOOT_DISABLECACHES "Disable CPU Caches"

#define MSG_MAIN_TITLE         "Amiga Early Startup Control"
#define MSG_MAIN_TOGGLE_INFO   "(press a key to toggle the display between NTSC and PAL)"
#define MSG_BOOT               "Boot Options..."
#define MSG_GFX                "Display Options..."
#define MSG_DIAG               "Expansion Board Diagnostic..."
#define MSG_DOBOOT             "Boot"
#define MSG_DOBOOTNOSTARTUP    "Boot With No Startup-Sequence"


/*****************************************************************************/


#define MAIN_BOOT_ID          0
#define MAIN_GFX_ID           1
#define MAIN_DIAG_ID          2
#define MAIN_DOBOOT_ID        3
#define MAIN_DOBOOTNS_ID      4
#define BOOT_USE_ID           5
#define BOOT_CANCEL_ID        6
#define BOOT_STARTUP_ID       7
#define BOOT_DEVICES_ID       8
#define BOOT_ACTIVEDEVICES_ID 9
#define BOOT_DISABLECACHES_ID 10
#define GFX_USE_ID            11
#define GFX_CANCEL_ID         12
#define GFX_CHIPTYPE_ID       13
#define GFX_DISPLAYTYPE_ID    14
#define DIAG_CONTINUE_ID      15


/*****************************************************************************/


static struct Gadget *BMCreateGadget(struct GlobalData *gd, UWORD kind, ULONG tag1, ...)
{
    return(CreateGadgetA(kind,gd->gd_LastAdded,gd->gd_NewGadget,(struct TagItem *)&tag1));
}


/*****************************************************************************/


static struct Gadget *BMCreateGadget2(struct GlobalData *gd, UWORD kind)
{
    return(BMCreateGadget(gd,kind,TAG_DONE));
}


/*****************************************************************************/


static VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)list;
}


/*****************************************************************************/


static struct Node *FindNum(struct List *list, UWORD number)
{
struct Node *node;

    node = list->lh_Head;
    while (node->ln_Succ && number--)
	node = node->ln_Succ;

    return(node);
}


/*****************************************************************************/


static VOID InitGlobalData(struct GlobalData *gd)
{
UWORD            pens = (UWORD)~0;
struct TagItem   taglist[2];
struct BMConfig *config;

    config                                = AllocMem(sizeof(struct BMConfig),MEMF_CLEAR|MEMF_CHIP|MEMF_REVERSE);
    gd->gd_Config                         = config;
    config->bc_KickMem.ml_Node.ln_Type    = NT_KICKMEM;
    config->bc_KickMem.ml_NumEntries      = 1;
    config->bc_KickMem.ml_ME[0].me_Addr   = config;
    config->bc_KickMem.ml_ME[0].me_Length = sizeof(struct BMConfig);
    NewList((struct List *)&config->bc_Devices);
    NewList((struct List *)&gd->gd_BootDevices);

    gd->gd_DisplayTypes[0] = MSG_GFX_NTSC;
    gd->gd_DisplayTypes[1] = MSG_GFX_PAL;
    gd->gd_ChipTypes[0] = MSG_GFX_A;
    gd->gd_ChipTypes[1] = MSG_GFX_ECS;

    gd->gd_NTSC                = (GfxBase->DisplayFlags & NTSC) ? TRUE : FALSE;
    gd->gd_AvailChips          = AC_ORIGINAL;
    config->bc_Mode            = !gd->gd_NTSC;
    config->bc_Chip            = BMCHIP_BEST;
    config->bc_StartupSequence = (ExpansionBase->Flags & EBF_DOSFLAG) ? FALSE : TRUE;
    config->bc_DisableCaches   = FALSE;

    IntuitionBase = TaggedOpenLibrary(OLTAG_INTUITION);
    GfxBase       = TaggedOpenLibrary(OLTAG_GRAPHICS);
    GadToolsBase  = TaggedOpenLibrary(OLTAG_GADTOOLS);

    if (GfxBase->ChipRevBits0 & GFXF_HR_AGNUS)
    {
        gd->gd_AvailChips = AC_HR_AGNUS;
        if (GfxBase->ChipRevBits0 & GFXF_HR_DENISE)
        {
            gd->gd_AvailChips = AC_ECS;
            if (GfxBase->ChipRevBits0 & GFXF_AA_MLISA)
            {
                gd->gd_AvailChips   = AC_AA;
                gd->gd_ChipTypes[2] = MSG_GFX_BEST;
            }
        }
    }

    if (config->bc_Mode > BMMODE_PAL)
        config->bc_Mode = BMMODE_PAL;

    if (config->bc_Chip >= BMCHIP_BEST)
    {
        if (gd->gd_AvailChips == AC_ECS)
            config->bc_Chip = BMCHIP_ECS;
        else
            config->bc_Chip = BMCHIP_BEST;
    }

    taglist[0].ti_Tag  = VTAG_BORDERSPRITE_SET;
    taglist[0].ti_Data = TRUE;
    taglist[1].ti_Tag  = TAG_DONE;

    screen = OpenScreenTags(NULL,SA_Depth,        2,
                                 SA_Font,         &topazAttr,
                                 SA_Type,         CUSTOMSCREEN,
                                 SA_DisplayID,    HIRES_KEY,
                                 SA_Interleaved,  TRUE,
                                 SA_Draggable,    FALSE,
                                 SA_Quiet,        TRUE,
                                 SA_Pens,         &pens,
                                 SA_VideoControl, taglist,
                                 TAG_DONE);

    window = OpenWindowTags(NULL,WA_IDCMP,         (IDCMP_RAWKEY | BUTTONIDCMP | LISTVIEWIDCMP | MXIDCMP),
                                 WA_CustomScreen,  screen,
                                 WA_Flags,         (WFLG_NOCAREREFRESH | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP),
                                 TAG_DONE);

    visualInfo = GetVisualInfoA(screen,NULL);
}


/*****************************************************************************/


/* We don't need this code when we're in ROM, since the last thing we do is
 * reboot
 */
#ifdef TEST_VERSION
static VOID CleanupGlobalData(struct GlobalData *gd)
{
    CloseWindow(window);
    FreeVisualInfo(visualInfo);
    CloseScreen(screen);
    FreeGadgets(gadgets);

    CloseLibrary(IntuitionBase);
    CloseLibrary(GfxBase);
    CloseLibrary(GadToolsBase);
}
#endif


/*****************************************************************************/


static VOID SumExecBase(struct GlobalData *gd)
{
ULONG sum;

    /* rechecksum everything */
    sum = SumKickData();
    SysBase->KickCheckSum = (APTR)sum;

    Permit();
    CacheClearU();
}


/*****************************************************************************/


/* install the config information in a rom tag we can find after reboot */
static VOID InstallKickTag(struct GlobalData *gd)
{
struct BMConfig *config = gd->gd_Config;

    config->bc_RomTag.rt_MatchWord     = RTC_MATCHWORD;
    config->bc_RomTag.rt_MatchTag      = &config->bc_RomTag;
    config->bc_RomTag.rt_EndSkip       = &config->bc_KickMem;
    config->bc_RomTag.rt_Pri           = -50;
    config->bc_RomTag.rt_Name          = MSG_MAIN_TITLE;
/*  config->bc_RomTag.rt_Flags         = 0;
 *  config->bc_RomTag.rt_Version       = 0;
 *  config->bc_RomTag.rt_Type          = NT_UNKNOWN;
 *  config->bc_RomTag.rt_IdString      = NULL;
 *  config->bc_RomTag.rt_Init          = NULL;
 */
    Forbid();

    config->bc_KickMem.ml_Node.ln_Succ = SysBase->KickMemPtr;
    SysBase->KickMemPtr                = &config->bc_KickMem;
    config->bc_KickTag.kt_TagAddr      = &config->bc_RomTag;

    if (config->bc_KickTag.kt_NextArray = SysBase->KickTagPtr)
       config->bc_KickTag.kt_NextArray = (APTR)((ULONG)config->bc_KickTag.kt_NextArray | 0x80000000);

    SysBase->KickTagPtr = (APTR)&config->bc_KickTag;

    SumExecBase(gd);  /* does a Permit() */
}


/*****************************************************************************/


/* remove all traces of the rom tag */
static VOID RemoveKickTag(struct GlobalData *gd)
{
struct MemList  *prevml, *ml;
struct BMConfig *config = gd->gd_Config;

    Forbid();

    UnlinkRomTag(&config->bc_RomTag);

    /* This unlinks our kick mem from the kick mem list */
    prevml = (struct MemList *)&SysBase->KickMemPtr;
    ml     = SysBase->KickMemPtr;
    while (ml != &config->bc_KickMem)
    {
        prevml = ml;
        ml = (struct MemList *)ml->ml_Node.ln_Succ;
    }
    prevml->ml_Node.ln_Succ = ml->ml_Node.ln_Succ;

    SumExecBase(gd);  /* does a Permit() */

    /* who needs this stuff? Noone! So get rid of it */
    FreeMem(config->bc_DevicesMem.me_Addr,config->bc_DevicesMem.me_Length);
    FreeMem(config,sizeof(struct BMConfig));
}


/*****************************************************************************/


static VOID PutText(struct GlobalData *gd, STRPTR text, UWORD x, UWORD y, WORD w, UWORD pen)
{
struct RastPort *rp = &screen->RastPort;

    if (w > 0)
        x += (w-TextLength(rp,text,strlen(text)))/2;

    SetAPen(rp,pen);
    Move(rp,x,y);
    Text(rp,text,strlen(text));
}


/*****************************************************************************/


#ifndef NO_DIAGNOSTIC
/* boxes used to create the boxes around the diagnostic area */
static const struct Rectangle boxes[] =
{
    {61, 18,576,176},

    {63, 19,190,31},
    {191,19,318,31},
    {319,19,446,31},
    {447,19,574,31},
    {63, 32,190,175},
    {191,32,318,175},
    {319,32,446,175},
    {447,32,574,175}
};
#endif


/*****************************************************************************/


static VOID PreparePage(struct GlobalData *gd, struct NewGadget *ng, STRPTR title)
{
    if (gadgets)
    {
        RemoveGList(window,gadgets,-1);
        FreeGadgets(gadgets);
        gd->gd_DisplayTypeGad = NULL;
        gd->gd_LastAdded = NULL;
        SetRast(&screen->RastPort,0);
    }
    PutText(gd,title,0,13,WIDTH,2);

    gadgets = CreateContext(&gd->gd_LastAdded);

    gd->gd_NewGadget  = ng;
    ng->ng_TextAttr   = NULL;
    ng->ng_Flags      = 0;
    ng->ng_VisualInfo = visualInfo;
    ng->ng_Width      = 87;
    ng->ng_Height     = 14;
    ng->ng_TopEdge    = 183;
}


/*****************************************************************************/


static VOID CompletePage(struct GlobalData *gd)
{
    AddGList(window,gadgets,-1,-1,NULL);
    RefreshGList(gadgets,window,NULL,-1);
    GT_RefreshWindow(window,NULL);
}


/*****************************************************************************/


static VOID MainPage(struct GlobalData *gd)
{
struct NewGadget ng;

    PreparePage(gd,&ng,MSG_MAIN_TITLE);
    PutText(gd,MSG_MAIN_TOGGLE_INFO,0,33,WIDTH,1);

    ng.ng_LeftEdge   = 30;
    ng.ng_Width      = 270;
    ng.ng_GadgetText = MSG_DOBOOT;
    ng.ng_GadgetID   = MAIN_DOBOOT_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_LeftEdge   = 640-300;
    ng.ng_GadgetText = MSG_DOBOOTNOSTARTUP;
    ng.ng_GadgetID   = MAIN_DOBOOTNS_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_LeftEdge   = 185;
    ng.ng_TopEdge    = 62;
    ng.ng_Width      = 270;
    ng.ng_GadgetText = MSG_BOOT;
    ng.ng_GadgetID   = MAIN_BOOT_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_TopEdge    = 82;
    ng.ng_GadgetText = MSG_GFX;
    ng.ng_GadgetID   = MAIN_GFX_ID;
    gd->gd_LastAdded = BMCreateGadget(gd,BUTTON_KIND,
                                      GA_Disabled, gd->gd_AvailChips == AC_ORIGINAL,
                                      TAG_DONE);

#ifdef NO_DIAGNOSTIC
    DrawBevelBox(&screen->RastPort,125,52,390,55,
                                    GT_VisualInfo,  visualInfo,
                                    GTBB_Recessed,  TRUE,
                                    GTBB_FrameType, BBFT_RIDGE,
                                    TAG_DONE);
#else
    ng.ng_TopEdge    = 102;
    ng.ng_GadgetText = MSG_DIAG;
    ng.ng_GadgetID   = MAIN_DIAG_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    DrawBevelBox(&screen->RastPort,125,52,390,75,
                                    GT_VisualInfo,  visualInfo,
                                    GTBB_Recessed,  TRUE,
                                    GTBB_FrameType, BBFT_RIDGE,
                                    TAG_DONE);
#endif

    CompletePage(gd);
}


/*****************************************************************************/


static VOID BootPage(struct GlobalData *gd)
{
struct NewGadget  ng;
struct BMNode    *node;
STRPTR            state;

    node = (struct BMNode *)gd->gd_Config->bc_Devices.mlh_Head;
    while (node->bn_Link.ln_Succ)
    {
        node->bn_TempEnabled = node->bn_Enabled;

        state = MSG_BOOT_ENABLED;
        if (!node->bn_TempEnabled)
            state = MSG_BOOT_DISABLED;

        CopyMem(state,node->bn_Info,strlen(state));

        node = (struct BMNode *)node->bn_Link.ln_Succ;
    }

    PreparePage(gd,&ng,MSG_BOOT_TITLE);

    ng.ng_LeftEdge   = 68;
    ng.ng_GadgetText = MSG_BOOT_USE;
    ng.ng_GadgetID   = BOOT_USE_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_LeftEdge   = 471;
    ng.ng_GadgetText = MSG_BOOT_CANCEL;
    ng.ng_GadgetID   = BOOT_CANCEL_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_LeftEdge   = 10;
    ng.ng_TopEdge    = 39;
    ng.ng_Width      = 122;
    ng.ng_Height     = 108;
    ng.ng_GadgetText = MSG_BOOT_DEVICES;
    ng.ng_GadgetID   = BOOT_DEVICES_ID;
    gd->gd_LastAdded = BMCreateGadget(gd,LISTVIEW_KIND,
                                      GTLV_Labels,       &gd->gd_BootDevices,
                                      LAYOUTA_SPACING,   1,
                                      GTLV_ScrollWidth,  18,
                                      GTLV_ShowSelected, NULL,
                                      GTLV_Selected,     0,
                                      TAG_DONE);

    ng.ng_LeftEdge   = 150;
    ng.ng_Width      = 480;
    ng.ng_GadgetText = MSG_BOOT_ACTIVEDEVICES;
    ng.ng_GadgetID   = BOOT_ACTIVEDEVICES_ID;
    gd->gd_DevicesGad = gd->gd_LastAdded = BMCreateGadget(gd,LISTVIEW_KIND,
                                      GTLV_Labels,       &gd->gd_Config->bc_Devices,
                                      LAYOUTA_SPACING,   1,
                                      GTLV_ScrollWidth,  18,
                                      TAG_DONE);

    ng.ng_LeftEdge   = 232;
    ng.ng_TopEdge    = 151;
    ng.ng_Width      = 16;
    ng.ng_GadgetText = MSG_BOOT_DISABLECACHES;
    ng.ng_GadgetID   = BOOT_DISABLECACHES_ID;
    ng.ng_Flags      = PLACETEXT_RIGHT;
    gd->gd_DisableCaches = gd->gd_LastAdded = BMCreateGadget(gd,CHECKBOX_KIND,
                                      GTCB_Checked, gd->gd_Config->bc_DisableCaches,
                                      TAG_DONE);

    CompletePage(gd);
}


/*****************************************************************************/


static VOID GfxPage(struct GlobalData *gd)
{
struct NewGadget ng;

    PreparePage(gd,&ng,MSG_GFX_TITLE);

    ng.ng_LeftEdge   = 68;
    ng.ng_GadgetText = MSG_GFX_USE;
    ng.ng_GadgetID   = GFX_USE_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_LeftEdge   = 471;
    ng.ng_GadgetText = MSG_GFX_CANCEL;
    ng.ng_GadgetID   = GFX_CANCEL_ID;
    gd->gd_LastAdded = BMCreateGadget2(gd,BUTTON_KIND);

    ng.ng_LeftEdge   = 194;
    if (gd->gd_AvailChips <= AC_HR_AGNUS)
        ng.ng_LeftEdge = 294;
    ng.ng_TopEdge    = 75;
    ng.ng_GadgetText = MSG_GFX_DISP_TYPE;
    ng.ng_GadgetID   = GFX_DISPLAYTYPE_ID;
    ng.ng_Flags      = PLACETEXT_RIGHT | NG_HIGHLABEL;
    gd->gd_DisplayTypeGad = gd->gd_LastAdded = BMCreateGadget(gd,MX_KIND,
                                      GTMX_Labels,     &gd->gd_DisplayTypes,
                                      GTMX_TitlePlace, PLACETEXT_ABOVE,
                                      GTMX_Spacing,    5,
                                      GTMX_Active,     gd->gd_Config->bc_Mode,
                                      TAG_DONE);

    if (gd->gd_AvailChips >= AC_ECS)
    {
        ng.ng_LeftEdge   = 354;
        ng.ng_GadgetText = MSG_GFX_CHIP_TYPE;
        ng.ng_GadgetID   = GFX_CHIPTYPE_ID;
        gd->gd_ChipTypeGad = gd->gd_LastAdded = BMCreateGadget(gd,MX_KIND,
                                          GTMX_Labels,     &gd->gd_ChipTypes,
                                          GTMX_TitlePlace, PLACETEXT_ABOVE,
                                          GTMX_Spacing,    5,
                                          GTMX_Active,     gd->gd_Config->bc_Chip,
                                          TAG_DONE);

        DrawBevelBox(&screen->RastPort,125,52,390,75,
                                        GT_VisualInfo,  visualInfo,
                                        GTBB_Recessed,  TRUE,
                                        GTBB_FrameType, BBFT_RIDGE,
                                        TAG_DONE);
    }

    CompletePage(gd);
}


/*****************************************************************************/


#ifndef NO_DIAGNOSTIC
static VOID DiagnosticPage(struct GlobalData *gd)
{
struct NewGadget  ng;
struct ConfigDev *cd;
char              buf[80];
UBYTE             cnt;
UBYTE             board;
ULONG             i;
ULONG             tag;

    PreparePage(gd,&ng,MSG_DIAG_TITLE);

    ng.ng_LeftEdge   = 270;
    ng.ng_GadgetText = MSG_CONTINUE;
    ng.ng_GadgetID   = DIAG_CONTINUE_ID;

    BMCreateGadget2(gd,BUTTON_KIND);

    PutText(gd,MSG_DIAG_HDR,82,28,-1,2);

    cd    = NULL;
    cnt   = 0;
    board = 1;
    while (cd = FindConfigDev(cd,-1,-1))
    {
        sprintf(buf,"%7ld%17ld%15ld          %s",board,
                                                 (ULONG)cd->cd_Rom.er_Manufacturer,
                                                 (ULONG)cd->cd_Rom.er_Product,
                                                 (cd->cd_Flags & CDF_BADMEMORY)?MSG_DEFECTIVE:MSG_WORKING);

        PutText(gd,buf,82,cnt*9+41,-1,1);

        if (!(cd->cd_Rom.er_Type & ERTF_CHAINEDCONFIG))
            board++;

        if (cnt > 15)
            break;

        cnt++;
    }

    tag = GTBB_Recessed;
    for (i = 0; i < 9; i++)
    {
        DrawBevelBox(&screen->RastPort,boxes[i].MinX,boxes[i].MinY,
                                       boxes[i].MaxX-boxes[i].MinX+1,
                                       boxes[i].MaxY-boxes[i].MinY+1,
                                       GT_VisualInfo,  visualInfo,
                                       tag,            TRUE,
                                       TAG_DONE);
        tag = TAG_IGNORE;
    }

    CompletePage(gd);
}
#endif


/*****************************************************************************/


static VOID EventLoop(struct GlobalData *gd, BOOL sysCheck)
{
ULONG                class;
UWORD                icode;
struct IntuiMessage *msg;
struct Custom       *chips = &custom;
struct Gadget       *gad;
BOOLEAN              quit;
struct BMNode       *node;
STRPTR               state;

    quit = FALSE;
    while (!quit)
    {
        WaitPort(window->UserPort);
        if (msg = GT_GetIMsg(window->UserPort))
        {
            class = msg->Class;
            icode = msg->Code;
            gad   = msg->IAddress;
            GT_ReplyIMsg(msg);

            if (class == IDCMP_RAWKEY)
            {
                if (icode < 0x80)  /* key down */
                {
                    gd->gd_NTSC = !gd->gd_NTSC;
                    chips->beamcon0 = (gd->gd_NTSC? 0x0000 : 0x0020);
                }
            }
            else if (class == IDCMP_GADGETUP)
            {
                switch (gad->GadgetID)
                {
                    case MAIN_BOOT_ID    : BootPage(gd);
                                           break;

                    case MAIN_GFX_ID     : GfxPage(gd);
                                           break;

#ifndef NO_DIAGNOSTIC
                    case MAIN_DIAG_ID    : DiagnosticPage(gd);
                                           break;
#endif

                    case MAIN_DOBOOT_ID  : InstallKickTag(gd);
                                           quit = TRUE;
                                           break;

                    case MAIN_DOBOOTNS_ID: gd->gd_Config->bc_StartupSequence = FALSE;
                                           InstallKickTag(gd);
                                           quit = TRUE;
                                           break;

                    case GFX_USE_ID      : GT_GetGadgetAttrs(gd->gd_DisplayTypeGad,window,NULL,GTMX_Active,&gd->gd_Config->bc_Mode,TAG_DONE);
                                           GT_GetGadgetAttrs(gd->gd_ChipTypeGad,window,NULL,GTMX_Active,&gd->gd_Config->bc_Chip,TAG_DONE);

                    case GFX_CANCEL_ID   : MainPage(gd);
                                           break;

                    case DIAG_CONTINUE_ID:
#ifndef NO_DIAGNOSTIC
                                           if (sysCheck)
                                               quit = TRUE;
                                           else
                                               MainPage(gd);
#else
					   quit = TRUE;
#endif
                                           break;

                    case BOOT_USE_ID     : node = (struct BMNode *)gd->gd_Config->bc_Devices.mlh_Head;
                                           while (node->bn_Link.ln_Succ)
                                           {
                                               node->bn_Enabled = node->bn_TempEnabled;
                                               node = (struct BMNode *)node->bn_Link.ln_Succ;
                                           }
                                           gd->gd_Config->bc_BootDevice = (struct BMNode *)((ULONG)gd->gd_BootDev->ln_Name - BNNAMEOFFSET);
                                           Remove(gd->gd_BootDev);
                                           AddHead((struct List *)&gd->gd_BootDevices,gd->gd_BootDev);
                                           GT_GetGadgetAttrs(gd->gd_DisableCaches,window,NULL,GTCB_Checked,&gd->gd_Config->bc_DisableCaches,TAG_DONE);

                    case BOOT_CANCEL_ID  : MainPage(gd);
                                           break;

                    case BOOT_DEVICES_ID : gd->gd_BootDev = FindNum((struct List *)&gd->gd_BootDevices,icode);
                                           break;

                    case BOOT_ACTIVEDEVICES_ID: node = (struct BMNode *)FindNum((struct List *)&gd->gd_Config->bc_Devices,icode);
                                                if (node->bn_TempEnabled)
                                                {
                                                    node->bn_TempEnabled = FALSE;
                                                }
                                                else if (!node->bn_NeverEnable)
                                                {
                                                    node->bn_TempEnabled = TRUE;
                                                }

                                                GT_SetGadgetAttrs(gd->gd_DevicesGad,window,NULL,
                                                                  GTLV_Labels, ~0,
                                                                  TAG_DONE);

                                                state = MSG_BOOT_ENABLED;
                                                if (!node->bn_TempEnabled)
                                                    state = MSG_BOOT_DISABLED;

                                                CopyMem(state,node->bn_Info,strlen(state));

                                                GT_SetGadgetAttrs(gd->gd_DevicesGad,window,NULL,
                                                                  GTLV_Labels, &gd->gd_Config->bc_Devices,
                                                                  TAG_DONE);
                                                break;
                }
            }
        }
    }

#ifdef TEST_VERSION
    CleanupGlobalData(gd);
#endif
}


/*****************************************************************************/


#ifndef NO_DIAGNOSTIC
ULONG SysCheckInit(VOID)
{
struct GlobalData  global;
struct GlobalData *gd;
struct Resident   *res;

    memset(&global,0,sizeof(global));

    gd            = &global;
    SysBase       = (*((struct ExecBase **) 4));
    ExpansionBase = TaggedOpenLibrary(OLTAG_EXPANSION);

    if (ExpansionBase->Flags & EBF_BADMEM)
    {
        /* we don't want to run SysCheck if there is a ROM tag */
        if (res = FindResident(MSG_MAIN_TITLE))
        {
            gd->gd_Config = (struct BMConfig *)((ULONG)res - ROMTAGOFFSET);

            if (gd->gd_Config->bc_SysCheckNode)
            {
                /* remove this so we don't find it again */
                RemoveKickTag(gd);
            }
        }
        else
        {
            InitGlobalData(gd);
            SetRGB4(&screen->ViewPort,0,15,4,4);
            DiagnosticPage(gd);
            EventLoop(gd,TRUE);
            gd->gd_Config->bc_SysCheckNode = TRUE;
            InstallKickTag(gd);
            ColdReboot();
        }
    }

    CloseLibrary(ExpansionBase);

    return(0);
}
#endif


/*****************************************************************************/


ULONG BootMenuInit(VOID)
{
struct GlobalData         global;
struct GlobalData        *gd;
struct BootNode          *bn, *next;
struct DeviceNode        *dn;
struct BMNode            *bnode;
struct Node              *node;
struct FileSysStartupMsg *fssm;
struct DosEnvec          *env;
char                      device[20], pri[30], dosType[5];
UWORD                     len;
UBYTE                     value;
UBYTE                    *ptr;
UWORD                     i;
ULONG                     devices;
ULONG                     bootDevices;
struct BMNode            *devNodes;
struct Node              *bootDevNodes;
BOOLEAN                   mbuttons;
struct Resident          *res;
UWORD                     reg;
struct BMConfig          *config;

    memset(&global,0,sizeof(global));

    gd            = &global;
    SysBase       = (*((struct ExecBase **) 4));

#ifdef CDGS
    if (!ReadGayle())   /* no Gayle chip, so we don't have a computer module */
        return(0);
#endif

    GfxBase       = TaggedOpenLibrary(OLTAG_GRAPHICS);
    ExpansionBase = TaggedOpenLibrary(OLTAG_EXPANSION);
    UtilityBase   = TaggedOpenLibrary(OLTAG_UTILITY);
    mbuttons      = TRUE;

    /* check left mouse button */
    if ((1<<6) & *((char *)0xBFE001))
    {
        mbuttons = FALSE;
    }

    /* set up port to read right mouse button */
    *((UWORD *)0xDFF034) = 0xffff;

    WaitTOF();
    WaitTOF();

    /* check right mouse button */
    reg = *((UWORD *)0xDFF016);
    if ((1<<10) & reg)
    {
        mbuttons = FALSE;
    }

    /* if we can find this rom tag, then we've been here before! */
    if (res = FindResident(MSG_MAIN_TITLE))
    {
        gd->gd_Config = config = (struct BMConfig *)((ULONG)res - ROMTAGOFFSET);

        if (config->bc_Chip == BMCHIP_ECS)
        {
            SetChipRev(SETCHIPREV_ECS);
            GfxBase->WantChips = SETCHIPREV_ECS;
        }
        else if (config->bc_Chip == BMCHIP_A)
        {
            SetChipRev(SETCHIPREV_A);
            GfxBase->WantChips = SETCHIPREV_A;
        }

        SetDefaultMonitor(config->bc_Mode == BMMODE_NTSC ? (NTSC_MONITOR_ID >> 16) : (PAL_MONITOR_ID >> 16));

        if (!config->bc_StartupSequence)
            ExpansionBase->Flags |= EBF_DOSFLAG;

        if (config->bc_DisableCaches)
            CacheControl(0,CACRF_EnableI | CACRF_EnableD | CACRF_CopyBack);

        /* enable/disable partitions */
        bn = (struct BootNode *)ExpansionBase->MountList.lh_Head;
        while (next = (struct BootNode *)bn->bn_Node.ln_Succ)
        {
            if (dn = (struct DeviceNode *)bn->bn_DeviceNode)
            {
                sprintf(device,"%b",dn->dn_Name);

                bnode = (struct BMNode *)config->bc_Devices.mlh_Head;
                while (bnode->bn_Link.ln_Succ)
                {
                    if (Stricmp(device,bnode->bn_Name) == 0)
                    {
                        if (bnode->bn_Enabled)
                        {
                            dn->dn_Handler &= (~0x80000000);
                        }
                        else
                        {
                            dn->dn_Handler |= 0x80000000;
                        }

                        /* do the magic needed for the boot partition */
                        if (bnode == config->bc_BootDevice)
                        {
                            Remove((struct Node *)bn);
                            bn->bn_Node.ln_Pri = 127;
                            AddHead(&ExpansionBase->MountList,(struct Node *)bn);
                        }
                        break;
                    }
                    bnode = (struct BMNode *)bnode->bn_Link.ln_Succ;
                }
            }
            bn = next;
        }

        /* remove this so we don't find it again */
        RemoveKickTag(gd);
    }
    else if (mbuttons)   /* if both mouse buttons are down, put up the GUI */
    {
        InitGlobalData(gd);

        devices     = 0;
        bootDevices = 0;
        config      = gd->gd_Config;

        /* count all devices, and also specifically boot devices */
        bn = (struct BootNode *)ExpansionBase->MountList.lh_Head;
        while (bn->bn_Node.ln_Succ)
        {
            if (dn = (struct DeviceNode *)bn->bn_DeviceNode)
            {
                if (bn->bn_Node.ln_Type == NT_BOOTNODE)
                    bootDevices++;
                devices++;
            }
            bn = (struct BootNode *)bn->bn_Node.ln_Succ;
        }

        devNodes     = AllocMem(sizeof(struct BMNode)*devices+8,MEMF_CHIP|MEMF_REVERSE);
        bootDevNodes = AllocMem(sizeof(struct Node)*bootDevices+8,MEMF_CHIP|MEMF_REVERSE);

        config->bc_KickMem.ml_NumEntries = 2;
        config->bc_DevicesMem.me_Addr    = devNodes;
        config->bc_DevicesMem.me_Length  = sizeof(struct BMNode)*devices+8;

        /* skip memory that will be trashed by mem header upon reboot */
        devNodes     = (struct BMNode *)((ULONG)devNodes + 8);
        bootDevNodes = (struct Node *)((ULONG)bootDevNodes + 8);

        devices     = 0;
        bootDevices = 0;

        /* gather all the info on the available devices */
        bn = (struct BootNode *)ExpansionBase->MountList.lh_Head;
        while (bn->bn_Node.ln_Succ)
        {
            if (dn = (struct DeviceNode *)bn->bn_DeviceNode)
            {
                bnode = &devNodes[devices++];

                if (bn->bn_Node.ln_Type == NT_BOOTNODE)
                {
                    node = &bootDevNodes[bootDevices++];
                    node->ln_Name = bnode->bn_Name;
                    AddTail((struct List *)&gd->gd_BootDevices,node);
                    sprintf(pri,"priority %ld",(signed char)bn->bn_Node.ln_Pri);
                }
                else
                {
                    strcpy(pri, "not bootable");
                }

                bnode->bn_Enabled     = (dn->dn_Handler & 0x80000000)?FALSE:TRUE;
                bnode->bn_NeverEnable = !bnode->bn_Enabled;

                fssm = (struct FileSysStartupMsg *)((ULONG)dn->dn_Startup * 4);
                env  = (struct DosEnvec *)((ULONG)fssm->fssm_Environ * 4);
                bnode->bn_Link.ln_Name = bnode->bn_Info;

                i   = 0;
                ptr = (UBYTE *)&env->de_DosType;
                while (i < 4)
                {
                    value = *ptr++;
                    if ((value >= 0x00) && (value <= 0x09))
                        value += '0';
                    else if ((value >= 0x0a) && (value <= 0x1f))
                        value = value - 0x0a + 'a';
                    else if ((value >= 0x80) && (value <= 0x9f))
                        value = value - 0x80 + '@';

                    dosType[i++] = value;
                }

                if (Strnicmp(dosType,"DOS",3) == 0)
                    strcpy(dosType,"ADOS");

                sprintf(device,"%b",fssm->fssm_Device);
                len = strlen(device);
                if ((len > 7) && (Stricmp(&device[len-7],".device") == 0))
                    device[len-7] = 0;

                sprintf(bnode->bn_Name,"%b",dn->dn_Name);
                sprintf(bnode->bn_Info,"         - %-11.11b %-13.13s %4.4s  %s-%ld",dn->dn_Name,pri,dosType,device,fssm->fssm_Unit);
                AddTail((struct List *)&config->bc_Devices,(struct Node *)bnode);
            }
            bn = (struct BootNode *)bn->bn_Node.ln_Succ;
        }
        gd->gd_BootDev = (struct Node *)gd->gd_BootDevices.mlh_Head;
        config->bc_BootDevice = (struct BMNode *)((ULONG)gd->gd_BootDev->ln_Name - BNNAMEOFFSET);

        MainPage(gd);
        EventLoop(gd,FALSE);
        ColdReboot();
    }
/*
    else if (!((1<<8) & reg))     /* check middle button */
    {
        ExpansionBase->Flags |= EBF_DOSFLAG;
    }
*/

    CloseLibrary(GfxBase);
    CloseLibrary(ExpansionBase);
    CloseLibrary(UtilityBase);

    return(0);
}
