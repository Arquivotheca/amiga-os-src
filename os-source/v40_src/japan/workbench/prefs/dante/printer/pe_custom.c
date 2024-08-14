
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
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <prefs/printertxt.h>
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
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/icon_pragmas.h>

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
#define IFFPrefChunkCnt 3
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_PTXT,
    ID_PREF, ID_PUNT,
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


VOID ProcessArgs(EdDataPtr ed, struct DiskObject *diskObj)
{
    if (diskObj)
    {
        if (FindToolType(diskObj->do_ToolTypes,"UNIT"))
        {
            ed->ed_ShowUnits = TRUE;
        }
    }
    else
        ed->ed_ShowUnits = ed->ed_Args[OPT_UNIT];
}


/*****************************************************************************/


EdStatus BuildList(EdDataPtr ed, struct List *list, STRPTR pattern, WORD strip)
{
UBYTE                exAllBuffer[512];
struct ExAllControl *eac;
struct ExAllData    *ead;
BPTR                 lock;
BPTR                 oldCD;
BOOL                 more;
char                 pat[20];
BOOL                 ok;
BOOL                 nomem = FALSE;
UWORD                len;
STRPTR		     name;
struct Node         *new;
struct Node         *node;
struct DevProc      *dvp;

    NewList(list);

    ok = FALSE;
    if (eac = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL,0))
    {
        ParsePatternNoCase(pattern,pat,20);
        eac->eac_LastKey     = 0;
        eac->eac_MatchString = pat;

        dvp = (struct DevProc *) GetDeviceProc("PRINTERS:",0);
        while (dvp)
        {
            eac->eac_LastKey = 0;                     /* use now as null BSTR */
            if (lock = DoPkt3(dvp->dvp_Port,ACTION_LOCATE_OBJECT,dvp->dvp_Lock,MKBADDR(&eac->eac_LastKey),SHARED_LOCK))
            {
                oldCD = CurrentDir(lock);

                ok = TRUE;
                do
                {
                    more = ExAll(lock,(struct ExAllData *)exAllBuffer,sizeof(exAllBuffer),ED_TYPE,eac);
                    if ((!more) && (IoErr() != ERROR_NO_MORE_ENTRIES))
                    {
                        ok = FALSE;
                        break;
                    }

                    if (eac->eac_Entries > 0)
                    {
                        ead = (struct ExAllData *) exAllBuffer;
                        do
                        {
                            if (ead->ed_Type < 0)
                            {
                                name = (STRPTR)ead->ed_Name;
                                len  = strlen(name) - strip;

                                if (new = AllocRemember(&ed->ed_Tracker,sizeof(struct Node)+len+1,MEMF_PUBLIC|MEMF_CLEAR))
                                {
                                    new->ln_Name = (STRPTR)((ULONG)new + sizeof(struct Node));
                                    CopyMem(name,new->ln_Name,len);

                                    node = list->lh_Head;
                                    while (node->ln_Succ)
                                    {
                                        if (Stricmp(node->ln_Name,new->ln_Name) >= 0)
                                            break;
                                        node = node->ln_Succ;
                                    }
                                    Insert(list,new,node->ln_Pred);
                                }
                                else
                                {
                                    nomem = TRUE;
                                    break;
                                }
                            }
                            ead = ead->ed_Next;
                        }
                        while (ead);
                    }
                }
                while (more);

                CurrentDir(oldCD);
                UnLock(lock);
            }
            dvp = GetDeviceProc("PRINTERS:",dvp);
        }

        FreeDosObject(DOS_EXALLCONTROL,eac);
    }

    if (nomem)
    {
        SetIoErr(ERROR_NO_FREE_STORE);
        ok = FALSE;
    }

    if (ok)
        return(ES_NORMAL);

    return(ES_DOSERROR);
}


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    strcpy(ed->ed_PrefsDefaults.ep_Basic.pt_Driver,"generic");
    ed->ed_PrefsDefaults.ep_Basic.pt_Port         = PP_PARALLEL;
    ed->ed_PrefsDefaults.ep_Basic.pt_PaperType    = PT_FANFOLD;
    ed->ed_PrefsDefaults.ep_Basic.pt_PaperSize    = PS_N_TRACTOR;
    ed->ed_PrefsDefaults.ep_Basic.pt_PaperLength  = 66;
    ed->ed_PrefsDefaults.ep_Basic.pt_Pitch        = PP_PICA;
    ed->ed_PrefsDefaults.ep_Basic.pt_Spacing      = PS_SIX_LPI;
    ed->ed_PrefsDefaults.ep_Basic.pt_LeftMargin   = 5;
    ed->ed_PrefsDefaults.ep_Basic.pt_RightMargin  = 75;
    ed->ed_PrefsDefaults.ep_Basic.pt_Quality      = PQ_DRAFT;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    return(ES_NORMAL);
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreeRemember(&ed->ed_Tracker,TRUE);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID == ID_PUNT && cn->cn_Type == ID_PREF)
    {
        if (ReadChunkBytes(iff,&ed->ed_PrefsWork.ep_Unit,sizeof(struct PrinterUnitPrefs)) == sizeof(struct PrinterUnitPrefs))
            return(ES_NORMAL);
    }
    else
    {
        if (cn->cn_ID != ID_PTXT || cn->cn_Type != ID_PREF)
            return(ES_IFF_UNKNOWNCHUNK);

        if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PrinterTxtPrefs)) == sizeof(struct PrinterTxtPrefs))
            return(ES_NORMAL);
    }

    return(ES_IFFERROR);
}


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (!PushChunk(iff,0,ID_PTXT,sizeof(struct PrinterTxtPrefs)))
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PrinterTxtPrefs)) == sizeof(struct PrinterTxtPrefs))
            if (!PopChunk(iff))

                if (!PushChunk(iff,0,ID_PUNT,sizeof(struct PrinterUnitPrefs)))
                    if (WriteChunkBytes(iff,&ed->ed_PrefsWork.ep_Unit,sizeof(struct PrinterUnitPrefs)) == sizeof(struct PrinterUnitPrefs))
                        if (!PopChunk(iff))
                            return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     556
