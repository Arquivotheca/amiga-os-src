******************************************************************************
*
*	$Id: systemcontrol.asm,v 40.10 93/07/30 16:05:40 vertex Exp $
*
******************************************************************************
*
*	$Log:	systemcontrol.asm,v $
* Revision 40.10  93/07/30  16:05:40  vertex
* Autodoc and include cleanup
* 
* Revision 40.9  93/05/05  09:52:18  gregm
* autodoc changes ...
* changed the ntsc (effective) value from 16667 to 16666, which causes a
* division to be "closer to correct" in the default values.
*
* Revision 40.8  93/04/20  14:27:38  Jim2
* Changed the autodoc for the SCON_StopInput tag.
*
* Revision 40.7  93/03/31  18:51:13  Jim2
* Oops, InstallInputMap can fail.  Even more so now.
*
* Revision 40.6  93/03/23  14:41:35  Jim2
* Polished autodocs.
*
* Revision 40.5  93/03/12  20:29:47  Jim2
* Added the two new tags to add and remove the routines that translate
* joysticks/game controllers to rawkey codes.  Also changed the
* structure of the main routine to use a jump table while parsing
* the tags.  Finally, altered the system ownership so it only
* applies to the two important tags TakeOverSys and KillInput.
*
* Revision 40.4  93/03/10  12:23:21  Jim2
* Cleaned up the results of Vertex code walkthrough.
*
* Revision 40.3  93/03/08  10:49:49  Jim2
* Corrected the register assignment in the autodoc.
*
* Revision 40.2  93/03/02  13:23:21  Jim2
* Changed all references from game.library to lowlevel.library.
*
* Revision 40.1  93/03/01  16:12:00  Jim2
* Added note to remind people about the nature of the beast.
*
* Revision 40.0  93/02/11  17:33:24  Jim2
* Change to use the new single include file.
*
* Revision 39.6  93/01/19  15:44:21  Jim2
* Added SCON_StopInput.
*
* Revision 39.5  93/01/18  13:32:26  Jim2
* Cleaned comments.  Changed tag to SCON_TakeOverSys from SCON_OwnSys.
*
* Revision 39.4  93/01/13  13:43:28  Jim2
* Added the tag SCON_CDReboot to the function descriptin.
*
* Revision 39.3  93/01/07  14:22:44  Jim2
* Cleaned up comments and minimized the specific include files.
*
* Revision 39.2  93/01/05  12:08:04  Jim2
* Documentation correction for SystemControlA.
*
* Revision 39.1  92/12/14  14:32:59  Jim2
* Tested the code for backing out and for clearing the process
* that can work with the system.
*
* Revision 39.0  92/12/11  14:03:18  Jim2
* Initial Release.
*
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

        INCLUDE 'exec/macros.i'
        INCLUDE 'utility/tagitem.i'

        INCLUDE '/macros.i'
        INCLUDE '/lowlevel.i'
        INCLUDE '/lowlevelbase.i'

                XDEF    SystemControlA

                XREF    TakeOverSystem
                XREF    CDRebootOnRemove
                XREF    TurnOffRequesters
                XREF    TrashInput
                                ;InputMap.asm
                XREF    InstallInputMap
                XREF    UnInstallInputMap


