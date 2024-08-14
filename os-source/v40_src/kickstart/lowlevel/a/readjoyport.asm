******************************************************************************
*
*	$Id: readjoyport.asm,v 40.21 93/07/30 16:06:49 vertex Exp $
*
******************************************************************************
*
*	$Log:	readjoyport.asm,v $
* Revision 40.21  93/07/30  16:06:49  vertex
* Autodoc and include cleanup
* 
* Revision 40.20  93/05/05  09:48:36  gregm
* LOts of changes.
*  (1) SetJoyPortAttrs() added.  Allows API for locking a controller in.
*  (2) Several optimizations were taken; move's where better than lea's, etc.
*  (3) Different versions of the code for different port #'s, different
*      controller types, etc.  Fixes mouse->joystick confusion, and speeds things
*      up.
*  (4) On port 0 & port 1 game controller code, when 'locked', we only clock in the
*      first 6 bits of the buttons.  HUGE savings.
*  (5) ReadJoyPort() was modified to redirect call when a controller type is locked in.
*
* Revision 40.19  93/04/20  14:17:33  Jim2
* Polished the Autodoc.
*
* Revision 40.18  93/04/19  14:25:47  Jim2
* Playing with the fire line must be inside the critical section.
*
* Revision 40.17  93/04/19  13:41:44  Jim2
* Removed some dead lines.  Improved the comments.  Reset the
* first time init flag if the allocation of the Potgo resources
* fail.
*
* Revision 40.16  93/04/15  14:25:38  Jim2
* Drive the fire button output low before changing it to an
* output.
*
* Revision 40.15  93/04/13  15:36:11  Jim2
* Play with the comments.  Change the initial delay to be
* five CIA accesses.  Move the raising of the clock to
* just before the dropping.
*
* Revision 40.14  93/04/02  11:35:26  Jim2
* Forgot when I changed d4 to be a bit to correct the reading
* of the fire button.
*
* Revision 40.13  93/04/01  18:09:41  Jim2
* Well this is the best I can do for now for optimizing
*
* Revision 40.12  93/04/01  13:59:09  Jim2
* Played with the delay to get CDGS to recognise hot insertion
* of the game controller
*
* Revision 40.11  93/03/31  18:47:32  Jim2
* One last try at minimizing the possiblity of transitioning
* incorrectly from unknown to joystick (CI49).  The initial
* read for a port is now taken at the same time the resources
* are aquired.  This means it is quite possible for the first
* read to return unknown.  However, it reduces possibly greatly
* the time between the first and second read.
*
* Also there were timing problems reading the joy pad as
* a result of allowing the code to run on 68000.  Doubling
* the delay while clocking was sufficient (CI114).
*
* Revision 40.10  93/03/26  17:41:10  Jim2
* Removed the transition from Unknown to Mouse state.  Always force this through the mouse state.
*
* Revision 40.9  93/03/26  14:10:35  Jim2
* Removed call to OpenResource.  Documented how/when this routine
* is safe from interrupts.
*
* Revision 40.8  93/03/26  11:18:32  Jim2
* Altered the test for possible joystick while in mouse state.
* Added a bit to allow direct transitions from the unknown state
* to the joystick state.
*
* Revision 40.7  93/03/19  11:31:44  Jim2
* Absorbed the resource grabbing code.  Changed the program flow
* to handle double controllers.
*
* Revision 40.6  93/03/12  20:21:51  Jim2
* Moved the status information to a port specific location which is what
* is actually placed into a2 rather than just a pointer to the
* last value read.
*
* Revision 40.5  93/03/10  19:52:45  Jim2
* The method for updating ll_Port?Status did not work well with
* the use of these fields for general ReadJoyPort status stuff, so
* I grew the seperate status stuff a new filed in the library base.
*
* Revision 40.4  93/03/04  11:53:54  Jim2
* Tumbled the code ordering and added support for returning the
* buttons for an unknown controller.
*
* Revision 40.3  93/03/02  13:24:24  Jim2
* Changed all of the references from game.library to lowlevel.library.
*
* Revision 40.2  93/03/01  16:01:34  Jim2
* Added NOTE to the autodoc to show the true nature of the beast.
*
* Revision 40.1  93/02/19  17:48:13  Jim2
* Brought the autodoc in line with standard format.  Added
* better description for the game controller buttons.  Removed
* Dave's controller picture.
*
* Revision 40.0  93/02/11  17:33:03  Jim2
* Change to use the new single include file.
*
* Revision 39.12  93/01/20  13:18:48  brummer
* fix autodoc tab alignment
*
* Revision 39.11  93/01/19  14:20:07  brummer
* Update AUTODOC with diagram of the Game Controller
*
* Revision 39.10  93/01/18  18:03:29  brummer
* Seperated initialization code into seperate module
* Updated to use "almost semaphore" with gameport and new initialization code
* Updated to prevent reentrance because of game controller clocking
* Updated Autodocs
*
* Revision 39.9  93/01/14  10:48:43  brummer
* Fix to prevent invalid game controller clocks on Jim's 4000.  Further
* testing on other platforms is still necessary.
*
* Revision 39.8  93/01/12  16:58:41  brummer
* Fixed the previous fix.
*
* Revision 39.7  93/01/12  10:56:12  brummer
* Fix to extra mouse state check added in previous rev,
* Updated comments in code to reflect actions in each state.
*
* Revision 39.6  93/01/11  16:04:55  brummer
* Add temporary fix for right mouse button being stuck on when mouse is in port 1.
*
* Revision 39.5  93/01/11  15:33:03  brummer
* Added extra test to mouse state to prevent sperious returns of
* joystick type.
*
* Revision 39.4  93/01/07  15:57:27  brummer
* Added /gamebase.i to the include file list.
*
* Revision 39.3  93/01/07  15:38:52  brummer
* Updated to support dynamic changing controller types.
* Added 1 extra CIA delay after enabling game ctlr MUX.
*
* Revision 39.2  93/01/05  12:28:45  Jim2
* *** empty log message ***
*
* Revision 39.1  93/01/05  12:02:14  Jim2
* Renamed from GetJoyPortEvent.asm.  Also changed the bit flag
* defintions.
*
* Revision 39.0  92/12/21  11:41:18  Jim2
* Initial Release.
*
*
*
*	(C) Copyright 1992 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

                NOLIST
                INCLUDE "exec/ables.i"
                INCLUDE "exec/interrupts.i"
                INCLUDE "exec/io.i"
                INCLUDE "exec/lists.i"
                INCLUDE "exec/macros.i"
                INCLUDE "exec/memory.i"
                INCLUDE "exec/nodes.i"
                INCLUDE "exec/strings.i"
                INCLUDE "exec/types.i"
                INCLUDE "hardware/custom.i"
                INCLUDE "hardware/cia.i"
                INCLUDE "hardware/intbits.i"
                LIST
                INCLUDE "JoyPortInternal.i"
                INCLUDE "/lowlevelbase.i"
                INCLUDE "/macros.i"
                INCLUDE "/lowlevel.i"


                XREF    _intena
                XREF    _custom
                XREF    _ciaa

                SECTION code

                XDEF    ReadJoyPort
                XDEF    SetJoyPortAttrs
                XDEF    RJP_VBlank_Int
                XDEF    RJP_Joy_Direction

