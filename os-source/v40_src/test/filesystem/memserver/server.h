/* Stuff the server has to deal with */

#include <libraries/configvars.h>


struct memItem {                     /* Descripts of memory block       */
    struct MinNode memBlocks;        /* for Exec MinList structure      */
    VOID       *memPtr;              /* pointer to RAM                  */
    ULONG       byteSize;            /* size of RAM                     */
    ULONG       openCount;           /* how many times mem is in use    */
    ULONG       readCount;           /* read locks on RAM               */
    ULONG       writeCount;          /* write/ncwrite locks on RAM      */
    BYTE        permitted;           /* access type permitted           */
    BYTE        deAlloc;             /* means of deallocating           */
    BYTE        location;            /* board where RAM is              */
    BYTE        locked;              /* if ram is locked                */
    ULONG       lock_owner;          /* owner of ramlock                */
    ULONG       owner;               /* address of clientName           */
};

/* NB: owner is a ptr to a name set by the client.  If NULL, the owner
       has said it's OK to free the block of memory.
       owner is compared against memRequest.clientName for lock purposes.
*/


struct boardInfo {
    ULONG start;              /* start of memory        */
    ULONG end;                /* last byte of memory    */
    ULONG size;               /* size in bytes          */
};

/* currently permitted access */
#define NOTHING_OK         0             /* no access */
#define READ_OK            1L<<0         /* bit 1, read OK  */
#define WRITE_OK           1L<<1         /* bit 2, write OK */
#define NCREAD_OK          1L<<2         /* nocare read OK  */
#define NCWRITE_OK         1L<<4         /* nocare write OK */

/* "NoCare" means that it's OK for the buffer's contents
    to be changed by other clients.
    Default is to give real locks... NC's are equivalent
    to dealing with NIL:, really....
*/


#define NO_ACCESS          0
#define READ_NOWRITE      (NO_ACCESS + 1) /* for verify */
#define READ_OKWRITE      (NO_ACCESS + 2) /* throwaway  */
#define WRITE_ANDREAD     (NO_ACCESS + 3) /* write 'n' lock */
#define WRITE_NOREAD      (NO_ACCESS + 4) /* exclusive write */
#define ANY_ACCESS        (NO_ACCESS + 5) /* no-care */
#define EXCLUSIVE_ACCESS  (NO_ACCESS + 6)   /* set locked, lock_owner, and permitted */

/* means of deallocating RAM */
#define DEALLOC_FREEMEM     0
#define DEALLOC_FREEVEC     1
#define DEALLOC_DONT        2  /* ROM */

/* see common_h for memory board locations */

#define MAX_BOARDS   7                   /* cpu=slot1, slots2-6, motherboard */
#define MOTHER_BOARD LOC_MOTHERBOARD-1   /* which must be < MAX_BOARDS, preferably max_boards-1 */