******* lowlevel.library/SystemControlA ***************************************
*
*   NAME
*	SystemControlA - Method for selectively disabling OS features. (V40)
*	SystemControl - varargs stub for SystemControlA().
*
*   SYNOPSIS
*	failTag = SystemControlA(tagList);
*	D0                       A1
*
*	ULONG SystemControlA(struct TagItem *);
*
*	failTag = SystemControl(firstTag, ...);
*
*	ULONG SystemControl(Tag, ...);
*
*   FUNCTION
*	This function is used to alter the operation of the system. Some of
*	the alterations involve controlling what are normally regarded as
*	system resources. In order to minimize confusion only one task is
*	allowed to control any part of the system resources. This prevents
*	the possiblity of two tasks fighting, each controlling a part of the
*	system. If a tag is identified as task exclusive, it means that
*	only one task can hold (set to TRUE) that tag. If
*	another task attempts to set the same tag to TRUE, the call to
*	SystemControl() will fail.
*
*	It is important to remember that SystemControl() can fail.
*
*	This is a low level function and certain tags do not fit the normal
*	Amiga multitasking model.
*
*   INPUTS
*	tagList - pointer to an array of tags listing the features of the
*	          system to be enabled/disabled.
*
*   TAGS
*	SCON_TakeOverSys (BOOL)
*		TRUE -	Takes over the CPU to ensure that a program gets every
*			ounce of CPU time (with the exception of crucial
*			interrupts). When in this mode, the CPU will belong
*			completely to the program. Task switching will be
*			disabled and the program will get all CPU cycles. This
*			means any calls to the OS that involve multitasking in
*			some way will not execute correctly. Other tasks will
*			not run until this tag is used with FALSE. However,
*			during a Wait() on a signal, multitasking will
*			automatically be turned back on until the signal is
*			received. Once received, multitasking will again be
*			disabled and the CPU will be exclusive to the owning
*			program.
*		FALSE - Relinquishes the CPU and reenables multitasking.
*			This tag is task exculsive. This tag nests. A task may
*			take over the CPU several times before relinquishing
*			it.
*
*	SCON_KillReq (BOOL)
*	       TRUE - 	Disables system requesters.  These are the reasons for
*			NOT disabling system requesters:
*
*	                    1- No calls in the program will cause a system
*			       requester.
*	                    2- The only thing that could cause a requester
*			       to appear is the lack of a CD in the drive and
*			       SCON_CDReboot is set to CDReboot_On, therefore a
*			       requester can't appear.
*	    		    3- The only disk I/O is via a CD with SCON_CDReboot
*			       set to CDReboot_On and/or nonvolatile.library.
*
*	    		When requesters should not be disabled.
*	    		GAME PROGRAMS:
*	    		No DOS calls are used after loading; or SCON_CDReboot
*			is CDReboot_On; and nonvolatile.library is used for
*			loading and saving user data.
*
*	    		This fits the above case since; After loading either
*			DOS calls are not used fitting reason 1, or the game
*			is accessing the CD and has SCON_CDReboot set to
*			CDReboot_On fitting reason 2. The game accesses high
*			scores, game position, etc through nonvolatile.library,
*			fitting reason 3.
*
*		FALSE -	Enables requesters for the program.
*
*	    This tag nests. Tasks may disable requesters several times before
*	    enabling them.  However, there must be a matching number of calls.
*
*	SCON_CDReboot (ULONG)
*		CDReboot_On - Ejecting the CD will cause a reboot of the
*			system. Use this only if the program cannot deal with
*			error conditions.
*		CDReboot_Off - Ejecting the CD will not cause a reboot of the
*	    		system. Use this if the program needs to insert CDs
*			while running.
*		CDReboot_Default - Restore the default reboot behavior for this
*			system. This should be used upon exit, if this tag had
*			been used to change the reboot behaviour. For the CD32
* 			this value is synonomous with CDReboot_On. For Amiga
*			computers this value is synonomous with CDReboot_Off.
*
*	    Note that the default reboot behavior differs depending on the
*	    platform. If a program requires a specific behavior it must
*	    use this function to set the behavior. For example, a CD audio
*	    mixer would use this tag with the data CDReboot_Off. This will
*	    allow the changing of audio CDs on the game machine as well as
*	    Amiga computers.
*
*	    If, however, there is no error detection code at all this tag
*	    should be used with the data CDReboot_On.
*
*	    It is hoped that no program will require CDReboot_On. If all
*	    programs check for error condition and recover gracefully such a
*	    call should never be necessary. With the default behavior the
*	    CD32 will always reset on disk ejects, and programs
*	    run from Amiga computers will not reset. Thus, leaving the
*	    default will increase the market for a program to include both
*	    types of platforms.
*
*	    This tag does not nest.
*
*	SCON_StopInput (BOOL) - When TRUE, stops input.device from using any
*	    CPU cycles. Also prevents input.device from passing along any
*	    events from either the keyboard and/or port 0.
*
*	    This tag is task exclusive. This tag is NOT reversible.
*	    Attempting to reverse will result in confused/garbled input
*	    events.
*
*	SCON_AddCreateKeys (ULONG) - Starts creating rawkey codes for the
*	    joystick/game controller on the given unit. The unit value is
*	    checked for validity and must be either 0 or 1. Each different unit
*	    used results in some code added to the VBlank interrupt chain.
*	    This tag nests. The tag SCON_RemCreateKeys is used to undo this
*	    tag. Tasks may create rawkey codes several times before stopping
*	    them.
*
*	    Note that when operating in an Intuition window, the controller's
*	    blue button is the equivilent of the mouse menu button. Therefore,
*	    Intuition will be capturing most blue button events. If
*	    notificiation of these events is important, review the
*	    documentation for WFLG_RMBTRAP in the
*	    intuition.library/OpenWindow() autodoc.
*
*	SCON_RemCreateKeys (ULONG) - stops rawkey codes for the joystick/game
*	    controller on the given unit. The unit value is checked for
*	    validity and must be either 0 or 1.
*
*   RESULT
*	failTag - zero if all tags succeeded. A non-zero return indicates a
*		  tag that has failed. It is possible that other tags may
*	          fail as well.
*
*	          If any tag fails there will be no change in the system due
*	          to other tags.
*
*   SEE ALSO
*	<libraries/lowlevel.h>
*
*******************************************************************************
*
*	d7      Task executing
*	a5      LowLevelBase
*	a6      ExecBase
*
*******************************************************************************
SystemControlA
                movem.l d7/a4-a6,-(sp)
                move.l  a6,a5

