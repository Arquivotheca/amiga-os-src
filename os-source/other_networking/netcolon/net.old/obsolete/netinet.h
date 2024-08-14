#ifndef D_NETINET_H
#define D_NETINET_H

#ifdef DOS_TRUE
#undef DOS_TRUE
#undef DOS_FALSE
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

#define NETBUFSIZE 8192
#define MAXCONNECT    24
#define INETMASK(f)    (1 << (f))

#define DEFPACKETS 10  /* Default number of RPackets to start with */


int OpenIPC(GLOBAL);
int ConnectIPC(GLOBAL) ;

VOID SetTimer (unsigned long, unsigned long, struct IOStdReq *) ;
VOID cleanup () ;

#define PACKET_PERM 0
#define PACKET_FREE 1
#define PACKET_TEMP 2

struct INETRPacket {
   struct RPacket rp;
   struct INETRPacket *next;
   struct INETCON *con;
   long   class;
};

typedef struct INETCON
{
   int fh;                     /* File handle for this connection */
   int usage;                  /* Usage count for this connection */
   struct INETRPacket *opack;  /* Outstanding RPackets for this connection */
} inetcon;

typedef struct
{
   struct sockaddr_in sin;      /* Socket handle                                   */
   int    fhscount;             /* Number of open connections                      */
   inetcon connect[MAXCONNECT]; /* All connections                                 */
   long   fhsmask;              /* Mask of all open connections                    */
   long   fhscurmask;           /* Mask of unprocessed connections                 */
   struct timeval timeout;      /* Timeout interval (defaults to 5 seconds)        */
   int    curcon;               /* Connection of current packet                    */
   int    sock;                 /* Base socket handle for pending connections      */
   long   sockmask;             /* Base socket mask                                */
   struct INETRPacket *tpack;   /* Temporary packet handles to track               */
   struct INETRPacket *ppack;   /* Permanent packet handles to track               */
} * INETGLOBAL;

extern int _alarm_timerbit, socket_sigio;

#endif
