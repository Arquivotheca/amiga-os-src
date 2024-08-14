/*
 * $Header: APPN:src/libs/nipc.library/RCS/route.h,v 1.2 92/02/17 10:06:39 kcd Exp $
 *
 * Data structures for holding the routine table and entries.
 *
 */

#include <exec/semaphores.h>

#define MAX_ROUTES 32

#define IP_ADDRLEN 4

#define IP_CLASSA(x) (((x>>24) & 0x80) == 0x00)
#define IP_CLASSB(x) (((x>>24) & 0xc0) == 0x80)
#define IP_CLASSC(x) (((x>>24) & 0xd0) == 0xc0)
#define IP_CLASSD(x) (((x>>24) & 0xf0) == 0xf0)
#define IP_CLASSE(x) (((x>>24) & 0xf0) == 0xf0)

struct NIPCRoute
{
    struct MinNode nr_Node;                     /* Next Route in chain */
    ULONG nr_Network;                           /* Destination machine or network */
    ULONG nr_Mask;                              /* Significant bits in nr_Network */
    ULONG nr_Gateway;                           /* Gateway for destination */
    struct sanadev *nr_Device;                  /* Which interface gateway is connected to */
    UWORD nr_Hops;                              /* Number of hops to destination */
    UWORD nr_TTL;                               /* Time to live for this route */
    UWORD nr_RefCnt;                            /* Number of references to this route */
    UWORD nr_UseCnt;                            /* Number of uses for this route */
};

struct NIPCRouteInfo
{
    struct SignalSemaphore nri_Lock;            /* Routing table arbitration */
    struct NIPCRoute *nri_Default;              /* Default Route */
    struct MinList nri_Routes[MAX_ROUTES];      /* Routing table size */
};

