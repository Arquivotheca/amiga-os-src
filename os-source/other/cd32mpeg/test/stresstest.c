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
#include "/mpeg_device.h"
#undef SysBase
#include "asyncio.h"


#define TEMPLATE	"NAME,SAVE/K"
#define DATA_SIZE	2328

UWORD my_Colors[12]=
{
	0,	0,	0x0,	0x0,
	1,	0xeeee,	0xeeee,	0xeeee,
	0xffff,	0,	0,	0
};

struct Library *SysBase,*DOSBase,*IntuitionBase;

ULONG OpenTest(VOID)
{

    struct RDArgs *rdargs;
    struct MsgPort *port,*data_port;
    struct IOMPEGReq *iomr_cmd,*iomr_data,*iomr_cmd2;
    LONG args[4]={0,0,0,0};
    ULONG iocnt=0,cnt;
    ULONG streamType = 0;
    WORD showVParams=100;
    BOOL done=FALSE;
    struct AsyncFile *file,*savefile=NULL;
    ULONG signals,waitsignals;
    struct Screen *myScreen;
    struct Window *myWindow;
	BOOL flag=FALSE;
	BOOL flag2=FALSE;
    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        Printf("%ld\n",sizeof(struct MPEGUnit));

        if(IntuitionBase = OpenLibrary("intuition.library",37L))
        {
            args[0]=args[1]=0L;
            if(rdargs = ReadArgs(TEMPLATE, args, NULL))
            {
                if(port = CreateMsgPort())
                {
                    if(data_port = CreateMsgPort())
                    {
                        if(iomr_cmd = (struct IOMPEGReq *)CreateIORequest(port,sizeof(struct IOMPEGReq)))
                        {
                            if(iomr_cmd2 = (struct IOMPEGReq *)CreateIORequest(port,sizeof(struct IOMPEGReq)))
                            {
                                if(!OpenDevice("cd32mpeg.device",0,(struct IORequest *)iomr_cmd,0))
                                {
                                    iomr_cmd2->iomr_Req.io_Device = iomr_cmd->iomr_Req.io_Device;
                                    iomr_cmd2->iomr_Req.io_Unit   = iomr_cmd->iomr_Req.io_Unit;

                                    PutStr("Device opened okay.\n");

                                    if(args[0])
                                    {
                                        if(file = OpenAsync((STRPTR)args[0],MODE_READ,65536))
                                        {
                                            if(args[1])
                                                savefile = OpenAsync((STRPTR)args[1],MODE_WRITE,65536);

                                            if(myScreen = OpenScreenTags(NULL, SA_Overscan, OSCAN_VIDEO,
                                                                               SA_ShowTitle, FALSE,
                                                                               SA_Depth, 1,
                                                                               SA_Behind, FALSE,
                                                                               SA_Colors, &my_Colors,
                                                                               SA_Quiet, TRUE,
                                                                               SA_DisplayID, DEFAULT_MONITOR_ID|LORESLACE_KEY,
                                                                               TAG_DONE))
                                            {
                                                if(myWindow = OpenWindowTags(NULL, WA_Activate, FALSE,
                                                                                   WA_CustomScreen, myScreen,
                                                                                   WA_Flags, WFLG_SIMPLE_REFRESH | WFLG_BORDERLESS | WFLG_BACKDROP,
                                                                                   TAG_DONE))

                                                {

                                                    for(cnt = 0; cnt < 5; cnt++)
                                                    {
                                                        if(iomr_data = (struct IOMPEGReq *)CreateIORequest(data_port,sizeof(struct IOMPEGReq)+DATA_SIZE))
                                                        {
                                                            iomr_data->iomr_Req.io_Device = iomr_cmd->iomr_Req.io_Device;
                                                            iomr_data->iomr_Req.io_Unit   = iomr_cmd->iomr_Req.io_Unit;
                                                            iomr_data->iomr_Req.io_Data = (APTR)((ULONG)iomr_data + sizeof(struct IOMPEGReq));

                                                            if((iomr_data->iomr_Req.io_Length = ReadAsync(file,iomr_data->iomr_Req.io_Data,DATA_SIZE)) > 0)
                                                            {
                                                                if(!streamType)
                                                                {
                                                                    switch(*(ULONG *)iomr_data->iomr_Req.io_Data)
                                                                    {
                                                                        case 0x1BA  : streamType = MPEGSTREAM_SYSTEM;
                                                                                      break;

                                                                        case 0x1B3  : streamType = MPEGSTREAM_VIDEO;
                                                                                      break;

                                                                        case 0xfffda080 : streamType = MPEGSTREAM_AUDIO;
                                                                                          break;
                                                                    }
                                                                }
                                                                iomr_data->iomr_Req.io_Command = CMD_WRITE;
                                                                iomr_data->iomr_StreamType = streamType;

                                                                SendIO((struct IORequest *)iomr_data);
                                                                iocnt++;
                                                            }
                                                            else
                                                            {
                                                                done = TRUE;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                    {
                                                        struct MPEGVideoParamsSet mvp;
                                                        mvp.mvp_Fade = 65535;
                                                        mvp.mvp_DisplayType = 0;
                                                        iomr_cmd->iomr_Req.io_Data = &mvp;
                                                        iomr_cmd->iomr_Req.io_Command = MPEGCMD_SETVIDEOPARAMS;
                                                        iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGVideoParamsSet);
                                                        DoIO((struct IORequest *)iomr_cmd);
						    }
                                                    iomr_cmd->iomr_Req.io_Command = MPEGCMD_PLAY;
                                                    iomr_cmd->iomr_StreamType = streamType;
                                                    iomr_cmd->iomr_SlowSpeed = 4;

                                                    SendIO((struct IORequest *)iomr_cmd);

                                                    waitsignals = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D |
                                                                  SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F |
                                                                  1L << port->mp_SigBit |
                                                                  1L << data_port->mp_SigBit;

                                                    while(!done || iocnt)
                                                    {
                                                        signals = Wait(waitsignals);

                                                        if(signals & SIGBREAKF_CTRL_C)
                                                        {
                                                            done=TRUE;
                                                        }
                                                        if(signals & SIGBREAKF_CTRL_F)
                                                        {
                                                            SeekAsync(file,DATA_SIZE,MODE_CURRENT);
                                                            iomr_cmd2->iomr_Req.io_Command = CMD_FLUSH;
//                                                            Printf("VHPOSR: %04lx\n",(*(UWORD *)0xdff006)>>8);
                                                            DoIO((struct IORequest *)iomr_cmd2);
                                                            flag=TRUE;
                                                        }
                                                        if(signals & SIGBREAKF_CTRL_E)
                                                        {
                                                            SeekAsync(file,-DATA_SIZE,MODE_CURRENT);
//                                                            Printf("VHPOSR: %04lx\n",(*(UWORD *)0xdff006)>>8);
                                                            iomr_cmd2->iomr_Req.io_Command = CMD_FLUSH;
                                                            DoIO((struct IORequest *)iomr_cmd2);
                                                            flag=TRUE;
                                                        }
#if 1
                                                        if(signals & SIGBREAKF_CTRL_D)
                                                        {
                                                            struct MPEGWindowParams mwp;
                                                            mwp.mwp_XOffset = 16;
                                                            mwp.mwp_YOffset = 0;
                                                            mwp.mwp_Width = 0;
                                                            mwp.mwp_Height = 0;
                                                            iomr_cmd2->iomr_Req.io_Data = &mwp;
                                                            iomr_cmd2->iomr_Req.io_Command = MPEGCMD_SETWINDOW;
                                                            iomr_cmd2->iomr_Req.io_Length = sizeof(struct MPEGWindowParams);
                                                            DoIO((struct IORequest *)iomr_cmd2);
                                                        }
#endif
                                                        while(iomr_data = (struct IOMPEGReq *)GetMsg(data_port))
                                                        {
#if 0
                                                            if(flag2)
                                                            {
                                                                flag2=FALSE;
                                                                iomr_cmd2->iomr_Req.io_Command = CMD_FLUSH;
                                                                DoIO((struct IORequest *)iomr_cmd2);
                                                            }
#endif
                                                            if(!iomr_data->iomr_Req.io_Error && savefile)
                                                            {
                                                                if(flag)
                                                                {
                                                                    flag=FALSE;
                                                                    *(ULONG *)iomr_data->iomr_Req.io_Data = 0x1ba;
                                                                }
                                                                WriteAsync(savefile,iomr_data->iomr_Req.io_Data,DATA_SIZE);
                                                            }
                                                            signals = SetSignal(0L,SIGBREAKF_CTRL_C);

                                                            if(signals & SIGBREAKF_CTRL_C)
                                                            {
                                                                done=TRUE;
                                                            }

                                                            iocnt--;

                                                            if(!done)
                                                            {
                                                            	while(1)
                                                            	{
                                                                    if((iomr_data->iomr_Req.io_Length = ReadAsync(file,iomr_data->iomr_Req.io_Data,DATA_SIZE)) > 0)
                                                                    {
                                                                        if(*(ULONG *)iomr_data->iomr_Req.io_Data == 0x1ba)
                                                                        {
                                                                            iomr_data->iomr_Req.io_Command = CMD_WRITE;
                                                                            SendIO((struct IORequest *)iomr_data);
                                                                            iocnt++;
                                                                            break;
                                                                        }
                                                                        else if(*(ULONG *)iomr_data->iomr_Req.io_Data == 0x1aa)
                                                                        {
                                                                            *(ULONG *)iomr_data->iomr_Req.io_Data = 0x1ba;
                                                                            iomr_cmd2->iomr_Req.io_Command = CMD_FLUSH;
                                                                            DoIO((struct IORequest *)iomr_cmd2);
									    flag2=TRUE;
                                                                            iomr_data->iomr_Req.io_Command = CMD_WRITE;
                                                                            SendIO((struct IORequest *)iomr_data);
                                                                            iocnt++;
                                                                            break;
									}
                                                                    }
                                                                    else
                                                                    {
                                                                        SeekAsync(file,0,MODE_START);
                                                                    }
                                                                }
                                                            }
                                                            else
                                                                DeleteIORequest((struct IORequest *)iomr_data);

                                                        }
                                                    }
                                                    if(!CheckIO((struct IORequest *)iomr_cmd));
                                                    {
                                                        AbortIO((struct IORequest *)iomr_cmd);
                                                        WaitIO((struct IORequest *)iomr_cmd);
                                                    }
                                                    CloseWindow(myWindow);
                                                }
                                                CloseScreen(myScreen);
                                            }
                                            if(savefile)
                                                CloseAsync(savefile);
                                            CloseAsync(file);
                                        }
                                    }
                                    CloseDevice((struct IORequest *)iomr_cmd);
                                }
                                else
                                    PutStr("Device open failed.\n");

                                DeleteIORequest((struct IORequest *)iomr_cmd2);
                            }
                            DeleteIORequest((struct IORequest *)iomr_cmd);
                        }
                        DeleteMsgPort(data_port);
                    }
                    DeleteMsgPort(port);
                }
                FreeArgs(rdargs);
	    }
            CloseLibrary(IntuitionBase);
        }
        CloseLibrary(DOSBase);
    }
    return(0L);
}