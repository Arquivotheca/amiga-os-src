/*
** $Source: HOG:Other/cd32mpeg/RCS/task.c,v $
** $State: Exp $
** $Revision: 40.1 $
** $Date: 94/01/26 11:55:31 $
** $Author: kcd $
**
** Amiga MPEG device driver.
**
** Basic Device Functions
**
** (C) Copyright 1993 Commodore-Amiga, Inc.
**
*/

#include "mpeg_device.h"

#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <dos/dostags.h>
#include <devices/timer.h>
#include <libraries/configvars.h>
#include <devices/cd.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/timer_protos.h>
#include <clib/expansion_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <pragmas/expansion_pragmas.h>

#include <string.h>
#ifndef _GENPROTO
#include "cl450_protos.h"
#include "l64111_protos.h"
#include "demux_protos.h"
#include "datapump_protos.h"
#include "dev_protos.h"
#include "task_protos.h"
#endif
/*
** External variables and functions
**
*/

#if 0
#define D(x)	x
#else
#define D(x)
#endif

#if 0
#define DS(x)	x
#else
#define DS(x)
#endif

// #define DUMP_CL450 yeppers

#define NEW_UCODE yeppers

// #define NO_AUDIO	yeppers

#define CL450INTERRUPTS	(CL450INTF_DER|CL450INTF_PIC|CL450INTF_SCN|CL450INTF_RDY|CL450INTF_BUF)

#undef SysBase
#define SysBase mpegUnit->mu_SysBase


/*
** FlushVideoStream
**
** Function: Aborts all IOReq's sitting on the parsed video stream queue
**
*/
VOID FlushVideoStream(struct MPEGUnit *mpegUnit)
{
    struct IOMPEGReq *req;

    ObtainSemaphore(&mpegUnit->mu_Lock);
    while(req = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_VideoStream))
    {
	mpegUnit->mu_VPackets--;
        req->iomr_Req.io_Error = IOERR_ABORTED;
        TermIO(req,mpegUnit->mu_Device);
    }
    ReleaseSemaphore(&mpegUnit->mu_Lock);
}

/*
** FlushAudioStream
**
** Function: Aborts all IOReq's sitting on the parsed audio stream queue
**
*/

VOID FlushAudioStream(struct MPEGUnit *mpegUnit)
{
    struct IOMPEGReq *req;
    register int i;
    for(i=0;i<128;i++)
        mpegUnit->mu_VPTS[i]=0;

    ObtainSemaphore(&mpegUnit->mu_Lock);
    while(req = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_AudioStream))
    {
	mpegUnit->mu_APackets--;
        req->iomr_Req.io_Error = IOERR_ABORTED;
        TermIO(req,mpegUnit->mu_Device);
    }
    ReleaseSemaphore(&mpegUnit->mu_Lock);

}

/****** cd32mpeg.device/CMD_FLUSH ********************************************
*
*   NAME
*       CMD_FLUSH -- cancel all pending CMD_WRITE requests
*
*   FUNCION
*       CMD_FLUSH aborts all CMD_WRITE requests in progress or queued.
*       This call will also flush any decoder buffers.  However, the current
*       play state will not be changed.  For the best visual results, the
*	currently executing MPEGCMD_PLAY command should be aborted, or the
*	device should be in a paused state.  Otherwise, undesirable video
*	or audio artifacts may occur due to decoder underdlow.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to reset
*       io_Command   - command number for CMD_FLUSH
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       None
*
*****************************************************************************/

/*
** CMDFlush
**
** Flush all IO requests.
**
*/
VOID CMDFlush(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct IOMPEGReq *req;
    BOOL status;

	D(kprintf("CMDFlush: Entry\n"));

    ObtainSemaphore(&mpegUnit->mu_Lock);

	D(kprintf("CMDFlush: Got semaphore\n"));

    while(req = (struct IOMPEGReq *)GetMsg(&mpegUnit->mu_Unit.unit_MsgPort))
    {
    	req->iomr_Req.io_Error = IOERR_ABORTED;
    	TermIO(req,mpegUnit->mu_Device);
    }

    ReleaseSemaphore(&mpegUnit->mu_Lock);

	D(kprintf("CMDFlush: Released Semaphore. Flushing Video Stream.\n"));

    FlushVideoStream(mpegUnit);

        D(kprintf("CMDFlush: Flushing Audio Stream.\n"));

    FlushAudioStream(mpegUnit);

    /* Flush the currently processing video packet */

	D(kprintf("CMDFlush: Flushing CL450.\n"));

    if(!(status = FlushCL450(mpegUnit)))
    {
        /* Oh shit, something bad happened. :( */
	mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
	mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
	Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
    }

    mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;
    mpegUnit->mu_IsVidOverflow = FALSE;

    if(mpegUnit->mu_VideoIO)
    {
    	mpegUnit->mu_VideoIO->iomr_Req.io_Error = 0;
    	TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
    	mpegUnit->mu_VideoIO = NULL;
    }

    mpegUnit->mu_IsAudOverflow = FALSE;
    /* Same for audio (in case there was one) */
    if(mpegUnit->mu_AudioIO)
    {
    	mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
    	TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
    	mpegUnit->mu_AudioIO = NULL;
    }

    /* This is so that I can call CMDFlush() without an IOMPEGReq */

    if(iomr)
    {
        iomr->iomr_Req.io_Error = 0;
    	TermIO(iomr,mpegUnit->mu_Device);
    }
    	D(kprintf("CMDFlush: Exit\n"));

}

/****** cd32mpeg.device/CMD_RESET ********************************************
*
*   NAME
*       CMD_RESET -- restore device to a known state
*
*   FUNCTION
*       CMD_RESET reset the hardware and/or software of the mpeg device
*       driver to a known state, cancels all pending I/O requests and resets
*       all error conditions.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to reset
*       io_Command   - command number for CMD_RESET
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       None
*
*****************************************************************************/
/*
** CMDReset
**
** Reset the device to a known state.
**
** So, we:
**
**    1) Flush out all pending IO requests with an error (call CMDFlush)
**    2) Reset the CL450
**    3) Reset the L64111
**    4) Reset MPEG Control Register Contents.
*/
VOID CMDReset(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct GfxBase *GfxBase;

    /* Step 1 */

    D(kprintf("CMDReset: Flushing IO queue\n"));

    CMDFlush((struct IOMPEGReq *)NULL,mpegUnit);

    /* Step 2 */

    D(kprintf("CMDReset: Resetting CL450\n"));

    ResetCL450(mpegUnit);

    if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",37L))
    {
        if(GfxBase->DisplayFlags & PAL)
        {
            SetCL450VideoMode(3,mpegUnit);
        }
        else
        {
            SetCL450VideoMode(4,mpegUnit);
        }

        CloseLibrary((struct Library *)GfxBase);
    }

    /* Step 3 */

    D(kprintf("CMDReset: Resetting L64111\n"));

    mpegUnit->mu_AudioID = 0;
    ResetL64111(mpegUnit);

    if(mpegUnit->mu_VideoIO)
    {
    	mpegUnit->mu_VideoIO->iomr_Req.io_Error = 0;
    	TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
    	mpegUnit->mu_VideoIO = NULL;
    }

    /* Same for audio (in case there was one) */
    if(mpegUnit->mu_AudioIO)
    {
    	mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
    	TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
    	mpegUnit->mu_AudioIO = NULL;
    }

    /* Step 4 */

    mpegUnit->mu_VEnable = FALSE;
    mpegUnit->mu_AEnable = FALSE;

    SetBlankMode(mpegUnit);

    if(iomr)
        TermIO(iomr,mpegUnit->mu_Device);
}


/*
** CMDInvalid
**
** Return an IOMPEGReq with io_Error set to IOERR_NOCMD
**
*/
VOID CMDInvalid(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    iomr->iomr_Req.io_Error = IOERR_NOCMD;
    TermIO(iomr,mpegUnit->mu_Device);
}


/****** cd32mpeg.device/CMD_WRITE ********************************************
*
*   NAME
*       CMD_WRITE -- normal I/O entry point
*
*   FUNCTION
*       CMD_WRITE is used to send data to the MPEG device driver when playing
*       back an MPEG stream using the MPEGCMD_PLAY command.
*
*       Depending on the capabilities of the MPEG device driver, the data
*       may be packs from an ISO System stream, or may be raw video and/or
*       audio streams.
*
*	The amount of data the needs to be written to the device depends on
*	the playback mode being used.  For normal rate playback, it would be
*	a good idea to keep a large amount of data queued up in order to
*	avoid video or audio glitches.
*
*	When sending MPEG system streams to the device, each block of data
*	sent by the CMD_WRITE command MUST correspond to exactly one pack
*	of data.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to queue data for
*       io_Command   - command number for CMD_WRITE
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       io_Data      - Pointer to a data buffer that contains valid data
*	io_Length    - Amount of data pointed to by io_Data.
*       iomr_PTSxxxx - A timestamp associated with this data segment.  This
*                      is typically the PTS value if this data segment was
*                      part of a system stream.
*       iomr_MPEGFlags - Status information regarding this data:
*                      MPEGFLAGF_VALIDCLK - io_PTSxxxx contains a valid
*                                           timestamp associated with
*                                           this data.
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occurred.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*   NOTES
*       For efficient operation, it will be best to keep a certain minimum
*       amount of data queued up at any given time, otherwise gaps or glitches
*	may appear in the audio and/or video output.   Approximately 64k for
*	video and 16k for audio is a good starting point.
*
*****************************************************************************/
/*
** CMDWrite
**
** Process a CMD_WRITE IO request.
**
*/
VOID CMDWrite(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
#if 0
    UWORD hi,mid,lo;
    ULONG scr,now;
#endif
    iomr->iomr_MPEGFlags &= ~(MPEGF_VALID_PTS|MPEGF_VALID_SCR);

    ObtainSemaphore(&mpegUnit->mu_Lock);
    switch(iomr->iomr_StreamType)
    {
    	case MPEGSTREAM_VIDEO:	iomr->iomr_DataStart = (ULONG)iomr->iomr_Req.io_Data;
    				iomr->iomr_DataSize = iomr->iomr_Req.io_Length;
    				mpegUnit->mu_VPackets++;
    				AddTail((struct List *)&mpegUnit->mu_VideoStream,(struct Node *)iomr);
    				break;

    	case MPEGSTREAM_AUDIO:	iomr->iomr_DataStart = (ULONG)iomr->iomr_Req.io_Data;
    				iomr->iomr_DataSize = iomr->iomr_Req.io_Length;
    				mpegUnit->mu_APackets++;
    				AddTail((struct List *)&mpegUnit->mu_AudioStream,(struct Node *)iomr);
    				break;

    	case MPEGSTREAM_SYSTEM:	SystemDemux(iomr,mpegUnit,mpegUnit->mu_Device);
#if 0
                                GetSCR(&hi,&mid,&lo,mpegUnit);
                                now=(hi<<30)|(mid<<15)|lo;
                                hi =iomr->iomr_SCRHigh;
                                mid=iomr->iomr_SCRMid;
                                lo =iomr->iomr_SCRLow;
                                scr=(hi<<30)|(mid<<15)|lo;

                                mpegUnit->mu_LastOffset = now-scr;
#endif
    				break;

    	default:		iomr->iomr_Req.io_Error = MPEGERR_BAD_PARAMETER;
	    	            	iomr->iomr_MPEGError = MPEGEXTERR_STREAM_MISMATCH;
    				TermIO(iomr,mpegUnit->mu_Device);
    				break;
    }
    ReleaseSemaphore(&mpegUnit->mu_Lock);

}

/*
** Set Video Enable/Disable mode
*/
VOID SetBlankMode(struct MPEGUnit *mpegUnit)
{
    switch(mpegUnit->mu_ProductID)
    {
        case MPEG_ZORRO2_ID : if(mpegUnit->mu_VEnable)
                                  mpegUnit->mu_ControlVal &= ~Z2_VIDEO_DISABLE;
                              else
                                  mpegUnit->mu_ControlVal |= Z2_VIDEO_DISABLE;
                              break;

        case MPEG_CDGS_ID:    if(mpegUnit->mu_VEnable)
        		      {
                                  mpegUnit->mu_ControlVal |= CD32_MPEG_ENABLE;
                              }
                              else
                              {
                                  mpegUnit->mu_ControlVal &= ~CD32_MPEG_ENABLE;
                              }
                              break;

    }
    *mpegUnit->mu_Control = mpegUnit->mu_ControlVal;
}

/*
** DoPlay
**
** Called to start video and/or audio decoding.  Sets up interrupts, blanking
** states, etc.
*/
BOOL DoPlay(struct MPEGUnit *mpegUnit)
{
    mpegUnit->mu_PlayFlags &= ~(MPEGPF_WAITSYNC|MPEGPF_INTMUTE);

    if(mpegUnit->mu_StreamType & MPEGSTREAM_AUDIO)
    {
    	/* If we are not starting normal playback with Valid timestamps, then
    	   clear out our table. */

    	if(!mpegUnit->mu_ValidPTS)
        {
            /* Clear timestamps */

            register int i;
            for(i=0;i<128;i++)
                mpegUnit->mu_VPTS[i]=0;

        }
        SetSignal(0L,mpegUnit->mu_SyncSigMask);
        mpegUnit->mu_L64111IntAble = 1;
        SetL64111StatusInt1(0,mpegUnit);
        SetL64111StatusInt2(PTS_AVAILABLE_W|NEW_FRAME_OUT_W,mpegUnit);
        PlayL64111(mpegUnit->mu_StreamType,mpegUnit);
        Signal(mpegUnit->mu_Task,1L << mpegUnit->mu_L64111SigBit);
//        mpegUnit->mu_PlayFlags |= MPEGPF_WAITSYNC;
    }

    /* Start appropriate decoders */
    if((mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO) && !(mpegUnit->mu_PlayFlags & MPEGPF_WAITSYNC))
    {
    	if(mpegUnit->mu_ValidPTS)
    	{
            SetSCR(mpegUnit->mu_PTSHi,mpegUnit->mu_PTSMid,mpegUnit->mu_PTSLo,mpegUnit);
            mpegUnit->mu_ValidPTS = FALSE;
        }
        mpegUnit->mu_CL450IntAble = 1;

        if(!SetCL450InterruptMask(CL450INTERRUPTS,mpegUnit))
            return(FALSE);
        if(!BlankCL450(FALSE,mpegUnit))
            return(FALSE);
        if(mpegUnit->mu_PlayFlags & MPEGPF_WAITSYNC)
        {
            if(!PauseCL450(mpegUnit))
                return(FALSE);
        }
        else
	{
            if(!PlayCL450(mpegUnit))
                return(FALSE);
	}
        SetBlankMode(mpegUnit);
    }
    return(TRUE);
}

