#ifndef D_NETPNET_H
#define D_NETPNET_H

#ifdef DOS_TRUE
#undef DOS_TRUE
#undef DOS_FALSE
#endif

#include "sys/socket.h"
#include "sys/un.h"
#include "ipcmacros.h"


#define NETBUFSIZE 1024
#define NETSERVER  65   /* The object number assigned to NET: */
#define NETNAME    "T:IPC_Net_Connection_Socket"
#define Timeout -101
#define DEFAULTDEV "ipc.device"

int OpenIPC(GLOBAL);
int ConnectIPC(GLOBAL) ;

VOID SetTimer (unsigned long, unsigned long, struct IOStdReq *) ;
VOID cleanup () ;

/* Defines for the 'flags' field of IPCGLOBAL */
#define NIPC_BADCON 0x00000001  /* Bad connection, do not IPCMD_DISCONNECT */

typedef struct
{
   struct MsgPort *IPCReplyPort;
   struct IOStdIpc *IPCReq;
   struct IOStdReq *timermsg;
   struct MsgPort *timerport;
   long flags;
   ULONG IPCMask,
         IPCBit;
   ULONG timerbit;
   ULONG wakeupbits;
   int errorcode;
#if 0
   BYTE *b,
        *c;
   int i,
       j,
       flag,
       message_size;
   int message_size;
   unsigned char node_name[10],
                 node_num[2];
#endif
} * IPCGLOBAL;

#endif