TEMP_SIZE       set     0                       ;Determine the number of bytes wanted for local variables
                LONGVAR TagState
                LONGVAR UndoTag

                ALLOCLOCALS sp                  ;Make some local variables on the stack.
                move.l  a1,TagState_l(sp)       ;Save the pointer to the Taglist.
                move.l  a1,UndoTag_l(sp)
                move.l  ll_ExecBase(a5),a6
                CLEARA  a1
                JSRLIB  FindTask                ;FindTask (a1>Null:Current Task, a6>ExecBase)
                move.l  d0,d7

tagparse:       move.l  ll_UtilBase(a5),a6
                lea     TagState_l(sp),a0
                JSRLIB  NextTagItem             ;NextTagItem(a6>UtilBase, a0>**TagItem)
                move.l  ll_ExecBase(a5),a6
                                ;d0 points to the tag to be handled, or NULL if complete.
                tst.l   d0
                bne.s   sc_NewTag
                                ;Completed parseing the tag list.
donetags:       DONELOCALS sp
                move.l  d0,-(sp)
                move.b  ll_NestOwn(a5),d0
                and.b   ll_NestKillIn(a5),d0
                bpl.s   StillControl
                sub.l   ll_SystemOwner(a5),d7
                bne.s   StillControl
                move.l  d7,ll_SystemOwner(a5)   ;No one owns any part of the system.
StillControl:   movem.l (sp)+,d0/d7/a4-a6
                rts

sc_NewTag:      move.l  d0,a0
                move.l  (a0)+,d0                ;Tag into d0.
                move.l  (a0)+,d1                ;Tag data into d1.
                sub.l   #SCON_TakeOverSys,d0    ;Lowest numbered tag is SCON_TakeOverSys.
                bmi.s   tagparse
                cmp.w   #SCON_RemCreateKeys-SCON_TakeOverSys,d0
                bgt.s   tagparse
                rol.w   #1,d0
                move.w  sc_JumpTable(pc,d0.w),d0
                jmp     sc_JumpTable(pc,d0.w)

sc_JumpTable:   dc.w    sc_TakeOverSys-sc_JumpTable
                dc.w    sc_KillReq-sc_JumpTable
                dc.w    sc_CDReboot-sc_JumpTable
                dc.w    sc_StopInput-sc_JumpTable
                dc.w    sc_AddCreateKeys-sc_JumpTable
                dc.w    sc_RemCreateKeys-sc_JumpTable




