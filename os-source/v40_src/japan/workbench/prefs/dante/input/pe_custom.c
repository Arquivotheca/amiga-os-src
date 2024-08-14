
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

VOID FreeKeyMap(EdDataPtr ed, struct KeyMapNode *kn);
VOID SetInputPrefs(EdDataPtr ed, struct InputPrefs *ip);


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_INPT,
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


struct KeyNode
{
    struct Node kn_Node;
    char        kn_RealName[16];
};


STRPTR far stupidKludge[] =
{
     "cdn",  "Canadien Français",
     "ch1",  "Suisse",
     "ch2",  "Schweiz",
     "d",    "Deutsch",
     "dk",   "Dansk",
     "e",    "Español",
     "f",    "Français",
     "gb",   "British",
     "i",    "Italiana",
     "n",    "Norsk",
     "po",   "Português",
     "s",    "Svenskt",
     "usa0", "American (A1000)",
     "usa2", "Dvorak",
     "usa3", "American (A600)",
     NULL,   NULL
};


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
struct KeyNode      *node;
struct KeyNode      *new;
UWORD                i;
struct DevProc      *dvp;

    NewList(list);

    len = strlen("American");
    if (new = AllocVec(sizeof(struct KeyNode)+len+1,MEMF_PUBLIC|MEMF_CLEAR))
    {
        strcpy(new->kn_RealName,"usa1");
        new->kn_Node.ln_Name = (STRPTR)((ULONG)new + sizeof(struct KeyNode));
        CopyMem("American",new->kn_Node.ln_Name,len);
        AddHead(list,(struct Node *)new);
    }

    ok = FALSE;
    if (eac = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL,0))
    {
        ParsePatternNoCase(pattern,pat,20);
        eac->eac_LastKey     = 0;
        eac->eac_MatchString = pat;

        dvp = (struct DevProc *) GetDeviceProc("KEYMAPS:",0);
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

                                i = 0;
                                while (stupidKludge[i])
                                {
                                    if (Strnicmp(name,stupidKludge[i],len) == 0)
                                    {
                                        name = stupidKludge[i+1];
                                        len = strlen(name);
                                        break;
                                    }
                                    i = i + 2;
                                }

                                if (new = AllocVec(sizeof(struct KeyNode)+len+1,MEMF_PUBLIC|MEMF_CLEAR))
                                {
                                    CopyMem((STRPTR)ead->ed_Name,new->kn_RealName,15);
                                    new->kn_Node.ln_Name = (STRPTR)((ULONG)new + sizeof(struct KeyNode));
                                    CopyMem(name,new->kn_Node.ln_Name,len);

                                    node = (struct KeyNode *)list->lh_Head;
                                    while (node->kn_Node.ln_Succ)
                                    {
                                        if (Stricmp(node->kn_Node.ln_Name,new->kn_Node.ln_Name) >= 0)
                                            break;
                                        node = (struct KeyNode *)node->kn_Node.ln_Succ;
                                    }
                                    Insert(list,new,node->kn_Node.ln_Pred);
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
            dvp = GetDeviceProc("KEYMAPS:",dvp);
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
    strcpy(ed->ed_PrefsDefaults.ip_Keymap,"usa1");
    ed->ed_PrefsDefaults.ip_PointerTicks         = 1;
    ed->ed_PrefsDefaults.ip_DoubleClick.tv_secs  = 1;
    ed->ed_PrefsDefaults.ip_DoubleClick.tv_micro = 500000;
    ed->ed_PrefsDefaults.ip_KeyRptDelay.tv_secs  = 0;
    ed->ed_PrefsDefaults.ip_KeyRptDelay.tv_micro = 600000;
    ed->ed_PrefsDefaults.ip_KeyRptSpeed.tv_secs  = 0;
    ed->ed_PrefsDefaults.ip_KeyRptSpeed.tv_micro = 50000;
    ed->ed_PrefsDefaults.ip_MouseAccel           = 0;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;
    ed->ed_FirstClick   = TRUE;

    return(ES_NORMAL);
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
struct Node *node;

    FreeKeyMap(ed,ed->ed_Keymap);

    if (ed->ed_Cancelled)
        SetInputPrefs(ed,&ed->ed_PrefsInitial);

    while (node = RemHead(&ed->ed_AvailKeymaps))
        FreeVec(node);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID != ID_INPT || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct InputPrefs)) == sizeof(struct InputPrefs))
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
    if (!PushChunk(iff,0,ID_INPT,sizeof(struct InputPrefs)))
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct InputPrefs)) == sizeof(struct InputPrefs))
            if (!PopChunk(iff))
                return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define MIN_DOUBLECLICK	1 	/* =  .20 s (in 50th's of a second) */
