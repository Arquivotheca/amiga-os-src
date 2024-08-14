*******************************************************************************
*
* $Id: inputmap.asm,v 40.9 93/07/30 16:08:21 vertex Exp $
*
* $Log:	inputmap.asm,v $
* Revision 40.9  93/07/30  16:08:21  vertex
* Autodoc and include cleanup
* 
* Revision 40.8  93/03/31  18:42:16  Jim2
* Before installing the interrupt do a read on the port.  This
* removes the interrupt unsafe nature of ReadJoyPort (CI106).
* Also this will prevent an interrupt being added to a port that
* cannot get the resources.
*
* Revision 40.7  93/03/25  14:27:20  Jim2
* Change the repeat delay based on the current values in input.device.
*
* Revision 40.6  93/03/23  14:38:04  Jim2
* Unfixed so the hold off delay applies for all keypresses.  I had
* created this bug ealier (CI44).  Added complete qualifiers by
* doing a PeekQualifier (CI45).
*
* Revision 40.5  93/03/15  16:28:34  Jim2
* Changed for more complete rawkey definitions.
*
* Revision 40.4  93/03/12  20:54:40  Jim2
* Changed the table values to use the constants defined in lowlevel.i.
* Altered the multibutton, rollover behaviour to match what is done
* by the keyboard.  Namely, if multiple keys are depressed the
* repeat is done of the last one until it specifically is released;
* rather than stopping the repeat on the first release.
* Secondly, the initial repeat delay is only for the depression of
* the first key.  It should not be used when a key is released,
* nor should it be restarted when a second(or more) key is pressed.
*
* Revision 40.3  93/03/11  10:15:16  mks
* Fixed up the un-install...
*
* Revision 40.2  93/03/11  08:24:35  mks
* Updated the test code...
*
* Revision 40.1  93/03/10  17:20:35  mks
* First release
*
*******************************************************************************
*
* The following routines are used to map the game controller and joystick
* to keys in the input food chain.  The code requires that certain structures
* be placed into RAM since this is ROM code...
*
* There is an INSTALL and an UNINSTALL function...
*
        include 'exec/types.i'
        include 'exec/nodes.i'
        include 'exec/lists.i'
        include 'exec/interrupts.i'
        include 'exec/memory.i'
        include 'exec/ports.i'
        include 'exec/io.i'
        include 'exec/macros.i'
        include 'exec/execbase.i'
        include 'exec/tasks.i'
        include 'devices/input.i'
        include 'devices/inputevent.i'
        include 'devices/timer.i'
        include 'hardware/intbits.i'
*
        include '/lowlevel.i'
        include '/lowlevelbase.i'


*
*The following is a duplicate of the input.device structure.  We can't
*use the include file since it trys to include something else that really
*isn't needed.
*
        include 'v:src/kickstart/rawinput/stddevice.i'

ID_STKSIZE      EQU     $1000

MOUSEAHEAD      EQU     7


 STRUCTURE      InputData,dd_SIZEOF
        STRUCT  id_Unit,MP_SIZE   ; the one and only unit
        STRUCT  id_TC,TC_SIZE           ; task space
        WORD    id_RepeatCode           ; repeating key code (or -1 for none)
        STRUCT  id_Stk,ID_STKSIZE       ;       and stack space
        STRUCT  id_HandlerList,LH_SIZE  ; input handler function list
        STRUCT  id_IEPort,MP_SIZE       ; Input Event message port,
        STRUCT  id_TData,ie_SIZEOF      ;       timer event area,
        STRUCT  id_TIOR,IOTV_SIZE       ;       and timer request block
        STRUCT  id_MData,ie_SIZEOF*MOUSEAHEAD ; mouse data area,
        STRUCT  id_MIOR,IOSTD_SIZE      ;       and mouse request block
        STRUCT  id_K1Data,ie_SIZEOF     ;       keyboard data area 1,
        STRUCT  id_K1IOR,IOSTD_SIZE     ;       and keyboard request block
        STRUCT  id_K2Data,ie_SIZEOF     ;       keyboard data area 2,
        STRUCT  id_K2IOR,IOSTD_SIZE     ;       and keyboard request block
        STRUCT  id_RIOR,IOTV_SIZE       ;       repeat key timer request block
        STRUCT  id_Thresh,TV_SIZE       ; repeat key threshold
        STRUCT  id_Period,TV_SIZE       ; repeat key period
        LABEL   id_SIZEOF


