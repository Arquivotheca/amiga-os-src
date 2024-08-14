
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <dos/dosasl.h>
#include <resources/filesysres.h>
#include <string.h>
#include <stdlib.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>
#include <clib/intuition_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* application includes */
#include "texttable.h"
#include "mount_rev.h"


/*****************************************************************************/


#define TEMPLATE   "DEVICE/M,FROM/K" VERSTAG
#define OPT_DEVICE 0
#define OPT_FROM   1
#define OPT_COUNT  2


/*****************************************************************************/


#define TOK_NUMBER    0
#define TOK_STRING    1
#define TOK_UNDECIDED 2


struct GlobalData
{
    struct DosLibrary *gd_DOSBase;
    struct Library    *gd_UtilityBase;
    struct Library    *gd_IntuitionBase;
    struct Library    *gd_SysBase;

    struct LocaleInfo  gd_LocaleInfo;

    struct Process    *gd_Process;

    /* mount list parser stuff */
    STRPTR             gd_FileName;
    BPTR               gd_InputFile;
    UWORD              gd_InputLine;
    UWORD              gd_InputColumn;
    UWORD              gd_TokenLine;
    UWORD              gd_TokenColumn;
    UBYTE              gd_TokenType;
    char               gd_CurrentToken[256];
    char	       gd_LastChar;
    BOOL               gd_TokenQuoted;   /* last token had quotes around it */

    /* secondary returns from Mount() */
    BOOL	       gd_AlreadyMounted;

    /* mount state */
    UWORD              gd_UninitializedCnt;
    BOOL               gd_InitStack;
    BOOL               gd_InitPriority;
    BOOL               gd_InitDosType;

    BOOL               gd_UnitIsStr;
    BOOL               gd_StartupIsStr;
};

#define SysBase       gd->gd_SysBase
#define DOSBase       gd->gd_DOSBase
#define UtilityBase   gd->gd_UtilityBase
#define LocaleBase    gd->gd_LocaleInfo.li_LocaleBase
#define IntuitionBase gd->gd_IntuitionBase


/*****************************************************************************/


char Keywords[] = ",SECTORSIZE=BLOCKSIZE,,SURFACES,SECTORSPERBLOCK,"\
                  "SECTORSPERTRACK=BLOCKSPERTRACK,RESERVED,PREALLOC," \
                  "INTERLEAVE,LOWCYL,HIGHCYL,BUFFERS,BUFMEMTYPE,MAXTRANSFER," \
                  "MASK,BOOTPRI,DOSTYPE,BAUD,CONTROL,DEVICE,UNIT,FLAGS,HANDLER," \
                  "STACKSIZE,PRIORITY,GLOBVEC,FILESYSTEM,STARTUP,ACTIVATE=MOUNT," \
                  "EHANDLER,FORCELOAD";

#define ENV_VECTORSIZE      0
#define ENV_SECTORSIZE      1
#define ENV_SECTORIGIN      2
#define ENV_SURFACES        3
#define ENV_SECTORSPERBLOCK 4
#define ENV_SECTORSPERTRACK 5
#define ENV_RESERVED        6
#define ENV_PREALLOC        7
#define ENV_INTERLEAVE      8
#define ENV_LOWCYL          9
#define ENV_HIGHCYL         10
#define ENV_BUFFERS         11
#define ENV_BUFMEMTYPE      12
#define ENV_MAXTRANSFER     13
#define ENV_MASK            14
#define ENV_BOOTPRI         15
#define ENV_DOSTYPE         16
#define ENV_BAUD            17
#define ENV_CONTROL         18
#define ENV_DEVICE          19
#define ENV_UNIT            20
#define ENV_FLAGS           21
#define ENV_HANDLER         22
#define ENV_STACKSIZE       23
#define ENV_PRIORITY        24
#define ENV_GLOBVEC         25
#define ENV_FILESYSTEM      26
#define ENV_STARTUP         27
#define ENV_ACTIVATE        28   /* old 'Mount' keyword */
#define ENV_EHANDLER        29
#define ENV_FORCELOAD       30

#define ENV_NUMSLOTS        31   /* number of slots in private environment array */

#define VECTORSIZE          18


/*****************************************************************************/


/* different possible parsing states */
#define SCAN_DEVICE   0
#define SCAN_KEYWORD  1
#define SCAN_EQUAL    2
#define SCAN_ARGUMENT 3
#define SCAN_DONE     4

/* possible failure codes returned by CreateDosEntry(),
 * ProcessKeywordArguments(), and ValidateEnvironment()
 */
#define CODE_SUCCESS          0
#define CODE_NOMEMORY         1
#define CODE_ALREADYMOUNTED   2
#define CODE_STRINGEXPECTED   3
#define CODE_NUMBEREXPECTED   4
#define CODE_EQUALEXPECTED    5
#define CODE_ARGUMENTEXPECTED 6
#define CODE_MISSINGKEYWORDS  7
#define CODE_UNKNOWNKEYWORD   8
#define CODE_DEVICENOTFOUND   9


