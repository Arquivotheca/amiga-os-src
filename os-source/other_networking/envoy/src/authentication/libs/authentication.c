/*
**  $Id: authentication.c,v 1.3 92/04/09 11:57:41 dlarson Exp Locker: dlarson $
**
**  Public authentication.library functions.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "authlib_internal.h"

/****** --background-- ********************************************************
*
*	Note that Authentication Server and the authentication.library provide
*	tools to application developers which allow them to write applications
*	with some protection against unauthorized use.  The intent is
*	basically to help keep honest people honest.  These protections do not
*	currently incorporate cryptographic techniques and will not be
*	sufficient to keep a knowledgeable or determined attacker from gaining
*	unauthorized access.  Do not use authentication.library to protect
*	critical or highly sensitive data.

*	Most calls to authentication.library will involve a transaction with
*	the authentication server.  If the authentication server for this
*	machine is non-local, such calls may block as long as the time-out
*	on an NIPC transaction.  There are no provisions for breaking a call
*	to authentication.library.
*
*	Like other Envoy libraries, authentication.library uses the common
*	<Envoy/errors.h> include file for error codes.  Since
*	authentication.library calls nipc.library, nipc.library error codes
*	may be returned from authentication.library functions.  ErrorText()
*	will return error text for any error code returned by
*	authentication.library, regardless of where the error originated.
*
*	Many functions have a pointer to a UserProfile authority argument.
*	Only the up_UserName and up_PassWord fields of the authority
*	argument need be filled in.  The contents of the authority argument
*	are never modified by authentication.library.
*
*	up_GID is a primary GID.  A user will usually (but not always) belong
*	to that group.  Most applications won't use up_GID, it exists primarily
*	for the filesystem's default GID on newly created files.
*	A new user belongs to no groups.  A new user should probably be
*	AddToGroup() his up_GID.
*
****************************************************************************
*
*/

/****** authentication.library/AllocUserProfile ********************************
*
*   NAME
*	AllocUserProfile -- Allocate a UserProfile to be filled in by user.
*
*   SYNOPSIS
*	profile = AllocUserProfile()
*	D0
*
*	struct UserProfile *AllocUserProfile(VOID);
*
*   FUNCTION
*	UserProfile structures may not be allocated by the user but must be
*	allocated by the authentication.library.  If the user needs to
*	fill in a UserProfile structure to pass authentication.library as an
*	argument, it must AllocUserProfile() the structure and FreeUserProfile()
*	it when finished.
*
*	All members of the new UserProfile structure will be set to zero.
*
*   INPUTS
*	None.
*
*   RESULT
*	profile		- Pointer to a UserProfile structure if successful,
*			  else NULL.
*
*   EXAMPLE
*
*   NOTES
*	Do not examine, modify or otherwise rely on non-public fields of the
*	UserProfile structure.
*
*   BUGS
*
*   SEE ALSO
*	FreeUserProfile()
*
*****************************************************************************
*
*/
struct UserProfile * LIBENT AllocUserProfile(register __a6 struct AuthenticationBase
				     *AuthenticationBase)
{
struct UserProfile *temp;

	temp = (struct UserProfile *)
	        AllocMem(sizeof(struct UserProfile), MEMF_ANY|MEMF_CLEAR);
BUG(("%lx = AllocUserProfile(): sizeof(struct UserProfile) = %ld.\n", temp, sizeof(struct UserProfile)));
	return(temp);
//	return (struct UserProfile *)
//	        AllocMem(sizeof(struct UserProfile), MEMF_ANY|MEMF_CLEAR);
}

