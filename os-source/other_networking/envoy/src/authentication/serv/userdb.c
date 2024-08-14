/*
**  $Id: userdb.c,v 1.4 92/04/09 12:02:34 dlarson Exp Locker: dlarson $
**
**  Authentication server database functions.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "AuthenticationServer.h"

#if DEBUG > 0
#define B(x) x
#else
#define B(x)
#endif

struct MinList *UserDBList;
USHORT HighUID;

BOOL FlushUserDB(VOID);

/****i* AuthenticationServer/InitUserDB ******************************************
*
*   NAME
*	InitUserDB -- Load the Authentication Server User DataBase into ram.
*
*   SYNOPSIS
*	success = InitUserDB()
*
*	BOOL InitUserDB(VOID);
*
*   FUNCTION
*	Loads the user database from disk into RAM.  This function must be
*	called before any other user authentication server database commands.
*	If successful, you must call FreeUserDB() before exit.
*
*   INPUTS
*	None.
*
*   RESULT
*	success		- TRUE if succedes, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	FlushUserDB, FreeUserDB()
*
*****************************************************************************
*
*/
BOOL InitUserDB(VOID)
{
BPTR file;
struct UserProfile *node;
LONG actual;

	if( !(UserDBList = AllocMem(sizeof(struct MinList), MEMF_ANY)) )
	{
		return FALSE;
	}
	NewList((struct List *)UserDBList);
/*
**  If a FlushUserDB() failed, there may be a PROGDIR:asuserdb.bak
**  but no PROGDIR:asuserdb.  If so, the Rename() will fix that, if
**  not, the Rename() will fail harmlessly because PROGDIR:asuserdb
**  exists.
*/
	Rename("PROGDIR:asuserdb.bak", "PROGDIR:asuserdb");
	if( !(file = Open("PROGDIR:asuserdb", MODE_OLDFILE)) )
	{
		FreeMem(UserDBList, sizeof(struct MinList));
		UserDBList = NULL;
		return FALSE;
	}
	if( (actual = Read(file, &HighUID, sizeof(USHORT))) != sizeof(USHORT) )
	{
		if(actual == 0)  /*  Zero length database is corrupt.  */
		{
			actual = 1;
		}
		goto ERROR;
	}
B(printf("InitUser: HighUID = %d.\n",HighUID));
	do
	{
		if( !(node = AllocMem(sizeof(struct MinUserProfile), MEMF_ANY)) )
		{
			actual = 1;
			break;
		}
		AddHead((struct List *)UserDBList, (struct Node *)node);
		actual = Read(file, &node->up_UserName, PROFILE_DSIZE);
if(actual == PROFILE_DSIZE)
B(printf("InitUser: %s,%s,%s,%d,%d\n", node->up_UserName,node->up_FullName,node->up_PassWord,node->up_UID,node->up_GID));
	} while(actual == PROFILE_DSIZE);
	Remove((struct Node *)node);
B(printf("InitUser() freeing %lx.\n", node));
	FreeMem(node,sizeof(struct MinUserProfile));
ERROR:
	switch(actual)
	{
		case 0:
			Close(file);
			return TRUE;
		case -1:
			PrintFault(IoErr(), NULL);
 			Close(file);
			FreeUserDB();
			return FALSE;
		default:
			printf("Corrupt user database.\n");
 			Close(file);
			FreeUserDB();
			return FALSE;
	}
}


