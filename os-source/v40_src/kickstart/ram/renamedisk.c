/* renamedisk.c */

#include "ram_includes.h"
#include "ram.h"

/* rename the volume.  arg1 is the new name */
LONG
renamedisk (pkt)
	struct DosPacket *pkt;
{
	char *name;
	char *ns;
	int len;

	name = (char *) BADDR(pkt->dp_Arg1);
	len = *name;

	fileerr = ERROR_INVALID_COMPONENT_NAME;
	if (len < 1 || len > MAX_FILENAME)
		return FALSE;

	ns = AllocVec(len+2,MEMF_PUBLIC);
	if (ns == NULL)
	{
		fileerr = ERROR_DISK_FULL;
		return FALSE;
	}
	memcpy(ns,name,len+1);
	ns[len+1] = '\0';
	memcpy(root->name,name+1,len);		/* sync with our root */
	root->name[len] = '\0';

	DateStamp((struct DateStamp *) root->date);

	/* since other people may look at it! DANGER! assumes forbid locking */
	/* FIX */
	Forbid();
	freevec(BADDR(volumenode->dl_Name));		/* remove old name */
	volumenode->dl_Name = TOBPTR(ns);		/* add new one */
	Permit();

	return DOS_TRUE;
}
