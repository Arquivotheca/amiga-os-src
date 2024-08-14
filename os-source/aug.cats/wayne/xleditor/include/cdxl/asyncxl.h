/*******************************************************************************
 *
 * (c) Copyright 1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 ******************************************************************************/

#ifndef ASYNCXL_H
#define ASYNCXL_H

/*****************************************************************************/


#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>


/*****************************************************************************/


typedef struct AsyncXLFile
{
    BPTR                   af_File;
    struct MsgPort       * af_Handler;
    ULONG	           af_BufferSize;
    APTR	           af_Buffers[2];
    struct StandardPacket  af_Packet;
    struct MsgPort         af_PacketPort;
    ULONG                  af_CurrentBuf;
    UBYTE	           af_PacketPending;

} ASYNCXLFILE;


/*****************************************************************************/


#endif /* ASYNCIO_H */