sc_TakeOverSys: move.l  #SCON_TakeOverSys,d0
                bsr     TestOwner
                bne     sc_Undo
                bsr     TakeOverSystem          ;TakeOverSystem(a5>GameBase, d1>TRUE/FALSE)
                bra.s   tagparse

sc_KillReq:     exg     a5,a6
                bsr     TurnOffRequesters       ;TurnOffRequesters(a6>GameBase, d1>TRUE/FALSE)
                exg     a5,a6
                bra.s   tagparse
                                ;SCON_CDReboot
sc_CDReboot:    bsr     CDRebootOnRemove        ;CDRebootOnRemove(a5>GameBase, d1>FORCEON,OFF,DEFAULT)
                tst.l   d0
                beq.s   tagparse

                move.l  #SCON_CDReboot,d0
                bra     sc_Undo
                                ;SCON_StopInput
sc_StopInput:   move.l  #SCON_StopInput,d0
                bsr     TestOwner
                bne     sc_Undo
                bsr     TrashInput
                bra     tagparse

sc_AddCreateKeys:
                move.l  #SCON_AddCreateKeys,d0
                cmp.l   #1,d1
                bgt.s   sc_Undo

                move.l  d1,d0
                mulu.w  #lljp_SIZE,d1
                lea     ll_Port0State(a5,d1.l),a4
                lea     lljp_AddSafely(a4),a0
                JSRLIB  ObtainSemaphore
                                ;
                                ;It is possible to fail during the add, so we need
                                ;to complete the addition before letting any other
                                ;process believe it can use the interrupts.
                                ;
                                ;It is not necessary to protect the removing, or
                                ;the adding in the backout since it should not
                                ;fail.  The port is available, the only problem
                                ;might be with the memory.
                                ;
                addq.b  #1,lljp_NestCnt(a4)
                bne.s   ValidAdd

                bsr     InstallInputMap
                move.l  d0,lljp_InputHandle(a4)
                bne.s   ValidAdd
                lea     lljp_AddSafely(a4),a0
                JSRLIB  ReleaseSemaphore
                move.l  #SCON_AddCreateKeys,d0
                bra.s   sc_Undo

ValidAdd:       lea     lljp_AddSafely(a4),a0
                JSRLIB  ReleaseSemaphore
                bra     tagparse

sc_RemCreateKeys:
                move.l  #SCON_RemCreateKeys,d0
                cmp.l   #1,d1
                bgt.s   sc_Undo

                move.l  d1,d0
                mulu.w  #lljp_SIZE,d1
                lea     ll_Port0State(a5,d1.l),a4
                subq.b  #1,lljp_NestCnt(a4)
                bpl     tagparse

                move.l  lljp_InputHandle(a4),a0
                bsr     UnInstallInputMap
                bra     tagparse

sc_Undo         move.l  UndoTag_l(sp),a1
                bsr.s   BackOutSysCon
                bra     donetags



*****l* a/SystemControl.asm/TestOwner ****************************************
*
*   NAME
*	TestOwner - Determines whether the current Task owns the system.
*
*   SYNOPSIS
*	TestOwner (TaskPtr, LowLevelBase, ExecBase)
*	           d7       a5            a6
*
*   FUNCTION
*	If no task owns the system changes the system owner to the current
*	task.  Sets the condition code as to whether the system owner task
*	is equal to the current task.
*
*   NOTE
*	MUST preserve the values in d0 and d1.
*
*
******************************************************************************
TestOwner
                JSRLIB  Forbid
                tst.l   ll_SystemOwner(a5)
                bne.s   to_Owned

                move.l  d7,ll_SystemOwner(a5)
to_Owned:       JSRLIB  Permit
                                ;
                                ;It is of course possible for the other task
                                ;which previously owned the system to come
                                ;along and release it between the permit and
                                ;this statement, but we don't care since we
                                ;have not set ourselves as owner.
                                ;
                cmp.l   ll_SystemOwner(a5),d7
                rts

