
#include <exec/types.h>
#include <dos/dos.h>
#include <sana2.h>
#include "nipcinternal.h"
#include "externs.h"

#include <appn/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>

extern APTR ACTB;
extern APTR ACFB;

/* amp.c */
struct MinList ANMPEntityList;
struct SignalSemaphore ANMPELSemaphore;
struct MinList HostList;
struct SignalSemaphore HLSemaphore;
ULONG TRANSSEQUENCE = 0;
struct RDPConnection *EntityResolver;

/* rdp.c */
struct ipproto RDPProto;        /* Struct that IP knows us by */
UWORD RDPSequence = 0;          /* Sequence # */
UWORD RDPPorts = 1024;          /* Global port numberer */
struct MinList RDPConnectionList;       /* Keeps track of connections */
struct SignalSemaphore RDPCLSemaphore;  /* Keeps us from walking on others
                                         * toes */

/* s2io.c */
ULONG inbuffs = 0, outbuffs = 0;
struct MsgPort *replyport;
ULONG s2sigmask;
struct List DeviceList;
long notags[2] = {TAG_DONE, 0};
long copytags[6] = {S2_CopyToBuff, &ACTB, S2_CopyFromBuff, &ACFB, TAG_DONE, 0};
struct MsgPort *CMDPort;        /* Both of these ports are for sharing
                                 * information with */
struct MsgPort *DataPort;       /* inet.library */
struct MinList ExchangeList;
BOOL Switching = FALSE;

/* center_custom.c */
struct Library *DOSBase;
struct Library *UtilityBase;
BOOL jumpstart;
struct SignalSemaphore openlock;
struct Process *nipcprocess;
ULONG shutdownbit;

/* ip_out.c */
UWORD IPIdentNumber;

/* arp.c */
struct MinList ARPBuffer;
struct SignalSemaphore ARPBufferSemaphore;
struct MinList ARPList;

/* icmp.c */
struct ipproto icmpproto;

/* monitor.c */
struct NIPCMonitorInfo MonitorInfo;

/* resolver.c */
struct SignalSemaphore HostNamesLock;
struct SignalSemaphore RequestLock;
struct SignalSemaphore DNSNamesLock;
struct MinList SimpleNames;
struct MinList RequestList;
struct MinList DNSNames;
struct UDPConnection *SimpleNamePort;
struct MsgPort *ResolverPort;
struct MsgPort *DNSPort;
ULONG resolversigmask;
UBYTE ResID;

/* route.c */
struct NIPCRouteInfo RouteInfo;

/* udp.c */
struct ipproto UDPProto;
struct MinList UDPConnectionList;
struct SignalSemaphore UDPCLSemaphore;
UWORD UDPPorts;

/* timer.c */
struct MsgPort *timerport;
struct timerequest *treq;
ULONG timersigmask;

/* ip_in.c */
struct MinList ProtocolList;


