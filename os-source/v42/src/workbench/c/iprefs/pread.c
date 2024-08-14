
/* includes */
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuitionbase.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/monitor.h>
#include <hardware/custom.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <prefs/prefhdr.h>
#include <prefs/font.h>
#include <prefs/icontrol.h>
#include <prefs/printertxt.h>
#include <prefs/printergfx.h>
#include <prefs/serial.h>
#include <prefs/input.h>
#include <prefs/overscan.h>
#include <prefs/screenmode.h>
#include <prefs/sound.h>
#include <prefs/wbpattern.h>
#include <prefs/pointer.h>
#include <prefs/palette.h>
#include <prefs/locale.h>
#include <internal/iprefs.h>
#include <internal/wbinternal.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/locale_protos.h>
#include <clib/utility_protos.h>
#include <clib/keymap_protos.h>
#include <clib/datatypes_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/keymap_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/* application includes */
#include "iprefs.h"
#include "texttable.h"
#include "sound.h"
#include "monitors.h"
#include "resetwb.h"
#include "backdrop.h"


/*****************************************************************************/


ULONG __stdargs DoMethodA (Object *obj, Msg message);

#define MONITOR_PART(id)	((id) & MONITOR_ID_MASK)


/*****************************************************************************/


extern struct ExecBase      *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase       *GfxBase;
extern struct Library       *DOSBase;
extern struct Library       *IFFParseBase;
extern struct Library       *LocaleBase;
extern struct Library	    *UtilityBase;
extern struct Library       *WorkbenchBase;
extern struct Library       *DataTypesBase;

extern struct SoundPrefs     sp;
extern BOOL                  doResetWB;

struct ScreenModePrefs oldSMP;
struct FontPrefs       oldFP;
struct OverscanPrefs   oldOP;
struct PalettePrefs    oldPP;
STRPTR                 oldPatterns[3];

struct WorkbenchFont  *oldIconText;
struct WorkbenchFont  *oldSystemText;

/* all set to FALSE by default */
BOOL gotSMP;
BOOL gotFP;
BOOL gotOP;
BOOL gotPP;


/*****************************************************************************/


VOID UserProblems(AppStringsID string, STRPTR extra)
{
struct EasyStruct ez;

    OpenCat();

    ez.es_StructSize    = sizeof(struct EasyStruct);
    ez.es_Flags         = 0;
    ez.es_Title         = GetStr(MSG_IP_REQTITLE);
    ez.es_TextFormat    = GetStr(string);
    ez.es_GadgetFormat  = GetStr(MSG_IP_CONTINUE_GADGET);

    EasyRequestArgs(NULL,&ez,0,&extra);

    CloseCat();
}


/*****************************************************************************/


static struct Node *FindNameNC(struct List *list, STRPTR name)
{
struct Node *node;
WORD         result;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        result = Stricmp(name,node->ln_Name);
        if (result == 0)
            return(node);

	node = node->ln_Succ;
    }

    return(NULL);
}


/*****************************************************************************/


#define MIN_DENISE mspc->DeniseMinDisplayColumn
#define TOTCLKS mspc->total_colorclocks
#define MINROW  mspc->min_row
#define HBSTRT 1
#define HBSTOP ((MIN_DENISE + 1) >> 1)
#define HSSTRT (((HBSTOP - HBSTRT) / 3) + HBSTRT)
#define HSSTOP ((((HBSTOP - HBSTRT) / 3) << 1) + HBSTRT)
#define VBSTRT 0
#define VBSTOP (MINROW * TOTCLKS)
#define VSSTRT ((MINROW / 3) * TOTCLKS)
#define VSSTOP (((MINROW / 3) << 1) * TOTCLKS)


/*****************************************************************************/


