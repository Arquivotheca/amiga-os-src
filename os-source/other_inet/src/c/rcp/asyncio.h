#ifndef ASYNCIO_H
#define ASYNCIO_H


/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif


/*****************************************************************************/


/* This structure is public only by necessity, don't muck with it yourself, or
 * you're looking for trouble
 */
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

#define OPT_COUNT 7

struct GlobalData
{
    struct Library  *EBase;           // ExecBase (SysBase)
    struct Library  *DBase;           // DOSBase
    struct Library  *SBase;           // SockBase
    ULONG            errno;           // The lovely Unixish errno!
    ULONG            Socket;          // Socket this activity is happening on.
    UBYTE            Flags;           // Flags that relate to our operation (see below)
    UBYTE            Filler;          // To word-align things
    UBYTE            SourceUser[80];  // Usernames for both sides of the transfer
    UBYTE            DestUser[80];    // Ditto.
    UBYTE            SourceHost[80];  // For user-started rcp's, the name of the source file host
    UBYTE            DestHost[80];    // Same as above, but the name of the destination file host
    ULONG            Opts[OPT_COUNT]; // ReadArgs options
    UBYTE            RAIn[256];       // Huge buffers for ReadArgs.  :'(  Also used as general
    UBYTE            RAToken[256];    // Same as above.                   purpose buffers.
    ULONG	     rc;
};


/*****************************************************************************/


#define MODE_READ   0  /* read an existing file                             */
#define MODE_WRITE  1  /* create a new file, delete existing file if needed */
#define MODE_APPEND 2  /* append to end of existing file, or create new     */

#define MODE_START   -1   /* relative to start of file         */
#define MODE_CURRENT  0   /* relative to current file position */
#define MODE_END      1   /* relative to end of file	     */


/*****************************************************************************/


struct AsyncFile *OpenAsync(struct GlobalData *g, const STRPTR fileName, UBYTE accessMode, LONG bufferSize);
LONG CloseAsync(struct GlobalData *g, struct AsyncFile *file);
LONG ReadAsync(struct GlobalData *g, struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG WriteAsync(struct GlobalData *g, struct AsyncFile *file, APTR buffer, LONG numBytes);


/*****************************************************************************/


#endif /* ASYNCIO_H */
