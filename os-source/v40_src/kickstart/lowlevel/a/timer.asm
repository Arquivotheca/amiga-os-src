******************************************************************************
*
*       $Id: timer.asm,v 40.6 93/07/30 16:06:11 vertex Exp $
*
******************************************************************************
*
*       $Log:	timer.asm,v $
* Revision 40.6  93/07/30  16:06:11  vertex
* Autodoc and include cleanup
* 
* Revision 40.5  93/05/05  09:47:19  gregm
* Jim added comments, but left this checked out.  No code changes.
*
* Revision 40.4  93/03/23  14:40:49  Jim2
* Polished autodocs.
*
* Revision 40.3  93/03/22  12:04:11  Jim2
* Forgot to save before releasing 40.12.
*
* Seperated out the 020 ElapsedTime code.  Tested to see if SetTimeInterval
* would benefit from 020 code.  The answer is no.  While examining
* the 020 ElapsedTime code I came up with a slight improvement.
* The correction on the shift (Revision 40.1) was incorrect.  Rather
* it was bringing in the least significant byte of the remainder.
*
* Revision 40.2  93/03/10  12:22:48  Jim2
* Cleaned up the results of Vertex code walkthrough.
*
* Revision 40.1  93/03/02  13:25:24  Jim2
* Changed all references from game.library to lowlevel.library.
* Corrected error that prevented single shot timers from working.
* The final shift on the time (non 020) needs to be long
* word to get the full range for the timer.
*
* Revision 40.0  93/02/19  16:50:33  Jim2
* Corrected autodoc format for SEE ALSO.
*
* Revision 39.5  93/01/18  13:27:48  Jim2
* Cleaned up comments.
*
* Revision 39.4  93/01/15  13:51:17  Jim2
* Removed the parameter for setting the time interval from AddTimeInt.
* Deleted the functions SetTimeInterval and ConvertToFracTime.
* Added two parameters (TimeInterval and Continuous) to StartTimerInt.
* When an interrupt is added it is added stopped, with any
* pending interrupt cleared.
*
* Revision 39.3  93/01/13  13:41:02  Jim2
* Added ConvertToFracTime and corrected some comments
*
* Revision 39.2  93/01/07  14:21:10  Jim2
* Added 020, or better, code with conditional.  Did not test
* the new code.
*
* Revision 39.1  93/01/05  12:04:50  Jim2
* Made CIAARESOURCE externally visible for keyboard work.
* Corrected the size used when retrieving the timer parameter
* for RemICRVector.
*
* Revision 39.0  92/12/17  18:06:00  Jim2
* Initial release, was tested.
*
*
*       (C) Copyright 1992,1993 Commodore-Amiga, Inc.
*           All Rights Reserved
*
******************************************************************************

        INCLUDE 'exec/macros.i'
        INCLUDE 'exec/types.i'
        INCLUDE 'exec/nodes.i'
        INCLUDE 'exec/lists.i'
        INCLUDE 'exec/memory.i'
        INCLUDE 'exec/io.i'

        INCLUDE 'resources/cia.i'
        INCLUDE 'hardware/cia.i'

        INCLUDE '/macros.i'
        INCLUDE '/lowlevelbase.i'

                XDEF    AddTimerInt
                XDEF    RemTimerInt
                XDEF    StartTimerInt
                XDEF    StopTimerInt
                XDEF    ElapsedTime
                XDEF    CIAARESOURCE

                XREF    _ciaa
                XREF    _ciab

