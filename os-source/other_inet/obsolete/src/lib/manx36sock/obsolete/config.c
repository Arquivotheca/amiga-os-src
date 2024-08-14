/*
** Common code for config system
*/
 
#include <exec/types.h>
#include <exec/memory.h>
#ifdef LATTICE
#include <proto/exec.h>
#endif

#include <config.h>

char	__config_name[] = "inet.configuration";

_makeconfig()
{
	register struct config *cf;
	extern void *AllocMem();

	Forbid();
	GETCONFIG(cf);
	if(!cf){
		cf = (struct config *)AllocMem(sizeof(*cf), MEMF_PUBLIC|MEMF_CLEAR);
		strcpy(cf->configname, __config_name);
		cf->mp.mp_Node.ln_Name = cf->configname;
		NewList(&cf->mp.mp_MsgList);	/* more style than form */

		cf->gid = cf->uid = -2;
		strcpy(cf->username, "nobody");
		cf->umask = 022;
		cf->tz_offset = 300; /* Eastern Standard Time (EST) */

		AddPort(cf);
	}
	Permit();
}
