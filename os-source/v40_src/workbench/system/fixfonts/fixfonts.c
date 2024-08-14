
/* FixFonts - update the font contents files in FONTS: */

/* TODO: output error messages! */

/* includes */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/exall.h>
#include <graphics/text.h>
#include <libraries/diskfont.h>
#include <workbench/startup.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* application includes */
#include "fixfonts_rev.h"


/*****************************************************************************/


#define	LIB_VERSION 36


/*****************************************************************************/


int main(int argc, char *argv[])
{
UBYTE                       exAllBuffer[1500];
struct ExAllControl        *eac;
struct ExAllData           *ead;
struct DevProc             *dvp;
struct MsgPort             *prevFS;
LONG                        prevCurrDir,fontFile;
char                        fontName[MAXFONTNAME];
struct FontContentsHeader  *fch;
LONG                        result;
struct Process	           *process;
BOOL			    more;
BPTR                        lock;

struct Library *SysBase = (*((struct Library **) 4));
struct Library *DOSBase;
struct Library *DiskfontBase;
struct WBStartup *WBenchMsg = NULL;


    process = (struct Process *)FindTask(NULL);
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    result = 20;

    if (DOSBase = OpenLibrary(DOSNAME VERSTAG,LIB_VERSION))
    {
        if (DiskfontBase = OpenLibrary("diskfont.library",34))
        {
            if (eac = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL,0))
            {
                result = 0;

                /* get FONTS: DevProc entry */
                dvp = (struct DevProc *) GetDeviceProc("FONTS:",0);

                while (dvp && (!(SetSignal(0,0) & SIGBREAKF_CTRL_C)) && (result == 0))
                {
                    prevFS      = SetFileSysTask(dvp->dvp_Port);
                    prevCurrDir = CurrentDir(dvp->dvp_Lock);

                    if (lock = DupLock(dvp->dvp_Lock))
                    {
                        eac->eac_LastKey = 0;

                        /* look in all directories in FONTS: directory */
                        do
                        {
                            more = ExAll(lock,(struct ExAllData *)exAllBuffer,sizeof(exAllBuffer),ED_TYPE,eac);
                            if ((!more) && (IoErr() != ERROR_NO_MORE_ENTRIES))
                            {
                                break;
                            }

                            if ((eac->eac_Entries > 0) && (result == 0))
                            {
                                ead = (struct ExAllData *) exAllBuffer;
                                do
                                {
                                    if ((ead->ed_Type > 0) && (strlen(ead->ed_Name) <= 25) && (!(SetSignal(0,0) & SIGBREAKF_CTRL_C)))
                                    {
                                        strcpy(fontName,ead->ed_Name);
                                        strcat(fontName,".font");

                                        if (fch = NewFontContents(dvp->dvp_Lock,fontName))
                                        {
                                            if (fch->fch_NumEntries > 0)
                                            {
                                                if (fontFile = Open(fontName,MODE_NEWFILE))
                                                {
                                                    /* write out font contents file */
                                                    if (Write(fontFile,fch,((LONG *) fch)[-1] - 4) < 0)
                                                    {
                                                        result = 10;
                                                    }
                                                    Close(fontFile);
                                                    SetProtection(fontName,FIBF_EXECUTE);
                                                }
                                                else
                                                {
                                                    result = 10;
                                                }
                                            }
                                            else
                                            {
                                                DeleteFile(fontName);
                                            }
                                            DisposeFontContents(fch);
                                        }
                                        else
                                        {
                                            result = 10;
                                        }
                                    }
                                    ead = ead->ed_Next;
                                }
                                while (ead);
                            }
                        }
                        while (more);

                        UnLock(lock);
                    }
                    CurrentDir(prevCurrDir);
                    SetFileSysTask(prevFS);
                    dvp = GetDeviceProc("FONTS:",dvp);
                }

                if (dvp)
                    FreeDeviceProc(dvp);

                FreeDosObject(DOS_EXALLCONTROL,eac);
            }
            CloseLibrary(DiskfontBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(result);
}
