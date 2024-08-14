******************************************************************************
*
*       $Id: keyboard.asm,v 40.11 93/07/30 16:07:39 vertex Exp $
*
******************************************************************************
*
*       $Log:	keyboard.asm,v $
* Revision 40.11  93/07/30  16:07:39  vertex
* Autodoc and include cleanup
* 
* Revision 40.10  93/05/05  09:54:40  gregm
* autodoc spelling errors fixed.
*
* Revision 40.9  93/04/20  14:16:10  Jim2
* Corrected AddKBInt behaviour when it fails to add the interrupt.
* Polished the comments.
*
* Revision 40.8  93/03/31  18:44:30  Jim2
* I should know better than to check it in before testing.  I coded
* up the port stuff incorrectly.
*
* Revision 40.7  93/03/31  18:14:41  Jim2
* The RKMs documente a6 as being scratch in an interrupt.  This
* is not the case in KeyboardExtender (CI 106).  Also I forgot
* to add the port to this DoIO.
*
* Revision 40.6  93/03/23  14:41:12  Jim2
* Polished autodocs.
*
* Revision 40.5  93/03/10  12:22:18  Jim2
* Cleaned up the results of Vertex code walkthrough.
*
* Revision 40.4  93/03/04  11:58:06  Jim2
* Oops, tst.l cannot be done to an address register unless 020 or better.
*
* Revision 40.3  93/03/04  11:45:40  Jim2
* AddKBInt properly returns a NULL when it does not allocate
* a new interrupt.  RemKBInt checks for a NULL before it does
* it's work.
*
* Revision 40.2  93/03/02  13:25:09  Jim2
* Changed all of the references from game.library to lowlevel.library
*
* Revision 40.1  93/03/01  16:02:18  Jim2
* Added NOTEs to the autodoc to warn people away from these routines.
*
* Revision 40.0  93/02/19  16:48:18  Jim2
* Corrected autodoc format for SEE ALSO.
*
* Revision 39.4  93/01/18  13:24:31  Jim2
* Cleaned up comments.  In QueryKeys require one less register.
*
* Revision 39.3  93/01/15  13:48:39  Jim2
* Removed the keyboard repeat stuff (KeySetThresh and KeySetPeriod)
* this simplified GetKeys.
*
* Revision 39.2  93/01/13  13:38:23  Jim2
* Dropped LastKey and PressedKeys.  Moved the interrupt extender
* from the startup routine.  Added KeySetThresh, KeySetPeriod
* and GetKey.  PressedKeys was superceeded by QueryKeys.  The setting of the
* delays was not tested.
*
* Revision 39.1  93/01/07  14:20:24  Jim2
* Tested and corrected.
*
* Revision 39.0  93/01/05  11:59:40  Jim2
* Initial Release - Untested.
*
*
*       (C) Copyright 1993 Commodore-Amiga, Inc.
*           All Rights Reserved
*
******************************************************************************

        INCLUDE 'exec/macros.i'
        INCLUDE 'exec/nodes.i'
        INCLUDE 'exec/interrupts.i'
        INCLUDE 'exec/memory.i'
        INCLUDE 'exec/io.i'
        INCLUDE 'exec/tasks.i'

        INCLUDE 'devices/keyboard.i'

        INCLUDE '/macros.i'
        INCLUDE '/lowlevelbase.i'


                XREF    _ciaasdr

                XDEF    GetKey
                XDEF    QueryKeys
                XDEF    AddKBInt
                XDEF    RemKBInt
                XDEF    KeyboardInterruptExtender

******* lowlevel.library/QueryKeys *******************************************
*
*   NAME
*	QueryKeys -- return the states for a set of keys. (V40)
*
*   SYNOPSIS
*	QueryKeys(queryArray, arraySize);
*	          A0          D1
*
*	VOID QueryKeys(struct KeyQuery *, UBYTE);
*
*   FUNCTION
*	Scans the keyboard to determine which of the rawkey codes
*	listed in the QueryArray are currently pressed. The state for each
*	key is returned in the array.
*
*	This function may be invoked from within an interrupt, but the size
*	of QueryArray should be kept as small as possible.
*
*	This is a low level function that does not fit the normal Amiga
*	multitasking model. The values returned have no knowledge
*	of which window/screen currently has input focus.
*
*   INPUTS
*	queryArray - an array of KeyQuery structures. The kq_KeyCode fields
*		     of these structures should be filled with the rawkey
*		     codes you wish to query about. Upon return from this
*		     function, the kq_Pressed field of these structures
*		     will be set to TRUE if the associated key is down,
*	             and FALSE if not.
*	arraySize - number of key code entries in queryArray
*
*   SEE ALSO
*	<libraries/lowlevel.h>
*
******************************************************************************
QueryKeys
                movem.l d2/a2/a5/a6,-(sp)
                move.l  a6,a5
                move.b  d1,d2                   ;Store the array size.
                move.l  a0,a2                   ;Store the array pointer.
                                ;Set up a port structure in case the DoIO blocks.
                move.l  ll_ExecBase(a6),a6
                CLEARA  a1
                JSRLIB  FindTask
                move.l  ll_ERSetFunction(a5),a0
                move.l  d0,MP_SIGTASK(a0)       ; SigTask
                move.b  #SIGB_SINGLE,MP_SIGBIT(a0)      ; SigBit
                move.b  #PA_SIGNAL,MP_FLAGS(a0) ; Signal...
