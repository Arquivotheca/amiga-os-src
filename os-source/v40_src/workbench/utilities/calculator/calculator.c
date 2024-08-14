
/* includes */
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <libraries/locale.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <devices/clipboard.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/graphics_protos.h>
#include <clib/locale_protos.h>
#include <clib/iffparse_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/* application includes */
#include "texttable.h"
#include "calculator_rev.h"


/*****************************************************************************/

/* for new look menus. These are here until they get added to the intuition
 * and gadtools include files
 */

/* Pass to OpenWindowTags() */
#ifndef	WA_NewLookMenus
#define WA_NewLookMenus	(WA_Dummy + 0x30)
#endif

/* Pass to LayoutMenus() and LayoutMenuItems() */
#ifndef GTMN_NewLookMenus
#define GTMN_NewLookMenus	    GT_TagBase+67
#endif


/*****************************************************************************/


#define TEMPLATE    "PUBSCREEN,TAPE/K" VERSTAG
#define OPT_SCREEN  0
#define OPT_TAPE    1
#define OPT_COUNT   2


/*****************************************************************************/


enum CalcCommand
{
    cc_NOP,
    cc_Zero,
    cc_One,
    cc_Two,
    cc_Three,
    cc_Four,
    cc_Five,
    cc_Six,
    cc_Seven,
    cc_Eight,
    cc_Nine,
    cc_Plus,
    cc_Minus,
    cc_Times,
    cc_Divide,
    cc_Period,
    cc_Delete,
    cc_CA,
    cc_CE,
    cc_UMinus,
    cc_Equals,
    cc_Cut,
    cc_Copy,
    cc_Paste,
    cc_Activate,
    cc_ShowPaper,
    cc_Quit
};


/*****************************************************************************/


enum CalcState
{
    cs_Normal,
    cs_DivideByZero,
    cs_Overflow
};


/*****************************************************************************/


struct CalcGadget
{
    WORD              cg_Column;
    WORD              cg_Row;
    AppStringsID      cg_Label;
    enum CalcCommand  cg_Cmd;
};


/*****************************************************************************/


struct CalcMenu
{
    UBYTE             cm_Type;        /* NM_XXX from gadtools.h */
    AppStringsID      cm_Label;
    enum CalcCommand  cm_Cmd;
    UWORD	      cm_ItemFlags;
};


/*****************************************************************************/


#define MAXINPUT  13
#define MAXOUTPUT 15


/*****************************************************************************/


#define ID_FTXT MAKE_ID('F','T','X','T')
#define ID_CHRS MAKE_ID('C','H','R','S')


/*****************************************************************************/


struct CalcGadget gadgets[] =
{
    {0, 1, MSG_CALC_SEVEN,        cc_Seven},
    {1, 1, MSG_CALC_EIGHT,        cc_Eight},
    {2, 1, MSG_CALC_NINE,         cc_Nine},
    {3, 1, MSG_CALC_CLEARALL,     cc_CA},
    {4, 1, MSG_CALC_CLEARENTRY,   cc_CE},

    {0, 2, MSG_CALC_FOUR,         cc_Four},
    {1, 2, MSG_CALC_FIVE,         cc_Five},
    {2, 2, MSG_CALC_SIX,          cc_Six},
    {3, 2, MSG_CALC_TIMES,        cc_Times},
    {4, 2, MSG_CALC_DIVIDE,       cc_Divide},

    {0, 3, MSG_CALC_ONE,          cc_One},
    {1, 3, MSG_CALC_TWO,          cc_Two},
    {2, 3, MSG_CALC_THREE,        cc_Three},
    {3, 3, MSG_CALC_PLUS,         cc_Plus},
    {4, 3, MSG_CALC_MINUS,        cc_Minus},

    {0, 4, MSG_CALC_ZERO,         cc_Zero},
    {1, 4, MSG_CALC_DECIMALPOINT, cc_Period},
    {2, 4, MSG_CALC_DELETE,       cc_Delete},
    {3, 4, MSG_CALC_UMINUS,       cc_UMinus},
    {4, 4, MSG_CALC_EQUALS,       cc_Equals}
};


/*****************************************************************************/


struct CalcMenu menus[] = {
    {NM_TITLE,  MSG_CALC_PROJECT_MENU,       cc_NOP, 0},
      {NM_ITEM, MSG_CALC_PROJECT_CLEARENTRY, cc_CE, 0},
      {NM_ITEM, MSG_CALC_PROJECT_CLEARALL,   cc_CA, 0},
      {NM_ITEM, MSG_NOTHING,                 cc_NOP, 0},
      {NM_ITEM, MSG_CALC_PROJECT_QUIT,       cc_Quit, 0},

