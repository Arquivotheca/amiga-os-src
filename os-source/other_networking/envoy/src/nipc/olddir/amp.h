
#include "nipc.h"

#define RESOLVERPORT 1        /* RDP port # for the entity name resolver */

struct Entity
   {
   struct MsgPort         entity_Port;         /* Message port */
   ULONG                  entity_Flags;        /* See below */
   struct SignalSemaphore entity_linksema;     /* link semaphore */
   struct MinList         entity_linklist;     /* All link entities to this entity */
   struct SignalSemaphore entity_transsema;    /* semaphore for below */
   struct MinList         entity_translist;    /* transactions that've been sent from this entity */
   struct Entity         *entity_owner;        /* Filled in only if this is a link */
   struct RDPConnection  *entity_connection;   /* The connection for this entity */
   char                  *entity_linkname;     /* hostname on which the entity that this link represents actually exists */
   struct MinList         entity_Partials;     /* Pieces of transmitted transactions, while they're assembled */
   };

/* Entity flags */
#define ENTF_PUBLIC 1            /* Set if this is a public port */
#define ENTB_PUBLIC 0
#define ENTF_ALLOCSIGNAL 2       /* Set if CreateEntity allocated a signal bit for this port */
#define ENTB_ALLOCSIGNAL 1
#define ENTF_LINK        4       /* Set if this is a link to an external entity, rather than a
                                  * local one */
#define ENTB_LINK        2       /* (From FindEntity rather than CreateEntity) */

 struct nameentry
   {
   struct MinNode ne_link;
   char           ne_name[80];
   ULONG          ne_Address;
   };

 struct NameRequest
   {
   char           nr_Name[80];         /* Name to find */
   char           nr_FromName[80];     /* Name of entity doing the finding .. */
   char           nr_FromHost[80];     /* Name of the machine on which the from entity exists */
   };

 struct NameReply
   {
   UWORD          nr_PortNumber;
   };


 struct RequestPacket
   {
   UBYTE          reqpack_Command;  /* From trans_Command */
   UBYTE          reqpack_Flags;
   UWORD          reqpack_Segment;  /* Segment number */
   ULONG          reqpack_Sequence;
   ULONG          reqpack_ResponseDataSize;
   ULONG          reqpack_DataLength;
   /* ... followed by DataLength bytes of data */
   };

#define REQFLAG_LASTSEGMENT 1
#define RESFLAG_LASTSEGMENT 1

 struct ResponsePacket
   {
   UBYTE          respack_Error;       /* Non NIPC error values - if the operation couldn't be completed */
   UBYTE          respack_Flags;
   UWORD          respack_Segment;
   ULONG          respack_Sequence;
   ULONG          respack_DataLength;  /* amount of response data */
   /* ... followed by DataLength bytes of data */
   };

