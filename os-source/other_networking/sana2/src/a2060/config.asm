************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: config.asm,v 1.1 92/05/05 18:41:46 gregm Exp Locker: gregm $
*
*
************************************************************************


        include     "exec/types.i"
        include     "exec/lists.i"
        include     "exec/libraries.i"
        include     "exec/memory.i"
        include     "libraries/expansion.i"
        include     "libraries/configvars.i"
        include     "libraries/configregs.i"
        include     "exec/interrupts.i"
        include     "hardware/intbits.i"
        include     "a2060.i"

        xdef    ConfigureHardware,DeconfigHardware

        XSYS    FindConfigDev
        XSYS    AllocMem
        XSYS    FreeMem
        XSYS    AddTail
        XSYS    RemHead
        XSYS    Disable
        XSYS    Enable
        XSYS    AddIntServer
        XSYS    RemIntServer

        xref    _kprintf

        xref    SoftIntCodeX,SoftIntCodeR,HardIntCode,GetTime



*********************
* ConfigureHardware *
*********************
*
* Attempts to scan the autoconfig lists for any A2060, A560, or Ameristar Arcnet boards
*
* Entry:
*       A6 - ptr to device base
*
* Exit:
*       D0 - number of units configured
*
ConfigureHardware:


* Try looking for C= A2060 boards
            moveq.l     #0,d0
            move.l      d0,a4
            moveq.l     #0,d5
100$
            move.l      a4,a0
            move.l      #MAN_COMMODORE,d0
            move.l      #PROD_A2060,d1
            move.l      a6,-(sp)
            move.l      ds_ExpBase(a6),a6
            SYS         FindConfigDev
            move.l      (sp)+,a6
            tst.l       d0
            beq         150$
            move.l      d0,a4
            bsr         BuildUnit
            addq.l      #1,d5
            bra         100$


* Try looking for Ameristar Arcnet boards
150$
            moveq.l     #0,d0
            move.l      d0,a4
151$
            move.l      a4,a0
            move.l      #MAN_AMER,d0
            move.l      #PROD_A2060,d1
            move.l      a6,-(sp)
            move.l      ds_ExpBase(a6),a6
            SYS         FindConfigDev
            move.l      (sp)+,a6
            tst.l       d0
            beq         200$
            move.l      d0,a4
            bsr         BuildUnit
            addq.l      #1,d5
            bra         151$

200$        move.l      d5,d0
            rts


*************
* BuildUnit *
*************
*
* Given a ConfigDev structure, this will allocate a Unit structure, add it to the device's
* list, and initialize the hardware on that board.
*
* Entry:
*       A4 - ptr to a ConfigDev structure
*
* Exit:
*
*
BuildUnit:

            btst.b      #CDB_CONFIGME,cd_Flags(a4)
            beq         99$
            bclr.b      #CDB_CONFIGME,cd_Flags(a4)

            move.l      #us_SIZE,d0
            move.l      #MEMF_PUBLIC|MEMF_CLEAR,d1      * I'm counting on the mem being 0'd!  Don't remove MEMF_CLEAR!
            move.l      a6,-(sp)
            move.l      ds_SysBase(a6),a6
            SYS         AllocMem
            move.l      (sp)+,a6
            tst.l       d0
            bne         1$
            bset.b      #CDB_CONFIGME,cd_Flags(a4)
            bra         99$
1$          move.l      d0,a2
            move.b      #USF_ONLINE,us_Flags(a2)

            movem.l     a0/d0/d1/a1/a6,-(sp)
            move.l      ds_SysBase(a6),a6
            lea.l       us_GlobStats+S2DS_LASTSTART(a2),a0
            jsr         GetTime                         * Timestamp this damned thing.
            movem.l     (sp)+,a1/a6/a0/d0/d1

            move.l      cd_BoardAddr(a4),a1
            move.l      a6,us_DeviceBase(a2)
            move.l      a1,us_HardwarePtr(a2)
            move.l      a4,us_ConfigDev(a2)
            add.l       #$4000,a1
            move.l      a1,us_StatusRegister(a2)
            add.l       #2,a1
            move.l      a1,us_CommandRegister(a2)
            move.b      #%10001101,(a1)                * Tell chip that it has a 2K SRAM.
            lea.l       us_ReadIOR(a2),a0
            NEWLIST     a0
            lea.l       us_WriteIOR(a2),a0
            NEWLIST     a0
            lea.l       us_Events(a2),a0
            NEWLIST     a0