    {NM_TITLE,  MSG_CALC_EDIT_MENU,          cc_NOP, 0},
      {NM_ITEM, MSG_CALC_EDIT_CUT,           cc_Cut, 0},
      {NM_ITEM, MSG_CALC_EDIT_COPY,          cc_Copy, 0},
      {NM_ITEM, MSG_CALC_EDIT_PASTE,         cc_Paste, 0},

    {NM_TITLE,  MSG_CALC_WINDOWS_MENU,       cc_NOP, 0},
      {NM_ITEM, MSG_CALC_WINDOWS_SHOWPAPER,  cc_ShowPaper, CHECKIT | MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                 cc_NOP, 0}
};


/*****************************************************************************/


VOID EventLoop(VOID);
VOID DoAction(enum CalcCommand cmd);
VOID RefreshDisplay(VOID);
VOID Evaluate(VOID);
VOID ClearAll(VOID);
VOID ClearEntry(VOID);
VOID ClearNumber(VOID);
VOID ShowError(enum CalcState);
BOOL FormatStr(STRPTR buffer);


/*****************************************************************************/


extern struct WBStartup *WBenchMsg;
extern struct ExecBase  *SysBase;
extern struct Library   *DOSBase;
       struct Library   *IntuitionBase;
       struct Library   *GfxBase;
       struct Library   *UtilityBase;
       struct Library   *GadToolsBase;
       struct Library   *IconBase;
       struct Library   *LocaleBase;

struct VisualInfo *visualInfo;
struct Window     *wp;
struct RastPort   *rp;
struct TextFont  *font;
struct Gadget    *lastAdded;
STRPTR            screenName;

WORD gadgetHeight;
WORD gadgetWidth;
WORD gadgetHSpace;
WORD gadgetVSpace;
WORD windowWidth;
WORD windowHeight;
WORD displayLeft;
WORD displayWidth;

struct Locale  *locale  = NULL;
struct Catalog *catalog = NULL;
char            decPoint;
char            multiply;
char            divide;

UBYTE            length,dec_flag,input_flag;
enum CalcCommand oldop,newop;
char             number[MAXINPUT+2],total[MAXOUTPUT+2];
double           op1,op2,constant;

/* things used by the paper tape display */
BPTR   paperConsole = NULL;
STRPTR paperName;
BOOL   openPaper;
BOOL   printEqual = FALSE;


/*****************************************************************************/


struct Window *OpenCalcWindow(ULONG tag1, ...)
{
    return(OpenWindowTagList(NULL,(struct TagItem *) &tag1));
}


/*****************************************************************************/


struct Gadget *CreateCalcGadget(struct CalcGadget *cg)
{
struct NewGadget ng;

    ng.ng_LeftEdge   = cg->cg_Column*(gadgetWidth+gadgetHSpace)+gadgetHSpace+wp->BorderLeft;
    ng.ng_TopEdge    = cg->cg_Row*(gadgetHeight+gadgetVSpace)+gadgetVSpace+wp->BorderTop;
    ng.ng_Width      = gadgetWidth;
    ng.ng_Height     = gadgetHeight;
    ng.ng_GadgetText = GetString(cg->cg_Label);
    ng.ng_TextAttr   = wp->WScreen->Font;
    ng.ng_GadgetID   = cg->cg_Cmd;
    ng.ng_Flags      = PLACETEXT_IN;
    ng.ng_VisualInfo = visualInfo;

    if ((cg->cg_Label == MSG_CALC_DECIMALPOINT) && locale)
        ng.ng_GadgetText = locale->loc_DecimalPoint;

    return(lastAdded = CreateGadgetA(BUTTON_KIND,lastAdded,&ng,NULL));
}


/*****************************************************************************/


BOOL LayoutCalcMenus(struct Menu *menus,ULONG tag1, ...)
{
    return(LayoutMenusA(menus,visualInfo,(struct TagItem *) &tag1));
}


/*****************************************************************************/


struct Menu *CreateCalcMenus(struct CalcMenu *cm)
{
UWORD           i;
struct NewMenu *nm;
struct Menu    *menus;

    i = 0;
    while (cm[i++].cm_Type != NM_END) {}

