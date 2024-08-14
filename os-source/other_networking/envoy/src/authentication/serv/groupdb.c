/*
**  $Id: groupdb.c,v 1.4 92/04/09 12:00:56 dlarson Exp Locker: dlarson $
**
**  Authentication server database functions.
**
**  Now that groups are fixed length, should be able to combine groupdb.c
**  and userdb.c into wrappers for a set of general database functions.
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

struct MinList *GroupDBList;
USHORT HighGID;

BOOL FlushGroupDB(VOID);

/****i* AuthenticationServer/InitGroupDB ******************************************
*
*   NAME
*	InitGroupDB -- Load the Authentication Server Group DataBase into ram.
*
*   SYNOPSIS
*	success = InitGroupDB()
*
*	BOOL InitGroupDB(VOID);
*
*   FUNCTION
*	Loads the group database from disk into RAM.  This function must be
*	called before any other group authentication server database commands.
*	If successful, you must call FreeGroupDB() before exit.
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
*	FlushGroupDB, FreeGroupDB()
*
*****************************************************************************
*
*/
BOOL InitGroupDB(VOID)
{
BPTR file;
struct Group *node;
LONG actual;

	if( !(GroupDBList = AllocMem(sizeof(struct MinList), MEMF_ANY)) )
	{
		return FALSE;
	}
	NewList((struct List *)GroupDBList);
/*
**  If a FlushGroupDB() failed, there may be a PROGDIR:asgroupdb.bak
**  but no PROGDIR:asgroupdb.  If so, the Rename() will fix that, if
**  not, the Rename() will fail harmlessly because PROGDIR:asgroupdb
**  exists.
*/
	Rename("PROGDIR:asgroupdb.bak", "PROGDIR:asgroupdb");
	if( !(file = Open("PROGDIR:asgroupdb", MODE_OLDFILE)) )
	{
		FreeMem(GroupDBList, sizeof(struct MinList));
		GroupDBList = NULL;
		return FALSE;
	}
	if( (actual = Read(file, &HighGID, sizeof(USHORT))) != sizeof(USHORT) )
	{
		if(actual == 0)  /*  Zero length database is corrupt.  */
		{
			actual = 1;
		}
		goto ERROR;
	}
B(printf("InitGroup: HighGID = %d.\n",HighGID));
	do
	{
		if( !(node = AllocMem(sizeof(struct Group), MEMF_ANY)) )
		{
			actual = 1;
			break;
		}
		AddHead((struct List *)GroupDBList, (struct Node *)node);
		actual = Read(file, &node->g_GroupName, GROUP_DSIZE);
if(actual == GROUP_DSIZE)
B(printf("InitGroup(): %s:%d -- %d,%d,%d\n", node->g_GroupName,node->g_GID,node->g_UserMembers[0],node->g_UserMembers[1],node->g_UserMembers[2]));
	} while(actual == GROUP_DSIZE);
	Remove((struct Node *)node);
	FreeMem(node,sizeof(struct Group));
ERROR:
	switch(actual)
	{
		case 0:
			Close(file);
			return TRUE;
		case -1:
			PrintFault(IoErr(), NULL);
 			Close(file);
			FreeGroupDB();
			return FALSE;
		default:
			printf("Corrupt group database.\n");
 			Close(file);
			FreeGroupDB();
			return FALSE;
	}
}


