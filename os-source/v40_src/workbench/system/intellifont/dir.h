/*
**	$Id: dir.h,v 37.6 91/03/11 14:21:19 kodiak Exp $
**
**	Fountain/dir.h -- structure for directory searches
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/


/* for use with MyDirStart, MyDirNext, MyDirEnd */

struct MyDir {
    struct FileInfoBlock md_FIB;
    char    md_BDir[256];		/* enough to hold a full BSTR */
    char    md_Path[256];		/* enough to hold full path */
    struct DevProc *md_pDVP;
    BPTR    md_DirLock;
    BPTR    md_OldCurrDir;		/* from MyDirStart call */
    char   *md_File;			/* pointer inside md_Path */
};

struct MyDir *MyDirStart(char *);
int MyDirNext(struct MyDir *);
void MyDirEnd(struct MyDir *);
