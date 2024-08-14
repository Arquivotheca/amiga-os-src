 
* ***************************************************************************
* 
* The primary include file for the video work of the Zaphod Project
*
* Copyright (C) 1986, Commodore-Amiga, Inc.
* 
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY   Name      Description
* ------------   --------------   --------------------------------------------
* 22 Feb 86   =RJ Mical=   Created this file
* 15 Dec 86     RJ >:-{)*       Added XREF for Manx C
*
* **************************************************************************/


* Comment out the following lines if not compiled by Manx Aztec
;MANX   EQU   1
;AZTEC   EQU   1


;   NOLIST

   IFND   EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

*   INCLUDE "exec/memory.i
*   INCLUDE "exec/tasks.i"
*   INCLUDE "libraries/dos.i"
*   INCLUDE "graphics/regions.h"
*   INCLUDE "exec/libraries.i"

   INCLUDE "hardware/custom.i"

   INCLUDE "graphics/gfx.i"

   IFND   AZTEC 
   INCLUDE "asmsupp.i"
   ENDC
   IFD   AZTEC
   INCLUDE "assmsupp.i"
   ENDC

   INCLUDE "intuition/intuition.i"

   LIST


* === System Macros ===================================================== */
            
INFOLEVEL   EQU   100

LINKGFX MACRO   
   LINKSYS \1,_GfxBase    
   ENDM

LINKINT MACRO
   LINKSYS \1,_IntuitionBase
   ENDM


* === MasterFlags Definitions =========================================== *
PC_HAS_MOUSE   EQU   $0001
PC_HAS_MOUSE_B   EQU   0



* === System Definitions ================================================= *

   XLIB   BltBitMapRastPort
   XLIB   Debug
   XLIB   DisownBlitter
   XLIB   Move
   XLIB   OwnBlitter
   XLIB   RectFill
   XLIB   SetAPen
   XLIB   SetBPen
   XLIB   SetDrMd
   XLIB   Text
   XLIB   WaitBlit



* === Cursor Structure Definitions ======================================= *

* These are the definitions for the CursorFlags variable *
CURSOR_ON       EQU $0001
CURSOR_MOVED       EQU $0002
CURSOR_INACTIVE     EQU $0004




* === Locking Protocol Definitions ======================================= */
    STRUCTURE Lock,0

   APTR LockingTask    ; Which task has the current lock *
   SHORT LockCount     ; How many times the task has locked *
   SHORT LockWanted    ; How many tasks are waiting for this lock *

    LABEL Lock_SIZEOF



* === Border Structure Definitions ======================================= *

    STRUCTURE BorderKontrol,0

   SHORT Left
   SHORT Top
   SHORT Right
   SHORT Bottom 
   APTR Title 
   APTR GadgetList

    LABEL BorderKontrol_SIZEOF


* These are the possible selectType args passed to BorderPatrol() *
BORDER_TOGGLE   EQU 1
BORDER_ON   EQU 2
BORDER_OFF   EQU 3




* === Abort() Definitions =============================================== *
ABORT_NO_SIGNALS   EQU 1
ABORT_INCOMPLETESYSTEM   EQU 2
ABORT_NO_MEMORY    EQU 3
ABORT_NO_LIBRARY   EQU 4
ABORT_NO_NEWREGION   EQU 5
ABORT_NO_INPUTPORT   EQU 6
ABORT_NO_PCFONT    EQU 7
ABORT_NO_TIMERPORT   EQU 8
ABORT_BAD_UNLOCK_TASK   EQU 9




* === Data Register Definitions ========================================= *
*IOREG_OFFSET           EQU (IoAccessOffset+IoRegOffset)

