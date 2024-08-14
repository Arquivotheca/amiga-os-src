
#include <exec/io.h>
#include <exec/execbase.h>
#include <devices/timer.h>
#include <devices/trackdisk.h>
#include <resources/disk.h>
#include <resources/cia.h>
#include <hardware/cia.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include <clib/disk_protos.h>
#include <pragmas/disk_pragmas.h>

extern struct CIA ciaa,ciab;

#define MAX_STEP 85

extern char drName[];

#define D(x)	;

void
DRInit ()
{
	struct DiscResource *DiskBase;
	struct timerequest ior;
	struct MsgPort *port;

	// these really should be a UBYTE * volatile, but lc 5.10b gives
	// a warning message that might mean internal confusion.
	volatile UBYTE * volatile ciaapra, * volatile ciabprb;
	UBYTE pb;
	int i;

	DiskBase = (void *) OpenResource(drName);
	if (!DiskBase || DiskBase->dr_Library.lib_Version < 36)
		return;

	if (GetUnitID(0) != DRT_AMIGA)
		return;

	// this code plays EVIL games.  We assume no one is using the
	// drive!  This must come IMMEDIATELY after timer.device (and as soon
	// as possible after disk.resource).

	port = CreateMsgPort();
	if (!port)
		return;

	ior.tr_node.io_Message.mn_ReplyPort = port;
	if (OpenDevice("timer.device",UNIT_MICROHZ,&(ior.tr_node),0L))
		goto cleanup;

	ior.tr_node.io_Command = TR_ADDREQUEST;

	// now, step the head 85 times max or until we see track 0
	// if we don't see track 0, mark it as no drive present.
	// This will take 85*~.004 + ~.016 seconds, or about .35 seconds

	// for better code
	ciaapra = &(ciaa.ciapra);
	ciabprb = &(ciab.ciaprb);

	// step out, drive 0, motor off
	pb = ~CIAF_DSKSEL0; // sets direction out, bottom side, motor off

	// set the values first
	*ciabprb = pb;
	for (i = 0; i < MAX_STEP; i++)
	{
		// check for track 0
		if (!(*ciaapra & CIAF_DSKTRACK0))
			break;		/* found a drive! */

		// step the head
		*ciabprb = pb & ~CIAF_DSKSTEP;
		*ciabprb = pb;

		// wait 3 ms
		ior.tr_time.tv_secs  = 0;
		ior.tr_time.tv_micro = 3000;
		DoIO(&(ior.tr_node));
	}

	ior.tr_time.tv_secs  = 0;
	ior.tr_time.tv_micro = 15000;	/* settle */
	DoIO(&(ior.tr_node));

	if (i == MAX_STEP)
	{
		// no drive found
		DiskBase->dr_UnitID[0] = DRT_EMPTY;
	}

	CloseDevice(&(ior.tr_node));
cleanup:
	DeleteMsgPort(port);
}

// for romtag

void
EndCode()
{
}