/****** authentication.library/FreeUserProfile ********************************
*
*   NAME
*	FreeUserProfile -- Free a UserProfile structure created by the library.
*
*   SYNOPSIS
*	FreeUserProfile(profile)
*		    A0
*
*	VOID FreeUserProfile(struct UserProfile *);
*
*   FUNCTION
*	All UserProfile structures must be created by authentication.library
*	(either through AllocUserProfile() or implicitly via other calls).  All
*	UserProfile structures must eventually be returned to the library via a
*	call to FreeUserProfile().  If the structure includes pointers to data
*	allocated by the library, that data will also be freed.
*
*	It is legal to call FreeUserProfile() with a NULL profile.
*
*   INPUTS
*	profile		- Pointer to a UserProfile structure to be freed.
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
*	AllocUserProfile()
*
*****************************************************************************
*
*/
VOID LIBENT FreeUserProfile(register __a0 struct UserProfile *foo,
			register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("FreeUserProfile(%lx): sizeof(struct UserProfile) = %ld.\n", foo, sizeof(struct UserProfile)));
	if(foo)
	{
		FreeMem(foo, sizeof(struct UserProfile));
	}
BUG(("exiting FreeUserProfile()\n"));
}
/****** authentication.library/AllocGroup ********************************
*
*   NAME
*	AllocGroup -- Allocate a Group to be filled in by user.
*
*   SYNOPSIS
*	profile = AllocGroup()
*	D0
*
*	struct Group *AllocGroup(VOID);
*
*   FUNCTION
*	Group structures may not be allocated by the user but must be
*	allocated by the authentication.library.  If the user needs to
*	fill in a Group structure to pass authentication.library as an
*	argument, it must AllocGroup() the structure and FreeGroup()
*	it when finished.
*
*	All members of the new Group structure will be set to zero.
*
*   INPUTS
*	None.
*
*   RESULT
*	profile		- Pointer to a Group structure if successful,
*			  else NULL.
*
*   EXAMPLE
*
*   NOTES
*	Do not examine, modify or otherwise rely on non-public fields of the
*	Group structure.
*
*   BUGS
*
*   SEE ALSO
*	FreeGroup()
*
*****************************************************************************
*
*/
struct Group * LIBENT AllocGroup(register __a6 struct AuthenticationBase
				     *AuthenticationBase)
{
struct Group *temp;

	temp = (struct Group *)
	        AllocMem(sizeof(struct Group), MEMF_ANY|MEMF_CLEAR);
BUG(("%lx = AllocGroup(): sizeof(struct Group) = %ld.\n", temp, sizeof(struct Group)));
	return(temp);
//	return (struct Group *)
//	        AllocMem(sizeof(struct Group), MEMF_ANY|MEMF_CLEAR);
}

/****** authentication.library/FreeGroup ********************************
*
*   NAME
*	FreeGroup -- Free a Group structure created by the library.
*
*   SYNOPSIS
*	FreeGroup(profile)
*		    A0
*
*	VOID FreeGroup(struct Group *);
*
*   FUNCTION
*	All Group structures must be created by authentication.library
*	(either through AllocGroup() or implicitly via other calls).  All
*	Group structures must eventually be returned to the library via a
*	call to FreeGroup().  If the structure includes pointers to data
*	allocated by the library, that data will also be freed.
*
*	It is legal to call FreeGroup() with a NULL profile.
*
*   INPUTS
*	profile		- Pointer to a Group structure to be freed.
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
*	AllocGroup()
*
*****************************************************************************
*
*/
VOID LIBENT FreeGroup(register __a0 struct Group *foo,
			register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("FreeGroup(%lx): sizeof(struct Group) = %ld.\n", foo, sizeof(struct Group)));
	if(foo)
	{
		FreeMem(foo, sizeof(struct Group));
	}
BUG(("exiting FreeGroup()\n"));
}

