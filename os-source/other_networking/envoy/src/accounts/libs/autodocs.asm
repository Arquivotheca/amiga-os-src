
******* accounts.library/AllocUserInfo *****************************************
*
*   NAME
*	AllocUserInfo -- Allocate a UserInfo structure.
*
*   SYNOPSIS
*	userinfo = AllocUserInfo()
*	D0
*
*   	struct UserInfo *AllocUserInfo(void);
*
*   FUNCTION
*	Allocates a structure for holding user information.  You *must*
*	use this function for allocating a UserInfo structure, or you
*	risk not being compatible with future versions of
*	accounts.library.
*
*   INPUTS
*	None
*
*   RESULT
*	userinfo - A pointer to a freshly allocated UserInfo structure, or
*	    NULL of not enough memory was available.
*
*   NOTES
*	UserInfo structures must be free with a call to FreeUserInfo().
*
*   SEE ALSO
*	FreeUserInfo()
*
******************************************************************************


******* accounts.library/AllocGroupInfo **************************************
*
*   NAME
*	AllocGroupInfo -- Allocate a GroupInfo structure.
*
*   SYNOPSIS
*	groupinfo = AllocGroupInfo()
*	D0
*
*   	struct GroupInfo *AllocGroupInfo(void);
*
*   FUNCTION
*	Allocates a structure for holding group information.  You *must*
*	use this function for allocating a GroupInfo structure, or you
*	risk not being compatible with future versions of
*	accounts.library.
*
*   INPUTS
*	None
*
*   RESULT
*	groupinfo - A pointer to a freshly allocated GroupInfo structure, or
*	    NULL of not enough memory was available.
*
*   NOTES
*	GroupInfo structures must be free with a call to FreeGroupInfo().
*
*   SEE ALSO
*	FreeGroupInfo()
*
******************************************************************************


******* accounts.library/FreeUserInfo ****************************************
*
*   NAME
*	FreeUserInfo -- Free a UserInfo structure.
*
*   SYNOPSIS
*	FreeUserInfo(userinfo)
*		     A0
*
*	VOID FreeUserInfo (struct UserInfo *);
*
*   FUNCTION
*	Frees a UserInfo structure that was allocated with AllocUserInfo().
*
*   INPUTS
*	userinfo - pointer to the UserInfo strcture that you want to free.
*
*   RESULT
*	None
*
*   NOTES
*	*Never* use this function to free a UserInfo structure that wasn't
*	allocated with AllocUserInfo().
*
*   SEE ALSO
*	AllocUserInfo()
*
******************************************************************************


******* accounts.library/FreeGroupInfo ***************************************
*
*   NAME
*	FreeGroupInfo -- Free a GroupInfo structure.
*
*   SYNOPSIS
*	FreeGroupInfo(groupinfo)
*		     A0
*
*	VOID FreeGroupInfo (struct GroupInfo *);
*
*   FUNCTION
*	Frees a GroupInfo structure that was allocated with AllocGroupInfo().
*
*   INPUTS
*	groupinfo - pointer to the GroupInfo strcture that you want to free.
*
*   RESULT
*	None
*
*   NOTES
*	*Never* use this function to free a GroupInfo structure that wasn't
*	allocated with AllocGroupInfo().
*
*   SEE ALSO
*	AllocGroupInfo()
*
******************************************************************************