*
* Comment out the line below if you don't want the test wrapper...
*TEST_IT:       ; Comment this out for no wrapper...
        ifd     TEST_IT
                include 'dos/dos.i'
                        move.l  4,a6                    ; Get ExecBase...
                        lea     LowName(pc),a1          ; Get library name...
                        moveq.l #0,d0                   ; Any version...
                        JSRLIB  OpenLibrary             ; Open it...
                        tst.l   d0                      ; Did it open?
                        beq.s   TestExit                ; Exit if not...
*
                        move.l  d0,a5                   ; LowLevel here...
                        moveq.l #0,d0                   ; Unit 0
                        bsr     InstallInputMap         ; Install it...
                        move.l  d0,-(sp)                ; Save it
                        beq.s   NoInstall               ; If error, exit...
                        move.l  #SIGBREAKF_CTRL_C,d0    ; Wait for CTRL-C...
                        JSRLIB  Wait                    ; (Wait)
NoInstall:              move.l  (sp)+,a0                ; Get handle back
                        bsr     UnInstallInputMap       ; Remove it...
                        move.l  a5,a1                   ; Get LowLevel base
                        JSRLIB  CloseLibrary            ; Close LowLevel
TestExit:               rts                             ; Return to CLI
LowName:                dc.b    'lowlevel.library',0
                        cnop    0,4
        endc
*
*
* Kludge until Amiga.lib has this...
_LVOReadJoyPort equ     -$1E
*
*******************************************************************************
*
* This is the data structure that will be used by the input mapping code
*
Q_SIZE  set     16      ; Size of the event queue...
*
 STRUCTURE      InputMapData,0
        STRUCT  imap_Interrupt,IS_SIZE
        ULONG   imap_ExecBase                   ; Storage of ExecBase
        ULONG   imap_LowLevel                   ; Storage of LowLevelBase
        ULONG   imap_Unit                       ; Unit number to handle...
        ULONG   imap_Repeat                     ; When it goes 0, we repeat
        ULONG   imap_RepeatRate                 ; Start count of repeat
        ULONG   imap_RepeatDelay                ; Start count on button down
*
        ULONG   imap_LastRead   ; The last stored READ from ReadJoyPort
        ULONG   imap_LastCode   ; The last keycode sent (or 0 for none)
*
        STRUCT  imap_Port,MP_SIZE               ; For queue control
        STRUCT  imap_IOSTD,IOSTD_SIZE*Q_SIZE    ; I/O structure 0...
        STRUCT  imap_Event,ie_SIZEOF*Q_SIZE     ; Input Event 0...
        LABEL   imap_SIZE
*
*******************************************************************************
*
* This is the actual interrupt code...
* The registers:  a0/a1/d0/d1/a5/a6 are all scrap...
* I use a5 as my data pointer...
*
InterruptCode:  move.l  a1,a5                   ; Get my data into safe place
                move.l  imap_LastCode(a5),d0    ; Check if we even have a code
                beq.s   ic_CheckPort            ; If not, go and check port
                subq.l  #1,imap_Repeat(a5)      ; un-count VBlank
                bne.s   ic_CheckPort            ; Check the port
*
* Ok, so we need to do a repeat...
*
* First set up for next repeat trigger...
*
                move.l  imap_RepeatRate(a5),imap_Repeat(a5)
*
* Now, send the repeated code...
*
                bsr     SendKey                 ; Sends the key in d0...
                ; Fall into ic_CheckPort
*
* Now, check the game port for a change in state...
*
ic_CheckPort:   move.l  imap_LowLevel(a5),a6    ; Get LowLevelBase
                move.l  imap_Unit(a5),d0        ; Get unit...
*
        ;       JSRLIB  ReadJoyPort             ; Read the JoyPort
                jsr     _LVOReadJoyPort(a6)     ; Read the JoyPort
*
                move.l  d0,d1                   ; Store for a bit...
                and.l   #JP_TYPE_MASK,d0        ; Mask type...
                beq.s   ic_Exit                 ; If nothing, we exit...
*
                move.l  imap_LastRead(a5),d0    ; Get last read...
                cmp.l   d0,d1                   ; Are they the same?
                beq.s   ic_Exit                 ; If so, we exit...
