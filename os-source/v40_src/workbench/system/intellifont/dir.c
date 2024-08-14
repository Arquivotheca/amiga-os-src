/*
**	$Id: dir.c,v 37.8 92/02/10 10:49:55 davidj Exp $
**
**	Fountain/dir.c -- robust directory search
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

#define	DB(x)	;
#define	DEBUG0	FALSE

extern struct Library *SysBase;
extern struct Library *DOSBase;

int myDirNextDVP (md)
    struct MyDir *md;
{

    DBG1 ("myDirNextDVP(\"0x%lx\")\n", md);
    DBG1 ("md->md_OldCurrDir=0x%lx\n", md->md_OldCurrDir);
    CurrentDir (md->md_OldCurrDir);
    do
    {
	if (md->md_DirLock)
	{
	    UnLock (md->md_DirLock);
	    md->md_DirLock = 0;
	}
	md->md_pDVP = GetDeviceProc (md->md_Path, md->md_pDVP);
	if (!md->md_pDVP)
	    return (FALSE);
#if 0
	DBG1 ("  assign flag %ld\n", (md->md_pDVP->dvp_Flags & DVPF_ASSIGN) ? 1 : 0);
	{
	    char dvpPath[256];

	    NameFromLock (md->md_pDVP->dvp_Lock, dvpPath, 256);
	    DBG2 ("  DoPkt3(,,\"%s\", \"%s\",)\n", dvpPath, md->md_BDir + 1);
	}
#endif
	md->md_DirLock = DoPkt3 (md->md_pDVP->dvp_Port, ACTION_LOCATE_OBJECT,
				 md->md_pDVP->dvp_Lock, MKBADDR (md->md_BDir), SHARED_LOCK);
	if (!md->md_DirLock)
	    continue;
	if ((Examine (md->md_DirLock, &md->md_FIB)) &&
	    (md->md_FIB.fib_DirEntryType >= 0))
	{
	    return (TRUE);
	}
    }
    while (md->md_pDVP->dvp_Flags & DVPF_ASSIGN);

    return (FALSE);
}


void
 MyDirEnd (md)
    struct MyDir *md;
{

    DBG1 ("MyDirEnd(0x%lx)\n", md);
    if (md)
    {
	CurrentDir (md->md_OldCurrDir);
	UnLock (md->md_DirLock);
	FreeDeviceProc (md->md_pDVP);
	free (md);
    }
    DBG1 ("MyDirEnd return\n", NULL);
}


struct MyDir *MyDirStart (path)
    char *path;
{
    struct MyDir *md;
    char *s;
    int n;

    DBG1 ("MyDirStart(\"%s\")\n", path);
    md = (struct MyDir *) malloc (sizeof (struct MyDir));
    if (md)
    {
	/* copy full path */
	strcpy (md->md_Path, path);

	/* corrupt path to sans-handler part for BDir creation */
	if (s = strchr (md->md_Path, ':'))
	    path = s + 1;
	strcpy (md->md_BDir + 1, path);
	md->md_BDir[0] = strlen (path);

	/* get md_File */
	n = strlen (md->md_Path);
	if ((md->md_Path[n - 1] != ':') && (md->md_Path[n - 1] != '/'))
	{
	    md->md_Path[n++] = '/';
	    md->md_Path[n] = '\0';
	}
	md->md_File = md->md_Path + n;

	/* clear previous dir info */
	md->md_pDVP = 0;
	md->md_DirLock = 0;

	/* save CurrentDir */
	md->md_OldCurrDir = CurrentDir (0);
#if 0
	{
	    char dvpPath[256];

	    NameFromLock (md->md_OldCurrDir, dvpPath, 256);
	    DBG1 ("  **OldCurrDir: \"%s\"\n", dvpPath);
	}
#endif

	/* prime MyDirNext */
	if (!myDirNextDVP (md))
	{
	    MyDirEnd (md);
	    return (0);
	}
    }
    DBG1 ("MyDirStart returns 0x%lx\n", md);
    return (md);
}


int
 MyDirNext (md)
    struct MyDir *md;
{
    BPTR lock1, lock2;
    int success;

    while ((md->md_pDVP) && (md->md_DirLock))
    {
	/* get next candidate */
	while (ExNext (md->md_DirLock, &md->md_FIB))
	{
	    /* verify that this is reachable */
	    strcpy (md->md_File, md->md_FIB.fib_FileName);
	    success = FALSE;
	    CurrentDir (md->md_OldCurrDir);
	    lock1 = Lock (md->md_Path, SHARED_LOCK);
	    if (lock1)
	    {
		CurrentDir (md->md_DirLock);
		lock2 = Lock (md->md_FIB.fib_FileName, SHARED_LOCK);
		if (lock2)
		{
		    success = (SameLock (lock1, lock2) == LOCK_SAME);
		    UnLock (lock2);
		}
		UnLock (lock1);
	    }
	    if (success)
		return (TRUE);
	}
	if (!(md->md_pDVP->dvp_Flags & DVPF_ASSIGN))
	    break;
	myDirNextDVP (md);
    }
    return (FALSE);
}


MyFileSize (char *path)
{
    UWORD fibBuffer[sizeof (struct FileInfoBlock) / 2 + 1];
    struct FileInfoBlock *fib;
    int result;
    BPTR lock;

    result = 0;
    lock = Lock (path, SHARED_LOCK);
    DB (kprintf ("Lock [%s] = 0x%lx\n", path, lock));
    if (lock)
    {
	fib = (struct FileInfoBlock *) ((((ULONG) fibBuffer) + 2) & 0xfffffffc);
	if (Examine (lock, fib))
	{
	    result = fib->fib_Size;
	}
#if DEBUG0
	else
	{
	    kprintf ("MyFileSize Examine failed\n");
	}
#endif
	UnLock (lock);
    }
#if DEBUG0
    else
    {
	kprintf ("MyFileSize Lock %s failed\n", path);
    }
#endif
    DB (kprintf ("MyFileSize returns %ld\n", result));
    return (result);
}