/****** authentication.library/AuthenticateUser *****************************
*
*   NAME
*	AuthenticateUser -- Confirm identity of a user.
*
*   SYNOPSIS
*	profile = AuthenticateUser(username, password)
*	D0			   A0	     A1
*
*	struct UserUserProfile *AuthenticateUser(UBYTE *, UBYTE *);
*
*   FUNCTION
*	Given a username and a password, returns a filled-in UserProfile
*	structure on a match or NULL on a miss.  The UserProfile structure
*	must eventually be freed with a call to FreeUserProfile().
*
*   INPUTS
*	username	- Pointer to a nul-terminated string.
*	password	- Pointer to a nul-terminated string.
*
*   RESULT
*	profile		- Pointer to a struct UserUserProfile which contains the
*			  user's full name, numeric UID and GIDs.
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	FreeUserProfile()
*
*****************************************************************************
*
*/
struct UserProfile * LIBENT AuthenticateUser(register __a0 UBYTE *username,
					 register __a1 UBYTE *password,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
struct UserProfile *newprofile;
ULONG scratch;

BUG(("AuthenticateUser() called.\n"));
	newprofile = AllocMem(sizeof(struct UserProfile), MEMF_ANY|MEMF_CLEAR);
	if(!newprofile)
	{
		return NULL;
	}
	CopyMem(username, newprofile->up_UserName, UP_UNLENGTH);
	CopyMem(password, newprofile->up_PassWord, UP_PWLENGTH);
	scratch = doCmd(newprofile, NULL, AUTHENTICATE_USER, NULL, AuthenticationBase);
	if(scratch)
	{
		FreeMem(newprofile, sizeof(struct UserProfile));
		newprofile = NULL;
	}
	return newprofile;
}

/****** authentication.library/AddUser **************************************
*
*   NAME
*	AddUser -- Add a user to the authentication server's database.
*
*   SYNOPSIS
*	error = AddUser(newuser, authority)
*	D0		A0	 A1
*
*	ULONG AddUser(struct UserProfile *, struct UserProfile *);
*
*   FUNCTION
*	Add newuser to the authentication server's database.  This function
*	will fail if executed on insufficient authority.  If it succeeds, the
*	user will be assigned a unique numeric UID.
*
*	All fields except UID (up_UserName, up_FullName, up_PassWord and
*	up_GID) must be valid when AddUser() is called. The UID and of the
*	newUserProfile will be valid on successful return from
*	AddUser().  New UIDs will always be unique on a given authentication
*	server (i.e., they are not recycled).
*
*	Only the up_UserName and up_PassWord members of the authority
*	UserProfile need to be valid and no fields are modified.
*
*   INPUTS
*	newuser		- Pointer to a structure containing information on the
*			  new user to be added.  This structure is updated if
*			  the call is successful.
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority on which the new user is to be added.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*	profile 	- Pointer to a strucutre containing information on the
*			  new user to be added. If success is TRUE, this
*			  structure is updated with any information allocated
*			  as part of the addition (UID, for example).
*
*   EXAMPLE
*
*   NOTES
*	AddUser() can fail for reasons other than lack of authority (i.e.,
*	bad communication with authentication server, authentication server
*	resource allocation trouble, etc.).
*
*   BUGS
*
*   SEE ALSO
*	DeleteUser(), AddToGroup()
*
*****************************************************************************
*
*/
ULONG LIBENT AddUser(register __a0 struct UserProfile *newuser,
		    register __a1 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("AddUser() called.\n"));
	return(doCmd(newuser, authority, ADD_USER, NULL, AuthenticationBase));
}

/****** authentication.library/DeleteUser ***********************************
*
*   NAME
*	DeleteUser -- if authority checks out, user is eliminated.
*
*   SYNOPSIS
*	error = DeleteUser(user, authority)
*	D0		   A0	 A1
*
*	ULONG DeleteUser(struct UserProfile *, struct UserProfile *);
*
*   FUNCTION
*	Removes a user from the authentication server's database.  This
*	function will fail if executed on insufficient authority.
*
*	Only the up_UserName of the UserProfile must be valid when
*	DeleteUser() is called.
*
*	Only the up_UserName and up_PassWord members of the authority
*	UserProfile need to be valid and no fields are modified.
*
*   INPUTS
*	user		- Pointer to UserProfile structure describing user to
*			  be deleted.
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority deletion is to take place.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*	DeleteUser() can fail for reasons other than lack of authority (i.e.,
*	bad communication with authentication server, authentication server
*	resource allocation trouble, etc.).
*
*   BUGS
*	Allows user 'admin' to be deleted.  This is bad.  We should also probably
*	have a utility to recover admin's password if lost.
*
*   SEE ALSO
*	AddUser()
*
*****************************************************************************
*
*/
ULONG LIBENT DeleteUser(register __a0 struct UserProfile *user,
		       register __a1 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("DeleteUser() called.\n"));
	return(doCmd(user, authority, DELETE_USER, NULL, AuthenticationBase));
}

