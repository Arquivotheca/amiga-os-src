

#include <stdio.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/iffparse.h>
#include <utility/tagitem.h>

#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <prefs/prefhdr.h>
#include <prefs/printertxt.h>
#include "printinstall.h"

 STRPTR revisionstring=VERSTAG;

#define OPT_INSTALL         0
#define OPT_REMOVE          1
#define OPT_COUNT           2
#define TEMPLATE "INSTALL/S,REMOVE/S"

BOOL MakeIcon(STRPTR name);

void start(void)
{
    struct Library *SysBase=(*(void **)4L);
    struct Library *DOSBase;
    struct Library *IFFParseBase;
    struct Library *IntuitionBase;
    struct Library *VersionBase;
    struct PrefHeader ph;
    struct PrinterTxtPrefs ptp;
    struct PrinterUnitPrefs pup;
    ULONG opts[OPT_COUNT]={0,0};
    UWORD found = 0;
    struct RDArgs *r;
    struct Process *p;
    struct WBStartup *w;
    BOOL install=FALSE;
    BOOL run=FALSE;

    p = (struct Process *) FindTask(0L);

    if (!p->pr_CLI)
        w = GetMsg(&p->pr_MsgPort);

    DOSBase = OpenLibrary("dos.library",37L);
    if (DOSBase)
    {
        IntuitionBase = OpenLibrary("intuition.library",37L);
        if (IntuitionBase)
        {
            VersionBase = OpenLibrary("version.library",37L);
            if (VersionBase)
            {
                if (p->pr_CLI)
                {
                    r = ReadArgs(TEMPLATE,opts,NULL);
                    if (r)
                    {
                        if (opts[OPT_INSTALL])
                            install = TRUE;
                        if (opts[OPT_REMOVE])
                            install = FALSE;
                        if ( (opts[OPT_INSTALL]) || (opts[OPT_REMOVE]) )
                        {
                            run = TRUE;
                        }
                        FreeArgs(r);
                    }
                }
                else
                {
                    int er;
                    struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Networking Printing",
                                           "Install or Remove Envoy network printing?","Install|Remove|Cancel"};
                    ULONG notags[1]={TAG_DONE};
                    er = EasyRequestArgs(0L,&ers,0L,&notags);
                    if (er)
                        run = TRUE;
                    if (er == 1)
                        install = TRUE;
                }
                if (run)
                {

                    IFFParseBase = OpenLibrary("iffparse.library",37L);
                    if (IFFParseBase)
                    {
                        struct IFFHandle *i;
                        i = AllocIFF();
                        if (i)
                        {
                            i->iff_Stream = (ULONG) Open("env:sys/printer.prefs",MODE_OLDFILE);
                            if (i->iff_Stream)
                            {
                                InitIFFasDOS(i);
                                if (!OpenIFF(i,IFFF_READ))
                                {
                                    BOOL foundsome=FALSE;
                                    ULONG ca[6]={ID_PREF,ID_PRHD,
                                                 ID_PREF,ID_PTXT,
                                                 ID_PREF,ID_PUNT};
                                    struct ContextNode *top;
                                    int err;
                                    if (err=CollectionChunks(i,ca,3))
                                    {
                                        if (!(err=CollectionChunks(i,ca,2)))
                                            foundsome = TRUE;
                                    }
                                    else
                                        foundsome = TRUE;

                                    if (TRUE)
                                    {
                                        int r;
                                        for (r = 0; r < sizeof(struct PrinterUnitPrefs); r++)
                                            ((char *) &pup)[r]=0;
                                    }

                                    if (foundsome)
                                    {
                                        int error;
                                        BOOL cont = TRUE;
                                        while (cont)
                                        {
                                            error = ParseIFF(i,IFFPARSE_STEP);
                                            if (!error)
                                                continue;
                                            if (error != IFFERR_EOC)
                                                break;

                                            top = CurrentChunk(i);
                                            if ((top->cn_Type == ID_PREF) && (top->cn_ID == ID_FORM))
                                                break;
                                        }
                                        if (error == IFFERR_EOC)
                                        {
                                            struct CollectionItem *ci;
                                            if (ci = FindCollection(i,ID_PREF,ID_PRHD))
                                            {
                                                while (ci)
                                                {
                                                    CopyMem(ci->ci_Data,&ph,sizeof(struct PrefHeader));
                                                    found |= 1;
                                                    ci = ci->ci_Next;
                                                }
                                            }
                                            if (ci = FindCollection(i,ID_PREF,ID_PTXT))
                                            {
                                                while (ci)
                                                {
                                                    CopyMem(ci->ci_Data,&ptp,sizeof(struct PrinterTxtPrefs));
                                                    found |= 2;
                                                    ci = ci->ci_Next;
                                                }
                                            }
                                            if (ci = FindCollection(i,ID_PREF,ID_PUNT))
                                            {
                                                while (ci)
                                                {
                                                    CopyMem(ci->ci_Data,&pup,sizeof(struct PrinterUnitPrefs));
                                                    found |= 4;
                                                    ci = ci->ci_Next;
                                                }
                                            }

                                        }

                                    }
                                    CloseIFF(i);
                                }
                                Close(i->iff_Stream);
                            }
                            else    /* defaults - got these from printer prefs program -- pointed by Martin */
                            {
                                memset(&ptp,0,sizeof(struct PrinterTxtPrefs));
                                memset(&pup,0,sizeof(struct PrinterUnitPrefs));
                                strcpy(&ptp.pt_Driver,"generic");
                                ptp.pt_Port = PP_PARALLEL;
                                ptp.pt_PaperType = PT_FANFOLD;
                                ptp.pt_PaperSize = PS_N_TRACTOR;
                                ptp.pt_PaperLength = 66;
                                ptp.pt_Pitch = PP_PICA;
                                ptp.pt_Spacing = PS_SIX_LPI;
                                ptp.pt_LeftMargin = 5;
                                ptp.pt_RightMargin = 75;
                                ptp.pt_Quality = PQ_DRAFT;
                                ph.ph_Version = 0;
                                ph.ph_Type = 0;
                                ph.ph_Flags = 0;
                                found = 7;
                            }
                            if ( (found & 3) == 3)
                            {
                                if (install)
                                {
                                    strcpy(&pup.pu_DeviceName,"envoyprint");
                                    pup.pu_UnitNum = 0L;    /* Hey .. If they're dumb enough to have other unit #'s ... */
                                    ptp.pt_Port = PP_PARALLEL;
                                }
                                else
                                {
                                    pup.pu_DeviceName[0]=0;
                                }
                                /* Write it out again. */
                                i->iff_Stream = Open("env:sys/printer.prefs",MODE_NEWFILE);
                                if (i->iff_Stream)
                                {
                                    InitIFFasDOS(i);
                                    if (!OpenIFF(i,IFFF_WRITE))
                                    {
                                        if (!PushChunk(i,ID_PREF,ID_FORM,IFFSIZE_UNKNOWN))
                                        {
                                            if (!PushChunk(i,ID_PREF,ID_PRHD,IFFSIZE_UNKNOWN))
                                            {
                                                if (WriteChunkBytes(i,&ph,sizeof(struct PrefHeader)) == sizeof(struct PrefHeader))
                                                {
                                                    if (!PopChunk(i))
                                                    {
                                                        if (!PushChunk(i,ID_PREF,ID_PTXT,sizeof(struct PrinterTxtPrefs)))
                                                        {
                                                            if (WriteChunkBytes(i,&ptp,sizeof(struct PrinterTxtPrefs)) == sizeof(struct PrinterTxtPrefs))
                                                            {
                                                                if (!PopChunk(i))
                                                                {
                                                                    if (!PushChunk(i,ID_PREF,ID_PUNT,sizeof(struct PrinterUnitPrefs)))
                                                                    {
                                                                        if (WriteChunkBytes(i,&pup,sizeof(struct PrinterUnitPrefs)) == sizeof(struct PrinterUnitPrefs))
                                                                        {
                                                                            PopChunk(i);
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            PopChunk(i);
                                        }
                                        CloseIFF(i);
                                    }
                                    Close(i->iff_Stream);

                                }
                                /* Write it out again. */
                                i->iff_Stream = Open("envarc:sys/printer.prefs",MODE_NEWFILE);
                                if (i->iff_Stream)
                                {
                                    InitIFFasDOS(i);
                                    if (!OpenIFF(i,IFFF_WRITE))
                                    {
                                        if (!PushChunk(i,ID_PREF,ID_FORM,IFFSIZE_UNKNOWN))
                                        {
                                            if (!PushChunk(i,ID_PREF,ID_PRHD,IFFSIZE_UNKNOWN))
                                            {
                                                if (WriteChunkBytes(i,&ph,sizeof(struct PrefHeader)) == sizeof(struct PrefHeader))
                                                {
                                                    if (!PopChunk(i))
                                                    {
                                                        if (!PushChunk(i,ID_PREF,ID_PTXT,sizeof(struct PrinterTxtPrefs)))
                                                        {
                                                            if (WriteChunkBytes(i,&ptp,sizeof(struct PrinterTxtPrefs)) == sizeof(struct PrinterTxtPrefs))
                                                            {
                                                                if (!PopChunk(i))
                                                                {
                                                                    if (!PushChunk(i,ID_PREF,ID_PUNT,sizeof(struct PrinterUnitPrefs)))
                                                                    {
                                                                        if (WriteChunkBytes(i,&pup,sizeof(struct PrinterUnitPrefs)) == sizeof(struct PrinterUnitPrefs))
                                                                        {
                                                                            PopChunk(i);
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            PopChunk(i);
                                        }
                                        CloseIFF(i);
                                    }
                                    Close(i->iff_Stream);

                                }

                            }
                        }
                        FreeIFF(i);
                        CloseLibrary(IFFParseBase);
                    }
                }
                else
                    if (p->pr_CLI)
                        PrintFault(ERROR_REQUIRED_ARG_MISSING,"Network Printing");
                if (VersionBase->lib_Version == 37L)
                {
                    struct Preferences *pf;
                    pf = (struct Preferences *) AllocMem(sizeof(struct Preferences),0L);
                    if (pf)
                    {
                        if (install)
                        {
                            /* Copy the kludge into SYS:WBStartup */
                            BPTR ourlock;
                            UBYTE ourpath[256];
                            STRPTR buff;
                            ourlock = GetProgramDir();
                            if (NameFromLock(ourlock,ourpath,256))
                            {
                                AddPart(ourpath,"Envoy V37 PrintStart",256);
                                buff = (STRPTR) AllocMem(256,0);
                                if (buff)
                                {
                                    BPTR inf, outf;
                                    inf = Open(ourpath,MODE_OLDFILE);
                                    if (inf)
                                    {
                                        int amount;
                                        outf = Open("SYS:WBStartup/Envoy V37 PrintStart",MODE_NEWFILE);
                                        if (outf)
                                        {
                                            while (amount = Read(inf,buff,256))
                                                Write(outf,buff,amount);
                                            MakeIcon("SYS:WBStartup/Envoy V37 PrintStart");
                                            Close(outf);
                                        }
                                        Close(inf);
                                    }
                                    FreeMem(buff,256);
                                }

                            }

                            /* Change it immediately */
                            GetPrefs(pf,sizeof(struct Preferences));
                            strcpy(pf->PrtDevName,"envoyprint");

                        }
                        else
                        {
                            /* Delete the kludge */
                            DeleteFile("SYS:WBStartup/Envoy V37 PrintStart");
                            DeleteFile("SYS:WBStartup/Envoy V37 PrintStart.info");

                            /* Change it immediately */
                            GetPrefs(pf,sizeof(struct Preferences));
                            pf->PrtDevName[0]=0;
                        }
                        SetPrefs(pf,sizeof(struct Preferences),TRUE);
                        FreeMem(pf,sizeof(struct Preferences));
                    }
                }
                CloseLibrary(VersionBase);
            }
            CloseLibrary(IntuitionBase);
        }
        if ( (p->pr_CLI) && (run) )
        {
            if (install)
                PutStr("Printing redirected through Envoy\n");
            else
                PutStr("Envoy network printing removed\n");
        }
        CloseLibrary(DOSBase);
        if (!p->pr_CLI)         /* If WB, yank out msg */
        {
            Forbid();
            ReplyMsg(w);
        }
    }

}

/*
 * success = MakeIcon(STRPTR name)
 *
 * Use icon.library to create a given default icon.
 *
 * Entry:
 *              name -- ptr to string of name of file for which
 *                      icon should be created.  Icon will be
 *                      name+".info".
 *
 * Exit:
 *              success -- either TRUE or FALSE, indicating
 *                         whether the operation has succeeded
 *                         or not.
 *
 */
BOOL MakeIcon(STRPTR name)
{

    struct Library *SysBase=(*(void **)4L);
    struct Library *IconBase;
    struct DiskObject *mdo;
    IconBase = OpenLibrary("icon.library",37L);
    if (IconBase)
    {
        mdo = (struct DiskObject *) GetDefDiskObject(WBTOOL);
        if (mdo)
        {
            PutDiskObject(name,mdo);
            FreeDiskObject(mdo);
        }
        else
        {
            CloseLibrary(IconBase);
            return(FALSE);
        }
        CloseLibrary(IconBase);
    }
    return(TRUE);
}




