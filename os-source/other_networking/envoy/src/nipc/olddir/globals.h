
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <sana2.h>
#include "nipcinternal.h"
#include <envoy/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>
#include <dos.h>


struct Globals
{

/* amp.c */
 struct MinList ANMPEntityList;
 struct SignalSemaphore ANMPELSemaphore;
 struct MinList HostList;
 struct SignalSemaphore HLSemaphore;
 ULONG TRANSSEQUENCE;
 struct RDPConnection *EntityResolver;

/* rdp.c */
 struct ipproto RDPProto;        /* Struct that IP knows us by */
 UWORD RDPSequence;          /* Sequence # */
 UWORD RDPPorts;          /* Global port numberer */
 struct MinList RDPConnectionList;       /* Keeps track of connections */
 struct SignalSemaphore RDPCLSemaphore;  /* Keeps us from walking on others
                                         * toes */

/* s2io.c */
 ULONG inbuffs, outbuffs;
 struct MsgPort *replyport;
 ULONG s2sigmask;
 struct List DeviceList;
 long notags[2];
 long copytags[6];
 struct MsgPort *CMDPort;        /* Both of these ports are for sharing
                                 * information with */
 struct MsgPort *DataPort;       /* inet.library */
 struct MinList ExchangeList;
 BOOL Switching;

/* center_custom.c */
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
}


