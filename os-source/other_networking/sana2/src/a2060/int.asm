************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: int.asm,v 1.1 92/05/05 18:42:47 gregm Exp Locker: gregm $
*
*
************************************************************************


        include     "exec/types.i"
        include     "a2060.i"

        xdef        SoftIntCodeX,SoftIntCodeR,HardIntCode

        xref        ReplyRequest

        xref        IncBytesSent,IncBytesReceived,IncPacketsSent,IncPacketsReceived

        XSYS        Disable
        XSYS        Enable
        XSYS        Cause

******************
** SoftIntCodeX **
******************
**
** The Software Interrupt routine used to yank transmit data off
** of the queue, and get it into the chip.
**
** The A2060 board has room for FOUR packet buffers -- and these
** are shared between Reading AND Writing.  The following code
** should attempt to see if any buffers are available.  If only one
** buffer is free, don't attempt to use it -- the driver needs to
** keep at least one available at all times for the 'next'
** receive.
**
** If more than one buffer is available, mark it as "used".
**
**
** Entry:
**          A1 - Ptr to the Unit structure
**
** Exit:
**          none; preserve registers like crazy
**
SoftIntCodeX:
        movem.l         a0-a3/a6/d0-d2,-(sp)

        move.l          a1,a2                       * Save Unit ptr
        move.l          us_DeviceBase(a2),a6        * Get device Base

XmitLoop:

        movem.l         a6,-(sp)
        move.l          ds_SysBase(a6),a6           * Nuke ints
        SYS             Disable
        movem.l         (sp)+,a6

* Find last free buffer, and number of free buffers
        lea.l           us_State(a2),a3             * Get a ptr to the State array
        moveq.l         #4-1,d0                     * 4 buffers available
        moveq.l         #0,d1
        moveq.l         #0,d2
1$      cmp.b           #STATE_EMPTY,(a3)+           * Is this buffer free?
        bne             2$                          * No
        add.b           #1,d1                       * Total Free ++
        move.b          d0,d2                       * Save last free buffer
        eor.b           #%11,d2                     * Fix it to represent
*                                                     the buffer number of the last buffer free
2$      dbra            d0,1$

        cmp.b           #2,d1
        blo             3$
        move.b          #STATE_ISBUSY,us_State(a2,d2.l)
3$

        movem.l         a6/d1,-(sp)
        move.l          ds_SysBase(a6),a6           * Allow ints
        SYS             Enable
        movem.l         (sp)+,a6/d1

        cmp.b           #2,d1                       * Too few buffers; exit
        blo             999$

* Now that we have a free hardware buffer, check to see if we have any
* IORequests that need to be transmitted.  If not, free the buffer
* that we just allocated, and exit.

* If there is a waiting IORequest, write it and loop around for
* another try.

        move.l          us_WriteIOR+MLH_HEAD(a2),a3
        tst.l           MLN_SUCC(a3)
        bne             35$
        move.b          #STATE_EMPTY,us_State(a2,d2.l)  * We don't need this buffer after all!
        bra             999$
35$

* Yes, this is a good IORequest; yank it from the list
        move.l          a3,a1
        REMOVE

* Build the output packet
        lsl.l           #2,d2
        move.l          ds_GPBuffer(a6),a0          * Ptr to our GP Buffer

        move.b          IOS2_SRCADDR(a3),0(a0)      * Fill in the Arcnet Src field
        move.b          IOS2_DSTADDR(a3),1(a0)      * Fill in the Arcnet dst field

        move.l          IOS2_DATALENGTH(a3),d0      * Find the length of data to xmit
        addq.l          #1,d0                       * Length += type byte
        cmp.l           #253,d0                     * Is this a 'Short' packet?
        bls             5$                          * Yes
*                                                     Otherwise, it's a 'Long' packet
        cmp.l           #257,d0                     * Is it in the "unsendable length" range?
        bhi             6$                          * No.
*                                                     Yes.  "Round" the length up.
        move.l          #257,d0                     * Use the smallest 'Long' packet length.