/****i* AuthenticationServer/FreeUserDB ******************************************
*
*   NAME
*	FreeUserDB -- Free the ram copy of the user database.
*
*   SYNOPSIS
*	FreeUserDB()
*
*	VOID FreeUserDB(VOID);
*
*   FUNCTION
*	Frees the ram copy of the user authentication server database.
*	OK to call even if InitUserDBList() not called successfully.
*
*   INPUTS
*	None.
*
*   RESULT
*	None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
VOID FreeUserDB(VOID)
{
struct UserProfile *node;

B(printf("FreeUserDB() called.\n"));
	if(!UserDBList)
	{
		return;
	}
	while(node = (struct UserProfile *)RemHead((struct List *)UserDBList))
	{
		B(printf("FreeUserDB() freeing %lx.\n", node));
		FreeMem(node, sizeof(struct MinUserProfile));
	}
	FreeMem(UserDBList, sizeof(struct MinList));
	UserDBList = NULL;
}


/****i* AuthenticationServer/FlushUserDB ******************************************
*
*   NAME
*	FlushUserDB -- Write user database out to disk.
*
*   SYNOPSIS
*	success = FlushUserDB()
*
*	BOOL FlushUserDB(VOID);
*
*   FUNCTION
*	Writes out the user database to disk.  If the operation fails,
*	PROGDIR:asuserdb may be non-existant.  If so, PROGDIR:asuserdb.bak
*	should still be usable.  This function should be called after every
*	change to the user database.
*
*	Does NOT free the user database in ram.
*
*   INPUTS
*	None.
*
*   RESULT
*	success		- TRUE if successful, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*	Only WriteUser() and RemoveUser() should call this function.
*
*   BUGS
*
*   SEE ALSO
*	InitUserDB()
*
*****************************************************************************
*
*/
BOOL FlushUserDB(VOID)
{
struct UserProfile *node;
LONG returned;
BPTR file;

	if( !(file = Open("PROGDIR:temp_asuserdb", MODE_NEWFILE)) )
	{
		return FALSE;
	}
	if( (returned = Write(file, &HighUID, sizeof(USHORT))) != sizeof(USHORT))
	{
	 	Close(file);
		goto ERROR;
	}
	for(node = (struct UserProfile *)UserDBList->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		if( (returned = Write(file, &node->up_UserName, PROFILE_DSIZE)) !=
	            PROFILE_DSIZE )
		{
			break;
		}
	}
	if(node->up_Node.mln_Succ)  /* If didn't get to end of list */
	{
		goto ERROR;
	}
	Close(file);
	DeleteFile("PROGDIR:asuserdb.bak");
	if( !Rename("PROGDIR:asuserdb", "PROGDIR:asuserdb.bak") )
	{
		printf("Can't rename PROGDIR:asuserdb to PROGDIR:asuserdb.bak.\n");
		returned = -1;
		goto ERROR;
	}
	if( !Rename("PROGDIR:temp_asuserdb", "PROGDIR:asuserdb") )
	{
		printf("Can't rename PROGDIR:temp_asuserdb to PROGDIR:asuserdb.\n");
		returned = -1;
		goto ERROR;
	}
	DeleteFile("PROGDIR:asuserdb.bak");
	return TRUE;
ERROR:
	if(returned == -1)
	{
		PrintFault(IoErr(), NULL);
	} else
	{
		printf("Unintelligible DOS error.\n");
 	}
	return FALSE;
}


/****i* AuthenticationServer/WriteUser ******************************************
*
*   NAME
*	WriteUser -- Writes a user profile to disk.
*
*   SYNOPSIS
*	success = WriteUser(user);
*
*	BOOL WriteUser(struct UserProfile *);
*
*   FUNCTION
*	Writes a user profile to disk.  If the user with this UID already
*	exists, it is overwritten.  If no UID, a new UID is assigned and
*	placed in the structure.
*
*	The ram and disk database will remain consistent even if the write
*	fails.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*  This function is complicated a bit by the effort required to keep the
*  RAM and disk databases consistent even if disk writes fail for some
*  reason. This is very important, however.
*/
BOOL WriteUser(struct UserProfile *user)
{
struct UserProfile *node, *new;
/*
**  Allocate space for a new node and copy user into it:
*/
	if( !(new = AllocMem(sizeof(struct MinUserProfile), MEMF_ANY)) )
	{
		return FALSE;
	}
	CopyMem(user, new, sizeof(struct MinUserProfile));
B(printf("WriteUser: %s,%s,%s,%d,%d\n", new->up_UserName,new->up_FullName,new->up_PassWord,new->up_UID,new->up_GID));
/*
**  Try to find up_UserName in the list:
*/

	for(node = (struct UserProfile *)UserDBList->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		if(Stricmp(user->up_UserName, node->up_UserName) == 0)
			break;
	}
/*
**  If it was there, remove the old one.  If it wasn't, assign a new UID.
**  Either way, add the new node:
*/
	if(node->up_Node.mln_Succ)  /*  If didn't get to end of list  */
	{
		Remove((struct Node *)node);
	} else
	{
		new->up_UID = ++HighUID;
	}
	AddHead((struct List *)UserDBList, (struct Node *)new);
/*
**  Write the updated database to disk.  If successful and there was an
**  old node, free the old node.  If failed, remove the new node and
**  reintroduce the old node (if any):
*/
	if(FlushUserDB())
	{
		if(node->up_Node.mln_Succ)  /* If didn't get to end of list */
		{
		B(printf("WriteUser() freeing %lx.\n", node));
			FreeMem(node, sizeof(struct MinUserProfile));
		}
		CopyMem(new, user, sizeof(struct MinUserProfile));
		return TRUE;
	}
	Remove((struct Node *)new);
	if(node->up_Node.mln_Succ) /* If didn't get to end of list */
	{
		AddHead((struct List *)UserDBList, (struct Node *)node);
	}
B(printf("WriteUser() freeing %lx.\n", new));
	FreeMem(new, sizeof(struct MinUserProfile));
	return FALSE;
}


