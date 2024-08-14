//
// XPT (transport) CAM layer.  Follows the UNIVOS (aka Unix) OS-Dependant
//	 specification of rev 3.0 of the SCSI-2 CAM spec.
//
// Implemented as an amiga shared library.
//
#include HEADER

#include <exec/types.h>
#include "modifiers.h"

#ifdef USES_CAM
#include <exec/lists.h>
#include <exec/memory.h>
#include <utility/hooks.h>
#include <devices/scsidisk.h>
#include <dos/dostags.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <stdlib.h>
#define USE_BUILTIN_MATH
#include <string.h>

#include "cam.h"
#include "scsi.h"

typedef CCB_SIZE_UNION CCB;

#include "xpt_protos.h"

#define AllocNew(x)	AllocMem(sizeof(x),MEMF_CLEAR)
#define ASM		__asm
#define REG(x)		register __ ## x
#define INLINE		__inline

extern struct Library *SysBase;	// FIX!

struct xpt_AsyncInfo {
	struct MinNode Node;
	CCB_SETASYNC csa;
};

struct xpt_Sim {
	CAM_SIM_ENTRY sim;
	struct MinList AsyncList;	/* list of xpt_AsyncInfo's */
	struct MinList LunList;		/* list of luns on this bus */
	UBYTE in_use;
};

struct xpt_LunInfo {
	struct MinNode Node;
	UBYTE id;
	UBYTE lun;
	UBYTE lun_InquiryData[INQLEN];
};

static struct xpt_Sim *sims;	// this is re-allocated on every sim add
static UBYTE num_sims;		// size of array


LONG
xpt_init ()
{
	// initialize, search for SIM drivers
	// FIX!

	// set up the default sim entry for xpt
	num_sims = 0;
	sims = AllocMem(sizeof(struct xpt_Sim),0);
	if (!sims)
		return; // FIX! ???
	sims[0].in_use = TRUE;		// really XPT
	sims[0].sim.sim_action = default_action;

	// initialize library...
}

CCB *
xpt_ccb_alloc ()
{
	CCB *ptr;

	// Allocate a CCB for the application
	// FIX!!!! use pools!!!! (under 3.0+)
	ptr = (void *) AllocMem(sizeof(CCB),MEMF_PUBLIC|MEMF_CLEAR);
	if (ptr)
	{
		ptr->csio.cam_ch.my_addr = &(ptr->csio.cam_ch);
		ptr->csio.cam_ch.cam_ccb_len = sizeof(CCB);
	}
	return ptr;
}

void
xpt_ccb_free (CCB *ccb)
{
	// FIX! use pools under 3.0
	FreeMem((void *) ccb,sizeof(*ccb));
}

// This is the main entry point for all requests