6$      move.b          #0,2(a0)                    * Fill in a '0' indicating a long packet
        neg.l           d0                          * Fill in $200-length for Arcnet's offset
        add.l           #$200,d0                    *
        move.b          d0,3(a0)                    *
        bra             7$
5$      neg.l           d0                          * For short packets, fill in $100-length
        add.l           #$100,d0                    * for Arcnet's offset
        move.b          d0,2(a0)                    *
7$
        add.l           d0,a0                       * Find the pointer to where the data is
*                                                     supposed to be copied to.

        move.l          IOS2_PACKETTYPE(a3),d0      * Tack this in there ...
        move.b          d0,(a0)+

        move.l          IOS2_DATA(a3),a1
        move.l          IOS2_DATALENGTH(a3),d0
        move.l          a6,-(sp)
        move.l          IOS2_BUFFERMANAGEMENT(a3),a6
        SYS             CopyFromBuff
        move.l          (sp)+,a6

* Now, copy from our GP Buffer to the Arcnet board's screwy even-byte stuff.
* This can be greatly improved . . .
        movem.l         a0/a1/d0/d1,-(sp)
        move.l          ds_GPBuffer(a6),a0          * (From)
        move.l          us_Buffers(a2,d2.l),a1      * (to) A1 is now a ptr to the buffer itself
        move.l          #512-1,d0
150$    move.b          (a0)+,d1
        move.b          d1,(a1)
        addq.l          #2,a1
        dbra            d0,150$
        movem.l         (sp)+,a0/a1/d0/d1

        move.l          a3,a1
        jsr             IncPacketsSent
        jsr             IncBytesSent

* Return IORequest
        movem.l         a0/a1/d0/d1,-(sp)
        move.l          a3,a1
        jsr             ReplyRequest
        movem.l         (sp)+,a0/a1/d0/d1

* We've now built the entire packet.
*
* Check to see if there're any states marked as 'STATE_XMTCURRENT'.  If not,
* then nobody is transmitting right now, and we can start this buffer up.
* If there already IS someone transmitting right now, we merely mark this
* buffer as 'STATE_XMTDATA', and when the hardware interrupt kicks in
* marking the completion of the one currently running, this one will eventually
* get started.

        add.l           #1,S2DS_PACKETSSENT+us_GlobStats(a2)
        lsr.l           #2,d2                       * Fix the *4 from above by /4.

        movem.l         a6,-(sp)
        move.l          ds_SysBase(a6),a6           * Nuke ints
        SYS             Disable
        movem.l         (sp)+,a6

        lea.l           us_State(a2),a3             * Get a ptr to the State array
        moveq.l         #4-1,d0                     * 4 buffers available
8$      cmp.b           #STATE_XMTCURRENT,(a3)+     * Find out if any are marked "xmt_current"
        beq             9$                          * Yep.  One is.
        dbra            d0,8$                       * lp
*                                                     None are.  Mark ours as such.
        move.b          #STATE_XMTCURRENT,us_State(a2,d2.l)

        move.l          us_CommandRegister(a2),a0   * Tell hw to xmit this page
        lsl.b           #3,d2
        or.b            #%11,d2
        move.b          d2,(a0)

        move.l          us_StatusRegister(a2),a0
        move.b          us_MirrorIntMask(a2),d0
        or.b            #SR_TA,d0                   * Int me when transmitter is done ....
        move.b          d0,us_MirrorIntMask(a2)
        move.b          d0,(a0)

        bra             10$

9$      move.b          #STATE_XMTDATA,us_State(a2,d2.l)    * Mark this as "pls send me"

10$

        movem.l         a6,-(sp)
        move.l          ds_SysBase(a6),a6           * Allow ints
        SYS             Enable
        movem.l         (sp)+,a6

        bra             XmitLoop                    * Go around for any more packets to send

999$
        movem.l         (sp)+,a0-a3/a6/d0-d2
        rts

