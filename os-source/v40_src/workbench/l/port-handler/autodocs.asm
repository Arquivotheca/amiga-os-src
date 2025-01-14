
******* port-handler/port_handler ********************************************
*
*   NAME
*       port_handler -- standard serial/parallel/printer port handler.
*
*   SYNOPSIS
*	Open("PRT:[RAW]",...);
*
*	Open("SER:[baud/control]",...);
*
*	Open("PAR:",...);
*
*   FUNCTION
*	Port-handler allows AmigaDOS to interface to a standard parallel
*	device, a standard printer device, and a standard serial device.
*
*	Accessing "PAR:" connects AmigaDOS to the system's parallel port.
*
*	Accessing "PRT:" connects AmigaDOS to the system's printer. Data
*	written to PRT: is processed by the Amiga's printer device, handling
*	conversions of ANSI sequences, and sent to the printer. Accessing
*	"PRT:RAW" instead causes carriage return translation to be turned
*	off in the printer device while printing.
*
*	Accessing "SER:" connects AmigaDOS to the system's default serial port.
*	Following SER:, the baud rate and some control information can be
*	supplied. This is done in the form "SER:baud/control" where baud is
*	a number indicating the baud rate, and where control is a 3 letter
*	sequence denoting the number of read/write bits, the parity, and the
*	number of stop bits. For example, "SER:9600/8N1" connects to the
*	serial port, sets the baud rate to 9600, with 8 bit data, no parity,
*	and 1 stop bit. The possible control settings are:
*
*		1st character: 7 or 8
*		2nd character: N (No parity), O (Odd parity), E (Even parity)
*			       M (Mark parity), S (Space parity)
*		3rd character: 1 or 2
*
*	Specifying no baud rate or control values when accessing SER: gives you
*	the values set in Serial preferences.
*
*   PACKETS
*	ACTION_FINDINPUT
*	ACTION_FINDOUTPUT
*	ACTION_FINDUPDATE
*	ACTION_READ
*	ACTION_WRITE
*	ACTION_END
*	ACTION_IS_FILESYSTEM
*
*   MOUNTLIST ENTRIES
*	dos.library automatically mounts PAR:, PRT:, and SER: upon
*	system boot up. The mount entries used by dos.library are equivalent
*	to:
*
*	   SER:    Handler   = L:Port-Handler
*	           Priority  = 5
*	           StackSize = 800
*		   GlobVec   = 1
*	           Startup   = 0
*	   #
*
*	   PAR:    Handler   = L:Port-Handler
*	           Priority  = 5
*	           StackSize = 800
*		   GlobVec   = 1
*	           Startup   = 1
*	   #
*
*	   PRT:    Handler   = L:Port-Handler
*	           Priority  = 5
*	           StackSize = 1000
*		   GlobVec   = 1
*	           Startup   = 2
*	   #
*
*	Starting with V40, you can also provide disk-based mount entries
*	which let you use port-handler as a serial handler on various devices
*	and units. The form of these mount entries is:
*
*	   SER0:   EHandler  = L:Port-Handler
*	           Priority  = 5
*	           StackSize = 800
*		   GlobVec   = 1
*	           Device    = serial.device
*	           Unit      = 0
*	           Flags     = 0
*		   Control   = "8N1"
*	           Baud      = 9600
*	#
*
*	The "Device", "Unit" and "Flags" keywords define the parameters
*	that the handler uses for OpenDevice(). "Baud" and "Control"
*	define the default values for baud rate and control information for
*	whenever that serial handler is accessed by name only, without
*	explicit baud rate and control information specified.
*
*   NOTE
*	Prior to V40, the baud rate and control information for SER: were
*	only recognized if an A2232 multi-serial card was installed in the
*	system. Starting with V40, these values are always recognized.
*
******************************************************************************

