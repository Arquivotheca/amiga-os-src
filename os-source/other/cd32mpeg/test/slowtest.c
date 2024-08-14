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


#define TEMPLATE	"NAME"
#define DATA_SIZE	2324

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
    struct IOMPEGReq *iomr_cmd,*iomr_data;
    LONG args[4]={0,0,0,0};
    ULONG iocnt=0,cnt;
    ULONG streamType = 0;
    WORD showVParams=100;
    BOOL done=FALSE;
    struct AsyncFile *file;
    ULONG signals,waitsignals;
    struct Screen *myScreen;
    struct Window *myWindow;

    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        Printf("%ld\n",sizeof(struct MPEGUnit));

        if(IntuitionBase = OpenLibrary("intuition.library",37L))
        {
            if(rdargs = ReadArgs(TEMPLATE, args, NULL))
            {
                if(port = CreateMsgPort())
                {
                    if(data_port = CreateMsgPort())
                    {
                        if(iomr_cmd = (struct IOMPEGReq *)CreateIORequest(port,sizeof(struct IOMPEGReq)))
                        {
                            if(!OpenDevice("cd32mpeg.device",0,(struct IORequest *)iomr_cmd,0))
                            {
                                PutStr("Device opened okay.\n");

                                if(args[0])
                                {
                                    if(file = OpenAsync((STRPTR)args[0],MODE_READ,65536))
                                    {
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

                                                for(cnt = 0; cnt < 50; cnt++)
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

                                                    while(iomr_data = (struct IOMPEGReq *)GetMsg(data_port))
                                                    {
                                                        signals = SetSignal(0L,SIGBREAKF_CTRL_C);

                                                        if(signals & SIGBREAKF_CTRL_C)
                                                        {
                                                            done=TRUE;
                                                        }

                                                        iocnt--;

                                                        if(!done)
                                                        {
                                                            if((iomr_data->iomr_Req.io_Length = ReadAsync(file,iomr_data->iomr_Req.io_Data,DATA_SIZE)) > 0)
                                                            {
                                                            	if(*(ULONG *)iomr_data->iomr_Req.io_Data != 0x1ba)
                                                            	{
                                                            		;
//                                                            	    Printf("Error Detected. Code: %08lx\n",*(ULONG *)iomr_data->iomr_Req.io_Data);
//                                                            	    DeleteIORequest((struct IORequest *)iomr_data);
//                                                            	    done = TRUE;
                                                            	}
                                                            	else
                                                            	{
                                                                    iomr_data->iomr_Req.io_Command = CMD_WRITE;
                                                                    SendIO((struct IORequest *)iomr_data);
                                                                    iocnt++;
                                                                }
                                                            }
                                                            else
                                                            {
                                                            	DeleteIORequest((struct IORequest *)iomr_data);
                                                                done = TRUE;
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
                                        CloseAsync(file);
                                    }
                                }
                                CloseDevice((struct IORequest *)iomr_cmd);
                            }
                            else
                                PutStr("Device open failed.\n");

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