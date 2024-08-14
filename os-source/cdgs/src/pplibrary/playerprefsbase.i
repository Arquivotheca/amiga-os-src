	
 INCLUDE "exec/libraries.i"
 INCLUDE "exec/semaphores.i"
 STRUCT PlayerPrefsBase,LIB_SIZE
 	APTR			ppb_DataArea
	LONG			ppb_SegList
	LONG			ppb_Flags
	APTR			ppb_ExecBase
	APTR			ppb_DeBoxBase
	APTR			ppb_GfxBase
	APTR			ppb_IntuitionBase
	UBYTE			ppb_Blanked
	UBYTE			ppb_TitleState
	UBYTE			ppb_TitleStatus
	UBYTE			ppb_TitleAns
	APTR			ppb_TMData
	APTR			ppb_TitleSigTask
	APTR			ppb_TitlePort
	STRUCT			ppb_CDTVVersionStr,8
	STRUCT			ppb_LibLock,SS_SIZE
	
	STRUCT			ppb_CRLock,SS_SIZE
	APTR			ppb_CRBm
	LONG			ppb_CROpenCnt
	LABEL			PlayerPrefsBase_SIZE