******* lowlevel.library/AddTimerInt *****************************************
*
*   NAME
*	AddTimerInt -- adds an interrupt that is executed at regular
*		       intervals. (V40)
*
*   SYNOPSIS
*	intHandle = AddTimerInt(intRoutine, intData);
*	D0                      A0          A1
*
*	APTR AddTimerInt(APTR, APTR);
*
*   FUNCTION
*	Calling this routine causes the system to allocate a CIA timer
*	and set up 'intRoutine' to service any interrupts caused by the timer.
*	Although the timer is allocated it is neither running, nor enabled.
*	StartIntTimer() must be called to establish the time interval and
*	start the timer.
*
*	The routine is called from within an interrupt, so normal
*	restrictions apply. The routine must preserve the following
*	registers: A2, A3, A4, A7, D2-D7. Other registers are
*	scratch, except for D0, which MUST BE SET TO 0 upon
*	exit. On entry to the routine, A1 holds 'intData' and A5
*	holds 'intRoutine'.
*
*	Only a single CIA timer will be allocated by this routine. So this
*	routine may only be called once without an intervening call to
*	RemTimerInt().
*
*	The CIA timer used by this routine is not guaranteed to always be
*	the same. This routine utilizes the CIA resource and uses an
*	unallocated CIA timer.
*
*	If your program is to exit without reboot, you MUST match all
*	calls to this function with calls to RemTimerInt() before exiting.
*
*	Even if you only use the function once in your program; checking
*	the return value will make your program more tolerant for
*	mulititasking on the Amiga computer platforms.
*
*   INPUTS
*	intRoutine - the routine to invoke upon timer interrupts. This routine
*		     should be as short as possible to minimize its effect on
*		     overall system performance.
*	intData - data passed to the routine in register A1. If more than one
*		  long word of data is required this should be a pointer to
*		  a structure that contains the required data.
*
*   RESULT
*	intHandle - a handle used to manipulate the interrupt, or NULL
*		    if it was not possible to attach the routine.
*
*   SEE ALSO
*	RemTimerInt(), StopTimerInt(), StartTimerInt()
*
******************************************************************************
AddTimerInt
                movem.l a0-a1/a4-a6,-(sp)       ;Store the parameters and some scratch registers.
                move.l  a6,a5
                move.l  ll_ExecBase(a6),a6
                JSRLIB  Forbid                  ;Only one interval timer is allowed.

                                ;We don't want to get multiple invokations of
                                ;lowlevel.library both thinking they can change it.

                tst.l   ll_CIATimer+llct_TimerInt+IS_CODE(a5)
                beq.s   2$
                                ;The interrupt is currently in use.
                JSRLIB  Permit
                movem.l (sp)+,a0-a1/a4-a6
                CLEAR   d0                      ;Set up the return value.
                rts

2$              movem.l (sp)+,a0-a1             ;Fortunately, the way movem works this is OK.
                lea     ll_CIATimer(a5),a4      ;Get the pointer to the CIA timer data
                                                ;structure and initialize it.
                move.l  a0,llct_TimerInt+IS_CODE(a4)
                move.l  a1,llct_TimerInt+IS_DATA(a4)
                clr.b   llct_TimerInt+LN_PRI(a4)
                move.b  #NT_INTERRUPT,llct_TimerInt+LN_TYPE(a4)
                move.l  LN_NAME(a5),llct_TimerInt+LN_NAME(a4)
                JSRLIB  Permit                  ;It is now safe to reinitialize multitasking.
                                ;
                                ;The only mechanism for finding a free timer
                                ;interrupt is to add the interrupts.  However,
                                ;if the interrupts are enabled the interrupt code
                                ;could be called before everything is completely
                                ;set up.  In order to prevent this from happening
                                ;disable the interrupts.
                                ;
                JSRLIB  Disable
                lea     CIAARESOURCE(pc),a1
                                ;Try to allocate a timer in CIAA.
                jsr     FindFreeTimer           ;a6 points to the CIAA resource.
                bpl.s   Opened                  ;The last move determines whether a timer was found.
                move.l  ll_ExecBase(a5),a6
                lea     CIABRESOURCE(pc),a1
                                ;Try to allocate a timer in CIAB.
                jsr     FindFreeTimer           ;a6 points to the CIAB resource.
                bpl.s   Opened

                                ;The last move determines whether a timer was found.
                                ;No timer was found, so clear the marker about a
                                ;an allocated interrupt; restore the
                                ;stack and address registers; and
                                ;return an NULL.

                moveq.l #0,d0
                move.l  d0,llct_TimerInt+IS_CODE(a4)
                move.l  ll_ExecBase(a5),a6
                bra.s   ExitTryCIA
Opened:                         ;Have allocated a CIA timer.
                move.l  a6,llct_Resource(a4)
                move.w  d0,llct_WhichTimer(a4)

                                ;Determine which timer is to be started and set d0 to
                                ;the offset to the register and d1 to the data to be
                                ;written.

                bne.s   TimerB
                move.l  #ciacra,d1
                move.l  #(CIAICRF_TA<<16)|CIACRAF_TODIN|CIACRAF_PBON|CIACRAF_RUNMODE|CIACRAF_OUTMODE|CIACRAF_SPMODE,d0
                move.l  #_ciaa,a1
                bra.s   Doit
TimerB:         move.l  #ciacrb,d1
                move.l  #(CIAICRF_TB<<16)|CIACRBF_ALARM|CIACRBF_PBON|CIACRBF_RUNMODE|CIACRBF_OUTMODE,d0
                move.l  #_ciab,a1