******************
** SoftIntCodeR **
******************
**
** Software Interrupt routine to bulk move data from the A2060's internal buffers to
** any IORequests that might be waiting for it.
**
** First thing to do is to scan the State's of each of the buffers for this Unit.
** When one is found with RCVDATA, scan the ReadIOR list for IORequests that
** fit the requirements.  If one is found, pull the IORequest from the list,
** Copy the data into the data area, and Reply the IORequest.
**
** Entry:
**          A1 - ptr to Unit structure
**
** Exit:
**          none; plenty registers saved.
**
SoftIntCodeR:
        movem.l         a0-a4/a6/d0-d1/d6/d3,-(sp)

        move.l          a1,a2                       * Keep Unit struct ptr in A2
        move.l          us_DeviceBase(a1),a6        * Get the device base

TopLoop:

        movem.l         a6,-(sp)
        move.l          ds_SysBase(a6),a6           * Disable Hardware Interrupts
        SYS             Disable
        movem.l         (sp)+,a6

0$
        lea.l           us_State(a2),a3                 * Get a ptr to the State array
        moveq.l         #4-1,d6                         * four entries
1$      cmp.b           #STATE_RCVDATA,(a3)+            * Does this one have received data
        bne             2$                              * in it?  No.  Try the next.
* Okay, we have a buffer 'in use'

        move.l          d6,d1
        eor.b           #%11,d1
        and.b           #%11,d1
        lsl.l           #2,d1
        move.l          us_Buffers(a2,d1.l),a0          * Find a pointer to that buffer
        moveq.l         #0,d0
        move.b          4(a0),d0
        tst.b           d0
        bne             3$
        move.b          6(a0),d0
3$      lsl.l           #1,d0                           * times 2 because of the screwy A2060 buffer
        lea.l           0(a0,d0.l),a4                   * Find a ptr to the data itself

* Find an IORequest that meets our requirements
        moveq.l         #0,d3                           * No Orphans this time through ...
39$     move.l          us_ReadIOR+MLH_HEAD(a2),a1      * Get 1st entry in linked list
4$      tst.l           MLN_SUCC(a1)                    * Is this valid?  (!End of List?)
        beq             5$                              * No.  (End of List!)

        move.l          d2,-(sp)                        * Save D2
        move.b          0(a4),d2                        * Get the 'type' byte from 1st byte in data area

        cmp.b           IOS2_PACKETTYPE+3(a1),d2        * Same packet type?
        beq             41$
        move.l          (sp)+,d2
        bra             6$
41$     move.l          (sp)+,d2

        tst.b           d3                              * If d3=0, only CMD_READ's are accepted as a match
        bne             42$                             * If d3 != 0, Orphans are allowed as matches
        cmp.w           #S2_READORPHAN,IO_COMMAND(a1)
        beq             6$
42$

* Yes, this IORequest will accept this packet!

        movem.l         a0/d0/a6/d1,-(sp)
        move.l          ds_SysBase(a6),a6               * Allow ints
        SYS             Enable
        movem.l         (sp)+,a6/d1/a0/d0


* Remove it.
        movem.l         a1/d0/a0,-(sp)
        REMOVE
        movem.l         (sp)+,a0/d0/a1

        move.b          0(a0),IOS2_SRCADDR(a1)
        move.b          2(a0),IOS2_DSTADDR(a1)
        lsr.l           #1,d0                       * /2 because of the *2 above
        neg.l           d0
        add.l           #$100,d0
        tst.b           4(a0)
        bne             7$
        add.l           #$100,d0
7$      sub.l           #1,d0                       * We're using 1 byte for the ARCNet type field
*                                                     D0 Now has the length of data in it.

*        cmp.l           IOS2_DATALENGTH(a1),d0      * Do we have more data than they want?
*        bls             71$
*        move.l          IOS2_DATALENGTH(a1),d0      * Only return how much they have room for ...
71$      move.l          d0,IOS2_DATALENGTH(a1)      * Let the IORequest know how much data's here

