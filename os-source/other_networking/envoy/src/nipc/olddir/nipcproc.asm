**
** $Header: ENVOY:src/nipc/RCS/nipcproc.asm,v 1.5 92/04/05 18:15:26 kcd Exp $
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
         xdef _NewList
         xdef @getnipcbase
         xdef __CXD33
         xdef __CXM33

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
_NewList NEWLIST  a0
         rts

__CXD33:
		move.l	a6,-(sp)
		move.l	nb_UtilityBase(a6),a6
		jsr		_LVOSDivMod32(a6)
		movea.l	(sp)+,a6
		rts

__CXM33:
		move.l	a6,-(sp)
		move.l	nb_UtilityBase(a6),a6
		jsr		_LVOSMult32(a6)
		movea.l	(sp)+,a6
		rts

@getnipcbase:
        move.l  a6,d0
        rts



        end