*
* Ok, so the two reads are not the same, so we need to deal with it...
* Most likely we need to cause events to happen...
*
ic_DoDiff:      movem.l a2/d2/d6/d7,-(sp)       ; Save these...
*
                move.l  d1,d7                   ; Store new read here...
                and.l   #JP_TYPE_MASK,d1        ; Check the type...
                and.l   #JP_TYPE_MASK,d0        ; Type of older controller...
                beq.s   ic_SameType             ; If no controller, the same...
                cmp.l   d0,d1                   ; Are they the same?
                beq.s   ic_SameType             ; If so, we continue...
*
* Ok, so all of the old information must be "lost" so we throw it out...
*
                move.l  d0,d1                   ; Get old type
                bsr.s   ic_DoDiff               ; Do the difference...
                move.l  d7,d1                   ; Get new type...
                and.l   #JP_TYPE_MASK,d1        ; Mask it...
                move.l  d1,d0                   ; Make the same...
                ; Fall into the same-type diff...
*
* Now, we have controllers of the same type, so figure out what changed
* and issue those key sequences.
*
ic_SameType:    move.l  imap_LastRead(a5),d6    ; Get old state...
                move.l  d7,imap_LastRead(a5)    ; Save new state...
                move.l  d6,d2                   ; Get old state...
                eor.l   d7,d2                   ; What changed...
*
                lea     BitKeyTable(pc),a2      ; Get table...
                bsr.s   DoKeys                  ; Do the keys
                move.l  d7,d0                   ; Get key
                and.l   #JP_TYPE_MASK,d0        ; Mask the type...
                cmp.l   #JP_TYPE_UNKNOWN,d0     ; Check if unknown...
                beq.s   ic_Unknown              ; If unknown, skip...
                cmp.l   #JP_TYPE_MOUSE,d0       ; Check if mouse...
                beq.s   ic_Mouse                ; If mouse, skip dir...
                lea     BitDirTable(pc),a2      ; Get table...
                bsr.s   DoKeys                  ; Do the keys
        ; fall  bra.s   ic_EndDiff              ; End of the KeyDiff
*
* So if we do mouse to joystick work, we would do it here...
*
ic_Mouse:       ; ... Maybe to be done later...
*
* There is not much that can be done with an unknown controller...
*
ic_Unknown:     ; Just fall into ic_EndDiff
ic_EndDiff:     movem.l (sp)+,a2/d2/d6/d7       ; Restore...
*
ic_Exit:        moveq.l #0,d0                   ; Set Z bit...
dk_RTS:         rts
*
* DoKeys - Inputs are   a2 - Table...
*                       d2 - Key XOR...
*                       d7 - New key bits...
*
DoKeys:         move.b  (a2)+,d0                ; Get bit number
                bmi.s   dk_RTS                  ; If negative, exit...
                move.b  (a2)+,d1                ; Get key code...
                btst.l  d0,d2                   ; Test bit...
                beq.s   DoKeys                  ; If not a delta, do next
                btst.l  d0,d7                   ; Check if a down key
                bne.s   dk_Down                 ; If so, a down...
                bset.l  #IECODEB_UP_PREFIX,d1   ; Set UP prefix...
                bra.s   dk_Continue
dk_Down:        move.l  imap_RepeatDelay(a5),imap_Repeat(a5)    ; Repeat setup
dk_Continue:    move.l  imap_Unit(a5),d0        ; Get unit...
                asl.l   #8,d0                   ; Shift unit number...
                or.b    d1,d0                   ; Or in the RAW code...
                bsr.s   SendKey                 ; Send the key in d0...
                bra.s   DoKeys
*
* SendKey - Send the keycode that is in D0.  Only D0 and A5 are set up...
* D0/D1/A0/A1/A6 are trash...
*
* If there are no STDIOs available, the event is just "lost"
* Note that if the key code is the same as the LastCode field,
* it is assumed to be a Repeat.  If the KeyCode is an UP qualifier,
* the LastCode field is cleared (as UP keys do not repeat)
*
SendKey:        movem.l d2/d3,-(sp)             ; Save d2...
                move.l  d0,d2                   ; Store code in d2...
                move.l  imap_IOSTD+IO_DEVICE(a5),a6
                JSRLIB  PeekQualifier
                move.l  d0,d3                   ;Store the current qualifiers.
                move.l  imap_ExecBase(a5),a6    ; Get ExecBase...
                lea     imap_Port(a5),a0        ; Get message port...
                JSRLIB  GetMsg                  ; Get the next queued STDIO
                tst.l   d0                      ; Check if it is ok...
                beq.s   sk_Exit                 ; If no STDIO, we exit...
                move.l  d0,a1                   ; Get STDIO into a1
                move.l  IO_DATA(a1),a0          ; Get the InputEvent...
