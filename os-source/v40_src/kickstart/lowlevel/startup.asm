******************************************************************************
*
*       $Id: startup.asm,v 40.21 93/07/30 16:26:50 vertex Exp $
*
******************************************************************************
*
*       $Log:	startup.asm,v $
* Revision 40.21  93/07/30  16:26:50  vertex
* CHanged the way DISK_BASED is referenced
* 
* Revision 40.20  93/07/30  16:09:03  vertex
* Autodoc and include cleanup
*
* Revision 40.19  93/05/21  11:17:44  gregm
* Added a moveq #0,d0 to GetLanguageSelection().  Otherwise, garbage would've filled
* the top 24 bits.
*
* Revision 40.18  93/05/18  15:02:59  gregm
* Jim was doing a CDREbootDefault() at Init time.
* Jerry informed me that the CD_INFO command won't return realisitic
* information for the default reboot behavior until well after that time.
* I now set the field to a negative value, and the first time someone
* tries to set the value, I do the query then (first) to find the
* current, or perceived default setting.
*
* Revision 40.17  93/05/05  09:55:45  gregm
* init code for port 2, 3 added, plus init code for clearing the RJP jump vectors.
*
* Revision 40.16  93/04/21  09:54:58  Jim2
* Reserved space for six additional vectors at the end of
* the jump table.
*
* Revision 40.15  93/04/20  14:18:59  Jim2
* Finally got around to updating the end label to reflect
* lowlevel rather than game.
*
* Revision 40.14  93/04/19  13:38:57  Jim2
* Played around with the comments.  Moved the storing of
* the original EasyRequest vector to the memory turd.
*
* Revision 40.13  93/04/14  14:12:59  Jim2
* Oops forgot to save a2 in SetLanguageSelection
*
* Revision 40.12  93/04/13  15:35:09  Jim2
* Changed the priority to 1 to allow ram kicking.
*
* Revision 40.11  93/04/06  12:34:47  Jim2
* Made the expunge code safe for the ROM version.
*
* Revision 40.10  93/03/31  18:57:11  Jim2
* Added code to support the VBlank counter for graphic <40.
* Also for closing graphic.library on an expunge (CI103).
* Adding the port stuff to CDDefault means it must be moved
* to after the setting up of the port.
*
* Revision 40.9  93/03/26  17:38:02  Jim2
* Open the PotGo resource at init time.  Also use the VBlank
* counter in graphics base (V40 and beyond).
*
* Revision 40.8  93/03/23  14:48:03  Jim2
* Corrected name for VBlank interrupts.
*
* Revision 40.7  93/03/19  11:38:48  Jim2
* Made allowances for a seperate disk base version of the library
* which has to be able to run on processesor that do not have
* 68020s or better.  Sucked in the joyport initialization and expunge code.
* The ROM version does not contain the expunge code.
*
* Revision 40.6  93/03/15  16:28:08  Jim2
* Fixed error in SetLanguageSelection.
*
* Revision 40.5  93/03/12  20:12:08  Jim2
* Actually made SetLanguageSelection not crash.
*
* Revision 40.4  93/03/10  19:56:19  Jim2
* Got expunge to work and managed to leave something in memory
* so we don't need to clean up the setfunction on EasyRequest.
*
* Revision 40.3  93/03/10  12:25:17  Jim2
* Let strap read the languages out of nonvolatile into utilitybase.
*
* Revision 40.2  93/03/08  10:52:40  Jim2
* Added accessing the nonvolatile memory to get the language.
* Set function of EasyRequest as a means of killing the system
* requesters.
*
* Revision 40.1  93/03/02  13:32:24  Jim2
* Changed all but the end reference from game to lowlevel.
* Removed the anim calls.
*
* Revision 40.0  93/02/19  17:47:31  Jim2
* First instruction is now moveq.l #-1 not #0.
*
* Revision 39.13  93/01/29  17:47:12  Jim2
* Added the ROMTAG flag RTF_COLDSTART.  This allows the library to
* be found prior to DOS running.
*
* Revision 39.12  93/01/19  15:48:08  Jim2
* Language is stored in UtilBase not GameBase.
*
* Revision 39.11  93/01/18  13:40:23  Jim2
* Changed method for determining CD default behavior.  Expunges
* all ReadJoyPort allocations.
*
* Revision 39.10  93/01/15  14:07:18  Jim2
* Removed SetKeyThresh, SetKeyPeriod and SetTimerInterval.
* Added LVOs for AddVBlankInt and RemVBlankInt.
*
* Revision 39.9  93/01/13  15:01:14  Jim2
* Removed KeyboardInterruptExtender.  Intialized the structures
* used for autorepeat of the keyboard.
*
* Revision 39.8  93/01/07  15:59:08  Jim2
* Added intitialization of port controller state.
*
* Revision 39.7  93/01/07  14:31:43  Jim2
* Cleaned up comments, minimized include file list and changed
* the initial LVO table back to relative.
*
* Revision 39.6  93/01/05  12:16:08  Jim2
* Added keyboard support.  Also renamed some routines to
* make them nicer sounding.  And moved ReadJoyPort to first
* non-standard function.
*
* Revision 39.5  92/12/17  18:18:23  Jim2
* Added LVOs for working with CIA timer interrupts.  Also
* changed the names for the routines dealing with the animation.
* And added the LVO for elapsed time.
*
* Revision 39.4  92/12/14  15:03:57  Jim2
* During testing had difficulty with relative offsets for the LVOs.
* Changed to absolute.
*
* Revision 39.3  92/12/11  14:17:25  Jim2
* Removed all of the currently implemented LVO other than
* the standard four:  open, close, expunge, and extra.
*
* Revision 39.2  92/12/09  18:16:57  Jim2
* Got the autodoc comments added and implemented the LVO for
* TurnOffRequesters.
*
* Revision 39.1  92/12/08  16:50:48  Jim2
* Now it actually opens, closes, expunges and TakeOverSystem.
*
* Revision 39.0  92/12/07  16:08:39  Jim2
* Initial release prior to testing.
*
*
*       (C) Copyright 1992,1993 Commodore-Amiga, Inc.
*           All Rights Reserved
*
******************************************************************************

        INCLUDE 'exec/macros.i'
        INCLUDE 'exec/lists.i'
        INCLUDE 'exec/resident.i'
        INCLUDE 'exec/libraries.i'
        INCLUDE 'exec/initializers.i'
        INCLUDE 'exec/ports.i'
        INCLUDE 'exec/memory.i'
        INCLUDE 'exec/execbase.i'

        INCLUDE 'graphics/gfxbase.i'

        INCLUDE 'intuition/preferences.i'

        INCLUDE 'utility/utility.i'

        INCLUDE 'devices/timer.i'

        INCLUDE 'hardware/cia.i'
        INCLUDE 'hardware/custom.i'
        INCLUDE 'hardware/intbits.i'

        INCLUDE 'resources/potgo.i'

        INCLUDE 'a/JoyPortInternal.i'
        INCLUDE 'macros.i'
        INCLUDE 'lowlevel.i'
        INCLUDE 'lowlevelbase.i'                ;Library base.
        INCLUDE 'lowlevel_rev.i'                ;Version string.

                XREF    _custom
                                ;a/SystemControl.asm
                XREF    SystemControlA
                                ;a/Timer.asm
                XREF    RemTimerInt
                XREF    AddTimerInt
                XREF    StopTimerInt
                XREF    StartTimerInt
                XREF    ElapsedTime
        IFD     MC68000
                XREF    ElapsedTime000
        ENDC
                XREF    CIAARESOURCE
                                ;a/ReadJoyPort.asm
                XREF    ReadJoyPort
                XREF    RJP_VBlank_Int
                XREF    SetJoyPortAttrs
                                ;a/Keyboard.asm
                XREF    GetKey
                XREF    QueryKeys
                XREF    AddKBInt
                XREF    RemKBInt
                XREF    KeyboardInterruptExtender
                                ;a/VBlank.asm
                XREF    AddVBlankInt
                XREF    RemVBlankInt
                                ;a/CDReboot.asm
                XREF    CDRebootDefault
                                ;a/Requesters.asm
                XREF    KillReq
                XREF    RestoreReq
                                ;EndMarker.ob
                XREF    LowLevel_end            ;Label placed at the end of the library at link time.

                                ;Amiga.lib
                XREF    _LVOElapsedTime
                XREF    _LVOEasyRequestArgs

                XREF	_LVOFreeNVData
