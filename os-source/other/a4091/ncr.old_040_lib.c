#include HEADER

#ifdef NCR53C710
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/alerts.h>
#include <resources/battmembitsshared.h>
#include <resources/battmembitsamiga.h>
#include <resources/battmem.h>
#include <devices/trackdisk.h>
#include <hardware/cia.h>
#include <clib/exec_protos.h>

#define SysBase (g->st_SysLib)

#include <pragmas/exec_pragmas.h>
#include <clib/battmem_protos.h>
#include <pragmas/battmem_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/alib_protos.h>

// for offsetof()
#include <stddef.h>
#define USE_BUILTIN_MATH
#include <string.h>	// for builtin min()
#include "scsi.h"

#include "modifiers.h"
#include "ncr710.h"
#include "device.h"
#include "iotask.h"
#include "scsitask.h"
#include "cam.h"
#include "ncr_ints.h"
#include "ncr_protos.h"

#include "script.c"

#define ASM		__asm
#define REG(x)		register __ ## x
#define INTERRUPT	__interrupt

void ASM WaitASecond(REG(a6) struct ExecBase *sysbase);
void kprintf(char *,...);
LONG INTERRUPT ASM ncr53c710_int(REG(a1) struct SCSIGlobals *g);
static void CleanUp(struct SCSIGlobals *g, struct CommandBlock *msg,
		    struct HDUnit *unit);

#define SIZEOF(x) (sizeof(x)/sizeof(x[0]))

#define LABEL(x)	(((ULONG) g->st_Script) + Ent_ ## x)

// For '040 caching issues, write to different address than read for longwords.
// Not required, but better, especially for 'EC040's (or any '040 not using
// MMU tables).
// Macro is ugly to avoid the "modifiable lvalue required for assignment"
// C annoyance.
#define WRITE_LONG(base,reg,val)	\
	((volatile ULONG *) (base))[NCR_WRITE_OFFSET/sizeof(ULONG) + \
			   offsetof(struct ncr710,reg)/sizeof(ULONG)] = val

// 50 MHz currently
#define NCR_CLOCK_FREQ 50

// turn on single-stepping mode...
//#define SINGLE_STEP

// do we have the new '040 library?  If not, use code to split xfers.
#define NEW_040_LIB

// can be used to force no disconnects
//#define DISABLE_DISCONNECT

// To disable tagged queuing
//#define DISABLE_TAGS

// To disable Sync negotiation initiation
//#define DISABLE_SYNC

// can be used to force polling from the CIA int chain...  max ~10ints/sec
//#define DISABLE_INTS

// can be used to force non-quick ints
//#define DISABLE_QUICK_INTS
#ifdef IS_A4000T
#define DISABLE_QUICK_INTS
#endif

#ifdef DISABLE_INTS
#define DISABLE_QUICK_INTS
#endif

#ifdef HUH
extern volatile struct CIA ciaa;

static void
poll_cia (ULONG micros)
{
	UBYTE foo;

//kprintf("ciaa.ciapra is at $%lx, %ldus\n",&(ciaa.ciapra),micros);
	while (micros--)
		foo = ciaa.ciapra;	// guaranteed >= 1us - harmless
}
#endif

extern void ASM WaitAWhile(REG(d0) ULONG secs, REG(d1) ULONG micros);

static void
poll_cia (ULONG micros)
{
//kprintf("%ldus\n",micros);
	WaitAWhile(0,micros);
}

// loops handling commands forever
static void
loop (struct SCSIGlobals *g)
{
	ULONG mask,cmdmask,waitmask;

	cmdmask = 1L << g->st_CmdPendSig;
	waitmask = g->st_WaitMask;
	for (;;) {
		mask = Wait(waitmask);
		if (g->st_TaskPointer != g->st_IntPointer)
			HandleInts(g);

		if (mask & cmdmask)		// save function call...
			HandleCommands(g);
	}
	// NOTREACHED
}

// Process commands that come to CmdPort
static void
HandleCommands (struct SCSIGlobals *g)
{
	struct CommandBlock *msg;

	while (msg = (struct CommandBlock *) GetMsg(g->st_CmdPort))
	{
		struct HDUnit *unit;

		msg->cb_ErrorCode = 0;		// assume no errors
		msg->cb_Programmed = 0;		// assume not Z-II
		msg->cb_DSA.status_buf[0] = 0;	// and status = OK

		// queue request for the appropriate unit
		unit = (struct HDUnit *) msg->cb_IORequest->io_Unit;
//kprintf("got IOR msg $%lx, unit $%lx, tail = $%lx\n",msg,unit);

		// don't allow it to select itself...
		if (unit->hu_Unit == g->st_OwnID)
		{
			msg->cb_ErrorCode = HFERR_SelfUnit;
			goto error_reply;
		}

		// if we're not supporting LUNs, don't bother even scheduling
		if (unit->hu_LUN > g->st_MaxLUN)
		{
//kprintf("ignoring unit %ld LUN %ld\n",unit->hu_Unit, unit->hu_LUN);
			msg->cb_ErrorCode = HFERR_SelTimeout;

error_reply:		msg->cb_SCSICmd->scsi_Actual = 0;
			msg->cb_SCSICmd->scsi_CmdActual = 0;
			msg->cb_SCSICmd->scsi_SenseActual = 0; // fix? needed?
			ReplyMsg(msg);
			continue;
		}

		// only one command active per unit, without tags
		if (unit->hu_QueueType == UNIT_QUIET ||
#ifndef DISABLE_TAGS
		    unit->hu_TagsAllowed)
#else
		    0)
#endif
		{
			// Wake up SCSI chip, tell it there is a command for
			// unit x.
			Schedule(g,unit,msg,TRUE);
		} else {
			// Add to list of unscheduled commands for this unit
			// no locking needed, only this task modifies the list
//kprintf("queued IOR $%lx for unit $%lx\n",msg,unit);
			AddTail((struct List *) &(unit->hu_CmdList),
				(struct Node *) msg);
		}
	}
}

// This must handle both quick ints and non-quick ints, since we may not
// get a quick int vector assigned.

#ifdef USE_C_INT_HANDLER

LONG INTERRUPT ASM
ncr53c710_int(REG(a1) struct SCSIGlobals *g)
{
	LONG handled = 0;		// for non-quick
	UBYTE istat;
	ULONG status;
	volatile struct ncr710 *b = g->st_BoardAddress;

	// make sure we _really_ got an interrupt
//	while ((istat = b->istat) & (ISTATF_SIP | ISTATF_DIP))
if ((istat = b->istat) & (ISTATF_SIP | ISTATF_DIP))
	{
		handled = 1;
		status = *((volatile ULONG *) &(b->sstat2));

#ifndef NEW_040_LIB
		// try to handle scatter DMA's that are not combined with
		// other ints and don't need to call CachePreDMA for '040
		// alignment handling
		if (g->st_Is040 &&
		    (istat & (ISTATF_SIP | ISTATF_DIP)) == ISTATF_DIP &&
		    (status & 0x7f) == DSTATF_SIR &&
		    b->dsps == scatter_dma)
		{
		    // ok, except for the case of VM or data overrun, we
		    // have a split alignment DMA.
		    struct CommandBlock *msg;
		    ULONG newactual,newcurr;
		    void *physaddr;

//kprintf("handling scatter in int...\n");
		    CacheClearE(&(g->st_CurrentDSA),4,CACRF_ClearD);
		    msg = (void *) (((UBYTE *)g->st_CurrentDSA) -
				    offsetof(struct CommandBlock,cb_DSA));
		    CacheClearE(msg,sizeof(*msg),CACRF_ClearD);

		    // make sure it's not an overflow or a VM-split xfer
		    newactual = msg->cb_SCSICmd->scsi_Actual +
				msg->cb_CurrentLength;
		    newcurr = msg->cb_SCSICmd->scsi_Length - newactual;

		    // two valid cases: just finished the head, and just
		    // finished the body.  In both cases, no CachePreDMA is
		    // needed.
		    if (newactual != msg->cb_SCSICmd->scsi_Length &&
			(newactual == msg->cb_HeadSize ||
			 newcurr   == msg->cb_TailSize))
		    {
			msg->cb_SCSICmd->scsi_Actual = newactual;

			if (newcurr == msg->cb_TailSize)
			{
			    // only tail section to finish
			    // no need to call CachePreDMA...
			    msg->cb_CurrentLength = newcurr;
			    physaddr		  = msg->cb_TailData;

			} else {
			    // we're doing the body, CachePreDMA already called
			    // size returned by it in msg->cb_CacheLength
			    msg->cb_CurrentLength = msg->cb_CacheLength;
			    physaddr		  = msg->cb_CacheData;
			}
//kprintf("new addr $%lx, len $%lx\n",physaddr,msg->cb_CurrentLength);
			msg->cb_DSA.move_data.addr = (ULONG) physaddr;
			msg->cb_DSA.move_data.len  = msg->cb_CurrentLength;
			msg->cb_DSA.final_ptr      = ((UBYTE *) physaddr) +
						     msg->cb_CurrentLength;
			CacheClearE(&(msg->cb_DSA.move_data),
				    sizeof(msg->cb_DSA.move_data),
				    CACRF_ClearD);

			// make the chip continue at the next instruction
			b->dcntl |= DCNTLF_STD;

			// ok, don't queue this interrupt, pray there were no
			// queued ints...   FIX?
			goto ignore_int;
		    }
		}
#endif // NEW_040_LIB

		g->st_IntData[g->st_IntPointer]   = status;
		g->st_ISTATData[g->st_IntPointer] = istat;

		// wake him up if needed (only signal once for multiple ints)
		if (g->st_IntPointer == g->st_TaskPointer)
			Signal(g->st_ThisTask,g->st_IntPendMask);

		g->st_IntPointer++;
		if (g->st_IntPointer == SIZEOF(g->st_IntData))
			g->st_IntPointer = 0;

//kprintf("int: istat $%02lx, status $%08lx (%ld:%ld)\n",istat,status,
//g->st_IntPointer,g->st_TaskPointer);

//if (g->st_IntPointer == g->st_TaskPointer)
//{kprintf("INTERRUPT OVERRUN $%08lx!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",
//istat);
//g->st_IntPointer--;
//}

ignore_int:	;
	} // while or if...

	return handled;
}
#endif // USE_C_INT_HANDLER

/* used to remember whether an IOR is active... */
#define STATE_ACTIVE 1
#define STATE_UNUSED 0

ULONG clear_ack_inst= 0x60000040;		// CLEAR ACK
ULONG my_nop_inst   = 0x80000000;		// NOP - jump never

static void
Schedule (struct SCSIGlobals *g,
	  struct HDUnit *unit,
	  struct CommandBlock *msg,
	  UBYTE running)
{
	struct SCSICmd *scsicmd = msg->cb_SCSICmd;
	void *physaddr;
	UWORD i,tag;

//kprintf("Schedule $%lx (%ld), DSA $%lx, unit $%lx(%ld:%ld), command $%lx\n",
//msg,msg->cb_Index,&(msg->cb_DSA),unit,unit->hu_Unit,unit->hu_LUN,
//*(scsicmd->scsi_Command));
//kprintf("scsi_Length = $%lx, io_Length = $%lx\n",scsicmd->scsi_Length,
//msg->cb_IORequest->io_Length);

	// add to queue of units waiting for device
	unit->hu_QueueType = UNIT_WAITING;

#ifdef ZORRO_3
	// is this in Zorro-II space?
	// NOTE: the first condition will make it really schedule it the
	// second time it's seen here, which in important for UnSchedule().
	if (!msg->cb_Programmed &&
	    !(((ULONG) scsicmd->scsi_Data) & 0xff000000) &&
// FIX!!!! REMOVE!!!! TEMP for kludge a4000t!
#ifdef IS_A4000T
	    1)
#else
	    !(TypeOfMem(scsicmd->scsi_Data)&MEMF_CHIP))
