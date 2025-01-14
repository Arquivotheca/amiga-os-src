************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: a2060.asm,v 1.2 92/05/06 11:14:13 gregm Exp $
*
*
************************************************************************

   include "exec/types.i"
   include "exec/devices.i"
   include "exec/io.i"
   include "exec/memory.i"
   include "exec/initializers.i"
   include "exec/resident.i"
   include "exec/errors.i"
   include "libraries/expansion.i"
   include "libraries/configvars.i"
   include "libraries/configregs.i"
   include "sana2.i"
   include "a2060.i"

TagFunctions equ 2
FuncTableSize equ (4*(TagFunctions+1))
BaseTableSize equ (6*(TagFunctions+1))

* External system stuff that we need:
      xref     _AbsExecBase

      XSYS     FreeMem
      XSYS     RemHead
      XSYS     Disable
      XSYS     Enable
      XSYS     Remove
      XSYS     OpenLibrary
      XSYS     CloseLibrary
      XSYS     AllocMem
      XSYS     MakeFunctions
      XSYS     NextTagItem
      XSYS     ReplyMsg

* External functions:
      xref     ConfigureHardware,DeconfigHardware

* External Command code:
      xref     Read,Write,DevQuery,GetStatAddress,ConfigInt,Broadcast,OnEvent,Online,Offline
      xref     TrackType,UnTrackType,GetTypeStats,GetGlobalStats,ReadOrphan,GetSpecialStats

* For External functions:
      xdef      ReplyRequest,SignalEvent

*
* In the event that somebody tries to execute this thing, fail.
*

AlwaysFail:
      moveq.l     #-1,d0
      rts


*
* The ROMTAG:
*

Romtag:
      dc.w     RTC_MATCHWORD
      dc.l     Romtag
      dc.l     EndCode
      dc.b     RTF_AUTOINIT
      dc.b     VERSION
      dc.b     NT_DEVICE
      dc.b     0
      dc.l     DevName
      dc.l     IdString
      dc.l     InitTable
EndCode:

DevName dc.b 'a2060.device',0
IdString:
 VSTRING
 VERSTAG
      cnop     0,2


InitTable:
      dc.l     DevBaseDataSize
      dc.l     functable
      dc.l     datatable
      dc.l     InitCode

functable:
      dc.l     Open
      dc.l     Close
      dc.l     Expunge
      dc.l     0

      dc.l     BeginIO
      dc.l     AbortIO

      dc.l     -1

datatable:
      INITBYTE LN_TYPE,NT_DEVICE
      INITLONG LN_NAME,DevName
      INITBYTE LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
      INITWORD LIB_VERSION,VERSION
      INITWORD LIB_REVISION,REVISION
      INITLONG LIB_IDSTRING,IdString
      dc.w     0


*
* InitCode:
*  Called when the device is allocated and brought into memory.
*
*  Input:
*        D0    DevBase
*        A0    SegList for the device
*
*  Output:
*        D0    DevBase if successful, or NULL if failed.
*
*

InitCode:
         movem.l     d1-d7/a0-a6,-(sp)
         move.l      d0,a6
         move.l      a0,ds_SegList(a6)
         move.l      _AbsExecBase,ds_SysBase(a6)

        move.l      a6,-(sp)
        move.l      ds_SysBase(a6),a6
        move.l      #512,d0
        move.l      #MEMF_PUBLIC,d1
        SYS         AllocMem
        move.l      (sp)+,a6
** Fixme -- no checking for allocmem fail
        move.l      d0,ds_GPBuffer(a6)
        tst.l       d0
        bne         0$
        move.l      d0,a6
        bra         1$
0$

        lea.l       ds_TypeStats(a6),a0
        NEWLIST     a0

         move.l      a6,-(sp)
         move.l      ds_SysBase(a6),a6
         lea.l       ExpName,a1
         moveq.l     #0,d0
         SYS         OpenLibrary
         move.l      (sp)+,a6
         move.l      d0,ds_ExpBase(a6)
         tst.l       d0
         beq         99$

         move.l      a6,-(sp)
         move.l      ds_SysBase(a6),a6
         lea.l       UtlName,a1
         moveq.l     #0,d0
         SYS         OpenLibrary
         move.l      (sp)+,a6
         move.l      d0,ds_UtlBase(a6)
         tst.l       d0
         bne         2$

         move.l      a6,-(sp)               * Close Expansion.library before quitting
         move.l      ds_SysBase(a6),a6
         move.l      ds_ExpBase,a1
         SYS         CloseLibrary
         move.l      (sp)+,a6

         bra         99$

