#ifndef NIPC_NIPCINTERNAL_H
#define NIPC_NIPCINTERNAL_H
/*
**      $Id: nipcinternal.h,v 1.14 92/06/09 15:09:34 gregm Exp Locker: gregm $
**
**      Internal structures and #defines used by nipc.library.
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

/*------------------------------------------------------------------------*/

#ifndef ENVOY_NIPC_H
#include "nipc.h"
#endif

#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef IFF_IFFPARSE_H
#include <libraries/iffparse.h>
#endif

/*------------------------------------------------------------------------*/

/* Misc type declarations */

typedef unsigned int u_int;


/*------------------------------------------------------------------------*/

/* Memory Management */

struct BuffEntry
{
    struct MinNode  be_link;
    UWORD           be_offset;
    UWORD           be_length;
    UWORD           be_physicallength;
    UBYTE          *be_data;
};

struct Buffer
{
    struct MinNode  buff_link;
    struct MinList  buff_list;
    UWORD           buff_timeout;
    UWORD           buff_refcnt;
};

/*------------------------------------------------------------------------*/

/* Defined RDP Ports */

#define RESOLVERPORT 1        /* RDP port # for the entity name resolver */

/*------------------------------------------------------------------------*/

/* Entity Structure */

struct Entity
{
    struct MsgPort          entity_Port;        /* Message port */
    ULONG                   entity_Flags;       /* See below */
    struct SignalSemaphore  entity_linksema;    /* link semaphore */
    struct MinList          entity_linklist;    /* All link entities to this entity */
    struct SignalSemaphore  entity_transsema;   /* semaphore for below */
    struct MinList          entity_translist;   /* transactions that've been sent from this entity */
    struct Entity          *entity_owner;       /* Filled in only if this is a link */
    struct RDPConnection   *entity_connection;  /* The connection for this entity */
    UBYTE                  *entity_linkname;    /* hostname on which the entity that this link represents actually exists */
    UWORD                   entity_LinkCount;   /* shows the number of people who're attached to this.  Must be 0 to delete it. */
    struct MinList          entity_Partials;    /* Pieces of transmitted transactions, while they're assembled */
};

/* Entity Flags */

#define ENTF_PUBLIC         1   /* Set if this is a public port */
#define ENTB_PUBLIC         0
#define ENTF_ALLOCSIGNAL    2   /* Set if CreateEntity allocated a signal bit for this port */
#define ENTB_ALLOCSIGNAL    1
#define ENTF_LINK           4   /* Set if this is a link to an external entity, rather than a
                                 * local one */
#define ENTB_LINK           2   /* (From FindEntity rather than CreateEntity) */
#define ENTF_DELAYEDDELETE  8
#define ENTB_DELAYEDDELETE  3


/*------------------------------------------------------------------------*/

/* HostName and Entity Resolver Structures */

struct nameentry
{
    struct MinNode  ne_link;
    UBYTE           ne_name[80];
    ULONG           ne_Address;
};

struct NameRequest
{
    UBYTE   nr_Name[80];        /* Name to find */
    UBYTE   nr_FromName[80];    /* Name of entity doing the finding .. */
    UBYTE   nr_FromHost[80];    /* Name of the machine on which the from entity exists */
};

struct NameReply
{
    UWORD   nr_PortNumber;
};


struct RequestPacket
{
    UBYTE   reqpack_Command;    /* From trans_Command */
    UBYTE   reqpack_Flags;
    UWORD   reqpack_Segment;    /* Segment number */
    ULONG   reqpack_Sequence;
    ULONG   reqpack_ResponseDataSize;
    ULONG   reqpack_DataLength;
    /* ... followed by DataLength bytes of data */
    };

struct NameInfo
{
    struct Message           ni_Msg;
    UWORD                    ni_ID;
    UWORD                    ni_TTL;
    UWORD                    ni_Error;
    UWORD                    ni_HeaderLength;
    struct Hook             *ni_Hook;
    struct NameHeader   *ni_Header;
};