#define NW_HEIGHT    215
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
    {BUTTON_KIND,   8,   189,  87,  20, MSG_SAVE_GAD,         EC_SAVE,         0},
    {BUTTON_KIND,   234, 189,  87,  20, MSG_USE_GAD,          EC_USE,          0},
    {BUTTON_KIND,   461, 189,  87,  20, MSG_CANCEL_GAD,       EC_CANCEL,       0},

    {LISTVIEW_KIND, 8,   20,   204, 140,MSG_PRT_PRINTERTYPE_GAD,  EC_PRINTERS,     0},
    {INTEGER_KIND,  504, 122,  44,  20, MSG_PRT_PAPERLENGTH_GAD,  EC_PAPERLENGTH,  0},
    {INTEGER_KIND,  504, 143,  44,  20, MSG_PRT_LEFTMARGIN_GAD,   EC_LEFTMARGIN,   0},
    {INTEGER_KIND,  504, 164,  44,  20, MSG_PRT_RIGHTMARGIN_GAD,  EC_RIGHTMARGIN,  0},

    {CYCLE_KIND,    372,  8,   176, 18, MSG_PRT_PRINTERPORT_GAD,  EC_PRINTERPORT,  0},
    {CYCLE_KIND,    372,  27,  176, 18, MSG_PRT_PRINTPITCH_GAD,   EC_PRINTPITCH,   0},
    {CYCLE_KIND,    372,  46,  176, 18, MSG_PRT_PRINTSPACING_GAD, EC_PRINTSPACING, 0},
    {CYCLE_KIND,    372,  65,  176, 18, MSG_PRT_PRINTQUALITY_GAD, EC_PRINTQUALITY, 0},
    {CYCLE_KIND,    372,  84,  176, 18, MSG_PRT_PAPERTYPE_GAD,    EC_PAPERTYPE,    0},
    {CYCLE_KIND,    372,  103,  176, 18, MSG_PRT_PAPERSIZE_GAD,    EC_PAPERSIZE,    0},

    {INTEGER_KIND,  166,  138, 46,  18, MSG_PRT_DEVICEUNIT_GAD,   EC_UNIT,         0}
};


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    BuildList(ed,&ed->ed_AvailPrinters,"~(#?.info)",0);

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_PrinterPortLabels[0]  = GetString(&ed->ed_LocaleInfo,MSG_PRT_PARALLEL);
    ed->ed_PrinterPortLabels[1]  = GetString(&ed->ed_LocaleInfo,MSG_PRT_SERIAL);
    ed->ed_PaperTypeLabels[0]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_FANFOLD);
    ed->ed_PaperTypeLabels[1]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_SINGLE);
    ed->ed_PaperSizeLabels[0]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_USLETTER);
    ed->ed_PaperSizeLabels[1]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_USLEGAL);
    ed->ed_PaperSizeLabels[2]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_NARROWTRACTOR);
    ed->ed_PaperSizeLabels[3]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_WIDETRACTOR);
    ed->ed_PaperSizeLabels[4]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_CUSTOM);
    ed->ed_PaperSizeLabels[5]    = GetString(&ed->ed_LocaleInfo,MSG_PRT_DINA4);
    ed->ed_PrintPitchLabels[0]   = GetString(&ed->ed_LocaleInfo,MSG_PRT_PICA10);
    ed->ed_PrintPitchLabels[1]   = GetString(&ed->ed_LocaleInfo,MSG_PRT_ELITE12);
    ed->ed_PrintPitchLabels[2]   = GetString(&ed->ed_LocaleInfo,MSG_PRT_FINE15);
    ed->ed_PrintSpacingLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_PRT_LPI6);
    ed->ed_PrintSpacingLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_PRT_LPI8);
    ed->ed_PrintQualityLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_PRT_DRAFT);
    ed->ed_PrintQualityLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_PRT_LETTER);

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
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_PRT_NAME),
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


