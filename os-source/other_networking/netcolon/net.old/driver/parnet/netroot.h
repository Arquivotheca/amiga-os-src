#ifndef D_NETPNET_H
#define D_NETPNET_H

#include "parnet.h"

#define NETBUFSIZE 8192

typedef struct IORequest IOR;

/* Pnet private routines */
int OpenPnet    U_ARGS((NGLOBAL, struct NPNGLOBAL *));
int ClosePnet   U_ARGS((NGLOBAL, struct NPNGLOBAL *));

struct NPNGLOBAL
{
   int MyAddr,
       DestAddr,
       Broke;
   struct MsgPort *IoPort;
   IOParReq Pioc;
   IOParReq Pior[2];
   IOParReq Piow;
   char     PiorIP[2];
   char     RxBuf[2][NETBUFSIZE+RPSIZEN];
};

#endif
