
#include <stdio.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <clib/alib_protos.h>

#include "cd.h"
#include "cdprivate.h"

extern struct ExecBase *SysBase;
extern struct DOSBase  *DOSBase;

extern struct CDDB *InitDevice(void);
extern __asm void   AddDevTask(register __a0 struct CDDB *);

extern __far UBYTE BCDTable[];

void __saveds DriveTask(void);
struct Task * AddDriveTask(struct CDDB *DB, STRPTR);

#define EOL(c)   ((!c) || c==10 || c==13)
#define IsAscDig(c) (c>='0' && c<='9')
#define ToDec(c) (((*c-'0')*10) + *(c+1)-'0')


BPTR   ISOFile;



ULONG LSNtoMSF(ULONG LSN) {

ULONG MSF;

    MSF  = LSN % 75;          LSN /= 75;
    MSF |= ((LSN % 60) << 8); LSN /= 60;
    MSF |= LSN << 16;

    return(MSF);
    }



ULONG ToBCD(UBYTE Dec) {

    return(BCDTable[Dec]);
    }



ULONG LSNtoMSFBCD(ULONG LSN) {

ULONG MSF;

    MSF = LSNtoMSF(LSN);

    return(ToBCD(MSF) | (ToBCD(MSF>>8)<<8) | (ToBCD(MSF>>16)<<16));
    }



ULONG MSFSTRtoLSN(char *MSFSTR) {

ULONG LSN;

    if (IsAscDig(*MSFSTR) && IsAscDig(*(MSFSTR+1))) {

        LSN = ToDec(MSFSTR)*60*75; MSFSTR+=2;

        if (*MSFSTR++ == ':') {

            if (IsAscDig(*MSFSTR) && IsAscDig(*(MSFSTR+1))) {

                LSN += ToDec(MSFSTR)*75; MSFSTR+=2;

                if (*MSFSTR++ == ':') {

                    if (IsAscDig(*MSFSTR) && IsAscDig(*(MSFSTR+1))) {

                        LSN += ToDec(MSFSTR);

                        return(LSN);
                        }
                    }
                }
            }
        }

    return(-1);
    }



int main (int argc, char **argv) {

BPTR   File;
char  *B, line[132], ISOFileName[132];
int    Track = 1, i, CDROM = 0;
ULONG  Position = 150;

struct FileInfoBlock  FIB;
struct CDDB          *DB;

    if (argc <= 1) printf("usage: %s <TOCFile> <-flags>\n", argv[0]);

    else if (FindName(&SysBase->DeviceList, "cd.device"))

        printf("cd.device is already in the system.  SimAmigaCD installation failed.\n");

    else if (DOSBase = (struct DOSBase *)OpenLibrary("dos.library", 0)) {

        if (File = Open((STRPTR)argv[1], MODE_OLDFILE)) {

            if (DB = InitDevice()) {

                while (B = FGets(File, line, 132)) {

                    while (*B==' ') B++;                                                // Find first character in line

                    if (!EOL(*B)) {                                                     // Is this an empty line?

                        if (Track == 1) if (*B<'0' || *B>'9') {                         // First track?  check for filename

                            for (i=0; !EOL(*B) && *B!=' '; i++) ISOFileName[i] = *B++;  // Store the ISO image filename
                            ISOFileName[i] = 0;

                            CDROM = 1;                                                  // This is a CD-ROM disk

                            if (ISOFile = Open((STRPTR)ISOFileName, MODE_OLDFILE)) {

                                ExamineFH(ISOFile, &FIB);

//                                printf("DiskKey = %08lx\n", FIB.fib_DiskKey);

                                DB->db_TOC[Track].Entry.CtlAdr       = CTL_DATA | 0x01; // CD_ROM || (Addr = 1)
                                DB->db_TOC[Track].Entry.Track        = Track;
                                DB->db_TOC[Track].Entry.Position.LSN = LSNtoMSF(Position);

                                Position += (FIB.fib_Size / 2048);

                                Close(ISOFile);
                                }

                            else printf("Could not open '%s'\n", ISOFileName);
                            }

                        if (Track>1 || !CDROM) {                                        // Check for MSF time?

                            if (MSFSTRtoLSN(B) == -1)
                                printf("Invalid time at track %d\n", Track);

                            else {

                                DB->db_TOC[Track].Entry.CtlAdr       = 0x01;            // Addr = 1
                                DB->db_TOC[Track].Entry.Track        = Track;
                                DB->db_TOC[Track].Entry.Position.LSN = LSNtoMSF(Position);

                                Position += MSFSTRtoLSN(B);

                                if (Position > 72*60*75)
                                    printf("TOC time exceeds 72:00:00\n");
                                }
                            }

                        Track++;
                        }
                    }

                DB->db_TOC[0].Summary.FirstTrack  = 1;
                DB->db_TOC[0].Summary.LastTrack   = Track - 1;
                DB->db_TOC[0].Summary.LeadOut.LSN = LSNtoMSF(Position);

                DB->db_TOC[Track].Entry.Position.LSN = LSNtoMSF(Position);

                Close(File);

                AddDevTask(DB);

                if (CDROM) AddDriveTask(DB, ISOFileName);

                Cli()->cli_Module = 0;                                                  // Do not free this program
                }

            else printf("Could not add cd.device\n");
            }

        else printf("Could not open '%s'\n", argv[1]);

        CloseLibrary((struct Library *)DOSBase);
        }

    return(0);
    }