/****i* AuthenticationServer/FreeGroupDB ******************************************
*
*   NAME
*	FreeGroupDB -- Free the ram copy of the group database.
*
*   SYNOPSIS
*	FreeGroupDB()
*
*	VOID FreeGroupDB(VOID);
*
*   FUNCTION
*	Frees the ram copy of the group authentication server database.
*	OK to call even if InitGroupDBList() not called successfully.
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
VOID FreeGroupDB(VOID)
{
struct Group *node;

	if(!GroupDBList)
	{
		return;
	}
	while(node = (struct Group *)RemHead((struct List *)GroupDBList))
	{
		FreeMem(node, sizeof(struct Group));
	}
	FreeMem(GroupDBList, sizeof(struct MinList));
	GroupDBList = NULL;
}


/****i* AuthenticationServer/FlushGroupDB ******************************************
*
*   NAME
*	FlushGroupDB -- Write group database out to disk.
*
*   SYNOPSIS
*	success = FlushGroupDB()
*
*	BOOL FlushGroupDB(VOID);
*
*   FUNCTION
*	Writes out the group database to disk.  If the operation fails,
*	PROGDIR:asgroupdb may be non-existant.  If so, PROGDIR:asgroupdb.bak
*	should still be usable.  This function should be called after every
*	change to the group database.
*
*	Does NOT free the group database in ram.
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
*	Only WriteGroup() and RemoveGroup() should call this function.
*
*   BUGS
*
*   SEE ALSO
*	InitGroupDB()
*
*****************************************************************************
*
*/
BOOL FlushGroupDB(VOID)
{
struct Group *node;
LONG returned;
BPTR file;

	if( !(file = Open("PROGDIR:temp_asgroupdb", MODE_NEWFILE)) )
	{
		return FALSE;
	}
	if( (returned = Write(file, &HighGID, sizeof(USHORT))) != sizeof(USHORT))
	{
		PrintFault(IoErr(), "Can't write group database because");
		Close(file);
		return FALSE;
	}
	for(node = (struct Group *)GroupDBList->mlh_Head;
	    node->g_Node.mln_Succ;
	    node = (struct Group *)node->g_Node.mln_Succ)
	{
		if( (returned = Write(file, &node->g_GroupName, GROUP_DSIZE)) !=
		     GROUP_DSIZE )
		{
			break;
		}
	}
	if(node->g_Node.mln_Succ)  /* If didn't get to end of list */
	{
		if(returned == -1)
		{
			PrintFault(IoErr(), "Couldn't write full group database because");
		} else
		{
			printf("Unintelligible DOS error while writing group database.\n");
		}
		return FALSE;
	}
	Close(file);
	DeleteFile("PROGDIR:asgroupdb.bak");
	if( !Rename("PROGDIR:asgroupdb", "PROGDIR:asgroupdb.bak") )
	{
		PrintFault(IoErr(), "Can't rename asgroupdb to asgroupdb.bak because");
		return FALSE;
	}
	if( !Rename("PROGDIR:temp_asgroupdb", "PROGDIR:asgroupdb") )
	{
		PrintFault(IoErr(), "Can't rename temp_asgroupdb to asgroupdb because");
		return FALSE;
	}
	DeleteFile("PROGDIR:asgroupdb.bak");
	return TRUE;
}


/****i* AuthenticationServer/WriteGroup ******************************************
*
*   NAME
*	WriteGroup -- Writes a group profile to disk.
*
*   SYNOPSIS
*	success = WriteGroup(group);
*
*	BOOL WriteGroup(struct Group *);
*
*   FUNCTION
*	Writes a group profile to disk.  If the group with this UID already
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
BOOL WriteGroup(struct Group *group)
{
struct Group *node, *new;

/*
**  Allocate space for a new node and copy group into it:
*/
	if( !(new = AllocMem(sizeof(struct Group), MEMF_ANY)) )
	{
		return FALSE;
	}
	CopyMem(group, new, sizeof(struct Group));
B(printf("\nWriteGroup(): %s:%d -- %d %d %d.\n",new->g_GroupName,new->g_GID,new->g_UserMembers[0],new->g_UserMembers[1],new->g_UserMembers[2]));
/*
**  Try to find GroupName in the list:
*/

	for(node = (struct Group *)GroupDBList->mlh_Head;
	    node->g_Node.mln_Succ;
	    node = (struct Group *)node->g_Node.mln_Succ)
	{
		if(Stricmp(group->g_GroupName, node->g_GroupName) == 0)
			break;
	}
/*
**  If it was there, remove the old one.  If it wasn't, assign a new UID.
**  Either way, add the new node:
*/
	if(node->g_Node.mln_Succ)  /*  If didn't get to end of list  */
	{
		Remove((struct Node *)node);
	} else
	{
	B(printf("New Group!\n"));
		new->g_GID = ++HighGID;
	}
	AddHead((struct List *)GroupDBList, (struct Node *)new);
/*
**  Write the updated database to disk.  If successful and there was an
**  old node, free the old node.  If failed, remove the new node and
**  reintroduce the old node (if any):
*/
	if(FlushGroupDB())
	{
		if(node->g_Node.mln_Succ)  /* If didn't get to end of list */
		{
			FreeMem(node, sizeof(struct Group));
		}
		CopyMem(new, group, sizeof(struct Group));
		return TRUE;
	}
B(printf("got to FlushGroupDB() failing.\n"));
	Remove((struct Node *)new);