/****** authentication.library/ChangePassWord *********************************
*
*   NAME
*	ChangePassWord -- Change a user's password.
*
*   SYNOPSIS
*	error = ChangePassWord(user, authority)
*	D0		     A0	   A1
*
*	ULONG ChangePassWord(struct UserProfile *, struct UserProfile *);
*
*   FUNCTION
*	Changes a user's password.  This function will fail if executed on
*	insufficient authority.  All users may change their own passwords
*	and members of group Administrators may change other user's passwords.
*
*	For a user to change his own password, UserProfile authority should
*	contain the user's current up_UserName and up_PassWord while UserProfile user
*	should contain the same up_UserName and the new up_PassWord.
*
*	Besides changing a user's password, this function can be used to
*	change a user's primary GID or FullName.
*
*   INPUTS
*	user		- Pointer to UserProfile structure containing updated
*			  information (i.e. PassWord, FullName, GID).
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority the change is to take place.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*	ChangePassWord() can fail for reasons other than lack of authority (i.e.,
*	bad communication with authentication server, authentication server
*	resource allocation trouble, etc.).
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
ULONG LIBENT ChangePassWord(register __a0 struct UserProfile *user,
			 register __a1 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("ChangePassWord() called.\n"));
	return doCmd(user, authority, CHANGE_PASS_WORD, NULL, AuthenticationBase);
}

/****** authentication.library/AddGroup **********************************
*
*   NAME
*	AddGroup -- Add a new group.
*
*   SYNOPSIS
*	error = AddGroup(group, authority)
*	D0		 A0	A1
*
*	ULONG AddGroup(struct Group *, struct UserProfile *);
*
*   FUNCTION
*	Adds another group to the authentication server's database.  This
*	function will fail if executed on insufficient authority.  A user
*	must be a member of the group AddGroups or Administrators in order
*	to create a new group.
*
*	If successful, the GID assigned to the new group is returned in the
*	Group structure.
*
*	A new group will initially have only it's creator as a member, and
*	s/he will have the ModifyGroup bit set.
*
*	Only the up_UserName and up_PassWord of authority need to be valid
*	when AddGroup() is called.  Only the g_GroupName of group is
*	examined by AddGroup().
*
*   INPUTS
*	group		- Pointer to a Group structure containing the name
*			  of the group to add, and returning the GID of new
*			  group on success.
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority the group is to be created.
*
*   RESULT
*	error		- See <envoy/errors.h>
*
*   EXAMPLE
*
*   NOTES
*	AddGroup() can fail for reasons other than lack of authority (i.e.,
*	bad communication with authentication server, authentication server
*	resource allocation trouble, etc.).
*
*   BUGS
*
*   SEE ALSO
*	DeleteGroup(), AddToGroup()
*
*****************************************************************************
*
*/
ULONG LIBENT AddGroup(register __a0 struct Group *group,
		     register __a1 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("AddGroup() called.\n"));
	return doCmd(NULL, authority, ADD_GROUP, group, AuthenticationBase);
}

/****** authentication.library/DeleteGroup **********************************
*
*   NAME
*	DeleteGroup -- Remove an existing group.
*
*   SYNOPSIS
*	error = DeleteGroup(group, authority)
*	D0		    A0	   A1
*
*	ULONG DeleteGroup(struct Group *, struct UserProfile *);
*
*   FUNCTION
*	Deletes a group from the authentication server's database.  This
*	function will fail if executed on insufficient authority.  A user
*	must be a member of the group Administrators or be the creator of the
*	group to be deleted in order to delete a group.
*
*	Only the up_UserName and up_PassWord of authority need to be valid
*	when DeleteGroup() is called.  Only the g_GroupName of the group is
*	examined by DeleteGroup().
*
*   INPUTS
*	group		- Pointer to a Group structure containing the name
*			  of the group to delete.
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority the group is to be created.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*	DeleteGroup() can fail for reasons other than lack of authority (i.e.,
*	bad communication with authentication server, authentication server
*	resource allocation trouble, etc.).
*
*   SEE ALSO
*	AddGroup(), RemoveFromGroup()
*
*****************************************************************************
*
*/
ULONG LIBENT DeleteGroup(register __a0 struct Group *group,
			register __a1 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("DeleteGroup() called.\n"));
	return doCmd(NULL, authority, DELETE_GROUP, group, AuthenticationBase);
}