*
*******************************************************************************
*
* Just a block to prevent illegal entry...
*
                moveq   #-1,d0
                rts
*
*******************************************************************************
*
* Also used for the Reserved function...
*
entry_reserved: moveq   #0,d0
                rts

*
*******************************************************************************
*
initLDescrip:
                dc.w    RTC_MATCHWORD   ; RT_MATCHWORD
                dc.l    initLDescrip    ; RT_MATCHTAG
                dc.l    LowLevel_end    ; RT_ENDSKIP
                dc.b    RTF_COLDSTART|RTF_AUTOINIT      ; RT_FLAGS
                dc.b    VERSION         ; RT_VERSION
                dc.b    NT_LIBRARY      ; RT_TYPE
                dc.b    1               ; RT_PRI
                dc.l    subsysName      ; RT_NAME
                dc.l    VERSTR          ; RT_IDSTRING
                dc.l    inittable       ; RT_INIT
*
*******************************************************************************
*
inittable:      dc.l ll_SIZE
                dc.l vectors
                dc.l initStruct
                dc.l initFunc
*
*******************************************************************************
*
* Define the vectors as needed...
*
V_DEF           MACRO
                DC.W    entry_\1-vectors
                ENDM
VEC_DEF         MACRO
                DC.W    \1+(*-vectors)
                ENDM