******* lowlevel.library/ReadJoyPort *****************************************
*
*   NAME
*	ReadJoyPort -- return the state of the selected joy/mouse port. (V40)
*
*   SYNOPSIS
*	portState = ReadJoyPort(portNumber);
*	D0                      D0
*
*	ULONG ReadJoyPort(ULONG);
*
*   FUNCTION
*	This function is used to determine what device is attached to the
*	joy port and the current position/button state. The user may attach
*	a mouse, game controller, or joystick to the port and this function
*	will dynamically detect which device is attached and return the
*	appropriatly formatted portState.
*
*	To determine the type of controller that is attached, this function
*	clocks the game controller and/or interprets changes in the joy
*	port data. Valid clocked data from the game controller is immediately
*	detected. However, to accuratly determine if a mouse or joystick is
*	attached, several calls to this function are required along with some
*	movement at the joy port by the user.
*
*	This function always executes immediatly.
*
*	This is a low level single threaded function that does not fit the
*	normal Amiga multitasking model. Only one task can be executing
*	this routine at any time. All others will return immediately with
*	JP_TYPE_NOTAVAIL.
*
*	The nature of this routine is not meant to encourage non-multitasking
*	friendly programming practices like polling loops. If your task
*	is waiting for a transition to be returned use a WaitTOF() between
*	calls to minimize the total system impact.
*
*	When called the first time, for each port, this function attempts to
*	acquire certain system resources. In order to aquire these resources
*	this function MUST be called from a task, or a DOS process. If this
*	function fails to aquire the necessary resources, it will
*	return with JP_TYPE_NOTAVAIL. Once the resources are aquired (return
*	value other than JP_TYPE_NOTAVAIL) this function may be used in
*	interrupts.
*
*   INPUTS
*	portNumber - port to read, in the range 0 to 3.
*
*   RESULT
*	portState - bit map that identifies the device and the current
*	            state of that device. The format of the bit map is
*	            dependant on the type of device attached.
*
*	            The following constants from <libraries/lowlevel.h>
*	            are used to determine which device is attached and
*	            the state of that device.
*
*	            The type of device can be determined by applying
*	            the mask JP_TYPE_MASK to the return value and comparing
*	            the resultant value with the following:
*
*	                JP_TYPE_NOTAVAIL        port data unavailable
*	                JP_TYPE_GAMECTLR        game controller
*	                JP_TYPE_MOUSE           mouse
*	                JP_TYPE_JOYSTK          joystick
*	                JP_TYPE_UNKNOWN         unknown device
*
*	            If type = JP_TYPE_GAMECTLR the bit map of portState is:
*	                JPF_BUTTON_BLUE         Blue - Stop
*	                JPF_BUTTON_RED          Red - Select
*	                JPF_BUTTON_YELLOW       Yellow - Repeat
*	                JPF_BUTTON_GREEN        Green - Shuffle
*	                JPF_BUTTON_FORWARD      Charcoal - Forward
*	                JPF_BUTTON_REVERSE      Charcoal - Reverse
*	                JPF_BUTTON_PLAY         Grey - Play/Pause
*	                JPF_JOY_UP              Up
*	                JPF_JOY_DOWN            Down
*	                JPF_JOY_LEFT            Left
*	                JPF_JOY_RIGHT           Right
*
*	            If type = JP_TYPE_JOYSTK the bit map of portState is:
*	                JPF_BUTTON_BLUE         Right
*	                JPF_BUTTON_RED          Fire
*	                JPF_JOY_UP              Up
*	                JPF_JOY_DOWN            Down
*	                JPF_JOY_LEFT            Left
*	                JPF_JOY_RIGHT           Right
*
*	            If type = JP_TYPE_MOUSE the bit map of portState is:
*	                JPF_BUTTON_BLUE         Right mouse
*	                JPF_BUTTON_RED          Left mouse
*	                JPF_BUTTON_PLAY         Middle mouse
*	                JP_MVERT_MASK           Mask for vertical counter
*	                JP_MHORZ_MASK           Mask for horizontal counter
*
*   SEE ALSO
*	SetJoyPortAttrs()
*
******************************************************************************
*
* The following registers are used thoughtout the routine
*	d4 - Bit in the CIA for the fire line.
*	d5 - Word flag for accessing the output enable for the middle mouse button.
*	d6 - Used for building the return value.
*	d7 - Value read from the correct joyport.
*	a2 - Points to port specific structure.
*	a3 - Points to the base of the custom registers
*	a4 - Points to the base of the CIA  A chip
*	a5 - Lowlevel.base
*
******************************************************************************

                XDEF    ReadJoyPort
PortOffsetTable:
                dc.w    ll_Port0State,ll_Port1State,ll_Port2State,ll_Port3State

ReadJoyPort:

** Painful redirection code.  I hate to do it, but it's necessary.  A little
** pain here saves MUCH more time later.

               move.w  d0,d1                               ; save port #
               add.b   d1,d1                               ; times 2 -- each table entry=2
               lea.l   PortOffsetTable,a0                  ;
               move.w  0(a0,d1.w),d1                       ; get base offset for that struct
               lea.l   0(a6,d1.w),a0                       ; get ptr to correct state struct
               move.l  lljp_RJPVector(a0),d1               ; grab contents of vector
               beq.s   201$                                ; if no redirect, use sensing code
               move.l  d1,a0                               ; else move ptr to address reg
               jmp     (a0)                                ; and jmp there.
201$

* regular code

                movem.l d2-d7/a2-a6,-(sp)

                move.l  #_custom,a3
                move.l  #_ciaa,a4
                move.l  a6,a5
                move.l  #JP_TYPE_NOTAVAIL,d6                ;Intialize the return value to NotAvail
                move.l  d0,d2
                cmp.l   #3,d2
                bhi     RJP_NotEntered                      ;Is this a valid port?
                                                            ;Yes, valid port.
                move.l  a0,a2                               ;Use the precalced state ptr

                btst.b  #0,d2
                bne.s   rjp_InPort1
                                                            ;Init Port 0 register.
                bset    #JP_STATUSB_ALREADYRUNNING,lljp_Status(a2)      ; is this reentrance ?
                bne     RJP_NotEntered                      ;if yes, j to exit

                move.l  #(joy0dat<<16),d7                   ;Save the offset for possibly reading the game controller again.
                move.w  joy0dat(a3),d7                      ;d7 now contains the port value, preserve d7.
                move.w  #CIAB_GAMEPORT0,d4
                move.w  #POTGOF_OE8,d5
                btst    #JP_STATUSB_VBLANK_ACQUIRED,lljp_Status(a2)
                bne.s   RJP_DonePortSetup                   ;Do we need to try getting the VBlank interrupt?
                                                            ;Yes, this must be the first time through here.
                move.l  ll_ExecBase(a5),a6
                                                            ;
                                                            ;Disable interrupts and find gameport.device on
                                                            ;the list.
                                                            ;
                move.l  IVVERTB+IV_DATA(a6),a0              ; a0 gets vert blank list
                lea     GAMEPORT_INT(pc),a1                 ; a1 gets ptr to name we're lookin fo
                JSRLIB  Forbid                              ;Don't want anyone else mucking with the list at the same time.
                JSRLIB  FindName                            ; d0 gets ptr to IS
                beq.s   2$                                  ;If not found, do nothing.
                                                            ;Entry found, save pointer and remove it from list.
                bset    #JP_STATUSB_VBLANK_ACQUIRED,lljp_Status(a2)     ; set acquired flag
                bclr    #JP_STATUSB_FIRST_INIT_REQ,lljp_Status(a2)
                move.l  d0,ll_RJP_Saved_IS(a5)              ; save ptr to gameport IS
                move.l  d0,a1                               ; a1 gets entry to remove
                moveq   #INTB_VERTB,d0                      ; d0 gets interrupt number
                JSRLIB  RemIntServer                        ; remove entry from list
                                                            ;Add ReadJoyPort's entry to the list.
                moveq   #INTB_VERTB,d0                      ; d0 gets interrupt number
                lea     ll_RJP_Vblank_IS(a5),a1             ; a1 gets ptr to our entry
                JSRLIB  AddIntServer                        ; add entry to list
2$:                                                         ;Enable interrupts.
                JSRLIB  Permit
                bsr     RJP_Joy_Direction
                beq.s   Port0Joy                            ;Was the initial reading a valid mouse?
                                                            ;
                                                            ;The joyport reading was for a mouse.  So set the
                                                            ;mouse as the current state.  Also set the count to
                                                            ;maximum.
                                                            ;
                or.l    #JP_TYPE_MOUSE|(JP_MOUSE_2_RECHECK<<16),d7
                bra.s   WritePort0
                                                            ;Can't tell for sure what type of controller is attached.