* Set up this Unit's SoftInt structure
            lea.l       us_SoftIntW(a2),a3
            move.b      #0,LN_PRI(a3)
            move.b      #NT_INTERRUPT,LN_TYPE(a3)
            lea.l       SoftIntName,a5
            move.l      a5,LN_NAME(a3)
            lea.l       SoftIntCodeX,a5
            move.l      a5,IS_CODE(a3)
            move.l      a2,IS_DATA(a3)

            lea.l       us_SoftIntR(a2),a3
            move.b      #0,LN_PRI(a3)
            move.b      #NT_INTERRUPT,LN_TYPE(a3)
            lea.l       SoftIntName,a5
            move.l      a5,LN_NAME(a3)
            lea.l       SoftIntCodeR,a5
            move.l      a5,IS_CODE(a3)
            move.l      a2,IS_DATA(a3)


* Ask for TA, RECON, and RESET (Power On Reset; POR) interrupts.
            move.b      #SR_TA|SR_RECON|SR_POR,us_MirrorIntMask(a2)

* Clear out four 1K buffers (2*512), and save the pointers to the buffers in the Unit structure
            moveq.l     #4-1,d0
            move.l      us_HardwarePtr(a2),a3
            add.l       #$8000,a3
            lea.l       us_Buffers(a2),a1
2$          move.l      a3,(a1)+
            move.l      a3,a0
            move.l      #1024-1,d1
3$          move.b      #0,(a0)+
            dbra        d1,3$
            add.l       #1024,a3
            dbra        d0,2$

* Unit structure is completely configured.  Add it to the device's list.
            lea.l       ds_UnitList(a6),a0
            move.l      a2,a1
            move.l      a6,-(sp)
            move.l      ds_SysBase(a6),a6
            SYS         AddTail
            move.l      (sp)+,a6

            move.l      a6,-(sp)
            lea.l       ds_HardInt(a6),a1
            move.b      #0,LN_PRI(a1)
            move.b      #NT_INTERRUPT,LN_TYPE(a1)
            lea.l       HardIntName,a5
            move.l      a5,LN_NAME(a1)
            lea.l       HardIntCode,a5
            move.l      a5,IS_CODE(a1)
            move.l      (sp),IS_DATA(a1)
            move.l      #INTB_PORTS,d0
            move.l      ds_SysBase(a6),a6
            SYS         AddIntServer
            move.l      (sp)+,a6

            move.l      us_HardwarePtr(a2),a0
            move.b      #$FF,(a0)               * Kick the darned chip into action.
            move.l      #$C000,d0
            move.b      #$FF,0(a0,d0.l)          * The A560 has this at a different address! ! !  Argh!

* Find an empty buffer for read, and kick off the receive process.
            move.l      a6,-(sp)
            move.l      ds_SysBase(a6),a6
            SYS         Disable
            move.l      (sp)+,a6

            lea.l       us_State(a2),a0                         * Find an empty buffer, and
            moveq.l     #4-1,d1                                 * Mark it as the 'current' rcv
10$         cmp.b       #STATE_EMPTY,(a0)+                      * buffer.
            beq         11$
            dbra        d1,10$
11$         eor.b       #%11,d1
            and.b       #%11,d1
            move.b      #STATE_RCVCURRENT,us_State(a2,d1.l)

            lsl.b       #3,d1                                   * Tell chip to receive to that page
            or.b        #%10000100,d1                            * Give correct command
            move.l      us_CommandRegister(a2),a0               *
            move.b      d1,(a0)
            move.b      us_MirrorIntMask(a2),d0
            or.b        #SR_RI,d0
            move.b      d0,us_MirrorIntMask(a2)
            move.l      us_StatusRegister(a2),a0                * Tell chip to notify us on finished
            move.b      d0,(a0)                                 * receives

            move.l      a6,-(sp)
            move.l      ds_SysBase(a6),a6
            SYS         Enable
            move.l      (sp)+,a6


99$
            rts


**********************
** DeconfigHardware **
**********************
**
** Frees up all of our Unit structures, and marks the ConfigDevs for these boards
** as "please config me".
**
** Entry:
**          A6 - ptr to Device Base
**
** Exit:
**          none
**
DeconfigHardware:

            move.l      a6,-(sp)
            lea.l       ds_UnitList(a6),a5
            move.l      ds_SysBase(a6),a6
1$          move.l      a5,a0
            SYS         RemHead
            tst.l       d0
            beq         9$
            move.l      d0,a1
            move.l      us_ConfigDev(a1),a2
            bset.b      #CDB_CONFIGME,cd_Flags(a2)
            move.l      #us_SIZE,d0
            SYS         FreeMem
            bra         1$
9$

* Pull the Hardware Interrupt server off of the system list
            move.l      (sp),a1
            lea.l       ds_HardInt(a1),a1
            move.l      #INTB_PORTS,d0
            SYS         RemIntServer

            move.l      (sp)+,a6
            rts



****** DATA ******
SoftIntName dc.b 'A2060 Software Interrupt',0
HardIntName dc.b 'A2060 Hardware Interrupt',0


            end