struct NameCache
{
    struct MinNode  nc_Node;
    ULONG           nc_IPNum;
    UBYTE           nc_HostName[64];
};

struct Realm
{
    struct MinNode  realm_Node;
    ULONG           realm_Server;
    ULONG           realm_Flags;
    ULONG           realm_NetworkIP;
    ULONG           realm_SubnetMask;
    ULONG           realm_Broadcast;
    UBYTE           realm_Name[64];
};

struct RIPHeader
{
    UBYTE   rip_Command;
    UBYTE   rip_Version;
    UWORD   rip_Unused0;
    UWORD   rip_AddFam;
    UWORD   rip_Unused1;
    ULONG   rip_DestIP;
    ULONG   rip_Unused2;
    ULONG   rip_Unused3;
    ULONG   rip_Metric;
};

/*------------------------------------------------------------------------*/

/* IFF Configuration File */

#define ID_PREF  MAKE_ID('P','R','E','F')
#define ID_NDEV  MAKE_ID('N','D','E','V')
#define ID_NRRM  MAKE_ID('N','R','R','M')
#define ID_NLRM  MAKE_ID('N','L','R','M')
#define ID_NIRT  MAKE_ID('N','I','R','T')
#define ID_HOST  MAKE_ID('H','O','S','T')

struct NIPCPrefs
{
    ULONG   np_RealmServer;
    ULONG   np_RealmName;
    UBYTE   np_UserName[16];
    UBYTE   np_AmigaName[64];
    ULONG   np_RealmServerName[64];
};

struct NIPCDevicePrefs
{
    ULONG   ndp_IPType;
    ULONG   ndp_ARPType;
    ULONG   ndp_Unit;
    UBYTE   ndp_HardAddress[16];
    UBYTE   ndp_HardString[40];
    ULONG   ndp_IPAddress;
    ULONG   ndp_IPSubnet;
    UBYTE   ndp_Flags;
    UBYTE   ndp_DevPathName[256];
};

struct NIPCIFFDevice
{
    struct Node            nd_Node;
    struct NIPCDevicePrefs nd_Prefs;
};

struct NIPCRoutePrefs
{
    ULONG   nrp_DestNetwork;
    ULONG   nrp_Gateway;
    UWORD   nrp_Hops;
};

struct NIPCIFFRoute
{
    struct Node             nr_Node;
    UBYTE                   nr_String[64];
    struct NIPCRoutePrefs   nr_Prefs;
};

struct NIPCRealmPrefs
{
    UBYTE   nzp_RealmName[64];
    ULONG   nzp_RealmAddr;
};

struct NIPCIFFRealm
{
    struct Node             nz_Node;
    UBYTE                   nz_String[128];
    struct NIPCRealmPrefs   nz_Prefs;
};

struct NIPCIFFHostPrefs
{
    UBYTE   nhp_HostName[64];
    UBYTE   nhp_RealmName[64];
    ULONG   nhp_RealmServAddr;
};

#define NDPFLAGB_SUBNET   0
#define NDPFLAGB_ONLINE   1
#define NDPFLAGB_USEARP   2
#define NDPFLAGB_IPTYPE   3
#define NDPFLAGB_ARPTYPE  4
#define NDPFLAGB_HARDADDR 5

#define NDPFLAGF_SUBNET   (1L << NDPFLAGB_SUBNET)
#define NDPFLAGF_ONLINE   (1L << NDPFLAGB_ONLINE)
#define NDPFLAGF_USEARP   (1L << NDPFLAGB_USEARP)
#define NDPFLAGF_IPTYPE   (1L << NDPFLAGB_IPTYPE)
#define NDPFLAGF_ARPTYPE  (1L << NDPFLAGB_ARPTYPE)
#define NDPFLAGF_HARDADDR (1L << NDPFLAGB_HARDADDR)

/*------------------------------------------------------------------------*/

/* NIPC Protocol Structures */