    if (!(nm = AllocVec(sizeof(struct NewMenu)*i,MEMF_CLEAR|MEMF_PUBLIC)))
        return(NULL);

    while (i--)
    {
        nm[i].nm_Type     = cm[i].cm_Type;
        nm[i].nm_Flags    = cm[i].cm_ItemFlags;
        nm[i].nm_UserData = (APTR)cm[i].cm_Cmd;

        if (cm[i].cm_Type == NM_TITLE)
        {
            nm[i].nm_Label   = GetString(cm[i].cm_Label);
        }
        else if (cm[i].cm_Label == MSG_NOTHING)
        {
            nm[i].nm_Label   = NM_BARLABEL;
        }
        else if (cm[i].cm_Type != NM_END)
        {
            nm[i].nm_CommKey = GetString(cm[i].cm_Label);
            nm[i].nm_Label   = &nm[i].nm_CommKey[2];
            if (nm[i].nm_CommKey[0] == ' ')
            {
                nm[i].nm_CommKey = NULL;
            }
        }
    }

    if (menus = CreateMenusA(nm,NULL))
    {
        if (!(LayoutCalcMenus(menus,GTMN_NewLookMenus,TRUE,
                                    TAG_DONE)))
        {
            FreeMenus(menus);
	    menus = NULL;
	}
    }

    FreeVec(nm);

    return(menus);
}


/*****************************************************************************/


