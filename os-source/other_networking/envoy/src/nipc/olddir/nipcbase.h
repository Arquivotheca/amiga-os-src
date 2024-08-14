
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <sana2.h>
#include <envoy/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>
#include <dos.h>

struct StartMessage
{
    struct Message sm_Msg;
    struct Library *sm_LibBase;
};
struct NBase
{
    struct Library       LibNode;
    struct DOSBase      *nb_DOSBase;
    struct UtilityBase  *nb_UtilityBase;
    struct Library      *nb_SysBase;
    struct Library      *nb_ServicesBase;
    ULONG                nb_SegList;
    struct Library      *nb_IFFParseBase;
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
  struct MsgPort *localport;
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
  struct StartMessage smsg;
  UWORD  TimeCount;             /* A counter to count out 10 .1sec heartbeats */

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
  struct SignalSemaphore ResponseCacheLock;
  struct SignalSemaphore RequestLock;
  struct SignalSemaphore DNSResolverLock;
  struct SignalSemaphore RealmListLock;
  struct MinList ResponseCache;
  struct MinList RequestList;
  struct MinList DNSNames;
  struct MinList RealmList;
  struct UDPConnection  *NameRequestPort;
  struct MsgPort                *ResolverPort;
  struct Hook           *DNSHook;
  ULONG resolversigmask;
  ULONG RealmServer;
  UBYTE RealmServerName[64];
  UBYTE RealmName[64];
  UBYTE LocalHostName[64];
  UWORD ResID;
  UWORD RIPTimer;


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
  struct Library *TimerBase;

 /* ip_in.c */
  struct MinList ProtocolList;

 /* iff.c */
 struct IFFHandle *iff;
 BPTR IFFStream;

 /* memory.c */

 struct MinList BufferCache;
 struct MinList BuffEntryCache;

};
