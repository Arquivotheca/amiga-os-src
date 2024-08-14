******************************************************************************
*
*       $Id: lowlevelbase.i,v 40.9 93/07/30 16:11:27 vertex Exp $
*
******************************************************************************
*
*       $Log:	lowlevelbase.i,v $
* Revision 40.9  93/07/30  16:11:27  vertex
* Autodoc and include cleanup
* 
* Revision 40.8  93/05/05  09:57:00  gregm
* added fields for RJPVector -- the readjoyport() locking jump vector.
* added structs for port2, 3.
*
* Revision 40.7  93/04/19  13:40:32  Jim2
* Moved the storage location for the orginal easyrequest vector
* from the library base to the setfunction memory.
*
* Revision 40.6  93/03/31  18:52:52  Jim2
* Extend the library base for the disk version to support the
* VBlank counter (CI105).  Also added macros for debugging.
*
* Revision 40.5  93/03/26  17:43:31  Jim2
* Added field to allow for VBlank counting.  Also to support
* opening the PotGo resource at init time.
*
* Revision 40.4  93/03/19  11:36:14  Jim2
* Yanked out the unused field ll_WaitPort.  Added a label for
* LL_Port2State so I could write the double controller code
* for now.
*
* Revision 40.3  93/03/12  20:07:11  Jim2
* Added the structure for supporting the joystick to rawkey convert
* routines.
*
* Revision 40.2  93/03/10  19:55:29  Jim2
* Declared the structure for holding the setfuction code.  Also took
* an extra byte to record General ReadJoyPort status.
*
* Revision 40.1  93/03/08  10:37:07  Jim2
* Added the new fields for setfunctioning EasyRequest.
*
* Revision 40.0  93/03/02  13:12:14  Jim2
* Renamed from GameBase.i.
*
* Revision 39.10  93/01/19  15:51:30  Jim2
* Removed language from structure.  Added nest count for killing
* input.device.
*
* Revision 39.9  93/01/18  13:36:26  Jim2
* Changed field that hold CD default behavior.
*
* Revision 39.8  93/01/15  14:04:58  Jim2
* Added fields from ReadJoyPort manipulation of gameport.device.
* Removed the fields used for keyboard repeat.
*
* Revision 39.7  93/01/13  13:55:13  Jim2
* Added structures for handling autorepeated keystrokes.
*
* Revision 39.6  93/01/07  14:29:46  Jim2
* Cleaned up comments and the ordering of fields.
*
* Revision 39.5  93/01/05  12:15:05  Jim2
* Added fields for keyboard support.
*
* Revision 39.4  92/12/17  18:17:22  Jim2
* Added the fields needed to support the routines in Timer.asm.
*
* Revision 39.3  92/12/14  15:01:09  Jim2
* *** empty log message ***
*
* Revision 39.2  92/12/11  14:16:18  Jim2
* Removed the list of process context and added nesting
* fields for the owning of the system and the killinf
* of requesters.
*
* Revision 39.1  92/12/09  18:15:32  Jim2
* Added the filed gpbi_OpenCnt to gpbi to guard against someone
* opening the library multiple times and our throwing away
* this information on the first close.
*
* Revision 39.0  92/12/07  15:59:46  Jim2
* Initial release prior to testing.
*
*
*       (C) Copyright 1992,1993 Commodore-Amiga, Inc.
*           All Rights Reserved
*
******************************************************************************

    IFND    LOWLEVELBASE_I