LONG
xpt_action (CCB *ccb)
{
	struct xpt_LunInfo *lun;
	struct xpt_Sim *sim;

	// always check the path id for all requests EXCEPT a few.
	// we'll handle those first
	switch (ccb->csio.cam_ch.cam_func_code) {
	case XPT_NOOP:		/* Execute Nothing */
		return reply_ccb(ccb,CAM_REQ_CMP);

	case XPT_PATH_INQ:	/* Path Inquiry */
		if (ccb->cpi.cam_ch.cam_path_id == 0xff)
		{
			// 0xff is special, return highest path value
			ccb->cpi.cam_hpath_id = num_sims-1;
			return reply_ccb(ccb,CAM_REQ_CMP);
		}
		sim = find_sim(ccb->cpi.cam_ch.cam_path_id);
		if (!sim)
			return reply_ccb(ccb,CAM_PATH_INVALID);
		// FIX!
		return ;
	}

	// all further requests require sim */
	sim = find_sim(ccb->csio.cam_ch.cam_path_id);
	if (!sim)
		return reply_ccb(ccb,CAM_PATH_INVALID);

	switch (ccb->csio.cam_ch.cam_func_code) {
	/* case XPT_NOOP: */
	/* case XPT_PATH_INQ: */

	case XPT_SCSI_IO:	/* Execute the requested SCSI IO */
	case XPT_REL_SIMQ:	/* Release the SIM queue that is frozen */
	case XPT_ABORT:		/* Abort the selected CCB */
	case XPT_RESET_BUS:	/* Reset the SCSI bus */
	case XPT_RESET_DEV:	/* Reset the SCSI device, BDR */
	case XPT_TERM_IO:	/* Terminate the I/O process */
	case XPT_ENG_INQ:	/* HBA engine inquiry */
	case XPT_ENG_EXEC:	/* HBA execute engine request */
	case XPT_EN_LUN:	/* Enable LUN, Target mode support */
	case XPT_TARGET_IO:	/* Execute the target IO request */
		// The SIM handles all of these
		return (*(sim->sim.sim_action))(&(ccb->csio.cam_ch));

	case XPT_GDEV_TYPE:	/* Get the device type information */
		lun = find_unit(sim,&(ccb->cgd.cam_ch));
		if (!lun)
			return reply_ccb(ccb,CAM_DEV_NOT_THERE);

		if (ccb->cgd.cam_inq_data)
			memcpy(ccb->cgd.cam_inq_data,lun->lun_InquiryData,
			       INQLEN);
		ccb->cgd.cam_pd_type = lun->lun_InquiryData[0] >> 5;
		return reply_ccb(ccb,CAM_REQ_CMP);


	case XPT_SASYNC_CB:	/* Set Async callback parameters */
		// the callback value is the key.  see if this is a change
		// all async requests must be passed to SIM, so it can
		// handle them directly if it wishes.  path/ID/LUN must be
		// valid, and it only applies to one LUN.
		{
			struct xpt_AsyncInfo *async;

			for (async = (void *) sim->AsyncList.mlh_Head;
			     async->Node.mln_Succ;
			     async = (void *) async->Node.mln_Succ)
			{
			    if (async->csa.cam_async_func ==
				ccb->csa.cam_async_func)
			    {
				if (ccb->csa.cam_async_flags)
				{
				    // change flags
				    async->csa.cam_async_flags =
						   ccb->csa.cam_async_flags;
				} else {
				    // flags = 0 means de-register
				    Remove((struct Node *) &(async->Node));
				    FreeMem(async,sizeof(*async));
				}
				return (*(sim->sim.sim_action))
							(&(ccb->csio.cam_ch));
			    }
			}

			// new request
			if (!(async = AllocNew(struct xpt_AsyncInfo)))
				return reply_ccb(ccb,CAM_REQ_CMP_ERR);

			// save copies of all fields
			async->csa = ccb->csa;

			// queue on the sim list of registered async's
			AddHead((struct List *) &(sim->AsyncList),
				(struct Node *) &(async->Node));
		}
		// FIX!? we shouldn't add unless the SIM accepts it!
		// do the sim first, _then_ us.!
		return (*(sim->sim.sim_action))(&(ccb->csio.cam_ch));
		
	case XPT_SDEV_TYPE:	/* Set the device type information */
		// fix? notify SIM?
		{
			UBYTE inq_data[1];

			inq_data[0] = ccb->csd.cam_dev_type << 5;
			if (add_unit(sim,ccb->csd.cam_ch.cam_target_id,
				     ccb->csd.cam_ch.cam_target_lun,
				     inq_data,1))
				reply_ccb(ccb,CAM_REQ_CMP);
			else
				reply_ccb(ccb,CAM_REQ_CMP_ERR);
		}
	default:
		if (ccb->csio.cam_ch.cam_func_code & XPT_VUNIQUE)
			// vendor-unique command, pass on to sim
			return (*(sim->sim.sim_action))(&(ccb->csio.cam_ch));
	}
	/* NOTREACHED */
}