vectors:
                DC.W    -1
                V_DEF   open
                V_DEF   close
                V_DEF   expunge
                V_DEF   reserved
                VEC_DEF ReadJoyPort
                VEC_DEF GetLanguageSelection
                VEC_DEF SetLanguageSelection
                VEC_DEF GetKey
                VEC_DEF QueryKeys
                VEC_DEF AddKBInt
                VEC_DEF RemKBInt
                VEC_DEF SystemControlA
                VEC_DEF AddTimerInt
                VEC_DEF RemTimerInt
                VEC_DEF StopTimerInt
                VEC_DEF StartTimerInt
                VEC_DEF ElapsedTime
                VEC_DEF AddVBlankInt
                VEC_DEF RemVBlankInt
                VEC_DEF KillReq
                VEC_DEF RestoreReq
                VEC_DEF SetJoyPortAttrs
                V_DEF   reserved
                V_DEF   reserved
                V_DEF   reserved
                V_DEF   reserved
                V_DEF   reserved
                DC.W    -1

initStruct:
                INITWORD        LIB_REVISION,REVISION
                INITBYTE        ll_TaskPri,100
                DC.L    0

*****l* Startup.asm/initFunc *************************************************
*
*   NAME
*       initFunc - Initialization function for a graphic library structure
*
*   SYNOPSIS
*       InitLib = SystemControlA (LibBase, SegList, ExecBase)
*       D0                        D0       A1       A6
*
*   FUNCTION
*       After handling the standard library intialization the lowlevel library
*       specific initialization is done.
*
*   INPUTS
*       LibBase - Pointer to the memory allocated for the library base.
*       SegList - Pointer to the memory allocated and loaded with the
*                 library code.
*       ExecBase - Pointer to the base of exec.library.
*
*   RESULT
*       Either returns the library base if sucessful, or NULL if there is
*       a problem in intialization.
*
******************************************************************************
initFunc:       movem.l d7/a5/a6,-(sp)          ;Need two address registers.
                move.l  d0,a5                   ;Get library base into a5.
                move.l  a6,ll_ExecBase(a5)      ;Store ExecBase.
                move.l  a0,ll_SegList(a5)       ;Save seglist.


                                ;lowlevel library specific initialization.
        IFD     MC68000
                move.w  AttnFlags(a6),d0
                and.w   #AFF_68020!AFF_68030!AFF_68040,d0
                bne.s   2$
                                ;Replace 68020 ElapsedTime with 68000 ElapsedTime
                move.l  a5,a1
                move.w  #_LVOElapsedTime,d0     ;load a function offset
                lea     ElapsedTime000(pc),a0   ;load a the replacement function POINTER
                exg     a0,d0
                JSRLIB  SetFunction             ;do the magic