VOID DrawCalcBevelBox(struct RastPort *rp,SHORT x, SHORT y,
                                          SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(rp,x+wp->BorderLeft,
                     y+wp->BorderTop,
                     w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


struct MenuItem *FindMenuItem(struct Menu *menu, enum CalcCommand cmd)
{
struct MenuItem *menuItem;

    while (menu)
    {
        menuItem = menu->FirstItem;
        while (menuItem)
        {
            if ((enum CalcCommand)MENU_USERDATA(menuItem) == cmd)
                return(menuItem);
            menuItem = menuItem->NextItem;
        }
        menu = menu->NextMenu;
    }
    return(NULL);
}


/*****************************************************************************/


VOID CalcSizes(VOID)
{
UWORD             len,max,lmax;
AppStringsID      id;
struct RastPort   rp;
STRPTR            str;

    InitRastPort(&rp);
    SetFont(&rp,font);

    max = 0;
    for (id = MSG_CALC_ZERO; id <= MSG_CALC_UMINUS; id++)
    {
        if ((id == MSG_CALC_DECIMALPOINT) && locale)
            str = locale->loc_DecimalPoint;
        else
            str = GetString(id);
        len = TextLength(&rp,str,strlen(str));
        if (len > max)
            max = len;
    }

    lmax = TextLength(&rp,GetString(MSG_CALC_TITLE),strlen(GetString(MSG_CALC_TITLE)))+58;

    str = GetString(MSG_CALC_ERROR_ZERODIVIDE);
    len = TextLength(&rp,str,strlen(str));
    if (len > lmax)
        lmax = len;

    str = GetString(MSG_CALC_ERROR_OVERFLOW);
    len = TextLength(&rp,str,strlen(str));
    if (len > lmax)
        lmax = len;

    gadgetHSpace = 3;
    gadgetVSpace = 3;
    gadgetWidth  = max+10;
    gadgetHeight = font->tf_YSize+3;

    for (;;)
    {
        windowWidth  = (gadgetWidth+gadgetHSpace)*5+gadgetHSpace;
        windowHeight = (gadgetHeight+gadgetVSpace)*5+gadgetVSpace;
        displayLeft  = gadgetHSpace;
        displayWidth = windowWidth-gadgetHSpace*2;

        if (lmax+8 < displayWidth)
            break;

        gadgetHSpace++;
    }
}


/*****************************************************************************/


VOID DoWork(VOID)
{
struct Screen   *screen;
WORD             zoomBox[4];
UWORD            i;
struct Gadget   *gadgetList = NULL;
struct DrawInfo *di;
struct Menu     *strip;

    if (paperName)
        openPaper = TRUE;
    else
        openPaper = FALSE;

    if (!(screen = LockPubScreen(screenName)))
        screen = LockPubScreen(NULL);

    if (screen)
    {
        if (visualInfo = GetVisualInfoA(screen,NULL))
        {
            if (font = OpenFont(screen->Font))
            {
                CalcSizes();
                zoomBox[0] = screen->MouseX;
                zoomBox[1] = screen->MouseY;
                zoomBox[2] = windowWidth+screen->WBorLeft+screen->WBorRight;
                zoomBox[3] = screen->WBorTop + (screen->Font->ta_YSize + 1);

                if (wp = OpenCalcWindow(WA_Left,        zoomBox[0],
                                        WA_Top,         zoomBox[1],
                                        WA_InnerWidth,  windowWidth,
                                        WA_InnerHeight, windowHeight,
                                        WA_IDCMP,       IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_REFRESHWINDOW | IDCMP_VANILLAKEY | IDCMP_MENUPICK,
                                        WA_Flags,       WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIMPLE_REFRESH,
                                        WA_Title,       GetString(MSG_CALC_TITLE),
                                        WA_PubScreen,   screen,
                                        WA_MinWidth,    zoomBox[2],
                                        WA_MinHeight,   zoomBox[3],
                                        WA_Zoom,        zoomBox,
                                        WA_NewLookMenus,TRUE,
                                        TAG_DONE))
                {
                    rp = wp->RPort;

                    lastAdded = CreateContext(&gadgetList);
                    for (i=0; i<20; i++)
                        CreateCalcGadget(&gadgets[i]);

                    if (lastAdded)
                    {
                        if (strip = CreateCalcMenus(menus))
                        {
                            SetMenuStrip(wp,strip);
                            SetFont(rp,font);
                            SetDrMd(rp,JAM2);
                            SetBPen(rp,0);
                            if (di = GetScreenDrawInfo(screen))
                            {
                                SetAPen(rp,di->dri_Pens[TEXTPEN]);
                                FreeScreenDrawInfo(screen,di);
                            }

                            AddGList(wp,gadgetList,-1,-1,NULL);
                            RefreshGList(gadgetList,wp,NULL,-1);
                            GT_RefreshWindow(wp,NULL);

                            DoAction(cc_CA);
                            if (openPaper)
                            {
                                FindMenuItem(strip,cc_ShowPaper)->Flags |= CHECKED;
                                DoAction(cc_ShowPaper);
                            }

                            RefreshDisplay();
                            EventLoop();

                            ClearMenuStrip(wp);
                            FreeMenus(strip);
                        }
                    }
                    CloseWindow(wp);
                    FreeGadgets(gadgetList);
                }
                CloseFont(font);
            }
            FreeVisualInfo(visualInfo);
        }
        UnlockPubScreen(NULL,screen);
    }

    if (paperConsole)
    {
        Write(paperConsole,"\033[ p",4); /* turn on cursor */
        Close(paperConsole);
    }
}


/*****************************************************************************/


LONG main(VOID)
{
LONG               failureLevel = RETURN_FAIL;
LONG               opts[OPT_COUNT];
struct WBArg      *wbarg;
struct DiskObject *diskObj;
BPTR               oldLock;
struct RDArgs     *rdargs;

    if (IntuitionBase = OpenLibrary("intuition.library",37))
    {
        if (GfxBase = OpenLibrary("graphics.library",37))
        {
            if (GadToolsBase = OpenLibrary("gadtools.library",37))
            {
                if (UtilityBase = OpenLibrary("utility.library",37))
                {
                    if (LocaleBase = OpenLibrary("locale.library",38))
                    {
                        locale = OpenLocale(NULL);
                        catalog = OpenCatalogA(locale,"sys/utilities.catalog",NULL);
                    }

                    decPoint = GetString(MSG_CALC_DECIMALPOINT)[0];
                    multiply = GetString(MSG_CALC_TIMES)[0];
                    divide   = GetString(MSG_CALC_DIVIDE)[0];

                    if (WBenchMsg)
                    {
                        if (IconBase = OpenLibrary("icon.library",37))
                        {
                            wbarg = WBenchMsg->sm_ArgList;

                            oldLock = CurrentDir(wbarg->wa_Lock);
                            if (diskObj = GetDiskObject(wbarg->wa_Name))
                            {
                                paperName  = FindToolType(diskObj->do_ToolTypes,"TAPE");
                                screenName = FindToolType(diskObj->do_ToolTypes,"PUBSCREEN");
                                DoWork();
                                failureLevel = RETURN_OK;
                                FreeDiskObject(diskObj);
                            }
                            CurrentDir(oldLock);
                            CloseLibrary(IconBase);
                        }
                    }
                    else
                    {
                        memset(opts,0,sizeof(opts));
                        if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                        {
                            failureLevel = RETURN_OK;
                            paperName  = (STRPTR)opts[OPT_TAPE];
                            screenName = (STRPTR)opts[OPT_SCREEN];
                            DoWork();
                            FreeArgs(rdargs);
                        }
                        else
                        {
                            PrintFault(IoErr(),NULL);
                        }
                    }

                    if (LocaleBase)
                    {
                        CloseCatalog(catalog);
                        CloseLocale(locale);
                        CloseLibrary(LocaleBase);
                    }

                    CloseLibrary(UtilityBase);
                }
                CloseLibrary(GadToolsBase);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


enum CalcCommand DoChar(char ch)
{
enum CalcCommand cmd;

    cmd = cc_NOP;

    if (ch >= '0' && ch <= '9')                                cmd = cc_Zero + ch - '0';
    else if (ch == '+')                                        cmd = cc_Plus;
    else if (ch == '-')                                        cmd = cc_Minus;
    else if (ch == '*' || ch == '×' || ch == 'x' || ch == 'X' || ch == multiply) cmd = cc_Times;
    else if (ch == '/' || ch == '÷' || ch == divide)           cmd = cc_Divide;
    else if (ch == '=' || ch == 13)                            cmd = cc_Equals;
    else if (ch == decPoint || ch == '.')                      cmd = cc_Period;
    else if (ch == 's' || ch == 'S' || ch == '±')              cmd = cc_UMinus;
    else if (ch == 8 || ch == '«')                             cmd = cc_Delete;
    else if (ch == 'a' || ch == 'A' || ch == 127)              cmd = cc_CA;
    else if (ch == 'e' || ch == 'E')                           cmd = cc_CE;
    else if (ch == 3 || ch == 27)                              cmd = cc_Quit;
    else if (ch == 6)                                          cmd = cc_Activate;

    return(cmd);
}


/*****************************************************************************/


VOID EventLoop(VOID)
{
struct IntuiMessage *intuiMsg;
ULONG                class;
UWORD		     icode;
struct Gadget       *gad;
ULONG                sigs;
UWORD		     menuNum;
struct MenuItem     *menuItem;

    while (TRUE)
    {
	sigs = Wait((1 << wp->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

        if (sigs & SIGBREAKF_CTRL_C)
            return;

        if (sigs & SIGBREAKF_CTRL_F)
            DoAction(cc_Activate);

        while (intuiMsg = (struct IntuiMessage *)GetMsg(wp->UserPort))
        {
	    class = intuiMsg->Class;
	    icode = intuiMsg->Code;

	    if (class == IDCMP_GADGETUP)
            {
                gad = (struct Gadget *) intuiMsg->IAddress;
                DoAction((enum CalcCommand)gad->GadgetID);
            }
            else if (class == IDCMP_VANILLAKEY)
            {
                DoAction(DoChar((char)icode));
            }
            else if (class == IDCMP_REFRESHWINDOW)
            {
                GT_BeginRefresh(wp);
                RefreshDisplay();
                GT_EndRefresh(wp,TRUE);
            }
            else if (class == IDCMP_MENUPICK)
            {
		menuNum = intuiMsg->Code;
                while (menuNum != MENUNULL)
                {
                    menuItem = ItemAddress(intuiMsg->IDCMPWindow->MenuStrip,menuNum);
                    DoAction((enum CalcCommand)MENU_USERDATA(menuItem));
                    menuNum = menuItem->NextSelect;
                }
            }
	    else if (class == IDCMP_CLOSEWINDOW)
	    {
                return;
            }

	    ReplyMsg(intuiMsg);
        }
    }
}


/*****************************************************************************/


ULONG header[] = {ID_FORM,
                  0,          /* form length  */
                  ID_FTXT,
                  ID_CHRS,
                  0};	      /* chunk length */

VOID Clip(VOID)
{
struct IOClipReq ioclip;
ULONG            len,pad;
char             buffer[50];

    if (FormatStr(buffer))
    {
        ioclip.io_Message.mn_Node.ln_Type = NT_MESSAGE;
        ioclip.io_Message.mn_Node.ln_Pri  = 0;
        ioclip.io_Message.mn_Length       = sizeof(ioclip);
        ioclip.io_Message.mn_ReplyPort    = &((struct Process *)SysBase->ThisTask)->pr_MsgPort;

        if (OpenDevice("clipboard.device",0,&ioclip,0))
          return;

        ioclip.io_Offset = 0;
        ioclip.io_ClipID = 0;
        ioclip.io_Error  = 0;

        /* write header */
        ioclip.io_Command = CMD_WRITE;
        ioclip.io_Data    = (APTR)header;
        ioclip.io_Length  = sizeof(header);
        DoIO(&ioclip);

        len = strlen(buffer);
        pad = len & 1;

        /* write data */
        ioclip.io_Command = CMD_WRITE;
        ioclip.io_Data    = (APTR)buffer;
        ioclip.io_Length  = len+pad;
        DoIO(&ioclip);

        /* correct the chunk length in the header */
        ioclip.io_Command = CMD_WRITE;
        ioclip.io_Data    = (APTR)&len;
        ioclip.io_Length  = 4;
        ioclip.io_Offset  = 16;
        DoIO(&ioclip);

        /* correct the form length in the header */
        len += 12 + pad;
        ioclip.io_Command = CMD_WRITE;
        ioclip.io_Data    = (APTR)&len;
        ioclip.io_Length  = 4;
        ioclip.io_Offset  = 4;
        DoIO(&ioclip);

        ioclip.io_Command = CMD_UPDATE;
        DoIO(&ioclip);

        CloseDevice(&ioclip);
    }
}


/*****************************************************************************/


VOID Paste(VOID)
{
struct Library     *IFFParseBase;
struct IFFHandle   *iff;
LONG                error;
struct ContextNode *cn;
char                ch;

    if (IFFParseBase = OpenLibrary("iffparse.library",37))
    {
	if (iff = AllocIFF())
	{
	    if (iff->iff_Stream = (ULONG) OpenClipboard(0))
	    {
                InitIFFasClip(iff);

	        if (!OpenIFF(iff,IFFF_READ))
		{
	            if (!StopChunk(iff,ID_FTXT,ID_CHRS))
                    {
	                while (TRUE)
                        {
                            error = ParseIFF(iff,IFFPARSE_SCAN);
                            if (error == IFFERR_EOC) continue;
                            else if (error) break;

                            if (cn = CurrentChunk(iff))
                            {
                                while (ReadChunkBytes(iff,&ch,1) == 1)
                                    DoAction(DoChar(ch));
		            }
		        }
                    }
                    CloseIFF(iff);
                }
		CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
            }
	    FreeIFF(iff);
	}
	CloseLibrary(IFFParseBase);
    }
}


/*****************************************************************************/


VOID PutCh(char ch)
{
char ch1;

    ch1 = ch;
    if (paperConsole)
        Write(paperConsole,&ch1,1);
}


/*****************************************************************************/


VOID PutNumber(enum CalcCommand op)
{
UWORD i;
char  buffer[50];
BOOL  tab;

    tab = TRUE;
    if (op == cc_Equals)
    {
        if (printEqual)
            PutCh('=');
    }
    else if (op == cc_Plus)
        PutCh('+');
    else if (op == cc_Minus)
        PutCh('-');
    else if (op == cc_Times)
        PutCh(multiply);
    else if (op == cc_Divide)
        PutCh(divide);
    else
        tab = FALSE;

    if (tab)
        PutCh('\t');

    FormatStr(buffer);
    i = 0;
    while (buffer[i])
    {
        PutCh(buffer[i]);
        i++;
    }
    PutCh('\n');
    Write(paperConsole,"\033[0 p",5); /* turn off cursor */
    printEqual = TRUE;
}


/*****************************************************************************/


VOID DoAction(enum CalcCommand cmd)
{
UBYTE            numlen;
BOOL             update;
struct MenuItem *menuItem;
struct Menu     *menus;
char             name[100];
STRPTR           conName;

    update = TRUE;
    switch (cmd)
    {
        case cc_Zero     : /* add 0 but prevent leading 0's */
                           numlen = dec_flag ? length-1 : length;
                           if (numlen<MAXINPUT && number[1] != '0' || !input_flag)
                           {
                               if (input_flag)
                                   ++length;
                               number[length] = '0';
                               input_flag=TRUE;
                               if (newop == cc_Equals)
                               {
                                   printEqual = FALSE;
                                   oldop=cc_Equals;
                               }
                           }
                           break;

        case cc_One      :
        case cc_Two      :
        case cc_Three    :
        case cc_Four     :
        case cc_Five     :
        case cc_Six      :
        case cc_Seven    :
        case cc_Eight    :
        case cc_Nine     : numlen = dec_flag ? length-1 : length;
                           if (numlen < MAXINPUT)      /* if room */
                           {
                               if (number[1] != '0')
                                   ++length; /* bump length if # not 0 */
                               number[length] = cmd - cc_Zero + '0';
                               input_flag=TRUE;
                               if (newop == cc_Equals)
                               {
                                   oldop = cc_Equals;
                                   printEqual = FALSE;
                               }
                           }
                           break;

        case cc_Plus     :
        case cc_Minus    :
        case cc_Times    :
        case cc_Divide   : if (input_flag)
                           {
                               PutNumber(newop);
			       newop = cmd;
                               op2 = op1;
                               op1 = atof(number);
                               constant = op1;
                               ClearNumber();
                               Evaluate();
                               oldop = newop;
                           }
                           else
                               oldop = newop = cmd;
                           break;

        case cc_Period   : if (!dec_flag)
                           {
                               if (number[1] != '0')
                                   ++length; /* bump length if not leading 0 */
                               number[length] = '.';   /* add decimal point */
                               dec_flag = TRUE;        /* flag that this # has a dp */
                               input_flag = TRUE;
                               if (newop == cc_Equals)
                               {
                                   oldop = cc_Equals;
                                   printEqual = FALSE;
                               }
                           }
                           break;

        case cc_Delete   : if (input_flag)
                           {
                               if (number[length] == '.')
                                   dec_flag = FALSE;
                               number[length--] = ' ';
                               if (!length)
                                   ClearEntry();
                           }
                           break;

        case cc_Cut      : Clip();
                           ClearAll();
                           break;

        case cc_CA       : ClearAll();
                           PutCh('\n');
                           printEqual = FALSE;
                           break;

        case cc_CE       : ClearEntry();
                           break;

        case cc_UMinus   : if (input_flag) /* if input # valid */
                               number[0] = (number[0] == ' ') ? '-' : ' ';
                           else
                               op1 = -op1;
                           PutCh((char)'±');
                           PutCh('\n');
                           break;

        case cc_Equals   : newop = cmd;
                           op2 = op1;
                           if (input_flag)
                           {
                               PutNumber(oldop);
		               op1 = atof(number);
                               constant=op1;
                           }
                           else
                               op1 = constant;
                           ClearNumber();
                           Evaluate();
                           break;

        case cc_Copy     : Clip();
                           break;

        case cc_Paste    : Paste();
                           break;

        case cc_Activate : WindowToFront(wp);
                           ActivateWindow(wp);
                           if (wp->Flags & ZOOMED)
                               ZipWindow(wp);
                           break;

        case cc_ShowPaper: menus    = wp->MenuStrip;
			   menuItem = FindMenuItem(menus,cc_ShowPaper);
                           ClearMenuStrip(wp);

                           if ((CHECKED & menuItem->Flags) && (!paperConsole))
                           {
                               if (!(conName = paperName))
                               {
                                   if (!screenName)
                                       screenName = "";

                                   if (wp->LeftEdge >= wp->Width)
                                       sprintf(name,GetString(MSG_CALC_TAPE_SPEC),wp->LeftEdge-wp->Width,wp->TopEdge,wp->Width,wp->Height,screenName);
                                   else
                                       sprintf(name,GetString(MSG_CALC_TAPE_SPEC),wp->LeftEdge+wp->Width,wp->TopEdge,wp->Width,wp->Height,screenName);

                                   conName = name;
                               }

                               if (paperConsole = Open(conName,MODE_NEWFILE))
                               {
                                   Write(paperConsole,"\033[0 p",5); /* turn off cursor */
                               }
                               else
                               {
                                   DisplayBeep(wp->WScreen);
                                   menuItem->Flags &= (~CHECKED);
                               }
                           }
                           else if ((!(CHECKED & menuItem->Flags)) & (paperConsole))
                           {
                               Close(paperConsole);
                               paperConsole = NULL;
                           }
                           ResetMenuStrip(wp,menus);
                           break;

        case cc_Quit     : Signal(SysBase->ThisTask,SIGBREAKF_CTRL_C);
                           break;

        default          : update = FALSE;
		           break;
    }

    if (update)
        RefreshDisplay();

    if (cmd == cc_Equals)
        PutNumber(cc_Equals);
}


/*****************************************************************************/


BOOL DoPrecision(VOID) /* determine how to display number */
{
STRPTR str;
UBYTE  i,j;
LONG   dec,sign;
double top1;

    top1 = op1;
    if (top1 < 0.0)
	top1 = -top1;

    if ((top1 + 5.0e-2) > 1.0e13)
        return(FALSE);

    str = ecvt(op1,13,&dec,&sign);

    if (sign)
        total[0] = '-';
    else
        total[0] = ' ';

    i = 0;
    if (dec <= 0)
    {
        total[1] = '0';
        total[2] = decPoint;
        j = 3;
        while ((dec < 0) && (j <= MAXOUTPUT))
        {
            total[j++] = '0';
            dec++;
        }

        while (str[i] && (j <= MAXOUTPUT))
            total[j++] = str[i++];
    }
    else
    {
        j = 1;
        while (str[i])
        {
            total[j++] = str[i++];
            if (dec == i)
                total[j++] = decPoint;
        }
    }

    total[j] = 0;

    i = j-1;
    while (total[i] == '0')
        total[i--] = 0;

    if (total[i] == decPoint)
    {
        total[i+1] = '0';
        total[i+2] = 0;
    }

    return(TRUE);
}


/*****************************************************************************/


BOOL FormatStr(STRPTR buffer)
{
UWORD i,j,k;

    if (input_flag)             /* if input # valid */
    {
        j = 0;
        for (i = 0; i <= length; i++)
            buffer[j++] = number[i];
    }
    else
    {
        if (DoPrecision())       /* format # */
        {
            k = strlen(total);
            j = 0;
            for (i=0; i <= k; i++)
                buffer[j++] = total[i];
        }
        else
        {
            ShowError(cs_Overflow);
            return(FALSE);
        }
    }

    for (i = 0; i <= length; i++)
    {
        if (buffer[i] == '.')
            buffer[i] = decPoint;
    }

    buffer[j] = 0;
    return(TRUE);
}


/*****************************************************************************/


VOID RefreshDisplay(VOID)
{
UBYTE buffer[50];
UBYTE apen;
UWORD len;

    if (!(wp->Flags & ZOOMED))
    {
        if (FormatStr(buffer))
        {
            len = TextLength(rp,buffer,strlen(buffer));
            apen = rp->FgPen;
            SetAPen(rp,0);
            RectFill(rp,displayLeft+wp->BorderLeft+2,
                        gadgetVSpace+wp->BorderTop+1,
                        displayLeft+wp->BorderLeft+displayWidth-len,
                        gadgetVSpace+wp->BorderTop+gadgetHeight-2);
            SetAPen(rp,apen);

            Move(rp,displayWidth-len+displayLeft,
                    gadgetVSpace+wp->BorderTop+font->tf_Baseline+2);
            Text(rp,buffer,strlen(buffer));

            DrawCalcBevelBox(rp,displayLeft,gadgetVSpace,displayWidth,gadgetHeight,
                             GT_VisualInfo,visualInfo,
                             GTBB_Recessed, TRUE,
                             TAG_DONE);
        }
    }
}


/*****************************************************************************/


VOID Evaluate()
{
    switch(oldop)
    {
	case cc_Plus  : op1 = op2 + op1;
                        break;

	case cc_Minus : op1 = op2 - op1;
	                break;

	case cc_Times : op1 = op2 * op1;
	                break;

	case cc_Divide: if (op1 == 0.0)
		            ShowError(cs_DivideByZero);
	                else
                            op1 = op2 / op1;

	default       : break;
    }
}


/*****************************************************************************/


VOID ClearEntry()
{
UWORD i;

    for (i=0; i <= MAXINPUT; i++)
        number[i]=' ';

    number[i]  = 0;
    length     = 1;	  /* set length for ' 0' */
    number[1]  = '0';	  /* set # to 0 */
    input_flag = TRUE;	  /* we are inputting */
    dec_flag   = FALSE;	  /* this # has no dp yet */
}


/*****************************************************************************/


VOID ClearNumber(VOID)
{
    ClearEntry();
    input_flag=FALSE;
}


/*****************************************************************************/


VOID ClearAll(VOID)
{
    op1   = op2   = constant = 0.0;
    oldop = newop = cc_Equals;
    ClearEntry();
}


/*****************************************************************************/


VOID ShowError(enum CalcState cs)
{
ULONG sigs;

    if (cs == cs_DivideByZero)
        strcpy(number,GetString(MSG_CALC_ERROR_ZERODIVIDE));

    else if (cs == cs_Overflow)
        strcpy(number,GetString(MSG_CALC_ERROR_OVERFLOW));

    length = strlen(number) - 1;
    input_flag = TRUE;
    RefreshDisplay();
    ClearAll();

    sigs = Wait((1 << wp->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);
    SetSignal(sigs,sigs);
}
