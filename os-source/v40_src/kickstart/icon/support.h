/*
 * $Id: support.h,v 38.3 92/07/28 13:48:58 mks Exp $
 *
 * $Log:	support.h,v $
 * Revision 38.3  92/07/28  13:48:58  mks
 * Changed prototypes to be FRead/FWrite
 * 
 * Revision 38.2  92/02/26  09:34:15  mks
 * Added two prototypes
 *
 * Revision 38.1  91/06/24  19:02:15  mks
 * Changed to V38 source tree - Trimmed Log
 *
 */

#include	<exec/types.h>
#include	<dos/dos.h>

/*
 * Stubs for EXEC...
 */
void *AllocMemStub(ULONG, ULONG);
void FreeMemStub(void *, ULONG);
void *AllocVecStub(ULONG, ULONG);
void FreeVecStub(void *);

/*
 * Stubs for DOS...
 */
BPTR ParentDirStub(BPTR);
BPTR LockStub(char *,ULONG);
void UnLockStub(BPTR);
LONG FReadStub(BPTR,VOID *,LONG,LONG);
LONG FWriteStub(BPTR,VOID *,LONG,LONG);
LONG SetVBufStub(BPTR,STRPTR,LONG,LONG);
LONG IoErrStub(VOID);
BPTR OpenStub(char *,ULONG);
LONG CloseStub(BPTR);
LONG SetProtectionStub(char *,LONG);
LONG DeleteFileStub(char *);
char *FilePartStub(char *);
char *PathPartStub(char *);

/*
 * Stub for the private UpdateWorkbench LVO
 */
VOID UpdateWorkbenchStub(struct Library *,char *,BPTR,LONG);

/*
 * Stubs for myself...
 */
BOOL AddFreeListStub(struct FreeList *,APTR,ULONG);
struct DiskObject *GetDiskObjectStub(char *);
BOOL PutIconStub(char *,struct DiskObject *);
struct DiskObject *GetDefDiskObjectStub(LONG);
struct OldWBObject *AllocWBObjectStub(VOID);
VOID FreeWBObjectStub(struct OldWBObject *);
VOID FreeDiskObjectStub(struct DiskObject *);
VOID FreeFreeListStub(struct FreeList *);
BOOL GetIconStub(char *,struct DiskObject *,struct FreeList *);


#ifdef DEBUGGING
#define DP( a )		{ kprintf a; }
#define DS( a )		{ a; }
#else
#define DP( a )
#define DS( a )
#endif DEBUGGING

#ifdef MAXDEBUG
#define MP( a )		{ kprintf a; }
#define MS( a )		{ a; }
#else
#define MP( a )
#define MS( a )
#endif MAXDEBUG


#ifdef DEBUGGING
#define ASSERT(ex) {if (!(ex)){setkprintfok();DP(("Assertion failed: %s/%ld <%s>\n", __FILE__, __LINE__, "ex" ));Debug();}}
#else
#define ASSERT(ex) ;
#endif

void kprintf(char *, ...);
