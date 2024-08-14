**
**      $Id: cddata.i,v 36.60 92/06/16 10:43:02 darren Exp $
**
**      console device private data definitions
**
**      (C) Copyright 1985,1989,1991,1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/tasks.i"
;       INCLUDE "exec/ports.i"                  ; included in conunit.i
        INCLUDE "exec/interrupts.i"
;       INCLUDE "exec/libraries.i"              ; included in macros.i
;       INCLUDE "exec/io.i"                     ; included in console.i
        INCLUDE "exec/semaphores.i"
        INCLUDE "devices/keymap.i"
        INCLUDE "devices/timer.i"
        INCLUDE "devices/inputevent.i"
        INCLUDE "graphics/rastport.i"
        INCLUDE "console.i"
        INCLUDE "conunit.i"
        INCLUDE "conmap.i"
        INCLUDE "macros.i"


CDERR_NOUNIT    EQU     1       ; not a real unit (OpenDevice w/ -1 unit)
CDERR_NOMAP     EQU     2       ; no character map available for this unit
CDERR_BADMAP    EQU     3       ; map not acquired via CD_OBTAINMAP

**      V36 and above

******* SnipHookMsg struct - use with Add/Remove snip hook functions (V36)

 STRUCTURE      SnipHookMsg,0
    ULONG   shm_Type            ; zero for this structure format
    ULONG   shm_SnipLen         ; length of snip data (not including null)
    APTR    shm_SnipData        ; null terminated snip data


* #define       CDERR_NOUNIT    1       /* not a real unit (OpenDevice w/ -1 unit) */
* #define       CDERR_NOMAP     2       /* no character map available for this unit */
* #define       CDERR_BADMAP    3       /* map not acquired via CD_OBTAINMAP */
*
* struct SnipHookMsg {
*    ULONG   shm_Type;          /* zero for this structure format */
*    ULONG   shm_SnipLen;       /* length of snip data (not including null) */
*    UBYTE  *shm_SnipData;      /* null terminated snip data */
* };
*

*------ constants ----------------------------------------------------
PM_ASM          EQU     -31             ; private AutoScroll mode is >1
PM_AWM          EQU     -47             ; private AutoWrap mode is ?7

CD_HANDLERPRI   EQU     0
CD_CONVBUFSIZE  EQU     255             ; used by CDInputHandler

CD_STKSIZE      EQU     $1000
CD_TASKPRI      EQU     5

CDSIGB_NEWACTIVE EQU    31              ; signal that new window is active
CDSIGB_REFRESH  EQU     30              ; signal to refresh window(s)
CDSIGB_SELECT   EQU     29              ; text selection
CDSIGB_TICK     EQU     28              ; text selection tick

CDSIGF_NEWACTIVE EQU    $80000000
CDSIGF_REFRESH  EQU     $40000000
CDSIGF_SELECT   EQU     $20000000
CDSIGF_TICK     EQU     $10000000

CDSIG_HARDCODED EQU     $f0000000

CD_TEVENTSECS   EQU     0
CD_TEVENTMICRO  EQU     100000          ; 10 times/sec

READBSIZE       EQU     256             ; read buffer size
PICBSIZE        EQU     8               ; intermediate character buffer size
PNPBSIZE        EQU     25*2            ; numeric parameter buffer size

*------ IO_FLAGS -----------------------------------------------------
        BITDEF  IO,SPECIAL,3    ; set if aRAV sequence, else cleared
        BITDEF  IO,QUEUED,4     ; command is queued to be performed
        BITDEF  IO,DONE,5       ; command is done
        BITDEF  IO,SERVICING,6  ; command is being actively performed
        BITDEF  IO,CURRENT,7    ; command is being performed (sign bit)

*------ cd_SelectionSnip  -----------------------------------------------------
 STRUCTURE  SelectionSnip,0     ; AllocVec'd
        UWORD   snip_Access     ; FreeVec when negative
        UWORD   snip_Length     ; valid Data
        LABEL   snip_Data       ; data follows...
        
*------ cd_Flags -----------------------------------------------------
        BITDEF  CD,CLIPINSTALLED,0      ; the cu_ClipRegion has been installed

*------ cd_SelectFlags -----------------------------------------------

        BITDEF  CDS,CIRCLING,2  ; selecting outside the display bounds
        BITDEF  CDS,DRAGGING,3  ; still determining whether to select
        BITDEF  CDS,SELECTING,4 ; already have selection anchor
        BITDEF  CDS,TICKING,5   ; the timer for selection polling is ticking
        BITDEF  CDS,EXTENDED,6  ; the select button is extended
        BITDEF  CDS,SELECTDOWN,7 ; the select button is down

CDS_SELECTMASK  EQU     $20     ; preserve CDS_TICKING

*------ cd_InputFlags -----------------------------------------------

        BITDEF  CDI,COPY,0      ; user selected RIGHT AMIGA C

