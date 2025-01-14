
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>
#include <dos/datetime.h>
#include <string.h>
#include <stdlib.h>

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
#include "makebuildlist_rev.h"


/*****************************************************************************/


#define TEMPLATE   "FROM/M/A,TO/A" VERSTAG
#define OPT_FROM   0
#define OPT_TO     1
#define OPT_COUNT  2


/*****************************************************************************/


struct GlobalData
{
    struct DosLibrary *gd_DOSBase;
    struct Library    *gd_UtilityBase;
    struct Library    *gd_SysBase;
    struct Library    *gd_IconBase;
    struct LocaleInfo  gd_LocaleInfo;

    struct FileInfoBlock gd_FIB;   /* long aligned! */
};

#define SysBase       gd->gd_SysBase
#define DOSBase       gd->gd_DOSBase
#define UtilityBase   gd->gd_UtilityBase
#define IconBase      gd->gd_IconBase
#define LocaleBase    gd->gd_LocaleInfo.li_LocaleBase


/*****************************************************************************/


VOID kprintf(STRPTR,...);
BOOL AddDescriptor(struct GlobalData *gd, BPTR file, struct FileInfoBlock *fib);
VOID PrintIoErr(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... );


/*****************************************************************************/


LONG main(VOID)
{
struct AnchorPath  __aligned anchor;
LONG               opts[OPT_COUNT];
struct RDArgs     *rdargs;
LONG               failureLevel = RETURN_FAIL;
STRPTR            *from;
BPTR               oldLock;
struct GlobalData  global;
struct GlobalData *gd;
LONG               result;
BOOL               bool;
BPTR               file;

    gd = &global;
    memset(gd,0,sizeof(struct GlobalData));

    SysBase = (*((struct Library **) 4));

    if (DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37))
    if (UtilityBase = OpenLibrary("utility.library",37))
    if (IconBase = OpenLibrary("icon.library",37))
    {
        if (LocaleBase = OpenLibrary("locale.library",38))
            gd->gd_LocaleInfo.li_Catalog = OpenCatalogA(NULL,"makewb.catalog",NULL);

        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
        {
            failureLevel = RETURN_OK;

            if (!(file = Open((STRPTR)opts[OPT_TO],MODE_NEWFILE)))
            {
                failureLevel = RETURN_WARN;
                PrintIoErr(gd,MSG_CANT_OPEN_DESTINATION,(STRPTR)opts[OPT_TO]);
            }

            from = (STRPTR *)opts[OPT_FROM];
            while ((*from) && (failureLevel == RETURN_OK))
            {
                memset(&anchor,0,sizeof(struct AnchorPath));
                anchor.ap_Flags     = APF_DOWILD;
                anchor.ap_BreakBits = SIGBREAKF_CTRL_C;

                if (!(result = MatchFirst(*from,&anchor)))
                {
                    while (TRUE)
                    {
                        if (CheckSignal(SIGBREAKF_CTRL_C))
                        {
                            failureLevel = RETURN_WARN;
                            PrintFault(ERROR_BREAK,NULL);
                            break;
                        }

                        if (anchor.ap_Flags & APF_DIDDIR)
                        {
                            anchor.ap_Flags &= ~APF_DIDDIR;
                        }
                        else
                        {
                            if (anchor.ap_Info.fib_DirEntryType > 0)
                                anchor.ap_Flags |= APF_DODIR;

                            oldLock = CurrentDir(anchor.ap_Current->an_Lock);
                            bool = AddDescriptor(gd,file,&anchor.ap_Info);
                            CurrentDir(oldLock);

                            if (!bool)
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
                else
                {
                    failureLevel = RETURN_FAIL;
                    PrintFault(IoErr(),NULL);
                }
                from++;
            }

            if (file)
                Close(file);

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


VOID PrintF(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... )
{
    VPrintf(GetString(&gd->gd_LocaleInfo,str),(LONG *)&arg1);
}


/*****************************************************************************/


VOID FPrintF(struct GlobalData *gd, BPTR file, STRPTR format, STRPTR arg1, ... )
{
    VFPrintf(file,format,(LONG *)&arg1);
}


/*****************************************************************************/


VOID PrintIoErr(struct GlobalData *gd, AppStringsID str, STRPTR arg1, ... )
{
ULONG ioerr = IoErr();

    PrintF(gd,MSG_ERROR,NULL);
    VPrintf(GetString(&gd->gd_LocaleInfo,str),(LONG *)&arg1);
    PrintFault(ioerr,"\n");
}


/*****************************************************************************/


BOOL AddDescriptor(struct GlobalData *gd, BPTR file, struct FileInfoBlock *fib)
{
char               date[20], time[20], protbits[20];
struct DateTime    dt;
UWORD              i;
UWORD              len;
struct DiskObject *diskObj;
char               fullPath[300];
BPTR               cd;

    cd = CurrentDir(0);
    CurrentDir(cd);
    NameFromLock(cd,fullPath,sizeof(fullPath));
    AddPart(fullPath,fib->fib_FileName,sizeof(fullPath));

    if (fib->fib_DirEntryType < 0)
        FPrintF(gd,file,"SOURCE		= \"V:src/workbench/.../%s\"\n",fib->fib_FileName);

    i = 0;
    while (fullPath[i] != ':')
        i++;

    FPrintF(gd,file,"DESTINATION	= \"%s\"\n",&fullPath[i+1]);
    FPrintF(gd,file,"INSTALLATION	= \"%s\"\n",&fullPath[i+1]);
    FPuts(file,"DISK		= \"");
    FWrite(file,fullPath,i,1);
    FPuts(file,"\"\n");

    strcpy(protbits,"hsparwed");
    for (i=0; i<8; i++)
    {
        if ((fib->fib_Protection & (1 << (7-i))) == ((i<4) ? NULL : (1 << (7-i))))
          protbits[i] = '-';
    }
    FPrintF(gd,file,"PROTECTION	= %s\n",protbits);

    dt.dat_Format  = FORMAT_DOS;
    dt.dat_StrDay  = NULL;
    dt.dat_StrDate = date;
    dt.dat_StrTime = time;
    dt.dat_Flags   = 0;
    dt.dat_Stamp   = fib->fib_Date;
    DateToStr(&dt);
    FPrintF(gd,file,"DATE		= %s\n",date);
    FPrintF(gd,file,"TIME		= %s\n",time);

    if (fib->fib_Comment[0])
        FPrintF(gd,file,"COMMENT		= \"%s\"\n",fib->fib_Comment);

    len = strlen(fullPath);
    if ((len > 5) && (Stricmp(&fullPath[len-5],".info") == 0))
    {
        fullPath[len-5] = 0;
        diskObj = GetDiskObject(fullPath);
        fullPath[len-5] = '.';

        if (diskObj)
        {
            FPrintF(gd,file,"ICONPOS		= %ld,%ld\n",(STRPTR)diskObj->do_CurrentX,(STRPTR)diskObj->do_CurrentY);

            if (diskObj->do_DrawerData)
            {
                FPrintF(gd,file,"DRAWERPOS	= %ld,%ld\n",(STRPTR)diskObj->do_DrawerData->dd_NewWindow.LeftEdge,(STRPTR)diskObj->do_DrawerData->dd_NewWindow.TopEdge);
                FPrintF(gd,file,"DRAWERSIZE	= %ld,%ld\n",(STRPTR)diskObj->do_DrawerData->dd_NewWindow.Width,(STRPTR)diskObj->do_DrawerData->dd_NewWindow.Height);
            }

            FreeDiskObject(diskObj);
        }
        else
        {
            PrintIoErr(gd,MSG_CANT_READ_ICON,fullPath);
            return(FALSE);
        }
    }

    if (fib->fib_DirEntryType < 0)
        FPuts(file,"#COPYFILE\n");
    else
        FPuts(file,"#CREATEDIR\n");

    FPuts(file,"\n/*****************************************************************************/\n\n");

    return(TRUE);
}
