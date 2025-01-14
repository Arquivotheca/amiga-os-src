struct IOTask_Globals {
	struct ExecBase *iot_SysLib;	// exec library
	struct HDDevice *iot_Device;	// device base
	struct ncr710 *iot_BoardAddress; // where the board base is
	struct Task *iot_ThisTask;	// self pointer to task
	struct MsgPort *iot_ReqPort;	// msg port for incoming IOReqs
	struct MsgPort *iot_CmdRetPort;	// msg port for returning cmds
	struct MsgPort *iot_TimerPort;	// msg port for returning timers
	struct MsgPort *iot_SCSITaskPort; // msg port for SCSITask to init

	UBYTE	iot_CmdRetSig;		// command returned signal
	UBYTE	iot_ReqPendSig;		// IORequest pending signal
	UBYTE	iot_TimerSig;		// Timer request pending signal
	UBYTE	iot_TimerSent;		// flag, 1 means timer pending

	ULONG	iot_CmdRetMask;		// command returned mask
	ULONG	iot_ReqPendMask;	// IORequest pending mask
	ULONG	iot_TimerMask;		// Returned timer request mask

	struct CommandBlock *iot_PollCmd;	// command block for polling
	struct timerrequest *iot_TimerReq;	// timer request for polling

// FIX! this only handles 8 removable _units_!!!
	struct HDUnit *iot_PollMe[8];	// units to be polled
	ULONG	iot_TimeOut;		// time to wait for checking

#ifdef ONE_TASK
	ULONG	iot_ATRegs[4];		// global regs for SCSITask
	ULONG	iot_IntPendMask;	// to handle WD ints
#endif
	struct MinList iot_FreeCmds;	// free command block queue
};

// An SCSITask io block, plus stuff needed by IOTask
// must be longword-sized!!
struct IOCmdBlock {
	struct CommandBlock iocb_CmdBlock;
	UBYTE iocb_Sense[24];
	struct SCSICmd iocb_Cmd1;	// not LW aligned
	UBYTE iocb_Command1[12];
	struct SCSICmd iocb_Cmd2;	// but 2 puts us aligned again
	UBYTE iocb_Command2[12];
#ifdef FIXING_READS
	UBYTE iocb_Kludge[1024];
#endif
};
