
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
#include <prefs/font.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>
#include <stdio.h>

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
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_FONT,
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


#define WRITE_ALL    0
#define WRITE_WB     1
#define WRITE_SYS    2
#define WRITE_SCREEN 3


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    ed->ed_PrefsDefaults.ep_WBFont.fp_Type              = FP_WBFONT;
    ed->ed_PrefsDefaults.ep_WBFont.fp_FrontPen          = 1;
    ed->ed_PrefsDefaults.ep_WBFont.fp_BackPen           = 0;
    ed->ed_PrefsDefaults.ep_WBFont.fp_DrawMode          = JAM2;
    ed->ed_PrefsDefaults.ep_WBFont.fp_TextAttr.ta_YSize = 8;
    ed->ed_PrefsDefaults.ep_WBFont.fp_TextAttr.ta_Flags = FPF_ROMFONT;
    strcpy(ed->ed_PrefsDefaults.ep_WBFont.fp_Name,"topaz.font");

    ed->ed_PrefsDefaults.ep_SysFont         = ed->ed_PrefsDefaults.ep_WBFont;
    ed->ed_PrefsDefaults.ep_SysFont.fp_Type = FP_SYSFONT;

    ed->ed_PrefsDefaults.ep_ScreenFont         = ed->ed_PrefsDefaults.ep_WBFont;
    ed->ed_PrefsDefaults.ep_ScreenFont.fp_Type = FP_SCREENFONT;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    return(ES_NORMAL);
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreeAslRequest(ed->ed_FontReq);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
struct FontPrefs  tmp;
struct FontPrefs *fp;

    if (cn->cn_ID != ID_FONT || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&tmp,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
    {
        if (tmp.fp_Type == FP_WBFONT)
            fp = &ed->ed_PrefsWork.ep_WBFont;

        else if (tmp.fp_Type == FP_SYSFONT)
            fp = &ed->ed_PrefsWork.ep_SysFont;

        else if (tmp.fp_Type == FP_SCREENFONT)
            fp = &ed->ed_PrefsWork.ep_ScreenFont;

        *fp = tmp;

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
    if ((ed->ed_Write == WRITE_WB) || (ed->ed_Write == WRITE_ALL))
    {
        if (!PushChunk(iff,0,ID_FONT,sizeof(struct FontPrefs)))
            if (WriteChunkBytes(iff,&ed->ed_PrefsWork.ep_WBFont,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
                PopChunk(iff);
    }

    if ((ed->ed_Write == WRITE_SYS) || (ed->ed_Write == WRITE_ALL))
    {
        if (!PushChunk(iff,0,ID_FONT,sizeof(struct FontPrefs)))
            if (WriteChunkBytes(iff,&ed->ed_PrefsWork.ep_SysFont,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
                PopChunk(iff);
    }

    if ((ed->ed_Write == WRITE_SCREEN) || (ed->ed_Write == WRITE_ALL))
    {
        if (!PushChunk(iff,0,ID_FONT,sizeof(struct FontPrefs)))
            if (WriteChunkBytes(iff,&ed->ed_PrefsWork.ep_ScreenFont,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
                PopChunk(iff);
    }

    return(ES_NORMAL);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
EdStatus result;

    result = ES_NORMAL;
    if (SysBase->LibNode.lib_Version < 39)
    {
        if (strcmp(name,ENV_NAME) == 0)
        {
            ed->ed_Write = WRITE_WB;
            WriteIFF(ed,"ENV:Sys/wbfont.prefs",&IFFPrefHeader,WritePrefs);

            ed->ed_Write = WRITE_SYS;
            result = WriteIFF(ed,"ENV:Sys/sysfont.prefs",&IFFPrefHeader,WritePrefs);
        }
        else if (strcmp(name,ARC_NAME) == 0)
        {
            ed->ed_Write = WRITE_WB;
            WriteIFF(ed,"ENVARC:Sys/wbfont.prefs",&IFFPrefHeader,WritePrefs);

            ed->ed_Write = WRITE_SYS;
            result = WriteIFF(ed,"ENVARC:Sys/sysfont.prefs",&IFFPrefHeader,WritePrefs);
        }
    }

    if (result == ES_NORMAL)
    {
        ed->ed_Write = WRITE_ALL;
        result = WriteIFF(ed,name,&IFFPrefHeader,WritePrefs);
    }

    return(result);
}


/*****************************************************************************/


#define NW_WIDTH     394
#define NW_HEIGHT    130
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
    {BUTTON_KIND,   8,   113,  87,  14, MSG_SAVE_GAD,         EC_SAVE,     0},
    {BUTTON_KIND,   153, 113,  87,  14, MSG_USE_GAD,          EC_USE,      0},
    {BUTTON_KIND,   299, 113,  87,  14, MSG_CANCEL_GAD,       EC_CANCEL,   0},

    {BUTTON_KIND,   10,  56,   376, 14, MSG_FNT_WBFONT_GAD,     EC_SELECTWBFONT,     0},
    {BUTTON_KIND,   10,  72,   376, 14, MSG_FNT_SYSFONT_GAD,    EC_SELECTSYSFONT,    0},
    {BUTTON_KIND,   10,  88,   376, 14, MSG_FNT_SCREENFONT_GAD, EC_SELECTSCREENFONT, 0}
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
    DoPrefsGadget(ed,&EG[3],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[4],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[5],NULL,TAG_DONE);

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
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_FNT_NAME),
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


VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
    Text(wp->RPort,str,len);
}


/*****************************************************************************/


VOID PutFont(EdDataPtr ed, AppStringsID id, UWORD x, UWORD y, struct FontPrefs *fp)
{
char   buffer[46];
char   name[FONTNAMESIZE+1];
UWORD  len;
STRPTR header;

    strcpy(name,fp->fp_Name);
    len = strlen(name);
    if ((len > 5) & (Stricmp(&name[len-5],".font") == 0))
        name[len-5] = 0;

    header = GetString(&ed->ed_LocaleInfo,id);
    if (strlen(name) + strlen(header) > 45)
        name[45-strlen(header)] = 0;

    sprintf(buffer,header,name,fp->fp_TextAttr.ta_YSize);

    len = strlen(buffer);
    while (len < 45)
        buffer[len++] = ' ';
    buffer[len] = 0;

    Move(window->RPort,x+window->BorderLeft,y+window->BorderTop);
    Text(window->RPort,buffer,strlen(buffer));
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
    if (window)
    {
        SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
        SetBPen(window->RPort,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

        CenterLine(ed,window,MSG_FNT_FONTS_HDR,10,12,376);
        DrawBB(ed,10,18,376,35,GT_VisualInfo, ed->ed_VisualInfo,
                               GTBB_Recessed, TRUE,
                               TAG_DONE);

        PutFont(ed,MSG_FNT_WBFONT,    14,28, &ed->ed_PrefsWork.ep_WBFont);
        PutFont(ed,MSG_FNT_SYSFONT,   14,37, &ed->ed_PrefsWork.ep_SysFont);
        PutFont(ed,MSG_FNT_SCREENFONT,14,46, &ed->ed_PrefsWork.ep_ScreenFont);
    }
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
    RenderDisplay(ed);
}


/*****************************************************************************/


#define ASM __asm
#define REG(x) register __## x

VOID ASM IntuiHook(REG(a0) struct Hook *hook,
                   REG(a2) struct FileRequester *fr,
                   REG(a1) struct IntuiMessage *intuiMsg);


/*****************************************************************************/


BOOL RequestPrefsFont(EdDataPtr ed, ULONG tag1, ...)
{
    if (!ed->ed_FontReq)
        ed->ed_FontReq = AllocAslRequest(ASL_FontRequest,NULL);

    return(AslRequest(ed->ed_FontReq,(struct TagItem *) &tag1));
}


/*****************************************************************************/


BOOL SelectFont(EdDataPtr ed, struct FontPrefs *fp, AppStringsID title,
                BOOL propFonts, BOOL pens)
{
ULONG       flags;
STRPTR      modes[5];
BOOL        result;
struct Hook hook;
UBYTE       table[8];
UWORD       i;
UWORD       numColors;
UBYTE       fpen, bpen;

    for (i = 0; i < 4; i++)
        table[i] = i;

    numColors = (1 << ed->ed_DrawInfo->dri_Depth);
    if (numColors > 4)
    {
        for (i = 4; i < 8; i++)
            table[i] = numColors - (8 - i);
    }

    hook.h_Entry = IntuiHook;

    flags = 0;

    if (!propFonts)
        flags |= FONF_FIXEDWIDTH;

    if (pens)
        flags |= FONF_FRONTCOLOR | FONF_BACKCOLOR | FONF_DRAWMODE;

    fpen = fp->fp_FrontPen % 8;
    if (fpen > 3)
        fpen = numColors - (8 - fpen);

    bpen = fp->fp_BackPen % 8;
    if (bpen > 3)
        bpen = numColors - (8 - bpen);

    modes[0] = GetString(&ed->ed_LocaleInfo,MSG_FNT_MODE_GAD);
    modes[1] = GetString(&ed->ed_LocaleInfo,MSG_FNT_TEXT);
    modes[2] = GetString(&ed->ed_LocaleInfo,MSG_FNT_TEXTFIELD);
    modes[3] = NULL;

    if (result = RequestPrefsFont(ed,ASLFO_TitleText,      GetString(&ed->ed_LocaleInfo,title),
                                     ASLFO_Window,         window,
                                     ASLFO_InitialHeight,  350,
                                     ASLFO_InitialName,    fp->fp_Name,
                                     ASLFO_InitialSize,    fp->fp_TextAttr.ta_YSize,
                                     ASLFO_InitialStyle,   fp->fp_TextAttr.ta_Style,
                                     ASLFO_InitialFlags,   fp->fp_TextAttr.ta_Flags,
                                     ASLFO_InitialFrontPen,fpen,
                                     ASLFO_InitialBackPen, bpen,
                                     ASLFO_InitialDrawMode,fp->fp_DrawMode,
                                     ASLFO_MinHeight,      4,
                                     ASLFO_MaxHeight,      124,
                                     ASLFO_Flags,          flags,
                                     ASLFO_IntuiMsgFunc,   &hook,
                                     ASLFO_ModeList,       modes,
                                     ASLFO_SleepWindow,    TRUE,
                                     ASLFO_MaxFrontPen,    8,
                                     ASLFO_MaxBackPen,     8,
                                     ASLFO_FrontPens,      table,
                                     ASLFO_BackPens,       table,
                                     TAG_DONE))
    {
        if ((ed->ed_FontReq->fo_Attr.ta_YSize >= 4)
        &&  (ed->ed_FontReq->fo_Attr.ta_YSize <= 124)
        &&  (propFonts || (!(ed->ed_FontReq->fo_Attr.ta_Flags & FPF_PROPORTIONAL))))
        {
            strcpy(fp->fp_Name,ed->ed_FontReq->fo_Attr.ta_Name);
            fp->fp_TextAttr.ta_YSize = ed->ed_FontReq->fo_Attr.ta_YSize;
            fp->fp_TextAttr.ta_Style = ed->ed_FontReq->fo_Attr.ta_Style;
            fp->fp_TextAttr.ta_Flags = ed->ed_FontReq->fo_Attr.ta_Flags;
            fp->fp_FrontPen          = ed->ed_FontReq->fo_FrontPen;
            fp->fp_BackPen           = ed->ed_FontReq->fo_BackPen;
            fp->fp_DrawMode          = ed->ed_FontReq->fo_DrawMode;

            if (fp->fp_FrontPen > 3)
                fp->fp_FrontPen = 248 + fp->fp_FrontPen + 8 - numColors;

            if (fp->fp_BackPen > 3)
                fp->fp_BackPen = 248 + fp->fp_BackPen + 8 - numColors;
        }
        else
        {
            DisplayBeep(NULL);
        }
    }

    return(result);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
BOOL refresh;

    refresh = FALSE;

    switch (ec)
    {
        case EC_SELECTWBFONT    : refresh = SelectFont(ed,&ed->ed_PrefsWork.ep_WBFont,MSG_FNT_WBFONT_TITLE,TRUE,TRUE);
                                  break;

        case EC_SELECTSYSFONT   : refresh = SelectFont(ed,&ed->ed_PrefsWork.ep_SysFont,MSG_FNT_SYSFONT_TITLE,FALSE,FALSE);
                                  break;

        case EC_SELECTSCREENFONT: refresh = SelectFont(ed,&ed->ed_PrefsWork.ep_ScreenFont,MSG_FNT_SCREENFONT_TITLE,TRUE,FALSE);
                                  break;

        default                 : break;
    }

    if (refresh)
        RenderDisplay(ed);
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
