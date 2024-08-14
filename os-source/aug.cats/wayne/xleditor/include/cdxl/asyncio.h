/*******************************************************************************
 *
 * (c) Copyright 1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 ******************************************************************************/

#ifndef ASYNCIO_H
#define ASYNCIO_H

/*****************************************************************************/


#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>


/*****************************************************************************/


struct AsyncFile
{
    BPTR                  af_File;
    ULONG                 af_BlockSize;
    struct MsgPort       *af_Handler;
    APTR                  af_Offset;
    LONG                  af_BytesLeft;
    ULONG	          af_BufferSize;
    APTR	          af_Buffers[2];
    struct StandardPacket af_Packet;
    struct MsgPort        af_PacketPort;
    ULONG                 af_CurrentBuf;
    ULONG                 af_SeekOffset;
    UBYTE	          af_PacketPending;
    UBYTE	          af_ReadMode;
};


/*****************************************************************************/


#define MODE_READ   0  /* read an existing file                             */
#define MODE_WRITE  1  /* create a new file, delete existing file if needed */
#define MODE_APPEND 2  /* append to end of existing file, or create new     */

#define MODE_START   -1   /* relative to start of file         */
#define MODE_CURRENT  0   /* relative to current file position */
#define MODE_END      1   /* relative to end of file	     */


/*****************************************************************************/


struct AsyncFile *OpenAsync(const STRPTR fileName, UBYTE accessMode, LONG bufferSize);
LONG CloseAsync(struct AsyncFile *file);
LONG ReadAsync(struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG ReadCharAsync(struct AsyncFile *file);
LONG WriteAsync(struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG WriteCharAsync(struct AsyncFile *file, char ch);
LONG SeekAsync(struct AsyncFile *file, LONG position, BYTE mode);


/*****************************************************************************/


#endif /* ASYNCIO_H */