/****** authentication.library/AddToGroup ***********************************
*
*   NAME
*	AddToGroup -- Add a user to a group.
*
*   SYNOPSIS
*	error = AddToGroup(group, user, authority)
*	D0		   A0	  A1	A2
*
*	ULONG AddToGroup(struct Group *, struct UserProfile *, struct UserProfile *);
*
*   FUNCTION
*	Adds a user to the membership list of a group in the authentication
*	server's database.  This function will fail if executed on
*	insufficient authority.  A user must be a member of the group
*	Administrators or be the creator of the group to be added to in
*	order to add a new member to the group.
*
*	Only the up_UserName of user needs to be valid when AddToGroup() is
*	called.  Only the up_UserName and up_PassWord of authority need to
*	be valid when AddToGroup() is called.	Only the g_GroupName of group
*	is examined by AddToGroup().
*
*   INPUTS
*	group		- Poingter to Group structure containing the name of
*			  the group to which to add user.
*	user		- Pointer to UserProfile structure of user to be added to
*			  group.
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority member is to be added.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*	AddToGroup() can fail for reasons other than lack of authority (i.e.,
*	bad communication with authentication server, authentication server
*	resource allocation trouble, etc.).
*
*   BUGS
*
*   SEE ALSO
*	RemoveFromGroup(), AddGroup()
*
*****************************************************************************
*
*/
ULONG LIBENT AddToGroup(register __a0 struct Group *group,
		       register __a1 struct UserProfile *user,
		       register __a2 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("AddToGroup() called.\n"));
	return doCmd(user, authority, ADD_TO_GROUP, group, AuthenticationBase);
}

/****** authentication.library/RemoveFromGroup ******************************
*
*   NAME
*	RemoveFromGroup -- Remove a user from a group.
*
*   SYNOPSIS
*	error = RemoveFromGroup(group, user, authority)
*	D0			A0     A1    A2
*
*	ULONG RemoveFromGroup(struct Group *, struct UserProfile *,
*			     struct UserProfile *);
*
*   FUNCTION
*	Remove a user from the membership list of a group.  This function will
*	fail if executed on insufficient authority.  A user must belong to the
*	group Administrators or belong to the group a member is to be removed from and
*	have the modify group bit set in order to remove a member from the
*	group.
*
*	Only the UID of UserProfile user needs to be valid when
*	RemoveFromGroup() is called.  Only the up_UserName and up_PassWord
*	of  authority need to be valid when RemoveFromGroup() is called.
*	Only g_GroupName of group is examined by RemoveFromGroup().
*
*   INPUTS
*	group		- Pointer to Group structure containing the name of the
*			  group from which user is to be removed.
*	user		- Pointer to UserProfile structure of user to be removed
*			  from group.
*	authority	- Pointer to UserProfile structure of user on who's
*			  authority member is to be removed.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*	RemoveFromGroup() can fail for reasons other than lack of authority
*	(i.e., bad communication with authentication server, authentication
*	server resource allocation trouble, etc.).
*
*   BUGS
*
*   SEE ALSO
*	AddToGroup(), DeleteGroup()
*
*****************************************************************************
*
*/
ULONG LIBENT RemoveFromGroup(register __a0 struct Group *group,
			    register __a1 struct UserProfile *user,
			    register __a2 struct UserProfile *authority,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("RemoveFromGroup() called.\n"));
	return doCmd(NULL, authority, REMOVE_FROM_GROUP, group, AuthenticationBase);
}