struct StartupMsg {

    struct Message Message;
    STRPTR         FileName;
    };

#define TASKMSGSIGBIT   16
#define TASKSIGNALBIT   17



struct Task * AddDriveTask(struct CDDB *DB, STRPTR ISOFileName) {

struct StartupMsg *StartupMsg;

    if (DB->db_TaskMsgPort = AllocMem(sizeof(struct MsgPort), MEMF_PUBLIC | MEMF_CLEAR)) {

        DB->db_TaskMsgPort->mp_Node.ln_Name = "SimAmigaCD_Drive_Port";
        DB->db_TaskMsgPort->mp_Node.ln_Type = NT_MSGPORT;
        DB->db_TaskMsgPort->mp_Flags        = PA_SIGNAL;
        DB->db_TaskMsgPort->mp_SigTask      = NULL;
        DB->db_TaskMsgPort->mp_SigBit       = TASKMSGSIGBIT;

        NewList(&DB->db_TaskMsgPort->mp_MsgList);
        AddPort(DB->db_TaskMsgPort);

        if (DB->db_DriveTask = CreateNewProcTags(NP_Entry, DriveTask, NP_Priority, 11)) {

            DB->db_TaskMsgPort->mp_SigTask = DB->db_DriveTask;

            if (StartupMsg = (struct StartupMsg *)AllocMem(sizeof(struct StartupMsg), MEMF_PUBLIC | MEMF_CLEAR)) {
    
                StartupMsg->Message.mn_Length    = 0;
                StartupMsg->Message.mn_ReplyPort = (struct MsgPort *)DB;
                StartupMsg->FileName             = ISOFileName;
                PutMsg(DB->db_TaskMsgPort, StartupMsg);

                return(DB->db_DriveTask);
                }
            }

        RemPort(DB->db_TaskMsgPort);
        FreeMem(DB->db_TaskMsgPort, sizeof(struct MsgPort));
        }

    return(NULL);
    }





#define ROMBUFFBLOCKS 64
#define ROMBUFFSIZE   ROMBUFFBLOCKS*2048



void StartROMInterrupt(struct timerequest *TReq, ULONG Micros) {

    TReq->tr_time.tv_secs    = 0;
    TReq->tr_time.tv_micro   = Micros;
    TReq->tr_node.io_Command = TR_ADDREQUEST;

    SendIO(TReq);
    }




UBYTE *FindFreeROMPage(struct CDDB *DB) {

ULONG i;

    for (i=0; i!=16; i++) {

        if (*((ULONG *)((ULONG)DB->db_CDROMPage + i * 4096 + ROM_STATUS)) & SECSTSF_INVALID)
            return((UBYTE *)DB->db_CDROMPage + i * 4096);
        }

    return(NULL);
    }



VOID __saveds __asm TimerInterrupt(register __a1 ULONG *is_Data) {

struct CDDB *DB;
UBYTE       *Page;

    DB = (struct CDDB *)is_Data;

    if (!DB->db_DriveAbortTimer) {

        if (Page = FindFreeROMPage(DB)) {

            CopyMemQuick(DB->db_ROMBuffer + DB->db_BuffIndex * 2048, Page + ROM_DATA, 2048);

            *((ULONG *)(Page + ROM_HEADER)) = (LSNtoMSFBCD(DB->db_Sector++ + 150) << 8) | 0x00000001;
            *((ULONG *)(Page + ROM_STATUS)) = (DB->db_CNT++ & SECSTSF_CNT);

            if (++DB->db_BuffIndex == ROMBUFFBLOCKS) DB->db_BuffIndex = 0;

            if (!DB->db_BuffIndex || (DB->db_BuffIndex == (ROMBUFFBLOCKS/2))) {

                Signal(DB->db_DriveTask, 1<<TASKSIGNALBIT);
                }

            StartROMInterrupt(DB->db_DriveTimerReq, DB->db_Speed);
            }

        else DB->db_Reading = 0;
        }

    else DB->db_DriveAbortTimer = 0;

    Signal(&DB->db_Task, SIGF_PBX);
    }