Doit            and.b   d0,0(a1,d1)             ;Touch the register in an atomic fashion.
                swap    d0
                JSRLIB  SetICR                  ;Clear any pending interrupt.
                move.l  ll_ExecBase(a5),a6
                move.l  a4,d0
ExitTryCIA:     JSRLIB  Enable                  ;Allow interrupts to happen
                movem.l (sp)+,a4-a6
                rts

******* lowlevel.library/StopTimerInt ****************************************
*
*   NAME
*	StopTimerInt -- stop the timer associated with the timer interrupt.
*			(V40)
*
*   SYNOPSIS
*	StopTimerInt(intHandle);
*	             A1
*
*	VOID StopTimerInt(APTR);
*
*   FUNCTION
*	Stops the timer associated with the timer interrupt handle passed.
*	This is used to stop a continuous timer started by
*	StartTimerInt().
*
*   INPUTS
*	intHandle - handle obtained from AddTimerInt().
*
*   SEE ALSO
*	AddTimerInt(), RemTimerInt(), StartTimerInt()
*
******************************************************************************
StopTimerInt
                tst.w   llct_WhichTimer(a1)

                                ;Determine which timer is to be started and set d0 to
                                ;the offset to the register and d1 to the data to be
                                ;written.

                bne.s   stopTimerB
                move.l  #ciacra,d1
                move.w  #CIACRAF_TODIN|CIACRAF_PBON|CIACRAF_RUNMODE|CIACRAF_OUTMODE|CIACRAF_SPMODE,d0
                move.l  #_ciaa,a1
                bra.s   stop
stopTimerB:     move.l  #ciacrb,d1
                move.w  #CIACRBF_ALARM|CIACRBF_PBON|CIACRBF_RUNMODE|CIACRBF_OUTMODE,d0
                move.l  #_ciab,a1
stop:           and.b   d0,0(a1,d1)             ;Touch the register in an atomic fashion.
                rts

******* lowlevel.library/StartTimerInt ***************************************
*
*   NAME
*	StartTimerInt -- start the timer associated with the timer interrupt.
*			 (V40)
*
*   SYNOPSIS
*	StartTimerInt(intHandle, timeInterval, continuous);
*	              A1         D0            D1
*
*	VOID StartTimerInt(APTR, ULONG, BOOL);
*
*   FUNCTION
*	This routine starts a stopped timer that is assocatied with a
*	timer interrupt created by AddTimerInt().
*
*   INPUTS
*	intHandle - handle obtained from AddTimerInt().
*	timeInterval - number of micoseconds between interrupts. The
*	               maximum value allowed is 90,000. If higher values
*	               are passed there will be unexpected results.
*	continuous - FALSE for a one shot interrupt. TRUE for multiple
*	             interrupts.
*
*   SEE ALSO
*	AddTimerInt(), RemTimerInt(), StopTimerInt()
*
******************************************************************************
StartTimerInt
                movem.l d1-d3,-(sp)             ;Save the handle.
                tst.w   llct_WhichTimer(a1)

                                ;Determine which timer is to be started and set d2 to
                                ;the offset to the register, d3 to the data to be
                                ;written and a1 to the base register for the time
                                ;interval.

                bne.s   startTimerB
                move.l  #ciacra,d2
                move.l  #ciatalo,a1
                move.l  #(CIACRAF_START<<16)|CIACRAF_TODIN|CIACRAF_PBON|CIACRAF_OUTMODE|CIACRAF_SPMODE,d3
                move.l  #_ciaa,a0
                bra.s   start
startTimerB:    move.l  #ciacrb,d2
                move.l  #ciatblo,a1
                move.l  #(CIACRBF_START<<16)|CIACRBF_ALARM|CIACRBF_PBON|CIACRBF_OUTMODE,d3
                move.l  #_ciab,a0
start:          and.b   d3,0(a0,d2)             ;Stop any timer
                tst.w   d1
                bne.s   ConMode
                or.b    #CIACRAF_RUNMODE,0(a0,d2)
ConMode:        move.l  ll_EClockConversion(a6),d1
                lsr.l   #1,d0                   ;Ignore the least significant bit.
                lsr.l   #4,d1                   ;Ignore the four least significant bits of the EClocks/second.
                                ;
                                ;All of this ignoring results in a sixteen bit by
                                ;sixteen bit multiply.  Since only the upper order
                                ;sixteen bits of the final answer are useful this
                                ;ignoring is meanless.
                                ;
                mulu.w  d1,d0
                divu.w  #31250,d0               ;Only want the high order sixteen bits.
                                ;
                                ;The preceeding four lines can be replaced by
                                ;the following two lines:
                                ;mulu.l d1,d0
                                ;divu.l #100000,d0
                                ;however this will acutally execute slower so
                                ;there really is no need for 020 > specific
                                ;code at this time.
                                ;
                bne.s   1$
                addq.b  #1,d0                   ;Can't have an interval of zero, must be at least one.