#define REQFLAG_LASTSEGMENT 1   /* Set when this request packet is the last in the fragmented pieces that make up a transaction */
#define REQFLAG_INPLACE     2   /* Set when the transaction is perceived as an "in place" modification */
#define RESFLAG_LASTSEGMENT 1

struct ResponsePacket
{
    ULONG   respack_Error;          /* Non NIPC error values - if the operation couldn't be completed */
    UBYTE   respack_NewCommand;
    UBYTE   respack_Flags;
    UWORD   respack_Segment;
    ULONG   respack_Sequence;
    ULONG   respack_DataLength;     /* amount of response data */
    /* ... followed by DataLength bytes of data */
};

/*------------------------------------------------------------------------*/

/* IP Routing Macros and #defines */

#define MAX_ROUTES 32
#define IP_ADDRLEN 4

#define IP_CLASSA(x) (((x>>24) & 0x80) == 0x00)
#define IP_CLASSB(x) (((x>>24) & 0xc0) == 0x80)
#define IP_CLASSC(x) (((x>>24) & 0xd0) == 0xc0)
#define IP_CLASSD(x) (((x>>24) & 0xf0) == 0xf0)
#define IP_CLASSE(x) (((x>>24) & 0xf0) == 0xf0)

/* Structure to hold routing information */

struct NIPCRoute
{
    struct MinNode nr_Node;     /* Next Route in chain */
    ULONG nr_Network;           /* Destination machine or network */
    ULONG nr_Mask;              /* Significant bits in nr_Network */
    ULONG nr_Gateway;           /* Gateway for destination */
    struct sanadev *nr_Device;  /* Which interface gateway is connected to */
    UWORD nr_Hops;              /* Number of hops to destination */
    UWORD nr_TTL;               /* Time to live for this route */
    UWORD nr_RefCnt;            /* Number of references to this route */
    UWORD nr_UseCnt;            /* Number of uses for this route */
};

struct NIPCRouteInfo
{
    struct SignalSemaphore nri_Lock;        /* Routing table arbitration */
    struct NIPCRoute *nri_Default;          /* Default Route */
    struct MinList nri_Routes[MAX_ROUTES];  /* Routing table size */
};

/*------------------------------------------------------------------------*/

/* General Packet IO Stuff */

/* Packet Types for PacketWrite */
#define PACKET_IP  1
#define PACKET_ARP 2

/* internal structure for keeping track of SANA II devices */
struct sanadev
{
    struct MinNode  sanadev_Node;
    struct Device  *sanadev_Device;
    struct Unit    *sanadev_Unit;
    APTR            sanadev_BuffMan;
    struct MinList  sanadev_Fragments;
    ULONG           sanadev_MTU;
    long            sanadev_HardwareType;
    UWORD           sanadev_AddressSize;
    ULONG           sanadev_BPS;
    ULONG           sanadev_IPType;
    ULONG           sanadev_ARPType;
    UWORD           sanadev_Flags;
    UBYTE           sanadev_HardAddress[16];

/* pieces of the structure for things other than the lowest level */

    ULONG           sanadev_IPAddress;
    ULONG           sanadev_IPNetwork;
    ULONG           sanadev_IPSubnet;
    ULONG           sanadev_IPMask;
    ULONG           sanadev_SubnetMask;
    BOOL            sanadev_SValid;
    UBYTE           sanadev_MaxIPReq;
    UBYTE           sanadev_MaxARPReq;
    UBYTE           sanadev_NumIPReq;
    UBYTE           sanadev_NumARPReq;
};

struct LocalIOReq
{
    struct Message lreq_Msg;
    struct Buffer *lreq_Buffer;
    struct sanadev *lreq_Device;
};

#define S2DB_USEARP  0
#define S2DF_USEARP  1
#define S2DB_OFFLINE 1
#define S2DF_OFFLINE 2
#define S2DB_WAITING 2
#define S2DF_WAITING 4

/*------------------------------------------------------------------------*/