Port0Joy:       or.l    #JP_TYPE_UNKNOWN,d7
WritePort0:     move.l  d7,lljp_PreviousValue(a2)           ;Set the previous counter to be the current counter.
                bra.s   RJP_DonePortSetup


                                                            ;Init port 1 registers :
rjp_InPort1:
                bset    #JP_STATUSB_ALREADYRUNNING,ll_Port0State+lljp_Status(a5)      ; is this reentrance ?
                bne     RJP_NotEntered                      ; if yes, j to exit

                move.l  #(joy1dat<<16),d7                   ;Save the offset for possibly reading the game controller again.
                move.w  joy1dat(a3),d7                      ;d7 now contains the port value, preserve d7.
                move.w  #CIAB_GAMEPORT1,d4
                move.w  #POTGOF_OE12,d5
RJP_DonePortSetup:                                          ;All port specific initialization is complete.
                move.l  ll_PotGoResource(a5),a6
                                                            ;Determine if first call init is required :
                bclr    #JP_STATUSB_FIRST_INIT_REQ,lljp_Status(a2)
                beq.s   RJP_PortInited                      ;if no, j to continue
                                                            ;Establish default type of controller, Mouse or Unknown.
                bsr     RJP_Joy_Direction
                beq.s   InitUnknown
                                                            ;The reading can only be a mouse.
                or.l    #JP_TYPE_MOUSE|(JP_MOUSE_2_RECHECK<<16),d7
                bra.s   DoneInit

InitUnknown:    or.l    #JP_TYPE_UNKNOWN,d7
DoneInit:       move.l  d7,lljp_PreviousValue(a2)           ;Set the initial port reading.
                                                            ;Generate the map of POTGO bits to be aquired.
                move.w  d5,d1
                lsr.w   #1,d1
                or.w    d5,d1
                move.w  d1,d0
                lsl.w   #2,d1
                or.w    d1,d0
                move.w  d0,d3
                JSRLIB  AllocPotBits
                cmp.w   d3,d0
                beq.s   RJP_PortInited                      ;Did we get the bits we wanted?
                                                            ;
                                                            ;No, free the bits be did get and reset the flag
                                                            ;so we will try again the next time through.
                                                            ;
                JSRLIB  FreePotBits
                bset    #JP_STATUSB_FIRST_INIT_REQ,lljp_Status(a2)
                bra     RJP_NotAvail_State

RJP_PortInited:                                             ;Check to see if we are a double controller.
rjp_NotDoubleController:                                    ;Init common registers.
                move.w  (a2),d2                             ;Get the count for the port.
                and.w   #$00FF,d2                           ;The count is in the low order byte.
                bne.s   RJP_NoGameTest

; *****************************************************************************
;
; The following block will clock the button settings out of the game
; controller.  The bits will be collected and returned in d1.
;
;       requires:       a2 = ptr to JoyPortState
;                       a3 = ptr to custom register base
;                       a4 = ptr to ciaa register base
;                       d4 = bit mask for Fire Button
;                       d5 = mask to set appropriate Output Enable for Mux
;
RJP_Clock_Game_Ctlr:

                bset    #JP_STATUSB_CLOCKING_CTLR,lljp_Status(a2)       ; critical section flag

                bset    d4,ciaddra(a4)                      ;Set direction for fire button.
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;
                                                            ;Enable game controller mux (enable the middle
                                                            ;button as an output and send it a low).
                                                            ;
                move.w  d5,d1
                lsr.w   #1,d1
                or.w    d5,d1                               ;d1 has the mask of bits to be changed.
                move.l  d1,-(sp)                            ;Save the mask for restoring.
                move.w  d5,d0                               ;Output a 0.
                JSRLIB  WritePotgo
                move.l  #_custom+potinp,a0                  ;Get read address into a register this decreases the time in the loop.
                move.w  d5,d1
                lsl.w   #1,d1                               ;Get mask for reading controller data.
                                                            ; Set loop counter and clear temp data register.
                CLEAR   d3
                moveq   #8,d0                               ; d0 is loop counter
                btst    #JP_STATUSB_2nd_CTLR,lljp_Status(a2)
                beq.s   rjp_GameStartHere
                                                            ;The double controller requires being clocked nine extra times.
                add.w   #9,d0
                bra.s   rjp_GameStartHere
                                                            ; Loop for all bits :
RJP_Clock_Game_Ctlr_Loop:
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
rjp_GameStartHere:
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                move.w  (a0),d2                             ; d2 gets POTGOR data
                                                            ;Clock for next value.
                bset    d4,ciapra(a4)                       ;Make the clock high
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;Munge the button data bit into d3.
                and.w   d1,d2
                bne.s   6$                                  ;Buttons are active low.
                                                            ;The button was pressed.
                bset    d0,d3                               ;set button down in temp reg.
6$:             dbra    d0,RJP_Clock_Game_Ctlr_Loop         ; loop for all bits
                                                            ;
                                                            ;Disable game controller mux (enable the middle
                                                            ;button as an output and send it a high).
                                                            ;
                move.l  (sp)+,d1
                move.w  d1,d0
                JSRLIB  WritePotgo
                bclr    d4,ciaddra(a4)                      ;Restore direction of fire button to input :
                bclr    #JP_STATUSB_CLOCKING_CTLR,lljp_Status(a2)       ; critical section flag
                and.w   #$1FF,d3                            ;Only want to return low order bits for a double controller.
                lsr.w   #1,d3
                bcc.s   RJP_NoGameTest                      ;Last bit clocked must be high for the controller.

                cmp.b   #$FF,d3
                bne     RJP_GameCtlr_State                  ;Next to last bit must be a zero for the controller.
                                                            ;The port is not contected to a controller.
RJP_NoGameTest:
                btst    #JP_STATUSB_2nd_CTLR,lljp_Status(a2)
                bne.s   RJP_NotAvail_State                  ;If this port is for a double controller and
                                                            ;we don't detected it it can't be used.
; *****************************************************************************
;
; The following section will take the raw button data from a Joyport and
; update the appropriate bits in the return value d6.  The buttons that will
; be updated are FIRE, RIGHT, and MIDDLE.
;
;       On Entry:       a3 = ptr to custom register base
;                       a4 = ptr to ciaa register base
;                       d4 = bit mask for the Fire Button.
;                       d5 = mask for Output Enable for the middle button.

                move.w  d5,d1                               ;Assemble the mask for setting the buttons to one.
                lsr.w   #1,d1
                or.w    d5,d1
                move.w  d1,d0
                lsl.w   #2,d1
                or.w    d1,d0
                move.w  d0,d1
                JSRLIB  WritePotgo                          ;Clear any accumulated button state.

                                                            ;Get fire button.
                move.b  ciapra(a4),d1                       ;d1 gets fire button state
                btst    d4,d1
                bne.s   1$                                  ;All buttons are active low.

                ori.l   #JPF_BTN2,d6                        ;set fire button in temp reg
1$:                                                         ;Get right button.
                move.w  potinp(a3),d1                       ;d1 gets POTGOR data
                not.w   d1                                  ;make data high active
                move.w  d1,d2                               ;save for later use
                lsr.w   #1,d1                               ;Shift so we can use the output enable as the mask.
                and.w   d5,d1                               ;Clear all except right button
                beq.s   2$                                  ;Test the right button.

                ori.l   #JPF_BTN1,d6                        ;set right button in temp reg
2$:                                                         ;Get middle button.
                move.w  d2,d1                               ;d1 gets saved data
                lsl.w   #1,d1                               ;Shift so we can use the output enable as the mask.
                and.w   d5,d1                               ;Clear all except middle button.
                beq.s   3$                                  ;Test the middle button.

                ori.l   #JPF_BTN7,d6                        ;set middle button in temp reg
3$:             move.l  (a2),d2                             ;Get the prevous controller type.
                and.l   #(3<<28),d2                         ;There are only four possible values.
                beq.s   RJP_Unknown_State
                                                            ;We've masked it down to one of the four.
                rol.l   #5,d2                               ;Shift the type so it can be used as an offset into a jump table.
                move.w  RJP_State_JTBL-2(pc,d2.w),d2
                jmp     RJP_State_JTBL(pc,d2.w)

