/****** socket/inet_up ******************************************
*
*   NAME
*		inet_up -- Check status of TCP/IP software
*
*   SYNOPSIS
*		return = inet_up()
*
*		int inet_up (void); 
*
*   FUNCTION
*		Returns true or false depending on whether the network is up or not.
*
*	INPUTS
*
*   RESULT
*		1 if the network is up. 0 otherwise.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/*
** inet_up() - return true or false depending on whether the network is
**	       up or not.
*/

#include <netinet/inet.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/nodes.h>

extern struct ExecBase *SysBase;

#ifdef LATTICE
#include <proto/exec.h>
#endif

int
inet_up()
{
	struct Node *found;

	Forbid();
	found = FindName(&SysBase->LibList, INETNAME);
	Permit();

	if(!found){
		return 0;
	}

	return 1;
}
