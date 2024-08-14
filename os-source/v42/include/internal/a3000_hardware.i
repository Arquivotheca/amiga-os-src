	IFND	A3000_HARDWARE_I
A3000_HARDWARE_I	SET	1
**
**	$Id: a3000_hardware.i,v 36.3 90/06/17 21:21:59 bryce Exp $
**
**	Definitions for hardware found only on the A3000
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

;Real time clock and SCSI chips
A3000_RTC			EQU $00dc0000	;base address
A3000_SCSI			EQU $00dd0000	;base address



;Gary control block.  One bit per byte sized memory location (Bit 7).
;pretend these are supervisor-only.
A3000_BusTimeoutMode		EQU $00de0000	;0=DSACK 1=BERR|0=ok 1=timed out
A3000_BusTimeoutEnable		EQU $00de0001	;Bit 7  0=enable 1=disable
A3000_PUD			EQU $00de0002	;Power-Up-Detect. Bit 7, 1=cold
A3000_Intena			EQU $00dfb000

	BITDEF BTM,BERR,7
	BITDEF BTE,DISABLE,7
	BITDEF PUD,CYCLED,7




;Ramsey control registers.  Will be supervisor-only with next silicon.
;The SCSI related Ramsey register is not listed here.
A3000_RamControl		EQU $00de0003
A3000_RamseyVersion		EQU $00de0043	;12D ramsey==$0d
ANCIENT_RAMSEY			EQU $7f

	BITDEF RAMC,PAGE,0	;1=enabled (0)
	BITDEF RAMC,BURST,1	;1=enabled (0)
	BITDEF RAMC,WRAP,2	;1=wrap, 0=kill backwards bursts (0)
	BITDEF RAMC,RAMSIZE,3	;1=4MB, 0=1MB (RSIZE input to ramsey)
	BITDEF RAMC,RAMWIDTH,4	;1=4 bits wide, 0=1 1 bit wide (1)
	BITDEF RAMC,REFRESH0,5	;00-154  10-380
	BITDEF RAMC,REFRESH1,6	;01-238  11-off
	BITDEF RAMC,TEST,7



	ENDC	; A3000_HARDWARE_I