LONG
xpt_bus_register (CAM_SIM_ENTRY *sim)
{
	LONG id;
	struct xpt_Sim *new_table;

// FIX! add locking!
	// find free path id
    	if (sims)
		for (id = 0; id < num_sims; id++)
			if (!sims[id].in_use)
			{
				sims[id].sim    = *sim;
				sims[id].in_use = TRUE;
				// async? fix?

// FIX! call sim_init(?)
				spawn_scan(id);	// start process to scan sim

				xpt_async(AC_SIM_REGISTER,0xff,-1,-1,&id,1);
				return id;
			}

	// need more space in sim table (sims[num_sims] is the xpt!)
	if (num_sims < 255)
	{
		if (new_table = AllocMem((num_sims+2)*sizeof(struct xpt_Sim),
					 0))
		{
		    for (id = 0; id <= num_sims; id++)
		    {
			new_table[id] = sims[id];

			// fix up lists
			new_table[id].AsyncList.mlh_Head->mln_Pred =
				(struct MinNode *)
					&(new_table[id].AsyncList.mlh_Head);
			new_table[id].AsyncList.mlh_TailPred->mln_Succ =
				(struct MinNode *)
					&(new_table[id].AsyncList.mlh_Tail);
			new_table[id].LunList.mlh_Head->mln_Pred =
				(struct MinNode *)
					&(new_table[id].LunList.mlh_Head);
			new_table[id].LunList.mlh_TailPred->mln_Succ =
				(struct MinNode *)
					&(new_table[id].LunList.mlh_Tail);
		    }
		    FreeMem(sims,(num_sims+1) * sizeof(struct xpt_Sim));

		    sims = new_table;
		    num_sims++;

		    id = num_sims-1;
		    sims[id].sim    = *sim;
		    sims[id].in_use = TRUE;
		    NewList((struct List *) &(sims[id].AsyncList));
		    NewList((struct List *) &(sims[id].LunList));

// FIX! call sim_init(?)
		    spawn_scan(id);	// start process to scan sim

		    xpt_async(AC_SIM_REGISTER,0xff,-1,-1,&id,1);
		    return id;
		}
	}
	return -1;	// failure
}

LONG
xpt_bus_deregister (UBYTE path_id)
{
	struct xpt_LunInfo *lun;

// FIX! remove luns from device table
	sims[path_id].in_use = FALSE;

	// forget all about that scsi bus...
	while (lun = (void *) sims[path_id].LunList.mlh_Head->mln_Succ)
	{
		Remove((struct Node *) &(lun->Node));
		FreeMem((void *) lun,sizeof(*lun));
	}

	// notify any waiters...
	xpt_async(AC_SIM_DEREGISTER,0xff,-1,-1,&path_id,1);

	return 0;
}

// -1's are wildcards...
void
xpt_async (LONG opcode, LONG path_id, LONG target_id, LONG lun,
	   void *buffer_ptr, LONG data_cnt)
{
	struct xpt_Sim *sim;
	struct xpt_AsyncInfo *async;
	UBYTE start,finish,id;		// starting and ending path id's
	LONG len;

	start  = 0;
	finish = num_sims-1;
	if (path_id != -1)
		start = finish = path_id;

	for (id = start; id <= finish; id++)
	{
	    sim = find_sim(id);
	    if (sim)
	    {
		for (async = (void *) sim->AsyncList.mlh_Head;
		     async->Node.mln_Succ;
		     async = (void *) async->Node.mln_Succ)
		{
		    if ((target_id == -1 ||
			 target_id == async->csa.cam_ch.cam_target_id) &&
			(lun == -1 ||
			 lun == async->csa.cam_ch.cam_target_lun) &&
			(async->csa.cam_async_flags & opcode))
		    {
			// bus, id, lun, and opcode match

			len = min(data_cnt,async->csa.pdrv_buf_len);
			if (len && buffer_ptr)
				memcpy(async->csa.pdrv_buf,buffer_ptr,len);

			// call user callback
			call_hook((CALLBACK *) async->csa.cam_async_func,
				  opcode,path_id,target_id,buffer_ptr,len);
		    }
		}
	    }
	}
}

/*************************************************************************/
// These are support functions, only visible inside XPT

// sims[0..num_sims-1] are real, sim[num_sims] is the xpt