2$:
        ENDC
                lea     POTGORESOURCE(pc),a1
                JSRLIB  OpenResource
                move.l  d0,ll_PotGoResource(a5) ;Get a pointer to the POTGO resource.
                                ;
                                ;Initialize the structures for handling the
                                ;reading of the ports.  This includes the structures
                                ;used in InstallInputMap() and the GamePort
                                ;VBlank interrupt replacement.
                                ;
                move.b  #JP_STATUSF_FIRST_INIT_REQ,ll_Port0State+lljp_Status(a5)
                move.b  #JP_STATUSF_FIRST_INIT_REQ,ll_Port1State+lljp_Status(a5)
                moveq.l #0,d0
                move.l  d0,ll_Port0State+lljp_RJPVector(a5)
                move.l  d0,ll_Port1State+lljp_RJPVector(a5)
                move.l  d0,ll_Port2State+lljp_RJPVector(a5)
                move.l  d0,ll_Port3State+lljp_RJPVector(a5)
                lea     ll_Port0State+lljp_AddSafely(a5),a0
                JSRLIB  InitSemaphore
                lea     ll_Port1State+lljp_AddSafely(a5),a0
                JSRLIB  InitSemaphore
                lea.l   ll_MsgPortSemaphore(a5),a0
                JSRLIB  InitSemaphore
                move.b  #NT_INTERRUPT,ll_RJP_Vblank_IS+LN_TYPE(a5)
                move.l  LN_NAME(a5),ll_RJP_Vblank_IS+LN_NAME(a5)
                lea     RJP_VBlank_Int(pc),a1
                move.l  a1,ll_RJP_Vblank_IS+IS_CODE(a5)
                move.l  a5,ll_RJP_Vblank_IS+IS_DATA(a5)
                moveq   #-1,d0
                                ;Nesting counts start at -1.
                move.b  d0,ll_NestOwn(a5)
                move.b  d0,ll_NestReq(a5)
                move.b  d0,ll_NestKillIn(a5)
                move.b  d0,ll_Port0State+lljp_NestCnt(a5)
                move.b  d0,ll_Port1State+lljp_NestCnt(a5)
                move.w  d0,ll_LastKey+2(a5)     ;LastKey set to FF.
                                ;
                                ;Set up the structures used in controlling the
                                ;extended keyboard interrupt.
                                ;
                move.b  #NT_INTERRUPT,ll_KBExtend+LN_TYPE(a5)
                move.l  LN_NAME(a5),ll_KBExtend+LN_NAME(a5)
                move.l  a5,ll_KBExtend+IS_DATA(a5)      ;Data is the lowlevel.library base.
                lea     KeyboardInterruptExtender(pc),a1
                move.l  a1,ll_KBExtend+IS_CODE(a5)
                lea     ll_KBInterrupts(a5),a1
                NEWLIST a1                      ;Initialize the list of user keyboard interrupts.
                                ;
                                ;In order to kill all requesters we actually need
                                ;to patch intuition.  Since intuition does not
                                ;expunge there is no need to hold it open after
                                ;we get the information we need.
                                ;However, since it is bad form to un-SetFunction
                                ;we leave the patch in memory.  Since we occasionally
                                ;need a port structure, the patch is stuck onto
                                ;a port.  This makes it possible to find it.
                                ;
                move.l  LN_NAME(a5),a1
                JSRLIB  FindPort
                move.l  d0,ll_ERSetFunction(a5)
                bne.w   SetFunced               ;If we found our port, then we have the setfunction data.

                move.l  #llsf_Size+VERSTR-subsysName,d0
                move.l  #MEMF_CLEAR,d1
                JSRLIB  AllocMem                ;Get the memory required for the port and patch.
                move.l  d0,ll_ERSetFunction(a5)
                beq     init_fail
                                ;Got enough memory.  Initialize the port.
                move.l  d0,a1
                move.l  #PA_IGNORE,MP_FLAGS(a1)
                move.b  #-1,LN_PRI(a1)
                lea     MP_MSGLIST(a1),a0
                NEWLIST a0
                move.l  a1,-(sp)
                lea     llsf_Size(a1),a0
                move.l  a0,LN_NAME(a1)
                move.l  LN_NAME(a5),a1