*
* Now, set up the STDIO as needed...
*
                move.w  #IND_WRITEEVENT,IO_COMMAND(a1)  ; Command
                move.l  #ie_SIZEOF,IO_LENGTH(a1)        ; Size of data...
*
* Now, set up the InputEvent...
*
                cmp.l   imap_LastCode(a5),d2    ; Same code?
                bne.s   2$                      ; If not same, not repeat
                bset.l  #IEQUALIFIERB_REPEAT,d3 ; Set Repeat key
2$

                cmp.l   #RAWKEY_PORT0_BUTTON_RED,d2 ; If left button on port 0,
                bne     3$
                bset.l  #IEQUALIFIERB_LEFTBUTTON,d3 ; Set the left button qualifier
3$

                clr.l   (a0)+                   ; APTR    ie_NextEvent
                move.b  #IECLASS_RAWKEY,(a0)+   ; UBYTE   ie_Class
                clr.b   (a0)+                   ; UBYTE   ie_SubClass
                move.w  d2,(a0)+                ; UWORD   ie_Code
                move.w  d3,(a0)+                ; UWORD   ie_Qualifier
                clr.l   (a0)+                   ; LABEL   ie_EventAddress
                JSRLIB  SendIO
                bclr.l  #IECODEB_UP_PREFIX,d2   ; Check for up key
                beq.s   sk_NotUp                ; If not up, do not clear
                sub.l   imap_LastCode(a5),d2
                bne.s   sk_Exit                 ; Clear key code if UP
sk_NotUp:       move.l  d2,imap_LastCode(a5)    ; Store new last code...
sk_Exit:        movem.l (sp)+,d2/d3             ; Restore d2...
                rts
*
*******************************************************************************
*
* handle = InstallInputMap(Unit,LowLevelBase,ExecBase)
* d0                       d0   a5           a6
*
* This routine will install the input mapping code for the game controller.
* The result of this routine is a handle needed to remove the mapping code.
* Note:  There is no locking in this code.  That is, if you call it more
* than once or at the same time, you will get more than one and it may not
* do what you wish.
*
InstallInputMap:        xdef    InstallInputMap
                movem.l d2/a2-a4,-(sp)          ; Save these...
                move.l  d0,d2                   ; Unit..
*
* Ok, first, we need some RAM...
*
                move.l  #imap_SIZE,d0           ; Our structure...
                move.l  #MEMF_PUBLIC!MEMF_CLEAR,d1
                JSRLIB  AllocVec                ; Try to get it...
                move.l  d0,a4                   ; Store our base data addr
                tst.l   d0                      ; Did we work?
                beq     IIM_Exit                ; If not, exit with a NULL...

                move.l  d2,d0
                exg     a5,a6
        ;       JSRLIB  ReadJoyPort             ; Read the JoyPort
                jsr     _LVOReadJoyPort(a6)     ; Read the JoyPort
                exg     a6,a5
                and.l   #JP_TYPE_MASK,d0
                beq     IIM_Exit                ;Cannot get resources for this port.

*
* Ok, we have memory, so start setting it up...
*
                move.l  a6,imap_ExecBase(a4)    ; Store ExecBase
                move.l  a5,imap_LowLevel(a4)    ; Store LowLevelBase
                move.l  d2,imap_Unit(a4)        ; Store Unit number
*
* Set up the interrupt structure
*
                move.l  #InterruptName,imap_Interrupt+LN_NAME(a4)
                move.l  a4,imap_Interrupt+IS_DATA(a4)
                move.l  #InterruptCode,imap_Interrupt+IS_CODE(a4)
*
* Next, set up the port...
*
                move.b  #PA_IGNORE,imap_Port+MP_FLAGS(a4)
                lea     imap_Port+MP_MSGLIST(a4),a0
                NEWLIST a0