******* accounts.library/VerifyUser ******************************************
*
*   NAME
*	VerifyUser -- Verify a user's name and password.
*
*   SYNOPSIS
*	error = VerifyUser(username, password, userinfo)
*	D0		   A0	     A1	       A2
*
*	ULONG VerifyUser(STRPTR, STRPTR, struct UserInfo *)
*
*   FUNCTION
*	This function will consult the Users database and check to see
*	if the username and password given are vaild.  If they are,
*	accounts.library will fill in the UserInfo structure with the
*	user's UID, primary GID, and the flags set for that user.
*
*   INPUTS
*	username - pointer to a user name.
*	password - pointer to a user's password.
*	userinfo - pointer to a struct UserInfo.
*
*   RESULTS
*	VerifyUser() will return 0 if the user and password were vaild.
*	Otherwise, an error code will be returned.  Please see
*	<envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/MemberOf ********************************************
*
*   NAME
*	MemberOf -- Verify a user's group membership.
*
*   SYNOPSIS
*	error = MemberOf(group, user)
*	D0		 A0     A1
*
*	ULONG MemberOf(struct GroupInfo *, struct UserInfo *)
*
*   FUNCTION
*	This function will consult the users and groups database to see
*	if the given user is a member of the given group.
*
*   INPUTS
*	group - Pointer to a GroupInfo that is filled in with the name
*		of the group or the group's GID.
*	user - Pointer to a UserInfo that is filled in with the name of
*		the user or the user's UID.
*
*   RESULTS
*	Returns 0 if the user is a member of the group, or an error.
*	Please see <envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/NameToUser ******************************************
*
*   NAME
*	NameToUser -- Find a user by name.
*
*   SYNOPSIS
*	error = NameToUser(userName, userinfo)
*	D0		   A0	     A1
*
*	ULONG NameToUser(STRPTR username, struct UserInfo *)
*
*   FUNCTION
*	This function will try to find a user given a user's name. If the
*	user exists in the users database, the UserInfo struct will be
*	filled in with the user's name, UID, primary GID, and flags.
*
*   INPUTS
*	userName - The name of the user you want information for.
*	userinfo - A UserInfo structure to be filled in.
*
*   RESULTS
*	Returns 0 if the user is a member of the group, or an error.
*	Please see <envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/NameToGroup *****************************************
*
*   NAME
*	NameToGroup -- Find a group by name.
*
*   SYNOPSIS
*	error = NameToGroup(groupName, groupinfo)
*	D0		    A0	       A1
*
*	ULONG NameToGroup(STRPTR, struct GroupInfo *)
*
*   FUNCTION
*	This function will try to find a named group in the groups
*	database and then fill in the GroupInfo structure with the
*	group's name and GID.
*
*   INPUTS
*	groupName - The name of the group you want information fo.
*	groupinfo - A GroupInfo structure to be filled in.
*
*   RESULTS
*	Returns 0 if the user is a member of the group, or an error.
*	Please see <envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/IDToUser ********************************************
*
*   NAME
*	IDToUser - Find a user by UID
*
*   SYNOPSIS
*       error = IDToUser(userid, userinfo)
*	D0		 D0	 A0
*
*       ULONG IDToUser(UWORD, struct UserInfo *)
*
*   FUNCTION
*	This function will try to find a user by his/her UID.  If
*	successfuly, the UserInfo will be filled in with the user's
*	name, UID, primary GID and flags.
*
*   INPUTS
*	userid - the 16-bit userid if the user you are looking for.
*	userinfo - Pointer to a UserInfo structure to fill in.
*
*   RESULTS
*	Returns 0 if the user is a member of the group, or an error.
*	Please see <envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/IDToUser ********************************************
*
*   NAME
*	IDToUser - Find a user by UID
*
*   SYNOPSIS
*       error = IDToUser(userid, userinfo)
*	D0		 D0	 A0
*
*       ULONG IDToUser(UWORD, struct UserInfo *)
*
*   FUNCTION
*	This function will try to find a user by his/her UID.  If
*	successfuly, the UserInfo will be filled in with the user's
*	name, UID, primary GID and flags.
*
*   INPUTS
*	userid - the 16-bit userid if the user you are looking for.
*	userinfo - Pointer to a UserInfo structure to fill in.
*
*   RESULTS
*	Returns 0 if the user is a member of the group, or an error.
*	Please see <envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/IDToGroup *******************************************
*
*   NAME
*	IDToGroup - Find a group by GID
*
*   SYNOPSIS
*       error = IDToGroup(groupid, groupinfo)
*	D0		 D0	 A0
*
*       ULONG IDToGroup(UWORD, struct GroupInfo *)
*
*   FUNCTION
*	This function will try to find a group by his/her UID.  If
*	successfuly, the GroupInfo will be filled in with the group's
*	name and GID.
*
*   INPUTS
*	groupid - the 16-bit groupid if the group you are looking for.
*	groupinfo - Pointer to a GroupInfo structure to fill in.
*
*   RESULTS
*	Returns 0 if the group is a member of the group, or an error.
*	Please see <envoy/errors.h> for possible error codes.
*
*   SEE ALSO
*
******************************************************************************


