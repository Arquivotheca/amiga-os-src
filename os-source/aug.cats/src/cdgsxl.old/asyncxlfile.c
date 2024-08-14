/***************************************
          AsyncXLFile.c

             930508

  Based upon Martin Taillefer's
  asyncio.c - See AmigaMail
  Vol.2 Section 2 P77 for more details.

****************************************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */


#include <exec/types.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <graphics/gfx.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "devices/cd.h"

#include "cdxl/asyncXL.h"

#include "cdxl/cdxlob.h"

#include "cdxl/debugsoff.h"

/*	// Uncomment to get debug output turned on
#define KPRINTF
#include "cdxl/debugson.h"
*/

int kprintf(const char *, ...);
VOID StartAudio( VOID );

IMPORT struct Library *DOSBase;
IMPORT struct Library *SysBase;

IMPORT	CDXLOB * CDXL_OB;	// Global CDXLOB
IMPORT	struct Custom far custom;
IMPORT	ULONG	Count;


VOID
WaitPacket(ASYNCXLFILE *xlfile)
{
    if ( !xlfile->af_PacketPending )
	return;

    /* This enables signalling when a packet comes back to the port */
    xlfile->af_PacketPort.mp_Flags = PA_SIGNAL;

    /* Wait for the packet to come back, and remove it from the message
     * list. Since we know no other packets can come in to the port, we can
     * safely use Remove() instead of GetMsg(). If other packets could come in,
     * we would have to use GetMsg(), which correctly arbitrates access in such
     * a case
     */
    Remove((struct Node *)WaitPort(&xlfile->af_PacketPort));

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
    xlfile->af_PacketPort.mp_Flags = PA_IGNORE;

    /* packet is no longer pending, we got it */
    xlfile->af_PacketPending = FALSE;

} // WaitPacket()


VOID
SendAsync(ASYNCXLFILE *xlfile, APTR arg2)
{
    /* send out an async packet to the file system. */

    xlfile->af_Packet.sp_Pkt.dp_Port = &xlfile->af_PacketPort;
    xlfile->af_Packet.sp_Pkt.dp_Arg2 = (LONG)arg2;
    PutMsg(xlfile->af_Handler, &xlfile->af_Packet.sp_Msg);
    xlfile->af_PacketPending = TRUE;

} // SendAsync()


LONG
CloseAsyncXL( ASYNCXLFILE *xlfile, CDXLOB * CDXL_ob )
{
    LONG result;
    LONG result2;

    result = 0;
    if (xlfile)
    {
        if (xlfile->af_PacketPending) {
            WaitPacket(xlfile);
	}

        result  = xlfile->af_Packet.sp_Pkt.dp_Res1;
        result2 = xlfile->af_Packet.sp_Pkt.dp_Res2;

        Close(xlfile->af_File);
        FreeVec(xlfile);

	CDXL_ob->Buffer[0] = CDXL_ob->Buffer[1] = NULL;

        SetIoErr(result2);
    }

    return(result);

} // CloseAsyncXL()


ASYNCXLFILE *
OpenAsyncXL(const STRPTR fileName, CDXLOB * CDXL_ob, LONG position)
{
    ASYNCXLFILE  * xlfile;
    struct FileHandle * fh;
    BPTR                handle;
    BPTR                lock;
    LONG                bufferSize = CDXL_ob->FrameSize << 1;

    handle = NULL;
    xlfile   = NULL;
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

	if (xlfile = AllocVec(sizeof(ASYNCXLFILE) + bufferSize+15 ,MEMF_CHIP|MEMF_CLEAR)) {
	    xlfile->af_File      = handle;

	    fh                     = BADDR(xlfile->af_File);
	    xlfile->af_Handler       = fh->fh_Type;
	    xlfile->af_BufferSize    = bufferSize / 2;
	    xlfile->af_Buffers[0]    = CDXL_ob->Buffer[0]
	                           = (APTR)(((ULONG)xlfile + sizeof(ASYNCXLFILE) + 15) & 0xfffffff0);
	    xlfile->af_Buffers[1]    = CDXL_ob->Buffer[1]
				   = (APTR)((ULONG)xlfile->af_Buffers[0] + xlfile->af_BufferSize);
	    xlfile->af_CurrentBuf    = 0;
	    xlfile->af_PacketPending = FALSE;

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

	    xlfile->af_PacketPort.mp_MsgList.lh_Head     = (struct Node *)&xlfile->af_PacketPort.mp_MsgList.lh_Tail;
	    xlfile->af_PacketPort.mp_MsgList.lh_Tail     = NULL;
	    xlfile->af_PacketPort.mp_MsgList.lh_TailPred = (struct Node *)&xlfile->af_PacketPort.mp_MsgList.lh_Head;
	    xlfile->af_PacketPort.mp_Node.ln_Type        = NT_MSGPORT;
	    xlfile->af_PacketPort.mp_Flags               = PA_IGNORE;
	    xlfile->af_PacketPort.mp_SigBit              = SIGB_SINGLE;
	    xlfile->af_PacketPort.mp_SigTask             = FindTask(NULL);

	    xlfile->af_Packet.sp_Pkt.dp_Link          = &xlfile->af_Packet.sp_Msg;
	    xlfile->af_Packet.sp_Pkt.dp_Arg1          = fh->fh_Arg1;
	    xlfile->af_Packet.sp_Pkt.dp_Arg3          = xlfile->af_BufferSize;
	    xlfile->af_Packet.sp_Pkt.dp_Res1          = 0;
	    xlfile->af_Packet.sp_Pkt.dp_Res2          = 0;
	    xlfile->af_Packet.sp_Msg.mn_Node.ln_Name  = (STRPTR)&xlfile->af_Packet.sp_Pkt;
	    xlfile->af_Packet.sp_Msg.mn_Node.ln_Type  = NT_MESSAGE;
	    xlfile->af_Packet.sp_Msg.mn_Length        = sizeof(struct StandardPacket);

	   /* Send out the first read packet to
	    * the file system. While the application is getting ready to
	    * read data, the file system will happily fill in this buffer
	    * with DMA transfers, so that by the time the application
	    * needs the data, it will be in the buffer waiting
	    */

	    xlfile->af_Packet.sp_Pkt.dp_Type = ACTION_READ;
/*
	    if (xlfile->af_Handler)
		SendAsync(xlfile,xlfile->af_Buffers[0]);
*/
	} else {
	    Close(handle);
	}
    }

    return(xlfile);

} // OpenAsyncXL()

