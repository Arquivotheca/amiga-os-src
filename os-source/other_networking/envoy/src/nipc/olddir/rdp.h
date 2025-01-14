
#include <exec/semaphores.h>
#include <exec/tasks.h>

 typedef unsigned int u_int;


#define MAXSEGS 10   /* Maximum # of segments that the host is allowed to queue */
#define MAXSIZE 1000 /* Max # of bytes in each packet */

 struct retransmit
   {
   struct MinNode xmit_link;
   struct Buffer *xmit_data;
   UBYTE          xmit_countdown;
   UBYTE          xmit_retrycount;
   UWORD          xmit_seqnum;
   ULONG          xmit_ToAddress;
   };

 struct rdphdr
   {
   u_int rdp_Flags:6,                  /* Connection flags */
         rdp_Version:2,                /* Version Number of the protocol */
         rdp_HeaderLength:8,           /* Length of this header */
         rdp_SrcPort:16;               /* Source Port # for this message */
   UWORD rdp_DestPort;                 /* Destination Port # for this message */
   UWORD rdp_DataLength;               /* Data Length */
   ULONG rdp_SeqNum;                   /* Sequence number for this packet */
   ULONG rdp_AckNum;                   /* ACKnowledged up to sequence # */
   UWORD rdp_Checksum;                 /* TCPish checksum */
   };

 struct rdphdrSYN
   {
   struct rdphdr rsyn_hdr;
   UWORD rsyn_MaxOutstanding;
   UWORD rsyn_MaxSegment;
   UWORD rsyn_Options;
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
      struct MinNode         conn_link;
      struct MinNode         conn_higherlink;   /* Higher level link (for amp) */
      struct SignalSemaphore conn_ReadListSemaphore;
      struct SignalSemaphore conn_WriteListSemaphore;
      struct MinList         conn_DataListR;    /* Incoming data list */
      struct MinList         conn_DataListW;    /* Outgoing data list */
      ULONG                  conn_Flags;        /* Assorted flags describing this connection */
      ULONG          conn_TheirAddress; /* IP Address of who we're connected to */
      ULONG          conn_OurAddress;   /* Which of our IP addresses are they connected to? */
      UWORD          conn_TheirPort;    /* The port number on their side that the connection is to */
      UWORD          conn_OurPort;      /* The port number on our side that the connection is to */
      void           (*conn_DataIn)();  /* Called whenever a packet of data arrives */
      void           (*conn_Status)();  /* Called whenever the connection status changes */
      UBYTE          conn_State;        /* Current state of this connection */
      UBYTE          conn_LastFlags;    /* The last type of flag sent */
      UBYTE          conn_LastConnTime; /* Reset every time we change state */
      UBYTE          conn_ConnRetransmits;
      UBYTE          conn_DataRetransmits;
      UWORD          conn_SendNxt;      /* Seq number to send next */
      UWORD          conn_SendOldest;   /* Seq # of oldest unacknowledged segment */
      UWORD          conn_SendMax;      /* Max # of outstanding segments allowed */
      UWORD          conn_InitSend;     /* Initial send sequence number */

      UWORD          conn_Current;      /* The sequence number of the last segment received in seq */
      UWORD          conn_RecvMax;      /* Max # of segments that can be buffered for this connection */
      UWORD          conn_InitRecv;     /* Initial receive sequence number */

      UWORD          conn_SendMaxSize;  /* Largest size of a packet in bytes that we're allowed to send */
      UWORD          conn_RecvMaxSize;  /* Largest size of a packet in bytes that we'll receive */

/* bookkeeping pieces */
      UWORD          conn_SeqDelivered; /* seq # of last packet delivered to ANMP */
      UWORD          conn_SeqContiguous; /* We have segments contiguous up to this seq # */

/* Things for the ANMP implementation */
      ULONG          conn_WakeBit;
      struct Task   *conn_WakeTask;
      struct SignalSemaphore conn_InSema;
      struct MinList conn_InList;
      struct Entity *conn_linkentity;

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


