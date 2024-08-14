
/* BUG: If ACTION_FORMAT is not recognized by a handler, this program
 *      assumes a 512-byte block size
 */

/* includes */
#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <dos/dosasl.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <devices/trackdisk.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <internal/crossdos.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/locale_protos.h>
#include <clib/diskfont_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

/* application includes */
#include "texttable.h"
#include "format_rev.h"


/****************************************************************************/


/* because <dos/dos.h> has these incorrectly defined in 2.04 */
#ifdef ID_INTER_DOS_DISK
#undef ID_INTER_DOS_DISK
#endif

#ifdef ID_INTER_FFS_DISK
#undef ID_INTER_FFS_DISK
#endif

#define WORDSPERBLOCK	  128
#define HASHSIZE	  (WORDSPERBLOCK - 56)
#define NUMBMPAGES	  26
#define T_SHORT	          2
#define ST_ROOT	          1
#define ID_BAD_DISK       (0x42414400L) /* 'BAD\0' */
#define ID_BUSY_DISK      (0x42555359L) /* 'BUSY'  */
#define ID_INTER_DOS_DISK (0x444F5302L)	/* 'DOS\2' */
#define ID_INTER_FFS_DISK (0x444F5303L)	/* 'DOS\3' */
#define ID_DC_DOS_DISK    (0x444F5304L) /* 'DOS\4' */
#define ID_DC_FFS_DISK    (0x444F5305L) /* 'DOS\5' */

struct RootBlock
{
    ULONG            rb_Type;
    ULONG            rb_HeaderKey;
    ULONG            rb_HighSeqNum;
    ULONG            rb_HashSize;
    ULONG            rb_Reserved;
    ULONG            rb_CheckSum;
    ULONG            rb_HashTable[HASHSIZE];
    ULONG            rb_BMFlag;
    ULONG            rb_BitMapPages[NUMBMPAGES];
    struct DateStamp rb_LastDate;
    UBYTE            rb_DiskName[52];
    struct DateStamp rb_CreateDate;
    ULONG            rb_HashChain;
    ULONG            rb_Parent;
    ULONG            rb_Extension;
    ULONG            rb_SecondaryType;
};


/****************************************************************************/


struct TextAttr topazAttr =
{
    "coral.font",
     16,
     FS_NORMAL,
     FPF_DISKFONT
};


/****************************************************************************/


#define TEMPLATE       "DEVICE=DRIVE/K/A,NAME/K/A,OFS/S,FFS/S,INTL=INTERNATIONAL/S,NOINTL=NOINTERNATIONAL/S,DIRCACHE/S,NODIRCACHE/S,NOICONS/S,QUICK/S" VERSTAG
#define OPT_DEVICE     0
#define OPT_NAME       1
#define OPT_OFS        2
#define OPT_FFS	       3
#define OPT_INTL       4
#define OPT_NOINTL     5
#define OPT_DIRCACHE   6
#define OPT_NODIRCACHE 7
#define OPT_NOICONS    8
#define OPT_QUICK      9
#define OPT_COUNT      10


/****************************************************************************/


#define MAXERRORS      2
#define BIGDRIVESIZE   (1024*1024*2)   /* 2 Megs */
#define NAMELENGTH     30

#define ERROR_BAD_DISK 1
#define ERROR_NO_TRASH 2


/****************************************************************************/


struct GlobalData
{
    APTR               gd_DOSBase;
    APTR               gd_IntuitionBase;
    APTR               gd_GadToolsBase;
    APTR               gd_UtilityBase;
    APTR               gd_GfxBase;
    APTR               gd_DiskfontBase;
    struct LocaleInfo  gd_LocaleInfo;

    struct Screen     *gd_Screen;
    struct Window     *gd_Window;
    APTR               gd_VisualInfo;
    struct DrawInfo   *gd_DrawInfo;
    struct TextFont   *gd_Font;
    ULONG              gd_IntuiSig;
    struct Gadget     *gd_LastAdded;
    struct Gadget     *gd_Gadgets;
    char	       gd_WindowTitle[80];

    char               gd_OldVolumeName[NAMELENGTH+1];
    char               gd_DeviceName[NAMELENGTH+1];
    char               gd_VolumeName[NAMELENGTH+1];
    BOOL               gd_Trashcan;
    BOOL               gd_Quick;
    BOOL               gd_OFS;
    BOOL               gd_FFS;
    BOOL               gd_Intl;
    BOOL               gd_NoIntl;
    BOOL	       gd_DirCache;
    BOOL	       gd_NoDirCache;
    BOOL               gd_GUI;
    WORD	       gd_Pad;
    struct DosList    *gd_DeviceNode;

    ULONG              gd_BytesPerBlock;
    ULONG              gd_NumBlocks;
    ULONG              gd_BlocksUsed;
    ULONG              gd_DiskType;
    char               gd_Capacity[80];

    ULONG              gd_NumberCyl;
    ULONG              gd_CurrentCyl;
    ULONG              gd_LowCyl;
};

#define DOSBase       global->gd_DOSBase
#define IntuitionBase global->gd_IntuitionBase
#define GadToolsBase  global->gd_GadToolsBase
#define UtilityBase   global->gd_UtilityBase
#define GfxBase       global->gd_GfxBase
#define DiskfontBase  global->gd_DiskfontBase
#define LocaleBase    global->gd_LocaleInfo.li_LocaleBase


/****************************************************************************/


#define BTOCSTR(bstr)  ((STRPTR)((ULONG)BADDR(bstr) + 1))
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);

#define GetStr(str)   (GetString(&global->gd_LocaleInfo,str))
#define CheckBreak()  (CheckSignal(global->gd_IntuiSig | SIGBREAKF_CTRL_C))
#define PrintErr(err) (PrintFault(err,GetStr(MSG_FMT_FAILED_TITLE)))

LONG Initialize(struct GlobalData *global);
VOID WBExplain(struct GlobalData *global);
BOOL ReallyFormat(struct GlobalData *global);
LONG GetFSInfo(struct GlobalData *global);
BOOL FindVolumeDevice(struct GlobalData *global, struct MsgPort *volumeHandler);

VOID VPrintF(struct GlobalData *global, STRPTR str, STRPTR arg1, ... );
VOID PrintF(struct GlobalData *global, AppStringsID str, STRPTR arg1, ... );
VOID kprintf(STRPTR,...);

BOOL OpenFmtWindow(struct GlobalData *global);
BOOL EditFmtOptions(struct GlobalData *global);
VOID CloseFmtWindow(struct GlobalData *global);
BOOL SelectFmtDevice(struct GlobalData *global);


/****************************************************************************/


#define GAUGE_LEFT     8
#define GAUGE_RIGHT    416
#define GAUGE_TOP      28
#define GAUGE_BOTTOM   69
#define GAUGE_WIDTH    (GAUGE_RIGHT-GAUGE_LEFT+1)
#define GAUGE_HEIGHT   (GAUGE_BOTTOM-GAUGE_TOP+1)


/****************************************************************************/


