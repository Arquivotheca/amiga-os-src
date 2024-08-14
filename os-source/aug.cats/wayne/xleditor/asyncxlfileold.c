/*******************
    AsyncXLFile.c

      930508

*******************/


#include <exec/types.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <graphics/gfx.h>
#include <graphics/modeid.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "devices/cd.h"

#include "cdxl/pan.h"
#include "cdxl/runcdxl.h"
#include "cdxl/asyncio.h"

#include "cdxl/cdxlob.h"

#include "cdxl/debugsoff.h"

/*	// Uncomment to get debug output turned on
#define KPRINTF
#include "cdxl/debugson.h"
*/

int kprintf(const char *, ...);

IMPORT struct Library *DOSBase;
IMPORT struct Library *SysBase;

IMPORT	CDXLOB * CDXL_OB;	// Global CDXLOB
IMPORT	struct Custom far custom;
IMPORT	ULONG	Count;


/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);


/*****************************************************************************/


VOID
WaitPacket(struct AsyncFile *file)
{
    if ( !file->af_PacketPending )
	return;

    /* This enables signalling when a packet comes back to the port */
    file->af_PacketPort.mp_Flags = PA_SIGNAL;

    /* Wait for the packet to come back, and remove it from the message
     * list. Since we know no other packets can come in to the port, we can
     * safely use Remove() instead of GetMsg(). If other packets could come in,
     * we would have to use GetMsg(), which correctly arbitrates access in such
     * a case
     */
    Remove((struct Node *)WaitPort(&file->af_PacketPort));

    if ( CDXL_OB->AudioSize ) {
	// disable AUD0 interrupt
	custom.intreq = INTF_AUD0;

	Count++;

	if ( Count==1 ) {
	    CDXL_OB->curVideo = 0;
	    CDXL_OB->curAudio = 0;
	    D(PRINTF("(%ld,%ld) ",CDXL_OB->curAudio,CDXL_OB->curVideo);)
	    StartAudio();
	} else {
	    CDXL_OB->curVideo ^= 1;
	    // enable AUD0 interrupt
	    D(PRINTF("(%ld,%ld) ",CDXL_OB->curAudio,CDXL_OB->curVideo);)
	    custom.intena = INTF_SETCLR|INTF_AUD0;
	}

    } else {
	CDXL_OB->curVideo ^= 1;
	Count++;
    }

    /* set the port type back to PA_IGNORE so we won't be bothered with
     * spurious signals
     */
    file->af_PacketPort.mp_Flags = PA_IGNORE;

    /* packet is no longer pending, we got it */
    file->af_PacketPending = FALSE;

} // WaitPacket()


VOID
SendAsync(struct AsyncFile *file, APTR arg2)
{
    /* send out an async packet to the file system. */

    file->af_Packet.sp_Pkt.dp_Port = &file->af_PacketPort;
    file->af_Packet.sp_Pkt.dp_Arg2 = (LONG)arg2;
    PutMsg(file->af_Handler, &file->af_Packet.sp_Msg);
    file->af_PacketPending = TRUE;

} // SendAsync()


LONG
ReadAsyncXL( struct AsyncFile *file, CDXLOB * CDXL_ob )
{
	LONG totalBytes;
	LONG bytesArrived;
	LONG numBytes = CDXL_ob->FrameSize;

    totalBytes = 0;

    /* if we need more bytes than there are in the current buffer, enter the
     * read loop
     */

    if (numBytes > file->af_BytesLeft)
    {
        /* this takes care of NIL: */
        if (!file->af_Handler)
            return(0);

        /* if there's no pending packet, then we hit EOF last time around */
        if (!file->af_PacketPending)
            return(totalBytes);

        while (TRUE)
        {
            WaitPacket(file);

            bytesArrived = file->af_Packet.sp_Pkt.dp_Res1;
            if (bytesArrived <= 0)
            {
                if (bytesArrived <= 0)
                {
                    /* error, get out of here */
                    SetIoErr(file->af_Packet.sp_Pkt.dp_Res2);

                    if (bytesArrived == 0)
                        return(totalBytes);

                    return(-1);
                }
            }

            /* ask that the buffer be filled */
            SendAsync(file,file->af_Buffers[1-file->af_CurrentBuf]);

            if (file->af_SeekOffset > bytesArrived)
                file->af_SeekOffset = bytesArrived;

            file->af_Offset     = (APTR)((ULONG)file->af_Buffers[file->af_CurrentBuf] + file->af_SeekOffset);
            file->af_CurrentBuf = 1 - file->af_CurrentBuf;
            file->af_BytesLeft  = bytesArrived - file->af_SeekOffset;
            file->af_SeekOffset = 0;

            if (numBytes <= file->af_BytesLeft)
                break;

            numBytes   -= file->af_BytesLeft;
//            buffer      = (APTR)((ULONG)buffer + file->af_BytesLeft);
            totalBytes += file->af_BytesLeft;
        }
    }

    file->af_BytesLeft -= numBytes;
    file->af_Offset     = (APTR)((ULONG)file->af_Offset + numBytes);

    return (totalBytes + numBytes);

} // ReadAsyncXL()