RJP_Return:                                                 ;Restore registers and exit.
                move.w  d7,2(a2)                            ;joyport reading for next time.
RJP_NotAvail_State:

RJP_1st_CTLR:   bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5)      ; clr for reentrance
RJP_NotEntered
                move.l  d6,d0
                movem.l (sp)+,d2-d7/a2-a6
                rts
;
; ReadJoyPort state jump table :
;
RJP_State_JTBL: dc.w    RJP_Unknown_State-RJP_State_JTBL        ;Was Game controller, now must be unknown.
                dc.w    RJP_Mouse_State-RJP_State_JTBL          ;Was mouse.
                dc.w    RJP_Joystk_State-RJP_State_JTBL         ;Was joystick.



; *****************************************************************************
;
; The following section will handle controller STATE = UNKNOWN.  The
; actions for this state are :
;
;       If there is a change in the port data try the joystick state.
;
;       On Entry:       a2 = ptr to port status word
;                       d6 = Current built return value.
;                       d7 = joyport data
;
;       On Exit:        Set new state.
;
RJP_Unknown_State:

                                                            ;Determine if Joy Port data has changed :
                cmp.w   2(a2),d7
                bne     ms_ChangeToJoy                      ;The joystick state will vector to the correct state based on the reading.

UnknownComplete:
                or.l    #JP_TYPE_UNKNOWN,d6                 ;Set the return state.
                move.w  #(JP_TYPE_UNKNOWN>>16),(a2)         ;Remember the state for next time.
                bra     RJP_Return                          ;Let's ge out of here.

; *****************************************************************************
;
; The following section will handle controller STATE = GAMECTLR.  The
; actions for this state are :
;
;       - Guard against someone inserting the mouse and holding down the
;         fire button.  This looks like a game controller, but we must be
;         transitioning from the unknown state.
;       - Get the buttons in the correct register and location.
;       - Reread the current position in case we are a double controller.
;       - Call subroutine to translate the direction values.
;       - Jump to common return location with return value in d6.
;
;       On Entry:       a2 = ptr to port status word
;                       d3 = button state.
;                       d7(high order word) = Offset of mouse counter register.
;
;       On Exit:        d6 updated with current controller state and jump to common exit
;
RJP_GameCtlr_State:

                move.w  #(JP_TYPE_GAMECTLR>>16),d0
                cmp.w   (a2),d0
                beq.s   RJP_FinishGame                      ;If the old value is a game controller no problem.
                                                            ;
                                                            ;The old value was not a game controller.

                move.w  d0,-(sp)
                move.w  d5,d1                               ; Clear the output enables that were probably set
                lsl.w   #2,d1                               ; by the joy/mouse read code.  For the game controller,
                or.w    d5,d1                               ; they should be INPUTS.
                moveq.l #0,d0                               ; They ought stay in this state until a possible
                JSRLIB  WritePotgo                          ; joystick/mouse comes into play.
                move.w  (sp)+,d0

                                                            ;Reclock so we don't get misled.
                                                            ;
                move.w  d0,(a2)                             ;Remember this type just incase.
                bra     RJP_Clock_Game_Ctlr
                                                            ;We're really sure we're a game controller.
RJP_FinishGame:
                swap    d3                                  ; move button data to return location
                move.l  d3,d6                               ; update return value
                swap    d7
                move.w  0(a3,d7.w),d7                       ;Read the joystick direction.  Needed for the double controller.
                bsr     RJP_Joy_Direction                   ; d0 gets direction values in 0..3
                or.w    d0,d6                               ; update return value
                ori.l   #JP_TYPE_GAMECTLR,d6                ; set controller type field
                bra     RJP_Return

; *****************************************************************************
;
; The following section will handle controller STATE = MOUSE.  The
; actions for this state are :
;
;               - No change do nothng.
;               - Check for a valid joystick.  If one is found decrement the
;                 counter.  When the counter gets to zero change state to
;                 joystick.
;
;       On Entry:       a2 = ptr to port status word
;                       d6 = Buttons pressed
;                       d7 = Joyport data
;
;       On Exit:        d6 updated with return value and jump to common exit
;
;       Regs used:      d0,a0
;
RJP_Mouse_State:
                                                            ;Determine if there is any change in port data.
                cmp.w   2(a2),d7
                beq.s   CompleteMouse
                                                            ;Data has changed check for possible joystick.
                bsr.s   RJP_Joy_Direction
                bne.s   RJP_Mouse_State_06                  ;If not a joystick reset counter and exit.
                                                            ;
                                                            ;Reading could be a joystick, check to see how
                                                            ;quickly it is changing.
                                                            ;
                move.l  ll_VBCounter(a5),a0
                move.l  (a0),d0                             ;Get the current VBlank count.
                sub.l   lljp_LastVBlank(a2),d0
                beq.s   ms_PossibleMouse                    ;Is it the same as the last VBlank count?
                                                            ;No the VBlank counter has changed.
                subq.l  #1,d0
                bge.s   ms_ManyVB                           ;Has at least one complete VBlank happened?
                                                            ;Yep.
                bset    #JP_STATUSB_WAS_JOY,lljp_Status(a2)     ;Remember there has been a joystick like event.
                bne.s   RJP_Mouse_State_06                  ;Two consecutive joystick values.  Not really a joystick.

ms_ManyVB:                                                  ;Ok we have a nice joystick value remember the
                                                            ;VBlank counter and decrement the count of joystick
                                                            ;events.  Once it reaches zero change to the
                                                            ;joystick state.
                                                            ;
                move.l  (a0),lljp_LastVBlank(a2)
                move.w  (a2),d0                             ;Get the count.
                subq.w  #1,d0
                cmp.w   #(JP_TYPE_MOUSE>>16),d0             ;The count also has the type, so if
                blt.s   ms_ChangeToJoy                      ;type changed the count went to -1.
                                                            ;Still a mouse.
                move.w  d0,(a2)                             ;Remember the new count.
                bra.s   ms_PossibleJoy

RJP_Mouse_State_06:                                         ;Not a joystick event remember the VBlank count.
                move.l  ll_VBCounter(a5),a0
                move.l  (a0),lljp_LastVBlank(a2)
ms_PossibleMouse:                                           ;A definite mouse event reset the counter and clear
                                                            ;the flag that show the last event was a joystick
                                                            ;event.
                move.w  #(JP_TYPE_MOUSE>>16)!JP_MOUSE_2_RECHECK,d0      ;reset counter.
                move.w  d0,(a2)                             ;and save it.
CompleteMouse:  and.b   #~JP_STATUSF_WAS_JOY,lljp_Status(a2)    ;Clear the joystick event flag.
ms_PossibleJoy: or.w    d7,d6                               ;Put mouse counters into the return value.
                ori.l   #JP_TYPE_MOUSE,d6                   ;Put the controller type into the return value.
                bra     RJP_Return                          ;Go to the common exit

ms_ChangeToJoy: move.w  #(JP_TYPE_JOYSTK>>16)!1,(a2)        ;Recheck, because it could be a game controller.


; *****************************************************************************
;
; The following subroutine will handle controller STATE = JOYSTK.  The
; actions for this state are :
;
;       - Call routine to get direction status and update return value
;       - Determine if data is valid for joystick :
;               - If not valid for joystick, clear counter and set state = MOUSE
;                 in saved state and jump to mouse state routine
;       - Decrement counter in saved status
;       - If the counter has not underflowed, return joystick type data
;               - underflow reset counter
;
;       On Entry:       a2 = ptr to port status word
;                       d6 = Buttons pressed
;                       d7 = Joyport data
;
;       On Exit:        d6 updated with return value and jump to common exit
;
;       Regs used:      d0
;
RJP_Joystk_State:
                                                            ;Call routine to get direction status.
                bsr.s   RJP_Joy_Direction                   ;d0 gets direction values in 0..3
                beq.s   RJP_Joystk_State_08                 ;Are the directions valid for joystick?
                                                            ;Data is invalid, go to mouse state.
                move.w  #(JP_TYPE_MOUSE>>16)|JP_MOUSE_2_RECHECK,(a2)
                bra     RJP_Mouse_State