/*
** DoPause
**
** Called to pause video decoding.
*/
BOOL DoPause(struct MPEGUnit *mpegUnit)
{
    mpegUnit->mu_PlayFlags &= ~MPEGPF_WAITSYNC;

    if(mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO)
    {
    	if(mpegUnit->mu_PlayFlags & MPEGPF_PLAY)
    	{
            if(!GetSCR(&mpegUnit->mu_PTSHi,&mpegUnit->mu_PTSMid,&mpegUnit->mu_PTSLo,mpegUnit))
                return(FALSE);
	}
        mpegUnit->mu_CL450IntAble = 1;
        if(!SetCL450InterruptMask(CL450INTERRUPTS,mpegUnit))
            return(FALSE);
        if(!BlankCL450(FALSE,mpegUnit))
            return(FALSE);
        if(!PauseCL450(mpegUnit))
            return(FALSE);
        SetBlankMode(mpegUnit);
        Disable();
        mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;
        Enable();
    }
    if(mpegUnit->mu_StreamType & MPEGSTREAM_AUDIO)
    {
        PauseL64111(mpegUnit);
    }
    return(TRUE);
}

BOOL DoSlow(struct MPEGUnit *mpegUnit)
{
    /* We need to set this first, otherwise we can miss the first interrupt */
    mpegUnit->mu_PlayFlags &= ~MPEGPF_WAITSYNC;

    Disable();
    mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;

//    mpegUnit->mu_PlayFlags |= MPEGPF_INTMUTE;
    Enable();
    if(mpegUnit->mu_StreamType & MPEGSTREAM_AUDIO)
    {
        ResetL64111(mpegUnit);
        FlushAudioStream(mpegUnit);
    }
    mpegUnit->mu_CL450IntAble = 1;
    if(!SetCL450InterruptMask(CL450INTERRUPTS,mpegUnit))
        return(FALSE);
    if(!BlankCL450(FALSE,mpegUnit))
        return(FALSE);
    if(!SlowMotionCL450(mpegUnit->mu_SlowSpeed,mpegUnit))
        return(FALSE);
    SetBlankMode(mpegUnit);
    return(TRUE);
}

/*
** Ugh.  cd.device doesn't grok CMD_FLUSH. :(
**
** Walk our list of "pending" CD_READ requests and AbortIO() all of them.
**
** Place all of the Aborted IO requests onto the DPReplyPort.
*/

VOID FlushCDReads(struct MPEGUnit *mpegUnit)
{
    struct MinNode *node;
#if 1
    node = mpegUnit->mu_CDIOList.mlh_Head;
    while(node->mln_Succ)
    {
        AbortIO((struct IORequest *)((ULONG)node + sizeof(struct MinNode)));
        node = node->mln_Succ;
    }
#endif
#if 0
    ObtainSemaphore(&mpegUnit->mu_Lock);
    while(node = (struct MinNode *)RemTail((struct List *)&mpegUnit->mu_CDIOList))
    {
        AbortIO((struct IORequest *)((ULONG)node + sizeof(struct MinNode)));
        WaitIO((struct IORequest *)((ULONG)node + sizeof(struct MinNode)));
        PutMsg(mpegUnit->mu_DPReplyPort,(struct Message *)((ULONG)node + sizeof(struct MinNode)));
    }
    ReleaseSemaphore(&mpegUnit->mu_Lock);
#endif
}

/*
** Fire off an IOMPEGReq to cd.device, and add it to our tracking list.
*/
static VOID SendCDIO(struct MPEGUnit *mpegUnit, struct IOMPEGReq *iomr)
{
    /* We don't have to arbitrate for access to this list. */
    AddTail((struct List *)&mpegUnit->mu_CDIOList,(struct Node *)((ULONG)iomr - sizeof(struct MinNode)));
    SendIO((struct IORequest *)iomr);
}

/*
** Remove an IOMPEGReq from our cd.device READ tracking list.
*/

static VOID RemoveCDRead(struct MPEGUnit *mpegUnit, struct IOMPEGReq *iomr)
{
    /* Pointer arithmetic is fun! */
    Remove((struct Node *)((ULONG)iomr - sizeof(struct MinNode)));
}

/*
** Allocate an IOMPEG request to be used with cd.device.  We have a
** special MinNode tacked onto the front of this structure so we can
** keep track of which IO requests have been sent to cd.device.
*/
static struct IOMPEGReq *CreateCDIORequest(struct MPEGUnit *mpegUnit)
{
    APTR mem;
    struct Message *msg;

    if(mem = AllocVec(sizeof(struct IOMPEGReq) + sizeof(struct MinNode),MEMF_CLEAR))
    {
        msg = (struct Message *)((ULONG)mem + sizeof(struct MinNode));
        msg->mn_ReplyPort = mpegUnit->mu_CDReplyPort;
    }
    return((struct IOMPEGReq *)msg);
}

/*
** Delete IOMPEGRequest structure that was being used with cd.device.
*/

static VOID DeleteCDIORequest(struct MPEGUnit *mpegUnit, struct IOMPEGReq *iomr)
{
    /* Bwahaha */
    FreeVec((APTR)((ULONG)iomr - sizeof(struct MinNode)));
}

/*
** StartDataPump
**
*/
ULONG StartDataPump(struct MPEGUnit *mpegUnit)
{
    ULONG cdTags[4]={TAGCD_SECTORSIZE, 0, TAG_END, TRUE};
    ULONG result=0;
    struct IOMPEGReq *iomr;

    mpegUnit->mu_DPReqs=0;

//	DS(kprintf("{StartAvail: %ld}",AvailMem(0)));

    if(mpegUnit->mu_CDCmdIO = (struct IOStdReq *)CreateIORequest(mpegUnit->mu_CDReplyPort,sizeof(struct IOStdReq)))
    {
    	if(!OpenDevice("cd.device",0,(struct IORequest *)mpegUnit->mu_CDCmdIO,0))
    	{
    	    cdTags[1] = mpegUnit->mu_SectorSize;
    	    mpegUnit->mu_CDCmdIO->io_Command = CD_CONFIG;
    	    mpegUnit->mu_CDCmdIO->io_Data    = (APTR)cdTags;
    	    mpegUnit->mu_CDCmdIO->io_Length  = 0;
    	    mpegUnit->mu_CDDevice = mpegUnit->mu_CDCmdIO->io_Device;
    	    mpegUnit->mu_CDUnit   = mpegUnit->mu_CDCmdIO->io_Unit;

    	    if(!DoIO((struct IORequest *)mpegUnit->mu_CDCmdIO))
    	    {
    	    	ULONG i;

    	    	for(i = 0; i < 40; i++)
    	    	{
    	    	    if(iomr = CreateCDIORequest(mpegUnit))
    	    	    {
    	    	    	if(iomr->iomr_Req.io_Data = AllocVec(mpegUnit->mu_SectorSize,MEMF_PUBLIC|MEMF_CLEAR))
    	    	    	{
    	    	    	    mpegUnit->mu_DPReqs++;
    	    	    	    iomr->iomr_Req.io_Device = mpegUnit->mu_CDDevice;
    	    	    	    iomr->iomr_Req.io_Unit = mpegUnit->mu_CDUnit;
    	    	    	    iomr->iomr_Req.io_Length = mpegUnit->mu_SectorSize;
    	    	    	    iomr->iomr_Req.io_Command = CD_READ;
    	    	    	    iomr->iomr_Req.io_Offset = mpegUnit->mu_CDOffset;
    	    	    	    mpegUnit->mu_CDOffset += mpegUnit->mu_SectorSize;
    	    	    	    SendCDIO(mpegUnit, iomr);
    	    	    	    result = TRUE;
    	    	    	}
    	    	    	else
    	    	    	{
    	    	    	    DeleteCDIORequest(mpegUnit,iomr);
    	    	    	    result = 0;
    	    	    	    break;
    	    	    	}
    	    	    }
    	    	    else
    	    	    {
    	    	    	result = 0;
    	    	        break;
    	    	    }
                }
                if(!result)
                {
//                    DS(kprintf("{SDP:NoMem}"));

                    /* Sigh */

                    FlushCDReads(mpegUnit);

                    while(mpegUnit->mu_DPReqs)
                    {
                        Wait(mpegUnit->mu_DPSigMask | mpegUnit->mu_CDSigMask);

                        while(iomr = (struct IOMPEGReq *)GetMsg(mpegUnit->mu_CDReplyPort))
                        {
                            mpegUnit->mu_DPReqs--;
                            FreeVec(iomr->iomr_Req.io_Data);
                            RemoveCDRead(mpegUnit, iomr);
                            DeleteCDIORequest(mpegUnit, iomr);
                        }
                        while(iomr = (struct IOMPEGReq *)GetMsg(mpegUnit->mu_DPReplyPort))
                        {
                            mpegUnit->mu_DPReqs--;
                            FreeVec(iomr->iomr_Req.io_Data);
                            DeleteCDIORequest(mpegUnit, iomr);
                        }
                    }
                    cdTags[1] = 2048;
//                    cdTags[3] = 75;
                    mpegUnit->mu_CDCmdIO->io_Command = CD_CONFIG;
                    mpegUnit->mu_CDCmdIO->io_Data    = (APTR)cdTags;
                    mpegUnit->mu_CDCmdIO->io_Length  = 0;
		    DoIO((struct IORequest *)mpegUnit->mu_CDCmdIO);
		}
            }
#if 0
	    else
	    	DS(kprintf("{SDP:NoConfig}"));
#endif
            if(!result)
                CloseDevice((struct IORequest *)mpegUnit->mu_CDCmdIO);
	}
#if 0
	else
	    DS(kprintf("{SDP:NoCDDev}"));
#endif
	if(!result)
	{
	    DeleteIORequest((struct IORequest *)mpegUnit->mu_CDCmdIO);
	    mpegUnit->mu_CDCmdIO  = NULL;
	    mpegUnit->mu_CDDevice = NULL;
	    mpegUnit->mu_CDUnit   = NULL;
	}
    }
#if 0
    else
        DS(kprintf("{SDP:NoCDIO}"));
    if(result)
        DS(kprintf("{StartDataPump:Ok}"));
    else
        DS(kprintf("{StartDataPump:Fail}"));
#endif
    return(result);
}

/*
** Shut down cd.device data pump.
*/

VOID StopDataPump(struct MPEGUnit *mpegUnit)
{
    ULONG cdTags[4]={TAGCD_SECTORSIZE, 2048, TAG_END, TRUE};
    struct IOMPEGReq *iomr,*req,*tmp;

    DS(kprintf("{StopDataPump}"));

    if(mpegUnit->mu_CDCmdIO)
    {
    	FlushCDReads(mpegUnit);
        FlushVideoStream(mpegUnit);
        FlushAudioStream(mpegUnit);

	/* I'm not sure if this is _really_ needed or not, but what the
	   hell. */

        ObtainSemaphore(&mpegUnit->mu_Lock);

        req = (struct IOMPEGReq *)mpegUnit->mu_Unit.unit_MsgPort.mp_MsgList.lh_Head;
        while(req->iomr_Req.io_Message.mn_Node.ln_Succ)
        {
            tmp = (struct IOMPEGReq *)req->iomr_Req.io_Message.mn_Node.ln_Succ;
            if(req->iomr_Req.io_Command == CMD_WRITE)
            {
                req->iomr_Req.io_Error = 0;
                Remove(&req->iomr_Req.io_Message.mn_Node);
                TermIO(req,mpegUnit->mu_Device);
            };
            req = tmp;
        }
        ReleaseSemaphore(&mpegUnit->mu_Lock);

	while(mpegUnit->mu_DPReqs)
	{
	    Wait(mpegUnit->mu_DPSigMask | mpegUnit->mu_CDSigMask);

            while(iomr = (struct IOMPEGReq *)GetMsg(mpegUnit->mu_CDReplyPort))
            {
            	mpegUnit->mu_DPReqs--;
                FreeVec(iomr->iomr_Req.io_Data);
                RemoveCDRead(mpegUnit, iomr);
                DeleteCDIORequest(mpegUnit, iomr);
            }
            while(iomr = (struct IOMPEGReq *)GetMsg(mpegUnit->mu_DPReplyPort))
            {
            	mpegUnit->mu_DPReqs--;
                FreeVec(iomr->iomr_Req.io_Data);
                DeleteCDIORequest(mpegUnit, iomr);
            }
        }
        mpegUnit->mu_CDCmdIO->io_Command = CD_CONFIG;
        mpegUnit->mu_CDCmdIO->io_Data    = (APTR)cdTags;
        mpegUnit->mu_CDCmdIO->io_Length  = 0;
        DoIO((struct IORequest *)mpegUnit->mu_CDCmdIO);

        CloseDevice((struct IORequest *)mpegUnit->mu_CDCmdIO);

        DeleteIORequest((struct IORequest *)mpegUnit->mu_CDCmdIO);
        mpegUnit->mu_CDCmdIO  = NULL;
    	mpegUnit->mu_CDDevice = NULL;
        mpegUnit->mu_CDUnit   = NULL;
    }
    mpegUnit->mu_DPState = DPSTATE_IDLE;

    DS(kprintf("{StopAvail: %ld}",AvailMem(0)));
}

