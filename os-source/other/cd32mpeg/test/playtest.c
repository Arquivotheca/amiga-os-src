#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "/mpeg.h"
#include "asyncio.h"

#define TEMPLATE	"NAME"
#define DATA_SIZE	2324

struct Library *SysBase;
struct Library *DOSBase;


ULONG OpenTest(VOID)
{
    struct RDArgs *rdargs;
    struct MsgPort *port;
    struct IOMPEGReq *iomr_cmd,*iomr_data;
    LONG args[4]={0,0,0,0};
    struct AsyncFile *file;
    ULONG iocnt=0,cnt;
    ULONG STYPE = MPEGSTREAM_VIDEO;

    BOOL done=FALSE;
    BOOL setType = TRUE;

    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(rdargs = ReadArgs(TEMPLATE, args, NULL))
        {
            if(port = CreateMsgPort())
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
                                for(cnt = 0; cnt < 100; cnt++)
                                {
                                    if(iomr_data = (struct IOMPEGReq *)CreateIORequest(port,sizeof(struct IOMPEGReq)+DATA_SIZE))
                                    {
                                    	iomr_data->iomr_Req.io_Device = iomr_cmd->iomr_Req.io_Device;
                                    	iomr_data->iomr_Req.io_Unit   = iomr_cmd->iomr_Req.io_Unit;
                                    	iomr_data->iomr_Req.io_Data = (APTR)((ULONG)iomr_data + sizeof(struct IOMPEGReq));

                                    	if((iomr_data->iomr_Req.io_Length = ReadAsync(file,iomr_data->iomr_Req.io_Data,DATA_SIZE)) > 0)
                                    	{
                                    	    if(setType && (*(ULONG *)iomr_data->iomr_Req.io_Data == 0x1ba))
                                    	    {
                                    	        STYPE = MPEGSTREAM_SYSTEM;
                                    	        setType = FALSE;
                                    	    }
                                    	    iomr_data->iomr_Req.io_Command = CMD_WRITE;
                                    	    iomr_data->iomr_StreamType = STYPE;

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
                                iomr_cmd->iomr_Req.io_Command = MPEGCMD_PLAY;
                                iomr_cmd->iomr_StreamType = STYPE;
                                DoIO((struct IORequest *)iomr_cmd);

                                while(!done)
                                {
                                    WaitPort(port);
                                    while(iomr_data = (struct IOMPEGReq *)GetMsg(port))
                                    {
                                        iocnt--;
                                        if((iomr_data->iomr_Req.io_Length = ReadAsync(file,iomr_data->iomr_Req.io_Data,DATA_SIZE)) != 0)
                                        {
                                            iomr_data->iomr_Req.io_Command = CMD_WRITE;
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
                                while(iocnt)
                                {
                                    WaitPort(port);
                                    while(iomr_data = (struct IOMPEGReq *)GetMsg(port))
                                    {
                                        iocnt--;
                                        DeleteIORequest((struct IORequest *)iomr_data);
                                    }
                                }
                                iomr_cmd->iomr_Req.io_Command = CMD_STOP;
                                DoIO((struct IORequest *)iomr_cmd);

                                CloseAsync(file);
                            }
                        }
                        CloseDevice((struct IORequest *)iomr_cmd);
                    }
                    else
                        PutStr("Device open failed.\n");

                    DeleteIORequest((struct IORequest *)iomr_cmd);
                }
                DeleteMsgPort(port);
            }
            CloseLibrary(DOSBase);
        }
    }
    return(0L);
}