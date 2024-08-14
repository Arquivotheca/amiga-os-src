
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <prefs/locale.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"


#define SysBase ed->ed_SysBase


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_ICTL,
};


/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,   /* version */
    0,   /* type    */
    0    /* flags   */
};


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    ed->ed_PrefsDefaults.ic_TimeOut     = 50;
    ed->ed_PrefsDefaults.ic_MetaDrag    = IEQUALIFIER_LCOMMAND;
    ed->ed_PrefsDefaults.ic_Flags       = ICF_COERCE_LACE | ICF_STRGAD_FILTER | ICF_MENUSNAP | ICF_MODEPROMOTE;
    ed->ed_PrefsDefaults.ic_WBtoFront   = 'N';
    ed->ed_PrefsDefaults.ic_FrontToBack = 'M';
    ed->ed_PrefsDefaults.ic_ReqTrue     = 'V';		/* CKey: Requester TRUE		*/
    ed->ed_PrefsDefaults.ic_ReqFalse    = 'B';

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    return(ES_NORMAL);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID != ID_ICTL || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct IControlPrefs)) == sizeof(struct IControlPrefs))
        return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (!PushChunk(iff,0,ID_ICTL,sizeof(struct IControlPrefs)))
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct IControlPrefs)) == sizeof(struct IControlPrefs))
            if (!PopChunk(iff))
                return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     420
#define NW_HEIGHT    150
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN, 0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS, 0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP, 0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE, 0},

    {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {
    {BUTTON_KIND,   8,   120, 90,  22, MSG_SAVE_GAD,       EC_SAVE,    0},
    {BUTTON_KIND,   166, 120, 90,  22, MSG_USE_GAD,        EC_USE,     0},
    {BUTTON_KIND,   325, 120, 90,  22, MSG_CANCEL_GAD,     EC_CANCEL,  0},

    {CHECKBOX_KIND, 10, 24,  87,  22, MSG_NOTHING,        EC_SHIFT,   PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 10, 44,  87,  22, MSG_ICTL_CTRL_GAD,  EC_CTRL,    PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 10, 66,  87,  22, MSG_ICTL_ALT_GAD,   EC_ALT,     PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 10, 88,  87,  22, MSG_NOTHING,        EC_AMIGA,   PLACETEXT_RIGHT},

    {CHECKBOX_KIND, 386, 24,  87,  22, MSG_ICTL_MENUSNAP_GAD,        EC_MENUSNAP,       0},
    {CHECKBOX_KIND, 386, 46,  87,  22, MSG_ICTL_GADGETFILTER_GAD,    EC_GADGETFILTER,   0},
    {CHECKBOX_KIND, 386, 66,  87,  22, MSG_ICTL_AVOIDFLICKER_GAD,    EC_AVOIDFLICKER,   0},
    {CHECKBOX_KIND, 386, 88,  87,  22, MSG_ICTL_MODEPROMOTE_GAD,     EC_MODEPROMOTE,    0}
};


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
    DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);

    RenderGadgets(ed);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,EM))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
                                            WA_InnerHeight, NW_HEIGHT,
                                            WA_MinWidth,    NW_MINWIDTH,
                                            WA_MinHeight,   NW_MINHEIGHT,
                                            WA_MaxWidth,    NW_MAXWIDTH,
                                            WA_MaxHeight,   NW_MAXHEIGHT,
                                            WA_IDCMP,       NW_IDCMP,
                                            WA_Flags,       NW_FLAGS,
                                            WA_Zoom,        zoomSize,
                                            WA_AutoAdjust,  TRUE,
                                            WA_PubScreen,   ed->ed_Screen,
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_ICTL_NAME),
                                            WA_NewLookMenus,TRUE,
                                            WA_Gadgets,     ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        return(TRUE);
    }

    DisposeDisplay(ed);
    return(FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay(EdDataPtr ed)
{
    if (ed->ed_Window)
    {
        ClearMenuStrip(ed->ed_Window);
        CloseWindow(ed->ed_Window);
    }
    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);
}


/*****************************************************************************/


WORD chip ShiftSymbolData[] =
{
    0x0300,		/* ......**........ */
    0x0780,		/* .....****....... */
    0x0cc0,		/* ....**..**...... */
    0x1860,		/* ...**....**..... */
    0x3cf0,		/* ..****..****.... */
    0x0cc0,		/* ....**..**...... */
    0x0cc0,		/* ....**..**...... */
    0x0fc0 		/* ....******...... */
};


WORD chip AmigaSymbolData[] =
{
    0x01c0,		/* .......***...... */
    0x03c0,		/* ......****...... */
    0x06c0,		/* .....**.**...... */
    0x0cc0,		/* ....**..**...... */
    0x1fc0,		/* ...*******...... */
    0x30c0,		/* ..**....**...... */
    0xf9e0 		/* *****..****..... */
};


/*****************************************************************************/


struct Image far ShiftSymbol =
{
    0, 0, 12, 8, 1, ShiftSymbolData, 0x01, 0x00, NULL
};

struct Image far AmigaSymbol =
{
    0, 0, 12, 7, 1, AmigaSymbolData, 0x01, 0x00, NULL
};


/*****************************************************************************/