2$
         lea.l       ds_UnitList(a6),a0
         NEWLIST     a0

         movem.l     a0-a6/d1-d7,-(sp)
         jsr         ConfigureHardware
         movem.l     (sp)+,a0-a6/d1-d7
         tst.l       d0
         bne         1$                   * No error ---> 1$

         movem.l     a5/a6,-(sp)          * If it fails, free up the
         move.l      a6,a5                * device base.
         move.l      ds_SysBase(a5),a6    *
         move.l      a5,a1
         moveq.l     #0,d0
         move.w      LIB_NEGSIZE(a5),d0
         sub.l       d0,a1
         add.w       LIB_POSSIZE(a5),d0
         SYS         FreeMem
         movem.l     (sp)+,a5/a6

99$
         movem.l     (sp)+,d1-d7/a0-a6
         moveq.l     #0,d0                * Couldn't configure; error.
         rts
1$       move.l      a6,d0                * No error, return.
         movem.l     (sp)+,d1-d7/a0-a6
         rts


*
*
* Open:
*  Called on each successive OpenDevice() call.
*
*  Input:
*        D0       Unit Number
*        D1       Flags
*        A1       IORequest
*        A6       DevBase
*
*  Output:
*        None
*
Open:
         movem.l  a2,-(sp)
         addq.w   #1,LIB_OPENCNT(a6)      * Inc the reference count
         cmp.l    #MAXDEVS,d0             * Did they ask for a reasonable unit num?
         blo      11$
2$       moveq.l  #IOERR_OPENFAIL,d0
         move.b   d0,IO_ERROR(a1)
         move.b   #NT_REPLYMSG,LN_TYPE(a1)
         movem.l  (sp)+,a2
         rts

11$
         move.l   ds_UnitList+MLH_HEAD(a6),a2
1$       tst.l    MLN_SUCC(a2)
         beq      2$
         tst.l    d0
         beq      3$
         subq.l   #1,d0
         move.l   MLN_SUCC(a2),a2
         bra      1$
3$

         move.l   a2,IO_UNIT(a1)
         move.l   a6,IO_DEVICE(a1)

         tst.l    IOS2_BUFFERMANAGEMENT(a1)
         beq      80$
         movem.l  d0-d7/a0-a6,-(sp)
         sub.l    #FuncTableSize,sp
         move.l   sp,a3
         moveq.l  #0,d2
40$      movem.l  a1/a6,-(sp)
         move.l   ds_UtlBase(a6),a6
         lea.l    IOS2_BUFFERMANAGEMENT(a1),a0
         SYS      NextTagItem
         move.l   d0,a4
         movem.l  (sp)+,a1/a6
         tst.l    d0
         beq      59$
50$
         move.l   ti_Data(a4),d0
         move.l   ti_Tag(a4),d1
         cmp.l    #S2_COPYTOBUFF,d1
         bne      51$
         move.l   d0,(a3)
         bset     #0,d2
51$      cmp.l    #S2_COPYFROMBUFF,d1
         bne      52$
         move.l   d0,4(a3)
         bset     #1,d2
52$      bra      40$
59$      cmp.b    #%11,d2   * Did we get both CTB and CFB???
         beq      60$
666$     add.l    #FuncTableSize,sp
         movem.l  (sp)+,d0-d7/a0-a6
         bra      2$
60$
         move.l   #-1,8(a3)
         move.l   #BaseTableSize,d0
         move.l   #MEMF_CLEAR|MEMF_PUBLIC,d1
         movem.l  a1/a6,-(sp)
         move.l   ds_SysBase(a6),a6
         SYS      AllocMem
         movem.l  (sp)+,a1/a6
         tst.l    d0
         beq      666$
