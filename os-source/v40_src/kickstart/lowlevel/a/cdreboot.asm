******************************************************************************
*
*       $Id: cdreboot.asm,v 40.5 93/07/30 16:04:33 vertex Exp $
*
******************************************************************************
*
*       $Log:	cdreboot.asm,v $
* Revision 40.5  93/07/30  16:04:33  vertex
* Autodoc and include file cleanup
* 
* Now includes <devices/cd.i> instead of bogusly-pathed file...
* 
* Revision 40.4  93/05/18  15:04:53  gregm
* Jim was move.l'ing into the word-sized field ll_CDRebootDefault.
* He was also using a bmi bpl pair that I replaced with blo, bhi.
* And lastly, I modified the code to determine the default the first
* time it's needed, rather than at init time -- as Jerry explained
* to me that things wouldn't be properly set up at init time.
*
* Revision 40.3  93/05/05  09:44:25  gregm
* fixes problems with systemcontrol's cdreboot tag -- Jim was using
* move.l's into IO_COMMAND instead of move.w's.
*
* Revision 40.2  93/03/31  18:52:06  Jim2
* Added a replyport to the DoIO message. (CI33).
*
* Revision 40.1  93/03/12  20:53:24  Jim2
* When simplifing the calling routine LowLevelBase stays in a5 and
* ExecBase appears in a6.
*
* Revision 40.0  93/03/02  13:22:32  Jim2
* Changed all references from game.library to lowlevel.library
*
* Revision 39.4  93/01/19  10:40:22  Jim2
* Was mainpulating the stack incorrectly in CDRebootOnRemove.
*
* Revision 39.3  93/01/18  13:34:41  Jim2
* Added CDRebootDefault.  Changed name for game library field
* that has the CD default behavior.
*
* Revision 39.2  93/01/07  14:23:49  Jim2
* Cleaned up comments and minimized the required include files.
*
* Revision 39.1  92/12/14  14:55:23  Jim2
* Tested failure, non-existant cd.device.
*
* Revision 39.0  92/12/11  14:02:00  Jim2
* Inital Release
*
*
*       (C) Copyright 1992,1993 Commodore-Amiga, Inc.
*           All Rights Reserved
*
******************************************************************************

        INCLUDE 'exec/macros.i'
        INCLUDE 'exec/io.i'
        INCLUDE 'exec/tasks.i'

        INCLUDE 'utility/tagitem.i'

        INCLUDE 'devices/cd.i'

        INCLUDE '/macros.i'
        INCLUDE '/lowlevelbase.i'


                XDEF    CDRebootOnRemove
                XDEF    CDRebootDefault


*****i* a/CDReboot.asm/CDRebootOnRemove **************************************
*
*   NAME
*       CDRebootOnRemove - Set the expected behaviour upon ejecting the CD.
*
*   SYNOPSIS
*       CDRebootOnRemove (resetState, LowLevelBase, ExecBase)
*                         D1          a5            a6
*
*   FUNCTION
*       Uses cd.device to change the system response to ejecting the CD.
*
*   INPUTS
*       resetState - Desired behavior.
*           0 - Do not reboot
*           1 - Reboot
*           2 - Reset to default behaviour.
*
*   RESULT
*       error - Zero if no error, otherwise sign extended io_error field from
*               the request.
*
******************************************************************************
CDRebootOnRemove

                move.l  d1,-(sp)
                move.w  ll_DefaultCDReboot(a5),d0
                bge     9$
                jsr     CDRebootDefault
9$
                move.l  (sp)+,d1

                moveq   #-1,d0                  ;Set return value.
                cmp.l   #2,d1
                bcs.s   NotDefault              ; blo.s   ---\ This is what he REALLY  0, 1
                bhi     CDROR_Exit              ;         ---/ wanted.  - Greg  for invalid codes

                                ;The user has requested the default behaviour.
                                ;This has been saved in ll_DefaultCDReboot at
                                ;library init time.


                moveq.l #0,d1
                move.w  ll_DefaultCDReboot(a5),d1
NotDefault:     tst.l   d1
                bmi     CDROR_Exit              ;No negative values should be passed.
                                ;d1 contains a valid desired reboot state (0,1).
ValidParameter:


