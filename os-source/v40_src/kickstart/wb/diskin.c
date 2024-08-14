/*
 * $Id: diskin.c,v 38.3 92/04/27 14:22:58 mks Exp $
 *
 * $Log:	diskin.c,v $
 * Revision 38.3  92/04/27  14:22:58  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 * 
 * Revision 38.2  92/02/13  14:09:09  mks
 * Changed the call to PlaceObj()
 *
 * Revision 38.1  91/06/24  11:34:50  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "intuition/intuition.h"
#include "global.h"
#include "support.h"
#include "diskchange.h"
#include "quotes.h"

FindDeviceNode( ad, volad )
struct ActiveDisk *ad;
struct ActiveDisk *volad;
{
    MP(("FindDeviceNode: ad->ad_Name='%s', volad->ad_Name='%s'\n",
	ad->ad_Name, volad->ad_Name));
    return( ad->ad_Active == VOL_DEVICE &&
	ad->ad_Handler == volad->ad_Handler );
}


DiskInserted(ad)
struct ActiveDisk *ad;
{
struct WorkbenchBase *wb = getWbBase();
struct WBObject *obj;
struct Window *win;
int result = 0;
LONG lock = NULL;
struct DeviceList *dl;
char *name = wb->wb_Buf0;

    DP(("DiskInserted: entry, ad=$%lx, name='%s'\n", ad, ad->ad_Name));

    if (!(obj = ad->ad_Object)) {
	DP(("DiskInserted: calling ManufactureDiskLock\n"));
	if (!(lock = ManufactureDiskLock(ad))) {
	    ad->ad_Active = VOL_IGNORE;
	    DP(("DiskInserted: ad=%lx (%s), state=%ld (VOL_IGNORE)\n",
		ad, ad->ad_Name, ad->ad_Active));
	    goto end;
	}

	DP(("DiskInserted: calling Info\n"));
	Info(lock, &ad->ad_Info);

	dl = (struct DeviceList *)BADDR(ad->ad_Volume);
	BtoC(name, (UBYTE *)BADDR(dl->dl_Name));

	DP(("DiskInserted: calling CURRENTDIR on lock %lx\n", lock));
	CURRENTDIR(lock);

	DP(("DiskInserted: calling GetWBObject for 'Disk'\n"));
	if (obj = WBGetWBObject(DiskIconName))
	{
	    /* Make sure we realy have a disk icon */
	    if (obj->wo_Type!=WBDISK)
	    {
		WBFreeWBObject(obj);
		obj=NULL;
	    }
	}

	if (obj)
	{
	    DP(("DiskInserted: obj=%lx (%s)(%s)\n",obj, obj->wo_Name, name));

	    /* now change the name */
	    obj->wo_Name = objscopy(obj, name);

	    SetIconNames(obj);
	    if (!obj->wo_Name) {
		DP(("DI: calling WBFreeWBObject(obj)\n"));
		WBFreeWBObject(obj);
		goto unlock;
	    }
	}
	else
	{
	    DP(("DiskInserted: calling AllocWBObject for '%s'\n", name));
	    if (!(obj = WBAllocWBObject(name))) {
		DP(("DiskInserted: could not AllocWBObject\n"));
		goto unlock;
	    }
	    DP(("DiskInserted: calling initgadget2\n"));
	    obj->wo_Type = WBDISK;
	    if (!initgadget2(obj)) {
		DP(("DiskInserted: could not initgadget2\n"));
		goto freeobj;
	    }

	    DP(("ActiveSearch(FindDeviceNode, ad)\n"));
	    /* see if this is a drive we are not tracking */
	    if (!ActiveSearch(FindDeviceNode, ad)) {
		/* forget about the disk copy program */
		obj->wo_DefaultTool = 0;
	    }
	}
	DP(("DiskInserted: default tool %s\n",
	    obj->wo_DefaultTool ? obj->wo_DefaultTool : "<no default>"));

	DP(("DI: calling AddDiskIcon(ad, obj) for '%s'\n", obj->wo_Name));
	AddDiskIcon(ad, obj);

    }
    else if (win = obj->wo_DrawerData->dd_DrawerWin) {
	DP(( "DI: disk %s already in... obj 0x%lx\n", name, obj ));
	WindowToFront(win);
    }

    result = 1; /* all ok */

    /* David Berezowski - April/90 */
    /* check for 'left out' icons. */
    if (obj) {
	DP(("DI: calling FillBackdrop\n"));
	FillBackdrop(GetParentsLock(obj));
    }

unlock:
    UNLOCK(lock);	/* UNLOCK checks for NULL */

end:
    DP(("DiskInserted: exit, result=%ld\n", result));
    return(result);

freeobj:
    DP(("DI: freeobj:: calling WBFreeWBObject(obj)\n"));
    WBFreeWBObject(obj);
    goto unlock;
}

/*
 * MKS: This new version tries not to force the insert of a disk
 *	when reading and will just let it wait until the next
 *	pass at the device list.
 */
LONG ManufactureDiskLock(struct ActiveDisk *ad)
{
struct WorkbenchBase *wb=getWbBase();
LONG lock=NULL;
short flag=FALSE;

	while (!flag)
	{
	struct MsgPort *port=NULL;

		/* Lock the device list for us to read... */
		LockDosList(LDF_VOLUMES | LDF_READ);
		if (VolumeListSearch(FindVolume,(LONG)ad,NULL,NULL,NULL))
		{
			port=ad->ad_Handler=((struct DeviceList *)BADDR(ad->ad_Volume))->dl_Task;
		}
		UnLockDosList(LDF_VOLUMES | LDF_READ);

		/* We assume that we won't do this again */
		flag=TRUE;

		/* If we got a port, we should try to get a lock... */
		if (port)
		{
			strcpy(wb->wb_Buf1,"\1:"); /* BSTR for ":" */
			lock=DoPkt3(port,ACTION_LOCATE_OBJECT,NULL,((LONG)wb->wb_Buf1 >> 2),SHARED_LOCK);
			if (!lock) flag=ErrorReport(IOERR(),REPORT_LOCK,NULL,port);
		}
		else
		{
			/*
			 * If we can not find the port, we will ask for the
			 * disk via the DOS.library support call...
			 */
			flag=ErrorReport(ERROR_DEVICE_NOT_MOUNTED,REPORT_INSERT,(ULONG)ad->ad_Name,NULL);
		}
	}
	return(lock);
}

void AddDiskIcon(ad, obj)
struct ActiveDisk *ad;
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();

    ad->ad_Object = obj;
    PlaceObj(obj, wb->wb_RootObject);
    AddIcon(obj, wb->wb_RootObject);
    MinMaxDrawer(wb->wb_RootObject);
}