B(printf("removed.\n"));
	if(node->g_Node.mln_Succ) /* If didn't get to end of list */
	{
		AddHead((struct List *)GroupDBList, (struct Node *)node);
	}
B(printf("about to FreeMem().\n"));
	FreeMem(new, sizeof(struct Group));
B(printf("about to return FALSE.\n"));
	return FALSE;
}


/****i* AuthenticationServer/ReadGroup ******************************************
*
*   NAME
*	ReadGroup -- Read a group's profile.
*
*   SYNOPSIS
*	success = ReadGroup(group);
*
*	BOOL ReadGroup(struct Group *group);
*
*   FUNCTION
*	If the database contains a group with groupname group->g_GroupName, that
*	group's UserProfile is copied into group.  Otherwise, group is left alone.
*
*   INPUTS
*	group		- Pointer to a UserProfile structure containing the
*			  groupname to read and into which that group's UserProfile
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
BOOL ReadGroup(struct Group *group)
{
struct Group *node;

	for(node = (struct Group *)GroupDBList->mlh_Head;
	    node->g_Node.mln_Succ;
	    node = (struct Group *)node->g_Node.mln_Succ)
	{
		if(Stricmp(group->g_GroupName, node->g_GroupName) == 0)
			break;
	}
	if(!node->g_Node.mln_Succ)  /* If got to end of list */
	{
		return FALSE;
	}
	CopyMem(node, group, sizeof(struct Group));
	return TRUE;
}


/****i* AuthenticationServer/ReadGroupID **************************************
*
*   NAME
*	ReadGroupID -- Read a user's profile.
*
*   SYNOPSIS
*	success = ReadGroupID(group);
*
*	BOOL ReadGroupID(struct UserProfile *user);
*
*   FUNCTION
*	If the database contains a group with GID group->up_GID, the Group
*	structure is copied into group.  Otherwise, group is left alone.
*
*   INPUTS
*	group		- Pointer to a Group structure containing the
*			  GID to read and into which that groups info
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
BOOL ReadGroupID(struct Group *group)
{
struct Group *node;

	for(node = (struct Group *)GroupDBList->mlh_Head;
	    node->g_Node.mln_Succ;
	    node = (struct Group *)node->g_Node.mln_Succ)
	{
		if(group->g_GID == node->g_GID)
			break;
	}
	if(!node->g_Node.mln_Succ)  /* If got to end of list */
	{
		return FALSE;
	}
	CopyMem(node, group, sizeof(struct Group));
	return TRUE;
}


/****i* AuthenticationServer/RemoveGroup ******************************************
*
*   NAME
*	RemoveGroup -- Remove a group from the authentiation group database.
*
*   SYNOPSIS
*	success = RemoveGroup(struct Group *group)
*
*	BOOL RemoveGroup(struct Group *group);
*
*   FUNCTION
*	Removes a group from the authentication server group database.
*	If the updated database cannot be successfully writen to disk,
*	the deletion does not take effect.
*
*   INPUTS
*	group		- Pointer to UserProfile structure of group to delete.
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
BOOL RemoveGroup(struct Group *group)
{
struct Group *node;

B(printf("RemoveGroup() called."));
	for(node = (struct Group *)GroupDBList->mlh_Head;
	    node->g_Node.mln_Succ;
	    node = (struct Group *)node->g_Node.mln_Succ)
	{
		if(Stricmp(group->g_GroupName, node->g_GroupName) == 0)
			break;
	}
	if(!node->g_Node.mln_Succ) /* If got to end of list */
	{
		return FALSE;
	}
	Remove((struct Node *)node);
	if(FlushGroupDB())
	{
		FreeMem(node, sizeof(struct Group));
		return TRUE;
	}
	AddHead((struct List *)GroupDBList, (struct Node *)node);
	return FALSE;
}


/****i* AuthenticationServer/GetGroupList ******************************************
*
*   NAME
*	GetGroupList -- Return all records in the authentiation group database.
*
*   SYNOPSIS
*	list = GetGroupList()
*
*	struct MinList *GetGroupList(VOID);
*
*   FUNCTION
*	Returns a pointer to a list of all groups records.
*
*   INPUTS
*	None.
*
*   RESULT
*	list		- Pointer to list of all group's records if successful,
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
struct MinList *GetGroupList(VOID)
{
	return GroupDBList;
}
