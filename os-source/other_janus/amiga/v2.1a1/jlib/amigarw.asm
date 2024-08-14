* ******************************************************************************
* 
* amigarw.asm -- read and write amiga memory from the pc
* 
* Copyright (c) 1986, Commodore Amiga Inc.  All rights reserved
* 
* Date       Name               Description
* ---------  -----------        -----------------------------------------------
* 15-Jul-88  -RJ                Changed all files to work with new includes
* 
* ******************************************************************************


      INCLUDE "assembly.i"

      NOLIST
      INCLUDE "exec/types.i"
      INCLUDE "exec/alerts.i"

      INCLUDE "janus/janusbase.i"
      INCLUDE "janus/janusvar.i"
      INCLUDE "janus/janusreg.i"
      INCLUDE "janus/services.i"
      INCLUDE "janus/memrw.i"

      INCLUDE "asmsupp.i"
      LIST

      XDEF   AmigaRW
      XDEF   JBCopy


      XLIB   GetParamOffset
      XLIB   SendJanusInt
      XLIB   JanusMemBase
      XLIB   Debug
      XLIB   Alert


; amigarw (januslib)(a1)
;
; This routine is the handler for the JSERV_READAMIGA interrupt.
; It reads the parameter block and does appropriate IO.
;
AmigaRW:
      movem.l a2/a3/a6,-(sp)
      move.l   a1,a6


      moveq   #JSERV_READAMIGA,d0
      CALLSYS GetParamOffset
      btst   #0,d0
      beq.s   AmigaRW_getbase

      ;------ we should ALWAYS have a param block...
      ALERT   $7ffffffa
      LINKEXE Debug
      bra   AmigaRW_end

AmigaRW_getbase:
      ;------ get the base of the parameter block
      move.l   d0,a2
      add.l   jb_ParamMem(a6),a2
      add.l   #WordAccessOffset,a2

   IFGE   INFOLEVEL-30
   move.l   a2,-(sp)
   PUTMSG   30,<'%s/arw_getbase: param @ 0x%lx'>
   addq.l   #4,sp
   ENDC

      ;------ mark the block as being in use
      move.w   #MRWS_INPROGRESS,mrw_Status(a2)

      ;------ get the arguments
      move.l   #MEMF_BUFFER,d0
      CALLSYS JanusMemBase
      move.l   d0,a0
      CLEAR   d0
      move.w   mrw_Buffer(a2),d0
      add.l   d0,a0

      move.l   mrw_Address(a2),a1
      move.w   mrw_Count(a2),d0

      ;------ check that the command is in range
      move.w   mrw_Command(a2),d1
      cmp.w   #(arw_CmdTabEnd-arw_CmdTab)>>1,d1
      bhs.s   AmigaRW_bad

      ;------ vector to the right command
      add.w   d1,d1
      move.w   arw_CmdTab(pc,d1.w),d1

   IFGE   INFOLEVEL-90
   movem.l d0/d1/a0/a1/a3,-(sp)
   clr.w   4(sp)          ; clear out upper word of d1
   PUTMSG   40,<'%s/call: cnt %ld, cmd %ld, buf 0x%lx, adr 0x%lx, pc 0x%lx'>
   movem.l (sp)+,d0/d1/a0/a1/a3
   LINKEXE Debug
   ENDC


      ;------ the args: (buffer:a0, address:a1, cnt:d0)
      jsr   arw_CmdTab(pc,d1.w)

   PUTMSG   1,<'%s/arw_postcall'>

AmigaRW_reply:
      move.w   d0,mrw_Status(a2)
      moveq   #JSERV_READAMIGA,d0
      CALLSYS SendJanusInt

AmigaRW_end:

      movem.l (sp)+,a2/a3/a6
      rts

AmigaRW_bad:
      bsr.s   arw_bad
      bra.s   AmigaRW_reply



; vector table for processing commands
arw_CmdTab:
      dc.w   arw_nop-arw_CmdTab       ; do nothing
      dc.w   arw_read-arw_CmdTab       ; read from amiga to 808x
      dc.w   arw_write-arw_CmdTab       ; write from 808x to amiga
      dc.w   arw_bad-arw_CmdTab       ; readio -- only for 808x
      dc.w   arw_bad-arw_CmdTab       ; writeio -- only for 808x
      dc.w   arw_readwrite-arw_CmdTab    ; write to amiga,
                      ;   then read back
arw_CmdTabEnd:


arw_nop:
      moveq   #MRWS_OK,d0
      rts


arw_bad:
      moveq   #MRWS_BADCMD,d0
      rts


arw_read:
      exg   a0,a1
      ;------ fall through

arw_write:
      bsr.s   JBCopy
      moveq   #MRWS_OK,d0
      rts


arw_readwrite:
      movem.l d0/a0/a1,-(sp)
      exg   a0,a1
      bsr.s   JBCopy
      movem.l (sp)+,d0/a0/a1
      bsr.s   JBCopy
      moveq   #MRWS_OK,d0
      rts



JBCopy:
******* janus.library/JBCopy ************************************************
*
*   NAME   
*     JBCopy -- Copy Janus memory as efficiently as possible
*
*   SYNOPSIS
*     VOID JBCopy(Source, Destination, Length);
*                 A0      A1           D0
*
*     VOID JBCopy(APTR,APTR,ULONG);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     Copies an arbitrarily-aligned block of Janus memory as efficiently 
*     as possible.  
*
*   INPUTS
*     Source      = address of start of memory block to be copied
*     Destination = address of destination of memory block to be copied
*     Length      = length of memory block to be copied
*
*   RESULT
*     None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     None
*
*****************************************************************************
*
*/

      ;------ check for < eight bytes
      cmp.l   #8,d0
      blo.s   bytecopy

      ;------ if a0 is odd copy one byte
      move.l   a0,d1
      btst   #0,d1
      beq.s   bcopy_a1check

      move.b   (a0)+,(a1)+
      subq.l   #1,d0

bcopy_a1check:
      ;------ if a1 is on an odd boundary we MUST copy byte by byte
      move.l   a1,d1
      btst   #0,d1
      bne.s   bytecopy

      ;------ OK. we are word aligned.  copy as many longwords as
      ;------ possible.  Then copy bytes.
      move.l   d0,d1
      and.b   #$fc,d1
      sub.l   d1,d0          ; uncopyied residue
      lsr.l   #2,d1          ; # of longwords to copy

      ;------ d0 is between 0 and 3.   Store the low word of
      ;------ d1 (the # of longwords to copy) in the low word
      ;------ of d0, then swap d1 (making high word available)

      swap   d0
      move.w   d1,d0
      swap   d1
      bra.s   bcopy_entry

bcopy_loop:
      move.l   (a0)+,(a1)+
bcopy_entry
      dbra   d0,bcopy_loop

      ;------ check low word of d1 to see if there is more to do
      dbra   d1,bcopy_loop

      ;------ all done with longwords.  restore d0
      clr.w   d0
      swap   d0

      ;------ and fall through to bytecopy

bytecopy:   ; ( source, dest, count )(a0/a1,d0)

      tst.l   d0
      beq.s   bytecopy_end

      move.w   d0,d1         ; d1 has low word of cound
      swap   d0         ; d0 has high word of count

      bra.s   bytecopy_entry

bytecopy_loop:
      move.b   (a0)+,(a1)+
bytecopy_entry:
      dbra   d1,bytecopy_loop
      dbra   d0,bytecopy_loop


bytecopy_end:
      rts

   END