/*****************************************************************************/


BOOL Mount(struct GlobalData *gd, STRPTR device, STRPTR fileName,
           STRPTR iconName, BOOL needDevice);


/*****************************************************************************/


VOID kprintf(STRPTR,...);


LONG main(VOID)
{
struct AnchorPath  __aligned anchor;
struct WBArg      *wbarg;
LONG               wbcnt;
BPTR               oldLock;
BPTR               lock;
LONG               opts[OPT_COUNT];
struct RDArgs     *rdargs;
LONG               failureLevel = RETURN_FAIL;
struct WBStartup  *WBenchMsg = NULL;
STRPTR            *ptr;
struct GlobalData  global;
struct GlobalData *gd;
LONG               result;
char               noColonName[40];
char               pathName[70];
BOOL               bool;

    gd = &global;
    memset(gd,0,sizeof(struct GlobalData));

    SysBase = (*((struct Library **) 4));

    gd->gd_Process = (struct Process *)FindTask(NULL);
    if (!(gd->gd_Process->pr_CLI))
    {
        WaitPort(&gd->gd_Process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&gd->gd_Process->pr_MsgPort);
    }

    if (DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37))
    {
        if (UtilityBase = OpenLibrary("utility.library",37))
        {
            if (IntuitionBase = OpenLibrary("intuition.library",37))
            {
                if (LocaleBase = OpenLibrary("locale.library",38))
                    gd->gd_LocaleInfo.li_Catalog = OpenCatalogA(NULL,"sys/c.catalog",NULL);

                if (WBenchMsg)
                {
                    failureLevel = RETURN_OK;
                    wbarg = WBenchMsg->sm_ArgList;
                    wbcnt = WBenchMsg->sm_NumArgs;

                    while (--wbcnt > 0)
                    {
                        wbarg++;
                        oldLock = CurrentDir(wbarg->wa_Lock);
                        Mount(gd,wbarg->wa_Name,wbarg->wa_Name,wbarg->wa_Name,FALSE);
                        CurrentDir(oldLock);
                    }
                }
                else
                {
                    memset(opts,0,sizeof(opts));
                    if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                    {
                        failureLevel = RETURN_OK;
                        if (ptr = (STRPTR *)opts[OPT_DEVICE])
                        {
                            while ((*ptr) && (failureLevel == RETURN_OK))
                            {
                                strncpy(noColonName,*ptr,sizeof(noColonName));
                                if (noColonName[0] && noColonName[strlen(noColonName)-1] != ':')
                                {
                                    memset(&anchor,0,sizeof(struct AnchorPath));
                                    anchor.ap_Flags     = APF_DOWILD;
                                    anchor.ap_BreakBits = SIGBREAKF_CTRL_C;

                                    if (!(result = MatchFirst(*ptr,&anchor)))
                                    {
                                        while (TRUE)
                                        {
                                            if (CheckSignal(SIGBREAKF_CTRL_C))
                                            {
                                                failureLevel = RETURN_WARN;
                                                PrintFault(ERROR_BREAK,NULL);
                                                break;
                                            }

                                            if (anchor.ap_Info.fib_DirEntryType < 0)
                                            {
                                                oldLock = CurrentDir(anchor.ap_Current->an_Lock);
                                                bool = Mount(gd,anchor.ap_Info.fib_FileName,anchor.ap_Info.fib_FileName,anchor.ap_Info.fib_FileName,FALSE);
                                                CurrentDir(oldLock);

                                                if (!bool && !gd->gd_AlreadyMounted)
                                                {
                                                    failureLevel = RETURN_FAIL;
                                                    break;
                                                }
                                            }

                                            if (result = MatchNext(&anchor))
                                            {
                                                if (result != ERROR_NO_MORE_ENTRIES)
                                                {
                                                    failureLevel = RETURN_FAIL;
                                                    PrintFault(result,NULL);
                                                }
                                                break;
                                            }
                                        }
                                        MatchEnd(&anchor);
                                    }
                                    else if (result != ERROR_NO_MORE_ENTRIES)
                                    {
                                        failureLevel = RETURN_FAIL;
                                        PrintFault(IoErr(),NULL);
                                    }
                                }
                                else
                                {
                                    noColonName[strlen(noColonName)-1] = 0;
                                    bool = FALSE;

                                    if (!opts[OPT_FROM])
                                    {
                                        strcpy(pathName,"DEVS:DOSDrivers/");
                                        strncat(pathName,noColonName,sizeof(pathName));
                                        if (lock = Lock(pathName,ACCESS_READ))
                                        {
                                            UnLock(lock);
                                            bool = Mount(gd,noColonName,pathName,pathName,FALSE);
                                        }
                                        else if (IoErr() == ERROR_OBJECT_NOT_FOUND)
                                        {
                                            strcpy(pathName,"SYS:Storage/DOSDrivers/");
                                            strncat(pathName,noColonName,sizeof(pathName));
                                            if (lock = Lock(pathName,ACCESS_READ))
                                            {
                                                UnLock(lock);
                                                bool = Mount(gd,noColonName,pathName,pathName,FALSE);
                                            }
                                            else if (IoErr() == ERROR_OBJECT_NOT_FOUND)
                                            {
                                                bool = Mount(gd,noColonName,"DEVS:MountList",NULL,TRUE);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        bool = Mount(gd,noColonName,(STRPTR)opts[OPT_FROM],NULL,TRUE);
                                    }

                                    if (!bool)
                                        failureLevel = RETURN_FAIL;
                                }
                                ptr++;
                            }
                        }
                        FreeArgs(rdargs);
                    }
                    else
                    {
                        PrintFault(IoErr(),NULL);
                    }
                }

                if (LocaleBase)
                {
                    CloseCatalog(gd->gd_LocaleInfo.li_Catalog);
                    CloseLibrary(LocaleBase);
                }

                CloseLibrary(IntuitionBase);
            }
            CloseLibrary(UtilityBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}


/*****************************************************************************/


VOID InitEnvironment(struct GlobalData *gd, ULONG *environment)
{
    gd->gd_UninitializedCnt = 5;
    gd->gd_InitStack        = TRUE;
    gd->gd_InitPriority     = TRUE;
    gd->gd_InitDosType      = TRUE;

    environment[ENV_VECTORSIZE]      = VECTORSIZE;
    environment[ENV_SECTORSIZE]      = 512;
    environment[ENV_SECTORIGIN]      = 0;
    environment[ENV_SECTORSPERBLOCK] = 1;
    environment[ENV_RESERVED]        = 2;
    environment[ENV_PREALLOC]        = 0;
    environment[ENV_INTERLEAVE]      = 0;
    environment[ENV_BUFFERS]         = 5;
    environment[ENV_BUFMEMTYPE]      = 3;
    environment[ENV_MAXTRANSFER]     = 0x7fffffff;
    environment[ENV_MASK]            = 0xffffffff;
    environment[ENV_BOOTPRI]         = 0;
    environment[ENV_DOSTYPE]         = 0x444F5300;
    environment[ENV_BAUD]            = 1200;
    environment[ENV_CONTROL]         = 0;
    environment[ENV_DEVICE]          = 0;
    environment[ENV_UNIT]            = 0;
    environment[ENV_FLAGS]           = 0;
    environment[ENV_HANDLER]         = 0;
    environment[ENV_STACKSIZE]       = 600;
    environment[ENV_PRIORITY]        = 10;
    environment[ENV_GLOBVEC]         = 2;   /* means use default */
    environment[ENV_FILESYSTEM]      = 0;
    environment[ENV_STARTUP]         = 0;
    environment[ENV_ACTIVATE]        = 0;
    environment[ENV_EHANDLER]        = 0;
    environment[ENV_FORCELOAD]       = 0;   /* means to try to get it from the filesystem resource first */
}


/*****************************************************************************/


VOID CleanupEnvironment(struct GlobalData *gd, ULONG *environment)
{
    FreeVec((APTR)(environment[ENV_HANDLER] * 4));
    FreeVec((APTR)(environment[ENV_FILESYSTEM] * 4));
    FreeVec((APTR)(environment[ENV_DEVICE] * 4));
    FreeVec((APTR)(environment[ENV_EHANDLER] * 4));
    FreeVec((APTR)(environment[ENV_CONTROL] * 4));

    if (gd->gd_UnitIsStr)
        FreeVec((APTR)(environment[ENV_UNIT]));

    if (gd->gd_StartupIsStr)
        FreeVec((APTR)(environment[ENV_STARTUP] * 4));
}


/*****************************************************************************/


BOOL ValidateEnvironment(struct GlobalData *gd, ULONG *environment)
{
    if (gd->gd_InitStack)
	if (environment[ENV_GLOBVEC] == -1)
            environment[ENV_STACKSIZE] = 2400;

    if (gd->gd_InitPriority)
        if (environment[ENV_HANDLER])
            environment[ENV_PRIORITY] = 5;

    if (gd->gd_InitDosType)
        if (environment[ENV_EHANDLER])
            environment[ENV_DOSTYPE] = 0;

    if ((environment[ENV_FILESYSTEM]) && (!environment[ENV_EHANDLER]))
    {
        if (gd->gd_UninitializedCnt)
        {
            return(CODE_MISSINGKEYWORDS);
        }
    }
    else
    {
        environment[ENV_FILESYSTEM] = environment[ENV_EHANDLER];
    }

    environment[ENV_MASK]       = environment[ENV_MASK] & 0xfffffffe;      /* bit 0 must be 0 */
    environment[ENV_BUFMEMTYPE] = environment[ENV_BUFMEMTYPE] | 1;         /* bit 0 must be 1 */
    environment[ENV_STACKSIZE]  = environment[ENV_STACKSIZE] & 0xfffffffc; /* must be long    */
    environment[ENV_SECTORSIZE] = environment[ENV_SECTORSIZE] >> 2;         /* turn into # of longs */

    return(CODE_SUCCESS);
}


/*****************************************************************************/


LONG GlobVec(LONG val, LONG def)
{
    if ((val >= -3) && (val <= 0))
        return(val);

    if (val == 2)
        return(def);

    return(-1);         /* No GV */
}


/*****************************************************************************/


UWORD CreateDosEntry(struct GlobalData *gd, ULONG *environment, STRPTR name)
/* 'name' should NOT be terminated by a colon */
{
struct DosList           *node;
char                      fixedName[30];
UWORD                     len;
struct FileSysStartupMsg *startup = NULL;
struct FileSysResource   *fsr;
struct FileSysEntry      *fse;
ULONG                    *vector = NULL;

    len = strlen(name);
    fixedName[len] = 0;
    while (len--)
        fixedName[len] = ToUpper(name[len]);

    if (node = MakeDosEntry(fixedName,DLT_DEVICE))
    {
        node->dol_Type                           = DLT_DEVICE;
        node->dol_misc.dol_handler.dol_StackSize = environment[ENV_STACKSIZE];
        node->dol_misc.dol_handler.dol_Priority  = environment[ENV_PRIORITY];

        if (environment[ENV_HANDLER])
        {
            /* create a stream handler */
            node->dol_misc.dol_handler.dol_Startup = environment[ENV_STARTUP];
            node->dol_misc.dol_handler.dol_Handler = environment[ENV_HANDLER];
            node->dol_misc.dol_handler.dol_GlobVec = GlobVec(environment[ENV_GLOBVEC],(LONG)gd->gd_Process->pr_GlobVec / 4);
        }
        else
        {
            /* create a file system handler */
            startup = AllocVec(sizeof(struct FileSysStartupMsg),MEMF_CLEAR|MEMF_PUBLIC);
            vector = AllocVec((VECTORSIZE+1)*4,MEMF_CLEAR|MEMF_PUBLIC);

            if ((!startup) || (!vector))
            {
                FreeVec(startup);
                FreeVec(vector);
                FreeDosEntry(node);
                return(CODE_NOMEMORY);
            }

            CopyMem(environment,vector,(VECTORSIZE+1)*4);

            startup->fssm_Unit    = environment[ENV_UNIT];
            startup->fssm_Device  = environment[ENV_DEVICE];
            startup->fssm_Environ = (BPTR)((ULONG)vector >> 2);
            startup->fssm_Flags   = environment[ENV_FLAGS];

            node->dol_misc.dol_handler.dol_Startup = (BPTR)((ULONG)startup >> 2);
            node->dol_misc.dol_handler.dol_GlobVec = GlobVec(environment[ENV_GLOBVEC],0);
            node->dol_misc.dol_handler.dol_Handler = environment[ENV_FILESYSTEM];

            if (!environment[ENV_FILESYSTEM])
                node->dol_misc.dol_handler.dol_SegList = DOSBase->dl_Root->rn_FileHandlerSegment;

            if (environment[ENV_FORCELOAD] == 0)
            {
                if (fsr = OpenResource("FileSystem.resource"))
                {
                    Forbid();

                    fse = (struct FileSysEntry *)fsr->fsr_FileSysEntries.lh_Head;
                    while (fse->fse_Node.ln_Succ)
                    {
                        if (fse->fse_DosType == environment[ENV_DOSTYPE])
                        {
                            if (fse->fse_PatchFlags & 0x0001)
                                node->dol_Type = fse->fse_Type;
                            if (fse->fse_PatchFlags & 0x0002)
                                node->dol_Task = (struct MsgPort *)fse->fse_Task;
                            if (fse->fse_PatchFlags & 0x0004)
                                node->dol_Lock = fse->fse_Lock;
                            if (fse->fse_PatchFlags & 0x0008)
                                node->dol_misc.dol_handler.dol_Handler = fse->fse_Handler;
                            if (fse->fse_PatchFlags & 0x0010)
                                node->dol_misc.dol_handler.dol_StackSize = fse->fse_StackSize;
                            if (fse->fse_PatchFlags & 0x0020)
                                node->dol_misc.dol_handler.dol_Priority = fse->fse_Priority;
                            if (fse->fse_PatchFlags & 0x0040)
                                node->dol_misc.dol_handler.dol_Startup = fse->fse_Startup;
                            if (fse->fse_PatchFlags & 0x0080)
                                node->dol_misc.dol_handler.dol_SegList = fse->fse_SegList;
                            if (fse->fse_PatchFlags & 0x0100)
                                node->dol_misc.dol_handler.dol_GlobVec = fse->fse_GlobalVec;
                            break;
                        }
                        fse = (struct FileSysEntry *)fse->fse_Node.ln_Succ;
                    }

                    Permit();
                }
            }
        }

        if (!AddDosEntry(node))
        {
            FreeDosEntry(node);
            FreeVec(startup);
            FreeVec(vector);
            return(CODE_ALREADYMOUNTED);
        }

        return(CODE_SUCCESS);
    }

    return(CODE_NOMEMORY);
}


/*****************************************************************************/


char GetCh(struct GlobalData *gd)
{
LONG num;

    num = FGetC(gd->gd_InputFile);
    if (num < 0)
        num = 0;

    if (gd->gd_LastChar == '\n')
    {
        gd->gd_InputLine++;
        gd->gd_InputColumn = 0;
    }
    gd->gd_InputColumn++;

    return(gd->gd_LastChar = (char)num);
}


/*****************************************************************************/


BOOL GetToken(struct GlobalData *gd)
{
char  ch, oldCh;
UWORD nest, tokenLen;

    tokenLen           = 0;
    ch                 = gd->gd_LastChar;
    gd->gd_TokenQuoted = FALSE;

    while ((ch == '\t') || (ch == ' ') || (ch == '\n') || (ch == ';'))
    {
        do
        {
            ch = GetCh(gd);
        }
        while ((ch == '\t') || (ch == ' ') || (ch == '\n') || (ch == ';'));

        if (ch == '/')
        {
            gd->gd_TokenLine   = gd->gd_InputLine;
            gd->gd_TokenColumn = gd->gd_InputColumn;

            ch = GetCh(gd);
            if (ch == '*')
            {
                oldCh = ' ';
                nest  = 1;
                do
                {
                    ch = GetCh(gd);

                    if ((ch == '/') && (oldCh == '*'))
                    {
                        nest--;
                        oldCh = ' ';
                    }
                    else if ((ch == '*') && (oldCh == '/'))
                    {
                        nest++;
                        oldCh = ' ';
                    }
                    else
                    {
                        oldCh = ch;
                    }
                }
                while (nest && ch);
                ch = GetCh(gd);
            }
            else
            {
                gd->gd_CurrentToken[0] = '/';
                tokenLen++;
            }
        }
    }

    if (tokenLen == 0)
    {
        gd->gd_TokenLine   = gd->gd_InputLine;
        gd->gd_TokenColumn = gd->gd_InputColumn;
    }

    if (ch == '\"')
    {
        ch = GetCh(gd);
        while ((ch != '\"') && (ch != '\n') && (ch != 0))
        {
            gd->gd_CurrentToken[tokenLen++] = ch;
            ch = GetCh(gd);
        }

        if (ch == '\"')
        {
            GetCh(gd);
            gd->gd_TokenQuoted = TRUE;
        }
        gd->gd_TokenType = TOK_STRING;
    }
    else if (ch == '=')
    {
        gd->gd_CurrentToken[0] = '=';
        tokenLen = 1;
        GetCh(gd);    /* keep mr.lookahead happy! */
        gd->gd_TokenType = TOK_STRING;
    }
    else
    {
        while ((ch != '\t') && (ch != ' ') && (ch != '\n') && (ch != ';') && (ch != '=') && (ch != 0))
        {
            gd->gd_CurrentToken[tokenLen++] = ch;
            ch = GetCh(gd);
        }
        gd->gd_TokenType = TOK_UNDECIDED;
    }
    gd->gd_CurrentToken[tokenLen] = 0;

    return((tokenLen > 0) || gd->gd_TokenQuoted);
}


/*****************************************************************************/


BOOL ConvToNum(struct GlobalData *gd, STRPTR string, ULONG *value)
{
char  c;
SHORT i;
BOOL  negate;
UWORD base;

    i    = 0;
    base = 10;

    if (negate = (string[0] == '-'))
        i++;

    if ((string[i] == '0') && (ToUpper(string[i+1]) == 'X'))
    {
        base = 16;
        i    = i+2;
    }

    *value = 0;
    for (;;)
    {
        c = ToUpper(string[i++]);
        if ((c >= '0') && (c <= '9'))
        {
            *value = *value*base + c - '0';
        }
        else if (((c >= 'A') && (c <= 'F')) && (base == 16))
        {
            *value = *value*16 + c - 'A' + 10;
        }
        else if (c != 0)
        {
            return(FALSE);
        }
        else
        {
            if (negate)
                *value = -(*value);

            return(TRUE);
        }
    }
}


/*****************************************************************************/


UWORD ProcessKeywordArguments(struct GlobalData *gd, ULONG *environment,
                              UWORD keyword)
{
STRPTR str;
STRPTR bstr;
LONG   num;
UWORD  len;
UWORD  chars;

    str = gd->gd_CurrentToken;
    if ((gd->gd_TokenType == TOK_UNDECIDED) && (!gd->gd_TokenQuoted))
    {
        if (ConvToNum(gd,str,&num))
        {
            str = NULL;
            gd->gd_TokenType = TOK_NUMBER;
        }
        else
        {
            gd->gd_TokenType = TOK_STRING;
        }
    }

    switch (keyword)
    {
        case ENV_DEVICE         : if (gd->gd_UninitializedCnt)
                                      gd->gd_UninitializedCnt--;
        case ENV_FILESYSTEM     : ;
        case ENV_CONTROL        : ;
        case ENV_HANDLER        : ;
        case ENV_EHANDLER       : ;
        case ENV_STARTUP        : if (keyword == ENV_STARTUP)
                                  {
                                      if (gd->gd_StartupIsStr)
                                          FreeVec((APTR)(environment[ENV_STARTUP] * 4));

                                      gd->gd_StartupIsStr = (str != NULL);
                                  }
                                  else
                                  {
                                      FreeVec((APTR)(environment[keyword] * 4));
                                  }
                                  environment[keyword] = 0;

                                  if (str)
                                  {
                                      if (str[0] == '(')
                                      {
                                          chars = StrToLong(&str[1],&num)+1;
                                          if (str[chars] == ',')
                                              chars++;

                                          len = strlen(&str[chars]);
                                          if ((len > 0) && (str[len-1] == ')'))
                                              len--;

                                          if (bstr = AllocVec(num+2,MEMF_PUBLIC|MEMF_CLEAR))
                                          {
                                              CopyMem(str,&bstr[1],len);
                                              bstr[0] = num;
                                              environment[keyword] = (LONG)bstr >> 2;
                                          }
                                          else
                                          {
                                              return(CODE_NOMEMORY);
                                          }
                                      }
                                      else if (gd->gd_TokenQuoted && ((keyword == ENV_STARTUP) || (keyword == ENV_CONTROL)))
                                      {
                                          len = strlen(str);
                                          if (bstr = AllocVec(len+4,MEMF_PUBLIC|MEMF_CLEAR))
                                          {
                                              CopyMem(str,&bstr[2],len);
                                              bstr[0] = len+2;
                                              bstr[1] = '\"';
                                              bstr[len+2] = '\"';
                                              environment[keyword] = (LONG)bstr >> 2;
                                          }
                                          else
                                          {
                                              return(CODE_NOMEMORY);
                                          }
                                      }
                                      else
                                      {
                                          len = strlen(str);
                                          if (bstr = AllocVec(len+2,MEMF_PUBLIC|MEMF_CLEAR))
                                          {
                                              CopyMem(str,&bstr[1],len);
                                              bstr[0] = len;
                                              environment[keyword] = (LONG)bstr >> 2;
                                          }
                                          else
                                          {
                                              return(CODE_NOMEMORY);
                                          }
                                      }
                                      break;
                                  }
                                  else if (keyword != ENV_STARTUP)
                                  {
                                      return(CODE_STRINGEXPECTED);
                                  }

        /* numeric ENV_STARTUP falls through */

        case ENV_UNIT           : if (keyword == ENV_UNIT)
                                  {
                                      if (gd->gd_UnitIsStr)
                                      {
                                          FreeVec((APTR)(environment[ENV_UNIT]));
                                          environment[ENV_UNIT] = 0;
                                      }

                                      gd->gd_UnitIsStr = (str != NULL);

                                      if (str)
                                      {
                                          len = strlen(str);
                                          if (environment[ENV_UNIT] = (ULONG)AllocVec(len+1,MEMF_PUBLIC|MEMF_CLEAR))
                                          {
                                              CopyMem(str,(APTR)environment[ENV_UNIT],len);
                                              break;
                                          }
                                          return(CODE_NOMEMORY);
                                      }
                                  }

        /* numeric ENV_STARTUP falls through */
        /* numeric ENV_UNIT falls through */

        case ENV_SURFACES       : ;
        case ENV_SECTORSPERTRACK: ;
        case ENV_LOWCYL         : ;
        case ENV_HIGHCYL        : if ((keyword != ENV_STARTUP) && (keyword != ENV_UNIT) && (gd->gd_UninitializedCnt))
                                      gd->gd_UninitializedCnt--;

        case ENV_SECTORSPERBLOCK: ;
        case ENV_RESERVED       : ;
        case ENV_SECTORSIZE     : ;
        case ENV_PREALLOC       : ;
        case ENV_INTERLEAVE     : ;
        case ENV_BUFFERS        : ;
        case ENV_BUFMEMTYPE     : ;
        case ENV_MAXTRANSFER    : ;
        case ENV_MASK           : ;
        case ENV_BOOTPRI        : ;
        case ENV_DOSTYPE        : ;
        case ENV_BAUD           : ;
        case ENV_FLAGS          : ;
        case ENV_STACKSIZE      : ;
        case ENV_PRIORITY       : ;
        case ENV_GLOBVEC        : ;
        case ENV_ACTIVATE       : ;
        case ENV_FORCELOAD      : if (keyword == ENV_STACKSIZE)
                                      gd->gd_InitStack = FALSE;
                                  else if (keyword == ENV_PRIORITY)
                                      gd->gd_InitPriority = FALSE;
                                  else if (keyword == ENV_DOSTYPE)
                                      gd->gd_InitDosType = FALSE;

                                  if (str)
                                      return(CODE_NUMBEREXPECTED);

                                  environment[keyword] = num;
                                  break;
    }

    return(CODE_SUCCESS);
}


/*****************************************************************************/


VOID PrintF(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... )
{
struct EasyStruct est;

    if (gd->gd_Process->pr_CLI)
    {
        VPrintf(GetString(&gd->gd_LocaleInfo,str),(LONG *)&arg1);
    }
    else
    {
        est.es_StructSize   = sizeof(struct EasyStruct);
        est.es_Flags        = 0;
        est.es_Title        = GetString(&gd->gd_LocaleInfo,MSG_MT_REQ_TITLE);
        est.es_TextFormat   = GetString(&gd->gd_LocaleInfo,str);
        est.es_GadgetFormat = GetString(&gd->gd_LocaleInfo,MSG_MT_OK_GAD);

        EasyRequestArgs(NULL,&est,NULL,&arg1);
    }
}


/*****************************************************************************/


/* device name must NOT have a colon! */
BOOL Mount(struct GlobalData *gd, STRPTR device, STRPTR fileName,
           STRPTR iconName, BOOL needDevice)
{
BOOL               foundDev;
UBYTE              state;
ULONG              environment[ENV_NUMSLOTS];
WORD               keyword;
UWORD              code;
UWORD              line;
UWORD              column;
char               key[30];
char              *ptr;
UWORD              i;
struct Library    *IconBase;
struct DiskObject *diskObj;
STRPTR            *ttypes;
STRPTR             argument;
char               colonName[40];
UWORD              len;
char               ch;

    gd->gd_AlreadyMounted = FALSE;

    strcpy(colonName,device);
    strcat(colonName,":");

    if (!(gd->gd_InputFile = Open(fileName,MODE_OLDFILE)))
    {
        PrintF(gd,MSG_MT_OPEN,fileName);
        return(FALSE);
    }

    InitEnvironment(gd,environment);

    gd->gd_FileName    = fileName;
    gd->gd_InputLine   = 1;
    gd->gd_InputColumn = 0;
    gd->gd_LastChar    = ' ';

    foundDev = TRUE;
    code     = CODE_SUCCESS;
    state    = SCAN_KEYWORD;

    if (needDevice)
    {
        state    = SCAN_DEVICE;
        foundDev = FALSE;
    }

    while ((code == CODE_SUCCESS) && (state != SCAN_DONE) && GetToken(gd))
    {
        switch (state)
        {
            case SCAN_DEVICE  : if (Stricmp(gd->gd_CurrentToken,colonName) == 0)
                                    foundDev = TRUE;

                                state = SCAN_KEYWORD;
                                break;

            case SCAN_KEYWORD : if (Stricmp(gd->gd_CurrentToken,"#") == 0)
                                {
                                    if (foundDev)
                                        state = SCAN_DONE;
                                    else
                                        state = SCAN_DEVICE;
                                    break;
                                }

                                if (foundDev)
                                {
                                    keyword = FindArg(Keywords,gd->gd_CurrentToken);
                                    if (keyword < 0)
                                    {
                                        code = CODE_UNKNOWNKEYWORD;
                                    }
                                    state = SCAN_EQUAL;
                                }
                                break;

            case SCAN_EQUAL   : if (Stricmp(gd->gd_CurrentToken,"=") != 0)
                                    code = CODE_EQUALEXPECTED;

                                state = SCAN_ARGUMENT;
                                break;

            case SCAN_ARGUMENT: if (foundDev)
                                    code = ProcessKeywordArguments(gd,environment,keyword);

                                state = SCAN_KEYWORD;
                                break;
        }
    }

    Close(gd->gd_InputFile);

    diskObj  = NULL;
    IconBase = NULL;
    if (iconName)
    {
        if (IconBase = OpenLibrary("icon.library",37))
        {
            diskObj = GetDiskObject(iconName);
        }
    }

    if (diskObj && (code == CODE_SUCCESS))
    {
        ttypes = (STRPTR *)diskObj->do_ToolTypes;
        while ((argument = *ttypes++) && (code == CODE_SUCCESS))
        {
            i = 0;
            while ((argument[i] != 0) && (argument[i] != '=') && (i < sizeof(key)))
            {
                key[i] = argument[i];
                if (key[i] == ' ')
                    key[i] = 0;
                i++;
            }
            key[i] = 0;

            keyword = FindArg(Keywords,key);
            if (keyword < 0)
            {
                code = CODE_UNKNOWNKEYWORD;
                break;
            }

            if (argument[i] != '=')
            {
                code = CODE_EQUALEXPECTED;
                break;
            }

            i++;
            while (argument[i] == ' ')
                i++;
            strncpy(gd->gd_CurrentToken,&argument[i],sizeof(gd->gd_CurrentToken)-1);

            ch  = gd->gd_CurrentToken[0];

            if (ch == '\"')
            {
                len = 0;
                ch  = gd->gd_CurrentToken[1];
                while ((ch != '\"') && (ch != 0))
                {
                    gd->gd_CurrentToken[len] = ch;
                    ch = gd->gd_CurrentToken[++len];
                }
                gd->gd_CurrentToken[len] = 0;
                gd->gd_TokenType = TOK_STRING;
            }
            else
            {
                gd->gd_TokenType = TOK_UNDECIDED;
            }

            code = ProcessKeywordArguments(gd,environment,keyword);
        }
        code = CODE_SUCCESS;   /* don't complain about bad tooltypes, its not polite */
    }

    if (IconBase)
    {
        if (diskObj)
        {
            FreeDiskObject(diskObj);
        }
        CloseLibrary(IconBase);
    }

    if (code == CODE_SUCCESS)
    {
        if (!foundDev)
            code = CODE_DEVICENOTFOUND;

        if (state == SCAN_EQUAL)
            code = CODE_EQUALEXPECTED;

        else if (state == SCAN_ARGUMENT)
            code = CODE_ARGUMENTEXPECTED;

        else if (code == CODE_SUCCESS)
        {
            code = ValidateEnvironment(gd,environment);
            if (code == CODE_SUCCESS)
            {
                code = CreateDosEntry(gd,environment,device);
                if (code == CODE_SUCCESS)
                    if (environment[ENV_ACTIVATE])
                        DeviceProc(colonName);
            }
        }
    }

    if (code != CODE_SUCCESS)
    {
        line   = gd->gd_TokenLine;
        column = gd->gd_TokenColumn;

        if (gd->gd_Process->pr_CLI)
            PrintF(gd,MSG_MT_ERROR,NULL);

        ptr = Keywords;
        while (keyword--)
        {
            while ((*ptr != ',') && (*ptr))
                ptr++;

            if (*ptr)
                ptr++;
        }

        if (*ptr)
        {
            i = 0;
            while ((*ptr != ',') && (*ptr))
                key[i++] = *ptr++;

            key[i] = 0;
        }

        switch (code)
        {
            case CODE_NOMEMORY         : PrintF(gd,MSG_MT_NOMEM,NULL);
                                         break;

            case CODE_ALREADYMOUNTED   : PrintF(gd,MSG_MT_ALREADY_MOUNTED,colonName);
                                         gd->gd_AlreadyMounted = TRUE;
                                         break;

            case CODE_STRINGEXPECTED   : PrintF(gd,MSG_MT_STRING_EXPECTED,key,fileName,line,column);
                                         break;

            case CODE_NUMBEREXPECTED   : PrintF(gd,MSG_MT_NUMBER_EXPECTED,key,fileName,line,column);
                                         break;

            case CODE_EQUALEXPECTED    : PrintF(gd,MSG_MT_EQUAL_EXPECTED,fileName,line,column);
                                         break;

            case CODE_ARGUMENTEXPECTED : PrintF(gd,MSG_MT_ARGUMENT_EXPECTED,key,fileName,line,column);
                                         break;

            case CODE_MISSINGKEYWORDS  : PrintF(gd,MSG_MT_MISSING_KEYWORDS,NULL);
                                         break;

            case CODE_UNKNOWNKEYWORD   : PrintF(gd,MSG_MT_UNKNOWN_KEYWORD,gd->gd_CurrentToken,fileName,line,column);
                                         break;

            case CODE_DEVICENOTFOUND   : PrintF(gd,MSG_MT_DEVICE_NOT_FOUND,colonName,fileName);
                                         break;

        }

        CleanupEnvironment(gd,environment);
    }

    return(code == CODE_SUCCESS);
}