CopyName:       move.b  (a1)+,(a0)+             ;Copy the name of the library to the port.
                bne.s   CopyName
                                ;
                                ;Name is all copied.  The port initialization is
                                ;now complete.  So let's add the port to the public
                                ;list.  Then we can start on the patch.
                                ;
                move.l  (sp)+,a1
                JSRLIB  AddPort
                lea     IntuitionName(pc),a1
                move.l  #37,d0                  ;Any version.
                JSRLIB  OpenLibrary
                tst.l   d0                      ;EasyRequest does not exist before v37 in Intuition.
                beq.w   init_fail               ;If NULL, we failed!

                move.l  ll_ERSetFunction(a5),a1
                lea     llsf_RTS(a1),a0         ;The code must be complete before we
                move.l  a0,llsf_Routine(a1)     ;SetFunction, so just RTS immediately.
                move.l  #$2F3AFFFA,llsf_Move(a1)
                move.w  #$7000,llsf_Clear(a1)
                move.w  #$4E75,llsf_RTS(a1)
                lea     llsf_Move(a1),a1
                move.l  #_LVOEasyRequestArgs,a0
                exg     d0,a1
                movem.l d0/a0/a1,-(sp)
                JSRLIB  CacheClearU             ;We've just created code in the data cache, flush them.
                movem.l (sp)+,d0/a0/a1
                JSRLIB  SetFunction
                move.l  ll_ERSetFunction(a5),a1
                move.l  d0,llsf_Original(a1)
                move.l  d0,llsf_Routine(a1)     ;Change the patch to point at the orginal vector.
                JSRLIB  CacheClearU             ;We've just changed code in the data cache, flush.
SetFunced:                      ;EasyRequest is now properly SetFunctioned.
                lea     UtilName(pc),a1         ;"utility.library"
                JSRLIB  OldOpenLibrary
                move.l  d0,ll_UtilBase(a5)      ;Store utility base
                beq.w   init_fail               ;If NULL, we failed!
                                ;
                                ;Found and opened Utility Library.
                                ;ReadJoyPort() needs a counter for VBlanks.
                                ;
                lea     GFXNAME(pc),a1
                move.l  #40,d0
                JSRLIB  OpenLibrary
                tst.l   d0                      ;There is a VBlank counter in v40 of graphics.

        IFGT     DISK_BASED
                                ;
                                ;If we are disk based then we need to be able to
                                ;fall back if graphics v40 is not present.
                                ;
                bne.s   init_UseGFX
                                ;The fallback is adding yet another VBlank interrupt.
                lea     ll_VBlankCNTR(a5),a1
                move.b  #NT_INTERRUPT,LN_TYPE(a1)
                move.l  LN_NAME(a5),LN_NAME(a1) ;Yet something else sharing the name of the library.
                lea     VBlankCNTR(pc),a0
                move.l  a0,IS_CODE(a1)
                lea     ll_VBlankCount(a5),a0
                move.l  a0,IS_DATA(a1)
                moveq   #INTB_VERTB,d0
                JSRLIB  AddIntServer
                lea     ll_VBlankCount(a5),a1   ;Move the address of the actual counter into a1.
                bra.s   init_SetVBCntr
        ELSE
                                ;Fail if we are ROM and graphics v40 is not present.
                beq.w   init_NoCounter
        ENDC

init_UseGFX:    move.l  d0,a0
                lea     gb_VBCounter(a0),a1     ;Set a1 to point to the actual counter.
init_SetVBCntr: move.l  a1,ll_VBCounter(a5)     ;Save the address of the counter into the library base.
*                bsr     CDRebootDefault         ;Establish the default behaviour of the system for when the CD is removed.
                move.w  #-1,ll_DefaultCDReboot(a5) ; flag it as "unknown"
                                ;Need some temporary variables.