RJP_Joystk_State_08:                                        ;Data is valid delay for several times before
                                                            ;checking for other controller.
                                                            ;
                move.w  d0,d6                               ;Save the directions into the return value.
                move.w  (a2),d0                             ;Get the count.
                sub.w   #1,d0
                cmp.w   #(JP_TYPE_JOYSTK>>16),d0            ;The count also has the type, so if
                bge.s   UpdateJoystk                        ;type changed the count went to -1.
                                                            ;Counter has gone to -1, reset.
                move.w  #(JP_TYPE_JOYSTK>>16)|JP_JOYSTK_2_RECHECK,d0
UpdateJoystk:   move.w  d0,(a2)                             ;Remember the counter.
                ori.l   #JP_TYPE_JOYSTK,d6                          ;set controller type field
                bra     RJP_Return

; *****************************************************************************
;
; The following subroutine will take the raw direction data from a Joy
; Port and update the appropriate bits for returning in d0.  The values
; will also be validated.
;
;       On Entry:       d7 = Reading from the joypot
;
;       On Exit:        d0 = bits 3..0 updated with direction status
;                       d1 = nonzero if data is not valid for joystick
;                       Cond Code = EQ if data is valid for joystick
;                       Cond Code = NE if data is not valid for joystick
;
;       Regs used:      d0,d1
;
RJP_Joy_Direction:
                move.w  d7,d1                               ; d1 gets port data
                move.w  d1,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d1                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d1           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d1                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d1                               ;Shift up,down to the correct place.
                or.w    d1,d0                               ;Assemble the answer in d0.
                move.b  d0,d1
                lsr.b   #1,d1                               ;line up left on top of right and up on top of down.
                and.b   #~JPF_JOY_LEFT,d1                   ;Don't want to be distracted by a diagonal to the left and down.
                and.b   d0,d1                               ;This will be non-zero if opposite directions are true.
                rts

; *****************************************************************************
;
; The following subroutine will be a server on the Vertical Blank
; interrupt list.  The purpose of this routine is to allow GAMEPORT server to
; run only when the ReadJoyPort function is NOT clocking the controller.
; A flag in the Gamebase will be set by RJP_Clock_Game_Ctlr routine during
; its critical clocking section.
; This server will be added by the RJP_First_Call_Init routine.
;
; The following actions will be taken :
;
;       - Determine if the Critical section is being executed by RJP_Clock_Game_Ctlr
;               - If yes, return
;               - If no, call GAMEPORT interrupt server
;
;       On Entry:       a1 = pointer to IS_DATA (game library base)
;                       a5 = vector to interrupt code
;
;       On Exit:        vector to GAMEPORT Vblank server or
;                       return to exec (Z flag must be set)
;
;       Regs used:      d0
;
RJP_VBlank_Int:
                                                            ;Determine if critical section is active.
                btst    #JP_STATUSB_CLOCKING_CTLR,ll_Port0State+lljp_Status(a1) ; critical section active ?
                bne.s   RJP_Vblank_Int_08                   ; if yes, j to skip GAMEPORT server
                                                            ;
                                                            ;Critical section not active, vector to GAMEPORT
                                                            ;server.
                                                            ;
                move.l  ll_RJP_Saved_IS(a1),a1              ; a1 gets pointer to GAMEPORT int structure
                move.l  IS_CODE(a1),a5                      ; a5 gets GAMEPORT int code vector
                move.l  IS_DATA(a1),a1                      ; a1 gets GAMEPORT int data pointer
                jmp     (a5)                                ; jump to GAMEPORT interrupt server

RJP_Vblank_Int_08:                                          ;Critical section is active, return to exec :
                moveq.l #0,d0                               ; set Z flag
                rts                                         ; return to exec

******* lowlevel.library/SetJoyPortAttrsA ***********************************
*
*   NAME
*	SetJoyPortAttrsA -- change the attributes of a port.  (V40.27)
*	SetJoyPortAttrs -- varargs stub for SetJoyPortAttrsA().  (V40.27)
*
*   SYNOPSIS
*	success = SetJoyPortAttrsA(portNumber, tagList);
*	D0                         D0          A1
*
*	BOOL SetJoyPortAttrsA(ULONG, struct TagItem *);
*
*	Success = SetJoyPortAttrs(portNumber, firstTag, ...);
*
*	BOOL SetJoyPortAttrs(Tag, ...);
*
*
*   FUNCTION
*	This function allows modification of several attributes held by
*	ReadJoyPort() about both it's operation and the type of controller
*	currently plugged into the port.
*
*	ReadJoyPort()'s default behavior is to attempt to automatically
*	sense the type of controller plugged into any given port, when
*	asked to read that port. This behavior is beneficial, to allow
*	simple detection of the type of controller plugged into the port.
*	Unfortunately, rare cases are possible where extremely fine
*	mouse movements appear to be real joystick movements. Also, this
*	ability to auto-sense the controller type causes most reads to
*	take longer than if there were no auto-sensing.
*
*	SetJoyPortAttrs() is intended to provide for both of these cases.
*	It allows the programmer to notify ReadJoyPort() to stop spending
*	time attempting to sense which type of controller is in use -- and,
*	optionally, to force ReadJoyPort() into utilizing a certain
*	controller type.
*
*   INPUTS
*	portNumber - the joyport in question (0-3).
*	tagList - a pointer to an array of tags providing parameters
*	          to SetJoyPortAttrs(); if NULL, the function
*	          will return TRUE, but do nothing.
*
*   TAGS
*	SJA_Type (ULONG) - Sets the current controller type to the mouse,
*			joystick, or game controller. Supply one of
*			SJA_TYPE_GAMECTLR, SJA_TYPE_MOUSE, SJA_TYPE_JOYSTK,
*			or SJA_TYPE_AUTOSENSE. If SJA_TYPE_AUTOSENSE is used,
*			ReadJoyPort() will attempt to determine the type of
*			controller plugged into the given port automatically.
*			If one of the other types is used, ReadJoyPort() will
*			make no attempt to read the given controller as
*			anything other than the type specified. The default
*			type is SJA_AUTOSENSE.
*
*	             	Note -- if you set the type to anything other than
*	             	auto-sense, it's your responsibility to return it
*	             	to auto-sense mode before exiting.
*
*	SJA_Reinitialize (VOID) - Return a given port to it's initial state,
*		     	forcing a port to deallocate any allocated resources;
*	             	return the implied type to SJA_TYPE_AUTOSENSE.
*
*   RESULT
*	success - TRUE if everything went according to plan, or FALSE upon
*		  failure
*
*   SEE ALSO
*	ReadJoyPort(), <libraries/lowlevel.h>
*
******************************************************************************
SetJoyPortAttrs:
                movem.l     d0/d2/a2/a3,-(sp)

* Save d0, a1, our parameters, on the stack, and then utilize some code snipped
* from ReadJoyPort() to initialize things.  Icky!  A stateless and state-full
* model all mixed up!  Don't blame me -- it's not my fault.  :'|

                movem.l     d0/a1/a5/a6,-(sp)
                move.l      a6,a5
                lea.l       ll_Port0State(a6),a2
                btst        #JP_STATUSB_VBLANK_ACQUIRED,lljp_Status(a2)
                bne.s       3$                                  ;Do we need to try getting the VBlank interrupt?
                                                                ;Yes, this must be the first time through here.
                move.l      ll_ExecBase(a5),a6
                                                                ;
                                                                ;Disable interrupts and find gameport.device on
                                                                ;the list.
                                                                ;
                move.l      IVVERTB+IV_DATA(a6),a0              ; a0 gets vert blank list
                lea         GAMEPORT_INT(pc),a1                 ; a1 gets ptr to name we're lookin fo
                JSRLIB      Forbid                              ;Don't want anyone else mucking with the list at the same time.
                JSRLIB      FindName                            ; d0 gets ptr to IS
                beq.s       2$                                  ;If not found, do nothing.
                                                                ;Entry found, save pointer and remove it from list.
                bset        #JP_STATUSB_VBLANK_ACQUIRED,lljp_Status(a2)     ; set acquired flag
                bclr        #JP_STATUSB_FIRST_INIT_REQ,lljp_Status(a2)
                move.l      d0,ll_RJP_Saved_IS(a5)              ; save ptr to gameport IS
                move.l      d0,a1                               ; a1 gets entry to remove
                moveq       #INTB_VERTB,d0                      ; d0 gets interrupt number
                JSRLIB      RemIntServer                        ; remove entry from list
                                                                ;Add ReadJoyPort's entry to the list.
                moveq       #INTB_VERTB,d0                      ; d0 gets interrupt number
                lea         ll_RJP_Vblank_IS(a5),a1             ; a1 gets ptr to our entry
                JSRLIB      AddIntServer                        ; add entry to list
