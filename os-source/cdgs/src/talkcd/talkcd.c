
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <hardware/intbits.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <devices/cd.h>
#include "cdprivate.h"
#include "cdgs_hw.h"
#include "TalkCD_rev.h"

#define SysBase (*((struct ExecBase **)4))

extern void IntrProc(void);


LONG GetByte(unsigned char *s) {

LONG          v;
unsigned char c;
int           flag = 0;

    v = 0;

    while(1) {

        c = *s++;

        if      ((c>='a')&&(c<='f')) c-=('a' - 10);
        else if ((c>='A')&&(c<='F')) c-=('A' - 10);
        else if ((c>='0')&&(c<='9')) c-='0';
        else {

            if (!flag) return(-1);
            else       return(v);
            }

        flag = 1;
        v=v*16+c;
        }
    }




struct DrvMsg {

    struct Message Message;
    unsigned char  Data[200];
    };



void ReceiveDriveTask(void) {

struct Library   *DOSBase;
BPTR              FileOut;
struct MsgPort   *RcvPort;
struct DrvMsg    *msg;
int               exit_code = 0;
ULONG             Event;
struct ExecBase  *ExecBase = SysBase;
struct Interrupt *Int;
struct CDDB      *db;

UBYTE             ResponseLen[] = { 1, 3, 3, 3, 3, 3, 16, 21, 3, 1, 3, 1, 1, 1, 1, 1 };
int               PackLen, i;
unsigned char    *p, OutBuff[100];
UBYTE             TaskRXInx;
APTR              SavedInitTask;
UBYTE             SavedInitSignal;

    if (Int = (struct Interrupt *)FindName((struct List *)ExecBase->IntVects[INTB_PORTS].iv_Data, "cd.device")) {

        db = Int->is_Data;

        SavedInitTask   = db->db_InitTask;
        SavedInitSignal = db->db_InitSignal;

        db->db_InitTask   = FindTask(NULL);
        db->db_InitSignal = AllocSignal(-1);
    
        TaskRXInx   = db->db_ComRXInx;

        if (DOSBase = OpenLibrary("dos.library", 0)) {
    
            if (FileOut = Open("CON:0/0/600/78/Receive_From_Drive", MODE_NEWFILE)) {
    
                if (RcvPort = CreateMsgPort()) {
    
                    RcvPort->mp_Node.ln_Name = "DriveRcvPort";
    
                    AddPort(RcvPort);
    
                    while (!exit_code) {
    
                        Event = Wait((1L << RcvPort->mp_SigBit) | (1L << db->db_InitSignal));
    
                        if (Event & (1L << RcvPort->mp_SigBit)) {
    
                            while (msg = (struct DrvMsg *)GetMsg(RcvPort)) {
    
                                if (!msg->Message.mn_Length) exit_code = 1;
    
                                else {
                                    Write(FileOut, "[", 1);
                                    Write(FileOut, &msg->Data[0], msg->Message.mn_Length);
                                    Write(FileOut, "]\n", 3);
                                    Flush(FileOut);
                                    }
    
                                ReplyMsg((struct Message *)msg);
                                }
                            }
    
                        if (Event & (1L << db->db_InitSignal)) {

                            PackLen = ResponseLen[db->db_CDCOMRXPage[TaskRXInx] & 0x0F];

                            for (i=0,p=&OutBuff[0]; i!=PackLen; i++,p+=3) {

                                sprintf(p, "%02x ", db->db_CDCOMRXPage[TaskRXInx++]);
                                }

                            Write(FileOut, &OutBuff[0], PackLen * 3);
                            Write(FileOut, "\n", 2);
                            Flush(FileOut);
                            }
                        }
    
                    RemPort(RcvPort);
    
                    DeleteMsgPort(RcvPort);
                    }
    
                Close(FileOut);
                }
    
            CloseLibrary(DOSBase);
            }

        db->db_InitTask   = SavedInitTask;
        db->db_InitSignal = SavedInitSignal;
        }
    }
    

struct CDDB      *db;