/* Statistical Data Structures */

struct stats
{
    struct MinList *DevList;
    struct MinList *EntList;
    struct MinList *ConnList;
    struct MinList *ProList;
    struct MinList *NamesList;
};


/*------------------------------------------------------------------------*/

/* IP Protocol Structures */

#define IPFLAG_MOREFRAGMENTS 1

/* For building and splitting those wonderful IP packets together and
 * into pieces.
 */

struct iphdr
{
    u_int   iph_Version:4;
    u_int   iph_IHL:4;
    u_int   iph_TypeOfService:8;
    u_int   iph_Length:16;
    u_int   iph_Ident:16;
    u_int   iph_Flags:3;
    u_int   iph_FragmentOffset:13;
    u_int   iph_TimeToLive:8;
    u_int   iph_Protocol:8;
    u_int   iph_Checksum:16;
    ULONG   iph_SrcAddr;
    ULONG   iph_DestAddr;

};

/* The structure that IP uses to keep track of all of the
 * protocols that sit on top of IP
 */

struct ipproto
{
    struct MinNode ipproto_link;
    UWORD          ipproto_ProtocolNumber;
    void         (*ipproto_Input)(struct Buffer *buff, ULONG length, struct sanadev *dev);
    void         (*ipproto_Deinit)(void);
    void         (*ipproto_Timeout)(void);
};

/*------------------------------------------------------------------------*/

/* IP ICMP Stuff */

struct icmphdrshort
{
    UBYTE   icmps_type;
    UBYTE   icmps_code;
    UWORD   icmps_checksum;
    UWORD   icmps_ident;
    UWORD   icmps_seqnum;
};

struct icmphdr
{
    struct iphdr        icmp_iphdr;
    struct icmphdrshort icmp_icmp;
};

#define ICMP_ECHOREPLY       0
#define ICMP_DESTUNREACH     3
#define ICMP_SRCQUENCH       4
#define ICMP_REDIRECT        5
#define ICMP_ECHO            8
#define ICMP_TIMESTAMP      13
#define ICMP_TIMESTAMPREPLY 14
#define ICMP_INFOREQUEST    15
#define ICMP_INFOREPLY      16


/*------------------------------------------------------------------------*/

/* Structures and #defines for the AS225 Kludge */

struct xstart
{
    struct Message      xs_Msg;
    BYTE                xs_Command;
    BYTE                xs_Filler;
    struct Device      *xs_Device;
    struct Unit        *xs_Unit;
    BOOL __stdargs    (*xs_CTB)();
    struct MsgPort     *xs_Link;
};

#define XCMD_START 0
#define XCMD_END 1


/*------------------------------------------------------------------------*/

/* RDP Protocol Structures */

struct retransmit
{
    struct MinNode  xmit_link;
    struct Buffer  *xmit_data;
    UBYTE           xmit_countdown;
    UBYTE           xmit_retrycount;
    UWORD           xmit_seqnum;
    ULONG           xmit_ToAddress;
};

struct rdphdr
{
    u_int   rdp_Flags:6,        /* Connection flags */
            rdp_Version:2,      /* Version Number of the protocol */
            rdp_HeaderLength:8, /* Length of this header */
            rdp_SrcPort:16;     /* Source Port # for this message */
    UWORD   rdp_DestPort;       /* Destination Port # for this message */
    UWORD   rdp_DataLength;     /* Data Length */
    ULONG   rdp_SeqNum;         /* Sequence number for this packet */
    ULONG   rdp_AckNum;         /* ACKnowledged up to sequence # */
    UWORD   rdp_Checksum;       /* TCPish checksum */
};

struct rdphdrSYN
{
    struct  rdphdr rsyn_hdr;
    UWORD   rsyn_MaxOutstanding;
    UWORD   rsyn_MaxSegment;
    UWORD   rsyn_Options;
};

#define RDPB_SYN 5
#define RDPB_ACK 4
#define RDPB_EAK 3
#define RDPB_RST 2
#define RDPB_NUL 1