#define MAX_DOUBLECLICK	200	/* = 4.00 s (in 50th's of a second) */
#define MIN_KEYDELAY	1 	/* =  .20 s (in 50th's of a second) */
#define MAX_KEYDELAY	75	/* = 1.50 s (in 50th's of a second) */
#define MIN_KEYSPEED	1	/* =    2 s (in 500th's of a second) */
#define MAX_KEYSPEED	125	/* =  .25 s (in 500th's of a second) */

/*  Conversion factors for DOS ticks: */
#define TICKS_PER_SEC	50
#define	USECS_PER_TICK	20000

/*  Conversion factors for kspeed units (1/500 sec): */
#define	KSUS_PER_SEC	500
#define	USECS_PER_KSU	2000
#define MICROS_PER_SEC	1000000


/*****************************************************************************/


#define NW_WIDTH     586
#define NW_HEIGHT    255
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
    {BUTTON_KIND,   8,   230, 87,  20, MSG_SAVE_GAD,         EC_SAVE,            0},
    {BUTTON_KIND,   244, 230, 87,  20, MSG_USE_GAD,          EC_USE,             0},
    {BUTTON_KIND,   476, 230, 87,  20, MSG_CANCEL_GAD,       EC_CANCEL,          0},

    {SLIDER_KIND,   216, 24,  180, 11, MSG_INP_MOUSESPEED_GAD,   EC_MOUSESPEED,      0},
    {CHECKBOX_KIND, 216, 36,  12,  14, MSG_INP_ACCELERATION_GAD, EC_ACCELERATION,    0},
    {SLIDER_KIND,   216, 58,  180, 11, MSG_INP_DOUBLECLICK_GAD,  EC_DOUBLECLICK,     0},
    {BUTTON_KIND,   10,  76,  207, 20, MSG_INP_SHOW_GAD,         EC_SHOWDOUBLECLICK, 0},
    {BUTTON_KIND,   10,  99,  207, 20, MSG_INP_TEST_GAD,         EC_TESTDOUBLECLICK, 0},
    {TEXT_KIND,     219, 99,  155, 20, MSG_NOTHING,	         EC_NOP,             0},

    {SLIDER_KIND,   216, 155, 156, 11, MSG_INP_KEYDELAY_GAD,     EC_KEYREPEATDELAY, 0},
    {SLIDER_KIND,   216, 171, 156, 11, MSG_INP_KEYRATE_GAD,      EC_KEYREPEATRATE,  0},
    {STRING_KIND,   187, 189, 188, 20, MSG_INP_KEYTEST_GAD,      EC_NOP,            0},

    {LISTVIEW_KIND, 405, 25,  156, 184,MSG_INP_KEYBOARDTYPE_GAD, EC_KEYBOARDTYPE,   NG_HIGHLABEL}
};