void __saveds DriveTask(void) {

struct CDDB        *DB;
struct StartupMsg  *StartupMsg;
struct Message     *Message;
struct MsgPort     *MsgPort;

struct Interrupt    TimerInt;
struct MsgPort     *TimerPort, *SeekTimerPort;
struct timerequest *SeekTimerReq;

STRPTR              ISOFileName;
BPTR                File;
LONG                Delay;
ULONG               Event;

    WaitPort(MsgPort = FindPort("SimAmigaCD_Drive_Port"));

    DB = (struct CDDB *)(StartupMsg = (struct StartupMsg *)GetMsg(MsgPort))->Message.mn_ReplyPort;

    ISOFileName = StartupMsg->FileName;

    FreeMem(StartupMsg, sizeof(struct StartupMsg));

    TimerInt.is_Node.ln_Type = NT_INTERRUPT;
    TimerInt.is_Node.ln_Pri  = 0;
    TimerInt.is_Node.ln_Name = "CDROM Interrupt";
    TimerInt.is_Data         = DB;
    TimerInt.is_Code         = (void (*)())TimerInterrupt;

    if (TimerPort = CreateMsgPort()) {

        TimerPort->mp_SigTask = &TimerInt;
        TimerPort->mp_Flags   = PA_SOFTINT;

        if (DB->db_DriveTimerReq = CreateIORequest(TimerPort, sizeof(struct timerequest))) {

            if (!OpenDevice("timer.device", UNIT_ECLOCK, DB->db_DriveTimerReq, 0)) {

                if (SeekTimerPort = CreateMsgPort()) {

                    if (SeekTimerReq = CreateIORequest(SeekTimerPort, sizeof(struct timerequest))) {

                        if (!OpenDevice("timer.device", UNIT_MICROHZ, SeekTimerReq, 0)) {

                            if (DB->db_ROMBuffer = AllocMem(ROMBUFFSIZE, MEMF_PUBLIC)) {

                                if (File = Open((STRPTR)ISOFileName, MODE_OLDFILE)) {

                                    while(1) {

                                        while (Message = GetMsg(DB->db_TaskMsgPort)) {

                                            switch(Message->mn_Length) {

                                                case 1: // Start Read

                                                        if ((Delay = (ULONG)Message->mn_ReplyPort - DB->db_SeekPosition) < 0)
                                                             Delay = -Delay;

                                                        if (Delay < 27777) Delay = 27777;

                                                        SeekTimerReq->tr_time.tv_secs    = 0;
                                                        SeekTimerReq->tr_time.tv_micro   = Delay * 12;
                                                        SeekTimerReq->tr_node.io_Command = TR_ADDREQUEST;

                                                        SendIO(SeekTimerReq);

                                                        DB->db_Sector = DB->db_SeekPosition = (ULONG)Message->mn_ReplyPort;

                                                        Seek(File, DB->db_SeekPosition * 2048, OFFSET_BEGINNING);

                                                        Read(File, DB->db_ROMBuffer, ROMBUFFSIZE);
                                                        DB->db_BuffIndex       =
                                                        DB->db_CNT             =
                                                        DB->db_DriveAbortTimer = 0;

                                                        WaitIO(SeekTimerReq);

                                                        StartROMInterrupt(DB->db_DriveTimerReq, DB->db_Speed);

                                                        break;

                                                case 2: // Stop Read

                                                        DB->db_DriveAbortTimer = 1;
                                                        break;


                                                case 3: // Seek

                                                        if ((Delay = (ULONG)Message->mn_ReplyPort - DB->db_SeekPosition) < 0)
                                                             Delay = -Delay;

                                                        if (Delay < 27777) Delay = 27777;

                                                        SeekTimerReq->tr_time.tv_secs    = 0;
                                                        SeekTimerReq->tr_time.tv_micro   = Delay * 12;
                                                        SeekTimerReq->tr_node.io_Command = TR_ADDREQUEST;

                                                        SendIO(SeekTimerReq);

                                                        DB->db_SeekPosition = (ULONG)Message->mn_ReplyPort;

                                                        Seek(File, DB->db_SeekPosition * 2048, OFFSET_BEGINNING);

                                                        WaitIO(SeekTimerReq);

                                                        Signal(&DB->db_Task, SIGF_CMDDONE);
                                                        break;

                                                }

                                            FreeMem(Message, sizeof(struct Message));
                                            }

                                        Event = Wait((1<<DB->db_TaskMsgPort->mp_SigBit) | (1<<TASKSIGNALBIT));

                                        if (Event & (1<<TASKSIGNALBIT)) {

                                            if (DB->db_BuffIndex < (ROMBUFFBLOCKS/2))
                                                 Read(File, DB->db_ROMBuffer + ROMBUFFSIZE/2, ROMBUFFSIZE/2);

                                            else Read(File, DB->db_ROMBuffer, ROMBUFFSIZE/2);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }





