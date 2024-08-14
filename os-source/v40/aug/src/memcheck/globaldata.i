;*****************************************************************************/
;
; globaldata.i
; private information used by MemCheck
;
;*****************************************************************************/

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND INTUITION_SCREENS_I
    INCLUDE "intuition/screens.i"
    ENDC

    INCLUDE "memcheckheader.i"

;*****************************************************************************/

; exec.library  LVO $ff3a -198  AllocMem(byteSize,requirements)(d0/d1)
; exec.library  LVO $ff34 -204  AllocAbs(byteSize,location)(d0/a1)
; exec.library  LVO $ff2e -210  FreeMem(memoryBlock,byteSize)(a1,d0)
; exec.library  LVO $ff28 -216  AvailMem(requirements)(d1)
; exec.library  LVO $fd54 -684  AllocVec(byteSize,requirements)(d0/d1)
; exec.library  LVO $fd4e -690  FreeVec(memoryBlock)(a1)

;*****************************************************************************/

LVO_ALLOCMEM	equ	-198
LVO_ALLOCABS	equ	-204
LVO_FREEMEM	equ	-210
LVO_AVAILMEM	equ	-216
LVO_ALLOCVEC	equ	-684
LVO_FREEVEC	equ	-690

;*****************************************************************************/

    BITDEF	CHECK,FREE,0
    BITDEF	CHECK,ALLOC,1

;*****************************************************************************/

ACALLER		equ	0
CCALLER		equ	2

;*****************************************************************************/

BIGBUFSZ	equ	20000
SBUFSZ		equ	80

;*****************************************************************************/

OPT_PRIORITY	equ	0
OPT_POPKEY	equ	1
OPT_POPUP	equ	2
OPT_PUBSCREEN	equ	3
OPT_STACKLINES	equ	4
OPT_COMPAT	equ	5
NUM_OPTS	equ	6

;*****************************************************************************/

GID_CONTEXT	equ	0
GID_ANCHOR	equ	1
GID_SET_FREE	equ	2
GID_SET_ALLOC	equ	3
GID_SET_ALL	equ	4
GID_CHECK_FREE	equ	5
GID_CHECK_ALLOC	equ	6
GID_CHECK_ALL	equ	7
GID_MAX		equ	8

;*****************************************************************************/

MMC_HIDE	equ	500
MMC_QUIT	equ	501

;*****************************************************************************/

 STRUCTURE GlobalData,0
    STRUCT	gd_MemCheckInfo,mci_SIZE	; Embedded MemCheckInfo

    ; Shared Libraries
    APTR	gd_SysBase
    APTR	gd_DOSBase
    APTR	gd_UtilityBase
    APTR	gd_IntuitionBase
    APTR	gd_GadToolsBase
    APTR	gd_CxBase
    APTR	gd_LayersBase
    ULONG	gd_LStart
    ULONG	gd_LEnd

    ; Control information
    APTR	gd_Process
    LONG	gd_ErrorLevel
    LONG	gd_ErrorNumber
    LONG	gd_ErrorString
    LONG	gd_Going
    ULONG	gd_Flags
    APTR	gd_RDArgs
    STRUCT	gd_Options,(4*NUM_OPTS)

    ; Functions
    APTR	gd_AllocMem
    APTR	gd_AllocAbs
    APTR	gd_FreeMem
    APTR	gd_AvailMem
    APTR	gd_AllocVec
    APTR	gd_FreeVec

    ; Memory tracking
    APTR	gd_MemHeader

    ; Application data
    APTR	gd_MsgPort;
    STRUCT	gd_WorkBuffer,(BIGBUFSZ)
    STRUCT	gd_ReadBuffer,(SBUFSZ)

    ; GUI information
    STRUCT	gd_ScreenNameBuffer,(MAXPUBSCREENNAME+1)
    APTR	gd_ScreenName
    APTR	gd_Screen
    APTR	gd_DrInfo
    APTR	gd_VI
    APTR	gd_Window
    APTR	gd_Menu
    STRUCT	gd_Gads,(4*GID_MAX)
    STRUCT	gd_WindowTitle,(80)
    APTR	gd_Memory

    ; Commodity information
    APTR	gd_CxPort
    APTR	gd_CxObj
    APTR	gd_CxFilter
    APTR	gd_CxPopKey
 LABEL		gd_SIZE

;*****************************************************************************/

    BITDEF	GD,CLOSEWINDOW,0
    ; Indicate that the window should close

    BITDEF	GD,OPENWINDOW,1
    ; Indicate that the window should open

    BITDEF	GD,BUSY,2
    ; Show that we are busy doing something

    BITDEF	GD,CLOSING,3
    ; Show that we want to go home to bed

    BITDEF	GD,ACTIVE,4
    ; Are we active?

    BITDEF	GD,COMPATIBLE,5
    ; Supposed to be compatible with MungWall?
