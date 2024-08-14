/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename: SDNDNet.h $ 
**      $Author: J_Toebes $
**      $Revision: 1.3 $
**      $Date: 91/01/06 20:56:47 $
**      $Log:	SDNDNet.h,v $
**/

#ifndef D_SDNDNET_H
#define D_SDNDNET_H

#ifdef DOS_TRUE
#undef DOS_TRUE
#undef DOS_FALSE
#endif

#ifdef min
#undef min
#endif

#include "sdn_pragmas.h"

#include "dnet.h"

#define STDFNLEN  0 

#define PORT_FHANDLER 9492

#define DNETTIMEOUT 2  /* tenths of a second */

#define NETBUFSIZE 8192
#define MAXPORT       32
#define MAXNAME       40

#define DEFPACKETS 10  /* Default number of RPackets to start with */

#define PACKET_PERM 0
#define PACKET_FREE 1
#define PACKET_TEMP 2

struct DNETRPacket {
   struct Message ms;
   struct RPacket rp;
   struct DNETRPacket *next;
   long   class;
};

#define RP2LRP(rp) ((struct DNETRPacket *)(((struct Message *)rp)-1))
#define LRP2RP(lrp) (&lrp->rp)

#define CSTATE_CLOSED  0
#define CSTATE_CONNECT 1
#define CSTATE_ACTIVE  2

typedef struct
{
   int   signalbit;              /* Signal bit for communications                */
   char  portname[MAXNAME+1];    /* Name of global port for communications       */
   char  nodename[MAXNAME+1];    /* Name of local reference node                 */
   int   state;                  /* State flag to indicate connection            */
   void *data;                   /* Generic data pointer for library users       */
   struct MsgPort *sendport;     /* Message port for sending messages            */
   struct MsgPort *port;         /* Message port for receiving messages          */
   struct MsgPort *replyport;    /* Message port for replies.                    */
   struct DNETRPacket *tpack;   /* Temporary packet handles to track            */
   struct DNETRPacket *ppack;   /* Permanent packet handles to track            */
   struct Library *tg_SysBase;   /* Pointer to good old sysbase                  */
   struct DrLibrary *DriverBase; /* Pointer to driver base                       */
} * DNETGLOBAL;

struct DrLibrary {
        struct       Library ml_Lib;
        ULONG        ml_SegList;
        ULONG        ml_Flags;
        APTR         ml_ExecBase;        /* pointer to exec base */
        LONG         ml_Data;                /* Global data */
        };

#define SysBase    (tg->tg_SysBase)
#define ABSEXECBASE (*(struct Library **)4)

#define LIBENT __asm

/* Prototypes for functions defined in sdnlocalutil.c */
int LIBENT DNETSDNSerInit      (register __a0 DNETGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT DNETSDNSerTerm      (register __a0 DNETGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT DNETSDNHanInit      (register __a0 DNETGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT DNETSDNHanTerm      (register __a0 DNETGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               );

struct RPacket *
    LIBENT DNETSDNAllocRPacket (register __a0 DNETGLOBAL tg,
                                register __a1 APTR con,
                                register __d0 int len,
                                register __a6 struct DrLibrary *DriverBase
                               );

struct RPacket *
    LIBENT DNETSDNDupRPacket   (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void LIBENT DNETSDNFreeRPacket (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void *LIBENT DNETSDNGetConData (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void LIBENT DNETSDNSetConData  (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a2 void *data,
                                register __a6 struct DrLibrary *DriverBase
                               );



/* Prototypes for functions defined in sdnlocalnode.c */
int LIBENT DNETSDNInitNode     (register __a0 DNETGLOBAL tg,
                                register __a1 char *name,
                                register __a1 APTR *con,
                                register __a6 struct DrLibrary *DriverBase
                               );
void LIBENT DNETSDNTermNode    (register __a0 DNETGLOBAL tg,
                                register __a1 APTR con,
                                register __a6 struct DrLibrary *DriverBase
                               );


/* Prototypes for functions defined in sdnlocalget.c */
int LIBENT DNETSDNAccept       (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

int LIBENT DNETSDNReceive      (register __a0 DNETGLOBAL tg,
                                register __d0 ULONG mask,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

/* Prototypes for functions defined in sdnlocalutil.c */
int LIBENT DNETSDNReply        (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT DNETSDNSend         (register __a0 DNETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

DNETGLOBAL DNetInit          (ULONG *maskp,
                                char *data
                               );
int DNETprivate(void);

void __stdargs kprintf(char *, ...);
int __stdargs kgetc(void);
#endif /* D_SDNDNET_H */