TEMPSIZE        set     0
                ARRAYVAR Request,IOSTD_SIZE     ;Need an IO Request.
                ARRAYVAR Matrix,16              ;And room for the keyboard matrix.
                ALLOCLOCALS sp
                move.l  a0,Request+MN_REPLYPORT(sp)
                move.l  sp,a1                   ;Pointer to IO request.
                move.l  ll_KBDevice(a5),IO_DEVICE(a1)
                move.w  #KBD_READMATRIX,IO_COMMAND(a1)
                lea     Matrix(sp),a0           ;Pointer to the keyboard matrix.
                move.l  a0,IO_DATA(a1)
                move.l  #16,IO_LENGTH(a1)       ;Largest possible value.  This could be a problem with kickstart v33/v34.
                JSRLIB  DoIO                    ;Get the keyboard matrix.  This will return immediately.
                tst.b   d0
                bne.s   Exit

Loop:           move.w  (a2)+,d1                ;Get raw keycode to be tested.
                move.w  d1,d0
                and.w   #$0007,d0               ;Which bit within the byte.
                lsr     #3,d1                   ;Which Byte within the matrix
                btst    d0,Matrix(sp,d1.w)

                                ;The following four lines of code can be replaced
                                ;by this line twice.    sne     (a2)+


                beq.s   KeyIsUp
                move.w  #$FFFF,(a2)+            ;Key down
                bra.s   LoopTest
KeyIsUp:        move.w  #$0000,(a2)+            ;Key up
LoopTest:       subq.b  #1,d2                   ;Completed test for one keycode.
                bne.s   Loop
Exit:           move.l  Request+MN_REPLYPORT(sp),a0
                DONELOCALS sp
                move.b  #PA_IGNORE,MP_FLAGS(a0) ;Don't even think about using the port without changing the task.
                movem.l (sp)+,d2/a2/a5/a6
                rts

******* lowlevel.library/AddKBInt ********************************************
*
*   NAME
*	AddKBInt -- adds a routine to the keyboard interrupt. (V40)
*
*   SYNOPSIS
*	intHandle = AddKBInt(intRoutine, intData);
*	D0                   A0          A1
*
*	APTR AddKBInt(APTR, APTR);
*
*   FUNCTION
*	This routine extends the functionality of the keyboard interrupt to
*	include intRoutine. Since this is an extention of the normal
*	keyboard interrupt all of the keyboard handshaking is handled. The
*	keyboard error codes are filtered out and not passed to intRoutine.
*
*	The routine is called whenever the user enters a key on the
*	keyboard.
*
*	The routine is called from within an interrupt, so normal
*	restrictions apply. The routine must preserve the following
*	registers: A2, A3, A4, A7, D2-D7. Other registers are
*	scratch, except for D0, which MUST BE SET TO 0 upon
*	exit. On entry to the routine, A1 holds 'intData' and A5
*	holds 'intRoutine', and D0 contains the rawkey code read
*	from the keyboard.
*
*	The routine is not called when a reset is received from the
*	keyboard.
*
*	This is a low level function that does not fit the normal Amiga
*	multitasking model. The interrupt installed will have no knowledge
*	of which window/screen currently has input focus.
*
*	If your program is to exit without reboot, you MUST call RemKBInt()
*	before exiting.
*
*	Only one interrupt routine may be added to the system.  ALWAYS
*	check the return value in case some other task has previously
*	used this function.
*
*   INPUTS
*	intRoutine - the routine to invoke every vblank. This routine should
*		     be as short as possible to minimize its effect on overall
*		     system performance.
*	intData - data passed to the routine in register A1. If more than one
*		  long word of data is required this should be a pointer to
*		  a structure that contains the required data.
*
*   RESULT
*	intHandle - a handle used to manipulate the interrupt, or NULL
*		    if it was not possible to attach the routine.
*
*   SEE ALSO
*	RemKBInt()
*
******************************************************************************
AddKBInt
                movem.l a0-a1/a5-a6,-(sp)
                move.l  a6,a5
                move.l  ll_ExecBase(a5),a6
                move.l  #IS_SIZE,d0
                move.l  #MEMF_PUBLIC|MEMF_CLEAR,d1
                JSRLIB  AllocMem                ;Get memory for an interrupt structure.
                tst.l   d0
                beq.s   akbi_NoMem

                move.l  d0,a1
                move.b  #NT_INTERRUPT,LN_TYPE(a1)
                move.l  (sp)+,IS_CODE(a1)
                move.l  (sp)+,IS_DATA(a1)       ;Initialize the structure.
                move.l  LN_NAME(a5),LN_NAME(a1)
                lea     ll_KBInterrupts(a5),a0
                JSRLIB  Forbid                  ;Forbid does not change a0,a1
                IFNOTEMPTY a0,akbi_ErrorExit    ;Limit ourselves to one interrupt.
                ADDHEAD                         ;Add it to the list of interrupts.
                JSRLIB  Permit                  ;Permit does not change a1
                move.l  a1,d0                   ;The pointer to the interrupt structure is returned.
                movem.l (sp)+,a5-a6
                rts

