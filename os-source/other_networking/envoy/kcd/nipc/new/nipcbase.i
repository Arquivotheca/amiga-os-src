
**
* $Id: nipcbase.i,v 1.16 93/04/05 11:19:03 gregm Exp $
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

   IFND  UTILITY_HOOKS_I
   INCLUDE  "utility/hooks.i"
   ENDC   ; UTILITY_HOOKS_I

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

 STRUCTURE NIPCBase,LIB_SIZE
 WORD nb_Pad0
 APTR nb_DOSBase
 APTR nb_UtilityBase
 LONG nb_Kludge0        ; Keep old versions of services.library from
 LONG nb_Kludge1        ; killing the machine.
 APTR nb_SysBase
 APTR nb_ServicesBase
 APTR nb_SegList
 APTR nb_IFFParseBase;

* amp.c *
 STRUCT ANMPEntityList,MLH_SIZE
 STRUCT ANMPELSemaphore,SS_SIZE
 WORD   Pad1
 STRUCT HostList,MLH_SIZE
 STRUCT HLSemaphore,SS_SIZE
 WORD   Pad2
 ULONG  TRANSSEQUENCE;
 APTR   EntityResolver
 STRUCT PingList,MLH_SIZE
 STRUCT PingListSemaphore,SS_SIZE
 WORD   Pad3

* rdp.c *
 STRUCT RDPProto,ipproto_SIZE
 UWORD Pad4
 UWORD RDPSequence
 UWORD RDPPorts
 STRUCT RDPConnectionList,MLH_SIZE
 STRUCT RDPCLSemaphore,SS_SIZE

 WORD   Pad5

* s2io.c *
 ULONG inbuffs
 ULONG outbuffs
 APTR replyport
 APTR localport
 ULONG s2sigmask
 STRUCT DeviceList,LH_SIZE
 BOOL   Switching
 STRUCT notags,2*4
 STRUCT copytags,6*4
 APTR   CMDPort
 APTR   DataPort
 STRUCT ExchangeList,MLH_SIZE
 ULONG  RandomSeed

* center_custom.c *
 STRUCT openlock,SS_SIZE
 BOOL jumpstart
 APTR   nipcprocess
 ULONG shutdownbit
 STRUCT smsg,StartMessage_SIZE
 UWORD  TimeCount

* ip_out.c *
 UWORD IPIdentNumber

* arp.c *
 STRUCT ARPBuffer,MLH_SIZE
 STRUCT ARPBufferSemaphore,SS_SIZE
 WORD   Pad51
 STRUCT ARPList,MLH_SIZE

* icmp.c *
 STRUCT icmpproto,ipproto_SIZE
 UWORD  Pad6

* monitor.c *
 STRUCT NIPCMonInfo,NIPCMonitorInfo_SIZE

* resolver.c *
 STRUCT ResponseCacheLock,SS_SIZE
 UWORD ResID
 STRUCT RequestLock,SS_SIZE
 UWORD RIPTimer
 STRUCT DNSResolverLock,SS_SIZE
 WORD   Pad7
 STRUCT RealmListLock,SS_SIZE
 WORD   Pad8
 STRUCT ResponseCache,MLH_SIZE
 STRUCT RequestList,MLH_SIZE
 STRUCT DNSNames,MLH_SIZE
 STRUCT RealmList,MLH_SIZE
 APTR NameRequestPort
 APTR ResolverPort
 APTR DNSHook
 APTR NameResponsePort
 ULONG resolversigmask
 ULONG RealmServer
 STRUCT RealmServerName,64
 STRUCT RealmName,64
 STRUCT LocalHostName,64
 STRUCT OwnerName,32
 ULONG  IsRealmServer

* route.c *
 STRUCT RouteInfo,NIPCRouteInfo_SIZE

* udp.c *
 STRUCT UDPProto,ipproto_SIZE
 UWORD UDPPorts
 STRUCT UDPConnectionList,MLH_SIZE
 STRUCT UDPCLSemaphore,SS_SIZE
 WORD   Pad9

* timer.c *
 APTR timerport
 APTR treq
 ULONG timersigmask
 APTR TimerDevice
 APTR TimerUnit

* ip_in.c *
 STRUCT ProtocolList,MLH_SIZE

* iff.c *
 APTR iffHandle
 BPTR iffStream

* memory.c *
 STRUCT BufferCache,MLH_SIZE
 STRUCT BuffEntryCache,MLH_SIZE

 UWORD TimerSeed

* more amp.c *
 STRUCT WakeFind,MLH_SIZE
 STRUCT WakeLock,SS_SIZE

* memory pool *
 APTR   MemoryPool
 STRUCT PoolLock,SS_SIZE

* packet hook *
 STRUCT PacketHook,h_SIZEOF

 LABEL nb_SIZE



   ENDC

