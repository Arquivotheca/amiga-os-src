************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: read.asm,v 1.1 92/05/05 18:43:18 gregm Exp Locker: gregm $
*
*
************************************************************************


        include     "a2060.i"

        XSYS        Cause
        XSYS        Disable
        XSYS        Enable

        xdef        Read,ReadOrphan


**********
** Read **
**********
**
** CMD_READ entry point.
**
** Entry:
**          A1 - ptr to IORequest
**
** Exit:
**          none
**
ReadOrphan:
Read:

            movem.l         a2/a6,-(sp)
            bclr.b          #IOB_QUICK,IO_FLAGS(a1)         * No quickio allowed.  Sorry!
            move.l          IO_DEVICE(a1),a6                * Get the Device base into A6
            move.l          IO_UNIT(a1),a2                  * Get a ptr to the Unit struct into A2

            movem.l         a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6               * Lock out interrupts
            SYS             Disable
            movem.l         (sp)+,a1/a6

            lea.l           us_ReadIOR(a2),a0               * Get it onto the Read List
            ADDTAIL

            move.l          a6,-(sp)
            move.l          ds_SysBase(a6),a6               * Allow interrupts
            SYS             Enable
            move.l          (sp)+,a6

            movem.l         (sp)+,a2/a6
            rts


            end
