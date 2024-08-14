
#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#define TASKMSGSIGBIT       16
#define DISKCHANGESIGBIT    17

#define MSG_KEYSTROKE   0x0000
#define MSG_TIMEMODE    0x0100
#define MSG_STARTUP     0x0200
#define MSG_SHUTDOWN    0x0300


#define SysBase (*((struct ExecBase **)0x00000004L))


#define FASTMODECOUNT   800000
#define SKIPTRACKCOUNT  200000

#define QUICKUPDATEFREQ 16666
#define SLOWUPDATEFREQ  100000

/***********************************************************************/

struct AnalData {

    char *Buff;
    UWORD BuffSize;
    UBYTE AvgCount1;
    UBYTE AvgCount2;
    UBYTE Amplitude;
    UBYTE Pad;
    };

struct AnalPack {

    struct List            XLList;
    struct CDXL            CDXL[2];
    BYTE                  *SampleData;
    BYTE                  *AnalBuff;
    BYTE                  *SampleReady;
    struct AnalData        AnalData[12];
    };


struct PlayerLibrary {

    struct Library          PlayerLib;

    struct AnalPack         AnalPack;
    UBYTE                   Analysis[12];

    APTR                    SegList;

    struct Task            *PlayerTask;
    struct MsgPort         *TaskMsgPort;
    struct MsgPort         *TaskReplyPort;

    struct PlayerOptions    PlayerOptions;
    struct PlayerState      PlayerState;
    struct PlayList         PlayList;

    struct SignalSemaphore  PlayListSemaphore;
    struct SignalSemaphore  PlayStateSemaphore;
    };


struct ChangeData {

    struct Task *LibraryTask;
    ULONG        Signal;
    };


struct V {

    struct PlayerLibrary  *PlayerBase;

    struct MsgPort        *InputReplyPort;
    struct IOStdReq       *InputReq;

    struct MsgPort        *TimeReplyPort;
    struct timerequest    *TimeReq;
    UWORD                  TimeReqOutstanding;

    struct MsgPort        *SQTimeReplyPort;
    struct timerequest    *SQTimeReq;
    UWORD                  SQTimeReqOutstanding;

    struct MsgPort        *CDPlayReplyPort;
    struct IOStdReq       *CDPlayReq;
    UWORD                  CDPlayReqOutstanding;

    struct MsgPort        *CDReplyPort;
    struct IOStdReq       *CDReq;

    struct MsgPort        *DiskChangeReplyPort;
    struct IOStdReq       *DiskChangeReq;

    struct QCode           QCodePacket;
    union  LSNMSF          InvalidPos;
    ULONG                  CurrentPos;
    ULONG                  SQUpdateFreq;

    ULONG                  TotalPlayTime;
    ULONG                  BegTrackPlayTime;

    UWORD                  PlayListAllocated;

    union CDTOC            TOC[100];
    };


struct CompTime {

    UBYTE        Tracks;
    union LSNMSF Time;
    };