static struct xpt_Sim *
find_sim (UBYTE path_id)
{
    if (path_id == 0xff)		// the XPT itself - mostly bogus entry
	return &(sims[num_sims]);

    if (path_id >= num_sims)
	return NULL;

    if (sims[path_id].in_use)
	    return &(sims[path_id]);
    return NULL;
}

static LONG INLINE
reply_ccb (CCB *ccb, LONG code)
{
    ccb->csio.cam_ch.cam_status = code;
    return code;
}

static LONG
default_action (CCB *ccb)
{
	reply_ccb(ccb,CAM_REQ_CMP_ERR);
}

static struct xpt_LunInfo *
find_unit (struct xpt_Sim *sim, CCB_HEADER *ccb)
{
	struct xpt_LunInfo *lun;

	for (lun = (void *) sim->LunList.mlh_Head;
	     lun->Node.mln_Succ;
	     lun = (void *) lun->Node.mln_Succ)
	{
		if (lun->id  == ccb->cam_target_id &&
		    lun->lun == ccb->cam_target_lun)
		{
			// found it
			return lun;
		}
	}
	return NULL;
}

static LONG
add_unit (struct xpt_Sim *sim, UBYTE id, UBYTE lun, UBYTE *data, ULONG datalen)
{
	struct xpt_LunInfo *newlun;

// FIX! add locking since we may be called from scan_path!
	// we're not checking for errors, like device already in list...
	newlun = AllocNew(struct xpt_LunInfo);
	if (newlun)
	{
		newlun->id  = id;
		newlun->lun = lun;
		// allocated clear...
		memcpy(&(newlun->lun_InquiryData[0]),data,
		       min(datalen,sizeof(newlun->lun_InquiryData)));
		AddTail((struct List *) &(sim->LunList),
			(struct Node *) &(newlun->Node));
	}
	return (LONG) newlun;	// really boolean success
}

typedef VOID (* ASM callhook)(REG(a0) CALLBACK *,
			      REG(a1) LONG *,
			      REG(a2) LONG *);

static void __stdargs
call_hook (CALLBACK *hook, LONG arg1, ...)
{
	// we know the args are on the stack, in order.  What an ugly cast...

	(*((callhook) hook->h_Entry))(hook,&arg1,(LONG *) NULL);
}

static void 
spawn_scan (UBYTE path_id)
{
	struct xpt_Sim *sim;
	struct Message *msg;
	struct Process *proc;
	struct Library *DOSBase;

	sim = find_sim(path_id);
	if (sim)
	{
		msg = AllocMem(sizeof(*msg),MEMF_PUBLIC | MEMF_CLEAR);
		if (!msg)
			return;

		DOSBase = OpenLibrary("dos.library",37);

		// we don't need much of a process.  Could use AddTask
		// and pass args on stack... CreateTask won't pass args.
		proc = CreateNewProcTags(NP_Entry,scan_path,
					 NP_Priority,5,
					 NP_Name,"XPT Scan",
					 NP_StackSize,1000,
					 NP_CopyVars,FALSE,
					 NP_CurrentDir,NULL,
					 NP_HomeDir,NULL);

		// mildly evil reuse of fields.  scan_path de-allocates...
		msg->mn_Node.ln_Pri = path_id;
		msg->mn_ReplyPort   = sim;
		PutMsg(&(proc->pr_MsgPort),msg);

		CloseLibrary(DOSBase);
	}
}

// we'll ignore the object field, since we know we're getting a CCB...
static void ASM
scan_callback (REG(a0) CALLBACK *hook, REG(a1) CCB *ccb)
{
	// we're being called on someone else's context...
	struct Library *SysBase = *((struct Library **) 4);

	Signal((struct Task *) ccb->csio.cam_pdrv_ptr, (ULONG) hook->h_Data);
}

static UBYTE
wait_callback (CCB *ccb, UBYTE sig)
{
	Wait(1L << sig);
	return ccb->csio.cam_ch.cam_status;
}

