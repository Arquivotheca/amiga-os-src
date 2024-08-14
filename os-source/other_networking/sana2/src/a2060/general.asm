************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: general.asm,v 1.2 92/05/06 11:13:11 gregm Exp $
*
*
************************************************************************


        include         "a2060.i"

        XSYS            Disable
        XSYS            Enable
        XSYS            ReplyMsg
        XSYS            RemHead
        XSYS            CopyMem
        XSYS            AllocMem
        XSYS            FreeMem

        xref            ReplyRequest,SignalEvent,GetTime

        xdef            DevQuery,ConfigInt,GetStatAddress,OnEvent
        xdef            Offline,Online,GetGlobalStats,GetSpecialStats

        xdef            IncPacketsReceived,IncPacketsSent
        xdef            IncBytesReceived,IncBytesSent

        xdef            TrackType,UnTrackType,GetTypeStats

**************
** DevQuery **
**************
**
** Responds to an attempt to issue an S2_DEVICEQUERY command.
** Returns data that describes the network interface.
**
** Entry:
**          A1 - IORequest
**
** Exit:
**          none
**
DevQuery:
            move.l      a1,-(sp)
            move.l      IOS2_STATDATA(a1),a1
            move.l      S2DQ_SIZEAVAILABLE(a1),d0           * If they're requesting more than the amount
            cmp.l       #30,d0                              * Of data we have, provide only what we have.
            blo         1$
            move.l      #30,d0
1$          move.l      d0,S2DQ_SIZESUPPLIED(a1)

            move.l      #22-1,d0                            * Copy the data
            lea.l       OurDevInfo,a0
            lea.l       S2DQ_FORMAT(a1),a1
2$          move.b      (a0)+,(a1)+
            dbra        d0,2$
            move.l      (sp)+,a1

            jsr         ReplyRequest
            rts

OurDevInfo:
            dc.l        0       * DevQuery formtat
            dc.l        0       * Level 0
            dc.w        8       * Address field size
            dc.l        507     * Maximum Transmission Unit
            dc.l        2500000 * 2.5Mbps
            dc.l        S2WIRETYPE_ARCNET


***************
** ConfigInt **
***************
**
** Configure Interface; respond to an S2_CONFIGINTERFACE command.
**
** Allows a protocol stack to specify the hardware address of a
** SANA II device.  In the A2060, this is a waste of time, because
** the board will insist on using the address specified by the
** jumpers on the board, regardless of what a protocol stack would
** like.
**
** Entry:
**          A1  - ptr to IORequest
**
** Exit:
**          none
**
ConfigInt:
            move.l      a6,-(sp)
            move.l      IO_DEVICE(a1),a6
            move.l      IO_UNIT(a1),a0
            move.b      us_HardwareAddress(a0),IOS2_SRCADDR(a1)
            jsr         ReplyRequest
            move.l      (sp)+,a6
            rts


********************
** GetStatAddress **
********************
**
** Get station address; return the hardware address of this A2060 board.
**
** Entry:
**          A1 - ptr to IORequest
**
** Exit:
**          none
**
GetStatAddress:
            move.l      a6,-(sp)
            move.l      IO_UNIT(a1),a0
            move.b      us_HardwareAddress(a0),IOS2_SRCADDR(a1)
            move.b      us_HardwareAddress(a0),IOS2_DSTADDR(a1)
            move.l      IO_DEVICE(a1),a6
            jsr         ReplyRequest
            move.l      (sp)+,a6
            rts

*************
** OnEvent **
*************
**
** Sets a grouping of events that, when any of which become true, the IORequest
** will return.
**
** Entry:
**          a1 - ptr to IORequest
**
** Exit:
**          none
**
OnEvent:
            bclr.b      #IOB_QUICK,IO_FLAGS(a1)
            move.l      a6,-(sp)
            move.l      IO_DEVICE(a1),a6
            move.l      IO_UNIT(a1),a0                  * Find a ptr to the Unit struct
            move.l      IOS2_WIREERROR(a1),d0
            and.l       #S2EVENT_ONLINE|S2EVENT_OFFLINE,d0
            tst.l       d0
            beq         1$
* Asking for either "online" or "offline".
            move.l      d0,d1
            and.l       #S2EVENT_OFFLINE,d1
            beq         2$