#define RDPF_SYN 32
#define RDPF_ACK 16
#define RDPF_EAK 8
#define RDPF_RST 4
#define RDPF_NUL 2

#define RDP_VERSION 2   /* Conforms to the 2nd implementation of the RDP */

struct RDPConnection
{
    struct MinNode          conn_link;
    struct MinNode          conn_higherlink;   /* Higher level link (for amp) */
    struct SignalSemaphore  conn_ReadListSemaphore;
    struct SignalSemaphore  conn_WriteListSemaphore;
    struct MinList          conn_DataListR;    /* Incoming data list */
    struct MinList          conn_DataListW;    /* Outgoing data list */
    ULONG                   conn_Flags;        /* Assorted flags describing this connection */
    ULONG                   conn_TheirAddress; /* IP Address of who we're connected to */
    ULONG                   conn_OurAddress;   /* Which of our IP addresses are they connected to? */
    UWORD                   conn_TheirPort;    /* The port number on their side that the connection is to */
    UWORD                   conn_OurPort;      /* The port number on our side that the connection is to */
    void                  (*conn_DataIn)(struct RDPConnection *c,struct Buffer *b);  /* Called whenever a packet of data arrives */
    BOOL                  (*conn_Status)(struct RDPConnection *c);                   /* Called whenever the connection status changes */
    UBYTE                   conn_State;        /* Current state of this connection */
    UBYTE                   conn_LastFlags;    /* The last type of flag sent */
    UBYTE                   conn_LastConnTime; /* Reset every time we change state */
    UBYTE                   conn_ConnRetransmits;
    UBYTE                   conn_DataRetransmits;
    UWORD                   conn_SendNxt;      /* Seq number to send next */
    UWORD                   conn_SendOldest;   /* Seq # of oldest unacknowledged segment */
    UWORD                   conn_SendMax;      /* Max # of outstanding segments allowed */
    UWORD                   conn_InitSend;     /* Initial send sequence number */

    UWORD                   conn_Current;      /* The sequence number of the last segment received in seq */
    UWORD                   conn_RecvMax;      /* Max # of segments that can be buffered for this connection */
    UWORD                   conn_InitRecv;     /* Initial receive sequence number */

    UWORD                   conn_SendMaxSize;  /* Largest size of a packet in bytes that we're allowed to send */
    UWORD                   conn_RecvMaxSize;  /* Largest size of a packet in bytes that we'll receive */

    ULONG                   conn_TimeStamp[2]; /* Used in connection phase for figuring the
                                                * round-trip-time.  When in STATE_OPEN, the 1st
                                                * longword contains the # of 1/1024ths of a second
                                                * that round-trip packet time takes. (rough)
                                                */

/* bookkeeping pieces */

    UWORD                   conn_SeqDelivered; /* seq # of last packet delivered to ANMP */

/* Things for the ANMP implementation */

    ULONG                   conn_WakeBit;
    struct Task            *conn_WakeTask;
    struct SignalSemaphore  conn_InSema;
    struct MinList          conn_InList;
    struct Entity          *conn_linkentity;


};

/* Various RDP states */

#define STATE_CLOSED     0
#define STATE_LISTEN     1
#define STATE_SENT       2
#define STATE_RCVD       3
#define STATE_OPEN       4
#define STATE_CLOSE      5

#define CONN_MULTIPLE    1    /* If this flag is set, multiple connections will be spawned when
                               * a connection is attempted to this port.
                               */
#define CONN_OWNEDSIGNAL 2    /* If set, CloseConnection() will attempt to free the signal bit
                               * associated with this connection.
                               */
#define CONN_WINDOWSIGNAL 4   /* If set, when the window size changes, the WakeBit/WakeTask
                               * pair are set on a window size change */


/*------------------------------------------------------------------------*/

/* UDP Protocol Structures and #defines */

struct udphdr
{
    UWORD udp_SrcPort;
    UWORD udp_DestPort;
    UWORD udp_Length;
    UWORD udp_Checksum;
};

