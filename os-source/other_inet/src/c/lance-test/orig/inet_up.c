/*
** inet_up() - return true or false depending on whether the network is
**	       up or not.
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <netinet/inet.h>

extern	struct	ExecBase *SysBase;

extern	int	inet_up(void);

int	inet_up()
{
	struct Node *	found;

	Forbid();
	found = FindName(&SysBase->LibList, INETNAME);
	Permit();

	if(!found){
		return 0;
	}

	return 1;
}