#define DCSHOW_LEFT   219
#define DCSHOW_TOP    76
#define DCSHOW_WIDTH  155
#define DCSHOW_HEIGHT 20


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    BuildList(ed,&ed->ed_AvailKeymaps,"~(#?.info)",0);

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
    DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[6],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[7],NULL,TAG_DONE);

    ed->ed_DoubleClickTest = DoPrefsGadget(ed,&EG[8],NULL,GTTX_Border,TRUE,
							  TAG_DONE);

    ed->ed_KeyTest = DoPrefsGadget(ed,&EG[11],NULL,GTST_MaxChars,100,
                                                   TAG_DONE);

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
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_INP_NAME),
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


VOID CenterLine(EdDataPtr ed, struct RastPort *rp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(rp,(w-TextLength(rp,str,len)) / 2 + window->BorderLeft + x,
            window->BorderTop+y);
    Text(rp,str,len);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y,
                                     SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
struct RastPort *rp;

    DrawBB(ed,DCSHOW_LEFT,DCSHOW_TOP,DCSHOW_WIDTH,DCSHOW_HEIGHT,
              GT_VisualInfo, ed->ed_VisualInfo,
              GTBB_Recessed, TRUE,
              TAG_DONE);

    rp = ed->ed_Window->RPort;
    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
    SetBPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    CenterLine(ed,rp,MSG_INP_MOUSE_HDR,8,20,362);
    CenterLine(ed,rp,MSG_INP_KEYBOARD_HDR,20,138,362);
}


/*****************************************************************************/


VOID SetInputPrefs(EdDataPtr ed, struct InputPrefs *ip)
{
struct Preferences prefs;

    GetPrefs(&prefs,sizeof(struct Preferences));

    prefs.PointerTicks = ip->ip_PointerTicks;
    prefs.DoubleClick  = ip->ip_DoubleClick;
    prefs.KeyRptDelay  = ip->ip_KeyRptDelay;
    prefs.KeyRptSpeed  = ip->ip_KeyRptSpeed;

    prefs.EnableCLI    &= (~MOUSE_ACCEL);
    if (ip->ip_MouseAccel)
        prefs.EnableCLI |= MOUSE_ACCEL;

    SetPrefs(&prefs,sizeof(struct Preferences),FALSE);
}


/*****************************************************************************/


struct Node *FindNameNC(EdDataPtr ed, struct List *list, STRPTR name)
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


struct KeyMapNode *GetKeyMap(EdDataPtr ed, STRPTR name)
{
char                   fileName[100];
BPTR                   segment;
struct KeyMapResource *kr;
struct KeyMapNode     *kn;

    if (!name[0])
        return(NULL);

    if (!(kr = (struct KeyMapResource *)OpenResource("keymap.resource")))
        return(FALSE);

    Forbid();
    kn = (struct KeyMapNode *)FindNameNC(ed,&kr->kr_List,name);
    Permit();

    if (!kn)
    {
        strcpy(fileName,"KEYMAPS:");
        AddPart(fileName,name,100);

        if (segment = LoadSeg(fileName))
        {
            kn = (struct KeyMapNode *)((segment << 2) + sizeof(BPTR));
            kn->kn_Node.ln_Succ = NULL;

            if (!TypeOfMem(kn->kn_Node.ln_Name) || (Stricmp(name,kn->kn_Node.ln_Name) != 0))
            {
                UnLoadSeg(segment);
                return(NULL);
            }
        }
    }

    return(kn);
}


/*****************************************************************************/