VOID PutLine(EdDataPtr ed, struct RastPort *rp, AppStringsID id,
                UWORD x, UWORD y, UWORD w, BOOL center)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    if (center)
        Move(rp,(w-TextLength(rp,str,len)) / 2 + window->BorderLeft + x,
                 window->BorderTop+y);
    else
        Move(rp,window->BorderLeft+x,window->BorderTop+y);

    Text(rp,str,len);
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
struct RastPort *rp;

    rp = ed->ed_Window->RPort;
    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
    SetBPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    PutLine(ed,rp,MSG_ICTL_SCRDRAG_HDR,10,20,194,TRUE);
    PutLine(ed,rp,MSG_ICTL_MISC_HDR,202,20,200,TRUE);

    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);

    PutLine(ed,rp,MSG_ICTL_SHIFT_GAD,57,40,0,FALSE);
    PutLine(ed,rp,MSG_ICTL_LAMIGA_GAD,57,103,0,FALSE);

    DrawImage(rp,&ShiftSymbol,42+window->BorderLeft,29+window->BorderTop);
    DrawImage(rp,&AmigaSymbol,42+window->BorderLeft,95+window->BorderTop);
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
    ed->ed_Shift = DoPrefsGadget(ed,&EG[3],ed->ed_Shift,
                                           GTCB_Checked,IEQUALIFIER_LSHIFT & ed->ed_PrefsWork.ic_MetaDrag,
                                           TAG_DONE);

    ed->ed_Ctrl = DoPrefsGadget(ed,&EG[4],ed->ed_Ctrl,
                                          GTCB_Checked,IEQUALIFIER_CONTROL & ed->ed_PrefsWork.ic_MetaDrag,
                                          TAG_DONE);

    ed->ed_Alt = DoPrefsGadget(ed,&EG[5],ed->ed_Alt,
                                         GTCB_Checked,IEQUALIFIER_LALT & ed->ed_PrefsWork.ic_MetaDrag,
                                         TAG_DONE);

    ed->ed_Amiga = DoPrefsGadget(ed,&EG[6],ed->ed_Amiga,
                                           GTCB_Checked,IEQUALIFIER_LCOMMAND & ed->ed_PrefsWork.ic_MetaDrag,
                                           TAG_DONE);

    ed->ed_MenuSnap = DoPrefsGadget(ed,&EG[7],ed->ed_MenuSnap,
                                              GTCB_Checked,ICF_MENUSNAP & ed->ed_PrefsWork.ic_Flags,
                                              TAG_DONE);

    ed->ed_GadgetFilter = DoPrefsGadget(ed,&EG[8],ed->ed_GadgetFilter,
                                                  GTCB_Checked,ICF_STRGAD_FILTER & ed->ed_PrefsWork.ic_Flags,
                                                  TAG_DONE);

    ed->ed_AvoidFlicker = DoPrefsGadget(ed,&EG[9],ed->ed_AvoidFlicker,
                                                  GTCB_Checked,ICF_COERCE_LACE & ed->ed_PrefsWork.ic_Flags,
                                                  TAG_DONE);

#undef GfxBase

    if (((struct GfxBase *)ed->ed_GfxBase)->ChipRevBits0 & GFXF_AA_ALICE)
        ed->ed_ModePromote = DoPrefsGadget(ed,&EG[10],ed->ed_ModePromote,
                                                      GTCB_Checked,ICF_MODEPROMOTE & ed->ed_PrefsWork.ic_Flags,
                                                      TAG_DONE);

#define GfxBase ed->ed_GfxBase
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
struct Gadget *gadget;
BOOL           selected;

    gadget   = ed->ed_CurrentMsg.IAddress;
    selected = (gadget->Flags & SELECTED);

    switch (ec)
    {
        case EC_SHIFT         : ed->ed_PrefsWork.ic_MetaDrag &= ~(IEQUALIFIER_LSHIFT);
                                if (selected)
                                    ed->ed_PrefsWork.ic_MetaDrag |= IEQUALIFIER_LSHIFT;
                                break;

        case EC_CTRL          : ed->ed_PrefsWork.ic_MetaDrag &= ~(IEQUALIFIER_CONTROL);
                                if (selected)
                                    ed->ed_PrefsWork.ic_MetaDrag |= IEQUALIFIER_CONTROL;
                                break;

        case EC_ALT           : ed->ed_PrefsWork.ic_MetaDrag &= ~(IEQUALIFIER_LALT);
                                if (selected)
                                    ed->ed_PrefsWork.ic_MetaDrag |= IEQUALIFIER_LALT;
                                break;

        case EC_AMIGA         : ed->ed_PrefsWork.ic_MetaDrag &= ~(IEQUALIFIER_LCOMMAND);
                                if (selected)
                                    ed->ed_PrefsWork.ic_MetaDrag |= IEQUALIFIER_LCOMMAND;
                                break;

        case EC_AVOIDFLICKER  : ed->ed_PrefsWork.ic_Flags &= ~(ICF_COERCE_LACE);
                                if (selected)
                                    ed->ed_PrefsWork.ic_Flags |= ICF_COERCE_LACE;
                                break;

        case EC_MENUSNAP      : ed->ed_PrefsWork.ic_Flags &= ~(ICF_MENUSNAP);
                                if (selected)
                                    ed->ed_PrefsWork.ic_Flags |= ICF_MENUSNAP;
                                break;

        case EC_GADGETFILTER  : ed->ed_PrefsWork.ic_Flags &= ~(ICF_STRGAD_FILTER);
                                if (selected)
                                    ed->ed_PrefsWork.ic_Flags |= ICF_STRGAD_FILTER;
                                break;

        case EC_MODEPROMOTE   : ed->ed_PrefsWork.ic_Flags &= ~(ICF_MODEPROMOTE);
                                if (selected)
                                    ed->ed_PrefsWork.ic_Flags |= ICF_MODEPROMOTE;
                                break;

        default               : break;
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
