/* ram.h */

#define BLKSHIFT	20			/* 1Meg blocks */
#define MIN_BLKSIZE	(1L << BLKSHIFT)
#define FIRSTBUFFPOS	8	/* offsetof(struct data,data) */
#define DATA_EXTRA	(FIRSTBUFFPOS >> 2)
#define MAX_WRITE	(FIRSTBUFFPOS+MIN_BLKSIZE-1)
#define DATA_BLOCK_SIZE ((MIN_BLKSIZE >> 2) + DATA_EXTRA)
#define MAX_FILENAME	30
#define MAX_COMMENT	79

#define xfer(a,b,c)	CopyMem(a,b,c)
#define TOBPTR(x)	(((LONG) (x)) >> 2)

/* file or directory */
struct node {
	struct node *next;		/* next in chain */
	struct node *back;		/* parent node */
	struct node *list;		/* also struct data * */
	ULONG size;
	ULONG prot;
	ULONG date[3];
	UBYTE *comment;
	UBYTE *name;
	BYTE type;
};

/* data block */
struct data {
	struct data *next_obsolete;	/* ALWAYS NULL! */
	ULONG size;		  /* DATA_EXTRA longs before data */
	char  data[MIN_BLKSIZE];  /* may actually be less */
};

/* my extended lock structure */
struct lock {
	struct FileLock lock;	/* fl_Key is ptr to node */
	struct data *block;	/* current block of data */
	ULONG offset;		/* offset within block */
	ULONG cpos;		/* position within file */
	ULONG flags;		/* random flags */
};

/* global structures */
extern	struct Library *SysBase;
extern	struct DosLibrary *DOSBase;
extern	struct Library *UtilityBase;

extern	BPTR lock_list;
extern	struct node *root;
extern	struct node *current_node;

extern	LONG path_pos;

extern	LONG spaceused;
extern	LONG fileerr;
extern	struct DeviceList *volumenode;

extern  struct Process *MyProc;
extern  struct MsgPort *MyPort;

extern	struct MinList orphaned;
extern	struct MinList FreeMessages;

#define copydate(a,b)	{(a)[0] = (b)[0]; (a)[1] = (b)[1]; (a)[2] = (b)[2];}

#define DOS_TRUE	-1
#define SAME 		0