*COLOR_CRT_OFFSET        EQU (IOREG_OFFSET+jio_ColorData)
*MONO_CRT_OFFSET        EQU (IOREG_OFFSET+jio_MonoData)
*COLOR_CONTROL_OFFSET        EQU (IOREG_OFFSET+jio_ColorControlReg)
*COLOR_SELECT_OFFSET        EQU (IOREG_OFFSET+jio_ColorSelectReg)
*COLOR_STATUS_OFFSET        EQU (IOREG_OFFSET+jio_ColorStatusReg)
*KEYBOARD_OFFSET        EQU (IOREG_OFFSET+jio_KeyboardData)
*PCINTGEN_OFFSET        EQU (IOREG_OFFSET+jio_PcIntGen)


* These are the byte offsets from the base address to the register *
CRT_R0      EQU 0
CRT_R1      EQU 2
CRT_R2      EQU 4
CRT_R3      EQU 6
CRT_R4      EQU 8
CRT_R5      EQU 10
CRT_R6      EQU 12
CRT_R7      EQU 14
CRT_R8      EQU 16
CRT_R9      EQU 18
CRT_R10    EQU 20
CRT_R11    EQU 22
CRT_R12    EQU 24
CRT_R13    EQU 26
CRT_R14    EQU 28
CRT_R15    EQU 30
CRT_IOBLOCKSIZE EQU 32       ; This *must* be an even number.

* These are the masks of the register information that interests us *
CRT_R1MASK   EQU $FF
CRT_R6MASK   EQU $7F
CRT_R8MASK   EQU $F3
CRT_R9MASK   EQU $1F
CRT_R10MASK   EQU $7F
CRT_R11MASK   EQU $1F
CRT_R12MASK   EQU $3F
CRT_R13MASK   EQU $FF
CRT_R14MASK   EQU $3F
CRT_R15MASK   EQU $FF

* Here are some individual masks and definitions of the values found in
* the various registers
VIDEO_MASK       EQU $03
VIDEO_INTERLACE     EQU $01
VIDEO_COMPRESS       EQU $03

CURSOR_BLINK_MASK   EQU $60
CURSOR_STEADY       EQU $00
CURSOR_OFF       EQU $20
CURSOR_BLINK16       EQU $40
CURSOR_BLINK32       EQU $60

CURSOR_START_MASK   EQU $1F




* === KeyFlags Definitions ============================================== *
KEY_ALTKEY       EQU $0001
KEY_AMIGA       EQU $0002
KEY_AMIGAPENDING    EQU $0004

*ALTKEYS        EQU (ALTLEFT|ALTRIGHT)

* The key output buffer size should be a power of two, to allow fast
* mask operation on the buffer counter variables (avoids a division
* every time a keystroke is queued and another division every time a
* keystroke is sent from the buffer to the PC).
* The PC's buffer is 16 wide.  I'm always typing to the end of the
* darned buffer.  How big shall our buffer be?
KEYBUFFER_SIZE       EQU 256
KEYBUFFER_MASK       EQU (KEYBUFFER_SIZE-1)




* === Color & Mono Display Modes Definitions ============================= *
MEDIUM_RES   EQU  $0001  ; 320 columns. Not set = 640 column mode
TEXT_MODE   EQU  $0002  ; Text mode.  Not set = then graphics mode
unused0004   EQU  $0004
BLINK_TEXT   EQU  $0008  ; 1 = text blinks.   0 = 16-color background
PALETTE_1   EQU  $0010  ; For medium-res graphics palette 0 or 1
OPEN_SCREEN   EQU  $0020  ; 1 = Wants own screen.  0 = Use Workbench
unused0040   EQU  $0040  ; 1 = Single-plane Monochrome display
unused0080   EQU  $0080
BORDER_VISIBLE   EQU  $0100
GOT_NEWSIZE   EQU  $0200
unused0400   EQU  $0400
COLOR_DISPLAY   EQU  $0800  ; 1 = color display.  0 = mono
VERBOSE    EQU  $1000  ; For verbose Zaphod, else shhhhh
unused2000   EQU  $2000
unused4000   EQU  $4000
unused8000   EQU  $8000