/****** authentication.library/FindUserName ********************************
*
*   NAME
*	FindUserName -- Get the name corresponding to a numeric UID.
*
*   SYNOPSIS
*	error = FindUserName(user)
*	D0		     A0
*
*	ULONG FindUserName(struct UserProfile *);
*
*   FUNCTION
*	If the UID in user is valid, fills in the UserProfile structure with the
*	up_UserName, up_FullName and up_GID.
*
*   INPUTS
*	user		- Pointer to UserProfile structure containing UID, to be
*			  filled in with username and fullname if successful.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*	Users should never see numeric UIDs.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
ULONG LIBENT FindUserName(register __a0 struct UserProfile *user,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("Find_UserName() called.\n"));
	return doCmd(user, NULL, FIND_USER_NAME, NULL, AuthenticationBase);
}

/****** authentication.library/FindGroupName ********************************
*
*   NAME
*	FindGroupName -- Get the name corresponding to a numeric GID.
*
*   SYNOPSIS
*	error = FindGroupName(group)
*	D0		      A0
*
*	ULONG FindGroupName(struct Group);
*
*   FUNCTION
*	If the g_GID in group is a valid group number, fills in g_GroupName.
*
*   INPUTS
*	group		- Pointer to Group structure containing g_GID to get
*			  g_GroupName of.
*
*   RESULT
*	error		- See <envoy/errors.h>
*
*   EXAMPLE
*
*   NOTES
*	Users should never see numeric GIDs.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
ULONG LIBENT FindGroupName(register __a0 struct Group *group,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("FindGroupName() called.\n"));
	return doCmd(NULL, NULL, FIND_GROUP_NAME, group, AuthenticationBase);
}

/****** authentication.library/FindGroupID ********************************
*
*   NAME
*	FindGroupID -- Get the numeric GID corresponding to a group name.
*
*   SYNOPSIS
*	error = FindGroupID(group)
*	D0		    A0
*
*	ULONG FindGroupID(struct Group *);
*
*   FUNCTION
*	If the g_GroupName in group is a valid group name, fills in the
*	numeric g_GID.
*
*   INPUTS
*	group		- Pointer to Group structure containing Name to get
*			  GID of.
*
*   RESULT
*	error		- See <envoy/errors.h>
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
ULONG LIBENT FindGroupID(register __a0 struct Group *group,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("FindGroupID() called.\n"));
	return doCmd(NULL, NULL, FIND_GROUP_ID, group, AuthenticationBase);
}

/****** authentication.library/ListGroups ********************************
*
*   NAME
*	ListGroups -- Get a list of all groups..
*
*   SYNOPSIS
*	error = ListGroups(list);
*	D0		      A0
*
*	ULONG ListGroups(struct MinList *);
*
*   FUNCTION
*	Fills a list with a structure for each group.  Each Group structure
*	must eventually be freed with FreeGroup().
*
*   INPUTS
*	groups		- Pointer to a list.
*
*   RESULT
*	error		- see <envoy/errors.h>
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
ULONG LIBENT ListGroups(register __a0 struct MinList *list,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("ListGroups called.\n"));
	return (doList(list, LIST_GROUPS, NULL, AuthenticationBase));
}

/****** authentication.library/ListUsers ********************************
*
*   NAME
*	ListUsers -- creates a List of all user's UserProfiles.
*
*   SYNOPSIS
*	error = ListUsers(list)
*	D0		  A0
*
*	ULONG ListUsers(struct MinList *);
*
*   FUNCTION
*	Fills a List of the UserProfiles of all users.  The members of
*	the List must eventually be freed with calls to FreeUserProfile().
*
*	The UserProfile structures in the List will have username, fullname,
*	UID and GIDs filled in.  The passwords will be blank.
*
*   INPUTS
*	list		- An initialized List to which UserProfile structures
*			  are added if successful.
*
*   RESULT
*	error		- See <envoy/errors.h>
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	FreeUserProfile()
*
*****************************************************************************
*
*/
ULONG LIBENT ListUsers(register __a0 struct MinList *list,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("ListUsers called.\n"));
	return( doList(list, LIST_USERS, NULL, AuthenticationBase) );
}

