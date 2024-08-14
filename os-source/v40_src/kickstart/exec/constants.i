**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*
**********************************************************************
*
* $Id: constants.i,v 39.10 93/05/14 15:11:49 mks Exp $
*
**********************************************************************

TEMP_SUP_STACK		EQU $000400 	; Used before memory allocator

*************** DEBUGGING COLORS AND INFORMAION ******************************
COLORON 	EQU $0200 ; colorburst on

OK_HARDWARE	EQU $0111 ; grey  -First software action
OK_DEBUG	EQU $000F ; blue  -debugging

CC_BADROMSUM	EQU $0F00 ; red   -Kickstart ROM is bad
CC_BADRAM	EQU $00F0 ; green -Chip memory is bad
CC_EXCEPTION	EQU $0FE5 ; yellow-Exception taken before software setup
CC_NOMODULES	EQU $0F0F ; purple-Return from InitResident(RTF_COLDSTART)
;CC_BADCHIPS	EQU $000F ; blue  -Regster test failed

BAUD_9600	EQU 372

******************************************************************************
ResetVector			EQU $00
INITOFFSET			EQU $02  ;Location of JMP instruction
SysBase 			EQU $04
BusErrorVector			EQU $08
AdddressErrorVector		EQU $0C
IllegalInstructionVector	EQU $10
ZeroDivideVector		EQU $14
CHKVector			EQU $18
TRAPccVector			EQU $1C
PrivTrapVector			EQU $20
TraceVector			EQU $24
Line1010Vector			EQU $28
Line1111Vector			EQU $2C
Trap15Vector			EQU $BC  ;Used by Wack
LastExceptVector		EQU $100 ;Last exception vector

LOCATION_ZERO			EQU 0
ALERT_STORE			EQU $100 ;Two longs (prior Alert number)

* The following should never be used in production code!
REG_STORE			EQU $180 ;All 16 registers on Alert()
USP_STORE			EQU $1c0 ;user stack pointer
FRAME_STORE			EQU $1c4 ;first 8 bytes of stack frame
WACKBASE			EQU $200 ;location of wack global data

;A non-cachable memory location that takes a long time to read.  Used for
;delays.
SLOW_READ			EQU $00dff01a	;_dskbytr
JMPINSTR			EQU $4EF9	;Warning: Self-modifying code!

*************** OPERATING CONSTANTS *******************************************

DEFAULT_QUANTUM EQU	4	;Default constant for round-robin scheduling.
				;expressed in VBLANK times (50 or 60/sec).

TASK_ID_WRAP	EQU	1024	;Constant that system ex_TaskID counter wraps
				;back to.  Some of the first 1024 task ID's will
				;typically be assigned to permanent tasks.

*************** CPU CONSTANTS *************************************************

;Private 68040 stuff
CACRF_040_DCache		EQU $80000000
CACRB_040_DCache		EQU 31
CACRF_040_ICache		EQU $00008000
CACRB_040_ICache		EQU 15

PAGESIZE_040			EQU 8*4096


STACK_COOKIE	EQU $BAD1BAD3	;Stack probe value
ZERO_COOKIE	EQU $C0EDBABE	;Value left at location zero
MEMMUNG_COOKIE	EQU $DEADBEEF	;Free memory is set to this value.
MEMMUNG_CRACKER	EQU $DEAD0051	;Non-clear allocated memory is set to this

*
* In LastAlert[3] will be stored a timeout value.  This value is in the
* number of frames the alert will be displayed for.  The frame rate
* depends on the system's display.  It is usually in the 50 to 60 fps range.
*
* This value will be inited by EXEC at boot time but exec will try its
* best to keep the user's value between warm boots.  (Much like the
* capture stuff)
*
;DEFAULT_TIMEOUT	equ	9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)...
; Defined in romconstants.i as it is different from machine to machine
