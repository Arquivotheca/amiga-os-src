*
* $Id: hardstart.asm,v 1.1 93/02/23 08:35:40 jerryh Exp Locker: jerryh $
*
* $Log:	hardstart.asm,v $
* Revision 1.1  93/02/23  08:35:40  jerryh
* Initial revision
* 
* Revision 37.6  92/03/10  15:16:40  darren
* Perform reset for all cards, even if its not a diag card.  Buzz
* loop for at least 10us with reset asserted for at least 10us to meet
* PCMCIA spec.  The idea is to make sure all cards which might assert
* a card interrupt stop doing so before exec enables interrupts.
*
* Revision 40.1  93/02/23  08:26:21  jerry
* strip down for AmigaCD and turn off guru
*
* Revision 37.5  92/02/25  11:36:24  andy
* assert reset on power on
* 
* Revision 37.4  92/02/14  16:44:21  andy
* fixed include file case
* 
* Revision 37.3  92/02/13  16:28:39  mks
* Yet another changed to the CDTV Magic HardStart routine...
* 
* Revision 37.2  92/02/13  15:35:50  mks
* Checked out for testing and while testing, cleaned it up somewhat...
*
* Revision 37.1  92/02/11  13:22:20  mks
* Added the magic locations needed for Version and Revision poking...
*
* Revision 37.0  92/01/27  13:30:36  mks
* Initial release for CDTV-CR
*
*******************************************************************************
*
* CDTV-CR Hardware Startup Magic
*
* This file contains the code that must be at the start of the CDTV-CR
* ROM at $00F00000 address space.  This code will be run by exec just after
* the hardware settle delay has been completed.
*
* This code has some very strict limitations that must be followed
* if it is to work correctly:
*
* 1)  No stack may be used.  This is why this is not a jsr routine...
* 2)  Only certain registers are available for use
* 3)  You are in supervisor mode and may have certain unknown CPU state
*     so you must not be too worried about this or rely on it.
*
*******************************************************************************
*
DIAG_CART   EQU $1111       ; Hard coded from romconstants.i
CDTV_CART   EQU $1115
*                           ; This can *NEVER* change.
*
* Ok, so life is a bit tricky here...  The $00F00000 space is the
* space that is overlayed onto the reset vectors.  In order to
* make this all work we need to have this reset point at the
* real 2.0 reset.  However, in order for the DIAG CART trick to work
* we need to get the right values here.  Anyway, it turns out that
* all of this works out by luck to have 2 bytes available to do a
* short branch out to the real DIAG CART code and still have the reset
* pointer (at location $4) point to the real ROM.
*
* One of the reasons for this is that $00F00000 space is not overlayed
* if the FLASH ROM is used but it is overlayed if the full 1-meg ROM
* is used.  For this reason we need to do things a bit out of the ordinary.
*
* Note:  This module *ASSUMES* that the EXEC/Kickstart ROMs contain the
* boot JUMP instruction at $00F80002...  This means that this version does
* not work in 1.3 ROMs.  Since this code is only needed for CDTV-CR, this
* currently is not a problem.  If it ever becomes a problem we may need to
* use a more complex reset method...
*
HardStart:  dc.w    DIAG_CART   ; Mark as DIAG CART (cool trick)
            bra.s   Diag_Code   ; Cute trick to get to right spot
            dc.l    $00F80002   ; Point at the real ROM start...
*
*******************************************************************************
*
* Magic locations for the version numbers to be poked...
*
            dc.l    $0000FFFF   ; Diagnostic test value...
            dc.w    0           ; Version filled in by build process
            dc.w    0           ; Revision filled in by build process
            dc.l    $FFFF0000   ; One more diagnostic...
*
*******************************************************************************
*
*
* Ok, so we now are running code after the real EXEC got through his magic.
* At this point, we got control of the CPU with a number of things set up
* so that we can return to EXEC.
*
*******************************************************************************
*
* Now, for a few words from our sponsor:
*
            dc.b    'AMIGA ROM Operating System/AmigaCD ROM',10
            dc.b    'Copyright � 1985-1993 Commodore-Amiga, Inc.',10
            dc.b    'All Rights Reserved.',10,0
            cnop    0,4
*
*******************************************************************************
*
Diag_Code:
            ; early startup...
            jmp    (a5)
*
*******************************************************************************
*
* hardstart version string...
*
        include 'hardstart_rev.i'
        VSTRING
*
*******************************************************************************
*
            end