*
* Ok, so now we need to open the device and set it all up...
*
                lea     inputDevice(pc),a0      ; Device name
                moveq.l #0,d0                   ; Unit 0
                lea     imap_IOSTD(a4),a1       ; IO Request structure
                moveq.l #0,d1                   ; Flags of 0
                JSRLIB  OpenDevice              ; Open the device
                tst.l   d0                      ; Check result...
                beq.s   OpenOK
                move.l  a4,a1                   ; Free the memory...
                JSRLIB  FreeVec                 ; because device fail...
                moveq.l #0,d0                   ; Return fail...
                bra     IIM_Exit                ; Exit...
OpenOK:
*
* Somewhere we must set up the repeat values...
*

                move.l  #20000,d0               ;The PAL micros/VBlank divisor.
                cmp.l   #715909,ll_EClockConversion(a5) ;715909 is the NTSC frequency.
                bne.s   NotNTSC

                sub.l   #3334,d0                ;The NTSC micros/VBlank divisior is about this much smaller.
NotNTSC:        move.l  imap_IOSTD+IO_DEVICE(a4),a0     ;Get the base for input.device.
                move.l  id_Thresh+TV_SECS(a0),d1
                mulu.w  #15625,d1
                lsl.l   #6,d1                   ;Multiply the seconds to get micros.
                add.l   id_Thresh+TV_MICRO(a0),d1       ;Add micros.
                divu.w  d0,d1                   ;Divide to get # of VBlanks.
                addq.w  #1,d1                   ;Account for the 'extra' VBlank caused by timer.device.
                move.w  d1,imap_RepeatDelay+2(a4)       ;Store into a long word.  Only change low order word.
                move.l  id_Period+TV_SECS(a0),d1
                mulu.w  #15625,d1
                lsl.l   #6,d1                   ;Multiply the seconds to get micros.
                add.l   id_Period+TV_MICRO(a0),d1       ;Add micros.
                divu.w  d0,d1                   ;Divide to get # of VBlanks.
                addq.w  #1,d1                   ;Account for the 'extra' VBlank caused by timer.device.
                move.w  d1,imap_RepeatRate+2(a4)        ;Store into a long word.  Only change low order word.
*
* So, now set up each of the input events and IOSTDs
*
                lea     imap_IOSTD(a4),a2       ; IOSTD...
                lea     imap_Event(a4),a3       ; Events...
                lea     imap_Port(a4),a0        ; Get port...
                move.l  a0,MN_REPLYPORT(a2)     ; Set up reply port...
                moveq.l #Q_SIZE-1,d2            ; Size of the queue -1
                bra.s   io_SetLoop
*
* Copy the root IO request to the next one we are doing...
*
SetUpLoop:      move.l  a2,a0                   ; Next IOSTD
                lea     imap_IOSTD(a4),a1       ; Root IOSTD
                move.l  #IOSTD_SIZE-1,d1        ; Number of bytes to copy
copy_Loop:      move.b  (a1)+,(a0)+             ; Copy it...
                dbra.s  d1,copy_Loop            ; Copy it all...
*
* Set up the pointers to point to the input event...
*
io_SetLoop:     move.l  a3,IO_DATA(a2)          ; Point to data
                move.l  a2,a1                   ; Get message (to reply)
                JSRLIB  ReplyMsg                ; Put it onto our port
                add.l   #IOSTD_SIZE,a2          ; Next IOSTD
                add.l   #ie_SIZEOF,a3           ; Next Event
                dbra.s  d2,SetUpLoop            ; Setup next one...
*
* Ok, so now, install the handler...
*
                lea     imap_Interrupt(a4),a1   ; Point at interrupt
                moveq.l #INTB_VERTB,d0          ; Which one...
                JSRLIB  AddIntServer            ; Add the server...
                move.l  a4,d0                   ; Return memory pointer
*
* Stack cleanup and exit...
*
IIM_Exit:       movem.l (sp)+,d2/a2-a4          ; restore...
                rts                             ; Return...
*
*******************************************************************************
*
* void UnInstallInputMap(handle)
*                        a0
*
* Removes and releases the memory for the input handle...
*
UnInstallInputMap:      xdef    UnInstallInputMap
                movem.l d2/a5/a6,-(sp)
                move.l  a0,a5                   ; Get our data pointer
                move.l  a0,d0                   ; Check for NULL...
                beq.s   UIM_Exit                ; Exit if NULL input...
                move.l  imap_ExecBase(a5),a6    ; Get ExecBase
