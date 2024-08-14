
// global data used EXCLUSIVELY by the main SCSI task and interrupt server
struct SCSIGlobals {
#ifdef NCR53C710
	struct Scheduler st_Scheduler;	// scheduler array for IO's
	struct DSA_entry *st_DSA_values[CMD_BLKS]; // array of ptrs
	struct IOCmdBlock st_CmdBlocks[CMD_BLKS];   // array of struct
	struct DSA_entry st_DefaultStorage;	// default command block
	struct DSA_entry *st_DefaultDSA;	// for scripts
	struct DSA_entry *st_CurrentDSA;	// for scripts
#ifdef ZORRO_3
	struct MinList st_Z2List;	// list of Z2 IO's waiting
	UBYTE	*st_Z2Buffer;		// Z2 buffer space
#endif	
	ULONG	st_ModifyData;		// longword for modify_ptrs storage
					// also used for bad length storage
	ULONG	st_Zero;		// longword zero source for NCR
	UBYTE	st_SyncBuf[4];		// incoming sync data goes here
	ULONG	*st_Script;		// Script is stored here
	UBYTE	st_ISTATData[8];	// interrupt queue for istat

	UWORD	st_SBCL;		// one of 3000, 1000, 1500, 2000
	UBYTE	st_QuickIntNum;		// 8 bit quick int number
	UBYTE	st_Is040;		// is this an '040 (16 byte alignment?)
#endif
	struct ExecBase *st_SysLib;	// exec library
	volatile struct ncr710	*st_BoardAddress;// where the board base is
	struct MsgPort	*st_CmdPort;		// message port for commands
	struct Task	*st_ThisTask;		// self pointer to task
	struct Interrupt *st_IntServer;		// the interrupt server
	struct HDDevice *st_DeviceBase;		// ptr to Device structure

#ifdef IS_IDE
	UBYTE	*st_IntAddress;		// pointer to gayle int addr
#endif

#ifdef SCSI_SUPPORTED
	volatile UWORD	st_IntPointer;	// next slot for storing int info
	UWORD	st_TaskPointer;		// next slot for reading int info
	ULONG	st_IntData[8];		// enough for 8 interrupts
#endif

	ULONG	st_IntPendMask;		// interrupt pending mask
	ULONG	st_WaitMask;		// all signals to Wait() on

#ifdef SCSI_SUPPORTED
	UBYTE	st_SyncValues[8];	// sync xfer vals for each address
#endif

	struct Unit *st_RunningUnit;	// currently selected/active unit
	struct MinList st_WorkingUnits; // units waiting for reselection
	struct MinList st_WaitingUnits; // units waiting for selection

	UBYTE	st_IntPendSig;		// interrupt pending signal
	UBYTE	st_CmdPendSig;		// command pending signal

	// for all scsi and A1000
#if !defined(IS_A300) && !defined(IS_CDTVCR)
	UBYTE	st_SelTimeout;		// 0 = 128msec   1 = 2sec
#endif

#ifdef SCSI_SUPPORTED
	UBYTE	st_SelectPending;	// so we don't select twice

	UBYTE	st_DMAGoing;		// flag, DMA was running
	UBYTE	st_LastPhase;		// for unexpected phase ints
	UBYTE	st_WDCRegister;		// WDC register being accessed
	UBYTE	st_ReselUnit;		// last unit to reselect

// these are flags used to modify the behaviour of various low level things
// they are modified by the jumper settings read at the second XT port (these
// used to be used for setting the SCSI ID of the controller card itself)
	UBYTE	st_MaxLUN;		// highest LUN to look at (0 or 7)
	UBYTE	st_ParityCheck;		//  0 do it, 1 don't
	UBYTE	st_Disconnect;		// 1 = support disconnection
// the Boyer DMA chip cannot DMA to odd word addresses.  Since WTCH can be
// written to and the next DMA chip won't have WTCH we can use this to detect
// which chip we have.  The following location is set to 1 if using Boyer DMA
	UBYTE	st_OwnID;		// ID to use for the WDC chip

	UBYTE	st_SendSync;		// should we initiate sync?
	UBYTE	st_Flags;		// see below (boyer, etc)

// variables that depend on which chip we have (...93 or ...93a)
	UBYTE	st_MaxOffset;		// 5 or 12
	UBYTE	st_MinXferPeriod;	//  or 2

	UBYTE	st_MinMsgPeriod;	// ((3 or 2)*CLOCK_PERIOD)/4
#ifdef A590_A2091_ONLY
	UBYTE	st_TerminalCount;	// do we require terminal count?
#endif
#endif

// commonly only used for 4091
	UBYTE	st_UseTags;		// is tagged queuing allowed?
	UBYTE	st_UseFast;		// should we use SCSI-2 Fast?
};

// values for st_Flags:
#define STF_FixBoyer	0
#define STF_33c93a	1
#define STF_NoSignalG2	2
#define	STF_IgnoredG2	4

// these are the queue type values for showing which SCSI queue a unit is on
// and also double up as flags for what to do when a disconnect occurs
#define UNIT_TIMED_OUT	5	// selection didn't work (fake disconnect code)
#define UNIT_GOT_ERROR	4	// if disconnected w/o a msg, then error
#define UNIT_RUNNING	3	// this is the active unit (it is selected now)
#define UNIT_WORKING	2	// this unit is waiting for reselection
#define UNIT_WAITING	1	// this unit needs selection to start a command
#define UNIT_QUIET	0	// this unit isn't on any of the SCSI task queues

// these are the values used to indicate which phase we were in when an
// unexpected phase interrupt came in.  We only do multiple byte transfers
// in data or command phases, so these are the only ones that need fixing
#define PAD_LAST	2	// wasting bytes was the last thing we did
#define COMMAND_LAST	1	// command phase was the last thing we did
#define DATA_LAST	0	// data phase was the last


// we need an extended SCSI structure to handle linked commands.
struct mySCSICmd {
	struct SCSICmd comm;
	struct mySCSICmd *scsi_NextLinked; // if linked, next should be valid
};

