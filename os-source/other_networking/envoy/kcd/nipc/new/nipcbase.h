
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <devices/sana2.h>
#include <envoy/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>
#include <dos.h>

struct StartMessage
{
    struct Message sm_Msg;
    struct Library *sm_LibBase;
};
struct NBase
{
    struct Library       LibNode;
    WORD                 nb_Pad0;
    struct Library      *nb_DOSBase;
    struct Library      *nb_UtilityBase;
    LONG                 nb_Kludge0;            /* Keep old versions of services.library from */
    LONG                 nb_Kludge1;            /* killing the machine */
    struct Library      *nb_SysBase;
    struct Library      *nb_ServicesBase;
    ULONG                nb_SegList;
    struct Library      *nb_IFFParseBase;
 /* amp.c */
  struct MinList ANMPEntityList;
  struct SignalSemaphore ANMPELSemaphore;
  WORD   Pad1;
  struct MinList HostList;
  struct SignalSemaphore HLSemaphore;
  WORD   Pad2;
  ULONG TRANSSEQUENCE;
  struct RDPConnection *EntityResolver;
  struct MinList    PingList;
  struct SignalSemaphore  PingListSemaphore;
  WORD   Pad3;

 /* rdp.c */
  struct ipproto RDPProto;        /* Struct that IP knows us by */
  UWORD Pad4;
  UWORD RDPSequence;          /* Sequence # */
  UWORD RDPPorts;          /* Global port numberer */
  struct MinList RDPConnectionList;       /* Keeps track of connections */
  struct SignalSemaphore RDPCLSemaphore;  /* Keeps us from walking on others
                                          * toes */
  WORD   Pad5;
 /* s2io.c */
  ULONG inbuffs, outbuffs;
  struct MsgPort *replyport;
  struct MsgPort *localport;
  ULONG s2sigmask;
  struct List DeviceList;
  BOOL   Switching;
  long notags[2];
  long copytags[6];
  struct MsgPort *CMDPort;        /* Both of these ports are for sharing
                                  * information with */
  struct MsgPort *DataPort;       /* inet.library */
  struct MinList ExchangeList;
  ULONG RandomSeed;

 /* center_custom.c */
  struct SignalSemaphore openlock;
  BOOL jumpstart;
  struct Process *nipcprocess;
  ULONG shutdownbit;
  struct StartMessage smsg;
  UWORD  TimeCount;             /* A counter to count out 10 .1sec heartbeats */

 /* ip_out.c */
  UWORD IPIdentNumber;

 /* arp.c */
  struct MinList ARPBuffer;
  struct SignalSemaphore ARPBufferSemaphore;
  WORD   Pad51;
  struct MinList ARPList;

 /* icmp.c */
  struct ipproto icmpproto;
  UWORD Pad6;

 /* monitor.c */
  struct NIPCMonitorInfo MonitorInfo;

 /* resolver.c */
  struct SignalSemaphore ResponseCacheLock;
  UWORD ResID;
  struct SignalSemaphore RequestLock;
  UWORD RIPTimer;
  struct SignalSemaphore DNSResolverLock;
  WORD  Pad7;
  struct SignalSemaphore RealmListLock;
  WORD  Pad8;
  struct MinList ResponseCache;
  struct MinList RequestList;
  struct MinList DNSNames;
  struct MinList RealmList;
  struct UDPConnection  *NameRequestPort;
  struct MsgPort                *ResolverPort;
  struct Hook           *DNSHook;
  struct MsgPort *NameResponsePort;
  ULONG resolversigmask;
  ULONG RealmServer;
  UBYTE RealmServerName[64];
  UBYTE RealmName[64];
  UBYTE LocalHostName[64];
  UBYTE OwnerName[32];
  ULONG IsRealmServer;

 /* route.c */
  struct NIPCRouteInfo RouteInfo;

 /* udp.c */
  struct ipproto UDPProto;
  UWORD UDPPorts;
  struct MinList UDPConnectionList;
  struct SignalSemaphore UDPCLSemaphore;
  WORD   Pad9;

 /* timer.c */
  struct MsgPort *timerport;
  struct timerequest *treq;
  ULONG timersigmask;
  struct Library *TimerDevice;
  struct Unit *TimerUnit;

 /* ip_in.c */
  struct MinList ProtocolList;

 /* iff.c */
 struct IFFHandle *iff;
 BPTR IFFStream;

 /* memory.c */

 struct MinList BufferCache;
 struct MinList BuffEntryCache;

 UWORD TimerSeed;

 /* More for amp.c */
 struct MinList WakeFind;
 struct SignalSemaphore WakeLock;

 /* Our memory pool */
 APTR   MemoryPool;
 struct SignalSemaphore PoolLock;

 /* Our New PacketHook */
 struct Hook PacketHook;

};
