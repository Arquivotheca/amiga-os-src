
#include <exec/types.h>
#include <dos/dos.h>
#include <sana2.h>
#include "nipcinternal.h"
#include <appn/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>

extern APTR ACTB;
extern APTR ACFB;

/* amp.c */
extern struct MinList ANMPEntityList;
extern struct SignalSemaphore ANMPELSemaphore;
extern struct MinList HostList;
extern struct SignalSemaphore HLSemaphore;
extern ULONG TRANSSEQUENCE;
extern struct RDPConnection *EntityResolver;

/* rdp.c */
extern struct ipproto RDPProto;        /* Struct that IP knows us by */
extern UWORD RDPSequence;          /* Sequence # */
extern UWORD RDPPorts;          /* Global port numberer */
extern struct MinList RDPConnectionList;       /* Keeps track of connections */
extern struct SignalSemaphore RDPCLSemaphore;  /* Keeps us from walking on others
                                         * toes */

/* s2io.c */
extern ULONG inbuffs, outbuffs;
extern struct MsgPort *replyport;
extern ULONG s2sigmask;
extern struct List DeviceList;
extern long notags[];
extern long copytags[];
extern struct MsgPort *CMDPort;        /* Both of these ports are for sharing
                                 * information with */
extern struct MsgPort *DataPort;       /* inet.library */
extern struct MinList ExchangeList;
extern BOOL Switching;

/* center_custom.c */
extern struct Library *DOSBase;
extern struct Library *UtilityBase;
extern BOOL jumpstart;
extern struct SignalSemaphore openlock;
extern struct Process *nipcprocess;

/* ip_out.c */
extern UWORD IPIdentNumber;

/* arp.c */
extern struct MinList ARPBuffer;
extern struct SignalSemaphore ARPBufferSemaphore;
extern struct MinList ARPList;

/* icmp.c */
extern struct ipproto icmpproto;

/* monitor.c */
extern struct NIPCMonitorInfo MonitorInfo;

/* resolver.c */
extern struct SignalSemaphore HostNamesLock;
extern struct SignalSemaphore RequestLock;
extern struct MinList SimpleNames;
extern struct MinList RequestList;
extern struct UDPConnection *SimpleNamePort;
extern struct MsgPort *ResolverPort;
extern ULONG resolversigmask;
extern UBYTE ResID;

/* route.c */
extern struct NIPCRouteInfo RouteInfo;

/* udp.c */
extern struct ipproto UDPProto;
extern struct MinList UDPConnectionList;
extern struct SignalSemaphore UDPCLSemaphore;
extern UWORD UDPPorts;

/* timer.c */
extern struct MsgPort *timerport;
extern struct timerequest *treq;
extern ULONG timersigmask;

/* ip_in.c */
extern struct MinList ProtocolList;


