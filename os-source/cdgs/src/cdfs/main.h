#include <exec/types.h>
#include "std.h"
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <devices/inputevent.h>
#include <devices/input.h>
#include <exec/resident.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include "cd.h"
#include "iso.h"

typedef	unsigned char LOGIC;

IMPORT	void	*MakeVector();
IMPORT	void	*FreeVector();

#define	DOS_FALSE 0L
#define	DOS_TRUE -1L

#define	MAX_NAME_LEN	30
#define	MAX_BSTR_LEN	34

#define	SECT_SIZE 2048
#define	SECT_SHIFT 11

#define COMPLEX_CACHE

#define	NOARG	0
#define	ARGS	0
#define	ARG1	1
#define	ARG2	2
#define	ARG3	4
#define	ARG4	8
#define	ARG12	3
#define	ARG123	7
#define	ARG1234	15
#define	ARG23	6
#define	ARG234	14

#define BTOA(x)	((APTR)(((ULONG)x) << 2))
#define ATOB(x)	((BPTR)(((ULONG)x) >> 2))
#define	ODD(v)  ((int)(v) & 1)

struct	CDLock
{
	BPTR	Link;		/* bcpl pointer to next lock */
	LONG	Key;		/* disk block number */
	LONG	Access;		/* exclusive or shared */
	struct	MsgPort *Task;	/* handler task's port */
	BPTR	Volume;		/* bptr to a DeviceList */
	LONG	PathNum;	/* dir path index number */
	LONG	ByteSize;	/* size of file in bytes */
	ULONG	DirOffset;	/* dir entry offset */
};

struct	VolDev
{
	struct	DeviceList dev;
	ULONG	MaxBlocks;
	ULONG	BlockSize;
	ULONG	PathTableSize;
	ULONG	MaxPaths;
	ULONG	RootDirLBN;
	ULONG	ReadErrs;
	struct	PathTableEntry *PathTable;
	struct	PathTableEntry **PathIndex;
	UCHAR	name[34];	/* long-word aligned */
};

struct FileControl
{
	struct FileControl *Next;/* Next fc in system.		*/
	struct CDLock *Lock;	/* Lock of file to be accessed	*/
	ULONG ReadPtr;		/* Current position in block	*/
	ULONG BaseLBN;		/* Key of current block		*/
	LONG  Size;		/* Size of file			*/
	ULONG Flags;		/* DirectRead flag		*/
};

struct MemPool
{
	APTR *head;
	APTR *tail;
	APTR *limit;
	ULONG size;
};

#ifdef COMPLEX_CACHE
struct Cache
{
	ULONG block;
	LONG  retry;
	struct IOStdReq IOR;	/* LW sized */
	UBYTE *data;
	UBYTE active;
	UBYTE pad;
};
#endif

typedef	struct FileLock	FLOCK;
typedef	struct CDLock	CDLOCK;
/*typedef	struct FileLock	LOCK;	*/
typedef	struct VolDev	VOLDEV;
typedef	struct FileInfoBlock FIB;
typedef	struct DeviceList  DEVLST;
typedef struct FileControl FCNTRL;
typedef	struct FileHandle  FHANDL;
typedef	struct DosPacket   DOSPKT;
typedef	struct MemPool POOL;

#define	MUST(e)	if (!(e)) return 0;

#define	FC_DIRECTREAD	1		/* file control flag */
#define FC_SEEKBACK	2		/* backwards seek done on fh */

#include "global.h"