TEMP_SIZE       set     0
                ARRAYVAR Request,IOTV_SIZE
                ALLOCLOCALS sp                  ;Need a timerrequest to open the timer device.
                lea     TIMERDEVICE(pc),a0
                move.l  sp,a1
                move.l  #UNIT_MICROHZ,d0
                moveq   #0,d1
                JSRLIB  OpenDevice              ;Open the microhz timer.
                tst.b   d0                      ;Test the return value from OpenDevice.
                                ;Successful open of timer.device.
                bne.s   NoTimer
                move.l  Request+IO_DEVICE(sp),a6
                move.l  a6,ll_TimerDevice(a5)   ;Save the pointer to the timer device.
                move.l  sp,a0
                JSRLIB  ReadEClock
                move.l  d0,ll_EClockConversion(a5)      ;Get the number of E clocks per second.
                                ;Open keyboard.device.  Reuse the timerrequest.
                move.l  ll_ExecBase(a5),a6
                lea     KEYBDEVICE(pc),a0
                move.l  sp,a1                   ;Still have temporary variables on the stack.
                moveq   #0,d0
                move.l  d0,d1
                JSRLIB  OpenDevice              ;Open keyboard.device.
                tst.b   d0                      ;Test the return value from OpenDevice.
                bne.s   NoKeyBoard
                                ;Successful open of keyboard.device.
                move.l  Request+IO_DEVICE(sp),ll_KBDevice(a5)
                                ;
                                ;CIAA resource timer B_SP is used by keyboard.device.
                                ;the interrupt attached there is the keyboard
                                ;interrupt.  AddICRVector to this timer will return
                                ;a pointer to the interrupt structure.
                                ;
                lea     CIAARESOURCE(pc),a1
                JSRLIB  OpenResource
                move.l  d0,a6
                move.w  #CIAICRB_SP,d0
                JSRLIB  AddICRVector
                move.l  d0,ll_KBOrig(a5)        ;Save the normal keyboard interrupt.
                move.l  d0,a1
                move.w  #CIAICRB_SP,d0
                JSRLIB  RemICRVector            ;Remove the keyboard interrupt service routine.
                lea     ll_KBExtend(a5),a1
                move.w  #CIAICRB_SP,d0
                JSRLIB  AddICRVector            ;Replace it with ours.
                move.l  a5,d0                   ;Set return value
                DONELOCALS sp                   ;Clear the stack.
                bra.s   init_go

NoKeyBoard:                     ;Could not open keyboard.device, however ...
                                ;timer.device was opened so close it before failing.

                move.l  ll_TimerDevice(a5),Request+IO_DEVICE(sp)
                move.l  sp,a1
                JSRLIB  CloseDevice             ;Close the microhz timer.
NoTimer:                        ;Timer.device is not open, however...
                DONELOCALS sp                   ;Clear the stack.
init_NoCounter:                 ;utility.libary was opened so close it before failing.
                move.l  ll_UtilBase(a5),a1
                JSRLIB  CloseLibrary            ;Close the utility library.
init_fail:      moveq   #0,d0                   ;Mark failure.
init_go         movem.l (sp)+,d7/a5/a6          ;Restore the address registers.
                rts

*****l* Startup.asm/entry_open ***********************************************
*
*   NAME
*       entry_open - Open the lowlevel library.
*
*   SYNOPSIS
*       LowLevelBase = entry_open(LowLevelBase)
*       D0                        A6
*
*   FUNCTION
*       Increments the library count and returns the pointer to the
*       library base.
*
*   INPUTS
*       LowLevelBase - Pointer to the base of the LowLevel library.
*
*   RESULT
*       Returns the library base.
*
******************************************************************************
entry_open:
                addq.w  #1,LIB_OPENCNT(a6)      ;Count the open
                move.l  a6,d0                   ;Return library base
                rts

*****l* Startup.asm/entry_close **********************************************
*
*   NAME
*       entry_close - Close the lowlevel library.
*
*   SYNOPSIS
*       entry_close(LowLevelBase)
*                   A6
*
*   FUNCTION
*       Decrements the library count.
*
*       This library does not autoexpunge.
*
*   INPUTS
*       LowLevelBase - Pointer to the base of the low level library.
*
*   RESULT
*       NONE
*
*
******************************************************************************
entry_close:    subq.w  #1,LIB_OPENCNT(a6)      ;Count the close
                moveq   #0,d0
                rts