LONG main(VOID)
{
struct InfoData __aligned  infoData;
struct GlobalData          gd;
struct GlobalData         *global;
struct Library            *SysBase = (*((struct Library **) 4));
struct Process            *process;
struct WBStartup          *WBenchMsg = NULL;
struct WBArg              *wbarg;
LONG                       wbcnt;
struct RdArgs             *rdargs;
LONG                       opts[OPT_COUNT];
STRPTR                     volName;
STRPTR                     devName;
LONG                       failureCode  = ERROR_INVALID_RESIDENT_LIBRARY;
UWORD                      i;
BPTR                       parent;
char                       ch;
BOOL                       abort;
BOOL			   printFailure;
char                       str[80];

    memset(&gd,0,sizeof(struct GlobalData));
    global = &gd;

    process = (struct Process *)FindTask(NULL);
    if (!process->pr_CLI)
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *)GetMsg(&process->pr_MsgPort);
    }

    DOSBase        = OpenLibrary("dos.library",39);
    IntuitionBase  = OpenLibrary("intuition.library",39);
    GfxBase        = OpenLibrary("graphics.library",39);
    UtilityBase    = OpenLibrary("utility.library",39);
    GadToolsBase   = OpenLibrary("gadtools.library",39);
    DiskfontBase   = OpenLibrary("diskfont.library",39);
    global->gd_GUI = (process->pr_CLI == NULL);

    if (DOSBase && IntuitionBase && GfxBase && UtilityBase && GadToolsBase && DiskfontBase)
    {
        if (LocaleBase = OpenLibrary("locale.library",38))
            global->gd_LocaleInfo.li_Catalog = OpenCatalogA(NULL,"sys/system.catalog",NULL);

        if (WBenchMsg)
        {
            strcpy(global->gd_VolumeName,GetStr(MSG_FMT_EMPTY_DISK));
            global->gd_Quick      = FALSE;
            global->gd_Trashcan   = TRUE;
            global->gd_OFS        = FALSE;
            global->gd_FFS        = FALSE;
            global->gd_Intl       = FALSE;
            global->gd_NoIntl     = FALSE;
            global->gd_DirCache   = FALSE;
            global->gd_NoDirCache = FALSE;

            /* We might want to read the Format program icon to find tooltypes
             * describing the default options instead of always initing to
             * fixed defaults.
             */

            failureCode  = ERROR_INVALID_COMPONENT_NAME;
            printFailure = TRUE;
            wbarg        = WBenchMsg->sm_ArgList;
            wbcnt        = WBenchMsg->sm_NumArgs;

            if (wbcnt == 1)   /* double-click of Format icon */
            {
                failureCode = ERROR_NO_FREE_STORE;
                if (OpenFmtWindow(global))
                {
                    failureCode = 0;
                    if (SelectFmtDevice(global))
                    {
                        global->gd_OFS        = FALSE;
                        global->gd_FFS        = FALSE;
                        global->gd_Intl       = FALSE;
                        global->gd_NoIntl     = FALSE;
                        global->gd_DirCache   = FALSE;
                        global->gd_NoDirCache = FALSE;
                        if (!(failureCode = GetFSInfo(global)))
                            if (EditFmtOptions(global))
                                if (ReallyFormat(global))
                                    if (failureCode = Initialize(global))
                                        printFailure = FALSE;
                    }
                }
                CloseFmtWindow(global);
            }
            else if (wbcnt > 1)
            {
                UnLock(wbarg->wa_Lock);
                wbarg->wa_Lock = NULL;

                while (--wbcnt)
                {
                    wbarg++;
                    if (wbarg->wa_Lock && wbarg->wa_Name)
                    {
                        /* there is a lock, so this is a previously formatted disk */

                        if (parent = ParentDir(wbarg->wa_Lock))
                        {
                            UnLock(parent);
                            failureCode = ERROR_OBJECT_WRONG_TYPE;
                            break;
                        }

                        if (IoErr() || !Info(wbarg->wa_Lock,&infoData))
                        {
                            failureCode = IoErr();
                            break;
                        }

                        if (!FindVolumeDevice(global,((struct DosList *)BADDR(infoData.id_VolumeNode))->dol_Task))
                        {
                            failureCode = ERROR_OBJECT_WRONG_TYPE;
                            break;
                        }

                        UnLock(wbarg->wa_Lock);
                        wbarg->wa_Lock = NULL;
                    }
                    else
                    {
                        /* wbarg did not have a lock, must be an uninitialized disk */
                        strcpy(global->gd_DeviceName,wbarg->wa_Name);
                    }

                    if (failureCode = GetFSInfo(global))
                        break;

                    if (OpenFmtWindow(global))
	            {
                        if (!EditFmtOptions(global))
                            break;

                        if (ReallyFormat(global))
                        {
                            if (failureCode = Initialize(global))
                            {
                                printFailure = FALSE;
                                break;
                            }
                        }
                    }
                    else
                    {
                        failureCode = ERROR_NO_FREE_STORE;
                        break;
                    }
                }
                CloseFmtWindow(global);
            }

            if (printFailure && failureCode)
            {
                Fault(failureCode,NULL,str,sizeof(str));
                PrintF(global,MSG_FMT_FAILED_DOS,str);
            }
        }
        else
        {
            memset(opts,0,sizeof(opts));

            if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
            {
                volName               =  (STRPTR)opts[OPT_NAME];
                devName               =  (STRPTR)opts[OPT_DEVICE];
                global->gd_OFS        =  (BOOL)opts[OPT_OFS];
                global->gd_FFS        =  (BOOL)opts[OPT_FFS];
                global->gd_Intl       =  (BOOL)opts[OPT_INTL];
                global->gd_NoIntl     =  (BOOL)opts[OPT_NOINTL];
                global->gd_DirCache   =  (BOOL)opts[OPT_DIRCACHE];
                global->gd_NoDirCache =  (BOOL)opts[OPT_NODIRCACHE];
                global->gd_Quick      =  (BOOL)opts[OPT_QUICK];
                global->gd_Trashcan   = !(BOOL)opts[OPT_NOICONS];

                i = 0;
                while ((i <= NAMELENGTH) && volName[i] && (volName[i] != ':') && (volName[i] != '/'))
                    i++;

                if ((strlen(devName) <= NAMELENGTH) && (volName[i] == 0))
                {
                    strcpy(global->gd_DeviceName,devName);
                    strcpy(global->gd_VolumeName,volName);

                    if (!(failureCode = GetFSInfo(global)))
                    {
                        VPrintF(global,GetStr(MSG_FMT_PLEASEINSERT),global->gd_DeviceName);
                        Flush(Output());

                        ch    = 0;
                        abort = FALSE;
                        if (IsInteractive(Input()))
                        {
                            while (WaitForChar(Input(),30))
                            {
                                if (Read(Input(),&ch,1) != 1)
                                    ch = -1;
                            }

                            ch    = -1;
                            while (TRUE)
                            {
                                if (WaitForChar(Input(),100000))
                                {
                                    if (Read(Input(),&ch,1) == 1)
                                        break;
                                }
                                else if (CheckBreak())
                                {
                                    abort = TRUE;
                                    break;
                                }
                            }
                        }

                        if (!abort)
                        {
                            if (ch != '\n')
                                Read(Input(),&ch,1);

                            PutStr("\2330 p\n");  /* turn off cursor */
                            failureCode = Initialize(global);
                            PutStr("\233 p");     /* turn on cursor  */
                        }
                        else
                        {
                            PutStr("\n");
                            failureCode = ERROR_BREAK;
                            PrintFault(ERROR_BREAK,NULL);
                        }
                    }
                    else
                    {
                        PrintErr(failureCode);
                    }
                }
                else
                {
                    failureCode = ERROR_INVALID_COMPONENT_NAME;
                    PrintErr(ERROR_INVALID_COMPONENT_NAME);
                }
                FreeArgs(rdargs);
            }
            else
            {
                failureCode = IoErr();
                PrintErr(failureCode);
            }
        }

        if (LocaleBase)
        {
            CloseCatalog(global->gd_LocaleInfo.li_Catalog);
            CloseLibrary(LocaleBase);
        }
    }

    if (DOSBase)
    {
        SetIoErr(failureCode);
        CloseLibrary(DiskfontBase);
        CloseLibrary(GadToolsBase);
        CloseLibrary(UtilityBase);
        CloseLibrary(GfxBase);
        CloseLibrary(IntuitionBase);
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
        Forbid();
        ReplyMsg(WBenchMsg);
    }

    if (!failureCode)
        return(RETURN_OK);

    if (failureCode == ERROR_BREAK)
        return(RETURN_WARN);

    return(RETURN_FAIL);
}


/****************************************************************************/


#define GetFSSM(global,device) ((struct FileSysStartupMsg *)(device->dol_misc.dol_handler.dol_Startup * 4))


/****************************************************************************/


#define ROLLOVER_POINT 9999