2$:                                                             ;Enable interrupts.
                JSRLIB      Permit
3$
                movem.l     (sp)+,d0/a1/a5/a6

* And now ... the normal code for SetJoyPortAttrs():

                cmp.l       #3,d0                   * Check port # for vailidity.
                bls.s       0$                      * If okay, continue
                moveq.l     #0,d0                   * else, set return value to FALSE
                bra         999$                    * and branch to the exit.
0$

                add.b       d0,d0                   * times 2 -- each table entry=2
                lea.l       PortOffsetTable,a0      *
                move.w      0(a0,d0.w),d0           * get base offset for that struct
                lea.l       0(a6,d0.w),a3           * get ptr to correct state struct

                moveq.l     #0,d2                   * d2 = -1, 0, 1.  Defines whether to unlock, do nothing, or lock
                move.l      a1,-(sp)                * tstate = tagptr (tstate = longword on stack)
1$              move.l      sp,a2                   * save &tstate
                move.l      a6,-(sp)                * Save library base
                move.l      ll_UtilBase(a6),a6      * Get UtilityBase
                move.l      a2,a0                   * get &tstate
                JSRLIB      NextTagItem             * d0 = NextTagItem(&tstate)
                move.l      (sp)+,a6                * recover library base
                tst.l       d0                      * end of taglist?
                beq         100$                    * yep, leave.
                move.l      d0,a0

                cmp.l       #SJA_Type,ti_Tag(a0)    * Is this tag SJA_Type?
                bne.s       11$
                move.l      ti_Data(a0),d0          * Get new type
                bne.s       111$                    * If not SJA_TYPE_SENSE, goto 101$
                moveq.l     #-1,d2                  * Set flag to "unlock"
                bra.s       112$
111$
                moveq       #12,d1                  * If a specific type, shift it into
                lsl.w       d1,d0                   * the top 4 bits of the word
                move.w      lljp_PreviousValue(a3),d1  * get old type
                and.w       #$00ff,d1                  * Mask out type bits in the old
                and.w       #$ff00,d0                  * mask out anything other than type in the new
                or.w        d0,d1                      * add new type to old
                move.w      d1,lljp_PreviousValue(a3)  * save it in the structure
                moveq.l     #1,d2                   * Set flag to "lock"
112$

11$             cmp.l       #SJA_Reinitialize,ti_Tag(a0)
                bne.s       12$


                moveq.l     #0,d0
                move.l      d0,lljp_RJPVector(a3)   * Clear jump vector (reset to autosense)

* Free potgo bits, if allocated
                FORBID
                bset        #JP_STATUSB_FIRST_INIT_REQ,lljp_Status(a3)
                bne.s       113$

                move.l      4(sp),d0                * Get port #
                bne.s       114$
                moveq.l     #0,d0
                move.w      #$0000,d0               * Port 0 bits can't be freed -- we don't own them.  What a kludge!!!
                bra.s       115$
114$            cmp.l       #1,d0                   * Port 1?
                bne.s       113$
                moveq.l     #0,d0
                move.w      #$F000,d0               * Port 1 bits
115$            move.l      a6,-(sp)
                move.l      ll_PotGoResource(a6),a6 * Get PotGoBase
                JSRLIB      FreePotBits
                move.l      (sp)+,a6
113$            PERMIT

12$

99$             bra         1$                      * Loop for next
100$
                tst.b       d2                      * What should we do?
                beq.s       101$                    * nothing ... to 101$
                bmi.s       102$                    * unlock ... to 102$
;                                                   * lock ... fall through
* Given port # and controller type, secure from the jump table the correct
* address for users to go to.
* Port = 0 to 3, controller type is also 0 to 3 (0 is worthless)
* Table entry = Port*4+Type
                moveq       #12,d0
                move.w      lljp_PreviousValue(a3),d1
                lsr.w       d0,d1                   * Controller type shift down
                move.l      4(sp),d0                * Fetch port # from the stack
                lsl.b       #2,d0                   * Port * 4
                or.b        d1,d0                   * d0 = table entry #
                lsl.b       #1,d0                   * times two, as each entry is a word
                lea.l       LockJMPTable,a0
                move.w      0(a0,d0.w),d0           * Get offset from table
                lea.l       0(a0,d0.w),a0           * Add that offset to our table ptr, get real ptr
                move.l      a0,lljp_RJPVector(a3)   * Store the pointer.

                bra.s       101$


102$            moveq.l     #0,d0                   * NULL out ptr, therefore UNLOCK
                move.l      d0,lljp_RJPVector(a3)   * And fall through ...

101$
                addq.l      #4,sp                   * remove tstate storage

                moveq.l     #1,d0                   * return TRUE

999$            movem.l     (sp)+,d0/d2/a2/a3
                rts

LockJMPTable:
                dc.w        Nothing-LockJMPTable    * Port 0, Controller unknown
                dc.w        Game0Lck-LockJMPTable   * Port 0, Game Controller
                dc.w        Mouse0Lck-LockJMPTable  * Port 0, Mouse
                dc.w        Joy0Lck-LockJMPTable    * Port 0, Joystick

                dc.w        Nothing-LockJMPTable    * Port 1, Controller unknown
                dc.w        Game1Lck-LockJMPTable   * Port 1, Game Controller
                dc.w        Mouse1Lck-LockJMPTable  * Port 1, Mouse
                dc.w        Joy1Lck-LockJMPTable    * Port 1, Joystick

                dc.w        Nothing-LockJMPTable    * Port 2, Controller unknown
                dc.w        Game2Lck-LockJMPTable   * Port 2, Game Controller
                dc.w        Nothing-LockJMPTable    * Port 2, Mouse (Not possible)
                dc.w        Nothing-LockJMPTable    * Port 2, Joystick (Not possible)

                dc.w        Nothing-LockJMPTable    * Port 3, Controller unknown
                dc.w        Game3Lck-LockJMPTable   * Port 3, Game Controller
                dc.w        Nothing-LockJMPTable    * Port 3, Mouse (Not possible)
                dc.w        Nothing-LockJMPTable    * Port 3, Joystick (Not possible)


** Nothing entry point -- for when the controller type is unknown;
**                        simply return.
Nothing:
                moveq.l     #0,d0
                rts

** Mouse1Lck -- Mouse (locked), port 1
**
Mouse1Lck:
                movem.l     d2/d4/a3-a5/a6,-(sp)
                move.l      a6,a5
                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure
                bne.s       999$

                move.w      #$f000,d1                       * Make the middle button an INPUT
                move.w      d1,d0
                move.l      ll_PotGoResource(a6),a6
                JSRLIB      WritePotgo

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT1,d4

                moveq.l     #0,d2
                move.b      ciapra(a4),d1
                btst        d4,d1
                bne.s       1$
                or.l        #JPF_BTN2,d2
1$

                move.w      potinp(a3),d1
                btst        #12,d1
                bne.s       2$
                or.l        #JPF_BTN7,d2
2$

                btst        #14,d1
                bne.s       3$
                or.l        #JPF_BTN1,d2
3$
                move.l      #JP_TYPE_MOUSE,d0
                move.w      joy1dat(a3),d0
                or.l        d2,d0

                bclr        #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure

                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts

999$            moveq.l     #0,d0
                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts


** Mouse0Lck -- Mouse (locked), port 0
**
Mouse0Lck:
                movem.l     d2/d4/a3-a5/a6,-(sp)
                move.l      a6,a5
                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure
                bne.s       999$

                move.w      #$f00,d1                       * Make the middle button an INPUT
                move.w      #$f00,d0
                move.l      ll_PotGoResource(a6),a6
                JSRLIB      WritePotgo

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT0,d4

                moveq.l     #0,d2
                move.b      ciapra(a4),d1
                btst        d4,d1
                bne.s       1$
                or.l        #JPF_BTN2,d2
1$

                move.w      potinp(a3),d1
                btst        #8,d1
                bne.s       2$
                or.l        #JPF_BTN7,d2
2$

                btst        #10,d1
                bne.s       3$
                or.l        #JPF_BTN1,d2
3$
                move.l      #JP_TYPE_MOUSE,d0
                move.w      joy0dat(a3),d0
                or.l        d2,d0

                bclr        #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure

                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts

999$            moveq.l     #0,d0
                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts


** Joy0Lck -- Joystick (locked), port 0
**
Joy0Lck:
                movem.l     d2/d4/a3-a5/a6,-(sp)
                move.l      a6,a5
                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure
                bne.s       999$

                move.w      #$f00,d1                       * Make the middle button an INPUT
                move.w      #$f00,d0
                move.l      ll_PotGoResource(a6),a6
                JSRLIB      WritePotgo

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT0,d4

                moveq.l     #0,d2
                move.b      ciapra(a4),d1
                btst        d4,d1
                bne.s       1$
                or.l        #JPF_BTN2,d2
1$

                move.w      potinp(a3),d1
                btst        #8,d1
                bne.s       2$
                or.l        #JPF_BTN7,d2
2$

                btst        #10,d1
                bne.s       3$
                or.l        #JPF_BTN1,d2
3$

                move.w  joy0dat(a3),d1                      ; Read UDLR

                move.w  d1,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d1                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d1           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d1                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d1                               ;Shift up,down to the correct place.
                or.w    d1,d0                               ;Assemble the answer in d0.

                or.l    d2,d0                               ; combine buttons & switches
                or.l    #JP_TYPE_JOYSTK,d0                  ; code = game controller

                bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure

                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts

999$            moveq.l     #0,d0
                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts



** Joy1Lck -- Joystick (locked), port 1
**
Joy1Lck:
                movem.l     d2/d4/a3-a5/a6,-(sp)
                move.l      a6,a5
                moveq.l     #0,d0
                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure
                bne.s       999$

                move.w      #$f000,d1                       * Make the middle button an INPUT
                move.w      #$f000,d0
                move.l      ll_PotGoResource(a6),a6
                JSRLIB      WritePotgo

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT1,d4

                moveq.l     #0,d2
                move.b      ciapra(a4),d1
                btst        d4,d1
                bne.s       1$
                or.l        #JPF_BTN2,d2
1$

                move.w      potinp(a3),d1
                btst        #12,d1
                bne.s       2$
                or.l        #JPF_BTN7,d2
2$

                btst        #14,d1
                bne.s       3$
                or.l        #JPF_BTN1,d2
3$

                move.w  joy1dat(a3),d1                      ; Read UDLR

                move.w  d1,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d1                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d1           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d1                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d1                               ;Shift up,down to the correct place.
                or.w    d1,d0                               ;Assemble the answer in d0.

                or.l    d2,d0                               ; combine buttons & switches
                or.l    #JP_TYPE_JOYSTK,d0                  ; code = game controller

                bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status+ll_Port0State(a5) ;Clear that bit in lljp_Status of the Port 0 jp structure

                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts

999$            moveq.l     #0,d0
                movem.l     (sp)+,d2/d4/a3-a5/a6
                rts



** Game0Lck -- Game controller (locked), port 0.
**
Game0Lck:
                movem.l     a2-a4/a6/d2-d4,-(sp)
                lea.l       ll_Port0State(a6),a2

                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status(a2)
                bne         99$

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT0,d4



                bset    #JP_STATUSB_CLOCKING_CTLR,lljp_Status(a2)       ; critical section flag

                bset    d4,ciaddra(a4)                      ;Set direction for fire button.
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;
                                                            ;Enable game controller mux (enable the middle
                                                            ;button as an output and send it a low).
                                                            ;

                move.l  ll_PotGoResource(a6),a6
                move.w  #$200,d0                            ;Output a 0.
                move.w  #$300,d1
                JSRLIB  WritePotgo
                move.l  #_custom+potinp,a0                  ;Get read address into a register this decreases the time in the loop.
                move.w  #$400,d1
                                                            ; Set loop counter and clear temp data register.
                CLEAR   d3
                moveq   #6,d0                               ; d0 is loop counter
                bra.s   1$
                                                            ; Loop for all bits :
0$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
1$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                move.w  (a0),d2                             ; d2 gets POTGOR data
                                                            ;Clock for next value.
                bset    d4,ciapra(a4)                       ;Make the clock high
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;Munge the button data bit into d3.
                and.w   d1,d2
                bne.s   6$                                  ;Buttons are active low.
                                                            ;The button was pressed.
                bset    d0,d3                               ;set button down in temp reg.
6$:             dbra    d0,0$                               ; loop for all bits
                                                            ;
                                                            ;Disable game controller mux (enable the middle
                                                            ;button as an output and send it a high).
                                                            ;
                move.w  #$300,d0
                move.w  d0,d1
                JSRLIB  WritePotgo
                bclr    d4,ciaddra(a4)                      ;Restore direction of fire button to input :
                bclr    #JP_STATUSB_CLOCKING_CTLR,lljp_Status(a2)       ; critical section flag

*	        lsr.w   #1,d3                               ; dump worthless bit.
                lsl.w   #1,d3
                swap    d3                                  ; move buttons to high word

                move.w  joy0dat(a3),d1                      ; Read UDLR

                move.w  d1,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d1                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d1           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d1                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d1                               ;Shift up,down to the correct place.
                or.w    d1,d0                               ;Assemble the answer in d0.

                or.l    d3,d0                               ; combine buttons & switches
                or.l    #JP_TYPE_GAMECTLR,d0                ; code = game controller

                bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status(a2)
99$
                movem.l (sp)+,a2-a4/a6/d2-d4
                rts


** Game1Lck -- Game controller (locked), port 1.
**
Game1Lck:
                movem.l     a2-a4/a6/d2-d4,-(sp)
                lea.l       ll_Port1State(a6),a2

                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status-ll_Port1State+ll_Port0State(a2)
                bne         99$

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT1,d4



                bset    #JP_STATUSB_CLOCKING_CTLR,lljp_Status-ll_Port1State+ll_Port0State(a2)       ; critical section flag

                bset    d4,ciaddra(a4)                      ;Set direction for fire button.
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;
                                                            ;Enable game controller mux (enable the middle
                                                            ;button as an output and send it a low).
                                                            ;

                move.l  ll_PotGoResource(a6),a6
                move.w  #$2000,d0                            ;Output a 0.
                move.w  #$3000,d1
                JSRLIB  WritePotgo
                move.l  #_custom+potinp,a0                  ;Get read address into a register this decreases the time in the loop.
                move.w  #$4000,d1
                                                            ; Set loop counter and clear temp data register.
                CLEAR   d3
                moveq   #6,d0                               ; d0 is loop counter
                bra.s   1$
                                                            ; Loop for all bits :
0$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
1$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                move.w  (a0),d2                             ; d2 gets POTGOR data
                                                            ;Clock for next value.
                bset    d4,ciapra(a4)                       ;Make the clock high
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;Munge the button data bit into d3.
                and.w   d1,d2
                bne.s   6$                                  ;Buttons are active low.
                                                            ;The button was pressed.
                bset    d0,d3                               ;set button down in temp reg.