VOID RenderGadgets(EdDataPtr ed)
{
UWORD           i;
struct Node    *node;
ULONG           pSize;
struct EdGadget eg;

    pSize = ed->ed_PrefsWork.ep_Basic.pt_PaperSize;
    if (pSize == PS_EURO_A4)
        pSize = PS_EURO_A0;

    eg = EG[3];
    if (ed->ed_ShowUnits)
        eg.eg_Height = eg.eg_Height-20;

    i    = 0;
    node = ed->ed_AvailPrinters.lh_Head;
    while (node->ln_Succ)
    {
        if (Stricmp(ed->ed_PrefsWork.ep_Basic.pt_Driver,node->ln_Name) == 0)
            break;
        i++;
        node = node->ln_Succ;
    }

    if (!node->ln_Succ)
        i = 0;

    ed->ed_Printers = DoPrefsGadget(ed,&eg,ed->ed_Printers,
                                           GTLV_Selected,     i,
                                           GTLV_MakeVisible,  i,
                                           GTLV_Labels,       &ed->ed_AvailPrinters,
                                           GTLV_ShowSelected, NULL,
                                           LAYOUTA_SPACING,   4,
                                           GTLV_ScrollWidth,  18,
                                           TAG_DONE);

    ed->ed_PaperLength = DoPrefsGadget(ed,&EG[4],ed->ed_PaperLength,
                                                 GTIN_Number,   ed->ed_PrefsWork.ep_Basic.pt_PaperLength,
                                                 GTIN_MaxChars, 3,
                                                 TAG_DONE);

    ed->ed_LeftMargin = DoPrefsGadget(ed,&EG[5],ed->ed_LeftMargin,
                                                GTIN_Number,   ed->ed_PrefsWork.ep_Basic.pt_LeftMargin,
                                                GTIN_MaxChars, 3,
						TAG_DONE);

    ed->ed_RightMargin = DoPrefsGadget(ed,&EG[6],ed->ed_RightMargin,
                                                 GTIN_Number,   ed->ed_PrefsWork.ep_Basic.pt_RightMargin,
                                                 GTIN_MaxChars, 3,
                                                 TAG_DONE);

    ed->ed_PrinterPort = DoPrefsGadget(ed,&EG[7],ed->ed_PrinterPort,
                                                 GTCY_Active, ed->ed_PrefsWork.ep_Basic.pt_Port,
                                                 GTCY_Labels, ed->ed_PrinterPortLabels,
                                                 TAG_DONE);

    ed->ed_PrintPitch = DoPrefsGadget(ed,&EG[8],ed->ed_PrintPitch,
                                                GTCY_Active, ed->ed_PrefsWork.ep_Basic.pt_Pitch,
                                                GTCY_Labels, ed->ed_PrintPitchLabels,
                                                TAG_DONE);

    ed->ed_PrintSpacing = DoPrefsGadget(ed,&EG[9],ed->ed_PrintSpacing,
                                                  GTCY_Active, ed->ed_PrefsWork.ep_Basic.pt_Spacing,
                                                  GTCY_Labels, ed->ed_PrintSpacingLabels,
                                                  TAG_DONE);

    ed->ed_PrintQuality = DoPrefsGadget(ed,&EG[10],ed->ed_PrintQuality,
                                                   GTCY_Active, ed->ed_PrefsWork.ep_Basic.pt_Quality,
                                                   GTCY_Labels, ed->ed_PrintQualityLabels,
                                                   TAG_DONE);

    ed->ed_PaperType = DoPrefsGadget(ed,&EG[11],ed->ed_PaperType,
                                                GTCY_Active, ed->ed_PrefsWork.ep_Basic.pt_PaperType,
                                                GTCY_Labels, ed->ed_PaperTypeLabels,
                                                TAG_DONE);

    ed->ed_PaperSize = DoPrefsGadget(ed,&EG[12],ed->ed_PaperSize,
                                                GTCY_Active, pSize,
                                                GTCY_Labels, ed->ed_PaperSizeLabels,
                                                TAG_DONE);

    if (ed->ed_ShowUnits)
        ed->ed_Unit = DoPrefsGadget(ed,&EG[13],ed->ed_Unit,
                                               GTIN_Number,   ed->ed_PrefsWork.ep_Unit.pu_UnitNum,
                                               GTIN_MaxChars, 3,
                                               TAG_DONE);
}