*
* Remove the interrupt...
*
                lea     imap_Interrupt(a5),a1   ; Point at interrupt
                moveq.l #INTB_VERTB,d0          ; VBlank...
                JSRLIB  RemIntServer            ; Remove it...
*
* Now, wait for all of the outstanding messages to come back...
* So, set up the message port...
*
                move.l  ThisTask(a6),imap_Port+MP_SIGTASK(a5)   ; SigTask
                move.b  #SIGB_SINGLE,imap_Port+MP_SIGBIT(a5)    ; SigBit
                move.b  #PA_SIGNAL,imap_Port+MP_FLAGS(a5)       ; Signal...
*
* Now loop here until we get all of the messages back...
* (They usually will all be here...)
*
                move.l  #Q_SIZE-1,d2
flush_Loop:     lea     imap_Port(a5),a0        ; Get port address...
                JSRLIB  WaitPort                ; Wait for something...
                lea     imap_Port(a5),a0        ; Get port address...
                JSRLIB  GetMsg                  ; Empty message from port
                dbra    d2,flush_Loop           ; Keep going until all return
*
* Now, close the device...
*
                lea     imap_IOSTD(a5),a1       ; Get IOSTD
                JSRLIB  CloseDevice             ; Close the device...
*
* Finally, free the memory...
*
                move.l  a5,a1                   ; Ready for FreeVec...
                JSRLIB  FreeVec                 ; Release the memory...
*
* Exit...
*
UIM_Exit:       movem.l (sp)+,d2/a5/a6          ; Restore
                rts                             ; return...
*
*******************************************************************************
*
* The following is a table of bit to key conversions...
* One byte for the bit, one byte for the key...
*
* Color or Description    ReadJoyPort        RAWKEY code     Reused from
* --------------------    -----------        -----------     -----------
* Blue / Back / Stop      JPB_BUTTON_BLUE       $72          CDTV Stop
* Red / Select            JPB_BUTTON_RED        $78          unused
* Yellow / Repeat         JPB_BUTTON_YELLOW     $77          CDTV FF
* Green / Shuffle         JPB_BUTTON_GREEN      $76          CDTV REW or Eject
* Next / Right-Ear        JPB_BUTTON_FORWARD    $75          CDTV Next
* Previous / Left-Ear     JPB_BUTTON_REVERSE    $74          CDTV Prev
* Play / Pause            JPB_BUTTON_PLAY       $73          CDTV Play/Pause
*
* Up                      JPB_JOY_UP            $79          unused
* Down                    JPB_JOY_DOWN          $7A          unused
* Left                    JPB_JOY_LEFT          $7C          unused
* Right                   JPB_JOY_RIGHT         $7B          unused
*
BitKeyTable:    dc.b    JPB_BUTTON_BLUE,RAWKEY_PORT0_BUTTON_BLUE        ; Stop          Right mouse
                dc.b    JPB_BUTTON_RED,RAWKEY_PORT0_BUTTON_RED          ; Select        Left Mouse      Fire
                dc.b    JPB_BUTTON_YELLOW,RAWKEY_PORT0_BUTTON_YELLOW    ; Repeat
                dc.b    JPB_BUTTON_GREEN,RAWKEY_PORT0_BUTTON_GREEN      ; Shuffle
                dc.b    JPB_BUTTON_FORWARD,RAWKEY_PORT0_BUTTON_FORWARD  ; Forward
                dc.b    JPB_BUTTON_REVERSE,RAWKEY_PORT0_BUTTON_REVERSE  ; Reverse
                dc.b    JPB_BUTTON_PLAY,RAWKEY_PORT0_BUTTON_PLAY        ; Play/Pause    Middle Mouse
                dc.b    -1              ; End of table
*
* Table for Up/Down/Left/Right
*
BitDirTable:    dc.b    JPB_JOY_UP,RAWKEY_PORT0_JOY_UP          ; Up
                dc.b    JPB_JOY_DOWN,RAWKEY_PORT0_JOY_DOWN      ; Down
                dc.b    JPB_JOY_LEFT,RAWKEY_PORT0_JOY_LEFT      ; Left
                dc.b    JPB_JOY_RIGHT,RAWKEY_PORT0_JOY_RIGHT    ; Right
                dc.b    -1              ; End of table
*
*******************************************************************************
*
InterruptName:  dc.b    'Input Mapper',0
inputDevice:    dc.b    'input.device',0
*
*******************************************************************************
*
                END