6$:             dbra    d0,0$                               ; loop for all bits
                                                            ;
                                                            ;Disable game controller mux (enable the middle
                                                            ;button as an output and send it a high).
                                                            ;
                move.w  #$3000,d0
                move.w  d0,d1
                JSRLIB  WritePotgo
                bclr    d4,ciaddra(a4)                      ;Restore direction of fire button to input :
                bclr    #JP_STATUSB_CLOCKING_CTLR,lljp_Status-ll_Port1State+ll_Port0State(a2)       ; critical section flag

*	        lsr.w   #1,d3                               ; dump worthless bit.
                lsl.w   #1,d3
                swap    d3                                  ; move buttons to high word

                move.w  joy1dat(a3),d1                      ; Read UDLR

                move.w  d1,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d1                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d1           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d1                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d1                               ;Shift up,down to the correct place.
                or.w    d1,d0                               ;Assemble the answer in d0.

                or.l    d3,d0                               ; combine buttons & switches
                or.l    #JP_TYPE_GAMECTLR,d0                ; code = game controller

                bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status-ll_Port1State+ll_Port0State(a2)
99$
                movem.l (sp)+,a2-a4/a6/d2-d4
                rts


** Game2Lck -- Game controller (locked), port 2.  A Double Controller.  2nd from 0.
**
Game2Lck:
                movem.l     a2-a4/a6/d2-d4,-(sp)
                lea.l       ll_Port2State(a6),a2

                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status-ll_Port2State+ll_Port0State(a2)
                bne         99$

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT0,d4



                bset    #JP_STATUSB_CLOCKING_CTLR,lljp_Status-ll_Port2State+ll_Port0State(a2)       ; critical section flag

                bset    d4,ciaddra(a4)                      ;Set direction for fire button.
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;
                                                            ;Enable game controller mux (enable the middle
                                                            ;button as an output and send it a low).
                                                            ;

                move.l  ll_PotGoResource(a6),a6
                move.w  #$200,d0                            ;Output a 0.
                move.w  #$300,d1
                JSRLIB  WritePotgo
                move.l  #_custom+potinp,a0                  ;Get read address into a register this decreases the time in the loop.
                move.w  #$400,d1
                                                            ; Set loop counter and clear temp data register.
                CLEAR   d3
                moveq   #17,d0                              ; d0 is loop counter
                bra.s   1$
                                                            ; Loop for all bits :
0$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
1$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                move.w  (a0),d2                             ; d2 gets POTGOR data
                                                            ;Clock for next value.
                bset    d4,ciapra(a4)                       ;Make the clock high
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;Munge the button data bit into d3.
                and.w   d1,d2
                bne.s   6$                                  ;Buttons are active low.
                                                            ;The button was pressed.
                bset    d0,d3                               ;set button down in temp reg.
6$:             dbra    d0,0$                               ; loop for all bits
                                                            ;
                                                            ;Disable game controller mux (enable the middle
                                                            ;button as an output and send it a high).
                                                            ;
                move.w  joy0dat(a3),d2                      ; Read UDLR
;                                                           ; Read UDLR here.  Dan tells me that
;                                                           ; duals are likely to multiplex between
;                                                           ; the two UDLR sets between clocking out
;                                                           ; the first and second sets of 8 bits.

                move.w  #$300,d0
                move.w  d0,d1
                JSRLIB  WritePotgo
                bclr    d4,ciaddra(a4)                      ;Restore direction of fire button to input :
                bclr    #JP_STATUSB_CLOCKING_CTLR,lljp_Status-ll_Port2State+ll_Port0State(a2)       ; critical section flag

*	        lsr.w   #1,d3                               ; dump worthless bit.
                lsl.w   #1,d3
                swap    d3                                  ; move buttons to high word


                move.w  d2,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d2                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d2           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d2                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d2                               ;Shift up,down to the correct place.
                or.w    d2,d0                               ;Assemble the answer in d0.

                or.l    d3,d0                               ; combine buttons & switches
                or.l    #JP_TYPE_GAMECTLR,d0                ; code = game controller

                bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status-ll_Port2State+ll_Port0State(a2)
99$
                movem.l (sp)+,a2-a4/a6/d2-d4
                rts


** Game3Lck -- Game controller (locked), port 3.  A double controller.  2nd from 1.
**
Game3Lck:
                movem.l     a2-a4/a6/d2-d4,-(sp)
                lea.l       ll_Port3State(a6),a2

                bset        #JP_STATUSB_ALREADYRUNNING,lljp_Status-ll_Port3State+ll_Port0State(a2) ;Test & set that bit in lljp_Status of the Port 0 jp structure
                bne         99$

                move.l      #_custom,a3
                move.l      #_ciaa,a4
                move.w      #CIAB_GAMEPORT1,d4



                bset    #JP_STATUSB_CLOCKING_CTLR,lljp_Status-ll_Port3State+ll_Port0State(a2)       ; critical section flag

                bset    d4,ciaddra(a4)                      ;Set direction for fire button.
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;
                                                            ;Enable game controller mux (enable the middle
                                                            ;button as an output and send it a low).
                                                            ;

                move.l  ll_PotGoResource(a6),a6
                move.w  #$2000,d0                           ;Output a 0.
                move.w  #$3000,d1
                JSRLIB  WritePotgo
                move.l  #_custom+potinp,a0                  ;Get read address into a register this decreases the time in the loop.
                move.w  #$4000,d1
                                                            ; Set loop counter and clear temp data register.
                CLEAR   d3
                moveq   #17,d0                              ; d0 is loop counter
                bra.s   1$
                                                            ; Loop for all bits :
0$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
1$
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                tst.b   ciapra(a4)
                move.w  (a0),d2                             ; d2 gets POTGOR data
                                                            ;Clock for next value.
                bset    d4,ciapra(a4)                       ;Make the clock high
                bclr    d4,ciapra(a4)                       ;Lower the clock.
                                                            ;Munge the button data bit into d3.
                and.w   d1,d2
                bne.s   6$                                  ;Buttons are active low.
                                                            ;The button was pressed.
                bset    d0,d3                               ;set button down in temp reg.
6$:             dbra    d0,0$                               ; loop for all bits
                                                            ;

                move.w  joy0dat(a3),d2                      ; Read UDLR
;                                                           ; Read UDLR here.  Dan tells me that
;                                                           ; duals are likely to multiplex between
;                                                           ; the two UDLR sets between clocking out
;                                                           ; the first and second sets of 8 bits.

                                                            ;Disable game controller mux (enable the middle
                                                            ;button as an output and send it a high).
                                                            ;
                move.w  #$3000,d0
                move.w  d0,d1
                JSRLIB  WritePotgo
                bclr    d4,ciaddra(a4)                      ;Restore direction of fire button to input :
                bclr    #JP_STATUSB_CLOCKING_CTLR,lljp_Status-ll_Port3State+ll_Port0State(a2)       ; critical section flag

*	        lsr.w   #1,d3                               ; dump worthless bit.
                lsl.w   #1,d3
                swap    d3                                  ; move buttons to high word

                move.w  d2,d0
                lsr.w   #1,d0                               ;Move left and write over top of up and down.
                eor.w   d0,d2                               ;Derive up and down.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d2           ;Mask out non-joystick bits.
                and.w   #JOYDATF_Y0|JOYDATF_X0,d0           ;Mask out non-joystick bits.
                ror.b   #1,d0                               ;Roll right to bit 7 so it is adjacent to left.
                ror.b   #1,d2                               ;Roll down to bit 7 so it is adjacent to up.
                lsr.w   #7,d0                               ;Shift left,right to the correct places.
                lsr.w   #5,d2                               ;Shift up,down to the correct place.
                or.w    d2,d0                               ;Assemble the answer in d0.

                or.l    d3,d0                               ; combine buttons & switches
                or.l    #JP_TYPE_GAMECTLR,d0                ; code = game controller

                bclr    #JP_STATUSB_ALREADYRUNNING,lljp_Status-ll_Port3State+ll_Port0State(a2) ;Clear that bit in lljp_Status of the Port 0 jp structure
99$
                movem.l (sp)+,a2-a4/a6/d2-d4
                rts





GAMEPORT_INT:   dc.b    'gameport.device',0


        END
