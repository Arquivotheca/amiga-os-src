/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef D_SDNINET_H
#define D_SDNINET_H

#ifdef DOS_TRUE
#undef DOS_TRUE
#undef DOS_FALSE
#endif

#ifdef min
#undef min
#endif

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/inet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <proto/inet.h>
#include "sdn_pragmas.h"

void __stdargs KPrintF(char *,...);

#ifdef BUGR
#undef BUGR
#endif
#ifdef BUG
#undef BUG
#endif
#ifdef BUGP
#undef BUGP
#endif

#define BUGR(s)
#define BUG(a)
#define BUGP(a)

#define NETBUFSIZE 8192
#define MAXCONNECT    24
#define INETMASK(f)    (1 << (f))
#define QUICKSIZE  268

#define DEFPACKETS 10  /* Default number of RPackets to start with */

#define PACKET_PERM 0
#define PACKET_FREE 1
#define PACKET_TEMP 2

struct INETRPacket {
   struct RPacket rp;
   char   quickbuf[QUICKSIZE];
   struct INETRPacket *next;
   struct INETCON *con;
   long   class;
};

typedef struct INETCON
{
   short fh;                    /* File handle for this connection               */
   short usage;                 /* Usage count for this connection               */
   struct INETRPacket *opack;   /* Outstanding RPackets for this connection      */
   void *data;                  /* Generic data pointer for library users        */
   long padd;                   /* Padding to make it 16 byte multiple           */
} inetcon;

typedef struct
{
   struct sockaddr_in sin;      /* Socket handle                                   */
   int    fhscount;             /* Number of open connections                      */
   inetcon connect[MAXCONNECT]; /* All connections                                 */
   long   fhsmask;              /* Mask of all open connections                    */
   long   fhscurmask;           /* Mask of unprocessed connections                 */
   struct timerequest tr;       /* timer                                           */
   int    alarm_bit;		/* alarm signal */
   int    curcon;               /* Connection of current packet                    */
   int    sock;                 /* Base socket handle for pending connections      */
   long   sockmask;             /* Base socket mask                                */
   struct INETRPacket *tpack;   /* Temporary packet handles to track               */
   struct INETRPacket *ppack;   /* Permanent packet handles to track               */
   void *_socks[FD_SETSIZE];    /* Sockets of open file handles                    */
   int    socket_sigio;         /* Signal bit for sockets                          */
   int    socket_sigurg;        /* Signal bit for sockets                          */
   int    hostaddr;             /* Default host address                            */
   struct Library *tg_SysBase;
   struct Library *tg_InetBase;
   struct DrLibrary *DriverBase;/* Pointer to driver base                          */
} * INETGLOBAL;

struct DrLibrary {
        struct       Library ml_Lib;
        ULONG        ml_SegList;
        ULONG        ml_Flags;
        APTR         ml_ExecBase;        /* pointer to exec base */
        LONG         ml_Data;                /* Global data */
        };

#define InetBase   (tg->tg_InetBase)
#define SysBase    (tg->tg_SysBase)
#define ABSEXECBASE (*(struct Library **)4)

#define LIBENT __asm

/* Prototypes for functions defined in sdninetutil.c */
int LIBENT INETSDNSerInit      (register __a0 INETGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT INETSDNSerTerm      (register __a0 INETGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT INETSDNHanInit      (register __a0 INETGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT INETSDNHanTerm      (register __a0 INETGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               );

struct RPacket *
    LIBENT INETSDNAllocRPacket (register __a0 INETGLOBAL tg,
                                register __a1 APTR con,
                                register __d0 int len,
                                register __a6 struct DrLibrary *DriverBase
                               );

struct RPacket *
    LIBENT INETSDNDupRPacket   (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void LIBENT INETSDNFreeRPacket (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void *LIBENT INETSDNGetConData (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

void LIBENT INETSDNSetConData  (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a2 void *data,
                                register __a6 struct DrLibrary *DriverBase
                               );



/* Prototypes for functions defined in sdninetnode.c */
int LIBENT INETSDNInitNode     (register __a0 INETGLOBAL tg,
                                register __a1 char *name,
                                register __a1 APTR *con,
                                register __a6 struct DrLibrary *DriverBase
                               );
void LIBENT INETSDNTermNode    (register __a0 INETGLOBAL tg,
                                register __a1 APTR con,
                                register __a6 struct DrLibrary *DriverBase
                               );


/* Prototypes for functions defined in sdninetget.c */
int LIBENT INETSDNAccept       (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

int LIBENT INETSDNReceive      (register __a0 INETGLOBAL tg,
                                register __d0 ULONG mask,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

/* Prototypes for functions defined in sdninetutil.c */
int LIBENT INETSDNReply        (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );
int LIBENT INETSDNSend         (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               );

/* Prototypes for functions defined in readn.c */
int readn (INETGLOBAL tg,
           int fd,
           char *ptr,
           int nbytes
          );

int writen(INETGLOBAL tg,
           int fd,
           char *ptr,
           int nbytes
          );

int set_sockopts(INETGLOBAL tg,
                 int fd
                );

int ibind(INETGLOBAL tg,
          int fd,
          void *name,
          int len
         );

int iconnect(INETGLOBAL tg,
             int fd,
             void *name,
             int len
            );

int iaccept(INETGLOBAL tg,
            int fd,
            void *name,
            int *lenp
            );

int isocket(INETGLOBAL tg
           );

void iclose(INETGLOBAL tg,
            int fd
           );

int INETprivate(void);


/* Prototypes for functions defined in inetsup.c */
#endif /* D_SDNINET_H */
