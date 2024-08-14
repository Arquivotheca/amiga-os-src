/* ram.h */

#include "dos/notify.h"

#define BLKSHIFT	10			/* 1K blocks */
#define MIN_BLKSIZE	(1L << BLKSHIFT)
#define FIRSTBUFFPOS	8	/* offsetof(struct data,data) */
#define DATA_EXTRA	(FIRSTBUFFPOS >> 2)
#define MAX_WRITE	(FIRSTBUFFPOS+MIN_BLKSIZE-1)
#define DATA_BLOCK_SIZE ((MIN_BLKSIZE >> 2) + DATA_EXTRA)
#define MAX_FILENAME	30
#define MAX_COMMENT	79

#define xfer(a,b,c)	CopyMem(a,b,c)
#define TOBPTR(x)	(((LONG) (x)) >> 2)

/* internal notify structure */
struct notify {
	struct notify *Succ;
	struct notify *Pred;
	char *Name;		   /* pointer to last portion of name */
	struct NotifyRequest *Req; /* we return it when removed */
	struct node *node;	   /* NULL if not in a chain */
	struct NotifyMessage *msg; /* for WAIT_REPLY */
};

struct rlock {
	struct rlock *next;
	struct rlock *prev;
	struct lock *lock;	/* the filehandle that holds this */
	LONG offset;
	LONG length;
	LONG mode;
};

struct rlock_wait {
	struct rlock_wait *next;
	struct rlock_wait *prev;
	struct timerequest iob;  /* the timer ior for this lock attempt */
	struct DosPacket *dp;	  /* the packet sent to me */
};

/* file or directory */
struct node {
	struct node *next;		/* next in chain */
	struct node *back;		/* parent node */
	struct node *list;		/* also struct data * */
	struct node *firstlink;		/* first hardlinked item */
	ULONG size;
	ULONG prot;
	ULONG date[3];
	UBYTE *comment;
	struct MinList notify_list;	/* people waiting for notifies */
	struct MinList record_list;	/* record locks on this file   */
	struct MinList rwait_list;	/* people waiting for record locks */
	ULONG delcount;			/* bumped for each delete or rename */
	BYTE type;
	UBYTE name[MAX_FILENAME+1];
};

/* data block */
struct data {
	struct data *next;
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
	ULONG delcount;		/* for exall: copy of dir's delcount */
};

#define LOCK_MODIFY	1	/* flag for lock structure */

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
extern	LONG disktype;
extern  struct Process *MyProc;
extern  struct MsgPort *MyPort;
extern  struct MsgPort *MyReplyPort;
extern  struct MsgPort *timerport;

extern	struct MinList orphaned;
extern	struct MinList FreeMessages;



#define makefile(a,b,c) openmakefile(a,b,c,0)
#define openfile(a,b,c) openmakefile(a,b,c,1)
#define copydate(a,b)	{(a)[0] = (b)[0]; (a)[1] = (b)[1]; (a)[2] = (b)[2];}

#define DOS_TRUE	-1
#define SAME 		0

