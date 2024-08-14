**
** $Source: HOG:Other/networking/sana2/src/slip/RCS/slip_device.i,v $
** $State: Exp $
** $Revision: 38.1 $
** $Date: 94/02/17 14:17:59 $
** $Author: kcd $
**
** SANA-II SLIP Device driver assembly include file
**
** (C) Copyright 1992 Commodore-Amiga, Inc.
**

    include "exec/types.i"
    include "exec/devices.i"
    include "exec/ports.i"
    include "exec/semaphores.i"


**
** SLIP Driver Device Base definition
**

SD_MAXUNITS	EQU	8

 STRUCTURE SLIPDev,LIB_SIZE
    UBYTE   sd_Flags
    UBYTE   sd_Pad1
    ULONG   sd_SysLib
    ULONG   sd_DosLib
    STRUCT  sd_DosTag,10*4
    ULONG   sd_SegList
    STRUCT  sd_Units,SD_MAXUNITS*4
    STRUCT  sd_Lock,SS_SIZE
    STRUCT  sd_Startup,MN_SIZE+8
    STRUCT  sd_DummyPFHook,h_SIZEOF

    LABEL SLIPDev_Sizeof

**
** Device Name Macro
**

SLIPDEVNAME	MACRO
		DC.B	'slip.device',0
		ENDM

**
** Handy system call macro
**

jsrlib	MACRO
	XREF	_LVO\1
	jsr	_LVO\1(a6)
	ENDM

