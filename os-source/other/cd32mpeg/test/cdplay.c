#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <devices/cd.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "/mpeg.h"

#define TEMPLATE	"TRACK/N"
#define DATA_SIZE	2324
#define SECTOR_SIZE	2328

UWORD my_Colors[12]=
{
	0,	0,	0x0,	0x0,
	1,	0xeeee,	0xeeee,	0xeeee,
	0xffff,	0,	0,	0
};

struct Library *SysBase,*DOSBase,*IntuitionBase;

struct TagItem cdTags1[] =
{
    { TAGCD_SECTORSIZE, 2328},
    { TAG_END, TRUE}
};

struct TagItem cdTags2[] =
{
    { TAGCD_SECTORSIZE, 2048},
    { TAG_END, TRUE}
};

ULONG OpenTest(VOID)
{
    struct RDArgs *rdargs;
    struct MsgPort *port,*data_port,*cd_port;
    struct IOStdReq *ioCD;
    struct IOMPEGReq *iomr_cmd,*iomr_data;
    LONG args[4]={0,0,0,0};
    union CDTOC toc[10];
    ULONG iocnt=0,cnt=0;
    ULONG streamType = 0;
    WORD showVParams=100;
    BOOL done=FALSE;
    ULONG signals,waitsignals;
    struct Screen *myScreen;
    struct Window *myWindow;
    ULONG trackNum, cdOffset;
    ULONG *data;
    BPTR file;

    streamType = MPEGSTREAM_SYSTEM;

    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(rdargs = ReadArgs(TEMPLATE, args, NULL))
        {
            if(args[0])
            {
                trackNum = *(ULONG *)args[0];

                if(cd_port = CreateMsgPort())
                {
                    if(ioCD = CreateIORequest(cd_port,sizeof(struct IOStdReq)))
                    {
                        if(!OpenDevice("cd.device",0,(struct IORequest *)ioCD,0L))
                        {
                            ioCD->io_Command = CD_TOCLSN;
                            ioCD->io_Data = toc;
                            ioCD->io_Length = 10;
                            ioCD->io_Offset = 0;

                            DoIO((struct IORequest *)ioCD);

                            toc[toc[0].Summary.LastTrack+1].Entry.Position.LSN = toc[0].Summary.LeadOut.LSN;

//                            cdOffset = toc[trackNum].Entry.Position.LSN*SECTOR_SIZE;
                            cdOffset = 0x15e0*SECTOR_SIZE;

                            ioCD->io_Data = (APTR)cdTags1;
                            ioCD->io_Length = 0;
                            ioCD->io_Command = CD_CONFIG;

                            if(!DoIO((struct IORequest *)ioCD))
                            {
                                if(data = AllocVec(SECTOR_SIZE,0))
                                {
                                    if(file = Open("sys:cdi.mpg",MODE_NEWFILE))
                                    {
                                        while(1)
                                        {
                                            if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
                                                break;

                                            if(cnt > 17000)
                                                break;

                                            ioCD->io_Data = data;
                                            ioCD->io_Length = SECTOR_SIZE;
                                            ioCD->io_Offset = cdOffset;
                                            ioCD->io_Command = CD_READ;

                                            cdOffset += SECTOR_SIZE;
                                            cnt++;

                                            if(!DoIO((struct IORequest *)ioCD))
                                            {
                                                Write(file,data,2328);
                                            }
                                        }
                                        Close(file);
                                    }
                                    FreeVec(data);
                                }
                                ioCD->io_Data = (APTR)cdTags2;
                                ioCD->io_Length = 0;
                                ioCD->io_Command = CD_CONFIG;
                            }
                            CloseDevice((struct IORequest *)ioCD);
                        }
                        DeleteIORequest((struct IORequest *)ioCD);
                    }
                    DeleteMsgPort(cd_port);
                }
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
    return(0L);
}