61$      add.l    #BaseTableSize,d0
         move.l   d0,-(sp)
         sub.l    a2,a2
         movem.l  a1/a6,-(sp)
         move.l   a3,a1
         move.l   d0,a0
         move.l   ds_SysBase(a6),a6
         SYS      MakeFunctions
         movem.l  (sp)+,a6/a1
         move.l   (sp)+,IOS2_BUFFERMANAGEMENT(a1)

         add.l    #FuncTableSize,sp
         movem.l  (sp)+,d0-d7/a0-a6
80$

         moveq.l  #0,d0
         move.b   d0,IO_ERROR(a1)
         move.b   #NT_REPLYMSG,LN_TYPE(a1)

         movem.l  (sp)+,a2
         rts

*
* Close
*  Called every time somebody calls CloseDevice()
*
*  Input:
*        A1       IORequest
*        A6       DevBase
*
*  Output:
*        D0       NULL if the device cannot be unloaded, or
*                 SegList if it has been expunged.
*
Close:
         moveq.l  #-1,d0
         move.l   d0,IO_UNIT(a1)
         move.l   d0,IO_DEVICE(a1)

         movem.l  d0-d1/a0-a1/a6,-(sp)
         move.l   ds_SysBase(a6),a6
         move.l   IOS2_BUFFERMANAGEMENT(a1),a1
         cmp.l    #0,a1
         beq      2$
         move.l   #BaseTableSize,d0
         sub.l    d0,a1
         SYS      FreeMem
2$       movem.l  (sp)+,d0-d1/a0-a1/a6

         moveq.l  #0,d0

         sub.w    #1,LIB_OPENCNT(a6)  * When this drops to zero, it might be expunged
         bne      1$

         btst     #LIBB_DELEXP,ds_Flags(a6)
         beq      1$
         bsr      Expunge
1$
         rts




*
* Expunge
*  Called when the Exec memory system runs low on memory;
*  If nobody has this device open, out it goes.
*
*  Input:
*        A6       DevBase
*
*  Output:
*        D0       NULL if cannot be expunged
*                 SegList if it can.
*
Expunge:

         tst.w    LIB_OPENCNT(a6)         * If somebody has it open, set the delayed expunge flag
         beq      1$
         bset     #LIBB_DELEXP,ds_Flags(a6)
         moveq.l  #0,d0
         rts
1$

         movem.l  a0-a6/d0-d7,-(sp)
         jsr      DeconfigHardware
         movem.l  (sp)+,a0-a6/d0-d7

         move.l   a6,-(sp)                * Make sure any libraries we opened are closed
         move.l   ds_ExpBase(a6),a1
         move.l   ds_SysBase(a6),a6
         SYS      CloseLibrary
         move.l   (sp),a6
         move.l   ds_UtlBase(a6),a1
         move.l   ds_SysBase(a6),a6
         SYS      CloseLibrary
         move.l   (sp)+,a6

         movem.l  a5/a6,-(sp)             * Pull us from the system's Device List
         move.l   a6,a5
         move.l   ds_SysBase(a5),a6
         move.l   ds_SegList(a5),a4
         move.l   a5,a1
         SYS      Remove

        move.l    ds_GPBuffer(a5),a1
        move.l    #512,d0
        SYS       FreeMem

         move.l   a5,a1                   * Free the DeviceBase
         moveq.l  #0,d0
         move.w   LIB_NEGSIZE(a5),d0
         sub.l    d0,a1
         add.w    LIB_POSSIZE(a5),d0
         SYS      FreeMem

         move.l   a4,d0
         movem.l  (sp)+,a5/a6
         rts


*************
** BeginIO **
*************
**
** The only entry point for the A2060's IORequests.
** If QuickIO is requested, and the IORequest can be immediately
** serviced, BeginIO() will return with the IORequest
** completed.  If not, the quick IO bit will be cleared on
** exit, and the caller should expect the iorequest to return on
** the replyport.
**
** Entry:
**          A1 - ptr to the IORequest
**          A6 - ptr to the A2060.device's device base
**
** Exit:
**
BeginIO:
            move.b      #NT_MESSAGE,LN_TYPE(a1) * Make this "in progress"
            moveq.l     #0,d0               * Null the top word of D0
            move.w      IO_COMMAND(a1),d0   * Get the cmd
            lsl.l       #2,d0               * Times 4 for offsets in the table
            lea.l       CommandTable,a0     * Get a ptr to the table
            add.l       d0,a0
            tst.l       (a0)                * Make sure we support this command
            beq         1$                  * No ... we don't
            move.l      (a0),a0
            move.b      #0,IO_ERROR(a1)     * Default Error field to 0
            jmp         (a0)                * Jump to the command's code
