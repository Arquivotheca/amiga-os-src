

#include "classbase.h"


/*****************************************************************************/


extern VOID BodyExpander(VOID);
extern VOID BodyExpanderAligned(VOID);

typedef VOID (*EXPANDERFUNC)(VOID);


/*****************************************************************************/


struct StartupPkt
{
    struct Message  sp_Msg;
    struct BitMap  *sp_BitMap;
    UWORD	    sp_DestBytesPerRow;
    UWORD           sp_SrcBytesPerRow;
    UWORD	    sp_Depth;
    UWORD           sp_MaskDepth;
    UBYTE           sp_CompressionType;
    UBYTE           sp_Pad;

    /* response from sub-task */
    struct MsgPort *sp_ExpanderPort;
};


/*****************************************************************************/


#define BUFFER_SIZE 4096

struct DataPkt
{
    struct Message dp_Msg;
    LONG           dp_NumBytes;          /* <0 for error, or all done */
    LONG           dp_Error;
    UBYTE          dp_Data[BUFFER_SIZE];
};


/*****************************************************************************/


#define ME_TASK 	0
#define ME_STACK	1
#define NUMENTRIES	2

struct FakeMemEntry
{
    ULONG fme_Reqs;
    ULONG fme_Length;
};

struct FakeMemList
{
    struct Node fml_Node;
    UWORD	fml_NumEntries;
    struct FakeMemEntry fml_ME[NUMENTRIES];
} TaskMemTemplate = {
    { 0 },
    NUMENTRIES,
    {
	{ MEMF_REVERSE | MEMF_CLEAR, sizeof( struct Task ) },
	{ MEMF_REVERSE | MEMF_CLEAR,	0 }					/* stack */
    }
};


#undef SysBase

static struct Task *MakeTask(struct Library *SysBase, STRPTR name, LONG pri,
                             APTR initPC, ULONG stackSize, APTR userData)
{
struct Task        *newTask;
struct FakeMemList  fakememlist;
struct MemList     *ml;
APTR                result;

    fakememlist = TaskMemTemplate;
    fakememlist.fml_ME[ME_STACK].fme_Length = stackSize;

    if (ml = (struct MemList *) AllocEntry((struct MemList *)&fakememlist))
    {
        newTask                  = (struct Task *) ml->ml_ME[ME_TASK].me_Addr;
        newTask->tc_SPLower      = ml->ml_ME[ME_STACK].me_Addr;
        newTask->tc_SPUpper      = (APTR)((ULONG)(newTask->tc_SPLower) + stackSize);
        newTask->tc_SPReg        = newTask->tc_SPUpper;
        newTask->tc_Node.ln_Type = NT_TASK;
        newTask->tc_Node.ln_Pri  = pri;
        newTask->tc_Node.ln_Name = name;
        newTask->tc_UserData     = userData;

        NewList(&newTask->tc_MemEntry);
        AddHead(&newTask->tc_MemEntry,ml);

        if (result = AddTask(newTask, initPC, 0))
            return(newTask);

        FreeEntry(ml);
    }

    return(NULL);
}

#define SysBase cb->cb_SysBase


/***************************************************************************/

#define cmpByteRun2 2
#define mskHasAlpha 4


LONG GetBody(struct ClassBase *cb,
             struct IFFHandle *iff,
             struct BitMapHeader *bmhd,
             struct BitMap *bitMap)
{
struct StartupPkt  startup;
struct DataPkt    *data;
struct DataPkt    *buffers[2];
struct MsgPort    *expanderPort;
struct MsgPort    *readerPort;
LONG               num;
LONG               error;
ULONG              flags;
EXPANDERFUNC       func;

    if (!bmhd->bmh_Width || !bmhd->bmh_Height || !bmhd->bmh_Depth)
        return(1);   /* success */

    if (bmhd->bmh_Compression > cmpByteRun2)
        return(IFFERR_MANGLED);

    if (bmhd->bmh_Masking > mskHasAlpha)
        return(IFFERR_MANGLED);

    error = IFFERR_NOMEM;

    /* set things up */
    readerPort = CreateMsgPort();
    buffers[0] = AllocVec(sizeof(struct DataPkt),MEMF_ANY);
    buffers[1] = AllocVec(sizeof(struct DataPkt),MEMF_ANY);

