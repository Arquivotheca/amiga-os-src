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
**      $Filename: SDNLocal.h $ 
**      $Author: J_Toebes $
**      $Revision: 1.3 $
**      $Date: 91/01/06 20:56:47 $
**      $Log:	SDNLocal.h,v $
 * Revision 1.3  91/01/06  20:56:47  J_Toebes
 * Correct strange bug where packets were being received by the local
 * sender.  This was due to overloading the Port field in the global structure.
 * 
**/

#ifndef D_SDNLOCAL_H
#define D_SDNLOCAL_H

#ifdef DOS_TRUE
#undef DOS_TRUE
#undef DOS_FALSE
#endif

#ifdef min
#undef min
#endif

#include "sdn_pragmas.h"

#define NETBUFSIZE 8192
#define MAXPORT       32
#define MAXNAME       40

#define DEFPACKETS 10  /* Default number of RPackets to start with */

#define PACKET_PERM 0
#define PACKET_FREE 1
#define PACKET_TEMP 2

struct LOCALRPacket {
   struct Message ms;
   struct RPacket rp;
   struct LOCALRPacket *next;
   long   class;
};

#define RP2LRP(rp) ((struct LOCALRPacket *)(((struct Message *)rp)-1))
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
   struct LOCALRPacket *tpack;   /* Temporary packet handles to track            */
   struct LOCALRPacket *ppack;   /* Permanent packet handles to track            */
   struct Library *tg_SysBase;   /* Pointer to good old sysbase                  */
   struct DrLibrary *DriverBase; /* Pointer to driver base                       */
} * LOCALGLOBAL;

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
int LIBENT LOCALSDNSerInit      (register __a0 LOCALGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT LOCALSDNSerTerm      (register __a0 LOCALGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT LOCALSDNHanInit      (register __a0 LOCALGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT LOCALSDNHanTerm      (register __a0 LOCALGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               );

struct RPacket *
    LIBENT LOCALSDNAllocRPacket (register __a0 LOCALGLOBAL tg,
                                register __a1 APTR con,
                                register __d0 int len,
                                register __a6 struct DrLibrary *DriverBase
                               );

struct RPacket *
    LIBENT LOCALSDNDupRPacket   (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void LIBENT LOCALSDNFreeRPacket (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void *LIBENT LOCALSDNGetConData (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void LIBENT LOCALSDNSetConData  (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a2 void *data,
                                register __a6 struct DrLibrary *DriverBase
                               );



/* Prototypes for functions defined in sdnlocalnode.c */
int LIBENT LOCALSDNInitNode     (register __a0 LOCALGLOBAL tg,
                                register __a1 char *name,
                                register __a1 APTR *con,
                                register __a6 struct DrLibrary *DriverBase
                               );
void LIBENT LOCALSDNTermNode    (register __a0 LOCALGLOBAL tg,
                                register __a1 APTR con,
                                register __a6 struct DrLibrary *DriverBase
                               );


/* Prototypes for functions defined in sdnlocalget.c */
int LIBENT LOCALSDNAccept       (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

int LIBENT LOCALSDNReceive      (register __a0 LOCALGLOBAL tg,
                                register __d0 ULONG mask,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

/* Prototypes for functions defined in sdnlocalutil.c */
int LIBENT LOCALSDNReply        (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT LOCALSDNSend         (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

LOCALGLOBAL LocalInit          (ULONG *maskp,
                                char *data
                               );
int LOCALprivate(void);

void __stdargs kprintf(char *, ...);
int __stdargs kgetc(void);
#endif /* D_SDNLOCAL_H */