1$          move.b      #IOERR_NOCMD,IO_ERROR(a1)
            bra         ReplyRequest



******************
** ReplyRequest **
******************
** Return an IORequest to the caller.
** Takes care not to ReplyMsg IORequests that were QuickIO.
**
** Entry:
**          A1 - ptr to IORequest
**          A6 - ptr to DeviceBase
**
** Exit:
**          none
**
ReplyRequest:

            tst.b           IO_ERROR(a1)
            beq             2$
            movem.l         a0-a2,-(sp)
            move.l          IO_UNIT(a1),a0
            move.l          #S2EVENT_ERROR,d0
            jsr             SignalEvent
            movem.l         (sp)+,a0-a2
2$

            move.b      #NT_REPLYMSG,LN_TYPE(a1)    * JIC someone (quickio) needs it ..
            btst.b      #IOB_QUICK,IO_FLAGS(a1)     * QuickIO?
            bne         1$                          * Yep.
            move.l      a6,-(sp)                    * Save devbase
            move.l      ds_SysBase(a6),a6           * Get ExecBase
            SYS         ReplyMsg                    * Reply it
            move.l      (sp)+,a6                    * get devbase back
1$
            rts

*******************
** AbortAllUnits **
*******************
** Same as below, but does all units
**
** Entry:
**          A6 - Device Base
**
** Exit:
**          none
**
AbortAllUnits:
            movem.l         a2/a6,-(sp)
            move.l          ds_UnitList+MLH_HEAD(a6),a2
            move.l          ds_SysBase(a6),a6
1$          tst.l           MLN_SUCC(a2)
            beq             2$
            bsr             AbortAll
            move.l          MLN_SUCC(a2),a2
            bra             1$
2$
            movem.l         (sp)+,a2/a6
            rts


**************
** AbortAll **
**************
**
** Free up all of the IORequests; abort all of them.
** This function goes through the list of Read IORequests, and the list of
** Write IORequests -- and Aborts each of them.
**
** Entry:
**          A2 - ptr to Unit to abort on.
**          A6 - ExecBase
**
** Exit:
**          None
**
AbortAll:
11$         lea.l           us_ReadIOR(a2),a0               * Get ptr to Read list
            SYS             RemHead                         * Yank the 1st one off the list
            tst.l           d0                              * Did we get one?
            beq             12$                             * no.
            move.l          d0,a1                           * into a1
            move.b          #IOERR_ABORTED,IO_ERROR(a1)     * mark it as errored
            SYS             ReplyMsg                        * return it ...
            bra             11$

12$         lea.l           us_WriteIOR(a2),a0              * Get ptr to write list
            SYS             RemHead                         * Yank the 1st one off the list
            tst.l           d0                              * Did we get one?
            beq             13$                             * no.
            move.l          d0,a1                           * into a1
            move.b          #IOERR_ABORTED,IO_ERROR(a1)     * Mark it as errored.
            SYS             ReplyMsg                        * return it ...
            bra             12$                             * loop for next
13$

14$         lea.l           us_Events(a2),a0                * Get ptr to event list
            SYS             RemHead                         * Yank the 1st one off the list
            tst.l           d0                              * Did we get one?
            beq             15$                             * no.
            move.l          d0,a1                           * into a1
            move.b          #IOERR_ABORTED,IO_ERROR(a1)     * Mark it as errored.
            SYS             ReplyMsg                        * return it ...
            bra             14$                             * loop for next
15$
19$
2$          rts