1$              add.l   a0,a1
                move.b  d0,(a1)                 ;Write the low byte.
                lsr.w   #8,d0                   ;Get the high byte in a position to be written.
                move.b  d0,ciatbhi-ciatblo(a1)  ;Write the high byte.
                tst.w   2(sp)
                beq.s   SingleShot
                swap    d3
                or.b    d3,0(a0,d2)             ;Touch the register in an atomic fashion.
SingleShot:     movem.l (sp)+,d1-d3
                rts

******* lowlevel.library/RemTimerInt *****************************************
*
*   NAME
*	RemTimerInt -- remove a previously installed timer interrupt. (V40)
*
*   SYNOPSIS
*	RemTimerInt(intHandle);
*	            A1
*
*	VOID RemTimerInt(APTR);
*
*   FUNCTION
*	Removes a timer interrupt routine previously installed with
*	AddTimerInt.
*
*   INPUTS
*	intHandle - handle obtained from AddTimerInt(). This may be NULL,
*		    in which case this function does nothing.
*
*   SEE ALSO
*	AddTimerInt(), StopTimerInt(), StartTimerInt()
*
******************************************************************************
RemTimerInt
                move.l  a6,-(sp)                ;Save the parameter and scratch registers.
                move.l  a1,-(sp)                ;Check for NULL.
                beq.s   rti_Error

                move.w  llct_WhichTimer(a1),d0  ;Get the parameters for RemICRVector.
                move.l  llct_Resource(a1),a6
                JSRLIB  RemICRVector
                movem.l (sp)+,a1                ;Restore the registers.
                clr.l   IS_CODE(a1)             ;Clear the marker for the allocated Timer Interrupt.
                move.l  (sp)+,a6
                rts

rti_Error       movem.l (sp)+,a1/a6
                rts

*****l* a/timer.asm/FindFreeTimer ********************************************
*  NAME
*	FindTimerTimer - Trys to own a CIA timer.
*
*  SYNOPSIS
*	Failure = FindTimerTimer(ExecBase, resName, Interrupt, CIAResource, TimerAlloc)
*	NFlag                    a6        a1       a4         a6           d0
*
*  FUNCTION
*	Opens the indicated CIA resource.  Then attempts to install the
*	interrupt routine for the A timer.  If it is unsuccessful then
*	the B timer is tried.
*
*  INPUTS
*	ExecBase - Pointer to ExecBase.
*	resName - Name of CIA resource to be opened.
*	Interrupt - Pointer to the interrupt structure to be added.
*
*  RESULT
*	Failure - Set if no timer was owned.
*	CIAResource - Points to the opened resource.
*	TimerAlloc - Identifies which timer was actually owned.
*
******************************************************************************
FindFreeTimer:
                JSRLIB  OpenResource
                move.l  d0,a6                   ;d0 is the return from OpenResource.
                move.l  a4,a1                   ;Ready the interrupt pointer.
                move.w  #CIAICRB_TA,d0          ;Other parameter is the timer.
                move.w  d0,-(sp)                ;Remember which timer is being tried.
                JSRLIB  AddICRVector
                tst.l   d0
                beq.s   OK
                                ;Could not allocate timer A, try timer B.
                move.l  a4,a1                   ;Ready the interrupt pointer.
                move.w  #CIAICRB_TB,d0          ;Other parmeter is the timer.
                move.w  d0,(sp)                 ;Remember which timer is being tried.
                                                ;Not pushed, previous value was never popped.
                JSRLIB  AddICRVector
                tst.l   d0
                beq.s   OK
                                ;Could not allocate timer B - Failure.
                move.w  #-1,(sp)                ;The word at the pointer indicates which timer, well, this is no timer.
OK:             move.w  (sp)+,d0                ;Return the allocated timer in d0.
                                                ;Set the N flag to indicate success/failure.
                rts