*****l* Startup.asm/entry_expunge ********************************************
*
*   NAME
*       entry_expunge - Remove the lowlevel library from memory.
*
*   SYNOPSIS
*       entry_expunge(LowLevelBase)
*                     A6
*
*   FUNCTION
*       All of library specific resources are returned.  Then the library
*       size is calculated and the library is removed from the list of
*       libraries and the memory for the library is freed.
*
*       The ROM based version of this library cannot be expunged.
*
*   INPUTS
*       LowLevelBase - Pointer to the base of the LowLevel library.
*
*   RESULT
*       NONE
*
******************************************************************************
entry_expunge:

                moveq   #0,d0

        IFGT    DISK_BASED
                tst.w   LIB_OPENCNT(a6)         ;Check if Zero opens.
                bne.w   expunge_done            ;If non-zero, we don't go

                movem.l a5-a6,-(sp)             ;Need a couple of address registers.
                move.l  a6,a5
                move.l  ll_ExecBase(a5),a6
                lea     CIAARESOURCE(pc),a1
                JSRLIB  OpenResource
                move.l  d0,a6
                move.w  #CIAICRB_SP,d0
                lea     ll_KBExtend(a5),a1
                JSRLIB  RemICRVector            ;Remove our keyboard service routine.
                move.l  ll_KBOrig(a5),a1
                move.l  #CIAICRB_SP,d0
                JSRLIB  AddICRVector            ;Put back the original keyboard service routine.
                move.l  ll_ExecBase(a5),a6
                move.l  ll_UtilBase(a5),a1
                JSRLIB  CloseLibrary            ;Close utility.library.
                                ;
                                ;If we added our own VBlank handler let's remove it.
                                ;If not we ought to be good and close graphics.
                                ;
                lea     GFXNAME(pc),a1
                move.l  #40,d0
                JSRLIB  OpenLibrary
                tst.l   d0
                beq.s   RemoveVBCNTR            ;Our handler was added iff graphics v40 could not be opened.
                                ;
                                ;If we could open graphics v40, then we need to close
                                ;it twice.  Once for this open and once for the open
                                ;at our init time.
                                ;
                move.l  d0,-(sp)
                move.l  d0,a1
                JSRLIB  CloseLibrary
                move.l  (sp)+,a1
                JSRLIB  CloseLibrary
                bra.s   DoneVBCNTR

RemoveVBCNTR:   lea     ll_VBlankCNTR(a5),a1
                moveq   #INTB_VERTB,d0
                JSRLIB  RemIntServer            ;Remove our VBlank handler
DoneVBCNTR:

TEMP_SIZE       set     0
                ARRAYVAR Request,IOTV_SIZE
                ALLOCLOCALS sp                  ;Need a timerrequest to close the timer device.
                move.l  ll_TimerDevice(a5),Request+IO_DEVICE(sp)
                move.l  sp,a1
                JSRLIB  CloseDevice             ;Close the microhz timer.
                move.l  ll_KBDevice(a5),Request+IO_DEVICE(sp)
                move.l  sp,a1
                JSRLIB  CloseDevice             ;Close keyboard.device.
                DONELOCALS sp                   ;Clear the stack.
                                ;Clean up any mischief caused by ReadJoyPort().
                bclr    #JP_STATUSB_VBLANK_ACQUIRED,ll_Port0State+lljp_Status(a5)
                beq.s   4$                      ;Did we replace the gameport VBlank interrupt.

                moveq   #INTB_VERTB,d0
                lea     ll_RJP_Vblank_IS(a5),a1
                JSRLIB  RemIntServer            ;Remove our replacement.
                moveq   #INTB_VERTB,d0
                move.l  ll_RJP_Saved_IS(a5),a1
                JSRLIB  AddIntServer            ;Restore the orginal.
                bset    #JP_STATUSB_FIRST_INIT_REQ,ll_Port0State+lljp_Status(a5)        ;Do not try to return the POTGO bits.
4$:             move.l  ll_PotGoResource(a5),a6
                btst    #JP_STATUSB_FIRST_INIT_REQ,ll_Port0State+lljp_Status(a5)
                bne.s   ee_NotAlloc0            ;Were the Potgo bits allocated for port 0.
                                ;Yes, so return them.
                move.w  #$0F00,d0
                JSRLIB  FreePotBits
ee_NotAlloc0:   btst    #JP_STATUSB_FIRST_INIT_REQ,ll_Port1State+lljp_Status(a5)
                bne.s   ee_NotAlloc1            ;Were the Potgo bits allocated for port 1.
                                ;Yes so return them.
                move.w  #$F000,d0
                JSRLIB  FreePotBits
