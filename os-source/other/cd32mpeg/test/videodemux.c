
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "/mpeg_device.h"
#include "asyncio.h"

/*****************************************************************************/

extern void __stdargs kputstr(STRPTR str);
extern void __stdargs kprintf(STRPTR fmt, ...);

#undef SysBase
#undef DOSBase

struct MPEGUnit mpegUnit;
struct MPEGDevice mpegDevice;

extern ASM SystemDemux(REG(a2) struct IOMPEGReq *iomr,
		       REG(a4) struct MPEGUnit *mpegUnit,
		       REG(a6) struct MPEGDevice *mpegDevice);
BOOL ASM GetNextVideoIO(REG(a2) BPTR file,
			REG(a4) struct MPEGUnit *mpegUnit,
			REG(a6) struct MPEGDevice *mpegDevice);

struct Library *SysBase;
struct Library *DOSBase;


ULONG vidBytes,fileBytes;

#define CD_SECTOR_SIZE 2324

VOID IOStreamProc(VOID)
{
    struct AsyncFile *file,*afile;
    struct IOMPEGReq *iomr;
    struct MsgPort *replyPort;
    ULONG packNum = 0;

    SysBase = (*(struct Library **)4L);

    NewList((struct List *)&mpegUnit.mu_AudioStream);
    NewList((struct List *)&mpegUnit.mu_VideoStream);
    InitSemaphore(&mpegUnit.mu_Lock);

    mpegDevice.md_SysBase = SysBase;

    if(mpegDevice.md_DOSBase = DOSBase = OpenLibrary("dos.library",37L))
    {
        if(file = OpenAsync("max:mpegfiles/seal.mpg",MODE_READ,262144))
        {
            if(afile = OpenAsync("max:mpegfiles/seal.vbs",MODE_WRITE,262144))
            {
                if(replyPort = CreateMsgPort())
                {
                    if(iomr = CreateIORequest(replyPort,sizeof(struct IOMPEGReq )+CD_SECTOR_SIZE))
                    {
                    	iomr->iomr_Req.io_Data = (APTR)((ULONG)iomr + sizeof(struct IOMPEGReq));
                        while((iomr->iomr_Req.io_Length = ReadAsync(file,iomr->iomr_Req.io_Data,CD_SECTOR_SIZE))== CD_SECTOR_SIZE)
                        {
                            if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
                                break;

                            packNum++;

                            SystemDemux(iomr,&mpegUnit,&mpegDevice);

			    if(RemHead((struct List *)&mpegUnit.mu_VideoStream))
			        WriteAsync(afile,iomr->iomr_DataStart,iomr->iomr_DataSize);

//			    GetNextVideoIO(afile,&mpegUnit,&mpegDevice);

                            RemHead((struct List *)&mpegUnit.mu_AudioStream);

                            if(GetMsg(replyPort))
                            {
                                BPTR pk;
                                UBYTE buff[80];

                                Printf("Pack %4.4ld\tConfused!\n",packNum);
                                sprintf(buff,"Pack%ld",packNum);
                                if(pk = Open(buff,MODE_NEWFILE))
                                {
                                    Write(pk,iomr->iomr_Req.io_Data,CD_SECTOR_SIZE);
                                    Close(pk);
                                }
                            }
                        }
                        Printf("vidBytes: %ld   fileBytes: %ld\n",vidBytes,fileBytes);
                        DeleteIORequest(iomr);
                    }
                    DeleteMsgPort(replyPort);
                }
                CloseAsync(afile);
	    }
            CloseAsync(file);
        }
	CloseLibrary(DOSBase);
    }
}

/*
** GetNextVideoIO
**
** Fetch the next IOMEGReq from the video stream if it's there,
** and handle odd length packets, time stamps, etc.
**
** a4 = Unit Pointer
** a6 = Device Base
*/
BOOL ASM GetNextVideoIO(REG(a2) BPTR file,
			REG(a4) struct MPEGUnit *mpegUnit,
			REG(a6) struct MPEGDevice *mpegDevice)
{
    struct IOMPEGReq *iomr;
    ULONG packetsize;
    UWORD kludge;
    UBYTE *hack;

    if(iomr = mpegUnit->mu_VideoIO = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_VideoStream))
    {
//    	D(kprintf("GNV: Good\n"));
    	packetsize = iomr->iomr_DataSize;
    	mpegUnit->mu_VideoData = iomr->iomr_DataStart;

	vidBytes += packetsize;

        /* Here's what's going on.  We can only send words to the CL450, so what we
           do if there is an odd length packet is increase the newpacket size parameter
           by one, and then steal the first byte of the next packet. */

        /* First, see if we need to send off the last byte of the last packet plus the
           first byte of the current packet. */

	if(mpegUnit->mu_IsVidOverflow)
	{
	    kludge = mpegUnit->mu_VidOverflowByte<<8;
	    kludge |= *(UBYTE *)mpegUnit->mu_VideoData;
	    fileBytes+=2;
	    Write(file,&kludge,2);
//	    *mpegUnit->mu_CL450Fifo = kludge;
	    packetsize--;
	    mpegUnit->mu_VideoData = (UWORD *)((ULONG)mpegUnit->mu_VideoData + 1);
	    mpegUnit->mu_IsVidOverflow = FALSE;
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
	}
#if 0
	NewPacket(packetsize, iomr->iomr_PTSHigh, iomr->iomr_PTSMid, iomr->iomr_PTSLow,
				(iomr->iomr_MPEGFlags & MPEGF_VALID_PTS),
				mpegUnit,mpegDevice);
#endif
	mpegUnit->mu_VideoSize = mpegUnit->mu_VideoSize >> 1;
	fileBytes += mpegUnit->mu_VideoSize<<1;
	Write(file,mpegUnit->mu_VideoData,mpegUnit->mu_VideoSize<<1);

	return(TRUE);
    }
    else
    {
//    	D(kprintf("GNV: Bad\n"));
    	return(FALSE);
    }
}

VOID TermIO(struct IOMPEGReq *iomr,struct MPEGDevice *mpegDevice)
{
    ReplyMsg((struct Message *)iomr);
}