* Asking for "offline".
            btst.b      #USB_ONLINE,us_Flags(a0)
            bne         2$
            move.l      d1,IOS2_WIREERROR(a1)           * Already offline.  return request
            jsr         ReplyRequest
            move.l      (sp)+,a6
            rts
2$          move.l      d0,d1
            and.l       #S2EVENT_ONLINE,d1
            beq         3$
            btst.b      #USB_ONLINE,us_Flags(a0)
            beq         3$
            move.l      d1,IOS2_WIREERROR(a1)           * Already online.  return request
            jsr         ReplyRequest
            move.l      (sp)+,a6
            rts
3$
1$
            movem.l     a1/a6,-(sp)
            move.l      ds_SysBase(a6),a6
            SYS         Disable                         * Disable interrupts
            movem.l     (sp)+,a1/a6

            move.l      IO_UNIT(a1),a0
            lea.l       us_Events(a0),a0                * Add this event to the end of the list
            ADDTAIL

            move.l      a6,-(sp)
            move.l      ds_SysBase(a6),a6               * Allow interrupts
            SYS         Enable
            move.l      (sp)+,a6

            move.l      (sp)+,a6
            rts

************
** Online **
************
**
** Takes the given unit Online.
**
** Entry:
**          A1 - ptr to Iorequest
**          A6 - devicebase
**
** Exit:
**          None
**
Online:
            move.l      IO_UNIT(a1),a0                  * Get Unit struct ptr in a0
            movem.l     a0/a1/a2,-(sp)                  *
            move.l      #S2EVENT_ONLINE,d0              * Let every event know that this unit
            jsr         SignalEvent                     * has gone online
            movem.l     (sp)+,a0/a1/a2                  *

            bset.b      #USB_ONLINE,us_Flags(a0)        * Bring the unit online
            movem.l     a1/a6,-(sp)
            move.l      ds_SysBase(a6),a6
            lea.l       us_GlobStats+S2DS_LASTSTART(a0),a0
            jsr         GetTime                         * Timestamp this damned thing.
            movem.l     (sp)+,a1/a6
            jsr         ReplyRequest
            rts

*************
** Offline **
*************
**
** Takes the given unit Offline.
**
** Entry:
**          A1 - ptr to IORequest
**          A6 - devicebase
**
** Exit:
**          None
**

Offline:
            move.l      IO_UNIT(a1),a0                      * get ptr to Unit struct in A0
            movem.l     a2/a0/a1,-(sp)                      * save a2, SignalEvent smashes it;
            move.l      #S2EVENT_OFFLINE,d0                 * Signal all events that this happened
            jsr         SignalEvent
            movem.l     (sp)+,a0/a1/a2                      * recover regs

            bclr.b      #USB_ONLINE,us_Flags(a0)            * Force Unit offline

            movem.l     a2/a0/a1/a6,-(sp)
* Abort all iorequests and mark as "unit offline".

            move.l          ds_UnitList+MLH_HEAD(a6),a2     * Get 1st Unit structure
            move.l          ds_SysBase(a6),a6               * Get ExecBase

1$          tst.l           MLN_SUCC(a2)                    * Is this a valid Unit struct?
            beq             2$                              * No.
11$         lea.l           us_ReadIOR(a2),a0               * Get ptr to Read list
            SYS             RemHead                         * Yank the 1st one off the list
            tst.l           d0                              * Did we get one?
            beq             12$                             * no.
            move.l          d0,a1                           * into a1
            move.b          #IOERR_ABORTED,IO_ERROR(a1)     * mark it as errored
            move.l          #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(a1)
            SYS             ReplyMsg                        * return it ...
            bra             11$

12$         lea.l           us_WriteIOR(a2),a0              * Get ptr to write list
            SYS             RemHead                         * Yank the 1st one off the list
            tst.l           d0                              * Did we get one?
            beq             13$                             * no.
            move.l          d0,a1                           * into a1
            move.b          #IOERR_ABORTED,IO_ERROR(a1)     * Mark it as errored.
            move.l          #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(a1)
            SYS             ReplyMsg                        * return it ...
            bra             12$                             * loop for next
13$

19$         move.l          MLN_SUCC(a2),a2                 * Get next Unit
            bra             1$
