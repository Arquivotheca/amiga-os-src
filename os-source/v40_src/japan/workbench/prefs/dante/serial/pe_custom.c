
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
#include <prefs/serial.h>
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


UWORD BaudToSlider(UWORD baudRate);
UWORD BufferToSlider(ULONG bufSize);
ULONG __stdargs SliderToBaud(struct Gadget *gadget, UWORD sliderLevel);
ULONG __stdargs SliderToBuffer(struct Gadget *gadget, UWORD sliderLevel);


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_SERL,
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
    {
        ed->ed_ShowUnits = ed->ed_Args[OPT_UNIT];
    }
}


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    ed->ed_PrefsDefaults.sp_Unit0Map        = 1;
    ed->ed_PrefsDefaults.sp_BaudRate        = 9600;
    ed->ed_PrefsDefaults.sp_InputBuffer     = 512;
    ed->ed_PrefsDefaults.sp_OutputBuffer    = 512;
    ed->ed_PrefsDefaults.sp_InputHandshake  = HSHAKE_XON;
    ed->ed_PrefsDefaults.sp_OutputHandshake = HSHAKE_XON;
    ed->ed_PrefsDefaults.sp_Parity          = PARITY_NONE;
    ed->ed_PrefsDefaults.sp_BitsPerChar     = 8;
    ed->ed_PrefsDefaults.sp_StopBits        = 1;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    return(ES_NORMAL);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID != ID_SERL || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct SerialPrefs)) == sizeof(struct SerialPrefs))
        return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
EdStatus result;

    result = ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs);

    if (ed->ed_PrefsWork.sp_Unit0Map == 0)
        ed->ed_PrefsWork.sp_Unit0Map = 1;

    return(result);
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (!PushChunk(iff,0,ID_SERL,sizeof(struct SerialPrefs)))
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct SerialPrefs)) == sizeof(struct SerialPrefs))
            if (!PopChunk(iff))
                return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     485
#define NW_HEIGHT    185
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
    {BUTTON_KIND,   8,   160,  87,  20, MSG_SAVE_GAD,         EC_SAVE,     0},
    {BUTTON_KIND,   191, 160,  87,  20, MSG_USE_GAD,          EC_USE,      0},
    {BUTTON_KIND,   375, 160,  87,  20, MSG_CANCEL_GAD,       EC_CANCEL,   0},

    {SLIDER_KIND,   300, 8,    150, 11, MSG_SER_BAUDRATE_GAD,     EC_BAUDRATE, 0},
    {SLIDER_KIND,   300, 26,   150, 11, MSG_SER_INPUTBUF_GAD,     EC_INPUTBUF, 0},

    {MX_KIND,       100, 65,   110, 11, MSG_SER_HANDSHAKING_GAD,  EC_HANDSHAKING, 0},
    {MX_KIND,       210, 65,   110, 11, MSG_SER_PARITY_GAD,       EC_PARITY,      0},
    {MX_KIND,       320, 65,   110, 11, MSG_SER_DATABITS_GAD,     EC_DATABITS,    0},
    {MX_KIND,       430, 65,   110, 11, MSG_SER_STOPBITS_GAD,     EC_STOPBITS,    0},

    {INTEGER_KIND,  400, 93,   46,  14, MSG_SER_DEFAULTUNIT_GAD,  EC_DEFAULTUNIT, 0}
};


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD              zoomSize[4];
struct LocaleInfo *li;

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    li = &ed->ed_LocaleInfo;
    ed->ed_HandshakingLabels[0]  = GetString(li,MSG_SER_XON_XOFF);
    ed->ed_HandshakingLabels[1]  = GetString(li,MSG_SER_RTS_CTS);
    ed->ed_HandshakingLabels[2]  = GetString(li,MSG_SER_NONE);
    ed->ed_ParityLabels[0]       = GetString(li,MSG_SER_PNONE);
    ed->ed_ParityLabels[1]       = GetString(li,MSG_SER_EVEN);
    ed->ed_ParityLabels[2]       = GetString(li,MSG_SER_ODD);
    ed->ed_ParityLabels[3]       = GetString(li,MSG_SER_MARK);
    ed->ed_ParityLabels[4]       = GetString(li,MSG_SER_SPACE);
    ed->ed_DataBitsLabels[0]     = "7";
    ed->ed_DataBitsLabels[1]     = "8";
    ed->ed_StopBitsLabels[0]     = "1";
    ed->ed_StopBitsLabels[1]     = "2";

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
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_SER_NAME),
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


ULONG far BaudRates[8] = {110, 300,  1200, 2400, 4800, 9600,  19200, 31250};
ULONG far BufferSizes[8] = {512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};


/*****************************************************************************/


UWORD BaudToSlider(UWORD baudRate)
{
UWORD i;

    for (i = 0; i < 8; i++)
    {
	if (baudRate == BaudRates[i])
	    return(i);
    }
    return(0);
}


/*****************************************************************************/


ULONG __stdargs SliderToBaud(struct Gadget *gadget, UWORD sliderLevel)
{
    return(BaudRates[sliderLevel]);
}


/*****************************************************************************/


UWORD BufferToSlider(ULONG bufSize)
{
UWORD i;

    for (i = 0; i < 8; i++)
    {
	if (bufSize == BufferSizes[i])
	    return(i);
    }

    return(0);
}


