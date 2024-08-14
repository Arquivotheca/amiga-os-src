        IFND    REALTIMEBASE_I
REALTIMEBASE_I      SET     1

;---------------------------------------------------------------------------

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND EXEC_NODES_I
    INCLUDE "exec/nodes.i"
    ENDC

    IFND EXEC_LISTS_I
    INCLUDE "exec/lists.i"
    ENDC

    IFND EXEC_LIBRARIES_I
    INCLUDE "exec/libraries.i"
    ENDC

    IFND EXEC_SEMAPHORES_I
    INCLUDE "exec/semaphores.i"
    ENDC

    IFND EXEC_INTERRUPTS_I
    INCLUDE "exec/interrupts.i"
    ENDC

    IFND LIBRARIES_REALTIME_I
    INCLUDE "realtime.i"
    ENDC

;---------------------------------------------------------------------------

   STRUCTURE ExtConductor,Conductor_SIZEOF
   LABEL ExtConductor_SIZEOF

ec_Stopped         equ LN_TYPE
ec_Players         equ cdt_Players
ec_ClockTime       equ cdt_ClockTime
ec_StartTime       equ cdt_StartTime
ec_ExternalTime    equ cdt_ExternalTime
ec_MaxExternalTime equ cdt_MaxExternalTime
ec_Metronome       equ cdt_Metronome
ec_Flags           equ cdt_Flags
ec_State           equ cdt_State

   BITDEF CONDSTOP,STOPPED,0   ; not running state
   BITDEF CONDSTOP,NOTICK,1    ; extsync & not gottick

;---------------------------------------------------------------------------

   STRUCTURE ExtPlayer,Player_SIZEOF
	ULONG ep_AlarmSigMask
   LABEL ExtPlayer_SIZEOF

ep_AlarmSigBit  equ pl_Reserved0
ep_Hook		equ pl_Hook
ep_Source       equ pl_Source
ep_Task         equ pl_Task
ep_MetricTime   equ pl_MetricTime
ep_AlarmTime    equ pl_AlarmTime
ep_UserData     equ pl_UserData
ep_PlayerID     equ pl_PlayerID
ep_Flags        equ pl_Flags

;---------------------------------------------------------------------------

   STRUCTURE RealTimeLib,RealTimeBase_SIZEOF
        APTR    rtl_UtilityBase
        ULONG   rtl_SegList
        APTR    rtl_SysBase

        ; this must follow rtb_SysBase!
        STRUCT  rtl_TimerSoftInt,IS_SIZE
        UBYTE   rtl_CIAChannel
        UBYTE   rtl_Pad1
        APTR    rtl_CIAResource
        APTR    rtl_CIACtrlReg
        STRUCT  rtl_Message,pmTime_SIZEOF

        STRUCT  rtl_CIAInt,IS_SIZE
        UWORD   rtl_ClockFreq
        STRUCT  rtl_CIAHandlerCode,6*2

	STRUCT  rtl_PlayerListLock,SS_SIZE
	UWORD   rtl_Pad3

    	STRUCT  rtl_ConductorList,MLH_SIZE
    	STRUCT  rtl_ConductorLock,SS_SIZE
   LABEL RealTimeLib_SIZEOF

rtl_TimeFreq equ rtb_Reserved1
rtl_Time     equ rtb_Time
rtl_TimeFrac equ rtb_TimeFrac
rtl_TickErr  equ rtb_TickErr

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
	xref _LVO\1
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
	xref _LVO\1
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; REALTIMEBASE_I