struct UDPConnection        /* Not really a "connection" */
{
    struct MinNode  conn_link;
    struct MinNode  conn_higherlink;
    ULONG           conn_Flags;
    UWORD           conn_OurPort;
    void          (*conn_DataIn)(struct UDPConnection *conn,struct Buffer *buff,struct sanadev *dev);

};

/* typedef's for indirect function pointers */

typedef void (*RDP_DATA)(struct RDPConnection *c, struct Buffer *b);
typedef BOOL (*RDP_STATUS)(struct RDPConnection *c);
typedef void (*UDP_DATA)(struct UDPConnection *c, struct Buffer *b,struct sanadev *dev);

/*------------------------------------------------------------------------*/

/* ARP Protocol Structures and #defines */

/* defines for constants */
#define TABLE_SIZE 15

/*
 * BUFFERTIME is the number of tenths of a second that packets are held onto
 * while we hope for an ARP resolution.
 */

#define BUFFERTIME 1

#define ARP_REQUEST 1
#define ARP_REPLY   2

/* currently hardcoded for ethernet.  It CAN be expanded, though. */

struct arppkt
{
    UWORD arp_HardwareType;
    UWORD arp_ProtocolType;
    UBYTE arp_HardAddrLen;
    UBYTE arp_ProtoAddrLen;
    UWORD arp_Operation;
    UBYTE arp_SenderEtherAddress[6];
    ULONG arp_SenderIPAddress;
    UBYTE arp_TargetEtherAddress[6];
    ULONG arp_TargetIPAddress;
};

struct arpentry
{
    struct MinNode arpentry_link;
    UBYTE arpentry_ether[6];
    ULONG arpentry_IP;
};

struct arpbuff
{
    struct MinNode ab_link;     /* link .. */
    ULONG ab_IPAddress;         /* The IP address this sucker is going to */
    struct Buffer *ab_Buffer;   /* The buffer for this thing */
    UWORD ab_CountDown;         /* # of tenths of a second this thing has
                                 * left to live */
    struct sanadev *ab_dev;
    ULONG ab_gateway;
    UWORD ab_PacketType;
};

/*------------------------------------------------------------------------*/

/* NIPC Monitor Structure */

struct NIPCMonitorInfo
{
    struct Task    *nmi_Monitor;
    UWORD           nmi_Reserved0;
    BYTE            nmi_SigBitNum;
    UBYTE           nmi_Reserved1;
    ULONG           nmi_IPIn;
    ULONG           nmi_IPOut;
    ULONG           nmi_IPForwarded;
    ULONG           nmi_IPKept;
    ULONG           nmi_IPBytesIn;
    ULONG           nmi_IPBytesOut;
    ULONG           nmi_IPBytesForwarded;
    ULONG           nmi_IPBytesKept;
    ULONG           nmi_ARPReqSent;
    ULONG           nmi_ARPReqReceived;
    ULONG           nmi_ARPReplySent;
    ULONG           nmi_ARPReplyReceived;
    ULONG           nmi_RDPIn;
    ULONG           nmi_RDPOut;
    ULONG           nmi_RDPBytesIn;
    ULONG           nmi_RDPBytesOut;
    ULONG           nmi_UDPIn;
    ULONG           nmi_UDPOut;
    ULONG           nmi_UDPBytesIn;
    ULONG           nmi_UDPBytesOut;
    ULONG           nmi_AvailChipStart;
    ULONG           nmi_AvailFastStart;
};

/* Profiling Stuff */

#ifdef PROFILE
#define ONTIMER(n) { ULONG *tic=NULL; *tic |= (1l<<(n)); }
#define OFFTIMER(n) { ULONG *tic=NULL; *tic &= (0xffffffffl-(1l<<(n))); }
#else
#define ONTIMER(n)
#define OFFTIMER(n)
#endif

#endif  /* NIPC_NIPCINTERNALS_H */