******* accounts.library/NextUser ********************************************
*
*   NAME
*	NextUser - Scan through the user database.
*
*   SYNOPSIS
*	error = NextUser(userinfo)
*	D0		 A0
*
*	ULONG NextUser(struct UserInfo *)
*
*   FUNCTION
*	This function is used for building a list of all user's on the
*	system.  The first time you call NextUser, the ui_UserID of the
*	UserInfo structure should be filled in with 0.
*
*	Each call to NextUser() will fill in the UserInfo structure with
*	the next user in the database.  When all user's have been listed,
*	NextUser() will return with ENVOYERR_LASTUSER.
*
*   INPUTS
*	userinfo - Pointer to a UserInfo structure to fill in with the
*	   next User in the database.
*
*   RESULTS
*	Returns 0 until the last user is filled in, and then returns
*	ENVOYERR_LASTUSER.  You *must* check for other possible error
*	conditions as well.
*
*   SEE ALSO
*	NextGroup(), NextMember()
*
******************************************************************************


******* accounts.library/NextGroup ********************************************
*
*   NAME
*	NextGroup - Scan through the group database.
*
*   SYNOPSIS
*	error = NextGroup(groupinfo)
*	D0		  A0
*
*	ULONG NextGroup(struct GroupInfo *)
*
*   FUNCTION
*	This function is used for building a list of all groups on the
*	system.  The first time you call NextGroup, the ui_GroupID of the
*	GroupInfo structure should be filled in with 0.
*
*	Each call to NextGroup() will fill in the GroupInfo structure with
*	the next group in the database.  When all group's have been listed,
*	NextGroup() will return with ENVOYERR_LASTGROUP.
*
*   INPUTS
*	groupinfo - Pointer to a GroupInfo structure to fill in with the
*	   next Group in the database.
*
*   RESULTS
*	Returns 0 until the last group is filled in, and then returns
*	ENVOYERR_LASTGROUP.  You *must* check for other possible error
*	conditions as well.
*
*   SEE ALSO
*	NextUser(), NextMember()
*
******************************************************************************


******* accounts.library/NextMember ******************************************
*
*   NAME
*	NextMember - Scan through the a group's member list.
*
*   SYNOPSIS
*	error = NextMember(groupinfo, userinfo)
*	D0		  A0	     A1
*
*	ULONG NextMember(struct GroupInfo *, struct UserInfo *)
*
*   FUNCTION
*	This function is used for building a list of all members in a
*	group.  The first time you call NextMember, the ui_UserID of the
*	UserInfo structure should be filled in with 0.  The GroupInfo
*	structure should contain either the GID or the name of the group
*	that you want to scan.  If both are given, the GID will have
*	priority.
*
*	Each call to NextMember() will fill in the UserInfo structure with
*	the next user in the group.  When all users's have been listed,
*	NextMember() will return with ENVOYERR_LASTMEMBER.
*
*   INPUTS
*	groupinfo - Pointer to a GroupInfo structure that is filled in
*	    for the group you want to scan.
*	userinfo - Pointer to a UserInfo structure to fill in with the
*	    next member of the group.
*
*   RESULTS
*	Returns 0 until the last group is filled in, and then returns
*	ENVOYERR_LASTMEMBER.  You *must* check for other possible error
*	conditions as well.
*
*   SEE ALSO
*	NextUser(), NextGroup()
*
******************************************************************************