VOID FreeKeyMap(EdDataPtr ed, struct KeyMapNode *kn)
{
    if (kn && (!kn->kn_Node.ln_Succ))
        UnLoadSeg( (BPTR) (((ULONG)kn - sizeof(BPTR)) >> 2) );
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
ULONG           ticks;
UWORD           i;
struct KeyNode *node;
struct KeyMap  *km;

    ticks = 4-ed->ed_PrefsWork.ip_PointerTicks;
    if (ticks == 0)
        ticks = 1;

    ed->ed_MouseSpeed = DoPrefsGadget(ed,&EG[3],ed->ed_MouseSpeed,
                                                GTSL_Level,       ticks,
                                                GTSL_Min,         1,
                                                GTSL_Max,         3,
                                                GTSL_MaxLevelLen, 4,
                                                GTSL_LevelFormat, "%4lu",
                                                GA_RelVerify,     TRUE,
                                                GA_Immediate,     TRUE,
                                                TAG_DONE);

    ed->ed_Acceleration = DoPrefsGadget(ed,&EG[4],ed->ed_Acceleration,
                                                  GTCB_Checked,ed->ed_PrefsWork.ip_MouseAccel,
                                                  TAG_DONE);

    ed->ed_DoubleClick = DoPrefsGadget(ed,&EG[5],ed->ed_DoubleClick,
                                                 GTSL_Level,       ((ed->ed_PrefsWork.ip_DoubleClick.tv_secs * TICKS_PER_SEC) + (ed->ed_PrefsWork.ip_DoubleClick.tv_micro / USECS_PER_TICK)),
                                                 GTSL_Min,         1,
                                                 GTSL_Max,         200,
                                                 GTSL_MaxLevelLen, 4,
                                                 GTSL_LevelFormat, "%4lu",
                                                 GA_RelVerify,     TRUE,
                                                 GA_Immediate,     TRUE,
                                                 TAG_DONE);

    ed->ed_KeyDelay = DoPrefsGadget(ed,&EG[9],ed->ed_KeyDelay,
                                              GTSL_Level,       ((ed->ed_PrefsWork.ip_KeyRptDelay.tv_secs * TICKS_PER_SEC) + (ed->ed_PrefsWork.ip_KeyRptDelay.tv_micro / USECS_PER_TICK)),
                                              GTSL_Min,         1,
                                              GTSL_Max,         75,
                                              GTSL_MaxLevelLen, 4,
                                              GTSL_LevelFormat, "%4lu",
                                              GA_RelVerify,     TRUE,
                                              GA_Immediate,     TRUE,
                                              TAG_DONE);

    ed->ed_KeyRate = DoPrefsGadget(ed,&EG[10],ed->ed_KeyRate,
                                              GTSL_Level,125-((ed->ed_PrefsWork.ip_KeyRptSpeed.tv_secs * KSUS_PER_SEC) + (ed->ed_PrefsWork.ip_KeyRptSpeed.tv_micro / USECS_PER_KSU)),
				              GTSL_Min,         1,
                                              GTSL_Max,         125,
                                              GTSL_MaxLevelLen, 4,
                                              GTSL_LevelFormat, "%4lu",
                                              GA_RelVerify,     TRUE,
                                              GA_Immediate,     TRUE,
                                              TAG_DONE);

    i    = 0;
    node = (struct KeyNode *)ed->ed_AvailKeymaps.lh_Head;
    while (node->kn_Node.ln_Succ)
    {
        if (Stricmp(ed->ed_PrefsWork.ip_Keymap,node->kn_RealName) == 0)
            break;
        i++;
        node = (struct KeyNode *)node->kn_Node.ln_Succ;
    }

    if (!node->kn_Node.ln_Succ)
    {
        node = (struct KeyNode *)ed->ed_AvailKeymaps.lh_Head;
        i = 0;
    }

    ed->ed_KeyboardType = DoPrefsGadget(ed,&EG[12],ed->ed_KeyboardType,
                                                   GTLV_Labels,       &ed->ed_AvailKeymaps,
                                                   GTLV_Selected,     i,
                                                   GTLV_MakeVisible,  i,
                                                   GTLV_ShowSelected, NULL,
                                                   LAYOUTA_SPACING,   4,
                                                   GTLV_ScrollWidth,  18,
                                                   TAG_DONE);

    SetInputPrefs(ed,&ed->ed_PrefsWork);

    if (!(ed->ed_Keymap = GetKeyMap(ed,node->kn_RealName)))
    {
        ShowError1(ed,MSG_INP_ERROR_NO_KEYMAP);
    }
    else
    {
        km = &ed->ed_Keymap->kn_KeyMap;
        ((struct StringInfo *)ed->ed_KeyTest->SpecialInfo)->AltKeyMap = km;
        ed->ed_KeyTest->Activation |= GACT_ALTKEYMAP;
    }
    strcpy(ed->ed_PrefsWork.ip_Keymap,node->kn_RealName);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD            icode;
struct RastPort *rp;
BOOL             setprefs;
BOOL             actstr;
struct KeyNode  *node;
struct KeyMap   *km;
struct Gadget   *gadget;

    if (ed->ed_CurrentMsg.Class == IDCMP_MOUSEMOVE)
        return;                         

    icode     = ed->ed_CurrentMsg.Code;
    rp        = window->RPort;
    gadget    = ed->ed_CurrentMsg.IAddress;
    setprefs  = FALSE;
    actstr    = FALSE;

    switch (ec)
    {
        case EC_MOUSESPEED     : icode = 4-icode;
                                 if (icode == 3)
                                     icode = 4;
                                 ed->ed_PrefsWork.ip_PointerTicks = icode;
                                 setprefs = TRUE;
                                 break;

        case EC_ACCELERATION   : ed->ed_PrefsWork.ip_MouseAccel = (SELECTED & gadget->Flags);
                                 setprefs = TRUE;
                                 break;

        case EC_DOUBLECLICK    : ed->ed_PrefsWork.ip_DoubleClick.tv_secs  = icode / TICKS_PER_SEC;
		                 ed->ed_PrefsWork.ip_DoubleClick.tv_micro = (icode % TICKS_PER_SEC) * USECS_PER_TICK;
		                 setprefs = TRUE;
                                 break;

        case EC_SHOWDOUBLECLICK: if (!ed->ed_SampleClick)
                                 {
                                     SetAPen(rp,ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
                                     RectFill(rp,DCSHOW_LEFT+8,
                                                 DCSHOW_TOP+window->BorderTop+2,
                                                 DCSHOW_LEFT+DCSHOW_WIDTH-1,
                                                 DCSHOW_TOP+DCSHOW_HEIGHT+window->BorderTop-3);
                                     ed->ed_SampleClick   = TRUE;
                                     ed->ed_SampleSeconds = ed->ed_CurrentMsg.Seconds;
                                     ed->ed_SampleMicros  = ed->ed_CurrentMsg.Micros;
                                 }
                                 break;

        case EC_TESTDOUBLECLICK: if (ed->ed_FirstClick)
        			 {
        			     if (!ed->ed_IntuiCounter)
        			     {
                                         SetGadgetAttr(ed,ed->ed_DoubleClickTest,GTTX_Text,GetString(&ed->ed_LocaleInfo,MSG_INP_CLICKAGAIN),
                                                                                 TAG_DONE);
                                         ed->ed_ClickSeconds = ed->ed_CurrentMsg.Seconds;
                                         ed->ed_ClickMicros  = ed->ed_CurrentMsg.Micros;
                                         ed->ed_IntuiCounter = 30;
                                     }
        			 }
        			 else
                                 {
                                     if (DoubleClick(ed->ed_ClickSeconds,ed->ed_ClickMicros,
                                                     ed->ed_CurrentMsg.Seconds,ed->ed_CurrentMsg.Micros))
                                     {
                                         SetGadgetAttr(ed,ed->ed_DoubleClickTest,GTTX_Text,GetString(&ed->ed_LocaleInfo,MSG_INP_DCYES),
                                                                                 TAG_DONE);
                                     }
                                     else
                                     {
                                         SetGadgetAttr(ed,ed->ed_DoubleClickTest,GTTX_Text,GetString(&ed->ed_LocaleInfo,MSG_INP_DCNO),
                                                                                 TAG_DONE);
                                     }
                                     ed->ed_IntuiCounter = 10;
                                 }
        			 ed->ed_FirstClick = !ed->ed_FirstClick;
				 break;

        case EC_KEYREPEATDELAY : ed->ed_PrefsWork.ip_KeyRptDelay.tv_secs  = icode / TICKS_PER_SEC;
		                 ed->ed_PrefsWork.ip_KeyRptDelay.tv_micro = (icode % TICKS_PER_SEC) * USECS_PER_TICK;
		                 setprefs = TRUE;
		                 actstr   = TRUE;
                                 break;

        case EC_KEYREPEATRATE  : icode = 125-icode;
				 ed->ed_PrefsWork.ip_KeyRptSpeed.tv_secs  = icode / KSUS_PER_SEC;
		                 ed->ed_PrefsWork.ip_KeyRptSpeed.tv_micro = (icode % KSUS_PER_SEC) * USECS_PER_KSU;
		                 setprefs = TRUE;
		                 actstr   = TRUE;
                                 break;

        case EC_KEYBOARDTYPE   : ed->ed_KeyTest->Activation &= (~GACT_ALTKEYMAP);
                                 ((struct StringInfo *)ed->ed_KeyTest->SpecialInfo)->AltKeyMap = NULL;

                                 node = (struct KeyNode *)ed->ed_AvailKeymaps.lh_Head;
                                 while (icode--)
                                 {
                                     node = (struct KeyNode *)node->kn_Node.ln_Succ;
                                 }
                                 FreeKeyMap(ed,ed->ed_Keymap);

                                 if (!(ed->ed_Keymap = GetKeyMap(ed,node->kn_RealName)))
                                 {
                                     ShowError1(ed,MSG_INP_ERROR_NO_KEYMAP);
                                 }
                                 else
                                 {
                                     km = &ed->ed_Keymap->kn_KeyMap;
                                     ((struct StringInfo *)ed->ed_KeyTest->SpecialInfo)->AltKeyMap = km;
                                     ed->ed_KeyTest->Activation |= GACT_ALTKEYMAP;
                                 }
                                 strcpy(ed->ed_PrefsWork.ip_Keymap,node->kn_RealName);
                                 actstr = TRUE;
                                 break;

        case EC_TIMER          : if (ed->ed_IntuiCounter)
                                 {
                                     if (!--ed->ed_IntuiCounter)
                                     {
                                         SetGadgetAttr(ed,ed->ed_DoubleClickTest,GTTX_Text,NULL,
                                                                                 TAG_DONE);
                                         ed->ed_FirstClick = TRUE;
                                     }
                                 }

                                 if (ed->ed_SampleClick)
                                 {
                                     if (!DoubleClick(ed->ed_SampleSeconds,ed->ed_SampleMicros,
                                                      ed->ed_CurrentMsg.Seconds,ed->ed_CurrentMsg.Micros))
                                     {
                                         SetAPen(rp,0);
                                         RectFill(rp,DCSHOW_LEFT+8,
                                                     DCSHOW_TOP+window->BorderTop+2,
                                                     DCSHOW_LEFT+DCSHOW_WIDTH-1,
                                                     DCSHOW_TOP+DCSHOW_HEIGHT+window->BorderTop-3);
                                         ed->ed_SampleClick = FALSE;
                                     }
                                 }
                                 break;

        default                : break;
    }

    if (setprefs)
        SetInputPrefs(ed,&ed->ed_PrefsWork);

    if (actstr)
    {
        SetGadgetAttr(ed,ed->ed_KeyTest,GTST_String,NULL,
                                        TAG_DONE);
	ActivateGadget(ed->ed_KeyTest,window,NULL);
    }
}


/*****************************************************************************/


EdCommand GetCommand(EdDataPtr ed)
{
    if (ed->ed_CurrentMsg.Class == IDCMP_INTUITICKS)
        return(EC_TIMER);

    return(EC_NOP);
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
