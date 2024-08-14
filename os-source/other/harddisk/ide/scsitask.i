; global data used EXCLUSIVELY by the main SCSI task and interrupt server
	STRUCTURE SCSIGlobals,0
		APTR	st_SysLib		exec library
		APTR	st_BoardAddress		where the board base is
		APTR	st_CmdPort		message port for commands
		APTR	st_ThisTask		self pointer to task
		APTR	st_IntServer		the interrupt server
		APTR	st_DeviceBase		ptr to Device structure

		IFD IS_IDE
		APTR	st_IntAddress		pointer to gayle int addr
		ENDC

		IFD SCSI_SUPPORTED
		UWORD	st_IntPointer		next slot for storing int info
		UWORD	st_TaskPointer		next slot for reading int info
		STRUCT	st_IntData,32		enough for 8 interrupts
		ENDC

		UBYTE	st_IntPendSig		interrupt pending signal
		UBYTE	st_CmdPendSig		command pending signal
		ULONG	st_IntPendMask		interrupt pending mask
		ULONG	st_WaitMask		all signals to Wait() on

		IFD SCSI_SUPPORTED
		STRUCT	st_SyncValues,8		sync xfer vals for each address
		ENDC

		APTR	st_RunningUnit		currently selected/active unit
		STRUCT	st_WorkingUnits,LH_SIZE	units waiting for reselection
		STRUCT	st_WaitingUnits,LH_SIZE	units waiting for selection

		IFD SCSI_SUPPORTED
		UBYTE	st_SelectPending	so we don't select twice
		UBYTE	st_DMAGoing		flag, DMA was running
		UBYTE	st_LastPhase		for unexpected phase ints
		UBYTE	st_WDCRegister		WDC register being accessed
		UBYTE	st_ReselUnit		last unit to reselect

; these are flags used to modify the behaviour of various low level things
; they are modified by the jumper settings read at the second XT port (these
; used to be used for setting the SCSI ID of the controller card itself)
		UBYTE	st_MaxLUN		highest LUN to look at (0 or 7)
		UBYTE	st_SelTimeout		0 = 128msec   1 = 2sec
		UBYTE	st_ParityCheck		0 do it, 1 don't
		UBYTE	st_Disconnect		1 = support disconnection
; the Boyer DMA chip cannot DMA to odd word addresses.  Since WTCH can be
; written to and the next DMA chip won't have WTCH we can use this to detect
; which chip we have.  The following location is set to 1 if using Boyer DMA
		UBYTE	st_OwnID		ID to use for the WDC chip
		UBYTE	st_SendSync		should we initiate sync?
		UBYTE	st_Flags		see below (boyer, etc)

; variables that depend on which chip we have (...93 or ...93a)
		UBYTE	st_MaxOffset		5 or 12
		UBYTE	st_MinXferPeriod	3 or 2
		UBYTE	st_MinMsgPeriod		((3 or 2)*CLOCK_PERIOD)/4

		ENDC SCSI_SUPPORTED

	LABEL st_SIZEOF

; values for st_Flags:
	BITDEF	ST,FixBoyer,0
	BITDEF	ST,33c93a,1
	BITDEF	ST,NoSignalG2,2
	BITDEF	ST,IgnoredG2,3

; these are the queue type values for showing which SCSI queue a unit is on
; and also double up as flags for what to do when a disconnect occurs
UNIT_TIMED_OUT	EQU	5	selection didn't work (fake disconnect code)
UNIT_GOT_ERROR	EQU	4	if disconnected w/o a msg, then error
UNIT_RUNNING	EQU	3	this is the active unit (it is selected now)
UNIT_WORKING	EQU	2	this unit is waiting for reselection
UNIT_WAITING	EQU	1	this unit needs selection to start a command
UNIT_QUIET	EQU	0	this unit isn't on any of the SCSI task queues

; these are the values used to indicate which phase we were in when an
; unexpected phase interrupt came in.  We only do multiple byte transfers
; in data or command phases, so these are the only ones that need fixing
PAD_LAST	EQU	2	wasting bytes was the last thing we did
COMMAND_LAST	EQU	1	command phase was the last thing we did
DATA_LAST	EQU	0	data phase was the last


; we need an extended SCSI structure to handle linked commands.
 STRUCTURE mySCSICmd,scsi_SIZEOF
    APTR    scsi_NextLinked     ; if this is a linked cmd, next should be valid
    LABEL   myscsi_SIZEOF