COLOR_LOW_INTENSITY   EQU 9
COLOR_HIGH_INTENSITY   EQU 15
MIN_RED       EQU (3<<8)
MIN_GREEN      EQU (3<<4)
MIN_BLUE      EQU (3<<0)
LOW_RED       EQU (COLOR_LOW_INTENSITY<<8)
LOW_GREEN      EQU (COLOR_LOW_INTENSITY<<4)
LOW_BLUE      EQU (COLOR_LOW_INTENSITY<<0)
HIGH_RED      EQU (COLOR_HIGH_INTENSITY<<8)
HIGH_GREEN      EQU (COLOR_HIGH_INTENSITY<<4)
HIGH_BLUE      EQU (COLOR_HIGH_INTENSITY<<0)

HORIZ_GADGET     EQU 1
VERT_GADGET     EQU 2


*DISPLAY_IDCMP_FLAGS EQU (NEWSIZE|REFRESHWINDOW|MOUSEBUTTONS\
*                |GADGETDOWN|GADGETUP|MENUPICK\
*                |CLOSEWINDOW|RAWKEY|ACTIVEWINDOW\
*                |INACTIVEWINDOW) 



   

* === Color & Mono Display Modes2 Definitions ============================= *
PC_MOUSE   EQU   $0001   ; Set when mouse is assigned to PC
;PC_MOUSE   EQU   0x0001   ; Set when mouse is assigned to PC



* === Text Routines Definitions ======================================= *
* These are here mostly as placeholders.  If any of these definitions
* needs to be changed, the text routines may have to be reworked since
* some of the shortcuts depend on these values.  See the comments in
* the text routines for more details.
BUFFER_WIDTH   EQU 80       ; For output text, a multiple of 20
CHAR_HEIGHT   EQU 8       ; To change this one won't be too bad
CHAR_WIDTH   EQU 8       ; To change this would be horrendous

CHAR_BASEROW     EQU 6
CHAR_BASECOL     EQU 0




* === The Grand Display Structure ===================================== *
    STRUCTURE Display,0

   APTR NextDisplay

   APTR FirstWindow


    ; This variable is set by the task when it starts to run, and cleared
    ; when she goes.
   APTR TaskAddress

    ; This is the address of the port where this task's front door.
    ; It is here that this task will receive callers.
   APTR TaskPort

    ; The buffer contains the Zaphod copy of the PC display data, whether that
    ; data is text or graphics.
   APTR Buffer
   APTR BufferKey

    ; Since the Modes variable is used so often, a copy of the active
    ; window's modes variable is kept up here in the display structure.
   SHORT Modes          ; The primary mode flags for the task
   SHORT Modes2          ; The primary mode flags for the task


    ; === Display Variables =============================================== *
    ; The "display" is the area of the window where the PC information is
    ; rendered.  This area is necessarily equal to or smaller than the 
    ; current size of the window.  Under normal circumstances, the display
    ; is either the area within the window's borders or the entire window
    ; when the borders have been hidden.
   SHORT DisplayWidth
   SHORT DisplayHeight
    ; These variables describe the position of the top-left pixel of the 
    ; visible display with respect to the true top-left corner (0,0) of 
    ; the whole display.
   SHORT DisplayStartCol
   SHORT DisplayStartRow


   ULONG OldSeconds       ; Used for checking double-click */
   ULONG OldMicros        ; Used for checking double-click */
   ;ULONG TripleSeconds       ; Used when checking triple-click */
   ;ULONG TripleMicros       ; Used when checking triple-click */


    ; === Text Buffer Variables =========================================== *
   STRUCT TextOutputBuffer,BUFFER_WIDTH
   STRUCT TextBitMap,bm_SIZEOF
   STRUCT TextNormalPlane,BUFFER_WIDTH*CHAR_HEIGHT
   STRUCT TextInversePlane,BUFFER_WIDTH*CHAR_HEIGHT

    ; The following are text-grid oriented */
   SHORT TextStartCol
   SHORT TextStartRow
   SHORT TextWidth
   SHORT TextHeight
   STRUCT TextNewStart,25*2
   STRUCT TextNewLength,25*2

    ; These two variables are display-oriented.  They describe the
    ; offset into the display (usually less than or equal to zero)
    ; of the top-left character of the text grid.
   SHORT TextBaseCol
   SHORT TextBaseRow

    ; All of the cursor variables are character-grid oriented */
   SHORT CursorTop
   SHORT CursorBottom
   SHORT CursorAltTop
   SHORT CursorRow
   SHORT CursorCol
   SHORT CursorOldRow
   SHORT CursorOldCol
   SHORT CursorFlags

    ; The "real" cursor variables describe the real window coordinates of
    ; the cursor box(es)
   SHORT CursorRealLeft
   SHORT CursorRealRight
   SHORT CursorRealTop
   SHORT CursorRealBottom
   SHORT CursorRealAltTop
   SHORT CursorRealAltBottom

    ; === Graphics variables ============================================ */

    ; The plane pointers are for blasting the graphics display into Zaphod
    ; windows, and therefore are not used by Mono display, though they will
    ; be initialized anyway (just in case we find some need for it someday).
   STRUCT BufferBitMap,bm_SIZEOF



    ; === PC Interface Variables ========================================= */

   APTR JanusStart

    ; The current address (including the current offset) of the PC
    ; buffer (text or graphics).
   APTR PCDisplay
    ; The address of the CRT registers */
   APTR PCCRTRegisterBase
    ; The offsets describe the byte offset from the base of the PC display
    ; memory to the current start of display and cursor position.
   SHORT PCDisplayOffset
   SHORT PCCursorOffset

   STRUCT ZaphodCRT,CRT_IOBLOCKSIZE
    ; These are copies of the Color Display registers.   These registers
    ; have no Monochrome equivalent.
   UBYTE ColorSelect
   UBYTE ColorControl

   ; === Locking mechanism variables ==================================== */
   STRUCT DisplayLock,Lock_SIZEOF

    ; === Janus Sig Management ========================================== */
   APTR DisplayWriteSig
   APTR CRTWriteSig

    LABEL Display_SIZEOF