akbi_ErrorExit: JSRLIB  Permit                  ;Permit does not change a1
                move.l  #IS_SIZE,d0
                JSRLIB  FreeMem                 ;Didn't add it so return the memory.
                CLEAR   d0
                subq.l  #8,sp
akbi_NoMem      movem.l (sp)+,a0-a1/a5-a6
                rts

******* lowlevel.library/RemKBInt ********************************************
*
*  NAME
*	RemKBInt -- remove a previously installed keyboard interrupt. (V40)
*
*  SYNOPSIS
*	RemKBInt(intHandle);
*	         A1
*
*	VOID RemKBInt(APTR);
*
*  FUNCTION
*	Remove a keyboard interrupt routine previously added with AddKBInt().
*
*  INPUTS
*	intHandle - handle obtained from AddKBInt(). This may be NULL,
*		    in which case this function does nothing.
*
*  SEE ALSO
*	AddKBInt()
*
******************************************************************************
RemKBInt
                cmp.l   #0,a0
                beq.s   rki_Exit                ;Check for NULL.
                                ;Non-Null handle.
                move.l  a6,-(sp)
                move.l  ll_ExecBase(a6),a6
                move.l  a1,-(sp)                ;Although Forbid() and Permit()...
                JSRLIB  Forbid                  ;... don't change a1 REMOVE does.
                REMOVE
                JSRLIB  Permit
                move.l  (sp)+,a1
                move.l  #IS_SIZE,d0
                JSRLIB  FreeMem                 ;Free the memory.
                move.l  (sp)+,a6
rki_Exit:       rts