static BOOL ReadOverscanPrefs(struct IFFHandle *iff)
{
struct OverscanPrefs         op;
struct IOverscanPrefs        iop;
BOOL                         result = FALSE;
ULONG                        wbModeID;
struct Screen               *wb;
struct AnalogSignalInterval *hsync, *vsync;
struct Custom               *custom = (struct Custom *)0xdff000;
struct MonitorInfo           monInfo;
struct MonitorSpec          *mspc;

    if (ReadChunkBytes(iff,&op,sizeof op) == sizeof op)
    {
        result = TRUE;

        /* funky origin's origin feature */
        if (op.os_Magic == OSCAN_MAGIC)
        {
            if (op.os_HStart && op.os_HStop && op.os_VStart && op.os_VStop)
            {
                if (GetDisplayInfoData(NULL,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,op.os_DisplayID))
                {
                    mspc = monInfo.Mspc;

                    if (mspc && mspc->ms_Special)
                    {
                        hsync = &mspc->ms_Special->hsync;
                        hsync->asi_Start = op.os_HStart;
                        hsync->asi_Stop  = op.os_HStop;

                        vsync = &mspc->ms_Special->vsync;
                        vsync->asi_Start = op.os_VStart;
                        vsync->asi_Stop  = op.os_VStop;

                        ObtainSemaphore(GfxBase->ActiViewCprSemaphore);

                        if (mspc == GfxBase->current_monitor)
                        {
                            custom->hsstrt = op.os_HStart;
                            custom->hsstop = op.os_HStop;
                            custom->vsstrt = op.os_VStart / TOTCLKS;
                            custom->vsstop = op.os_VStop / TOTCLKS;
                        }

                        ReleaseSemaphore(GfxBase->ActiViewCprSemaphore);
                    }
                }
            }
        }

        iop.ios_DisplayID = op.os_DisplayID;
        iop.ios_ViewPos   = op.os_ViewPos;
        iop.ios_Text      = op.os_Text;
        iop.ios_Standard  = op.os_Standard;
        SetIPrefs(&iop,(LONG)sizeof iop, (LONG)IP_OVERSCAN);

        wbModeID = INVALID_ID;
        if (wb = LockWB())
        {
            wbModeID = GetVPModeID(&wb->ViewPort);
            UnlockPubScreen(NULL,wb);
        }

        if (MONITOR_PART(wbModeID) == MONITOR_PART(DEFAULT_MONITOR_ID))
            wbModeID = (GfxBase->monitor_id << 16) | EXTENDED_MODE;

        if (MONITOR_PART(op.os_DisplayID) == MONITOR_PART(wbModeID))
        {
            if (gotOP && (MONITOR_PART(op.os_DisplayID) == MONITOR_PART(oldOP.os_DisplayID)))
            {
                if ((op.os_Text.x != oldOP.os_Text.x) || (op.os_Text.y != oldOP.os_Text.y))
                {
                    doResetWB = TRUE;
                }
            }
            else
            {
                doResetWB = TRUE;
            }

            oldOP = op;
            gotOP = TRUE;
        }
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadIControlPrefs(struct IFFHandle *iff)
{
struct IControlPrefs icp;
BOOL                 result = FALSE;

    if (ReadChunkBytes(iff,&icp,sizeof icp) == sizeof icp)
    {
        result = TRUE;
	SetIPrefs(&icp.ic_TimeOut, ((LONG)sizeof icp) - 16, (LONG)IP_ICONTROL);
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadFontPrefs(struct IFFHandle *iff)
{
struct IFontPrefs     ifp;
struct TextFont	     *font;
struct FontPrefs      fontPrefs;
ULONG                 fontType;
BOOL		      result;
struct WorkbenchFont *wf;
struct Library       *DiskfontBase;

    result = TRUE;
    if (DiskfontBase = OpenLibrary("diskfont.library",38))
    {
        result = FALSE;
        if (ReadChunkBytes(iff,&fontPrefs,sizeof fontPrefs) == sizeof fontPrefs)
        {
            result = TRUE;

            fontType = fontPrefs.fp_Type;

            if (fontType != FP_SCREENFONT)
            {
                if (WorkbenchBase)
                {
                    if (wf = AllocVec(sizeof(struct WorkbenchFont)+strlen(fontPrefs.fp_Name)+1+sizeof(struct TextAttr),MEMF_CLEAR|MEMF_PUBLIC))
                    {
                        wf->wf_FrontPen      = fontPrefs.fp_FrontPen;
                        wf->wf_BackPen       = fontPrefs.fp_BackPen;
                        wf->wf_DrawMode      = fontPrefs.fp_DrawMode;
                        wf->wf_Attr          = (struct TextAttr *)((ULONG)wf + sizeof(struct WorkbenchFont));
                        *wf->wf_Attr         = fontPrefs.fp_TextAttr;
                        wf->wf_Attr->ta_Name = (STRPTR)((ULONG)wf->wf_Attr + sizeof(struct TextAttr));
                        strcpy(wf->wf_Attr->ta_Name,fontPrefs.fp_Name);

                        if (wf->wf_Font = OpenDiskFont(wf->wf_Attr))
                        {
                            if (fontType == FP_WBFONT)
                            {
/*  need to handle changes in pen and draw modes
                                if (oldIconText->wf_Font != wf->wf_Font)
                                {
*/
                                    oldIconText = wf;
                                    wf = (struct WorkbenchFont *)WBConfig(WORKBENCH_ICON_FONT,(ULONG)wf);
/*
                                }
*/
                            }
                            else
                            {
                                if (oldSystemText->wf_Font != wf->wf_Font)
                                {
                                    oldSystemText = wf;
                                    wf = (struct WorkbenchFont *)WBConfig(WORKBENCH_TEXT_FONT,(ULONG)wf);
                                }
                            }

                            if (wf)
                                CloseFont(wf->wf_Font);
                        }
                        else
                        {
                            UserProblems(MSG_IP_ERROR_NO_FONT,wf->wf_Attr->ta_Name);
                        }
                        FreeVec(wf);
                    }
                }
            }

            if (fontType != FP_WBFONT)
            {
                ifp.ifp_TABuff.tab_TAttr         = fontPrefs.fp_TextAttr;
                ifp.ifp_SysFontType              = IPF_SCREENFONT;
                ifp.ifp_TABuff.tab_TAttr.ta_Name = ifp.ifp_TABuff.tab_NameBuff;

                if (fontType == FP_SYSFONT)
                    ifp.ifp_SysFontType = IPF_SYSFONT;

                strncpy(ifp.ifp_TABuff.tab_NameBuff,fontPrefs.fp_Name,MAXFONTNAME);
                ifp.ifp_TABuff.tab_NameBuff[MAXFONTNAME-1] = '\0';

                if (font = OpenDiskFont(&ifp.ifp_TABuff.tab_TAttr))
                {
                    SetIPrefs(&ifp, (LONG) sizeof ifp, (LONG)IP_FONT);

                    CloseFont(font);    /* don't need to hold it open   */

                    if (fontType == FP_SCREENFONT)
                    {
                        if (gotFP)
                        {
                            if ((fontPrefs.fp_TextAttr.ta_YSize != oldFP.fp_TextAttr.ta_YSize)
                             || (fontPrefs.fp_TextAttr.ta_Flags != oldFP.fp_TextAttr.ta_Flags)
                             || (fontPrefs.fp_TextAttr.ta_Style != oldFP.fp_TextAttr.ta_Style)
                             || (strcmp(fontPrefs.fp_Name,oldFP.fp_Name) != 0))
                            {
                                doResetWB = TRUE;
                            }
                        }
                        else
                        {
                            doResetWB = TRUE;
                        }

                        oldFP = fontPrefs;
                        gotFP = TRUE;
                    }
                }
                else
                {
                    UserProblems(MSG_IP_ERROR_NO_FONT,ifp.ifp_TABuff.tab_TAttr.ta_Name);
                }
            }
        }
        CloseLibrary(DiskfontBase);
    }
    else
    {
        UserProblems(MSG_IP_ERROR_NO_DISKFONT_LIB,NULL);
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadScreenModePrefs(struct IFFHandle *iff)
{
struct ScreenModePrefs  smp;
struct IScreenModePrefs ismp;
BOOL		        result = FALSE;
struct DimensionInfo    dimInfo;

    if (ReadChunkBytes(iff,&smp,sizeof smp) == sizeof smp)
    {
        result = TRUE;

        if (GetDisplayInfoData(NULL,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,smp.smp_DisplayID))
        {
            if (smp.smp_Width != -1)
            {
                if (smp.smp_Width < dimInfo.MinRasterWidth)
                    smp.smp_Width = dimInfo.MinRasterWidth;

                if (smp.smp_Width > dimInfo.MaxRasterWidth)
                    smp.smp_Width = dimInfo.MaxRasterWidth;
            }

            if (smp.smp_Height != -1)
            {
                if (smp.smp_Height < dimInfo.MinRasterHeight)
                    smp.smp_Height = dimInfo.MinRasterHeight;

                if (smp.smp_Height > dimInfo.MaxRasterHeight)
                    smp.smp_Height = dimInfo.MaxRasterHeight;
            }

            if (smp.smp_Depth > dimInfo.MaxDepth)
                smp.smp_Depth = dimInfo.MaxDepth;
        }

        ismp.ism_DisplayID = smp.smp_DisplayID;
        ismp.ism_Width     = smp.smp_Width;
        ismp.ism_Height    = smp.smp_Height;
        ismp.ism_Depth     = smp.smp_Depth;
    	ismp.ism_Control   = smp.smp_Control | WBINTERLEAVED;

	if (ismp.ism_Width != -1)
	    ismp.ism_Width = max(640,ismp.ism_Width);

	if (ismp.ism_Height != -1)
	    ismp.ism_Height = max(200,ismp.ism_Height);

	SetIPrefs(&ismp, (LONG)sizeof ismp, (LONG)IP_SCREENMODE);

        if (gotSMP)
        {
            if ((smp.smp_DisplayID != oldSMP.smp_DisplayID)
             || (smp.smp_Width     != oldSMP.smp_Width)
             || (smp.smp_Height    != oldSMP.smp_Height)
             || (smp.smp_Depth     != oldSMP.smp_Depth)
             || (smp.smp_Control   != oldSMP.smp_Control))
            {
                doResetWB = TRUE;
            }
        }
        else
        {
            doResetWB = TRUE;
        }

	oldSMP = smp;
	gotSMP = TRUE;
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadPrinterPrefs(struct IFFHandle *iff)
{
WORD                   i;
struct PrinterTxtPrefs ptp;
BOOL                   result = FALSE;
struct Preferences     oprefs;

    if (ReadChunkBytes(iff,&ptp,sizeof ptp) == sizeof ptp)
    {
        result = TRUE;

	GetPrefs(&oprefs, sizeof(struct Preferences));

	for (i = 0; i < DRIVERNAMESIZE; i++)
	    oprefs.PrinterFilename[i] = ptp.pt_Driver[i];

	oprefs.PrinterPort      = ptp.pt_Port;
	oprefs.PaperType        = (ptp.pt_PaperType) << 7;
	oprefs.PaperSize        = (ptp.pt_PaperSize) << 4;
	oprefs.PaperLength      = ptp.pt_PaperLength;
	oprefs.PrintPitch       = (ptp.pt_Pitch) << 10;
	oprefs.PrintSpacing     = (UWORD)((ptp.pt_Spacing == 1) ? 0x0200 : 0);
	oprefs.PrintLeftMargin  = ptp.pt_LeftMargin;
	oprefs.PrintRightMargin = ptp.pt_RightMargin;
	oprefs.PrintQuality     = (ptp.pt_Quality) << 8;
	oprefs.PrinterType      = 0;

	SetPrefs(&oprefs, sizeof(struct Preferences), FALSE);
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadPrinterUnitPrefs(struct IFFHandle *iff)
{
struct PrinterUnitPrefs pup;
BOOL                    result = FALSE;
UWORD                   i;
struct Preferences      oprefs;

    if (ReadChunkBytes(iff,&pup,sizeof pup) == sizeof pup)
    {
        result = TRUE;

	GetPrefs(&oprefs, sizeof(struct Preferences));

	for (i = 0; i < DEVNAME_SIZE; i++)
	    oprefs.PrtDevName[i] = pup.pu_DeviceName[i];

        oprefs.DefaultPrtUnit = pup.pu_UnitNum;

	SetPrefs(&oprefs, sizeof(struct Preferences), FALSE);
    }

    return (result);
}


/*****************************************************************************/


/* local structure that looks like a IPenPrefs, but with more fields at
 * the end
 */
struct IPenPrefs2
{
    UWORD ipn_NumPens;
    UWORD ipn_PenType;
    ULONG ipn_Reserved;
    UWORD ipn_Pens[33];
};


static BOOL ReadPalettePrefs(struct IFFHandle *iff)
{
struct PalettePrefs pp;
BOOL                result = FALSE;
BOOL                same;
struct IPenPrefs2   ipp;
UWORD               num4Pens;
UWORD               num8Pens;
UWORD               numColors;
UWORD               i;
struct ColorSpec    cspec[33];

    if (ReadChunkBytes(iff,&pp,sizeof pp) == sizeof pp)
    {
        result = TRUE;

        for (numColors = 0; numColors < 32; numColors++)
        {
            cspec[numColors] = pp.pap_Colors[numColors];
            if (cspec[numColors].ColorIndex == -1)
                break;
        }
        cspec[numColors].ColorIndex = -1;

        for (num4Pens = 0; num4Pens < 32; num4Pens++)
        {
            ipp.ipn_Pens[num4Pens] = pp.pap_4ColorPens[num4Pens];
            if (ipp.ipn_Pens[num4Pens] == ~0)
                break;
        }
        ipp.ipn_Pens[num4Pens] = (UWORD)~0;
        ipp.ipn_PenType        = PENTYPE_FOURCOLORPENS;
        ipp.ipn_NumPens        = num4Pens+1;
        ipp.ipn_Reserved       = 0;
        SetIPrefs(&ipp,sizeof ipp,IP_PENS);

        for (num8Pens = 0; num8Pens < 32; num8Pens++)
        {
            ipp.ipn_Pens[num8Pens] = pp.pap_8ColorPens[num8Pens];
            if (ipp.ipn_Pens[num8Pens] == ~0)
                break;
        }
        ipp.ipn_Pens[num8Pens] = (UWORD)~0;
        ipp.ipn_PenType        = PENTYPE_EIGHTCOLORPENS;
        ipp.ipn_NumPens        = num8Pens+1;
        SetIPrefs(&ipp,sizeof ipp,IP_PENS);

        if (gotPP)
        {
            same = TRUE;

            for (i=0; i < num4Pens; i++)
                if (pp.pap_4ColorPens[i] != oldPP.pap_4ColorPens[i])
                    same = FALSE;

            for (i=0; i < num8Pens; i++)
                if (pp.pap_8ColorPens[i] != oldPP.pap_8ColorPens[i])
                    same = FALSE;

            if (!same)
                doResetWB = TRUE;
        }
        else
        {
            doResetWB = TRUE;
        }

	oldPP = pp;
	gotPP = TRUE;

        SetIPrefs(cspec,(numColors+1)*sizeof(struct ColorSpec),IP_NEWPALETTE);
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadPrinterGfxPrefs(struct IFFHandle *iff)
{
struct PrinterGfxPrefs pgp;
BOOL                   result = FALSE;
struct Preferences     oprefs;

    if (ReadChunkBytes(iff,&pgp,sizeof pgp) == sizeof pgp)
    {
        result = TRUE;

	GetPrefs(&oprefs,sizeof(struct Preferences));

	oprefs.PrintAspect    = pgp.pg_Aspect;
        oprefs.PrintShade     = pgp.pg_Shade;
	oprefs.PrintFlags    &= ~(UWORD)0x1000; /* old GREY_SCALE2 */
	oprefs.PrintImage     = pgp.pg_Image;
	oprefs.PrintThreshold = pgp.pg_Threshold;
	oprefs.PrintFlags    &= ~(UWORD)0x0007;
	oprefs.PrintFlags    |= pgp.pg_ColorCorrect;
	oprefs.PrintDensity   = pgp.pg_PrintDensity;
	oprefs.PrintMaxWidth  = pgp.pg_PrintMaxWidth;
	oprefs.PrintMaxHeight = pgp.pg_PrintMaxHeight;
	oprefs.PrintXOffset   = pgp.pg_PrintXOffset;

	oprefs.PrintFlags &= ~(UWORD)0x00f0;
	if (pgp.pg_Dimensions != IGNORE_DIMENSIONS)
	    oprefs.PrintFlags |= 1 << (pgp.pg_Dimensions + 3);

	oprefs.PrintFlags &= ~(UWORD)0x0600;
	if (pgp.pg_Dithering != 0x0000)		/* new ORDERED_DITHERING */
	    oprefs.PrintFlags |= 1 << (pgp.pg_Dithering + 8);

	oprefs.PrintFlags &= ~(UWORD)0x0908;
	if (pgp.pg_GraphicFlags & 0x0001)	/* new CENTER_IMAGE */
	    oprefs.PrintFlags |= 0x0008;	/* old CENTER_IMAGE */
	if (pgp.pg_GraphicFlags & 0x0002)	/* new INTEGER_SCALING */
	    oprefs.PrintFlags |= 0x0100;	/* old INTEGER_SCALING */
	if (pgp.pg_GraphicFlags & 0x0004)	/* new ANTI_ALIAS */
	    oprefs.PrintFlags |= 0x0800;	/* old ANTI_ALIAS */

	if (pgp.pg_Shade == 0x03)		/* new GREY_SCALE2 */
	{
	    oprefs.PrintShade = 0x01;		/* old SHADE_GREYSCALE */
	    oprefs.PrintFlags |= 0x1000;	/* old GREY_SCALE2 */
	}

	SetPrefs(&oprefs, sizeof(struct Preferences), FALSE);
    }

    return (result);
}


/*****************************************************************************/


static WORD BaudToProp(WORD level)
{
WORD        i;
static LONG bauds[8] = { 110, 300, 1200, 2400, 4800, 9600, 19200, 31250 };

    for (i = 0; i < 8; i++)
    {
	if (level == bauds[i])
	    return (i);
    }
    return (0);
}


/*****************************************************************************/


static WORD BufferToProp(LONG level)
{
       WORD i;
static LONG bufs[] = { 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };

    for (i = 0; i < 8; i++)
    {
	if (level == bufs[i])
	    return (i);
    }

    return (0);
}


/*****************************************************************************/


static BOOL ReadSerialPrefs(struct IFFHandle *iff)
{
struct SerialPrefs srp;
BOOL               result = FALSE;
struct Preferences oprefs;

    if (ReadChunkBytes(iff,&srp,sizeof srp) == sizeof srp)
    {
        result = TRUE;

	GetPrefs(&oprefs,sizeof(struct Preferences));

        oprefs.DefaultSerUnit = srp.sp_Unit0Map;
        if (oprefs.DefaultSerUnit == 0)
            oprefs.DefaultSerUnit = 1;

        oprefs.DefaultSerUnit--;
	oprefs.BaudRate       = BaudToProp((WORD)srp.sp_BaudRate);
	oprefs.SerStopBuf     = (UBYTE)BufferToProp((WORD)srp.sp_InputBuffer) | ((srp.sp_StopBits - 1) << 4);
	oprefs.SerParShk      = srp.sp_InputHandshake | (srp.sp_Parity << 4);
	oprefs.SerRWBits      = (8 - srp.sp_BitsPerChar) | ((8 - srp.sp_BitsPerChar) << 4);

	SetPrefs(&oprefs, sizeof(struct Preferences), FALSE);
    }

    return (result);
}


/*****************************************************************************/


static VOID SetMap(STRPTR name)
{
char                   fileName[100];
BPTR                   segment;
struct KeyMapResource *kr;
struct KeyMapNode     *kn;
struct Library        *KeymapBase;

    if (name[0])
    {
        if (kr = (struct KeyMapResource *)OpenResource("keymap.resource"))
        {
            strcpy(fileName,"KEYMAPS:");
            AddPart(fileName,name,sizeof(fileName));

            Forbid();
            if (!(kn = (struct KeyMapNode *)FindNameNC(&kr->kr_List,name)))
            {
                if (segment = LoadSeg(fileName))
                {
                    if (!(kn = (struct KeyMapNode *)FindNameNC(&kr->kr_List,name)))
                    {
                        kn = (struct KeyMapNode *)((segment << 2) + sizeof(BPTR));
                        AddHead(&(kr->kr_List),kn);
                    }
                    else
                    {
                        UnLoadSeg(segment);
                    }
                }
                else
                {
                    UserProblems(MSG_IP_ERROR_NO_KEYMAP,fileName);
                }
            }
            Permit();

            if (kn)
            {
                if (KeymapBase = OpenLibrary("keymap.library",37))
                {
                    SetKeyMapDefault(&kn->kn_KeyMap);
                    CloseLibrary(KeymapBase);
                }
            }
        }
    }
}


/*****************************************************************************/


static BOOL ReadInputPrefs(struct IFFHandle *iff)
{
struct InputPrefs  ipp;
BOOL               result = FALSE;
struct Preferences oprefs;

    if (ReadChunkBytes(iff,&ipp,sizeof ipp) == sizeof ipp)
    {
        result = TRUE;

	GetPrefs(&oprefs,sizeof(struct Preferences));

	oprefs.PointerTicks = ipp.ip_PointerTicks;
	oprefs.DoubleClick  = ipp.ip_DoubleClick;
	oprefs.KeyRptDelay  = ipp.ip_KeyRptDelay;
	oprefs.KeyRptSpeed  = ipp.ip_KeyRptSpeed;

        oprefs.EnableCLI    &= (~MOUSE_ACCEL);
        if (ipp.ip_MouseAccel)
            oprefs.EnableCLI |= MOUSE_ACCEL;

	SetPrefs(&oprefs, sizeof(struct Preferences), FALSE);
	SetMap(ipp.ip_Keymap);
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadLocalePrefs(STRPTR name)
{
struct Locale *locale;
BOOL           result;

    result = TRUE;
    if (LocaleBase)
    {
        result = FALSE;
        if (locale = OpenLocale(name))
        {
            result = TRUE;

            CloseLocale(SetDefaultLocale(locale));

            IWBConfig(WORKBENCH_NEW_LOCALE,NULL);

            NameMonitors();
        }
    }
    else
    {
        UserProblems(MSG_IP_ERROR_NO_LOCALE_LIB,NULL);
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadSoundPrefs(struct IFFHandle *iff)
{
    if (ReadChunkBytes(iff,&sp,sizeof sp) == sizeof sp)
    {
        if (!ChangeAudio())
            UserProblems(MSG_IP_ERROR_NO_SOUND,sp.sop_AudioFileName);

        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


#define SIZEPERPLANE  (16 * sizeof(UWORD))
#define	PAT_REPEAT    4


static BOOL ReadWBPatternPrefs(struct IFFHandle *iff)
{
struct WBPatternPrefs   wbp;
BOOL                    result;
struct BitMap          *tbm;
struct BitMap          *bm;
WORD                    plane;
struct IPatternBitMap  *pbm;
UWORD                   width, height;
UWORD                   i,j;

    result = FALSE;

    if (ReadChunkBytes(iff,&wbp,sizeof wbp) == sizeof wbp)
    {
	if (wbp.wbp_Which <= WBP_SCREEN)
	{
            result = TRUE;

            if (wbp.wbp_Flags & WBPF_PATTERN)  /* reading a pattern */
            {
                FreeVec(oldPatterns[wbp.wbp_Which]);
                oldPatterns[wbp.wbp_Which] = NULL;

                if (wbp.wbp_Depth == 0)
                {
                    BackdropConfig(wbp.wbp_Which,NULL);
                }
                else
                {
                    if (tbm = AllocBitMap(PAT_WIDTH, PAT_HEIGHT, wbp.wbp_Depth, BMF_CLEAR|BMF_STANDARD, NULL))
                    {
                        for (plane = 0; plane < tbm->Depth; plane++)
                        {
                            if (ReadChunkBytes(iff,tbm->Planes[plane], SIZEPERPLANE) != SIZEPERPLANE)
                            {
                                result = FALSE;
                                break;
                            }
                        }

                        if (result)
                        {
                            width  = PAT_WIDTH*PAT_REPEAT;
                            height = PAT_HEIGHT*PAT_REPEAT;

                            if (bm = AllocBitMap(width,height, 3, BMF_CLEAR|BMF_STANDARD, NULL))
                            {
                                for (i = 3; i < 8; i++)
                                    bm->Planes[i] = bm->Planes[2];
                                bm->Depth = 8;

                                for (i = 0; i < PAT_REPEAT; i++)
                                    for (j = 0; j < PAT_REPEAT; j++)
                                        BltBitMap(tbm, 0, 0, bm, i*PAT_WIDTH, j*PAT_HEIGHT, PAT_WIDTH, PAT_HEIGHT, 0xC0, 0xFF, NULL);
                            }
                            else
                            {
                                result = FALSE;
                            }

                            WaitBlit();
                            FreeBitMap(tbm);

                            if (result)
                            {
                                if (pbm = AllocVec(sizeof(struct IPatternBitMap),MEMF_CLEAR))
                                {
                                    pbm->pbm_BitMap  = bm;
                                    pbm->pbm_Width   = width;
                                    pbm->pbm_Height  = height;
                                    pbm->pbm_DoRemap = FALSE;

                                    BackdropConfig(wbp.wbp_Which,pbm);
                                }
                            }
                        }
                    }
                }
            }
            else    /* reading a picture */
            {
                if (pbm = AllocVec(sizeof(struct IPatternBitMap)+wbp.wbp_DataLength+1,MEMF_CLEAR))
                {
                    if (ReadChunkBytes(iff,&pbm->pbm_Name,wbp.wbp_DataLength) == wbp.wbp_DataLength)
                    {
                        if (oldPatterns[wbp.wbp_Which] && !Stricmp(oldPatterns[wbp.wbp_Which],&pbm->pbm_Name))
                        {
                            FreeVec(pbm);
                        }
                        else
                        {
                            FreeVec(oldPatterns[wbp.wbp_Which]);
                            oldPatterns[wbp.wbp_Which] = NULL;

                            pbm->pbm_DoRemap = !(WBPF_NOREMAP & wbp.wbp_Flags);
                            if (pbm->pbm_Name)
                            {
                                if (oldPatterns[wbp.wbp_Which] = AllocVec(strlen(&pbm->pbm_Name)+1,MEMF_ANY))
                                    strcpy(oldPatterns[wbp.wbp_Which],&pbm->pbm_Name);

                                BackdropConfig(wbp.wbp_Which,pbm);
                            }
                            else
                            {
                                BackdropConfig(wbp.wbp_Which,NULL);
                            }
                        }
                    }
                    else
                    {
                        FreeVec(pbm);
                        result = FALSE;
                    }
                }
            }
	}
    }

    return (result);
}


/*****************************************************************************/


static BOOL ReadPointerPrefs(struct IFFHandle *iff)
{
struct PointerPrefs     pp;
struct ColorSpec        cspec[2];
BOOL                    error;
UWORD                   i, numColors;
struct BitMap          *bm;
ULONG                   columns;
UWORD                   plane, row;
struct RGBTable         color;
struct INewPointerPrefs inp;
struct BitMap           *oldbm;

    if (ReadChunkBytes(iff,&pp,sizeof pp) == sizeof pp)
    {
        cspec[1].ColorIndex = -1;

	numColors = (1 << pp.pp_Depth) - 1;
	for (i = 0; i < numColors; i++)
	{
            if (ReadChunkBytes(iff, &color, sizeof(struct RGBTable)) != sizeof(struct RGBTable))
                return(FALSE);

            cspec[0].ColorIndex = i+8;
            cspec[0].Red        = (((UWORD)color.t_Red)   << 8) | (UWORD)color.t_Red;
            cspec[0].Green      = (((UWORD)color.t_Green) << 8) | (UWORD)color.t_Green;
            cspec[0].Blue       = (((UWORD)color.t_Blue)  << 8) | (UWORD)color.t_Blue;
            SetIPrefs(cspec,2 * sizeof(struct ColorSpec),IP_NEWPALETTE);
	}

        if (bm = AllocBitMap(pp.pp_Width,pp.pp_Height,pp.pp_Depth,BMF_CLEAR,NULL))
        {
            error = FALSE;

            columns = ((pp.pp_Width + 15) / 16) * 2;
            for (plane = 0; plane < pp.pp_Depth; plane++)
            {
                for (row = 0; row < pp.pp_Height; row++)
                {
                    if (ReadChunkBytes(iff, (APTR)((ULONG)bm->Planes[plane] + (ULONG)row*bm->BytesPerRow), columns) != columns)
                    {
                        error = TRUE;
                        break;
                    }
                }

                if (error)
                    break;
            }

            if (!error)
            {
                inp.inp_BitMap       = bm;
                inp.inp_HotSpotX     = pp.pp_X;
                inp.inp_HotSpotY     = pp.pp_Y;
                inp.inp_WordWidth    = (pp.pp_Width + 15) / 16;
                inp.inp_XResolution  = pp.pp_Size;
                inp.inp_YResolution  = pp.pp_YSize;
                inp.inp_Type         = pp.pp_Which;
                inp.inp_Flags        = 0;

                oldbm = (struct BitMap *)SetIPrefs(&inp,sizeof(struct INewPointerPrefs),IP_NEWPOINTER);

                if (oldbm == NULL)
                    FreeBitMap(bm);
                else if (oldbm != (struct BitMap *)~0)
                    FreeBitMap(oldbm);

                return((BOOL)(oldbm != NULL));
            }
            else
            {
                FreeBitMap(bm);
            }
        }
    }

    return (FALSE);
}


/*****************************************************************************/


#define IFFPrefChunkCnt 15
static LONG __far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_OSCN,
    ID_PREF, ID_ICTL,
    ID_PREF, ID_FONT,
    ID_PREF, ID_SCRM,
    ID_PREF, ID_PTXT,
    ID_PREF, ID_PGFX,
    ID_PREF, ID_SERL,
    ID_PREF, ID_INPT,
    ID_PREF, ID_SOND,
    ID_PREF, ID_PUNT,
    ID_PREF, ID_PTRN,
    ID_PREF, ID_PALT,
    ID_PREF, ID_PNTR,
    ID_PREF, ID_LCLE
};


VOID ReadPrefsFile(STRPTR name)
{
BPTR                fp;
BOOL                ok;
struct IFFHandle   *iff;
struct ContextNode *cn;
struct PrefHeader   phead;

    if (!(IFFParseBase = OpenLibrary("iffparse.library",39)))
    {
        UserProblems(MSG_IP_ERROR_NO_IFFPARSE_LIB,NULL);
        return;
    }

    if (fp = Open(name,MODE_OLDFILE))
    {
        if (iff = AllocIFF())
        {
            iff->iff_Stream = fp;
            InitIFFasDOS(iff);

            if (!OpenIFF(iff,IFFF_READ))
            {
                if (!ParseIFF(iff,IFFPARSE_STEP))
                {
                    cn = CurrentChunk(iff);
                    if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_PREF)
                    {
                        if (!StopChunks(iff,IFFPrefChunks,IFFPrefChunkCnt))
                        {
                            ok = TRUE;
                            while (ok)
                            {
                                if (ParseIFF(iff,IFFPARSE_SCAN))
                                    break;

                                cn = CurrentChunk(iff);

                                if (cn->cn_Type == ID_PREF)
                                {
                                    WorkbenchBase = OpenLibrary("workbench.library",39);

                                    switch (cn->cn_ID)
                                    {
                                        case ID_PRHD: if (ReadChunkBytes(iff,&phead,sizeof(struct PrefHeader)) != sizeof(struct PrefHeader))
                                                      {
                                                          ok = FALSE;
                                                          break;
                                                      }

                                                      if (phead.ph_Version != 0)
                                                      {
                                                          ok = FALSE;
                                                          break;
                                                      }
                                                      break;

                                        case ID_OSCN: ok = ReadOverscanPrefs(iff);
                                                      break;

                                        case ID_ICTL: ok = ReadIControlPrefs(iff);
                                                      break;

                                        case ID_FONT: ok = ReadFontPrefs(iff);
                                                      break;

                                        case ID_SCRM: ok = ReadScreenModePrefs(iff);
                                                      break;

                                        case ID_PTXT: ok = ReadPrinterPrefs(iff);
                                                      break;

                                        case ID_PUNT: ok = ReadPrinterUnitPrefs(iff);
                                                      break;

                                        case ID_PGFX: ok = ReadPrinterGfxPrefs(iff);
                                                      break;

                                        case ID_SERL: ok = ReadSerialPrefs(iff);
                                                      break;

                                        case ID_INPT: ok = ReadInputPrefs(iff);
                                                      break;

                                        case ID_SOND: ok = ReadSoundPrefs(iff);
                                                      break;

                                        case ID_PTRN: ok = ReadWBPatternPrefs(iff);
                                                      break;

                                        case ID_PALT: ok = ReadPalettePrefs(iff);
                                                      break;

                                        case ID_PNTR: ok = ReadPointerPrefs(iff);
                                                      break;

                                        case ID_LCLE: ok = ReadLocalePrefs(name);
                                                      break;

                                        default     : break;
                                    }

                                    CloseLibrary(WorkbenchBase);
                                }
                            }

                            if (!ok)
                                UserProblems(MSG_IP_ERROR_BAD_PREFS_FILE,name);
                        }
                    }
                }
                CloseIFF(iff);
            }
            FreeIFF(iff);
        }
        Close(fp);
    }
    else
    {
        if (IoErr() != ERROR_OBJECT_NOT_FOUND)
            UserProblems(MSG_IP_ERROR_CANT_OPEN,name);
    }

    CloseLibrary(IFFParseBase);
}
