/*
 * $Id: support.h,v 38.8 92/11/12 08:10:04 mks Exp $
 *
 * $Log:	support.h,v $
 * Revision 38.8  92/11/12  08:10:04  mks
 * Fixed a problem with one of the compiler include files
 * 
 * Revision 38.7  92/06/11  15:48:28  mks
 * Added FORBID and PERMIT macros that use the stubs
 *
 * Revision 38.6  92/05/30  18:03:42  mks
 * Added ThisTask() stub...  (It calls FindTask(NULL))
 *
 * Revision 38.5  92/05/30  16:36:45  mks
 * Added some stub definitions
 *
 * Revision 38.4  92/05/14  17:12:54  mks
 * Added prototype for index()
 *
 * Revision 38.3  92/03/10  11:33:20  mks
 * Added support for NameFromLock()
 *
 * Revision 38.2  91/11/12  18:43:33  mks
 * Updated as needed
 *
 * Revision 38.1  91/06/24  11:38:53  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#ifdef index
#undef index
#endif	/* We don't want the index from the strings.h file */

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
#define ASSERT(ex) {if (!(ex)){setkprintfok();DP(("Assertion failed: %s/%ld <%s>\n", __FILE__, __LINE__, "ex" ));Debug(0L);}}
#else
#define ASSERT(ex) ;
#endif

#ifdef	ALLOCDEBUG

#define	ALLOCMEM(a,b)	myallocmem(a,b)
#define	FREEMEM(a,b)	myfreemem(a,b)
#define	FREEENTRY(a)	myfreeentry(a)

/* These should be able to go into debugging... */
#define	ALLOCVEC(a,b)	AllocVec(a,b)
#define	FREEVEC(a)	FreeVec(a)

#else

#ifdef	USE_STUBS

#define	ALLOCMEM(a,b)	AllocMemStub(a,b)
#define	FREEMEM(a,b)	FreeMemStub(a,b)

#define	ALLOCVEC(a,b)	AllocVecStub(a,b)
#define	FREEVEC(a)	FreeVecStub(a)

#define	GETMSG(a)	GetMsgStub(a)
#define	WAITPORT(a)	WaitPortStub(a)

void *AllocMemStub(ULONG, ULONG);
void FreeMemStub(void *, ULONG);

void *AllocVecStub(ULONG, ULONG);
void FreeVecStub(void *);

struct Message *GetMsgStub(struct MsgPort *);
struct Message *WaitPortStub(struct MsgPort *);

#else

#define	ALLOCMEM(a,b)	AllocMem(a,b)
#define	FREEMEM(a,b)	FreeMem(a,b)
#define	ALLOCVEC(a,b)	AllocVec(a,b)
#define	FREEVEC(a)	FreeVec(a)
#define	GETMSG(a)	GetMsg(a)
#define	WAITPORT(a)	WaitPort(a)

#endif	/* USE_STUBS */

#define	FREEENTRY(a)	FreeEntry(a)

#endif	/* ALLOCDEBUG */

/*********************************************************************/

#ifdef	USE_STUBS

#define	REMOVE(a)		RemoveStub(a)
#define	ADDHEAD(a,b)		AddHeadStub(a,b)
#define	ADDTAIL(a,b)		AddTailStub(a,b)
#define	OPENLIBRARY(a,b)	OpenLibraryStub(a,b)
#define	CLOSELIBRARY(a)		CloseLibraryStub(a)
#define	PUTMSG(a,b)		PutMsgStub(a,b)
#define REPLYMSG(a)		ReplyMsgStub(a)

void RemoveStub(struct Node *node);
void AddHeadStub(struct List *list,struct Node *node);
void AddTailStub(struct List *list,struct Node *node);
struct Library *OpenLibraryStub(char *,ULONG);
void CloseLibraryStub(struct Library *);
void PutMsgStub(struct MsgPort *,struct Message *);
void ReplyMsgStub(struct Message *);

#else

#define	REMOVE(a)		Remove(a)
#define	ADDHEAD(a,b)		AddHead(a,b)
#define	ADDTAIL(a,b)		AddTail(a,b)
#define	OPENLIBRARY(a,b)	OpenLibrary(a,b)
#define	CLOSELIBRARY(a)		CloseLibrary(a)
#define	PUTMSG(a,b)		PutMsg(a,b)
#define REPLYMSG(a)		ReplyMsg(a)

#endif	/* USE_STUBS */

/*********************************************************************/

#ifdef	LOCKDEBUG

#define	PARENTDIR(a)	myparentdir(a)
#define	DUPLOCK(a)	myduplock(a)
#define	CREATEDIR(a)	mycreatedir(a)
#define	LOCK(a,b)	mylock(a,b)
#define	UNLOCK(a)	myunlock(a)
#define	CURRENTDIR(a)	mycurrentdir(a)

#else

#ifdef	USE_STUBS

#define	PARENTDIR(a)	ParentDirStub(a)
#define	DUPLOCK(a)	DupLockStub(a)
#define	CREATEDIR(a)	CreateDirStub(a)
#define	LOCK(a,b)	LockStub(a,b)
#define	UNLOCK(a)	UnLockStub(a)
#define	CURRENTDIR(a)	CurrentDirStub(a)