2$

**
            movem.l     (sp)+,a0/a1/a6/a2
            jsr         ReplyRequest                        * return iorequest
            rts

********************
** GetGlobalStats **
********************
** Retrieve the current GlobStats.
**
** Entry:
**          A1 - ptr to IORequest
**
** Exit:
**          None
**
GetGlobalStats:
            movem.l         a6/a1,-(sp)                     * save devbase/iorequest
            move.l          IO_UNIT(a1),a0                  * Get a ptr to the Unit struct
            lea.l           us_GlobStats(a0),a0             * Make a ptr to our global stats
            move.l          IOS2_STATDATA(a1),a1            * Get a ptr to the destination
            move.l          #S2DS_SIZE,d0                   * The length of the global stats structure
            move.l          ds_SysBase(a6),a6               * ExecBase
            SYS             CopyMem                         * Copy the stuff for them
            movem.l         (sp)+,a1/a6                     * recover devbase/iorequest
            jsr             ReplyRequest                    * return the iorequest
            rts

*********************
** GetSpecialStats **
*********************
**
** Handles requests for Special statistics.
**
** Luckily, ARCNET has none.  :')
**
** Entry:
**          a1  - ptr to IORequest
**
** Exit:
**          none
**
GetSpecialStats:
            move.l          IOS2_STATDATA(a1),a0                    * Get ptr to structure
            move.l          #0,S2SSH_RECORDCOUNTSUPPLIED(a0)        * Tell them that we have NO special stats.  :')
            jmp             ReplyRequest                            * Return the iorequest