static void
scan_path ()
{
	struct xpt_Sim *sim;
	struct Message *msg;
	struct Hook scan_hook;
	CCB *ccb;
	UBYTE inquiry_data[INQLEN];
	UBYTE sense_data[12];
	UBYTE path_id,id,lun,hba_id;
	UBYTE i,sig;
	UBYTE rc;

	WaitPort(&(((struct Process *) FindTask(0))->pr_MsgPort));
	msg = GetMsg(&(((struct Process *) FindTask(0))->pr_MsgPort));

	// mildly evil, I'm reusing these fields to avoid allocating more...
	path_id = (UBYTE) msg->mn_Node.ln_Pri;
	sim = msg->mn_ReplyPort;
	FreeMem(msg,sizeof(*msg);

	sig = AllocSignal(-1);
	if (!sig)
		return;
	SetSignal(0,1L << sig);		// make sure it's clear...

	ccb = xpt_ccb_alloc();
	if (!ccb)
	{
		FreeSignal(sig);
		return;
	}

	// set up callback function
	scan_hook.h_Entry = (ULONG (*)()) scan_callback;
	scan_hook.h_Data  = (APTR) (1L << sig);	// signal to wake up parent

	// determine if the device exists
	// set up for inquiry command
	ccb->cpi.cam_ch.cam_func_code  = XPT_PATH_INQ;
	ccb->cpi.cam_ch.cam_path_id    = path_id;
	ccb->cpi.cam_ch.cam_target_id  = 0;
	ccb->cpi.cam_ch.cam_target_lun = 0;
// fix? are these really virtual?
	ccb->cpi.cam_ch.cam_flags      = CAM_DIR_IN | CAM_CDB_PHYS |
					 CAM_DATA_PHYS | CAM_SNS_BUF_PHYS |
					 CAM_CALLBCK_PHYS;

	if (xpt_action(ccb) != CAM_REQ_CMP)
	{
		hba_id = ccb->cpi.cam_initiator_id;

		// reset for next call...
		ccb->csio.cam_ch.cam_func_code	= XPT_SCSI_IO;
		ccb->csio.cam_data_ptr		= inquiry_data;
		ccb->csio.cam_dxfer_len		= sizeof(inquiry_data);
		ccb->csio.cam_sense_ptr		= sense_data;
		ccb->csio.cam_sense_len		= sizeof(sense_data);
		ccb->csio.cam_cdb_io.cam_cdb_ptr= NULL;
		ccb->csio.cam_cdb_len		= 6;
		for (i=0; i < sizeof(ccb->csio.cam_cdb_io.cam_cdb_bytes); i++)
			ccb->csio.cam_cdb_io.cam_cdb_bytes[i] = 0;
		ccb->csio.cam_cdb_io.cam_cdb_bytes[0] = S_INQUIRY;
		ccb->csio.cam_cdb_io.cam_cdb_bytes[4] = sizeof(inquiry_data);
		ccb->csio.cam_cbfcnp		= &scan_hook;
		ccb->csio.cam_pdrv_ptr		= (UBYTE *) FindTask(0);
		// flags already set for data in

		for (id = 0; id <= 7; id++)
		{
			if (id == hba_id)	// avoid selecting self
				continue;
			ccb->cpi.cam_ch.cam_target_id  = id;

			for (lun = 0; lun <= 7; lun++)
			{
				ccb->csio.cam_ch.cam_target_lun = lun;
				ccb->csio.cam_cdb_io.cam_cdb_bytes[1] = 
						lun << 5;

				rc = xpt_action(ccb);
				if (rc == CAM_REQ_INPROG)
					rc = wait_callback(ccb,sig);
				if (rc == CAM_REQ_CMP &&
				    inquiry_data[0] != 0x7f)
				{
					// we found a device!
// fix? is resid bytes not xfered (request - actual), or other way around?
					add_unit(sim,id,lun,inquiry_data,
						 sizeof(inquiry_data)-
						 ccb->csio.cam_resid);
				}
			}
		}
	}
	xpt_ccb_free(ccb);
	FreeSignal(sig);
}

#endif /* USES_CAM */