/*****************************************************************************/


VOID ProcessTextGadget(EdDataPtr ed, EdCommand ec, BOOL screenStuff)
{
LONG           num;
BOOL           refresh;
BOOL           shift;
struct Gadget *act;

    refresh = FALSE;
    act     = NULL;
    shift   = (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT) & ed->ed_CurrentMsg.Qualifier;

    switch (ec)
    {
        case EC_PAPERLENGTH    : num = ((struct StringInfo *)ed->ed_PaperLength->SpecialInfo)->LongInt;
                                 if (num < 1)
                                 {
                                     num = 1;
                                     act = ed->ed_PaperLength;
                                     refresh = TRUE;
                                 }
                                 else
                                 {
                                     if (shift)
                                         act = ed->ed_RightMargin;
                                     else
                                         act = ed->ed_LeftMargin;
                                 }
                                 ed->ed_PrefsWork.ep_Basic.pt_PaperLength = num;
				 break;

        case EC_LEFTMARGIN     : num = ((struct StringInfo *)ed->ed_LeftMargin->SpecialInfo)->LongInt;
                                 if (num < 0)
                                 {
                                     num = 0;
                                     act = ed->ed_LeftMargin;
                                     refresh = TRUE;
                                 }
                                 else if (num > ed->ed_PrefsWork.ep_Basic.pt_RightMargin)
                                 {
                                     num = ed->ed_PrefsWork.ep_Basic.pt_RightMargin;
                                     act = ed->ed_LeftMargin;
                                     refresh = TRUE;
                                  }
                                 else
                                 {
                                     if (shift)
                                         act = ed->ed_PaperLength;
                                     else
                                         act = ed->ed_RightMargin;
                                 }
                                 ed->ed_PrefsWork.ep_Basic.pt_LeftMargin = num;
			         break;

        case EC_RIGHTMARGIN    : num = ((struct StringInfo *)ed->ed_RightMargin->SpecialInfo)->LongInt;
                                 if (num < ed->ed_PrefsWork.ep_Basic.pt_LeftMargin)
                                 {
                                     num = ed->ed_PrefsWork.ep_Basic.pt_LeftMargin;
                                     act = ed->ed_RightMargin;
                                     refresh = TRUE;
                                 }
                                 else
                                 {
                                     if (shift)
                                         act = ed->ed_LeftMargin;
                                     else
                                         act = ed->ed_PaperLength;
                                 }
                                 ed->ed_PrefsWork.ep_Basic.pt_RightMargin = num;
				 break;

        case EC_UNIT           : num = ((struct StringInfo *)ed->ed_Unit->SpecialInfo)->LongInt;
                                 if (num < 0)
                                 {
                                     num = 0;
                                     refresh = TRUE;
                                 }
                                 else if (num > 255)
                                 {
                                     num = 255;
                                     refresh = TRUE;
                                 }
                                 ed->ed_PrefsWork.ep_Unit.pu_UnitNum = num;
				 break;
    }

    if (screenStuff)
    {
        if (refresh)
        {
            DisplayBeep(window->WScreen);
            RenderGadgets(ed);
        }

        if (act)
            ActivateGadget(act,window,NULL);
    }
}


