#include <exec/types.h>

#include "nipcinternal.h"
#include "externs.h"

#include <envoy/errors.h>
#include <string.h>

#include <exec/devices.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <clib/timer_protos.h>

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <devices/sana2.h>

#define MIN(x, y) ((x) < (y) ? (x):(y))
#define MAX(x, y) ((x) > (y) ? (x):(y))
#define OURHOSTNAME ((char *)&(gb->LocalHostName))

/* This next line is here only temporarily, until I can get access to nipcinternal_protos.h */
extern void RespondToPing(struct RDPConnection *c, struct PingRequest *pr);
extern void GotResponse(struct PingResponse *pr);
extern void __asm Mul64by16(register __a0 ULONG *ptr,register __d0 ULONG multiplier);
void ReturnTransaction(struct Transaction *t);


#define MAXSEGS 5
#define MAX_ACK_DIFF 3
#define MAXSIZE 8192


/* #define DEBUGMSGS */
#define RETRIES  8
#define TIMEOUT  2             /* Temporary until I can work in an adaptive (2*10) timeout */
#define CONNTIME 1             /* 10ths of a second between connection state
                                 * timeouts */
#define TOTALALLOWED 10         /* # of times we'll send connection
                                 * retranmissions before giving up */

#define FRAGTIMEOUT 3          /* 3 seconds timeout for fragments */


extern APTR far ACTB;
extern APTR far ACFB;

/*------------------------------------------------------------------------*/

/* worthwhile constants */
#define TOTALQUEUE 25           /* Number of IP read requests that I keep
                                 * running at any given time */

#define MIN(x, y) ((x) < (y) ? (x):(y))
#define MAX(x, y) ((x) > (y) ? (x):(y))

/*------------------------------------------------------------------------*/

BOOL __stdargs(*ExCTB) (UBYTE * from, ULONG length, APTR to);


/* superstructure */
struct IOSana2ReqSS
{
    struct IOSana2Req ss;
    struct sanadev *ios2_DevPtr;
};

/* Structures used by the resolver */

struct ServicesLib
{
    struct Library           SVCS_Lib;
    APTR                     SVCS_NIPCBase;
    APTR                     SVCS_UtilityBase;
    struct ExecBase         *SVCS_SysBase;
    BPTR                     SVCS_SegList;

    /* Services Library and Services Manager Common */
    struct SignalSemaphore   SVCS_ServicesLock;
    struct MinList           SVCS_Services;
    struct SignalSemaphore   SVCS_OpenLock;
};

struct Service
{
    struct Node     svc_Node;                   /* Chain of services */
    UBYTE           svc_PathName[256];          /* Who to run to start a server */
    UBYTE           svc_SvcName[64];            /* Name of this service */
    ULONG           svc_Flags;                  /* Various Flags */
    UWORD           svc_UID;                    /* User allowed to use this service */
    UWORD           svc_GID;                    /* Group allowed to use this service */
};

#define SVCB_EXPUNGE    1
#define SVCF_EXPUNGE    (1 << SVCB_EXPUNGE)

#define NIP_UDP_PORT	376

//extern void dbprint(STRPTR message);
