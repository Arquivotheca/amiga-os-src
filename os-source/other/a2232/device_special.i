	    IFND    EXEC_INTERRUPTS_I
	    INCLUDE "exec/interrupts.i"
	    ENDC

DISK_DEVICE	EQU 1	;1=disk device 0=rom device
VERSION 	EQU 33	;Major
REVISION	EQU 13	;Minor
ROMTAG_PRI	EQU 0

DEVICE_NAME MACRO
	    dc.b 'serial.device',0
	    ENDM

DEVICE_ID   MACRO
	    dc.b 'A2232 driver 33.13 (01 Aug 91)',13,10,0
	    ENDM

SPECIAL     MACRO
	    UBYTE   md_TestIntFreq    ;Millisecons between server interrupts
	    UBYTE   md_Pad1
	    UWORD   md_Pad2
	    APTR    md_OurConfigDev   ;Zero if our board could not be found
	    APTR    md_ExpansionBase  ;Base of expansion.library
	    APTR    md_IntServer      ;Pointer to interrupt server, or NULL
	    ENDM