/*****************************************************************************/


VOID SyncTextGadgets(EdDataPtr ed)
{
    ProcessTextGadget(ed,EC_PAPERLENGTH,FALSE);
    ProcessTextGadget(ed,EC_LEFTMARGIN,FALSE);
    ProcessTextGadget(ed,EC_RIGHTMARGIN,FALSE);

    if (ed->ed_Unit)
        ProcessTextGadget(ed,EC_UNIT,FALSE);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD        icode;
struct Node *node;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
        case EC_PRINTERS       : node = ed->ed_AvailPrinters.lh_Head;
                                 while (icode--)
                                 {
                                    node = node->ln_Succ;
                                 }
                                 strcpy(ed->ed_PrefsWork.ep_Basic.pt_Driver,node->ln_Name);
                                 break;

        case EC_PRINTERPORT    : ed->ed_PrefsWork.ep_Basic.pt_Port = icode;
                                 break;

        case EC_PAPERTYPE      : ed->ed_PrefsWork.ep_Basic.pt_PaperType = icode;
                                 break;

        case EC_PAPERSIZE      : if (icode == PS_EURO_A0)
                                     icode = PS_EURO_A4;
                                 ed->ed_PrefsWork.ep_Basic.pt_PaperSize = icode;
                                 break;

        case EC_PRINTPITCH     : ed->ed_PrefsWork.ep_Basic.pt_Pitch = icode;
                                 break;

        case EC_PRINTSPACING   : ed->ed_PrefsWork.ep_Basic.pt_Spacing = icode;
                                 break;

        case EC_PRINTQUALITY   : ed->ed_PrefsWork.ep_Basic.pt_Quality = icode;
                                 break;

        case EC_PAPERLENGTH    :
        case EC_LEFTMARGIN     :
        case EC_RIGHTMARGIN    :
        case EC_UNIT           : ProcessTextGadget(ed,ec,TRUE);
				 break;

        default                : break;
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