/****** authentication.library/ListMembers ********************************
*
*   NAME
*	ListMembers -- returns a List of group member's UserProfiles.
*
*   SYNOPSIS
*	error = ListMembers(list, group)
*	D0		    A0    A1
*
*	ULONG ListMembers(struct MinList *, struct Group *);
*
*   FUNCTION
*	Returns a List of the UserProfiles of all members of a group.  This
*	function will fail if executed on insufficient authority.  A user
*	must belong to the group Administrators or be the creator of the group
*	to have its members listed and have the modify group bit set in order to
*	successfully ListMembers().
*
*	The UserProfile structures in the List will have username, fullname
*	and UID filled in (note that the GIDs are not filled in).
*
*   INPUTS
*	list		- An initialized List to which UserProfile structures
*			  are added if successful.
*	group		- Contains g_GroupName of group to list members of.
*
*   RESULT
*	error		- See <envoy/errors.h>
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
ULONG LIBENT ListMembers(register __a0 struct MinList *list,
			register __a1 struct Group *group,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("ListMembers called.\n"));
	return( doList(list, LIST_MEMBERS, group, AuthenticationBase) );
}

/****** authentication.library/ChooseServer ********************************
*
*   NAME
*	ChooseAuthServer -- Select the authentication server to resolve calls.
*
*   SYNOPSIS
*	error = ChooseAuthServer(hostname)
*	D0		     A0
*
*	ULONG ChooseAuthServer(UBYTE *);
*
*   FUNCTION
*	Selects the authentication server which will be used to resolve
*	all other calls to authentication.library.  Only the services manager
*	and preferences editors should need to use this function.
*
*	If authentication.library is used without a call to ChooseServer(),
*	authentication.library will find any authentication server.  This is
*	useful for simple networks, but requires that care be taken to call
*	ChooseServer() at boot if required.
*
*   INPUTS
*	hostname	- A nul-terminated string.
*
*   RESULT
*	error		- See <envoy/errors.h>.
*
*   EXAMPLE
*
*   NOTES
*	The authentication server used may be changed as desired (as to edit
*	a remote server's database then to switch back to the local database).
*	If this is done, exported services should be temporarily disabled so
*	that a security hole on the local machine isn't created during such
*	remote editing.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
ULONG LIBENT ChooseAuthServer(register __a0 UBYTE *hostname,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
VOID *tempEntity;
ULONG err;

BUG(("In ChooseAuthServer().\n"));
	tempEntity = FindEntity(hostname, "AuthenticationServer", SrcEntity, &err);
	if(!tempEntity)
	{
	BUG(("Didn't get tempEntity for %s %s.\n", hostname, "AuthenticationServer"));
		return err;
	}
	if(ServerEntity)
	{
	BUG(("Losing ServerEntity.\n"));
		LoseEntity(ServerEntity);
		ServerEntity = NULL;
	}
BUG(("Returning with new ServerEntity.\n"));
	ServerEntity = tempEntity;
	return ENVOYERR_NOERROR;
}

/****** authentication.library/AuthErrorText ********************************
*
*   NAME
*	AuthErrorText -- Get a text error message for a numeric error code.
*
*   SYNOPSIS
*	success = AuthErrorText(code, buffer, length)
*	D0		    D0    A0	   D1
*
*	BOOL AuthErrorText(ULONG, UBYTE *, ULONG);
*
*   FUNCTION
*	Provides a text error message for a numeric error code returned by
*	authentication.library.  Provides localized error messages if
*	locale.library and the appropriate text catalogues are available.
*	Usually fails only on buffer length too small.
*
*   INPUTS
*	code		- A numeric error code.
*	buffer		- A buffer in which to place the error message text.
*	length		- The length, in bytes, of buffer.
*
*   RESULT
*	success		- TRUE if code is valid and text provided, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*	Like other Envoy libraries, authentication.library uses the common
*	Envoy/errors.h/i include file for error codes.  Since
*	authentication.library calls nipc.library, nipc.library error codes
*	may be returned from authentication.library functions.  ErrorText()
*	will return error text for any error code returned by
*	authentication.library, regardless of where the error originated.
*
*   BUGS
*	Not fully implemented -- only returns proper error text for errors
*	originating with authentication.library (i.e., not nipc.library
*	errors).  Further, is not yet localized.
*
*   SEE ALSO
*
*****************************************************************************
*
*/
void __asm putchproc(register __d0 char c, register __a3 char **s)
{
	**s = c;
	(*s)++;
}

