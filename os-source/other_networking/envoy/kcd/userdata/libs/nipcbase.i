
**
* $Id: nipcbase.i,v 1.6 92/04/30 13:19:09 gregm Exp Locker: kcd $
**

   IFND  SAMPLE_BASE_I
SAMPLE_BASE_I SET 1


   IFND  EXEC_TYPES_I
   INCLUDE  "exec/types.i"
   ENDC   ; EXEC_TYPES_I

   IFND  EXEC_LISTS_I
   INCLUDE  "exec/lists.i"
   ENDC   ; EXEC_LISTS_I

   IFND  EXEC_LIBRARIES_I
   INCLUDE  "exec/libraries.i"
   ENDC   ; EXEC_LIBRARIES_I

   include "globals.i"


;-----------------------------------------------------------------------
;
; library data structures
;
;-----------------------------------------------------------------------

;  Note that the library base begins with a library node

NIPCNAME   MACRO
      DC.B   'nipc.library',0
      ENDM

 STRUCTURE NIPCBase,0
 STRUCT nb_Link,LIB_SIZE
 APTR nb_DOSBase
 APTR nb_UtilityBase
 APTR nb_SysBase
 APTR nb_ServicesBase
 APTR nb_SegList
 APTR nb_IFFParseBase;

* amp.c *
 STRUCT ANMPEntityList,MLH_SIZE
 STRUCT ANMPELSemaphore,SS_SIZE
 STRUCT HostList,MLH_SIZE
 STRUCT HLSemaphore,SS_SIZE
 ULONG  TRANSSEQUENCE;
 APTR   EntityResolver

* rdp.c *
 STRUCT RDPProto,ipproto_SIZE
 UWORD RDPSequence
 UWORD RDPPorts
 STRUCT RDPConnectionList,MLH_SIZE
 STRUCT RDPCLSemaphore,SS_SIZE

* s2io.c *
 ULONG inbuffs
 ULONG outbuffs
 APTR replyport
 APTR localport
 ULONG s2sigmask
 STRUCT DeviceList,LH_SIZE
 STRUCT notags,2*4
 STRUCT copytags,6*4
 APTR   CMDPort
 APTR   DataPort
 STRUCT ExchangeList,MLH_SIZE
 BOOL Switching

* center_custom.c *
 BOOL jumpstart
 STRUCT openlock,SS_SIZE
 APTR   nipcprocess
 ULONG shutdownbit
 STRUCT smsg,StartMessage_SIZE
 UWORD  TimeCount

* ip_out.c *
 UWORD IPIdentNumber

* arp.c *
 STRUCT ARPBuffer,MLH_SIZE
 STRUCT ARPBufferSemaphore,SS_SIZE
 STRUCT ARPList,MLH_SIZE

* icmp.c *
 STRUCT icmpproto,ipproto_SIZE

* monitor.c *
 STRUCT MonitorInfo,NIPCMonitorInfo_SIZE

* resolver.c *
 STRUCT ResponseCacheLock,SS_SIZE
 STRUCT RequestLock,SS_SIZE
 STRUCT DNSResolverLock,SS_SIZE
 STRUCT RealmListLock,SS_SIZE
 STRUCT ResponseCache,MLH_SIZE
 STRUCT RequestList,MLH_SIZE
 STRUCT DNSNames,MLH_SIZE
 STRUCT RealmList,MLH_SIZE
 APTR NameRequestPort
 APTR ResolverPort
 APTR DNSHook
 ULONG resolversigmask
 ULONG RealmServer
 STRUCT RealmServerName,64
 STRUCT RealmName,64
 STRUCT LocalHostName,64
 UWORD ResID
 UWORD RIPTimer

* route.c *
 STRUCT RouteInfo,NIPCRouteInfo_SIZE

* udp.c *
 STRUCT UDPProto,ipproto_SIZE
 STRUCT UDPConnectionList,MLH_SIZE
 STRUCT UDPCLSemaphore,SS_SIZE
 UWORD UDPPorts

* timer.c *
 APTR timerport
 APTR treq
 ULONG timersigmask
 APTR TimerBase

* ip_in.c *
 STRUCT ProtocolList,MLH_SIZE

* iff.c *
 APTR IFFHandle
 BPTR IFFStream


 LABEL nb_SIZE



   ENDC