/****i* AuthenticationServer/ReadUser ******************************************
*
*   NAME
*	ReadUser -- Read a user's profile.
*
*   SYNOPSIS
*	success = ReadUser(user);
*
*	BOOL ReadUser(struct UserProfile *user);
*
*   FUNCTION
*	If the database contains a user with username user->up_UserName, that
*	user's UserProfile is copied into user.  Otherwise, user is left alone.
*
*   INPUTS
*	user		- Pointer to a UserProfile structure containing the
*			  username to read and into which that user's UserProfile
*			  is copied if successful.
*
*   RESULT
*	success		- TRUE if successful, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
BOOL ReadUser(struct UserProfile *user)
{
struct UserProfile *node;

	for(node = (struct UserProfile *)UserDBList->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		if(Stricmp(user->up_UserName, node->up_UserName) == 0)
			break;
	}
	if(!node->up_Node.mln_Succ)  /* If got to end of list */
	{
		return FALSE;
	}
	CopyMem(node, user, sizeof(struct MinUserProfile));
	return TRUE;
}


/****i* AuthenticationServer/ReadUID ******************************************
*
*   NAME
*	ReadUID -- Read a user's profile.
*
*   SYNOPSIS
*	success = ReadUID(user);
*
*	BOOL ReadUID(struct UserProfile *user);
*
*   FUNCTION
*	If the database contains a user with UID user->up_UID, that
*	user's UserProfile is copied into user.  Otherwise, user is left alone.
*
*   INPUTS
*	user		- Pointer to a UserProfile structure containing the
*			  UID to read and into which that user's UserProfile
*			  is copied if successful.
*
*   RESULT
*	success		- TRUE if successful, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
BOOL ReadUID(struct UserProfile *user)
{
struct UserProfile *node;

	for(node = (struct UserProfile *)UserDBList->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		if(user->up_UID == node->up_UID)
			break;
	}
	if(!node->up_Node.mln_Succ)  /* If got to end of list */
	{
		return FALSE;
	}
	CopyMem(node, user, sizeof(struct MinUserProfile));
	return TRUE;
}


/****i* AuthenticationServer/RemoveUser ******************************************
*
*   NAME
*	RemoveUser -- Remove a user from the authentiation user database.
*
*   SYNOPSIS
*	success = RemoveUser(struct UserProfile *user)
*
*	BOOL RemoveUser(struct UserProfile *user);
*
*   FUNCTION
*	Removes a user from the authentication server user database.
*	If the updated database cannot be successfully writen to disk,
*	the deletion does not take effect.
*
*   INPUTS
*	user		- Pointer to UserProfile structure of user to delete.
*
*   RESULT
*	success		- TRUE if successful, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*  This function is complicated a bit by the effort required to keep the
*  RAM and disk databases consistent even if disk writes fail for some
*  reason.  This is very important, however.
*/
BOOL RemoveUser(struct UserProfile *user)
{
struct UserProfile *node;

	for(node = (struct UserProfile *)UserDBList->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		if(Stricmp(user->up_UserName, node->up_UserName) == 0)
			break;
	}
	if(!node->up_Node.mln_Succ) /* If got to end of list */
	{
		return FALSE;
	}
	Remove((struct Node *)node);
	if(FlushUserDB())
	{
		FreeMem(node, sizeof(struct MinUserProfile));
		return TRUE;
	}
	AddHead((struct List *)UserDBList, (struct Node *)node);
	return FALSE;
}


/****i* AuthenticationServer/GetUserList ******************************************
*
*   NAME
*	GetUserList -- Return all records in the authentiation user database.
*
*   SYNOPSIS
*	list = GetUserList()
*
*	struct MinList *GetUserList(VOID);
*
*   FUNCTION
*	Returns a pointer to a list of all users records.
*
*   INPUTS
*	None.
*
*   RESULT
*	list		- Pointer to list of all user's records if successful,
*			  else NULL.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
struct MinList *GetUserList(VOID)
{
	return UserDBList;
}