#endif
	{
kprintf("DMA from Z-3 to Z-2 space ($%lx)\n",scsicmd->scsi_Data);
	    msg->cb_Programmed = 1;	// flag to use buffer

	    // if we don't have a buffer, get one.
	    if (g->st_Z2Buffer == NULL)
	    {
		if (!(g->st_Z2Buffer = AllocMem(Z2_BUFFERSIZE,
					        MEMF_FAST)))
		{
		    // NO BUFFER! nuke it.  CleanUp replies msg
no_buffer:	    msg->cb_ErrorCode = TDERR_NoMem;
		    CleanUp(g,msg,unit);
		    return;
		}
		// this may deallocate and reallocate if it got chip mem...
		if (!(((LONG) g->st_Z2Buffer) & 0xff000000))
		{
kprintf("reallocating Z3->Z2 buffer ($%lx)\n",g->st_Z2Buffer);
		    // we allocated Z-2 memory.  Oops.  Free and try chip.
		    FreeMem(g->st_Z2Buffer,Z2_BUFFERSIZE);
		    if (!(g->st_Z2Buffer = AllocMem(Z2_BUFFERSIZE,
						    MEMF_CHIP)))
		    {
			goto no_buffer;
		    }
		}
				
	    }
	    // ok, Z-II.  Make sure the 1 buffer is avail.
	    if (!IsListEmpty((struct List *) &(g->st_Z2List)))
	    {
		// if we aren't scheduled, the previous owner
		// will schedule us when they're done.
		AddTail((struct List *) &(g->st_Z2List),msg);
		return;
	    }
	    // no one using the buffer, start the IO
	}
#endif
	// mark this entry as active...  various things need to scan for
	// active IO's.
	msg->cb_StateSave = STATE_ACTIVE;

	// fetch index for this entry, so we can access related info
	i = msg->cb_Index;

	// don't need to update move commands in reselect script
	// (see page 11-3 in the '710 manual)
	// Instruction is Move Memory 4,addr,DSA
	// addr is &(DSA_values[i]), and though the DSA value
	// can change, the DSA_values array is never moved.
	// DSA_values[i]   = &(msg->cb_DSA.label);
	// Since we now never change the index, even the DSA_Values array can
	// never change.

	// We assume that the lun already has an entry in the ID queue
	// We must set up the reselect entry for this nexus
	// and add it into the lun list for reselect (after flushing these).
#ifndef DISABLE_TAGS
	tag = unit->hu_TagsAllowed ? i+1 : 0;
#else
	tag = 0;
