
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/doshunks.h>
#include <dos/stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "texttable.h"
#include "memloadseg.h"
#include "makewb_rev.h"


/*****************************************************************************/


#define TEMPLATE          "BUILDLIST/A,TO,LOGFILE/K,SYMBOL/M,QUIET/S,MAKEARCS/S,TESTBUILDLIST/S" VERSTAG
#define OPT_BUILDLIST     0
#define OPT_TO            1
#define OPT_LOGFILE       2
#define OPT_SYMBOL        3
#define OPT_QUIET         4
#define OPT_MAKEARCS      5
#define OPT_TESTBUILDLIST 6
#define OPT_COUNT         7


/*****************************************************************************/


#define BOOLEAN UBYTE

#define TOK_NUMBER  0
#define TOK_STRING  1
#define TOK_NOTHING 2


struct GlobalData
{
    struct Library    *gd_DOSBase;
    struct Library    *gd_UtilityBase;
    struct Library    *gd_SysBase;
    struct Library    *gd_IconBase;
    struct LocaleInfo  gd_LocaleInfo;

    /* mount list parser stuff */
    BPTR               gd_InputFile;
    UWORD              gd_InputLine;
    UWORD              gd_InputColumn;
    UWORD              gd_TokenLine;
    UWORD              gd_TokenColumn;
    UWORD              gd_TokenType;
    char               gd_CurrentToken[256];
    char	       gd_LastChar;
    UBYTE              gd_Pad;  /* long align */

    struct FileInfoBlock gd_FIB;   /* long aligned! */

    /* global sorta stuff */
    struct MinList     gd_Symbols;     /* symbols defined on the cmd-line */
    struct DateStamp   gd_Now;         /* date & time of program execution    */
    STRPTR             gd_To;
    BOOL               gd_Quiet;
    BOOL               gd_MakeArcs;
    BOOL	       gd_TestMode;

    BPTR               gd_LhASegment;
    char               gd_LhACmdLine[512];

    BPTR               gd_LogFile;
    STRPTR             gd_LogFileName;

    /* stuff for StripSymbols() */
    ULONG             *gd_LoadFile;
    LONG               gd_LFLongs;
    LONG               gd_LFReadIndex;
    LONG               gd_LFWriteIndex;
    BOOL               gd_StripError;
    BOOL               gd_WasExecutable;

    /* item state */
    BOOL               gd_DoItem;
    struct DateStamp   gd_Date;
    LONG               gd_IconX;
    LONG               gd_IconY;
    WORD               gd_DrawerX;
    WORD               gd_DrawerY;
    WORD               gd_DrawerW;
    WORD               gd_DrawerH;
    ULONG              gd_Protection;
    STRPTR             gd_Source;
    STRPTR             gd_Destination;
    STRPTR             gd_Comment;
    STRPTR             gd_Disk;
    STRPTR             gd_DefaultDisk;
    STRPTR             gd_RCSTag;
    BOOL               gd_GotProtection;
    WORD               gd_Pad2;
};

/* The above structure MUST be an even longword multiple in size */

#define SysBase       gd->gd_SysBase
#define DOSBase       gd->gd_DOSBase
#define UtilityBase   gd->gd_UtilityBase
#define IconBase      gd->gd_IconBase
#define LocaleBase    gd->gd_LocaleInfo.li_LocaleBase


/*****************************************************************************/


static char Keywords[] = "SOURCE,DESTINATION,#CREATEDIR,PROTECTION,DATE,TIME,COMMENT,"\
                         "ICONPOS,DRAWERPOS,DRAWERSIZE,IFDEF,IFNDEF,"\
                         "#COPYFILE,#DEFINE,#UNDEF,DISK,#DEFAULTDISK,RCSTAG";

#define KEY_SOURCE         0
#define KEY_DESTINATION    1
#define KEY_CREATEDIR      2
#define KEY_PROTECTION     3
#define KEY_DATE           4
#define KEY_TIME           5
#define KEY_COMMENT        6
#define KEY_ICONPOS        7
#define KEY_DRAWERPOS      8
#define KEY_DRAWERSIZE     9
#define KEY_IFDEF          10
#define KEY_IFNDEF         11
#define KEY_COPYFILE       12
#define KEY_DEFINE         13
#define KEY_UNDEF          14
#define KEY_DISK           15
#define KEY_DEFAULTDISK    16
#define KEY_RCSTAG         17


/*****************************************************************************/


/* different possible parsing states */
#define SCAN_KEYWORD  0
#define SCAN_EQUAL    1
#define SCAN_ARGUMENT 2
#define SCAN_DONE     3


/* possible failure codes returned by ProcessKeywordArguments() */
#define CODE_SUCCESS           0
#define CODE_DOITEMERROR       1
#define CODE_NOMEMORY          2
#define CODE_STRINGEXPECTED    3
#define CODE_NUMBERSEXPECTED   4
#define CODE_EQUALEXPECTED     5
#define CODE_ARGUMENTEXPECTED  6
#define CODE_UNKNOWNKEYWORD    7
#define CODE_BADDATE           8
#define CODE_BADTIME           9
#define CODE_BADPROTECTION     10
#define CODE_NOTWITHDIRECTORY  11
#define CODE_NOSOURCE          12
#define CODE_NODESTINATION     13
#define CODE_SYMBOLNOTFOUND    14
#define CODE_ABORTED           15


/*****************************************************************************/


/* This structure must only be allocated by version.library and is READ-ONLY! */
struct VersionInfo
{
    ULONG            vi_Flags;        /* see flag bit definitions below      */
    ULONG            vi_Location;     /* where this info comes from          */
    LONG             vi_Version;      /* version number, -1 when unknown     */
    LONG             vi_Revision;     /* revision number, -1 when unknown    */
    struct DateStamp vi_Date;         /* date, fields set to -1 when unknown */
};

/* constants for VersionInfo.vi_Location and for the GV_Location tag */
#define VILOCB_EXECRESIDENT  0  /* Came from the Exec resident module list */
#define VILOCB_DOSRESIDENT   1  /* Came from the DOS resident segment list */
#define VILOCB_FILESYSTEM    2  /* Came from the DOS file system list      */
#define VILOCB_LOADEDLIBRARY 3  /* Came from the Exec library list         */
#define VILOCB_LOADEDDEVICE  4  /* Came from the Exec device list          */
#define VILOCB_PATHNAME      5  /* Came from a file                        */
#define VILOCB_LIBSASSIGN    6  /* Came from a file in LIBS:               */
#define VILOCB_DEVSASSIGN    7  /* Came from a file in DEVS:               */

#define VILOCF_EXECRESIDENT  (1L << 0)
#define VILOCF_DOSRESIDENT   (1L << 1)
#define VILOCF_FILESYSTEM    (1L << 2)
#define VILOCF_LOADEDLIBRARY (1L << 3)
#define VILOCF_LOADEDDEVICE  (1L << 4)
#define VILOCF_PATH          (1L << 5)
#define VILOCF_LIBSASSIGN    (1L << 6)
#define VILOCF_DEVSASSIGN    (1L << 7)


