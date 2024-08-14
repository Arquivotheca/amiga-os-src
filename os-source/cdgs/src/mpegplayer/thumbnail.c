
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <graphics/text.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <dos/dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "mpegplayerbase.h"
#include "tasks.h"
#include "utils.h"
#include "defs.h"
#include "diskinfo.h"
#include "halvechunky.h"
#include "thumbnail.h"


/*****************************************************************************/


#undef SysBase

static void ThumbnailTask(void)
{
struct MPEGPlayerLib      *MPEGPlayerBase;
struct ExecBase           *SysBase;
struct MsgPort            *port;
struct IOMPEGReq          *ioreq;
struct IOMPEGReq          *ioreq2;
struct MPEGFrameStore      fs;
struct MPEGVideoParamsGet  vpg;
struct CDSequence         *track;
APTR                       dst;
LONG                       x0,x1,y0,y1,w,h;
struct RastPort            rp1,rp2;
BOOL                       first;

    SysBase        = (*((struct ExecBase **) 4));
    MPEGPlayerBase = SysBase->ThisTask->tc_UserData;
    track          = MPEGPlayerBase->mp_ThumbnailTrack;

    rp1 = MPEGPlayerBase->mp_DBRastPort[0];
    SetWriteMask(&rp1,0xff);

    rp2 = MPEGPlayerBase->mp_DBRastPort[1];
    SetWriteMask(&rp2,0xff);

    port   = CreateMsgPort();
    ioreq  = CreateIORequest(port,sizeof(struct IOMPEGReq));
    ioreq2 = CreateIORequest(port,sizeof(struct IOMPEGReq));
    ioreq->iomr_Req.io_Device  = MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Device;
    ioreq->iomr_Req.io_Unit    = MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Unit;
    ioreq2->iomr_Req.io_Device = MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Device;
    ioreq2->iomr_Req.io_Unit   = MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Unit;

    fs.mfs_Luma = NULL;
    fs.mfs_Cr   = NULL;
    fs.mfs_Cb   = NULL;
    first       = TRUE;
    dst         = NULL;

    ioreq2->iomr_Req.io_Command = MPEGCMD_PAUSE;
    ioreq2->iomr_Req.io_Data    = NULL;
    ioreq2->iomr_Req.io_Length  = 0;
    ioreq2->iomr_Req.io_Offset  = 0;
    ioreq2->iomr_PauseMode      = TRUE;
    DoIO(ioreq2);

    ioreq->iomr_Req.io_Command = MPEGCMD_PLAYLSN;
    ioreq->iomr_Req.io_Data    = NULL;
    ioreq->iomr_Req.io_Length  = track->cds_TrackEnd - track->cds_TrackStart + 1;
    ioreq->iomr_Req.io_Offset  = track->cds_TrackStart + 5*SECTORS_PER_SECOND;
    ioreq->iomr_StreamType     = MPEGSTREAM_SYSTEM;
    ioreq->iomr_StreamStart    = track->cds_TrackStart;
    ioreq->iomr_SectorSize     = MPEG_SECTOR_SIZE;
    SendIO(ioreq);

    while ((SetSignal(0,0) & SIGBREAKF_CTRL_C) == 0)
    {
        ioreq2->iomr_Req.io_Command = MPEGCMD_SCAN;
        ioreq2->iomr_Req.io_Data    = NULL;
        ioreq2->iomr_Req.io_Length  = 0;
        ioreq2->iomr_Req.io_Offset  = 0;
        ioreq2->iomr_SearchSpeed    = 150;
        ioreq2->iomr_MPEGFlags     |= MPEGF_ONESHOT;
        DoIO(ioreq2);

        if (!fs.mfs_Luma)
        {
            ioreq2->iomr_Req.io_Command = MPEGCMD_GETVIDEOPARAMS;
            ioreq2->iomr_Req.io_Data    = &vpg;
            ioreq2->iomr_Req.io_Length  = sizeof(vpg);
            ioreq2->iomr_MPEGFlags      = 0;
            DoIO(ioreq2);

            fs.mfs_Width  = vpg.mvp_PictureWidth;
            fs.mfs_Height = vpg.mvp_PictureHeight;
            fs.mfs_Luma   = AllocVec(fs.mfs_Width * fs.mfs_Height,MEMF_ANY);

            w  = (fs.mfs_Width >> 1);
            h  = (fs.mfs_Height >> 1);
            x0 = (((CREDIT_LEFT-PAGE_LEFT) - w) / 2) + PAGE_LEFT;
            y0 = (PAGE_HEIGHT - h) / 2 + PAGE_TOP;
            x1 = x0+w-1;
            y1 = y0+h-1;

            dst = AllocVec(w*h,MEMF_ANY);
        }

	ioreq2->iomr_Req.io_Command = MPEGCMD_READFRAMEYUV;
	ioreq2->iomr_Req.io_Data    = &fs;
	ioreq2->iomr_Req.io_Length  = sizeof(fs);
	if (DoIO(ioreq2) == 0)
	{
            HalveChunky(fs.mfs_Luma, dst, fs.mfs_Width, fs.mfs_Height);

            if (first)
            {
                first = FALSE;

                SetABPenDrMd(&rp1,2,0,JAM2);
                RectFill(&rp1,x0-3,y0-2,x1+3,y0-1);
                RectFill(&rp1,x1+1,y0,x1+3,y1);
                RectFill(&rp1,x0-3,y1+1,x1+3,y1+2);
                RectFill(&rp1,x0-3,y0,x0-1,y1);

                SetABPenDrMd(&rp2,2,0,JAM2);
                RectFill(&rp2,x0-3,y0-2,x1+3,y0-1);
                RectFill(&rp2,x1+1,y0,x1+3,y1);
                RectFill(&rp2,x0-3,y1+1,x1+3,y1+2);
                RectFill(&rp2,x0-3,y0,x0-1,y1);
            }

            WCP(GfxBase,&rp1,x0,y0,x1,y1,dst,fs.mfs_Width >> 1);
            WCP(GfxBase,&rp2,x0,y0,x1,y1,dst,fs.mfs_Width >> 1);
	}

	if (CheckIO(ioreq))
	{
	    WaitIO(ioreq);
            ioreq->iomr_Req.io_Command = MPEGCMD_PLAYLSN;
            ioreq->iomr_Req.io_Data    = NULL;
            ioreq->iomr_Req.io_Length  = track->cds_TrackEnd - track->cds_TrackStart + 1;
            ioreq->iomr_Req.io_Offset  = track->cds_TrackStart + 5*SECTORS_PER_SECOND;
            ioreq->iomr_StreamType     = MPEGSTREAM_SYSTEM;
            ioreq->iomr_StreamStart    = track->cds_TrackStart;
            ioreq->iomr_SectorSize     = MPEG_SECTOR_SIZE;
            SendIO(ioreq);
	}
    }

    AbortIO(ioreq);
    WaitIO(ioreq);

    FreeVec(fs.mfs_Luma);
    FreeVec(dst);
    DeleteMsgPort(port);
    DeleteIORequest(ioreq);
    DeleteIORequest(ioreq2);

    Forbid();
    MPEGPlayerBase->mp_ThumbnailTask = NULL;
}

#define SysBase MPEGPlayerBase->mp_SysLib


/*****************************************************************************/


void StartThumbnail(struct MPEGPlayerLib *MPEGPlayerBase,
		    struct CDSequence *track)
{
    if (!MPEGPlayerBase->mp_ThumbnailTask)
    {
        MPEGPlayerBase->mp_ThumbnailTrack = track;
        MPEGPlayerBase->mp_ThumbnailTask = MakeTask(MPEGPlayerBase, "Thumber", -1,
                                                    ThumbnailTask, 4000);
    }
}


/*****************************************************************************/


void StopThumbnail(struct MPEGPlayerLib *MPEGPlayerBase)
{
    if (MPEGPlayerBase->mp_ThumbnailTask)
    {
        Signal(MPEGPlayerBase->mp_ThumbnailTask,SIGBREAKF_CTRL_C);

        while (MPEGPlayerBase->mp_ThumbnailTask)
            WaitTOF();
    }
}
