/* Prevent Lint from complaining about ANSI prototype extensions */
#ifdef _lint

#define __asm
#define __stdargs
#define __regargs
#define R_D0
#define R_D1
#define R_A0

#else

#define R_D0	register __d0
#define R_D1	register __d1
#define R_A0	register __a0

#endif

#include <dos.h>
#include <libraries/dosextens.h>

/*lint -save	*/
/*lint -library */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <proto/exec.h>
#include <proto/dos.h>
/*lint -restore	*/

extern int tolower (char);

#define MEMF_PUBLIC (1L<<0)
#define MEMF_CHIP   (1L<<1)
#define MEMF_FAST   (1L<<2)
#define MEMF_CLEAR  (1L<<16)

/* Structure used to hold file info in a linked list */
struct FibEntry
{
  struct MinNode fe_Node;
  struct FileInfoBlock *fe_Fib;
};

/* Flag bits for LSFLAGS */
#define BREAKFLAG	(1L << 0)
#define CONSOLE		(1L << 1)
#define SHOWDIRS	(1L << 2)
#define SHOWFILES	(1L << 3)
#define LISTALL		(1L << 4)
#define LONGLIST	(1L << 5)
#define NOSORTFLAG	(1L << 6)
#define NOTEFLAG	(1L << 7)
#define PATHNAMED	(1L << 8)
#define REVFLAG		(1L << 9)
#define LSUNUSEDFLAG	(1L << 10)
#define FULLPATHNAMES	(1L << 11)
#define ANTIMATCH	(1L << 12)
#define TOTALIZE	(1L << 13)
#define NOHEADERS	(1L << 14)
#define NOINTERACT	(1L << 15)
#define FILESFIRST	(1L << 16)
#define MIXFILESDIRS	(1L << 17)
#define SHOWOLDERTHAN	(1L << 18)
#define SHOWNEWERTHAN	(1L << 19)


/* this is a FileInfoBlock with added stuff for NFS */

struct MyInfoBlock {
   LONG	  fib_DiskKey;
   LONG	  fib_DirEntryType;  /* Type of Directory. If < 0, then a plain file.
			      * If > 0 a directory */
   char	  fib_FileName[108]; /* Null terminated. Max 30 chars used for now */
   LONG	  fib_Protection;    /* bit mask of protection, rwxd are 3-0.	   */
   LONG	  fib_EntryType;
   LONG	  fib_Size;	     /* Number of bytes in file */
   LONG	  fib_NumBlocks;     /* Number of blocks in file */
   struct DateStamp fib_Date;/* Date file last changed */
   char	  fib_Comment[80];  /* Null terminated comment associated with file */
   LONG   is_remote;		/* 1 if remote file, 0 otherwise */
   LONG   uid;				/* userid */
   LONG   gid;				/* group id */
   LONG   mode;				/* mode */
   char	  fib_Reserved[20]; /* pad to size of real FileInfoBlock */
}; /* MyInfoBlock */

#define NFSMASK (NFSMODE_LNK | NFSMODE_DIR | NFSMODE_CHR | NFSMODE_BLK | NFSMODE_SOCK)
#define NFS_OWN_RDPERM 0400
#define NFS_OWN_WRPERM 0200
#define NFS_OWN_EXPERM 0100
#define NFS_GID_RDPERM 0040
#define NFS_GID_WRPERM 0020
#define NFS_GID_EXPERM 0010
#define NFS_PUB_RDPERM 0004
#define NFS_PUB_WRPERM 0002
#define NFS_PUB_EXPERM 0001
#define NFS_SETUID 04000
#define NFS_SETGID 02000