LONG GetFSInfo(struct GlobalData *global)
{
struct InfoData __aligned  infoData;
struct FileSysStartupMsg  *fssm;
struct DosEnvec           *env;
UWORD                      i;
struct DosList            *dl;
char                       colonName[NAMELENGTH+2];
ULONG                      used;
ULONG                      percent;
ULONG                      total;
char                       totalchar;
ULONG                      msg;

    i = 0;
    while ((global->gd_VolumeName[i] != ':') && global->gd_VolumeName[i])
        i++;
    global->gd_VolumeName[i] = 0;

    i = 0;
    while ((global->gd_DeviceName[i] != ':') && global->gd_DeviceName[i])
        i++;
    global->gd_DeviceName[i] = 0;

    global->gd_OldVolumeName[0] = 0;
/*
kprintf("GFSI: 1\n");
*/
    dl = LockDosList(LDF_DEVICES | LDF_VOLUMES | LDF_READ);
    if (global->gd_DeviceNode = FindDosEntry(dl,global->gd_DeviceName,LDF_DEVICES | LDF_READ))
    {
/*
kprintf("GFSI: 2, searching for %b\n",global->gd_DeviceNode->dol_Name);
*/
        sprintf(global->gd_DeviceName,"%b",global->gd_DeviceNode->dol_Name);
        while (dl = NextDosEntry(dl,LDF_VOLUMES | LDF_READ))
        {
            if (dl->dol_Task == global->gd_DeviceNode->dol_Task)
            {
/*
kprintf("GFSI: 3, found match with %b\n",dl->dol_Name);
*/
                sprintf(global->gd_OldVolumeName,"%b",dl->dol_Name);
                break;
            }
        }
    }
    UnLockDosList(LDF_DEVICES | LDF_VOLUMES | LDF_READ);

    if (!global->gd_DeviceNode)
    {
/*
kprintf("GFSI: 4, did find match\n");
*/
        return(ERROR_DEVICE_NOT_MOUNTED);
    }

    strcpy(colonName,global->gd_DeviceName);
    strcat(colonName,":");
/*
kprintf("GFSI: 5, calling IsFileSystem on %s\n",colonName);
*/
    if (!IsFileSystem(colonName))
        return(ERROR_OBJECT_WRONG_TYPE);
/*
kprintf("GFSI: 6, calling GetFSSM()\n");
*/
    if (!(fssm = GetFSSM(global,global->gd_DeviceNode)))
        return(ERROR_OBJECT_WRONG_TYPE);

    if (!(env = (struct DosEnvec *)BADDR(fssm->fssm_Environ)))
        return(ERROR_OBJECT_WRONG_TYPE);
/*
kprintf("GFSI: 7, sending a ACTION_DISK_INFO to %lx\n",global->gd_DeviceNode->dol_Task);
*/
    if (!DoPkt1(global->gd_DeviceNode->dol_Task,ACTION_DISK_INFO,MKBADDR(&infoData)))
        return(ERROR_NO_DISK);
/*
kprintf("GFSI: 8\n");
*/

    global->gd_BlocksUsed    = infoData.id_NumBlocksUsed;
    global->gd_BytesPerBlock = env->de_SizeBlock*4;
    global->gd_NumBlocks     = env->de_BlocksPerTrack * env->de_Surfaces * (env->de_HighCyl-env->de_LowCyl+1);
    global->gd_DiskType      = ID_DOS_DISK;

    if (global->gd_NumBlocks == 0)
    {
        global->gd_NumBlocks = infoData.id_NumBlocks;
        if (global->gd_NumBlocks == 0)
        {
            return(ERROR_OBJECT_WRONG_TYPE);
        }
    }

    if (env->de_TableSize >= DE_DOSTYPE)
    {
        global->gd_DiskType = env->de_DosType;
        if ((global->gd_DiskType == ID_FFS_DISK) || (global->gd_DiskType == ID_INTER_FFS_DISK) || (global->gd_DiskType == ID_DC_FFS_DISK))
            global->gd_FFS = TRUE;

        if ((global->gd_DiskType == ID_INTER_DOS_DISK) || (global->gd_DiskType == ID_INTER_FFS_DISK))
            global->gd_Intl = TRUE;

        if ((global->gd_DiskType == ID_DC_DOS_DISK) || (global->gd_DiskType == ID_DC_FFS_DISK))
            global->gd_DirCache = TRUE;
    }

    if (global->gd_OldVolumeName[0])
        msg = MSG_FMT_CAPACITY_1;
    else
        msg = MSG_FMT_CAPACITY_2;

    total   = global->gd_NumBlocks;
    used    = global->gd_BlocksUsed;
    percent = (((used * 100) + (total>>1)) / total);
    total  *= global->gd_BytesPerBlock;

    totalchar = 'K';
    if ((total >>= 10) > ROLLOVER_POINT)
    {
        totalchar='M';
        total >>= 10;
    }

    sprintf(global->gd_Capacity,GetStr(msg),total,totalchar,percent);
    return(0);
}


/****************************************************************************/


BOOL FindVolumeDevice(struct GlobalData *global, struct MsgPort *volumeHandler)
{
struct DosList *dl;
BOOL            state;

    state = FALSE;
    dl    = LockDosList(LDF_DEVICES | LDF_READ);
    while (dl = NextDosEntry(dl,LDF_DEVICES | LDF_READ))
    {
	if (dl->dol_Task == volumeHandler)
	{
	    state = TRUE;
            sprintf(global->gd_DeviceName,"%b",dl->dol_Name);
            break;
	}
    }

    UnLockDosList(LDF_DEVICES | LDF_READ);

    return(state);
}


/****************************************************************************/


BOOL OpenFmtWindow(struct GlobalData *global)
{
struct TagItem tags[10];

    if (global->gd_Window)
        return(TRUE);

    if (global->gd_Screen = LockPubScreen(NULL))
    {
        if (global->gd_DrawInfo = GetScreenDrawInfo(global->gd_Screen))
        {
            if (global->gd_VisualInfo = GetVisualInfoA(global->gd_Screen,NULL))
            {
                if (global->gd_Font = OpenDiskFont(&topazAttr))
                {
                    tags[0].ti_Tag  = WA_Title;
                    tags[0].ti_Data = (ULONG)GetStr(MSG_FMT_FORMAT_TITLE);
                    tags[1].ti_Tag  = WA_InnerWidth;
                    tags[1].ti_Data = 424;
                    tags[2].ti_Tag  = WA_InnerHeight;
                    tags[2].ti_Data = 200;
                    tags[3].ti_Tag  = WA_Flags;
                    tags[3].ti_Data = WFLG_SMART_REFRESH | WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_NOCAREREFRESH;
                    tags[4].ti_Tag  = WA_AutoAdjust;
                    tags[4].ti_Data = TRUE;
                    tags[5].ti_Tag  = WA_PubScreen;
                    tags[5].ti_Data = (ULONG)global->gd_Screen;
                    tags[6].ti_Tag  = WA_IDCMP;
                    tags[6].ti_Data = CHECKBOXIDCMP | BUTTONIDCMP | STRINGIDCMP | LISTVIEWIDCMP | IDCMP_DISKINSERTED | IDCMP_DISKREMOVED;
                    tags[7].ti_Tag  = TAG_DONE;

                    if (global->gd_Window = OpenWindowTagList(NULL,tags))
                    {
                        SetFont(global->gd_Window->RPort,global->gd_Font);
                        global->gd_IntuiSig = (1L << global->gd_Window->UserPort->mp_SigBit);
                        return(TRUE);
                    }
                }
            }
        }
    }
    CloseFmtWindow(global);

    return(FALSE);
}


/*****************************************************************************/


struct Node *FindNameNC(struct GlobalData *global, struct List *list, STRPTR name)
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


/****************************************************************************/


struct Gadget *CreateFmtGadget(struct GlobalData *global, ULONG kind,
                               struct NewGadget *ng, ULONG tags, ...)
{
    ng->ng_TextAttr   = &topazAttr;
    ng->ng_UserData   = 0;
    ng->ng_Flags      = NULL;
    ng->ng_VisualInfo = global->gd_VisualInfo;
    ng->ng_GadgetText = GetStr((ULONG)ng->ng_GadgetText);

    return(global->gd_LastAdded = CreateGadgetA(kind,global->gd_LastAdded,ng,(struct TagItem *)&tags));
}


/****************************************************************************/


VOID DrawBB(struct GlobalData *global, WORD x, WORD y,
                                       WORD w, WORD h, ULONG tags, ...)
{
    DrawBevelBoxA(global->gd_Window->RPort,x+global->gd_Window->BorderLeft,
                                           y+global->gd_Window->BorderTop,
                                           w,h,(struct TagItem *)&tags);
}


/****************************************************************************/


VOID PlaceText(struct GlobalData *global, STRPTR text, WORD x, WORD y)
{
struct Window   *window;
struct RastPort *rp;

    window = global->gd_Window;
    rp     = window->RPort;

    SetAPen(rp,global->gd_DrawInfo->dri_Pens[TEXTPEN]);
    Move(rp,x+window->BorderLeft,y+window->BorderTop);
    Text(rp,text,strlen(text));
}


/****************************************************************************/


VOID PutTick(struct GlobalData *global, WORD left)
{
struct Window *window;

    window = global->gd_Window;
    RectFill(window->RPort,window->BorderLeft+left,
                           window->BorderTop+GAUGE_BOTTOM+1,
                           window->BorderLeft+left+1,
                           window->BorderTop+GAUGE_BOTTOM+2);
}