#endif
	msg->cb_DSA.reselect_entry.check_tag.data = tag;
	msg->cb_DSA.reselect_entry.check_tag.addr =
					    unit->hu_Find_lun.check_tag.addr;

	// reset DSA_entry in CmdBlock
	// make the set sync instruction use the right bits
	// also set the table-indirect Select block for the unit/sync
	msg->cb_DSA.select_data.sync		 = unit->hu_Addr->ha_SyncValue;
	msg->cb_DSA.select_data.id		 = 1 << unit->hu_Unit;

	// Call CachePreDMA to make sure the 680x0 caches have been flushed
	// to memory before we DMA into it.  We must do CachePostDMA after
	// io completion.
	// Also could build gather-scatter list here...
	// FIX! needs to be done in BeginIO!  (How do we handle SIMs?)
	msg->cb_DidCacheStuff	= 0;
	scsicmd->scsi_Actual	= 0;
	scsicmd->scsi_CmdActual = 0;

	msg->cb_CurrentLength = scsicmd->scsi_Length;
	msg->cb_CacheLength   = msg->cb_CurrentLength;
	physaddr	      = (UBYTE *) scsicmd->scsi_Data;
	msg->cb_CacheData     = physaddr;

	if (msg->cb_CurrentLength != 0)
	{
		UBYTE *data = physaddr;
		UBYTE *newdata,*newend;
		ULONG flags;

		flags = (scsicmd->scsi_Flags & (1<<SCSIB_READ_WRITE) ?
			 0 : DMA_ReadFromRAM);
			 
//if (g->st_Is040)
//kprintf("directon: %s\n",
//scsicmd->scsi_Flags & (1<<SCSIB_READ_Write) ? "Read" : "Write");
//if (*((UBYTE *)scsicmd->scsi_Command) == S_WRITE &&
//    (scsicmd->scsi_Flags & (1<<SCSIB_READ_WRITE)))
//kprintf("scsi_Flags ($%lx) wrong!  SCSIF_WRITE not given for Write command!\n",
//scsicmd->scsi_Flags);
//if (*((UBYTE *)scsicmd->scsi_Command) == S_READ &&
//    !(scsicmd->scsi_Flags & (1<<SCSIB_READ_WRITE)))
//kprintf("scsi_Flags ($%lx) wrong!  SCSIF_READ not given for Read command!\n",
//scsicmd->scsi_Flags);

#ifndef NEW_040_LIB
		if (g->st_Is040)
		{
			// '040's don't like DMA to non-16 byte boundaries.
			// Use cb_HeadData/cb_TailData to force it to 16 byte
			// boundaries.
			newdata = (UBYTE *) (((ULONG) data + 15) & 0xfffffff0);
			newend  = (UBYTE *) (((ULONG) data +
					              msg->cb_CurrentLength) &
					     0xfffffff0);
			msg->cb_HeadSize = newdata - data;
			msg->cb_TailSize = (data + msg->cb_CurrentLength) -
					   newend;
// FIX!!! broken!!!!  length = 0xfffffff0??

			// we want to do this for writes as well.  They don't
			// break, but this avoids calling CachePreDMA on a
			// non-16 byte alignment, so the caches stay on.
			if (!(scsicmd->scsi_Flags & (1 << SCSIB_READ_WRITE)))
			{
				// for writes, we must copy the head/tail data
				// to the head/tail areas.
			    if (msg->cb_HeadSize)
				memcpy(msg->cb_HeadData,data,
				       msg->cb_HeadSize);

			    if (msg->cb_TailSize)
				memcpy(msg->cb_TailData,newend,
				       msg->cb_TailSize);

			}

			// the entire message is CacheClearE()'d later...

			// could add optimization so that if there is no
			// middle, we only have a 1-31 byte head, since we
			// know tail follows head.  Happens so rarely it's not
			// worth bothering with.
		} else
			msg->cb_HeadSize = msg->cb_TailSize = 0;

//if (msg->cb_HeadSize || msg->cb_TailSize)
//{kprintf("old ptr $%08lx, len $%lx\n",data,msg->cb_CurrentLength);
//kprintf("head = %ld, tail = %ld, headdata = $%lx, taildata = $%lx\n",
//msg->cb_HeadSize,msg->cb_TailSize,msg->cb_HeadData,msg->cb_TailData);
//}
		// If there is going to be a "center section", call CachePreDMA
		// This saves the overhead of calling it in the middle of
		// a transfer.
		if (msg->cb_HeadSize + msg->cb_TailSize !=
		    scsicmd->scsi_Length)
		{
			msg->cb_CurrentLength -= msg->cb_HeadSize +
						 msg->cb_TailSize;
			data += msg->cb_HeadSize;
#else
		{
#endif // NEW_040_LIB

//if (((LONG)msg->cb_CurrentLength) < 0)
//{struct SCSICmd *scsicmd = msg->cb_SCSICmd;
//kprintf("CurrentLength < 0!!! ($%lx)\n",msg->cb_CurrentLength);
//kprintf("Msg $%lx (%ld), DSA $%lx, command $%lx, io_Command %ld\n",
//msg,msg->cb_Index,&(msg->cb_DSA),
//*(scsicmd->scsi_Command),msg->cb_IORequest->io_Command);
//kprintf("scsi_Length = $%lx, io_Length = $%lx\n",scsicmd->scsi_Length,
//msg->cb_IORequest->io_Length);
//}

#ifdef ZORRO_3
			// we need to handle non-DMAable memory - Z-II memory
			if (msg->cb_Programmed)
			{
			    // In Zorro-II space, not chip ram.
			    // this is not DMA-able from Zorro-III
			    // Use 1 buffer.  Code above makes certain only
			    // one command can access the buffer
			    data = g->st_Z2Buffer;
			    if (Z2_BUFFERSIZE < msg->cb_CurrentLength)
				msg->cb_CurrentLength = Z2_BUFFERSIZE;
kprintf("Z2: changed ptr to $%lx, len to $%lx\n",data,msg->cb_CurrentLength);

			    // if writing, copy the first batch into the buffer
			    if (!(msg->cb_SCSICmd->scsi_Flags &
				  (1 << SCSIB_READ_WRITE)))
			    {
				CopyMem(physaddr,data,msg->cb_CurrentLength);
			    }
			}
			// do CachePreDMA...
#endif
//kprintf("CachePreDMA: $%lx, $%lx\n",data,msg->cb_CurrentLength);
			// cb_CurrentLength may have been modified on
			// funny dma starting aligned, ending unaligned
			physaddr = CachePreDMA(data,
					       &(msg->cb_CurrentLength),flags);

			flags |= DMA_Continue;
		}

		// Remember resulting length/addr from CachePreDMA for use in
		// scatter_dma (AND in the interrupt handler)
		msg->cb_CacheLength = msg->cb_CurrentLength;
		msg->cb_CacheData   = physaddr;
		msg->cb_DidCacheStuff  = flags;

#ifndef NEW_040_LIB
		if (msg->cb_HeadSize)
		{
			// we have a 1-15 byte part to DMA first to align us.

			physaddr = msg->cb_HeadData;
			msg->cb_CurrentLength = msg->cb_HeadSize;
		}
#endif
	}

//if (msg->cb_CurrentLength != scsicmd->scsi_Length)
//kprintf("CachePreDMA modified the length from %ld to %ld!\n",
//scsicmd->scsi_Length,msg->cb_CurrentLength);
//if (physaddr != scsicmd->scsi_Data)
//kprintf("CachePreDMA modified the destination pointer $%08lx to $%08lx!\n",
//scsicmd->scsi_Data,physaddr);

	// set move table entry for the transfer
	// Also set up save area, which is only modified by save_data_ptrs,
	// and replaces move_data on reselect.  Final_ptr = addr+len, and is
	// used to avoid adding addresses in NCR code on successful data xfer.
	msg->cb_DSA.move_data.addr = (ULONG) physaddr;
	msg->cb_DSA.move_data.len  = msg->cb_CurrentLength;
	msg->cb_DSA.save_data.addr = (ULONG) physaddr;
	msg->cb_DSA.save_data.len  = msg->cb_CurrentLength;
	msg->cb_DSA.final_ptr      = ((UBYTE *) physaddr) +
				     msg->cb_CurrentLength;

	// set up the command output block
	msg->cb_DSA.command_data.addr = (ULONG) scsicmd->scsi_Command;
	msg->cb_DSA.command_data.len  = scsicmd->scsi_CmdLength;

	// set up identify message in send_buf, will be sent on selection.
	// Also set up simple_queue_tag message if needed.  We're using
	// fixed tag values, on the assumption that no more than 255 command
	// blocks will be allocated.  If we allow more, we'll need a get_tag()
	// function.
	// ha_Reselect is 0x00 or 0x40.  hu_Flags is rdb flag.
#ifndef DISABLE_DISCONNECT
	msg->cb_DSA.send_buf[0] = unit->hu_LUN | 0x80 |
	  (unit->hu_Flags & RDBFF_NORESELECT ? 0 : unit->hu_Addr->ha_Reselect);
#else
	msg->cb_DSA.send_buf[0] = unit->hu_LUN | 0x80; // no disconnect ever
#endif
	// SCSI spec says must return BUSY if noreselect and a tag - so don't
	// add a tag unless reselect is allowed
#ifndef DISABLE_TAGS
	if (unit->hu_TagsAllowed && (msg->cb_DSA.send_buf[0] & 0x40)) {
#else
	if (0) {
#endif
		msg->cb_DSA.send_buf[1] = SIMPLE_QUEUE_TAG;
		msg->cb_DSA.send_buf[2] = tag;
		msg->cb_DSA.send_msg.len = 3;
		// since we're going to have a tag, we must accept it
		*((ULONG *) &(msg->cb_DSA.reselect_entry.clear_ack)) =
			clear_ack_inst;
	} else {
		msg->cb_DSA.send_msg.len = 1;
		// since we're not going to have a tag, leave ack alone
		*((ULONG *) &(msg->cb_DSA.reselect_entry.clear_ack)) =
			my_nop_inst;
	}

	// see if we want to add a sync message to these
#ifndef DISABLE_SYNC
	if (unit->hu_Addr->ha_DoneSync == 0 &&
	    unit->hu_Addr->ha_SupportsSync &&
	    g->st_SendSync)
	{
		unit->hu_Addr->ha_DoneSync = 1;
		addsync(msg,g->st_UseFast ? 100/4 : 200/4,8);
	}
#endif
	// Flush caches to make sure the data is in RAM.
	// We must do this to make sure it's there before writing the 
	// scheduler entry, since the '710 may come along asynch to us and
	// try to schedule.
	CacheClearE(msg,sizeof(*msg),CACRF_ClearD);

	// now add our reselect entry to the queue for the address.  Must
	// be done after we've flushed the modifications to the entry to
	// RAM, and before the modifications to the scheduler to start this IO.
	// It's safe to add it, since it's not going to have a nexus match.
	unit->hu_Find_lun.check_tag.addr = (LONG)&(msg->cb_DSA.reselect_entry);
	CacheClearE(&(unit->hu_Find_lun.check_tag.addr),4,CACRF_ClearD);

	// Now make our entry in the scheduler array active so this will
	// get scheduled.
	// Set the jump on true/false bit to 0 (never jump). 
	g->st_Scheduler.array[i].next_sched.control = 0x00; // make into NOP

	// Flush caches to make sure the data is in RAM.
	CacheClearE(&(g->st_Scheduler.array[i].next_sched.control),1,
		    CACRF_ClearD);

//kprintf("scheduled...");
	// Now tweak the scsi chip to make it schedule if it's not busy.
	// don't bother if we were called from UnSchedule...  all running
	// scripts eventually wander back to the scheduler.
	if (running)
	{
//		Disable();
		// not needed, but nice to do here.  We don't want to be
		// interrupted while checking this bit.  It won't kill us,
		// but it's better to avoid it.
		// The worst that will happen is an extra loop through the
		// scheduler, so we'll ignore the disable/enable

		// ISTAT is the only register we can access safely while a
		// script is running.
		// Is it possible to have a race condition here???  No.
		if (!(g->st_BoardAddress->istat & ISTATF_CON))
		{
			// not connected, wake it up
			// this will interrupt Wait For Reselect/Select
			// if it selects/reselects before the bit is set, it
			// will clear the bit and ignore it.
			g->st_BoardAddress->istat |= ISTATF_SIGP;
//g->st_BoardAddress->istat = ISTATF_SIGP;
//kprintf("SIGP\n");
		}
//else kprintf("connected\n");
		// if we are connected, it will handle it on disconnect
//		Enable();
	}
//else kprintf("already running\n");

//kprintf("done\n");
}

// remove an IOR from use (completed or failed)
static void
UnSchedule (struct SCSIGlobals *g, struct CommandBlock *msg)
{
	struct Find_nexus_entry *nexus,*my_nexus;
	struct HDUnit *unit = (struct HDUnit *) msg->cb_IORequest->io_Unit;
	void *addr;

//if (!msg) {kprintf("Unschedule NULL!!\n");return;}
//kprintf("UnSchedule $%lx(%ld), unit $%lx, stat $%lx, %ld of %ld\n",
//msg,msg->cb_Index,unit,
//msg->cb_DSA.status_buf[0],
//msg->cb_SCSICmd->scsi_Actual,msg->cb_SCSICmd->scsi_Length);

	// DMA_Continue is set if we called CachePreDMA
	if (msg->cb_SCSICmd->scsi_Length != 0 &&
	    (msg->cb_DidCacheStuff & DMA_Continue))
	{
//kprintf("CachePostDMA: $%lx, $%lx\n",msg->cb_SCSICmd->scsi_Data,
//msg->cb_SCSICmd->scsi_Length);
		// must make sure CachePostDma matches predma for split xfer
		msg->cb_CurrentLength = msg->cb_SCSICmd->scsi_Length -
#ifndef NEW_040_LIB
					(msg->cb_HeadSize + msg->cb_TailSize);
#else
					0;
#endif
		CachePostDMA(((UBYTE *) msg->cb_SCSICmd->scsi_Data) +
#ifndef NEW_040_LIB
			     msg->cb_HeadSize,
#else
			     0,
#endif
			     &(msg->cb_CurrentLength),
			     msg->cb_DidCacheStuff & ~DMA_Continue);
	}
	// remove from reselect list (singly-linked)
	// common case is one entry, check that first

	my_nexus = &(msg->cb_DSA.reselect_entry);
	if (unit->hu_Find_lun.check_tag.addr == (ULONG) my_nexus)
	{
	   // This is by far the most common case...
	   addr = &(unit->hu_Find_lun.check_tag.addr);
	   unit->hu_Find_lun.check_tag.addr = my_nexus->check_tag.addr;

	} else {
		for (nexus = (void *) unit->hu_Find_lun.check_tag.addr;
		     nexus->check_tag.addr != (ULONG) LABEL(no_nexus_found);
		     nexus = (void *) nexus->check_tag.addr)
		{
			if (nexus->check_tag.addr == (ULONG) my_nexus)
			{
			    addr = &(nexus->check_tag.addr);
			    nexus->check_tag.addr = my_nexus->check_tag.addr;
			    break;
			}
		}
	}
	// Flush the changes to the reselect chains
	CacheClearE(addr,4,CACRF_ClearD);

	// Launch queued IO's if any, reply message
	CleanUp(g,msg,unit);
}

// separated so we can call it for IOR's that never got scheduled fully
// (mainly Z2 IORs if we fail allocation)
static void
CleanUp (struct SCSIGlobals *g, struct CommandBlock *msg, struct HDUnit *unit)
{
	// so we know it's not linked in any more (for sync & reset mainly)
	msg->cb_StateSave = STATE_UNUSED;

	// Check if there are unscheduled IO's for this unit that we should
	// now schedule.
	if (!IsListEmpty(((struct List *) &(unit->hu_CmdList))))
	{
		struct CommandBlock *newmsg;

//kprintf("unqueuing $%lx from unit $%lx\n",unit->hu_CmdList.mlh_Head,unit);
		newmsg = (struct CommandBlock *)
			 RemHead((struct List *) &(unit->hu_CmdList));
		Schedule(g,unit,newmsg,FALSE);
//kprintf("scheduled\n");
	} else
		unit->hu_QueueType = UNIT_QUIET;	// nothing waiting

#ifdef ZORRO_3
	// if this was a Z-II dma remove it.  If others are waiting, start the
	// next.
	if (msg->cb_Programmed)
	{
//kprintf("Z2: removing msg $%lx\n",msg);
		// remove message from Z2List...
		Remove(msg);

		if (!IsListEmpty((struct List *) &(g->st_Z2List)))
		{
			struct CommandBlock *msg2 = (void *)
						    g->st_Z2List.mlh_Head;

//kprintf("Z2: scheduling next msg $%lx\n",msg2);
			Schedule(g,(struct HDUnit *)
				   msg2->cb_IORequest->io_Unit,msg2,FALSE);
		}
	}
#endif

	// HFERR_BadStatus comes from iotask.asm...
	ReplyMsg(msg);
}

// template for a DSA entry
struct DSA_entry standard_DSA_entry = {
	// fake move for table-indirect data move inst
	{0x00000000, 0x00000000,},
	// fake move for table-indirect data move inst - save area
	{0x00000000, 0x00000000,},
	// final_ptr - so NCR doesn't have to add 2 longs...
	0x00000000,
	// select data for indirect select_data	(needs id/sync filled in)
	{0x00000000, 0x00, 0x80, 0x00},
	// fake move for table-indirect status_data
	{0x00000001, 0x00000000,},
	// fake move for table-indirect recv_msg
	{0x00000001, 0x00000000,},
	// fake move for table-indirect send_msg
	{0x00000001, 0x00000000,},
	// fake move for table-indirect command_data
	{0x00000000, 0x00000000,},
};

// MUST be on a LW boundary!!!! (fix!)
ULONG jump_rel_inst = 0x88888888;		// make a NOP a jump

// template for a scheduler entry
struct Sched_entry standard_sched_entry = {
	// jump REL(next) (or NOP next)
	{0x80,0x88,0x00,0x00, sizeof(struct Sched_entry) -
			      offsetof(struct Sched_entry,make_jump)},
	// memmove 1, jump_rel_inst+1, next_sched+1  (needs address filled in)
	{0xc0, {0x00,0x00,0x01}, ((LONG)&jump_rel_inst)+1, 0x00000000},
	// memmove 4, &(DSA_values[i]), current_dsa (needs address filled in)
	{0xc0, {0x00,0x00,0x04}, 0x00000000, 0x00000000},
	// memmove 4, &(DSA_values[i]), DSA	(needs address filled in)
	{0xc0, {0x00,0x00,0x04}, 0x00000000, 0x00000000},
	// jump select 				(needs address filled in)
	{0x80,0x08,0x00,0x00, 0x00000000},
};

struct jump_inst standard_scheduler_jump = {
	// jump reselect
	0x80,0x08,0x00,0x00, 0x00000000,
};

// template for a reselect entry
struct Find_lun_entry standard_lun_entry = {
	// jump no_nexus_found IF NOT 0x00
	{0x80,0x04,0x00,0x00, 0x00000000},
	// call get_tag
	{0x88,0x08,0x00,0x00, 0x00000000},
	// jump no_nexus_found
	{0x80,0x08,0x00,0x00, 0x00000000},	// patched
};

// template for a reselect entry
struct Find_nexus_entry standard_nexus_entry = {
	// jump no_nexus_found IF NOT 0x00
	{0x80,0x04,0x00,0x00, 0x00000000},
	// clear ack (accept tag if any) - this is a NOP if this entry is for
	// a non-tagged request.
	{0x60,0x00,0x00,0x40, 0x00000000},
	// memmove 4, &(DSA_values[i]), current_dsa (needs address filled in)
	{0xc0, {0x00,0x00,0x04}, 0x00000000, 0x00000000},
	// memmove 4, &(DSA_values[i]), DSA	(needs address filled in)
	{0xc0, {0x00,0x00,0x04}, 0x00000000, 0x00000000},
	// move CTEST0 | CTEST0F_ERF to CTEST0	(or & ~(CTEST0F_ERF) for S2Fast
//FIX!	{0x7a, offsetof(struct ncr710,ctest0),CTEST0F_ERF,0x00},
// alternate for scsi-fast...
//	{0x7c, offsetof(struct ncr710,ctest0),~CTEST0F_ERF,0x00},
	// jump nexus_found
	{0x80,0x08,0x00,0x00, 0x00000000},
};

#define EXTERNAL(sym,val)				 \
	for (i = 0; i < SIZEOF(E_ ## sym ## _Used); i++) \
{\
		script[E_ ## sym ## _Used[i]] = (val) \
;/*kprintf(# sym ": $%lx = $%lx\n",&(script[E_ ## sym ## _Used[i]]),val);*/\
}

static void
init_structs (struct SCSIGlobals *g, struct IOTask_Globals *IOGlobals)
{
	int i;

	ULONG *script;

	// Copy scripts to fastram
	script = AllocMem(sizeof(SCRIPT),0);
	// FIX! errors

	g->st_Script = script;
	memcpy(script,SCRIPT,sizeof(SCRIPT));
//kprintf("Script is at $%lx\n",script);

	// do relocation for externals
	EXTERNAL(_SCRATCH,((ULONG) g->st_BoardAddress) +
			  offsetof(struct ncr710,scratch));
// fix! use _SCRATCH
	EXTERNAL(_SCRATCH3,((ULONG) g->st_BoardAddress) +
			  offsetof(struct ncr710,scratch) + 0);
	EXTERNAL(_DSA,((ULONG) g->st_BoardAddress) +
			  offsetof(struct ncr710,dsa));
	EXTERNAL(current_dsa,(ULONG) &(g->st_CurrentDSA));
	EXTERNAL(default_dsa,(ULONG) &(g->st_DefaultDSA));
	EXTERNAL(default_send_buf,(ULONG) &(g->st_DefaultStorage.send_buf[0]));
	EXTERNAL(modify_data,(ULONG) &(g->st_ModifyData));
	EXTERNAL(Scheduler,(ULONG) &(g->st_Scheduler));
	EXTERNAL(sync_buf,(ULONG) &(g->st_SyncBuf));
	EXTERNAL(zero,(ULONG) &(g->st_Zero));

	// fix ownid mask in reselection code
	((struct rw_reg_inst *) LABEL(patch_ownid_mask))->imm = ~(1<<g->st_OwnID);

	// patch absolute references to the script from inside it
	for (i = 0; i < PATCHES; i++)
		script[LABELPATCHES[i]] += (ULONG) script;

	// set up our command blocks and scheduler
	for (i = 0; i < CMD_BLKS; i++)
	{
		// set up the index
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_Index = i;

		// array of DSA values - always tied to cmdblock
		g->st_DSA_values[i] =
				 &(g->st_CmdBlocks[i].iocb_CmdBlock.cb_DSA);

		// build the DSA block
		init_dsa(g,&(g->st_CmdBlocks[i].iocb_CmdBlock.cb_DSA),i);

#ifndef NEW_040_LIB
		// init Head/TailData to point to 16-byte aligned buffers
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_HeadData = (UBYTE *)
		 (((ULONG)
		 &(g->st_CmdBlocks[i].iocb_CmdBlock.cb_DSA.head_tail_data[15]))
		 & 0xfffffff0);
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_TailData = 
			g->st_CmdBlocks[i].iocb_CmdBlock.cb_HeadData + 16;
#endif
		// build the scheduler entry
		g->st_Scheduler.array[i] = standard_sched_entry;
		g->st_Scheduler.array[i].make_jump.dest =
			(ULONG) &(g->st_Scheduler.array[i].next_sched.control);
		g->st_Scheduler.array[i].save_dsa.source =
			(ULONG) &(g->st_DSA_values[i]);
		g->st_Scheduler.array[i].get_dsa.source =
			(ULONG) &(g->st_DSA_values[i]);
		g->st_Scheduler.array[i].save_dsa.dest =
			(ULONG) &(g->st_CurrentDSA);
		g->st_Scheduler.array[i].get_dsa.dest =
			((ULONG) g->st_BoardAddress) +
			 offsetof(struct ncr710,dsa);
		g->st_Scheduler.array[i].start.addr = (LONG) g->st_Script;

		// initialize the other iotask fields...
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_SenseData =
					&(g->st_CmdBlocks[i].iocb_Sense[0]);
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_SCSICmd =
					&(g->st_CmdBlocks[i].iocb_Cmd1);
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_SCSICmd->scsi_Command =
					&(g->st_CmdBlocks[i].iocb_Command1[0]);
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_SpareSCSICmd =
					&(g->st_CmdBlocks[i].iocb_Cmd2);
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_SpareSCSICmd->scsi_Command =
					&(g->st_CmdBlocks[i].iocb_Command2[0]);
#ifdef FIXING_READS
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_KlugeData =
					&(g->st_CmdBlocks[i].iocb_Kludge[0]);
#endif
		g->st_CmdBlocks[i].iocb_CmdBlock.cb_Msg.mn_Length =
					sizeof(struct CommandBlock);

		// add to iotask free command block list
		AddTail((struct List *) &(IOGlobals->iot_FreeCmds),
			&(g->st_CmdBlocks[i]));
	}

	// finish setting up scheduler...
	g->st_Scheduler.jump_sleep      = standard_scheduler_jump;
	g->st_Scheduler.jump_sleep.addr = LABEL(resel);

	// init default dsa (really only needs msg fields...)
	g->st_DefaultDSA = &(g->st_DefaultStorage);
	g->st_CurrentDSA = &(g->st_DefaultStorage);
	init_dsa(g,g->st_DefaultDSA,0);

	// Might as well blow the entire D-cache here...
	CacheClearE((void *) 0,0xffffffff,CACRF_ClearD);
}

static void
init_dsa (struct SCSIGlobals *g, struct DSA_entry *dsa, LONG i)
{
	// build the DSA block
	*dsa = standard_DSA_entry;
	dsa->reselect_entry = standard_nexus_entry;

	// fix absolute addresses in table-indirect tables...
	dsa->status_data.addr = (ULONG) &(dsa->status_buf[0]);
	dsa->recv_msg.addr    = (ULONG) &(dsa->recv_buf[0]);
	dsa->send_msg.addr    = (ULONG) &(dsa->send_buf[0]);

	// fix reselect entry
	dsa->reselect_entry.check_tag.addr  = LABEL(no_nexus_found);
	dsa->reselect_entry.save_dsa.source = (ULONG) &(g->st_DSA_values[i]);
	dsa->reselect_entry.get_dsa.source  = (ULONG) &(g->st_DSA_values[i]);
	dsa->reselect_entry.save_dsa.dest   = (ULONG) &(g->st_CurrentDSA);
	dsa->reselect_entry.get_dsa.dest    = ((ULONG) g->st_BoardAddress) +
						offsetof(struct ncr710,dsa);
	dsa->reselect_entry.set_up.addr     = LABEL(nexus_found);
}

static void
init_chip(struct SCSIGlobals *g)
{
	volatile struct ncr710 *b = g->st_BoardAddress;
	UBYTE istat;
	ULONG status;	// not used, really

	// reset ncr chip
//kprintf("initing chip @ $%lx\n",b);
	b->istat |= ISTATF_ABRT;
	poll_cia(50000);
	b->istat |= ISTATF_RST;
	b->istat  = 0;
	poll_cia(100000);

	b->ctest7 |= 0x80;	// disable burst bus mode

	// disable timer FIRST.  This avoids select timeouts.
	// no byte-to-byte timer, enable active negation, filter REQ/ACK
	// (filter must be turned off if we do fast sync)
	if (g->st_UseFast)
		b->ctest0 = CTEST0F_BTD | CTEST0F_EAN;
	else
		b->ctest0 = CTEST0F_BTD | CTEST0F_EAN | CTEST0F_ERF;

	// clear ints from reset/abort/timeout
	while ((istat = b->istat & (ISTATF_SIP|ISTATF_DIP)) != 0)
{poll_cia(50000);
		status = *((volatile ULONG *) &(b->sstat2));
}
	// Enable Parity Checking, Enable Parity Generation, Assert ATN on
	// Parity Error (in initiator mode).
	// power up is SCNTL0F_ARB1 | SCNTL0F_ARB1
//	b->scntl0 |= SCNTL0F_EPC | SCNTL0F_EPG | SCNTL0F_AAP;
b->scntl0 |= SCNTL0F_EPG;


	// must wait 25us before de-asserting RST (SCSI spec) - poll CIA
	b->scntl1 = SCNTL1F_RST;
	poll_cia(25);
	b->scntl1 &= ~SCNTL1F_RST;
	// now must wait 250ms before trying to use the bus...
// FIX! send timer request!
poll_cia(250000);

	// Set SCSI ID mask
	b->scid = 1 << g->st_OwnID;

	// Enable Selection/Reselection (after setting ID)
	b->scntl1 |= SCNTL1F_ESR;

	// disable halt on parity error ???  Asynch
	b->sxfer = SXFERF_DHP;

#if (NCR_CLOCK_FREQ > 50)	// actually 50.01
#define SBCL_DEFAULT	3000	// clock/3
#define DCNTL_VAL	(DCNTLF_CF1 | DCNTLF_CF0)
#else
#if (NCR_CLOCK_FREQ > 37)	// actually 37.5
#define SBCL_DEFAULT	2000	// clock/2
#define DCNTL_VAL	(0)
#else
#if (NCR_CLOCK_FREQ > 25)	// actually 25.01
#define SBCL_DEFAULT	1500	// clock/1.5
#define DCNTL_VAL	(DCNTLF_CF0)
#else
#define SBCL_DEFAULT	1000	// clock/1
#define DCNTL_VAL	(DCNTLF_CF1)
#endif
#endif
#endif
	// sync clock is clock/n
	// SCLK/3   gives us 1.52 MB/s to 4.17 MB/s range
	// SCLK/2   gives us 2.27 MB/s to 6.25 MB/s range (and 5.0 MB/s)
	// SCLK/1.5 gives us 3.0 MB/s to 8.33 MB/s range
	// SCLK/1   gives us 4.54 MB/s to 12.5 MB/s range (and 10.0 MB/s)
	if (!g->st_UseFast)
	{
		// default to SCLK/2 for sync
		b->sbcl    = SBCLF_SSCF1 | SBCLF_SSCF0;
		g->st_SBCL = 2000;
	} else {
// FIX! change dynamically
		// default to SCLK/1 for fast sync
		b->sbcl = SBCLF_SSCF0;
		g->st_SBCL = 1000;
	}

	// burst length = 8, function code = 10x
	b->dmode = DMODEF_BL1 | DMODEF_BL0 | DMODEF_FC2;
//b->dmode = DMODEF_FC2;

	// 50MHz clock, don't auto-switch - selection id in LCRC only
	b->dcntl = DCNTL_VAL | DCNTLF_COM |
#ifdef IS_A4000T
		   DCNTLF_EA;
#else
		   0;
#endif

#ifdef SINGLE_STEP
b->dcntl |= DCNTLF_SSM;
#endif

kprintf("enabling ints\n");
	// Enable all SCSI interrupts except select and function complete
#ifndef DISABLE_INTS
	b->sien = (UBYTE) ~(SIENF_FCMP | SIENF_SEL);
#endif

	// Set DMA interrupt mask
	// Bus Fault, Abort, SCRIPT Interrupt Received, Illegal Instruction
	// single-step
#ifndef DISABLE_INTS
	b->dien = DIENF_BF | DIENF_ABRT | DIENF_SIR | DIENF_IID | DIENF_SSI;
#endif

	// make SURE no ints are coming...  This will eat any from enable...
	// (there may be some reset/abort ints hanging around)
	poll_cia(1000);
	g->st_TaskPointer = g->st_IntPointer;
}

void
add_unit (struct SCSIGlobals *g, struct HDUnit *unit)
{
//kprintf("Adding unit $%lx (ID %ld, LUN %ld)\n",unit,unit->hu_Unit,unit->hu_LUN);
	unit->hu_Find_lun = standard_lun_entry;
	unit->hu_Find_lun.get_tag.addr = LABEL(get_tag);
	unit->hu_Find_lun.check_tag.addr = LABEL(no_nexus_found);
	unit->hu_Find_lun.check_lun.data = unit->hu_LUN;
	unit->hu_Find_lun.check_lun.addr = ((struct Find_addr_entry *)
		 LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr;
	CacheClearE(&(unit->hu_Find_lun.check_lun.addr),4,CACRF_ClearD);
	((struct Find_addr_entry *)
	 LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr =
						 (ULONG) &(unit->hu_Find_lun);
	CacheClearE(&(((struct Find_addr_entry *)
		      LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr),4,
		    CACRF_ClearD);
//kprintf("unit added\n");
}

void
remove_unit (struct SCSIGlobals *g, struct HDUnit *unit)
{
	struct Find_lun_entry *lun,*my_lun;
	void *addr;

//kprintf("Removing unit $%lx (%ld)\n",unit,unit->hu_Unit);
	// do not allow remove_unit to be called if there are outstanding
	// openers of the unit.

	// remove from reselect list (singly-linked)
	// common case is one entry, check that first

	my_lun = &(unit->hu_Find_lun);

	if (((struct Find_addr_entry *)
	      LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr ==
							    (ULONG) my_lun)
	{
	   // This is by far the most common case...
	   addr = &(((struct Find_addr_entry *)
		     LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr);
	   ((struct Find_addr_entry *)
		     LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr = 
					my_lun->check_lun.addr;

	} else {
		for (lun = (void *)((struct Find_addr_entry *)
			 LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr;
		     lun->check_lun.addr != (ULONG) LABEL(no_nexus_found);
		     lun = (struct Find_lun_entry *) lun->check_lun.addr)
		{
			if (lun->check_lun.addr == (ULONG) my_lun)
			{
				addr = &(lun->check_lun.addr);
				lun->check_lun.addr =
				     (ULONG) my_lun->check_lun.addr;
				break;
			}
		}
	}
	// Flush the changes to the reselect chains
	CacheClearE(addr,4,CACRF_ClearD);

//kprintf("removed unit\n");
}

// possible states here: nothing (for renegotiate), identify, and identify
// with 2-byte tag message following.  Caller decides when to flush cache.
static void
addsync (struct CommandBlock *msg, LONG period, LONG offset)
{
	int len = msg->cb_DSA.send_msg.len;	// 0 or 1 or 3

	msg->cb_DSA.send_buf[len++] = EXTENDED_MSG;
	msg->cb_DSA.send_buf[len++] = 3;
	msg->cb_DSA.send_buf[len++] = SYNC_REQUEST;
	msg->cb_DSA.send_buf[len++] = period;
	msg->cb_DSA.send_buf[len++] = offset;

//kprintf("adding sync message, period %ldns, offset %ld, msg len = %ld\n",
//period*4,offset,len);
	msg->cb_DSA.send_msg.len = len;	// 5 or 6 or 8
}

void
abort_all (struct SCSIGlobals *g,LONG code)
{
	struct CommandBlock *msg;
	struct HDUnit *unit;
	struct HDAddr *addr;
	int i;

kprintf("aborting all IO's, code = $%lx\n",code);
	// abort all active requests, finished or not.
	for (i = 0; i <= MAX_ADDR; i++)
	{
		addr = &(g->st_DeviceBase->hd_HDUnits[i]);
		for (unit = (void *) addr->ha_LUNList.mlh_Head;
		     unit->hu_Node.mln_Succ;
		     unit = (void *) unit->hu_Node.mln_Succ)
		{
		    struct CommandBlock *next;

		    next = (void *) ((struct CommandBlock *)
			   unit->hu_CmdList.mlh_Head)->cb_Msg.mn_Node.ln_Succ;

		    for (msg = (void *) unit->hu_CmdList.mlh_Head,
			 next = (void *) msg->cb_Msg.mn_Node.ln_Succ;
		         msg->cb_Msg.mn_Node.ln_Succ;
		         msg = next)
		    {
			// must get next pointer BEFORE Remove()ing msg!
			next = (void *) msg->cb_Msg.mn_Node.ln_Succ;

kprintf("replying queued IO $%lx\n",msg);
			Remove(msg);
			msg->cb_SCSICmd->scsi_Actual = 0;
			msg->cb_SCSICmd->scsi_CmdActual = 0;
			msg->cb_SCSICmd->scsi_SenseActual = 0;
			msg->cb_ErrorCode = code;
			ReplyMsg(msg);
		    }
		}
	}
	// make sure the queues are empty before doing this
	// now any active commands
	for (i = 0; i < CMD_BLKS; i++)
	{
		msg = &(g->st_CmdBlocks[i].iocb_CmdBlock);

		// if active, remove and return with error
		if (msg->cb_StateSave == STATE_ACTIVE)
		{
unit = (struct HDUnit *) msg->cb_IORequest->io_Unit;
kprintf("aborting active IO $%lx\n",msg);
kprintf("\t$%lx (%ld), DSA $%lx, unit $%lx(%ld:%ld), command $%lx\n",
msg,msg->cb_Index,&(msg->cb_DSA),unit,unit->hu_Unit,unit->hu_LUN,
*(msg->cb_SCSICmd->scsi_Command));
kprintf("\tstat $%lx, %ld of %ld\n",
msg->cb_DSA.status_buf[0],
msg->cb_SCSICmd->scsi_Actual,msg->cb_SCSICmd->scsi_Length);
			msg->cb_ErrorCode = code;
			UnSchedule(g,msg);
		}

		// reset scheduler jump instruction in case it hadn't been
		// scheduled.
		g->st_Scheduler.array[i].next_sched =
					 standard_sched_entry.next_sched;
	}

	// no command active - entire cache is blown below, so it's safe
	g->st_CurrentDSA = g->st_DefaultDSA;

	// Might as well blow the entire D-cache here...
	CacheClearE((void *) 0,0xffffffff,CACRF_ClearD);

	// ok, all commands returned.  Re-init chip and restart it.
	// disable ints from paranoia
	g->st_BoardAddress->sien = 0;
	g->st_BoardAddress->dien = 0;
	init_chip(g);

// FIX! duplicate code with main!
	// must load default DSA in case IO gets scheduled before NCR chip
	// gets to wait reselect.  theoretically possible.
	WRITE_LONG(g->st_BoardAddress,dsa,(ULONG) g->st_DefaultDSA);
	WRITE_LONG(g->st_BoardAddress,dsp,(ULONG) &(g->st_Scheduler));

#ifdef SINGLE_STEP
	g->st_BoardAddress->dcntl |= DCNTLF_STD;
#endif

}

// FIX!!!!!!!  some of this stuff must be handled by the task.  Use a queue,
// since interrupts can queue up in the chip.  Handle all we can here.

#define DO_RESTART 0
#define DO_JUMP	   1

#define RESTART	restart=DO_RESTART
#define GOTO(x)	new_dsp = (x),restart=DO_JUMP

static void
HandleInts (struct SCSIGlobals *g)
{
	UBYTE sstat0,sstat1,sstat2,dstat,istat;
	ULONG status;
	volatile struct ncr710 *b = g->st_BoardAddress;
	struct CommandBlock *msg;
	struct HDUnit *unit;
	UBYTE restart=DO_RESTART;	// by default, restart at next inst
	ULONG new_dsp;			// new PC for ncr, if any

    // Must loop here - we must process all pending ints before hitting
    // restart or DSP register (since if another is pending we'll hit the
    // chip while it's running...)
    while (g->st_TaskPointer != g->st_IntPointer)
    {
	istat  = g->st_ISTATData[g->st_TaskPointer];
	status = g->st_IntData[g->st_TaskPointer];

	g->st_TaskPointer++;
	if (g->st_TaskPointer == SIZEOF(g->st_IntData))
		g->st_TaskPointer = 0;

//kprintf("handling int: istat $%02lx, status $%08lx (%ld:%ld)\n",istat,status,
//g->st_IntPointer,g->st_TaskPointer);

	dstat =  status & 0xff;

	// Must blow any cached value for current_dsa
	CacheClearE(&(g->st_CurrentDSA),4,CACRF_ClearD);
	if (g->st_CurrentDSA != g->st_DefaultDSA)
		msg = (struct CommandBlock *) (((UBYTE *)g->st_CurrentDSA) -
					offsetof(struct CommandBlock,cb_DSA));
	else {
		msg = NULL;	// force enforcer hits if we use it...
	}

// FIX? should we move it lower to only those that need it?

	if (msg)
	{
		CacheClearE(msg,sizeof(*msg),CACRF_ClearD);
		unit = (struct HDUnit *) msg->cb_IORequest->io_Unit;
	} else {
		CacheClearE(g->st_DefaultDSA,sizeof(*(g->st_DefaultDSA)),
			    CACRF_ClearD);
		unit = NULL;
	}

	// OK, an int occurred.  Let's see what happened.
	if (istat & ISTATF_DIP)	// DMA interrupt
	{
#ifdef SINGLE_STEP
if (dstat & DSTATF_SSI) {
// single-step
kprintf("ss: $%lx\n",b->dsp); //,*((ULONG *) b->dsp),
//*((ULONG *) (b->dsp+4)));
// we already default to restart
}
#endif
//if ((dstat & 0x7f) && ((dstat & 0x7f) != 0x08))
//kprintf("dstat = %lx\n",dstat);
		if (dstat & DSTATF_SIR)	// script interrupt
		{
		    switch (b->dsps) {

		    case io_complete:
			// make chip goto scheduler - do this early
			GOTO((ULONG) &(g->st_Scheduler));
//kprintf("io complete $%lx\n",msg);

			// assume the entire command was used, who cares...
			msg->cb_SCSICmd->scsi_CmdActual =
					msg->cb_SCSICmd->scsi_CmdLength;
#ifdef ZORRO_3
			// Handle copying Z2 buffers
			if (msg->cb_Programmed)
			{
			    if ((msg->cb_SCSICmd->scsi_Flags &
				 (1 << SCSIB_READ_WRITE)))
			    {
				CopyMem(g->st_Z2Buffer,
					((UBYTE *)
					 msg->cb_SCSICmd->scsi_Data) +
					msg->cb_SCSICmd->scsi_Actual,
					msg->cb_CurrentLength);
			    }
			}
#endif
			// all earlier scatters are included already.
			// move_data.len will be 0 if all the data xferred.
			// note the '040 alignment stuff works even if we
			// only have a head section.  However, on read we must
			// still transfer the head and tail via cpu to the
			// memory.  We'll do that here to make it easier.
			// the buffers have been flushed, since they're part
			// of the DSA structure.  We don't really care whether
			// they actually got filled in or not.
#ifndef NEW_040_LIB
			if ((msg->cb_SCSICmd->scsi_Flags &
			      (1 << SCSIB_READ_WRITE)))
			{
			    // only on reads!!!
			    if (msg->cb_HeadSize)
				memcpy(msg->cb_SCSICmd->scsi_Data,
				       msg->cb_HeadData,
				       msg->cb_HeadSize);

			    if (msg->cb_TailSize)
				memcpy((UBYTE *)
					((((ULONG) msg->cb_SCSICmd->scsi_Data)
					  + msg->cb_SCSICmd->scsi_Length) &
					0xfffffff0),
					msg->cb_TailData, 
					msg->cb_TailSize);
			}
#endif
			msg->cb_SCSICmd->scsi_Actual +=
					msg->cb_CurrentLength -
					msg->cb_DSA.move_data.len;
// FIX! overrun/underrun!
			msg->cb_SCSICmd->scsi_Status =
					 msg->cb_DSA.status_buf[0];
			if (msg->cb_SCSICmd->scsi_Status == CHECK_CONDITION)
			{
		// since we got a check_condition, the unit _may_ have been
		// reset.  Since it should be rare to get, and a power-up
		// or reset will lose sync info, we must reset our sync state
		// on ANY check_condition.  We'll set things to force a
		// renegotiation on the next message out phase (normally
		// identify, usually for REQUEST_SENSE).  Of course, if we
		// hadn't set up sync we'll ignore this.

			    if (unit->hu_Addr->ha_SyncValue)
				unit->hu_Addr->ha_DoneSync = 0;
			}

//kprintf("returning IO $%lx, status $%lx, $%lx of $%lx bytes\n",
//msg,msg->cb_SCSICmd->scsi_Status,msg->cb_SCSICmd->scsi_Actual,
//msg->cb_SCSICmd->scsi_Length);

			// We're done!!!!!
			UnSchedule(g,msg);
			break;

		    case scatter_dma:
handle_scatter:	// illegal instruction occasionally should really be
		// scatter dma.

			// either CachePreDMA broke it up, or the size was
			// wrong, or the device fucked up.
			// or it was broken up due to '040 alignment
			// or it was broken up due to Z-2 DMA restrictions
//kprintf("scsi_Data = $%lx, scsi_Actual = $%lx, currentlength = $%lx\n",
//msg->cb_SCSICmd->scsi_Data,msg->cb_SCSICmd->scsi_Actual,msg->cb_CurrentLength);

			// WARNING! this code is largely duplicated in the
			// interrupt handler!  Make matching changes!!!
			msg->cb_SCSICmd->scsi_Actual +=	msg->cb_CurrentLength;

			if (msg->cb_SCSICmd->scsi_Actual !=
			    msg->cb_SCSICmd->scsi_Length)
			{
			  void *physaddr = ((UBYTE *)
					    msg->cb_SCSICmd->scsi_Data) +
					   msg->cb_SCSICmd->scsi_Actual;

#ifdef ZORRO_3
			  // Handle copying Z2 buffers
			  // on read, copy it out.  On write, copy the next
			  // batch into the buffer.  A bit ugly.  Do PostDMA
			  // and then a new PreDMA.
			  if (msg->cb_Programmed)
			  {
//kprintf("Z2: $%lx of $%lx done\n",msg->cb_SCSICmd->scsi_Actual,
//msg->cb_SCSICmd->scsi_Length);
			    original_pos  = ((UBYTE *)
					     msg->cb_SCSICmd->scsi_Data) +
					    msg->cb_SCSICmd->scsi_Actual;
			    original_len  = msg->cb_CurrentLength;
			    original_left = msg->cb_SCSICmd->scsi_Length -
					    (msg->cb_SCSICmd->scsi_Actual +
					     msg->cb_CurrentLength);

			    // must move data for write to buffer before 
			    // CachePreDMA...  read must come after...
			    if (!(msg->cb_SCSICmd->scsi_Flags &
				  (1 << SCSIB_READ_WRITE)))
			    {
				ULONG size = Z2_BUFFERSIZE;

				// write
				if (original_left < size)
				    size = original_left;

				CopyMem(original_pos + original_len,
					g->st_Z2Buffer,
					size);
			    }
			    CachePostDMA(g->st_Z2Buffer,
					 &(msg->cb_CurrentLength),
					 msg->cb_DidCacheStuff & ~DMA_Continue);
			    // We're no longer in pre-dma.
			    msg->cb_DidCacheStuff &= ~DMA_Continue;

			    if ((msg->cb_SCSICmd->scsi_Flags &
				 (1 << SCSIB_READ_WRITE)))
			    {
				// read
				CopyMem(g->st_Z2Buffer,
					original_pos,
					original_len);
			    }

			    // the next xfer will also be to the buffer
			    physaddr = g->st_Z2Buffer;


			  }
#endif
			  // How much do we have left to do?
			  msg->cb_CurrentLength =
					msg->cb_SCSICmd->scsi_Length -
					msg->cb_SCSICmd->scsi_Actual;

#ifndef NEW_040_LIB
			  // handle Head/Tail '040 splitting...
			  if (msg->cb_CurrentLength == msg->cb_TailSize)
			  {
				// only tail section to finish
				// no need to call CachePreDMA...
				physaddr = msg->cb_TailData;

			  } else {
				// leave tail section for last...
				msg->cb_CurrentLength -= msg->cb_TailSize;

				// Schedule() tries to call CachePreDMA for us
				if (msg->cb_HeadSize !=
				    msg->cb_SCSICmd->scsi_Actual)
				{
#endif
				  // 68000's & CachePreDMA have a problem, not
				  // our problem (no Z-III 68000's).
				  // With head/tail, this may be the first
				  // CachePreDMA() call...
//kprintf("scatter CachePreDMA: $%lx, $%lx\n",physaddr,msg->cb_CurrentLength);
				  physaddr = CachePreDMA(physaddr,
						 &(msg->cb_CurrentLength),
						 msg->cb_DidCacheStuff);
#ifndef NEW_040_LIB
				} else {
				  // Ok, time to do the body of a alignment
				  // xfer.  Since CPreDMA() was called
				  // earlier, use the length/addr it returned
				  // to us.
				  msg->cb_CurrentLength = msg->cb_CacheLength;
				  physaddr		= msg->cb_CacheData;
				}
			  }
#endif
//kprintf("scatter DMA, new ptr $%lx, new len $%lx\n",physaddr,
//msg->cb_CurrentLength);

			  msg->cb_DSA.move_data.addr = (ULONG) physaddr;
			  msg->cb_DSA.move_data.len  = msg->cb_CurrentLength;
			  msg->cb_DSA.final_ptr      = ((UBYTE *) physaddr) +
						       msg->cb_CurrentLength;
			  CacheClearE(&(msg->cb_DSA.move_data),
				      sizeof(msg->cb_DSA.move_data),
				      CACRF_ClearD);

			  // make the chip continue at the next instruction
			  RESTART;

			} else {
kprintf("io complete, target still in data - eat it\n");
kprintf("  transferred $%lx of $%lx bytes (move_data.len = $%lx)\n",
msg->cb_SCSICmd->scsi_Actual,msg->cb_SCSICmd->scsi_Length,
msg->cb_DSA.move_data.len);

			  // we transferred all we expected to, but it's still
			  // in data phase.  Eat bytes (if reading), write 0's
			  // if writing...
			  g->st_ModifyData = 0;	   // garbage buffer
			  WRITE_LONG(b,scratch,0); // counter for extra bytes
			  CacheClearE(&(g->st_ModifyData),
				      sizeof(g->st_ModifyData),CACRF_ClearD);

			  // this will generate data_eaten ints when done
			  GOTO(LABEL(eat_data));
			}
			break;

		    case sync_received:
			{
				UBYTE sxfer = 0;
				UBYTE xferp = 0;
				UWORD i;

//kprintf("sync received: period %ldns, offset %ld\n",
//g->st_SyncBuf[0]*4,g->st_SyncBuf[1]);
				// successful sync negotiation
				// Do they do sync?
				if (g->st_SyncBuf[1] != 0)
				{
					// set up our transfer register values
					sxfer = min(g->st_SyncBuf[1],8);
// xferp = ((TP * Freq (in Mhz))/ (SBCL divisor (1-3)*1000)) - 4
// for example, if the other side returns 64 (and sbcl is 2):
// xferp = (64*4 * 66)/(2000) - 4 = 4.48 rounded up is 5
// or for 25:
// xferp = (25*4 * 50)/(3000) - 4 = 
					// make sure it rounds up
					xferp = (g->st_SyncBuf[0]*4*
						 	NCR_CLOCK_FREQ +
						 g->st_SBCL-1)/(g->st_SBCL);
					// if he's faster, use xferp = 0
					if (xferp >= 4)
						xferp -= 4;
					else
						xferp = 0;

//kprintf("sxfer = %ld, xferp = %ld\n",sxfer,xferp);
					if (xferp > 7)
					{
//kprintf("rejecting sync, can't go that slow\n");
					  // we can't go that slow!
					  // we must renegotiate async
					  if (msg->cb_DSA.send_msg.len <= 3)
					  {
					    // they initiated. too slow, reject
					    // we'll actually just ask for
					    // async.  This is tested later
					    sxfer = 0;

					  } else {
					    // we initiated.  renegotiate async
					    unit->hu_Addr->ha_DoneSync = 1;
					    msg->cb_DSA.send_msg.len = 0;
					    // 0 offset is asynch
					    addsync(msg,200/4,0);
					  }
					  // turn around and send reject/sync
					  GOTO(LABEL(message_reply));
					  break;

					} else {
						sxfer |= xferp << SXFERB_TP0; 
					}
					
				}
//kprintf("accepted sync\n");
				unit->hu_Addr->ha_SyncValue = sxfer;
				((struct Find_addr_entry *)
				 LABEL(find_addr_table))[unit->hu_Unit].
							set_sxfer.imm = sxfer;
				b->sxfer = sxfer;

				// set up cmdblock select value
				msg->cb_DSA.select_data.sync = sxfer;

				// scan other active IO's for the Addr and
				// update their sync values!
				for (i = 0; i < CMD_BLKS; i++)
				{
				  // if active and same ID
				  if ((g->st_CmdBlocks[i].iocb_CmdBlock.
				       cb_StateSave == STATE_ACTIVE) &&
				      (((struct HDUnit *)
				        (g->st_CmdBlocks[i].iocb_CmdBlock.
				          cb_IORequest->io_Unit))->hu_Unit ==
				       unit->hu_Unit))
				  {
//kprintf("updating cmd $%lx for sync\n",&(g->st_CmdBlocks[i].iocb_CmdBlock));
				    // same SCSI ID - update sxfer vals
				    // set up select value
				    g->st_CmdBlocks[i].iocb_CmdBlock.
				       cb_DSA.select_data.sync = sxfer;
				  }
				}

			    // SYNC is the only extended message we send...
			    // FIX! what if he sends us a second one?
			    // 3 is Identify + tag
			    if (msg->cb_DSA.send_msg.len <= 3)
			    {
// FIX! sync doesn't apply until he accepts our message
// FIX! check Quantum LPS105!  see scsitask.asm
				// ok, he initiated, reply
				unit->hu_Addr->ha_DoneSync = 1;

				// if sxfer is 0, we want to do async
				addsync(msg,g->st_UseFast ? 100/4 : 200/4,
					sxfer ? 8 : 0);

				// flush the cache for the entire block
				CacheClearE(&(g->st_CmdBlocks[0]),
					    sizeof(g->st_CmdBlocks),
					    CACRF_ClearD);

				GOTO(LABEL(message_reply));	// start script
			    } else {
				// we initiated.  all is good.
				// flush the cache for the entire block
				CacheClearE(&(g->st_CmdBlocks[0]),
					    sizeof(g->st_CmdBlocks),
					    CACRF_ClearD);
				GOTO(LABEL(message_ok));	// start script
			    }
			}
			break;

		    case modify_pointer_fetched:
kprintf("got modify pointers $%lx\n",g->st_ModifyData);
			// we got a modify pointers message(!)
			// add to pointer (subtract from length).  Better not
			// overflow or underflow!
			msg->cb_DSA.move_data.len  -= g->st_ModifyData;
			msg->cb_DSA.move_data.addr += g->st_ModifyData;

			CacheClearE(&(msg->cb_DSA.move_data),
				    sizeof(msg->cb_DSA.move_data),
				    CACRF_ClearD);

			// make the chip continue at the next instruction
			RESTART;
			break;

		    case unusual_message_in:
kprintf("Got unusual message $%lx\n",msg->cb_DSA.recv_buf[0]);
			goto reject_msg;

		    case extended_not_len_3:
kprintf("unusual extended message length\n");
		    case extended_not_sync:
kprintf("extended message not sync\n");
			// sync is the only extended message we support
reject_msg:		msg->cb_DSA.send_buf[0] = MESSAGE_REJECT;
			msg->cb_DSA.send_msg.len = 1;
			CacheClearE(msg,sizeof(*msg),CACRF_ClearD);

			GOTO(LABEL(message_reply));	// starts script
			break;

		    case target_rejected_msg:
			// the target rejected our last message.  Almost
			// certainly a sync negotiation message.
			if (msg->cb_DSA.send_msg.len > 3)
			{
				// rejected a sync negotiation message.
				// probably already set...
				unit->hu_Addr->ha_DoneSync = 1; // don't try again
//kprintf("target rejected sync message\n");
// FIX!!!!! undo sync negotiation!!!!!!
			} else {
				// what the hell did he reject? identify?
				// ignore...  I guess...
//kprintf("target rejected message $%lx\n",msg->cb_DSA.send_buf[0]);
			}
			GOTO(LABEL(message_ok));
			break;

		    case data_eaten:
kprintf("data eaten!!!!! ($%lx bytes)\n",b->scratch);
kprintf("$%lx(%ld), unit $%lx, %ld of %ld\n",
msg,msg->cb_Index,unit,
msg->cb_SCSICmd->scsi_Actual,msg->cb_SCSICmd->scsi_Length);

			msg->cb_ExtraData = b->scratch;
			WRITE_LONG(b,scratch,-1);	// all data xfered
			GOTO(LABEL(phase_switch)); // continue with next phase
			break;


// debugging... remove!!!! FIX!
case 20:
// kprintf("In identify, SFBR = $%lx, SCRATCH = $%08lx\n",b->sfbr,b->scratch);
/*
 kprintf("find_addr[%ld]($%08lx) = $%08lx ($%08lx - $%08lx)\n",unit->hu_Unit,
 &(((struct Find_addr_entry *)
	      LABEL(find_addr_table))[unit->hu_Unit]),
 ((struct Find_addr_entry *)
	      LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr,
 &(unit->hu_Find_lun),LABEL(no_nexus_found));
 kprintf("   -> $%08lx $%08lx $%08lx ($%08lx)\n",
 ((ULONG *) (((struct Find_addr_entry *)
	      LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr))[0],
 ((ULONG *) (((struct Find_addr_entry *)
	      LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr))[1],
 ((ULONG *) (((struct Find_addr_entry *)
	      LABEL(find_addr_table))[unit->hu_Unit].check_lun.addr))[2],
 &(msg->cb_DSA.reselect_entry));
*/
 RESTART;
 break;
case 21:
// kprintf("In get_tag, SCRATCH = $%08lx, TEMP = $%08lx\n",b->scratch,b->temp);
 RESTART;
 break;
case 22:
// kprintf("No nexus match found! SCRATCH = $%08lx, SFBR = $%lx\n",
// b->scratch,b->sfbr);
 RESTART;
 break;
case 23:
// if (!msg)
// {
// kprintf("Wierd! Nexus found: DSA $%lx, msg $%lx, SCRATCH = $%08lx\n",b->dsa,msg,
// b->scratch);
// kprintf("default DSA = $%lx, current dsa = $%lx\n",g->st_DefaultDSA,
// g->st_CurrentDSA);
// }
 RESTART;
 break;
case 24:
// if (g->st_SyncValues[0])
// {
//  kprintf("got message, SFBR = $%lx, dsa = $%lx, SCRATCH = $%08lx\n",
//  b->sfbr,b->dsa,b->scratch);
//  if (g->st_SyncValues[0]++ == 10)
//    g->st_SyncValues[0] = 0;
// }
// kprintf("Waiting for reselect\n");
// kprintf("data done: move 4,$%08lx($%08lx),$%08lx($%08lx)\n",
// ((ULONG *) LABEL(new_addr))[1],&(msg->cb_DSA.final_ptr),
// ((ULONG *) LABEL(new_addr))[2],&(msg->cb_DSA.move_data.addr));
// kprintf("\t   move 4,$%08lx($%08lx),$%08lx($%08lx)\n",
// ((ULONG *) LABEL(new_addr))[4],&(g->st_Zero),
// ((ULONG *) LABEL(new_addr))[5],&(msg->cb_DSA.move_data.len));
 RESTART;
 break;
		    case unknown_phase:
		    case no_msg_after_status:
		    case not_command_complete:
		    case bad_reselect:
		    case bad_address:
		    case selected_as_target:
		    case big_error_1:
		    case big_error_2:
			goto bad_int;

		    case reselect_during_select:
			// we were reselected during select.  Reset DSA
			// scheduler entry, since the select didn't happen
			// (i.e. "rearm" it, so it will get started later)
//kprintf("Reselect during select\n");
//kprintf("Was trying to select %ld.%ld, msg $%lx(%ld)\n",
//unit->hu_Unit,unit->hu_LUN,msg,msg->cb_Index);
//g->st_SyncValues[0] = 1;
			g->st_Scheduler.array[msg->cb_Index].
					next_sched.control = 0x00; // NOP
			// Flush caches to make sure the data is in RAM.
			CacheClearE(&(g->st_Scheduler.array[msg->cb_Index].
				      next_sched.control),1,CACRF_ClearD);

			// set dsa to default_dsa!  No command active.
			// CAREFUL! must flush this because the '710 will
			// want to modify it!
			g->st_CurrentDSA = g->st_DefaultDSA;
			CacheClearE(&(g->st_CurrentDSA),4,CACRF_ClearD);

			WRITE_LONG(b,dsa,(ULONG) g->st_DefaultDSA);
			RESTART;		// go handle reselect
			break;

#ifdef USE_CAM
		    case target_disconnect:
			// disconnect when in target mode.  If dsa == default,
			// then we ignored a bad lun request.  Otherwise we
			// have completed a response for the given DSA.
			RESTART;		// default to restart
			if (msg != NULL)	// not default_dsa
			{
				// never mark inquiries as used - reuse them
				if (*((UBYTE *) msg->cb_DSA.command_data.addr)
				    != S_INQUIRY)
				{
				    // FIX! use CCB
				    CachePostDMA(msg->cb_SCSICmd->scsi_Data,
					    &(msg->cb_SCSICmd->scsi_Length),0);

				    // FIX!!  set various actual fields, etc

				    TargetMsgFinished(g,msg);
				}
			}
			break;

		    case target_has_command:
			// We've fetched the command for a valid unit.  Decide
			// what to do with it, then jump to one of:
			//	get_data
			//	send_data
			//	send_status
			//	send_disconnect
			//	send_message
			// SCRATCH2 = select data,
			// SCRATCH1 = first command byte,
			// SCRATCH0 = identify command (no LUNTARs)
			{
			    struct TargetUnit *unit;
			    UBYTE selector,lun,command;
			    CCB *ccb;

			    // aren't bit encodings wonderful?
			    selector = find_bit(((b->scratch&0x00ff0000) >> 16)
						& ~(1 << g->st_OwnID));
			    lun = b->scratch & 0x00000007;
			    unit = g->st_MyLUNs[lun];
			    command = (b->scratch & 0x0000ff00) >> 8;

			    // test unit ready return success
			    if (command == S_TEST_UNIT_READY)
			    {
				GOTO(LABEL(send_success));
				break;
			    }

// FIX! handle request sense for unsupported units
// can handle in NCR probably
			    // see if we have a command block for this
			    // command.  Prefers available ones.
			    ccb = find_command(unit,command);
			    if (!ccb)
			    {
				// no CCB for that command exists
				// set normal sense data for this
				// initiator
				set_sense_data(unit,selector,
					       ILLEGAL_REQUEST,
					       INVALID_COMMAND,0);
				// return check condition using default_dsa
				GOTO(LABEL(send_check));
				break;
			    }
			    if (!(ccb->cam_Flags & TARGET_CCB_AVAILABLE))
			    {
				GOTO(LABEL(send_busy));
				break;
			    }

			    // have command, and it's available.  Use it.
			    ccb->???  (in use)
			    msg = ???
			    WRITE_LONG(b,dsa,(ULONG) &(msg->cb_DSA));

			    switch (command) {
			    case S_INQUIRY:
// FIX! handle length!  Copy command!
				GOTO(LABEL(send_data));
				break;

			    case S_SEND:
// FIX!
				GOTO(LABEL(get_data);
				break;

			    case S_RECEIVE:
// FIX!
				GOTO(LABEL(send_data);
				break;

			    case S_REQUEST_SENSE:
// FIX!
				GOTO(LABEL(send_data);
				break;

			    default:
				break;
			    }
			}
			break;

		    case target_abort:
			// initiator aborted an IO.  Clean everything up.
			RESTART;	// this sends it to disconnect
			if (msg)
			{
				// DSA will be reset when command is
				// next used.
			}
#endif // USE_CAM

		    default:
bad_int:
kprintf("Strange INT instruction: code %ld, at $%lx\n",b->dsps,b->dsp);
			// we got a BIG error.  we should probably reset
			// everything, abort all IO's, reset the bus (maybe),
			// and reinit the chip.
			abort_all(g,HFERR_Phase);
			return;
		    }
		}
		if (dstat & DSTATF_IID) {
			// Illegal instruction
			// very, very bad.  Call Alert().
kprintf("Illegal instruction: at $%lx ($%08lx ->$%08lx $%08lx)\n",b->dsp,
*((ULONG *)(b->dsp-4)),*((ULONG *)(b->dsp)),*((ULONG *)(b->dsp+4)));
kprintf("msg = $%lx, unit = $%lx (%ld:%ld), move_data = $%lx\n",
msg,unit,unit->hu_Unit,unit->hu_LUN,msg->cb_DSA.move_data.len);
			if (msg && msg->cb_DSA.move_data.len == 0 &&
			    (b->dsp == LABEL(dataout) + 8 ||
			     b->dsp == LABEL(datain) + 8))
			{
				// The target went to data phase when we didn't
				// expect any more data.  Eat the extra data
				// by acting as if we had a scsi scatter_dma
				// int, which it _should_ have generated.
				dstat &= ~DSTATF_IID;	// don't do it again
				goto handle_scatter;
			}
			abort_all(g,HFERR_Phase);
			return;
		}
// clear aborts for playing by hand
//if (dstat & DSTATF_ABRT)
//b->istat &= ~(ISTATF_ABRT);

		if (dstat &
		    ~(DSTATF_DFE |0x40| DSTATF_IID | DSTATF_SIR | DSTATF_SSI))
		{
		    // something evil happened.  what do we do now?  abort?
sstat0 = (status >> 8) & 0xff;
sstat1 = (status >> 16) & 0xff;
sstat2 = status >> 24;
kprintf("Strange DMA int: $%lx, at $%lx ($%08lx $%08lx ^ $%08lx $%08lx)\n",
dstat,b->dsp,
*((ULONG *)(b->dsp - 8)),*((ULONG *)(b->dsp - 4)),
*((ULONG *)b->dsp),*((ULONG *)(b->dsp + 4)));
kprintf("istat = $%02lx, sstat0 = $%02lx, sstat1 = $%02lx, sstat2 = $%02lx\n",
istat,sstat0,sstat1,sstat2);
kprintf("dbc = $%08lx, dsps = $%08lx, dnad = $%08lx, scratch = $%08lx\n",
b->dbc,b->dsps,b->dnad,b->scratch);
			abort_all(g,HFERR_Phase);
			return;
		}
	} // if (DIP)

	if (istat & ISTATF_SIP)	// SCSI interrupt?
	{
		// no need to grab them until now...
		sstat0 = (status >> 8) & 0xff;

//if (sstat0)
//kprintf("sstat0 = %lx, status = $%08lx\n",sstat0,status);
		if (sstat0 & SSTAT0F_MA)	// phase mismatch on move
		{
			// we're probably going to msg_in, to get a save_ptrs
			// and a disconnect.  Remember where we are in the
			// transfer, though don't move it into the move_data
			// table yet (wait for save_ptrs).
			UBYTE *addr;	// new DMA location
			ULONG len;
			LONG offset;	
//kprintf("phase mismatch on move\n");

			sstat1 = (status >> 16) & 0xff;
			sstat2 = status >> 24;

			// make certain we were in data_in or data_out!
			if (b->dsp != LABEL(dataout)+sizeof(struct move_inst)&&
			    b->dsp != LABEL(datain)+sizeof(struct move_inst))
			{
//len  = b->dbc & 0x00ffffff;
//addr = (UBYTE *) b->dnad;
//kprintf("phase mismatch not in data phase! dsp=$%lx, len=$%lx, addr=$%lx\n",
//b->dsp,len,addr);
				abort_all(g,HFERR_Phase);
				return;
			} else {
//kprintf("ctest8 = $%lx\n",b->ctest8);
//while (b->ctest8);
			    // bytes remaining in transfer
			    // dbc is only 3 low bytes...
			    len  = b->dbc & 0x00ffffff;
			    addr = (UBYTE *) b->dnad; // current DMA address

			    // calculate number of bytes left in fifo
			    // DDIR is 0 for host->SCSI - I assume it's still set!
			    offset = 0;
			    if (!(b->ctest0 & CTEST0F_DDIR))
			    {
//kprintf("we were writing...\n");
				// bytes left in FIFO - p4-25 '710 data manual
				if (!(dstat & DSTATF_DFE))
					offset = ((b->dfifo & 0x7f) -
						  (len & 0xff)) & 0x7f;

				// if OLF=1, there's a byte in b->sodl
				if (sstat1 & SSTAT1F_OLF)
					offset += 1;

				if (unit->hu_Addr->ha_SyncValue != 0)
				{
					// synch
					// if ORF=1, there's a byte in b->sodr
					if (sstat1 & SSTAT1F_ORF)
						offset += 1;
				}

				// flush DMA and SCSI FIFO's
				b->ctest8 |= CTEST8F_CLF;
			    } else {
//kprintf("we were reading, len = $%lx...\n",len);
//offset = 0;
//if (!(dstat & DSTATF_DFE))
//offset = ((b->dfifo & 0x7f) - (len & 0xff)) & 0x7f;
//if (sstat1 & SSTAT1F_ILF) offset++;
//if (offset)
//kprintf("read phase mismatch, offset %ld!\n",offset);
//offset = 0;
				// Programmer's manual says all bytes flush
				// first, so no bytes should be in chip.  We'll
				// assume this IS the case.  We may have to
				// flush on other interrupts.
			    }
//kprintf("$%lx bytes in fifo, len = $%lx, addr = $%lx\n",offset,len,addr);

			    len  += offset;	// adjust remaining xfer size
			    addr -= offset;	// and pointer

			    // remember how many bytes we've xfered
			    msg->cb_SCSICmd->scsi_Actual += 
						msg->cb_CurrentLength - len;
			    msg->cb_CurrentLength = len;
			    msg->cb_DSA.move_data.len  = len;
			    msg->cb_DSA.move_data.addr = (ULONG) addr;
			    // msg->cb_DSA.final_ptr should be unchanged

			    // reset the internal buffers/fifos...
			    // then wait for it to finish (resets when done)
// FIX! make simpler
			    if ((dstat & DSTATF_DFE) ||
				(sstat1 & (SSTAT1F_ILF | SSTAT1F_ORF)) ||
				(unit->hu_Addr->ha_SyncValue != 0 &&
				 sstat1 & SSTAT1F_OLF) ||
				(sstat2 & 0xF0))
			    {
				    // Flush fifo's
				    b->ctest8 = CTEST8F_CLF;
				    while (b->ctest8 & CTEST8F_CLF)
					;  // waiting for flush to finish
			    }

			    CacheClearE(&(msg->cb_DSA.move_data),
				        sizeof(msg->cb_DSA.move_data),
					CACRF_ClearD);

			    GOTO(LABEL(phase_switch)); // handle new phase
			    // we had better be getting a msg_in of save_ptrs,
			    // or a status (things like inquiry...)
			}

		}
		if (sstat0 & SSTAT0F_STO) {	// selection timeout
//kprintf("select timeout, id %ld lun %ld\n",unit->hu_Unit,unit->hu_LUN);
//if (!msg)
//kprintf("NULL MESSAGE!\n");
//else {
			// a selection timed out, or wait for disconnect
			// took more than 250ms, or there was a 250ms delay
			// between bytes transferred over the scsi bus.
			// We could get out of hung scsi busses here.  For now,
			// ignore all but selection timeout.  Hopefully we
			// won't have any timeouts during disconnect...

			// select timeout.  return CCB with error
			msg->cb_ErrorCode = HFERR_SelTimeout;
			UnSchedule(g,msg);
//}

			// see if there's anything to do...
			GOTO((ULONG) &(g->st_Scheduler));
		}
		if (sstat0 & SSTAT0F_SGE) {
kprintf("SCSI Gross error at $%lx ($%lx)\n",b->dsp,*((ULONG *) b->dsp));
			// SCSI Gross error
			// Something screwed up good.  What do we do?
			// Probably should force the connection to terminate,
			// and return an error.
			abort_all(g,HFERR_Phase);
			return;
		}
		if (sstat0 & SSTAT0F_UDC) {
kprintf("unexpected disconnect at $%lx ($%lx)\n",b->dsp,*((ULONG *) b->dsp));
			// Unexpected disconnect (unless _STO was set also)
			// return ccb with error
			abort_all(g,HFERR_Phase);
			return;
		}
		if (sstat0 & SSTAT0F_RST) {
kprintf("scsi reset\n");
			// reset - ignore, probably another machine and
			// a cable that doesn't have the reset line cut.
			// We'll need to flush ALL IO's, and restart them.
			// Also flush ALL negotiated sync values.
			// Nuke current_dsa, if any, go back to scheduler
// FIX!!!
		}
		if (sstat0 & SSTAT0F_PAR) {
kprintf("scsi parity error at $%lx ($%lx)\n",b->dsp,*((ULONG *) b->dsp));
			// parity error
			// what do we do?
			abort_all(g,HFERR_Parity);
			return;
// FIX!!!!
		}
	} // if (SIP)
    } // while (ints)

//if (restart==DO_RESTART)kprintf("+");else kprintf("goto $%lx\n",new_dsp);

    // we must restart or goto ONLY ONCE!
    if (restart==DO_RESTART) b->dcntl |= DCNTLF_STD;
    else		     WRITE_LONG(b,dsp,new_dsp);

#ifdef SINGLE_STEP
if (restart!=DO_RESTART)
 b->dcntl |= DCNTLF_STD;	// goto won't go until I hit start in SS
#endif
}

// we have an asm stub in scsitask.asm that allocates globals, sets up
// st_SysLib, and jumps to _SCSITask after inserting the global pointer under
// the Exec return address...

extern struct SCSIGlobals *quick_global;
extern void quick_int(void);

void __stdargs
SCSITask (struct SCSIGlobals *g,
	  volatile struct ncr710 *boardaddr,
	  struct Task *parent,
	  struct MsgPort *port,
	  LONG garbage,
	  struct HDDevice *device)
{
	struct Library *BattMemBase;

kprintf("In ncr!  board = $%lx, parent = $%lx, port = $%lx, g=$%lx\n",
boardaddr,parent,port,g);

	// use address of 53c710...
	g->st_BoardAddress = (void *) (((ULONG) boardaddr)+A3090_CHIP_OFFSET);
	g->st_CmdPort	= port;
	g->st_ThisTask	= FindTask(NULL);
	g->st_DeviceBase = device;
	device->hd_STGlobals = g;	// so lib can get to them for add_unit
//kprintf("HDDevice = $%lx, IOGlobals = $%lx\n",device,device->hd_IOGlobals);
//kprintf("s(SCSIGlobals) = %ld, s(DSA_entry) = %ld, s(IOCmdBlock) = %ld\n",
//sizeof(*g),sizeof(struct DSA_entry),sizeof(struct IOCmdBlock));
//kprintf("s(CommandBlock) = %ld, s(Scheduler) = %ld, s(Sched_entry) = %ld\n",
//sizeof(struct CommandBlock),sizeof(struct Scheduler),sizeof(struct Sched_entry));
//kprintf("scheduler = $%lx, DSA_values = $%lx, default dsa = $%lx\n",
//&(g->st_Scheduler),&(g->st_DSA_values[0]),&(g->st_DefaultStorage));
//kprintf("board @ $%lx\n",g->st_BoardAddress);

	// allocate signals for ints and commands
	g->st_IntPendSig	= AllocSignal(-1);
	g->st_IntPendMask	= 1L << g->st_IntPendSig;
	g->st_WaitMask		= g->st_IntPendMask;
	g->st_CmdPendSig	= AllocSignal(-1);
//	g->st_CmdPendMask	= 1L << g->st_CmdPendSig;
	g->st_WaitMask		|= 1L << g->st_CmdPendSig;

	// init port
	g->st_CmdPort->mp_SigBit	= g->st_CmdPendSig;
	g->st_CmdPort->mp_SigTask	= g->st_ThisTask;
	g->st_CmdPort->mp_Node.ln_Type	= NT_MSGPORT;
	NewList(&(g->st_CmdPort->mp_MsgList));
#ifdef ZORRO_3
	NewList((struct List *) &(g->st_Z2List));
#endif

#ifndef NEW_040_LIB
	// do we need 16-byte alignment?
	g->st_Is040 = ((SysBase->AttnFlags & AFF_68040) != 0);
#endif

#ifdef IS_A3090
	// make sure the quick int can find the globals...
	quick_global = g;
#endif

// FIX!!!! add V39 check for real roms!
#ifndef DISABLE_QUICK_INTS
#ifndef V39
	if (SysBase->LibNode.lib_Version >= 39)
#endif
		g->st_QuickIntNum = ObtainQuickVector(quick_int);
#ifndef V39
	else
		g->st_QuickIntNum = 0;
#endif
#endif

#ifdef IS_A4000T
	// must set EA in DCNTL _before_ adding the int server.  THis is
	// because the first access will never end unless the chip is set
	// to link STERM and SLAC internally.
	g->st_BoardAddress->dcntl = DCNTLF_EA | DCNTLF_COM;
#endif

#ifndef DISABLE_QUICK_INTS
	if (g->st_QuickIntNum == 0)
	{
#endif
		// not running V39.109 KS or later, make int2 server
		g->st_IntServer = AllocMem(sizeof(*(g->st_IntServer)),
					   MEMF_CLEAR|MEMF_PUBLIC);
// fix
		g->st_IntServer->is_Node.ln_Type	= NT_INTERRUPT;
		g->st_IntServer->is_Node.ln_Pri		= INTPRI;
		g->st_IntServer->is_Node.ln_Name	= "NCR SCSI";
		g->st_IntServer->is_Data		= g;
		g->st_IntServer->is_Code		= (VOID (*)())
							  ncr53c710_int;

		AddIntServer(INTNUM,g->st_IntServer);

#ifndef DISABLE_QUICK_INTS
	} else {
//kprintf("turned on quick int %ld\n",g->st_QuickIntNum);

		((UBYTE *) boardaddr)[A3090_VECTOR] = g->st_QuickIntNum;
	}
#endif

#ifdef IN_KICK
	// set up for battmem stuff
	g->st_MaxLUN = 0;
	g->st_OwnID  = 7;	// can be changed by battmem or jumpers
	g->st_SelTimeout = 0;

	BattMemBase = OpenResource(BATTMEMNAME);
	if (BattMemBase)
	{
		ReadBattMem(&(g->st_SelTimeout),BATTMEM_SCSI_TIMEOUT_ADDR,
			    BATTMEM_SCSI_TIMEOUT_LEN);

		ReadBattMem(&(g->st_MaxLUN),BATTMEM_SCSI_LUNS_ADDR,
			    BATTMEM_SCSI_LUNS_LEN);
		if (g->st_MaxLUN)
			g->st_MaxLUN = 7;

		ReadBattMem(&(g->st_OwnID),BATTMEM_SCSI_HOST_ID_ADDR,
			    BATTMEM_SCSI_HOST_ID_LEN);
		g->st_OwnID ^= 7;			// value is complemented

		ReadBattMem(&(g->st_SendSync),BATTMEM_SCSI_SYNC_XFER_ADDR,
			    BATTMEM_SCSI_SYNC_XFER_LEN);

// FIX!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// add st_UseTags, st_UseFast!!!!
	}
#endif

#ifndef	IN_KICK
	// read jumpers
	{
		UBYTE jumpers = ((UBYTE *) boardaddr)[A3090_JUMPERS];
//kprintf("jumpers = $%lx\n",jumpers);

#define J_ID	0x07
#define J_ID_SHIFT 0
#define J_FAST	0x08
#define J_SLOW	0x10
#define J_SYNC	0x20
//#define J_TAGS 0x40	termination
#define J_LUNS	0x80

		g->st_UseFast	= jumpers & J_FAST;
		g->st_SendSync	= jumpers & J_SYNC;
//		g->st_UseTags	= jumpers & J_TAGS;
		g->st_MaxLUN	= (jumpers & J_LUNS ? 0 : 7);
		g->st_SelTimeout= !(jumpers & J_SLOW);
		g->st_OwnID	= (jumpers & J_ID) >> J_ID_SHIFT;
	}
#endif

//if ((((ULONG) &jump_rel_inst) & 0x03) != 0)
//kprintf("WARNING!!!! jump_rel_inst not on LW boundary!\n");
kprintf("fast = %ld, Sync enable = %ld\n",g->st_UseFast,g->st_SendSync);
kprintf("OwnID = %ld, UseTags = %ld, MaxLUN = %ld\n",g->st_OwnID,g->st_UseTags,
g->st_MaxLUN);
//g->st_SyncValues[0] = 0;

//kprintf("initing structs\n");
	init_structs(g,device->hd_IOGlobals);

//kprintf("initing chip\n");
	// this enables ints
	init_chip(g);

	// _After_ scsi bus reset, wait 15 seconds for drives to come up if
	// needed.
	if (g->st_SelTimeout)
	{
		int i;

		for (i = 0; i < LONG_SPINUP_DELAY; i++)
			WaitASecond(SysBase);
	}

	// start dsp in the scheduler

	// must load default DSA in case IO gets scheduled before NCR chip
	// gets to wait reselect.  theoretically possible.
	WRITE_LONG(g->st_BoardAddress,dsa,(ULONG) g->st_DefaultDSA);
kprintf("starting ncr chip...");
	WRITE_LONG(g->st_BoardAddress,dsp,(ULONG) &(g->st_Scheduler));

#ifdef SINGLE_STEP
	g->st_BoardAddress->dcntl |= DCNTLF_STD;
#endif

	// done with init, signal parent
	Signal(parent,SIGF_SINGLE);

	loop(g);
	// NOTREACHED
}

#endif