* First, copy the data into our GP buffer -- a nice, contiguous block.

        movem.l         a0/a1/d0/d1,-(sp)
        lea.l           2(a4),a0                    * (from)
        move.l          ds_GPBuffer(a6),a1          * (to)
        tst.l           d0
        beq             101$
        subq.l          #1,d0
100$    move.b          (a0)+,d1
        move.b          d1,(a1)+
        addq.l          #1,a0
        dbra            d0,100$
101$    movem.l         (sp)+,a0/a1/d0/d1

        movem.l         a5/a6/a1,-(sp)
        move.l          a6,a5
        move.l          IOS2_BUFFERMANAGEMENT(a1),a6
        move.l          IOS2_DATA(A1),a0
        move.l          ds_GPBuffer(a5),a1
        SYS             CopyToBuff
        movem.l         (sp)+,a6/a1/a5

        add.l           #1,S2DS_PACKETSRECEIVED+us_GlobStats(a2)
        jsr             IncPacketsReceived
        jsr             IncBytesReceived

        jsr             ReplyRequest                * Return the IORequest to the caller

* Packet read.
        move.b          #STATE_EMPTY,-1(a3)         * Mark this Buffer as "free"
        bra             TopLoop                     * Loop around for more

6$
        move.l          MLN_SUCC(a1),a1
        bra             4$

5$      tst.b           d3
        bne             51$
        move.b          #1,d3                       * Try again; this time we'll accept orphans
        bra             39$
51$
        add.l           #1,us_GlobStats+S2DS_UNKNOWNTYPESRECEIVED(a2)
* Packet dropped.
        move.b          #STATE_EMPTY,-1(a3)


2$      dbra            d6,1$

* Now, a little extra checking.  Because of the possibility of all four
* buffers being eaten up with receive data and current ptr is possible,
* we might end up with transmit IORequests being queued up on the
* Unit struct -- with no TA interrupt ready to kick them off.

* So, we'll check two things:
*    If the Xmit is is NOT empty,
*   (and)
*    If the TA bit is not already set in the interrupt mask
*    (it should be, if a xmit is currently running)
*
*
* We'll jumpstart the transmit process by kicking the TA bit in the
* interrupt mask on again.  This should cause a hardware interrupt,
* and (eventually) resolve this unpleasant condition.
*

        move.l          us_WriteIOR+MLH_HEAD(a2),a0             * Get a ptr to the 1st entry
        tst.l           MLN_SUCC(a0)                            * If null, the list is empty
        beq             400$                                    * it is.
        btst.b          #SRB_TA,us_MirrorIntMask(a2)            * Is something currently xmitting?
        bne             400$                                    * yes.
* None are true.  Set the bit in the mask, and copy the mask to the
* hardware.
        bset.b          #SRB_TA,us_MirrorIntMask(a2)
        move.l          us_StatusRegister(a2),a0
        move.b          us_MirrorIntMask(a2),(a0)
400$


        move.l          ds_SysBase(a6),a6           * Allow ints
        SYS             Enable

        movem.l         (sp)+,a0-a4/a6/d0-d1/d3/d6
        rts




*****************
** HardIntCode **
*****************
**
** Hardware interrupt code.  Triggered by, RI, TA, POR, or RECON conditions.
**
** First, check all of our units, in sequence, looking for an interrupt
** condition.  If one is found, deal with it quickly, or issue a software
** interrupt to handle the situation.
**
** Entry:
**          A1 - Device Base
**
** Exit:
**          Save puh-lenty of registers.
**
HardIntCode:
        movem.l     a0-a3/d0-d1,-(sp)

        move.l      a1,a6                           * Keep Dev base in A6

        move.l      ds_UnitList+MLH_HEAD(a6),a2     * Get 1st Unit struct
