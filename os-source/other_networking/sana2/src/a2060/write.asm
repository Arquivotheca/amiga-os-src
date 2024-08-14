************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: write.asm,v 1.1 92/05/05 18:44:26 gregm Exp Locker: gregm $
*
*
************************************************************************


        include     "a2060.i"

        XSYS        Cause
        XSYS        Disable
        XSYS        Enable

        xdef        Write,Broadcast
        xref        ReplyRequest


***************
** Broadcast **
***************
**
** Same as Write, but 'send' the packet to everyone on the physical network.
**
** Entry:
**          A1 - ptr to IORequest
**
** Exit:
**          none
**
Broadcast:
            move.b      #0,IOS2_DSTADDR(a1)
            bra         Write

***********
** Write **
***********
**
** CMD_WRITE entry point.
**
** Entry:
**          A1 - ptr to IORequest
**
** Exit:
**          None
**
Write:

            cmp.l           #507,IOS2_DATALENGTH(a1)        * If they try sending a too-large packet,
            bls             1$                              * Refuse -- and tell them why.
            move.b          #S2ERR_MTU_EXCEEDED,IO_ERROR(a1)
            clr.l           IOS2_WIREERROR(a1)
            jsr             ReplyRequest
            rts
1$

            movem.l         a2/a6,-(sp)
            bclr.b          #IOB_QUICK,IO_FLAGS(a1)         * No Quick IO!  Nix!  Nix!
            move.l          IO_DEVICE(a1),a6                * Get Device Base into A6
            move.l          IO_UNIT(a1),a2                  * Get Unit struct ptr into A2

            movem.l         a1/a6,-(sp)
            move.l          ds_SysBase(a6),a6               * Lock out interrupts
            SYS             Disable
            movem.l         (sp)+,a6/a1

            lea.l           us_WriteIOR(a2),a0
            ADDTAIL

            move.l          a6,-(sp)
            move.l          ds_SysBase(a6),a6               * Permit interrupts
            SYS             Enable
            move.l          (sp)+,a6

            move.l          a6,-(sp)                        * Force a call to the Software Int
            lea.l           us_SoftIntW(a2),a1              * routine that handles all real
            move.l          ds_SysBase(a6),a6               * packet outputting.
            SYS             Cause
            move.l          (sp)+,a6

            movem.l         (sp)+,a2/a6
            rts


            end




