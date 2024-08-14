#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <devices/cd.h>
#include <graphics/rastport.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include "/mpeg.h"
#include "asyncio.h"

#define TEMPLATE	"TRACK/N,SHOW/S"
#define DATA_SIZE	2328

struct Library *SysBase,*DOSBase,*IntuitionBase,*GfxBase;

extern VOID __asm WCP(register __a0 struct  RastPort *rp, register __d0 LONG xstart,
		      register __d1 LONG ystart, register __d2 LONG xstop,
		      register __d3 LONG ystop, register __a2 UBYTE *a2,
		      register __d4 LONG bytesperrow, register __a6 struct Library *GfxBase);

extern void InitColorDither(void);
extern void EndColorDither(void);
extern void ColorDitherImage(unsigned char *lum, unsigned char *cr,    unsigned char *cb,
			     unsigned char *red, unsigned char *green, unsigned char *blue,
			     int cols, int rows);

ULONG OpenTest(VOID)
{
    struct RDArgs *rdargs;
    struct MsgPort *port,*data_port,*playport;
    struct IOMPEGReq *iomr_cmd,*iomr_data,*iomr_PlayCmd;
    struct IOStdReq *iocd;
    LONG args[4]={0,0,0,0};
    struct AsyncFile *file;
    ULONG iocnt=0,cnt;
    ULONG streamType = 0;
    WORD showVParams=100;
    BOOL done=FALSE,pend=FALSE;
    ULONG signals,waitsignals;
    struct Screen *myScreen;
    struct Window *myWindow;
    union CDTOC *toc;
    struct MPEGFrameStore mfs;
    UBYTE *red, *green, *blue;

    LONG pause, slow, scan;

    ULONG secTags[] = { TAGCD_SECTORSIZE, 2328, TAG_END, TRUE };

    pause = slow = scan = FALSE;

    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(IntuitionBase = OpenLibrary("intuition.library",37L))
        {
            if(GfxBase = OpenLibrary("graphics.library",40L))
            {
                if(rdargs = ReadArgs(TEMPLATE, args, NULL))
                {
                    if(port = CreateMsgPort())
                    {
                        if(playport = CreateMsgPort())
                        {
                            if(data_port = CreateMsgPort())
                            {
                                if(iomr_cmd = (struct IOMPEGReq *)CreateIORequest(port,sizeof(struct IOMPEGReq)))
                                {
                                    if(iocd = (struct IOStdReq *)CreateIORequest(port,sizeof(struct IOStdReq)))
                                    {
                                        if(iomr_PlayCmd = (struct IOMPEGReq *)CreateIORequest(playport,sizeof(struct IOMPEGReq)))
                                        {
                                            if(!OpenDevice("cd.device",0,(struct IORequest *)iocd,0))
                                            {
                                                Printf("cd.device opened.\n");

                                                if(toc = AllocVec(40*sizeof(union CDTOC),MEMF_CLEAR))
                                                {
                                                        Printf("Got TOC memory.\n");

                                                    iocd->io_Command = CD_TOCLSN;
                                                    iocd->io_Data = (APTR)toc;
                                                    iocd->io_Length = 40;

                                                    if(!DoIO((struct IORequest *)iocd))
                                                    {
                                                        Printf("Read TOC\n");

                                                        /* Ick */

                                                        toc[toc[0].Summary.LastTrack+1].Entry.Position.LSN = toc[0].Summary.LeadOut.LSN;

                                                        if(!OpenDevice("cd32mpeg.device",0,(struct IORequest *)iomr_cmd,0))
                                                        {
                                                            PutStr("Device opened okay.\n");

                                                            if(args[0])
                                                            {


                                                                if(myScreen = OpenScreenTags(NULL, SA_Overscan, OSCAN_VIDEO,
                                                                                                   SA_ShowTitle, FALSE,
                                                                                                   SA_Depth, 2,
                                                                                                   SA_Behind, FALSE,
                                                                                                   SA_Quiet, TRUE,
                                                                                                   SA_DisplayID, DEFAULT_MONITOR_ID,
                                                                                                   TAG_DONE))
                                                                {
                                                                    if(myWindow = OpenWindowTags(NULL, WA_Activate, FALSE,
                                                                                                       WA_CustomScreen, myScreen,
                                                                                                       WA_Flags, WFLG_SIMPLE_REFRESH | WFLG_BORDERLESS | WFLG_BACKDROP,
                                                                                                       TAG_DONE))

                                                                    {
                                                                        {
                                                                            struct MPEGVideoParamsSet mvp;
                                                                            mvp.mvp_Fade = 65535;
                                                                            iomr_cmd->iomr_Req.io_Data = &mvp;
                                                                            iomr_cmd->iomr_Req.io_Command = MPEGCMD_SETVIDEOPARAMS;
                                                                            iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGVideoParamsSet);
                                                                            DoIO((struct IORequest *)iomr_cmd);
                                                                        }
                                                                        iomr_PlayCmd->iomr_Req.io_Device = iomr_cmd->iomr_Req.io_Device;
                                                                        iomr_PlayCmd->iomr_Req.io_Unit   = iomr_cmd->iomr_Req.io_Unit;

                                                                        iomr_PlayCmd->iomr_Req.io_Command = MPEGCMD_PLAYLSN;
                                                                        iomr_PlayCmd->iomr_Req.io_Data = 0;
                                                                        iomr_PlayCmd->iomr_Req.io_Length = toc[*(LONG *)args[0]+1].Entry.Position.LSN - toc[*(LONG *)args[0]].Entry.Position.LSN;
                                                                        iomr_PlayCmd->iomr_Req.io_Offset = toc[*(LONG *)args[0]].Entry.Position.LSN;
                                                                        iomr_PlayCmd->iomr_MPEGFlags = 0;
                                                                        iomr_PlayCmd->iomr_StreamType = MPEGSTREAM_SYSTEM;
                                                                        iomr_PlayCmd->iomr_Arg1 = 2328;
									iomr_PlayCmd->iomr_Arg2 = toc[*(LONG *)args[0]].Entry.Position.LSN;

                                                                        SendIO((struct IORequest *)iomr_PlayCmd);
									pend = TRUE;

                                                                        waitsignals = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D |
                                                                                      SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F |
                                                                                      1L << port->mp_SigBit |
                                                                                      1L << playport->mp_SigBit |
                                                                                      1L << data_port->mp_SigBit;

                                                                        while(!done)
                                                                        {
                                                                            signals = Wait(waitsignals);

                                                                            if(signals & SIGBREAKF_CTRL_C)
                                                                            {
                                                                                if(!CheckIO((struct IORequest *)iomr_PlayCmd))
                                                                                {
                                                                                    AbortIO((struct IORequest *)iomr_PlayCmd);
                                                                                    WaitIO((struct IORequest *)iomr_PlayCmd);
                                                                                }
                                                                                iomr_cmd->iomr_Req.io_Command = CMD_FLUSH;
                                                                                DoIO((struct IORequest *)iomr_cmd);
                                                                                done = TRUE;
                                                                            }
                                                                            if(GetMsg(playport))
                                                                            {
                                                                                done = TRUE;
                                                                                break;
                                                                            }
                                                                        }
                                                                        CloseWindow(myWindow);
                                                                    }
                                                                    CloseScreen(myScreen);
                                                                }
                                                            }
                                                            CloseDevice((struct IORequest *)iomr_cmd);
                                                        }
                                                        else
                                                            PutStr("Device open failed.\n");
                                                    }
                                                    FreeVec(toc);
                                                }
                                                CloseDevice((struct IORequest *)iocd);
                                            }
                                            DeleteIORequest((struct IORequest *)iomr_PlayCmd);
                                        }
                                        DeleteIORequest((struct IORequest *)iocd);
                                    }
                                    DeleteIORequest((struct IORequest *)iomr_cmd);
                                }
                                DeleteMsgPort(data_port);
                            }
                            DeleteMsgPort(playport);
                        }
                        DeleteMsgPort(port);
                    }
                    CloseLibrary(GfxBase);
                }
                CloseLibrary(IntuitionBase);
            }
            CloseLibrary(DOSBase);
        }
    }
    return(0L);
}