*------ cd_CmdBytes --------------------------------------------------
*
*   if <  0, this command executes immediately, ignoring STOPPED.
*   if >= 0, this command is queued on the list at
*               *IO_UNIT + ((n-1)*MP_SIZE)
*       if it cannot be satisfied quickly
*
 STRUCTURE  ConsoleDevice,LIB_SIZE
        UWORD   cd_NumCommands          ; the number of commands supported
        APTR    cd_Segment              ; A0 when initialized
        APTR    cd_CmdVectors           ; command table for device commands
        APTR    cd_CmdBytes             ; bytes describing which command queue
        APTR    cd_ExecLib              ; exec library base
        APTR    cd_KeymapLib            ; keymap library base
        APTR    cd_GraphicsLib          ; graphics library base
        APTR    cd_LayersLib            ; layers library base
        APTR    cd_IntuitionLib         ; intuition library base
        APTR    cd_Active               ; active unit, if any
        APTR    cd_ActiveWindow         ; active window
        APTR    cd_SelectedUnit         ; unit w/ text selected in it, if any
        ULONG   cd_WindowCount          ; number of windows opened as consoles

        STRUCT  cd_USemaphore,SS_SIZE
        STRUCT  cd_UHead,LH_SIZE        ; linked list of open units

                                        ; LONG WORD ALIGNED AGAIN

        UWORD   cd_SelectDownMX         ; mouse location of initial select down
        UWORD   cd_SelectDownMY         ;
        UWORD   cd_SelectDownCPX        ; CP location of initial select down
        UWORD   cd_SelectDownCPY        ;
        UWORD   cd_SelectedAnchorX      ; location of selection start
        UWORD   cd_SelectedAnchorY      ;
        UWORD   cd_SelectedTailX        ; location of selection end
        UWORD   cd_SelectedTailY        ;

        STRUCT  cd_RPSemaphore,SS_SIZE
        STRUCT  cd_SelectionSemaphore,SS_SIZE

                                        ; LONG WORD ALIGNED AGAIN

        STRUCT  cd_SelectionHooks,MLH_SIZE
                                        ;
        APTR    cd_SelectionSnip        ; selection itself
        STRUCT  cd_HandlerIOR,IOSTD_SIZE ; input handler request
        STRUCT  cd_HandlerIS,IS_SIZE    ; input handler node

        UBYTE   cd_SelectFlags          ; flags re: selecting status
        UBYTE   cd_Flags

                                        ; LONG WORD ALIGNED AGAIN


        APTR    cd_PrevClipRegion       ; previous clip region for layer for RP
        STRUCT  cd_AreaPtrn,4
        STRUCT  cd_TmpRas,tr_SIZEOF     ;                   [8 bytes]
        STRUCT  cd_RastPort,rp_SIZEOF   ;                   [100 bytes]

        STRUCT  cd_Stk,CD_STKSIZE       ;   and stack space [1024 bytes]

        STRUCT  cd_TC,TC_SIZE           ; task space [92 bytes]

        STRUCT  cd_TPort,MP_SIZE        ;
        STRUCT  cd_TIOR,IOTV_SIZE       ;   and request block
        STRUCT  cd_EVSemaphore,SS_SIZE  ; Event list handling semaphore
        UWORD   cd_MouseX               ; temporary hold of window values
        UWORD   cd_MouseY               ; X/Y can be moved here with a
                                        ; move.l instr (atomic)

	UWORD	cd_MouseQual		; Mouse event qualifier
        UBYTE   cd_InputFlags           ; flags from inputhandler
        UBYTE   cd_Hooks                ; Count of hooks installed


        ALIGNWORD
        LABEL   cd_SIZEOF

*------ du_Flags -----------------------------------------------------
        BITDEF  DU,DISPATCH,0   ; output posted to the unit
        BITDEF  DU,STOPPED,7    ; commands are not to be performed

*------ DeviceUnit ---------------------------------------------------
*       array of current & pending command queues, a command is
*       removed after it is done, it is ACTIVE while in progress unless
*       it is quick.  The queues are message ports.
du_Flags        EQU     LN_PRI  ; various unit flags


*------ cu_Flags -----------------------------------------------------
        BITDEF  CU,NEWACTIVE,0  ; cursor activation or deactivation pending
        BITDEF  CU,RESIZE,1     ; resize pending
        BITDEF  CU,REFRESH,2    ; refresh pending
        BITDEF  CU,TOOSMALL,3   ; window is too small to render text
        BITDEF  CU,SELECTED,4   ; select hilighted
        BITDEF  CU,KERNLESS,5   ; font does not lie outside cell
        BITDEF  CU,TABBED,6     ; tab just occurred
        BITDEF  CU,IMPLICITNL,7 ; implicit NL just occurred (sign bit)

CU_RESET0MASK   EQU     $01     ; only preserve NEWACTIVE

*------ cu_CursorFlags -----------------------------------------------

        BITDEF  CU,CURSON,2     ; cursor is logically on
        BITDEF  CU,CURSSELECT,3 ; cursor is within a selection
        BITDEF  CU,CURSACTIVE,4 ; cursor is active, or no ghosting allowed