void SendDriveTask(void) {

struct Library *DOSBase;
FILE           *FileIn;
struct MsgPort *RcvPort;
struct DrvMsg   msg;
int             exit_code = 0, i;
unsigned char   Bytes[100], *p;
int             NumBytes = 0, x;
unsigned char   CheckSum;

    if (DOSBase = OpenLibrary("dos.library", 0)) {

        while (!(RcvPort = FindPort("DriveRcvPort"VERSTAG))) Delay(10);

        if (msg.Message.mn_ReplyPort = CreateMsgPort()) {

            if (FileIn = fopen("CON:0/78/600/78/Send_To_Drive", "w+")) {

                for (i=0; i!=30; i++) printf("\n");

                printf("Example: \"05 01 *\" (turns on drive light (* means CheckSum))\n");
                printf("Example: \"07 *\"    (returns ID packet)\n");
                printf("Packet index is calculated automatically.  q = quit\n");

                while (!exit_code) {

                    fscanf(FileIn, "%s", &msg.Data[0]);

                    if ((msg.Data[0] == 'q') || (msg.Data[0] == 'Q')) exit_code = 1;

                    else if (msg.Data[0] == '*') {

                        Bytes[0] &= 0x0F;
                        Bytes[0] |= db->db_PacketIndex;

                        if (!(db->db_PacketIndex += 0x10)) db->db_PacketIndex = 0x10;

                        for (i=CheckSum=0; i!=NumBytes; i++) CheckSum += Bytes[i];

                        Bytes[NumBytes++] = (unsigned char)0xFF - CheckSum;

                        for (i=0,p=&msg.Data[0]; i!=NumBytes; i++,p+=3) {

                            sprintf(p, "%02x ", Bytes[i]);
                            }

                        msg.Message.mn_Length = NumBytes * 3 - 1;
                        PutMsg(RcvPort, &msg);
                        WaitPort(msg.Message.mn_ReplyPort);
                        while (GetMsg(msg.Message.mn_ReplyPort));

                        for (i=0; i!=NumBytes; i++) db->db_CDCOMTXPage[db->db_ComTXInx++] = Bytes[i];
                        *((UBYTE *)(CDHARDWARE + CDCOMTXCMP))    = db->db_ComTXInx;
                        *((ULONG *)(CDHARDWARE + CDINT2ENABLE)) |= INTF_TXDMADONE;

                        NumBytes = 0;
                        }

                    else if ((x = GetByte(&msg.Data[0])) >= 0) Bytes[NumBytes++] = x;

                    else {

                        printf("WARNING: Byte %d eliminated (Not a valid value)\n", NumBytes);
                        }
                    }

                msg.Message.mn_Length = 0;
                PutMsg(RcvPort, &msg);
                WaitPort(msg.Message.mn_ReplyPort);
                while (GetMsg(msg.Message.mn_ReplyPort));

                fclose(FileIn);
                }

            DeleteMsgPort(msg.Message.mn_ReplyPort);
            }

        CloseLibrary(DOSBase);
        }
    }



int main(int argc, char **argv) {

struct Library   *DOSBase;
struct Process   *Process;
struct IOStdReq   Req;
VOID            (*OldCode)();


    if (!OpenDevice("cd.device", 0, &Req, 0)) {

        db = (struct CDDB *)Req.io_Device;

        OldCode = db->db_StatIntr.is_Code;

        db->db_StatIntr.is_Code = IntrProc;

        *((UBYTE *)0xbfe001) |= 0x01;

        if (DOSBase = OpenLibrary("dos.library", 0)) {

            if (Process = CreateNewProcTags(NP_Entry,       ReceiveDriveTask,
                                            NP_Name,        "Receive Drive Process",
                                            NP_StackSize,   4000,
                                            NP_Cli,         
                                            TAG_DONE)) {

                SendDriveTask();
                }

            CloseLibrary(DOSBase);
            }

        db->db_StatIntr.is_Code = OldCode;

        *((UBYTE *)0xbfe001) &= 0xfe;

        CloseDevice(&Req);
        }

    exit(0);
    }