/****** cd32mpeg.device/MPEGCMD_PLAYLSN *****************************************
*
*   NAME
*       MPEGCMD_PLAYLSN -- Start playing a MPEG stream from CD.
*
*   FUNCTION
*       MPEGCMD_PLAYLSN tells the MPEG device driver to begin standard rate
*       MPEG playback from CD using cd.device.
*
*	The command will not return until it has played io_Length number of
*	sectors or either "end" of the stream is reached.  Play may also be
*	aborted using AbortIO().
*
*	Due to the nature of MPEG video, you MUST supply the position of
*	the start of the MPEG stream you are playing in iomr_Arg2.  This is
*	because the current decoder must read the MPEG Video Sequence
*	Header to dertmine parameters such as picture size, frame rate, etc.
*	Once this has been done, the device will seek to the position given
*	in iomr_Offset.
*
*	If the current Scan speed setting is less than zero, then the device
*	will start scanning from the end of the sequence once it has located
*	the sequence header.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to begin playback on
*       io_Command   - command number for MPEGCMD_PLAY
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       iomr_StreamType  - Set to one of MPEGSTREAM_VIDEO, MPEGSTREAM_AUDIO
*                      or MPEGSTREAM_SYSTEM depending on the stream type.
*	iomr_Data    - unused.
*	iomr_Offset  - Logical sector number to start playback at.
*	iomr_Length  - Number of sectors to play
*	iomr_Arg1    - Sector size to use with cd.device
*	iomr_Arg2    - The real beginning of the stream on the disk.
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/
/*
** CMDPlayLSN
**
** Play stream off of a CD.
**
*/
VOID CMDPlayLSN(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    /* See if we're already playing, and if so, make sure everthing
       matches up, such as the stream type, stream start and end
       positions, sector size, etc. */

    DS(kprintf("[CMDPlayLSN]"));

    if(mpegUnit->mu_PlayFlags & MPEGPF_PLAY)
    {
        ObtainSemaphore(&mpegUnit->mu_Lock);
        AddTail((struct List *)&mpegUnit->mu_PlayQueue,(struct Node *)iomr);
        ReleaseSemaphore(&mpegUnit->mu_Lock);
        return;
    }
    mpegUnit->mu_StreamType   = iomr->iomr_StreamType;
    mpegUnit->mu_SectorSize   = iomr->iomr_Arg1;
    mpegUnit->mu_StreamBegin  = iomr->iomr_Arg2 * mpegUnit->mu_SectorSize;
    mpegUnit->mu_StreamEnd    = (iomr->iomr_Req.io_Offset + iomr->iomr_Req.io_Length - 1) * mpegUnit->mu_SectorSize;
    mpegUnit->mu_IsVidOverflow = FALSE;
    mpegUnit->mu_IsAudOverflow = FALSE;

    if(mpegUnit->mu_ScanSpeed >= 0)
    {
        mpegUnit->mu_StreamOffset = iomr->iomr_Req.io_Offset * mpegUnit->mu_SectorSize;
    }
    else
    {
        mpegUnit->mu_StreamOffset = mpegUnit->mu_StreamEnd - (20*75*mpegUnit->mu_SectorSize);
    }

    /* If starting from ground zero, then kick off the cd.device data pump */

    if(!mpegUnit->mu_DPState)
    {
        mpegUnit->mu_CDOffset = mpegUnit->mu_StreamBegin;

    	if(!StartDataPump(mpegUnit))
    	{
    	    iomr->iomr_Req.io_Error = MPEGERR_CDERROR;
    	    TermIO(iomr,mpegUnit->mu_Device);
    	    return;
    	}

    	if((mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO)  &&
	   (mpegUnit->mu_StreamOffset != mpegUnit->mu_StreamBegin))
	{
    	    mpegUnit->mu_DPState = DPSTATE_FINDSEQHEADER;
    	}
    	else
    	{
    	    mpegUnit->mu_DPState = DPSTATE_HOLDING;
    	}
    }
    Disable();
    mpegUnit->mu_PlayFlags |= MPEGPF_PLAY;
    Enable();

    /*
    ** If we need to find the sequence header for playing video, then blank out the
    ** display while the CL450 looks for it.  Once we get the SEQ interrupt, then
    ** we'll flush and seek to the "real" start position on the disk, wait for the
    ** first new picture display and then unblank the CL450.  Ugh, what a mess.
    */


    if(mpegUnit->mu_DPState == DPSTATE_FINDSEQHEADER)
    {
        mpegUnit->mu_CL450IntAble = 1;

        if(!BlankCL450(TRUE,mpegUnit))
        {
            /* Oh shit, something bad happened. :( */
            iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
            iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
            mpegUnit->mu_PlayFlags &= MPEGPF_PLAY;
            StopDataPump(mpegUnit);
            TermIO(iomr,mpegUnit->mu_Device);
            return;
        }


        /* Place the CL450 into Pause mode.  This will allow it to decode data, */
        /* But only up until the first picture.  We'd like to stop feeding it as */
        /* soon as possible so we can seek to the real place in the stream. */

	/* Note that I'm actually using Play mode here, because for some reason,
	   Pause doesn't want to generate a PIC interrupt. */

	/* Something to try would be to use the SCAN interrupt, which would minimize
	   the amount of data that we had to feed to the CL450 before realizing that
	   the sequence header has been found.  To do this, you would need to have
	   a way to make the ProcessCL450Interrupt() routine somehow ignore the scan
	   interrupt and instead treat it as if it was a PIC interrupt. */

        if(!(BlankCL450(TRUE,mpegUnit) && SetCL450InterruptMask(CL450INTERRUPTS,mpegUnit) && PlayCL450(mpegUnit)))
        {
            /* Oh shit, something bad happened. :( */
            iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
            iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
            mpegUnit->mu_PlayFlags &= MPEGPF_PLAY;
            StopDataPump(mpegUnit);
            TermIO(iomr,mpegUnit->mu_Device);
            return;
        }
        SetBlankMode(mpegUnit);
        Disable();
        mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;
        Enable();
    }
    else
    {
        /* Okay, since Scan and Pause mode don't really need to have a lot of */
        /* data queued up, we can initiate these modes immediately, provied */
        /* that we don't need to find the sequence header for this track */

        if(mpegUnit->mu_PlayFlags & MPEGPF_SCAN)
        {
            if(DoScan(mpegUnit))
            {
                mpegUnit->mu_DPState = DPSTATE_RUNNING;
            }
            else
            {
                /* Oh shit, something bad happened. :( */
                iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                mpegUnit->mu_PlayFlags &= MPEGPF_PLAY;
                StopDataPump(mpegUnit);
                TermIO(iomr,mpegUnit->mu_Device);
                return;
            }
        }
        else if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
        {
            if(DoPause(mpegUnit))
            {
                mpegUnit->mu_DPState = DPSTATE_RUNNING;
            }
            else
            {
                /* Oh shit, something bad happened. :( */
                iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                mpegUnit->mu_PlayFlags &= MPEGPF_PLAY;
                StopDataPump(mpegUnit);
                TermIO(iomr,mpegUnit->mu_Device);
                return;
            }
        }

        /* If we're in our "holding pattern", Slow/Normal playback */
        /* Will happen later on. */
    }
    mpegUnit->mu_CurrentPlayCmd = iomr;
    mpegUnit->mu_AEnable = 1;
}

/****** cd32mpeg.device/MPEGCMD_PLAY *****************************************
*
*   NAME
*       MPEGCMD_PLAY -- Start decoding a MPEG stream
*
*   FUNCTION
*       MPEGCMD_PLAY tells the MPEG device driver to begin standard rate MPEG
*       playback.
*
*	The MPEG stream data must be supplied via the CMD_WRITE device
*	command.  If you are playing back MPEG from a CD-ROM, you may wish to
*	use the MPEGCMD_PLAYLSN command.
*
*	This command currently will run forever until it is aborted.  Future
*       versions of the device driver may automatically detect the end of an
*	MPEG stream.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to begin playback on
*       io_Command   - command number for MPEGCMD_PLAY
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       iomr_StreamType  - Set to one of MPEGSTREAM_VIDEO, MPEGSTREAM_AUDIO
*                      or MPEGSTREAM_SYSTEM depending on the stream type.
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*   NOTES
*       Best results will be acheived if you queue up some data before
*       sending this command.  See cd32mpeg.device/CMD_WRITE for more details.
*
*****************************************************************************/
/*
** CMDPlay
**
** Start full-speed MPEG decode with external data input
**
** Notes:
**
** Should we support independant playing of separate audio and video
** streams?  Can we support this in the future (like the CL450A?).
** How useful is it?
*/
VOID CMDPlay(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    BOOL status=TRUE;

    /* See if we're already playing, and if so, make sure the
       iomr_StreamType matches what we're already playing. */

	DS(kprintf("[CMDPlay]"));

    if(mpegUnit->mu_PlayFlags & MPEGPF_PLAY)
    {
    	ObtainSemaphore(&mpegUnit->mu_Lock);
    	AddTail((struct List *)&mpegUnit->mu_PlayQueue,(struct Node *)iomr);
    	ReleaseSemaphore(&mpegUnit->mu_Lock);
    	return;
    }
    mpegUnit->mu_StreamType = iomr->iomr_StreamType;
    mpegUnit->mu_IsVidOverflow = FALSE;
    mpegUnit->mu_IsAudOverflow = FALSE;

    if(mpegUnit->mu_PlayFlags & MPEGPF_SCAN)
    {
        if(!DoScan(mpegUnit))
            status=FALSE;
    }
    else if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
    {
        if(!DoPause(mpegUnit))
            status=FALSE;
    }
    else if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
    {
        if(!DoSlow(mpegUnit))
            status=FALSE;
    }
    else
    {
        if(!DoPlay(mpegUnit))
            status=FALSE;
    }
    if(status)
    {
        *mpegUnit->mu_Control = mpegUnit->mu_ControlVal;
        Disable();
        mpegUnit->mu_PlayFlags |= MPEGPF_PLAY;
        Enable();
        mpegUnit->mu_CurrentPlayCmd = iomr;
    }
    else
    {
        /* Oh shit, something bad happened. :( */
        iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
        mpegUnit->mu_PlayFlags &= MPEGPF_PLAY;
        TermIO(iomr,mpegUnit->mu_Device);
        return;
    }

}

/****** cd32mpeg.device/MPEGCMD_SEEKLSN *****************************************
*
*   NAME
*       MPEGCMD_PLAY -- Seek to an arbitrary position with an MPEG stream.
*
*   FUNCTION
*       MPEGCMD_SEEKLSN tells the MPEG device driver to seek to an arbitrary
*	position withing the stream when playing from CD.
*
*	This command may only be used while there is a currently executing
*	MPEGCMD_PLAYLSN command.  Also, the fields of the IOMPEGReq structure
*	must be set up exactly as if you were issuing the MPEGCMD_SEEKLSN
*	command.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to begin playback on
*       io_Command   - command number for MPEGCMD_PLAY
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       iomr_StreamType  - Set to one of MPEGSTREAM_VIDEO, MPEGSTREAM_AUDIO
*                      or MPEGSTREAM_SYSTEM depending on the stream type.
*	iomr_Data    - unused.
*	iomr_Offset  - Logical sector number to seek to.
*	iomr_Length  - Number of sectors to play
*	iomr_Arg1    - Sector size to use with cd.device
*	iomr_Arg2    - The real beginning of the stream on the disk.
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/
/*
** CMDSeekLSN
**
** Seek to a new position within a Stream on CD.
**
*/
VOID CMDSeekLSN(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    /* See if we're already playing, and if so, make sure everthing
       matches up, such as the stream type, stream start and end
       positions, sector size, etc. */

    BOOL noTerm = FALSE;

    DS(kprintf("[CMDSeekLSN]"));

    if((mpegUnit->mu_PlayFlags & MPEGPF_PLAY) && mpegUnit->mu_DPState && (mpegUnit->mu_DPState != DPSTATE_STOPPING))
    {
        /* Do some sanity checking.  Make sure they're not trying to switch
           streams on us. */

        if((mpegUnit->mu_StreamType  == iomr->iomr_StreamType)	&&
           (mpegUnit->mu_SectorSize  == iomr->iomr_Arg1)	&&
           (mpegUnit->mu_StreamBegin == iomr->iomr_Arg2 * mpegUnit->mu_SectorSize) &&
           (mpegUnit->mu_StreamEnd   == (iomr->iomr_Req.io_Offset + iomr->iomr_Req.io_Length - 1) * mpegUnit->mu_SectorSize))
        {
            /* Okay, we think they're sane.  Now, this is a bit tricky.  It is
               possible that we may be in a state where we are looking for the sequence
               header within this stream.  If this is the case, the *only* thing we have
               to do is modify the mu_StreamOffset variable, and ProcessCL450Interrupt()
               will take care of the rest.

               Otherwise, we need to Flush the current stream and then modify the mu_CDOffset
               variable to be equal to where we want to seek to. */

            if(mpegUnit->mu_DPState == DPSTATE_FINDSEQHEADER)
            {
                mpegUnit->mu_StreamOffset = iomr->iomr_Req.io_Offset * mpegUnit->mu_SectorSize;
            }
            else
            {
            	/* New Addition:  It's possible for this command to show up before we've found
            	                  a sequence header if they started playing from the beginning of
            	                  the video stream.  This was found to be a serious problem with
            	                  CD-I movie disks.  So, we make sure that we've displayed at least
            	                  one picture before doing this.  If we haven't, we place the command
            	                  on our queue of pending requests waiting for the sequence header to
            	                  show up. */

            	if(mpegUnit->mu_PicCount)
            	{
                    /* Flush out the data pump. */

    //              kprintf("Last Offset: %ld\n",mpegUnit->mu_LastOffset);
    #if 0
                    if(mpegUnit->mu_VideoIO || GetNextVideoIO(mpegUnit))
                    {
                        kprintf("VideoIO Offset: %ld\n",mpegUnit->mu_VideoIO->iomr_Req.io_Offset+(mpegUnit->mu_SectorSize-mpegUnit->mu_VideoSize));
                    }
    #endif

                    FlushCDReads(mpegUnit);
                    FlushVideoStream(mpegUnit);
                    FlushAudioStream(mpegUnit);

                    /* Flush out the CL450 and reset the L64111. */

                    if(!(mpegUnit->mu_PlayFlags & (MPEGPF_PAUSE|MPEGPF_SCAN)) && (mpegUnit->mu_DPState != DPSTATE_HOLDING))
                    {
                        mpegUnit->mu_DPState = DPSTATE_HOLDING;
                    }
                    if(!FlushCL450(mpegUnit))
                    {
                        /* Oh shit, something bad happened. :( */
                        iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                        iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                        mpegUnit->mu_PlayFlags &= MPEGPF_PLAY;
                        TermIO(iomr,mpegUnit->mu_Device);

                        mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                        mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                        Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
                        return;
                    }
                    DoScan(mpegUnit);
                    ResetL64111(mpegUnit);

                    /* Flush the currently processing video packet */

                    mpegUnit->mu_IsVidOverflow = FALSE;
                    mpegUnit->mu_IsAudOverflow = FALSE;
                    mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;

                    if(mpegUnit->mu_VideoIO)
                    {
                        mpegUnit->mu_VideoIO->iomr_Req.io_Error = 0;
                        TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
                        mpegUnit->mu_VideoIO = NULL;
                    }

                    if(mpegUnit->mu_AudioIO)
                    {
                        mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
                        TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
                        mpegUnit->mu_AudioIO = NULL;
                    }
                    /* Set the next place to read from on the CD. */
                    mpegUnit->mu_CDOffset = iomr->iomr_Req.io_Offset * mpegUnit->mu_SectorSize;
    //                kprintf("SeekLSN To: %ld\n",mpegUnit->mu_CDOffset);

                    iomr->iomr_Req.io_Error = 0;
                }
                else
                {
                    ObtainSemaphore(&mpegUnit->mu_Lock);
                    AddTail((struct List *)&mpegUnit->mu_SeqHeadQueue,(struct Node *)iomr);
                    ReleaseSemaphore(&mpegUnit->mu_Lock);
                    noTerm = TRUE;
                }
            }

        }
        else
	{
            iomr->iomr_Req.io_Error = MPEGERR_BAD_STATE;
	}
    }
    else
    {
        iomr->iomr_Req.io_Error = MPEGERR_BAD_STATE;
    }
    if(!noTerm)
        TermIO(iomr,mpegUnit->mu_Device);
}

/*
** CMDStop
**
** Stop MPEG decoding
**
*/
VOID CMDStop(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    DS(kprintf("[CMDStop]"));

    if(mpegUnit->mu_PlayFlags & MPEGPF_PLAY)
    {
    	if(mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO)
    	{
    	    D(kprintf("CMDStop: Resetting CL450\n"));
    	    ResetCL450(mpegUnit);
    	    D(kprintf("CMDStop: CL450 Has been reset.\n"));
            mpegUnit->mu_CL450IntAble = 1;
    	    Disable();
            mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;
            Enable();

        }
    	if(mpegUnit->mu_StreamType & MPEGSTREAM_AUDIO)
    	{
	    mpegUnit->mu_L64111IntAble = 1;
    	    ResetL64111(mpegUnit);
	}
    }
    mpegUnit->mu_IsVidOverflow = FALSE;
    mpegUnit->mu_IsAudOverflow = FALSE;
    if(mpegUnit->mu_VideoIO)
    {
    D(kprintf("CMDStop: TermIO(VideoIO)\n"));
	TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
	mpegUnit->mu_VideoIO = NULL;
    }
    if(mpegUnit->mu_AudioIO)
    {
    D(kprintf("CMDStop: TermIO(AudioIO)\n"));
    	TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
    	mpegUnit->mu_AudioIO = NULL;
    }
    if(mpegUnit->mu_ScanIO)
    {
    D(kprintf("CMDStop: TermIO(ScanIO)\n"));
    	mpegUnit->mu_ScanIO->iomr_Req.io_Error = IOERR_ABORTED;
    	TermIO(mpegUnit->mu_ScanIO,mpegUnit->mu_Device);
    	mpegUnit->mu_ScanIO = NULL;
    }

    if(mpegUnit->mu_DPState)
    {
    D(kprintf("CMDStop: Stopping data pump\n"));
        StopDataPump(mpegUnit);
    }

    Disable();
    mpegUnit->mu_PlayFlags &= ~MPEGPF_PLAY;
    Enable();
    mpegUnit->mu_StreamType = 0;
    mpegUnit->mu_PendingScan = 0;

    if(iomr)
    {
        iomr->iomr_Req.io_Error = 0;
        TermIO(iomr,mpegUnit->mu_Device);
    }
}

BOOL DoScan(struct MPEGUnit *mpegUnit)
{
    BOOL status=TRUE;
    mpegUnit->mu_PlayFlags &= ~MPEGPF_WAITSYNC;
    if(mpegUnit->mu_StreamType & MPEGSTREAM_AUDIO)
    {
        ResetL64111(mpegUnit);

        FlushAudioStream(mpegUnit);
    }
    {
        register int i;
        for(i=0;i<128;i++)
            mpegUnit->mu_VPTS[i]=0;

    }
    if((mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO) && !mpegUnit->mu_PendingScan)
    {
        mpegUnit->mu_PendingScan = TRUE;
        mpegUnit->mu_ValidPTS = FALSE;
        mpegUnit->mu_CL450IntAble = 1;
        Disable();
        mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;
        mpegUnit->mu_PlayFlags |= MPEGPF_INTMUTE;
        Enable();
        if(!(SetCL450InterruptMask(CL450INTERRUPTS,mpegUnit) && BlankCL450(FALSE,mpegUnit)
             && ScanCL450(mpegUnit)))
            status=FALSE;
        SetBlankMode(mpegUnit);
    }
    return(status);
}


BOOL DoStep(struct MPEGUnit *mpegUnit)
{
    BOOL status=TRUE;
    mpegUnit->mu_ValidPTS = 0;
    mpegUnit->mu_PlayFlags &= ~MPEGPF_WAITSYNC;

    if(mpegUnit->mu_StreamType & MPEGSTREAM_AUDIO)
    {
        ResetL64111(mpegUnit);

        /* Flush Audio Buffers */
        if(mpegUnit->mu_AudioIO)
        {
            mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
            TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
            mpegUnit->mu_AudioIO = NULL;
        }
        FlushAudioStream(mpegUnit);
    }
    if(mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO)
    {
        if(!(SetCL450InterruptMask(CL450INTERRUPTS,mpegUnit) &&
             BlankCL450(FALSE,mpegUnit) &&
             SingleStepCL450(mpegUnit)))
            status=FALSE;
        SetBlankMode(mpegUnit);
        Disable();
        mpegUnit->mu_PlayFlags &= ~MPEGPF_SYNC;
        Enable();
    }
    return(status);
}

#ifdef DUMP_CL450

extern ULONG DramDumpStart;
extern ULONG DramDumpEnd;
extern ULONG DumpCL450Flag;

VOID DumpCL450(struct MPEGUnit *mpegUnit)
{
    ULONG i,j;
    UWORD cmem_status1,cmem_status2;

    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    Disable();

    WriteCL450Register(HOST_RADDR1,READ_INT_STATUS);
    kprintf("Interrupt Status: %04lx\n",ReadCL450Register(HOST_RDATA1));

    InquireCL450BufferFullness(mpegUnit);
    WriteCL450Register(HOST_RADDR1,READ_BS_BUF_FULL);
    kprintf("Buffer Fullness: %ld\n",ReadCL450Register(HOST_RDATA1));

    cmem_status1 = ReadCL450Register(CMEM_STATUS);
    kprintf("CMEM_STATUS1: %04lx\n",cmem_status1);

    for(i=0; i<12; i++)
        *(volatile UWORD *)mpegUnit->mu_CL450Fifo = 0xff;

    cmem_status2 = ReadCL450Register(CMEM_STATUS);
    kprintf("CMEM_STATUS2: %04lx\n",cmem_status2);
    if(cmem_status1 == cmem_status2)
        kprintf("CMEM LOCKUP!\n");

    Enable();

#if 0
    WrCL450Register(CPU_CONTROL,0);

    kprintf("Microcode Halted.\n");

    kprintf("CPU_PC:     %04lx\n",RdCL450Register(CPU_PC));
    kprintf("CPU_INT:    %04lx\n",RdCL450Register(CPU_INT));
    kprintf("CPU_INTENB: %04lx\n\n",RdCL450Register(CPU_INTENB));
    kprintf("DRAM_CMDENABLE: %04lx\n",RdCL450Register(DRAM_CMDENABLE));
    for(i = 0; i < 128; i = i + 8)
    {
        kprintf("TMEM %02lx : ",i);

        for(j=0; j < 8; j++)
        {
            WrCL450Register(CPU_TADDR,i+j);
            kprintf("%04lx ",RdCL450Register(CPU_TMEM));
        }
        kprintf("\n");
    }
    kprintf("\n");
    for(i = DramDumpStart; i < DramDumpEnd; i = i + 16)
    {
        kprintf("DRAM %08lx : %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n",i>>1,
        	ReadCL450Dram(i),
        	ReadCL450Dram((i+2)),
        	ReadCL450Dram((i+4)),
        	ReadCL450Dram((i+6)),
        	ReadCL450Dram((i+8)),
        	ReadCL450Dram((i+10)),
        	ReadCL450Dram((i+12)),
        	ReadCL450Dram((i+14)));
    }

    kprintf("Reinitializing CL450 Microcode.\n");
    InitCL450(mpegUnit);
#endif
}
#endif

/****** cd32mpeg.device/MPEGCMD_SLOWMOTION ***********************************
*
*   NAME
*       MPEGCMD_SLOWMOTION -- Set slow motion playback mode.
*
*   FUNCTION
*	MPEGCMD_SLOWMOTION is used to set or clear slow motion playbakc.
*
*	For this command, iomr_Arg1 is used to set the slow-motion frame
*	rate.  The display framerate will be equal to (stream frame rate/
*	iomr_Arg1).  So a value of 2 would play the stream at half speed,
*	4 would be quarter speed, etc.  A value of 0 will clear the slow
*	motion mode.  A value of 1 will cause video to play at the normal
*	rate, but no audio decoding will occur.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied
*                      from I/O block set by) OpenDevice function
*       io_Unit      - unit number to step
*       io_Command   - command number for MPEGCMD_SLOWMOTION
*	iomr_Arg1    - see above.
*       io_Flags     - flags, must be cleared if not used:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/

/*
** CMDSlow
**
** Set slow-motion mode
**
*/
VOID CMDSlow(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    BOOL change=FALSE;
    BOOL status=TRUE;
    if((iomr->iomr_Arg1) && (!(mpegUnit->mu_PlayFlags & MPEGPF_SLOW) || iomr->iomr_Arg1 != mpegUnit->mu_SlowSpeed))
    {
    	Disable();
        mpegUnit->mu_PlayFlags |= MPEGPF_SLOW;
        Enable();
        mpegUnit->mu_SlowSpeed = iomr->iomr_Arg1;
	DS(kprintf("[CMDSlowOn]"));
        change = TRUE;
#ifdef DUMP_CL450
        if(DumpCL450Flag)
	    DumpCL450(mpegUnit);
#endif
    }
    else if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
    {
    	Disable();
        mpegUnit->mu_PlayFlags &= ~MPEGPF_SLOW;
        Enable();
	DS(kprintf("[CMDSlowOff]"));
        mpegUnit->mu_SlowSpeed = 0;
        change = TRUE;
    }
#ifdef DUMP_CL450
    if(!DumpCL450Flag)
    {
#endif
        if(change)
        {
            if((mpegUnit->mu_PlayFlags & MPEGPF_PLAY) &&
               !(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE) &&
               !(mpegUnit->mu_PlayFlags & MPEGPF_SCAN))
            {
                if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
                {
                    if(!DoSlow(mpegUnit))
                        status=FALSE;
                }
                else
                {
                    if(!DoPlay(mpegUnit))
                        status=FALSE;
                }
            }
        }
#ifdef DUMP_CL450
    }
#endif
    if(status)
    {
        iomr->iomr_Req.io_Error = 0;
    }
    else
    {
        /* Oh shit, something bad happened. :( */
        iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;

        mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
        Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_PAUSE ****************************************
*
*   NAME
*       MPEGCMD_PAUSE -- Set the pause mode of the device
*
*   FUNCTION
*       MPEGCMD_PAUSE is used to temporarily suspend the playback or recording
*       process of an MPEG stream, the intention being to resume playback at
*       some later time.  All state information as well as any unused buffers
*       will be maintained by the device driver.  The last displayed video
*	frame will remain on the screen, and any audio output will be
*	suspended.
*
*	To enable pause, io_Arg1 should be set to a non-zero value.  To
*	resume normal/slow motion playback, io_Arg1 should be cleared.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to begin playback on
*       io_Command   - command number for MPEGCMD_PLAY
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       iomr_StreamType  - Set to one of MPEGSTREAM_VIDEO, MPEGSTREAM_AUDIO or
*                      MPEGSTREAM_SYSTEM depending on the stream type.
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/

/*
** CMDPause
**
** Set Pause mode
**
*/
VOID CMDPause(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    BOOL change=FALSE;
    BOOL status=TRUE;
    if((iomr->iomr_Arg1) && !(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE))
    {
        Disable();
        mpegUnit->mu_PlayFlags |= MPEGPF_PAUSE;
        Enable();
	DS(kprintf("[CMDPauseOn]"));

        change = TRUE;
    }
    else if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
    {
    	Disable();
        mpegUnit->mu_PlayFlags &= ~MPEGPF_PAUSE;
        Enable();
	DS(kprintf("[CMDPauseOff]"));

        change = TRUE;
    }

    if(change)
    {
        if((mpegUnit->mu_PlayFlags & MPEGPF_PLAY) &&
           !(mpegUnit->mu_PlayFlags & MPEGPF_SCAN))
        {
            if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
            {
                if(DoPause(mpegUnit))
		{
                    if(mpegUnit->mu_DPState)
                        mpegUnit->mu_DPState = DPSTATE_RUNNING;

                    if(!(mpegUnit->mu_PlayFlags & MPEGPF_SLOW))
                        mpegUnit->mu_ValidPTS = TRUE;
                }
                else
                    status=FALSE;

            }
            else if(mpegUnit->mu_DPState)
            {
                mpegUnit->mu_DPState = DPSTATE_HOLDING;
            }
	    else if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
            {
                if(!DoSlow(mpegUnit))
                    status=FALSE;
            }
            else
            {
                if(!DoPlay(mpegUnit))
                    status=FALSE;
            }
        }
    }
    if(status)
    {
        iomr->iomr_Req.io_Error = 0;
    }
    else
    {
        /* Oh shit, something bad happened. :( */
        iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;

        mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
        Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_SINGLESTEP ***********************************
*
*   NAME
*       MPEGCMD_STEP -- Step one video frame
*
*   FUNCTION
*       MPEGCMD_STEP may be used with mpeg device drivers that support it to
*       single step through successive frames of an MPEG movie.  This command
*       will cause the device to enable pause mode if it wasn't already.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to step
*       io_Command   - command number for CMD_STEP
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/

/*
** CMDStep
**
** Step ahead one video frame.
**
** Note: This command is a NOP as far as audio is
**       concerned.
*/
VOID CMDStep(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    BOOL status=TRUE;
    D(kprintf("CMDStep: Entry\n"));

	DS(kprintf("[CMDStep]"));

    if(!(mpegUnit->mu_PlayFlags & MPEGPF_PLAY))
    {
        iomr->iomr_Req.io_Error = MPEGERR_BAD_STATE;
        TermIO(iomr,mpegUnit->mu_Device);
        return;

    }
    if(!(mpegUnit->mu_PlayFlags & MPEGPF_SCAN))
    {
    	if(!(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE))
    	{
    	    Disable();
    	    mpegUnit->mu_PlayFlags |= MPEGPF_PAUSE;
    	    Enable();
    	    if(DoPause(mpegUnit))
 	    {
                if(!(mpegUnit->mu_PlayFlags & MPEGPF_SLOW))
                    mpegUnit->mu_ValidPTS = TRUE;
            }
            else
                status=FALSE;
    	}
    	else
    	{
    	    if(DoStep(mpegUnit))
    	        mpegUnit->mu_ValidPTS = FALSE;
    	    else
    	        status=FALSE;
    	}
    }
    if(status)
    {
        iomr->iomr_Req.io_Error = 0;
    }
    else
    {
        /* Oh shit, something bad happened. :( */
        iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;

        mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
        Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
    }
    TermIO(iomr,mpegUnit->mu_Device);
}


/****** cd32mpeg.device/MPEGCMD_SEARCH ***************************************
*
*   NAME
*       MPEGCMD_SEARCH -- Set forward or reverse search mode and/or search
*			  for a video I-frame.
*
*   FUNCTION
*	MPEGCMD_SEARCH is used to either set the forward or reverse search
*	mode, or to do a one-shot search for a video I-frame.
*
*	The iomr_Arg1 field of the IO request should be set to a non-zero
*	value to enable search mode.  If you are playing an MPEG stream
*	using the MPEGCMD_PLAYLSN command, iomr_Arg1 should be set to the
*	number of sectors to skip after each I-frame is found.  This value
*	may be positive or negative.  If you are using the MPEGCMD_PLAY
*	command, this value should be a 1.
*
*	If you would like the device to search for the next I-frame and then
*	enter pause mode, you may set the MPEGF_ONESHOT flag in the
*	iomr_MPEGFlags field of the IO request structure.  In this case, the
*	IO request will not come back until the frame has been found and the
*	device has entered the paused state.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to step
*       io_Command   - command number for MPEGCMD_SEARCH
*       iomr_StreamType  - Set to MPEGSTREAM_SYSTEM or MPEGSTREAM_VIDEO.
*       io_Flags     - flags, must be cleared if not used:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/
/*
** CMDSearch
**
** Set Search mode.
**
** Note: This command is a NOP as far as audio is
**       concerned.
*/
VOID CMDSearch(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    BOOL status=TRUE;
    /* Set the search mode */

    if(mpegUnit->mu_ScanSpeed = iomr->iomr_Arg1)
    {
    	Disable();
        mpegUnit->mu_PlayFlags |= MPEGPF_SCAN;
        Enable();
        DS(kprintf("[CMDSearchOn:%ld]",mpegUnit->mu_ScanSpeed));
    }
    else
    {
	mpegUnit->mu_PendingScan = FALSE;
	Disable();
    	mpegUnit->mu_PlayFlags &= ~MPEGPF_SCAN;
    	Enable();
        DS(kprintf("[CMDSearchOff]"));
    }

    iomr->iomr_Req.io_Error = 0;

    if(iomr->iomr_MPEGFlags & MPEGF_ONESHOT)
    {
        ObtainSemaphore(&mpegUnit->mu_Lock);

        /* If there is a scan request already pending, then queue the new one. */
        if(mpegUnit->mu_ScanIO)
        {
            AddTail((struct List *)&mpegUnit->mu_CMDQueue,(struct Node *)iomr);
        }
        /* If we're not in play mode, this command doesn't do much. */

        else if((mpegUnit->mu_PlayFlags & MPEGPF_PLAY) && (mpegUnit->mu_DPState != DPSTATE_FINDSEQHEADER))
        {
            mpegUnit->mu_ScanIO = iomr;
            if(!DoScan(mpegUnit))
            {
                /* Oh shit, something bad happened. :( */
                iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                TermIO(iomr,mpegUnit->mu_Device);
                mpegUnit->mu_ScanIO = NULL;

                mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
            }

	}
	else
	{
            iomr->iomr_Req.io_Error = MPEGERR_BAD_STATE;
            TermIO(iomr,mpegUnit->mu_Device);
	}
        ReleaseSemaphore(&mpegUnit->mu_Lock);
    }
    else
    {
        if((mpegUnit->mu_PlayFlags & MPEGPF_PLAY) && (mpegUnit->mu_DPState != DPSTATE_FINDSEQHEADER))
        {
            if(mpegUnit->mu_PlayFlags & MPEGPF_SCAN)
            {
                if(!DoScan(mpegUnit))
                    status=FALSE;
            }
            else if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
            {
                if(!DoPause(mpegUnit))
                    status=FALSE;
            }
            else if(mpegUnit->mu_DPState)
            {
                mpegUnit->mu_DPState = DPSTATE_HOLDING;
            }
            else if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
            {
                if(!DoSlow(mpegUnit))
                    status=FALSE;
            }
            else
            {
                if(!DoPlay(mpegUnit))
                    status=FALSE;
            }
	}
        if(status)
        {
            iomr->iomr_Req.io_Error = 0;
        }
        else
        {
            /* Oh shit, something bad happened. :( */
            iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
            iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;

            mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
            mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
            Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
        }
        TermIO(iomr,mpegUnit->mu_Device);
    }
}

#if 0
/*
** Do some quick accountng.
*/
VOID DoAccounting(struct MPEGUnit *mpegUnit)
{
    struct IORequest *ioreq;
    ULONG cnt, total;

    ObtainSemaphore(&mpegUnit->mu_Lock);

    /* I don't want _anything_ to move while I'm doing all of this. */

    Forbid();
    Disable();

    kprintf("DPReq: %ld\n",mpegUnit->mu_DPReqs);

    cnt=total=0;

    ioreq = (struct IORequest *)mpegUnit->mu_CDIOList.mlh_Head;

    while(ioreq->io_Message.mn_Node.ln_Succ)
    {
        cnt++;
        ioreq = (struct IORequest *)ioreq->io_Message.mn_Node.ln_Succ;
    }
    kprintf("CD IO:  %ld\n",cnt);

    total+= cnt;
    cnt = 0;

    ioreq = (struct IORequest *)mpegUnit->mu_VideoStream.mlh_Head;

    while(ioreq->io_Message.mn_Node.ln_Succ)
    {
        cnt++;
        ioreq = (struct IORequest *)ioreq->io_Message.mn_Node.ln_Succ;
    }
    kprintf("Video:  %ld\n",cnt);

    total+= cnt;
    cnt = 0;

    ioreq = (struct IORequest *)mpegUnit->mu_AudioStream.mlh_Head;

    while(ioreq->io_Message.mn_Node.ln_Succ)
    {
        cnt++;
        ioreq = (struct IORequest *)ioreq->io_Message.mn_Node.ln_Succ;
    }
    kprintf("Audio:  %ld\n",cnt);

    total+= cnt;
    cnt = 0;

    ioreq = (struct IORequest *)mpegUnit->mu_DPReplyPort->mp_MsgList.lh_Head;

    while(ioreq->io_Message.mn_Node.ln_Succ)
    {
        cnt++;
        ioreq = (struct IORequest *)ioreq->io_Message.mn_Node.ln_Succ;
    }
    kprintf("DPPrt:  %ld\n",cnt);

    total+= cnt;
    cnt = 0;

    ioreq = (struct IORequest *)mpegUnit->mu_CDReplyPort->mp_MsgList.lh_Head;

    while(ioreq->io_Message.mn_Node.ln_Succ)
    {
        cnt++;
        ioreq = (struct IORequest *)ioreq->io_Message.mn_Node.ln_Succ;
    }
    kprintf("CDPrt:  %ld\n",cnt);

    total+= cnt;

    kprintf("Acntd: %ld\n",total);

    Enable();
    Permit();

    ReleaseSemaphore(&mpegUnit->mu_Lock);
}
#endif

/*
** This is called when the CL450 gives us a SCN interrupt.
**
*/
VOID ScanComplete(struct MPEGUnit *mpegUnit)
{
    struct IOMPEGReq *req;
    BOOL status=TRUE;

//	kprintf("{ScanComplete}");
    /* Disable CL450 Interrupts, but only if we're really in
    /* scan mode. */

    mpegUnit->mu_PicCount++;

    mpegUnit->mu_PlayFlags &= ~MPEGPF_INTMUTE;

    ObtainSemaphore(&mpegUnit->mu_Lock);

    mpegUnit->mu_ValidPTS = 0;
#if 0
    /* Flush out the CL450 Bitstream Buffer */

    if(!FlushCL450(mpegUnit))
    {
        status=FALSE;
        if(mpegUnit->mu_ScanIO)
        {
            mpegUnit->mu_ScanIO->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
            mpegUnit->mu_ScanIO->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
            TermIO(mpegUnit->mu_ScanIO,mpegUnit->mu_Device);
            mpegUnit->mu_ScanIO = NULL;
	}
    }
    /* Flush the currently processing video packet */

    if(mpegUnit->mu_VideoIO)
    {
        mpegUnit->mu_VideoIO->iomr_Req.io_Error = 0;
        TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
        mpegUnit->mu_VideoIO = NULL;
    }

    /* Same for audio (in case there was one) */
    if(mpegUnit->mu_AudioIO)
    {
        mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
        TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
        mpegUnit->mu_AudioIO = NULL;
    }
    mpegUnit->mu_IsVidOverflow = FALSE;
    mpegUnit->mu_IsAudOverflow = FALSE;
#endif

    if(mpegUnit->mu_PendingScan || mpegUnit->mu_ScanIO)
    {
        mpegUnit->mu_PendingScan = FALSE;

        if(mpegUnit->mu_PlayFlags & MPEGPF_SCAN)
        {
            /* Flush out the CL450 Bitstream Buffer */

            if(!FlushCL450(mpegUnit))
            {
                status=FALSE;
                if(mpegUnit->mu_ScanIO)
                {
                    mpegUnit->mu_ScanIO->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                    mpegUnit->mu_ScanIO->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                    TermIO(mpegUnit->mu_ScanIO,mpegUnit->mu_Device);
                    mpegUnit->mu_ScanIO = NULL;
                }
            }
            /* Flush the currently processing video packet */

            if(mpegUnit->mu_VideoIO)
            {
                mpegUnit->mu_VideoIO->iomr_Req.io_Error = 0;
                TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
                mpegUnit->mu_VideoIO = NULL;
            }

            /* Same for audio (in case there was one) */
            if(mpegUnit->mu_AudioIO)
            {
                mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
                TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
                mpegUnit->mu_AudioIO = NULL;
            }
            mpegUnit->mu_IsVidOverflow = FALSE;
            mpegUnit->mu_IsAudOverflow = FALSE;

            if(mpegUnit->mu_DPState)
            {
                /* Flush out the other queues. */

                FlushCDReads(mpegUnit);
                FlushVideoStream(mpegUnit);
                FlushAudioStream(mpegUnit);

                /* Ugh. This sucks. */

                Disable();

                req = (struct IOMPEGReq *)mpegUnit->mu_CDReplyPort->mp_MsgList.lh_Head;
                while(req->iomr_Req.io_Message.mn_Node.ln_Succ)
                {
                    if(req->iomr_Req.io_Command == CD_READ)
                        req->iomr_Req.io_Error = CDERR_ABORTED;

                    req = (struct IOMPEGReq *)req->iomr_Req.io_Message.mn_Node.ln_Succ;
                }

                Enable();
            }
	}

        /* Now complete the MPEGCMD_SCAN request if it's there */
        if(mpegUnit->mu_ScanIO)
        {
            BOOL oneshot;

            oneshot = mpegUnit->mu_ScanIO->iomr_MPEGFlags & MPEGF_ONESHOT;

            if(oneshot)
            {
                Disable();
                mpegUnit->mu_PlayFlags |= MPEGPF_PAUSE;
                Enable();
            }

            mpegUnit->mu_ScanIO->iomr_Req.io_Error = 0;
            TermIO(mpegUnit->mu_ScanIO,mpegUnit->mu_Device);
            mpegUnit->mu_ScanIO = NULL;

            /* Check for another pending MPEGCMD_SCAN request and kick it off. */
            if(oneshot)
            {
		if(req = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_CMDQueue))
                    CMDSearch(req,mpegUnit);

                ReleaseSemaphore(&mpegUnit->mu_Lock);
                return;
            }
        }
	if((mpegUnit->mu_DPState) && (mpegUnit->mu_PlayFlags & MPEGPF_SCAN))
	{
	    mpegUnit->mu_CDOffset += mpegUnit->mu_ScanSpeed*mpegUnit->mu_SectorSize;
	}
        if(mpegUnit->mu_PlayFlags & MPEGPF_SCAN)
        {
	    if(!DoScan(mpegUnit))
	        status=FALSE;
        }
        else if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
        {
            if(!DoPause(mpegUnit))
                status=FALSE;
        }
        else if(mpegUnit->mu_DPState)
        {
            mpegUnit->mu_DPState = DPSTATE_HOLDING;
        }
        else if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
        {
            if(!DoSlow(mpegUnit))
                status=FALSE;
        }
        else
        {
            if(!DoPlay(mpegUnit))
                status=FALSE;
        }
    }
    if(!status)
    {
        mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
        mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
        Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
    }
    ReleaseSemaphore(&mpegUnit->mu_Lock);
}

/****** cd32mpeg.device/MPEGCMD_GETDEVINFO ***********************************
*
*   NAME
*       MPEGCMD_GETDEVINFO -- Determine the capabilities of an MPEG device
*
*   FUNCTION
*       MPEGCMD_GETDEVINFO is used to determine what types of operations a
*       MPEG device is capable of, as well as getting a short description of
*       that board.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to begin playback on
*       io_Command   - command number for CMD_GETDEVINFO
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       io_Data      - Pointer to a MPEGDevInfo structure to be filled in
*                      by the device.
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*   NOTES
*       Best results will be acheived if you queue up some data before
*       sending this command.  See cd32mpeg.device/CMD_WRITE for more details.
*
*****************************************************************************/
/*
** CMDGetDevInfo
**
** Get Device Information.
**
*/
VOID CMDGetDevInfo(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGDevInfo *mdi;

    if(mdi = (struct MPEGDevInfo *)iomr->iomr_Req.io_Data)
    {
    	mdi->mdi_BoardCapabilities = MPEGCF_PLAYRAWVIDEO | MPEGCF_PLAYRAWAUDIO | MPEGCF_PLAYSYSTEM |
    				     MPEGCF_WINDOWVIDEO  | MPEGCF_STEPPLAY | MPEGCF_SCANPLAY;

    	strcpy(mdi->mdi_BoardDesc,"CD32 MPEG Module");
    	iomr->iomr_Req.io_Actual = sizeof(struct MPEGDevInfo);
    }
    else
    {
    	iomr->iomr_Req.io_Actual = sizeof(struct MPEGDevInfo);
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_SETWINDOW ************************************
*
*   NAME
*       MPEGCMD_SETWINDOW -- Set portion of MPEG video to display
*
*   FUNCTION
*       MPEGCMD_SETWINDOW is used to set the portion of a MPEG video stream
*	that will be displayed.  The io_Data parameter must point to a
*	MPEGWindowParams structure that contains offset/size information.
*
*	The fields in the MPEGWindowParams structure are used as follows:
*
*	mwp_XOffset - X Offset into the decoded picture for display start
*	mwp_YOffset - Y Offset into the decoded picture for display start
*	mwp_Width - How wide the displayed portion should be
*	mwp_Height - How high the displayed portion should be
*
*	mwp_XOffset and mwp_Width are specified in Amiga Hi-Res pixels.
*	mwp_YOffset and mwp_Height are specified in non-interlaced scan
*	lines.
*
*	Note: The CL450 is limited to positioning the decode window on
*	lo-res pixel boundaries, so mwp_XOffset will be scaled down by
*	two.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to step
*       io_Command   - command number for MPEGCMD_SETWINDOW
*	io_Data	     - pointer to a MPEGWindowParams structure.
*	io_Length    - sizeof(struct MPEGWindowParams)
*       io_Flags     - flags, must be cleared if not used:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/
/*
** CMDSetWindow
**
** Set Output Window Location.
**
*/
VOID CMDSetWindow(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGWindowParams *mwp;

    if(mwp = (struct MPEGWindowParams *)iomr->iomr_Req.io_Data)
    {
	if(SetCL450Window(mwp->mwp_XOffset, mwp->mwp_YOffset,
		          mwp->mwp_Width, mwp->mwp_Height,
			  mpegUnit))
	    iomr->iomr_Req.io_Error = 0;
	else
	{
	    iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
	    iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
	}
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_SETBORDER ************************************
*
*   NAME
*       MPEGCMD_SETBORDER -- Set display position and border color
*
*   FUNCTION
*       MPEGCMD_SETBORDER is used to set the display position and border color
*	for the decoded video stream.
*
*	The io_Data field of the IO request must point to a MPEGBorderParams
*	structure.   That structure should be set up as follows:
*
*	mbp_LeftBorder  - Offset from left of display (Hi-Res pixels)
*	mbp_TopBorder	- Offset from top of display  (NI scanlines)
*	mbp_BorderRed	- 8-bit red value for the border color
*	mbp_BorderGreen - 8-bit green value for the border color
*	mbp_BorderBlue  - 8-bit blue value for the border color
*
*	Values of 0 for mbp_LeftBorder and/or mbp_TopBorder mean that the
*	video should be centered horizontally and/or vertically centered
*	on the display.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to step
*       io_Command   - command number for MPEGCMD_SETBORDER
*	io_Data	     - pointer to a MPEGBorderParams structure.
*	io_Length    - sizeof(struct MPEGWindowParams)
*       io_Flags     - flags, must be cleared if not used:
*                      IOF_QUICK - (CLEAR) reply I/O request
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************/
/*
** CMDSetBorder
**
** Set Border Parameters.
**
*/
VOID CMDSetBorder(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGBorderParams *mbp;

    if(mbp = (struct MPEGBorderParams *)iomr->iomr_Req.io_Data)
    {
    	if(SetCL450Border(mbp->mbp_BorderLeft,
    			  mbp->mbp_BorderTop,
    			  mbp->mbp_BorderRed,
    			  mbp->mbp_BorderGreen,
    			  mbp->mbp_BorderBlue,
    			  mpegUnit))
    	{
	    iomr->iomr_Req.io_Error = 0;
	}
	else
	{
	    iomr->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
	    iomr->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
	}
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_SETVIDEOPARAMS *******************************
*
*   NAME
*       MPEGCMD_SETVIDEOPARAMS -- Set parameters for playback/recording
*
*   FUNCTION
*       MPEGCMD_SETVIDEOPARAMS allows you to set such things as picture size,
*	display type (NTSC/PAL), etc.
*
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to set parameters for
*       io_Command   - command number for MPEGCMD_SETVIDEOPARAMS
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       io_Data      - Pointer to a MPEGVideoParams structure.
*	io_Length    - Sizeof(struct MPEGVideoParams)
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*   NOTES
*       Not all devices can support all features listed in the include files,
*       so please use the MPEGCMD_GETDEVINFO command to determine what the
*       device is actually capable of doing.
*
*	The mvp_DisplayType parameter will be ignored during normal playback.
*	If you need to explicitly set what type of screen the MPEG will be
*	displayed on, you must set it before playback begins.
*
*****************************************************************************/
/*
** CMDSetVideo
**
** Set Misc. Video Parameters.
**
*/
VOID CMDSetVideo(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGVideoParamsSet *mvp;

    if(mvp = (struct MPEGVideoParamsSet *)iomr->iomr_Req.io_Data)
    {
    	if(iomr->iomr_Req.io_Length >= sizeof(struct MPEGVideoParamsSet))
    	{
    	    mpegUnit->mu_VEnable = mvp->mvp_Fade >> 15;

	    if(!(mpegUnit->mu_PlayFlags & MPEGPF_PLAY))
	    {
                if((mvp->mvp_DisplayType == 3) || (mvp->mvp_DisplayType == 4))
                    SetCL450VideoMode(mvp->mvp_DisplayType,mpegUnit);
	    }
	    SetBlankMode(mpegUnit);
        }
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_GETVIDEOPARAMS *******************************
*
*   NAME
*       MPEGCMD_GETVIDEOPARAMS -- Get settings for video playback/recording
*
*   FUNCTION
*       MPEGCMD_GETVIDEOPARAMS may be used to determine what the current
*       settings are for playback and/or recording.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to get parameters from
*       io_Command   - command number for MPEGCMD_GETVIDEOPARAMS
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       io_Data      - Pointer to a MPEGVideoParams structure.
*	io_Length    - Sizeof(struct MPEGVideoParams)
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*****************************************************************************
/*
** CMDGetVideo
**
** Get Misc. Video Parameters.
**
*/
VOID CMDGetVideo(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGVideoParamsGet *mvp;

    iomr->iomr_Req.io_Error = MPEGERR_BAD_PARAMETER;

	DS(kprintf("[CMDGetVideo]"));

    if(mvp = (struct MPEGVideoParamsGet *)iomr->iomr_Req.io_Data)
    {
    	if(iomr->iomr_Req.io_Length >= sizeof(struct MPEGVideoParamsGet))
    	{
    	    if(mpegUnit->mu_PicCount)
    	    {
                ObtainCL450SeqSemaphore(mpegUnit);

                mvp->mvp_PictureWidth = ReadCL450Dram(HORIZONTAL_SIZE);
                mvp->mvp_PictureHeight = ReadCL450Dram(VERTICAL_SIZE);
                mvp->mvp_PictureRate = ReadCL450Dram(PICTURE_RATE);

                ReleaseCL450SeqSemaphore(mpegUnit);

                iomr->iomr_Req.io_Actual = sizeof(struct MPEGVideoParamsGet);
                iomr->iomr_Req.io_Error = 0;
           }
           else
           {
           	ObtainSemaphore(&mpegUnit->mu_Lock);
                AddTail((struct List *)&mpegUnit->mu_SeqHeadQueue,(struct Node *)iomr);
                ReleaseSemaphore(&mpegUnit->mu_Lock);
           }
        }
    }
    if(mpegUnit->mu_PicCount)
        TermIO(iomr,mpegUnit->mu_Device);
}

/****** cd32mpeg.device/MPEGCMD_SETAUDIOPARAMS *******************************
*
*   NAME
*       MPEGCMD_SETAUDIOPARAMS -- Set parameters for playback/recording
*
*   FUNCTION
*       MPEGCMD_SETPARAMS allows you to set such things as DAC attenuation,
*       sample rage, target bitrate for decoding, etc.
*
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to set parameters for
*       io_Command   - command number for MPEGCMD_SETAUDIOPARAMS
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       io_Data      - Pointer to a MPEGAudioParams structure.
*	io_Length    - Sizeof(struct MPEGAudioParams)
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*   NOTES
*       Not all devices can support all features listed in the include files,
*       so please use the CMD_GETDEVINFO command to determine what the board
*       is actually capable of doing.
*
*****************************************************************************/
/*
** CMDSetAudio
**
** Set Audio Parameters.
**
*/
VOID CMDSetAudio(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGAudioParams *map;
    BOOL mute;

    iomr->iomr_Req.io_Error = MPEGERR_BAD_PARAMETER;

    if(map = (struct MPEGAudioParams *)iomr->iomr_Req.io_Data)
    {
    	if(iomr->iomr_Req.io_Length >= sizeof(struct MPEGAudioParams))
    	{
    		kprintf("SetAudio:  Vol: %04lx %04lx  Stream: %04lx\n",
    			map->map_VolumeLeft,map->map_VolumeRight,map->map_StreamID);

            mute = (map->map_VolumeLeft >> 15 | map->map_VolumeRight >> 15) ? FALSE : TRUE;

            if(mute)
                mpegUnit->mu_PlayFlags |= MPEGPF_EXTMUTE;
            else
                mpegUnit->mu_PlayFlags &= ~MPEGPF_EXTMUTE;

	    mpegUnit->mu_AudioID = map->map_StreamID;
	    SetL64111AudioID(mpegUnit);

            switch(mpegUnit->mu_ProductID)
            {
                case MPEG_ZORRO2_ID : if(mpegUnit->mu_PlayFlags & (MPEGPF_EXTMUTE))
                                          mpegUnit->mu_ControlVal |= Z2_MUTE;
                                      else
                                          mpegUnit->mu_ControlVal &= ~Z2_MUTE;
                                      break;

                case MPEG_CDGS_ID:    if(mpegUnit->mu_PlayFlags & (MPEGPF_EXTMUTE))
                                          mpegUnit->mu_ControlVal |= CD32_MUTE;
                                      else
                                          mpegUnit->mu_ControlVal &= ~CD32_MUTE;
                                      break;

            }
            *mpegUnit->mu_Control = mpegUnit->mu_ControlVal;
            iomr->iomr_Req.io_Error = 0;
        }
    }
    TermIO(iomr,mpegUnit->mu_Device);
}

extern ULONG Y_START;
extern ULONG C_START;
extern ULONG FB_ADDR;

/****** cd32mpeg.device/MPEGCMD_READFRAMEYUV *********************************
*
*   NAME
*       MPEGCMD_READFRAMEYUV -- Read a single video frame in YUV format
*
*   FUNCTION
*	MPEGCMD_READFRAMEYUV may be used to read out the currently displayed
*	video frame.  The data is supplied in LCrCb format according to MPEG
*	specifications.
*
*	The io_Data field of the IO request should point to a
*	MPEGFrameStore structure filled in as follows:
*
*	mfs_Width    - The width of the video stream
*	mfs_Height   - The height of the video stream
*	mfs_Luma     - Pointer to an array of UBYTE's to store the Luminance
*		       information for the current frame.  Must be at least
*		       mfs_Width*mfs_Height bytes in size.
*	mfs_Cr	     - Pointer to an array of UBYTE's to store the Cr Chroma
*		       information for the current frame.  Must be at least
*		       mfs_Width*mfs_Height/4 bytes in size.
*	mfs_Cb	     - Pointer to an array of UBYTE's to store the Cb Chroma
*                      information for the current frame.  Must be at least
*                      mfs_Width*mfs_Height/4 bytes in size.
*
*	Note that the Cr and Cb components of an MPEG frame have half the
*	horizontal and vertical resolution of the Luminance data.
*
*   INPUTS
*       mn_ReplyPort - pointer to message port that receives I/O request
*                      if the quick flag (IOF_QUICK) is clear
*       io_Device    - pointer to device node, must be set by (or copied from
*                      I/O block set by) OpenDevice function
*       io_Unit      - unit number to set parameters for
*       io_Command   - command number for MPEGCMD_SETAUDIOPARAMS
*       io_Flags     - flags, must be cleared if not use:
*                      IOF_QUICK - (CLEAR) reply I/O request
*       io_Data      - Pointer to a MPEGFrameStore structure.
*	io_Length    - Sizeof(struct MPEGFrameStore)
*
*   OUTPUTS
*       io_Error     - If non-zero, then an error of some kind occured.
*                      See <exec/errors.h> and <devices/mpeg.h> for more
*                      details.
*
*   NOTES
*       Not all devices can support all features listed in the include files,
*       so please use the CMD_GETDEVINFO command to determine what the board
*       is actually capable of doing.
*
*****************************************************************************/
/*
** CMDReadFrameYUV
**
** Read digital frame data in YUV format
**
*/
VOID CMDReadFrame(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    struct MPEGFrameStore *mfs;
    ULONG fbaddr, Cr_src, Cb_src;

    if(!(mpegUnit->mu_PlayFlags & MPEGPF_PLAY && mpegUnit->mu_PlayFlags & MPEGPF_PAUSE))
    {
        iomr->iomr_Req.io_Error = MPEGERR_BAD_STATE;
        TermIO(iomr,mpegUnit->mu_Device);
        return;
    }
    if(iomr->iomr_Req.io_Data)
    {
    	if(iomr->iomr_Req.io_Length = sizeof(struct MPEGFrameStore))
    	{
    	    mfs = (struct MPEGFrameStore *)iomr->iomr_Req.io_Data;

    	    /* Find Current Picture Number */

    	    fbaddr = ReadCL450Dram(FB_ADDR);

	    Cr_src = C_START + fbaddr - Y_START;
	    Cb_src = Cr_src+8;

	    /* No attempt at speed here at all.  Just make it work first. */
	    /* Just do Luma for right now */

	    if(mfs->mfs_Luma)
	        ReadLumaBuffer((UBYTE *)((fbaddr<<1)+(ULONG)mpegUnit->mu_CL450Dram),
	    		    mfs->mfs_Luma,mfs->mfs_Width,mfs->mfs_Height);

	    if(mfs->mfs_Cb)
	        ReadChromaBuffer((UBYTE *)((Cb_src<<1)+(ULONG)mpegUnit->mu_CL450Dram),
	        	    mfs->mfs_Cb,mfs->mfs_Width>>1,mfs->mfs_Height>>1);

	    if(mfs->mfs_Cr)
	        ReadChromaBuffer((UBYTE *)((Cr_src<<1)+(ULONG)mpegUnit->mu_CL450Dram),
	        	    mfs->mfs_Cr,mfs->mfs_Width>>1,mfs->mfs_Height>>1);

	    iomr->iomr_Req.io_Error = 0;
    	}
    	else
    	    iomr->iomr_Req.io_Error = MPEGERR_BAD_PARAMETER;
    }
    else
        iomr->iomr_Req.io_Error = MPEGERR_BAD_PARAMETER;

    TermIO(iomr,mpegUnit->mu_Device);
}

/*
** MPEG Device Function Dispatch Table
**
** Just when you thought C couldn't get any more cryptic...
*/

VOID (* mpegCmdDispatch[MPEGCMD_END])(struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)=
{
	&CMDInvalid,	/* CMD_INVALID */
	&CMDReset,	/* CMD_RESET */
	&CMDInvalid,	/* CMD_READ */
	&CMDWrite,	/* CMD_WRITE */
	&CMDInvalid,	/* CMD_UPDATE */
	&CMDInvalid,	/* CMD_CLEAR */
	&CMDInvalid,	/* CMD_STOP */
	&CMDInvalid,	/* CMD_START */
	&CMDFlush,	/* CMD_FLUSH */

	/* Extra MPEG Commands */

	&CMDPlay,	/* MPEGCMD_PLAY */
	&CMDPause,	/* MPEGCMD_PAUSE */
	&CMDSlow,	/* MPEGCMD_SLOWMOTION */
	&CMDStep,	/* MPEGCMD_SINGLESTEP */
	&CMDSearch,	/* MPEGCMD_SEARCH */
	&CMDInvalid,	/* MPEGCMD_RECORD */
	&CMDGetDevInfo,	/* MPEGCMD_GETDEVINFO */
	&CMDSetWindow,	/* MPEGCMD_SETWINDOW */
	&CMDSetBorder,	/* MPEGCMD_SETBORDER */
	&CMDGetVideo,	/* MPEGCMD_GETVIDEOPARAMS */
	&CMDSetVideo,	/* MPEGCMD_SETVIDEOPARAMS */
	&CMDSetAudio,	/* MPEGCMD_SETAUDIOPARAMS */

	&CMDPlayLSN,	/* MPEGCMD_PLAYLSN	*/
	&CMDSeekLSN,	/* MPEGCMD_SEEKLSN	*/
	&CMDReadFrame	/* MPEGCMD_READFRAMEYUV	*/
};


/*
** GetNextVideoIO
**
** Fetch the next IOMEGReq from the video stream if it's there,
** and handle odd length packets, time stamps, etc.
**
** a4 = Unit Pointer
** a6 = Device Base
*/
BOOL ASM GetNextVideoIO(REG(a4) struct MPEGUnit *mpegUnit)
{
    struct IOMPEGReq *iomr;
    ULONG packetsize;
    UWORD kludge;
    UBYTE *hack;
#if 0
    UWORD hi,mid,lo;
    ULONG scr,now;
#endif
    ObtainSemaphore(&mpegUnit->mu_Lock);

    if(iomr = mpegUnit->mu_VideoIO = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_VideoStream))
    {
#if 0
	if(!((*(volatile UBYTE *)0xbfe001) & 0x40))
	{
	    TermIO(iomr,mpegUnit->mu_Device);
	    mpegUnit->mu_VideoIO = NULL;
	    return(NULL);
	}
#endif
#if 0
//        GetSCR(&hi,&mid,&lo,mpegUnit);
//        now=(hi<<30)|(mid<<15)|lo;
        hi =iomr->iomr_SCRHigh;
        mid=iomr->iomr_SCRMid;
        lo =iomr->iomr_SCRLow;
        scr=(hi<<30)|(mid<<15)|lo;

	mpegUnit->mu_LastOffset = scr;
#endif
    	mpegUnit->mu_VPackets--;

//	mpegUnit->mu_LastOffset = iomr->iomr_Req.io_Offset;

        packetsize = iomr->iomr_DataSize;
        mpegUnit->mu_VideoData = (UWORD *)iomr->iomr_DataStart;

        /* Here's what's going on.  We can only send words to the CL450, so what we
           do if there is an odd length packet is increase the newpacket size parameter
           by one, and then steal the first byte of the next packet. */

        /* First, see if we need to send off the last byte of the last packet plus the
           first byte of the current packet. */

        if(mpegUnit->mu_IsVidOverflow)
        {
            kludge = mpegUnit->mu_VidOverflowByte<<8;
            kludge |= *(UBYTE *)mpegUnit->mu_VideoData;

	    {
	        ULONG counter = 100000;

	        while(
			(!(*mpegUnit->mu_Control & mpegUnit->mu_CFLevelBit))
		     && counter)
	        {
                    mpegUnit->mu_ControlVal = CD32_CL450_RESET | CD32_MAP_UMPEG;
                    SetBlankMode(mpegUnit);

                    for(;;)
                    {
                        *(volatile UWORD *)0xdff180 = 0xf00;
                        *(volatile UWORD *)0xdff181 = 0xf00;
                        *(volatile UWORD *)0xdff182 = 0xf00;
                        *(volatile UWORD *)0xdff183 = 0xf00;
		    }
//	            kprintf("!");
	            counter--;
	        }

	        // If counter didn't count all the way down, then the FIFO is okay
	        // to write to.

	        if(counter)
	        {
                    *mpegUnit->mu_CL450Fifo = kludge;
                }
		else
		{
		    AddHead((struct List *)&mpegUnit->mu_VideoStream,(struct Node *)iomr);
		    return(NULL);
		}
	    }
            packetsize--;
            mpegUnit->mu_VideoData = (UWORD *)((ULONG)mpegUnit->mu_VideoData + 1);
            mpegUnit->mu_IsVidOverflow = FALSE;
            iomr->iomr_MPEGFlags &= ~MPEGF_VALID_PTS;
        }
        mpegUnit->mu_VideoSize = packetsize;

        /* Here we deal with an odd length packet.  Strip off the last byte and store
           it in mpegUnit->mu_VidOverflowByte for use later.  Then decrease mpegUnit->mu_VideoSize
           by one, and increase packetsize by one.  Also, set the mpegUnit->mu_IsOverflow
           flag to a 1. */

        if(packetsize & 1)
        {
            hack = (UBYTE *)mpegUnit->mu_VideoData;
            mpegUnit->mu_VideoSize--;

            /* In the chance that this byte is the beginning of a start code associated with
               the time stamp for this data, kill the MPEGF_VALID_PTS flag for this IOMPEGReq
               so that the CL450 doesn't get confused. */

            mpegUnit->mu_VidOverflowByte = hack[mpegUnit->mu_VideoSize];

            mpegUnit->mu_IsVidOverflow = TRUE;
            packetsize++;
            iomr->iomr_MPEGFlags &= ~MPEGF_VALID_PTS;
        }
#if 0
	if(!((*(volatile UBYTE *)0xbfe001) & 0x40))
	{
//		kprintf("%4ld",256);
//	    NewPacket(256, iomr, mpegUnit);
	    mpegUnit->mu_VideoSize = 0;
//	    iomr->iomr_PTSMid = 0;
//	    packetsize -= 8;
	}
//	kprintf("%4ld ",packetsize);
#endif
        if(NewPacket(packetsize, iomr, mpegUnit))
        {

            mpegUnit->mu_VideoSize = mpegUnit->mu_VideoSize >> 1;
            ReleaseSemaphore(&mpegUnit->mu_Lock);
            return(TRUE);
        }
        else
        {
            /* oops, something barfed */
            mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
            mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
            Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
        }

    }
    ReleaseSemaphore(&mpegUnit->mu_Lock);
    return(FALSE);
}

/*
** GetNextAudioIO
**
** Fetch the next IOMEGReq from the audio stream if it's there,
** and handle odd length packets, etc.
**
** a4 = Unit Pointer
** a6 = Device Base
*/
BOOL ASM GetNextAudioIO(REG(a4) struct MPEGUnit *mpegUnit)
{
    struct IOMPEGReq *iomr;
    ULONG packetsize;
    UWORD kludge;
    UBYTE *hack;
#if 0
    UWORD hi,mid,lo;
    ULONG scr,now;
#endif
    ObtainSemaphore(&mpegUnit->mu_Lock);

fixme:

    if(iomr = mpegUnit->mu_AudioIO = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_AudioStream))
    {
	mpegUnit->mu_APackets--;
#if 0
//        GetSCR(&hi,&mid,&lo,mpegUnit);
//        now=(hi<<30)|(mid<<15)|lo;
        hi =iomr->iomr_SCRHigh;
        mid=iomr->iomr_SCRMid;
        lo =iomr->iomr_SCRLow;
        scr=(hi<<30)|(mid<<15)|lo;

        if((mpegUnit->mu_StreamType & MPEGSTREAM_VIDEO) && !(mpegUnit->mu_PlayFlags & MPEGPF_SYNC) && (scr < mpegUnit->mu_LastOffset))
        {
            TermIO(iomr,mpegUnit->mu_Device);
            iomr = mpegUnit->mu_AudioIO = NULL;
            goto fixme;
        }
#endif

    	packetsize = iomr->iomr_DataSize;
    	mpegUnit->mu_AudioData = (UWORD *)iomr->iomr_DataStart;

        /* Here's what's going on.  We can only send words to the L64111, so what we
           do if there is an odd length packet is increase the newpacket size parameter
           by one, and then steal the first byte of the next packet. */

        /* First, see if we need to send off the last byte of the last packet plus the
           first byte of the current packet. */

	if(mpegUnit->mu_IsAudOverflow)
	{
	    kludge = mpegUnit->mu_AudOverflowByte<<8;
	    kludge |= *(UBYTE *)mpegUnit->mu_AudioData;
	    *(UWORD *)mpegUnit->mu_L64111 = kludge;
	    packetsize--;
	    mpegUnit->mu_AudioData = (UWORD *)((ULONG)mpegUnit->mu_AudioData + 1);
	    mpegUnit->mu_IsAudOverflow = FALSE;
	}
	mpegUnit->mu_AudioSize = packetsize;

        /* Here we deal with an odd length packet.  Strip off the last byte and store
           it in mpegUnit->mu_AudOverflowByte for use later.  Then decrease mpegUnit->mu_AudioSize
           by one, and increase packetsize by one.  Also, set the mpegUnit->mu_IsAudOverflow
           flag to a 1. */

	if(packetsize & 1)
	{
	    hack = (UBYTE *)mpegUnit->mu_AudioData;
	    mpegUnit->mu_AudioSize--;
	    mpegUnit->mu_AudOverflowByte = hack[mpegUnit->mu_AudioSize];
	    mpegUnit->mu_IsAudOverflow = TRUE;
	}
	mpegUnit->mu_AudioSize = mpegUnit->mu_AudioSize >> 1;
        ReleaseSemaphore(&mpegUnit->mu_Lock);

	return(TRUE);
    }
    else
    {
        ReleaseSemaphore(&mpegUnit->mu_Lock);
    	return(FALSE);
    }
}

#define SF_FEEDAUDIO	1
#define SF_AUDIODAT	2
#define SF_DOAUDIO	3

#define SF_FEEDVIDEO	4
#define SF_VIDEODAT	8
#define SF_DOVIDEO	12

ULONG ProcessCL450Interrupts(ULONG StatFlags, struct MPEGUnit *mpegUnit)
{
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;
    BOOL clrInts = FALSE;
    UWORD cl450Ints;

    StartCL450Cmd();     /* Make sure nobody plays with the chip */

    /* Read Interrupt Status */

    WrCL450Register(HOST_RADDR1,READ_INT_STATUS);
    cl450Ints = RdCL450Register(HOST_RDATA1);

    EndCL450Cmd();

    /* Okay, it's hungry, so feed it. */

#if 0
    if(cl450Ints & CL450INTF_RDY)
    {
        StatFlags |= SF_FEEDVIDEO;
    }
#endif

    if(cl450Ints & (CL450INTF_BUF|CL450INTF_DER|CL450INTF_RDY))
    {
        StatFlags |= SF_FEEDVIDEO;
    }
#if 0
    if(cl450Ints & CL450INTF_DER)
    {
        kprintf("CL450 Data Error\n");
    }
    if(cl450Ints & CL450INTF_BUF)
    {
    	kprintf("CL450 Underflow %ld\n",mpegUnit->mu_VPackets);
    }
#endif
    /* A Scan has been completed, so deal with it. */

    if(cl450Ints & CL450INTF_SCN)
    {
#if 0
        UWORD cmds,ints;
        StartCL450Cmd();
        WrCL450Register(HOST_RADDR1,0xd);
        cmds = RdCL450Register(HOST_RDATA1);
        WrCL450Register(HOST_RADDR1,0xf);
        ints = RdCL450Register(HOST_RDATA1);
        EndCL450Cmd();
        kprintf("Cmds: %ld, Ints: %ld\n",cmds,ints);
#endif
        ScanComplete(mpegUnit);
        StatFlags &= ~SF_FEEDVIDEO;
        clrInts=TRUE;
        mpegUnit->mu_PicCount++;
    }
#if 0
    if(cl450Ints & ~(CL450INTF_RDY|CL450INTF_SCN|CL450INTF_PIC))
    {
        kprintf("%04lx\n",cl450Ints);
    }
#endif
    /*
    ** If we get a picture decode interrupt, it means we were blanked
    ** while looking for the sequence header.
    */
    if(cl450Ints & CL450INTF_PIC)
    {
    	mpegUnit->mu_PicCount++;

	ObtainCL450SeqSemaphore(mpegUnit);
	{
	    UWORD sec_control;
	    sec_control = ReadCL450Dram(SEQ_CONTROL);
	    sec_control |= 0x04;
	    WriteCL450Dram(SEQ_CONTROL,sec_control);
	}
	ReleaseCL450SeqSemaphore(mpegUnit);

	if(mpegUnit->mu_DPState == DPSTATE_FINDSEQHEADER)
	{
            /* Now skip to the "real" start. */

            FlushCDReads(mpegUnit);
            FlushVideoStream(mpegUnit);
            FlushAudioStream(mpegUnit);
            if(!FlushCL450(mpegUnit))
            {
                mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
                return(0);
	    }
            mpegUnit->mu_IsVidOverflow = FALSE;
	    mpegUnit->mu_IsAudOverflow = FALSE;

            /* Flush the currently processing video packet */
            if(mpegUnit->mu_VideoIO)
            {
                mpegUnit->mu_VideoIO->iomr_Req.io_Error = 0;
                TermIO(mpegUnit->mu_VideoIO,mpegUnit->mu_Device);
                mpegUnit->mu_VideoIO = NULL;
            }

            /* Same for audio (in case there was one) */
            if(mpegUnit->mu_AudioIO)
            {
                mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
                TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
                mpegUnit->mu_AudioIO = NULL;
            }

            mpegUnit->mu_CDOffset = mpegUnit->mu_StreamOffset;

            mpegUnit->mu_DPState = DPSTATE_HOLDING;

            /* Now do whatever it was we were supposed to be doing */

            if(mpegUnit->mu_PlayFlags & MPEGPF_SCAN)
            {
                if(!DoScan(mpegUnit))
                {
                    mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                    mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                    Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
                    return(0);
		}
                mpegUnit->mu_DPState = DPSTATE_RUNNING;
            }
            else if(mpegUnit->mu_PlayFlags & MPEGPF_PAUSE)
            {
            	if(!DoPause(mpegUnit))
            	{
                    mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                    mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                    Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
                    return(0);
		}
                mpegUnit->mu_DPState = DPSTATE_RUNNING;
            }

            /* If we're in our "holding pattern", Slow/Normal playback */
            /* Will happen later on. */
        }

	/* We may have some pending MPEGCMD_GETVIDEOPARAMS or other commands that
	   are waiting for the sequence header information to become valid.
	   Process them now. */
        {
            struct IOMPEGReq *iomr;
            ObtainSemaphore(&mpegUnit->mu_Lock);

	    while(iomr = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_SeqHeadQueue))
	    	PerformIO(iomr);

            ReleaseSemaphore(&mpegUnit->mu_Lock);
	}
        clrInts = TRUE;
    }
    if(clrInts)
    {
        StartCL450Cmd();
        WrCL450Register(HOST_RADDR1, READ_INT_STATUS);
        WrCL450Register(HOST_RDATA1, 0);
        EndCL450Cmd();
    }
    return(StatFlags);
}

#undef SysBase
#define SysBase (*(struct Library **)4L)

/*
** This	is the C entry point for the Unit process.
** a6 has been set up by the assembly stub in mpeg_dev.asm.
*/

VOID DevTaskCEntry(VOID)
{
    struct MPEGUnit *mpegUnit;
    struct MPEGDevice *mpegDevice;
    struct Task *master;

    ULONG signals;
    UWORD StatFlags=0;

    /* Wait for tc_UserData to be correctly set up. */

    Wait(SIGF_SINGLE);

    {
        struct Task *task;
        task = FindTask(0L);
        mpegUnit = (struct MPEGUnit *)task->tc_UserData;
        mpegDevice = (struct MPEGDevice *)mpegUnit->mu_Device;
        master = mpegUnit->mu_Task;

        mpegUnit->mu_Task = task;

#undef SysBase
#define SysBase mpegUnit->mu_SysBase

        /* Attempt to allocate a signal bit for our Unit MsgPort. */

        mpegUnit->mu_Unit.unit_MsgPort.mp_SigBit = AllocSignal(-1L);
        mpegUnit->mu_Unit.unit_MsgPort.mp_SigTask = mpegUnit->mu_Task;
        mpegUnit->mu_Unit.unit_MsgPort.mp_Flags = PA_SIGNAL;

        mpegUnit->mu_CL450SigBit = AllocSignal(-1L);
        mpegUnit->mu_L64111SigBit = AllocSignal(-1L);
        mpegUnit->mu_SyncSigBit = AllocSignal(-1L);
        mpegUnit->mu_AbortSigBit = AllocSignal(-1L);

        mpegUnit->mu_CDReplyPort = CreateMsgPort();
        mpegUnit->mu_DPReplyPort = CreateMsgPort();

        /* I only check the message port creations.  If the signal allocations don't work,
           something is _really_ hosed in the machine. */

	if(!(mpegUnit->mu_CDReplyPort && mpegUnit->mu_DPReplyPort))
	{
	    if(mpegUnit->mu_CDReplyPort)
	    {
	        DeleteMsgPort(mpegUnit->mu_CDReplyPort);
	        mpegUnit->mu_Task = NULL;
	    }
	}
    }

    /* Okay, if our task pointer is still set, then we must be ready to go. */

    if(mpegUnit->mu_Task)
    {
        Signal(master,SIGF_SINGLE);	/* Signal the opener that we're alive */

	mpegUnit->mu_WaitMask      = 0;
	mpegUnit->mu_PortSigMask   = 1L << mpegUnit->mu_Unit.unit_MsgPort.mp_SigBit;
	mpegUnit->mu_SyncSigMask   = 1L << mpegUnit->mu_SyncSigBit;
	mpegUnit->mu_CL450SigMask  = 1L << mpegUnit->mu_CL450SigBit;
	mpegUnit->mu_L64111SigMask = 1L << mpegUnit->mu_L64111SigBit;
	mpegUnit->mu_CDSigMask	   = 1L << mpegUnit->mu_CDReplyPort->mp_SigBit;
	mpegUnit->mu_DPSigMask	   = 1L << mpegUnit->mu_DPReplyPort->mp_SigBit;
	mpegUnit->mu_AbortSigMask  = 1L << mpegUnit->mu_AbortSigBit;
	mpegUnit->mu_WaitMask	   = mpegUnit->mu_PortSigMask  | mpegUnit->mu_SyncSigMask   |
				     mpegUnit->mu_CL450SigMask | mpegUnit->mu_L64111SigMask |
				     mpegUnit->mu_CDSigMask    | mpegUnit->mu_DPSigMask     |
				     mpegUnit->mu_AbortSigMask | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F;

	/* Loop...*/

	while(TRUE)
	{
	    if(((StatFlags & SF_DOAUDIO) == SF_DOAUDIO) ||
	       ((StatFlags & SF_DOVIDEO) == SF_DOVIDEO))
	        signals = SetSignal(0L,mpegUnit->mu_WaitMask);
	    else
	    {
	        signals = Wait(mpegUnit->mu_WaitMask);
	    }
	    /* Have we been signaled to	shut down? */

	    if(signals & SIGBREAKF_CTRL_F)
		break;

	    /* Check to see if we have any internal IO requests to send back to cd.device */

	    if(signals & mpegUnit->mu_DPSigMask)
	    {
	    	struct IOMPEGReq *iomr;

	    	while(iomr = (struct IOMPEGReq *)GetMsg(mpegUnit->mu_DPReplyPort))
	    	{
	    	    /* Don't read past the last sector of the track */

	    	    if((mpegUnit->mu_CDOffset >= mpegUnit->mu_StreamBegin) &&
		       (mpegUnit->mu_CDOffset <= mpegUnit->mu_StreamEnd))
		    {
			iomr->iomr_Req.io_Unit = mpegUnit->mu_CDUnit;
			iomr->iomr_Req.io_Device = mpegUnit->mu_CDDevice;
	    	    	iomr->iomr_Req.io_Command = CD_READ;
                        iomr->iomr_Req.io_Message.mn_ReplyPort = mpegUnit->mu_CDReplyPort;
                        iomr->iomr_Req.io_Offset = mpegUnit->mu_CDOffset;
                        mpegUnit->mu_CDOffset += mpegUnit->mu_SectorSize;
                        SendCDIO(mpegUnit, iomr);
                    }
                    else
                    {
                        mpegUnit->mu_DPState = DPSTATE_STOPPING;
                        mpegUnit->mu_DPReqs--;
                	FreeVec(iomr->iomr_Req.io_Data);
                    	DeleteCDIORequest(mpegUnit, iomr);
                    }
                }
            }

	    /* See if we have any IO requests that have come back from cd.device. */

            /* If we get a real read error that wasn't caused by us aborting the
               IO request, then abort the current play request and shut down. */

	    if(signals & mpegUnit->mu_CDSigMask)
	    {
	        struct IOMPEGReq *iomr;

	    	while(iomr = (struct IOMPEGReq *)GetMsg(mpegUnit->mu_CDReplyPort))
	    	{
	    	    /* A read request has completed, so remove it from our list of
	    	       them that are pending with cd.device */

	    	    if(iomr->iomr_Req.io_Command == CD_READ)
                        RemoveCDRead(mpegUnit, iomr);

		    /* Note that we modify our signals variable and StatFlags so that
		       we abort on this loop, instead of coming around again.  This avoids
		       unnecessary time spent feeding either the vide or audio decoder. */

                    if(iomr->iomr_Req.io_Error && (iomr->iomr_Req.io_Error != CDERR_ABORTED))
                    {
                        signals |= mpegUnit->mu_AbortSigMask;
                        signals &= ~(mpegUnit->mu_CL450SigMask|mpegUnit->mu_L64111SigMask);
                        StatFlags &= ~(SF_FEEDAUDIO|SF_FEEDVIDEO);
		    }
	    	    if(!iomr->iomr_Req.io_Error && iomr->iomr_Req.io_Command == CD_READ
	    	       && mpegUnit->mu_DPState)
	    	    {
			iomr->iomr_Req.io_Command = CMD_WRITE;
			iomr->iomr_Req.io_Unit = (struct Unit *)mpegUnit;
			iomr->iomr_Req.io_Device = (struct Device *)mpegDevice;
	    	    	iomr->iomr_Req.io_Message.mn_ReplyPort = mpegUnit->mu_DPReplyPort;
    	    	    	iomr->iomr_Req.io_Length = mpegUnit->mu_SectorSize;
    	    	    	iomr->iomr_Req.io_Offset = 0;
	    	    	iomr->iomr_StreamType = MPEGSTREAM_SYSTEM;
	    	    	iomr->iomr_MPEGFlags = 0;
	    	    	iomr->iomr_Req.io_Flags = 0;
	    	        CMDWrite(iomr,mpegUnit);
	    	    }
	    	    else
	    	    {
	    	    	/* Note, if this happens and the loop is allowed to run again,
	    	    	   an IOTorture hit will occur because the message type will
	    	    	   still be set to NT_MESSAGE when we later pull this message
	    	    	   off of the DPReplyPort via GetMsg().  */

	    	    	PutMsg(mpegUnit->mu_DPReplyPort,(struct Message *)iomr);
	    	    }
	    	}
	    }

	    /*
	    ** Process Incoming I/O requests
	    */
	    if(signals & mpegUnit->mu_PortSigMask)
	    {
	        struct IOMPEGReq *iomr;

	    	while(iomr = (struct IOMPEGReq *)GetMsg((struct MsgPort *)mpegUnit))
	    	{
	    	    PerformIO(iomr);
		}
	    }

	    /*
	    ** Update our flags
	    */

            if(mpegUnit->mu_VPackets)
                StatFlags |= SF_VIDEODAT;

            if(mpegUnit->mu_APackets)
                StatFlags |= SF_AUDIODAT;

	    /*
	    ** This is part of the startup/error recovery stuff.  If we detect an
	    ** underflow condition and are in a normal playback mode, we Pause and
	    ** go into holding mode and wait to catch up.  Once we've done that,
	    ** we resume normal playback.
	    */
#if 0
	    if(    (mpegUnit->mu_DPState == DPSTATE_RUNNING)
		&& ((mpegUnit->mu_VPackets + mpegUnit->mu_APackets) < 10)
		&& !(mpegUnit->mu_PlayFlags & (MPEGPF_SCAN|MPEGPF_SLOW|MPEGPF_PAUSE)) )

	    {
	    	mpegUnit->mu_DPState = DPSTATE_HOLDING;
		DoPause(mpegUnit);
	    }
#endif
	    if(mpegUnit->mu_DPState == DPSTATE_HOLDING)
	    {
		if((mpegUnit->mu_VPackets+mpegUnit->mu_APackets) > 30)
		{
                    mpegUnit->mu_DPState = DPSTATE_RUNNING;

                    if(mpegUnit->mu_PlayFlags & MPEGPF_SLOW)
                    {
                        DoSlow(mpegUnit);
                        mpegUnit->mu_ValidPTS = FALSE;
                    }
                    else
                    {
                        DoPlay(mpegUnit);
                    }
                }
	    }

	    if((signals & mpegUnit->mu_SyncSigMask) && (mpegUnit->mu_PlayFlags & MPEGPF_WAITSYNC))
	    {
	        mpegUnit->mu_PlayFlags &= ~MPEGPF_WAITSYNC;
                if(!PlayCL450(mpegUnit))
                {
                    mpegUnit->mu_CurrentPlayCmd->iomr_Req.io_Error = MPEGERR_CMD_FAILED;
                    mpegUnit->mu_CurrentPlayCmd->iomr_MPEGError = MPEGEXTERR_MICROCODE_FAILURE;
                    signals |= mpegUnit->mu_AbortSigMask;
                }
            }

	    /*
	    ** Process CL450 Interrupts and update our status flags.
	    */
            if(signals & mpegUnit->mu_CL450SigMask)
		StatFlags = ProcessCL450Interrupts(StatFlags,mpegUnit);

	    /*
	    ** Luckily, the audio part is pretty simple to deal with.
	    */
	    if(signals & mpegUnit->mu_L64111SigMask)
	    {
	    	StatFlags |= SF_FEEDAUDIO;
            }

	    /*
	    ** Okay, if we're in a mode where we're consuming data but
	    ** aren't using up any audio data, toss audio data out.
	    **
	    ** KCD: This should probably be done somewhere else, probably
	    **	    where the data is parsed.  The system stream demuxer
	    **	    could do this, for instance.
	    */

	    if((mpegUnit->mu_PlayFlags & (MPEGPF_SCAN|MPEGPF_SLOW|MPEGPF_INTMUTE)) ||
		(mpegUnit->mu_DPState == DPSTATE_FINDSEQHEADER))
	    {
            	FlushAudioStream(mpegUnit);

                /* Same for audio (in case there was one) */
                if(mpegUnit->mu_AudioIO)
                {
                    mpegUnit->mu_AudioIO->iomr_Req.io_Error = 0;
                    TermIO(mpegUnit->mu_AudioIO,mpegUnit->mu_Device);
                    mpegUnit->mu_AudioIO = NULL;
                }
                StatFlags &= ~SF_AUDIODAT;
            }

	    /*
	    ** Feed Video and Audio decoders if they want data
	    */

	    if((StatFlags & SF_DOAUDIO) == SF_DOAUDIO)
	    	StatFlags = FeedL64111(StatFlags,mpegUnit,mpegDevice);

            if((StatFlags & SF_DOVIDEO) == SF_DOVIDEO)
	        StatFlags = FeedCL450(StatFlags,mpegUnit,mpegDevice);

#if 0
	    if(signals & SIGBREAKF_CTRL_E)
	    {
	        struct IOMPEGReq *iomr;

		CMDStop(NULL,mpegUnit);

		ObtainSemaphore(&mpegUnit->mu_Lock);

		TermIO(mpegUnit->mu_CurrentPlayCmd,mpegDevice);
		mpegUnit->mu_CurrentPlayCmd = NULL;

		kprintf("CL450 CMEM Failure!....doing hardware reset.\n");
		InitCL450(mpegUnit);

		if(iomr = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_PlayQueue))
		    PerformIO(iomr);

		ReleaseSemaphore(&mpegUnit->mu_Lock);
	    }
#endif
	    /* Well, if we're out of data or we are asked to terminate the */
	    /* currently executing play command then terminate the current */
	    /* Play request */

	    if((!(StatFlags & (SF_AUDIODAT|SF_VIDEODAT)) &&
		(mpegUnit->mu_DPState == DPSTATE_STOPPING)) || (signals & mpegUnit->mu_AbortSigMask))
	    {
	        struct IOMPEGReq *iomr;

		CMDStop(NULL,mpegUnit);

		ObtainSemaphore(&mpegUnit->mu_Lock);

		if(mpegUnit->mu_CurrentPlayCmd)
		{
                    TermIO(mpegUnit->mu_CurrentPlayCmd,mpegDevice);
                    mpegUnit->mu_CurrentPlayCmd = NULL;
		}
		if(iomr = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_PlayQueue))
		    PerformIO(iomr);

		ReleaseSemaphore(&mpegUnit->mu_Lock);
	    }
	}

	/* We're shutting down. */

	CMDReset(NULL,mpegUnit);
        DeleteMsgPort(mpegUnit->mu_CDReplyPort);
        DeleteMsgPort(mpegUnit->mu_DPReplyPort);

        Forbid();
        Signal((struct Task *)mpegUnit->mu_Task,SIGF_SINGLE);
    }
    /* We Forbid() so the "master" won't wake up before we can exit. */
    else
    {
        Forbid();
        Signal(master,SIGF_SINGLE);
    }
    RemTask(0L);
}

/* List	init routine. */
VOID NewList(struct List *list)
{
    list->lh_Head = (struct Node *)&list->lh_Tail;
    list->lh_Tail = NULL;
    list->lh_TailPred =	(struct	Node *)list;
}

