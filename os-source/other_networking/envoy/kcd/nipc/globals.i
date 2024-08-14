
 include "exec/types.i"
 include "exec/libraries.i"
 include "dos/dos.i"
 include "devices/sana2.i"
 include "devices/timer.i"
 include "exec/io.i"
 include "utility/tagitem.i"
 include "exec/semaphores.i"


MAX_ROUTES equ 32


 STRUCTURE StartMessage,0
  STRUCT sm_SMsg,MN_SIZE
  APTR   sm_LibBase
  LABEL StartMessage_SIZE

 STRUCTURE ipproto,0
 STRUCT         ipproto_link,MLN_SIZE
 APTR           ipproto_Input
 APTR           ipproto_Deinit
 APTR           ipproto_Timeout
 UWORD          ipproto_ProtocolNumber
 LABEL          ipproto_SIZE

 STRUCTURE NIPCMonitorInfo,0
    APTR            nmi_Monitor
    UWORD           nmi_Reserved0
    BYTE            nmi_SigBitNum
    UBYTE           nmi_Reserved1
    ULONG           nmi_IPIn
    ULONG           nmi_IPOut
    ULONG           nmi_IPForwarded
    ULONG           nmi_IPKept
    ULONG           nmi_IPBytesIn
    ULONG           nmi_IPBytesOut
    ULONG           nmi_IPBytesForwarded
    ULONG           nmi_IPBytesKept
    ULONG           nmi_ARPReqSent
    ULONG           nmi_ARPReqReceived
    ULONG           nmi_ARPReplySent
    ULONG           nmi_ARPReplyReceived
    ULONG           nmi_RDPIn
    ULONG           nmi_RDPOut
    ULONG           nmi_RDPBytesIn
    ULONG           nmi_RDPBytesOut
    ULONG           nmi_UDPIn
    ULONG           nmi_UDPOut
    ULONG           nmi_UDPBytesIn
    ULONG           nmi_UDPBytesOut
    ULONG           nmi_AvailChipStart
    ULONG           nmi_AvailFastStart
    LABEL           NIPCMonitorInfo_SIZE

 STRUCTURE NIPCRouteInfo,0
 APTR   nri_Default
 STRUCT nri_Routes,MLH_SIZE*MAX_ROUTES
 STRUCT nri_Lock,SS_SIZE
 LABEL NIPCRouteInfo_SIZE


