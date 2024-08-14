;*************************************************************************
;
; jddata.i -- the software data structures for the jdisk device
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************

	IFND	EXEC_LIBRARIES_I
	INCLUDE "exec/libraries.i"
	ENDC
	IFND	EXEC_INTERRUPTS_I
	INCLUDE "exec/interrupts.i"
	ENDC


;------ extensions to IO_FLAGS -------------------------------------------
    BITDEF  IO,CURRENT,4
    BITDEF  IO,DONE,5
    BITDEF  IO,BIOSERR,7

;------ the user should never see this, but here it is -------------------
JD_WRONGLENGTH	EQU $30


;------ device structure -------------------------------------------------
 STRUCTURE  JDiskDevice,LIB_SIZE
    APTR    jd_Segment		    ; the dos segment for this device
    APTR    jd_ExecBase
    APTR    jd_JanusBase
    APTR    jd_TrackBuffer
    ULONG   jd_TBSize		    ; the size of TrackBuffer
    APTR    jd_AmigaDskReq
    STRUCT  jd_CmdQueue,LH_SIZE     ; the command queue
    STRUCT  jd_CmdTerm,IS_SIZE	    ; cmd for JSERV_HARDDISK termination
    ULONG   jd_RequestedCount	    ; the length asked of the 8088 code
    LABEL   JDiskDevice_SIZEOF

