/*
** inet_up() - return true or false depending on whether the network is
**	       up or not.
*/

#include <netinet/inet.h>
#include <exec/types.h>
#include <exec/execbase.h>
extern struct ExecBase *SysBase;

#ifdef LATTICE
#include <proto/exec.h>
#endif

inet_up()
{
	int	found;

	Forbid();
	found = FindName(&SysBase->LibList, INETNAME);
	Permit();

	if(!found){
		return 0;
	}

	return 1;
}