    if (readerPort && buffers[0] && buffers[1])
    {
        /* if we got our resources, initialize the data packets */
        buffers[0]->dp_Msg.mn_ReplyPort = readerPort;
        buffers[0]->dp_Msg.mn_Length    = sizeof(struct DataPkt);
        buffers[0]->dp_Error            = 0;
        buffers[1]->dp_Msg.mn_ReplyPort = readerPort;
        buffers[1]->dp_Msg.mn_Length    = sizeof(struct DataPkt);
        buffers[1]->dp_Error            = 0;

        /* initialize the startup packet */
        startup.sp_Msg.mn_ReplyPort = readerPort;
        startup.sp_Msg.mn_Length    = sizeof(struct StartupPkt);
        startup.sp_BitMap           = bitMap;
        startup.sp_CompressionType  = bmhd->bmh_Compression;
        startup.sp_Depth            = bitMap->Depth;
        startup.sp_SrcBytesPerRow   = RowBytes(bmhd->bmh_Width);
        startup.sp_DestBytesPerRow  = bitMap->BytesPerRow;
        startup.sp_MaskDepth        = 0;

        if (bmhd->bmh_Masking == mskHasMask)
            startup.sp_MaskDepth = 1;

        else if (bmhd->bmh_Masking == mskHasAlpha)
            startup.sp_MaskDepth = bmhd->bmh_Transparent;

        func  = BodyExpander;
        flags = GetBitMapAttr(bitMap,BMA_FLAGS);
        if ((flags & BMF_INTERLEAVED) && !startup.sp_MaskDepth && (startup.sp_CompressionType > cmpNone))
        {
            /* if the number of bytes for each line of the picture matches
             * in both the source file, and in the target bitmap, then
             * fake things so it looks like we have a 1 bit-plane deep picture,
             * with (realBytesPerRow * realDepth) number of bytes in it. This
             * avoids the (small) overhead associated with stepping through
             * the planes of the picture, as only one plane is handled.
             */
            if (startup.sp_SrcBytesPerRow == (ULONG)bitMap->Planes[1] - (ULONG)bitMap->Planes[0])
            {
                startup.sp_SrcBytesPerRow  = bitMap->BytesPerRow;
                startup.sp_DestBytesPerRow = bitMap->BytesPerRow;
                startup.sp_Depth           = 1;
                func = BodyExpanderAligned;
            }
        }

        /* create the subtask */
        if (MakeTask(SysBase,"« BodyExpander »",0,func,792,&startup))
        {
            /* wait for the startup packet to come back */
            WaitPort(readerPort);
            GetMsg(readerPort);

            /* check if the subtask managed to allocate a message port */
            if (expanderPort = startup.sp_ExpanderPort)
            {
                /* prime the pump, sorta... */
                PutMsg(readerPort,buffers[0]);
                PutMsg(readerPort,buffers[1]);

                while (TRUE)
                {
		    /* get a data packet to read data into */
                    WaitPort(readerPort);
                    data = (struct DataPkt *)GetMsg(readerPort);

		    /* if we got a data packet with errors in it, quit */
                    if (data->dp_Error)
                    {
                        error = IFFERR_MANGLED;
                        break;
                    }

                    /* fill the packet in */
                    num = ReadChunkBytes(iff,data->dp_Data,BUFFER_SIZE);
                    if (num <= 0)
                    {
                        if (num)
                        {
                            /* error reading file */
                            error = IFFERR_READ;
                        }
                        else
                        {
                            /* we're done, indicate success */
                            error = 1;
                        }
                        break;
                    }

		    /* send the packet to the subtask for processing */
                    data->dp_NumBytes = num;
                    PutMsg(expanderPort,data);
                }

		/* indicate the end of the world to the subtask */
                data->dp_NumBytes = -1;
                PutMsg(expanderPort,data);

                /* make sure everything came back before quiting */
                WaitPort(readerPort);
                GetMsg(readerPort);
                WaitPort(readerPort);
                GetMsg(readerPort);
            }
        }
    }

    /* clean up */
    DeleteMsgPort(readerPort);
    FreeVec(buffers[0]);
    FreeVec(buffers[1]);

    return(error);
}