/****************************************************************************/


VOID SetGadgetAttr(struct GlobalData *global, struct Gadget *gad, ULONG tags, ...)
{
    GT_SetGadgetAttrsA(gad,global->gd_Window,NULL,(struct TagItem *)&tags);
}


/****************************************************************************/


#define CMD_NOP         0
#define CMD_FORMAT      1
#define CMD_QUICKFORMAT 2
#define CMD_CANCEL      3
#define CMD_ABORT       4
#define CMD_FFS         5
#define CMD_INTL        6
#define CMD_DC          7
#define CMD_DEVICE      8  /* in select device window */
#define CMD_CONTINUE    9  /* in select device window */


BOOL EditFmtOptions(struct GlobalData *global)
{
struct Library      *SysBase = (*((struct Library **) 4));
struct NewGadget     ng;
struct IntuiMessage *intuiMsg;
struct Window       *window;
struct Gadget       *volumeGad;
struct Gadget       *trashGad;
struct Gadget       *ffsGad;
struct Gadget       *intlGad;
struct Gadget       *info1Gad;
struct Gadget       *info2Gad;
struct Gadget       *info3Gad;
struct Gadget       *quickGad;
BOOL                 result;
BOOL                 exitLoop;
STRPTR               str;
char                 string0[80];
char                 string1[80];
char                 string2[80];
BOOL                 printInfo;
BOOL                 oldIntl;
struct Gadget       *dcGad;

    result             = FALSE;
    window             = global->gd_Window;
    global->gd_Gadgets = CreateContext(&global->gd_LastAdded);

    strncpy(global->gd_WindowTitle,GetStr(MSG_FMT_FORMAT_TITLE),sizeof(global->gd_WindowTitle));
    strncat(global->gd_WindowTitle," - ",sizeof(global->gd_WindowTitle));
    strncat(global->gd_WindowTitle,global->gd_DeviceName,sizeof(global->gd_WindowTitle));
    SetWindowTitles(window,global->gd_WindowTitle,(STRPTR)-1);

    SetAPen(window->RPort,global->gd_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    RectFill(window->RPort,window->BorderLeft,window->BorderTop,
                           window->Width-window->BorderRight-1,
                           window->Height-window->BorderBottom-1);

    ng.ng_TopEdge    = window->BorderTop+6;
    ng.ng_LeftEdge   = window->BorderLeft+192;
    ng.ng_Width      = 232;
    ng.ng_Height     = 20;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_INFO_GAD;
    ng.ng_GadgetID   = CMD_NOP;

    info1Gad = CreateFmtGadget(global,TEXT_KIND,&ng,GTTX_Clipped,TRUE,
                                                    TAG_DONE);

    ng.ng_TopEdge    = window->BorderTop+27;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_NOTHING;
    info2Gad = CreateFmtGadget(global,TEXT_KIND,&ng,GTTX_Clipped, TRUE,
                                                    TAG_DONE);

    ng.ng_TopEdge    = window->BorderTop+48;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_NOTHING;
    info3Gad = CreateFmtGadget(global,TEXT_KIND,&ng,GTTX_Clipped, TRUE,
                                                    TAG_DONE);

    ng.ng_TopEdge    = window->BorderTop+69;
    ng.ng_Width      = 226;
    ng.ng_Height     = 20;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_VOLUMENAME_GAD;
    ng.ng_GadgetID   = CMD_NOP;

    volumeGad = CreateFmtGadget(global,STRING_KIND,&ng,GTST_String,   global->gd_VolumeName,
                                                       GTST_MaxChars, NAMELENGTH,
                                                       TAG_DONE);
                                 
    ng.ng_TopEdge    = window->BorderTop+90;
    ng.ng_Width      = 20;
    ng.ng_Height     = 20;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_TRASH_GAD;
    ng.ng_GadgetID   = CMD_NOP;

    trashGad = CreateFmtGadget(global,CHECKBOX_KIND,&ng,GTCB_Checked, global->gd_Trashcan,
                                                        TAG_DONE);

    ffsGad  = NULL;
    intlGad = NULL;
    dcGad   = NULL;

    if ((global->gd_DiskType >= ID_DOS_DISK) && (global->gd_DiskType <= ID_DC_FFS_DISK))
    {
        ng.ng_TopEdge    = window->BorderTop+111;
        ng.ng_Width      = 20;
        ng.ng_GadgetText = (STRPTR)MSG_FMT_FFS_GAD;
        ng.ng_GadgetID   = CMD_FFS;

        ffsGad = CreateFmtGadget(global,CHECKBOX_KIND,&ng,GTCB_Checked, global->gd_FFS,
                                                          TAG_DONE);

        ng.ng_TopEdge    = window->BorderTop+132;
        ng.ng_Width      = 20;
        ng.ng_GadgetText = (STRPTR)MSG_FMT_INTL_GAD;
        ng.ng_GadgetID   = CMD_INTL;

        intlGad = CreateFmtGadget(global,CHECKBOX_KIND,&ng,GTCB_Checked, global->gd_Intl || global->gd_DirCache,
        						   GA_Disabled,  global->gd_DirCache,
                                                           TAG_DONE);

        ng.ng_TopEdge    = window->BorderTop+153;
        ng.ng_Width      = 20;
        ng.ng_GadgetText = (STRPTR)MSG_FMT_DIRCACHE_GAD;
        ng.ng_GadgetID   = CMD_DC;

        dcGad = CreateFmtGadget(global,CHECKBOX_KIND,&ng,GTCB_Checked,global->gd_DirCache,
                                                         TAG_DONE);
    }

    ng.ng_TopEdge    = window->BorderTop+176;
    ng.ng_LeftEdge   = window->BorderLeft+8;
    ng.ng_Width      = 134;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_FORMAT_GAD;
    ng.ng_GadgetID   = CMD_FORMAT;

    CreateFmtGadget(global,BUTTON_KIND,&ng,TAG_DONE);

    ng.ng_LeftEdge   = window->BorderLeft+145;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_QFORMAT_GAD;
    ng.ng_GadgetID   = CMD_QUICKFORMAT;

/* GA_Disabled was commented out because the check for the old volume name
 * is insufficient. The correct check would be to attepmt to read the first
 * and last cylinder of the device. If that works, then the disk can support
 * quick format. No time to do this now, so we just punt.
 */
    quickGad = CreateFmtGadget(global,BUTTON_KIND,&ng,/* GA_Disabled,(global->gd_OldVolumeName[0] == 0), */
                                                      TAG_DONE);

    ng.ng_LeftEdge   = window->BorderLeft+282;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_CANCEL_GAD;
    ng.ng_GadgetID   = CMD_CANCEL;

    CreateFmtGadget(global,BUTTON_KIND,&ng,TAG_DONE);

    if (global->gd_LastAdded)
    {
        AddGList(window,global->gd_Gadgets,-1,-1,NULL);
        RefreshGList(global->gd_Gadgets,window,NULL,-1);
        GT_RefreshWindow(window,NULL);

        ActivateGadget(volumeGad,window,NULL);

        printInfo = TRUE;
        oldIntl   = global->gd_Intl;

        exitLoop = FALSE;
        while (!exitLoop)
        {
            if (printInfo)
            {
                sprintf(string0,GetStr(MSG_FMT_DEVNAME),global->gd_DeviceName);
                if (global->gd_OldVolumeName[0] == 0)
                {
                    strcpy(string1,global->gd_Capacity);
                    string2[0] = 0;
                }
                else
                {
                    sprintf(string1,GetStr(MSG_FMT_VOLNAME),global->gd_OldVolumeName);
                    strcpy(string2,global->gd_Capacity);
                }

                SetGadgetAttr(global,info1Gad,GTTX_Text,string0,TAG_DONE);
                SetGadgetAttr(global,info2Gad,GTTX_Text,string1,TAG_DONE);
                SetGadgetAttr(global,info3Gad,GTTX_Text,string2,TAG_DONE);

                printInfo = FALSE;
            }

            WaitPort(window->UserPort);
            if (intuiMsg = GT_GetIMsg(window->UserPort))
            {
                switch (intuiMsg->Class)
                {
                    case IDCMP_DISKINSERTED  :
                    case IDCMP_DISKREMOVED   : GetFSInfo(global);
                                               printInfo = TRUE;
                                               SetGadgetAttr(global,quickGad,GA_Disabled,(global->gd_OldVolumeName[0] == 0),
                                                                             TAG_DONE);
                                               break;

                    case IDCMP_GADGETUP      :
                    case IDCMP_GADGETDOWN    : switch (((struct Gadget *)intuiMsg->IAddress)->GadgetID)
                                               {
                                                   case CMD_FORMAT      : global->gd_Quick = FALSE;
                                                                          result = TRUE;
                                                                          exitLoop = TRUE;
                                                                          break;

                                                   case CMD_QUICKFORMAT : global->gd_Quick = TRUE;
                                                                          result = TRUE;
                                                                          exitLoop = TRUE;
                                                                          break;

                                                   case CMD_CANCEL      : exitLoop = TRUE;
                                                                          break;

                                                   case CMD_INTL        : oldIntl = ((GFLG_SELECTED & intlGad->Flags) != 0);
                                                                          break;

                                                   case CMD_DC          : SetGadgetAttr(global,intlGad,GA_Disabled, (GFLG_SELECTED & dcGad->Flags),
                                                                                                       GTCB_Checked, oldIntl || (GFLG_SELECTED & dcGad->Flags),
                                                                                                       TAG_DONE);
                                                                          break;

                                                   default              : break;
                                               }
                }

                GT_ReplyIMsg(intuiMsg);
            }
        }

        RemoveGList(window,global->gd_Gadgets,-1);

        ModifyIDCMP(window,IDCMP_GADGETUP);

        if (result)
        {
            if (intlGad)
            {
                global->gd_Intl   = (SELECTED & intlGad->Flags);
                global->gd_NoIntl = !(SELECTED & intlGad->Flags);
            }

            if (ffsGad)
            {
                global->gd_OFS = !(SELECTED & ffsGad->Flags);
                global->gd_FFS = (SELECTED & ffsGad->Flags);
            }

            if (dcGad)
            {
                global->gd_DirCache   = (SELECTED & dcGad->Flags);
                global->gd_NoDirCache = !(SELECTED & dcGad->Flags);
            }

            global->gd_Trashcan = (SELECTED & trashGad->Flags);
            strcpy(global->gd_VolumeName,((struct StringInfo *)volumeGad->SpecialInfo)->Buffer);

            SetAPen(window->RPort,global->gd_DrawInfo->dri_Pens[BACKGROUNDPEN]);
            RectFill(window->RPort,window->BorderLeft,window->BorderTop,
                                   window->Width-window->BorderRight-1,
                                   window->Height-window->BorderBottom-1);

            FreeGadgets(global->gd_Gadgets);
            global->gd_Gadgets   = NULL;
            global->gd_LastAdded = NULL;

            if (!global->gd_Quick)
            {
                SizeWindow(window,0,-53);

                str = GetStr(MSG_FMT_FORMATTING_HDR);
                PlaceText(global,str,(window->Width-TextLength(window->RPort,str,strlen(str))) / 2,GAUGE_TOP-6);

                DrawBB(global,GAUGE_LEFT,GAUGE_TOP,GAUGE_WIDTH,GAUGE_HEIGHT,GTBB_Recessed,TRUE,
                                                                            GT_VisualInfo, global->gd_VisualInfo,
                                                                            TAG_DONE);

                SetAPen(window->RPort,global->gd_DrawInfo->dri_Pens[TEXTPEN]);
                PutTick(global,GAUGE_LEFT);
                PutTick(global,GAUGE_LEFT+(GAUGE_WIDTH/4));
                PutTick(global,GAUGE_LEFT+(GAUGE_WIDTH/2));
                PutTick(global,GAUGE_LEFT+((GAUGE_WIDTH/4)*3));
                PutTick(global,GAUGE_RIGHT-1);

                PlaceText(global,"0%",GAUGE_LEFT,GAUGE_TOP+GAUGE_HEIGHT+12);
                PlaceText(global,"50%",GAUGE_LEFT+(GAUGE_WIDTH/2)-12,GAUGE_TOP+GAUGE_HEIGHT+12);
                PlaceText(global,"100%",GAUGE_RIGHT-32,GAUGE_TOP+GAUGE_HEIGHT+12);

                global->gd_Gadgets = CreateContext(&global->gd_LastAdded);

                ng.ng_TopEdge    = window->BorderTop+176;
                ng.ng_LeftEdge   = window->BorderLeft+146;
                ng.ng_Width      = 134;
                ng.ng_GadgetText = (STRPTR)MSG_FMT_STOP_GAD;
                ng.ng_GadgetID   = CMD_ABORT;

                CreateFmtGadget(global,BUTTON_KIND,&ng,TAG_DONE);

                if (global->gd_LastAdded)
                {
                    AddGList(window,global->gd_Gadgets,-1,-1,NULL);
                    RefreshGList(global->gd_Gadgets,window,NULL,-1);
                    GT_RefreshWindow(window,NULL);
                }
                else
                {
                    result = FALSE;
                }
            }
        }
    }

    return(result);
}


/****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}


/*****************************************************************************/


VOID GetDeviceList(struct GlobalData *global, struct List *list)
{
struct Library *SysBase = (*((struct Library **) 4));
struct DosList *dl;
struct Node    *device;
STRPTR          name;
struct Node    *node;
struct List     temp;

    while (device = RemHead(list))
        FreeVec(device);

    NewList(&temp);

    dl = LockDosList(LDF_DEVICES|LDF_READ);
    while (dl = NextDosEntry(dl,LDF_DEVICES|LDF_READ))
    {
        name = (STRPTR)((ULONG)dl->dol_Name * 4);
        strcpy(global->gd_DeviceName,&name[1]);

        if (device = AllocVec(sizeof(struct Node) + (ULONG)name[0] + 2 + 100,MEMF_CLEAR|MEMF_PUBLIC))
        {
            device->ln_Name = (STRPTR)((ULONG)device + sizeof(struct Node));
            CopyMem(&name[1],device->ln_Name,(ULONG)name[0]);
/*
kprintf("GDL: 1, %s\n",device->ln_Name);
*/
            node = temp.lh_Head;
            while (node->ln_Succ)
            {
                if (Stricmp(node->ln_Name,device->ln_Name) >= 0)
                    break;
                node = node->ln_Succ;
            }
            Insert(&temp,(struct Node *)device,node->ln_Pred);
        }
    }

    UnLockDosList(LDF_DEVICES|LDF_READ);

    while (node = RemHead(&temp))
    {
        strcpy(global->gd_DeviceName,node->ln_Name);
        if (GetFSInfo(global) == 0)
        {
            strcat(node->ln_Name," (");
            strcat(node->ln_Name,global->gd_Capacity);
            strcat(node->ln_Name,")");
            AddTail(list,node);
        }
        else
        {
            FreeVec(node);
        }
    }
}


/****************************************************************************/


BOOL SelectFmtDevice(struct GlobalData *global)
{
struct Library      *SysBase = (*((struct Library **) 4));
struct NewGadget     ng;
struct IntuiMessage *intuiMsg;
struct Window       *window;
struct Gadget       *devicesGad;
struct Gadget       *continueGad;
BOOL                 result;
BOOL                 exitLoop;
WORD                 devNum;
UWORD                i;
ULONG                oldMicros;
ULONG                oldSecs;
struct List          deviceList;
struct Node         *node;

    result             = FALSE;
    window             = global->gd_Window;
    global->gd_Gadgets = CreateContext(&global->gd_LastAdded);
    devNum             = 0;
    oldMicros          = 0;
    oldSecs            = 0;

    NewList(&deviceList);
    GetDeviceList(global,&deviceList);

    ng.ng_TopEdge    = window->BorderTop+20;
    ng.ng_LeftEdge   = window->BorderLeft+8;
    ng.ng_Width      = 408;
    ng.ng_Height     = 143; 
    ng.ng_GadgetText = (STRPTR)MSG_FMT_SELECTDEV_GAD;
    ng.ng_GadgetID   = CMD_DEVICE;

    devicesGad = CreateFmtGadget(global,LISTVIEW_KIND,&ng,GTLV_Labels,       &deviceList,
                                                          GTLV_ShowSelected, NULL,
                                                          GTLV_Selected,     ~0,
                                                          LAYOUTA_SPACING, 4,
                                                          GTLV_ScrollWidth,  18,
                                                          TAG_DONE);

    ng.ng_TopEdge    = window->BorderTop+178;
    ng.ng_LeftEdge   = window->BorderLeft+8;
    ng.ng_Width      = 134;
    ng.ng_Height     = 20;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_CONTINUE_GAD;
    ng.ng_GadgetID   = CMD_CONTINUE;

    continueGad = CreateFmtGadget(global,BUTTON_KIND,&ng,GA_Disabled, TRUE,
                                                         TAG_DONE);

    ng.ng_LeftEdge   = window->BorderLeft+282;
    ng.ng_GadgetText = (STRPTR)MSG_FMT_CANCEL_GAD;
    ng.ng_GadgetID   = CMD_CANCEL;

    CreateFmtGadget(global,BUTTON_KIND,&ng,TAG_DONE);

    if (global->gd_LastAdded)
    {
        AddGList(window,global->gd_Gadgets,-1,-1,NULL);
        RefreshGList(global->gd_Gadgets,window,NULL,-1);
        GT_RefreshWindow(window,NULL);

        exitLoop = FALSE;
        while (!exitLoop)
        {
            WaitPort(window->UserPort);
            if (intuiMsg = GT_GetIMsg(window->UserPort))
            {
                switch (intuiMsg->Class)
                {
                    case IDCMP_DISKINSERTED  :
                    case IDCMP_DISKREMOVED   : SetGadgetAttr(global,devicesGad,GTLV_Labels,~0,
                                                                               TAG_DONE);
                                               GetDeviceList(global,&deviceList);
                                               SetGadgetAttr(global,devicesGad,GTLV_Labels,      &deviceList,
                                                                               GTLV_Selected,    devNum,
                                                                               GTLV_MakeVisible, devNum,
                                                                               TAG_DONE);
                                               break;

                    case IDCMP_GADGETUP      :
                    case IDCMP_GADGETDOWN    : switch (((struct Gadget *)intuiMsg->IAddress)->GadgetID)
                                               {
                                                   case CMD_DEVICE      : SetGadgetAttr(global,continueGad,GA_Disabled,FALSE,
                                                                                                           TAG_DONE);
                                                                          devNum = intuiMsg->Code;
                                                                          if (!DoubleClick(oldSecs,oldMicros,intuiMsg->Seconds,intuiMsg->Micros))
                                                                          {
                                                                              oldSecs   = intuiMsg->Seconds;
                                                                              oldMicros = intuiMsg->Micros;
                                                                              break;
                                                                          }
                                                   case CMD_CONTINUE    : result = TRUE;
                                                   case CMD_CANCEL      : exitLoop = TRUE;
                                                                          break;

                                                   default              : break;
                                               }
                }

                GT_ReplyIMsg(intuiMsg);
            }
        }

        RemoveGList(window,global->gd_Gadgets,-1);
    }

    while (node = RemHead(&deviceList))
    {
        if (devNum == 0)
        {
            i = 0;
            while (node->ln_Name[i] != ' ')
            {
                global->gd_DeviceName[i] = node->ln_Name[i];
                i++;
            }
            global->gd_DeviceName[i] = 0;
        }
        devNum--;
        FreeVec(node);
    }

    FreeGadgets(global->gd_Gadgets);
    global->gd_Gadgets = NULL;

    if (result && !GetFSInfo(global))
        return(TRUE);

    return(FALSE);
}


/****************************************************************************/


VOID CloseFmtWindow(struct GlobalData *global)
{
    if (global->gd_Window)
        CloseWindow(global->gd_Window);

    if (global->gd_DrawInfo)
        FreeScreenDrawInfo(global->gd_Screen,global->gd_DrawInfo);

    if (global->gd_VisualInfo)
        FreeVisualInfo(global->gd_VisualInfo);

    if (global->gd_Screen)
        UnlockPubScreen(NULL,global->gd_Screen);

    if (global->gd_Font)
        CloseFont(global->gd_Font);

    FreeGadgets(global->gd_Gadgets);

    global->gd_Window     = NULL;
    global->gd_DrawInfo   = NULL;
    global->gd_VisualInfo = NULL;
    global->gd_Screen     = NULL;
    global->gd_Font       = NULL;
    global->gd_Gadgets    = NULL;
}


/****************************************************************************/


BOOL CreateTrashcan(struct GlobalData *global, STRPTR deviceName)
{
struct Library    *SysBase = (*((struct Library **) 4));
struct Library    *IconBase;
BPTR               lock;
BPTR               oldCD;
BPTR               new;
struct DiskObject *diskObj;
BOOL               result;

    result = FALSE;

    if (lock = Lock(deviceName,ACCESS_READ))
    {
        oldCD = CurrentDir(lock);
        if (new = CreateDir("Trashcan"))
        {
            UnLock(new);

            if (IconBase = OpenLibrary("icon.library",37))
            {
                if (diskObj = (struct DiskObject *)GetDefDiskObject(WBGARBAGE))
                {
                    result = PutDiskObject("Trashcan",diskObj);
                    FreeDiskObject(diskObj);
                }
                CloseLibrary(IconBase);
            }
        }
        CurrentDir(oldCD);
        UnLock(lock);
    }

    return(result);
}


/****************************************************************************/


VOID VPrintF(struct GlobalData *global, STRPTR str, STRPTR arg1, ... )
{
    VPrintf(str,(LONG *)&arg1);
}


/*****************************************************************************/


VOID PrintF(struct GlobalData *global, AppStringsID str, STRPTR arg1, ... )
{
struct EasyStruct est;

    if (!global->gd_GUI)
    {
        VPrintf(GetStr(str),(LONG *)&arg1);
        VPrintf("\n",NULL);
    }
    else
    {
        est.es_StructSize   = sizeof(struct EasyStruct);
        est.es_Flags        = 0;
        est.es_Title        = GetStr(MSG_FMT_FAILED_TITLE);
        est.es_TextFormat   = GetStr(str);
        est.es_GadgetFormat = GetStr(MSG_FMT_CANCEL_GAD);

        EasyRequestArgs(NULL,&est,NULL,&arg1);
    }
}


/*****************************************************************************/


BOOL WriteProtect(struct GlobalData *global, STRPTR deviceName)
{
struct EasyStruct est;
ULONG             idcmp;

    est.es_StructSize   = sizeof(struct EasyStruct);
    est.es_Flags        = 0;
    est.es_Title        = GetStr(MSG_FMT_REQUEST_TITLE);
    est.es_TextFormat   = GetStr(MSG_FMT_WRITEPROTECT);
    est.es_GadgetFormat = GetStr(MSG_FMT_WRITEPROTECT_GAD);

    idcmp = IDCMP_DISKINSERTED;

    return (EasyRequestArgs(NULL,&est,&idcmp,&deviceName) != 0);
}


/*****************************************************************************/


BOOL ReallyFormat(struct GlobalData *global)
{
struct EasyStruct est;
STRPTR            name;
ULONG             msg;
char              prompt[200];
STRPTR            args[1];
BOOL              biggy;
BOOL              result;

    if (global->gd_OldVolumeName[0] == 0)
    {
        name = global->gd_DeviceName;
        msg  = MSG_FMT_CONFIRM_2;
    }
    else
    {
        name = global->gd_OldVolumeName;
        msg  = MSG_FMT_CONFIRM_1;
    }

    sprintf(prompt,GetStr(msg),name,global->gd_Capacity);
    args[0] = prompt;

    est.es_StructSize   = sizeof(struct EasyStruct);
    est.es_Flags        = 0;
    est.es_Title        = GetStr(MSG_FMT_REQUEST_TITLE);
    est.es_TextFormat   = "%s";
    est.es_GadgetFormat = GetStr(MSG_FMT_CONFIRM_GAD);

    biggy = ((global->gd_NumBlocks * global->gd_BytesPerBlock) > BIGDRIVESIZE);
    if ((result = EasyRequestArgs(NULL,&est,NULL,&args)) && biggy)
    {
        sprintf(prompt,GetStr(MSG_FMT_CONFIRM_3),global->gd_DeviceName,global->gd_Capacity);
        result = EasyRequestArgs(NULL,&est,NULL,&args);
    }

    return(result);
}


/*****************************************************************************/


VOID ShowProgress(struct GlobalData *global, BOOL writing, BOOL forceUpdate)
{
struct Window *window;
ULONG          currentCyl, totalCyl;

    currentCyl = global->gd_CurrentCyl-global->gd_LowCyl;
    totalCyl   = global->gd_NumberCyl;

    if (totalCyl >= 1000)
    {
        if (currentCyl % 50 == 0)
            forceUpdate = TRUE;
    }
    else if (totalCyl >= 100)
    {
        if (currentCyl % 10 == 0)
            forceUpdate = TRUE;
    }
    else
    {
        forceUpdate = TRUE;
    }

    if (forceUpdate)
    {
        if (!global->gd_GUI)
        {
            if (writing)
                VPrintF(global,GetStr(MSG_FMT_FORMATTING),(STRPTR)currentCyl,(STRPTR)(totalCyl-currentCyl-1));
            else
                VPrintF(global,GetStr(MSG_FMT_VERIFYING),(STRPTR)currentCyl,(STRPTR)(totalCyl-currentCyl-1));
        }
        else if (window = global->gd_Window)
        {
            SetAPen(window->RPort,global->gd_DrawInfo->dri_Pens[FILLPEN]);

            RectFill(window->RPort,window->BorderLeft+GAUGE_LEFT+2,
                                   window->BorderTop+GAUGE_TOP+1,
                                   window->BorderLeft+GAUGE_LEFT+((GAUGE_WIDTH-5)*currentCyl/totalCyl)+2,
                                   window->BorderTop+GAUGE_BOTTOM-1);
        }
    }
}


/*****************************************************************************/


VOID ShowComplete(struct GlobalData *global)
{
    if (!global->gd_GUI)
    {
        VPrintF(global,"%s","\n");
    }
}


/*****************************************************************************/


VOID ShowIniting(struct GlobalData *global)
{
struct Window *window;
STRPTR         str;

    if (!global->gd_GUI)
    {
        PrintF(global,MSG_FMT_INITING,NULL);
    }
    else if (window = global->gd_Window)
    {
        SetAPen(window->RPort,global->gd_DrawInfo->dri_Pens[BACKGROUNDPEN]);
        RectFill(window->RPort,window->BorderLeft,window->BorderTop,
                               window->Width-window->BorderRight-1,
                               window->Height-window->BorderBottom-1);
        str = GetStr(MSG_FMT_INITING);
        PlaceText(global,str,(window->Width-TextLength(window->RPort,str,strlen(str))) / 2,
                             (window->Height - window->RPort->TxHeight - window->BorderTop - window->BorderBottom) / 2 + window->RPort->TxBaseline);
    }
}


/****************************************************************************/


LONG DoFormat(struct GlobalData *global, struct DosList *deviceNode,
              BOOL ofs, BOOL ffs,
              BOOL intl, BOOL nointl,
              BOOL dircache, BOOL nodircache,
              BOOL quick,
              STRPTR volumeName)
{
struct Library            *SysBase = (*((struct Library **) 4));
struct InfoData __aligned  infoData;
char            __aligned  volumeNameB[40];
struct MsgPort            *fsPort;
struct FileSysStartupMsg  *fssm;
struct DosEnvec           *env;
ULONG                      memType;
LONG                       wordsPerBlk;
LONG                       bytesPerBlk;
LONG                       lowCyl;
LONG                       highCyl;
LONG                       blocksPerCyl;
LONG			   blocksPerDisk;
LONG                       bytesPerCyl;
LONG                       rootBlk;
struct IOStdReq           *devIO;
struct MsgPort            *devPort;
struct RootBlock          *root;
LONG                      *cylinder;
LONG                       error;
LONG                       i, cyl, sum;
LONG                      *p;
UBYTE                      failCnt;
BOOL                       write;
ULONG                      dosType;
ULONG                      maxTrans;
ULONG                      transCnt;
ULONG                      transSize;

    fsPort = deviceNode->dol_Task;

    error = ERROR_NO_DISK;
    if (DoPkt1(fsPort,ACTION_DISK_INFO,MKBADDR(&infoData)))
    {
        /* check if disk present */
        if (infoData.id_DiskType != ID_NO_DISK_PRESENT)
        {
            /* check volume for inhibited already */
            error = ERROR_OBJECT_IN_USE;
            if (infoData.id_DiskType != ID_BUSY_DISK)
            {
                error = ERROR_DISK_WRITE_PROTECTED;
                if (infoData.id_DiskState != ID_WRITE_PROTECTED)
                {
                    /* extract the disk parameters */
                    fssm = GetFSSM(global,deviceNode);
                    env  = (struct DosEnvec *)BADDR(fssm->fssm_Environ);

                    memType = MEMF_CHIP | MEMF_CLEAR;
                    dosType = ID_DOS_DISK;

                    if (env->de_TableSize >= DE_DOSTYPE)
                        dosType = env->de_DosType;

                    if (ffs && ((dosType == ID_DOS_DISK) || (dosType == ID_INTER_DOS_DISK) || (dosType == ID_DC_DOS_DISK)))
                        dosType = dosType + 1;  /* OFS --> FFS */

                    if (ofs && ((dosType == ID_FFS_DISK) || (dosType == ID_INTER_FFS_DISK) || (dosType == ID_DC_FFS_DISK)))
                        dosType = dosType - 1;  /* FFS --> OFS */

                    if (dircache && ((dosType == ID_DOS_DISK) || (dosType == ID_FFS_DISK)))
                        dosType = dosType + 4;  /* OFS/FFS --> QFS */

                    if (nodircache && ((dosType == ID_DC_DOS_DISK) || (dosType == ID_DC_FFS_DISK)))
                        dosType = dosType - 4;  /* QFS --> OFS/FFS */

                    if (intl && ((dosType == ID_DOS_DISK) || (dosType == ID_FFS_DISK)))
                        dosType = dosType + 2;  /* NOINTL --> INTL */

                    if (nointl && ((dosType == ID_INTER_DOS_DISK) || (dosType == ID_INTER_FFS_DISK)))
                        dosType = dosType - 2;  /* INTL --> NOINTL */

                    if (env->de_TableSize >= DE_BUFMEMTYPE)
                        memType = env->de_BufMemType | MEMF_CLEAR;

                    wordsPerBlk   = env->de_SizeBlock;
                    bytesPerBlk   = wordsPerBlk * 4;
                    blocksPerCyl  = env->de_BlocksPerTrack * env->de_Surfaces;
                    lowCyl        = env->de_LowCyl;
                    highCyl       = env->de_HighCyl;
                    blocksPerDisk = blocksPerCyl * (highCyl - lowCyl + 1);
                    bytesPerCyl   = bytesPerBlk * blocksPerCyl;
                    rootBlk       = (blocksPerDisk - 1 + env->de_Reserved) >> 1;
                    maxTrans      = bytesPerCyl;

                    if (env->de_TableSize >= DE_MAXTRANSFER)
                        maxTrans = env->de_MaxTransfer;

                    if (maxTrans > bytesPerCyl)
                        maxTrans = bytesPerCyl;

                    if (maxTrans > 65536)   /* no reason for this number, just sounds good to me */
                    {
                        if (bytesPerBlk < 65536)
                            maxTrans = (65536 / bytesPerBlk) * bytesPerBlk;
                        else
                            maxTrans = bytesPerBlk;
                    }

                    global->gd_NumberCyl = highCyl-lowCyl+1;

                    error   = ERROR_NO_FREE_STORE;
                    devPort = CreateMsgPort();
                    if (devIO = (struct IOStdReq *)CreateIORequest(devPort,sizeof(struct IOStdReq)))
                    {
                        error = ERROR_BREAK;
                        if (!CheckBreak())
                        {
                            error = ERROR_NO_DISK;
                            if (DoPkt1(fsPort,ACTION_INHIBIT,DOSTRUE))
                            {
                                error = IOERR_OPENFAIL;
                                if (!OpenDevice(BTOCSTR(fssm->fssm_Device),fssm->fssm_Unit,devIO,fssm->fssm_Flags))
                                {
                                    error = 0;
                                    if (!quick)
                                    {
                                        /* do the track-level formatting */
                                        error = ERROR_NO_FREE_STORE;
                                        if (cylinder = AllocVec(maxTrans,memType))
                                        {
                                            error = 0;
                                            cyl   = lowCyl;

                                            global->gd_CurrentCyl = cyl;
                                            global->gd_LowCyl     = cyl;
                                            ShowProgress(global,TRUE,TRUE);

                                            while ((cyl <= highCyl) && !error)
                                            {
                                                global->gd_CurrentCyl = cyl;
                                                for (i = 0; i < maxTrans / 4; i++)
                                                    cylinder[i] = cyl - lowCyl << 16 | i | dosType;

                                                if (cyl == lowCyl)
                                                    cylinder[0] = ID_BAD_DISK;

                                                failCnt   = 0;
                                                transCnt  = 0;
                                                transSize = maxTrans;
                                                write     = TRUE;
                                                while (TRUE)
                                                {
                                                    if (CheckBreak())
                                                    {
                                                        error = ERROR_BREAK;
                                                        break;
                                                    }

                                                    if (write)
                                                        devIO->io_Command = TD_FORMAT;
                                                    else
                                                        devIO->io_Command = CMD_READ;

                                                    devIO->io_Data   = cylinder;
                                                    devIO->io_Length = transSize;
                                                    devIO->io_Offset = cyl * bytesPerCyl + transCnt;

                                                    ShowProgress(global,write,FALSE);

                                                    transCnt += transSize;

                                                    if (error = DoIO(devIO))
                                                    {
                                                        ShowProgress(global,write,TRUE);
                                                        failCnt++;
                                                    }
                                                    else if (!write)
                                                    {
                                                        break;
                                                    }
                                                    else if (transCnt == bytesPerCyl)
                                                    {
                                                        write     = FALSE;
                                                        transCnt  = 0;
                                                        transSize = maxTrans;
                                                    }
                                                    else if (transSize > bytesPerCyl - transCnt)
                                                    {
                                                        transSize = bytesPerCyl - transCnt;
                                                    }

                                                    if (failCnt > MAXERRORS)
                                                    {
                                                        break;
                                                    }
                                                }
                                                cyl++;
                                            }
                                            ShowProgress(global,write,TRUE);
                                            ShowComplete(global);
                                        }
                                        FreeVec(cylinder);
                                    }

                                    if (!error)
                                    {
                                        ShowIniting(global);

                                        /* now put a root block on this sucker */
                                        volumeNameB[0] = strlen(volumeName);
                                        strcpy(&volumeNameB[1],volumeName);

                                        if (!DoPkt2(fsPort,ACTION_FORMAT,MKBADDR(volumeNameB),dosType)
                                        && ((error = IoErr()) == ERROR_ACTION_NOT_KNOWN))
                                        {
                                            /* try the old fashion way, then */
                                            error = ERROR_NO_FREE_STORE;
                                            if (root = AllocVec(bytesPerBlk,memType))
                                            {
                                                root->rb_Type          = T_SHORT;
                                                root->rb_HashSize      = HASHSIZE;
                                                root->rb_SecondaryType = ST_ROOT;
                                                root->rb_BMFlag        = 0;

                                                DateStamp(&root->rb_CreateDate);
                                                DateStamp(&root->rb_LastDate);

                                                root->rb_DiskName[0] = strlen(volumeName);
                                                strcpy(&root->rb_DiskName[1],volumeName);

                                                root->rb_CheckSum = sum = 0;
                                                for (i = 0, p = (LONG *)root; i < wordsPerBlk; sum = sum + *p++, i++);
                                                root->rb_CheckSum = -sum;

                                                devIO->io_Command = CMD_WRITE;
                                                devIO->io_Data    = root;
                                                devIO->io_Length  = bytesPerBlk;
                                                devIO->io_Offset  = rootBlk * bytesPerBlk + lowCyl * bytesPerCyl;
                                                error = DoIO(devIO);

                                                if (!error)
                                                {
                                                    /* make disk look valid (boot block) */
                                                    cylinder = (LONG *)root;
                                                    for (i = 0; i < wordsPerBlk; i++)
                                                        cylinder[i] = dosType;

                                                    devIO->io_Command = CMD_WRITE;
                                                    devIO->io_Length  = bytesPerBlk;
                                                    devIO->io_Offset  = lowCyl * bytesPerCyl;
                                                    devIO->io_Data    = cylinder;
                                                    error = DoIO(devIO);
                                                }

                                                FreeVec(root);
                                            }
                                        }
                                    }

                                    if (!error)
                                    {
                                        devIO->io_Command = CMD_UPDATE;
                                        devIO->io_Length  = 0;
                                        devIO->io_Offset  = 0;
                                        devIO->io_Data    = 0;
                                        error = DoIO(devIO);
                                    }

                                    if (!error)
                                    {
                                        devIO->io_Command = CMD_CLEAR;
                                        devIO->io_Length  = 0;
                                        devIO->io_Offset  = 0;
                                        devIO->io_Data    = 0;
                                        error = DoIO(devIO);
                                    }

                                    /* turn disk motor off */
                                    devIO->io_Command = TD_MOTOR;
                                    devIO->io_Length  = 0;
                                    devIO->io_Offset  = 0;
                                    devIO->io_Data    = 0;
                                    DoIO(devIO);

                                    CloseDevice(devIO);
                                }
                                DoPkt1(fsPort,ACTION_INHIBIT,DOSFALSE);

                                if (!error)
                                {
                                    /* wait for drive to get ready. Need time for file
                                     * system to validate before we can sanction it as
                                     * valid, and maybe write a trashcan to it
                                     */

                                    error = ERROR_NOT_A_DOS_DISK;
                                    i = 0;
                                    while (i < 100)
                                    {
                                        Delay(15);
                                        if (DoPkt1(fsPort,ACTION_DISK_INFO,MKBADDR(&infoData)))
                                        {
                                            if (infoData.id_DiskState != ID_VALIDATING)
                                            {
                                                if ((infoData.id_DiskType == dosType)
                                                ||  (infoData.id_DiskType >= ID_DOS_DISK)
                                                &&  (infoData.id_DiskType <= ID_DC_FFS_DISK))
                                                {
                                                    error = 0;
                                                    break;
                                                }
                                            }
                                        }
                                        i++;
                                    }
                                }
                            }
                        }
                    }
                    DeleteMsgPort(devPort);
                    DeleteIORequest(devIO);
                }
            }
        }
    }

    return(error);
}


/****************************************************************************/


LONG Initialize(struct GlobalData *global)
{
LONG   error;
char   dosString[100];
STRPTR deviceName;

    deviceName = global->gd_DeviceName;

    while (TRUE)
    {
        error = DoFormat(global,global->gd_DeviceNode,
                         global->gd_OFS,global->gd_FFS,
                         global->gd_Intl,global->gd_NoIntl,
                         global->gd_DirCache,global->gd_NoDirCache,
                         global->gd_Quick,global->gd_VolumeName);

        if ((error != ERROR_DISK_WRITE_PROTECTED) && (error != TDERR_WriteProt))
            break;

        if (!WriteProtect(global,deviceName))
        {
            error = ERROR_DISK_WRITE_PROTECTED;
            break;
        }
    }

    if (!error && global->gd_Trashcan)
    {
        strcpy(dosString,deviceName);
        strcat(dosString,":");
        if (!CreateTrashcan(global,dosString))
            error = ERROR_NO_TRASH;
    }

    if (error)
    {
        if ((error != ERROR_BREAK) && (!global->gd_GUI))
            VPrintF(global,"%s: ",GetStr(MSG_FMT_FAILED_TITLE));

        switch (error)
        {
            case ERROR_NO_TRASH      : PrintF(global,MSG_FMT_FAILED_TRASHCAN,global->gd_VolumeName);
                                       return(0);

            case IOERR_OPENFAIL      : PrintF(global,MSG_FMT_FAILED_NODEVICE,deviceName);
                                       return(ERROR_DEVICE_NOT_MOUNTED);

            case TDERR_DiskChanged   : PrintF(global,MSG_FMT_FAILED_DISKCHANGED,deviceName);
                                       return(ERROR_NO_DISK);

            case TDERR_WriteProt     : error = ERROR_DISK_WRITE_PROTECTED;

            case TDERR_SeekError     : if (error == TDERR_SeekError)
                                           error = ERROR_SEEK_ERROR;

            default                  : if (error >= ERROR_NO_FREE_STORE)
                                       {
                                           if ((error != ERROR_BREAK) || (!global->gd_GUI))
                                           {
                                               Fault(error,NULL,dosString,sizeof(dosString));
                                               PrintF(global,MSG_FMT_FAILED_DOS,dosString);
                                           }
                                           return(error);
                                       }

            case TDERR_BadSecPreamble:
            case TDERR_BadSecID      :
            case TDERR_BadHdrSum     :
            case TDERR_BadSecSum     :
            case TDERR_TooFewSecs    :
            case TDERR_BadSecHdr     : PrintF(global,MSG_FMT_FAILED_BADCYL,(STRPTR)global->gd_CurrentCyl);
                                       return(ERROR_NOT_A_DOS_DISK);

        }
    }

    return(0);
}
