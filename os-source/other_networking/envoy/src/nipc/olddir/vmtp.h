
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>

 typedef unsigned int u_int;

/* The current definition for an entity.  This may well change radically.
 * The Port specification may become an actual hardware address on the
 * end machine (unlikely); and the Address portion may disappear entirely
 * from the structure (it doesn't HAVE to be there.  A matter of
 * convienence.)
 */
 struct entity
   {
   u_int ent_Flags:4;
   u_int ent_Port:28;      /* MsgPort Identifier (not a mem address) */
   ULONG ent_Address;      /* Internet address of site */
   };

/* RequestCodes - AMP's are private values */
#define AMP_DATA 0xDDC0DE

/* ResponseCodes */
#define RSP_OK                 0
#define RSP_RETRY              1
#define RSP_RETRY_ALL          2
#define RSP_BUSY               3
#define RSP_NONEXISTANT_ENTITY 4
#define RSP_ENTITY_MIGRATED    5
#define RSP_NO_PERMISSION      6
#define RSP_NOT_AWAITING_MSG   7
#define RSP_VMTP_ERROR         8
#define RSP_MSGTRANS_OVERFLOW  9
#define RSP_BAD_TRANSACTION_ID 10
#define RSP_STREAMING_NOT_SUPPORTED 11
#define RSP_NO_RUN_RECORD      12
#define RSP_RETRANS_TIMEOUT    13
#define RSP_USER_TIMEOUT       14
#define RSP_RESPONSE_DISCARDED 15
#define RSP_SECURITY_NOT_SUPPORTED 16
#define RSP_BAD_REPLY_SEGMENT  17
#define RSP_SECURITY_REQUIRED  18
#define RSP_STREAMED_RESPONSE  19
#define RSP_TOO_MANY_RETRIES   20
#define RSP_NO_PRINCIPAL       21
#define RSP_NO_KEY             22
#define RSP_ENCRYPTION_NOT_SUPPORTED 23
#define RSP_NO_AUTHENTICATOR   24


/* PacketFlag bits */
#define VMTP_HCO 4
#define VMTP_EPG 2
#define VMTP_MPG 1

/* ControlFlags bits */
#define VMTP_NRS 256
#define VMTP_APG 128
#define VMTP_NSR 64
#define VMTP_NER 32
#define VMTP_NRT 16
#define VMTP_MDG 8
#define VMTP_CMG 4
#define VMTP_STI 2
#define VMTP_DRT 1

/* Code flag bits */
#define VMTP_CMD (1<<31)
#define VMTP_DGM (1<<30)
#define VMTP_MDM (1<<29)
#define VMTP_SDA (1<<28)
#define VMTP_CRE (1<<26)
#define VMTP_MRD (1<<25)
#define VMTP_PIC (1<<24)



   struct request
      {
      struct entity req_Client;           /* 64 bit client identifier */
      u_int req_Version:3,                /* Protocol revision # */
            req_Domain:13,                /* domain for client/server */
            req_PacketFlags:3,            /* varied */
            req_Length:13;                /* # of longwords in the data field */
      u_int req_ControlFlags:9,           /* M O R E  Flags! */
            req_RetransmitCount:3,        /* # of times this packet has been transmitted MOD 8 */
            req_ForwardCount:4,           /* # of times this packet has been forwarded */
            req_InterpacketGap:8,         /* Sheesh.  Read the RFC for this one */
            req_Priority:4,               /* self-explan. */
            req_ResFlags:3,               /* Reserved flag bits */
            req_FunctionCode:1;           /* A request or a response ? */
      ULONG req_Transaction;              /* Identifier for this message transaction */
      ULONG req_PacketDelivery;           /* Which segment blocks are in this packet */
      struct entity req_Server;           /* 64 bit server identifier */
      ULONG req_Code;                     /* Even _more_ flags! */
      struct entity req_CoResidentEntity; /* Read the RFC.  */
      UBYTE req_UserData[12];             /* General Purpose space */
      ULONG req_MsgDelivery;              /* Read the RFC.  */
      ULONG req_SegmentSize;              /* Size of the segment */
      };

/* A VMTP request looks like this:
 *
 * struct request
 * {0-4M of data}
 * 32 bit Checksum
 */

   struct response
      {
      struct entity rsp_Client;           /* 64 bit client identifier */
      u_int rsp_Version:3,                /* Protocol revision # */
            rsp_Domain:13,                /* domain for client/server */
            rsp_PacketFlags:3,            /* varied */
            rsp_Length:13;                /* # of longwords in the data field */
      u_int rsp_ControlFlags:9,           /* M O R E  Flags! */
            rsp_RetransmitCount:3,        /* # of times this packet has been transmitted MOD 8 */
            rsp_ForwardCount:4,           /* # of times this packet has been forwarded */
            rsp_PGCount:8,                /* For acking lots of packets */
            rsp_Priority:4,               /* self-explan. */
            rsp_ResFlags:3,               /* reserved flags */
            rsp_FunctionCode:1;           /* A request or a response ? */
      ULONG rsp_Transaction;              /* Identifier for this message transaction */
      ULONG rsp_PacketDelivery;           /* Which segment blocks are in this packet */
      struct entity rsp_Server;           /* 64 bit server identifier */
      ULONG rsp_Code;                     /* Even _more_ flags! */
      UBYTE rsp_UserData[20];             /* General Purpose space */
      ULONG rsp_MsgDelivery;              /* Read the RFC.  */
      ULONG rsp_SegmentSize;              /* Size of the segment */
      };

/* A VMTP response looks like this:
 *
 * struct response
 * {0-4M of data}
 * 32 bit Checksum
 */


/* The staterecord is created for every outgoing message.  It keeps track
 * of which data is associated with this message, who the message was
 * sent to, and how many times it has, at any given instant, been
 * retransmitted.
 */

   struct staterecord
      {
      struct MinNode      state_link;
      struct entity       state_DestEntity;
      struct Transaction *state_TransPtr;
      ULONG               state_TransNumber; /* Filled in on a request */
      UWORD               state_HeartBeats;  /* The number of heartbeats remaining b4 a retransmission */
      ULONG               state_TransmitCount;
      };

/* The entityentry (not to be confused with the "entity" structure defined
 * above) is created for each local network message port.  It defines a
 * number of things, including it's entity identifier (-now- see the
 * entity structure above) for matching up incoming packets with this
 * entry.
 */

   struct entityentry
      {
      struct MinNode  ee_link;       /* The link for multiple entries */
      struct entity   ee_ident;      /* Our entity identifier */
      struct MsgPort *ee_port;      /* The message port that read data is sent to */
      struct MinList  ee_statercds;  /* Outgoings that're waiting here */
      };



