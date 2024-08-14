**
** $Header: APPN:src/nipc/RCS/nipcproc.asm,v 1.8 93/03/02 16:23:03 gregm Exp $
**

         include "exec/types.i"
         include "exec/ports.i"
         include "exec/lists.i"
         include "dos/dosextens.i"
                 include "nipcbase.i"

         xref _AbsExecBase
         xref _LVOGetMsg
         xref _LVOReplyMsg
         xref _LVOWaitPort
         xref _LVOFindTask
         xref _LVOOpenLibrary
         xref _LVOCloseLibrary
         xref _LibraryProcess
         xref _LVOSDivMod32
         xref _LVOSMult32

         xdef @EntryPoint
         xdef @NewList
         xdef @getnipcbase
         xdef @stccpy
         xdef _makefastrand
         xdef _DivSASSucks

;         xdef __CXD33
;         xdef __CXM33

@EntryPoint:
         movea.l  _AbsExecBase,a6
         suba.l   a1,a1
         jsr      _LVOFindTask(a6)
         movea.l  d0,a2
         lea.l    pr_MsgPort(a2),a0
         jsr      _LVOWaitPort(a6)
         lea.l    pr_MsgPort(a2),a0
         jsr      _LVOGetMsg(a6)
         movea.l  d0,a1
         movea.l  MN_SIZE(a1),a3
         movea.l  a3,a6
         jmp      _LibraryProcess

         cnop   0,4

@NewList NEWLIST  a0
         rts

;-----------------------------------------------------------------------
; stccpy        - Smaller version of the copy routine...
;  a0 - destination
;  a1 - source
;  d0 - destination length

@stccpy:        subq.l  #1,d0                   ; One less than given...
                bmi.s   2$                      ; if we end up with no bytes!
1$:             move.b  (a1)+,(a0)+             ; Copy the data...
                dbeq.s  d0,1$                   ; Loop for max or null...
                clr.b   -(a0)                   ; Make sure NULL terminated
2$:             rts

;__CXD33:
;               move.l  a6,-(sp)
;               move.l  nb_UtilityBase(a6),a6
;               jsr             _LVOSDivMod32(a6)
;               movea.l (sp)+,a6
;               rts

;__CXM33:
;               move.l  a6,-(sp)
;               move.l  nb_UtilityBase(a6),a6
;               jsr             _LVOSMult32(a6)
;               movea.l (sp)+,a6
;               rts

@getnipcbase:
        move.l  a6,d0
        rts

_makefastrand:
        mulu.w  #$1efd,d0
        add.w   #$dff,d0
        rts

_DivSASSucks:
        divu    d1,d0
        ext.l   d0
        rts

        end