LONG
CloseAsyncXL( struct AsyncFile *file, CDXLOB * CDXL_ob )
{
    LONG result;
    LONG result2;

    result = 0;
    if (file)
    {
        if (file->af_PacketPending) {
            WaitPacket(file);
	}

        result  = file->af_Packet.sp_Pkt.dp_Res1;
        result2 = file->af_Packet.sp_Pkt.dp_Res2;

        Close(file->af_File);
        FreeVec(file);

	CDXL_ob->Buffer[0] = CDXL_ob->Buffer[1] = NULL;

        SetIoErr(result2);
    }

    return(result);

} // CloseAsyncXL()


struct AsyncFile *
OpenAsyncXL(const STRPTR fileName, CDXLOB * CDXL_ob, LONG position)
{
    struct AsyncFile  * file;
    struct FileHandle * fh;
    BPTR                handle;
    BPTR                lock;
    LONG                bufferSize = CDXL_ob->FrameSize << 1;
    D_S(struct InfoData,infoData);

    handle = NULL;
    file   = NULL;
    lock   = NULL;

    if (handle = Open(fileName,MODE_OLDFILE)) {

	if ( position && (Seek(handle,position,OFFSET_BEGINNING) < 0) ) {
	    Close(handle);
	    return( NULL );
	}

        /* now allocate the ASyncFile structure, as well as the read buffers.
         * Add 15 bytes to the total size in order to allow for later
         * quad-longword alignement of the buffers
         */

	if (file = AllocVec(sizeof(struct AsyncFile) + bufferSize+15 ,MEMF_CHIP|MEMF_CLEAR)) {
	    file->af_File      = handle;

	    fh                     = BADDR(file->af_File);
	    file->af_Handler       = fh->fh_Type;
	    file->af_BufferSize    = bufferSize / 2;
	    file->af_Buffers[0]    = CDXL_ob->Buffer[0]
	                           = (APTR)(((ULONG)file + sizeof(struct AsyncFile) + 15) & 0xfffffff0);
	    file->af_Buffers[1]    = CDXL_ob->Buffer[1]
				   = (APTR)((ULONG)file->af_Buffers[0] + file->af_BufferSize);
	    file->af_Offset        = file->af_Buffers[0];
	    file->af_CurrentBuf    = 0;
	    file->af_SeekOffset    = 0;
	    file->af_PacketPending = FALSE;

	    /* this is the port used to get the packets we send out back.
	     * It is initialized to PA_IGNORE, which means that no signal is
	     * generated when a message comes in to the port. The signal bit
	     * number is initialized to SIGB_SINGLE, which is the special bit
	     * that can be used for one-shot signalling. The signal will never
	     * be set, since the port is of type PA_IGNORE. We'll change the
	     * type of the port later on to PA_SIGNAL whenever we need to wait
	     * for a message to come in.
	     *
	     * The trick used here avoids the need to allocate an extra signal
	     * bit for the port. It is quite efficient.
	     */

	    file->af_PacketPort.mp_MsgList.lh_Head     = (struct Node *)&file->af_PacketPort.mp_MsgList.lh_Tail;
	    file->af_PacketPort.mp_MsgList.lh_Tail     = NULL;
	    file->af_PacketPort.mp_MsgList.lh_TailPred = (struct Node *)&file->af_PacketPort.mp_MsgList.lh_Head;
	    file->af_PacketPort.mp_Node.ln_Type        = NT_MSGPORT;
	    file->af_PacketPort.mp_Flags               = PA_IGNORE;
	    file->af_PacketPort.mp_SigBit              = SIGB_SINGLE;
	    file->af_PacketPort.mp_SigTask             = FindTask(NULL);

	    file->af_Packet.sp_Pkt.dp_Link          = &file->af_Packet.sp_Msg;
	    file->af_Packet.sp_Pkt.dp_Arg1          = fh->fh_Arg1;
	    file->af_Packet.sp_Pkt.dp_Arg3          = file->af_BufferSize;
	    file->af_Packet.sp_Pkt.dp_Res1          = 0;
	    file->af_Packet.sp_Pkt.dp_Res2          = 0;
	    file->af_Packet.sp_Msg.mn_Node.ln_Name  = (STRPTR)&file->af_Packet.sp_Pkt;
	    file->af_Packet.sp_Msg.mn_Node.ln_Type  = NT_MESSAGE;
	    file->af_Packet.sp_Msg.mn_Length        = sizeof(struct StandardPacket);

	   /* Send out the first read packet to
	    * the file system. While the application is getting ready to
	    * read data, the file system will happily fill in this buffer
	    * with DMA transfers, so that by the time the application
	    * needs the data, it will be in the buffer waiting
	    */

	    file->af_Packet.sp_Pkt.dp_Type = ACTION_READ;
	    file->af_BytesLeft             = 0;
/*
	    if (file->af_Handler)
		SendAsync(file,file->af_Buffers[0]);
*/
	} else {
	    Close(handle);
	}
    }

    return(file);

} // OpenAsyncXL()