* === The SuperWindow Structure ======================================== */
    STRUCTURE SuperWindow,0

   APTR   NextWindow

   SHORT DisplayModes            ; Primary modes flags
   SHORT DisplayModes2            ; Primary modes flags
   SHORT Flags


    ; === Window/Screen Variables ======================================== */
   STRUCT DisplayNewWindow,nw_SIZE    ; For this display's window */
   APTR DisplayWindow         ; This display's window */
   STRUCT DisplayNewScreen,ns_SIZEOF  ; Display screen (as needed)
   APTR DisplayScreen         ; If this display opens a screen */
   STRUCT HorizImage,ig_SIZEOF ; Dummies for prop gadgets */
   STRUCT VertImage,ig_SIZEOF  ; Dummies for prop gadgets */
   STRUCT HorizInfo,pi_SIZEOF  ; For prop gadgets */
   STRUCT VertInfo,pi_SIZEOF   ; For prop gadgets */
   STRUCT HorizGadget,gg_SIZEOF ; For prop gadgets */
   STRUCT VertGadget,gg_SIZEOF ; For prop gadgets */

    ; Every universe needs border control measures */
   STRUCT DisplayBorder,BorderKontrol_SIZEOF


    ; These variables are used to record the most recent 
    ; less-than-full-screen size and position of the window, in case the
    ; user wants the window blown back out to that size (using either
    ; triple-click or a menu option)
   SHORT LastSmallX
   SHORT LastSmallY
   SHORT LastSmallWidth
   SHORT LastSmallHeight
            
    ; This guy can have the user's preference for text depth.
   SHORT TextDepth

   SHORT DisplayDepth

    ; Each window gets to specify its own cursor rate */
   LONG CursorSeconds
   LONG CursorMicros


    LABEL SuperWindow_SIZEOF





* === And Finally ====================================================   */
   IFND   EGLOBAL_CANCEL
   INCLUDE "eglobal.i"
   ENDC



