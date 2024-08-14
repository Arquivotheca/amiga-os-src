#ifndef MELT2SIZE
#include "exec/types.h"
#include "exec/ports.h"
#include "exec/lists.h"
#include "libraries/dos.h"
/**
*
* This header file supplies information needed to interface with the
* particular operating system and C compiler being used.
*
**/

/**
*
* The following definitions specify the particular C compiler being used.
*
*	LATTICE		Lattice C compiler
*
*/
#define LATTICE 1
#define LATTICE_V4 1
#define LATTICE_V5 1
/**
*
* The following type definitions take care of the particularly nasty
* machine dependency caused by the unspecified handling of sign extension
* in the C language.  When converting "char" to "int" some compilers
* will extend the sign, while others will not.  Both are correct, and
* the unsuspecting programmer is the loser.  For situations where it
* matters, the new type "byte" is equivalent to "unsigned char".
*
*/
typedef unsigned char byte;


/**
*
* Miscellaneous definitions
*
*/
#define SECSIZ 512		/* disk sector size */

/**
*
* The following symbols define the sizes of file names and node names.
*
*/
#define FNSIZE 108	/* maximum file node size      - DOS limit */
#define FMSIZE 256	/* maximum file name size      - DOS limit */
#define FESIZE 32	/* maximum file extension size - arbitrary */

/**
*
* This structure contains disk size information returned by the getdfs
* function.
*/
#define DISKINFO InfoData

/**
*
* The following structure is used by the dfind and dnext functions to
* hold file information.
*
*/
#define FILEINFO FileInfoBlock

/**
*
* The following structure appears at the beginning (low address) of
* each free memory block.
*
*/
struct MELT
	{
	struct MELT *fwd;	/* points to next free block */
	long size;		/* number of MELTs in this block */
	};
#define MELTSIZE sizeof(struct MELT)

/**
*
* The following structure is used to keep track of currently allocated
* system memory
*
*/
struct MELT2
	{
	struct MELT2 *fwd;	/* points to next block */
	struct MELT2 *bwd;	/* points to previous block */
	unsigned long size;	/* size of this block */
	};
#define MELT2SIZE sizeof(struct MELT2)

/**
*
* The following structures are used with the AmigaDOS fork() and wait()
* functions
*
*/

struct ProcID {				/* packet returned from fork()  */
	struct ProcID *nextID;		/* link to next packet		*/
	struct Process *process;	/* process ID of child		*/
	int UserPortFlag;
	struct MsgPort *parent;		/* termination msg destination	*/
	struct MsgPort *child;		/* child process' task msg port	*/
	BPTR seglist;			/* child process' segment list	*/
	};

struct FORKENV {
	long priority;			/* new process priority		*/
	long stack;			/* stack size for new process	*/
	BPTR std_in;			/* stdin for new process	*/
	BPTR std_out;			/* stdout for new process	*/
	BPTR console;			/* console window for new process */
	struct MsgPort *msgport;	/* msg port to receive termination */
	};				/* message from child		*/

struct TermMsg {			/* termination message from child */
	struct Message msg;
	long class;			/* class == 0			*/
	short type;			/* message type == 0		*/
	struct Process *process;	/* process ID of sending task	*/
	long ret;			/* return value			*/
	};

#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif

extern int forkl __ARGS((char *, char *,...));
extern int forkv __ARGS((char *, char **, struct FORKENV *, struct ProcID *));
extern int wait __ARGS((struct ProcID *));
extern struct ProcID *waitm __ARGS((struct ProcID **));

/**
*
* Level 0 I/O services
*
**/
extern int _dclose __ARGS((long));
extern long _dcreat __ARGS((char *, int));
extern long _dcreatx __ARGS((char *, int));
extern int dfind __ARGS((struct FILEINFO *, char *, int));
extern int dnext __ARGS((struct FILEINFO *));
extern long _dopen __ARGS((char *, int));
extern unsigned _dread __ARGS((long, char *, unsigned));
extern long _dseek __ARGS((long, long, int));
extern unsigned _dwrite __ARGS((long, char *, unsigned));
extern int getcd __ARGS((int,char *));
extern int getdfs __ARGS((char *, struct DISKINFO *));
extern int getfa __ARGS((char *));
extern long getft __ARGS((char *));
extern int getpath __ARGS((BPTR, char *));
extern BPTR findpath __ARGS((char *));
/**
*
* Miscellaneous external definitions
*
*/
extern int datecmp __ARGS((long[], long[]));
extern int chgclk __ARGS((unsigned char *));
extern void getclk __ARGS((unsigned char *));
extern int onbreak __ARGS((int __ARGS((*)) __ARGS(())));
extern int poserr __ARGS((char *));
extern void chkabort __ARGS((void));
extern void Chk_Abort __ARGS((void));
struct DeviceList *getasn __ARGS((char *));

#define geta4 __builtin_geta4
extern void geta4 __ARGS((void));

#define getreg __builtin_getreg
extern long getreg __ARGS((int));

#define putreg __builtin_putreg
extern void putreg __ARGS((int, long));

#define __emit __builtin_emit
extern void __emit __ARGS((int));

#define REG_D0 0
#define REG_D1 1
#define REG_D2 2
#define REG_D3 3
#define REG_D4 4
#define REG_D5 5
#define REG_D6 6
#define REG_D7 7
#define REG_A0 8
#define REG_A1 9
#define REG_A2 10
#define REG_A3 11
#define REG_A4 12
#define REG_A5 13
#define REG_A6 14
#define REG_A7 15

extern int _OSERR;
extern int os_nerr;
extern struct { short err_no; char *msg; } os_errlist[];

#endif
