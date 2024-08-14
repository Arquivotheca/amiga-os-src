	IFND	JANUS_SETUPSIG_I
JANUS_SETUPSIG_I    EQU 1
;*************************************************************************
;
; setupsig.i -- data structure for SetupJanusSig() routine
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
;
;*************************************************************************


	IFND	EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC

	IFND	EXEC_INTERRUPTS_I
	INCLUDE 'exec/interrupts.i'
	ENDC



 STRUCTURE SetupSig,IS_SIZE
    APTR	ss_TaskPtr
    ULONG	ss_SigMask
    APTR	ss_ParamPtr
    ULONG	ss_ParamSize
    UWORD	ss_JanusIntNum
    LABEL	SetupSig_SIZEOF

	ENDC