TEMP_SIZE       set     0                       ;Determine the number of bytes wanted for local variables
                ARRAYVAR Request,IOSTD_SIZE     ;Need an standard IO Requester.
                LONGVAR EjectTag                ;A tag list starting with the eject tag.
                LONGVAR EjectParameter
                LONGVAR LastTag                 ;And ending with the end tag.

                ALLOCLOCALS sp

                move.l  d1,-(sp)
                lea.l   ll_MsgPortSemaphore(a5),a0
                JSRLIB  ObtainSemaphore
                move.l  (sp)+,d1

                CLEARA  a1
                move.l  d1,-(sp)
                JSRLIB  FindTask
                move.l  (sp)+,d1
                move.l  ll_ERSetFunction(a5),a0
                move.l  d0,MP_SIGTASK(a0)       ; SigTask
                move.b  #SIGB_SINGLE,MP_SIGBIT(a0)      ; SigBit
                move.b  #PA_SIGNAL,MP_FLAGS(a0) ; Signal...
                move.l  a0,Request+MN_REPLYPORT(sp)

                                ;Initialize the tag list
                move.l  #TAG_DONE,LastTag_l(sp)
                move.l  d1,EjectParameter_l(sp)
                move.l  #TAGCD_EJECTRESET,EjectTag_l(sp)
                lea     deviceName(pc),a0
                moveq   #0,d0
                lea     Request(sp),a1
                move.l  d0,d1
                JSRLIB  OpenDevice              ;OpenDevice(a0>ptr devicename, a1>ptr IOStdReq, d0>UnitNumber, d1>flags, a6>ExecBase)
                tst.l   d0
                bne.s   CDROR_Failed
                                ;Opened cd.device.
                move.w  #CD_CONFIG,Request+IO_COMMAND(sp)
                lea     EjectTag_l(sp),a1
                move.l  a1,Request+IO_DATA(sp)
                move.l  #0,Request+IO_LENGTH(sp)
                lea     Request(sp),a1
                JSRLIB  DoIO                    ;DoIO(a1>ptr to IOStdReq, a6>ExecBase)
                lea     Request(sp),a1
                move.l  d0,-(sp)                ;Store the return value of DoIO as return value for this funciton.
                JSRLIB  CloseDevice             ;CloseDevice(a1>ptr to IOStdReq, a6>ExecBase)
                move.l  (sp)+,d0                ;Restore return value.

CDROR_Failed:   move.l  Request+MN_REPLYPORT(sp),a0
                DONELOCALS sp
                move.b  #PA_IGNORE,MP_FLAGS(a0)
CDROR_Exit:
                move.l  d0,-(sp)
                lea.l   ll_MsgPortSemaphore(a5),a0
                JSRLIB  ReleaseSemaphore
                move.l  (sp)+,d0

                rts

*****i* a/CDReboot.asm/CDRebootDefault ***************************************
*
*   NAME
*       CDRebootDefault - Queries to establish the default reboot behaviour.
*
*   SYNOPSIS
*       CDRebootDefault (LowLevelBase, ExecBase)
*                        a5            a6
*
*   FUNCTION
*       Uses cd.device determine the current CD reboot behaviour and sets
*       ll_DefaultCDReboot.
*
*   INPUTS
*       LowLevelBase - Pointer to lowlevel.library
*       ExecBase - Pointer to exec.library
*
*   RESULT
*       NONE
*
******************************************************************************
CDRebootDefault
TEMP_SIZE       set     0                       ;Determine the number of bytes wanted for local variables
                ARRAYVAR Request,IOSTD_SIZE     ;Need an standard IO Requester.
                ARRAYVAR Data,CDINFO_Reserved1  ;and room for the return data.

                lea.l   ll_MsgPortSemaphore(a5),a0
                JSRLIB  ObtainSemaphore

                ALLOCLOCALS sp
                CLEARA  a1
                JSRLIB  FindTask
                move.l  ll_ERSetFunction(a5),a0 ;Get our general port structure.
                move.l  d0,MP_SIGTASK(a0)       ; SigTask
                move.b  #SIGB_SINGLE,MP_SIGBIT(a0)      ; SigBit
                move.b  #PA_SIGNAL,MP_FLAGS(a0) ; Signal...
                move.l  a0,Request+MN_REPLYPORT(sp)
                lea     deviceName(pc),a0
                moveq   #0,d0
                lea     Request(sp),a1
                move.l  d0,d1
                JSRLIB  OpenDevice              ;OpenDevice(a0>ptr devicename, a1>ptr IOStdReq, d0>UnitNumber, d1>flags, a6>ExecBase)
                tst.l   d0
                bne.s   Failed
                                ;Opened cd.device.
                move.w  #CD_INFO,Request+IO_COMMAND(sp)
                lea     Data(sp),a1
                move.l  a1,Request+IO_DATA(sp)
                move.l  #CDINFO_Reserved1,Request+IO_LENGTH(sp)
                lea     Request(sp),a1
                JSRLIB  DoIO                    ;DoIO(a1>ptr to IOStdReq, a6>ExecBase)
                lea     Request(sp),a1

                move.l  d0,-(sp)                ;Store the return value of DoIO as return value for this funciton.
                JSRLIB  CloseDevice             ;CloseDevice(a1>ptr to IOStdReq, a6>ExecBase)
                move.l  (sp)+,d0                ;Restore return value.
                beq.s   Succeed
Failed:         moveq   #0,d0
                bra.s   Exit
Succeed:        move.w  Data+CDINFO_EjectReset(sp),d0
Exit:           move.l  Request+MN_REPLYPORT(sp),a0
                DONELOCALS sp
                move.b  #PA_IGNORE,MP_FLAGS(a0)
                move.w  d0,ll_DefaultCDReboot(a5)

                lea.l   ll_MsgPortSemaphore(a5),a0
                JSRLIB  ReleaseSemaphore

                rts

deviceName      dc.b    'cd.device',0


                END