**
** Disable cursor kludge code for V39 ROM to save ROM space -- Darren 
** Do NOT reuse these flags without modifying cursor.asm code, or
** you will get the wrong cursor patterns
**
**      BITDEF  CU,CURSNDRAW,6  ; known that cursor drawing is *not* allowed
**      BITDEF  CU,CURSQDRAW,7  ; not known that cursor drawing *is* allowed
**                              ; (sign bit)


*------ cu_FixedFlags ------------------------------------------------
        BITDEF  CUF,FIXEDLL,0   ; windows have a specified line length
        BITDEF  CUF,FIXEDPL,1   ; windows have a specified page length
        BITDEF  CUF,FIXEDLO,2   ; windows have a specified x offset 
        BITDEF  CUF,FIXEDTO,3   ; windows have a specified y offset

        BITDEF  CUF,FIXEDBG,7   ; background color is explicit (sign bit)


*------ cu_PState parse states ---------------------------------------
cu_PSVANILLA   EQU 0       ;cannot be non-zero
cu_PSESC       EQU 1
cu_PSESCI      EQU 2
cu_PSCSI       EQU 3
cu_PSCSII      EQU 4

*------ cu_State flags - Extra flags for state info ------------------

        BITDEF  CU,FIRSTTIME,0          ;first time opened
        BITDEF  CU,ISACTIVE,1           ;This window is active
	BITDEF	CU,CTRLG,2		;CTRL-G pending
	BITDEF	CU,BORDERFILL,3		;BorderFill pending

*------ Flags used in the input list of events -----------------------

        BITDEF  IP,ISACTIVE,0           ;is this window active at this time?
        BITDEF  IP,REDRAW,1             ;this window has been resized/damaged
        BITDEF  IP,MAKEACTIVE,2         ;this window has been activated
        BITDEF  IP,MAKEINACTIVE,3       ;this window is no longer active

 STRUCTURE  ConsoleUnit,ConUnit_SIZEOF
	ALIGNLONG
        ULONG   cu_DevUnit              ; from OpenDevice register d0
        ;------ optional output data
        STRUCT  cu_CM,ConsoleMap_SIZEOF
        ;------ input data
        UWORD   cu_ReadLastIn           ; head
        UWORD   cu_ReadLastOut          ; tail
        STRUCT  cu_ReadData,READBSIZE   ; circular buffer for read characters

        ;------ initialized by WriteReset
        APTR    cu_GLHandler

        WORD    cu_PICNext
        STRUCT  cu_PICData,PICBSIZE     ; intermediate character buffer
        WORD    cu_PNPCurr
        STRUCT  cu_PNPData,PNPBSIZE     ; numeric parameter buffer

        UWORD   cu_PState

        ;------ clip region for this window
        APTR    cu_ClipRegion
        STRUCT  cu_ClipRect,ra_SIZEOF

        ; and all the rest
        ULONG   cu_CursorPattern        ; current cursor pattern
        UWORD   cu_FixedXMax            ; when CUF._FIXEDLL
        UWORD   cu_FixedYMax            ; when CUF._FIXEDPL
        UWORD   cu_DisplayXL            ; position after last displayed char
        UWORD   cu_DisplayYL            ;
        BYTE    cu_RPNestCnt		; semaphore nest count
        BYTE    cu_CDNestCnt		; same
        UBYTE   cu_CursorFlags
        UBYTE   cu_Flags
        UBYTE   cu_FixedFlags		; e.g., Fixed line length
        UBYTE   cu_BgColor		; global background color
        UBYTE   cu_States               ; extra flags for state info
        UBYTE   cu_SpecialModes         ; Special modes field (flags on open)
	UBYTE	cu_ConcealMask		; Flag/Mask for conceal mode
	UBYTE	cu_ScrollMask		; cache scroll mask
	UBYTE	cu_MinMask		; minimum bitplane scroll mask
	UBYTE	cu_CursorMask		; ClearRaster Mask
	UBYTE	cu_Depth		; bitmap depth
	UBYTE	cu_ConcealFG		; FG pen hold when concealed
	UBYTE	cu_ConcealBG		; BG pen hold when concealed
	UBYTE	cu_ConcealDRMD		; DRaw MoDe when concealed
	UBYTE	cu_NormFG		; Normal FG color
	UBYTE	cu_NormBG		; Normal BG color
	UBYTE	cu_NormDMode		; Normal Draw Mode
	UBYTE	cu_NormStyle		; flag; Normal style set
	UBYTE	cu_ClearDrMd		; Draw Mode at ClearDisplay time
	ALIGNWORD
	UWORD	cu_NormAlgoStyle	; & TxFlags
	UWORD	cu_FullXRExtant		; XRExtant + up to border
	UWORD	cu_FullYRExtant		; YRExtant + up to border
        UWORD   cu_WWidth               ; window width after resize
        UWORD   cu_WHeight              ; window height after resize
        UBYTE   cu_IHead                ; head of input list (task)
        UBYTE   cu_ITail                ; tail of input list (inputhandler)
        STRUCT  cu_IList,256            ; list of input events (both)

        ALIGNWORD                       ; for InitStruct bug

        LABEL   cu_SIZE
