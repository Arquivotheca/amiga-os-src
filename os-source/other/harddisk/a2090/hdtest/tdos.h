#ifndef	LIBRARIES_DOS_H
#define	LIBRARIES_DOS_H
#ifndef	EXEC_TYPES_H
#include	"exec/types.h"
#endif
#define	DOSNAME	"dos.library"
#define	MODE_OLDFILE	1005
#define	MODE_NEWFILE	1006
#define	OFFSET_BEGINNING	-1
#define	OFFSET_CURRENT	0
#define	OFFSET_END	1
#define	OFFSET_BEGINING	OFFSET_BEGINNING
#define	BITSPERBYTE	8
#define	BYTESPERLONG	4
#define	BITSPERLONG	32
#define	MAXINT	0x7FFFFFFF
#define	MININT	0x80000000
#define	SHARED_LOCK	-2
#define	ACCESS_READ	-2
#define	EXCLUSIVE_LOCK	-1
#define	ACCESS_WRITE	-1
struct	DateStamp	{
LONG	ds_Days;
LONG	ds_Minute;
LONG	ds_Tick;
};
#define	TICKS_PER_SECOND	50
struct	FileInfoBlock	{
LONG	fib_DiskKey;
LONG	fib_DirEntryType;
char	fib_FileName[108];
LONG	fib_Protection;
LONG	fib_EntryType;
LONG	fib_Size;
LONG	fib_NumBlocks;
struct	DateStamp	fib_Date;
char	fib_Comment[116];
};
#define	FIBB_READ	3
#define	FIBB_WRITE	2
#define	FIBB_EXECUTE	1
#define	FIBB_DELETE	0
#define	FIBF_READ	(1<<FIBB_READ)
#define	FIBF_WRITE	(1<<FIBB_WRITE)
#define	FIBF_EXECUTE	(1<<FIBB_EXECUTE)
#define	FIBF_DELETE	(1<<FIBB_DELETE)
typedef	long	BPTR;
typedef	long	BSTR;
#define	BADDR(	bptr	)	(((ULONG)bptr)	<<	2)
struct	InfoData	{
LONG	id_NumSoftErrors;
LONG	id_UnitNumber;
LONG	id_DiskState;
LONG	id_NumBlocks;
LONG	id_NumBlocksUsed;
LONG	id_BytesPerBlock;
LONG	id_DiskType;
BPTR	id_VolumeNode;
LONG	id_InUse;
};
#define	ID_WRITE_PROTECTED	80
#define	ID_VALIDATING	81
#define	ID_VALIDATED	82
#define	ID_NO_DISK_PRESENT	(-1)
#define	ID_UNREAD