*****l* a/SystemControl.asm/BackOutSysCon ************************************
*
*   NAME
*	BackOutSysCon - Undo any tags executed by SystemControlA before the
*	                failed tag.
*
*   SYNOPSIS
*	BackOutSysCon (tagItemPtr, FailedTag)
*	               a1          d0
*
*   FUNCTION
*	Rewalks the tag list passed to SystemControlA until the failed tag
*	is encountered.  Before processing each tag the data associated with
*	that tag is inverted.
*
******************************************************************************
BackOutSysCon
                movem.l d0/d7,-(sp)
                move.l  d0,d7
                move.l  a1,-(sp)
NextTag         move.l  ll_UtilBase(a5),a6
                lea     (sp),a0
                JSRLIB  NextTagItem             ;NextTagItem(a6>UtilBase, a0>**TagItem)
                move.l  ll_ExecBase(a5),a6
                                ;d0 points to the tag to be handled, or NULL if complete.
                move.l  d0,a0
                move.l  (a0)+,d0                ;Tag into d0.
                cmp.l   d0,d7
                bne.s   bosc_NewTag
                                ;Completed parseing the tag list.
                move.l  (sp)+,a1
                movem.l (sp)+,d0/d7
                rts

bosc_NewTag:    move.l  (a0)+,d1                ;Tag data into d1.
                sub.l   #SCON_TakeOverSys,d0    ;Lowest numbered tag is SCON_TakeOverSys.
                bmi.s   NextTag
                cmp.w   #SCON_RemCreateKeys-SCON_TakeOverSys,d0
                bgt.s   NextTag

                rol.w   #1,d0
                move.w  bosc_JumpTable(pc,d0.w),d0
                jmp     bosc_JumpTable(pc,d0.w)


bosc_JumpTable: dc.w    bosc_TakeOverSys-bosc_JumpTable
                dc.w    bosc_KillReq-bosc_JumpTable
                dc.w    bosc_CDReboot-bosc_JumpTable
                dc.w    bosc_StopInput-bosc_JumpTable
                dc.w    bosc_AddCreateKeys-bosc_JumpTable
                dc.w    bosc_RemCreateKeys-bosc_JumpTable




bosc_TakeOverSys:
                bsr.s   InvertBOOL
                bsr     TakeOverSystem          ;TakeOverSystem(a5>GameBase, d1>TRUE/FALSE)
                bra.s   NextTag

bosc_KillReq:   bsr.s   InvertBOOL
                exg     a5,a6
                bsr     TurnOffRequesters       ;TurnOffRequesters(a6>GameBase, d1>TRUE/FALSE)
                exg     a5,a6
                bra.s   NextTag
                                ;SCON_CDReboot
bosc_CDReboot:  cmp.l   #1,d1
                bpl.s   1$
                eori.s  #1,d1
1$:             bsr     CDRebootOnRemove        ;CDRebootOnRemove(a5>GameBase, d1>FORCEON,OFF,DEFAULT)
                bra.s   NextTag
                                ;SCON_StopInput
bosc_StopInput: bsr.s   InvertBOOL
                bsr     TrashInput
                bra.s   NextTag

bosc_RemCreateKeys:
                move.l  d1,d0
                mulu.w  #lljp_SIZE,d1
                lea     ll_Port0State(a5,d1.l),a4
                addq.b  #1,lljp_NestCnt(a4)
                bne     NextTag

                bsr     InstallInputMap
                move.l  d0,lljp_InputHandle(a4)
                bra     NextTag

bosc_AddCreateKeys:
                move.l  d1,d0
                mulu.w  #lljp_SIZE,d1
                lea     ll_Port0State(a5,d1.l),a4
                subq.b  #1,lljp_NestCnt(a4)
                bpl     NextTag

                move.l  lljp_InputHandle(a4),d0
                bsr     UnInstallInputMap
                bra     NextTag


*****l* a/SystemControl.asm/InvertBOOL ***************************************
*
*   NAME
*	InvertBOOL - Invert the boolean variable.
*
*   SYNOPSIS
*	InvertBOOL (Bool)
*	            d1
*
*   FUNCTION
*	Takes the current value in d1, determines if it is TRUE, or FALSE and
*	then inverts it.
*
*	TRUE is non-zero, FALSE is zero.
*
******************************************************************************
InvertBOOL
                tst.w   d1
                seq     d1
                rts


                END
