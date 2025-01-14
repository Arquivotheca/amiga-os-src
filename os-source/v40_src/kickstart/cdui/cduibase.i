	IFND	CDUIBASE_I
CDUIBASE_I	SET	1

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "utility/hooks.i"

	INCLUDE "animate.i"

;---------------------------------------------------------------------------

NUM_PLANES           equ 6
NUM_PLAYFIELD_COLORS equ 64
NUM_SPRITE_COLORS    equ 16
NUM_COLORS           equ (NUM_PLAYFIELD_COLORS+NUM_SPRITE_COLORS)

   STRUCTURE RGBEntry,0
	ULONG Red
	ULONG Green
	ULONG Blue
   LABEL RGBEntry_SIZEOF

   STRUCTURE ColorPack,0
	UWORD  cp_NumColors
	UWORD  cp_FirstColor
	STRUCT cp_Colors,(RGBEntry_SIZEOF*NUM_COLORS)
	UWORD  cp_EndMarker
   LABEL ColorPack_SIZEOF

;---------------------------------------------------------------------------

   STRUCTURE ColorCycle,0
	ULONG cy_Effect
	WORD  cy_Phase
	UWORD cy_Halted
	UWORD cy_NumColors
	UWORD cy_ColorBase
	UWORD cy_CountDown
	UWORD cy_Termination
   LABEL ColorCycle_SIZEOF

NUM_CYCLES equ 10

;---------------------------------------------------------------------------

   STRUCTURE CDUILib,LIB_SIZE
        UWORD  cb_Pad0
	ULONG  cb_SysLib
	ULONG  cb_GfxLib
	ULONG  cb_IntuitionLib
	ULONG  cb_DeBoxLib
	ULONG  cb_LowLevelLib
	ULONG  cb_NVLib
	ULONG  cb_SegList

	ULONG  cb_AllocPatch

        ULONG  cb_CDUITask
        ULONG  cb_CDUIPort

	ULONG  cb_Screen
	ULONG  cb_Window
	ULONG  cb_RastPort
	ULONG  cb_BitMap
	ULONG  cb_ViewPort
	ULONG  cb_Pointer
	LONG   cb_XOffset
	LONG   cb_YOffset
	STRUCT cb_Sprites,(72*4)

	ULONG  cb_LowCostScreen
	ULONG  cb_LowCostBM
	ULONG  cb_LowCostBMInfo
	ULONG  cb_LowCostColors

	STRUCT cb_Range,(cor_SIZEOF*160)
	STRUCT cb_Colors,ColorPack_SIZEOF
	UWORD  cb_Pad1

	ULONG  cb_ScrollerTask
	ULONG  cb_FaderTask

	ULONG  cb_SpinnerTask
	ULONG  cb_WaverTask
	ULONG  cb_SaveBM
	ULONG  cb_WorkBM
	ULONG  cb_CurrentFlagBM
	STRUCT cb_FlagLock,SS_SIZE
	BYTE   cb_FlagToWave
	BYTE   cb_DoBetterFlag

        ULONG  cb_BackgroundBM
        ULONG  cb_FanfareTask
        ULONG  cb_CyclerTask
        ULONG  cb_FlipperTask
	BOOL   cb_DidAnimation
	WORD   cb_CDYOffset
	UWORD  cb_AuroraIndex
	UWORD  cb_Pad3
        ULONG  cb_BgSaveBM
	STRUCT cb_Cycles,(NUM_CYCLES*ColorCycle_SIZEOF)
	ULONG  cb_BitMapColors
	ULONG  cb_AuroraColors0
	ULONG  cb_AuroraColors1
	ULONG  cb_AuroraBM0
	ULONG  cb_AuroraBM1

	ULONG  cb_CreditRP
	ULONG  cb_CreditBM
	ULONG  cb_CreditVP
   LABEL CDUILib_SIZEOF

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; CDUIBASE_I