BPTR ParentDirStub(BPTR);
BPTR DupLockStub(BPTR);
BPTR CreateDirStub(char *);
BPTR LockStub(char *,ULONG);
void UnLockStub(BPTR);
BPTR CurrentDirStub(BPTR);

#else

#define	PARENTDIR(a)	ParentDir(a)
#define	DUPLOCK(a)	DupLock(a)
#define	CREATEDIR(a)	CreateDir(a)
#define	LOCK(a,b)	Lock(a,b)
#define	UNLOCK(a)	UnLock(a)
#define	CURRENTDIR(a)	CurrentDir(a)

#endif	/* USE_STUBS */

#endif	/* LOCKDEBUG */

/*********************************************************************/

#ifdef	USE_STUBS

#define	OPEN_fh(a,b)		OpenStub(a,b)
#define	CLOSE_fh(a)		CloseStub(a)
#define	READ_fh(a,b,c)		ReadStub(a,b,c)
#define	WRITE_fh(a,b,c)		WriteStub(a,b,c)
#define	SEEK_fh(a,b,c)		SeekStub(a,b,c)
#define	IOERR(x)		IoErrStub()
#define	SAMELOCK(a,b)		SameLockStub(a,b)
#define	EXAMINE(a,b)		ExamineStub(a,b)
#define	EXNEXT(a,b)		ExNextStub(a,b)
#define	DELETEFILE(a)		DeleteFileStub(a)
#define	RENAME(a,b)		RenameStub(a,b)
#define	SETPROTECTION(a,b)	SetProtectionStub(a,b)
#define	SETCOMMENT(a,b)		SetCommentStub(a,b)
#define	CREATENEWPROC(a)	CreateNewProcStub(a)
#define	CREATEPROC(a,b,c,d)	CreateProcStub(a,b,c,d)
#define	STRTOLONG(a,b)		StrToLongStub(a,b)
#define	SAMEDEVICE(a,b)		SameDeviceStub(a,b)
#define	PATHPART(a)		PathPartStub(a)
#define	FILEPART(a)		FilePartStub(a)
#define	ADDPART(a,b,c)		AddPartStub(a,b,c)
#define	SETFILEDATE(a,b)	SetFileDateStub(a,b)
#define	NAMEFROMLOCK(a,b,c)	NameFromLockStub(a,b,c)
#define	FORBID			ForbidStub()
#define	PERMIT			PermitStub()

BPTR OpenStub(char *,ULONG);
LONG CloseStub(BPTR);
LONG IoErrStub(void);
LONG SameLockStub(BPTR,BPTR);
LONG ExamineStub(BPTR,struct FileInfoBlock *);
LONG ExNextStub(BPTR,struct FileInfoBlock *);
LONG ReadStub(BPTR,APTR,long);
LONG WriteStub(BPTR,APTR,long);
LONG SeekStub(BPTR,long,long);
LONG DeleteFileStub(char *);
LONG RenameStub(char *,char *);
LONG SetProtectionStub(char *,ULONG);
LONG SetCommentStub(char *,char *);
struct Process *CreateNewProcStub(struct TagItem *);
struct MsgPort *CreateProcStub(char *,long,long,BPTR);
LONG StrToLongStub(char *,long *);
LONG SameDeviceStub(BPTR,BPTR);
char *PathPartStub(char *);
char *FilePartStub(char *);
LONG AddPartStub(char *,char *,ULONG);
VOID ForbidStub(VOID);
VOID PermitStub(VOID);

#else

#define	OPEN_fh(a,b)		Open(a,b)
#define	CLOSE_fh(a)		Close(a)
#define	READ_fh(a,b,c)		Read(a,b,c)
#define	WRITE_fh(a,b,c)		Write(a,b,c)
#define	SEEK_fh(a,b,c)		Seek(a,b,c)
#define	IOERR(x)		IoErr()
#define	SAMELOCK(a,b)		SameLock(a,b)
#define	EXAMINE(a,b)		Examine(a,b)
#define	EXNEXT(a,b)		ExNext(a,b)
#define	DELETEFILE(a)		DeleteFile(a)
#define	RENAME(a,b)		Rename(a,b)
#define	SETPROTECTION(a,b)	SetProtection(a,b)
#define	SETCOMMENT(a,b)		SetComment(a,b)
#define	CREATENEWPROC(a)	CreateNewProc(a)
#define	CREATEPROC(a,b,c,d)	CreateProc(a,b,c,d)
#define	STRTOLONG(a,b)		StrToLong(a,b)
#define	SAMEDEVICE(a,b)		SameDevice(a,b)
#define	PATHPART(a)		PathPart(a)
#define	FILEPART(a)		FilePart(a)
#define	ADDPART(a,b,c)		AddPart(a,b,c)
#define	SETFILEDATE(a,b)	SetFileDateStub(a,b)
#define	NAMEFROMLOCK(a,b,c)	NameFromLockStub(a,b,c)
#define	FORBID			Forbid()
#define	PERMIT			Permit()

#endif	/* USE_STUBS */

LONG SetFileDateStub(char *,struct DateStamp *);
LONG NameFromLockStub(BPTR,char *,long);
void SetResult2(LONG);
void AlphaEnqueue(struct List *,struct Node *);
char *index(char *,long);
struct Task *ThisTask(void);