**********************
** IncBytesReceived **
**********************
**
** Increments the total number of bytes received for this packet type.
**
**  Entry:
**          A1  - ptr to the IORequest being dealt with
**                (I'll fetch PacketType, DataLength, and the DeviceBase off of this)
**
** Exit:
**          None
**
IncBytesReceived:
            movem.l         d0/d1/a0/a6,-(sp)

            move.l          IO_DEVICE(a1),a6                    * Ptr to Device
            move.l          IOS2_PACKETTYPE(a1),d0              * PacketType to update stats for
            move.l          ds_TypeStats+MLH_HEAD(a6),a0        * Get 1st type stats node
1$          tst.l           MLN_SUCC(a0)                        * End of list?
            beq             2$                                  * yes.
            move.l          IO_UNIT(a1),d1
            cmp.l           tn_Unit(a0),d1                      * Make sure Unit is same
            bne             3$
            cmp.l           tn_PacketType(a0),d0                * Same type?
            bne             3$
            move.l          tn_Stats+S2PTS_RXBYTES(a0),d1
            add.l           IOS2_DATALENGTH(a1),d1
            move.l          d1,tn_Stats+S2PTS_RXBYTES(a0)
3$          move.l          MLN_SUCC(a0),a0                     * get Next Node
            bra             1$                                  * Loop for next
2$          movem.l         (sp)+,a0/d0/d1/a6
            rts

******************
** IncBytesSent **
******************
**
** Increments the total number of bytes sent for this packet type.
**
**  Entry:
**          A1  - ptr to the IORequest being dealt with
**                (I'll fetch PacketType, DataLength, and the DeviceBase off of this)
**
** Exit:
**          None
**
IncBytesSent:
            movem.l         d0/d1/a0/a6,-(sp)

            move.l          IO_DEVICE(a1),a6                    * Ptr to Device
            move.l          IOS2_PACKETTYPE(a1),d0              * PacketType to update stats for
            move.l          ds_TypeStats+MLH_HEAD(a6),a0        * Get 1st type stats node
1$          tst.l           MLN_SUCC(a0)                        * End of list?
            beq             2$                                  * yes.
            move.l          IO_UNIT(a1),d1
            cmp.l           tn_Unit(a0),d1                      * Make sure Unit is same
            bne             3$
            cmp.l           tn_PacketType(a0),d0                * Same type?
            bne             3$
            move.l          tn_Stats+S2PTS_TXBYTES(a0),d1
            add.l           IOS2_DATALENGTH(a1),d1
            move.l          d1,tn_Stats+S2PTS_TXBYTES(a0)
3$          move.l          MLN_SUCC(a0),a0                     * get Next Node
            bra             1$                                  * Loop for next
2$          movem.l         (sp)+,a0/d0/d1/a6
            rts

************************
** IncPacketsReceived **
************************
**
** Increments the total number of packets received for this packet type.
**
**  Entry:
**          A1  - ptr to the IORequest being dealt with
**                (I'll fetch PacketType, DataLength, and the DeviceBase off of this)
**
** Exit:
**          None
**
IncPacketsReceived:
            movem.l         d0/d1/a0/a6,-(sp)

            move.l          IO_DEVICE(a1),a6                    * Ptr to Device
            move.l          IOS2_PACKETTYPE(a1),d0              * PacketType to update stats for
            move.l          ds_TypeStats+MLH_HEAD(a6),a0        * Get 1st type stats node
1$          tst.l           MLN_SUCC(a0)                        * End of list?
            beq             2$                                  * yes.
            move.l          IO_UNIT(a1),d1
            cmp.l           tn_Unit(a0),d1                      * Make sure Unit is same
            bne             3$
            cmp.l           tn_PacketType(a0),d0                * Same type?
            bne             3$

            add.l           #1,tn_Stats+S2PTS_RXPACKETS(a0)

3$          move.l          MLN_SUCC(a0),a0                     * get Next Node
            bra             1$                                  * Loop for next
2$          movem.l         (sp)+,a0/d0/d1/a6
            rts

********************
** IncPacketsSent **
********************
**
** Increments the total number of packets sent for this packet type.
**
**  Entry:
**          A1  - ptr to the IORequest being dealt with
**                (I'll fetch PacketType, DataLength, and the DeviceBase off of this)
**
** Exit:
**          None
**
IncPacketsSent:
            movem.l         d0/d1/a0/a6,-(sp)

            move.l          IO_DEVICE(a1),a6                    * Ptr to Device
            move.l          IOS2_PACKETTYPE(a1),d0              * PacketType to update stats for
            move.l          ds_TypeStats+MLH_HEAD(a6),a0        * Get 1st type stats node
1$          tst.l           MLN_SUCC(a0)                        * End of list?
            beq             2$                                  * yes.
            move.l          IO_UNIT(a1),d1
            cmp.l           tn_Unit(a0),d1                      * Make sure Unit is same
            bne             3$
            cmp.l           tn_PacketType(a0),d0                * Same type?
            bne             3$

            add.l           #1,tn_Stats+S2PTS_TXPACKETS(a0)

3$          move.l          MLN_SUCC(a0),a0                     * get Next Node
            bra             1$                                  * Loop for next
2$          movem.l         (sp)+,a0/d0/d1/a6
            rts


***************
** TrackType **
***************
**
** Tells the device to begin tracking the given Packet Type's statistics.
**
** Entry:
**          A1 - ptr to iorequest
**
** Exit:
**          none
**
TrackType:
            move.l          IOS2_PACKETTYPE(a1),d0                  * Keep packet type to add in d0

            movem.l         a1/a6,-(sp)                             * Allocate the memory for the node
            move.l          ds_SysBase(a6),a6                       * ExecBase->a6
            move.l          #TypeNodeSize,d0                        * Size of the node
            move.l          #MEMF_CLEAR|MEMF_PUBLIC,d1              * type of memory
            SYS             AllocMem                                * Alloc it
            tst.l           d0                                      * Did it work?
            bne             1$                                      * Yep!
            movem.l         (sp)+,a1/a6                             * Uhh .. No.
            move.b          #S2ERR_NO_RESOURCES,IO_ERROR(a1)        * Tell them.
            move.l          #S2WERR_NOT_TRACKED,IOS2_WIREERROR(a1)  * Tell them why.
            jsr             ReplyRequest                            * Return their IORequest
            rts
1$          SYS             Disable
            movem.l         (sp),a1/a6                              * recover devbase, iorequest
            lea.l           ds_TypeStats(a6),a0                     * Find the list to keep this stuff
            move.l          d0,-(sp)                                * save ptr to tracking node
            move.l          d0,a1                                   * Add it to the list
            ADDTAIL
            move.l          ds_SysBase(a6),a6                       *
            SYS             Enable
            movem.l         (sp)+,a0/a1/a6
            move.l          IOS2_PACKETTYPE(a1),tn_PacketType(a0)
            move.l          IO_UNIT(a1),tn_Unit(a0)

            jsr             ReplyRequest
            rts

*****************
** UnTrackType **
*****************
**
** Stop tracking a certain packet type.
**
** Entry:
**          A1 - ptr to an IORequest
**
** Exit:
**          none
**
UnTrackType:

* First, try to find the darned thing.

            movem.l         a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6
            SYS             Disable                             * Disable Interrupts
            movem.l         (sp)+,a1/a6

            move.l          ds_TypeStats+MLH_HEAD(a6),a0        * Get 1st entry in list
            move.l          IOS2_PACKETTYPE(a1),d0              * Keep the target packet type in D0
1$          tst.l           MLN_SUCC(a0)                        * End of the list?
            beq             9$                                  * Yes.
            cmp.l           tn_PacketType(a0),d0                * Same packet type?
            beq             8$                                  * yes.
            move.l          MLN_SUCC(a0),a0                     * Grab the next node; loop
            bra             1$

9$          movem.l         a0/a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6                   * Allow Interrupts
            SYS             Enable
            movem.l         (sp)+,a0/a1/a6

            move.b          #S2ERR_BAD_STATE,IO_ERROR(a0)       * Notify caller why we failed.
            move.l          #S2WERR_NOT_TRACKED,IOS2_WIREERROR(a0)
            jsr             ReplyRequest                        * return iorequest
            rts

8$          movem.l         a0/a1,-(sp)                         * Now that we've found the node,
            move.l          a0,a1                               * pull it from the list
            REMOVE
            movem.l         (sp)+,a1/a0

            movem.l         a0/a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6                   * Allow interrupts
            SYS             Enable
            movem.l         (sp)+,a0/a1/a6

            movem.l         a0/a1/a6,-(sp)                      * Free the memory for the node
            move.l          ds_SysBase(a6),a6
            move.l          a0,a1
            move.l          #TypeNodeSize,d0
            SYS             FreeMem
            movem.l         (sp)+,a0/a1/a6

            jsr             ReplyRequest                        * return the iorequest
            rts

******************
** GetTypeStats **
******************
** Retrieves the statistics for a given type.
**
** Entry:
**          A1 - ptr to IORequest
**
** Exit:
**          none
**
GetTypeStats:

            move.l          IOS2_PACKETTYPE(a1),d0              * Find the packet type they want
            movem.l         d0/a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6                   * Disable Interrupts
            SYS             Disable
            movem.l         (sp)+,d0/a1/a6

            move.l          ds_TypeStats+MLH_HEAD(a6),a0        * Get the 1st entry in the list
1$          tst.l           MLN_SUCC(a0)                        * Is this the end of the list?
            beq             2$                                  * Yes.
            cmp.l           tn_PacketType(a0),d0                * Is this tracking node of the same type?
            beq             3$                                  * yes - return this data
            move.l          MLN_SUCC(a0),a0                     * no.  try next node
            bra             1$
2$
            movem.l         a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6
            SYS             Enable                              * Enable interrupts
            movem.l         (sp)+,a1/a6
                                                                    * Explain that this type isn't tracked.
            move.b          #S2ERR_BAD_STATE,IO_ERROR(a1)
            move.l          #S2WERR_NOT_TRACKED,IOS2_WIREERROR(a1)
            jsr             ReplyRequest                        * Return the iorequest
            rts
3$
            movem.l         a1/a6,-(sp)                         * Save devbase/iorequest
            move.l          ds_SysBase(a6),a6                   * ExecBase
            lea.l           tn_Stats(a0),a0                     * Get a ptr to these stats
            move.l          IOS2_STATDATA(a1),a1                * Get a ptr to the destination
            move.l          #S2PTS_SIZE,d0                      * Length of stat area
            SYS             CopyMem                             * Copy the data
            movem.l         (sp),a1/a6                          * Recover devbase/iorequest

            move.l          ds_SysBase(a6),a6                   * ExecBase
            SYS             Enable                              * Allow interrupts
            movem.l         (sp)+,a1/a6                         * Recover devbase/iorequest

            jsr             ReplyRequest                        * Return iorequest
            rts


            end
