/* Common_h.c - for precompiled headers */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/dosasl.h>  /* for additional error return codes */
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/expansion_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
/* #include <clib/alib_protos.h> */
#include <clib/alib_exec_protos.h>
#include <math.h>
#include "jwldefs.h"                      /* includes extern DOSBase reference */

/* memRequest is the major structure used by both server and clients to deal with
   memory pools.  If memPtr is NULL, server will treat it as an allocation request.
   Otherwise, it'll treat it as a request to access the referenced memory in the
   mode specified by memRequest.access.  These modes are as defined for locks
   in <dos/dos.h>: SHARED_LOCK, ACCESS_READ, EXCLUSIVE_LOCK, ACCESS_WRITE.

   Note:  For allocation requests, non-zero memUpper will use AllocAbs(); 
   otherwise, AllocVec will be used 
 */

struct memRequest {
    struct Message msg;   /* exec message (note that Message.MsgPort is ID)           */
    VOID   *memPtr;       /* pointer to memory                                        */
    ULONG  memSize;       /* size of memory (bytes)                                   */
    ULONG  memType;       /* type of memory (MEMF_CHIP, etc.)                         */
    ULONG  memUpper;      /* highest possible memory address for block, if applicable */
    ULONG  memLower;      /* lowest possible memory address for block, if applicable  */
    LONG   board;         /* board in system, -1 for n/a                              */
    LONG   access;        /* just like DOS Lock access; see <dos/dos.h>               */
    LONG   newAccess;     /* for changing access to a block                           */
    struct DateTime *dt;  /* For timing and record purposes                           */
    STRPTR log1;          /* Message for log file                                     */
    STRPTR log2;          /* Second msg string for log file                           */
    STRPTR clientName;    /* name of msgport of program making request (for log)      */
    UBYTE  status;        /* starting or stopping a copy...                           */
    UBYTE  cmd;           /* request to free said RAM, or whatever                    */
    SHORT  rc;            /* return code                                              */
    SHORT  rc2;           /* further info, if available                               */
};

/* A note on DateTime:  initialize everything EXCEPT for the
   buffers for date and time.  Server will plug in its own buffers
   and do a DateToStr(dt) on the structure provided in the memRequest
 */
 

/* From <dos/dos.h>, used in memRequest.access:
    SHARED_LOCK           / for read access  /
    ACCESS_READ           / for read access  /   / all DOS values are negative /
    EXCLUSIVE_LOCK        / for write access /
    ACCESS_WRITE          / for write access /
Additional codes and synonyms:
*/
#define NC_READ   1               /* NoCare read (buffer may be changing) */
#define NC_WRITE  2               /* NoCare write (buffer may change)     */
#define OK_READ   ACCESS_READ     /* Access_Read, no writers allowed      */
#define OK_WRITE  ACCESS_WRITE    /* Access_Write, no readers allowed     */

#define MEM_CONDITION                    0    /* Error returns                   */
#define MEM_LOCKED      (MEM_CONDITION + 1)   /* memory was locked for writing   */
#define MEM_READ_ONLY    MEM_LOCKED           /* ie, 'twas ROM                   */
#define MEM_OK          (MEM_CONDITION + 2)   /* return_OK                       */
#define MEM_DENIED      (MEM_CONDITION + 3)   /* memory unavailable, unknown why */
#define MEM_NOT_AVAIL   (MEM_CONDITION + 4)   /* out of RAM                      */
#define MEM_NOT_VALID   (MEM_CONDITION + 5)   /* bad board #; mem nonexistent    */
#define MEM_FREED       (MEM_CONDITION + 6)   /* memory was freed */

#define STATUS_BASE                      0
#define STATUS_DONE       (STATUS_BASE + 1)   /* just finished (check rc/rc2)           */
#define STATUS_BUSY       (STATUS_BASE + 2)   /* busy, or about to be busy              */
#define STATUS_IDLE       (STATUS_BASE + 3)   /* doing nothing                          */
#define STATUS_EXIT       (STATUS_BASE + 4)   /* leaving                                */
#define STATUS_NOT_DONE   (STATUS_BASE + 5)   /* couldn't perform requested funct       */
#define STATUS_CLIENT_ERR (STATUS_BASE + 6)   /* client reporting error with CMD_LOGIT  */
#define STATUS_SEVERE_ERR (STATUS_BASE + 7)   /* client has serious error.  Continue?   */
#define STATUS_CHECK_CMD  (STATUS_BASE + 8)   /* server has advice for client in mr.cmd */

#define CMD_BASE                    0         /* Requests a client can make            */
#define CMD_FREE_MEM    (CMD_BASE + 1)        /* Done with RAM, ok to free it          */
#define CMD_LOGIT       (CMD_BASE + 2)        /* Make a logfile entry                  */
#define CMD_MEMREQ      (CMD_BASE + 3)        /* Request memory described              */
#define CMD_ABEO        (CMD_BASE + 4)        /* client is exiting                     */
#define CMD_NEW_ACCESS  (CMD_BASE + 5)        /* change access to mem block            */
#define CMD_QUIT        (CMD_BASE + 6)        /* Tell server or client to quit         */
#define CMD_CONTINUE    (CMD_BASE + 7)        /* Tell client to continue               */

/* NB:  Server *really* *will* quit if CMD_QUIT is received; recommend separate prog
   to make it quit.  Done; it's called KillServer.
*/

#define RC2_EXITING 6666

/* A few ways to refer to the server's port */
#define SERVER_NAME "A2091_MemServer"
#define SERVER_PORT SERVER_NAME
#define SERVER_PORT_NAME SERVER_NAME

#define PORT_PRIORITY 0      /* for CreatePort() */

#define ROM_SIZE 512 * 1024    /* maximum ROM size --- assumes 2.0x, 512K ROM */

/* Locations of memory */
#define LOC_BASE            0
#define LOC_BUS_1           (LOC_BASE + 1)
#define LOC_BUS_2           (LOC_BASE + 2)
#define LOC_BUS_3           (LOC_BASE + 3)
#define LOC_BUS_4           (LOC_BASE + 4)
#define LOC_BUS_5           (LOC_BASE + 5)
#define LOC_BUS_6           (LOC_BASE + 6)
#define LOC_MOTHERBOARD     (LOC_BASE + 7)   /* often chip ram */
#define LOC_CHIP            (LOC_BASE + 8)   /* any chip ram   */
#define LOC_FAST            (LOC_BASE + 9)   /* any fast ram   */
#define LOC_ANY             (LOC_BASE + 10)  /* any ram at all */
#define LOC_ROM             (LOC_BASE + 11)  /* read-only memory */
#define LOC_CPU_BOARD       LOC_BUS_1

/* Stuff for fancy error reporting -- VT100 codes for serial output */
#define BELL_CHAR              0x07
#define SPACE_CHAR             ' '
#define NORMAL_VIDEO_STRING    "\x1b[0m"
#define BOLDFACE_VIDEO_STRING  "\x1b[1m"
#define UNDERLINE_VIDEO_STRING "\x1b[4m"
#define BLINKING_VIDEO_STRING  "\x1b[5m"
#define INVERSE_VIDEO_STRING   "\x1b[7m"

