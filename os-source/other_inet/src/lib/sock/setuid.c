
/****** socket/setuid  **********************************************
*
*	NAME
*		setuid - set the real and effective user ID 
*
*
*	SYNOPSIS
*		success = setuid( uid ) 
*
*		int setuid( uid_t uid )
*
*	FUNCTION
*		Sets the user ID to the value specified.
*
*	INPUTS
*		uid - the desired user id value
*
*	RESULT
*		success - 0 (zero) upon success, -1 upon failure.
*
*	NOTES
*		setuid() and seteuid() are equivilent on the Amiga
*
*	SEE ALSO
*		seteuid, setgid, setegid
*   
********************************************************************
*
*/

#include <config.h>

int seteuid(uid_t);
int setgid(uid_t);
int setegid(uid_t);
int setgroups( int num, uid_t *gid);


int setuid( uid_t u)
{
	return seteuid(u);
}

/****** socket/seteuid  ******************************************
*
*	NAME
*		seteuid - set the effective user ID
*
*	SYNOPSIS
*		success = seteuid( uid )
*
*		int seteuid( uid_t uid )
*
*	FUNCTION
*		Sets the effective user ID as specified
*
*	INPUTS
*		uid - The desired ID value
*
*	RESULT
*		success - 0 (zero) upon success and -1 upon failure
*
*
*	SEE ALSO
*		setuid, setegid
*   
********************************************************************
*
*
*/

int seteuid(uid_t u)
{
	struct config *cf;

	GETCONFIG(cf);
	if(!cf)
		return -1;
		
	cf->uid = u;

	return 0;
}



/****** socket/setgid *****************************************
*
*	NAME
*		setgid - Set the group ID and the effective group ID
*
*	SYNOPSIS
*		success = setgid( gid )
*
*		int setgid( uid_t gid )
*
*	FUNCTION
*		Sets the the group ID and the effective group ID to the 
*		specified value		
*
*	INPUTS
*		gid - the desired ID value
*
*	RESULT
*		success - 0 (zero) upon success, -1 upon failure
*
*	NOTES
*		setgid() and setegid() are equivilent on the Amiga 
*
*	SEE ALSO
*		setegid, setuid, seteuid
*   
********************************************************************
*
*
*/


int setgid(uid_t g)
{
	return( setegid(g) ) ;
}

/****** socket/setegid *****************************************
*
*	NAME
*		setegid - Set the effective group ID
*
*	SYNOPSIS
*		success = setegid( gid )
*
*		int setegid( uid_t gid )
*
*	FUNCTION
*		Sets the the effective group ID to the specified value		
*
*	INPUTS
*		gid - the desired ID value
*
*	RESULT
*		success - 0 (zero) upon success, -1 upon failure
*
*	NOTES
*		setgid() and setegid() are equivilent on the Amiga 
*
*	SEE ALSO
*		setegid, setuid, seteuid
*
*   
*   
********************************************************************
*
*
*/

int setegid( uid_t g)
{
	struct config *cf;

	GETCONFIG(cf);
	if(!cf)
		return -1;
		
	cf->gid = g;

	return 0;
}


/****** socket/setgroups *************************************************
*
*	NAME
*		setgroups -  Set the group access list.
*
*	SYNOPSIS
*		success = setgroups( count, gidlist )
*
*		int  setgroups( int, uid_t * )
*
*	FUNCTION
*		Setgroups() sets the group access list. 
*		process according to the array gidset. The parameter
*		ngroups indicates the number of entries in the array and
*		must be no more than NGROUPS, as defined in <sys/param.h>.
*
*	INPUTS
*		count1  - An integer representing the number if groups in
*		          the gidlist. No more than NGROUPS as defined in
*		          <sys/param.h> will be used.
*		gidlist - An array in pointers to type uid_t which hold
*		          the desired group IDs.
*
*
*	RESULT
*		success - 0 (zero) opon success, -1 upon failure
*   
********************************************************************
*
*
*/

int setgroups( int num, uid_t *gid)
{
	struct config *cf;
	register int i;

	GETCONFIG(cf);
	if(!cf)
		return -1;
		
	for(i = 0; i < num && i < NGROUP; i++){
		cf->gids[i] = gid[i];
	}

	return 0;
}

