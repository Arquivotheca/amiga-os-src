	STRUCTURE IOTask_Globals,0
		APTR	iot_SysLib		exec library
		APTR	iot_Device		device base
		APTR	iot_BoardAddress	where the board base is
		APTR	iot_ThisTask		self pointer to task
		APTR	iot_ReqPort		msg port for incoming IOReqs
		APTR	iot_CmdRetPort		msg port for returning cmds
		APTR	iot_TimerPort		msg port for returning timers
		APTR	iot_SCSITaskPort	msg port for SCSITask to init

		UBYTE	iot_CmdRetSig		command returned signal
		UBYTE	iot_ReqPendSig		IORequest pending signal
		UBYTE	iot_TimerSig		Timer request pending signal
		UBYTE	iot_TimerSent		flag, 1 means timer pending

		ULONG	iot_CmdRetMask		command returned mask
		ULONG	iot_ReqPendMask		IORequest pending mask
		ULONG	iot_TimerMask		Returned timer request mask

		APTR	iot_PollCmd		command block for polling
		APTR	iot_TimerReq		timer request for polling

		STRUCT	iot_PollMe,8*4		units to be polled
		ULONG	iot_TimeOut		time to wait for checking

	IFD ONE_TASK
		STRUCT	iot_ATRegs,4*4		global regs for SCSITask
		ULONG	iot_IntPendMask		to handle WD ints
	ENDC
		STRUCT	iot_FreeCmds,MLH_SIZE	free command block queue
	LABEL iot_SIZEOF

; command blocks have all of their separate parts allocated in one big chunk
; which is logically split into the smaller parts for pointer initialisation

; An SCSITask io block, plus stuff needed by IOTask
; must be longword-sized!!
 STRUCTURE IOCmdBlock,0
	STRUCT iocb_CmdBlock,cb_SIZEOF
	STRUCT iocb_Sense,24
	STRUCT iocb_Cmd1,scsi_SIZEOF	; not LW aligned
	STRUCT iocb_Command1,12
	STRUCT iocb_Cmd2,scsi_SIZEOF	; but 2 puts us aligned again
	STRUCT iocb_Command2,12
    IFD FIXING_READS
	STRUCT iocb_Kludge,1024
    ENDC
 LABEL CBSZ