ee_NotAlloc1:   move.l  ll_ExecBase(a5),a6
                                ;Free my own library (we have no openers)
                move.l  a5,a1
                CLEAR   d0
                move.w  LIB_NEGSIZE(a5),d0      ;Get negative size
                sub.l   d0,a1                   ;Point at start of library
                add.w   LIB_POSSIZE(a5),d0      ;Add in positive size
                                ;Now, set up for the exit...
                movem.l d2,-(sp)                ;Save
                move.l  ll_SegList(a5),d2       ;Get seglist...
                movem.l d0/a1,-(sp)             ;Save these...
                move.l  a5,a1                   ;We need to remove ourselves from
                JSRLIB  Remove                  ;the library list...
                movem.l (sp)+,d0/a1             ;Restore these...
                JSRLIB  FreeMem                 ;Free the memory...
                move.l  d2,d0                   ;Get seglist into return...
                movem.l (sp)+,d2                ;Restore
                movem.l (sp)+,a5-a6

        ENDC
expunge_done:   rts                             ;Return from expunge...


*****i* lowlevel.library/SetLanguageSelection ********************************
*
*   NAME
*       SetLanguageSelection - Sets the current language selection
*
*   SYNOPSIS
*       SetLanguageSelection (Language)
*                             d1
*
*   FUNCTION
*       Sets the default language in utility.library.
*
*   NOTE
*       This function must be called by a process.
*
*   INPUTS
*       Language - User specified language.
*
*   RESULT
*       NONE
*
******************************************************************************
SetLanguageSelection
                movem.l d1/a2/a5/a6,-(sp)
                move.l  a6,a5
                move.l  ll_UtilBase(a5),a0
                move.b  d1,ub_Language(a0)      ;Set the language in utility base.
                                ;
                                ;Set the language in nonvolatile.library.  This
                                ;cannot be done reliably unless we are a process.
                                ;
                move.l  ll_ExecBase(a5),a6
                lea     NVName(pc),a1
                JSRLIB  OldOpenLibrary
                move.l  d0,-(sp)
                beq.s   NoNV
                                ;Alright, nonvolatile library opened.
                move.l  d0,a6
                lea     VERSTR-1(pc),a0
                move.l  a0,a1
                CLEAR   d1
                JSRLIB  GetCopyNV               ;Get the system area from the NVRAM.
                move.l  4(sp),d1                ;Get the language back.
                move.l  d0,-(sp)
                beq.s   sls_Exit
                                ;Get the system area.
                move.l  d0,a2
                move.b  d1,(a2)                 ;Store the language in the system area.
                lea     VERSTR-1(pc),a0
                move.l  a0,a1
                CLEAR   d1
                JSRLIB  StoreNV                 ;Store back the system area to the NVRAM.
sls_Exit:       move.l  (sp)+,a0                ;Restore the pointer to the system area.
                JSRLIB  FreeNVData              ;Free whatever GetCopyNV returned.
NoNV:           move.l  (sp)+,a1                ;Restore nonvolatile.library base.
                move.l  ll_ExecBase(a5),a6
                JSRLIB  CloseLibrary            ;Close nonvolatile.
                movem.l (sp)+,d1/a2/a5/a6
                rts

******* lowlevel.library/GetLanguageSelection ********************************
*
*   NAME
*	GetLanguageSelection -- returns the current language selection. (V40)
*
*   SYNOPSIS
*	language = GetLanguageSelection();
*	D0
*
*	ULONG GetLanguageSelection (VOID);
*
*   FUNCTION
*	Determine what the user has specified as a language.
*
*   RESULT
*	language - user specified language, or zero if none has yet been
*		   specified. See <libraries/lowlevel.h> for a definition
*		   of the currently supported language.
*
*   SEE ALSO
*	<libraries/lowlevel.h>, locale.doc
*
******************************************************************************
GetLanguageSelection
                move.l  ll_UtilBase(a6),a0
                moveq.l #0,d0
                move.b  ub_Language(a0),d0
                rts

*****l* lowlevel.library/VBlankCNTR ******************************************
*
*   NAME
*       VBlankCNTR - Counts VBlanks
*
******************************************************************************
VBlankCNTR
                addq.l  #1,(a1)
                CLEAR   d0
                rts

subsysName      LOWLEVELNAME
VERSTR:         VSTRING
UtilName:       dc.b    'utility.library',0
NVName          dc.b    'nonvolatile.library',0
IntuitionName   dc.b    'intuition.library',0
GFXNAME         dc.b    'graphics.library',0
TIMERDEVICE     dc.b    'timer.device',0
KEYBDEVICE      dc.b    'keyboard.device',0
POTGORESOURCE:  POTGONAME
                END
