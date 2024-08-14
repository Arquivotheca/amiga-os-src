	IFND	MPEGPLAYERBASE_I
MPEGPLAYERBASE_I	SET	1

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE "graphics/text.i"
	INCLUDE "graphics/view.i"
	INCLUDE "libraries/cdgprefs.i"

;---------------------------------------------------------------------------

MAXLINES  EQU 30
MAXPOINTS EQU 20

NUM_COLORS EQU 128
NUM_PLANES EQU 7

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
	UWORD  cp_Pad
   LABEL ColorPack_SIZEOF

   STRUCTURE Box,0
        STRUCT box_X,(2*MAXPOINTS)
        STRUCT box_Y,(2*MAXPOINTS)
   LABEL Box_SIZEOF

   STRUCTURE MPEGPlayerLib,LIB_SIZE
        UWORD  mp_Pad0
	ULONG  mp_SysLib
	ULONG  mp_GfxLib
	ULONG  mp_IntuitionLib
	ULONG  mp_DeBoxLib
	ULONG  mp_LowLevelLib
	ULONG  mp_VideoCDLib
	ULONG  mp_UtilityLib
	ULONG  mp_CDGLib
	ULONG  mp_SegList
	APTR   mp_ScreenDepth

	ULONG  mp_Screen
	ULONG  mp_Window
	APTR   mp_Pointer
	ULONG  mp_RenderRP

        STRUCT mp_BgColors,ColorPack_SIZEOF
        APTR   mp_BgBMInfo
        APTR   mp_BgBitMap

        STRUCT mp_DBBitMap,(2*4)
	STRUCT mp_DBRastPort,(rp_SIZEOF*2)
	STRUCT mp_DBBuffers,(2*4)
	APTR   mp_DBNotify
	ULONG  mp_DBCurrentBuffer

	APTR   mp_IconBMInfo
	APTR   mp_IconBitMap

	APTR   mp_ArrowBMInfo
	APTR   mp_ArrowBitMap

	STRUCT mp_Compact24,tf_SIZEOF
	STRUCT mp_Compact31,tf_SIZEOF

        APTR   mp_MainTask
	LONG   mp_RemovalSigBit
	LONG   mp_TimerSigBit
	APTR   mp_TimerReq

	APTR   mp_MPEGReq
	APTR   mp_MPEGReq2
	APTR   mp_AudioReq
	APTR   mp_AudioReq2
	APTR   mp_IOPort
	APTR   mp_Outstanding
	ULONG  mp_LowSector
	ULONG  mp_HighSector
	ULONG  mp_MPEGChannel

	APTR   mp_ThumbnailTask
	APTR   mp_ThumbnailTrack

	APTR   mp_PlayScreen
	APTR   mp_PlayWindow
        APTR   mp_DPFBitMap
        STRUCT mp_DPFRasInfo,ri_SIZEOF
        STRUCT mp_DPFRastPort,rp_SIZEOF
        ULONG  mp_LastTrack
        ULONG  mp_LastOptions
	ULONG  mp_LastTrackElapsed
	ULONG  mp_LastTrackRemaining
	ULONG  mp_LastDiskElapsed
	ULONG  mp_LastDiskRemaining
	ULONG  mp_IconCountdown
	ULONG  mp_TimeCountdown
        BOOL   mp_TimeDisplay
        UWORD  mp_Pad1

	APTR   mp_TrackBM
	ULONG  mp_OldLeft
	ULONG  mp_OldTop
	ULONG  mp_TrackCount
	BOOL   mp_AudioLayout
	WORD   mp_TrackBMXOffset
	WORD   mp_TrackBMYOffset
	UWORD  mp_Pad2

	APTR   mp_CreditsBM
	ULONG  mp_CreditsBMHeight
	LONG   mp_OldCreditsTop

        STRUCT mp_TextList,MLH_SIZE
        APTR   mp_TextPool
        APTR   mp_TopNode

	ULONG  mp_CurrentPage
        BOOL   mp_RandomModeChanged
        WORD   mp_Pad3

	APTR   mp_CDGPrefs
	APTR   mp_CDGSprites
	ULONG  mp_CDGState
	ULONG  mp_CDGChannel

	APTR   mp_SplinerTask
	STRUCT mp_Spliner_Context,16
	ULONG  mp_Spliner_seed
	APTR   mp_Spliner_ptr
	APTR   mp_Spliner_eptr
	STRUCT mp_Spliner_store,(Box_SIZEOF*MAXLINES)
	WORD   mp_Spliner_screenWidth
	WORD   mp_Spliner_screenHeight
	WORD   mp_Spliner_maxPoints
	WORD   mp_Spliner_numlines
	WORD   mp_Spliner_mdelta
	WORD   mp_Spliner_maxlines
	STRUCT mp_Spliner_dx,(2*MAXPOINTS)
	STRUCT mp_Spliner_dy,(2*MAXPOINTS)
	STRUCT mp_Spliner_ox,(2*MAXPOINTS)
	STRUCT mp_Spliner_oy,(2*MAXPOINTS)
	STRUCT mp_Spliner_nx,(2*MAXPOINTS)
	STRUCT mp_Spliner_ny,(2*MAXPOINTS)
	WORD   mp_Spliner_dr
	WORD   mp_Spliner_dg
	WORD   mp_Spliner_db
	WORD   mp_Spliner_nr
	WORD   mp_Spliner_ng
	WORD   mp_Spliner_nb
	WORD   mp_Spliner_closed
	STRUCT mp_Spliner_realfunc,14
   LABEL MPEGPlayerLib_SIZEOF

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; MPEGPLAYERBASE_I