/*****************************************************************************/


ULONG __stdargs SliderToBuffer(struct Gadget *gadget, UWORD sliderLevel)
{
    return(BufferSizes[sliderLevel]);
}


/*****************************************************************************/


VOID RightAlign(EdDataPtr ed, struct RastPort *rp, AppStringsID id,
                UWORD x, UWORD y)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(rp,x - TextLength(rp,str,len) + window->BorderLeft+5,
            window->BorderTop+y);
    Text(rp,str,len);
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
struct RastPort *rp;

    rp = ed->ed_Window->RPort;
    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
    SetBPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    RightAlign(ed,rp,MSG_SER_HANDSHAKING_GAD,105,60);
    RightAlign(ed,rp,MSG_SER_PARITY_GAD,215,60);
    RightAlign(ed,rp,MSG_SER_DATABITS_GAD,325,60);
    RightAlign(ed,rp,MSG_SER_STOPBITS_GAD,435,60);
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
STRPTR ptr;

    ptr = "%7lu";
    if (LocaleBase)
        ptr = "%7lU";

    ed->ed_BaudRate = DoPrefsGadget(ed,&EG[3],ed->ed_BaudRate,
                                              GTSL_Level,       BaudToSlider(ed->ed_PrefsWork.sp_BaudRate),
                                              GTSL_Max,         7,
                                              GTSL_MaxLevelLen, 7,
                                              GTSL_LevelFormat, ptr,
                                              GTSL_DispFunc,    SliderToBaud,
                                              GA_Immediate,     TRUE,
                                              TAG_DONE);

    ed->ed_InputBuf = DoPrefsGadget(ed,&EG[4],ed->ed_InputBuf,
                                              GTSL_Level,       BufferToSlider(ed->ed_PrefsWork.sp_InputBuffer),
                                              GTSL_Max,         7,
                                              GTSL_MaxLevelLen, 7,
                                              GTSL_LevelFormat, ptr,
                                              GTSL_DispFunc,    SliderToBuffer,
                                              GA_Immediate,     TRUE,
                                              TAG_DONE);

    ed->ed_Handshaking = DoPrefsGadget(ed,&EG[5],ed->ed_Handshaking,
                                                 GTMX_Active,     ed->ed_PrefsWork.sp_InputHandshake,
                                                 GTMX_Labels,     ed->ed_HandshakingLabels,
                                                 LAYOUTA_SPACING, 8,
                                                 TAG_DONE);

    ed->ed_Parity = DoPrefsGadget(ed,&EG[6],ed->ed_Parity,
                                            GTMX_Active,     ed->ed_PrefsWork.sp_Parity,
                                            GTMX_Labels,     ed->ed_ParityLabels,
                                            LAYOUTA_SPACING, 8,
                                            TAG_DONE);

    ed->ed_DataBits = DoPrefsGadget(ed,&EG[7],ed->ed_DataBits,
                                              GTMX_Active,     ed->ed_PrefsWork.sp_BitsPerChar-7,
                                              GTMX_Labels,     ed->ed_DataBitsLabels,
                                              LAYOUTA_SPACING, 8,
                                              TAG_DONE);

    ed->ed_StopBits = DoPrefsGadget(ed,&EG[8],ed->ed_StopBits,
                                              GTMX_Active,     ed->ed_PrefsWork.sp_StopBits-1,
                                              GTMX_Labels,     ed->ed_StopBitsLabels,
                                              LAYOUTA_SPACING, 8,
                                              TAG_DONE);

    if (ed->ed_ShowUnits)
        ed->ed_DefaultUnit = DoPrefsGadget(ed,&EG[9],ed->ed_DefaultUnit,
                                                     GTIN_Number,   ed->ed_PrefsWork.sp_Unit0Map,
                                                     GTIN_MaxChars, 3,
						     TAG_DONE);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD icode;
LONG  num;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
        case EC_BAUDRATE   : ed->ed_PrefsWork.sp_BaudRate = SliderToBaud(NULL,icode);
                             break;

        case EC_INPUTBUF   : ed->ed_PrefsWork.sp_InputBuffer = SliderToBuffer(NULL,icode);
                             break;

        case EC_HANDSHAKING: ed->ed_PrefsWork.sp_InputHandshake = icode;
                             break;

        case EC_PARITY     : ed->ed_PrefsWork.sp_Parity = icode;
                             break;

        case EC_DATABITS   : ed->ed_PrefsWork.sp_BitsPerChar = icode+7;
                             break;

        case EC_STOPBITS   : ed->ed_PrefsWork.sp_StopBits = icode+1;
                             break;

        case EC_DEFAULTUNIT: num = ((struct StringInfo *)ed->ed_DefaultUnit->SpecialInfo)->LongInt;
                             if (num < 0)
                             {
                                 num = 0;
                                 DisplayBeep(window->WScreen);
                                 RenderGadgets(ed);
                             }
                             else if (num > 255)
                             {
                                 num = 255;
                                 DisplayBeep(window->WScreen);
                                 RenderGadgets(ed);
                             }
                             ed->ed_PrefsWork.sp_Unit0Map = num;
                             break;

        default            : break;
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
