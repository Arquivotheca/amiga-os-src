	STRUCTURE IOTask_Globals,0
		APTR	iot_SysLib		exec library
		APTR	iot_BoardAddress	where the board base is
		APTR	iot_ThisTask		self pointer to task
		APTR	iot_ReqPort		msg port for incoming IOReqs
		APTR	iot_CmdRetPort		msg port for returning cmds
		APTR	iot_SCSITaskPort	msg port for SCSITask to init

		UBYTE	iot_CmdRetSig		command returned signal
		UBYTE	iot_ReqPendSig		IORequest pending signal
		ULONG	iot_CmdRetMask		command returned mask
		ULONG	iot_ReqPendMask		IORequest pending mask

		STRUCT	iot_FreeCmds,LH_SIZE	free command block queue
	LABEL iot_SIZEOF