LOWLEVELBASE_I  SET 1

    IFND    EXEC_LISTS_I
    include 'exec/lists.i'
    ENDC

    IFND    EXEC_LIBRARIES_I
    include 'exec/libraries.i'
    ENDC

    IFND    EXEC_PORTS_I
    include 'exec/ports.i'
    ENDC

    IFND    EXEC_INTERRUPTS_I
    include 'exec/interrupts.i'
    ENDC

    IFND    EXEC/SEMAPHORES_I
    include 'exec/semaphores.i'
    ENDC

    IFND    DEVICES_TIMER_I
    include 'devices/timer.i'
    ENDC



 STRUCTURE llct_CIATimer,0
        STRUCT  llct_TimerInt,IS_SIZE           ;Interrupt sturcture.  (word sized)
        UWORD   llct_WhichTimer                 ;Timer within the CIA where the interrupt is hung.
        APTR    llct_Resource                   ;CIA resource where the interrupt is hung.
        LABEL   llct_Size

 STRUCTURE llsf_SetFunction,0
        STRUCT  llsf_Port,MP_SIZE               ;Random message port.  Used for DOIO calls.  Allows this code to be found.
        APTR    llsf_Original                   ;Original LVO for Intuition EasyRequest.
        APTR    llsf_Routine                    ;Pointer to the current EasyRequest routine to be executed.
        ULONG   llsf_Move                       ;move   llsf_Routine(pc),-(sp)
        UWORD   llsf_Clear                      ;CLEAR  d0
        UWORD   llsf_RTS                        ;rts
        LABEL   llsf_Size

 STRUCTURE lljp_JoyPortState,0
        ULONG   lljp_PreviousValue              ;Previous return value for this port.
        ULONG   lljp_LastVBlank                 ;Value of the VBlank counter for the last mouse reading.
        APTR    lljp_InputHandle                ;Handle for the input map code, if it is installed.
        UBYTE   lljp_NestCnt                    ;Nest count for the handle.
        UBYTE   lljp_Status                     ;ReadJoyPort status information.
        STRUCT  lljp_AddSafely,SS_SIZE          ;(word sized)
        APTR    lljp_RJPVector                  ; If NZ, Jump vector for locked controller
        LABEL   lljp_SIZE


 STRUCTURE  LowLevelBase,LIB_SIZE               ;(word sized)
        UBYTE   ll_TaskPri                      ;Orginal Task priority, if a task owns the system; 127 otherwise.
        UBYTE   ll_NestOwn                      ;Nest count for owning the system; starts at -1.
        UBYTE   ll_NestReq                      ;Nest count for killing requesters; starts at -1.
        UBYTE   ll_NestKillIn                   ;Nest count for killing input.device; starts at -1.
        UWORD   ll_DefaultCDReboot              ;CD Reboot behaviour reported at init time.
        ULONG   ll_LastKey                      ;Last key code read from the keyboard including modifiers.
        ULONG   ll_EClockConversion             ;E Clocks per second, used for converting from milliseconds to E Clocks.
        APTR    ll_SegList                      ;Segment Pointer for the game library.
        APTR    ll_ExecBase                     ;Pointer to exec.library.
        APTR    ll_UtilBase                     ;Pointer to utility.library.
        APTR    ll_SystemOwner                  ;Pointer to the task that has taken over part of the system via SystemControlA.
        APTR    ll_TimerDevice                  ;Pointer to timer.device.
        APTR    ll_KBDevice                     ;Pointer to keyboard.device.
        APTR    ll_KBOrig                       ;Pointer to the original keyboard interrupt.
        APTR    ll_RJP_Saved_IS                 ;Pointer to the original gameport interrrupt.
        APTR    ll_VBlankInt                    ;Pointer to the user's VBlank interrupt.
        APTR    ll_ERSetFunction                ;Pointer to the Easy Request SetFunction code.
        APTR    ll_PotGoResource                ;Pointer to the PotGo Resource.
        APTR    ll_VBCounter                    ;Pointer to the VBlank counter.

                        ;These structures have to be sequential and the first
                        ;needs to be less than 256 bytes from the start.

        STRUCT  ll_Port0State,lljp_SIZE         ;State information on Port 0.  (word sized)
        STRUCT  ll_Port1State,lljp_SIZE         ;State information on Port 1.  (word sized)
        STRUCT  ll_CIATimer,llct_Size           ;Structure used for allocating a CIA timer interrupt.
        STRUCT  ll_KBInterrupts,MLH_SIZE        ;List of extended keyboard interrupts.
        STRUCT  ll_RJP_Vblank_IS,IS_SIZE        ;Interrupt structure for our replacement gameport interrupt. (word sized)
        STRUCT  ll_KBExtend,IS_SIZE             ;Interrupt structure for our replacement keyboard interrupt. (word sized)

        STRUCT  ll_Port2State,lljp_SIZE         ;2
        STRUCT  ll_Port3State,lljp_SIZE         ;3
        STRUCT  ll_MsgPortSemaphore,SS_SIZE     ; Used because Jim keeps using the same port over and over again without locking it

   IFD  DISK_BASED
        STRUCT  ll_VBlankCNTR,IS_SIZE
        ULONG   ll_VBlankCount
   ENDC

        LABEL   ll_SIZE

* handy name macro

LOWLEVELNAME    MACRO
                DC.B  'lowlevel.library',0
                ENDM


*******************************************************************************
*
* Define DEBUGGING for debugging of code...  (0=NO, 1=YES)
*
DEBUGGING       SET     0
*
*******************************************************************************
*
                IFND    PUTCHAR
PUTCHAR         MACRO   ; #'c'  (Character)
                IFNE    DEBUGGING
                movem.l d0/d1/a0/a1/a6,-(sp)
                move.b  \1,d0
                move.l  4.w,a6
                JSRLIB  RawPutChar
                movem.l (sp)+,d0/d1/a0/a1/a6
                ENDC    ; DEBUGGING
                ENDM    ; MACRO PUTCHAR
                ENDC    ; PUTCHAR
*
*******************************************************************************
*
* A macro for PRINTF that does not touch the registers
* Also, it only produces code when DEBUGGING is defined above
*
                IFND    PRINTf
PRINTf          MACRO   ; <string>,...
                IFNE    DEBUGGING
                XREF    KPrintF
PUSHCOUNT       SET     0

                IFNC    '\9',''
                move.l  \9,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\8',''
                move.l  \8,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\7',''
                move.l  \7,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\6',''
                move.l  \6,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\5',''
                move.l  \5,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\4',''
                move.l  \4,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\3',''
                move.l  \3,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                IFNC    '\2',''
                move.l  \2,-(a7)
PUSHCOUNT       SET     PUSHCOUNT+4
                ENDC

                movem.l a0/a1/d0/d1,-(a7)
                lea.l   PSS\@(pc),a0
                lea.l   16(a7),a1
                jsr     KPrintF
                movem.l (a7)+,a0/a1/d0/d1
                bra.s   PSE\@

PSS\@           dc.b    \1
                dc.b    10
                dc.b    0
                cnop    0,2
PSE\@
                IFNE    PUSHCOUNT
                lea.l   PUSHCOUNT(a7),a7
                ENDC    ;IFNE   PUSHCOUNT
                ENDC    ;IFD    DEBUGGING
                ENDM    ;PRINTf MACRO
                ENDC    ;IFND   PRINTf
*
*******************************************************************************

    ENDC