/* Flag bits for VersionInfo.vi_Flags */
#define VIB_FOUNDITEM    0        /* found an object matching the name */
#define VIB_FOUNDINFO    1        /* found some version information    */
#define VIB_VERSION      3        /* found a version number            */
#define VIB_REVISION     4        /* found a revision number           */
#define VIB_DATE         5        /* found a date                      */

#define VIF_FOUNDITEM    (1L << 0)
#define VIF_FOUNDINFO    (1L << 1)
#define VIF_VERSION      (1L << 3)
#define VIF_REVISION     (1L << 4)
#define VIF_DATE         (1L << 5)


/*****************************************************************************/


static BOOL MakeWB(struct GlobalData *gd, STRPTR buildList, STRPTR destDir);
static VOID NewList(struct List *list);
static BOOL AddSymbol(struct GlobalData *gd, STRPTR symbol);
static BOOL RemoveSymbol(struct GlobalData *gd, STRPTR symbol);
static VOID PrintIoErr(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... );
VOID kprintf(STRPTR,...);


/*****************************************************************************/


LONG main(VOID)
{
LONG               opts[OPT_COUNT];
struct RDArgs     *rdargs;
LONG               failureLevel = RETURN_FAIL;
STRPTR            *symbol;
struct GlobalData  global;
struct GlobalData *gd;
struct Node       *node;
char               date[50];
char               time[50];
struct DateTime    dt;

    gd = &global;
    memset(gd,0,sizeof(struct GlobalData));

    SysBase = (*((struct Library **) 4));

    if (DOSBase = OpenLibrary("dos.library",37))
    if (UtilityBase = OpenLibrary("utility.library",37))
    if (IconBase = OpenLibrary("icon.library",37))
    {
        if (LocaleBase = OpenLibrary("locale.library",38))
            gd->gd_LocaleInfo.li_Catalog = OpenCatalogA(NULL,"makewb.catalog",NULL);

        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
        {
            failureLevel = RETURN_OK;

            NewList((struct List *)&gd->gd_Symbols);
            DateStamp(&gd->gd_Now);
            gd->gd_To = (STRPTR)opts[OPT_TO];
            gd->gd_Quiet = (BOOL)opts[OPT_QUIET];
            gd->gd_MakeArcs = (BOOL)opts[OPT_MAKEARCS];
            gd->gd_TestMode = (BOOL)opts[OPT_TESTBUILDLIST];

            if (symbol = (STRPTR *)opts[OPT_SYMBOL])
            {
                while (*symbol)
                {
                    if (!AddSymbol(gd,*symbol))
                    {
                        failureLevel = RETURN_FAIL;
                        break;
                    }
                    symbol++;
                }
            }

            if (failureLevel == RETURN_OK)
            {
                if (gd->gd_LogFileName = (STRPTR)opts[OPT_LOGFILE])
                {
                    if (!(gd->gd_LogFile = Open(gd->gd_LogFileName,MODE_NEWFILE)))
                    {
                        failureLevel = RETURN_FAIL;
                        PrintIoErr(gd,MSG_CANT_OPEN_LOG,gd->gd_LogFileName);
                    }
                    else
                    {
                        SetVBuf(gd->gd_LogFile,NULL,BUF_FULL,4096);
                        FPuts(gd->gd_LogFile,"MakeWB Log File\n\n");
                        FPuts(gd->gd_LogFile,"BuildList      : ");
                        FPuts(gd->gd_LogFile,(STRPTR)opts[OPT_BUILDLIST]);
                        FPuts(gd->gd_LogFile,"\nDate Generated : ");

                        dt.dat_Format  = FORMAT_DOS;
                        dt.dat_StrDay  = NULL;
                        dt.dat_StrDate = date;
                        dt.dat_StrTime = time;
                        dt.dat_Flags   = 0;
                        dt.dat_Stamp   = gd->gd_Now;
                        DateToStr(&dt);

                        FPuts(gd->gd_LogFile,date);
                        FPuts(gd->gd_LogFile," ");
                        FPuts(gd->gd_LogFile,time);

                        FPuts(gd->gd_LogFile,"\nSymbols        : ");
                        node = (struct Node *)gd->gd_Symbols.mlh_Head;
                        while (node->ln_Succ)
                        {
                            FPuts(gd->gd_LogFile,node->ln_Name);
                            if (node = node->ln_Succ)
                                FPuts(gd->gd_LogFile,", ");
                        }
                        FPuts(gd->gd_LogFile,"\n\n--------------------------------------------------------------------------\n\n");
                    }
                }
            }

            if (failureLevel == RETURN_OK)
            {
                if (failureLevel == RETURN_OK)
                {
                    if (!MakeWB(gd,(STRPTR)opts[OPT_BUILDLIST],(STRPTR)opts[OPT_TO]))
                        failureLevel = RETURN_FAIL;
                }

                while (node = RemHead((struct List *)&gd->gd_Symbols))
                    FreeVec(node);

                if (gd->gd_LogFile)
                    Close(gd->gd_LogFile);
            }

            FreeArgs(rdargs);
        }
        else
        {
            PrintFault(IoErr(),NULL);
        }
    }

    if (DOSBase)
    {
        if (LocaleBase)
            CloseCatalog(gd->gd_LocaleInfo.li_Catalog);

        CloseLibrary(LocaleBase);
        CloseLibrary(IconBase);
        CloseLibrary(UtilityBase);
        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


static BOOL AddSymbol(struct GlobalData *gd, STRPTR symbol)
{
struct Node *node;

    if (node = AllocVec(sizeof(struct Node)+strlen(symbol)+1,MEMF_ANY))
    {
        node->ln_Name = (STRPTR)((ULONG)node + sizeof(struct Node));
        strcpy(node->ln_Name,symbol);
        AddTail((struct List *)&gd->gd_Symbols,node);
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static BOOL RemoveSymbol(struct GlobalData *gd, STRPTR symbol)
{
struct Node *node;

    if (node = FindName((struct List *)&gd->gd_Symbols,symbol))
    {
        Remove(node);
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static BOOL FindSymbol(struct GlobalData *gd, STRPTR symbol)
{
    if (FindName((struct List *)&gd->gd_Symbols,symbol))
        return(TRUE);

    return(FALSE);
}


/*****************************************************************************/


static char GetCh(struct GlobalData *gd)
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


static BOOL GetToken(struct GlobalData *gd)
{
char  ch, oldCh;
UWORD nest, tokenLen;

    tokenLen = 0;
    ch       = gd->gd_LastChar;

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

    gd->gd_TokenType = TOK_STRING;
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
            gd->gd_CurrentToken[tokenLen] = 0;
            return(TRUE);
        }
    }
    else if (ch == '=')
    {
        gd->gd_CurrentToken[0] = '=';
        tokenLen = 1;
        GetCh(gd);    /* keep mr.lookahead happy! */
    }
    else
    {
        if (((ch >= '0') && (ch <= '9')) || (ch == '-'))
            gd->gd_TokenType = TOK_NUMBER;

        while ((ch != '\t') && (ch != ' ') && (ch != '\n') && (ch != ';') && (ch != '=') && (ch != 0))
        {
            gd->gd_CurrentToken[tokenLen++] = ch;
            ch = GetCh(gd);
        }
    }
    gd->gd_CurrentToken[tokenLen] = 0;

    return((BOOL)(tokenLen > 0));
}


/*****************************************************************************/


static VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}


/*****************************************************************************/


static VOID PrintF(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... )
{
    VPrintf(GetString(&gd->gd_LocaleInfo,str),(LONG *)&arg1);
}


/*****************************************************************************/


static VOID PrintIoErr(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... )
{
ULONG ioerr = IoErr();

    PrintF(gd,MSG_ERROR,NULL);
    VPrintf(GetString(&gd->gd_LocaleInfo,str),(LONG *)&arg1);
    PrintFault(ioerr," - ");
    PutStr("\n");
}


/*****************************************************************************/


static VOID PrintIoErrLF(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... )
{
ULONG ioerr = IoErr();

    if (!gd->gd_Quiet)
        PutStr("\n");

    PrintF(gd,MSG_ERROR,NULL);
    VPrintf(GetString(&gd->gd_LocaleInfo,str),(LONG *)&arg1);
    PrintFault(ioerr," - ");
    PutStr("\n");
}


/*****************************************************************************/


struct LockList
{
    BPTR NextPath;
    BPTR PathLock;
};


static BPTR LoadSegPath(struct GlobalData *gd, STRPTR name)
{
struct CommandLineInterface *cli;
struct LockList             *node;
BPTR                         seg;
BPTR                         oldCD;

    cli  = Cli();
    node = (struct LockList *)((ULONG)cli->cli_CommandDir*4);
    seg  = NULL;

    while (node && !seg)
    {
        oldCD = CurrentDir(node->PathLock);
        seg = LoadSeg(name);
        CurrentDir(oldCD);
        node = (struct LockList *)((ULONG)node->NextPath*4);
    }

    return(seg);
}


/*****************************************************************************/


static VOID InitEnvironment(struct GlobalData *gd)
{
    FreeVec(gd->gd_Source);
    FreeVec(gd->gd_Destination);
    FreeVec(gd->gd_Comment);
    FreeVec(gd->gd_Disk);
    FreeVec(gd->gd_RCSTag);

    gd->gd_DoItem	 = TRUE;
    gd->gd_GotProtection = FALSE;
    gd->gd_Date	         = gd->gd_Now;
    gd->gd_IconX	 = NO_ICON_POSITION;
    gd->gd_IconY	 = NO_ICON_POSITION;
    gd->gd_DrawerX	 = 0;
    gd->gd_DrawerY	 = 0;
    gd->gd_DrawerW	 = 100;
    gd->gd_DrawerH	 = 100;
    gd->gd_Protection	 = FIBF_OTR_READ | FIBF_OTR_WRITE | FIBF_OTR_EXECUTE | FIBF_GRP_READ | FIBF_GRP_WRITE | FIBF_GRP_EXECUTE;
    gd->gd_Source	 = NULL;
    gd->gd_Destination	 = NULL;
    gd->gd_Comment	 = NULL;
    gd->gd_Disk          = NULL;
    gd->gd_RCSTag        = NULL;
}


/*****************************************************************************/


static VOID GetLong(struct GlobalData *gd, LONG *l)
{
    if (!gd->gd_StripError && (gd->gd_LFReadIndex < gd->gd_LFLongs))
        *l = gd->gd_LoadFile[gd->gd_LFReadIndex++];
    else
        gd->gd_StripError = TRUE;
}


/*****************************************************************************/


static VOID SkipLongs(struct GlobalData *gd, ULONG numLongs)
{
    if (!gd->gd_StripError && (gd->gd_LFReadIndex + numLongs <= gd->gd_LFLongs))
        gd->gd_LFReadIndex += numLongs;
    else
        gd->gd_StripError = TRUE;
}


/*****************************************************************************/


static VOID PutLong(struct GlobalData *gd, LONG l)
{
    if (!gd->gd_StripError && (gd->gd_LFWriteIndex < gd->gd_LFReadIndex))
        gd->gd_LoadFile[gd->gd_LFWriteIndex++] = l;
    else
        gd->gd_StripError = TRUE;
}


/*****************************************************************************/


static VOID CopyLongs(struct GlobalData *gd, ULONG numLongs)
{
    if (!gd->gd_StripError && (gd->gd_LFReadIndex + numLongs <= gd->gd_LFLongs))
    {
        CopyMemQuick(&gd->gd_LoadFile[gd->gd_LFReadIndex],&gd->gd_LoadFile[gd->gd_LFWriteIndex],numLongs * 4);
        gd->gd_LFReadIndex  += numLongs;
        gd->gd_LFWriteIndex += numLongs;
    }
    else
    {
        gd->gd_StripError = TRUE;
    }
}


/*****************************************************************************/


#define	TYPEMASK 0x3fffffff


static LONG StripSymbols(struct GlobalData *gd, APTR buffer, LONG bufSize)
{
LONG hunkLong, hunkType, size, ext;

    gd->gd_LoadFile      = buffer;
    gd->gd_LFLongs       = bufSize / 4;
    gd->gd_LFReadIndex   = 0;
    gd->gd_LFWriteIndex  = 0;
    gd->gd_StripError    = FALSE;
    gd->gd_WasExecutable = FALSE;

    GetLong(gd,&hunkLong);
    if (gd->gd_StripError)
        return(bufSize);

    if ((hunkLong & TYPEMASK) != HUNK_HEADER)
        return(bufSize);

    gd->gd_WasExecutable = TRUE;

    while (TRUE)
    {
        hunkType = hunkLong & TYPEMASK;

	switch (hunkType)
        {
	    case HUNK_UNIT    :
	    case HUNK_NAME    : /* warning, skipping these as well! */
	    case HUNK_DEBUG   : GetLong(gd,&size);
                                SkipLongs(gd,size);
                                break;

	    case HUNK_CODE    :
	    case HUNK_DATA    : PutLong(gd,hunkLong);
                                GetLong(gd,&size);
                                PutLong(gd,size);
                                CopyLongs(gd,size);
                                break;

	    case HUNK_BSS     : PutLong(gd,hunkLong);
                                CopyLongs(gd,1);
                                break;

	    case HUNK_RELOC32 : PutLong(gd,hunkType);
                                do
                                {
                                    GetLong(gd,&size);
                                    PutLong(gd,size);
                                    if (size != 0)
                                        CopyLongs(gd,size+1);
                                }
                                while ((size != 0) && !gd->gd_StripError);
                                break;

	    case HUNK_RELOC16 :
	    case HUNK_RELOC8  :
	    case HUNK_DREL32  :
	    case HUNK_DREL16  :
	    case HUNK_DREL8   : /* what the heck? This is not supposed to be in a load file */
                                do
                                {
                                    GetLong(gd,&size);
                                    if (size != 0)
                                        SkipLongs(gd,size+1);
                                }
                                while ((size != 0) && !gd->gd_StripError);
                                break;

	    case HUNK_EXT     : /* warnhunk(hunkType); */
	    case HUNK_SYMBOL  : do
                                {
                                    GetLong(gd,&ext);
                                    if (ext != 0)
                                    {
                                        size = ext & 0xffffff;
                                        switch ((ext>>24) & 0xff)
                                        {
                                            case EXT_SYMB  :
                                            case EXT_DEF   :
                                            case EXT_ABS   : SkipLongs(gd,size+1);
                                                             break;
                                            case EXT_REF32 :
                                            case EXT_REF16 :
                                            case EXT_REF8  :
                                            case EXT_DEXT32:
                                            case EXT_DEXT16:
                                            case EXT_DEXT8 : SkipLongs(gd,size);
                                                             GetLong(gd,&size);
                                                             SkipLongs(gd,size);
                                                             break;

                                            case EXT_COMMON: SkipLongs(gd,size+1);
                                                             GetLong(gd,&size);
                                                             SkipLongs(gd,size);
                                                             break;
                                        }
                                    }
                                }
                                while ((ext != 0) && !gd->gd_StripError);
                                break;

	    case HUNK_HEADER  : PutLong(gd,hunkLong);
                                do
                                {
                                    GetLong(gd,&size);
                                    PutLong(gd,size);
                                    if (size != 0)
                                        CopyLongs(gd,size);
                                }
                                while ((size != 0) && !gd->gd_StripError);
                                CopyLongs(gd,1);
                                GetLong(gd,&size);
                                PutLong(gd,size);
                                GetLong(gd,&ext);
                                PutLong(gd,ext);
                                CopyLongs(gd,ext-size+1);
                                break;

	    case HUNK_OVERLAY : PutLong(gd,hunkLong);
                                GetLong(gd,&size);
                                PutLong(gd,size);
                                CopyLongs(gd,size);
                                break;

	    case HUNK_END     :
	    case HUNK_BREAK   : PutLong(gd,hunkLong);
                                break;
	}

        if (gd->gd_StripError)
            return(-1);

	GetLong(gd,&hunkLong);

        if (gd->gd_StripError && ((hunkType == HUNK_END) || (hunkType == HUNK_BREAK)))
            return(gd->gd_LFWriteIndex * 4);
    }
}


/*****************************************************************************/


static LONG ConvertRCSTag(struct GlobalData *gd, STRPTR name,
                          UBYTE *buffer, LONG bufSize)
{
LONG i;
LONG skip;
LONG year, month, day;
LONG version, revision;
LONG start;
LONG len;
char stash[100];

    if (bufSize > 7)
    {
        for (i = 0; i < bufSize-7; i++)
        {
            if (buffer[i] == '$')
            {
                if (strncmp(&buffer[i],"$Date: ",7) == 0)
                {
                    start = i;

                    i += 7;   /* skip over "$Date: " */

                    skip = StrToLong(&buffer[i],&year);
                    if (skip > 0)
                        i += skip;

                    if (buffer[i] != '/')
                        return(-1);

                    i++;

                    skip = StrToLong(&buffer[i],&month);
                    if (skip > 0)
                        i += skip;

                    if (buffer[i] != '/')
                        return(-1);

                    i++;

                    StrToLong(&buffer[i],&day);

                    while ((buffer[i] != '$') && buffer[i])
                        i++;

                    if (buffer[i] != '$')
                        return(-1);

                    i++;

                    while ((buffer[i] != '$') && buffer[i] && (buffer[i] != '\n'))
                        i++;

                    if (strncmp(&buffer[i],"$Revision: ",11) != 0)
                        return(-1);

                    i += 11;      /* skip over "$Revision: " */

                    skip = StrToLong(&buffer[i],&version);
                    if (skip > 0)
                        i += skip;

                    if (buffer[i] != '.')
                        return(-1);

                    i++;

                    skip = StrToLong(&buffer[i],&revision);
                    if (skip > 0)
                        i += skip;

                    while ((buffer[i] != '$') && buffer[i] && (buffer[i] != '\n'))
                        i++;

                    if (buffer[i] != '$')
                        return(-1);

                    i++;

                    sprintf(stash,"$VER: %s %ld.%ld (%ld.%ld.%ld)",name,version,revision,day,month,year);
                    len = strlen(stash);

                    memmove(&buffer[start+len],&buffer[i],bufSize - i);
                    memmove(&buffer[start],stash,len);

                    return(bufSize - (i-(start+len)));
                }
            }
        }
    }

    return(-2);
}


/*****************************************************************************/


static struct Resident *FindRomTag(BPTR segList)
{
struct Resident *tag;
UWORD           *seg;
ULONG            i;
ULONG           *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        seg     = (UWORD *)((ULONG)ptr);
        segList = *ptr;

        for (i=*(ptr-1)>>1; (i>0) ; i--)
        {
            if (*(seg++) == RTC_MATCHWORD)
            {
                tag = (struct Resident *)(--seg);
                if (tag->rt_MatchTag == tag)
                {
                    return(tag);
                }
            }
        }
    }

    return(NULL);
}


/*****************************************************************************/


static APTR FindSegListVersion(BPTR segList)
{
ULONG  i;
UBYTE *address;
ULONG  length;
ULONG *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        address = (UBYTE *)ptr;
        segList = *ptr;
        length  = *(ptr - 1);

        if (length > 4)
        {
            for (i = 0;  i < length-4; i++)
            {
                if (address[i] == '$')
                {
                    if (!strncmp(&address[i],"$VER:",5))
                    {
                        return(&address[i]);
                    }
                }
            }
        }
    }

    return(NULL);
}




/*****************************************************************************/


static STRPTR FindMemoryVersion(struct GlobalData *gd,
                                struct VersionInfo *vi,
                                UBYTE *buffer, ULONG bufSize)
{
ULONG i;

    if (bufSize > 5)
        for (i = 0; i < bufSize-5; i++)
            if (buffer[i] == '$')
                if (strncmp(&buffer[i],"$VER:",5) == 0)
                    return(&buffer[i]);

    return(NULL);
}


/*****************************************************************************/


#define SKIPSPACES     {while (versionStr[i] == ' ') i++;}
#define TERMINATOR(ch) (((ch == 0) || (ch == '\n') || (ch == '\r')) ? TRUE : FALSE)


static BOOL ParseVersionStr(struct GlobalData *gd,
                            struct VersionInfo *vi,
                            STRPTR versionStr,
                            BOOL skipVer, BOOL skipRev)
{
ULONG            i,k;
LONG             len;
LONG             conv;
LONG             day;
LONG             month;
LONG             year;
LONG             seconds;
struct ClockData cd;
char             date[20];
struct DateTime  dt;

    vi->vi_Flags |= (VIF_FOUNDITEM | VIF_FOUNDINFO);

    if (skipVer)
        vi->vi_Flags |= (VIF_FOUNDINFO | VIF_VERSION);

    if (skipRev)
        vi->vi_Flags |= (VIF_FOUNDINFO | VIF_REVISION);

    i = 0;

    if (strncmp(versionStr,"$VER:",5) == 0)
        i = 5;

    SKIPSPACES;

    do
    {
        while ((versionStr[i] != ' ') && !TERMINATOR(versionStr[i]))
            i++;

        SKIPSPACES;
    }
    while (!TERMINATOR(versionStr[i]) && (versionStr[i] < '0') && (versionStr[i] > '9'));

    len = StrToLong(&versionStr[i],&conv);
    if (len > 0)
    {
        if (!skipVer)
        {
            vi->vi_Version  = conv;
            vi->vi_Flags   |= VIF_VERSION;
        }

        i += len;
    }
    else
    {
        SKIPSPACES;
    }

    if (versionStr[i] == '.')
    {
        i++;
        len = StrToLong(&versionStr[i],&conv);
        if (len > 0)
        {
            if (!skipRev)
            {
                vi->vi_Revision = conv;
                vi->vi_Flags   |= VIF_REVISION;
            }

            i += len;
            while (!TERMINATOR(versionStr[i]) && (versionStr[i] != '('))
                i++;

            if (versionStr[i] == '(')
            {
                k = 1;
                while ((k < sizeof(date)) && (versionStr[i+k] != ')'))
                {
                    date[k-1] = versionStr[i+k];
                    k++;
                }
                date[k-1] = 0;

                i++;
                len = StrToLong(&versionStr[i],&day);
                if (len > 0)
                {
                    i += len;
                    if (versionStr[i] == '.')
                    {
                        i++;

                        len = StrToLong(&versionStr[i],&month);
                        if (len > 0)
                        {
                            i += len;

                            if (versionStr[i] == '.')
                            {
                                i++;
                                len = StrToLong(&versionStr[i],&year);
                                if (len > 0)
                                {
                                    cd.year  = year+1900;
                                    cd.month = month;
                                    cd.mday  = day;
                                    cd.hour  = 0;
                                    cd.min   = 0;
                                    cd.sec   = 0;

                                    if (seconds = Date2Amiga(&cd))
                                    {
                                        vi->vi_Date.ds_Days   = seconds / (60*60*24);
                                        vi->vi_Date.ds_Minute = (seconds % (60*60*24)) / 60;
                                        vi->vi_Date.ds_Tick   = (seconds % 60) * 50;
                                        vi->vi_Flags |= VIF_DATE;
                                    }
                                }
                            }
                        }
                    }
                }

                if (!(vi->vi_Flags & VIF_DATE))
                {
                    memset(&dt,0,sizeof(dt));
                    dt.dat_Format  = FORMAT_DOS;
                    dt.dat_StrDate = date;
                    if (StrToDate(&dt))
                    {
                        vi->vi_Date   = dt.dat_Stamp;
                        vi->vi_Flags |= VIF_DATE;
                    }
                }
            }
        }
    }

    return(TRUE);
}


/*****************************************************************************/


static BOOL ParseRomTag(struct GlobalData *gd,
                 struct VersionInfo *vi,
                 struct Resident *romTag)
{
    vi->vi_Version = romTag->rt_Version;
    return(ParseVersionStr(gd,vi,romTag->rt_IdString,TRUE,FALSE));
}


/*****************************************************************************/


static UWORD DoItem(struct GlobalData *gd)
{
BPTR                file;
BPTR                lock;
APTR                buffer;
char                dest[300];
char                home[300];
char                arcName[300];
UWORD               len;
struct DiskObject  *diskObj;
BOOL                doIt;
LONG                bufSize;
BOOL                isIcon;
char                date[20];
char                protbits[10];
UWORD               i;
struct VersionInfo  vi;
BPTR                segList;
STRPTR              vstring;
struct DateTime     dt;
struct Resident    *resident;
BOOL                gotVersion;

    gotVersion = FALSE;

    if (gd->gd_To)
        strcpy(dest,gd->gd_To);
    else
        dest[0] = 0;

    if (!gd->gd_Disk && gd->gd_DefaultDisk)
    {
        if (!(gd->gd_Disk = AllocVec(strlen(gd->gd_DefaultDisk)+1,MEMF_ANY)))
            return(CODE_NOMEMORY);
        strcpy(gd->gd_Disk,gd->gd_DefaultDisk);
    }

    if (gd->gd_Disk)
    {
        if (gd->gd_Disk[0])
            AddPart(dest,gd->gd_Disk,sizeof(dest));
    }

    stccpy(home,dest,sizeof(home));
    AddPart(home,"",sizeof(home));

    if (gd->gd_Destination[0])
        AddPart(dest,gd->gd_Destination,sizeof(dest));

    if (gd->gd_Source)
    {
        if (!gd->gd_Quiet)
        {
            PutStr(FilePart(dest));
            PutStr("..");
        }

        if (!(lock = Lock(gd->gd_Source,ACCESS_READ)))
        {
            PrintIoErrLF(gd,MSG_CANT_LOCK_FILE,gd->gd_Source);
            return(CODE_DOITEMERROR);
        }

        if (!gd->gd_TestMode)
        {
            if (!Examine(lock,&gd->gd_FIB))
            {
                PrintIoErrLF(gd,MSG_CANT_EXAMINE,gd->gd_Source);
                UnLock(lock);
                return(CODE_DOITEMERROR);
            }

            if (gd->gd_FIB.fib_DirEntryType > 0)
            {
                UnLock(lock);
                SetIoErr(ERROR_OBJECT_WRONG_TYPE);
                PrintIoErrLF(gd,MSG_SOURCE_IS_DIR,gd->gd_Source);
                return(CODE_DOITEMERROR);
            }

            bufSize = gd->gd_FIB.fib_Size;

            if (!(buffer = AllocVec(bufSize+100,MEMF_ANY)))  /* 100 is slack for ConvertRCSTag(), in case it needs to expand the string */
            {
                if (!gd->gd_Quiet)
                    PutStr("\n");

                UnLock(lock);
                return(CODE_NOMEMORY);
            }

            if (!(file = Open(gd->gd_Source,MODE_OLDFILE)))
            {
                PrintIoErrLF(gd,MSG_CANT_OPEN_FILE,gd->gd_Source);
                FreeVec(buffer);
                UnLock(lock);
                return(CODE_DOITEMERROR);
            }

            UnLock(lock);

            if (Read(file,buffer,bufSize) < bufSize)
            {
                PrintIoErrLF(gd,MSG_CANT_READ_FILE,gd->gd_Source);
                FreeVec(buffer);
                Close(file);
                return(CODE_DOITEMERROR);
            }

            Close(file);

            bufSize = StripSymbols(gd,buffer,bufSize);
            if (bufSize < 0)
            {
                SetIoErr(ERROR_OBJECT_WRONG_TYPE);
                PrintIoErrLF(gd,MSG_BAD_LOADFILE,gd->gd_Source);
                FreeVec(buffer);
                return(CODE_DOITEMERROR);
            }

            if (gd->gd_RCSTag)
            {
                bufSize = ConvertRCSTag(gd,gd->gd_RCSTag,buffer,bufSize);
                if (bufSize < 0)
                {
                    SetIoErr(ERROR_OBJECT_WRONG_TYPE);

                    if (bufSize == -1)
                        PrintIoErrLF(gd,MSG_BAD_RCSTAG,gd->gd_Source);
                    else
                        PrintIoErrLF(gd,MSG_NO_RCSTAG,gd->gd_Source);

                    FreeVec(buffer);
                    return(CODE_DOITEMERROR);
                }
            }

            if (!gd->gd_WasExecutable && !gd->gd_GotProtection)
                gd->gd_Protection = FIBF_EXECUTE;

            if (!(file = Open(dest,MODE_NEWFILE)))
            {
                PrintIoErrLF(gd,MSG_CANT_OPEN_FILE,dest);
                FreeVec(buffer);
                return(CODE_DOITEMERROR);
            }

            if (Write(file,buffer,bufSize) < bufSize)
            {
                PrintIoErrLF(gd,MSG_CANT_WRITE_FILE,dest);
                FreeVec(buffer);
                Close(file);
                DeleteFile(dest);
                return(CODE_DOITEMERROR);
            }

            if (segList = MemoryLoadSeg(DOSBase,buffer,bufSize))
            {
                if (vstring = FindSegListVersion(segList))
                {
                    gotVersion = ParseVersionStr(gd,&vi,vstring,FALSE,FALSE);
                }
                else if (resident = FindRomTag(segList))
                {
                    gotVersion = ParseRomTag(gd,&vi,resident);
                }
                UnLoadSeg(segList);
            }
            else if (IoErr() == ERROR_OBJECT_WRONG_TYPE)
            {
                if (vstring = FindMemoryVersion(gd,&vi,buffer,bufSize))
                {
                    gotVersion = ParseVersionStr(gd,&vi,vstring,FALSE,FALSE);
                }
            }
            else
            {
                PrintIoErrLF(gd,MSG_CANT_PROCESS_FILE,dest);
                FreeVec(buffer);
                Close(file);
                DeleteFile(dest);
                return(CODE_DOITEMERROR);
            }

            Close(file);
            FreeVec(buffer);

            isIcon = FALSE;
            len = strlen(dest);
            if ((len > 5) && (Stricmp(&dest[len-5],".info") == 0))
                isIcon = TRUE;

            if (isIcon)
            {
                dest[len-5] = 0;
                if (diskObj = GetDiskObject(dest))
                {
                    diskObj->do_CurrentX = gd->gd_IconX;
                    diskObj->do_CurrentY = gd->gd_IconY;

                    if (diskObj->do_DrawerData)
                    {
                        diskObj->do_DrawerData->dd_NewWindow.LeftEdge = gd->gd_DrawerX;
                        diskObj->do_DrawerData->dd_NewWindow.TopEdge  = gd->gd_DrawerY;
                        diskObj->do_DrawerData->dd_NewWindow.Width    = gd->gd_DrawerW;
                        diskObj->do_DrawerData->dd_NewWindow.Height   = gd->gd_DrawerH;
                    }

                    if (!PutDiskObject(dest,diskObj))
                    {
                        strcat(dest,".info");
                        PrintIoErrLF(gd,MSG_CANT_WRITE_ICON,dest);
                        return(CODE_DOITEMERROR);
                    }

                    FreeDiskObject(diskObj);
                    strcat(dest,".info");
                }
                else
                {
                    strcat(dest,".info");
                    PrintIoErrLF(gd,MSG_CANT_READ_ICON,dest);
                    return(CODE_DOITEMERROR);
                }
            }

            if (!SetFileDate(dest,&gd->gd_Date))
            {
                PrintIoErrLF(gd,MSG_CANT_SET_DATE,dest);
                return(CODE_DOITEMERROR);
            }

            if (!SetProtection(dest,gd->gd_Protection))
            {
                PrintIoErrLF(gd,MSG_CANT_SET_PROTECTION,dest);
                return(CODE_DOITEMERROR);
            }

            if (gd->gd_Comment)
            {
                if (!SetComment(dest,gd->gd_Comment))
                {
                    PrintIoErrLF(gd,MSG_CANT_SET_COMMENT,dest);
                    return(CODE_DOITEMERROR);
                }
            }
        }
        else
        {
            UnLock(lock);
        }

        if (!gd->gd_Quiet)
            PrintF(gd,MSG_COPIED,NULL);
    }
    else
    {
        if (!gd->gd_Quiet)
        {
            PutStr(dest);
            PutStr("  ");
        }

        if (!gd->gd_TestMode)
        {
            if (!(lock = CreateDir(dest)))
            {
                if ((IoErr() != ERROR_OBJECT_EXISTS) && (IoErr() != ERROR_OBJECT_IN_USE))
                {
                    PrintIoErrLF(gd,MSG_CANT_CREATE_DIR,dest);
                    return(CODE_DOITEMERROR);
                }
            }

            UnLock(lock);

            if (!SetFileDate(dest,&gd->gd_Date))
            {
                PrintIoErrLF(gd,MSG_CANT_SET_DATE,dest);
                return(CODE_DOITEMERROR);
            }

            if (!SetProtection(dest,gd->gd_Protection))
            {
                PrintIoErrLF(gd,MSG_CANT_SET_PROTECTION,dest);
                return(CODE_DOITEMERROR);
            }

            if (gd->gd_Comment)
            {
                if (!SetComment(dest,gd->gd_Comment))
                {
                    PrintIoErrLF(gd,MSG_CANT_SET_COMMENT,dest);
                    return(CODE_DOITEMERROR);
                }
            }
        }

        if (!gd->gd_Quiet)
            PrintF(gd,MSG_CREATED,NULL);
    }

    if (gd->gd_LhASegment && gd->gd_Disk)
    {
        doIt = FALSE;
        if (gd->gd_Destination[0])
        {
            doIt = TRUE;
            strcpy(dest,gd->gd_Destination);
        }

        if (doIt)
        {
            if (gd->gd_To)
                strcpy(arcName,gd->gd_To);
            else
                arcName[0] = 0;
            AddPart(arcName,gd->gd_Disk,sizeof(arcName));
            strncat(arcName,".LHA",sizeof(arcName));

            sprintf(gd->gd_LhACmdLine,"-aemrnNqx2 -v0 -b32 a \"%s\" \"%s\" \"%s\"\n",arcName,home,dest);
            RunCommand(gd->gd_LhASegment,16384,gd->gd_LhACmdLine,strlen(gd->gd_LhACmdLine));
        }
    }

    if (gd->gd_LogFile)
    {
        if (gotVersion)
        {
            dt.dat_Format  = FORMAT_DOS;
            dt.dat_StrDay  = NULL;
            dt.dat_StrDate = date;
            dt.dat_StrTime = NULL;
            dt.dat_Flags   = 0;
            dt.dat_Stamp   = vi.vi_Date;
            DateToStr(&dt);

            sprintf(home,"%2ld.%ld (%-8s)",vi.vi_Version,vi.vi_Revision,date);
        }
        else
        {
            strcpy(home,"                  ");
        }

        strcpy(protbits,"hsparwed");
        for (i=0; i<8; i++)
        {
            if ((gd->gd_Protection & (1 << (7-i))) == ((i<4) ? NULL : (1 << (7-i))))
                protbits[i] = '-';
        }

        if (gd->gd_Source)
        {
            sprintf(arcName,"  %-18.18s  %6ld  %s  %s:%s\n",home,bufSize,protbits,gd->gd_Disk,gd->gd_Destination);
        }
        else if (gd->gd_Destination && gd->gd_Destination[0])
        {
            sprintf(arcName,"                      Drawer  %s  %s:%s\n",protbits,gd->gd_Disk,gd->gd_Destination);
        }
        else
        {
            sprintf(arcName,"                        Disk            %s:%s\n",gd->gd_Disk,gd->gd_Destination);
        }

        if (FPuts(gd->gd_LogFile,arcName))
        {
            PrintIoErr(gd,MSG_CANT_WRITE_LOG,gd->gd_LogFileName);
            return(CODE_DOITEMERROR);
        }
    }

    return(CODE_SUCCESS);
}


/*****************************************************************************/


static UWORD ProcessKeywordArguments(struct GlobalData *gd, UWORD keyword)
{
STRPTR          str;
LONG            num1,num2;
LONG            chars1, chars2;
struct DateTime dt;
ULONG           len;
STRPTR          new;
UWORD           i;
UWORD           code;

    if ((gd->gd_TokenType == TOK_NOTHING)
    &&  (keyword != KEY_COPYFILE)
    &&  (keyword != KEY_CREATEDIR))
        return(CODE_ARGUMENTEXPECTED);

    code = CODE_SUCCESS;
    str  = gd->gd_CurrentToken;

    if ((keyword == KEY_ICONPOS)
    ||  (keyword == KEY_DRAWERPOS)
    ||  (keyword == KEY_DRAWERSIZE))
    {
        if (gd->gd_TokenType != TOK_NUMBER)
            return(CODE_NUMBERSEXPECTED);

        chars1 = StrToLong(str,&num1);
        if (str[chars1] != ',')
            return(CODE_NUMBERSEXPECTED);

        chars2 = StrToLong(&str[chars1+1],&num2);
        if (chars2 < 0)
            return(CODE_NUMBERSEXPECTED);

        if (str[chars1+1+chars2] != 0)
            return(CODE_NUMBERSEXPECTED);
    }

    switch (keyword)
    {
        case KEY_SOURCE         : len = strlen(str);
                                  if (!(new = AllocVec(len+1,MEMF_ANY)))
                                      return(CODE_NOMEMORY);

                                  FreeVec(gd->gd_Source);
                                  strcpy(new,str);
                                  gd->gd_Source = new;
                                  break;

        case KEY_DESTINATION    : len = strlen(str);
                                  if (!(new = AllocVec(len+1,MEMF_ANY)))
                                      return(CODE_NOMEMORY);

                                  FreeVec(gd->gd_Destination);
                                  strcpy(new,str);
                                  gd->gd_Destination = new;
                                  break;

        case KEY_PROTECTION     : gd->gd_Protection = FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE;
                                  i = 0;
                                  while (str[i])
                                  {
                                      switch (str[i])
                                      {
                                          case 'r': gd->gd_Protection &= (~FIBF_READ);
                                                    gd->gd_Protection |= (FIBF_OTR_READ | FIBF_GRP_READ);
                                                    break;

                                          case 'w': gd->gd_Protection &= (~FIBF_WRITE);
                                                    gd->gd_Protection |= (FIBF_OTR_WRITE | FIBF_GRP_WRITE);
                                                    break;

                                          case 'e': gd->gd_Protection &= (~FIBF_EXECUTE);
                                                    gd->gd_Protection |= (FIBF_OTR_EXECUTE | FIBF_GRP_EXECUTE);
                                                    break;

                                          case 'd': gd->gd_Protection &= (~FIBF_DELETE);
                                                    break;

                                          case 's': gd->gd_Protection |= FIBF_SCRIPT;
                                                    break;

                                          case 'p': gd->gd_Protection |= FIBF_PURE;
                                                    break;

                                          case 'a': gd->gd_Protection |= FIBF_ARCHIVE;
                                                    break;

                                          case '-': break;

                                          default : return(CODE_BADPROTECTION);
                                      }
                                      i++;
                                  }
                                  gd->gd_GotProtection = TRUE;
                                  break;

        case KEY_DATE           : dt.dat_StrDate = str;
                                  dt.dat_StrTime = NULL;
                                  dt.dat_Format  = FORMAT_DOS;
                                  dt.dat_Flags   = 0;
                                  if (!StrToDate(&dt))
                                      return(CODE_BADDATE);
                                  gd->gd_Date.ds_Days = dt.dat_Stamp.ds_Days;
                                  break;

        case KEY_TIME           : dt.dat_StrTime = str;
                                  dt.dat_StrDate = NULL;
                                  dt.dat_Format  = FORMAT_DOS;
                                  dt.dat_Flags   = 0;
                                  if (!StrToDate(&dt))
                                      return(CODE_BADTIME);
                                  gd->gd_Date.ds_Minute = dt.dat_Stamp.ds_Minute;
                                  gd->gd_Date.ds_Tick   = dt.dat_Stamp.ds_Tick;
                                  break;

        case KEY_COMMENT        : len = strlen(str);
                                  if (!(new = AllocVec(len+1,MEMF_ANY)))
                                      return(CODE_NOMEMORY);

                                  FreeVec(gd->gd_Comment);
                                  strcpy(new,str);
                                  gd->gd_Comment = new;
                                  break;

        case KEY_ICONPOS        : gd->gd_IconX = num1;
                                  gd->gd_IconY = num2;
                                  break;

        case KEY_DRAWERPOS      : gd->gd_DrawerX = num1;
                                  gd->gd_DrawerY = num2;
                                  break;

        case KEY_DRAWERSIZE     : gd->gd_DrawerW = num1;
                                  gd->gd_DrawerH = num2;
                                  break;

        case KEY_IFDEF          : if (!FindSymbol(gd,str))
                                      gd->gd_DoItem = FALSE;
                                  break;

        case KEY_IFNDEF         : if (FindSymbol(gd,str))
                                      gd->gd_DoItem = FALSE;
                                  break;

        case KEY_DEFINE         : if (!AddSymbol(gd,str))
                                      return(CODE_NOMEMORY);
                                  break;

        case KEY_UNDEF          : if (!RemoveSymbol(gd,str))
                                      return(CODE_SYMBOLNOTFOUND);
                                  break;

        case KEY_COPYFILE       : if (gd->gd_Source && !gd->gd_Destination)
                                      return(CODE_NODESTINATION);

                                  if (!gd->gd_Source && gd->gd_Destination)
                                      return(CODE_NOSOURCE);

                                  if (CheckSignal(SIGBREAKF_CTRL_C))
                                      return(CODE_ABORTED);

                                  if (gd->gd_DoItem)
                                      code = DoItem(gd);

                                  InitEnvironment(gd);
                                  break;

        case KEY_CREATEDIR      : if (gd->gd_Source)
                                      return(CODE_NOTWITHDIRECTORY);

                                  if (!gd->gd_Destination)
                                      return(CODE_NODESTINATION);

                                  if (CheckSignal(SIGBREAKF_CTRL_C))
                                      return(CODE_ABORTED);

                                  if (gd->gd_DoItem)
                                      code = DoItem(gd);

                                  InitEnvironment(gd);
                                  break;

        case KEY_DISK           : len = strlen(str);
                                  if (!(new = AllocVec(len+1,MEMF_ANY)))
                                      return(CODE_NOMEMORY);

                                  FreeVec(gd->gd_Disk);
                                  strcpy(new,str);
                                  gd->gd_Disk = new;
                                  break;

        case KEY_DEFAULTDISK    : if (gd->gd_DoItem)
                                  {
                                      len = strlen(str);
                                      if (!(new = AllocVec(len+1,MEMF_ANY)))
                                          return(CODE_NOMEMORY);

                                      FreeVec(gd->gd_DefaultDisk);
                                      strcpy(new,str);
                                      gd->gd_DefaultDisk = new;
                                  }

                                  InitEnvironment(gd);
                                  break;

        case KEY_RCSTAG         : len = strlen(str);
                                  if (!(new = AllocVec(len+1,MEMF_ANY)))
                                      return(CODE_NOMEMORY);

                                  FreeVec(gd->gd_RCSTag);
                                  strcpy(new,str);
                                  gd->gd_RCSTag = new;
                                  break;
    }

    return(code);
}


/*****************************************************************************/


static BOOL MakeWB(struct GlobalData *gd, STRPTR buildList, STRPTR destDir)
{
UBYTE              state;
WORD               keyword;
UWORD              code;
UWORD              line;
UWORD              column;
char               key[30];
char              *ptr;
UWORD              i;

    if (!(gd->gd_InputFile = Open(buildList,MODE_OLDFILE)))
    {
        PrintIoErr(gd,MSG_CANT_OPEN_FILE,buildList);
        return(FALSE);
    }
    SetVBuf(gd->gd_InputFile,NULL,BUF_FULL,4096);

    if (gd->gd_MakeArcs)
    {
        if (!(gd->gd_LhASegment = LoadSegPath(gd,"LhA")))
        {
            PrintIoErr(gd,MSG_CANT_LOAD_LHA,NULL);
            Close(gd->gd_InputFile);
            return(FALSE);
        }
    }

    InitEnvironment(gd);

    gd->gd_InputLine   = 1;
    gd->gd_InputColumn = 0;
    gd->gd_LastChar    = ' ';

    code  = CODE_SUCCESS;
    state = SCAN_KEYWORD;

    while ((code == CODE_SUCCESS) && (state != SCAN_DONE) && GetToken(gd))
    {
        switch (state)
        {
            case SCAN_KEYWORD : keyword = FindArg(Keywords,gd->gd_CurrentToken);
                                if (keyword < 0)
                                {
                                    code = CODE_UNKNOWNKEYWORD;
                                    state = SCAN_EQUAL;
                                }
                                else
                                {
                                    gd->gd_TokenType = TOK_NOTHING;  /* tell the keyword processor this token was eaten up */
                                    code = ProcessKeywordArguments(gd,keyword);
                                    if (code == CODE_ARGUMENTEXPECTED)
                                    {
                                        /* this token needed more args, let's give it to him */
                                        code  = CODE_SUCCESS;
                                        state = SCAN_EQUAL;
                                    }
                                }
                                break;

            case SCAN_EQUAL   : if (Stricmp(gd->gd_CurrentToken,"=") != 0)
                                    code = CODE_EQUALEXPECTED;

                                state = SCAN_ARGUMENT;
                                break;

            case SCAN_ARGUMENT: code = ProcessKeywordArguments(gd,keyword);
                                state = SCAN_KEYWORD;
                                break;
        }
    }

    Close(gd->gd_InputFile);
    UnLoadSeg(gd->gd_LhASegment);

    if (code == CODE_SUCCESS)
    {
        if (state == SCAN_EQUAL)
            code = CODE_EQUALEXPECTED;

        else if (state == SCAN_ARGUMENT)
            code = CODE_ARGUMENTEXPECTED;
    }

    if ((code != CODE_SUCCESS) && (code != CODE_DOITEMERROR))
    {
        line   = gd->gd_TokenLine;
        column = gd->gd_TokenColumn;

        if (code != CODE_ABORTED)
            PrintF(gd,MSG_ERROR,NULL);

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
            case CODE_NOMEMORY         : PrintF(gd,MSG_NOMEM,NULL);
                                         break;

            case CODE_STRINGEXPECTED   : PrintF(gd,MSG_STRING_EXPECTED,key,buildList,line,column);
                                         break;

            case CODE_NUMBERSEXPECTED  : PrintF(gd,MSG_NUMBERS_EXPECTED,key,buildList,line,column);
                                         break;

            case CODE_EQUALEXPECTED    : PrintF(gd,MSG_EQUAL_EXPECTED,buildList,line,column);
                                         break;

            case CODE_ARGUMENTEXPECTED : PrintF(gd,MSG_ARGUMENT_EXPECTED,key,buildList,line,column);
                                         break;

            case CODE_UNKNOWNKEYWORD   : PrintF(gd,MSG_UNKNOWN_KEYWORD,gd->gd_CurrentToken,buildList,line,column);
                                         break;

            case CODE_BADDATE          : PrintF(gd,MSG_INVALID_DATE,gd->gd_CurrentToken,buildList,line,column);
                                         break;

            case CODE_BADTIME          : PrintF(gd,MSG_INVALID_TIME,gd->gd_CurrentToken,buildList,line,column);
                                         break;

            case CODE_BADPROTECTION    : PrintF(gd,MSG_INVALID_PROTECTION,gd->gd_CurrentToken,buildList,line,column);
                                         break;

            case CODE_NOTWITHDIRECTORY : PrintF(gd,MSG_INVALID_WITHDIR,key,buildList,line,column);
                                         break;

            case CODE_NOSOURCE         : PrintF(gd,MSG_NOSOURCE_FOUND,buildList,line,column);
                                         break;

            case CODE_NODESTINATION    : PrintF(gd,MSG_NODESTINATION_FOUND,buildList,line,column);
                                         break;

            case CODE_SYMBOLNOTFOUND   : PrintF(gd,MSG_SYMBOL_NOT_FOUND,gd->gd_CurrentToken,buildList,line,column);
                                         break;

            case CODE_ABORTED          : PrintFault(ERROR_BREAK,NULL);
                                         break;
        }
    }

    InitEnvironment(gd);  /* free up any memory */
    FreeVec(gd->gd_DefaultDisk);

    return((BOOL)(code == CODE_SUCCESS));
}