StepFrame( /*struct AsyncFile * xlfile,*/ CDXLOB * CDXL_ob, 
 DISP_DEF * disp_def, BOOL Forward )
{
 IMPORT struct AsyncFile	* xlfile;


    if ( !(xlfile && CDXL_ob) )
	return( RC_FAILED );

    if ( Forward ) {
	if ( CDXL_ob->FrameNum >= (CDXL_ob->NumFrames-1) )
	    return( RC_OK );

	CDXL_ob->FrameNum++;
    } else {
	if ( CDXL_ob->FrameNum < 1 )
	    return( RC_OK );

	CDXL_ob->FrameNum--;
    }

    if ( !Forward && Seek(xlfile->af_File,(CDXL_ob->FrameSize * CDXL_ob->FrameNum),OFFSET_BEGINNING) == -1L )
	return( RC_FAILED );

    Read( xlfile->af_File, CDXL_ob->Buffer[CDXL_ob->curVideo],
	CDXL_ob->FrameSize );

    D(PRINTF("StepFrame() FrameNum= %ld, ((PAN *)CDXL_ob->Buffer[CDXL_ob->curVideo])->Frame= %ld\n",
	CDXL_ob->FrameNum, ((PAN *)CDXL_ob->Buffer[CDXL_ob->curVideo])->Frame );)


    if ( CDXL_ob->flags & CDXL_BLIT ) {
	SHORT DstX = CDXL_ob->rect.MinX;
	SHORT DstY = (disp_def->ModeID & LORESSDBL_KEY) ? (CDXL_ob->rect.MinY>>1) : CDXL_ob->rect.MinY;
	SHORT Wid = (CDXL_ob->rect.MaxX - CDXL_ob->rect.MinX) - CDXL_ob->xoff;
	SHORT Ht = (CDXL_ob->rect.MaxY - CDXL_ob->rect.MinY) - CDXL_ob->yoff;
/*
	BltBitMap( &CDXL_ob->bm[CDXL_ob->curVideo],0,0,disp_def->bm[CDXL_ob->curVideo],
	    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);
*/
	BltBitMap( &CDXL_ob->bm[CDXL_ob->curVideo],0,0,disp_def->bm[0],
	    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);

	BltBitMap( &CDXL_ob->bm[CDXL_ob->curVideo],0,0,disp_def->bm[1],
	    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);
    }


    return( RC_OK );

} // StepFrame()


GoToFrame( /*struct AsyncFile * xlfile,*/ CDXLOB * CDXL_ob, 
 DISP_DEF * disp_def )
{
 IMPORT struct AsyncFile	* xlfile;


    if ( !(xlfile && CDXL_ob) )
	return( RC_FAILED );

    if ( CDXL_ob->FrameNum > (CDXL_ob->NumFrames-1) )
	CDXL_ob->FrameNum = (CDXL_ob->NumFrames-1);

    if ( CDXL_ob->FrameNum < 0 )
	CDXL_ob->FrameNum = 0;

    if ( Seek(xlfile->af_File,(CDXL_ob->FrameSize * CDXL_ob->FrameNum),OFFSET_BEGINNING) == -1L )
	return( RC_FAILED );

    Read( xlfile->af_File, CDXL_ob->Buffer[CDXL_ob->curVideo],
	CDXL_ob->FrameSize );

    D(PRINTF("StepFrame() FrameNum= %ld, ((PAN *)CDXL_ob->Buffer[CDXL_ob->curVideo])->Frame= %ld\n",
	CDXL_ob->FrameNum, ((PAN *)CDXL_ob->Buffer[CDXL_ob->curVideo])->Frame );)

    if ( CDXL_ob->flags & CDXL_BLIT ) {
	SHORT DstX = CDXL_ob->rect.MinX;
	SHORT DstY = (disp_def->ModeID & LORESSDBL_KEY) ? (CDXL_ob->rect.MinY>>1) : CDXL_ob->rect.MinY;
	SHORT Wid = (CDXL_ob->rect.MaxX - CDXL_ob->rect.MinX) - CDXL_ob->xoff;
	SHORT Ht = (CDXL_ob->rect.MaxY - CDXL_ob->rect.MinY) - CDXL_ob->yoff;
/*
	BltBitMap( &CDXL_ob->bm[CDXL_ob->curVideo],0,0,disp_def->bm[CDXL_ob->curVideo],
	    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);
*/
	BltBitMap( &CDXL_ob->bm[CDXL_ob->curVideo],0,0,disp_def->bm[0],
	    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);

	BltBitMap( &CDXL_ob->bm[CDXL_ob->curVideo],0,0,disp_def->bm[1],
	    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);
    }


    return( RC_OK );

} // GoToFrame()


