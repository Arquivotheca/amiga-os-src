******************************************************************
*
*
* Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.
* Permission granted for non-commercial use
*
*****************************************************************
*
* pardev.i -- external declarations
*
*****************************************************************


;--- Assemble-time options
INFO_LEVEL  EQU 0     ; Specify amount of debugging info desired
		      ; If > 0 you must link with debug.lib!
		      ; You will need to run a terminal program to
		      ; set the baud rate.
*INTRRUPT   SET 1     ; Remove "*" to enable fake interrupt code
AUTOMOUNT   EQU 0     ; Work with the "mount" command if 0
		      ; Do it automatically if 1

;-----------------------------------------------------------------------
;
; device data structures
;
;-----------------------------------------------------------------------

; maximum number of units in this device

MD_NUMUNITS   EQU   4

    STRUCTURE MyDev,LIB_SIZE
   UBYTE   md_Flags
   UBYTE   md_Pad1
   ;now longword aligned
   ULONG   md_SysLib
   ULONG   md_SegList

    STRUCT	PAR_queue,MLH_SIZE
    APTR	Misc
    APTR	Intr
    APTR	SoftIntr
    APTR	Cia
    STRUCT	DefaultArray,8
    APTR	PAR_request

	APTR	Timer_Port
	APTR	Timer_Req
	APTR	ResourceOK

    BOOL	PAR_shared
    BOOL	PAR_stopped
    BOOL	ACKwasON
    BOOL	TimerUsed

	UBYTE	ICRmask
	UBYTE	OpenTimerError
	UBYTE	DefaultServer
	UBYTE	GotCIAFlag

   LABEL   MyDev_Sizeof

MYDEVNAME	MACRO
	DC.B	'parallel.device',0
	ENDM