BOOL LIBENT AuthErrorText(register __d0 ULONG code,
		      register __a0 UBYTE *buffer,
		      register __d1 ULONG length,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
	if(length < 50)  /*  Not enough buffer space.  */
	{
		return FALSE;
	}
	if(!code)
	{
		CopyMem((APTR)"No error.",
			buffer,
			length);
		return TRUE;
	}
	switch(code)
	{
	case ENVOYERR_NOAUTHSERV:
		CopyMem((APTR)"No Authentication Server set.",
			buffer,
			length);
		break;
	case ENVOYERR_NOAUTHORITY:
		CopyMem((APTR)"Insufficient Authority.",
			buffer,
			length);
		break;
	case ENVOYERR_NOOBJECT:
		CopyMem((APTR)"Object not found.",
			buffer,
			length);
		break;
	case ENVOYERR_OBJEXISTS:
		CopyMem((APTR)"Object already exists.",
			buffer,
			length);
		break;
	case ENVOYERR_IOERR:
		CopyMem((APTR)"Server has disk IO problems.",
			buffer,
			length);
		break;
	default:
		{
		ULONG tempc;
		APTR  tempb;

			tempc = code;
			tempb = buffer;
			RawDoFmt("Unknown error code: %ld.", &tempc, putchproc, &tempb);
			return TRUE;
		}
	}
}

/****** authentication.library/MemberOf ********************************
*
*   NAME
*	MemberOf -- Check a user for membership in a particular group.
*
*   SYNOPSIS
*	result = MemberOf(group, user)
*
*	BOOL MemberOf(struct Group *, struct UserProfile *);
*
*   FUNCTION
*	Checks the UserProfile to determine whether the user belongs to
*	the group with the name specified in group.  It properly masks the
*	modify-group bit in making it's determination.
*
*   INPUTS
*	gid		- Numeric GID.
*	user		- Pointer to UserProfile structure.
*
*   RESULT
*	result		- TRUE if member of, else FALSE.
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
BOOL LIBENT MemberOf(register __a0 struct Group *group,
		    register __a1 struct UserProfile *user,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("MemberOf() called.\n"));
	if(doCmd(user, NULL, MEMBER_OF, group, AuthenticationBase) ==
	   ENVOYERR_NOERROR)
	{
		return TRUE;
	}
	return FALSE;
}


/****** authentication.library/NoSecurity ***********************************
*
*   NAME
*	NoSecurity -- Stop authenticating users before granting access.
*
*   SYNOPSIS
*	NoSecurity(onoff)
*		   d0
*
*	VOID NoSecurity(BOOL);
*
*   FUNCTION
*	Sets a flag in AuthenticationBase which forces calls to
*	authentication.library functions to return immediately with
*	an error code of ENVOYERR_NOERROR.  This lets the user turn off
*	all security without applications needing to incorporate this
*	feature.
*
*	Alloc functions, Free functions, AuthErrorText() and ChooseServer()
*	are not affected by calls to NoSecurity().
*
*   INPUTS
*	onoff		- TRUE turns security off, FALSE turns it on,
*			  regardless of previous state.
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
VOID LIBENT NoSecurity(register __d0 BOOL onoff,
	     register __a6 struct AuthenticationBase *AuthenticationBase)
{
BUG(("NoSecurity(%ld) called.\n", (LONG)onoff));
	OnOff = onoff;
}
