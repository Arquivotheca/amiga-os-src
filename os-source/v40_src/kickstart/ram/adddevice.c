/* adddevice.c */

#include "ram_includes.h"
#include "ram.h"

extern struct DosLibrary *DOSBase;

/* should really be called under forbid, so caller can set things up */

struct DeviceList *
adddevice (name)
	char *name;
{
	struct DosInfo *devinfo;
	struct DeviceList *devslot;
	int len;
	char *devname;

	devinfo = (struct DosInfo *)
		  BADDR(((struct RootNode *) DOSBase->dl_Root)->rn_Info);
	DBUG1("DevInfo is at 0x%lx",(LONG)devinfo);
	devslot = (struct DeviceList *)
		  getvecclear(sizeof(struct DeviceList) >> 2);
	DBUG1("DevSlot is at 0x%lx",(LONG)devslot);

	len = strlen(name);
	devname = getvec((len+1+1+3) >> 2);
	if (!devslot || !devname)
	{
		freevec((char *) devslot);
		freevec(devname);
		return NULL;
	}
	strcpy(devname,name);
	CtoBstr(((LONG) devname) >> 2);		/* takes a BPTR to C str */
	devname[*devname+1] = '\0';

	devslot->dl_Name = (BSTR) (((LONG) devname) >> 2);

	/* insert into devlist (forbid locking) */
	Forbid();
	devslot->dl_Next    = devinfo->di_DevInfo;
	devinfo->di_DevInfo = ((LONG) devslot) >> 2;
	Permit();

	return devslot;
}