*************
** AbortIO **
*************
**
** Abort an IORequest.
**
** Entry:
**          A1 - ptr to IORequest
**          A6 - ptr to device base
**
** Exit:
**          none
**
AbortIO:
            move.l          a6,-(sp)                    * Save DeviceBase
            move.l          ds_SysBase(a6),a6           * Get ExecBase
            move.l          a1,-(sp)                    * Save the IORequest
            SYS             Disable                     * No interrupts, thanks
            move.l          (sp),a1                     * recover iorequest
            cmp.b           #NT_MESSAGE,LN_TYPE(a1)     * Is it currently queued?
            bne             1$                          * No.  Don't touch it.
            SYS             Remove                      * Pull it from our lists
            move.l          (sp)+,a1                    * recover the iorequest
            move.b          #IOERR_ABORTED,IO_ERROR(a1) * Let them know what happened
            SYS             ReplyMsg                    * reply it back to the caller
            bra             2$
1$          addq.l          #4,sp                       * bump the stack; don't need iorequest
2$          SYS             Enable                      * allow interrupts
            move.l          (sp)+,a6                    * recover devicebase
            rts

*****************
** SignalEvent **
*****************
** For the given Unit, scan through the list of event iorequests;
** If the given event meets that condition where the iorequest is
** completed, return it.
**
** Entry:
**          D0  - bitmap of events that have occurred.
**          A0  - pointer to Unit structure
**
** Exit:
**          none
**          Smashes A2
**
SignalEvent:
            move.l          us_Events+MLH_HEAD(a0),a1       * Get 1st Event IORequest
1$          tst.l           MLN_SUCC(a1)                    * Are we at the end of the list?
            beq             2$                              * Yes.

            move.l          IOS2_WIREERROR(a1),d1           * Get the bitmask of events
            and.l           d0,d1                           * find the union of our events & those being waited for
            tst.l           d1                              * empty set; don't touch this iorequest
            beq             3$                              *
            move.l          d1,IOS2_WIREERROR(a1)           * Tell IORequest what events occurred.
            movem.l         d0/a1/a6,-(sp)
            REMOVE
            movem.l         (sp)+,d0/a1/a6
            move.l          MLN_PRED(a1),a2
            movem.l         d0/a1/a6,-(sp)
            jsr             ReplyRequest
            movem.l         (sp)+,d0/a1/a6
            move.l          a2,a1
3$          move.l          MLN_SUCC(a1),a1
            bra             1$
2$
            rts


***********
** Flush **
***********
**
** Abort all IORequests.
**
** Entry:
**          A1 - ptr to iorequest
**
** Exit:
**          none
**
Flush:
            movem.l         a2/a1/a6,-(sp)
            move.l          IO_UNIT(a1),a2
            move.l          ds_SysBase(a6),a6
            bsr             AbortAll
            movem.l         (sp)+,a1/a2/a6
            jsr             ReplyRequest
            rts


******* Data ********
ExpName dc.b 'expansion.library',0
UtlName dc.b 'utility.library',0

CommandTable:
        dc.l  0                     * CMD_INVALID
        dc.l  0                     * CMD_RESET
        dc.l  Read                  * CMD_READ
        dc.l  Write                 * CMD_WRITE
        dc.l  0                     * CMD_UPDATE
        dc.l  0                     * CMD_CLEAR
        dc.l  0                     * CMD_STOP
        dc.l  0                     * CMD_START
        dc.l  Flush                 * CMD_FLUSH
        dc.l  DevQuery              * S2_DEVICEQUERY
        dc.l  GetStatAddress        * S2_GETSTATIONADDRESS
        dc.l  ConfigInt             * S2_CONFIGINTERFACE
        dc.l  0                     * S2_ADDSTATIONALIAS
        dc.l  0                     * S2_DELSTATIONALIAS
        dc.l  0                     * S2_ADDMULTICASTADDRESS
        dc.l  0                     * S2_DELMULTICASTADDRESS
        dc.l  0                     * S2_MULTICAST
        dc.l  Broadcast             * S2_BROADCAST
        dc.l  TrackType             * S2_TRACKTYPE
        dc.l  UnTrackType           * S2_UNTRACKTYPE
        dc.l  GetTypeStats          * S2_GETTYPESTATS
        dc.l  GetSpecialStats       * S2_GETSPECIALSTATS
        dc.l  GetGlobalStats        * S2_GETGLOBALSTATS
        dc.l  OnEvent               * S2_ONEVENT
        dc.l  ReadOrphan            * S2_READORPHAN
        dc.l  Online                * S2_ONLINE
        dc.l  Offline               * S2_OFFLINE

        end