*****i* lowlevel.library/KeyboardInterruptExtender ***************************
*
*   NAME
*	KeyboardInterruptExtender - Extends the functionality of the keyboard
*	                            interrupt.
*
*   SYNOPSIS
*	KeyBoardInterrupt(LowLevelBase)
*	                  a1
*
*   FUNCTION
*	This is a kind of SetFunction for the keyboard interrupt.  During the
*	development of the lowlevel library it was decided to add functionality
*	the the existing keyboard.device.  In particular it was desireable to
*	get the last key hit and piggyback on top the the keyboard interrupt.
*
*	These two functions were best handled by replacing the keyboard
*	interrupt.  However, this is not practical for a library that is
*	intended to work on many platforms.  So this routine was developed to
*	place a wrapper around the existing keyboard interrupt.
*
*	When invoked this routine grabs the current keyboard key.  Then the
*	orginal keyboard interrupt is invoked to handle all of the hand
*	shaking.  Once this is done the key is checked.  If it is a modifier
*	its bit within last KeyPressed is toggled.  If it is a non-modifier
*	keypress, it is stored as the last key press.  If the code is a key
*	release it is checked against the last key pressed.  If they match
*	the last key pressed is changed to none.  As long as the key code is
*	not a keyboard error the list of user keyboard interrupts is called.
*
*	This is a low level function that does not fit the normal Amiga
*	multitasking model. The values returned by this function are
*	not modified by which window/screen currently has input focus.
*
*   INPUTS
*	LowLevelBase - Pointer to lowlevel.library.  This is not in a6 since this
*	           function is an interrupt.
*
*   RESULT
*	NONE
******************************************************************************
KeyboardInterruptExtender
                movem.l d2/a4-a6,-(sp)
                move.l  a1,a4                   ;Get LowLevel base into a4.
                moveq   #0,d2                   ;clear upper byte
                move.b  _ciaasdr,d2             ;get the key.
                not.b   d2                      ;signal is active low
                ror.b   #1,d2                   ;move up/down to sign bit
                move.l  ll_KBOrig(a4),a0
                move.l  IS_DATA(a0),a1
                move.l  IS_CODE(a0),a5
                JSR     (A5)                    ;Execute the original keyboard interrupt.
                cmp.w   #$78,d2                 ;$78 is reset don't give it to the user.
                beq.b   NoChange
                                ;Not reset.
                move.w  d2,d1                   ;Checking to see if it is modifier key trashes the keycode.
                and.w   #$78,d1                 ;Modifier keys are in the range 60-67 inclusive.
                cmp.w   #$60,d1
                bne.s   PassKey
                                ;Modifier key code.
                move.w  ll_LastKey(a4),d0       ;Get the modifier word
                move.w  d2,d1
                and.w   #$07,d1                 ;Which modifer is to be changed?
                bchg    d1,d0                   ;Change it.
                move.w  d0,ll_LastKey(a4)       ;Save the new modifier.
                bra.s   DoneNewKey

PassKey:        btst    #7,d2                   ;If the most significant bit is set it is a key release.
                bne.s   KeyUp
                                ;Non modifier key press.
                move.w  d2,ll_LastKey+2(a4)     ;Ok, remember the key.
                bra.s   DoneNewKey

KeyUp:                          ;Non modifier key release.
                move.w  d2,d1
                bclr    #7,d1
                cmp.w   ll_LastKey+2(a4),d1     ;Was it the last key pressed?
                bne.s   DoneNewKey
                                ;LastKey has been released.
                or.w    #$FF,ll_LastKey+2(a4)
DoneNewKey:     move.w  d2,d0                   ;Codes above $F8 are special...
                subi.w  #$F9,d2                 ;...control codes, the user...
                bpl.s   NoChange                ;...doesn't want them.

                move.l  ll_KBInterrupts+MLH_HEAD(a4),a1
                bra.s   Test                    ;Get ready to scan the list, do it with a bottom testing loop.
ProcessList:    move.l  IS_CODE(a1),a5
                move.l  IS_DATA(a1),a1
                JSR     (a5)                    ;Execute the users interrupt.
                move.l  (sp)+,a1                ;Restore the list pointer.
                move.w  (sp)+,d0                ;Restore the key.
Test:           move.w  d0,-(sp)                ;Remember the key.
                move.l  MLN_SUCC(a1),-(sp)      ;Remember the list pointer.
                bne.s   ProcessList

                addq    #6,sp                   ;Remove the pointer from the stack.
NoChange:       movem.l (sp)+,d2/a4-a6
                rts

******* lowlevel.library/GetKey **********************************************
*
*   NAME
*	GetKey -- returns the currently pressed rawkey code and qualifiers.
*		  (V40)
*
*   SYNOPSIS
*	key = GetKey();
*	D0
*
*	ULONG GetKey(VOID);
*
*   FUNCTION
*	This function returns the currently pressed non-qualifier key and
*	all pressed qualifiers.
*
*	This function is safe within an interrupt.
*
*	This is a low level function that does not fit the normal Amiga
*	multitasking model. The values returned by this function are
*	not modified by which window/screen currently has input focus.
*
*   RESULT
*	key - key code for the last non-qualifier key pressed in the low
*	      order word. If no key is pressed this word will be FF. The
*	      upper order word contains the qualifiers which can be found
*	      within the long word as follows:
*	                Qualifier               Key
*	                LLKB_LSHIFT             Left Shift
*	                LLKB_RSHIFT             Rigt Shift
*	                LLKB_CAPSLOCK           Caps Lock
*	                LLKB_CONTROL            Control
*	                LLKB_LALT               Left Alt
*	                LLKB_RALT               Right Alt
*	                LLKB_LAMIGA             Left Amiga
*	                LLKB_RAMIGA             Right Amiga
*
*   SEE ALSO
*	<libraries/lowlevel.h>
*
******************************************************************************
GetKey
                move.l  ll_LastKey(a6),d0
                rts

                end