******* lowlevel.library/ElapsedTime *****************************************
*
*   NAME
*	ElapsedTime -- returns the time elapsed since it was last called. (V40)
*
*   SYNOPSIS
*	fractionalSeconds = ElapsedTime(context);
*	D0                              A0
*
*	ULONG ElapsedTime(struct EClockVal *);
*
*   FUNCTION
*	This function utilizes the timer.device/ReadEClock() function to get
*	an accurate elapsed time value. Since the context needs to be
*	established the first call to this routine will return a nonsense
*	value.
*
*	The return value for this function only allows for sixteen bits
*	worth for the integer number of seconds and sixteen bits for the
*	factional number of seconds.
*
*	With sixteen bits worth of integer seconds this function can be
*	used to timer an interval up to about 16 hours. If the actual time
*	interval is larger this function will return this maximum value.
*
*	The sixteen bits for fractional seconds gives a resolution of
*	approximately 20 microseconds. However, it is not recomended
*	to expect this function to be accurate for a time interval of
*	less than 200 microseconds.
*
*   INPUTS
*	context - pointer to an EClockVal structure. The first time you
*		  call this function, you should initialize the structure
*		  to 0s. You should then reuse the same structure for
*		  subsequent calls to this function, as this is how the
*		  elapsed time is calculated.
*
*   RESULT
*	fractionalSeconds - The elapsed time as a fixed point 32-bit
*	                    number with the point fixed in the middle.
*	                    That is, the upper order sixteen bits represent
*	                    the number of seconds elapsed. The low order
*	                    sixteen bit represent the fractional number of
*	                    seconds elapsed. This value is limited to about
*	                    sixteen hours. Although this value is precise
*	                    to nearly 20 microseconds it is only accurate to
*	                    within 200 microseconds.
*
*   WARNING
*	The first call to this function will return a non-sense value. Only
*	rely on its result starting with the second call.
*
*   SEE ALSO
*	timer.device/ReadEClock()
*
******************************************************************************
        OPT     P=68020
ElapsedTime
                movem.l d2-d3/a6,-(sp)          ;Need a few registers.
                move.l  4(a0),-(sp)
                move.l  (a0),-(sp)              ;Copy the last EClock value to the stack.
                move.l  ll_TimerDevice(a6),a6
                JSRLIB  ReadEClock
                movem.l (a0)+,d1/d2             ;Get new EClock value in d1:d2.
                sub.l   4(sp),d2                ;Subtract low order EClock long word.
                move.l  (sp)+,d3                ;Get old EClock value in d3.
                addq    #4,sp                   ;Clear old EClock value from stack.
                subx.l  d3,d1                   ;Subtract high order EClock long word including borrow.
                bpl.s   2$

                neg.l   d2
                negx.l  d1
2$:             swap    d1                      ;Multiply by 2^16.
                tst.w   d1
                bne.s   et_Overflow

                swap    d2
                move.w  d2,d1
                clr.w   d2
                divu.l  d0,d1:d2                ;Divide 64bit/32bit get 32bit Rem:32bit Quo
                bvs.s   et_Overflow
                move.l  d2,d0

        OPT     P=68000

et_Exit         movem.l (sp)+,d2-d3/a6          ;Restore scratch registers.
                rts

et_Overflow:    moveq   #-1,d0
                bra.s   et_Exit


                XDEF    ElapsedTime000

*
*
* NOTE to self:  This code assume the E Clocks per second is 20 bit number.
*	         If this ever changes the code needs to be rewritten.
ElapsedTime000
                movem.l d2-d3/a6,-(sp)          ;Need a few registers.
                move.l  4(a0),-(sp)
                move.l  (a0),-(sp)              ;Copy the last EClock value to the stack.
                move.l  ll_TimerDevice(a6),a6
                JSRLIB  ReadEClock
                movem.l (a0)+,d1/d2             ;Get new EClock value in d1:d2.
                sub.l   4(sp),d2                ;Subtract low order EClock long word.
                move.l  (sp)+,d3                ;Get old EClock value in d3.
                addq    #4,sp                   ;Clear old EClock value from stack.
                subx.l  d3,d1                   ;Subtract high order EClock long word including borrow.
                bpl.s   1$

                neg.l   d2
                negx.l  d1
1$:             cmp.l   #$F,d1
                bgt.s   et_Overflow
                                ;vanilla 68000 code can only do 32bit/16bit.
                                ;Tick/sec is twenty bits so shift everyone over four.
                lsr.l   #4,d0
                and.w   #$000F,d1
                and.w   #$FFF0,d2
                or.w    d1,d2
                ror.l   #4,d2
                divu.w  d0,d2
                bvs.s   et_Overflow
                move.l  d2,d1
                swap    d2
                clr.w   d1
                divu.w  d0,d1
                move.w  d1,d2

                move.l  d2,d0
                movem.l (sp)+,d2-d3/a6          ;Restore scratch registers.
                rts



CIAARESOURCE    CIAANAME
CIABRESOURCE    CIABNAME
                end
