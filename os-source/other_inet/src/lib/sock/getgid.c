/****** socket/getgid ******************************************
*
*   NAME
*		getgid -- get group id
*
*   SYNOPSIS
*		gid = getgid()
*
*		int getgid (); 
*
*   FUNCTION
*		returns the group id 
*
*	INPUTS
*
*   RESULT
*		gid		group id
*		if not found, gid will be -1.
*
*   EXAMPLE
*
*   NOTES
*		gid is read by config from inet:s/inet.config.
*		It is then stored in a global memory location.
*
*		getegid() is equivalent to getgid() on the Amiga
*
*   BUGS
*
*   SEE ALSO
*		getegid(), getgroups(), getuid()
*
******************************************************************************
*
*/

/****** socket/getegid ******************************************
*
*   NAME
*		getegid -- get effective group id
*
*   SYNOPSIS
*		gid = getegid()
*
*		int getegid (); 
*
*   FUNCTION
*		returns the group id 
*
*	INPUTS
*
*   RESULT
*		gid		group id
*		if not found, gid will be -1.
*
*   EXAMPLE
*
*   NOTES
*		gid is read by config from inet:s/inet.config.
*		It is then stored in a global memory location.
*
*		getegid() is equivalent to getgid() on the Amiga
*
*   BUGS
*
*   SEE ALSO
*		getgid(), getgroups(), getuid()
*
******************************************************************************
*
*/

#include <config.h>

int
getgid()
{
	return getegid();
}

int
getegid()
{
	register struct config *cf;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	return cf->gid;
}

/****** socket/getgroups ******************************************
*
*   NAME
*		getgroups -- get group access list
*
*   SYNOPSIS
*		num = getgroups(max_gids, gids)
*
*		int getgroups (int, int *); 
*
*   FUNCTION
*		The array "gids" is filled with the group ids that the current
*		user belongs to.  The list may be up to "max_gids" long.  
*		The actual number of gids is returned by the function.
*
*	INPUTS
*		max_gids	length of gids array
*		gids		integer array
*
*   RESULT
*		The number of gids is returned if successful.  
*		-1 is returned on error.
*		errno will be set to the specific error code.
*
*   EXAMPLE
*
*   NOTES
*		The gids are read by config from inet:s/inet.config.
*		They are then stored in a global memory location.
*
*   BUGS
*
*   SEE ALSO
*		getgid()
*
******************************************************************************
*
*/

int
getgroups(num, gp)
	int	num, *gp;
{
	register struct config *cf;
	register int i;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	for(i = 0; i < num && i < cf->num_gid; i++){
		gp[i] = cf->gids[i];
	}

	return (int)i;
}