1$      tst.l       MLN_SUCC(a2)
        beq         99$

        move.l      us_StatusRegister(a2),a0
        move.l      us_CommandRegister(a2),a1
        move.b      (a0),d0
        and.b       us_MirrorIntMask(a2),d0     * If one of the ints we asked for has gone off ...
        beq         90$

        btst        #SRB_RECON,d0                   * Has the network reconfig'd?
        beq         10$

        add.l       #1,us_GlobStats+S2DS_RECONFIGURATIONS(a2)

        move.b      #%10001101,(a1)                 * Send a Size Definition (config) cmd
        move.b      #CMD_CLR_RECON,(a1)
        or.b        #SR_RECON,us_MirrorIntMask(a2)

10$
        btst        #SRB_POR,d0                     * Something caused a reset.
        beq         20$

        move.b      #CMD_CLR_POR,(a1)
        or.b        #SR_POR,us_MirrorIntMask(a2)

        move.l      a1,-(sp)
        move.l      us_Buffers(a2),a1
        move.b      2(a1),us_HardwareAddress(a2)
        move.l      (sp)+,a1


20$
        btst        #SRB_TA,d0                      * Transmitter Available.  A transmission has stopped.
        beq         30$
* Scan the states, looking for a buffer marked as currently xmitting.
* If found, mark it as free.
* When done, scan the states again - looking for the 1st buffer marked XMTDATA; queue it up.

        bclr.b      #SRB_TA,us_MirrorIntMask(a2)

        lea.l       us_State(a2),a3
        moveq.l     #4-1,d1
21$     cmp.b       #STATE_XMTCURRENT,(a3)+
        beq         22$
        dbra        d1,21$
        bra         23$
22$     move.b      #STATE_EMPTY,-1(a3)
23$

        lea.l       us_State(a2),a3
        moveq.l     #4-1,d1
24$     cmp.b       #STATE_XMTDATA,(a3)+
        beq         25$
        dbra        d1,24$
        bra         26$
25$     move.b      #STATE_XMTCURRENT,-1(a3)
        eor.b       #%11,d1
        and.b       #%11,d1
        lsl.b       #3,d1
        or.b        #%11,d1
        move.b      d1,(a1)
        or.b        #SR_TA,us_MirrorIntMask(a2)

        movem.l     a0/a1/d0/a6,-(sp)
        move.l      ds_SysBase(a6),a6
        lea.l       us_SoftIntW(a2),a1
        SYS         Cause
        movem.l     (sp)+,d0/a6/a0/a1

26$


30$
        btst        #SRB_RI,d0
        beq         40$

* Check the States, to find any receives marked 'Current'.  If found,
* change to RCVDATA, and queue another receive buffer up.

        lea.l       us_State(a2),a3
        moveq.l     #4-1,d1
31$     cmp.b       #STATE_RCVCURRENT,(a3)+
        beq         32$
        dbra        d1,31$
        bra         40$
32$     move.b      #STATE_RCVDATA,-1(a3)

        lea.l       us_State(a2),a3
        moveq.l     #4-1,d1
33$     cmp.b       #STATE_EMPTY,(a3)+
        beq         34$
        dbra        d1,33$

        add.l       #1,S2DS_OVERRUNS+us_GlobStats(a2)

        bclr        #SRB_RI,us_MirrorIntMask(a2)
        bra         40$

34$     move.b      #STATE_RCVCURRENT,-1(a3)
        eor.b       #%11,d1
        and.b       #%11,d1
        lsl.b       #3,d1
        or.b        #%10000100,d1
        move.b      d1,(a1)

        movem.l     a0/a1/d0/a6,-(sp)
        move.l      ds_SysBase(a6),a6
        lea.l       us_SoftIntR(a2),a1
        SYS         Cause
        movem.l     (sp)+,d0/a6/a0/a1

        or.b        #SR_RI,us_MirrorIntMask(a2)
40$
        move.b      us_MirrorIntMask(a2),(a0)       * Update status register

90$
        move.l      MLN_SUCC(a2),a2                 * Get next Unit struct, and look at it.
        bra         1$

99$
        movem.l     (sp)+,a0-a3/d0-d1
        rts





        end
