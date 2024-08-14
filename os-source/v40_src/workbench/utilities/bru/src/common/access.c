/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	access.c    routines for testing file accessibility
 *
 *  SCCS
 *
 *	@(#)access.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Since bru is owned by root and runs with set-user-id bit
 *	set, it must be extra careful about allowing access to files.
 *	These routines test for accessibility using the real user id
 *	and not the effective user id.
 *
 */

#include "globals.h"


/*
 *  FUNCTION
 *
 *	file_access    test for file accessible with given mode
 *
 *  SYNOPSIS
 *
 *	BOOLEAN file_access (name, amode, flag)
 *	char *name;
 *	int amode;
 *	BOOLEAN flag;
 *
 *  DESCRIPTION
 *
 *	Uses the process real user id and real group id to test the
 *	pathname pointed to by "name" for access in the given "amode".
 *	If access is denied and "flag" is TRUE, the user will be
 *	notified that access was denied.
 *
 *	This routine is very important because bru must run with
 *	the "set user id" bit on, and be owned by root, in order
 *	to make directories and special files.  This can be a
 *	serious security loophole since now bru itself must
 *	take responsibility for denying the user access to
 *	any files he would not normally be able to access.
 *
 *	Since the system "access" call uses the real id's instead
 *	of the effective id's, this function should be used to
 *	carefully control access to nodes in the filesystem.
 *
 *	Note that the problem could be "solved" by forking
 *	"mkdir" or "mknod" as appropriate, but doing it ourselves
 *	is noticeably faster and gets around some sticky
 *	gotcha's with misleading error messages and permissions.
 *
 *	4.2 BSD symbolic links introduce lots of problems.  Since the
 *	system "access" routine goes through them to the real file, if a
 *	symbolic link points to a non-existant file, "access" will fail.
 *	This is not quite desirable.  So, what we'll do is try a readlink
 *	on the file. If it succeeds, and the test was for existance or
 *	readability, then return true, otherwise false. (If a symlink exists,
 *	another call to symlink on it [i.e. "writing" to it] will fail.)
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin file_access
 *	    Result is FALSE
 *	    If readlink on file succeeds then
 *		If access was read or existance then
 *		    Result is TRUE
 *		End if
 *	    Else if specified access is allowed then
 *		Result is TRUE
 *	    End if
 *	    If Result is false and error reporting enabled then
 *		Switch on type of access mode
 *		    Case "write access":
 *			Report write access failure
 *			Break
 *		    Case "read access":
 *			Report read access failure
 *			Break
 *		    Default:
 *			Report file does not exist
 *			Break
 *		End switch
 *	    End if
 *	    Return result
 *	End file_access
 *
 */


BOOLEAN file_access (name, amode, flag)
char *name;
int amode;
BOOLEAN flag;
{
    int rtnval = FALSE;
    char buf[BRUPATHMAX];

    DBUG_ENTER ("file_access");
    DBUG_PRINT ("faccess", ("test %s for access %d", name, amode));
    if (s_readlink (name, buf, sizeof (buf)) != SYS_ERROR) {
	if (amode != A_WRITE) {
	    rtnval = TRUE;
	}
    } else if (s_access (name, amode) != SYS_ERROR) {
	rtnval = TRUE;
    }
    if (!rtnval && flag) {
	switch (amode) {
	    case A_WRITE:
		bru_message (MSG_WACCESS, name);
		break;
	    case A_READ:
		bru_message (MSG_RACCESS, name);
		break;
	    default:
		bru_message (MSG_EACCESS, name);
		break;
	}
    }
    DBUG_PRINT ("faccess", ("result is %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	dir_access    check the parent directory of a node for access
 *
 *  SYNOPSIS
 *
 *	BOOLEAN dir_access (name, amode, flag)
 *	char *name;
 *	int amode;
 *	BOOLEAN flag;
 *
 *  DESCRIPTION
 *
 *	Uses the process real user id and real group id to test the
 *	parent directory of the pathname pointed to by "name" for
 *	access in the given "amode".  If access is denied and
 *	"flag" is TRUE, the user will be notified that access
 *	was denied.
 *
 *	Returns TRUE or FALSE for access permitted or denied
 *	respectively.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin dir_access
 *	    Initialize default result to FALSE
 *	    If name pointer is trashed then
 *		Warn user about internal bug
 *	    Else
 *		Find pointer to last slash in name
 *		If no slash in name
 *		    Test "." for access
 *		Else
 *		    Replace slash with end of string char
 *		    If was a leading slash then
 *			Test "/" for access
 *		    Else
 *			Test the stem portion of name
 *		    End if
 *		    Restore the slash
 *		End if
 *	    End if
 *	    Return result
 *	End dir_access
 *
 */

BOOLEAN dir_access (name, amode, flag)
char *name;
int amode;
BOOLEAN flag;
{
    char saveit;
    char *endp;
    BOOLEAN result;

    DBUG_ENTER ("dir_access");
    DBUG_PRINT ("daccess", ("test parent of %s for access %d", name, amode));
    result = FALSE;
    if (name == NULL) {
	bru_message (MSG_BUG, "dir_access");
    } else {
	endp = s_strrchr (name, '/');
#if amigados
	if (endp == NULL) {
	    endp = s_strrchr (name, ':');
	    if (endp != NULL) {
		endp++;
	    }
	}
#endif
	if (endp == NULL) {
#if amigados
	    result = TRUE;
#else
	    result = file_access (".", amode, flag);
#endif
	} else {
	    saveit = *endp;
	    *endp = EOS;
	    if (*name == EOS) {
		result = file_access ("/", amode, flag);
	    } else {
		result = file_access (name, amode, flag);
	    }
	    *endp = saveit;
	}
    }
    DBUG_PRINT ("daccess", ("result is %d", result));
    DBUG_RETURN (result);
}
